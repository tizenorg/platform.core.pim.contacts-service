/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <image_util.h>
#include <unicode/ulocdata.h>
#include <unicode/uset.h>
#include <unicode/ustring.h>
#include <libexif/exif-data.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_utils.h"
#include "ctsvc_mutex.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_schema.h"
#include "ctsvc_notification.h"
#include "ctsvc_struct.h"
#include "ctsvc_normalize.h"
#include "ctsvc_localize.h"
#include "ctsvc_localize_utils.h"
#include "ctsvc_setting.h"
#include "ctsvc_notify.h"

#ifdef _CONTACTS_IPC_SERVER
#include "ctsvc_server_change_subject.h"
#endif

static __thread int transaction_count = 0;
static __thread int transaction_ver = 0;
static __thread bool version_up = false;

#define CTS_SECURITY_IMAGE_PERMISSION 0440
#define CTS_IMAGE_ENCODE_QUALITY	50

#define CTS_COMMIT_TRY_MAX 500000 // For 3second

int ctsvc_begin_trans(void)
{
	int ret = -1, progress;

	if (transaction_count <= 0) {
		ret = ctsvc_query_exec("BEGIN IMMEDIATE TRANSACTION");
		progress = 100000;
		while (CONTACTS_ERROR_DB == ret && progress < CTS_COMMIT_TRY_MAX) {
			usleep(progress);
			ret = ctsvc_query_exec("BEGIN IMMEDIATE TRANSACTION");
			progress *= 2;
		}
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_query_exec() Fail(%d)", ret);
			return ret;
		}

		transaction_count = 0;

		const char *query = "SELECT ver FROM "CTS_TABLE_VERSION;
		ret = ctsvc_query_get_first_int_result(query, &transaction_ver);
		version_up = false;
	}
	transaction_count++;
	INFO("transaction_count : %d.", transaction_count);
	return CONTACTS_ERROR_NONE;
}

int ctsvc_end_trans(bool is_success)
{
	int ret = -1, progress;
	char query[CTS_SQL_MIN_LEN] = {0};

	transaction_count--;
	INFO("%s, transaction_count : %d", is_success?"True": "False",  transaction_count);

	if (0 != transaction_count) {
		CTS_DBG("contact transaction_count : %d.", transaction_count);
		return CONTACTS_ERROR_NONE;
	}

	if (false == is_success) {
		ctsvc_nofitication_cancel();
#ifdef _CONTACTS_IPC_SERVER
		ctsvc_change_subject_clear_changed_info();
#endif
		ret = ctsvc_query_exec("ROLLBACK TRANSACTION");

		return CONTACTS_ERROR_NONE;
	}

	if (version_up) {
		transaction_ver++;
		snprintf(query, sizeof(query), "UPDATE %s SET ver = %d",
				CTS_TABLE_VERSION, transaction_ver);
		ret = ctsvc_query_exec(query);
		WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_query_exec(version up) Fail(%d)", ret);
	}

	INFO("start commit");
	progress = 100000;
	ret = ctsvc_query_exec("COMMIT TRANSACTION");
	while (CONTACTS_ERROR_DB == ret && progress<CTS_COMMIT_TRY_MAX) {
		usleep(progress);
		ret = ctsvc_query_exec("COMMIT TRANSACTION");
		progress *= 2;
	}
	INFO("%s", (CONTACTS_ERROR_NONE == ret)?"commit": "rollback");

	if (CONTACTS_ERROR_NONE != ret) {
		int tmp_ret;
		CTS_ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_nofitication_cancel();
#ifdef _CONTACTS_IPC_SERVER
		ctsvc_change_subject_clear_changed_info();
#endif
		tmp_ret = ctsvc_query_exec("ROLLBACK TRANSACTION");
		WARN_IF(CONTACTS_ERROR_NONE != tmp_ret, "ctsvc_query_exec(ROLLBACK) Fail(%d)", tmp_ret);
		return ret;
	}

	ctsvc_notification_send();
#ifdef _CONTACTS_IPC_SERVER
	ctsvc_change_subject_publish_changed_info();
#endif

	CTS_DBG("Transaction shut down! : (%d)\n", transaction_ver);

	return CONTACTS_ERROR_NONE;
}

const char* ctsvc_get_display_column(void)
{
	contacts_name_display_order_e order;

	contacts_setting_get_name_display_order(&order);
	if (CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST == order)
		return "display_name";
	else
		return "reverse_display_name";
}

const char* ctsvc_get_sort_name_column(void)
{
	contacts_name_sorting_order_e order;

	contacts_setting_get_name_sorting_order(&order);
	if (CONTACTS_NAME_SORTING_ORDER_FIRSTLAST == order)
		return "sort_name, display_name_language";
	else
		return "reverse_sort_name, reverse_display_name_language";
}

const char* ctsvc_get_sort_column(void)
{
	contacts_name_sorting_order_e order;

	contacts_setting_get_name_sorting_order(&order);
	if (CONTACTS_NAME_SORTING_ORDER_FIRSTLAST == order)
		return "display_name_language, sortkey";
	else
		return "reverse_display_name_language, reverse_sortkey";
}

void ctsvc_utils_make_image_file_name(int parent_id, int id, char *src_img, char *dest, int dest_size)
{
	char *ext;
	char *temp;
	char *lower_ext;

	ext = strrchr(src_img, '.');
	if (NULL == ext || strchr(ext, '/'))
		ext = "";

	lower_ext = strdup(ext);
	temp = lower_ext;
	while (*temp) {
		*temp = tolower(*temp);
		temp++;
	}

	if (0 < parent_id)
		snprintf(dest, dest_size, "%d_%d%s", parent_id, id, lower_ext);
	else
		snprintf(dest, dest_size, "%d%s", id, ext);
	free(lower_ext);
}

static inline bool ctsvc_check_available_image_space(void) {
	int ret;
	struct statfs buf;
	long long size;
	ret = statfs(CTSVC_NOTI_IMG_REPERTORY, &buf);

	RETVM_IF(ret!=0, false, "statfs Fail(%d)", ret);

	size = (long long)buf.f_bavail * (buf.f_bsize);

	if (1024*1024 < size) // if available space to copy a image is larger than 1M
		return true;
	return false;
}

static image_util_rotation_e __ctsvc_get_rotation_info(const char *path)
{
	ExifData *ed = NULL;
	ExifEntry *entry;
	image_util_rotation_e rotation = IMAGE_UTIL_ROTATION_NONE;
	int orientation = 0;

	ed = exif_data_new_from_file(path);
	if (ed == NULL) {
		CTS_ERR("exif_data_new_from_file : ExifData is NULL");
		return IMAGE_UTIL_ROTATION_NONE;
	}

	entry = exif_data_get_entry(ed, EXIF_TAG_ORIENTATION);
	if (entry) {
		ExifByteOrder mByteOrder = exif_data_get_byte_order(ed);
		orientation = (int)exif_get_short(entry->data, mByteOrder);
		if (orientation < 0 || 8 < orientation)
			orientation = 0;
	}

	if (ed)
		exif_data_unref(ed);

	switch(orientation) {
	case 1:	// Top-left
		rotation = IMAGE_UTIL_ROTATION_NONE;
		break;
	case 2:	// Top-right
		rotation = IMAGE_UTIL_ROTATION_FLIP_HORZ;
		break;
	case 3:	// Bottom-right
		rotation = IMAGE_UTIL_ROTATION_180;
		break;
	case 4:	// Bottom-left
		rotation = IMAGE_UTIL_ROTATION_FLIP_VERT;
		break;
	case 6: 	// Right-top
		rotation = IMAGE_UTIL_ROTATION_90;
		break;
	case 8:	// Left-bottom
		rotation = IMAGE_UTIL_ROTATION_270;
		break;
	case 5:	// Left-top
	case 7:	// Right-bottom
	case 0:
	default:
		break;
	};

	return rotation;
}

static int image_size = 480;

typedef struct {
	const char *src;
	const char *dest;
	int ret;
}image_info;

static bool __ctsvc_image_util_supported_jpeg_colorspace_cb(image_util_colorspace_e colorspace, void *user_data)
{
	image_info *info = (image_info*)user_data;
	image_util_error_e ret;
	int width = 0, height = 0;
	unsigned int size_decode = 0;
	int resized_width, resized_height;
	unsigned char * img_target = 0;
	unsigned char * img_source = 0;
	int dest_fd;
	image_util_rotation_e rotation;

	// temporary code
	if (colorspace == IMAGE_UTIL_COLORSPACE_YV12 || colorspace == IMAGE_UTIL_COLORSPACE_I420) {
		info->ret = CONTACTS_ERROR_SYSTEM;
		return true;
	}

	rotation = __ctsvc_get_rotation_info(info->src);

	// load jpeg sample file
	CTS_DBG("colorspace %d src : %s, dest : %s", colorspace, info->src, info->dest);
	ret = image_util_decode_jpeg(info->src, colorspace, &img_source, &width, &height, &size_decode);
	if (ret!=IMAGE_UTIL_ERROR_NONE) {
		info->ret = CONTACTS_ERROR_SYSTEM;
		return true;
	}

#if 0
	if (0>image_size) {
		int w,h;
		ecore_x_window_size_get(
				ecore_x_window_root_get(ecore_x_window_focus_get())
				, &w, &h);

		if (w>h)
			image_size = h;
		else
			image_size = w;

	}
#endif

	if (image_size < width || image_size < height) {
		// image resize
		if (image_size<=0 || width <=0 || height <= 0) {
			free(img_source);
			CTS_ERR("image size error(%d)", image_size);
			info->ret = CONTACTS_ERROR_SYSTEM;
			return false;
		}

		if (height < width) {
			resized_width = image_size;
			resized_height = height*image_size/width;
		}
		else {
			resized_height = image_size;
			resized_width = width*image_size/height;
		}

		if (resized_height%8)
			resized_height += 8 - (resized_height%8);
		if (resized_width%8)
			resized_width += 8 - (resized_width%8);

		CTS_DBG("size(%d, %d) -> resize(%d,%d)", width, height, resized_width, resized_height);

		image_util_calculate_buffer_size(resized_width, resized_height, colorspace, &size_decode);

		img_target = malloc(size_decode);

		// do resize
		ret = image_util_resize(img_target, &resized_width, &resized_height,
				img_source, width, height, colorspace);
		if (ret!=IMAGE_UTIL_ERROR_NONE) {
			CTS_ERR("image_util_resize Fail(%d)", ret);
			free(img_target);
			free(img_source);
			info->ret = CONTACTS_ERROR_SYSTEM;
			return false;
		}
		free(img_source);
	}
	else {
		resized_width = width;
		resized_height = height;
		img_target = img_source;
	}

	// image rotation
	if (IMAGE_UTIL_ROTATION_NONE != rotation) {
		int rotated_width, rotated_height;
		unsigned char *img_rotate = 0;
		img_rotate = malloc(size_decode);
		ret= image_util_rotate(img_rotate, &rotated_width, &rotated_height,
					rotation, img_target, resized_width, resized_height, colorspace);
		free(img_target);
		if (IMAGE_UTIL_ERROR_NONE != ret) {
			CTS_ERR("image_util_rotate Fail(%d)", ret);
			info->ret = CONTACTS_ERROR_SYSTEM;
			free(img_rotate);
			return false;
		}
		resized_width = rotated_width;
		resized_height = rotated_height;
		img_target = img_rotate;
	}

	// image encode
	ret = image_util_encode_jpeg(img_target, resized_width, resized_height, colorspace, CTS_IMAGE_ENCODE_QUALITY, info->dest);
	free(img_target);
	if (ret != IMAGE_UTIL_ERROR_NONE) {
		CTS_ERR("image_util_encode_jpeg Fail(%d)", ret);
		info->ret = CONTACTS_ERROR_SYSTEM;
		return false;
	}

	dest_fd = open(info->dest, O_RDONLY);
	if (dest_fd < 0) {
		CTS_ERR("System : Open Fail(%d)", errno);
		info->ret = CONTACTS_ERROR_SYSTEM;
		return false;
	}

	ret = fchown(dest_fd, getuid(), CTS_SECURITY_FILE_GROUP);
	if (0 != ret) {
		CTS_ERR("fchown Fail(%d)", errno);
		info->ret = CONTACTS_ERROR_SYSTEM;
		close(dest_fd);
		return false;
	}

	ret = fchmod(dest_fd, CTS_SECURITY_IMAGE_PERMISSION);
	if (0 != ret) {
		CTS_ERR("fchmod Fail(%d)", errno);
		info->ret = CONTACTS_ERROR_SYSTEM;
		close(dest_fd);
		return false;
	}
	close(dest_fd);

	info->ret = CONTACTS_ERROR_NONE;
	return false;
}

static int __ctsvc_resize_and_copy_image(const char *src, const char *dest)
{
	int ret;
	image_info info = {.src = src, .dest = dest, ret = CONTACTS_ERROR_SYSTEM};

	ret = image_util_foreach_supported_jpeg_colorspace(__ctsvc_image_util_supported_jpeg_colorspace_cb, &info);

	if (ret != IMAGE_UTIL_ERROR_NONE)
		return CONTACTS_ERROR_SYSTEM;

	return info.ret;
}

#define CTSVC_COPY_SIZE_MAX 4096
int ctsvc_utils_copy_image(const char *dir, const char *src, const char *file)
{
	int ret;
	int size;
	int src_fd, dest_fd;
	char buf[CTSVC_COPY_SIZE_MAX] = {0};

	if (NULL == file || *file == '\0')
		return CONTACTS_ERROR_INVALID_PARAMETER;

	char dest[strlen(dir) + strlen(file) + 2];
	snprintf(dest, sizeof(dest), "%s/%s", dir, file);

	if (false == ctsvc_check_available_image_space())
		return CONTACTS_ERROR_FILE_NO_SPACE;

	ret = __ctsvc_resize_and_copy_image(src, dest);
	if (CONTACTS_ERROR_NONE == ret) {
		return ret;
	}
	else
		CTS_ERR("__ctsvc_resize_and_copy_image Fail(%d)", ret);

	src_fd = open(src, O_RDONLY);
	RETVM_IF(src_fd < 0, CONTACTS_ERROR_SYSTEM, "System : Open(src:%s) Fail(%d)", src, errno);
	dest_fd = open(dest, O_WRONLY|O_CREAT|O_TRUNC, 0660);
	if (dest_fd < 0) {
		CTS_ERR("Open Fail(%d)", errno);
		close(src_fd);
		return CONTACTS_ERROR_SYSTEM;
	}

	while (0 < (size = read(src_fd, buf, CTSVC_COPY_SIZE_MAX))) {
		ret = write(dest_fd, buf, size);
		if (ret <= 0) {
			if (EINTR == errno)
				continue;
			else {
				CTS_ERR("write() Fail(%d)", errno);
				if (ENOSPC == errno)
					ret = CONTACTS_ERROR_FILE_NO_SPACE;	// No space
				else
					ret = CONTACTS_ERROR_SYSTEM;			// IO error
				close(src_fd);
				close(dest_fd);
				unlink(dest);
				return ret;
			}
		}
	}

	ret = fchown(dest_fd, getuid(), CTS_SECURITY_FILE_GROUP);
	if (0 != ret) {
		CTS_ERR("fchown() Fail(%d)", ret);
	}
	ret = fchmod(dest_fd, CTS_SECURITY_IMAGE_PERMISSION);
	if (0 != ret) {
		CTS_ERR("fchmod() Fail(%d)", ret);
	}
	close(src_fd);
	close(dest_fd);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_next_ver(void)
{
	const char *query;
	int version;
	int ret;

	if (0 < transaction_count) {
		version_up = true;
		return transaction_ver + 1;
	}

	query = "SELECT ver FROM "CTS_TABLE_VERSION;
	ret = ctsvc_query_get_first_int_result(query, &version);

	// In this case, contacts-service already works abnormally.
	if (CONTACTS_ERROR_NONE != ret)
		CTS_ERR("ctsvc_query_get_first_int_result : get version error(%d)", ret);

	return (1 + version);
}

int ctsvc_get_current_version(int* out_current_version) {
	if (transaction_count <= 0) {
		int ret;
		int version = 0;
		const char *query = "SELECT ver FROM "CTS_TABLE_VERSION;
		ret = ctsvc_query_get_first_int_result(query, &version);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_get_first_int_result() Fail(%d)", ret);
		*out_current_version = version;
	}
	else
		*out_current_version = transaction_ver;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_transaction_ver(void)
{
	return transaction_ver;
}

int SAFE_SNPRINTF(char **buf, int *buf_size, int len, const char *src)
{
	int remain;
	int temp_len;

	if (len < 0)
		return -1;

	remain = *buf_size - len;
	if (strlen(src) + 1 < remain) {
		temp_len = snprintf((*buf)+len, remain, "%s", src);
		return temp_len;
	}
	else {
		char *temp;
		while (1) {
			temp = realloc(*buf, *buf_size*2);
			if (NULL == temp)
				return -1;
			*buf = temp;
			*buf_size = *buf_size * 2;
			remain = *buf_size - len;
			if (strlen(src) + 1 < remain)
				break;
		}
		temp_len = snprintf((*buf)+len, remain, "%s", src);
		return temp_len;
	}
}


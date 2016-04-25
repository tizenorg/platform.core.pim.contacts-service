/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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
#include "ctsvc_db_utils.h"
#include "ctsvc_mutex.h"
#include "ctsvc_db_sqlite.h"
#include "ctsvc_db_schema.h"
#include "ctsvc_notification.h"
#include "ctsvc_struct.h"
#include "ctsvc_normalize.h"
#include "ctsvc_localize.h"
#include "ctsvc_localize_utils.h"
#include "ctsvc_server_setting.h"
#include "ctsvc_notify.h"
#include "ctsvc_image_util.h"

#ifdef _CONTACTS_IPC_SERVER
#include "ctsvc_server_change_subject.h"
#endif

static __thread int transaction_count = 0;
static __thread int transaction_ver = 0;
static __thread bool version_up = false;

#define CTS_SECURITY_IMAGE_PERMISSION 0440
#define CTS_COMMIT_TRY_MAX 500000 /* For 3second */

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
			ERR("ctsvc_query_exec() Fail(%d)", ret);
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
	INFO("%s, transaction_count : %d", is_success ? "True" : "False",  transaction_count);

	if (0 != transaction_count) {
		DBG("contact transaction_count : %d.", transaction_count);
		return CONTACTS_ERROR_NONE;
	}

	if (false == is_success) {
		ctsvc_nofitication_cancel();
		ctsvc_change_subject_clear_changed_info();
		ret = ctsvc_query_exec("ROLLBACK TRANSACTION");

		return CONTACTS_ERROR_NONE;
	}

	if (version_up) {
		transaction_ver++;
		snprintf(query, sizeof(query), "UPDATE %s SET ver = %d",
				CTS_TABLE_VERSION, transaction_ver);
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret)
			ERR("ctsvc_query_exec(version up) Fail(%d)", ret);
	}

	INFO("start commit");
	progress = 100000;
	ret = ctsvc_query_exec("COMMIT TRANSACTION");
	while (CONTACTS_ERROR_DB == ret && progress < CTS_COMMIT_TRY_MAX) {
		usleep(progress);
		ret = ctsvc_query_exec("COMMIT TRANSACTION");
		progress *= 2;
	}
	INFO("%s", (CONTACTS_ERROR_NONE == ret) ? "commit" : "rollback");

	if (CONTACTS_ERROR_NONE != ret) {
		int tmp_ret;
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_nofitication_cancel();
		ctsvc_change_subject_clear_changed_info();

		tmp_ret = ctsvc_query_exec("ROLLBACK TRANSACTION");
		if (CONTACTS_ERROR_NONE != tmp_ret)
			ERR("ctsvc_query_exec(ROLLBACK) Fail(%d)", tmp_ret);

		return ret;
	}

	ctsvc_notification_send();
	ctsvc_change_subject_publish_changed_info();

	DBG("Transaction shut down! : (%d)\n", transaction_ver);

	return CONTACTS_ERROR_NONE;
}

const char* ctsvc_get_display_column(void)
{
	contacts_name_display_order_e order;

	ctsvc_setting_get_name_display_order(&order);
	if (CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST == order)
		return "display_name";
	else
		return "reverse_display_name";
}

const char* ctsvc_get_sort_name_column(void)
{
	contacts_name_sorting_order_e order;

	ctsvc_setting_get_name_sorting_order(&order);
	if (CONTACTS_NAME_SORTING_ORDER_FIRSTLAST == order)
		return "sort_name, display_name_language";
	else
		return "reverse_sort_name, reverse_display_name_language";
}

const char* ctsvc_get_sort_column(void)
{
	contacts_name_sorting_order_e order;

	ctsvc_setting_get_name_sorting_order(&order);
	if (CONTACTS_NAME_SORTING_ORDER_FIRSTLAST == order)
		return "display_name_language, sortkey";
	else
		return "reverse_display_name_language, reverse_sortkey";
}

void ctsvc_utils_make_image_file_name(int parent_id, int id, char *src_img, char *dest,
		int dest_size)
{
	char *ext;
	char *temp;
	char *lower_ext;

	ext = strrchr(src_img, '.');
	if (NULL == ext || strchr(ext, '/'))
		ext = "";

	lower_ext = strdup(ext);
	if (NULL == lower_ext) {
		ERR("strdup() Fail");
		return;
	}
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

static inline bool _ctsvc_check_available_image_space(int need_size)
{
	int ret;
	struct statfs buf;
	long long size;

	ret = statfs(CTSVC_NOTI_IMG_REPERTORY, &buf);
	RETVM_IF(ret != 0, false, "statfs() Fail(%d)", ret);

	size = (long long)buf.f_bavail * (buf.f_bsize);

	if (need_size < size)  /* if available space to copy a image is larger than need_size */
		return true;
	return false;
}

static image_util_rotation_e _ctsvc_image_get_rotation_info(const char *path)
{
	ExifData *ed = NULL;
	ExifEntry *entry;
	image_util_rotation_e rotation = IMAGE_UTIL_ROTATION_NONE;
	int orientation = 0;

	ed = exif_data_new_from_file(path);
	if (NULL == ed) {
		ERR("exif_data_new_from_file() Fail");
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

	switch (orientation) {
	case 1: /* Top-left */
		rotation = IMAGE_UTIL_ROTATION_NONE;
		break;
	case 2: /* Top-right */
		rotation = IMAGE_UTIL_ROTATION_FLIP_HORZ;
		break;
	case 3: /* Bottom-right */
		rotation = IMAGE_UTIL_ROTATION_180;
		break;
	case 4: /* Bottom-left */
		rotation = IMAGE_UTIL_ROTATION_FLIP_VERT;
		break;
	case 6: /* Right-top */
		rotation = IMAGE_UTIL_ROTATION_90;
		break;
	case 8: /* Left-bottom */
		rotation = IMAGE_UTIL_ROTATION_270;
		break;
	case 5: /* Left-top */
	case 7: /* Right-bottom */
	case 0:
	default:
		break;
	};

	return rotation;
}


typedef struct {
	const char *src;
	const char *dest;
	int max_size;
	int ret;
} image_info;

static bool _ctsvc_image_util_supported_jpeg_colorspace_cb(
		image_util_colorspace_e colorspace, void *user_data)
{
	int width = 0;
	int height = 0;
	int dest_fd = 0;
	int mimetype = 0;
	uint64_t size = 0;
	void *buffer = NULL;
	void *buffer_temp = NULL;
	int ret;
	image_util_rotation_e rotation;
	image_info *info = user_data;

	ret = ctsvc_image_util_get_mimetype(colorspace, &mimetype);
	if (CONTACTS_ERROR_NONE != ret) {
		info->ret = CONTACTS_ERROR_SYSTEM;
		return true;
	}

	ret = image_util_decode_jpeg(info->src, colorspace, (unsigned char **)&buffer,
			&width, &height, (unsigned int *)&size);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		info->ret = CONTACTS_ERROR_SYSTEM;
		return true;
	}

	rotation = _ctsvc_image_get_rotation_info(info->src);
	if (IMAGE_UTIL_ROTATION_NONE != rotation) { /* need rotate */
		media_format_h fmt;
		media_packet_h packet;

		fmt = ctsvc_image_util_create_media_format(mimetype, width, height);
		if (NULL == fmt) {
			ERR("ctsvc_image_util_create_media_format() Fail");
			info->ret = CONTACTS_ERROR_SYSTEM;
			free(buffer);
			return false;
		}

		packet = ctsvc_image_util_create_media_packet(fmt, buffer, (unsigned int)size);
		if (NULL == packet) {
			ERR("ctsvc_image_util_create_media_packet() Fail");
			media_format_unref(fmt);
			info->ret = CONTACTS_ERROR_SYSTEM;
			free(buffer);
			return false;
		}

		ret = ctsvc_image_util_rotate(packet, rotation, &buffer_temp, &size);

		media_packet_destroy(packet);
		media_format_unref(fmt);

		if (CONTACTS_ERROR_NONE != ret) {
			free(buffer);
			info->ret = CONTACTS_ERROR_SYSTEM;
			return false;
		}

		if (rotation == IMAGE_UTIL_ROTATION_90 || rotation == IMAGE_UTIL_ROTATION_270) {
			int temp = width;
			width = height;
			height = temp;
		}
		free(buffer);
		buffer = buffer_temp;
	}

	if (info->max_size < width || info->max_size < height) { /* need resize */
		int resized_width;
		int resized_height;
		media_format_h fmt;
		media_packet_h packet;

		/* set resize */
		if (width > height) {
			resized_width = info->max_size;
			resized_height = height * info->max_size / width;
		} else {
			resized_height = info->max_size;
			resized_width = width * info->max_size / height;
		}

		if (resized_height % 8)
			resized_height += (8 - (resized_height % 8));

		if (resized_width % 8)
			resized_width += (8 - (resized_width % 8));

		fmt = ctsvc_image_util_create_media_format(mimetype, width, height);
		if (NULL == fmt) {
			ERR("ctsvc_image_util_create_media_format() Fail");
			info->ret = CONTACTS_ERROR_SYSTEM;
			free(buffer);
			return false;
		}

		packet = ctsvc_image_util_create_media_packet(fmt, buffer, (unsigned int)size);
		if (NULL == packet) {
			ERR("ctsvc_image_util_create_media_packet() Fail");
			media_format_unref(fmt);
			info->ret = CONTACTS_ERROR_SYSTEM;
			free(buffer);
			return false;
		}

		ret = ctsvc_image_util_resize(packet, resized_width, resized_height, &buffer_temp,
				&size);

		media_packet_destroy(packet);
		media_format_unref(fmt);

		if (CONTACTS_ERROR_NONE != ret) {
			free(buffer);
			info->ret = -1;
			return false;
		}
		free(buffer);
		buffer = buffer_temp;

		width = resized_width;
		height = resized_height;
	}

	ret = image_util_encode_jpeg(buffer, width, height, colorspace,
			CTSVC_IMAGE_ENCODE_QUALITY, info->dest);
	free(buffer);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		ERR("image_util_encode_jpeg Fail(%d)", ret);
		info->ret = CONTACTS_ERROR_SYSTEM;
		return false;
	}

	dest_fd = open(info->dest, O_RDONLY);
	if (dest_fd < 0) {
		ERR("System : Open Fail(%d)", errno);
		info->ret = CONTACTS_ERROR_SYSTEM;
		return false;
	}

	ret = fchmod(dest_fd, CTS_SECURITY_IMAGE_PERMISSION);
	if (0 != ret) {
		ERR("fchmod Fail(%d)", errno);
		info->ret = CONTACTS_ERROR_SYSTEM;
		close(dest_fd);
		return false;
	}
	close(dest_fd);

	info->ret = CONTACTS_ERROR_NONE;
	return false;
}

static int _ctsvc_image_encode(const char *src, const char *dest, int max_size)
{
	int ret;
	image_info info = {src, dest, max_size, CONTACTS_ERROR_SYSTEM};

	ret = image_util_foreach_supported_jpeg_colorspace(
			_ctsvc_image_util_supported_jpeg_colorspace_cb, &info);

	if (IMAGE_UTIL_ERROR_NONE != ret)
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

	if (false == _ctsvc_check_available_image_space(1204 * 1204)) /*need larger than 1M*/
		return CONTACTS_ERROR_FILE_NO_SPACE;

	ret = _ctsvc_image_encode(src, dest, CTSVC_IMAGE_MAX_SIZE);
	if (CONTACTS_ERROR_NONE == ret)
		return ret;
	else
		ERR("_ctsvc_image_encode Fail(%d)", ret);

	src_fd = open(src, O_RDONLY);
	if (src_fd < 0) {
		ERR("System : Open(%s) Fail(%d)", src, errno);
		return CONTACTS_ERROR_SYSTEM;
	}

	dest_fd = open(dest, O_WRONLY|O_CREAT|O_TRUNC, 0660);
	if (dest_fd < 0) {
		ERR("Open Fail(%d)", errno);
		close(src_fd);
		return CONTACTS_ERROR_SYSTEM;
	}

	while (0 < (size = read(src_fd, buf, CTSVC_COPY_SIZE_MAX))) {
		ret = write(dest_fd, buf, size);
		if (ret <= 0) {
			if (EINTR == errno) {
				continue;
			} else {
				ERR("write() Fail(%d)", errno);
				if (ENOSPC == errno)
					ret = CONTACTS_ERROR_FILE_NO_SPACE; /* No space */
				else
					ret = CONTACTS_ERROR_SYSTEM; /* IO error */
				close(src_fd);
				close(dest_fd);
				unlink(dest);
				return ret;
			}
		}
	}

	ret = fchmod(dest_fd, CTS_SECURITY_IMAGE_PERMISSION);
	if (0 != ret)
		ERR("fchmod() Fail(%d)", ret);

	close(src_fd);
	close(dest_fd);

	return CONTACTS_ERROR_NONE;
}

char* ctsvc_utils_get_thumbnail_path(const char *image_path)
{
	int name_len = 0;
	int full_len = 0;
	char *thumbnail_path = NULL;
	char *ext = NULL;

	RETV_IF(NULL == image_path, NULL);
	RETV_IF(NULL != strstr(image_path, CTSVC_IMAGE_THUMBNAIL_SUFFIX), NULL);

	full_len = strlen(image_path) + strlen(CTSVC_IMAGE_THUMBNAIL_SUFFIX) + 1;
	thumbnail_path = calloc(1, full_len);
	if (NULL == thumbnail_path) {
		ERR("calloc() Fail");
		return NULL;
	}

	ext = strrchr(image_path, '.');
	if (ext) {
		name_len = ext -image_path;
		strncpy(thumbnail_path, image_path, name_len);
		snprintf(thumbnail_path+name_len, full_len-name_len, "%s%s", CTSVC_IMAGE_THUMBNAIL_SUFFIX, ext);
	} else {
		snprintf(thumbnail_path, full_len, "%s%s", image_path, CTSVC_IMAGE_THUMBNAIL_SUFFIX);
	}

	return thumbnail_path;
}

char* ctsvc_utils_get_image_path(const char *thumbnail_path)
{
	int name_len = 0;
	int full_len = 0;
	char *image_path = NULL;
	char *ext = NULL;

	RETV_IF(NULL == thumbnail_path, NULL);
	RETV_IF(NULL == strstr(thumbnail_path, CTSVC_IMAGE_THUMBNAIL_SUFFIX), NULL);

	full_len = strlen(thumbnail_path) - strlen(CTSVC_IMAGE_THUMBNAIL_SUFFIX) + 1;
	image_path = calloc(1, full_len);
	if (NULL == image_path) {
	   ERR("calloc() Fail");
	   return NULL;
	}

	ext = strrchr(thumbnail_path, '.');
	if (ext) {
		name_len = ext -thumbnail_path - strlen(CTSVC_IMAGE_THUMBNAIL_SUFFIX);
		strncpy(image_path, thumbnail_path, name_len);
		snprintf(image_path + name_len, full_len -name_len, "%s", ext);
	} else {
		name_len = strlen(thumbnail_path) - strlen(CTSVC_IMAGE_THUMBNAIL_SUFFIX);
		strncpy(image_path, thumbnail_path, name_len);
	}
	return image_path;
}

char* ctsvc_utils_make_thumbnail(const char *image_path)
{
	int ret;
	char src[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
	char dest[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
	char *thumbnail_path = NULL;

	RETV_IF(NULL == image_path, NULL);
	RETV_IF(NULL != strstr(image_path, CTSVC_IMAGE_THUMBNAIL_SUFFIX), NULL);

	if (false == _ctsvc_check_available_image_space(
			CTSVC_IMAGE_THUMBNAIL_SIZE * CTSVC_IMAGE_THUMBNAIL_SIZE)) {
			ERR("No space to make thumbnail");
			return NULL;
	}

	thumbnail_path = ctsvc_utils_get_thumbnail_path(image_path);
	if (NULL == thumbnail_path) {
		ERR("ctsvc_image_util_get_thumbnail_path() Fail");
		return NULL;
	}

	if (0 == access(dest, F_OK)) {
		DBG("already exist");
		return thumbnail_path;
	}

	snprintf(src, sizeof(src), "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, image_path);
	snprintf(dest, sizeof(dest), "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, thumbnail_path);

	ret = _ctsvc_image_encode(src, dest, CTSVC_IMAGE_THUMBNAIL_SIZE);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("_ctsvc_image_encode() Fail(%d)", ret);
		free(thumbnail_path);
		return NULL;
	}

	return strdup(thumbnail_path);
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

	/* In this case, contacts-service already works abnormally. */
	if (CONTACTS_ERROR_NONE != ret)
		ERR("ctsvc_query_get_first_int_result : get version error(%d)", ret);

	return (1 + version);
}

int ctsvc_get_current_version(int *out_current_version)
{
	if (transaction_count <= 0) {
		int ret;
		int version = 0;
		const char *query = "SELECT ver FROM "CTS_TABLE_VERSION;

		ret = ctsvc_query_get_first_int_result(query, &version);
		if (CONTACTS_ERROR_NONE != ret) {
			ERR("ctsvc_query_get_first_int_result() Fail(%d)", ret);
			return ret;
		}

		*out_current_version = version;
	} else {
		*out_current_version = transaction_ver;
	}
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
	} else {
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


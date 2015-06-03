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

struct image_transform{
	int ret;
	uint64_t size;
	void *buffer;
	GCond cond;
	GMutex mutex;
};

#define CTS_SECURITY_IMAGE_PERMISSION 0440
#define CTS_IMAGE_ENCODE_QUALITY 50
#define CTS_IMAGE_MAX_SIZE 1080
#define CTS_COMMIT_TRY_MAX 500000 /* For 3second */
#define CTS_IMAGE_TRANSFORM_WAIT_TIME 500 * G_TIME_SPAN_MILLISECOND /* 0.5 sec */

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
	if (NULL == lower_ext) {
		CTS_ERR("strdup() Fail");
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

static inline bool ctsvc_check_available_image_space(void) {
	int ret;
	struct statfs buf;
	long long size;
	ret = statfs(CTSVC_NOTI_IMG_REPERTORY, &buf);

	RETVM_IF(ret!=0, false, "statfs Fail(%d)", ret);

	size = (long long)buf.f_bavail * (buf.f_bsize);

	if (1024*1024 < size)  /* if available space to copy a image is larger than 1M */
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
	int ret;
}image_info;

static int _ctsvc_image_get_mimetype(image_util_colorspace_e colorspace,
		int *p_mimetype)
{
	media_format_mimetype_e mimetype;
	switch (colorspace) {
	case IMAGE_UTIL_COLORSPACE_YUV422:
		mimetype = MEDIA_FORMAT_422P;
		break;
	case IMAGE_UTIL_COLORSPACE_NV12:
		mimetype = MEDIA_FORMAT_NV12;
		break;
	case IMAGE_UTIL_COLORSPACE_UYVY:
		mimetype = MEDIA_FORMAT_UYVY;
		break;
	case IMAGE_UTIL_COLORSPACE_YUYV:
		mimetype = MEDIA_FORMAT_YUYV;
		break;
	case IMAGE_UTIL_COLORSPACE_RGB565:
		mimetype = MEDIA_FORMAT_RGB565;
		break;
	case IMAGE_UTIL_COLORSPACE_RGB888:
		mimetype = MEDIA_FORMAT_RGB888;
		break;
	case IMAGE_UTIL_COLORSPACE_ARGB8888:
		mimetype = MEDIA_FORMAT_ARGB;
		break;
	case IMAGE_UTIL_COLORSPACE_RGBA8888:
		mimetype = MEDIA_FORMAT_RGBA;
		break;
	case IMAGE_UTIL_COLORSPACE_NV21:
		mimetype = MEDIA_FORMAT_NV21;
		break;
	case IMAGE_UTIL_COLORSPACE_NV16:
		mimetype = MEDIA_FORMAT_NV16;
		break;
	case IMAGE_UTIL_COLORSPACE_BGRA8888: /* not supported */
	case IMAGE_UTIL_COLORSPACE_BGRX8888: /* not supported */
	case IMAGE_UTIL_COLORSPACE_NV61: /* not supported */
	case IMAGE_UTIL_COLORSPACE_YV12: /* not supported */
	case IMAGE_UTIL_COLORSPACE_I420: /* not supported */
	default:
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	*p_mimetype = mimetype;
	return CONTACTS_ERROR_NONE;
}

static media_format_h _ctsvc_image_create_media_format(int mimetype, int width,
		int height)
{
	int ret;
	media_format_h fmt = NULL;

	ret = media_format_create(&fmt);
	if (MEDIA_FORMAT_ERROR_NONE != ret) {
		CTS_ERR("media_format_create() Fail(%d)", ret);
		return NULL;
	}

	ret = media_format_set_video_mime(fmt, mimetype);
	if (MEDIA_FORMAT_ERROR_NONE != ret) {
		CTS_ERR("media_format_set_video_mime() Fail(%d)", ret);
		media_format_unref(fmt);
		return NULL;
	}

	ret = media_format_set_video_width(fmt, width);
	if (MEDIA_FORMAT_ERROR_NONE != ret) {
		CTS_ERR("media_format_set_video_width() Fail(%d)", ret);
		media_format_unref(fmt);
		return NULL;
	}

	ret = media_format_set_video_height(fmt, height);
	if (MEDIA_FORMAT_ERROR_NONE != ret) {
		CTS_ERR("media_format_set_video_height() Fail(%d)", ret);
		media_format_unref(fmt);
		return NULL;
	}

	ret = media_format_set_video_avg_bps(fmt, 2000000); /* image_util guide */
	if (MEDIA_FORMAT_ERROR_NONE != ret) {
		CTS_ERR("media_format_set_video_avg_bps() Fail(%d)", ret);
		media_format_unref(fmt);
		return NULL;
	}

	ret = media_format_set_video_max_bps(fmt, 15000000); /* image_util guide */
	if (MEDIA_FORMAT_ERROR_NONE != ret) {
		CTS_ERR("media_format_set_video_max_bps() Fail(%d)", ret);
		media_format_unref(fmt);
		return NULL;
	}

	return fmt;
}

static int _ctsvc_image_packet_create_alloc_finalize_cb(media_packet_h packet,
		int error_code, void *user_data)
{
	return MEDIA_PACKET_FINALIZE;
}

static media_packet_h _ctsvc_image_create_media_packet(media_format_h fmt,
		void *buffer, unsigned int buffer_size)
{
	int ret;
	void *mp_buffer = NULL;
	media_packet_h packet = NULL;
	uint64_t mp_buffer_size = 0;

	RETV_IF(NULL == fmt, NULL);
	RETV_IF(NULL == buffer, NULL);

	ret = media_packet_create_alloc(fmt, _ctsvc_image_packet_create_alloc_finalize_cb,
			NULL, &packet);
	if (MEDIA_PACKET_ERROR_NONE != ret) {
		CTS_ERR("media_packet_create_alloc() Fail(%d)", ret);
		return NULL;
	}

	ret = media_packet_get_buffer_size(packet, &mp_buffer_size);
	if (MEDIA_PACKET_ERROR_NONE != ret) {
		CTS_ERR("media_packet_get_buffer_size() Fail(%d)", ret);
		media_packet_destroy(packet);
		return NULL;
	}

	ret = media_packet_get_buffer_data_ptr(packet, &mp_buffer);
	if (MEDIA_PACKET_ERROR_NONE != ret) {
		CTS_ERR("media_packet_get_buffer_data_ptr() Fail(%d)", ret);
		media_packet_destroy(packet);
		return NULL;
	}

	if (mp_buffer)
		memcpy(mp_buffer, buffer, (int)buffer_size);

	return packet;
}

static void _ctsvc_image_transform_completed_cb(media_packet_h *dst,
		image_util_error_e error, void *user_data)
{
	int ret;
	uint64_t size = 0;
	void *buffer = 0;
	struct image_transform *info = user_data;

	if (NULL == info) {
		media_packet_destroy(*dst);
		return;
	}

	if (IMAGE_UTIL_ERROR_NONE == error) {
		ret = media_packet_get_buffer_size(*dst, &size);
		if (MEDIA_PACKET_ERROR_NONE != ret) {
			CTS_ERR("media_packet_get_buffer_size() Fail(%d)", ret);
			info->ret = CONTACTS_ERROR_SYSTEM;
			media_packet_destroy(*dst);
			g_mutex_lock(&info->mutex);
			g_cond_signal(&info->cond);
			g_mutex_unlock(&info->mutex);
			return;
		}

		ret = media_packet_get_buffer_data_ptr(*dst, &buffer);
		if (MEDIA_PACKET_ERROR_NONE != ret) {
			CTS_ERR("media_packet_get_buffer_data_ptr() Fail(%d)", ret);
			info->ret = CONTACTS_ERROR_SYSTEM;
			media_packet_destroy(*dst);
			g_mutex_lock(&info->mutex);
			g_cond_signal(&info->cond);
			g_mutex_unlock(&info->mutex);
			return;
		}

		info->buffer = calloc(1, (int)size);
		if (NULL == info->buffer) {
			CTS_ERR("calloc() Fail");
			info->ret = CONTACTS_ERROR_SYSTEM;
			media_packet_destroy(*dst);
			g_mutex_lock(&info->mutex);
			g_cond_signal(&info->cond);
			g_mutex_unlock(&info->mutex);
			return;
		}
		memcpy(info->buffer, buffer, (int)size);
		info->size = size;
		info->ret = CONTACTS_ERROR_NONE;
	}
	else {
		CTS_ERR("transform_run() Fail(%d)", error);
		info->ret = CONTACTS_ERROR_SYSTEM;
	}
	media_packet_destroy(*dst);
	g_mutex_lock(&info->mutex);
	g_cond_signal(&info->cond);
	g_mutex_unlock(&info->mutex);
}

static int _ctsvc_image_util_transform_run(transformation_h transform,
		media_packet_h packet, void **p_buffer, uint64_t *p_size)
{
	gint64 end_time;
	struct image_transform *info = NULL;

	RETV_IF(NULL == transform, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == packet, CONTACTS_ERROR_INVALID_PARAMETER);

	info = calloc(1, sizeof(struct image_transform));
	if (NULL == info) {
		CTS_ERR("calloc() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	g_cond_init(&info->cond);
	g_mutex_init(&info->mutex);

	g_mutex_lock(&info->mutex);
	image_util_transform_run(transform, packet, _ctsvc_image_transform_completed_cb, info);

	end_time = g_get_monotonic_time() + CTS_IMAGE_TRANSFORM_WAIT_TIME;
	if (!g_cond_wait_until(&info->cond, &info->mutex, end_time)) {
		/* timeout has passed */
		CTS_ERR("g_cond_wait_until() return FALSE");
		info->ret = CONTACTS_ERROR_SYSTEM;
	}
	g_mutex_unlock(&info->mutex);
	g_mutex_clear(&info->mutex);
	g_cond_clear(&info->cond);

	if (CONTACTS_ERROR_NONE != info->ret) {
		CTS_ERR("image_util_transform_run() Fail(%d)", info->ret);
		free(info->buffer);
		free(info);
		return CONTACTS_ERROR_SYSTEM;
	}

	*p_size = info->size;
	*p_buffer = info->buffer;
	free(info);

	return CONTACTS_ERROR_NONE;
}

static int _ctsvc_image_rotate(media_packet_h packet, image_util_rotation_e rotation,
		void **p_buffer, uint64_t *p_size)
{
	int ret;
	transformation_h transform = NULL;

	ret = image_util_transform_create(&transform);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		CTS_ERR("image_util_transform_create() Fail(%d)", ret);
		return CONTACTS_ERROR_SYSTEM;
	}

	ret = image_util_transform_set_rotation(transform, rotation);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		CTS_ERR("image_util_transform_set_rotation() Fail(%d)", ret);
		image_util_transform_destroy(transform);
		return CONTACTS_ERROR_SYSTEM;
	}

	ret = _ctsvc_image_util_transform_run(transform, packet, p_buffer, p_size);

	image_util_transform_destroy(transform);
	return ret;
}

static int _ctsvc_image_resize(media_packet_h packet, int width, int height,
		void **p_buffer, uint64_t *p_size)
{
	int ret;
	transformation_h transform = NULL;

	ret = image_util_transform_create(&transform);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		CTS_ERR("image_util_transform_create() Fail(%d)", ret);
		return CONTACTS_ERROR_SYSTEM;
	}

	ret = image_util_transform_set_resolution(transform, width, height);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		CTS_ERR("image_util_transform_set_resolution() Fail(%d)", ret);
		image_util_transform_destroy(transform);
		return CONTACTS_ERROR_SYSTEM;
	}

	ret = _ctsvc_image_util_transform_run(transform, packet, p_buffer, p_size);

	image_util_transform_destroy(transform);

	return ret;
}

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

	ret = _ctsvc_image_get_mimetype(colorspace, &mimetype);
	if (CONTACTS_ERROR_NONE != ret) {
		info->ret = CONTACTS_ERROR_SYSTEM;
		return true;
	}

	ret = image_util_decode_jpeg(info->src, colorspace, (unsigned char **)&buffer, &width,
			&height, (unsigned int *)&size);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		info->ret = CONTACTS_ERROR_SYSTEM;
		return true;
	}

	rotation = _ctsvc_image_get_rotation_info(info->src);
	if (IMAGE_UTIL_ROTATION_NONE != rotation) { /* need rotate */
		media_format_h fmt;
		media_packet_h packet;

		fmt = _ctsvc_image_create_media_format(mimetype, width, height);
		if (NULL == fmt) {
			CTS_ERR("_ctsvc_image_create_media_format() Fail");
			info->ret = CONTACTS_ERROR_SYSTEM;
			free(buffer);
			return false;
		}

		packet = _ctsvc_image_create_media_packet(fmt, buffer, (unsigned int)size);
		if (NULL == packet) {
			CTS_ERR("_ctsvc_image_create_media_packet() Fail");
			media_format_unref(fmt);
			info->ret = CONTACTS_ERROR_SYSTEM;
			free(buffer);
			return false;
		}

		ret = _ctsvc_image_rotate(packet, rotation, &buffer_temp, &size);

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

	if (CTS_IMAGE_MAX_SIZE < width || CTS_IMAGE_MAX_SIZE < height) { /* need resize */
		int resized_width;
		int resized_height;
		media_format_h fmt;
		media_packet_h packet;

		/* set resize */
		if (width>height) {
			resized_width = CTS_IMAGE_MAX_SIZE;
			resized_height = height * CTS_IMAGE_MAX_SIZE / width;
		}
		else {
			resized_height = CTS_IMAGE_MAX_SIZE;
			resized_width = width * CTS_IMAGE_MAX_SIZE / height;
		}

		if (resized_height % 8)
			resized_height += (8 - (resized_height % 8));

		if (resized_width % 8)
			resized_width += (8 - (resized_width % 8));

		fmt = _ctsvc_image_create_media_format(mimetype, width, height);
		if (NULL == fmt) {
			CTS_ERR("_ctsvc_image_create_media_format() Fail");
			info->ret = CONTACTS_ERROR_SYSTEM;
			free(buffer);
			return false;
		}

		packet = _ctsvc_image_create_media_packet(fmt, buffer, (unsigned int)size);
		if (NULL == packet) {
			CTS_ERR("_ctsvc_image_create_media_packet() Fail");
			media_format_unref(fmt);
			info->ret = CONTACTS_ERROR_SYSTEM;
			free(buffer);
			return false;
		}

		ret = _ctsvc_image_resize(packet, resized_width, resized_height, &buffer_temp,
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
			CTS_IMAGE_ENCODE_QUALITY, info->dest);
	free(buffer);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
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

static int _ctsvc_image_encode(const char *src, const char *dest)
{
	int ret;
	image_info info = {.src = src, .dest = dest, ret = CONTACTS_ERROR_SYSTEM};

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

	if (false == ctsvc_check_available_image_space())
		return CONTACTS_ERROR_FILE_NO_SPACE;

	ret = _ctsvc_image_encode(src, dest);
	if (CONTACTS_ERROR_NONE == ret) {
		return ret;
	}
	else
		CTS_ERR("_ctsvc_image_encode Fail(%d)", ret);

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

	/* In this case, contacts-service already works abnormally. */
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


/*
 * Contacts Service
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved.
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

#include "ctsvc_internal.h"
#include "ctsvc_image_util.h"

struct image_transform {
	int ret;
	uint64_t size;
	void *buffer;
	GCond cond;
	GMutex mutex;
};

int ctsvc_image_util_get_mimetype(image_util_colorspace_e colorspace,
		int *p_mimetype)
{
	RETV_IF(NULL == p_mimetype, CONTACTS_ERROR_INVALID_PARAMETER);

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
		//LCOV_EXCL_START
		ERR("Not supported %d", colorspace);
		return CONTACTS_ERROR_INVALID_PARAMETER;
		//LCOV_EXCL_STOP
	}
	*p_mimetype = mimetype;
	return CONTACTS_ERROR_NONE;
}

media_format_h ctsvc_image_util_create_media_format(int mimetype, int width,
		int height)
{
	int ret;
	media_format_h fmt = NULL;

	ret = media_format_create(&fmt);
	if (MEDIA_FORMAT_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("media_format_create() Fail(%d)", ret);
		return NULL;
		//LCOV_EXCL_STOP
	}

	ret = media_format_set_video_mime(fmt, mimetype);
	if (MEDIA_FORMAT_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("media_format_set_video_mime() Fail(%d)", ret);
		media_format_unref(fmt);
		return NULL;
		//LCOV_EXCL_STOP
	}

	ret = media_format_set_video_width(fmt, width);
	if (MEDIA_FORMAT_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("media_format_set_video_width() Fail(%d)", ret);
		media_format_unref(fmt);
		return NULL;
		//LCOV_EXCL_STOP
	}

	ret = media_format_set_video_height(fmt, height);
	if (MEDIA_FORMAT_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("media_format_set_video_height() Fail(%d)", ret);
		media_format_unref(fmt);
		return NULL;
		//LCOV_EXCL_STOP
	}

	ret = media_format_set_video_avg_bps(fmt, 2000000); /* image_util guide */
	if (MEDIA_FORMAT_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("media_format_set_video_avg_bps() Fail(%d)", ret);
		media_format_unref(fmt);
		return NULL;
		//LCOV_EXCL_STOP
	}

	ret = media_format_set_video_max_bps(fmt, 15000000); /* image_util guide */
	if (MEDIA_FORMAT_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("media_format_set_video_max_bps() Fail(%d)", ret);
		media_format_unref(fmt);
		return NULL;
		//LCOV_EXCL_STOP
	}

	return fmt;
}

int _ctsvc_image_packet_create_alloc_finalize_cb(media_packet_h packet,
		int error_code, void *user_data)
{
	return MEDIA_PACKET_FINALIZE;
}

media_packet_h ctsvc_image_util_create_media_packet(media_format_h fmt,
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
		//LCOV_EXCL_START
		ERR("media_packet_create_alloc() Fail(%d)", ret);
		return NULL;
		//LCOV_EXCL_STOP
	}

	ret = media_packet_get_buffer_size(packet, &mp_buffer_size);
	if (MEDIA_PACKET_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("media_packet_get_buffer_size() Fail(%d)", ret);
		media_packet_destroy(packet);
		return NULL;
		//LCOV_EXCL_STOP
	}

	ret = media_packet_get_buffer_data_ptr(packet, &mp_buffer);
	if (MEDIA_PACKET_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("media_packet_get_buffer_data_ptr() Fail(%d)", ret);
		media_packet_destroy(packet);
		return NULL;
		//LCOV_EXCL_STOP
	}

	if (mp_buffer)
		memcpy(mp_buffer, buffer, (int)((buffer_size < mp_buffer_size) ? buffer_size : mp_buffer_size));

	return packet;
}


static void _image_transform_completed_cb(media_packet_h *dst,
		image_util_error_e error, void *user_data)
{
	int ret;
	uint64_t size = 0;
	void *buffer = 0;
	struct image_transform *info = user_data;

	if (NULL == info) {
		ERR("NULL == info");
		media_packet_destroy(*dst);
		return;
	}

	if (IMAGE_UTIL_ERROR_NONE == error) {
		ret = media_packet_get_buffer_size(*dst, &size);
		if (MEDIA_PACKET_ERROR_NONE != ret) {
			//LCOV_EXCL_START
			ERR("media_packet_get_buffer_size() Fail(%d)", ret);
			info->ret = CONTACTS_ERROR_SYSTEM;
			media_packet_destroy(*dst);
			g_mutex_lock(&info->mutex);
			g_cond_signal(&info->cond);
			g_mutex_unlock(&info->mutex);
			return;
			//LCOV_EXCL_STOP
		}

		ret = media_packet_get_buffer_data_ptr(*dst, &buffer);
		if (MEDIA_PACKET_ERROR_NONE != ret) {
			//LCOV_EXCL_START
			ERR("media_packet_get_buffer_data_ptr() Fail(%d)", ret);
			info->ret = CONTACTS_ERROR_SYSTEM;
			media_packet_destroy(*dst);
			g_mutex_lock(&info->mutex);
			g_cond_signal(&info->cond);
			g_mutex_unlock(&info->mutex);
			return;
			//LCOV_EXCL_STOP
		}

		info->buffer = calloc(1, (size_t)size);
		if (NULL == info->buffer) {
			//LCOV_EXCL_START
			ERR("calloc() Fail");
			info->ret = CONTACTS_ERROR_SYSTEM;
			media_packet_destroy(*dst);
			g_mutex_lock(&info->mutex);
			g_cond_signal(&info->cond);
			g_mutex_unlock(&info->mutex);
			return;
			//LCOV_EXCL_STOP
		}
		memcpy(info->buffer, buffer, (size_t)size);
		info->size = size;
		info->ret = CONTACTS_ERROR_NONE;
	} else {
		ERR("transform_run() Fail(%d)", error);
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
	int ret;
	gint64 end_time;
	struct image_transform *info = NULL;

	RETV_IF(NULL == transform, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == packet, CONTACTS_ERROR_INVALID_PARAMETER);

	info = calloc(1, sizeof(struct image_transform));
	if (NULL == info) {
		//LCOV_EXCL_START
		ERR("calloc() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		//LCOV_EXCL_STOP
	}

	g_cond_init(&info->cond);
	g_mutex_init(&info->mutex);

	g_mutex_lock(&info->mutex);
	ret = image_util_transform_run(transform, packet, _image_transform_completed_cb, info);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("image_util_transform_run() Fail(%d)", ret);
		g_mutex_unlock(&info->mutex);
		g_mutex_clear(&info->mutex);
		g_cond_clear(&info->cond);
		return CONTACTS_ERROR_SYSTEM;
		//LCOV_EXCL_STOP
	}

	end_time = g_get_monotonic_time() + 4000 * G_TIME_SPAN_MILLISECOND;
	if (!g_cond_wait_until(&info->cond, &info->mutex, end_time)) {
		/* timeout has passed */
		ERR("g_cond_wait_until() return FALSE");
		info->ret = CONTACTS_ERROR_SYSTEM;
	}
	g_mutex_unlock(&info->mutex);
	g_mutex_clear(&info->mutex);
	g_cond_clear(&info->cond);

	if (CONTACTS_ERROR_NONE != info->ret) {
		//LCOV_EXCL_START
		ERR("image_util_transform_run() Fail(%d)", info->ret);
		free(info->buffer);
		free(info);
		return CONTACTS_ERROR_SYSTEM;
		//LCOV_EXCL_STOP
	}

	*p_size = info->size;
	*p_buffer = info->buffer;
	free(info);
	return CONTACTS_ERROR_NONE;
}


int ctsvc_image_util_rotate(media_packet_h packet, image_util_rotation_e rotation,
		void **p_buffer, uint64_t *p_size)
{
	int ret;
	transformation_h transform = NULL;

	ret = image_util_transform_create(&transform);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("image_util_transform_create() Fail(%d)", ret);
		return CONTACTS_ERROR_SYSTEM;
		//LCOV_EXCL_STOP
	}

	ret = image_util_transform_set_rotation(transform, rotation);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("image_util_transform_set_rotation() Fail(%d)", ret);
		image_util_transform_destroy(transform);
		return CONTACTS_ERROR_SYSTEM;
		//LCOV_EXCL_STOP
	}

	ret = _ctsvc_image_util_transform_run(transform, packet, p_buffer, p_size);

	image_util_transform_destroy(transform);
	return ret;
}

int ctsvc_image_util_resize(media_packet_h packet, int width, int height,
		void **p_buffer, uint64_t *p_size)
{
	int ret;
	transformation_h transform = NULL;

	ret = image_util_transform_create(&transform);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("image_util_transform_create() Fail(%d)", ret);
		return CONTACTS_ERROR_SYSTEM;
		//LCOV_EXCL_STOP
	}

	ret = image_util_transform_set_resolution(transform, width, height);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("image_util_transform_set_resolution() Fail(%d)", ret);
		image_util_transform_destroy(transform);
		return CONTACTS_ERROR_SYSTEM;
		//LCOV_EXCL_STOP
	}

	ret = _ctsvc_image_util_transform_run(transform, packet, p_buffer, p_size);

	image_util_transform_destroy(transform);

	return ret;
}


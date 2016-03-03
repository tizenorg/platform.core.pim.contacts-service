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
#ifndef __CTSVC_IMAGE_UTIL_H__
#define __CTSVC_IMAGE_UTIL_H__

#include <image_util.h>

#define CTSVC_IMAGE_MAX_SIZE 1080
#define CTSVC_IMAGE_ENCODE_QUALITY 50

int ctsvc_image_util_get_mimetype(image_util_colorspace_e colorspace, int *p_mimetype);
media_format_h ctsvc_image_util_create_media_format(int mimetype, int width, int height);
media_packet_h ctsvc_image_util_create_media_packet(media_format_h fmt,
		void *buffer, unsigned int buffer_size);
int ctsvc_image_util_rotate(media_packet_h packet, image_util_rotation_e rotation,
		void **p_buffer, uint64_t *p_size);
int ctsvc_image_util_resize(media_packet_h packet, int width, int height,
		void **p_buffer, uint64_t *p_size);

#endif /* __CTSVC_IMAGE_UTIL_H__ */

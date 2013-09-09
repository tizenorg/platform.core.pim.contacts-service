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

#ifndef __TIZEN_SOCIAL_CTSVC_UTILS_H__
#define __TIZEN_SOCIAL_CTSVC_UTILS_H__

const char* ctsvc_get_display_column(void);
const char* ctsvc_get_sort_column(void);
const char* ctsvc_get_sort_name_column(void);

int ctsvc_begin_trans(void);
int ctsvc_end_trans(bool is_success);
int ctsvc_get_next_ver(void);
int ctsvc_get_current_version( int* out_current_version );
int ctsvc_get_transaction_ver(void);

int ctsvc_utils_copy_image(const char *dir, const char *src, const char *file);
void ctsvc_utils_make_image_file_name(int parent_id, int id, char *src_img, char *dest, int dest_size);

#endif /* __TIZEN_SOCIAL_CTSVC_UTILS_H__ */
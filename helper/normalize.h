/*
 * Contacts Service Helper
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjae Shin <yj99.shin@samsung.com>
 *          Donghee Ye <donghee.ye@samsung.com>
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
#ifndef __CTS_HELPER_NORMALIZE_H__
#define __CTS_HELPER_NORMALIZE_H__

#include "cts-normalize.h"

int helper_normalize_str(const char *src, char *dest, int dest_size);
int helper_collation_str(const char *src, char *dest, int dest_size);
int helper_unicode_to_utf8(char *src, int src_len, char *dest, int dest_size);

#endif // __CTS_HELPER_NORMALIZE_H__


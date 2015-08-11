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

#ifndef __CTSVC_LOCALIZE_KOR_H__
#define __CTSVC_LOCALIZE_KOR_H__

int ctsvc_get_chosung(const char *src, char *dest, int dest_size);
int ctsvc_get_korean_search_pattern(const char *src, char *dest, int dest_size);
bool ctsvc_has_korean(const char *src);
bool ctsvc_has_chosung(const char *src);
bool ctsvc_is_chosung(const char *src);
bool ctsvc_is_hangul(UChar src);
void ctsvc_hangul_compatibility2jamo(UChar *src);

#endif /* __CTSVC_LOCALIZE_KOR_H__ */
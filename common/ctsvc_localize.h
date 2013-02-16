/*
 * Contacts Service Helper
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
#ifndef __TIZEN_SOCIAL_CTSVC_LOCALIZE_H__
#define __TIZEN_SOCIAL_CTSVC_LOCALIZE_H__

#include <unicode/utypes.h>

int ctsvc_check_utf8(char c);
int ctsvc_check_language(UChar *word);
int ctsvc_check_language_type(const char *src);
int ctsvc_get_language_type(const char *system_lang);
void ctsvc_extra_normalize(UChar *word, int32_t word_size);
int ctsvc_get_chosung(const char *src, char *dest, int dest_size);
#endif // __TIZEN_SOCIAL_CTSVC_LOCALIZE_H__

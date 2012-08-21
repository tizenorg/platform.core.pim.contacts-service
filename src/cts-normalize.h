/*
 * Contacts Service
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
#ifndef __CTS_NORMALIZE_H__
#define __CTS_NORMALIZE_H__

#include "cts-sqlite.h"

#define CTS_COMPARE_BETWEEN(left_range, value, right_range) (((left_range) <= (value)) && ((value) <= (right_range)))
#define CTS_VCONF_DEFAULT_LANGUAGE "file/private/contacts-service/default_lang"

/**
 * Language Type
 */
enum LANGTYPE{
	CTS_LANG_NUMBER = 0,
	CTS_LANG_DEFAULT = 1,
	CTS_LANG_SYMBOL = 2,
	CTS_LANG_ENGLISH = 3,
	CTS_LANG_KOREAN = 4, /* always last-first */
	CTS_LANG_CHINESE = 5,
	CTS_LANG_JAPANESE = 6,
	CTS_LANG_FRENCH = 7,
	CTS_LANG_GERMAN = 8,
	CTS_LANG_ITALIAN = 9,
	CTS_LANG_RUSSIAN = 10,
	CTS_LANG_DUTCH = 11,
	CTS_LANG_PORTUGUESE = 12,
	CTS_LANG_TURKISH = 13,
	CTS_LANG_GREEK = 14,
	CTS_LANG_SPANISH = 15,
	CTS_LANG_OTHERS = 16,
};

enum{
	CTS_NN_FIRST,
	CTS_NN_LAST,
	CTS_NN_SORTKEY,
	CTS_NN_MAX,
};

int cts_normalize_str(const char *src, char *dest, int dest_size);
int cts_normalize_name(cts_name *src, char dest[][CTS_SQL_MAX_LEN], bool is_display);
void cts_set_extra_normalize_fn(int (*fn)(char dest[][CTS_SQL_MAX_LEN]));
const char* cts_normalize_number(const char *src);
int cts_clean_number(const char *src, char *dest, int dest_size);

#endif //__CTS_NORMALIZE_H__

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
#ifndef __TIZEN_SOCIAL_CTSVC_LOCALIZE_H__
#define __TIZEN_SOCIAL_CTSVC_LOCALIZE_H__

#include <unicode/utypes.h>

#include "ctsvc_localize_jp.h"
#include "ctsvc_localize_kor.h"

enum SORTTYPE{
	CTSVC_SORT_OTHERS,		// 0??
	CTSVC_SORT_NUMBER,		// 1
	CTSVC_SORT_PRIMARY,		// 2
	CTSVC_SORT_SECONDARY,	// 3
	CTSVC_SORT_WESTERN,		// 4
	CTSVC_SORT_KOREAN,		// 5
	CTSVC_SORT_JAPANESE,		// 6
	CTSVC_SORT_CJK,			// 7
	CTSVC_SORT_CYRILLIC,		// 8
	CTSVC_SORT_GREEK,			// 9
	CTSVC_SORT_ARMENIAN,        // 10
	CTSVC_SORT_ARABIC,          // 11
	CTSVC_SORT_DEVANAGARI,	// hindi
	CTSVC_SORT_GEORGIAN,    // 13
	CTSVC_SORT_TURKISH,     // 14
	CTSVC_SORT_THAI,        // 15
	CTSVC_SORT_BENGALI,     // 16
	CTSVC_SORT_PUNJABI,     // 17
	CTSVC_SORT_MALAYALAM,   // 18
	CTSVC_SORT_TELUGU,      // 19
	CTSVC_SORT_TAMIL,       // 20
	CTSVC_SORT_ORIYA,       // 21
	CTSVC_SORT_SINHALA,     // 22
	CTSVC_SORT_GUJARATI,    // 23
	CTSVC_SORT_KANNADA,     // 24
	CTSVC_SORT_LAO,         // 25
	CTSVC_SORT_HEBREW,	    // 26
	CTSVC_SORT_BURMESE,		// 27
	CTSVC_SORT_KHMER,		// 28
};

char* ctsvc_get_langset();
void ctsvc_set_langset(char *new_langset);

int ctsvc_get_name_sort_type(const char *src);
int ctsvc_get_sort_type_from_language(int language);

int ctsvc_get_language_type(const char *system_lang);
const char *ctsvc_get_language_locale(int lang);
void ctsvc_extra_normalize(UChar *word, int32_t word_size);

#endif // __TIZEN_SOCIAL_CTSVC_LOCALIZE_H__

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
#ifndef __CTSVC_LOCALIZE_H__
#define __CTSVC_LOCALIZE_H__

#include <unicode/utypes.h>

#include "ctsvc_localize_jp.h"
#include "ctsvc_localize_kor.h"

enum SORTTYPE{
	CTSVC_SORT_OTHERS = 0,
	CTSVC_SORT_NUMBER,
	CTSVC_SORT_PRIMARY,
	CTSVC_SORT_SECONDARY,
	CTSVC_SORT_WESTERN,
	CTSVC_SORT_KOREAN,   /* 5 */
	CTSVC_SORT_JAPANESE,
	CTSVC_SORT_CJK,
	CTSVC_SORT_CYRILLIC,
	CTSVC_SORT_GREEK,
	CTSVC_SORT_ARMENIAN, /* 10 */
	CTSVC_SORT_ARABIC,
	CTSVC_SORT_DEVANAGARI, /* hindi */
	CTSVC_SORT_GEORGIAN,
	CTSVC_SORT_TURKISH,
	CTSVC_SORT_THAI,     /* 15 */
	CTSVC_SORT_BENGALI,
	CTSVC_SORT_PUNJABI,
	CTSVC_SORT_MALAYALAM,
	CTSVC_SORT_TELUGU,
	CTSVC_SORT_TAMIL,    /* 20 */
	CTSVC_SORT_ORIYA,
	CTSVC_SORT_SINHALA,
	CTSVC_SORT_GUJARATI,
	CTSVC_SORT_KANNADA,
	CTSVC_SORT_LAO,      /* 25 */
	CTSVC_SORT_HEBREW,
	CTSVC_SORT_BURMESE,
	CTSVC_SORT_KHMER,
};

char* ctsvc_get_langset();
void ctsvc_set_langset(char *new_langset);

int ctsvc_get_name_sort_type(const char *src);
int ctsvc_get_sort_type_from_language(int language);

int ctsvc_get_language_type(const char *system_lang);
const char *ctsvc_get_language_locale(int lang);
void ctsvc_extra_normalize(UChar *word, int32_t word_size);

#endif /* __CTSVC_LOCALIZE_H__ */
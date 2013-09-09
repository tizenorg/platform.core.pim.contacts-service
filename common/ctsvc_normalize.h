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
#ifndef __TIZEN_SOCIAL_CTSVC_NORMALIZE_H__
#define __TIZEN_SOCIAL_CTSVC_NORMALIZE_H__

enum LANGTYPE{
	CTSVC_LANG_NUMBER = 0,
	CTSVC_LANG_DEFAULT = 1,
	CTSVC_LANG_SECONDARY = 2,
	CTSVC_LANG_ENGLISH = 3,
	CTSVC_LANG_KOREAN = 4, /* always last-first */
	CTSVC_LANG_CHINESE = 5,
	CTSVC_LANG_JAPANESE = 6,
	CTSVC_LANG_FRENCH = 7,
	CTSVC_LANG_GERMAN = 8,
	CTSVC_LANG_ITALIAN = 9,
	CTSVC_LANG_RUSSIAN = 10,
	CTSVC_LANG_DUTCH = 11,
	CTSVC_LANG_PORTUGUESE = 12,
	CTSVC_LANG_TURKISH = 13,
	CTSVC_LANG_GREEK = 14,
	CTSVC_LANG_SPANISH = 15,
	CTSVC_LANG_DANISH = 16,
	CTSVC_LANG_AZERBAIJAN = 17,
	CTSVC_LANG_ARABIC = 18,
	CTSVC_LANG_BULGARIAN = 19,
	CTSVC_LANG_CATALAN = 20,
	CTSVC_LANG_CZECH = 21,
	CTSVC_LANG_ESTONIAN = 22,
	CTSVC_LANG_BASQUE = 23,
	CTSVC_LANG_FINNISH = 24,
	CTSVC_LANG_IRISH = 25,
	CTSVC_LANG_GALICIAN = 26,
	CTSVC_LANG_HINDI = 27,
	CTSVC_LANG_CROATIAN = 28,
	CTSVC_LANG_HUNGARIAN= 29,
	CTSVC_LANG_ARMENIAN = 30,
	CTSVC_LANG_ICELANDIC = 31,
	CTSVC_LANG_GEORGIAN = 32,
	CTSVC_LANG_KAZAKHSTAN = 33,
	CTSVC_LANG_LITHUANIAN = 34,
	CTSVC_LANG_LATVIAN = 35,
	CTSVC_LANG_MACEDONIA = 36,
	CTSVC_LANG_NORWAY= 37,
	CTSVC_LANG_POLISH = 38,
	CTSVC_LANG_ROMANIA = 39,
	CTSVC_LANG_SLOVAK = 40,
	CTSVC_LANG_SLOVENIAN = 41,
	CTSVC_LANG_SERBIAN = 42,
	CTSVC_LANG_SWEDISH = 43,
	CTSVC_LANG_UKRAINE = 44,
	CTSVC_LANG_OTHERS = 1000,
};

int ctsvc_clean_number(const char *src, char *dest, int dest_size);
int ctsvc_normalize_number(const char *src, char *dest, int dest_size, int min_match);
int ctsvc_normalize_str(const char *src, char **dest);
int ctsvc_collation_str(char *src, char **dest);
int ctsvc_normalize_index(const char *src, char **dest);

bool ctsvc_is_phonenumber(const char* src);
int ctsvc_get_halfwidth_string(const char *src, char** dest, int* dest_size);

#endif /*  __TIZEN_SOCIAL_CTSVC_NORMALIZE_H__ */
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

#define VCONFKEY_CONTACTS_SVC_SECONDARY_LANGUAGE "db/contacts-svc/secondary_lang"	// It should be added to vconf-internal-keys

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
	CTSVC_LANG_DANISH,
	CTSVC_LANG_AZERBAIJAN,
	CTSVC_LANG_ARABIC,
	CTSVC_LANG_BULGARIAN,
	CTSVC_LANG_CATALAN,
	CTSVC_LANG_CZECH,
	CTSVC_LANG_ESTONIAN,
	CTSVC_LANG_BASQUE,
	CTSVC_LANG_FINNISH,
	CTSVC_LANG_IRISH,
	CTSVC_LANG_GALICIAN,
	CTSVC_LANG_HINDI,
	CTSVC_LANG_CROATIAN,
	CTSVC_LANG_HUNGARIAN,
	CTSVC_LANG_ARMENIAN,
	CTSVC_LANG_ICELANDIC,
	CTSVC_LANG_GEORGIAN,
	CTSVC_LANG_KAZAKHSTAN,
	CTSVC_LANG_LITHUANIAN,
	CTSVC_LANG_LATVIAN,
	CTSVC_LANG_MACEDONIA,
	CTSVC_LANG_NORWAY,
	CTSVC_LANG_POLISH,
	CTSVC_LANG_ROMANIA,
	CTSVC_LANG_SLOVAK,
	CTSVC_LANG_SLOVENIAN,
	CTSVC_LANG_SERBIAN,
	CTSVC_LANG_SWEDISH,
	CTSVC_LANG_UKRAINE,
	CTSVC_LANG_OTHERS = 1000,
};

int ctsvc_clean_number(const char *src, char *dest, int dest_size);
int ctsvc_normalize_number(const char *src, char *dest, int dest_size);
int ctsvc_normalize_str(const char *src, char *dest, int dest_size);
int ctsvc_collation_str(char *src, char **dest);
int ctsvc_normalize_index(const char *src, char *dest, int dest_size);

#endif /*  __TIZEN_SOCIAL_CTSVC_NORMALIZE_H__ */

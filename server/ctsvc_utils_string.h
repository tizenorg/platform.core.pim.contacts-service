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

#ifndef __CTSVC_UTILS_STRING_H__
#define __CTSVC_UTILS_STRING_H__

#include "ctsvc_struct.h"
#include "ctsvc_record.h"

enum {
	CTSVC_ENUM_URI_PERSON,
	CTSVC_ENUM_URI_READ_ONLY_PERSON_CONTACT,
	CTSVC_ENUM_URI_READ_ONLY_PERSON_NUMBER,
	CTSVC_ENUM_URI_READ_ONLY_PERSON_EMAIL,
	CTSVC_ENUM_URI_READ_ONLY_PERSON_GROUP,
	CTSVC_ENUM_URI_READ_ONLY_PERSON_GROUP_ASSIGNED,
	CTSVC_ENUM_URI_READ_ONLY_PERSON_GROUP_NOT_ASSIGNED,
};

int ctsvc_utils_string_strstr(const char *haystack, const char *needle, int *len);
char *ctsvc_utils_get_modified_str(char *temp, bool is_snippet, const char *keyword,
		const char *start_match, const char *end_match, int token_number);

#endif /*  __CTSVC_UTILS_STRING_H__ */


/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Dohyung Jin <dh.jin@samsung.com>
 *                 Jongwon Lee <gogosing.lee@samsung.com>
 *                 Donghee Ye <donghee.ye@samsung.com>
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

#ifndef __CTSVC_SNIPPET_H__
#define __CTSVC_SNIPPET_H__

#include "ctsvc_struct.h"
#include "ctsvc_record.h"

void ctsvc_snippet_free(ctsvc_snippet_s *snippet);
bool ctsvc_snippet_is_snippet(contacts_record_h record, char **out_text,
		char **out_start_match, char **out_end_match);
char *ctsvc_snippet_mod_string(char *origin_string, const char *text,
		char *start_match, char *end_match, int len_start_match, int len_end_match);

#endif /*  __CTSVC_SNIPPET_H__ */


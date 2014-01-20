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

#ifndef __TIZEN_SOCIAL_CTSVC_PHONELOG_H__
#define __TIZEN_SOCIAL_CTSVC_PHONELOG_H__

#include "ctsvc_sqlite.h"
void ctsvc_db_phone_log_delete_callback(sqlite3_context * context,
		int argc, sqlite3_value ** argv);
int ctsvc_db_phone_log_update_person_id(const char *number, int old_person_id, int candidate_person_id, bool person_link);

#endif // __TIZEN_SOCIAL_CTSVC_PHONELOG_H__
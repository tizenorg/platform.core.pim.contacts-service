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

#ifndef __CTSVC_DB_PLUGIN_PROFILE_HELPER_H__
#define __CTSVC_DB_PLUGIN_PROFILE_HELPER_H__

#include "contacts.h"
#include "ctsvc_sqlite.h"

int ctsvc_db_profile_insert(contacts_record_h record, int contact_id, bool is_my_profile, int *id);
int ctsvc_db_profile_update(contacts_record_h record, int contact_id, bool is_my_profile);
int ctsvc_db_profile_delete(int id);

int ctsvc_db_profile_get_value_from_stmt(cts_stmt stmt, contacts_record_h *record, int start_count);

#endif // __CTSVC_DB_PLUGIN_PROFILE_HELPER_H__

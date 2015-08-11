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

#ifndef __CTSVC_DB_PLUGIN_PERSON_HELPER_H__
#define __CTSVC_DB_PLUGIN_PERSON_HELPER_H__

#include "contacts.h"
#include "ctsvc_db_sqlite.h"

#define ADDRESSBOOK_ID_DELIM	" "

int ctsvc_db_insert_person(contacts_record_h contact);
int ctsvc_db_update_person(contacts_record_h contact);
int ctsvc_db_person_create_record_from_stmt(cts_stmt stmt, contacts_record_h *record);
int ctsvc_db_person_create_record_from_stmt_with_query(cts_stmt stmt, contacts_query_h query, contacts_record_h *record);
int ctsvc_db_person_create_record_from_stmt_with_projection(cts_stmt stmt, unsigned int *projection, int projection_count, contacts_record_h *record);
void ctsvc_db_normalize_str_callback(sqlite3_context * context,	int argc, sqlite3_value ** argv);
int ctsvc_db_person_set_favorite(int person_id, bool set, bool propagate);

#endif /* __CTSVC_DB_PLUGIN_PERSON_HELPER_H__ */

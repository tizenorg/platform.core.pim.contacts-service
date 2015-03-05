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

#ifndef __TIZEN_SOCIAL_CTSVC_DB_QUERY_H__
#define __TIZEN_SOCIAL_CTSVC_DB_QUERY_H__

#include "contacts.h"
#include "contacts_db_status.h"
#include "ctsvc_struct.h"
#include "ctsvc_sqlite.h"

int ctsvc_db_insert_records(contacts_list_h list, int **ids, unsigned int *count);
int ctsvc_db_update_records(contacts_list_h list);
int ctsvc_db_delete_records(const char* view_uri, int *ids, int count);
int ctsvc_db_replace_records(contacts_list_h list, int ids[], unsigned int count);

int ctsvc_db_make_get_records_query_stmt(ctsvc_query_s *s_query, int offset, int limit, cts_stmt *stmt);
int ctsvc_db_create_set_query(contacts_record_h record, char **set, GSList **bind_text);
int ctsvc_db_update_record_with_set_query(const char *set, GSList *bind_text, const char *table, int id);

void ctsvc_db_set_status(contacts_db_status_e status);

#endif /*  __TIZEN_SOCIAL_CTSVC_DB_QUERY_H__ */


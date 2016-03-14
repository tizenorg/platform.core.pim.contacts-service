/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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
#ifndef __CTSVC_DB_QUERY_H__
#define __CTSVC_DB_QUERY_H__

#include "contacts.h"
#include "contacts_db_status.h"
#include "ctsvc_struct.h"
#include "ctsvc_db_sqlite.h"

int ctsvc_db_insert_record(contacts_record_h record, int *id);
int ctsvc_db_get_record(const char *view_uri, int id, contacts_record_h *out_record);
int ctsvc_db_update_record(contacts_record_h record);
int ctsvc_db_delete_record(const char *view_uri, int id);
int ctsvc_db_replace_record(contacts_record_h record, int id);
int ctsvc_db_get_all_records(const char *view_uri, int offset, int limit,
		contacts_list_h *out_list);
int ctsvc_db_get_records_with_query(contacts_query_h query, int offset, int limit,
		contacts_list_h *out_list);
int ctsvc_db_get_count(const char *view_uri, int *out_count);
int ctsvc_db_get_count_with_query(contacts_query_h query, int *out_count);
int ctsvc_db_get_changes_by_version(const char *view_uri, int addressbook_id,
		int version, contacts_list_h *out_list, int *out_current_version);
int ctsvc_db_get_current_version(int *out_current_version);
int ctsvc_db_search_records(const char *view_uri, const char *keyword, int offset,
		int limit, contacts_list_h *out_list);
int ctsvc_db_search_records_with_range(const char *view_uri, const char *keyword,
		int offset, int limit, int range, contacts_list_h *out_list);
int ctsvc_db_search_records_with_query(contacts_query_h query, const char *keyword,
		int offset, int limit, contacts_list_h *out_list);
int ctsvc_db_search_records_for_snippet(const char *view_uri,
		const char *keyword,
		int offset,
		int limit,
		const char *start_match,
		const char *end_match,
		contacts_list_h *out_list);
int ctsvc_db_search_records_with_range_for_snippet(const char *view_uri,
		const char *keyword,
		int offset,
		int limit,
		int range,
		const char *start_match,
		const char *end_match,
		contacts_list_h *out_list);
int ctsvc_db_search_records_with_query_for_snippet(contacts_query_h query,
		const char *keyword,
		int offset,
		int limit,
		const char *start_match,
		const char *end_match,
		contacts_list_h *out_list);
int ctsvc_db_get_status(contacts_db_status_e *status);
int ctsvc_db_insert_records_with_vcard(const char *vcard_stream, int **record_id_array,
		int *count);
int ctsvc_db_replace_records_with_vcard(const char *vcard_stream, int *record_id_array,
		int count);


int ctsvc_db_insert_records(contacts_list_h list, int **ids, int *count);
int ctsvc_db_update_records(contacts_list_h list);
int ctsvc_db_delete_records(const char *view_uri, int *ids, int count);
int ctsvc_db_replace_records(contacts_list_h list, int ids[], int count);

int ctsvc_db_make_get_records_query_stmt(ctsvc_query_s *s_query, int offset, int limit,
		cts_stmt *stmt);
int ctsvc_db_create_set_query(contacts_record_h record, char **set, GSList **bind_text);
int ctsvc_db_update_record_with_set_query(const char *set, GSList *bind_text,
		const char *table, int id);

void ctsvc_db_set_status(contacts_db_status_e status);

#endif /*  __CTSVC_DB_QUERY_H__ */


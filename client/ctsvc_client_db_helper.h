/*
 * Contacts Service
 *
 * Copyright (c) 2014 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __CTSVC_CLIENT_DB_HELPER_H__
#define __CTSVC_CLIENT_DB_HELPER_H__

#include "contacts_types.h"

int ctsvc_client_db_insert_record(contacts_h contact, contacts_record_h record, int *id);
int ctsvc_client_db_get_record(contacts_h contact, const char* view_uri, int id, contacts_record_h* out_record);
int ctsvc_client_db_update_record(contacts_h contact, contacts_record_h record);
int ctsvc_client_db_delete_record(contacts_h contact, const char* view_uri, int id);
int ctsvc_client_db_replace_record(contacts_h contact, contacts_record_h record, int id);
int ctsvc_client_db_get_all_records(contacts_h contact, const char* view_uri, int offset, int limit, contacts_list_h* out_list);
int ctsvc_client_db_get_records_with_query(contacts_h contact, contacts_query_h query, int offset, int limit, contacts_list_h* out_list);
int ctsvc_client_db_get_count(contacts_h contact, const char* view_uri, int *out_count);
int ctsvc_client_db_get_count_with_query(contacts_h contact, contacts_query_h query, int *out_count);
int ctsvc_client_db_insert_records(contacts_h contact, contacts_list_h list, int **ids, int *count);
int ctsvc_client_db_update_records(contacts_h contact, contacts_list_h list);
int ctsvc_client_db_delete_records(contacts_h contact, const char* view_uri, int ids[], int count);
int ctsvc_client_db_replace_records(contacts_h contact, contacts_list_h list, int ids[], int count);
int ctsvc_client_db_get_changes_by_version(contacts_h contact, const char* view_uri, int addressbook_id, int ctsvc_client_db_version, contacts_list_h* record_list, int* current_ctsvc_client_db_version);
int ctsvc_client_db_get_current_version(contacts_h contact, int* ctsvc_client_db_version);
int ctsvc_client_db_search_records(contacts_h contact, const char* view_uri, const char *keyword, int offset, int limit, contacts_list_h* out_list);
int ctsvc_client_db_search_records_with_range(contacts_h contact, const char* view_uri, const char *keyword, int offset, int limit, int range, contacts_list_h* out_list);
int ctsvc_client_db_search_records_with_query(contacts_h contact, contacts_query_h query, const char *keyword, int offset, int limit, contacts_list_h* out_list);
int ctsvc_client_db_get_last_change_version(contacts_h contact, int* last_version);
int ctsvc_client_db_get_status(contacts_h contact, contacts_db_status_e *status);
int ctsvc_client_db_add_status_changed_cb(contacts_h contact, contacts_db_status_changed_cb cb, void* user_data);
int ctsvc_client_db_remove_status_changed_cb(contacts_h contact, contacts_db_status_changed_cb cb, void* user_data);

#endif /* __CTSVC_CLIENT_DB_HELPER_H__ */
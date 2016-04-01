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

#ifndef __CTSVC_DB_INIT_H__
#define __CTSVC_DB_INIT_H__

#include "contacts.h"
#include "ctsvc_struct.h"

typedef int (*ctsvc_db_insert_record_cb)(contacts_record_h record, int *id);
typedef int (*ctsvc_db_get_record_cb)(int id, contacts_record_h *out_record);
typedef int (*ctsvc_db_update_record_cb)(contacts_record_h record);
typedef int (*ctsvc_db_delete_record_cb)(int id);
typedef int (*ctsvc_db_replace_record_cb)(contacts_record_h record, int id);

typedef int (*ctsvc_db_insert_records_cb)(const contacts_list_h in_list, int **ids);
typedef int (*ctsvc_db_update_records_cb)(const contacts_list_h in_list);
typedef int (*ctsvc_db_delete_records_cb)(int ids[], int count);
typedef int (*ctsvc_db_replace_records_cb)(const contacts_list_h in_list, int ids[],
		int count);

typedef int (*ctsvc_db_get_all_records_cb)(int offset, int limit, contacts_list_h *out_list);
typedef int (*ctsvc_db_get_records_with_query_cb)(contacts_query_h query, int offset,
		int limit, contacts_list_h *out_list);
typedef int (*ctsvc_db_get_count_cb)(int *out_count);
typedef int (*ctsvc_db_get_count_with_query_cb)(contacts_query_h query, int *out_count);

typedef struct {
	bool is_query_only;
	ctsvc_db_insert_record_cb insert_record;
	ctsvc_db_get_record_cb get_record;
	ctsvc_db_update_record_cb update_record;
	ctsvc_db_delete_record_cb delete_record;
	ctsvc_db_replace_record_cb replace_record;
	ctsvc_db_get_all_records_cb get_all_records;
	ctsvc_db_get_records_with_query_cb get_records_with_query;
	ctsvc_db_insert_records_cb insert_records;
	ctsvc_db_update_records_cb update_records;
	ctsvc_db_delete_records_cb delete_records;
	ctsvc_db_replace_records_cb replace_records;
	ctsvc_db_get_count_cb get_count;
	ctsvc_db_get_count_with_query_cb get_count_with_query;
} ctsvc_db_plugin_info_s;

int ctsvc_db_init();
int ctsvc_db_deinit();

int ctsvc_db_plugin_init();
int ctsvc_db_plugin_deinit();

int ctsvc_db_get_table_name(const char *view_uri, const char **out_table);
ctsvc_db_plugin_info_s* ctsvc_db_get_plugin_info(ctsvc_record_type_e type);

int ctsvc_required_read_permission(const char *view_uri);
int ctsvc_required_write_permission(const char *view_uri);

bool ctsvc_should_ab_access_control(const char *view_uri);

#endif /* __CTSVC_DB_INIT_H__ */

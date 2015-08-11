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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_db_schema.h"
#include "ctsvc_db_sqlite.h"
#include "ctsvc_db_utils.h"
#include "ctsvc_list.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_query.h"
#include "ctsvc_record.h"
#include "ctsvc_db_access_control.h"

static int __ctsvc_db_grouprelation_insert_record(contacts_record_h record, int *id);
static int __ctsvc_db_grouprelation_get_record(int id, contacts_record_h* out_record);
static int __ctsvc_db_grouprelation_update_record(contacts_record_h record);
static int __ctsvc_db_grouprelation_delete_record(int id);
static int __ctsvc_db_grouprelation_get_all_records(int offset, int limit, contacts_list_h* out_list);
static int __ctsvc_db_grouprelation_get_records_with_query(contacts_query_h query, int offset, int limit, contacts_list_h* out_list);
//static int __ctsvc_db_grouprelation_insert_records(const contacts_list_h in_list, int **ids);
//static int __ctsvc_db_grouprelation_update_records(const contacts_list_h in_list);
//static int __ctsvc_db_grouprelation_delete_records(int ids[], int count);

ctsvc_db_plugin_info_s ctsvc_db_plugin_grouprelation = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_grouprelation_insert_record,
	.get_record = __ctsvc_db_grouprelation_get_record,
	.update_record = __ctsvc_db_grouprelation_update_record,
	.delete_record = __ctsvc_db_grouprelation_delete_record,
	.get_all_records = __ctsvc_db_grouprelation_get_all_records,
	.get_records_with_query = __ctsvc_db_grouprelation_get_records_with_query,
	.insert_records = NULL, //__ctsvc_db_grouprelation_insert_records,
	.update_records = NULL, //__ctsvc_db_grouprelation_update_records,
	.delete_records = NULL, //__ctsvc_db_grouprelation_delete_records
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};

static int __ctsvc_db_grouprelation_insert_record(contacts_record_h record, int *id)
{
	CTS_ERR("Please use the contacts_group_add_contact()");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_db_grouprelation_get_record(int id, contacts_record_h* out_record)
{
	CTS_ERR("Not support get group-relation");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_db_grouprelation_update_record(contacts_record_h record)
{
	CTS_ERR("Not support update group-relation");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_db_grouprelation_delete_record(int id)
{
	CTS_ERR("Please use the contacts_group_remove_contact()");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_db_grouprelation_get_all_records(int offset, int limit, contacts_list_h* out_list)
{
	int len;
	int ret;
	contacts_list_h list;
	ctsvc_group_relation_s *grouprel;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	char *temp;

	len = snprintf(query, sizeof(query),
			"SELECT group_id, contact_id, group_name FROM "CTSVC_DB_VIEW_GROUP_RELATION);

	if (0 != limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}
		contacts_record_create(_contacts_group_relation._uri, (contacts_record_h*)&grouprel);

		if (grouprel) {
			grouprel->group_id = ctsvc_stmt_get_int(stmt, 0);
			grouprel->id = grouprel->group_id;
			grouprel->contact_id = ctsvc_stmt_get_int(stmt, 1);
			temp = ctsvc_stmt_get_text(stmt, 2);
			grouprel->group_name = SAFE_STRDUP(temp);

			ctsvc_list_prepend(list, (contacts_record_h)grouprel);
		}
	}

	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_grouprelation_get_records_with_query(contacts_query_h query,
		int offset, int limit, contacts_list_h* out_list)
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_group_relation_s *group_relation;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s *)query;

	ret = ctsvc_db_make_get_records_query_stmt(s_query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_db_make_get_records_query_stmt fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		contacts_record_create(_contacts_group_relation._uri, &record);
		group_relation = (ctsvc_group_relation_s*)record;
		if (0 == s_query->projection_count)
			field_count = s_query->property_count;
		else {
			field_count = s_query->projection_count;

			if (CONTACTS_ERROR_NONE != ctsvc_record_set_projection_flags(record, s_query->projection, s_query->projection_count, s_query->property_count)) {
				ASSERT_NOT_REACHED("To set projection is Fail.\n");
			}
		}

		for (i=0;i<field_count;i++) {
			char *temp;
			int property_id;
			if (0 == s_query->projection_count)
				property_id = s_query->properties[i].property_id;
			else
				property_id = s_query->projection[i];

			if (CTSVC_PROPERTY_GROUP_RELATION_ID == property_id)
				continue;

			switch(property_id) {
			case CTSVC_PROPERTY_GROUP_RELATION_CONTACT_ID:
				group_relation->contact_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_GROUP_RELATION_GROUP_ID:
				group_relation->group_id = ctsvc_stmt_get_int(stmt, i);
				group_relation->id = group_relation->group_id;
				break;
			case CTSVC_PROPERTY_GROUP_RELATION_GROUP_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(group_relation->group_name);
				group_relation->group_name = SAFE_STRDUP(temp);
				break;
			default:
				break;
			}
		}
		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}

//static int __ctsvc_db_grouprelation_insert_records(const contacts_list_h in_list, int **ids) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_grouprelation_update_records(const contacts_list_h in_list) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_grouprelation_delete_records(int ids[], int count) { return CONTACTS_ERROR_NONE; }

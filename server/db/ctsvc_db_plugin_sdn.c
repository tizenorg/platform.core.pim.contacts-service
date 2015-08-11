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
#include "ctsvc_db_plugin_sdn.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_query.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"
#include "ctsvc_server_utils.h"

//static int __ctsvc_db_sdn_insert_record(contacts_record_h record, int *id);
static int __ctsvc_db_sdn_get_record(int id, contacts_record_h* record);
//static int __ctsvc_db_sdn_update_record(contacts_record_h record);
//static int __ctsvc_db_sdn_delete_record(int id);
static int __ctsvc_db_sdn_get_all_records(int offset, int limit, contacts_list_h* out_list);
static int __ctsvc_db_sdn_get_records_with_query(contacts_query_h query, int offset, int limit, contacts_list_h* out_list);
//static int __ctsvc_db_sdn_insert_records(const contacts_list_h in_list, int **ids);
//static int __ctsvc_db_sdn_update_records(const contacts_list_h in_list);
//static int __ctsvc_db_sdn_delete_records(int ids[], int count);

ctsvc_db_plugin_info_s ctsvc_db_plugin_sdn = {
	.is_query_only = false,
	.insert_record = NULL,//__ctsvc_db_sdn_insert_record,
	.get_record = __ctsvc_db_sdn_get_record,
	.update_record = NULL,//__ctsvc_db_sdn_update_record,
	.delete_record = NULL,//__ctsvc_db_sdn_delete_record,
	.get_all_records = __ctsvc_db_sdn_get_all_records,
	.get_records_with_query = __ctsvc_db_sdn_get_records_with_query,
	.insert_records = NULL,//__ctsvc_db_sdn_insert_records,
	.update_records = NULL,//__ctsvc_db_sdn_update_records,
	.delete_records = NULL,//__ctsvc_db_sdn_delete_records
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};

static int __ctsvc_db_sdn_value_set(cts_stmt stmt, contacts_record_h *record)
{
	int i;
	int ret;
	char *temp;
	ctsvc_sdn_s *sdn;

	ret = contacts_record_create(_contacts_sdn._uri, record);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create Fail(%d)", ret);
	sdn = (ctsvc_sdn_s*)*record;

	i = 0;
	sdn->id = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	sdn->name = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	sdn->number = SAFE_STRDUP(temp);
	sdn->sim_slot_no = ctsvc_stmt_get_int(stmt, i++);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_sdn_get_record(int id, contacts_record_h* out_record)
{
	RETVM_IF(false == ctsvc_server_have_telephony_feature(), CONTACTS_ERROR_NOT_SUPPORTED, "Telephony feature disabled");

	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_record_h record;

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	snprintf(query, sizeof(query),
				"SELECT id, name, number, sim_slot_no FROM "CTS_TABLE_SDN" WHERE id = %d", id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		CTS_ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}

	ret = __ctsvc_db_sdn_value_set(stmt, &record);

	ctsvc_stmt_finalize(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_sdn_value_set(ALL) Fail(%d)", ret);
		return ret;
	}

	*out_record = record;

	return CONTACTS_ERROR_NONE;
}

#if 0
static int __ctsvc_db_sdn_insert_record(contacts_record_h record, int *id)
{
	int ret;
	ctsvc_sdn_s *sdn = (ctsvc_sdn_s*)record;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == sdn->name, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(CTSVC_RECORD_SDN != sdn->base.r_type, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : record is invalid type(%d)", sdn->base.r_type);

	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"INSERT INTO "CTS_TABLE_SDN"(name, number, sim_slot_no) VALUES(?, ?, ?)");

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	ctsvc_stmt_bind_text(stmt, 1, sdn->name);
	ctsvc_stmt_bind_text(stmt, 2, sdn->number);
	ctsvc_stmt_bind_int(stmt, 3, sdn->sim_slot_no);

	ret = ctsvc_begin_trans();
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_begin_trans() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		return ret;
	}

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}

	//sdn->id = ctsvc_db_get_last_insert_id();
	if (id)
		*id = ctsvc_db_get_last_insert_id();
	ctsvc_stmt_finalize(stmt);

	ctsvc_set_sdn_noti();
	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_sdn_update_record(contacts_record_h record)
{
	int sdn_id;
	int ret = CONTACTS_ERROR_NONE;
	char* set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	ctsvc_sdn_s *sdn = (ctsvc_sdn_s*)record;
	char query[CTS_SQL_MIN_LEN] = {0};

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(CTSVC_RECORD_SDN != sdn->base.r_type, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : record is invalid type(%d)", sdn->base.r_type);
	RETVM_IF(!sdn->id, CONTACTS_ERROR_INVALID_PARAMETER, "sdn of contact has no ID.");
	RETV_IF(NULL == sdn->name, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(CTSVC_PROPERTY_FLAG_DIRTY != (sdn->base.property_flag & CTSVC_PROPERTY_FLAG_DIRTY), CONTACTS_ERROR_NONE, "No update");

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "DB error : ctsvc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_SDN" WHERE id = %d", sdn->id);
	ret = ctsvc_query_get_first_int_result(query, &sdn_id);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_query_get_first_int_result Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	do {
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_create_set_query(record, &set, &bind_text))) break;
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_update_record_with_set_query(set, bind_text, CTS_TABLE_SDN, sdn->id))) break;
		ctsvc_set_sdn_noti();
	} while (0);

	CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s *)record);
	CONTACTS_FREE(set);
	if (bind_text) {
		for (cursor=bind_text;cursor;cursor=cursor->next)
			CONTACTS_FREE(cursor->data);
		g_slist_free(bind_text);
	}

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "DB error : ctsvc_end_trans() Fail(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_sdn_delete_record(int sdn_id)
{
	int ret;
	int id;
	char query[CTS_SQL_MAX_LEN] = {0};

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "DB error : ctsvc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_SDN" WHERE id = %d", sdn_id);
	ret = ctsvc_query_get_first_int_result(query, &id);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_query_get_first_int_result Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_SDN" WHERE id = %d", sdn_id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_db_change();
	if (ret <= 0) {
		ret = CONTACTS_ERROR_NO_DATA;
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_sdn_noti();
	ret = ctsvc_end_trans(true);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	}

	return CONTACTS_ERROR_NONE;
}
#endif

static int __ctsvc_db_sdn_get_all_records(int offset, int limit,
	contacts_list_h* out_list)
{
	RETVM_IF(false == ctsvc_server_have_telephony_feature(), CONTACTS_ERROR_NOT_SUPPORTED, "Telephony feature disabled");

	int ret;
	int len;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;

	len = snprintf(query, sizeof(query),
				"SELECT id, name, number, sim_slot_no FROM "CTS_TABLE_SDN);

	if (0 != limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}
		__ctsvc_db_sdn_value_set(stmt, &record);

		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_sdn_get_records_with_query(contacts_query_h query, int offset, int limit, contacts_list_h* out_list)
{
	RETVM_IF(false == ctsvc_server_have_telephony_feature(), CONTACTS_ERROR_NOT_SUPPORTED, "Telephony feature disabled");

	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_sdn_s *sdn;

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

		contacts_record_create(_contacts_sdn._uri, &record);
		sdn = (ctsvc_sdn_s*)record;
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

			switch(property_id) {
			case CTSVC_PROPERTY_SDN_ID:
				sdn->id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_SDN_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(sdn->name);
				sdn->name = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_SDN_NUMBER:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(sdn->number);
				sdn->number = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_SDN_SIM_SLOT_NO:
				sdn->sim_slot_no = ctsvc_stmt_get_int(stmt, i);
				break;
			default:
				break;
			}
		}
		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;

	return CONTACTS_ERROR_NONE;
}

#if 0
static int __ctsvc_db_sdn_insert_records(const contacts_list_h in_list, int **ids)
{
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_sdn_update_records(const contacts_list_h in_list)
{
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_sdn_delete_records(int ids[], int count)
{
	return CONTACTS_ERROR_NONE;
}
#endif

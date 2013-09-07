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
#include <account.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_utils.h"
#include "ctsvc_list.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_query.h"
#include "ctsvc_db_plugin_person_helper.h"
#include "ctsvc_person.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"
#include "ctsvc_db_access_control.h"

static int __ctsvc_db_addressbook_insert_record( contacts_record_h record, int *id );
static int __ctsvc_db_addressbook_get_record( int id, contacts_record_h* record );
static int __ctsvc_db_addressbook_update_record( contacts_record_h record );
static int __ctsvc_db_addressbook_delete_record( int id );
static int __ctsvc_db_addressbook_get_all_records( int offset, int limit, contacts_list_h* out_list );
static int __ctsvc_db_addressbook_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list );
//static int __ctsvc_db_addressbook_insert_records(const contacts_list_h in_list, int **ids);
//static int __ctsvc_db_addressbook_update_records(const contacts_list_h in_list);
//static int __ctsvc_db_addressbook_delete_records(int ids[], int count);

ctsvc_db_plugin_info_s ctsvc_db_plugin_addressbook = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_addressbook_insert_record,
	.get_record = __ctsvc_db_addressbook_get_record,
	.update_record = __ctsvc_db_addressbook_update_record,
	.delete_record = __ctsvc_db_addressbook_delete_record,
	.get_all_records = __ctsvc_db_addressbook_get_all_records,
	.get_records_with_query = __ctsvc_db_addressbook_get_records_with_query,
	.insert_records = NULL,//__ctsvc_db_addressbook_insert_records,
	.update_records = NULL,//__ctsvc_db_addressbook_update_records,
	.delete_records = NULL,//__ctsvc_db_addressbook_delete_records
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};

static int __ctsvc_db_addressbook_value_set(cts_stmt stmt, contacts_record_h *record)
{
	int i;
	int ret;
	char *temp;
	ctsvc_addressbook_s *addressbook;

	ret = contacts_record_create(_contacts_address_book._uri, record);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create is failed(%d)", ret);
	addressbook = (ctsvc_addressbook_s*)*record;

	i = 0;
	addressbook->id = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	addressbook->name = SAFE_STRDUP(temp);
	addressbook->account_id = ctsvc_stmt_get_int(stmt, i++);
	addressbook->mode = ctsvc_stmt_get_int(stmt, i++);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_addressbook_get_record( int id, contacts_record_h* out_record )
{
	int ret;
	int len;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_record_h record;

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	len = snprintf(query, sizeof(query),
				"SELECT addressbook_id, addressbook_name, account_id, mode, last_sync_ver "
				"FROM "CTS_TABLE_ADDRESSBOOKS" WHERE addressbook_id = %d", id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}

	ret = __ctsvc_db_addressbook_value_set(stmt, &record);

	cts_stmt_finalize(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_addressbook_value_set(ALL) Failed(%d)", ret);
		return ret;
	}

	*out_record = record;

	return CONTACTS_ERROR_NONE;
}


static int __ctsvc_db_addressbook_insert_record( contacts_record_h record, int *id )
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_addressbook_s *addressbook = (ctsvc_addressbook_s*)record;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == addressbook->name, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(CTSVC_RECORD_ADDRESSBOOK != addressbook->base.r_type, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : record is invalid type(%d)", addressbook->base.r_type);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "DB error : ctsvc_begin_trans() Failed(%d)", ret);

	// Can not insert addressbook which has same account_id
	int addresbook_id;
	account_h account = NULL;
	snprintf(query, sizeof(query),
		"SELECT addressbook_id FROM "CTS_TABLE_ADDRESSBOOKS" WHERE account_id = %d",
		addressbook->account_id);
	ret = ctsvc_query_get_first_int_result(query, &addresbook_id);
	if (CONTACTS_ERROR_NO_DATA != ret) {
		ctsvc_end_trans(false);
		if (CONTACTS_ERROR_NONE == ret) {
			CTS_ERR("One addressbook which has account_id(%d) already exists", addressbook->account_id);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		else {
			CTS_ERR("DB error : ctsvc_query_get_first_int_result() Failed (%d)", ret);
			return ret;
		}
	}

	if (0 < addressbook->account_id) {
		// check account_id validation
		ret = account_create(&account);
		if (ACCOUNT_ERROR_NONE != ret) {
			CTS_ERR("account_create() Failed(%d)", ret);
			ctsvc_end_trans(false);
			return CONTACTS_ERROR_SYSTEM;
		}
		ret = account_query_account_by_account_id(addressbook->account_id, &account);
		if (ACCOUNT_ERROR_NONE != ret) {
			CTS_ERR("account_query_account_by_account_id Faild(%d) : account_id(%d)", ret, addressbook->account_id);
			ret = account_destroy(account);
			WARN_IF(ret != ACCOUNT_ERROR_NONE, "account_destroy Fail(%d)", ret);
			ctsvc_end_trans(false);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		ret = account_destroy(account);
		WARN_IF(ret != ACCOUNT_ERROR_NONE, "account_destroy Fail(%d)", ret);
	}

	snprintf(query, sizeof(query),
			"INSERT INTO %s(addressbook_name, account_id, mode) "
			"VALUES(?, %d, %d)",
			CTS_TABLE_ADDRESSBOOKS, addressbook->account_id, addressbook->mode);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("DB error : cts_query_prepare() Failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	cts_stmt_bind_text(stmt, 1, addressbook->name);

	/* DOING JOB */
	do {
		ret = cts_stmt_step(stmt);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			break;
		}

		//int index = cts_db_get_last_insert_id();
		if (id)
			*id = cts_db_get_last_insert_id();
		cts_stmt_finalize(stmt);

		ctsvc_set_addressbook_noti();
		ret = ctsvc_end_trans(true);
		if(ret < CONTACTS_ERROR_NONE )
		{
			CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
			return ret;
		}
		//addressbook->id = index;

		// SUCCESS
		return CONTACTS_ERROR_NONE;

	} while(0);

	/* ROLLBACK TRANSACTION */
	ctsvc_end_trans(false);

	return ret;
}

static int __ctsvc_db_addressbook_update_record( contacts_record_h record )
{
	int ret = CONTACTS_ERROR_NONE;
	char* set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	ctsvc_addressbook_s *addressbook = (ctsvc_addressbook_s *)record;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(CTSVC_PROPERTY_FLAG_DIRTY != (addressbook->base.property_flag & CTSVC_PROPERTY_FLAG_DIRTY), CONTACTS_ERROR_NONE, "No update");
	RETV_IF(NULL == addressbook->name, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(CTSVC_RECORD_ADDRESSBOOK != addressbook->base.r_type, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : record is invalid type(%d)", addressbook->base.r_type);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	do {
		char query[CTS_SQL_MAX_LEN] = {0};
		cts_stmt stmt = NULL;

		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_create_set_query(record, &set, &bind_text))) break;
		if (NULL == set || '\0' == *set)
			break;

		snprintf(query, sizeof(query), "UPDATE %s SET %s WHERE addressbook_id = %d", CTS_TABLE_ADDRESSBOOKS, set, addressbook->id);
		stmt = cts_query_prepare(query);
		if (NULL == stmt) {
			CTS_ERR("DB error : cts_query_prepare() Failed");
			ret = CONTACTS_ERROR_DB;
			break;
		}
		if (bind_text) {
			int i = 0;
			for (cursor=bind_text,i=1;cursor;cursor=cursor->next,i++) {
				const char *text = cursor->data;
				if (text && *text)
					cts_stmt_bind_text(stmt, i, text);
			}
		}
		ret = cts_stmt_step(stmt);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			break;
		}
		cts_stmt_finalize(stmt);

		ctsvc_set_addressbook_noti();
	} while (0);
	CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s *)record);
	CONTACTS_FREE(set);

	if (bind_text) {
		for (cursor=bind_text;cursor;cursor=cursor->next)
			CONTACTS_FREE(cursor->data);
		g_slist_free(bind_text);
	}

	if (CONTACTS_ERROR_NONE != ret) {
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "DB error : ctsvc_end_trans() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_db_addressbook_reset_internal_addressbook(void)
{
	CTS_FN_CALL;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "UPDATE %s SET deleted=1, person_id=0, "
			"changed_ver = ((SELECT ver FROM cts_version) + 1) WHERE addressbook_id = %d",
			CTS_TABLE_CONTACTS, 0 /*CTS_ADDRESSBOOK_INTERNAL*/);

	/* BEGIN_TRANSACTION */
	int ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE > ret, ret, "DB error : ctsvc_begin_trans() Failed(%d)", ret);

	/* DOING JOB */
	do {
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
			break;
		}

		snprintf(query, sizeof(query), "DELETE FROM %s WHERE addressbook_id = %d",
				CTS_TABLE_MY_PROFILES, 0);

		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret)
		{
			CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
			break;
		}

		snprintf(query, sizeof(query), "DELETE FROM %s WHERE addressbook_id = %d",
				CTS_TABLE_GROUPS, 0 /*CTS_ADDRESSBOOK_INTERNAL*/);
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
			break;
		}

		snprintf(query, sizeof(query), "DELETE FROM %s WHERE addressbook_id = %d",
				CTS_TABLE_GROUP_DELETEDS, 0 /*CTS_ADDRESSBOOK_INTERNAL*/);
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
			break;
		}

		snprintf(query, sizeof(query), "DELETE FROM %s WHERE addressbook_id = %d",
				CTS_TABLE_DELETEDS, 0 /*CTS_ADDRESSBOOK_INTERNAL*/);
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
			break;
		}

		ret = ctsvc_person_do_garbage_collection();
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : cts_person_garbagecollection() Failed(%d)", ret);
			break;
		}

		ctsvc_set_contact_noti();
		ctsvc_set_my_profile_noti();
		// person noti will set in ctsvc_person_do_garbage_collection : ctsvc_set_person_noti();
		ctsvc_set_group_noti();
		ret = ctsvc_end_trans(true);
		if (ret < CONTACTS_ERROR_NONE) {
			CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
			return ret;
		}

		return CONTACTS_ERROR_NONE;
	} while(0);

	/* ROLLBACK TRANSACTION */
	ctsvc_end_trans(false);

	return ret;
}

static int __ctsvc_db_addressbook_delete_record( int addressbook_id )
{
	CTS_FN_CALL;

	if (0 /*CTS_ADDRESSBOOK_INTERNAL*/ == addressbook_id)
		return __ctsvc_db_addressbook_reset_internal_addressbook();

	char query[CTS_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query), "DELETE FROM %s WHERE addressbook_id = %d",
			CTS_TABLE_ADDRESSBOOKS, addressbook_id);

	/* BEGIN_TRANSACTION */
	int ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE > ret, ret, "DB error : ctsvc_begin_trans() Failed(%d)", ret);

	/* DOING JOB */
	do {
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
			break;
		}

		ret = cts_db_change();
		if (0 < ret) {
			ctsvc_set_my_profile_noti();
			ctsvc_set_contact_noti();
			// person noti will set in ctsvc_person_do_garbage_collection : ctsvc_set_person_noti();
			ctsvc_set_group_noti();
			ctsvc_set_addressbook_noti();
		}
		else {
			ret = CONTACTS_ERROR_NO_DATA;
			break;
		}

		ret = ctsvc_person_do_garbage_collection();
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : cts_person_garbagecollection() Failed(%d)", ret);
			break;
		}

		ret = ctsvc_end_trans(true);
		if (ret < CONTACTS_ERROR_NONE)
		{
			CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
			return ret;
		}

		return CONTACTS_ERROR_NONE;

	} while(0);

	ctsvc_end_trans(false);

	return ret;
}

static int __ctsvc_db_addressbook_get_all_records( int offset, int limit,
	contacts_list_h* out_list )
{
	int ret;
	int len;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;

	len = snprintf(query, sizeof(query),
				"SELECT addressbook_id, addressbook_name, account_id, mode, last_sync_ver "
				"FROM "CTS_TABLE_ADDRESSBOOKS" ORDER BY account_id, addressbook_id");

	if (0 != limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB , "DB error : cts_query_prepare() Failed");

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return CONTACTS_ERROR_NO_DATA;
		}
		__ctsvc_db_addressbook_value_set(stmt, &record);

		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_addressbook_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list )
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_addressbook_s *addressbook;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s *)query;

	ret = ctsvc_db_make_get_records_query_stmt(s_query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_db_make_get_records_query_stmt fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return CONTACTS_ERROR_NO_DATA;
		}

		contacts_record_create(_contacts_address_book._uri, &record);
		addressbook = (ctsvc_addressbook_s*)record;
		if (0 == s_query->projection_count)
			field_count = s_query->property_count;
		else {
			field_count = s_query->projection_count;
			ret = ctsvc_record_set_projection_flags(record, s_query->projection,
					s_query->projection_count, s_query->property_count);

			if (CONTACTS_ERROR_NONE != ret)
				ASSERT_NOT_REACHED("To set projection is failed.\n");
		}

		for(i=0;i<field_count;i++) {
			char *temp;
			int property_id;
			if (0 == s_query->projection_count)
				property_id = s_query->properties[i].property_id;
			else
				property_id = s_query->projection[i];

			switch(property_id) {
			case CTSVC_PROPERTY_ADDRESSBOOK_ID:
				addressbook->id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_ADDRESSBOOK_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				addressbook->name = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_ADDRESSBOOK_MODE:
				addressbook->mode = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_ADDRESSBOOK_ACCOUNT_ID:
				addressbook->account_id = ctsvc_stmt_get_int(stmt, i);
				break;
			default:
				break;
			}
		}
		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;

	return CONTACTS_ERROR_NONE;
}

#if 0
static int __ctsvc_db_addressbook_insert_records(const contacts_list_h in_list, int **ids)
{
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_addressbook_update_records(const contacts_list_h in_list)
{
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_addressbook_delete_records(int ids[], int count)
{
	return CONTACTS_ERROR_NONE;
}
#endif

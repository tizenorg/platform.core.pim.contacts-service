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
#include <stdio.h>
#include <badge.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_normalize.h"
#include "ctsvc_utils.h"
#include "ctsvc_list.h"
#include "ctsvc_record.h"
#include "ctsvc_db_query.h"
#include "ctsvc_db_init.h"
#include "ctsvc_notification.h"

#ifdef _CONTACTS_IPC_SERVER
#include "ctsvc_server_change_subject.h"
#endif

static int __ctsvc_db_phonelog_insert_record( contacts_record_h record, int *id );
static int __ctsvc_db_phonelog_get_record( int id, contacts_record_h* out_record );
static int __ctsvc_db_phonelog_update_record( contacts_record_h record );
static int __ctsvc_db_phonelog_delete_record( int id );
static int __ctsvc_db_phonelog_get_all_records( int offset, int limit, contacts_list_h* out_list );
static int __ctsvc_db_phonelog_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list );
//static int __ctsvc_db_phonelog_insert_records(const contacts_list_h in_list, int **ids);
//static int __ctsvc_db_phonelog_update_records(const contacts_list_h in_list);
//static int __ctsvc_db_phonelog_delete_records( int ids[], int count);

ctsvc_db_plugin_info_s ctsvc_db_plugin_phonelog = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_phonelog_insert_record,
	.get_record = __ctsvc_db_phonelog_get_record,
	.update_record = __ctsvc_db_phonelog_update_record,
	.delete_record = __ctsvc_db_phonelog_delete_record,
	.get_all_records = __ctsvc_db_phonelog_get_all_records,
	.get_records_with_query = __ctsvc_db_phonelog_get_records_with_query,
	.insert_records = NULL,//__ctsvc_db_phonelog_insert_records,
	.update_records = NULL,//__ctsvc_db_phonelog_update_records,
	.delete_records = NULL,//__ctsvc_db_phonelog_delete_records
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};

static int __ctsvc_db_phonelog_value_set(cts_stmt stmt, contacts_record_h *record)
{
	int i;
	int ret;
	char *temp;
	ctsvc_phonelog_s *phonelog;

	ret = contacts_record_create(_contacts_phone_log._uri, record);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create is failed(%d)", ret);
	phonelog = (ctsvc_phonelog_s*)*record;

	i = 0;
	phonelog->id = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	phonelog->address = SAFE_STRDUP(temp);
	phonelog->person_id = ctsvc_stmt_get_int(stmt, i++);
	phonelog->log_type = ctsvc_stmt_get_int(stmt, i++);
	phonelog->log_time = ctsvc_stmt_get_int(stmt, i++);
	phonelog->extra_data1 = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	phonelog->extra_data2 = SAFE_STRDUP(temp);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_phonelog_get_record( int id, contacts_record_h* out_record )
{
	int ret;
	int len;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_record_h record;

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	len = snprintf(query, sizeof(query),
			"SELECT id, number, person_id, log_type, log_time, data1, data2 "
			"FROM "CTS_TABLE_PHONELOGS" WHERE id = %d", id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CONTACTS_ERROR_NO_DATA;
	}

	ret = __ctsvc_db_phonelog_value_set(stmt, &record);

	cts_stmt_finalize(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_phonelog_value_set(ALL) Failed(%d)", ret);
		return ret;
	}

	*out_record = record;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_phonelog_update_record( contacts_record_h record )
{
	int phonelog_id;
	char query[CTS_SQL_MIN_LEN] = {0};
	ctsvc_phonelog_s *phonelog = (ctsvc_phonelog_s *)record;
	int ret = CONTACTS_ERROR_NONE;
	char* set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;

	RETVM_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER, "Invaild parameter : record is null");
	RETVM_IF(phonelog->id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : The phone_log has ID(%d)", phonelog->id);
	RETVM_IF(phonelog->log_type != CONTACTS_PLOG_TYPE_VOICE_INCOMMING_SEEN &&
			phonelog->log_type != CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_SEEN, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invaild parameter : the type is can not updated(%d)", phonelog->log_type);
	RETVM_IF(CTSVC_PROPERTY_FLAG_DIRTY != (phonelog->base.property_flag & CTSVC_PROPERTY_FLAG_DIRTY), CONTACTS_ERROR_NONE, "No update");

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_PHONELOGS" WHERE id = %d", phonelog->id);
	ret = ctsvc_query_get_first_int_result(query, &phonelog_id);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_query_get_first_int_result Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	do {
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_create_set_query(record, &set, &bind_text))) break;
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_update_record_with_set_query(set, bind_text, CTS_TABLE_PHONELOGS, phonelog->id))) break;

		if (cts_db_change()) {
			ctsvc_set_phonelog_noti();

#ifdef _CONTACTS_IPC_SERVER
			ctsvc_change_subject_add_changed_phone_log_id(CONTACTS_CHANGE_UPDATED, phonelog->id);
#endif
		}
	} while (0);

	CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s *)record);
	CONTACTS_FREE(set);
	if (bind_text) {
		for (cursor=bind_text;cursor;cursor=cursor->next)
			CONTACTS_FREE(cursor->data);
		g_slist_free(bind_text);
	}

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "DB error : ctsvc_end_trans() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_phonelog_delete_record( int id )
{
	int ret;
	int phonelog_id;
	char query[CTS_SQL_MAX_LEN] = {0};

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_PHONELOGS" WHERE id = %d", id);
	ret = ctsvc_query_get_first_int_result(query, &phonelog_id);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_query_get_first_int_result Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE id = %d",
			CTS_TABLE_PHONELOGS, id);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_phonelog_noti();
	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
	{
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_phonelog_get_all_records( int offset, int limit,
		contacts_list_h* out_list )
{
	int ret;
	int len;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;

	len = snprintf(query, sizeof(query),
			"SELECT id, number, person_id, log_type, log_time, data1, data2 FROM "CTS_TABLE_PHONELOGS);

	if (0 < limit) {
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
		__ctsvc_db_phonelog_value_set(stmt, &record);

		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_phonelog_get_records_with_query( contacts_query_h query, int offset,
		int limit, contacts_list_h* out_list )
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_phonelog_s *phonelog;

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
			return ret;
		}

		contacts_record_create(_contacts_phone_log._uri, &record);
		phonelog = (ctsvc_phonelog_s*)record;
		if (0 == s_query->projection_count)
			field_count = s_query->property_count;
		else
		{
			field_count = s_query->projection_count;

			if( CONTACTS_ERROR_NONE != ctsvc_record_set_projection_flags(record, s_query->projection, s_query->projection_count, s_query->property_count) )
			{
				ASSERT_NOT_REACHED("To set projection is failed.\n");
			}
		}

		for(i=0;i<field_count;i++) {
			char *temp;
			int property_id;
			if (0 == s_query->projection_count)
				property_id = s_query->properties[i].property_id;
			else
				property_id = s_query->projection[i];

			switch(property_id) {
			case CTSVC_PROPERTY_PHONELOG_ID:
				phonelog->id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_PHONELOG_PERSON_ID:
				phonelog->person_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_PHONELOG_ADDRESS:
				temp = ctsvc_stmt_get_text(stmt, i);
				phonelog->address = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_PHONELOG_LOG_TIME:
				phonelog->log_time = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_PHONELOG_LOG_TYPE:
				phonelog->log_type = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_PHONELOG_EXTRA_DATA1:
				phonelog->extra_data1= ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_PHONELOG_EXTRA_DATA2:
				temp = ctsvc_stmt_get_text(stmt, i);
				phonelog->extra_data2= SAFE_STRDUP(temp);
				break;
			default:
				break;
			}
		}
		ctsvc_list_prepend(list, record);
	}

	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;

	return CONTACTS_ERROR_NONE;
}
//static int __ctsvc_db_phonelog_insert_records(const contacts_list_h in_list, int **ids) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_phonelog_update_records(const contacts_list_h in_list) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_phonelog_delete_records( int ids[], int count) { return CONTACTS_ERROR_NONE; }

static int __ctsvc_cb_phonelog_increase_outgoing_count(int person_id)
{
	int ret;
	int id;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT person_id FROM %s WHERE person_id = %d and usage_type = %d ",
			CTS_TABLE_CONTACT_STAT, person_id, CONTACTS_USAGE_STAT_TYPE_OUTGOING_CALL);

	ret = ctsvc_query_get_first_int_result(query, &id);
	if (CONTACTS_ERROR_NO_DATA == ret) {
		snprintf(query, sizeof(query),
				"INSERT INTO %s(person_id, usage_type, times_used) VALUES(%d, %d, 1)",
				CTS_TABLE_CONTACT_STAT, person_id, CONTACTS_USAGE_STAT_TYPE_OUTGOING_CALL);
	}
	else {
		snprintf(query, sizeof(query),
				"UPDATE %s SET times_used = times_used + 1 WHERE person_id = %d and usage_type = %d",
				CTS_TABLE_CONTACT_STAT, person_id, CONTACTS_USAGE_STAT_TYPE_OUTGOING_CALL);
	}

	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "DB error : cts_query_exec() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static int  __ctsvc_db_phonelog_insert(ctsvc_phonelog_s *phonelog, int *id)
{
	int ret;
	cts_stmt stmt = NULL;
	char clean_num[CTSVC_NUMBER_MAX_LEN] = {0};
	char query[CTS_SQL_MAX_LEN] = {0};
	char normal_num[CTSVC_NUMBER_MAX_LEN] = {0};

	RETVM_IF(phonelog->log_type <= CONTACTS_PLOG_TYPE_NONE
			|| CONTACTS_PLOG_TYPE_MAX <= phonelog->log_type,
			CONTACTS_ERROR_INVALID_PARAMETER, "phonelog type(%d) is invaid", phonelog->log_type);

	snprintf(query, sizeof(query), "INSERT INTO "CTS_TABLE_PHONELOGS"("
			"number, normal_num, person_id, log_type, log_time, data1, data2) "
			"VALUES(?, ?, ?, %d, %d, %d, ?)",
			phonelog->log_type, phonelog->log_time, phonelog->extra_data1);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	if (phonelog->address) {
		cts_stmt_bind_text(stmt, 1, phonelog->address);
		if (phonelog->log_type < CONTACTS_PLOG_TYPE_EMAIL_RECEIVED) {
			ret = ctsvc_clean_number(phonelog->address, clean_num, sizeof(clean_num));
			if (0 < ret) {
				ret = ctsvc_normalize_number(clean_num, normal_num, CTSVC_NUMBER_MAX_LEN, CTSVC_MIN_MATCH_NORMALIZED_NUMBER_SIZE);
				cts_stmt_bind_text(stmt, 2, normal_num);
			}
		}
	}

	if (0 < phonelog->person_id)
		cts_stmt_bind_int(stmt, 3, phonelog->person_id);

	if (phonelog->extra_data2)
		cts_stmt_bind_text(stmt, 4, phonelog->extra_data2);

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret)
	{
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}
	if (id)
		*id = cts_db_get_last_insert_id();
	cts_stmt_finalize(stmt);

	ctsvc_set_phonelog_noti();
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_phonelog_insert_record( contacts_record_h record, int *id )
{
	int ret;
	ctsvc_phonelog_s *phonelog = (ctsvc_phonelog_s *)record;

	RETVM_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER, "Invaild parameter : record is null");
	RETVM_IF(phonelog->id, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : The phone_log has ID(%d)", phonelog->id);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	ret = __ctsvc_db_phonelog_insert(phonelog, id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_phonelog_insert() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (0 < phonelog->person_id) {
		ret = __ctsvc_cb_phonelog_increase_outgoing_count(phonelog->person_id);
		WARN_IF(CONTACTS_ERROR_NONE != ret, "cts_increase_outgoing_count() Failed(%d)", ret);
	}

#ifdef _CONTACTS_IPC_SERVER
	// add id for subscribe
	ctsvc_change_subject_add_changed_phone_log_id(CONTACTS_CHANGE_INSERTED, *id);
#endif
	ret = ctsvc_end_trans(true);

	// set missed call Badge number to Apptray
	if(phonelog-> log_type == CONTACTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN || phonelog-> log_type == CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN) {

		#define PHONE_PACKAGE_NAME		"org.tizen.phone"
		unsigned int call_cnt = 0;
		bool	bBadgeExist = FALSE;

		badge_is_existing(PHONE_PACKAGE_NAME, &bBadgeExist);
		if(bBadgeExist == FALSE)
			badge_create(PHONE_PACKAGE_NAME, PHONE_PACKAGE_NAME);

		badge_get_count(PHONE_PACKAGE_NAME, &call_cnt);
		call_cnt++;
		badge_set_count(PHONE_PACKAGE_NAME, call_cnt);
	}


	if (ret < CONTACTS_ERROR_NONE)
	{
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}


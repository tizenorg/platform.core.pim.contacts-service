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
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_query.h"
#include "ctsvc_normalize.h"
#include "ctsvc_number_utils.h"
#include "ctsvc_db_plugin_number_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"
#include "ctsvc_setting.h"

static int __ctsvc_db_number_reset_default(int number_id, int contact_id)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_DATA" SET is_default = 0, is_primary_default = 0 WHERE id != %d AND contact_id = %d AND datatype = %d",
			number_id, contact_id, CTSVC_DATA_NUMBER);
	ret = ctsvc_query_exec(query);

	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_query_exec() Failed(%d)", ret);
	return ret;
}

int ctsvc_db_number_insert(contacts_record_h record, int contact_id, bool is_my_profile, int *id)
{
	int ret;
	int number_id;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_number_s *number = (ctsvc_number_s *)record;
	char minmatch[CTSVC_NUMBER_MAX_LEN] = {0};
	char normal_num[CTSVC_NUMBER_MAX_LEN] = {0};

	RETV_IF(NULL == number->number, CONTACTS_ERROR_NONE);
	RETVM_IF(contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : contact_id(%d) is mandatory field to insert number record ", number->contact_id);
	RETVM_IF(0 < number->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : id(%d), This record is already inserted", number->id);

	snprintf(query, sizeof(query),
		"INSERT INTO "CTS_TABLE_DATA"(contact_id, is_my_profile, datatype, is_default, data1, data2, data3, data4, data5) "
									"VALUES(%d, %d, %d, %d, %d, ?, ?, ?, ?)",
			contact_id, is_my_profile, CTSVC_DATA_NUMBER, number->is_default, number->type);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Failed(%d)", ret);

	if (number->label)
		ctsvc_stmt_bind_text(stmt, 1, number->label);

	ctsvc_stmt_bind_text(stmt, 2, number->number);
	ret = ctsvc_normalize_number(number->number, normal_num, sizeof(normal_num));
	if (0 < ret) {
		ctsvc_stmt_bind_text(stmt, 4, normal_num);
		ret = ctsvc_get_minmatch_number(normal_num, minmatch, CTSVC_NUMBER_MAX_LEN, ctsvc_get_phonenumber_min_match_digit());
		if (CONTACTS_ERROR_NONE == ret)
			ctsvc_stmt_bind_text(stmt, 3, minmatch);
	}

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_stmt_step() Failed(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		return ret;
	}

	number_id = ctsvc_db_get_last_insert_id();
	if (id)
		*id = number_id;
	ctsvc_stmt_finalize(stmt);

	if (ctsvc_record_check_property_flag((ctsvc_record_s *)record, _contacts_number.is_default, CTSVC_PROPERTY_FLAG_DIRTY)) {
		if (number->is_default)
			__ctsvc_db_number_reset_default(number_id, contact_id);
	}

	if (!is_my_profile)
		ctsvc_set_number_noti();

	return CONTACTS_ERROR_NONE;
}


int ctsvc_db_number_get_value_from_stmt(cts_stmt stmt, contacts_record_h *record, int start_count)
{
	int ret;
	char *temp;
	ctsvc_number_s *number;

	ret = contacts_record_create(_contacts_number._uri, (contacts_record_h *)&number);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create is failed(%d)", ret);

	number->id = ctsvc_stmt_get_int(stmt, start_count++);
	number->contact_id = ctsvc_stmt_get_int(stmt, start_count++);
	number->is_default = ctsvc_stmt_get_int(stmt, start_count++);
	number->type = ctsvc_stmt_get_int(stmt, start_count++);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	number->label = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	number->number = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	number->lookup = SAFE_STRDUP(temp);

	*record = (contacts_record_h)number;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_number_update(contacts_record_h record, bool is_my_profile)
{
	int id;
	int ret = CONTACTS_ERROR_NONE;
	char* set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	ctsvc_number_s *number = (ctsvc_number_s *)record;
	char normal_num[CTSVC_NUMBER_MAX_LEN] = {0};
	char minmatch[CTSVC_NUMBER_MAX_LEN] = {0};
	char query[CTS_SQL_MAX_LEN] = {0};

	RETVM_IF(!number->id, CONTACTS_ERROR_INVALID_PARAMETER, "number of contact has no ID.");
	RETVM_IF(CTSVC_PROPERTY_FLAG_DIRTY != (number->base.property_flag & CTSVC_PROPERTY_FLAG_DIRTY),
				CONTACTS_ERROR_NONE, "No update");

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_DATA" WHERE id = %d", number->id);
	ret = ctsvc_query_get_first_int_result(query, &id);
	RETV_IF(ret != CONTACTS_ERROR_NONE, ret);

	if (ctsvc_record_check_property_flag((ctsvc_record_s *)record, _contacts_number.is_default, CTSVC_PROPERTY_FLAG_DIRTY)) {
		if (number->is_default)
			__ctsvc_db_number_reset_default(number->id, number->contact_id);
	}

	do {
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_create_set_query(record, &set, &bind_text))) break;
		if (ctsvc_record_check_property_flag((ctsvc_record_s *)record, CTSVC_PROPERTY_NUMBER_NUMBER, CTSVC_PROPERTY_FLAG_DIRTY)) {
			ret = ctsvc_normalize_number(number->number, normal_num, sizeof(normal_num));
			if (0 < ret) {
				char query_set[CTS_SQL_MAX_LEN] = {0};
				ret = ctsvc_get_minmatch_number(normal_num, minmatch, CTSVC_NUMBER_MAX_LEN, ctsvc_get_phonenumber_min_match_digit());
				if (CONTACTS_ERROR_NONE == ret) {
					snprintf(query_set, sizeof(query_set), "%s, data4=?, data5=?", set);
					free(set);
					set = strdup(query_set);
					bind_text = g_slist_append(bind_text, strdup(minmatch));
					bind_text = g_slist_append(bind_text, strdup(normal_num));
				}
				else {
					snprintf(query_set, sizeof(query_set), "%s, data5=?", set);
					free(set);
					set = strdup(query_set);
					bind_text = g_slist_append(bind_text, strdup(normal_num));
				}
			}
		}
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_update_record_with_set_query(set, bind_text, CTS_TABLE_DATA, number->id))) break;

		if (!is_my_profile)
			ctsvc_set_number_noti();
	} while (0);

	CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s *)record);
	CONTACTS_FREE(set);
	if (bind_text) {
		for (cursor=bind_text;cursor;cursor=cursor->next)
			CONTACTS_FREE(cursor->data);
		g_slist_free(bind_text);
	}

	return ret;
}

int ctsvc_db_number_delete(int id, bool is_my_profile)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_DATA" WHERE id = %d AND datatype = %d",
			id, CTSVC_DATA_NUMBER);

	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Failed(%d)", ret);

	if (!is_my_profile)
		ctsvc_set_number_noti();

	return CONTACTS_ERROR_NONE;
}


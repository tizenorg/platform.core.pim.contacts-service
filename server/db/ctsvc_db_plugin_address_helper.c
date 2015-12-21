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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_db_schema.h"
#include "ctsvc_db_sqlite.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_query.h"
#include "ctsvc_db_plugin_address_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"

int ctsvc_db_address_get_value_from_stmt(cts_stmt stmt, contacts_record_h *record, int start_count)
{
	int ret;
	char *temp;
	ctsvc_address_s *address;

	ret = contacts_record_create(_contacts_address._uri, (contacts_record_h*)&address);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create Fail(%d)", ret);

	address->id = ctsvc_stmt_get_int(stmt, start_count++);
	address->contact_id = ctsvc_stmt_get_int(stmt, start_count++);
	address->is_default = ctsvc_stmt_get_int(stmt, start_count++);
	address->type = ctsvc_stmt_get_int(stmt, start_count++);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	address->label = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	address->pobox = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	address->postalcode = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	address->region = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	address->locality = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	address->street = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	address->extended = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	address->country = SAFE_STRDUP(temp);

	*record = (contacts_record_h)address;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_address_reset_default(int address_id, int contact_id)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_DATA" SET is_default = 0, is_primary_default = 0 "
			"WHERE id != %d AND contact_id = %d AND datatype = %d",
			address_id, contact_id, CTSVC_DATA_POSTAL);
	ret = ctsvc_query_exec(query);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_query_exec() Fail(%d)", ret);
	return ret;
}

int ctsvc_db_address_insert(contacts_record_h record, int contact_id, bool is_my_profile, int *id)
{
	int ret;
	int address_id;
	cts_stmt stmt = NULL;
	ctsvc_address_s *address = (ctsvc_address_s*)record;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETVM_IF(contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : contact_id(%d) is mandatory field to insert address record ", address->contact_id);
	RETVM_IF(0 < address->id, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : id(%d), This record is already inserted", address->id);

	if (address->pobox || address->postalcode || address->region || address->locality
			|| address->street || address->extended || address->country) {
		snprintf(query, sizeof(query),
				"INSERT INTO "CTS_TABLE_DATA"(contact_id, is_my_profile, datatype, is_default, data1, data2, data3, "
				"data4, data5, data6, data7, data8, data9) "
				"VALUES(%d, %d, %d, %d, %d, ?, ?, ?, ?, ?, ?, ?, ?)",
				contact_id, is_my_profile, CTSVC_DATA_POSTAL, address->is_default, address->type);

		ret = ctsvc_query_prepare(query, &stmt);
		RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

		if (address->label)
			ctsvc_stmt_bind_text(stmt, 1, address->label);
		if (address->pobox)
			ctsvc_stmt_bind_text(stmt, 2, address->pobox);
		if (address->postalcode)
			ctsvc_stmt_bind_text(stmt, 3, address->postalcode);
		if (address->region)
			ctsvc_stmt_bind_text(stmt, 4, address->region);
		if (address->locality)
			ctsvc_stmt_bind_text(stmt, 5, address->locality);
		if (address->street)
			ctsvc_stmt_bind_text(stmt, 6, address->street);
		if (address->extended)
			ctsvc_stmt_bind_text(stmt, 7, address->extended);
		if (address->country)
			ctsvc_stmt_bind_text(stmt, 8, address->country);

		ret = ctsvc_stmt_step(stmt);
		if (CONTACTS_ERROR_NONE != ret) {
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			return ret;
		}
		address_id = ctsvc_db_get_last_insert_id();
		if (id)
			*id = address_id;
		ctsvc_stmt_finalize(stmt);

		if (ctsvc_record_check_property_flag((ctsvc_record_s*)record, _contacts_address.is_default, CTSVC_PROPERTY_FLAG_DIRTY)) {
			if (address->is_default)
				__ctsvc_db_address_reset_default(address_id, contact_id);
		}

		if (false == is_my_profile)
			ctsvc_set_address_noti();
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_address_update(contacts_record_h record, bool is_my_profile)
{
	int id;
	int ret = CONTACTS_ERROR_NONE;
	char *set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	ctsvc_address_s *address = (ctsvc_address_s*)record;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETVM_IF(address->id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : id(%d), This record is already inserted", address->id);
	RETVM_IF(CTSVC_PROPERTY_FLAG_DIRTY != (address->base.property_flag & CTSVC_PROPERTY_FLAG_DIRTY), CONTACTS_ERROR_NONE, "No update");
	RETVM_IF(NULL == address->pobox && NULL == address->postalcode && address->region
			&& address->locality && address->street && address->extended && address->country,
			CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : address is NULL");

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_DATA" WHERE id = %d", address->id);
	ret = ctsvc_query_get_first_int_result(query, &id);
	RETV_IF(ret != CONTACTS_ERROR_NONE, ret);

	if (ctsvc_record_check_property_flag((ctsvc_record_s*)record, _contacts_address.is_default, CTSVC_PROPERTY_FLAG_DIRTY)) {
		if (address->is_default)
			__ctsvc_db_address_reset_default(address->id, address->contact_id);
	}

	do {
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_create_set_query(record, &set, &bind_text))) break;
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_update_record_with_set_query(set, bind_text, CTS_TABLE_DATA, address->id))) break;
		if (false == is_my_profile)
			ctsvc_set_address_noti();
	} while (0);

	CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s*)record);
	free(set);

	if (bind_text) {
		for (cursor = bind_text; cursor; cursor = cursor->next) {
			free(cursor->data);
			cursor->data = NULL;
		}
		g_slist_free(bind_text);
	}

	return ret;
}

int ctsvc_db_address_delete(int id, bool is_my_profile)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_DATA" WHERE datatype = %d AND id = %d",
			CTSVC_DATA_POSTAL, id);

	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	if (false == is_my_profile)
		ctsvc_set_address_noti();

	return ret;
}

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
#include "ctsvc_db_plugin_address_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"

int ctsvc_db_address_get_value_from_stmt(cts_stmt stmt, contacts_record_h *record, int start_count)
{
	int ret;
	char *temp;
	ctsvc_address_s *address;

	ret = contacts_record_create(_contacts_address._uri, (contacts_record_h *)&address);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create is failed(%d)", ret);

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

int ctsvc_db_address_insert(contacts_record_h record, int contact_id, bool is_my_profile, int *id)
{
	int ret;
	cts_stmt stmt = NULL;
	ctsvc_address_s *address = (ctsvc_address_s*)record;
	char query[CTS_SQL_MAX_LEN] = {0};

//	RETVM_IF(address->deleted, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : deleted address record");
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

		stmt = cts_query_prepare(query);
		RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

		if (address->label)
			sqlite3_bind_text(stmt, 1, address->label,
					strlen(address->label), SQLITE_STATIC);
		if (address->pobox)
			sqlite3_bind_text(stmt, 2, address->pobox,
					strlen(address->pobox), SQLITE_STATIC);
		if (address->postalcode)
			sqlite3_bind_text(stmt, 3, address->postalcode,
					strlen(address->postalcode), SQLITE_STATIC);
		if (address->region)
			sqlite3_bind_text(stmt, 4, address->region,
					strlen(address->region), SQLITE_STATIC);
		if (address->locality)
			sqlite3_bind_text(stmt, 5, address->locality,
					strlen(address->locality), SQLITE_STATIC);
		if (address->street)
			sqlite3_bind_text(stmt, 6, address->street,
					strlen(address->street), SQLITE_STATIC);
		if (address->extended)
			sqlite3_bind_text(stmt, 7, address->extended,
					strlen(address->extended), SQLITE_STATIC);
		if (address->country)
			sqlite3_bind_text(stmt, 8, address->country,
					strlen(address->country), SQLITE_STATIC);

		ret = cts_stmt_step(stmt);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
		if (id)
			*id = cts_db_get_last_insert_id();
		cts_stmt_finalize(stmt);

		if (!is_my_profile)
			ctsvc_set_address_noti();
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_address_update(contacts_record_h record, int contact_id, bool is_my_profile)
{
	int ret;
	cts_stmt stmt = NULL;
	ctsvc_address_s *address = (ctsvc_address_s*)record;
	char query[CTS_SQL_MAX_LEN] = {0};

//	RETVM_IF(address->deleted, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : deleted address record");
	RETVM_IF(NULL == address->pobox && NULL == address->postalcode && address->region
			&& address->locality && address->street && address->extended && address->country,
			CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : address is NULL");
	RETVM_IF(contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : contact_id(%d) is mandatory field to insert address record ", address->contact_id);
	RETVM_IF(address->id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : id(%d), This record is already inserted", address->id);

	snprintf(query, sizeof(query),
		"UPDATE "CTS_TABLE_DATA" SET is_my_profile=%d, is_default=?, data1 = ?, data2 = ?, data3 = ?, "
					"data4 = ?, data5 = ?, data6 = ?, data7 = ?, data8 = ?, data9 = ? "
					"WHERE id = %d", is_my_profile, address->id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	sqlite3_bind_int(stmt, 1, address->is_default);
	sqlite3_bind_int(stmt, 2, address->type);

	if (address->label)
		sqlite3_bind_text(stmt, 3, address->label,
				strlen(address->label), SQLITE_STATIC);
	if (address->pobox)
		sqlite3_bind_text(stmt, 4, address->pobox,
				strlen(address->pobox), SQLITE_STATIC);
	if (address->postalcode)
		sqlite3_bind_text(stmt, 5, address->postalcode,
				strlen(address->postalcode), SQLITE_STATIC);
	if (address->region)
		sqlite3_bind_text(stmt, 6, address->region,
				strlen(address->region), SQLITE_STATIC);
	if (address->locality)
		sqlite3_bind_text(stmt, 7, address->locality,
				strlen(address->locality), SQLITE_STATIC);
	if (address->street)
		sqlite3_bind_text(stmt, 8, address->street,
				strlen(address->street), SQLITE_STATIC);
	if (address->extended)
		sqlite3_bind_text(stmt, 9, address->extended,
				strlen(address->extended), SQLITE_STATIC);
	if (address->country)
		sqlite3_bind_text(stmt, 10, address->country,
				strlen(address->country), SQLITE_STATIC);

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}
	cts_stmt_finalize(stmt);

	if (!is_my_profile)
		ctsvc_set_address_noti();

	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_address_delete(int id)
{
	int ret;
	cts_stmt stmt;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_DATA" WHERE datatype = %d AND id = %d",
			CTSVC_DATA_POSTAL, id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}
	cts_stmt_finalize(stmt);

	ctsvc_set_address_noti();

	return ret;
}

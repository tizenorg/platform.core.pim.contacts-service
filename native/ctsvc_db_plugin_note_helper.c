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
#include "ctsvc_utils.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_plugin_note_helper.h"
#include "ctsvc_notification.h"

int ctsvc_db_note_get_value_from_stmt(cts_stmt stmt, contacts_record_h *record, int start_count)
{
	int ret;
	char *temp;
	ctsvc_note_s *note;

	ret = contacts_record_create(_contacts_note._uri, (contacts_record_h *)&note);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create is failed(%d)", ret);

	note->id = ctsvc_stmt_get_int(stmt, start_count++);
	note->contact_id = ctsvc_stmt_get_int(stmt, start_count++);
	start_count++;
	start_count++;
	start_count++;
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	note->note = SAFE_STRDUP(temp);

	*record = (contacts_record_h)note;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_note_insert(contacts_record_h record, int contact_id, bool is_my_profile, int *id)
{
	int ret;
	cts_stmt stmt = NULL;
	ctsvc_note_s *note = (ctsvc_note_s*)record;
	char query[CTS_SQL_MAX_LEN] = {0};

//	RETVM_IF(note->deleted, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : deleted note record");
	RETV_IF(NULL == note->note, CONTACTS_ERROR_NONE);
	RETVM_IF(contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : contact_id(%d) is mandatory field to insert note record ", note->contact_id);
	RETVM_IF(0 < note->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : id(%d), This record is already inserted", note->id);

	snprintf(query, sizeof(query),
		"INSERT INTO "CTS_TABLE_DATA"(contact_id, is_my_profile, datatype, data3) "
					"VALUES(%d, %d, %d, ?)", contact_id, is_my_profile, CTSVC_DATA_NOTE);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	sqlite3_bind_text(stmt, 1, note->note,
			strlen(note->note), SQLITE_STATIC);

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
		ctsvc_set_note_noti();
	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_note_update(contacts_record_h record, int contact_id, bool is_my_profile)
{
	int ret;
	ctsvc_note_s *note = (ctsvc_note_s*)record;
	char query[CTS_SQL_MAX_LEN] = {0};
	cts_stmt stmt;

	RETVM_IF(!note->id, CONTACTS_ERROR_INVALID_PARAMETER, "note of contact has no ID.");
	snprintf(query, sizeof(query),
		"UPDATE "CTS_TABLE_DATA" SET contact_id=%d, is_my_profile=%d, data3=? WHERE id=%d",
				contact_id, is_my_profile, note->id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	if (note->note)
		sqlite3_bind_text(stmt, 1, note->note,
			strlen(note->note), SQLITE_STATIC);

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}

	cts_stmt_finalize(stmt);

	if (!is_my_profile)
		ctsvc_set_note_noti();
	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_note_delete(int id)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_DATA" WHERE id = %d AND datatype = %d",
			id, CTSVC_DATA_NOTE);

	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Failed(%d)", ret);
	ctsvc_set_note_noti();

	return CONTACTS_ERROR_NONE;
}


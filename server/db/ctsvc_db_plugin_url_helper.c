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
#include "ctsvc_db_plugin_contact_helper.h"
#include "ctsvc_db_plugin_url_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"


int ctsvc_db_url_get_value_from_stmt(cts_stmt stmt, contacts_record_h *record, int start_count)
{
	int ret;
	char *temp;
	ctsvc_url_s *url;

	ret = contacts_record_create(_contacts_url._uri, (contacts_record_h*)&url);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create Fail(%d)", ret);

	url->id = ctsvc_stmt_get_int(stmt, start_count++);
	url->contact_id = ctsvc_stmt_get_int(stmt, start_count++);
	start_count++;
	url->type = ctsvc_stmt_get_int(stmt, start_count++);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	url->label = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	url->url = SAFE_STRDUP(temp);

	*record = (contacts_record_h)url;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_url_insert(contacts_record_h record, int contact_id, bool is_my_profile, int *id)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_url_s *url = (ctsvc_url_s*)record;

	RETV_IF(NULL == url->url, CONTACTS_ERROR_NONE);
	RETVM_IF(contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"contact_id(%d) is mandatory field to insert url record ", url->contact_id);
	RETVM_IF(0 < url->id, CONTACTS_ERROR_INVALID_PARAMETER,
			"id(%d), This record is already inserted", url->id);

	snprintf(query, sizeof(query),
			"INSERT INTO "CTS_TABLE_DATA"(contact_id, is_my_profile, datatype, data1, data2, data3) "
			"VALUES(%d, %d, %d, %d, ?, ?)",
			contact_id, is_my_profile, CTSVC_DATA_URL, url->type);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	if (url->label)
		ctsvc_stmt_bind_text(stmt, 1, url->label);
	if (url->url)
		ctsvc_stmt_bind_text(stmt, 2, url->url);

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		return ret;
	}

	/* url->id = ctsvc_db_get_last_insert_id(); */
	if (id)
		*id = ctsvc_db_get_last_insert_id();
	ctsvc_stmt_finalize(stmt);

	if (false == is_my_profile)
		ctsvc_set_url_noti();

	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_url_update(contacts_record_h record, bool is_my_profile)
{
	int id;
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_url_s *url = (ctsvc_url_s*)record;
	char *set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETVM_IF(0 == url->id, CONTACTS_ERROR_INVALID_PARAMETER, "url of contact has no ID.");
	RETVM_IF(CTSVC_PROPERTY_FLAG_DIRTY != (url->base.property_flag & CTSVC_PROPERTY_FLAG_DIRTY), CONTACTS_ERROR_NONE, "No update");

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_DATA" WHERE id = %d", url->id);
	ret = ctsvc_query_get_first_int_result(query, &id);
	RETV_IF(ret != CONTACTS_ERROR_NONE, ret);

	do {
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_create_set_query(record, &set, &bind_text))) break;
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_update_record_with_set_query(set, bind_text, CTS_TABLE_DATA, url->id))) break;
		if (false == is_my_profile)
			ctsvc_set_url_noti();
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

int ctsvc_db_url_delete(int id, bool is_my_profile)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_DATA" WHERE id = %d AND datatype = %d",
			id, CTSVC_DATA_URL);

	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	if (false == is_my_profile)
		ctsvc_set_url_noti();

	return CONTACTS_ERROR_NONE;
}


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
#include "ctsvc_db_plugin_activity_photo_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_list.h"
#include "ctsvc_notification.h"


int ctsvc_db_activity_photo_get_value_from_stmt(cts_stmt stmt, contacts_record_h *record)
{
	int ret;
	char *temp;
	ctsvc_activity_photo_s *activity_photo;
	int start_count = 0;

	ret = contacts_record_create(_contacts_activity_photo._uri, (contacts_record_h*)&activity_photo);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create Fail(%d)", ret);

	activity_photo->id = ctsvc_stmt_get_int(stmt, start_count++);
	activity_photo->activity_id = ctsvc_stmt_get_int(stmt, start_count++);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	activity_photo->photo_url = SAFE_STRDUP(temp);
	activity_photo->sort_index = ctsvc_stmt_get_int(stmt, start_count++);

	*record = (contacts_record_h)activity_photo;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_activity_photo_insert(contacts_record_h record, int activity_id, int *id)
{
	int ret;
	cts_stmt stmt = NULL;
	ctsvc_activity_photo_s *activity_photo = (ctsvc_activity_photo_s*)record;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETVM_IF(activity_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"activity_id(%d) Invalid", activity_id);
	RETVM_IF(0 < activity_photo->id, CONTACTS_ERROR_INVALID_PARAMETER,
			"id(%d), This record is already inserted", activity_photo->id);
	RETV_IF(NULL == activity_photo->photo_url, CONTACTS_ERROR_INVALID_PARAMETER);

	snprintf(query, sizeof(query),
			"INSERT INTO "CTS_TABLE_ACTIVITY_PHOTOS"(activity_id, photo_url, sort_index) "
			"VALUES(%d, ?, %d)",
			activity_id, activity_photo->sort_index);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	sqlite3_bind_text(stmt, 1, activity_photo->photo_url, strlen(activity_photo->photo_url), SQLITE_STATIC);

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	if (id)
		*id = ctsvc_db_get_last_insert_id();
	ctsvc_stmt_finalize(stmt);

	ctsvc_set_activity_noti();
	ctsvc_set_activity_photo_noti();

	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_activity_photo_update(contacts_record_h record)
{
	int id;
	int ret = CONTACTS_ERROR_NONE;
	char *set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	ctsvc_activity_photo_s *activity_photo = (ctsvc_activity_photo_s*)record;
	char query[CTS_SQL_MIN_LEN] = {0};

	RETVM_IF(0 == activity_photo->id, CONTACTS_ERROR_INVALID_PARAMETER, "activity_photo has no ID.");
	RETVM_IF(CTSVC_PROPERTY_FLAG_DIRTY != (activity_photo->base.property_flag & CTSVC_PROPERTY_FLAG_DIRTY), CONTACTS_ERROR_NONE, "No update");

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_ACTIVITY_PHOTOS" WHERE id = %d", activity_photo->id);

	ret = ctsvc_query_get_first_int_result(query, &id);
	RETV_IF(ret != CONTACTS_ERROR_NONE, ret);

	do {
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_create_set_query(record, &set, &bind_text))) break;
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_update_record_with_set_query(set, bind_text, CTS_TABLE_ACTIVITY_PHOTOS, activity_photo->id))) break;
		ctsvc_set_activity_noti();
		ctsvc_set_activity_photo_noti();
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

int ctsvc_db_activity_photo_delete(int id)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_ACTIVITY_PHOTOS" WHERE id = %d", id);

	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	ctsvc_set_activity_noti();
	ctsvc_set_activity_photo_noti();

	return CONTACTS_ERROR_NONE;
}


int ctsvc_db_activity_photo_get_records(int activity_id, contacts_record_h record_activity)
{
	char query[CTS_SQL_MAX_LEN] = {0};
	int ret;
	cts_stmt stmt = NULL;
	contacts_list_h list;

	snprintf(query, sizeof(query), "SELECT activity_id, activity_id, photo_url, sort_index "
			"FROM "CTSVC_DB_VIEW_ACTIVITY_PHOTOS" WHERE activity_id = %d "
			"ORDER BY sort_index ASC", activity_id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		if (1 != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
			/* LCOV_EXCL_STOP */
		}
		contacts_record_h record = NULL;
		ctsvc_db_activity_photo_get_value_from_stmt(stmt, &record);
		ctsvc_list_prepend(list, record);
	}

	ctsvc_stmt_finalize(stmt);

	((ctsvc_activity_s*)record_activity)->photos = (ctsvc_list_s*)list;

	return CONTACTS_ERROR_NONE;
}





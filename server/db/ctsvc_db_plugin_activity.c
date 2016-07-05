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
#include "ctsvc_list.h"
#include "ctsvc_db_plugin_activity_photo_helper.h"
#include "ctsvc_db_plugin_contact_helper.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_utils.h"
#include "ctsvc_record.h"
#include "ctsvc_db_query.h"
#include "ctsvc_notification.h"
#include "ctsvc_db_access_control.h"


static int __ctsvc_db_activity_insert_record(contacts_record_h record, int *id)
{
	int ret;
	int activity_id;
	int addressbook_id;
	cts_stmt stmt = NULL;
	int count = 0;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_activity_s *activity = (ctsvc_activity_s*)record;

	RETV_IF(NULL == activity, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(activity->id, CONTACTS_ERROR_INVALID_PARAMETER,
			"The activity has ID(%d)", activity->id);

	RETVM_IF(activity->contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"The contact_id(%d) does not exist", activity->contact_id);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "contacts_svc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT addressbook_id from %s WHERE contact_id = %d",
			CTSVC_DB_VIEW_CONTACT, activity->contact_id);

	ret = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		ctsvc_end_trans(false);
		if (CONTACTS_ERROR_NO_DATA == ret) {
			//LCOV_EXCL_START
			ERR("No data : contact id (%d)", activity->contact_id);
			return CONTACTS_ERROR_INVALID_PARAMETER;
			//LCOV_EXCL_STOP
		} else {
			//LCOV_EXCL_START
			ERR("ctsvc_query_get_first_int_result() Fail(%d)", ret);
			return ret;
			//LCOV_EXCL_STOP
		}
	}

	ret = ctsvc_is_owner(addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		if (CONTACTS_ERROR_PERMISSION_DENIED == ret)
			//LCOV_EXCL_START
			ERR("Does not have permission of address_book (%d)", addressbook_id);
		else
			ERR("ctsvc_is_owner() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		//LCOV_EXCL_STOP
	}

	snprintf(query, sizeof(query), "INSERT INTO "CTS_TABLE_ACTIVITIES"("
			"contact_id, source_name, status, timestamp, "
			"service_operation, uri) "
			"VALUES(%d, ?, ?, %d, ?, ?)",
			activity->contact_id, activity->timestamp);

	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		//LCOV_EXCL_START
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		//LCOV_EXCL_STOP
	}

	if (activity->source_name)
		ctsvc_stmt_bind_text(stmt, 1, activity->source_name);
	if (activity->status)
		ctsvc_stmt_bind_text(stmt, 2, activity->status);
	if (activity->service_operation)
		ctsvc_stmt_bind_text(stmt, 3, activity->service_operation);
	if (activity->uri)
		ctsvc_stmt_bind_text(stmt, 4, activity->uri);

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
		//LCOV_EXCL_STOP
	}

	activity_id = ctsvc_db_get_last_insert_id();

	ctsvc_stmt_finalize(stmt);

	if (activity->photos) {
		ret = contacts_list_get_count((contacts_list_h)activity->photos, &count);
		if (CONTACTS_ERROR_NONE == ret && 0 < count) {
			ctsvc_activity_photo_s *photo = NULL;
			contacts_record_h record = NULL;

			contacts_list_first((contacts_list_h)activity->photos);
			do {
				contacts_list_get_current_record_p((contacts_list_h)activity->photos, &record);
				photo = (ctsvc_activity_photo_s*)record;
				ret = ctsvc_db_activity_photo_insert((contacts_record_h)photo, activity_id, NULL);
				if (CONTACTS_ERROR_DB == ret) {
					//LCOV_EXCL_START
					ERR("return (%d)", ret);
					break;
					//LCOV_EXCL_STOP
				}
			} while (CONTACTS_ERROR_NONE == contacts_list_next((contacts_list_h)activity->photos));
		}
	}

	ctsvc_set_activity_noti();
	ctsvc_set_person_noti();

	if (id)
		*id = activity_id;

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
		return ret;
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_activity_value_set(cts_stmt stmt, contacts_record_h *record)
{
	int i = 0;
	char *temp;
	ctsvc_activity_s *activity;

	contacts_record_create(_contacts_activity._uri, record);
	activity = (ctsvc_activity_s*)*record;

	activity->id = ctsvc_stmt_get_int(stmt, i++);
	activity->contact_id = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	activity->source_name = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	activity->status = SAFE_STRDUP(temp);
	activity->timestamp = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	activity->service_operation = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	activity->uri = SAFE_STRDUP(temp);

	return CONTACTS_ERROR_NONE;
}


static int __ctsvc_db_activity_get_record(int id, contacts_record_h *out_record)
{
	char query[CTS_SQL_MAX_LEN] = {0};
	int ret;
	cts_stmt stmt = NULL;
	contacts_record_h record;

	snprintf(query, sizeof(query), "SELECT id, contact_id, source_name, status, "
			"timestamp, service_operation, uri "
			"FROM "CTSVC_DB_VIEW_ACTIVITY" WHERE id = %d", id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		//LCOV_EXCL_START
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
		//LCOV_EXCL_STOP
	}

	__ctsvc_db_activity_value_set(stmt, &record);
	ctsvc_stmt_finalize(stmt);

	ctsvc_db_activity_photo_get_records(id, record);

	*out_record = (contacts_record_h)record;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_activity_update_record(contacts_record_h record)
{
	//LCOV_EXCL_START
	ERR("Invalid operation : activity can not update, only insert/delete");
	return CONTACTS_ERROR_INVALID_PARAMETER;
	//LCOV_EXCL_STOP
}

static int __ctsvc_db_activity_delete_record(int id)
{
	int ret;
	int addressbook_id;
	char query[CTS_SQL_MAX_LEN] = {0};

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT addressbook_id FROM "CTSVC_DB_VIEW_CONTACT" "
			"WHERE contact_id = (SELECT contact_id FROM "CTSVC_DB_VIEW_ACTIVITY" WHERE id = %d)", id);
	ret = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("No data : id (%d)", id);
		ctsvc_end_trans(false);
		return ret;
		//LCOV_EXCL_STOP
	}

	ret = ctsvc_is_owner(addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		if (CONTACTS_ERROR_PERMISSION_DENIED == ret)
			//LCOV_EXCL_START
			ERR("Does not have permission of address_book (%d)", addressbook_id);
		else
			ERR("ctsvc_is_owner Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		//LCOV_EXCL_STOP
	}

	snprintf(query, sizeof(query),
			"DELETE FROM "CTS_TABLE_ACTIVITIES" WHERE id = %d", id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		//LCOV_EXCL_STOP
	}

	ctsvc_set_activity_noti();
	ctsvc_set_person_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
		return ret;
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_activity_get_all_records(int offset, int limit,
		contacts_list_h *out_list)
{
	int ret;
	int len;
	int activity_id;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;

	len = snprintf(query, sizeof(query),
			"SELECT id FROM "CTSVC_DB_VIEW_ACTIVITY);

	if (0 != limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			//LCOV_EXCL_START
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
			//LCOV_EXCL_STOP
		}
		activity_id = ctsvc_stmt_get_int(stmt, 0);
		ret = ctsvc_db_get_record(_contacts_activity._uri, activity_id, &record);
		if (CONTACTS_ERROR_NONE != ret) {
			//LCOV_EXCL_START
			ERR("contacts_db_get_record() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
			//LCOV_EXCL_STOP
		}
		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_activity_get_records_with_query(contacts_query_h query, int offset, int limit, contacts_list_h *out_list)
{
	int ret;
	int i;
	int field_count;
	int activity_id = 0;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_activity_s *activity;
	bool had_activity_id = false;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s*)query;

	if (s_query->projection) {
		for (i = 0; i < s_query->projection_count; i++) {
			if (s_query->projection[i] == CTSVC_PROPERTY_ACTIVITY_ID) {
				had_activity_id = true;
				break;
			}
		}
	} else {
		s_query->projection_count = 0;
		had_activity_id = true;
	}

	if (false == had_activity_id) {
		s_query->projection = realloc(s_query->projection, s_query->projection_count+1);
		if (NULL == s_query->projection) {
			//LCOV_EXCL_START
			ERR("realloc() Fail");
			return CONTACTS_ERROR_OUT_OF_MEMORY;
			//LCOV_EXCL_STOP
		}
		s_query->projection[s_query->projection_count] = CTSVC_PROPERTY_ACTIVITY_ID;
		s_query->projection_count++;
	}

	ret = ctsvc_db_make_get_records_query_stmt(s_query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_db_make_get_records_query_stmt fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 /*CTS_TRUE */ != ret) {
			//LCOV_EXCL_START
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
			//LCOV_EXCL_STOP
		}

		contacts_record_create(_contacts_activity._uri, &record);
		activity = (ctsvc_activity_s*)record;
		if (0 == s_query->projection_count) {
			field_count = s_query->property_count;
		} else {
			field_count = s_query->projection_count;
			ret = ctsvc_record_set_projection_flags(record, s_query->projection,
					s_query->projection_count, s_query->property_count);

			if (CONTACTS_ERROR_NONE != ret)
				ASSERT_NOT_REACHED("To set projection is Fail\n");
		}

		for (i = 0; i < field_count; i++) {
			char *temp;
			int property_id;
			if (0 == s_query->projection_count)
				property_id = s_query->properties[i].property_id;
			else
				property_id = s_query->projection[i];


			switch (property_id) {
			case CTSVC_PROPERTY_ACTIVITY_ID:
				activity_id = ctsvc_stmt_get_int(stmt, i);
				if (had_activity_id)
					activity->id = activity_id;
				break;
			case CTSVC_PROPERTY_ACTIVITY_CONTACT_ID:
				activity->contact_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_ACTIVITY_SOURCE_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(activity->source_name);
				activity->source_name = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_ACTIVITY_STATUS:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(activity->status);
				activity->status = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_ACTIVITY_TIMESTAMP:
				activity->timestamp = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_ACTIVITY_SERVICE_OPERATION:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(activity->service_operation);
				activity->service_operation = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_ACTIVITY_URI:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(activity->uri);
				activity->uri = SAFE_STRDUP(temp);
				break;
			default:
				break;
			}
		}
		ctsvc_db_activity_photo_get_records(activity_id, record);

		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}

ctsvc_db_plugin_info_s ctsvc_db_plugin_activity = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_activity_insert_record,
	.get_record = __ctsvc_db_activity_get_record,
	.update_record = __ctsvc_db_activity_update_record,
	.delete_record = __ctsvc_db_activity_delete_record,
	.get_all_records = __ctsvc_db_activity_get_all_records,
	.get_records_with_query = __ctsvc_db_activity_get_records_with_query,
	.insert_records = NULL,
	.update_records = NULL,
	.delete_records = NULL,
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};


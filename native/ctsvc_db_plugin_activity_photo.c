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
#include "ctsvc_db_access_control.h"
#include "ctsvc_db_plugin_activity_photo_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"
#include "ctsvc_db_query.h"
#include "ctsvc_list.h"

static int __ctsvc_db_activity_photo_insert_record(contacts_record_h record, int *id);
static int __ctsvc_db_activity_photo_get_record(int id, contacts_record_h* out_record);
static int __ctsvc_db_activity_photo_update_record(contacts_record_h record);
static int __ctsvc_db_activity_photo_delete_record(int id);
static int __ctsvc_db_activity_photo_get_all_records(int offset, int limit, contacts_list_h* out_list);
static int __ctsvc_db_activity_photo_get_records_with_query(contacts_query_h query, int offset, int limit, contacts_list_h* out_list);

ctsvc_db_plugin_info_s ctsvc_db_plugin_activity_photo = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_activity_photo_insert_record,
	.get_record = __ctsvc_db_activity_photo_get_record,
	.update_record = __ctsvc_db_activity_photo_update_record,
	.delete_record = __ctsvc_db_activity_photo_delete_record,
	.get_all_records = __ctsvc_db_activity_photo_get_all_records,
	.get_records_with_query = __ctsvc_db_activity_photo_get_records_with_query,
	.insert_records = NULL,
	.update_records = NULL,
	.delete_records = NULL,
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};

static int __ctsvc_db_activity_photo_insert_record(contacts_record_h record, int *id)
{
	int ret;
	int addressbook_id;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_activity_photo_s *activity_photo = (ctsvc_activity_photo_s *)record;

	RETVM_IF(NULL == activity_photo->photo_url, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : photo_url is NULL");

	ret = ctsvc_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT addressbook_id from "CTSVC_DB_VIEW_CONTACT" "
			"WHERE contact_id = (SELECT contact_id from "CTS_TABLE_ACTIVITIES" WHERE id = %d)", activity_photo->activity_id);
	ret = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		ctsvc_end_trans(false);
		if (CONTACTS_ERROR_NO_DATA == ret) {
			CTS_ERR("No data : activity_id (%d) is not exist", activity_photo->activity_id);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		else {
			CTS_ERR("ctsvc_query_get_first_int_result Fail(%d)", ret);
			return ret;
		}
	}

	if (false == ctsvc_have_ab_write_permission(addressbook_id)) {
		CTS_ERR("Does not have permission to insert this activity_photo record : addresbook_id(%d)", addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	ret = ctsvc_db_activity_photo_insert(record, activity_photo->activity_id, id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_db_activity_photo_insert() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "DB error : ctsvc_end_trans() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_activity_photo_get_record(int id, contacts_record_h* out_record)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	snprintf(query, sizeof(query),
			"SELECT id, activity_id, photo_url, sort_index "
			"FROM "CTSVC_DB_VIEW_ACTIVITY_PHOTOS" "
			"WHERE id = %d",
			id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Failed(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		CTS_ERR("ctsvc_stmt_step() Failed(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}

	ctsvc_db_activity_photo_get_value_from_stmt(stmt, out_record);

	ctsvc_stmt_finalize(stmt);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_activity_photo_update_record(contacts_record_h record)
{
	int ret;
	int addressbook_id;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_activity_photo_s *activity_photo = (ctsvc_activity_photo_s *)record;

	RETVM_IF(NULL == activity_photo->photo_url, CONTACTS_ERROR_INVALID_PARAMETER, "photo_url is empty");

	ret = ctsvc_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT addressbook_id from "CTSVC_DB_VIEW_CONTACT" "
			"WHERE contact_id = (SELECT contact_id from "CTS_TABLE_ACTIVITIES" WHERE id = %d)",
			activity_photo->activity_id);

	ret = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("No data : activity_id (%d) is not exist", activity_photo->activity_id);
		ctsvc_end_trans(false);
		return ret;
	}

	if (false == ctsvc_have_ab_write_permission(addressbook_id)) {
		CTS_ERR("Does not have permission to update this activity_photo record : addresbook_id(%d)", addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	ret = ctsvc_db_activity_photo_update(record);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("Update record Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "DB error : ctsvc_end_trans() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_activity_photo_delete_record(int id)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};
	int addressbook_id;

	ret = ctsvc_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT C.addressbook_id FROM "CTSVC_DB_VIEW_CONTACT" C, "CTSVC_DB_VIEW_ACTIVITY" A, "CTSVC_DB_VIEW_ACTIVITY_PHOTOS" P "
			"ON C.contact_id = A.contact_id AND A.id = P.activity_id "
			"WHERE P.id = %d", id);

	ret = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("No data : id (%d) is not exist", id);
		ctsvc_end_trans(false);
		return ret;
	}

	if (false == ctsvc_have_ab_write_permission(addressbook_id)) {
		CTS_ERR("Does not have permission to delete this activity_photo record : addresbook_id(%d)", addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	ret = ctsvc_db_activity_photo_delete(id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_db_activity_photo_delete() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "DB error : ctsvc_end_trans() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_activity_photo_get_all_records(int offset, int limit, contacts_list_h* out_list)
{
	int len;
	int ret;
	contacts_list_h list;
	ctsvc_activity_photo_s *activity_photo;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	len = snprintf(query, sizeof(query),
			"SELECT P.id, P.activity_id, P.photo_url, P.sort_index "
				"FROM "CTSVC_DB_VIEW_CONTACT" C, "CTSVC_DB_VIEW_ACTIVITY" A, "CTSVC_DB_VIEW_ACTIVITY_PHOTOS" P "
				"ON C.contact_id = A.contact_id AND A.id = P.activity_id ");

	if (0 != limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Failed(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		if (1 /*CTS_TRUE */ != ret) {
			CTS_ERR("DB : ctsvc_stmt_step fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}
		ctsvc_db_activity_photo_get_value_from_stmt(stmt, (contacts_record_h*)&activity_photo);
		ctsvc_list_prepend(list, (contacts_record_h)activity_photo);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_activity_photo_get_records_with_query(contacts_query_h query, int offset,
		int limit, contacts_list_h* out_list)
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_activity_photo_s *activity_photo;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s *)query;

	ret = ctsvc_db_make_get_records_query_stmt(s_query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_db_make_get_records_query_stmt fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 /*CTS_TRUE */ != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Failed(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		contacts_record_create(_contacts_activity_photo._uri, &record);
		activity_photo = (ctsvc_activity_photo_s*)record;
		if (0 == s_query->projection_count)
			field_count = s_query->property_count;
		else {
			field_count = s_query->projection_count;
			ret = ctsvc_record_set_projection_flags(record, s_query->projection,
					s_query->projection_count, s_query->property_count);

			if (CONTACTS_ERROR_NONE != ret)
				ASSERT_NOT_REACHED("To set projection is failed.\n");
		}

		for (i=0;i<field_count;i++) {
			char *temp;
			int property_id;
			if (0 == s_query->projection_count)
				property_id = s_query->properties[i].property_id;
			else
				property_id = s_query->projection[i];

			switch(property_id) {
			case CTSVC_PROPERTY_ACTIVITY_PHOTO_ID:
				activity_photo->id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_ACTIVITY_PHOTO_ACTIVITY_ID:
				activity_photo->activity_id= ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_ACTIVITY_PHOTO_URL:
				temp = ctsvc_stmt_get_text(stmt, i);
				activity_photo->photo_url = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_ACTIVITY_PHOTO_SORT_INDEX:
				activity_photo->sort_index= ctsvc_stmt_get_int(stmt, i);
				break;
			default:
				break;
			}
		}
		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}


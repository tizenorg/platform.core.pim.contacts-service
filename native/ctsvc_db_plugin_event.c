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
#include "ctsvc_db_plugin_event_helper.h"
#include "ctsvc_db_plugin_contact_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_db_query.h"
#include "ctsvc_list.h"
#include "ctsvc_notification.h"

static int __ctsvc_db_event_insert_record(contacts_record_h record, int *id);
static int __ctsvc_db_event_get_record(int id, contacts_record_h* out_record);
static int __ctsvc_db_event_update_record(contacts_record_h record);
static int __ctsvc_db_event_delete_record(int id);
static int __ctsvc_db_event_get_all_records(int offset, int limit, contacts_list_h* out_list);
static int __ctsvc_db_event_get_records_with_query(contacts_query_h query, int offset, int limit, contacts_list_h* out_list);
//static int __ctsvc_db_event_insert_records(const contacts_list_h in_list, int **ids);
//static int __ctsvc_db_event_update_records(const contacts_list_h in_list);
//static int __ctsvc_db_event_delete_records(int ids[], int count);

ctsvc_db_plugin_info_s ctsvc_db_plugin_event = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_event_insert_record,
	.get_record = __ctsvc_db_event_get_record,
	.update_record = __ctsvc_db_event_update_record,
	.delete_record = __ctsvc_db_event_delete_record,
	.get_all_records = __ctsvc_db_event_get_all_records,
	.get_records_with_query = __ctsvc_db_event_get_records_with_query,
	.insert_records = NULL,//__ctsvc_db_event_insert_records,
	.update_records = NULL,//__ctsvc_db_event_update_records,
	.delete_records = NULL,//__ctsvc_db_event_delete_records
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};

static int __ctsvc_db_event_insert_record(contacts_record_h record, int *id)
{
	int ret;
	int addressbook_id;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_event_s *event = (ctsvc_event_s *)record;

	RETVM_IF(event->date <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : event date(%d)", event->date);

	ret = ctsvc_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT addressbook_id FROM "CTSVC_DB_VIEW_CONTACT" WHERE contact_id = %d", event->contact_id);
	ret = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		ctsvc_end_trans(false);
		if (CONTACTS_ERROR_NO_DATA == ret) {
			CTS_ERR("No data : contact_id (%d) is not exist", event->contact_id);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		else {
			CTS_ERR("ctsvc_query_get_first_int_result Fail(%d)", ret);
			return ret;
		}
	}

	if (false == ctsvc_have_ab_write_permission(addressbook_id)) {
		CTS_ERR("Does not have permission to update this event record : addresbook_id(%d)", addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	ret = ctsvc_db_event_insert(record, event->contact_id, false, id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_db_contact_update_changed_time(event->contact_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_db_contact_update_changed_time() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}
	ctsvc_set_person_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_event_get_record(int id, contacts_record_h* out_record)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	snprintf(query, sizeof(query),
			"SELECT id, data.contact_id, is_default, data1, data2, data3, data4, data5 "
				"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
				"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
				"WHERE id = %d AND datatype = %d ",
				id, CTSVC_DATA_EVENT);

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

	ctsvc_db_event_get_value_from_stmt(stmt, out_record, 0);

	ctsvc_stmt_finalize(stmt);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_event_update_record(contacts_record_h record)
{
	int ret;
	int addressbook_id;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_event_s *event = (ctsvc_event_s *)record;

	ret = ctsvc_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT addressbook_id FROM "CTSVC_DB_VIEW_CONTACT" WHERE contact_id = %d", event->contact_id);
	ret = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("No data : contact_id (%d) is not exist", event->contact_id);
		ctsvc_end_trans(false);
		return ret;
	}

	if (false == ctsvc_have_ab_write_permission(addressbook_id)) {
		CTS_ERR("Does not have permission to update this event record : addresbook_id(%d)", addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	ret = ctsvc_db_event_update(record, false);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("update record failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	// TODO ; contact display event update
	ret = ctsvc_db_contact_update_changed_time(event->contact_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_db_contact_update_changed_time() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}
	ctsvc_set_person_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_event_delete_record(int id)
{
	int ret;
	int contact_id;
	char query[CTS_SQL_MAX_LEN] = {0};
	cts_stmt stmt = NULL;
	int addressbook_id;

	ret = ctsvc_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT contact_id, addressbook_id FROM "CTSVC_DB_VIEW_CONTACT " "
				"WHERE contact_id = (SELECT contact_id FROM "CTS_TABLE_DATA" WHERE id = %d)", id);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		CTS_ERR("DB error : ctsvc_query_prepare() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}
	ret = ctsvc_stmt_step(stmt);
	if (1 != ret) {
		CTS_ERR("The id(%d) is Invalid(%d)", id, ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}

	contact_id = ctsvc_stmt_get_int(stmt, 0);
	addressbook_id = ctsvc_stmt_get_int(stmt, 1);
	ctsvc_stmt_finalize(stmt);

	if (false == ctsvc_have_ab_write_permission(addressbook_id)) {
		CTS_ERR("Does not have permission to delete this event record : addresbook_id(%d)", addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	ret = ctsvc_db_event_delete(id, false);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_db_contact_update_changed_time(contact_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_db_contact_update_changed_time() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}
	ctsvc_set_person_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_event_get_all_records(int offset, int limit, contacts_list_h* out_list)
{
	int len;
	int ret;
	contacts_list_h list;
	ctsvc_event_s *event;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	len = snprintf(query, sizeof(query),
			"SELECT id, data.contact_id, is_default, data1, data2, data3, data4, data5 "
				"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
				"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
				"WHERE datatype = %d AND is_my_profile=0 ",
				CTSVC_DATA_EVENT);

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
		ctsvc_db_event_get_value_from_stmt(stmt, (contacts_record_h*)&event, 0);
		ctsvc_list_prepend(list, (contacts_record_h)event);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_event_get_records_with_query(contacts_query_h query, int offset,
		int limit, contacts_list_h* out_list)
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_event_s *event;

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

		contacts_record_create(_contacts_event._uri, &record);
		event = (ctsvc_event_s*)record;
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
			case CTSVC_PROPERTY_EVENT_ID:
				event->id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_EVENT_CONTACT_ID:
				event->contact_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_EVENT_TYPE:
				event->type = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_EVENT_LABEL:
				temp = ctsvc_stmt_get_text(stmt, i);
				event->label = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_EVENT_DATE:
				event->date = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_EVENT_CALENDAR_TYPE:
				event->calendar_type = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_EVENT_IS_LEAP_MONTH:
				event->is_leap_month = ctsvc_stmt_get_int(stmt, i);
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

//static int __ctsvc_db_event_insert_records(const contacts_list_h in_list, int **ids) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_event_update_records(const contacts_list_h in_list) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_event_delete_records(int ids[], int count) { return CONTACTS_ERROR_NONE; }

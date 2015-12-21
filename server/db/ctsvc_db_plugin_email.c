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
#include "ctsvc_db_utils.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_db_plugin_contact_helper.h"
#include "ctsvc_db_plugin_email_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"
#include "ctsvc_db_query.h"
#include "ctsvc_list.h"


static int __ctsvc_db_email_get_person_default_email(int person_id)
{
	int ret;
	int default_email_id;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTSVC_DB_VIEW_CONTACT" c, "CTS_TABLE_DATA" d "
			"WHERE c.person_id = %d AND d.datatype = %d AND c.contact_id = d.contact_id AND d.is_default = 1",
			person_id, CTSVC_DATA_EMAIL);
	ret = ctsvc_query_get_first_int_result(query, &default_email_id);
	if (CONTACTS_ERROR_NONE != ret)
		return 0;
	return default_email_id;
}

static int __ctsvc_db_email_update_person_has_email(int person_id, bool has_email)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_PERSONS" SET has_email = %d WHERE person_id = %d",
			has_email, person_id);

	ret = ctsvc_query_exec(query);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_query_exec() Fail(%d)", ret);
	return ret;
}

static int __ctsvc_db_email_update_default(int email_id, int contact_id, bool is_default, bool is_primary_default)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_DATA" SET is_default = %d, is_primary_default = %d WHERE id = %d",
			is_default, is_primary_default, email_id);
	ret = ctsvc_query_exec(query);

	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_query_exec() Fail(%d)", ret);
	return ret;
}

static int __ctsvc_db_email_get_default_email_id(int contact_id)
{
	int ret;
	int email_id = 0;
	char query[CTS_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_DATA" WHERE datatype=%d AND contact_id=%d AND is_default=1",
			CTSVC_DATA_EMAIL, contact_id);
	ret = ctsvc_query_get_first_int_result(query, &email_id);
	if (CONTACTS_ERROR_NONE != ret)
		return 0;
	return email_id;
}

static int __ctsvc_db_email_get_primary_default_email_id(int contact_id)
{
	int ret;
	int email_id = 0;
	char query[CTS_SQL_MAX_LEN] = {0};
	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_DATA" WHERE datatype=%d AND contact_id=%d AND is_primary_default=1",
			CTSVC_DATA_EMAIL, contact_id);
	ret = ctsvc_query_get_first_int_result(query, &email_id);
	if (CONTACTS_ERROR_NONE != ret)
		return 0;
	return email_id;
}

static int __ctsvc_db_email_get_primary_default_contact_id(int person_id)
{
	int ret;
	int default_contact_id;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT c.contact_id FROM "CTS_TABLE_CONTACTS" c, "CTS_TABLE_DATA" d "
			"WHERE c.person_id = %d AND d.datatype = %d AND c.contact_id = d.contact_id AND d.is_primary_default = 1",
			person_id, CTSVC_DATA_EMAIL);
	ret = ctsvc_query_get_first_int_result(query, &default_contact_id);
	if (CONTACTS_ERROR_NONE != ret)
		return 0;
	return default_contact_id;
}

static int __ctsvc_db_email_set_primary_default(int email_id, bool is_primary_default)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_DATA" SET is_primary_default = %d WHERE id = %d",
			is_primary_default, email_id);
	ret = ctsvc_query_exec(query);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_query_exec() Fail(%d)", ret);
	return ret;
}

static int __ctsvc_db_email_insert_record(contacts_record_h record, int *id)
{
	int ret;
	int addressbook_id = 0;
	int person_id = 0;
	int old_default_email_id = 0;
	char query[CTS_SQL_MAX_LEN] = {0};
	cts_stmt stmt = NULL;
	ctsvc_email_s *email = (ctsvc_email_s*)record;
	RETV_IF(NULL == email->email_addr, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = ctsvc_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_begin_trans() Fail(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT addressbook_id, person_id FROM "CTSVC_DB_VIEW_CONTACT" WHERE contact_id = %d", email->contact_id);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_stmt_step(stmt);
	if (1 != ret) {
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_INVALID_PARAMETER;
		else
			return ret;
	}
	addressbook_id = ctsvc_stmt_get_int(stmt, 0);
	person_id = ctsvc_stmt_get_int(stmt, 1);
	ctsvc_stmt_finalize(stmt);

	if (false == ctsvc_have_ab_write_permission(addressbook_id)) {
		ERR("Does not have permission to update this email record : addresbook_id(%d)", addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	old_default_email_id = __ctsvc_db_email_get_default_email_id(email->contact_id);
	if (0 == old_default_email_id)
		email->is_default = true;

	ret = ctsvc_db_email_insert(record, email->contact_id, false, id);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_begin_trans() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_CONTACTS" SET has_email = %d, changed_ver = %d, changed_time = %d "
			"WHERE contact_id = %d",
			1, ctsvc_get_next_ver(), (int)time(NULL), email->contact_id);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (email->is_default) {
		int primary_default_contact_id = 0;

		__ctsvc_db_email_update_person_has_email(person_id, true);

		primary_default_contact_id = __ctsvc_db_email_get_primary_default_contact_id(person_id);
		if (0 == primary_default_contact_id || email->contact_id == primary_default_contact_id)
			__ctsvc_db_email_set_primary_default(*id, true);

		ctsvc_contact_update_display_name(email->contact_id, CONTACTS_DISPLAY_NAME_SOURCE_TYPE_EMAIL);
	}

	ctsvc_set_contact_noti();
	ctsvc_set_person_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		ERR("ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	} else {
		return CONTACTS_ERROR_NONE;
	}
}

static int __ctsvc_db_email_get_record(int id, contacts_record_h *out_record)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	snprintf(query, sizeof(query),
			"SELECT id, data.contact_id, is_default, data1, data2, data3 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE id = %d AND datatype = %d ",
			id, CTSVC_DATA_EMAIL);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}

	ctsvc_db_email_get_value_from_stmt(stmt, out_record, 0);

	ctsvc_stmt_finalize(stmt);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_email_update_record(contacts_record_h record)
{
	int ret;
	int addressbook_id;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_email_s *email = (ctsvc_email_s*)record;
	RETVM_IF(NULL == email->email_addr, CONTACTS_ERROR_INVALID_PARAMETER, "email is empty");

	ret = ctsvc_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_begin_trans() Fail(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT addressbook_id FROM "CTSVC_DB_VIEW_CONTACT" WHERE contact_id = %d", email->contact_id);
	ret = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("No data : contact_id (%d) is not exist", email->contact_id);
		ctsvc_end_trans(false);
		return ret;
	}

	if (false == ctsvc_have_ab_write_permission(addressbook_id)) {
		ERR("Does not have permission to update this email record : addresbook_id(%d)", addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	ret = ctsvc_db_email_update(record, false);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("update record Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (email->is_default) {
		int old_primary_default_email_id = 0;
		old_primary_default_email_id = __ctsvc_db_email_get_primary_default_email_id(email->contact_id);
		if (old_primary_default_email_id)
			__ctsvc_db_email_set_primary_default(email->id, true);
	}
	ctsvc_contact_update_display_name(email->contact_id, CONTACTS_DISPLAY_NAME_SOURCE_TYPE_EMAIL);

	ret = ctsvc_db_contact_update_changed_time(email->contact_id);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_db_contact_update_changed_time() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}
	ctsvc_set_person_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		ERR("ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	} else {
		return CONTACTS_ERROR_NONE;
	}
}

static int __ctsvc_db_email_delete_record(int id)
{
	int ret;
	int email_id;
	int contact_id;
	int person_id;
	int is_default;
	int is_primary_default;
	char query[CTS_SQL_MAX_LEN] = {0};
	bool has_email = false;
	cts_stmt stmt = NULL;
	int addressbook_id;

	ret = ctsvc_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_begin_trans() Fail(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT contact_id, person_id, addressbook_id FROM "CTSVC_DB_VIEW_CONTACT " "
			"WHERE contact_id = (SELECT contact_id FROM "CTS_TABLE_DATA" WHERE id = %d)", id);

	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_stmt_step(stmt);
	if (1 != ret) {
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}
	contact_id = ctsvc_stmt_get_int(stmt, 0);
	person_id = ctsvc_stmt_get_int(stmt, 1);
	addressbook_id = ctsvc_stmt_get_int(stmt, 2);
	ctsvc_stmt_finalize(stmt);

	if (false == ctsvc_have_ab_write_permission(addressbook_id)) {
		ERR("Does not have permission to delete this email record : addresbook_id(%d)", addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	snprintf(query, sizeof(query),
			"SELECT is_default, is_primary_default FROM "CTS_TABLE_DATA" WHERE id = %d", id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 != ret) {
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}
	is_default = ctsvc_stmt_get_int(stmt, 0);
	is_primary_default = ctsvc_stmt_get_int(stmt, 1);
	ctsvc_stmt_finalize(stmt);

	ret = ctsvc_db_email_delete(id, false);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_begin_trans() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_DATA" WHERE datatype = %d AND contact_id = %d AND is_my_profile = 0 limit 1",
			CTSVC_DATA_EMAIL, contact_id);
	ret = ctsvc_query_get_first_int_result(query, &email_id);
	if (0 < ret)
		has_email = true;

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_CONTACTS" SET has_email = %d, changed_ver = %d, changed_time = %d "
			"WHERE contact_id = %d",
			has_email, ctsvc_get_next_ver(), (int)time(NULL), contact_id);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (is_default) {

		if (email_id) {
			__ctsvc_db_email_update_default(email_id, contact_id, is_default, is_primary_default);
		} else if (is_primary_default) {
			int default_email_id = 0;
			default_email_id = __ctsvc_db_email_get_person_default_email(person_id);
			if (default_email_id)
				__ctsvc_db_email_set_primary_default(default_email_id, true);
			else
				__ctsvc_db_email_update_person_has_email(person_id, false);
		}
		ctsvc_contact_update_display_name(contact_id, CONTACTS_DISPLAY_NAME_SOURCE_TYPE_EMAIL);
	}

	ctsvc_set_contact_noti();
	ctsvc_set_person_noti();

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_end_trans() Fail(%d)", ret);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_email_get_all_records(int offset, int limit, contacts_list_h *out_list)
{
	int len;
	int ret;
	contacts_list_h list;
	ctsvc_email_s *email;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	len = snprintf(query, sizeof(query),
			"SELECT id, data.contact_id, is_default, data1, data2, data3 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile=0 ",
			CTSVC_DATA_EMAIL);

	if (0 != limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		if (1 /*CTS_TRUE */ != ret) {
			ERR("DB : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}
		ctsvc_db_email_get_value_from_stmt(stmt, (contacts_record_h*)&email, 0);
		ctsvc_list_prepend(list, (contacts_record_h)email);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_email_get_records_with_query(contacts_query_h query, int offset,
		int limit, contacts_list_h *out_list)
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_email_s *email;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s*)query;

	ret = ctsvc_db_make_get_records_query_stmt(s_query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_db_make_get_records_query_stmt fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 /*CTS_TRUE */ != ret) {
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		contacts_record_create(_contacts_email._uri, &record);
		email = (ctsvc_email_s*)record;
		if (0 == s_query->projection_count) {
			field_count = s_query->property_count;
		} else {
			field_count = s_query->projection_count;
			ret = ctsvc_record_set_projection_flags(record, s_query->projection,
					s_query->projection_count, s_query->property_count);

			if (CONTACTS_ERROR_NONE != ret)
				ASSERT_NOT_REACHED("To set projection is Fail.\n");
		}

		for (i = 0; i < field_count; i++) {
			char *temp;
			int property_id;
			if (0 == s_query->projection_count)
				property_id = s_query->properties[i].property_id;
			else
				property_id = s_query->projection[i];

			switch (property_id) {
			case CTSVC_PROPERTY_EMAIL_ID:
				email->id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_EMAIL_CONTACT_ID:
				email->contact_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_EMAIL_TYPE:
				email->type = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_EMAIL_LABEL:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(email->label);
				email->label = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_EMAIL_EMAIL:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(email->email_addr);
				email->email_addr = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_EMAIL_IS_DEFAULT:
				email->is_default = ctsvc_stmt_get_int(stmt, i);
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

ctsvc_db_plugin_info_s ctsvc_db_plugin_email = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_email_insert_record,
	.get_record = __ctsvc_db_email_get_record,
	.update_record = __ctsvc_db_email_update_record,
	.delete_record = __ctsvc_db_email_delete_record,
	.get_all_records = __ctsvc_db_email_get_all_records,
	.get_records_with_query = __ctsvc_db_email_get_records_with_query,
	.insert_records = NULL,
	.update_records = NULL,
	.delete_records = NULL,
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};


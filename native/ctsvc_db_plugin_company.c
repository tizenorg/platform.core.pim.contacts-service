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

#include <unistd.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_utils.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_plugin_contact_helper.h"
#include "ctsvc_db_plugin_company_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"
#include "ctsvc_db_query.h"
#include "ctsvc_list.h"

#define CTS_LOGO_IMAGE_LOCATION "/opt/usr/data/contacts-svc/img/logo"

static int __ctsvc_db_company_insert_record( contacts_record_h record, int *id );
static int __ctsvc_db_company_get_record( int id, contacts_record_h* out_record );
static int __ctsvc_db_company_update_record( contacts_record_h record );
static int __ctsvc_db_company_delete_record( int id );
static int __ctsvc_db_company_get_all_records( int offset, int limit, contacts_list_h* out_list );
static int __ctsvc_db_company_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list );
//static int __ctsvc_db_company_insert_records(const contacts_list_h in_list, int **ds);
//static int __ctsvc_db_company_update_records(const contacts_list_h in_list);
//static int __ctsvc_db_company_delete_records( int ids[], int count);

ctsvc_db_plugin_info_s ctsvc_db_plugin_company = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_company_insert_record,
	.get_record = __ctsvc_db_company_get_record,
	.update_record = __ctsvc_db_company_update_record,
	.delete_record = __ctsvc_db_company_delete_record,
	.get_all_records = __ctsvc_db_company_get_all_records,
	.get_records_with_query = __ctsvc_db_company_get_records_with_query,
	.insert_records = NULL,//__ctsvc_db_company_insert_records,
	.update_records = NULL,//__ctsvc_db_company_update_records,
	.delete_records = NULL,//__ctsvc_db_company_delete_records
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};

static int __ctsvc_db_company_get_record( int id, contacts_record_h* out_record )
{
	int ret;
	int len;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	len = snprintf(query, sizeof(query),
			"SELECT id, data.contact_id, is_default, data1, data2, "
				"data3, data4, data5, data6, data7, data8, data9, data10, data11, data12 "
				"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
				"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
				"WHERE id = %d AND datatype = %d ",
				id, CTSVC_DATA_COMPANY);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CONTACTS_ERROR_NO_DATA;
	}

	ctsvc_db_company_get_value_from_stmt(stmt, out_record, 0);
	cts_stmt_finalize(stmt);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_company_insert_record( contacts_record_h record, int *id )
{
	int ret;
	int contact_id;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_company_s *company = (ctsvc_company_s *)record;

	RETVM_IF(NULL == company->name && NULL == company->department && NULL == company->job_title
		&& NULL == company->role && NULL == company->assistant_name && NULL == company->logo
		&& NULL == company->location && NULL == company->description && NULL == company->phonetic_name,
		CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : company is NULL");

	ret = ctsvc_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT contact_id FROM "CTSVC_DB_VIEW_CONTACT" WHERE contact_id = %d", company->contact_id);
	ret = ctsvc_query_get_first_int_result(query, &contact_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("No data : contact_id (%d) is not exist", contact_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	ret = ctsvc_db_company_insert(record, company->contact_id, false, id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_db_contact_update_changed_time(company->contact_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_contact_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
	{
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_company_update_record( contacts_record_h record )
{
	int ret;
	int contact_id;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_company_s *company = (ctsvc_company_s *)record;

	ret = ctsvc_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT contact_id FROM "CTSVC_DB_VIEW_CONTACT" WHERE contact_id = %d", company->contact_id);
	ret = ctsvc_query_get_first_int_result(query, &contact_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("No data : contact_id (%d) is not exist", contact_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	ret = ctsvc_db_company_update(record, company->contact_id, false);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	// TODO ; contact display company update
	ret = ctsvc_db_contact_update_changed_time(company->contact_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_contact_noti();
	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
	{
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}


static int __ctsvc_db_company_delete_record( int id )
{
	int ret;
	int contact_id;
	char query[CTS_SQL_MAX_LEN] = {0};

	ret = ctsvc_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT contact_id FROM "CTSVC_DB_VIEW_CONTACT" "
			"WHERE contact_id = (SELECT contact_id FROM "CTS_TABLE_DATA" WHERE id = %d)", id);
	ret = ctsvc_query_get_first_int_result(query, &contact_id);
	if( ret != CONTACTS_ERROR_NONE ) {
		CTS_ERR("The id(%d) is Invalid(%d)", id, ret);
		ctsvc_end_trans(false);
		return contact_id;
	}

	ret = ctsvc_db_company_delete(id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_db_contact_update_changed_time(contact_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_contact_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
	{
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_company_get_all_records( int offset, int limit, contacts_list_h* out_list )
{
	int len;
	int ret;
	contacts_list_h list;
	ctsvc_company_s *company;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	len = snprintf(query, sizeof(query),
			"SELECT id, data.contact_id, is_default, data1, data2, "
				"data3, data4, data5, data6, data7, data8, data9, data10, data11, data12 "
				"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
				"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
				"WHERE datatype=%d AND is_my_profile=0 ",
				CTSVC_DATA_COMPANY);
	if (0 < limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		if (1 /*CTS_TRUE */ != ret) {
			CTS_ERR("DB : cts_stmt_step fail(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}
		ctsvc_db_company_get_value_from_stmt(stmt, (contacts_record_h*)&company, 0);
		ctsvc_list_prepend(list, (contacts_record_h)company);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_company_get_records_with_query( contacts_query_h query, int offset,
		int limit, contacts_list_h* out_list )
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_company_s *company;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s *)query;

	ret = ctsvc_db_make_get_records_query_stmt(s_query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_db_make_get_records_query_stmt fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 /*CTS_TRUE */ != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		contacts_record_create(_contacts_company._uri, &record);
		company = (ctsvc_company_s*)record;
		if (0 == s_query->projection_count)
			field_count = s_query->property_count;
		else {
			field_count = s_query->projection_count;
			ret = ctsvc_record_set_projection_flags(record, s_query->projection,
					s_query->projection_count, s_query->property_count);

			if(CONTACTS_ERROR_NONE != ret)
				ASSERT_NOT_REACHED("To set projection is failed.\n");
		}

		for(i=0;i<field_count;i++) {
			char *temp;
			int property_id;
			if (0 == s_query->projection_count)
				property_id = s_query->properties[i].property_id;
			else
				property_id = s_query->projection[i];

			switch(property_id) {
			case CTSVC_PROPERTY_COMPANY_ID:
				company->id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_COMPANY_CONTACT_ID:
				company->contact_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_COMPANY_TYPE:
				company->type = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_COMPANY_LABEL:
				temp = ctsvc_stmt_get_text(stmt, i);
				company->label = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_COMPANY_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				company->name = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_COMPANY_DEPARTMENT:
				temp = ctsvc_stmt_get_text(stmt, i);
				company->department = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_COMPANY_JOB_TITLE:
				temp = ctsvc_stmt_get_text(stmt, i);
				company->job_title = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_COMPANY_ASSISTANT_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				company->assistant_name = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_COMPANY_ROLE:
				temp = ctsvc_stmt_get_text(stmt, i);
				company->role = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_COMPANY_LOGO:
				temp = ctsvc_stmt_get_text(stmt, i);
				company->logo = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_COMPANY_LOCATION:
				temp = ctsvc_stmt_get_text(stmt, i);
				company->location = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_COMPANY_DESCRIPTION:
				temp = ctsvc_stmt_get_text(stmt, i);
				company->description = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_COMPANY_PHONETIC_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				company->phonetic_name = SAFE_STRDUP(temp);
				break;
			default:
				break;
			}
		}
		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}

//static int __ctsvc_db_company_insert_records(const contacts_list_h in_list, int **ids) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_company_update_records(const contacts_list_h in_list) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_company_delete_records( int ids[], int count) { return CONTACTS_ERROR_NONE; }

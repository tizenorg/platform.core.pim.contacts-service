/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjae Shin <yj99.shin@samsung.com>
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
#include <time.h>

#include "cts-internal.h"
#include "cts-schema.h"
#include "cts-sqlite.h"
#include "cts-utils.h"
#include "cts-normalize.h"
#include "cts-restriction.h"
#include "cts-im.h"
#include "cts-contact.h"

#define CTS_MAIN_CTS_GET_UID (1<<0)
#define CTS_MAIN_CTS_GET_RINGTON (1<<1)
#define CTS_MAIN_CTS_GET_NOTE (1<<2)
#define CTS_MAIN_CTS_GET_DEFAULT_NUM (1<<3)
#define CTS_MAIN_CTS_GET_DEFAULT_EMAIL (1<<4)
#define CTS_MAIN_CTS_GET_FAVOR (1<<5)
#define CTS_MAIN_CTS_GET_IMG (1<<6)
#define CTS_MAIN_CTS_GET_ALL (1<<0|1<<1|1<<2|1<<3|1<<4|1<<5|1<<6)

static int cts_get_main_contacts_info(int op_code, int index, contact_t *contact)
{
	int ret, len;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	char *temp;

	len = snprintf(query, sizeof(query), "SELECT ");

	len += snprintf(query+len, sizeof(query)-len,
			"contact_id, person_id, addrbook_id, changed_time");

	if(op_code & CTS_MAIN_CTS_GET_UID)
		len += snprintf(query+len, sizeof(query)-len, ", uid");
	if (op_code & CTS_MAIN_CTS_GET_RINGTON)
		len += snprintf(query+len, sizeof(query)-len, ", ringtone");
	if (op_code & CTS_MAIN_CTS_GET_NOTE)
		len += snprintf(query+len, sizeof(query)-len, ", note");
	if (op_code & CTS_MAIN_CTS_GET_DEFAULT_NUM)
		len += snprintf(query+len, sizeof(query)-len, ", default_num");
	if (op_code & CTS_MAIN_CTS_GET_DEFAULT_EMAIL)
		len += snprintf(query+len, sizeof(query)-len, ", default_email");
	if (op_code & CTS_MAIN_CTS_GET_FAVOR)
		len += snprintf(query+len, sizeof(query)-len, ", is_favorite");
	if (op_code & CTS_MAIN_CTS_GET_IMG) {
		len += snprintf(query+len, sizeof(query)-len, ", image0");
		len += snprintf(query+len, sizeof(query)-len, ", image1");
	}

	snprintf(query+len, sizeof(query)-len,
			" FROM %s WHERE contact_id = %d", CTS_TABLE_CONTACTS, index);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}
	int count=0;

	contact->base = (cts_ct_base *)contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);
	if (NULL == contact->base) {
		cts_stmt_finalize(stmt);
		ERR("contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO) Failed");
		return CTS_ERR_OUT_OF_MEMORY;
	}

	contact->base->id = cts_stmt_get_int(stmt, count++);
	contact->base->person_id = cts_stmt_get_int(stmt, count++);
	contact->base->addrbook_id = cts_stmt_get_int(stmt, count++);
	contact->base->changed_time = cts_stmt_get_int(stmt, count++);

	if (op_code & CTS_MAIN_CTS_GET_UID)
	{
		contact->base->embedded = true;
		temp = cts_stmt_get_text(stmt, count++);
		contact->base->uid = SAFE_STRDUP(temp);
	}
	if (op_code & CTS_MAIN_CTS_GET_RINGTON)
	{
		contact->base->embedded = true;
		temp = cts_stmt_get_text(stmt, count++);
		if (temp && CTS_SUCCESS == cts_exist_file(temp))
			contact->base->ringtone_path = strdup(temp);
		else
			contact->base->ringtone_path = NULL;
	}
	if (op_code & CTS_MAIN_CTS_GET_NOTE)
	{
		contact->base->embedded = true;
		temp = cts_stmt_get_text(stmt, count++);
		contact->base->note = SAFE_STRDUP(temp);
	}
	if (op_code & CTS_MAIN_CTS_GET_DEFAULT_NUM)
		contact->default_num = cts_stmt_get_int(stmt, count++);
	if (op_code & CTS_MAIN_CTS_GET_DEFAULT_EMAIL)
		contact->default_email = cts_stmt_get_int(stmt, count++);
	if (op_code & CTS_MAIN_CTS_GET_FAVOR)
		contact->base->is_favorite = cts_stmt_get_int(stmt, count++);

	if (op_code & CTS_MAIN_CTS_GET_IMG) {
		char tmp_path[CTS_IMG_PATH_SIZE_MAX];
		contact->base->embedded = true;
		temp = cts_stmt_get_text(stmt, count++);
		if (temp) {
			snprintf(tmp_path, sizeof(tmp_path), "%s/%s", CTS_IMAGE_LOCATION, temp);
			contact->base->img_path = strdup(tmp_path);
		}
		temp = cts_stmt_get_text(stmt, count++);
		if (temp) {
			snprintf(tmp_path, sizeof(tmp_path), "%s/%s", CTS_IMAGE_LOCATION, temp);
			contact->base->full_img_path = strdup(tmp_path);
		}
	}

	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}

static inline int cts_get_data_info_number(cts_stmt stmt, contact_t *contact)
{
	cts_number *result;

	result = (cts_number *)contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (result) {
		int cnt = 1;
		result->embedded = true;
		cnt = cts_stmt_get_number(stmt, result, cnt);

		if (result->id == contact->default_num)
			result->is_default = true;

		result->is_favorite = cts_stmt_get_int(stmt, cnt);
		contact->numbers = g_slist_append(contact->numbers, result);
	}
	return CTS_SUCCESS;
}

static inline int cts_get_data_info_email(cts_stmt stmt, contact_t *contact)
{
	cts_email *result;

	result = (cts_email *)contacts_svc_value_new(CTS_VALUE_EMAIL);
	if (result) {
		result->embedded = true;
		cts_stmt_get_email(stmt, result, 1);

		if (result->id == contact->default_email)
			result->is_default = true;

		contact->emails = g_slist_append(contact->emails, result);
	}
	return CTS_SUCCESS;
}

static inline cts_name* cts_get_data_info_name(cts_stmt stmt)
{
	cts_name *result;

	result = (cts_name *)contacts_svc_value_new(CTS_VALUE_NAME);
	if (result) {
		result->embedded = true;
		cts_stmt_get_name(stmt, result, 1);
	}
	return result;
}

static inline int cts_get_data_info_event(cts_stmt stmt, contact_t *contact)
{
	int cnt=1;
	cts_event *result;

	result = (cts_event *)contacts_svc_value_new(CTS_VALUE_EVENT);
	if (result) {
		result->embedded = true;
		result->id = cts_stmt_get_int(stmt, cnt++);
		result->type = cts_stmt_get_int(stmt, cnt++);
		result->date = cts_stmt_get_int(stmt, cnt++);

		contact->events = g_slist_append(contact->events, result);
	}
	return CTS_SUCCESS;
}

static inline int cts_get_data_info_messenger(cts_stmt stmt, contact_t *contact)
{
	int cnt=1;
	cts_messenger *result;

	result = (cts_messenger *)contacts_svc_value_new(CTS_VALUE_MESSENGER);
	if (result) {
		char *temp;
		result->embedded = true;
		result->id = cts_stmt_get_int(stmt, cnt++);
		result->type = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->im_id = SAFE_STRDUP(temp);
		if (CTS_IM_TYPE_NONE == result->type) {
			temp = cts_stmt_get_text(stmt, cnt++);
			result->svc_name = SAFE_STRDUP(temp);
			temp = cts_stmt_get_text(stmt, cnt++);
			result->svc_op = SAFE_STRDUP(temp);
		}
		contact->messengers = g_slist_append(contact->messengers, result);
	}
	return CTS_SUCCESS;
}

static inline int cts_get_data_info_postal(cts_stmt stmt, contact_t *contact)
{
	int cnt=1;
	cts_postal *result;

	result = (cts_postal *)contacts_svc_value_new(CTS_VALUE_POSTAL);
	if (result) {
		char *temp;
		result->embedded = true;
		result->id = cts_stmt_get_int(stmt, cnt++);
		result->type = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->pobox= SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->postalcode = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->region= SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->locality = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->street = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->extended = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->country = SAFE_STRDUP(temp);

		contact->postal_addrs = g_slist_append(contact->postal_addrs, result);
	}
	return CTS_SUCCESS;
}

static inline int cts_get_data_info_web(cts_stmt stmt, contact_t *contact)
{
	int cnt=1;
	cts_web *result;

	result = (cts_web *)contacts_svc_value_new(CTS_VALUE_WEB);
	if (result) {
		char *temp;
		result->embedded = true;
		result->id = cts_stmt_get_int(stmt, cnt++);
		result->type = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->url = SAFE_STRDUP(temp);

		contact->web_addrs = g_slist_append(contact->web_addrs, result);
	}
	return CTS_SUCCESS;
}

static inline int cts_get_data_info_nick(cts_stmt stmt, contact_t *contact)
{
	int cnt=1;
	cts_nickname *result;

	result = (cts_nickname *)contacts_svc_value_new(CTS_VALUE_NICKNAME);
	if (result) {
		char *temp;
		result->embedded = true;
		result->id = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt+1);
		result->nick = SAFE_STRDUP(temp);

		contact->nicknames = g_slist_append(contact->nicknames, result);
	}
	return CTS_SUCCESS;
}

static inline cts_company* cts_get_data_info_company(cts_stmt stmt)
{
	int cnt=1;
	cts_company *result;

	result = (cts_company *)contacts_svc_value_new(CTS_VALUE_COMPANY);
	retvm_if(NULL == result, NULL, "contacts_svc_value_new() Failed");

	char *temp;
	result->embedded = true;
	result->id = cts_stmt_get_int(stmt, cnt++);
	cnt++;
	temp = cts_stmt_get_text(stmt, cnt++);
	result->name = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, cnt++);
	result->department = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, cnt++);
	result->jot_title = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, cnt++);
	result->role = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, cnt++);
	result->assistant_name = SAFE_STRDUP(temp);

	if (result->name || result->department || result->jot_title || result->role ||  result->assistant_name)
		return result;
	else {
		contacts_svc_value_free((CTSvalue *)result);
		return NULL;
	}
}

static cts_extend* cts_make_extend_data(cts_stmt stmt, int type, int cnt)
{
	cts_extend *result;
	result = (cts_extend *)contacts_svc_value_new(CTS_VALUE_EXTEND);
	if (result)
	{
		char *temp;
		result->type = type;
		result->id = cts_stmt_get_int(stmt, cnt++);
		result->data1 = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data2= SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data3 = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data4= SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data5 = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data6 = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data7 = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data8 = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data9 = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data10 = SAFE_STRDUP(temp);
	}
	return result;
}

static inline int cts_get_data_info_extend(cts_stmt stmt, int type,
		contact_t *contact)
{
	cts_extend *result;

	result = cts_make_extend_data(stmt, type, 1);
	if (result) {
		result->embedded = true;
		contact->extended_values = g_slist_append(contact->extended_values, result);
	}
	else
		return CTS_ERR_OUT_OF_MEMORY;

	return CTS_SUCCESS;
}


int cts_get_data_info(int op_code, int field, int index, contact_t *contact)
{
	int ret, datatype, len;
	const char *data;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	switch (op_code)
	{
	case CTS_GET_DATA_BY_CONTACT_ID:
		len = snprintf(query, sizeof(query), "SELECT datatype, id, data1, data2,"
				"data3, data4, data5, data6, data7, data8, data9, data10 "
				"FROM %s WHERE contact_id = %d", data, index);
		break;
	case CTS_GET_DATA_BY_ID:
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	if (CTS_DATA_FIELD_ALL != field && CTS_DATA_FIELD_EXTEND_ALL != field)
	{
		bool first= true;
		len += snprintf(query+len, sizeof(query)-len, " AND datatype IN (");

		if (field & CTS_DATA_FIELD_NAME) {
			first=false;
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_NAME);
		}
		if (field & CTS_DATA_FIELD_EVENT) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_EVENT);
		}
		if (field & CTS_DATA_FIELD_MESSENGER) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_MESSENGER);
		}
		if (field & CTS_DATA_FIELD_POSTAL) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_POSTAL);
		}
		if (field & CTS_DATA_FIELD_WEB) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_WEB);
		}
		if (field & CTS_DATA_FIELD_NICKNAME) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_NICKNAME);
		}
		if (field & CTS_DATA_FIELD_COMPANY) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_COMPANY);
		}
		if (field & CTS_DATA_FIELD_NUMBER) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_NUMBER);
		}
		if (field & CTS_DATA_FIELD_EMAIL) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_EMAIL);
		}

		len += snprintf(query+len, sizeof(query)-len, ")");
	}

	if (CTS_DATA_FIELD_ALL != field && field & CTS_DATA_FIELD_EXTEND_ALL) {
		len += snprintf(query+len, sizeof(query)-len, " AND datatype>=%d",
				CTS_DATA_EXTEND_START);
	}

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	do {
		datatype = cts_stmt_get_int(stmt, 0);

		switch (datatype)
		{
		case CTS_DATA_NAME:
			if (contact->name)
				ERR("name already Exist");
			else
				contact->name = cts_get_data_info_name(stmt);
			break;
		case CTS_DATA_EVENT:
			cts_get_data_info_event(stmt, contact);
			break;
		case CTS_DATA_MESSENGER:
			cts_get_data_info_messenger(stmt, contact);
			break;
		case CTS_DATA_POSTAL:
			cts_get_data_info_postal(stmt, contact);
			break;
		case CTS_DATA_WEB:
			cts_get_data_info_web(stmt, contact);
			break;
		case CTS_DATA_NICKNAME:
			cts_get_data_info_nick(stmt, contact);
			break;
		case CTS_DATA_NUMBER:
			cts_get_data_info_number(stmt, contact);
			break;
		case CTS_DATA_EMAIL:
			cts_get_data_info_email(stmt, contact);
			break;
		case CTS_DATA_COMPANY:
			if (contact->company)
				ERR("company already Exist");
			else
				contact->company = cts_get_data_info_company(stmt);
			break;
		default:
			if (CTS_DATA_EXTEND_START <= datatype) {
				cts_get_data_info_extend(stmt, datatype, contact);
				break;
			}
			ERR("Unknown data type(%d)", datatype);
			continue;
		}
	}while(CTS_TRUE == cts_stmt_step(stmt));

	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}

static inline int cts_get_groups_info(int index, contact_t *contact)
{
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	GSList *result_list=NULL;

	snprintf(query, sizeof(query), "SELECT group_id, addrbook_id,"
			" group_name"
			" FROM %s WHERE group_id IN (SELECT group_id"
			" FROM %s WHERE contact_id = %d)"
			" ORDER BY group_name COLLATE NOCASE",
			CTS_TABLE_GROUPS, CTS_TABLE_GROUPING_INFO, index);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	while (CTS_TRUE == cts_stmt_step(stmt))
	{
		cts_group *group_info;
		group_info = (cts_group *)contacts_svc_value_new(CTS_VALUE_GROUP_RELATION);

		if (group_info)
		{
			group_info->id = cts_stmt_get_int(stmt, 0);
			group_info->addrbook_id = cts_stmt_get_int(stmt, 1);
			group_info->embedded = true;
			group_info->name = SAFE_STRDUP(cts_stmt_get_text(stmt, 2));
			group_info->img_loaded = false; //It will load at cts_value_get_str_group()

			result_list = g_slist_append(result_list, group_info);
		}
	}

	cts_stmt_finalize(stmt);
	contact->grouprelations = result_list;

	return CTS_SUCCESS;

}

static inline int cts_get_number_value(int op_code, int id, CTSvalue **value)
{
	int ret;
	cts_stmt stmt;
	const char *data;
	cts_number *number;
	char query[CTS_SQL_MAX_LEN] = {0};

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	if (CTS_GET_DEFAULT_NUMBER_VALUE == op_code) {
		snprintf(query, sizeof(query),
				"SELECT B.id, B.data1, B.data2 FROM %s A, %s B "
				"WHERE A.contact_id = %d AND B.id=A.default_num AND B.datatype = %d",
				CTS_TABLE_CONTACTS, data, id, CTS_DATA_NUMBER);
	}
	else if (CTS_GET_NUMBER_VALUE == op_code) {
		snprintf(query, sizeof(query),
				"SELECT id, data1, data2, contact_id FROM %s "
				"WHERE id = %d AND datatype = %d",
				data, id, CTS_DATA_NUMBER);
	}
	else {
		ERR("Invalid op_code(%d)", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	number = (cts_number *)contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number) {
		ret = CTS_SUCCESS;
		number->v_type = CTS_VALUE_RDONLY_NUMBER;
		number->embedded = true;
		cts_stmt_get_number(stmt, number, 0);

		if (CTS_GET_DEFAULT_NUMBER_VALUE == op_code)
			number->is_default = true;
		else
			ret = cts_stmt_get_int(stmt, 3);

		*value = (CTSvalue*) number;

		cts_stmt_finalize(stmt);
		return ret;
	}
	else {
		ERR("contacts_svc_value_new() Failed");
		cts_stmt_finalize(stmt);
		return CTS_ERR_OUT_OF_MEMORY;
	}
}

static inline int cts_get_email_value(int op_code, int id, CTSvalue **value)
{
	int ret;
	cts_stmt stmt;
	const char *data;
	cts_email *email;
	char query[CTS_SQL_MAX_LEN] = {0};

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	if (CTS_GET_DEFAULT_EMAIL_VALUE == op_code) {
		snprintf(query, sizeof(query),
				"SELECT B.id, B.data1, B.data2 FROM %s A, %s B "
				"WHERE A.contact_id = %d AND B.id=A.default_email AND B.datatype = %d",
				CTS_TABLE_CONTACTS, data, id, CTS_DATA_EMAIL);
	}
	else if (CTS_GET_EMAIL_VALUE == op_code) {
		snprintf(query, sizeof(query),
				"SELECT id, data1, data2, contact_id FROM %s "
				"WHERE id = %d AND datatype = %d",
				data, id, CTS_DATA_EMAIL);
	}
	else {
		ERR("Invalid op_code(%d)", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	email = (cts_email *)contacts_svc_value_new(CTS_VALUE_EMAIL);
	if (email)
	{
		ret = CTS_SUCCESS;
		email->v_type = CTS_VALUE_RDONLY_EMAIL;
		email->embedded = true;
		cts_stmt_get_email(stmt, email, 0);

		if (CTS_GET_DEFAULT_EMAIL_VALUE == op_code)
			email->is_default = true;
		else
			ret = cts_stmt_get_int(stmt, 3);

		*value = (CTSvalue*) email;

		cts_stmt_finalize(stmt);
		return ret;
	}
	else
	{
		ERR("contacts_svc_value_new() Failed");
		cts_stmt_finalize(stmt);
		return CTS_ERR_OUT_OF_MEMORY;
	}
}

static inline int cts_get_extend_data(int type, int id, CTSvalue **value)
{
	int ret;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query), "SELECT id, data1, data2,"
			"data3, data4, data5, data6, data7, data8, data9, data10 "
			"FROM %s WHERE datatype = %d AND contact_id = %d", CTS_TABLE_DATA, type, id);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	*value = (CTSvalue *)cts_make_extend_data(stmt, type, 0);
	cts_stmt_finalize(stmt);

	retvm_if(NULL == *value, CTS_ERR_OUT_OF_MEMORY, "cts_make_extend_data() return NULL");

	return CTS_SUCCESS;
}

API int contacts_svc_get_contact_value(cts_get_contact_val_op op_code,
		int id, CTSvalue **value)
{
	int ret;
	contact_t temp={0};

	retv_if(NULL == value, CTS_ERR_ARG_NULL);
	CTS_START_TIME_CHECK;

	if ((int)CTS_DATA_EXTEND_START <= op_code) {
		ret = cts_get_extend_data(op_code, id, value);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_get_extend_data() Failed(%d)", ret);
	}
	else {
		switch (op_code)
		{
		case CTS_GET_NAME_VALUE:
			ret = cts_get_data_info(CTS_GET_DATA_BY_CONTACT_ID,
					CTS_DATA_FIELD_NAME, id, &temp);
			retvm_if(CTS_SUCCESS != ret, ret,
					"cts_get_data_info(CTS_GET_DATA_BY_CONTACT_ID) Failed(%d)", ret);
			if (temp.name) {
				temp.name->v_type = CTS_VALUE_RDONLY_NAME;
				*value = (CTSvalue *)temp.name;
			}else
				*value = NULL;
			break;
		case CTS_GET_DEFAULT_NUMBER_VALUE:
		case CTS_GET_NUMBER_VALUE:
			ret = cts_get_number_value(op_code, id, value);
			retvm_if(ret < CTS_SUCCESS, ret,
					"cts_get_number_value() Failed(%d)", ret);
			break;
		case CTS_GET_DEFAULT_EMAIL_VALUE:
		case CTS_GET_EMAIL_VALUE:
			ret = cts_get_email_value(op_code, id, value);
			retvm_if(ret < CTS_SUCCESS, ret, "cts_get_email_value() Failed(%d)", ret);
			break;
		case CTS_GET_COMPANY_VALUE:
			ret = cts_get_data_info(CTS_GET_DATA_BY_CONTACT_ID,
					CTS_DATA_FIELD_COMPANY, id, &temp);
			retvm_if(CTS_SUCCESS != ret, ret,
					"cts_get_data_info(CTS_DATA_FIELD_COMPANY) Failed(%d)", ret);
			if (temp.company) {
				temp.company->v_type = CTS_VALUE_RDONLY_COMPANY;
				*value = (CTSvalue *)temp.company;
			}else
				*value = NULL;
			break;
		default:
			ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
			return CTS_ERR_ARG_INVALID;
		}
	}
	if (NULL == *value) return CTS_ERR_NO_DATA;

	CTS_END_TIME_CHECK();
	return ret;
}

static inline cts_ct_base* cts_get_myprofile_base(cts_stmt stmt)
{
	cts_ct_base *result;

	result = (cts_ct_base *)contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);
	if (result) {
		char *temp;

		result->embedded = true;
		temp = cts_stmt_get_text(stmt, 2);
		result->note = SAFE_STRDUP(temp);
		result->img_path = cts_get_img(CTS_MY_IMAGE_LOCATION, 0, NULL, 0);
	}
	return result;
}

int cts_get_myprofile_data(contact_t *contact)
{
	int ret, datatype, len;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN];

	len = snprintf(query, sizeof(query), "SELECT datatype, id, data1, data2,"
			"data3, data4, data5, data6, data7, data8, data9, data10 "
			"FROM %s ", CTS_TABLE_MY_PROFILES);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret) {
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	do {
		datatype = cts_stmt_get_int(stmt, 0);

		switch (datatype)
		{
		case 0:
			contact->base = cts_get_myprofile_base(stmt);
			break;
		case CTS_DATA_NAME:
			if (contact->name)
				ERR("name already Exist");
			else
				contact->name = cts_get_data_info_name(stmt);
			break;
		case CTS_DATA_EVENT:
			cts_get_data_info_event(stmt, contact);
			break;
		case CTS_DATA_MESSENGER:
			cts_get_data_info_messenger(stmt, contact);
			break;
		case CTS_DATA_POSTAL:
			cts_get_data_info_postal(stmt, contact);
			break;
		case CTS_DATA_WEB:
			cts_get_data_info_web(stmt, contact);
			break;
		case CTS_DATA_NICKNAME:
			cts_get_data_info_nick(stmt, contact);
			break;
		case CTS_DATA_NUMBER:
			cts_get_data_info_number(stmt, contact);
			break;
		case CTS_DATA_EMAIL:
			cts_get_data_info_email(stmt, contact);
			break;
		case CTS_DATA_COMPANY:
			if (contact->company)
				ERR("company already Exist");
			else
				contact->company = cts_get_data_info_company(stmt);
			break;
		default:
			if (CTS_DATA_EXTEND_START <= datatype) {
				cts_get_data_info_extend(stmt, datatype, contact);
				break;
			}
			ERR("Unknown data type(%d)", datatype);
			continue;
		}
	}while(CTS_TRUE == cts_stmt_step(stmt));

	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}

int cts_get_myprofile(CTSstruct **contact)
{
	int ret;
	contact_t *record;

	retv_if(NULL == contact, CTS_ERR_ARG_NULL);

	record = (contact_t *)contacts_svc_struct_new(CTS_STRUCT_CONTACT);

	ret = cts_get_myprofile_data(record);
	if (CTS_SUCCESS != ret) {
		ERR("cts_get_myprofile_data() Failed(%d)", ret);
		contacts_svc_struct_free((CTSstruct *)record);
		return ret;
	}

	*contact = (CTSstruct *)record;
	return CTS_SUCCESS;
}

API int contacts_svc_get_contact(int index, CTSstruct **contact)
{
	int ret;
	contact_t *record;

	retv_if(NULL == contact, CTS_ERR_ARG_NULL);
	if (0 == index)
		return cts_get_myprofile(contact);
	CTS_START_TIME_CHECK;

	record = (contact_t *)contacts_svc_struct_new(CTS_STRUCT_CONTACT);

	ret = cts_get_main_contacts_info(CTS_MAIN_CTS_GET_ALL, index, record);
	if (CTS_SUCCESS != ret) {
		ERR("cts_get_main_contacts_info(ALL) Failed(%d)", ret);
		goto CTS_RETURN_ERROR;
	}

	ret = cts_get_data_info(CTS_GET_DATA_BY_CONTACT_ID,
			CTS_DATA_FIELD_ALL, index, record);
	if (CTS_SUCCESS != ret) {
		ERR("cts_get_data_info(CTS_GET_DATA_BY_CONTACT_ID) Failed(%d)", ret);
		goto CTS_RETURN_ERROR;
	}

	ret = cts_get_groups_info(index, record);
	if (CTS_SUCCESS != ret) {
		ERR("cts_get_group_info(CTS_GET_DATA_BY_CONTACT_ID) Failed(%d)", ret);
		goto CTS_RETURN_ERROR;
	}

	*contact = (CTSstruct *)record;

	CTS_END_TIME_CHECK();
	return CTS_SUCCESS;

CTS_RETURN_ERROR:
	contacts_svc_struct_free((CTSstruct *)record);
	return ret;
}

API int contacts_svc_find_contact_by(cts_find_op op_code,
		const char *user_data)
{
	int ret;
	const char *temp, *data;
	char query[CTS_SQL_MAX_LEN] = {0};
	char normalized_val[CTS_SQL_MIN_LEN];

	CTS_START_TIME_CHECK;
	retv_if(NULL == user_data, CTS_ERR_ARG_NULL);

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	switch (op_code)
	{
	case CTS_FIND_BY_NUMBER:
		ret = cts_clean_number(user_data, normalized_val, sizeof(normalized_val));
		retvm_if(ret <= 0, CTS_ERR_ARG_INVALID, "Number(%s) is invalid", user_data);

		temp = cts_normalize_number(normalized_val);
		snprintf(query, sizeof(query), "SELECT contact_id "
				"FROM %s WHERE datatype = %d AND data3 = '%s' LIMIT 1",
				data, CTS_DATA_NUMBER, temp);
		ret = cts_query_get_first_int_result(query);
		break;
	case CTS_FIND_BY_EMAIL:
		snprintf(query, sizeof(query), "SELECT contact_id "
				"FROM %s WHERE datatype = %d AND data2 = '%s' LIMIT 1",
				data, CTS_DATA_EMAIL, user_data);
		ret = cts_query_get_first_int_result(query);
		break;
	case CTS_FIND_BY_NAME:
		ret = cts_normalize_str(user_data, normalized_val, sizeof(normalized_val));
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_str() Failed(%d)", ret);

		if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			temp = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
		else
			temp = CTS_SCHEMA_DATA_NAME_LOOKUP;

		snprintf(query, sizeof(query), "SELECT contact_id FROM %s "
				"WHERE %s LIKE '%%%s%%' LIMIT 1",
				data, temp, normalized_val);

		ret = cts_query_get_first_int_result(query);
		break;
	case CTS_FIND_BY_UID:
		snprintf(query, sizeof(query), "SELECT contact_id "
				"FROM %s WHERE uid = '%s' LIMIT 1", CTS_TABLE_CONTACTS, user_data);
		ret = cts_query_get_first_int_result(query);
		break;
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	CTS_END_TIME_CHECK();
	return ret;
}

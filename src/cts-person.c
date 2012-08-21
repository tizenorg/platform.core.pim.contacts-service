/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
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
#include "cts-internal.h"
#include "cts-utils.h"
#include "cts-sqlite.h"
#include "cts-schema.h"
#include "cts-struct-ext.h"
#include "cts-normalize.h"
#include "cts-restriction.h"
#include "cts-person.h"

API int contacts_svc_link_person(int base_person_id, int sub_person_id)
{
	int ret;
	char query[CTS_SQL_MIN_LEN];

	retvm_if(base_person_id == sub_person_id, CTS_ERR_ARG_INVALID,
		"base_person_id(%d), sub_person_id(%d)", base_person_id, sub_person_id);

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"UPDATE %s SET person_id=%d WHERE person_id=%d",
			CTS_TABLE_CONTACTS, base_person_id, sub_person_id);
	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query),
			"UPDATE %s "
			"SET outgoing_count=(SELECT MAX(outgoing_count) FROM %s WHERE person_id IN (%d, %d))"
			"WHERE person_id=%d",
			CTS_TABLE_PERSONS,
			CTS_TABLE_PERSONS, base_person_id, sub_person_id,
			base_person_id);
	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE person_id = %d",
			CTS_TABLE_PERSONS, sub_person_id);
	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	cts_set_link_noti();
	contacts_svc_end_trans(true);

	return CTS_SUCCESS;
}


int cts_insert_person(int contact_id, int outgoing_cnt)
{
	int ret, index;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN];

	snprintf(query, sizeof(query),
			"INSERT INTO %s(person_id, outgoing_count) VALUES(%d, %d)",
			CTS_TABLE_PERSONS, contact_id, outgoing_cnt);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret) {
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}
	index = cts_db_get_last_insert_id();
	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}


int cts_person_change_primary_contact(int person_id)
{
	int ret, new_person;
	char query[CTS_SQL_MIN_LEN];

	snprintf(query, sizeof(query),
			"SELECT B.contact_id "
			"FROM %s A, %s B ON A.contact_id = B.contact_id "
			"WHERE A.datatype = %d AND B.person_id = %d AND B.contact_id != %d "
			"ORDER BY data1, %s",
			CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME, person_id, person_id,
			CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	new_person = cts_query_get_first_int_result(query);
	retvm_if(new_person < CTS_SUCCESS, new_person,
		"cts_query_get_first_int_result() Failed(%d)", new_person);

	snprintf(query, sizeof(query),
			"UPDATE %s SET person_id=%d WHERE person_id=%d",
			CTS_TABLE_CONTACTS, new_person, person_id);
	ret = cts_query_exec(query);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_query_exec() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"UPDATE %s SET person_id=%d WHERE person_id=%d",
			CTS_TABLE_PERSONS, new_person, person_id);
	ret = cts_query_exec(query);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_query_exec() Failed(%d)", ret);

	return new_person;
}


API int contacts_svc_unlink_person(int person_id, int contact_id)
{
	int ret, outgoing_cnt;
	char query[CTS_SQL_MIN_LEN];

	snprintf(query, sizeof(query),
			"SELECT outgoing_count FROM %s WHERE person_id = %d",
			CTS_TABLE_PERSONS, person_id);

	outgoing_cnt = cts_query_get_first_int_result(query);
	retvm_if(outgoing_cnt < CTS_SUCCESS, outgoing_cnt,
		"cts_query_get_first_int_result() Failed(%d)", outgoing_cnt);

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	if (person_id == contact_id) {
		ret = cts_person_change_primary_contact(person_id);
		if (CTS_SUCCESS != ret) {
			ERR("cts_person_change_primary_contact() Failed(%d)", ret);
			contacts_svc_end_trans(false);
			return ret;
		}
	}

	ret = cts_insert_person(contact_id, outgoing_cnt);
	if (CTS_SUCCESS != ret) {
		ERR("cts_insert_person() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query),
			"UPDATE %s SET person_id=%d WHERE contact_id=%d",
			CTS_TABLE_CONTACTS, contact_id, contact_id);
	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}
	cts_set_link_noti();
	contacts_svc_end_trans(true);

	return CTS_SUCCESS;
}


API int contacts_svc_get_person(int person_id, CTSstruct **person)
{
	int ret;
	cts_stmt stmt;
	CTSstruct *contact;
	char query[CTS_SQL_MAX_LEN];

	snprintf(query, sizeof(query), "SELECT contact_id FROM %s "
		"WHERE person_id = %d", CTS_TABLE_CONTACTS, person_id);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	contact = contacts_svc_struct_new(CTS_STRUCT_CONTACT);
	do {
		CTSstruct *tmp;
		ret = contacts_svc_get_contact(cts_stmt_get_int(stmt, 0), &tmp);
		if (CTS_SUCCESS != ret) {
			ERR("contacts_svc_get_contact() Failed(%d)", ret);
			contacts_svc_struct_free(contact);
			cts_stmt_finalize(stmt);
			return ret;
		}
		if (cts_stmt_get_int(stmt, 0) == person_id) {
			contacts_svc_struct_merge(tmp, contact);
			contacts_svc_struct_free(contact);
			contact = tmp;
		}
		else {
			contacts_svc_struct_merge(contact, tmp);
			contacts_svc_struct_free(tmp);
		}
	}while(CTS_TRUE == cts_stmt_step(stmt));
	cts_stmt_finalize(stmt);

	*person = contact;

	return CTS_SUCCESS;
}

/**
 * The Number can be made with a set of values by specifying one or more values.
 * \n Example : CTS_SIMILAR_NAME|CTS_SIMILAR_NUMBER
 */
typedef enum {
	CTS_SIMILAR_NONE = 0,
	CTS_SIMILAR_NAME = 1<<0,
	CTS_SIMILAR_NUMBER = 1<<1,
	CTS_SIMILAR_EMAIL = 1<<2,
}cts_similar_op;


API int contacts_svc_find_similar_person(cts_similar_op op_code, CTSstruct *contact)
{
	return CTS_SUCCESS;
}


/**
 * This function gets index of person related with the contact.
 * @param[in] contact_id index of contact
 * @return index of found person on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void get_person(void)
 {
    int index, ret=-1;
    CTSstruct *person = NULL;

    index = contacts_svc_find_person_by_contact(123);
    if(CTS_SUCCESS < index)
      ret = contacts_svc_get_person(index, &person);
    if(ret < CTS_SUCCESS)
    {
       printf("No found record\n");
       return;
    }
 }
 * @endcode
 */
API int contacts_svc_find_person_by_contact(int contact_id)
{
	int ret;
	char query[CTS_SQL_MIN_LEN];

	snprintf(query, sizeof(query), "SELECT person_id "
			"FROM %s WHERE contact_id = %d", CTS_TABLE_CONTACTS, contact_id);
	ret = cts_query_get_first_int_result(query);

	return ret;
}

API int contacts_svc_find_person_by(cts_find_op op_code,
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
		snprintf(query, sizeof(query), "SELECT person_id FROM %s "
				"WHERE contact_id = (SELECT contact_id FROM %s "
					"WHERE datatype = %d AND data3 = '%s' LIMIT 1)",
				CTS_TABLE_CONTACTS, CTS_TABLE_DATA, CTS_DATA_NUMBER, temp);
		ret = cts_query_get_first_int_result(query);
		break;
	case CTS_FIND_BY_EMAIL:
		snprintf(query, sizeof(query), "SELECT person_id FROM %s "
				"WHERE contact_id = (SELECT contact_id FROM %s "
					"WHERE datatype = %d AND data2 = '%s' LIMIT 1)",
				CTS_TABLE_CONTACTS, data, CTS_DATA_EMAIL, user_data);
		ret = cts_query_get_first_int_result(query);
		break;
	case CTS_FIND_BY_NAME:
		ret = cts_normalize_str(user_data, normalized_val, sizeof(normalized_val));
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_str() Failed(%d)", ret);

		if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			temp = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
		else
			temp = CTS_SCHEMA_DATA_NAME_LOOKUP;

		snprintf(query, sizeof(query), "SELECT person_id FROM %s "
				"WHERE contact_id = (SELECT contact_id FROM %s "
					"WHERE %s LIKE '%%%s%%' LIMIT 1)",
				CTS_TABLE_CONTACTS, data, temp, normalized_val);

		ret = cts_query_get_first_int_result(query);
		break;
	case CTS_FIND_BY_UID:
		snprintf(query, sizeof(query), "SELECT person_id "
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


static inline int _cts_get_person_def_email_value(int id, CTSvalue **value)
{
	int ret;
	const char *data;
	cts_stmt stmt;
	cts_email *email;
	char query[CTS_SQL_MAX_LEN];

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	snprintf(query, sizeof(query),
			"SELECT B.id, B.data1, B.data2 "
			"FROM %s A, %s B ON B.id=A.default_email "
			"WHERE A.contact_id = %d",
			CTS_TABLE_CONTACTS, data, id);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS == ret) {
		cts_stmt_finalize(stmt);

		snprintf(query, sizeof(query),
				"SELECT B.id, B.data1, B.data2 "
				"FROM %s A, %s B ON B.id=A.default_email "
				"WHERE A.person_id = %d LIMIT 1",
				CTS_TABLE_CONTACTS, data, id);

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

		ret = cts_stmt_step(stmt);
		if (CTS_SUCCESS == ret)
			ret = CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	if (ret < CTS_SUCCESS) {
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}

	email = (cts_email *)contacts_svc_value_new(CTS_VALUE_EMAIL);
	if (email) {
		email->v_type = CTS_VALUE_RDONLY_EMAIL;
		email->embedded = true;
		email->is_default = true;
		cts_stmt_get_email(stmt, email, 0);

		*value = (CTSvalue*)email;
		ret = CTS_SUCCESS;
	}
	else {
		ERR("contacts_svc_value_new() Failed");
		ret = CTS_ERR_OUT_OF_MEMORY;
	}

	cts_stmt_finalize(stmt);
	return ret;
}

static inline int _cts_get_person_def_number_value(int id, CTSvalue **value)
{
	int ret;
	cts_stmt stmt;
	const char *data;
	cts_number *number;
	char query[CTS_SQL_MAX_LEN];

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	snprintf(query, sizeof(query),
			"SELECT B.id, B.data1, B.data2 "
			"FROM %s A, %s B ON B.id=A.default_num "
			"WHERE A.contact_id = %d",
			CTS_TABLE_CONTACTS, data, id);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS == ret) {
		cts_stmt_finalize(stmt);

		snprintf(query, sizeof(query),
				"SELECT B.id, B.data1, B.data2 "
				"FROM %s A, %s B ON B.id=A.default_num "
				"WHERE A.person_id = %d LIMIT 1",
				CTS_TABLE_CONTACTS, data, id);

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

		ret = cts_stmt_step(stmt);
		if (CTS_SUCCESS == ret)
			ret = CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	number = (cts_number *)contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number) {
		number->v_type = CTS_VALUE_RDONLY_NUMBER;
		number->embedded = true;
		number->is_default = true;
		cts_stmt_get_number(stmt, number, 0);

		*value = (CTSvalue*)number;
		ret = CTS_SUCCESS;
	}
	else {
		ERR("contacts_svc_value_new() Failed");
		ret = CTS_ERR_OUT_OF_MEMORY;
	}

	cts_stmt_finalize(stmt);
	return ret;
}


API int contacts_svc_get_person_value(cts_get_person_val_op op_code,
		int person_id, CTSvalue **value)
{
	int ret;
	contact_t temp={0};

	retv_if(NULL == value, CTS_ERR_ARG_NULL);
	CTS_START_TIME_CHECK;

	switch (op_code)
	{
	case CTS_GET_PERSON_NAME_VALUE:
		ret = cts_get_data_info(CTS_GET_DATA_BY_CONTACT_ID,
				CTS_DATA_FIELD_NAME, person_id, &temp);
		retvm_if(CTS_SUCCESS != ret, ret,
				"cts_get_data_info(CTS_GET_DATA_BY_CONTACT_ID) Failed(%d)", ret);
		if (temp.name) {
			temp.name->v_type = CTS_VALUE_RDONLY_NAME;
			*value = (CTSvalue *)temp.name;
		}else
			*value = NULL;
		break;
	case CTS_GET_PERSON_DEFAULT_NUMBER_VALUE:
		ret = _cts_get_person_def_number_value(person_id, value);
		retvm_if(ret < CTS_SUCCESS, ret, "_cts_get_person_def_number_value() Failed(%d)", ret);
		break;
	case CTS_GET_PERSON_DEFAULT_EMAIL_VALUE:
		ret = _cts_get_person_def_email_value(person_id, value);
		retvm_if(ret < CTS_SUCCESS, ret, "_cts_get_person_def_email_value() Failed(%d)", ret);
		break;
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	if (NULL == *value)
		return CTS_ERR_NO_DATA;

	CTS_END_TIME_CHECK();
	return ret;
}


int cts_check_linked_contact(int contact_id)
{
	int ret;
	char query[CTS_SQL_MIN_LEN];

	snprintf(query, sizeof(query),
			"SELECT COUNT(person_id) FROM %s WHERE person_id = %d",
			CTS_TABLE_CONTACTS, contact_id);
	ret = cts_query_get_first_int_result(query);
	retvm_if(ret < CTS_SUCCESS, ret, "cts_query_get_first_int_result() Failed(%d)", ret);

	if (0 == ret)
		return CTS_LINKED_SECONDARY;
	else if (1 == ret)
		return CTS_LINKED_NONE;
	else
		return CTS_LINKED_PRIMARY;
}


int cts_delete_person(int index)
{
	int ret;
	char query[CTS_SQL_MIN_LEN];

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE person_id = %d",
			CTS_TABLE_PERSONS, index);
	ret = cts_query_exec(query);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_query_exec() Failed(%d)", ret);

	return CTS_SUCCESS;
}


int cts_person_garbagecollection(void)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN];

	snprintf(query, sizeof(query), "SELECT MIN(contact_id), person_id FROM %s "
		"WHERE person_id NOT IN (SELECT person_id FROM %s WHERE contact_id = person_id) "
		"GROUP BY person_id", CTS_TABLE_CONTACTS, CTS_TABLE_CONTACTS);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	while (CTS_TRUE == cts_stmt_step(stmt)) {
		int contact_id, person_id;

		contact_id = cts_stmt_get_int(stmt, 0);
		person_id = cts_stmt_get_int(stmt, 1);

		snprintf(query, sizeof(query),
				"UPDATE %s SET person_id=%d WHERE person_id=%d",
				CTS_TABLE_CONTACTS, contact_id, person_id);
		ret = cts_query_exec(query);
		if (CTS_SUCCESS != ret) {
			ERR("cts_query_exec() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
		snprintf(query, sizeof(query),
				"UPDATE %s SET person_id=%d WHERE person_id=%d",
				CTS_TABLE_PERSONS, contact_id, person_id);
		ret = cts_query_exec(query);
		if (CTS_SUCCESS != ret) {
			ERR("cts_query_exec() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}
	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}

/**
 * This function deletes all contacts related with a person.
 * It is not only deletes contact records from contact table,
 * but also clears up all the info of these contacts(group relation info, favorites info and etc.).
 *
 * @param[in] index The index of person to delete in database.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
API int contacts_svc_delete_person(int index)
{
	CTS_FN_CALL;
	int ret;
	char query[CTS_SQL_MIN_LEN];

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"INSERT INTO %s SELECT contact_id, addrbook_id, %d FROM %s WHERE person_id = %d",
			CTS_TABLE_DELETEDS, cts_get_next_ver(), CTS_TABLE_CONTACTS, index);
	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE person_id = %d",
			CTS_TABLE_CONTACTS, index);
	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE person_id = %d",
			CTS_TABLE_PERSONS, index);
	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	cts_set_contact_noti();

	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}



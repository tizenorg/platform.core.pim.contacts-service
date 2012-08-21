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
#include "cts-internal.h"
#include "cts-schema.h"
#include "cts-sqlite.h"
#include "cts-utils.h"
#include "cts-person.h"
#include "cts-addressbook.h"

static inline int cts_reset_internal_addressbook(void)
{
	CTS_FN_CALL;
	int ret;
	char query[CTS_SQL_MIN_LEN];

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE addrbook_id = %d",
			CTS_TABLE_CONTACTS, CTS_ADDRESSBOOK_INTERNAL);

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE addrbook_id = %d",
			CTS_TABLE_GROUPS, CTS_ADDRESSBOOK_INTERNAL);

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	cts_set_contact_noti();
	cts_set_group_noti();
	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

API int contacts_svc_insert_addressbook(CTSvalue *addressbook)
{
	int ret, index;
	cts_stmt stmt = NULL;
	cts_addrbook *record = (cts_addrbook *)addressbook;
	char query[CTS_SQL_MAX_LEN] = {0};

	retv_if(NULL == addressbook, CTS_ERR_ARG_NULL);
	retv_if(NULL == record->name, CTS_ERR_ARG_INVALID);
	retvm_if(CTS_VALUE_ADDRESSBOOK != record->v_type, CTS_ERR_ARG_INVALID,
			"addressbook is invalid type(%d)", record->v_type);

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"INSERT INTO %s(addrbook_name, acc_id, acc_type, mode) "
			"VALUES(?, %d, %d, %d)",
			CTS_TABLE_ADDRESSBOOKS, record->acc_id, record->acc_type, record->mode);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		ERR("cts_query_prepare() Failed");
		contacts_svc_end_trans(false);
		return CTS_ERR_DB_FAILED;
	}

	cts_stmt_bind_text(stmt, 1, record->name);

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret) {
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		contacts_svc_end_trans(false);
		return ret;
	}
	index = cts_db_get_last_insert_id();
	cts_stmt_finalize(stmt);

	cts_set_addrbook_noti();
	ret = contacts_svc_end_trans(true);
	retvm_if(ret < CTS_SUCCESS, ret, "contacts_svc_end_trans() Failed(%d)", ret);

	return index;
}

API int contacts_svc_delete_addressbook(int addressbook_id)
{
	CTS_FN_CALL;
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};

	if (CTS_ADDRESSBOOK_INTERNAL == addressbook_id)
		return cts_reset_internal_addressbook();

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE addrbook_id = %d",
			CTS_TABLE_ADDRESSBOOKS, addressbook_id);

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	ret = cts_person_garbagecollection();
	if (CTS_SUCCESS != ret) {
		ERR("cts_person_garbagecollection() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	ret = cts_db_change();

	if (0 < ret) {
		cts_set_contact_noti();
		cts_set_group_noti();
		cts_set_addrbook_noti();
		ret = contacts_svc_end_trans(true);
	}
	else {
		contacts_svc_end_trans(false);
		ret = CTS_ERR_NO_DATA;
	}
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

API int contacts_svc_update_addressbook(CTSvalue *addressbook)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};
	cts_addrbook *record = (cts_addrbook *)addressbook;

	retv_if(NULL == addressbook, CTS_ERR_ARG_NULL);
	retv_if(NULL == record->name, CTS_ERR_ARG_INVALID);
	retvm_if(CTS_VALUE_ADDRESSBOOK != record->v_type, CTS_ERR_ARG_INVALID,
			"addressbook is invalid type(%d)", record->v_type);

	snprintf(query, sizeof(query),
			"UPDATE %s SET addrbook_name=?, acc_id=%d, acc_type=%d, mode=%d "
			"WHERE addrbook_id=%d", CTS_TABLE_ADDRESSBOOKS,
			record->acc_id, record->acc_type, record->mode, record->id);

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		ERR("cts_query_prepare() Failed");
		contacts_svc_end_trans(false);
		return CTS_ERR_DB_FAILED;
	}

	cts_stmt_bind_text(stmt, 1, record->name);

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret) {
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		contacts_svc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	cts_set_addrbook_noti();

	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

API int contacts_svc_get_addressbook(int addressbook_id, CTSvalue **ret_value)
{
	int ret;
	cts_addrbook *ab;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	retv_if(NULL == ret_value, CTS_ERR_ARG_NULL);

	snprintf(query, sizeof(query),
			"SELECT addrbook_id, addrbook_name, acc_id, acc_type, mode "
			"FROM %s WHERE addrbook_id = %d", CTS_TABLE_ADDRESSBOOKS, addressbook_id);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret) {
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	ab = (cts_addrbook *)contacts_svc_value_new(CTS_VALUE_ADDRESSBOOK);
	if (ab) {
		ab->embedded = true;
		cts_stmt_get_addressbook(stmt, ab);
	}

	cts_stmt_finalize(stmt);

	*ret_value = (CTSvalue *)ab;

	return CTS_SUCCESS;
}

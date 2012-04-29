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
#include "cts-list.h"
#include "cts-group.h"

API int contacts_svc_find_group(int addressbook_id, const char *name)
{
	char query[CTS_SQL_MIN_LEN];

	retv_if(NULL == name, CTS_ERR_ARG_NULL);

	snprintf(query, sizeof(query), "SELECT group_id FROM %s "
			"WHERE group_name = '%s' LIMIT 1",
			CTS_TABLE_GROUPS, name);

	return cts_query_get_first_int_result(query);
}


API int contacts_svc_get_group(int index, CTSvalue **retgroup)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	retv_if(NULL == retgroup, CTS_ERR_ARG_NULL);

	snprintf(query, sizeof(query),
			"SELECT group_id, addrbook_id, group_name, ringtone "
			"FROM %s WHERE group_id = %d", CTS_TABLE_GROUPS, index);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}
	cts_group *group;
	group = (cts_group *)contacts_svc_value_new(CTS_VALUE_GROUP);

	if (group)
	{
		group->embedded = true;
		group->id = cts_stmt_get_int(stmt, 0);
		group->addrbook_id = cts_stmt_get_int(stmt, 1);
		group->name = SAFE_STRDUP(cts_stmt_get_text(stmt, 2));
		group->ringtone_path = SAFE_STRDUP(cts_stmt_get_text(stmt, 3));
	}
	cts_stmt_finalize(stmt);

	*retgroup = (CTSvalue *)group;

	return CTS_SUCCESS;
}

API int contacts_svc_update_group(CTSvalue *group)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};
	cts_group *record = (cts_group *)group;

	retv_if(NULL == group, CTS_ERR_ARG_NULL);
	retvm_if(CTS_VALUE_GROUP != group->v_type, CTS_ERR_ARG_INVALID,
			"group is invalid type(%d)", group->v_type);
	retvm_if(NULL == record->name, CTS_ERR_ARG_INVALID,
			"The name of group is empty.");

	snprintf(query, sizeof(query), "UPDATE %s SET group_name=?, ringtone=? "
			"WHERE group_id=%d", CTS_TABLE_GROUPS, record->id);

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		ERR("cts_query_prepare() Failed");
		contacts_svc_end_trans(false);
		return CTS_ERR_DB_FAILED;
	}

	cts_stmt_bind_text(stmt, 1, record->name);
	if (record->ringtone_path)
		cts_stmt_bind_text(stmt, 2, record->ringtone_path);

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		contacts_svc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	cts_set_group_noti();

	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

API int contacts_svc_insert_group(int addressbook_id, CTSvalue *group)
{
	int ret, index;
	cts_stmt stmt = NULL;
	cts_group *record = (cts_group *)group;
	char query[CTS_SQL_MAX_LEN] = {0};

	retv_if(NULL == group, CTS_ERR_ARG_NULL);
	retvm_if(CTS_VALUE_GROUP != group->v_type, CTS_ERR_ARG_INVALID,
			"group is invalid type(%d)", group->v_type);
	retvm_if(NULL == record->name, CTS_ERR_ARG_INVALID,
			"The name of group is empty.");

	snprintf(query, sizeof(query),
			"INSERT INTO %s(addrbook_id, group_name, ringtone) "
			"VALUES(%d, ?, ?)",
			CTS_TABLE_GROUPS, addressbook_id);

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		ERR("cts_query_prepare() Failed");
		contacts_svc_end_trans(false);
		return CTS_ERR_DB_FAILED;
	}

	cts_stmt_bind_text(stmt, 1, record->name);

	if (record->ringtone_path)
		cts_stmt_bind_text(stmt, 2, record->ringtone_path);

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		contacts_svc_end_trans(false);
		return ret;
	}
	index = cts_db_get_last_insert_id();
	cts_stmt_finalize(stmt);

	cts_set_group_noti();
	ret = contacts_svc_end_trans(true);
	retvm_if(ret < CTS_SUCCESS, ret,
			"contacts_svc_end_trans(true) Failed(%d)", ret);

	return index;
}

API int contacts_svc_delete_group_with_members(int index)
{
	int ret;
	char  query[CTS_SQL_MAX_LEN] = {0};

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE contact_id "
			"IN (SELECT contact_id FROM %s A WHERE group_id = %d AND "
			"(SELECT COUNT(*) FROM %s B WHERE A.contact_id = B.contact_id) = 1)",
			CTS_TABLE_CONTACTS, CTS_TABLE_GROUPING_INFO, index, CTS_TABLE_GROUPING_INFO);
	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE group_id = %d",
			CTS_TABLE_GROUPS, index);
	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	ret = cts_db_change();
	if (0 < ret) {
		cts_set_contact_noti();
		cts_set_group_noti();
		ret = contacts_svc_end_trans(true);
	} else {
		contacts_svc_end_trans(false);
		ret = CTS_ERR_NO_DATA;
	}

	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

API int contacts_svc_delete_group(int index)
{
	int ret;
	char  query[CTS_SQL_MAX_LEN] = {0};

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE group_id=%d",
			CTS_TABLE_GROUPS, index);

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	ret = cts_db_change();
	if (0 < ret) {
		cts_set_group_noti();
		ret = contacts_svc_end_trans(true);
	} else {
		contacts_svc_end_trans(false);
		ret = CTS_ERR_NO_DATA;
	}

	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

int cts_group_set_relation(int group_id, int contact_id, int contact_acc)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN];

	snprintf(query, sizeof(query),
			"SELECT addrbook_id FROM %s WHERE group_id = %d",
			CTS_TABLE_GROUPS, group_id);
	int grp_acc = cts_query_get_first_int_result(query);
	retvm_if(CTS_ERR_DB_RECORD_NOT_FOUND == grp_acc, CTS_ERR_ARG_INVALID,
			"group_id(%d) is Invalid", group_id);

	retvm_if(contact_acc != grp_acc, CTS_ERR_ARG_INVALID,
			"addrbook_id(%d) of the contact and addrbook_id(%d) of the group is not same",
			contact_acc, grp_acc);

	snprintf(query, sizeof(query), "INSERT OR IGNORE INTO %s VALUES(%d, %d)",
			CTS_TABLE_GROUPING_INFO, group_id, contact_id);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	warn_if(CTS_SUCCESS != ret, "cts_stmt_step() Failed(%d)", ret);

	cts_stmt_finalize(stmt);

	return ret;
}

API int contacts_svc_group_set_relation(int group_id, int contact_id)
{
	int ret, ct_acc=0;
	char query[CTS_SQL_MIN_LEN];

	snprintf(query, sizeof(query),
			"SELECT addrbook_id FROM %s WHERE contact_id = %d LIMIT 1",
			CTS_TABLE_CONTACTS, contact_id);
	ct_acc = cts_query_get_first_int_result(query);
	retvm_if(CTS_ERR_DB_RECORD_NOT_FOUND == ct_acc, CTS_ERR_ARG_INVALID,
			"contact_id(%d) is Invalid", contact_id);

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	ret = cts_group_set_relation(group_id, contact_id, ct_acc);
	if (ret) {
		contacts_svc_end_trans(false);
		ERR("cts_group_set_relation() Failed(%d)", ret);
		return ret;
	}

	ret = cts_update_contact_changed_time(contact_id);
	if (CTS_SUCCESS != ret) {
		ERR("cts_update_contact_changed_time() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	cts_set_contact_noti();
	cts_set_group_rel_noti();
	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

int cts_group_unset_relation(int group_id, int contact_id)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN];

	snprintf(query, sizeof(query),
			"DELETE FROM %s WHERE group_id = %d AND contact_id = %d",
			CTS_TABLE_GROUPING_INFO, group_id, contact_id);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	warn_if(CTS_SUCCESS != ret, "cts_stmt_step() Failed(%d)", ret);

	cts_stmt_finalize(stmt);

	return ret;
}

API int contacts_svc_group_unset_relation(int group_id, int contact_id)
{
	int ret;

	retvm_if(!group_id, CTS_ERR_ARG_INVALID, "group_id is 0");
	retvm_if(!contact_id, CTS_ERR_ARG_INVALID, "contact_id is 0");

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	ret = cts_group_unset_relation(group_id, contact_id);
	if (ret) {
		contacts_svc_end_trans(false);
		ERR("cts_group_unset_relation() Failed(%d)", ret);
		return ret;
	}

	ret = cts_update_contact_changed_time(contact_id);
	if (CTS_SUCCESS != ret) {
		ERR("cts_update_contact_changed_time() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	cts_set_contact_noti();
	cts_set_group_rel_noti();
	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

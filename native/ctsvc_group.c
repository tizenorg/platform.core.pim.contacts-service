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
#include "ctsvc_db_plugin_contact_helper.h"
#include "ctsvc_group.h"
#include "ctsvc_notification.h"

int ctsvc_group_add_contact_in_transaction(int group_id, int contact_id)
{
	int ret;
	int version;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};
	int rel_changed = 0;
	int grp_acc, contact_acc;
	int exist;

	snprintf(query, sizeof(query),
			"SELECT COUNT(*) FROM %s WHERE group_id = %d AND contact_id=%d AND deleted = 0",
			CTS_TABLE_GROUP_RELATIONS, group_id, contact_id);

	ret = ctsvc_query_get_first_int_result(query, &exist);
	if (1 == exist)	{
		CTS_DBG("group relation already exist (group_id:%d, contac_id:%d)", group_id, contact_id);
		return CONTACTS_ERROR_NONE;
	}

	version = ctsvc_get_next_ver();
	snprintf(query, sizeof(query),
			"SELECT addressbook_id FROM %s WHERE group_id = %d",
			CTS_TABLE_GROUPS, group_id);

	ret = ctsvc_query_get_first_int_result(query, &grp_acc);
	RETVM_IF(CONTACTS_ERROR_NO_DATA == grp_acc, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid Parameter: group_id(%d) is Invalid", group_id);

	snprintf(query, sizeof(query),
			"SELECT addressbook_id FROM %s WHERE contact_id = %d AND deleted = 0",
			CTS_TABLE_CONTACTS, contact_id);

	ret = ctsvc_query_get_first_int_result(query, &contact_acc);
	RETVM_IF( contact_acc != grp_acc, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid Parameter: group_acc(%d) is differ from contact_acc(%d) Invalid", grp_acc, contact_acc);

	snprintf(query, sizeof(query), "INSERT OR REPLACE INTO %s VALUES(%d, %d, %d, 0)",
			CTS_TABLE_GROUP_RELATIONS, group_id, contact_id, version);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "cts_stmt_step() Failed(%d)", ret);

	rel_changed = cts_db_change();
	cts_stmt_finalize(stmt);

	if (0 < rel_changed) {
		snprintf(query, sizeof(query),
				"UPDATE "CTS_TABLE_GROUPS" SET member_changed_ver=%d WHERE group_id=%d",
				version, group_id);
		ret = ctsvc_query_exec(query);
		RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_set_group_rel_noti();
		return rel_changed;
	}

	return ret;
}

API int contacts_group_add_contact(int group_id, int contact_id)
{
	RETVM_IF( group_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid Parameter: group_id should be greater than 0");
	RETVM_IF( contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid Parameter: contact_id should be greater than 0");

	/* BEGIN_TRANSACTION */
	int ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	/* DOING JOB */
	do {
		int changed = ctsvc_group_add_contact_in_transaction(group_id, contact_id);
		if (changed < CONTACTS_ERROR_NONE) {
			CTS_ERR("DB error : ctsvc_group_add_contact_in_transaction() Failed(%d)", changed);
			ret = changed;
			break;
		}

		ret = ctsvc_db_contact_update_changed_time(contact_id);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : ctsvc_db_contact_update_changed_time() Failed(%d)", ret);
			ret = CONTACTS_ERROR_DB;
			break;
		}
		ctsvc_set_person_noti();

		ret = ctsvc_end_trans(true);
		if(ret < CONTACTS_ERROR_NONE )
		{
			CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
			return ret;
		}

		return CONTACTS_ERROR_NONE;

	} while(0);

	/* ROLLBACK TRANSACTION */
	ctsvc_end_trans(false);

	return ret;
}

int ctsvc_group_remove_contact_in_transaction(int group_id, int contact_id)
{
	int ret;
	int version;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	version = ctsvc_get_next_ver();
	snprintf(query, sizeof(query),
			"UPDATE %s SET deleted=1, ver = %d WHERE group_id = %d AND contact_id = %d",
			CTS_TABLE_GROUP_RELATIONS, version, group_id, contact_id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB Error: cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "DB Error: cts_stmt_step() Failed(%d)", ret);

	int rel_changed = cts_db_change();
	cts_stmt_finalize(stmt);

	if (0 <= rel_changed) {
		snprintf(query, sizeof(query),
				"UPDATE "CTS_TABLE_GROUPS" SET member_changed_ver=%d WHERE group_id=%d",
				version, group_id);
		ret = ctsvc_query_exec(query);
		RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_set_group_rel_noti();
		return rel_changed;
	}

	return CONTACTS_ERROR_NONE;
}

API int contacts_group_remove_contact(int group_id, int contact_id)
{
	RETVM_IF( group_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid Parameter: group_id should be greater than 0");
	RETVM_IF( contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid Parameter: contact_id should be greater than 0");

	/* BEGIN_TRANSACTION */
	int ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	/* DOING JOB */
	do {
		int changed = ctsvc_group_remove_contact_in_transaction(group_id, contact_id);
		if (changed < CONTACTS_ERROR_NONE) {
			CTS_ERR("DB error : ctsvc_group_remove_contact_in_transaction() Failed(%d)", changed);
			ret = changed;
			break;
		}

		ret = ctsvc_db_contact_update_changed_time(contact_id);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : ctsvc_db_contact_update_changed_time() Failed(%d)", ret);
			ret = CONTACTS_ERROR_DB;
			break;
		}
		ctsvc_set_person_noti();

		ret = ctsvc_end_trans(true);
		if(ret < CONTACTS_ERROR_NONE )
		{
			CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
			return ret;
		}

		return CONTACTS_ERROR_NONE;

	} while(0);

	/* ROLLBACK TRANSACTION */
	ctsvc_end_trans(false);

	return ret;
}


API int contacts_group_set_group_order(int group_id, int previous_group_id, int next_group_id)
{
	int ret;
	double previous_prio = 0.0;
	double next_prio = 0.0;
	int previous_addressbook_id = -1, next_addressbook_id = -1, addressbook_id = -1;
	double prio;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query), "SELECT group_prio, addressbook_id FROM "CTS_TABLE_GROUPS" WHERE group_id = ?");

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "cts_query_prepare() Failed");

	cts_stmt_bind_int(stmt, 1, previous_group_id);
	ret = cts_stmt_step(stmt);
	if (1 /*CTS_TRUE*/  == ret) {
		previous_prio = ctsvc_stmt_get_dbl(stmt, 0);
		previous_addressbook_id = ctsvc_stmt_get_int(stmt, 1);
	}
	cts_stmt_reset(stmt);
	cts_stmt_bind_int(stmt, 1, next_group_id);
	ret = cts_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ == ret) {
		next_prio = ctsvc_stmt_get_dbl(stmt, 0);
		next_addressbook_id = ctsvc_stmt_get_int(stmt, 1);
	}
	cts_stmt_reset(stmt);
	cts_stmt_bind_int(stmt, 1, group_id);
	ret = cts_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ == ret) {
		addressbook_id = ctsvc_stmt_get_int(stmt, 1);
	}
	cts_stmt_finalize(stmt);

	RETVM_IF(0.0 == previous_prio && 0.0 == next_prio, CONTACTS_ERROR_INVALID_PARAMETER,
			"The indexes for previous and next are invalid.");
	RETVM_IF(previous_group_id && previous_addressbook_id != addressbook_id , CONTACTS_ERROR_INVALID_PARAMETER,
				"previous group(%d) and group(%d) are not the same addressbook(%d, %d) groups",
				previous_group_id, group_id, previous_addressbook_id, addressbook_id);
	RETVM_IF(next_group_id && next_addressbook_id != addressbook_id , CONTACTS_ERROR_INVALID_PARAMETER,
				"next group(%d) and group(%d) are not the same addressbook(%d, %d) groups",
				next_group_id, group_id, next_addressbook_id, addressbook_id);

	if (0.0 == next_prio)
		prio = previous_prio + 1;
	else
		prio = (previous_prio + next_prio) / 2;

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"UPDATE %s SET group_prio = %f WHERE group_id = %d",
			CTS_TABLE_GROUPS, prio, group_id);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret)	{
		CTS_ERR("ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_group_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
	{
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

/*
API int contacts_group_add_person(int group_id, int person_id)
{
	return CONTACTS_ERROR_NONE;
}
API int contacts_group_remove_person(int group_id, int person_id)
{
	return CONTACTS_ERROR_NONE;
}
*/

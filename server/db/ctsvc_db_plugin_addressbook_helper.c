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
#include "ctsvc_db_sqlite.h"
#include "ctsvc_db_schema.h"
#include "ctsvc_db_utils.h"
#include "ctsvc_server_person.h"
#include "ctsvc_notification.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_db_plugin_addressbook_helper.h"

int ctsvc_addressbook_reset_internal_addressbook(void)
{
	CTS_FN_CALL;
	char query[CTS_SQL_MIN_LEN] = {0};
	int ret;
	int version;

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	ret = ctsvc_is_owner(0);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		if (CONTACTS_ERROR_PERMISSION_DENIED == ret)
			ERR("Does not have permission of address_book (0)");
		else
			ERR("ctsvc_is_owner Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	version = ctsvc_get_next_ver();
	snprintf(query, sizeof(query),
			"UPDATE %s SET deleted = 1, person_id = 0, changed_ver=%d WHERE addressbook_id = %d",
			CTS_TABLE_CONTACTS, version, 0 /*CTS_ADDRESSBOOK_INTERNAL*/);

	/* DOING JOB */
	do {
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_query_exec() Fail(%d)", ret);
			break;
			/* LCOV_EXCL_STOP */
		}

		snprintf(query, sizeof(query), "DELETE FROM %s WHERE addressbook_id = %d",
				CTS_TABLE_MY_PROFILES, 0);

		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_query_exec() Fail(%d)", ret);
			break;
			/* LCOV_EXCL_STOP */
		}

		snprintf(query, sizeof(query), "DELETE FROM %s WHERE addressbook_id = %d",
				CTS_TABLE_GROUPS, 0 /*CTS_ADDRESSBOOK_INTERNAL*/);
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_query_exec() Fail(%d)", ret);
			break;
			/* LCOV_EXCL_STOP */
		}

		snprintf(query, sizeof(query), "DELETE FROM %s WHERE addressbook_id = %d",
				CTS_TABLE_GROUP_DELETEDS, 0 /*CTS_ADDRESSBOOK_INTERNAL*/);
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_query_exec() Fail(%d)", ret);
			break;
			/* LCOV_EXCL_STOP */
		}

		snprintf(query, sizeof(query), "DELETE FROM %s WHERE addressbook_id = %d",
				CTS_TABLE_DELETEDS, 0 /*CTS_ADDRESSBOOK_INTERNAL*/);
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_query_exec() Fail(%d)", ret);
			break;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_person_do_garbage_collection();
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_person_garbagecollection() Fail(%d)", ret);
			break;
			/* LCOV_EXCL_STOP */
		}

		ctsvc_set_contact_noti();
		ctsvc_set_my_profile_noti();
		/* person noti will set in ctsvc_person_do_garbage_collection : ctsvc_set_person_noti(); */
		ctsvc_set_group_noti();
		ret = ctsvc_end_trans(true);
		if (ret < CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_end_trans() Fail(%d)", ret);
			return ret;
			/* LCOV_EXCL_STOP */
		}

		return CONTACTS_ERROR_NONE;
	} while (0);

	/* ROLLBACK TRANSACTION */
	ctsvc_end_trans(false);

	return ret;
}

int ctsvc_addressbook_delete(int account_id)
{
	CTS_FN_CALL;
	int ret;
	int addressbook_id = -1;
	char query[CTS_SQL_MAX_LEN] = {0};
	RETVM_IF(account_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "Account_id(%d) is invalid", account_id);

	/* delete addressbook whish has account_id */
	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query), "SELECT addressbook_id FROM %s WHERE account_id = %d",
			CTS_TABLE_ADDRESSBOOKS, account_id);
	ret = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_get_first_int_result() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (addressbook_id == 0) {
		ret = ctsvc_addressbook_reset_internal_addressbook();
		if (ret == CONTACTS_ERROR_NONE)
			ret = ctsvc_end_trans(true);
		else
			ctsvc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE account_id = %d",
			CTS_TABLE_ADDRESSBOOKS, account_id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_change();
	if (0 < ret) {
		ctsvc_set_my_profile_noti();
		ctsvc_set_contact_noti();
		/* person noti will set in ctsvc_person_do_garbage_collection : ctsvc_set_person_noti(); */
		ctsvc_set_group_noti();
		ctsvc_set_addressbook_noti();
	} else {
		/* LCOV_EXCL_START */
		ERR("There is no addressbook which has account_id (%d)", account_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_NO_DATA;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_person_do_garbage_collection();
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_person_garbagecollection() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	return ctsvc_end_trans(true);
}


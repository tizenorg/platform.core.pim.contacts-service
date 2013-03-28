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
#include "ctsvc_sqlite.h"
#include "ctsvc_schema.h"
#include "ctsvc_utils.h"
#include "ctsvc_person.h"
#include "ctsvc_notification.h"
#include "ctsvc_db_plugin_addressbook_helper.h"

int ctsvc_addressbook_delete(int account_id)
{
	CTS_FN_CALL;
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};
	RETVM_IF(account_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "Account_id(%d) is invalid", account_id);

	// delete addressbook whish has account_id
	ret = ctsvc_begin_trans();
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE account_id = %d",
			CTS_TABLE_ADDRESSBOOKS, account_id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = cts_db_change();
	if (0 < ret) {
		ctsvc_set_my_profile_noti();
		ctsvc_set_contact_noti();
		// person noti will set in ctsvc_person_do_garbage_collection : ctsvc_set_person_noti();
		ctsvc_set_group_noti();
		ctsvc_set_addressbook_noti();
	}
	else {
		CTS_ERR("There is no addressbook which has account_id (%d)", account_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_NO_DATA;
	}

	ret = ctsvc_person_do_garbage_collection();
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : cts_person_garbagecollection() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	return ctsvc_end_trans(true);
}


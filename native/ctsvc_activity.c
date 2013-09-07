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
#include "ctsvc_notification.h"
#include "ctsvc_db_access_control.h"

API int contacts_activity_delete_by_contact_id(int contact_id)
{
	char query[CTS_SQL_MAX_LEN] = {0};

	RETVM_IF(!ctsvc_have_permission(CTSVC_PERMISSION_CONTACT_WRITE), CONTACTS_ERROR_PERMISSION_DENIED,
				"Permission denied : contact write (contact activity)");
	RETV_IF(contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER);

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_ACTIVITIES" WHERE contact_id = %d", contact_id);

	int ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_activity_noti();
	ctsvc_set_person_noti();

	ret = ctsvc_end_trans(true);
	return ret;
}

API int contacts_activity_delete_by_account_id(int account_id)
{
	char query[CTS_SQL_MAX_LEN] = {0};

	RETVM_IF(!ctsvc_have_permission(CTSVC_PERMISSION_CONTACT_WRITE), CONTACTS_ERROR_PERMISSION_DENIED,
				"Permission denied : contact write (contact activity)");
	RETV_IF(account_id < 0, CONTACTS_ERROR_INVALID_PARAMETER);

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE contact_id IN "
			"(SELECT C.contact_id FROM %s C, %s A ON C.addressbook_id = A.addressbook_id "
			"WHERE A.account_id = %d)",
			CTS_TABLE_ACTIVITIES, CTS_TABLE_CONTACTS, CTS_TABLE_ADDRESSBOOKS, account_id);

	int ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_activity_noti();
	ctsvc_set_person_noti();

	ret = ctsvc_end_trans(true);
	return ret;
}



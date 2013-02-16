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
#include "ctsvc_notification.h"

#ifdef _CONTACTS_IPC_SERVER
#include "ctsvc_server_change_subject.h"
#endif

API int contacts_phone_log_reset_statistics()
{
	char query[CTS_SQL_MIN_LEN] = {0};
	snprintf(query, sizeof(query),"DELETE FROM "CTS_TABLE_PHONELOG_STAT);
	return ctsvc_query_exec(query);
}

API int contacts_phone_log_delete(contacts_phone_log_delete_e op, ...)
{
	int ret;
	int extra_data1;
	char query[CTS_SQL_MAX_LEN] = {0};
	char *number = NULL;
	va_list args;

	switch(op) {
	case CONTACTS_PHONE_LOG_DELETE_BY_ADDRESS:
		va_start(args, op);
		number = va_arg(args, char *);
		va_end(args);
		RETV_IF(NULL == number, CONTACTS_ERROR_INVALID_PARAMETER);
		snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_PHONELOGS" WHERE number = '%s'", number);
		break;
	case CONTACTS_PHONE_LOG_DELETE_BY_MESSAGE_EXTRA_DATA1:
		va_start(args, op);
		extra_data1 = va_arg(args, int);
		va_end(args);
		snprintf(query, sizeof(query),
				"DELETE FROM "CTS_TABLE_PHONELOGS" "
				"WHERE data1 = %d AND %d <= log_type AND log_type <= %d",
						extra_data1, CONTACTS_PLOG_TYPE_MMS_INCOMMING, CONTACTS_PLOG_TYPE_MMS_BLOCKED);
		break;
	case CONTACTS_PHONE_LOG_DELETE_BY_EMAIL_EXTRA_DATA1:
		va_start(args, op);
		extra_data1 = va_arg(args, int);
		va_end(args);
		snprintf(query, sizeof(query),
				"DELETE FROM "CTS_TABLE_PHONELOGS" "
				"WHERE data1 = %d AND %d <= log_type AND log_type <= %d",
						extra_data1, CONTACTS_PLOG_TYPE_EMAIL_RECEIVED, CONTACTS_PLOG_TYPE_EMAIL_SENT);
		break;
	default:
		CTS_ERR("Invalid parameter : the operation is not proper (op : %d)", op);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}
	ctsvc_set_phonelog_noti();
	ret = ctsvc_end_trans(true);
	return ret;
}

void ctsvc_db_phone_log_delete_callback(sqlite3_context * context,
		int argc, sqlite3_value ** argv)
{
#ifdef _CONTACTS_IPC_SERVER
	int phone_log_id;

	if (argc < 1) {
		sqlite3_result_null(context);
		return;
	}

	phone_log_id = sqlite3_value_int(argv[0]);
	ctsvc_change_subject_add_changed_phone_log_id(CONTACTS_CHANGE_DELETED, phone_log_id);

	sqlite3_result_null(context);
	return;
#endif
}

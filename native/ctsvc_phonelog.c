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
#include "contacts_phone_log_internal.h"

#include "ctsvc_internal.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_utils.h"
#include "ctsvc_notification.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_number_utils.h"
#include "ctsvc_localize_utils.h"
#include "ctsvc_setting.h"

#ifdef _CONTACTS_IPC_SERVER
#include "ctsvc_server_change_subject.h"
#ifdef ENABLE_SIM_FEATURE
#include "ctsvc_server_sim.h"
#endif // ENABLE_SIM_FEATURE
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

static int __ctsvc_db_phone_log_find_person_id(char *number, char *normal_num, char *minmatch, int person_id, int *find_number_type)
{
	int ret;
	int find_person_id = -1;
	char query[CTS_SQL_MAX_LEN] = {0};
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	int i = 0;

	*find_number_type = -1;
	if (normal_num) {
		ret = snprintf(query, sizeof(query),
				"SELECT person_id, data1 FROM "CTS_TABLE_CONTACTS", "CTS_TABLE_DATA" "
					"ON "CTS_TABLE_CONTACTS".contact_id = "CTS_TABLE_DATA".contact_id "
						"AND datatype = %d AND is_my_profile = 0 AND deleted = 0 "
						"WHERE data4 = ? AND _NUMBER_COMPARE_(data5, ?, NULL, NULL)",
					CTSVC_DATA_NUMBER);
		bind_text = g_slist_append(bind_text, strdup(minmatch));
		bind_text = g_slist_append(bind_text, strdup(normal_num));
	}

	if (*query) {
		// several person can have same number
		cts_stmt stmt = NULL;
		int id;
		int number_type = -1;

		ret = ctsvc_query_prepare(query, &stmt);
		if (stmt == NULL) {
			CTS_ERR("ctsvc_query_prepare fail(%d)", ret);
			if (bind_text) {
				for (cursor=bind_text;cursor;cursor=cursor->next)
					free(cursor->data);
				g_slist_free(bind_text);
			}
			return CONTACTS_ERROR_DB;
		}

		if (bind_text) {
			for (cursor=bind_text,i=1;cursor;cursor=cursor->next,i++) {
				const char *text = cursor->data;
				if (text && *text)
					ctsvc_stmt_bind_text(stmt, i, text);
			}
		}

		while ((ret = ctsvc_stmt_step(stmt))) {
			id = ctsvc_stmt_get_int(stmt, 0);
			number_type = ctsvc_stmt_get_int(stmt, 1);
			if (find_person_id <= 0 && id > 0) {
				find_person_id = id;		// find first match person_id
				*find_number_type = number_type;
				if (person_id <= 0)
					break;
			}

			if (id == person_id) {
				find_person_id = person_id;
				*find_number_type = number_type;
				break;
			}
		}
		ctsvc_stmt_finalize(stmt);
	}

	if (bind_text) {
		for (cursor=bind_text;cursor;cursor=cursor->next)
			free(cursor->data);
		g_slist_free(bind_text);
	}

	return find_person_id;
}

int ctsvc_db_phone_log_update_person_id(const char *number, int old_person_id, int candidate_person_id, bool person_link)
{
	CTS_FN_CALL;
	int ret;
	int len = 0;
	char query[CTS_SQL_MAX_LEN] = {0};
	cts_stmt get_log = NULL;
	cts_stmt update_log = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	int i = 0;

	RETVM_IF(old_person_id <= 0 && NULL == number, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : old person_id (%d), numberis NULL", old_person_id);

	len = snprintf(query, sizeof(query),
					"SELECT id, number, normal_num, minmatch FROM "CTS_TABLE_PHONELOGS" "
										"WHERE log_type <= %d ",
					CONTACTS_PLOG_TYPE_EMAIL_RECEIVED);

	if (number && *number) {
		char clean_num[strlen(number)+1];
		len += snprintf(query+len, sizeof(query)-len, "AND ((number = ? ) ");
		bind_text = g_slist_append(bind_text, strdup(number));

		ret = ctsvc_clean_number(number, clean_num, sizeof(clean_num), true);
		if (0 < ret) {
			char normal_num[sizeof(clean_num) + 20];
			ret = ctsvc_normalize_number(clean_num, normal_num, sizeof(normal_num), true);
			if (0 < ret) {
				char minmatch[sizeof(normal_num)+1];
				ret = ctsvc_get_minmatch_number(normal_num, minmatch, sizeof(minmatch), ctsvc_get_phonenumber_min_match_digit());
				if (CONTACTS_ERROR_NONE == ret) {
					len += snprintf(query+len, sizeof(query)-len,
						"OR (minmatch = ? AND _NUMBER_COMPARE_(normal_num, ?, NULL, NULL))) ");
					bind_text = g_slist_append(bind_text, strdup(minmatch));
					bind_text = g_slist_append(bind_text, strdup(normal_num));
				}
				else
					len += snprintf(query+len, sizeof(query)-len, ") ");
			}
			else
				len += snprintf(query+len, sizeof(query)-len, ") ");
		}
		else
			len += snprintf(query+len, sizeof(query)-len, ") ");
	}

	if (old_person_id > 0)
		len += snprintf(query+len, sizeof(query)-len, "AND person_id = %d ", old_person_id);
	else
		len += snprintf(query+len, sizeof(query)-len, "AND person_id IS NULL ");

	ret = ctsvc_query_prepare(query, &get_log);
	if (get_log == NULL) {
		CTS_ERR("ctsvc_query_prepare fail(%d)", ret);
		if (bind_text) {
			for (cursor=bind_text;cursor;cursor=cursor->next)
				free(cursor->data);
			g_slist_free(bind_text);
		}
		return CONTACTS_ERROR_DB;
	}

	if (bind_text) {
		for (cursor=bind_text,i=1;cursor;cursor=cursor->next,i++) {
			const char *text = cursor->data;
			if (text && *text)
				ctsvc_stmt_bind_text(get_log, i, text);
		}
	}

	snprintf(query, sizeof(query), "UPDATE "CTS_TABLE_PHONELOGS" SET person_id=?, number_type = ? WHERE id = ?");
	ret = ctsvc_query_prepare(query, &update_log);
	if (update_log == NULL) {
		CTS_ERR("ctsvc_query_prepare fail(%d)", ret);
		ctsvc_stmt_finalize(get_log);

		if (bind_text) {
			for (cursor=bind_text;cursor;cursor=cursor->next)
				free(cursor->data);
			g_slist_free(bind_text);
		}
		return CONTACTS_ERROR_DB;
	}

	while ((ret = ctsvc_stmt_step(get_log))) {
		int phonelog_id;
		int new_person_id = -1;
		int temp_id;
		int number_type= -1;
		char *address;
		char *normal_address;
		char *minmatch_address;

		phonelog_id = ctsvc_stmt_get_int(get_log, 0);
		address = ctsvc_stmt_get_text(get_log, 1);
		normal_address = ctsvc_stmt_get_text(get_log, 2);
		minmatch_address = ctsvc_stmt_get_text(get_log, 3);

		//CASE : number is inserted (contact insert/update) => update person_id of phone logs from NULL
		if (number && old_person_id <= 0 && candidate_person_id > 0) {
			__ctsvc_db_phone_log_find_person_id(address, normal_address, minmatch_address, candidate_person_id, &number_type);
			new_person_id = candidate_person_id;
		}
		//CASE : phonelog insert without person_id
		else if (number && old_person_id <= 0) {
			// address == number
			new_person_id = __ctsvc_db_phone_log_find_person_id(address, normal_address, minmatch_address, -1, &number_type);
			if (new_person_id <= 0) continue;
		}
		// CASE : number update/delete (contact update/delete) => find new_person_id by address
		// CASE : phonelog insert with person_id
		else if (number && old_person_id > 0) {
			// address == number
			// although new_person_id and old_person_id are same, update phonelog for setting number_type
			new_person_id = __ctsvc_db_phone_log_find_person_id(address, normal_address, minmatch_address, old_person_id, &number_type);
		}
		// CASE : person link => deleted person_id -> new person_id (base_person_id)
		else if (NULL == number && old_person_id  > 0 && candidate_person_id > 0 && person_link) {
			new_person_id = candidate_person_id;
		}
		// CASE : person unlink => check person_id of the address,
		// if person_id is not old_person_id then change person_id to new_person_id
		else if (NULL == number && old_person_id  > 0 && candidate_person_id > 0) {
			temp_id = __ctsvc_db_phone_log_find_person_id(address, normal_address, minmatch_address, candidate_person_id, &number_type);
			if (temp_id > 0 && temp_id == old_person_id)
				continue;
			else if (temp_id > 0 && temp_id != old_person_id)
				new_person_id = temp_id;
		}
		// CASE : person delete => find new_person_id by address
		else if (NULL == number && old_person_id  > 0) {
			new_person_id = __ctsvc_db_phone_log_find_person_id(address, normal_address, minmatch_address, candidate_person_id, &number_type);
		}
		// Already check this case as above : RETVM_IF(old_person_id <= 0 && NULL == number, ...
//		else
//			continue;

		if (new_person_id > 0)
			ctsvc_stmt_bind_int(update_log, 1, new_person_id);
		if (number_type >= 0)
			ctsvc_stmt_bind_int(update_log, 2, number_type);
		ctsvc_stmt_bind_int(update_log, 3, phonelog_id);
		ctsvc_stmt_step(update_log);
		ctsvc_stmt_reset(update_log);
	}
	ctsvc_stmt_finalize(get_log);
	ctsvc_stmt_finalize(update_log);

	if (bind_text) {
		for (cursor=bind_text;cursor;cursor=cursor->next)
			free(cursor->data);
		g_slist_free(bind_text);
	}

	return CONTACTS_ERROR_NONE;
}


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
#include "cts-contact.h"
#include "cts-utils.h"
#include "cts-types.h"
#include "cts-normalize.h"
#include "cts-phonelog.h"

#define CTS_NAME_LEN_MAX 128

static int cts_phonelog_accumulation_handle(cts_plog *plog)
{
	int ret, cnt, duration, total_cnt, total_duration, deal_cnt=0;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query), "SELECT * FROM %s WHERE id <= 2",
			CTS_TABLE_PHONELOG_ACC);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	while (CTS_TRUE == cts_stmt_step(stmt))
	{
		ret = cts_stmt_get_int(stmt, 0);
		if (1 == ret) {
			cnt = cts_stmt_get_int(stmt, 1);
			duration = cts_stmt_get_int(stmt, 3);
			deal_cnt++;
		}else if (2 == ret) {
			total_cnt = cts_stmt_get_int(stmt, 1);
			total_duration = cts_stmt_get_int(stmt, 3);
			deal_cnt++;
		}
	}
	cts_stmt_finalize(stmt);

	if (deal_cnt != 2) {
		ERR("Getting plog accumulation data is Failed");
		return CTS_ERR_DB_FAILED;
	}

	snprintf(query, sizeof(query), "INSERT OR REPLACE INTO %s VALUES(?, ?, NULL, ?)",
			CTS_TABLE_PHONELOG_ACC);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	cts_stmt_bind_int(stmt, 1, 1);
	cts_stmt_bind_int(stmt, 2, cnt+1);
	cts_stmt_bind_int(stmt, 3, duration + plog->extra_data1);

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}
	cts_stmt_reset(stmt);

	cts_stmt_bind_int(stmt, 1, 2);
	cts_stmt_bind_int(stmt, 2, total_cnt+1);
	cts_stmt_bind_int(stmt, 3, total_duration + plog->extra_data1);

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}

	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}

static inline int cts_insert_phonelog(cts_plog *plog)
{
	int ret;
	cts_stmt stmt = NULL;
	char clean_num[CTS_NUMBER_MAX_LEN], query[CTS_SQL_MAX_LEN] = {0};
	const char *normal_num;

	retvm_if(plog->log_type <= CTS_PLOG_TYPE_NONE
			|| CTS_PLOG_TYPE_MAX <= plog->log_type,
			CTS_ERR_ARG_INVALID, "phonelog type(%d) is invaid", plog->log_type);

	cts_clean_number(plog->number, clean_num, sizeof(clean_num));

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query), "INSERT INTO %s("
			"number, normal_num, related_id, log_type, log_time, data1, data2) "
			"VALUES(?, ?, ?, %d, %d, %d, ?)",
			CTS_TABLE_PHONELOGS, plog->log_type,
			plog->log_time, plog->extra_data1);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		ERR("cts_query_prepare() Failed");
		contacts_svc_end_trans(false);
		return CTS_ERR_DB_FAILED;
	}

	if (*clean_num) {
		cts_stmt_bind_text(stmt, 1, clean_num);
		normal_num = cts_normalize_number(clean_num);
		cts_stmt_bind_text(stmt, 2, normal_num);
	}

	if (0 < plog->related_id)
		cts_stmt_bind_int(stmt, 3, plog->related_id);

	if (plog->extra_data2)
		cts_stmt_bind_text(stmt, 4, plog->extra_data2);

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		contacts_svc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	if (CTS_PLOG_TYPE_VOICE_OUTGOING == plog->log_type
			|| CTS_PLOG_TYPE_VIDEO_OUTGOING == plog->log_type)
	{
		ret = cts_phonelog_accumulation_handle(plog);
		if (CTS_SUCCESS != ret) {
			ERR("cts_phonelog_accumulation_handle() Failed");
			contacts_svc_end_trans(false);
			return ret;
		}
	}

	if (CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN == plog->log_type ||
			CTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN == plog->log_type)
		cts_set_missed_call_noti();

	cts_set_plog_noti();
	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

//extra_data1 : duration, message_id
//extra_data2 : short message
API int contacts_svc_insert_phonelog(CTSvalue* phone_log)
{
	int ret;
	cts_plog *plog = (cts_plog *)phone_log;

	retv_if(NULL == phone_log, CTS_ERR_ARG_NULL);
	retvm_if(plog->id, CTS_ERR_ARG_INVALID, "The phone_log has ID(%d)", plog->id);

	ret = cts_insert_phonelog(plog);
	retvm_if(CTS_SUCCESS != ret, ret,"cts_insert_phonelog() Failed(%d)", ret);

	if (0 < plog->related_id) {
		ret = cts_increase_outgoing_count(plog->related_id);
		warn_if(CTS_SUCCESS != ret, "cts_increase_outgoing_count() Failed(%d)", ret);
	}

	return CTS_SUCCESS;
}

API int contacts_svc_delete_phonelog(cts_del_plog_op op_code, ...)
{
	int id, ret;
	char *number;
	char query[CTS_SQL_MAX_LEN];
	va_list args;

	switch (op_code)
	{
	case CTS_PLOG_DEL_BY_ID:
		va_start(args, op_code);
		id = va_arg(args, int);
		va_end(args);
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE id = %d",
				CTS_TABLE_PHONELOGS, id);
		break;
	case CTS_PLOG_DEL_BY_NUMBER:
		va_start(args, op_code);
		number = va_arg(args, char *);
		va_end(args);
		retv_if(NULL == number, CTS_ERR_ARG_NULL);
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE number = '%s'",
				CTS_TABLE_PHONELOGS, number);
		break;
	case CTS_PLOG_DEL_BY_MSGID:
		va_start(args, op_code);
		id = va_arg(args, int);
		va_end(args);
		snprintf(query, sizeof(query), "DELETE FROM %s "
				"WHERE data1 = %d AND %d <= log_type AND log_type <= %d",
				CTS_TABLE_PHONELOGS,
				id, CTS_PLOG_TYPE_MMS_INCOMMING, CTS_PLOG_TYPE_MMS_BLOCKED);
		break;
	case CTS_PLOG_DEL_NO_NUMBER:
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE number ISNULL",
				CTS_TABLE_PHONELOGS);
		break;
	default:
		ERR("Invalid op_code. Your op_code(%d) is not supported.", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}
	cts_set_plog_noti();
	if (CTS_PLOG_DEL_BY_MSGID != op_code)
		cts_set_missed_call_noti();

	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

API int contacts_svc_get_phonelog(int plog_id, CTSvalue **phonelog)
{
	int ret;
	cts_stmt stmt;
	cts_plog *plog;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT id, number, related_id, log_type, log_time, data1, data2 "
			"FROM %s WHERE id = %d",
			CTS_TABLE_PHONELOGS, plog_id);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	plog = (cts_plog *)contacts_svc_value_new(CTS_VALUE_PHONELOG);
	if (plog) {
		ret = CTS_SUCCESS;
		plog->v_type = CTS_VALUE_RDONLY_PLOG;
		plog->id = cts_stmt_get_int(stmt, 0);
		plog->number = SAFE_STRDUP(cts_stmt_get_text(stmt, 1));
		plog->related_id = cts_stmt_get_int(stmt, 2);
		plog->log_type = cts_stmt_get_int(stmt, 3);
		plog->log_time = cts_stmt_get_int(stmt, 4);
		plog->extra_data1 = cts_stmt_get_int(stmt, 5);
		plog->extra_data2 = SAFE_STRDUP(cts_stmt_get_text(stmt, 6));

		*phonelog = (CTSvalue*)plog;

		cts_stmt_finalize(stmt);
		return CTS_SUCCESS;
	}
	else {
		ERR("contacts_svc_value_new() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_OUT_OF_MEMORY;
	}
}


API int contacts_svc_phonelog_set_seen(int index, int type)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};

	retvm_if(CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN != type &&
			CTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN != type &&
			CTS_PLOG_TYPE_NONE != type,
			CTS_ERR_ARG_INVALID,
			"The type is invalid. It must be CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN"
			" or CTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN");

	if (0 == index) {
		if (CTS_PLOG_TYPE_NONE == type)
			snprintf(query, sizeof(query), "UPDATE %s SET log_type = log_type + 1 WHERE log_type = %d OR log_type = %d",
					CTS_TABLE_PHONELOGS, CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN,
					CTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN);
		else
			snprintf(query, sizeof(query), "UPDATE %s SET log_type = %d WHERE log_type = %d",
					CTS_TABLE_PHONELOGS, type+1, type);
	}
	else {
		snprintf(query, sizeof(query), "UPDATE %s SET log_type = %d WHERE id = %d",
				CTS_TABLE_PHONELOGS, type+1, index);
	}

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	if (cts_db_change()) {
		cts_set_plog_noti();
		cts_set_missed_call_noti();
	}

	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

/**
 * This is the signature of a callback function added with contacts_svc_phonelog_get_all_number(),
 * \n This function is invoked in the above functions.
 * \n If this function doesn't return #CTS_SUCCESS, foreach function is terminated.
 *
 * @param[in] number number.
 * @param[in] user_data The data which is set by contacts_svc_phonelog_get_all_number(),
 * @return #CTS_SUCCESS on success, other value on error
 */
typedef int (*cts_plog_foreach_fn)(const char *number, void *user_data);

/**
 * This function calls #cts_plog_foreach_fn for each number of all number list.
 * The all number list doesn't have duplicated numbers.
 *
 * @param[in] cb callback function pointer(#cts_plog_foreach_fn)
 * @param[in] user_data data which is passed to callback function
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
API int contacts_svc_phonelog_get_all_number(cts_plog_foreach_fn cb,
		void *user_data)
{
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	retv_if(NULL == cb, CTS_ERR_ARG_NULL);

	snprintf(query, sizeof(query),
			"SELECT DISTINCT number FROM %s", CTS_TABLE_PHONELOGS);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	while (CTS_TRUE == cts_stmt_step(stmt)) {
		if (cb(cts_stmt_get_text(stmt, 0), user_data))
			break;
	}
	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}

API char* contacts_svc_phonelog_get_last_number(cts_plog_get_last_op op)
{
	int ret;
	cts_stmt stmt;
	char *number;
	char query[CTS_SQL_MAX_LEN] = {0};

	switch (op) {
	case CTS_PLOG_LAST_ALL:
		snprintf(query, sizeof(query),
				"SELECT id, number FROM %s "
				"WHERE id = (SELECT MAX(id) FROM %s WHERE log_type = %d OR log_type = %d)",
				CTS_TABLE_PHONELOGS, CTS_TABLE_PHONELOGS,
				CTS_PLOG_TYPE_VOICE_OUTGOING, CTS_PLOG_TYPE_VIDEO_OUTGOING);
		break;
	case CTS_PLOG_LAST_CALL_ONLY:
		snprintf(query, sizeof(query),
				"SELECT id, number FROM %s "
				"WHERE id = (SELECT MAX(id) FROM %s WHERE log_type = %d)",
				CTS_TABLE_PHONELOGS, CTS_TABLE_PHONELOGS, CTS_PLOG_TYPE_VOICE_OUTGOING);
		break;
	case CTS_PLOG_LAST_VIDEO_CALL_ONLY:
		snprintf(query, sizeof(query),
				"SELECT id, number FROM %s "
				"WHERE id = (SELECT MAX(id) FROM %s WHERE log_type = %d)",
				CTS_TABLE_PHONELOGS, CTS_TABLE_PHONELOGS, CTS_PLOG_TYPE_VIDEO_OUTGOING);
		break;
	default:
		ERR("Invalid op(%d)", op);
		return NULL;
	}

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, NULL, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return NULL;
	}
	number = SAFE_STRDUP(cts_stmt_get_text(stmt, 1));

	cts_stmt_finalize(stmt);

	return number;
}

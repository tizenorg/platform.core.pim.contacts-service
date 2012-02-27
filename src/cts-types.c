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
#include "cts-sqlite.h"
#include "cts-schema.h"
#include "cts-utils.h"
#include "cts-types.h"

API char* contacts_svc_get_custom_type(cts_custom_type_class type_class,
		int index)
{
	int ret;
	char *ret_val = NULL;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	CTS_START_TIME_CHECK;

	if (CTS_TYPE_CLASS_EXTEND_DATA == type_class) index -= CTS_DATA_EXTEND_START;
	else if (CTS_TYPE_CLASS_NUM == type_class) index ^= CTS_NUM_TYPE_CUSTOM;

	snprintf(query, sizeof(query),
			"SELECT name FROM %s WHERE id = %d",
			CTS_TABLE_CUSTOM_TYPES, index);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, NULL, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE == ret)
		ret_val = cts_stmt_get_text(stmt, 0);
	cts_stmt_finalize(stmt);

	ret_val = SAFE_STRDUP(ret_val);

	CTS_END_TIME_CHECK();
	return ret_val;
}

API int contacts_svc_insert_custom_type(cts_custom_type_class type_class,
		char *type_name)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	retv_if(NULL == type_name, CTS_ERR_ARG_NULL);

	snprintf(query, sizeof(query),
			"INSERT INTO %s(class, name) VALUES(%d, ?)",
			CTS_TABLE_CUSTOM_TYPES, type_class);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	cts_stmt_bind_text(stmt, 1, type_name);

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		contacts_svc_end_trans(false);
		return ret;
	}
	ret = cts_db_get_last_insert_id();
	cts_stmt_finalize(stmt);

	int trans_ret = contacts_svc_end_trans(true);
	retvm_if(trans_ret < CTS_SUCCESS, trans_ret,
			"contacts_svc_end_trans(true) Failed(%d)", trans_ret);

	if (CTS_TYPE_CLASS_EXTEND_DATA == type_class) ret += CTS_DATA_EXTEND_START;
	else if (CTS_TYPE_CLASS_NUM == type_class) ret |= CTS_NUM_TYPE_CUSTOM;
	return ret;
}

API int contacts_svc_delete_custom_type(cts_custom_type_class type_class,
		int index)
{
	int ret;
	char  query[CTS_SQL_MIN_LEN] = {0};

	retvm_if(CTS_TYPE_CLASS_NUM == type_class && index <= CTS_NUM_TYPE_ASSISTANT,
			CTS_ERR_ARG_INVALID,
			"This custom number type(System Number Type) is diable to delete");

	if (CTS_TYPE_CLASS_EXTEND_DATA == type_class) index -= CTS_DATA_EXTEND_START;
	else if (CTS_TYPE_CLASS_NUM == type_class) index ^= CTS_NUM_TYPE_CUSTOM;

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE class = %d AND id = %d",
			CTS_TABLE_CUSTOM_TYPES, type_class, index);

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

API int contacts_svc_find_custom_type(cts_custom_type_class type_class,
		char *type_name)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};

	retv_if(NULL == type_name, CTS_ERR_ARG_NULL);

	CTS_START_TIME_CHECK;

	snprintf(query, sizeof(query),
			"SELECT id FROM %s WHERE class = %d AND name = '%s'",
			CTS_TABLE_CUSTOM_TYPES, type_class, type_name);
	ret = cts_query_get_first_int_result(query);
	retvm_if(ret < CTS_SUCCESS, ret, "cts_query_get_first_int_result() Failed(%d)", ret);

	if (CTS_TYPE_CLASS_EXTEND_DATA == type_class) ret += CTS_DATA_EXTEND_START;
	else if (CTS_TYPE_CLASS_NUM == type_class) ret |= CTS_NUM_TYPE_CUSTOM;

	CTS_END_TIME_CHECK();
	return ret;
}


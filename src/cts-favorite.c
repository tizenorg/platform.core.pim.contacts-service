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
#include "cts-restriction.h"
#include "cts-favorite.h"

API int contacts_svc_set_favorite(cts_favor_type op, int related_id)
{
	int ret;
	double prio = 0.0;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};

	retvm_if(CTS_FAVOR_PERSON != op && CTS_FAVOR_NUMBER != op, CTS_ERR_ARG_INVALID,
			"op(%d) is invalid", op);

	snprintf(query, sizeof(query),
			"SELECT MAX(favorite_prio) FROM %s", CTS_TABLE_FAVORITES);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE == ret) {
		prio = cts_stmt_get_dbl(stmt, 0);
	}
	else if (CTS_SUCCESS != ret) {
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		contacts_svc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	prio = prio + 1.0;
	snprintf(query, sizeof(query),
			"INSERT INTO %s SELECT NULL, %d, contact_id, %f FROM %s WHERE person_id = %d",
			CTS_TABLE_FAVORITES, op, prio, CTS_TABLE_CONTACTS, related_id);

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	cts_set_favor_noti();

	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

API int contacts_svc_unset_favorite(cts_favor_type op, int related_id)
{
	int ret;
	cts_stmt stmt;
	char query[CTS_SQL_MIN_LEN] = {0};

	if (CTS_FAVOR_PERSON == op) {
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE type = %d AND related_id IN "
				"(SELECT contact_id FROM %s WHERE person_id = %d)",
				CTS_TABLE_FAVORITES, CTS_FAVOR_PERSON, CTS_TABLE_CONTACTS, related_id);
	} else if (CTS_FAVOR_NUMBER == op) {
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE type = %d AND related_id = %d",
				CTS_TABLE_FAVORITES, CTS_FAVOR_NUMBER, related_id);
	} else {
		ERR("op(%d) is invalid", op);
		return CTS_ERR_ARG_INVALID;
	}

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = contacts_svc_begin_trans();
	if (ret) {
		cts_stmt_finalize(stmt);
		ERR("contacts_svc_begin_trans() Failed(%d)", ret);
		return ret;
	}

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		contacts_svc_end_trans(false);
		return ret;
	}
	ret = cts_db_change();
	cts_stmt_finalize(stmt);

	if (0 < ret) {
		cts_set_favor_noti();
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

API int contacts_svc_delete_favorite(int favorite_id)
{
	int ret;
	cts_stmt stmt;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE id = %d",
			CTS_TABLE_FAVORITES, favorite_id);

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		contacts_svc_end_trans(false);
		return ret;
	}
	ret = cts_db_change();
	cts_stmt_finalize(stmt);

	if (0 < ret) {
		cts_set_favor_noti();
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

static inline int cts_modify_favorite_prio(int favorite_id, int front, int back)
{
	int ret;
	cts_stmt stmt;
	double front_prio=0.0, back_prio=0.0, prio;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "SELECT favorite_prio FROM %s WHERE id = ?",
			CTS_TABLE_FAVORITES);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	cts_stmt_bind_int(stmt, 1, front);
	ret = cts_stmt_step(stmt);
	if (CTS_TRUE == ret)
		front_prio = cts_stmt_get_dbl(stmt, 0);

	cts_stmt_reset(stmt);
	cts_stmt_bind_int(stmt, 1, back);
	ret = cts_stmt_step(stmt);
	if (CTS_TRUE == ret)
		back_prio = cts_stmt_get_dbl(stmt, 0);

	cts_stmt_finalize(stmt);

	retvm_if(0.0 == front_prio && 0.0 == back_prio, CTS_ERR_ARG_INVALID,
			"The indexes for front and back are invalid.");

	if (0.0 == back_prio)
		prio = front_prio + 1;
	else
		prio = (front_prio + back_prio) / 2;

	snprintf(query, sizeof(query),
			"UPDATE %s SET favorite_prio = %f WHERE id = %d",
			CTS_TABLE_FAVORITES, prio, favorite_id);

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_query_exec() Failed(%d)", ret);
		return ret;
	}

	return CTS_SUCCESS;
}

API int contacts_svc_favorite_order(int favorite_id,
		int front_favorite_id, int back_favorite_id)
{
	int ret;

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	ret = cts_modify_favorite_prio(favorite_id, front_favorite_id, back_favorite_id);

	if (CTS_SUCCESS != ret)
	{
		ERR("cts_modify_favorite_prio() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}
	cts_set_favor_noti();

	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

API int contacts_svc_set_speeddial(int speed_num, int number_id)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"INSERT INTO %s(speed_num, number_id) VALUES(%d, %d)",
			CTS_TABLE_SPEEDDIALS, speed_num, number_id);

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}
	cts_set_speed_noti();

	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

API int contacts_svc_unset_speeddial(int speed_num)
{
	int ret;
	cts_stmt stmt;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE speed_num = %d",
			CTS_TABLE_SPEEDDIALS, speed_num);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = contacts_svc_begin_trans();
	if (ret) {
		cts_stmt_finalize(stmt);
		ERR("contacts_svc_begin_trans() Failed(%d)", ret);
		return ret;
	}

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret) {
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		contacts_svc_end_trans(false);
		return ret;
	}
	ret = cts_db_change();
	cts_stmt_finalize(stmt);

	if (0 < ret) {
		cts_set_speed_noti();
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

API int contacts_svc_get_speeddial(int speed_num, CTSvalue **value)
{
	int ret;
	cts_stmt stmt;
	const char *data;
	cts_number *number;
	char query[CTS_SQL_MAX_LEN] = {0};

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	snprintf(query, sizeof(query),
			"SELECT A.id, A.data1, A.data2, A.contact_id FROM %s A, %s B "
			"WHERE A.datatype = %d AND B.speed_num = %d AND A.id = B.number_id",
			data, CTS_TABLE_SPEEDDIALS, CTS_DATA_NUMBER, speed_num);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	number = (cts_number *)contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number) {
		ret = CTS_SUCCESS;
		number->v_type = CTS_VALUE_RDONLY_NUMBER;
		number->embedded = true;
		number->id = cts_stmt_get_int(stmt, 0);
		number->type = cts_stmt_get_int(stmt, 1);
		number->number = SAFE_STRDUP(cts_stmt_get_text(stmt, 2));
		ret = cts_stmt_get_int(stmt, 3);

		*value = (CTSvalue*) number;

		cts_stmt_finalize(stmt);
		return ret;
	}
	else {
		ERR("contacts_svc_value_new() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_OUT_OF_MEMORY;
	}
}

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
#include <string.h>
#include <db-util.h>

#include "cts-internal.h"
#include "cts-schema.h"
#include "cts-sqlite.h"

static sqlite3 *cts_db;

int cts_db_open(void)
{
	CTS_FN_CALL;
	int ret;

	if (!cts_db) {
		ret = db_util_open(CTS_DB_PATH, &cts_db, 0);
		retvm_if(SQLITE_OK != ret, CTS_ERR_DB_NOT_OPENED,
				"db_util_open() Failed(%d)", ret);
	}
	return CTS_SUCCESS;
}

int cts_db_close(void)
{
	int ret = 0;

	if (cts_db) {
		ret = db_util_close(cts_db);
		warn_if(SQLITE_OK != ret, "db_util_close() Failed(%d)", ret);
		cts_db = NULL;
		CTS_DBG("The database disconnected really.");
	}

	return CTS_SUCCESS;
}

int cts_db_change(void)
{
	return sqlite3_changes(cts_db);
}

int cts_db_get_last_insert_id(void)
{
	return sqlite3_last_insert_rowid(cts_db);
}

int cts_db_get_next_id(const char *table)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query), "SELECT seq FROM %s WHERE name = '%s'",
			CTS_SCHEMA_SQLITE_SEQ, table);

	ret = cts_query_get_first_int_result(query);
	if (ret < CTS_SUCCESS) {
		if (CTS_ERR_DB_RECORD_NOT_FOUND == ret)
			return 1;
		else
			return ret;
	} else {
		return (1 + ret);
	}
}

int cts_query_get_first_int_result(const char *query)
{
	int ret;
	cts_stmt stmt = NULL;
	retvm_if(NULL == cts_db, CTS_ERR_DB_NOT_OPENED, "Database is not opended");

	ret = sqlite3_prepare_v2(cts_db, query, strlen(query), &stmt, NULL);
	retvm_if(SQLITE_OK != ret, CTS_ERR_DB_FAILED,
			"sqlite3_prepare_v2(%s) Failed(%s)", query, sqlite3_errmsg(cts_db));

	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		ERR("sqlite3_step() Failed(%d, %s)", ret, sqlite3_errmsg(cts_db));
		sqlite3_finalize(stmt);
		if (SQLITE_DONE == ret) return CTS_ERR_DB_RECORD_NOT_FOUND;
		return CTS_ERR_DB_FAILED;
	}

	ret = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);

	return ret;
}

int cts_query_exec(char *query)
{
	int ret;
	char *err_msg = NULL;

	retvm_if(NULL == cts_db, CTS_ERR_DB_NOT_OPENED, "Database is not opended");
	CTS_DBG("query : %s", query);

	ret = sqlite3_exec(cts_db, query, NULL, NULL, &err_msg);
	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, err_msg);
		sqlite3_free(err_msg);
		switch (ret) {
		case SQLITE_BUSY:
		case SQLITE_LOCKED:
			return CTS_ERR_DB_LOCK;
		case SQLITE_IOERR:
			return CTS_ERR_IO_ERR;
		case SQLITE_FULL:
			return CTS_ERR_NO_SPACE;
		default:
			return CTS_ERR_DB_FAILED;
		}
	}

	return CTS_SUCCESS;
}

cts_stmt cts_query_prepare(char *query)
{
	int ret = -1;
	cts_stmt stmt = NULL;

	retvm_if(NULL == cts_db, NULL, "Database is not opended");
	CTS_DBG("prepare query : %s", query);

	ret = sqlite3_prepare_v2(cts_db, query, strlen(query), &stmt, NULL);
	retvm_if(SQLITE_OK != ret, NULL,
			"sqlite3_prepare_v2(%s) Failed(%s)", query, sqlite3_errmsg(cts_db));

	return stmt;
}

int cts_stmt_get_first_int_result(cts_stmt stmt)
{
	int ret;
	retvm_if(NULL == cts_db, CTS_ERR_DB_NOT_OPENED, "Database is not opended");

	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		ERR("sqlite3_step() Failed(%d, %s)", ret, sqlite3_errmsg(cts_db));
		sqlite3_finalize(stmt);
		if (SQLITE_DONE == ret) return CTS_ERR_DB_RECORD_NOT_FOUND;
		return CTS_ERR_DB_FAILED;
	}

	ret = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);

	return ret;
}

int cts_stmt_step(cts_stmt stmt)
{
	int ret;
	ret = sqlite3_step(stmt);
	switch (ret) {
	case SQLITE_BUSY:
	case SQLITE_LOCKED:
		ret = CTS_ERR_DB_LOCK;
		break;
	case SQLITE_IOERR:
		ret = CTS_ERR_IO_ERR;
		break;
	case SQLITE_FULL:
		ret = CTS_ERR_NO_SPACE;
		break;
	case SQLITE_CONSTRAINT:
		ret = CTS_ERR_ALREADY_EXIST;
		break;
	case SQLITE_ROW:
		ret = CTS_TRUE;
		break;
	case SQLITE_DONE:
		ret = CTS_SUCCESS;
		break;
	default:
		ERR("sqlite3_step() Failed(%d)", ret);
		ret = CTS_ERR_DB_FAILED;
		break;
	}
	return ret;
}

void cts_stmt_reset(cts_stmt stmt)
{
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
}

void cts_stmt_finalize(cts_stmt stmt)
{
	int ret;

	if (NULL == stmt)
		return;

	ret = sqlite3_finalize(stmt);
	warn_if(ret != SQLITE_OK,
			"sqlite3_finalize Failed(%d, %s)", ret, sqlite3_errmsg(cts_db));
}


int cts_stmt_bind_name(cts_stmt stmt, int start_cnt, cts_name *name_struct)
{
	if (name_struct->first)
		sqlite3_bind_text(stmt, start_cnt+1, name_struct->first,
				strlen(name_struct->first), SQLITE_STATIC);
	if (name_struct->last)
		sqlite3_bind_text(stmt, start_cnt+2, name_struct->last,
				strlen(name_struct->last), SQLITE_STATIC);
	if (name_struct->addition)
		sqlite3_bind_text(stmt, start_cnt+3, name_struct->addition,
				strlen(name_struct->addition), SQLITE_STATIC);
	if (name_struct->display)
		sqlite3_bind_text(stmt, start_cnt+4, name_struct->display,
				strlen(name_struct->display), SQLITE_STATIC);
	if (name_struct->prefix)
		sqlite3_bind_text(stmt, start_cnt+5, name_struct->prefix,
				strlen(name_struct->prefix), SQLITE_STATIC);
	if (name_struct->suffix)
		sqlite3_bind_text(stmt, start_cnt+6, name_struct->suffix,
				strlen(name_struct->suffix), SQLITE_STATIC);
	return start_cnt+7;
}

int cts_stmt_bind_event(cts_stmt stmt, int start_cnt, cts_event *event_struct)
{
	sqlite3_bind_int(stmt, start_cnt++, event_struct->type);
	sqlite3_bind_int(stmt, start_cnt++, event_struct->date);
	return CTS_SUCCESS;
}

int cts_stmt_bind_messenger(cts_stmt stmt, int start_cnt, cts_messenger *im_struct)
{
	sqlite3_bind_int(stmt, start_cnt++, im_struct->type);
	if (im_struct->im_id)
		sqlite3_bind_text(stmt, start_cnt++, im_struct->im_id,
				strlen(im_struct->im_id), SQLITE_STATIC);
	return CTS_SUCCESS;
}

int cts_stmt_bind_postal(cts_stmt stmt, int start_cnt, cts_postal *postal_struct)
{
	sqlite3_bind_int(stmt, start_cnt, postal_struct->type);
	if (postal_struct->pobox)
		sqlite3_bind_text(stmt, start_cnt+1, postal_struct->pobox,
				strlen(postal_struct->pobox), SQLITE_STATIC);
	if (postal_struct->postalcode)
		sqlite3_bind_text(stmt, start_cnt+2, postal_struct->postalcode,
				strlen(postal_struct->postalcode), SQLITE_STATIC);
	if (postal_struct->region)
		sqlite3_bind_text(stmt, start_cnt+3, postal_struct->region,
				strlen(postal_struct->region), SQLITE_STATIC);
	if (postal_struct->locality)
		sqlite3_bind_text(stmt, start_cnt+4, postal_struct->locality,
				strlen(postal_struct->locality), SQLITE_STATIC);
	if (postal_struct->street)
		sqlite3_bind_text(stmt, start_cnt+5, postal_struct->street,
				strlen(postal_struct->street), SQLITE_STATIC);
	if (postal_struct->extended)
		sqlite3_bind_text(stmt, start_cnt+6, postal_struct->extended,
				strlen(postal_struct->extended), SQLITE_STATIC);
	if (postal_struct->country)
		sqlite3_bind_text(stmt, start_cnt+7, postal_struct->country,
				strlen(postal_struct->country), SQLITE_STATIC);
	return CTS_SUCCESS;
}

int cts_stmt_bind_company(cts_stmt stmt, int start_cnt, cts_company *company_struct)
{
	if (company_struct->name)
		sqlite3_bind_text(stmt, start_cnt+1, company_struct->name,
				strlen(company_struct->name), SQLITE_STATIC);
	if (company_struct->department)
		sqlite3_bind_text(stmt, start_cnt+2, company_struct->department,
				strlen(company_struct->department), SQLITE_STATIC);
	if (company_struct->jot_title)
		sqlite3_bind_text(stmt, start_cnt+3, company_struct->jot_title,
				strlen(company_struct->jot_title), SQLITE_STATIC);
	if (company_struct->role)
		sqlite3_bind_text(stmt, start_cnt+4, company_struct->role,
				strlen(company_struct->role), SQLITE_STATIC);
	if (company_struct->assistant_name)
		sqlite3_bind_text(stmt, start_cnt+5, company_struct->assistant_name,
				strlen(company_struct->assistant_name), SQLITE_STATIC);
	return CTS_SUCCESS;
}

int cts_stmt_bind_web(cts_stmt stmt, int start_cnt, cts_web *web_struct)
{
	sqlite3_bind_int(stmt, start_cnt++, web_struct->type);
	if (web_struct->url)
		sqlite3_bind_text(stmt, start_cnt++, web_struct->url,
				strlen(web_struct->url), SQLITE_STATIC);
	return CTS_SUCCESS;
}

int cts_stmt_bind_extend(cts_stmt stmt, int start_cnt, cts_extend *extend_struct)
{
	sqlite3_bind_int(stmt, start_cnt, extend_struct->data1);
	if (extend_struct->data2)
		sqlite3_bind_text(stmt, start_cnt+1, extend_struct->data2,
				strlen(extend_struct->data2), SQLITE_STATIC);
	if (extend_struct->data3)
		sqlite3_bind_text(stmt, start_cnt+2, extend_struct->data3,
				strlen(extend_struct->data3), SQLITE_STATIC);
	if (extend_struct->data4)
		sqlite3_bind_text(stmt, start_cnt+3, extend_struct->data4,
				strlen(extend_struct->data4), SQLITE_STATIC);
	if (extend_struct->data5)
		sqlite3_bind_text(stmt, start_cnt+4, extend_struct->data5,
				strlen(extend_struct->data5), SQLITE_STATIC);
	if (extend_struct->data6)
		sqlite3_bind_text(stmt, start_cnt+5, extend_struct->data6,
				strlen(extend_struct->data6), SQLITE_STATIC);
	if (extend_struct->data7)
		sqlite3_bind_text(stmt, start_cnt+6, extend_struct->data7,
				strlen(extend_struct->data7), SQLITE_STATIC);
	if (extend_struct->data8)
		sqlite3_bind_text(stmt, start_cnt+7, extend_struct->data8,
				strlen(extend_struct->data8), SQLITE_STATIC);
	if (extend_struct->data9)
		sqlite3_bind_text(stmt, start_cnt+8, extend_struct->data9,
				strlen(extend_struct->data9), SQLITE_STATIC);
	if (extend_struct->data10)
		sqlite3_bind_text(stmt, start_cnt+9, extend_struct->data10,
				strlen(extend_struct->data10), SQLITE_STATIC);
	return CTS_SUCCESS;
}

int cts_stmt_get_addressbook(cts_stmt stmt, cts_addrbook *ab)
{
	int cnt = 0;
	char *temp;

	ab->id = cts_stmt_get_int(stmt, cnt++);
	temp = cts_stmt_get_text(stmt, cnt++);
	ab->name = SAFE_STRDUP(temp);
	ab->acc_id = cts_stmt_get_int(stmt, cnt++);
	ab->acc_type = cts_stmt_get_int(stmt, cnt++);
	ab->mode = cts_stmt_get_int(stmt, cnt++);

	return CTS_SUCCESS;
}

int cts_stmt_get_number(cts_stmt stmt, cts_number *result, int start_cnt)
{
	char *temp;

	result->id = cts_stmt_get_int(stmt, start_cnt++);
	result->type = cts_stmt_get_int(stmt, start_cnt++);
	temp = cts_stmt_get_text(stmt, start_cnt++);
	result->number = SAFE_STRDUP(temp);

	return start_cnt;
}

int cts_stmt_get_email(cts_stmt stmt, cts_email *result, int start_cnt)
{
	char *temp;

	result->id = cts_stmt_get_int(stmt, start_cnt++);
	result->type = cts_stmt_get_int(stmt, start_cnt++);
	temp = cts_stmt_get_text(stmt, start_cnt++);
	result->email_addr = SAFE_STRDUP(temp);

	return start_cnt;
}

int cts_stmt_get_name(cts_stmt stmt, cts_name *result, int start_cnt)
{
	char *temp;

	result->id = cts_stmt_get_int(stmt, start_cnt++);
	result->lang_type = cts_stmt_get_int(stmt, start_cnt++);
	temp = cts_stmt_get_text(stmt, start_cnt++);
	result->first = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, start_cnt++);
	result->last = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, start_cnt++);
	result->addition = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, start_cnt++);
	result->display = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, start_cnt++);
	result->prefix = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, start_cnt++);
	result->suffix = SAFE_STRDUP(temp);

	return CTS_SUCCESS;
}


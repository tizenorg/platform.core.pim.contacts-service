/*
 * Contacts Service Helper
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
#include <unistd.h>
#include <string.h>
#include <db-util.h>
#include <fcntl.h>

#include "cts-errors.h"
#include "cts-schema.h"
#include "cts-sqlite.h"
#include "cts-utils.h"

#include "internal.h"
#include "normalize.h"
#include "utils.h"

static sqlite3 *helper_db;

int helper_db_open(sqlite3 **db)
{
	HELPER_FN_CALL;
	int ret;

	if (!helper_db)
	{
		ret = db_util_open(CTS_DB_PATH, &helper_db, 0);
		h_retvm_if(ret != SQLITE_OK, CTS_ERR_DB_NOT_OPENED,
				"db_util_open() Failed(%d)", ret);
	}
	if (db)
		*db = helper_db;
	return CTS_SUCCESS;
}

int helper_db_close(void)
{
	if (helper_db)
	{
		db_util_close(helper_db);
		helper_db = NULL;
	}

	return CTS_SUCCESS;
}

int helper_begin_trans(void)
{
	int ret = -1;

	ret = sqlite3_exec(helper_db, "BEGIN IMMEDIATE TRANSACTION",
			NULL, NULL, NULL);

	while (SQLITE_BUSY == ret) {
		sleep(1);
		ret = sqlite3_exec(helper_db, "BEGIN IMMEDIATE TRANSACTION",
				NULL, NULL, NULL);
	}

	if (SQLITE_OK != ret)
	{
		ERR("sqlite3_exec() Failed(%d)", ret);
		return CTS_ERR_DB_FAILED;
	}
	return CTS_SUCCESS;
}

#define CTS_COMMIT_TRY_MAX 3
int helper_end_trans(bool success)
{
	int ret = -1, i=0;
	char *errmsg = NULL;

	if (success) {
		ret = sqlite3_exec(helper_db, "COMMIT TRANSACTION",
				NULL, NULL, &errmsg);
		if (SQLITE_OK != ret)
		{
			ERR("sqlite3_exec(COMMIT) Failed(%d, %s)", ret, errmsg);
			sqlite3_free(errmsg);

			while (SQLITE_BUSY == ret && i<CTS_COMMIT_TRY_MAX) {
				i++;
				sleep(1);
				ret = sqlite3_exec(helper_db, "COMMIT TRANSACTION",
						NULL, NULL, NULL);
			}
			if (SQLITE_OK != ret) {
				ERR("sqlite3_exec() Failed(%d)", ret);
				sqlite3_exec(helper_db, "ROLLBACK TRANSACTION",
						NULL, NULL, NULL);
				return CTS_ERR_DB_FAILED;
			}
		}
	}
	else {
		sqlite3_exec(helper_db, "ROLLBACK TRANSACTION", NULL, NULL, NULL);
	}

	return CTS_SUCCESS;
}

int helper_update_default_language(int prev_lang, int new_lang)
{
	int ret;
	sqlite3* db = NULL;
	char *errmsg = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	ret = helper_db_open(&db);
	h_retvm_if(CTS_SUCCESS != ret, ret, "helper_db_open() Failed(%d)", ret);

	ret = helper_begin_trans();
	h_retvm_if(ret, ret, "helper_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query), "UPDATE %s SET %s=%d WHERE %s=%d",
			CTS_TABLE_DATA, CTS_SCHEMA_DATA_NAME_LANG_INFO, prev_lang,
			CTS_SCHEMA_DATA_NAME_LANG_INFO, CTS_LANG_DEFAULT);

	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret)
	{
		ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		helper_end_trans(false);
		return CTS_ERR_DB_FAILED;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET %s=%d WHERE %s=%d",
			CTS_TABLE_DATA, CTS_SCHEMA_DATA_NAME_LANG_INFO, CTS_LANG_DEFAULT,
			CTS_SCHEMA_DATA_NAME_LANG_INFO, new_lang);

	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret)
	{
		ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		helper_end_trans(false);
		return CTS_ERR_DB_FAILED;
	}

	ret = helper_set_default_language(new_lang);
	if (CTS_SUCCESS != ret) {
		helper_end_trans(false);
		return ret;
	}
	ret = helper_end_trans(true);
	helper_db_close();

	return ret;
}

int helper_insert_SDN_contact(const char *name, const char *number)
{
	int ret;
	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	ret = helper_db_open(&db);
	h_retvm_if(CTS_SUCCESS != ret, ret, "helper_db_open() Failed(%d)", ret);

	snprintf(query, sizeof(query), "INSERT INTO %s(name, number) VALUES(?,?)",
			CTS_TABLE_SIM_SERVICES);

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	h_retvm_if(SQLITE_OK != ret, CTS_ERR_DB_FAILED,
			"sqlite3_prepare_v2(%s) Failed(%s)", query, sqlite3_errmsg(db));

	sqlite3_bind_text(stmt, 1, name, strlen(name), SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, number, strlen(number), SQLITE_STATIC);

	ret = sqlite3_step(stmt);
	if (SQLITE_DONE != ret)
	{
		ERR("sqlite3_step() Failed(%d)", ret);
		sqlite3_finalize(stmt);
		return CTS_ERR_DB_FAILED;
	}
	sqlite3_finalize(stmt);

	helper_db_close();
	return CTS_SUCCESS;
}

int helper_delete_SDN_contact(void)
{
	int ret;
	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	ret = helper_db_open(&db);
	h_retvm_if(CTS_SUCCESS != ret, ret, "helper_db_open() Failed(%d)", ret);

	snprintf(query, sizeof(query), "DELETE FROM %s", CTS_TABLE_SIM_SERVICES);

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	h_retvm_if(SQLITE_OK != ret, CTS_ERR_DB_FAILED,
			"sqlite3_prepare_v2(%s) Failed(%s)", query, sqlite3_errmsg(db));

	ret = sqlite3_step(stmt);
	if (SQLITE_DONE != ret)
	{
		ERR("sqlite3_step() Failed(%d)", ret);
		sqlite3_finalize(stmt);
		return CTS_ERR_DB_FAILED;
	}
	sqlite3_finalize(stmt);

	helper_db_close();
	return CTS_SUCCESS;
}

static inline int helper_get_display_name(char *display, char *first,
		char *last, char *dest, int dest_size)
{
	if (display) {
		snprintf(dest, dest_size, "%s", display);
	}
	else {
		if (NULL == first && NULL == last)
			return CTS_ERR_NO_DATA;
		if (!last)
			snprintf(dest, dest_size, "%s", first);
		else if (!first)
			snprintf(dest, dest_size, "%s", last);
		else if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			snprintf(dest, dest_size, "%s %s", first, last);
		else
			snprintf(dest, dest_size, "%s, %s", last, first);
	}
	return CTS_SUCCESS;
}

int helper_update_collation()
{
	int ret;
	sqlite3* db = NULL;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};
	char sortkey[CTS_SQL_MIN_LEN];
	char dest[CTS_SQL_MIN_LEN];

	ret = helper_db_open(&db);
	h_retvm_if(CTS_SUCCESS != ret, ret, "helper_db_open() Failed(%d)", ret);

	ret = helper_begin_trans();
	if(CTS_SUCCESS != ret) {
		ERR("helper_begin_trans() Failed(%d)", ret);
		helper_db_close();
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT contact_id, data2, data3, data5 FROM %s WHERE datatype = %d",
			CTS_TABLE_DATA, CTS_DATA_NAME);
	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if(SQLITE_OK != ret) {
		ERR("sqlite3_prepare_v2(%s) Failed(%s)", query, sqlite3_errmsg(db));
		helper_end_trans(false);
		helper_db_close();
		return CTS_ERR_DB_FAILED;
	}

	while (SQLITE_ROW == (ret = sqlite3_step(stmt))) {
		cts_stmt update_stmt = NULL;
		int contact_id = sqlite3_column_int(stmt, 0);
		char *first = (char*)sqlite3_column_text(stmt, 1);
		char *last = (char*)sqlite3_column_text(stmt, 2);
		char *display = (char*)sqlite3_column_text(stmt, 3);

		ret = helper_get_display_name(display, first, last, dest, sizeof(dest));
		if (CTS_SUCCESS != ret)
			continue;

		ret = helper_collation_str(dest, sortkey, sizeof(sortkey));
		if (CTS_SUCCESS != ret) {
			ERR("helper_collation_str() Failed(%d)", ret);
			sqlite3_finalize(stmt);
			helper_end_trans(false);
			helper_db_close();
			return CTS_ERR_DB_FAILED;
		}
		snprintf(query, sizeof(query), "UPDATE %s SET %s=? WHERE contact_id=%d",
				CTS_TABLE_DATA, CTS_SCHEMA_DATA_NAME_SORTING_KEY,
				contact_id);
		ret = sqlite3_prepare_v2(db, query, strlen(query), &update_stmt, NULL);
		if(SQLITE_OK != ret) {
			ERR("sqlite3_prepare_v2(%s) Failed(%s)", query, sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			helper_end_trans(false);
			helper_db_close();
			return CTS_ERR_DB_FAILED;
		}

		sqlite3_bind_text(update_stmt, 1, sortkey, strlen(sortkey), SQLITE_STATIC);
		HELPER_DBG("query : %s", query);
		ret = sqlite3_step(update_stmt);
		if (SQLITE_DONE != ret) {
			ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			sqlite3_finalize(update_stmt);
			helper_end_trans(false);
			helper_db_close();
			return CTS_ERR_DB_FAILED;
		}
		sqlite3_finalize(update_stmt);
	}

	if (SQLITE_ROW != ret && SQLITE_DONE != ret) {
		ERR("sqlite3_step() Failed(%d)", ret);
		sqlite3_finalize(stmt);
		helper_end_trans(false);
		helper_db_close();
		return CTS_ERR_DB_FAILED;
	}

	sqlite3_finalize(stmt);

	ret = helper_end_trans(true);
	if (CTS_SUCCESS == ret) {
		int fd = open(CTS_NOTI_CONTACT_CHANGED_DEF, O_TRUNC | O_RDWR);
		if (0 <= fd)
			close(fd);
	}
	helper_db_close();

	return ret;
}

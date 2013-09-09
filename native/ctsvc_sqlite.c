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

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <db-util.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_notification.h"

#include "ctsvc_db_init.h"
#include "ctsvc_db_plugin_person_helper.h"
#include "ctsvc_db_plugin_image_helper.h"
#include "ctsvc_db_plugin_company_helper.h"
#include "ctsvc_db_plugin_group_helper.h"


#include "ctsvc_phonelog.h"
#include "ctsvc_person.h"

#define CTSVC_QUERY_RETRY_TIME	8
#define CTSVC_QUERY_RETRY_INTERVAL	50*1000

static __thread sqlite3 *ctsvc_db = NULL;

static inline int __ctsvc_db_busyhandler(void *pData, int count)
{
	if(10 - count > 0) {
		CTS_INFO("Busy Handler Called! : Thread(%08x) / CNT(%d)\n", pthread_self(), count+1);
		usleep(20000);
		return 1;
	} else {
		CTS_INFO("Busy Handler will be returned SQLITE_BUSY error : Thread(%08x) \n", pthread_self());
		return 0;
	}
}

int ctsvc_db_open(void) {
	CTS_FN_CALL;
	int ret;

	if (!ctsvc_db) {
		ret = db_util_open(CTSVC_DB_PATH, &ctsvc_db, 0);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB /*CTS_ERR_DB_NOT_OPENED*/,
				"DB error : db_util_open() Failed(%d)", ret);
		ret = sqlite3_create_function(ctsvc_db, "_DATA_DELETE_", 2, SQLITE_UTF8, NULL,
					ctsvc_db_data_delete_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Failed(%d)", ret);
		ret = sqlite3_create_function(ctsvc_db, "_DATA_IMAGE_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_image_delete_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Failed(%d)", ret);
		ret = sqlite3_create_function(ctsvc_db, "_DATA_COMPANY_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_company_delete_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Failed(%d)", ret);
		ret = sqlite3_create_function(ctsvc_db, "_NORMALIZE_INDEX_", 2, SQLITE_UTF8, NULL,
					ctsvc_db_normalize_str_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Failed(%d)", ret);
		ret = sqlite3_create_function(ctsvc_db, "_PHONE_LOG_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_phone_log_delete_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Failed(%d)", ret);
		ret = sqlite3_create_function(ctsvc_db, "_PERSON_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_person_delete_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Failed(%d)", ret);
		ret = sqlite3_create_function(ctsvc_db, "_GROUP_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_group_delete_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Failed(%d)", ret);

		/* Register Busy handler */
		ret = sqlite3_busy_handler(ctsvc_db, __ctsvc_db_busyhandler, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_busy_handler() Failed(%d)", ret);
	}
	return CONTACTS_ERROR_NONE /*CTS_SUCCESS*/;
}

int ctsvc_db_close(void) {
	int ret = 0;

	if (ctsvc_db) {
		ret = db_util_close(ctsvc_db);
		WARN_IF(SQLITE_OK != ret, "db_util_close() Failed(%d)", ret);
		ctsvc_db = NULL;
		CTS_DBG("The database disconnected really.");
	}

	return CONTACTS_ERROR_NONE /*CTS_SUCCESS*/;
}

int cts_db_change(void) {
	return sqlite3_changes(ctsvc_db);
}

int cts_db_get_last_insert_id(void) {
	return sqlite3_last_insert_rowid(ctsvc_db);
}

int cts_db_get_next_id(const char *table) {
	int id;
	int ret;
	char query[CTS_SQL_MAX_LEN] = { 0 };

	snprintf(query, sizeof(query), "SELECT seq FROM %s WHERE name = '%s'",
			CTS_SCHEMA_SQLITE_SEQ, table);

	ret = ctsvc_query_get_first_int_result(query, &id);
	if (ret != CONTACTS_ERROR_NONE /*CTS_SUCCESS*/) {
		if (CONTACTS_ERROR_NO_DATA /*CONTACTS_ERR_DB_RECORD_NOT_FOUND*/ == ret)
			return 1;
		else
			return id;
	} else {
		return (1 + id);
	}
}

int ctsvc_query_get_first_int_result(const char *query, int *result) {
	int ret;
	struct timeval from, now, diff;
	bool retry = false;
	cts_stmt stmt = NULL;
	RETVM_IF(NULL == ctsvc_db, CONTACTS_ERROR_DB /*CTS_ERR_DB_NOT_OPENED*/, "DB error : Database is not opended");

	gettimeofday(&from, NULL);
	do {
		ret = sqlite3_prepare_v2(ctsvc_db, query, strlen(query), &stmt, NULL);
		if (ret != SQLITE_OK)
			CTS_ERR("DB error : sqlite3_prepare_v2() Failed(%d, %s)", ret, sqlite3_errmsg(ctsvc_db));

		if (ret == SQLITE_BUSY || ret == SQLITE_LOCKED) {
			gettimeofday(&now, NULL);
			timersub(&now, &from, &diff);
			retry = (diff.tv_sec < CTSVC_QUERY_RETRY_TIME)? true:false;
			if (retry)
				usleep(CTSVC_QUERY_RETRY_INTERVAL);
		} else
			retry = false;
	}while(retry);

	if (SQLITE_OK != ret) {
		CTS_ERR("DB error : sqlite3_prepare_v2(%s) Failed(%s)", query, sqlite3_errmsg(ctsvc_db));
		return CONTACTS_ERROR_DB;
	}

	retry = false;
	gettimeofday(&from, NULL);
	do {
		ret = sqlite3_step(stmt);
		if (ret != SQLITE_ROW && SQLITE_DONE != ret)
			CTS_ERR("DB error : sqlite3_step() Failed(%d, %s)", ret, sqlite3_errmsg(ctsvc_db));

		if (ret == SQLITE_BUSY || ret == SQLITE_LOCKED) {
			gettimeofday(&now, NULL);
			timersub(&now, &from, &diff);
			retry = (diff.tv_sec < CTSVC_QUERY_RETRY_TIME)? true:false;
			if (retry)
				usleep(CTSVC_QUERY_RETRY_INTERVAL);
		}
		else
			retry = false;
	}while(retry);

	if (SQLITE_ROW != ret) {
		sqlite3_finalize(stmt);
		CTS_DBG("query : %s", query);
		if (SQLITE_DONE == ret) {
			CTS_INFO("sqlite3_step() return with SQLITE_DONE (it means NO_DATA) (%d, %s)",
				ret, sqlite3_errmsg(ctsvc_db));
			return CONTACTS_ERROR_NO_DATA /*CONTACTS_ERR_DB_RECORD_NOT_FOUND*/;
		}
		CTS_ERR("sqlite3_step() Failed(%d, %s)", ret, sqlite3_errmsg(ctsvc_db));
		return CONTACTS_ERROR_DB;
	}

	*result = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_query_exec(const char *query) {
	int ret;
	struct timeval from, now, diff;
	bool retry = false;
	char *err_msg = NULL;

	RETVM_IF(NULL == ctsvc_db, CONTACTS_ERROR_DB /*CTS_ERR_DB_NOT_OPENED*/, "DB error : Database is not opended");
	CTS_DBG("query : %s", query);

	gettimeofday(&from, NULL);
	do {
		ret = sqlite3_exec(ctsvc_db, query, NULL, NULL, &err_msg);

		if (ret != SQLITE_OK)
			CTS_ERR("DB error : sqlite3_exec() Failed(%d, %s)", ret, sqlite3_errmsg(ctsvc_db));

		if (ret == SQLITE_BUSY || ret == SQLITE_LOCKED) {
			gettimeofday(&now, NULL);
			timersub(&now, &from, &diff);
			retry = (diff.tv_sec < CTSVC_QUERY_RETRY_TIME)? true:false;
			if (retry)
				usleep(CTSVC_QUERY_RETRY_INTERVAL);
		}
		else
			retry = false;
	} while(retry);

	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, err_msg);
		sqlite3_free(err_msg);
		switch (ret) {
		case SQLITE_BUSY:
		case SQLITE_LOCKED:
			return CONTACTS_ERROR_DB /*CTS_ERR_DB_LOCK*/;
		case SQLITE_IOERR:
			return CONTACTS_ERROR_DB /*CTS_ERR_IO_ERR*/;
		case SQLITE_FULL:
			return CONTACTS_ERROR_FILE_NO_SPACE /*CTS_ERR_NO_SPACE*/;
		default:
			return CONTACTS_ERROR_DB;
		}
	}

	return CONTACTS_ERROR_NONE /*CTS_SUCCESS*/;
}

cts_stmt cts_query_prepare(char *query) {
	int ret = -1;
	cts_stmt stmt = NULL;
	struct timeval from, now, diff;
	bool retry = false;

	RETVM_IF(NULL == ctsvc_db, NULL, "DB error : Database is not opened");
	CTS_DBG("prepare query : %s", query);

	gettimeofday(&from, NULL);
	do {
		ret = sqlite3_prepare_v2(ctsvc_db, query, strlen(query), &stmt, NULL);

		if (ret != SQLITE_OK)
			CTS_ERR("DB error : sqlite3_prepare_v2() Failed(%d, %s)", ret, sqlite3_errmsg(ctsvc_db));

		if (ret == SQLITE_BUSY || ret == SQLITE_LOCKED) {
			gettimeofday(&now, NULL);
			timersub(&now, &from, &diff);
			retry = (diff.tv_sec < CTSVC_QUERY_RETRY_TIME)? true:false;
			if (retry)
				usleep(CTSVC_QUERY_RETRY_INTERVAL);
		} else
			retry = false;
	}while(retry);

	return stmt;
}

int ctsvc_stmt_get_first_int_result(cts_stmt stmt, int *result) {
	int ret;
	struct timeval from, now, diff;
	bool retry = false;
	RETVM_IF(NULL == ctsvc_db, CONTACTS_ERROR_DB /*CTS_ERR_DB_NOT_OPENED*/, "DB error : Database is not opened");

	gettimeofday(&from, NULL);
	do {
		ret = sqlite3_step(stmt);
		if (SQLITE_ROW != ret && SQLITE_DONE != ret)
			CTS_ERR("sqlite3_step() Failed(%d, %s)", ret, sqlite3_errmsg(ctsvc_db));

		if (ret == SQLITE_BUSY || ret == SQLITE_LOCKED) {
			gettimeofday(&now, NULL);
			timersub(&now, &from, &diff);
			retry = (diff.tv_sec < CTSVC_QUERY_RETRY_TIME)? true:false;
			if (retry)
				usleep(CTSVC_QUERY_RETRY_INTERVAL);
		}
		else
			retry = false;
	}while(retry);

	if (SQLITE_ROW != ret) {
		CTS_ERR("sqlite3_step() Failed(%d, %s)", ret, sqlite3_errmsg(ctsvc_db));
		sqlite3_finalize(stmt);
		if (SQLITE_DONE == ret)
			return CONTACTS_ERROR_NO_DATA /*CONTACTS_ERR_DB_RECORD_NOT_FOUND*/;
		return CONTACTS_ERROR_DB;
	}

	*result = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);

	return CONTACTS_ERROR_NONE;
}

int cts_stmt_step(cts_stmt stmt) {
	int ret;
	struct timeval from, now, diff;
	bool retry = false;

	gettimeofday(&from, NULL);
	do {
		ret = sqlite3_step(stmt);

		if (ret != SQLITE_ROW && ret != SQLITE_DONE)
			CTS_ERR("DB error : sqlite3_step() Failed(%d, %s)", ret, sqlite3_errmsg(ctsvc_db));

		if (ret == SQLITE_BUSY || ret == SQLITE_LOCKED) {
			gettimeofday(&now, NULL);
			timersub(&now, &from, &diff);
			retry = (diff.tv_sec < CTSVC_QUERY_RETRY_TIME)? true:false;
			if (retry)
				usleep(CTSVC_QUERY_RETRY_INTERVAL);
		}
		else
			retry = false;
	}while(retry);

	switch (ret) {
	case SQLITE_BUSY:
	case SQLITE_LOCKED:
		ret = CONTACTS_ERROR_DB /*CTS_ERR_DB_LOCK*/;
		break;
	case SQLITE_IOERR:
		ret = CONTACTS_ERROR_DB /*CTS_ERR_IO_ERR*/;
		break;
	case SQLITE_FULL:
		ret = CONTACTS_ERROR_FILE_NO_SPACE /*CTS_ERR_NO_SPACE*/;
		break;
	case SQLITE_CONSTRAINT:
		ret = CONTACTS_ERROR_DB /*CTS_ERR_ALREADY_EXIST*/;
		break;
	case SQLITE_ROW:
		ret = 1 /*CTS_TRUE*/;
		break;
	case SQLITE_DONE:
		ret = CONTACTS_ERROR_NONE /*CTS_SUCCESS*/;
		break;
	case SQLITE_CORRUPT:
		ASSERT_NOT_REACHED("the database disk image is malformed");
		ret = CONTACTS_ERROR_DB;
		break;
	default:
		CTS_ERR("sqlite3_step() Failed(%d)", ret);
		ret = CONTACTS_ERROR_DB;
		break;
	}
	return ret;
}

void cts_stmt_reset(cts_stmt stmt) {
	sqlite3_reset(stmt);
	sqlite3_clear_bindings(stmt);
}

void cts_stmt_finalize(cts_stmt stmt) {
	int ret;

	if (NULL == stmt)
		return;

	ret = sqlite3_finalize(stmt);
	WARN_IF(ret != SQLITE_OK, "sqlite3_finalize Failed(%d, %s)",
			ret, sqlite3_errmsg(ctsvc_db));
}

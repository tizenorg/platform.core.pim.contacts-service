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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <db-util.h>
#include <sqlite3.h>

#include "contacts.h"

#include "ctsvc_internal.h"
#include "ctsvc_server_sqlite.h"
#include "schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_schema_recovery.h"
#include "ctsvc_schema.h"

// Additional Error
enum {
	CTSVC_ERR_NO_DB_FILE = -10000,
	CTSVC_ERR_NO_TABLE,
};

static inline int __ctsvc_server_check_db_file(void)
{
	int fd = open(CTSVC_DB_PATH, O_RDONLY);
	RETVM_IF(-1 == fd, CTSVC_ERR_NO_DB_FILE, "DB file is not exist");

	close(fd);
	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_server_remake_db_file()
{
	int ret, fd;
	char *errmsg;
	sqlite3 *db;

	ret = ctsvc_server_db_open(&db);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_db_open() Failed(%d)", ret);

	ret = sqlite3_exec(db, schema_query, NULL, 0, &errmsg);
	if (SQLITE_OK != ret) {
		CTS_ERR("remake contacts DB file is Failed : %s", errmsg);
		sqlite3_free(errmsg);
	}

	ctsvc_server_db_close();

	fd = open(CTSVC_DB_PATH, O_CREAT | O_RDWR, 0660);
	RETVM_IF(-1 == fd, CONTACTS_ERROR_SYSTEM, "open Failed");

	ret = fchown(fd, getuid(), CTS_SECURITY_FILE_GROUP);
	if (0 != ret)
		CTS_ERR("fchown(%s) Failed(%d)", CTSVC_DB_PATH, ret);
	ret = fchmod(fd, CTS_SECURITY_DEFAULT_PERMISSION);
	if (0 != ret)
		CTS_ERR("fchown(%s) Failed(%d)", CTSVC_DB_PATH, ret);
	close(fd);

	fd = open(CTSVC_DB_JOURNAL_PATH, O_CREAT | O_RDWR, 0660);
	RETVM_IF(-1 == fd, CONTACTS_ERROR_SYSTEM, "open Failed");

	ret = fchown(fd, getuid(), CTS_SECURITY_FILE_GROUP);
	if (0 != ret)
		CTS_ERR("fchown(%s) Failed(%d)", CTSVC_DB_JOURNAL_PATH, ret);
	ret = fchmod(fd, CTS_SECURITY_DEFAULT_PERMISSION);
	if (0 != ret)
		CTS_ERR("fchown(%s) Failed(%d)", CTSVC_DB_JOURNAL_PATH, ret);
	close(fd);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_server_check_table()
{
	int ret;
	sqlite3 *db;
	char query[CTS_SQL_MAX_LEN] = {0};
	cts_stmt stmt = NULL;

	ret = ctsvc_server_db_open(&db);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_db_open() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT name FROM sqlite_master WHERE type='table' AND name='%s'",
			CTS_TABLE_CONTACTS);
	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("DB error : sqlite3_prepare_v2(%s) Failed(%s)", query, sqlite3_errmsg(db));
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		CTS_ERR("contacts table does not exist in contacts DB file : %s", sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		ctsvc_server_db_close();
		return CTSVC_ERR_NO_TABLE;
	}

	sqlite3_finalize(stmt);
	ctsvc_server_db_close();
	return CONTACTS_ERROR_NONE;
}

int ctsvc_server_check_schema(void)
{
	if (CTSVC_ERR_NO_DB_FILE == __ctsvc_server_check_db_file())
		__ctsvc_server_remake_db_file();
	else if (CTSVC_ERR_NO_TABLE == __ctsvc_server_check_table())
		__ctsvc_server_remake_db_file();

	return CONTACTS_ERROR_NONE;
}


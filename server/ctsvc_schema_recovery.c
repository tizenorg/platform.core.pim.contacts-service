/*
 * Contacts Service Helper
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
#include <contacts.h>

#include "internal.h"
#include "ctsvc_server_sqlite.h"
#include "schema.h"
#include "ctsvc_schema_recovery.h"
#include "ctsvc_schema.h"

static inline int __ctsvc_server_check_db_file(void)
{
	int fd = open(CTSVC_DB_PATH, O_RDONLY);
	h_retvm_if(-1 == fd, CTSVC_ERR_NO_DB_FILE,
			"DB file(%s) is not exist", CTSVC_DB_PATH);

	close(fd);
	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_server_remake_db_file()
{
	int ret, fd;
	char *errmsg;
	sqlite3 *db;

	ret = ctsvc_server_db_open(&db);
	h_retvm_if(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_db_open() Failed(%d)", ret);

	ret = sqlite3_exec(db, schema_query, NULL, 0, &errmsg);
	if (SQLITE_OK != ret) {
		ERR("remake contacts DB file is Failed : %s", errmsg);
		sqlite3_free(errmsg);
	}

	ctsvc_server_db_close();

	fd = open(CTSVC_DB_PATH, O_CREAT | O_RDWR, 0660);
	h_retvm_if(-1 == fd, CONTACTS_ERROR_SYSTEM, "open Failed");

	ret = fchown(fd, getuid(), CTS_SECURITY_FILE_GROUP);
	if (0 != ret)
		ERR("fchown(%s) Failed(%d)", CTSVC_DB_PATH, ret);
	ret = fchmod(fd, CTS_SECURITY_DEFAULT_PERMISSION);
	if (0 != ret)
		ERR("fchown(%s) Failed(%d)", CTSVC_DB_PATH, ret);
	close(fd);

	fd = open(CTSVC_DB_JOURNAL_PATH, O_CREAT | O_RDWR, 0660);
	h_retvm_if(-1 == fd, CONTACTS_ERROR_SYSTEM, "open Failed");

	ret = fchown(fd, getuid(), CTS_SECURITY_FILE_GROUP);
	if (0 != ret)
		ERR("fchown(%s) Failed(%d)", CTSVC_DB_JOURNAL_PATH, ret);
	ret = fchmod(fd, CTS_SECURITY_DEFAULT_PERMISSION);
	if (0 != ret)
		ERR("fchown(%s) Failed(%d)", CTSVC_DB_JOURNAL_PATH, ret);
	close(fd);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_server_check_schema(void)
{
	if (CTSVC_ERR_NO_DB_FILE == __ctsvc_server_check_db_file())
		__ctsvc_server_remake_db_file();

	return CONTACTS_ERROR_NONE;
}


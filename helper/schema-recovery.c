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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <db-util.h>
#include <sqlite3.h>
#include <contacts-svc.h>

#include "internal.h"
#include "sqlite.h"
#include "schema.h"
#include "schema-recovery.h"
#include "cts-schema.h"

static inline int helper_check_db_file(void)
{
	int fd = open(CTS_DB_PATH, O_RDONLY);
	h_retvm_if(-1 == fd, CTS_ERR_NO_DB_FILE,
			"DB file(%s) is not exist", CTS_DB_PATH);

	close(fd);
	return CTS_SUCCESS;
}

static inline int remake_db_file()
{
	int ret, fd;
	char *errmsg;
	sqlite3 *db;

	ret = helper_db_open(&db);
	h_retvm_if(CTS_SUCCESS != ret, ret, "helper_db_open() Failed(%d)", ret);

	ret = sqlite3_exec(db, schema_query, NULL, 0, &errmsg);
	if (SQLITE_OK != ret) {
		ERR("remake contacts DB file is Failed : %s", errmsg);
		sqlite3_free(errmsg);
	}

	helper_db_close();

	fd = open(CTS_DB_PATH, O_CREAT | O_RDWR, 0660);
	h_retvm_if(-1 == fd, CTS_ERR_FAIL, "open Failed");

	fchown(fd, getuid(), CTS_SECURITY_FILE_GROUP);
	fchmod(fd, CTS_SECURITY_DEFAULT_PERMISSION);
	close(fd);

	fd = open(CTS_DB_JOURNAL_PATH, O_CREAT | O_RDWR, 0660);
	h_retvm_if(-1 == fd, CTS_ERR_FAIL, "open Failed");

	fchown(fd, getuid(), CTS_SECURITY_FILE_GROUP);
	fchmod(fd, CTS_SECURITY_DEFAULT_PERMISSION);
	close(fd);

	return CTS_SUCCESS;
}

int helper_check_schema(void)
{
	if (CTS_ERR_NO_DB_FILE == helper_check_db_file())
		remake_db_file();

	return CTS_SUCCESS;
}


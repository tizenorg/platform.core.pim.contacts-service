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
#include <unistd.h>
#include <string.h>
#include <db-util.h>
#include <fcntl.h>

#include "internal.h"
#include "contacts.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_notify.h"
#include "ctsvc_normalize.h"
#include "ctsvc_localize.h"
#include "ctsvc_server_utils.h"

#include "ctsvc_notification.h"
#include "ctsvc_db_plugin_image_helper.h"
#include "ctsvc_db_plugin_company_helper.h"

#include "ctsvc_person.h"
#include "ctsvc_phonelog.h"

static sqlite3 *server_db;

int ctsvc_server_db_open(sqlite3 **db)
{
	SERVER_FN_CALL;
	int ret;

	if (!server_db) {
		ret = db_util_open(CTSVC_DB_PATH, &server_db, 0);
		h_retvm_if(ret != SQLITE_OK, CONTACTS_ERROR_DB,
				"db_util_open() Failed(%d)", ret);

		ret = sqlite3_create_function(server_db, "_DATA_DELETE_", 2, SQLITE_UTF8, NULL,
					ctsvc_db_data_delete_callback, NULL, NULL);
		h_retvm_if(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Failed(%d)", ret);
		ret = sqlite3_create_function(server_db, "_DATA_IMAGE_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_data_image_delete_callback, NULL, NULL);
		h_retvm_if(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Failed(%d)", ret);
		ret = sqlite3_create_function(server_db, "_DATA_COMPANY_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_data_company_delete_callback, NULL, NULL);
		h_retvm_if(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Failed(%d)", ret);
		ret = sqlite3_create_function(server_db, "_PHONE_LOG_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_phone_log_delete_callback, NULL, NULL);
		h_retvm_if(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Failed(%d)", ret);
		ret = sqlite3_create_function(server_db, "_PERSON_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_person_delete_callback, NULL, NULL);
		h_retvm_if(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Failed(%d)", ret);
	}
	if (db)
		*db = server_db;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_server_db_close(void)
{
	if (server_db) {
		db_util_close(server_db);
		server_db = NULL;
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_server_begin_trans(void)
{
	int ret = -1;

	ret = sqlite3_exec(server_db, "BEGIN IMMEDIATE TRANSACTION",
			NULL, NULL, NULL);

	while (SQLITE_BUSY == ret) {
		sleep(1);
		ret = sqlite3_exec(server_db, "BEGIN IMMEDIATE TRANSACTION",
				NULL, NULL, NULL);
	}

	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec() Failed(%d)", ret);
		return CONTACTS_ERROR_DB;
	}
	return CONTACTS_ERROR_NONE;
}

#define CTS_COMMIT_TRY_MAX 3
int ctsvc_server_end_trans(bool success)
{
	int ret = -1, i=0;
	char *errmsg = NULL;

	if (success) {
		ret = sqlite3_exec(server_db, "COMMIT TRANSACTION",
				NULL, NULL, &errmsg);
		if (SQLITE_OK != ret) {
			ERR("sqlite3_exec(COMMIT) Failed(%d, %s)", ret, errmsg);
			sqlite3_free(errmsg);

			while (SQLITE_BUSY == ret && i<CTS_COMMIT_TRY_MAX) {
				i++;
				sleep(1);
				ret = sqlite3_exec(server_db, "COMMIT TRANSACTION",
						NULL, NULL, NULL);
			}
			if (SQLITE_OK != ret) {
				ERR("sqlite3_exec() Failed(%d)", ret);
				sqlite3_exec(server_db, "ROLLBACK TRANSACTION",
						NULL, NULL, NULL);
				return CONTACTS_ERROR_DB;
			}
		}
	}
	else {
		sqlite3_exec(server_db, "ROLLBACK TRANSACTION", NULL, NULL, NULL);
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_server_update_sort(int prev_sort_primary, int prev_sort_secondary, int new_sort_primary, int new_sort_secondary)
{
	SERVER_FN_CALL;
	int ret;
	sqlite3* db = NULL;
	char *errmsg = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	ret = ctsvc_server_db_open(&db);
	h_retvm_if(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_db_open() Failed(%d)", ret);

	ret = ctsvc_server_begin_trans();
	h_retvm_if(ret, ret, "ctsvc_server_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query), "UPDATE %s SET display_name_language=%d WHERE display_name_language =%d"	,
			CTS_TABLE_CONTACTS, prev_sort_primary, CTSVC_SORT_PRIMARY);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET display_name_language=%d WHERE display_name_language = %d"	,
			CTS_TABLE_CONTACTS, prev_sort_secondary, CTSVC_SORT_SECONDARY);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET display_name_language=%d WHERE display_name_language=%d",
				CTS_TABLE_CONTACTS, CTSVC_SORT_PRIMARY, new_sort_primary);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET display_name_language=%d WHERE display_name_language =%d"	,
			CTS_TABLE_CONTACTS, CTSVC_SORT_SECONDARY, new_sort_secondary);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET reverse_display_name_language=%d WHERE reverse_display_name_language = %d" ,
				CTS_TABLE_CONTACTS, prev_sort_primary, CTSVC_SORT_PRIMARY);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET reverse_display_name_language=%d WHERE reverse_display_name_language = %d"	,
			CTS_TABLE_CONTACTS, prev_sort_secondary, CTSVC_SORT_SECONDARY);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET reverse_display_name_language=%d WHERE reverse_display_name_language = %d",
				CTS_TABLE_CONTACTS, CTSVC_SORT_PRIMARY, new_sort_primary);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET reverse_display_name_language=%d WHERE reverse_display_name_language = %d" ,
			CTS_TABLE_CONTACTS, CTSVC_SORT_SECONDARY, new_sort_secondary);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	ret = ctsvc_server_set_default_sort(new_sort_primary);
	if (CONTACTS_ERROR_NONE != ret) {
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return ret;
	}
	ret = ctsvc_server_end_trans(true);

	if (CONTACTS_ERROR_NONE == ret) {
		int fd = open(CTSVC_NOTI_PERSON_CHANGED, O_TRUNC | O_RDWR);
		if (0 <= fd)
			close(fd);
	}
	ctsvc_server_db_close();

	return ret;
}

int ctsvc_server_insert_sdn_contact(const char *name, const char *number)
{
#if 1
	int ret;
	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	ret = ctsvc_server_db_open(&db);
	h_retvm_if(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_db_open() Failed(%d)", ret);

	snprintf(query, sizeof(query), "INSERT INTO %s(name, number) VALUES(?,?)",
			CTS_TABLE_SDN);

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if(SQLITE_OK != ret) {
		ERR("sqlite3_prepare_v2(%s) Failed(%s)", query, sqlite3_errmsg(db));
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	sqlite3_bind_text(stmt, 1, name, strlen(name), SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, number, strlen(number), SQLITE_STATIC);

	ret = sqlite3_step(stmt);
	if (SQLITE_DONE != ret) {
		ERR("sqlite3_step() Failed(%d)", ret);
		sqlite3_finalize(stmt);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}
	sqlite3_finalize(stmt);

	ctsvc_server_db_close();
#else
	contacts_record_h record;
	contacts_record_create(_contacts_sdn._uri, &record);
	contacts_record_set_str(record, _contacts_sdn.name, name);
	contacts_record_set_str(record, _contacts_sdn.number, number);
	contacts_db_insert_record(record);
	contacts_record_destroy(record, true);
#endif

	return CONTACTS_ERROR_NONE;
}

int ctsvc_server_delete_sdn_contact(void)
{
	int ret;
	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	ret = ctsvc_server_db_open(&db);
	h_retvm_if(CONTACTS_ERROR_NONE != ret, ret, "helper_db_open() Failed(%d)", ret);

	snprintf(query, sizeof(query), "DELETE FROM %s", CTS_TABLE_SDN);

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if(SQLITE_OK != ret) {
		ERR("sqlite3_prepare_v2(%s) Failed(%s)", query, sqlite3_errmsg(db));
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}
	ret = sqlite3_step(stmt);
	if (SQLITE_DONE != ret) {
		ERR("sqlite3_step() Failed(%d)", ret);
		sqlite3_finalize(stmt);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}
	sqlite3_finalize(stmt);

	ctsvc_server_db_close();
	return CONTACTS_ERROR_NONE;
}

int ctsvc_server_update_collation()
{
	int ret = 0;
	sqlite3* db = NULL;
	cts_stmt stmt = NULL;
	cts_stmt update_stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

//	char dest[CTS_SQL_MIN_LEN] = {0};

	ret = ctsvc_server_db_open(&db);
	h_retvm_if(CONTACTS_ERROR_NONE != ret, ret, "helper_db_open() Failed(%d)", ret);

	ret = ctsvc_server_begin_trans();
	if(CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_server_begin_trans() Failed(%d)", ret);
		ctsvc_server_db_close();
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT contact_id, sort_name, reverse_sort_name "
				"FROM "CTS_TABLE_CONTACTS" WHERE deleted = 0");
	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if(SQLITE_OK != ret) {
		ERR("sqlite3_prepare_v2(%s) Failed(%s)", query, sqlite3_errmsg(db));
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_CONTACTS" SET sortkey=?, reverse_sortkey=? "
				"WHERE contact_id = ?");
	ret = sqlite3_prepare_v2(db, query, strlen(query), &update_stmt, NULL);
	if(SQLITE_OK != ret) {
		ERR("sqlite3_prepare_v2(%s) Failed(%s)", query, sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	while (SQLITE_ROW == (ret = sqlite3_step(stmt))) {

		int contact_id = sqlite3_column_int(stmt, 0);
		char *sort_name = (char*)sqlite3_column_text(stmt, 1);
		char *reverse_sort_name = (char*)sqlite3_column_text(stmt, 2);
		char *sortkey = NULL;
		char *reverse_sortkey = NULL;

		if (sort_name) {
			ret = ctsvc_collation_str(sort_name, &sortkey);
			if (CONTACTS_ERROR_NONE == ret && sortkey)
				sqlite3_bind_text(update_stmt, 1, sortkey, strlen(sortkey), SQLITE_STATIC);
		}

		if (reverse_sort_name) {
			ret = ctsvc_collation_str(reverse_sort_name, &reverse_sortkey);
			if (CONTACTS_ERROR_NONE == ret && reverse_sortkey)
				sqlite3_bind_text(update_stmt, 2, reverse_sortkey, strlen(reverse_sortkey), SQLITE_STATIC);
		}

		sqlite3_bind_int(update_stmt, 3, contact_id);

		ret = sqlite3_step(update_stmt);

		free(sortkey);
		free(reverse_sortkey);

		if (SQLITE_DONE != ret) {
			ERR("sqlite3_exec(%s) Failed(%d, %s)", query, ret, sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			sqlite3_finalize(update_stmt);
			ctsvc_server_end_trans(false);
			ctsvc_server_db_close();
			return CONTACTS_ERROR_DB;
		}

		sqlite3_reset(update_stmt);

	}

	if (SQLITE_ROW != ret && SQLITE_DONE != ret) {
		ERR("sqlite3_step() Failed(%d)", ret);
		sqlite3_finalize(update_stmt);
		sqlite3_finalize(stmt);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	sqlite3_finalize(update_stmt);
	sqlite3_finalize(stmt);

	ret = ctsvc_server_end_trans(true);
	if (CONTACTS_ERROR_NONE == ret) {
		int fd = open(CTSVC_NOTI_CONTACT_CHANGED, O_TRUNC | O_RDWR);
		if (0 <= fd)
			close(fd);
	}
	ctsvc_server_db_close();

	return ret;
}

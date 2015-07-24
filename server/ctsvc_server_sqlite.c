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
#include <string.h>
#include <db-util.h>
#include <fcntl.h>

#include "ctsvc_internal.h"
#include "contacts.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_notify.h"
#include "ctsvc_setting.h"
#include "ctsvc_normalize.h"
#include "ctsvc_localize.h"
#include "ctsvc_localize_utils.h"
#include "ctsvc_localize_ch.h"
#include "ctsvc_server_utils.h"
#include "ctsvc_number_utils.h"
#include "ctsvc_server_change_subject.h"

#include "ctsvc_notification.h"
#include "ctsvc_db_query.h"
#include "ctsvc_db_plugin_image_helper.h"
#include "ctsvc_db_plugin_company_helper.h"
#include "ctsvc_db_plugin_group_helper.h"
#include "ctsvc_db_plugin_contact_helper.h"

#include "ctsvc_person.h"

#ifdef ENABLE_LOG_FEATURE
#include "ctsvc_phonelog.h"
#endif // ENABLE_LOG_FEATURE

static sqlite3 *server_db;

int ctsvc_server_db_open(sqlite3 **db)
{
	CTS_FN_CALL;
	int ret;

	if (!server_db) {
		ret = db_util_open(CTSVC_DB_PATH, &server_db, 0);
		RETVM_IF(ret != SQLITE_OK, CONTACTS_ERROR_DB,
				"db_util_open() Fail(%d)", ret);

		ret = sqlite3_create_function(server_db, "_DATA_DELETE_", 2, SQLITE_UTF8, NULL,
					ctsvc_db_data_delete_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Fail(%d)", ret);
		ret = sqlite3_create_function(server_db, "_DATA_IMAGE_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_image_delete_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Fail(%d)", ret);
		ret = sqlite3_create_function(server_db, "_DATA_COMPANY_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_company_delete_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Fail(%d)", ret);
#ifdef ENABLE_LOG_FEATURE
		ret = sqlite3_create_function(server_db, "_PHONE_LOG_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_phone_log_delete_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Fail(%d)", ret);
#endif // ENABLE_LOG_FEATURE
		ret = sqlite3_create_function(server_db, "_PERSON_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_person_delete_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Fail(%d)", ret);
		ret = sqlite3_create_function(server_db, "_GROUP_DELETE_", 1, SQLITE_UTF8, NULL,
					ctsvc_db_group_delete_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Fail(%d)", ret);
		ret = sqlite3_create_function(server_db, "_NUMBER_COMPARE_", 4, SQLITE_UTF8, NULL,
					ctsvc_db_phone_number_equal_callback, NULL, NULL);
		RETVM_IF(SQLITE_OK != ret, CONTACTS_ERROR_DB,
						"sqlite3_create_function() Fail(%d)", ret);
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
		CTS_ERR("sqlite3_exec() Fail(%d)", ret);
		return CONTACTS_ERROR_DB;
	}
	return CONTACTS_ERROR_NONE;
}

#define CTS_COMMIT_TRY_MAX 5
int ctsvc_server_end_trans(bool success)
{
	int ret = -1, i=0;
	char *errmsg = NULL;

	if (success) {
		do {
			ret = sqlite3_exec(server_db, "COMMIT TRANSACTION",
					NULL, NULL, &errmsg);
			if (SQLITE_OK != ret) {
				CTS_ERR("sqlite3_exec(COMMIT) Fail(%d, %s)", ret, errmsg);
				sqlite3_free(errmsg);
				i++;
				sleep(1);
			}
			else {
				INFO("commit end");
				return CONTACTS_ERROR_NONE;
			}
		} while ((SQLITE_BUSY == ret || SQLITE_LOCKED == ret) && i < CTS_COMMIT_TRY_MAX);

		CTS_ERR("Commit error : %d", ret);
	}

	i = 0;
	do {
		ret = sqlite3_exec(server_db, "ROLLBACK TRANSACTION",
				NULL, NULL, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("sqlite3_exec(ROLLBACK) Fail(%d, %s)", ret, errmsg);
			sqlite3_free(errmsg);
			i++;
			sleep(1);
		}
	} while ((SQLITE_BUSY == ret || SQLITE_LOCKED == ret) && i < CTS_COMMIT_TRY_MAX);

	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_exec(ROLLBACK) Fail(%d) : DB lock", ret);
		return CONTACTS_ERROR_DB;
	}
	else {
		INFO("rollback end");
		return CONTACTS_ERROR_NONE;
	}
}

int ctsvc_server_update_sort(int prev_sort_primary, int prev_sort_secondary, int new_sort_primary, int new_sort_secondary)
{
	CTS_FN_CALL;
	int ret;
	sqlite3* db = NULL;
	char *errmsg = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	ret = ctsvc_server_db_open(&db);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_db_open() Fail(%d)", ret);

	ret = ctsvc_server_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_server_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query), "UPDATE %s SET display_name_language=%d WHERE display_name_language =%d",
			CTS_TABLE_CONTACTS, prev_sort_primary, CTSVC_SORT_PRIMARY);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_exec(%s) Fail(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET display_name_language=%d WHERE display_name_language = %d",
			CTS_TABLE_CONTACTS, prev_sort_secondary, CTSVC_SORT_SECONDARY);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_exec(%s) Fail(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET display_name_language=%d WHERE display_name_language=%d",
				CTS_TABLE_CONTACTS, CTSVC_SORT_PRIMARY, new_sort_primary);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_exec(%s) Fail(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET display_name_language=%d WHERE display_name_language =%d",
			CTS_TABLE_CONTACTS, CTSVC_SORT_SECONDARY, new_sort_secondary);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_exec(%s) Fail(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET reverse_display_name_language=%d WHERE reverse_display_name_language = %d",
				CTS_TABLE_CONTACTS, prev_sort_primary, CTSVC_SORT_PRIMARY);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_exec(%s) Fail(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET reverse_display_name_language=%d WHERE reverse_display_name_language = %d",
			CTS_TABLE_CONTACTS, prev_sort_secondary, CTSVC_SORT_SECONDARY);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_exec(%s) Fail(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET reverse_display_name_language=%d WHERE reverse_display_name_language = %d",
				CTS_TABLE_CONTACTS, CTSVC_SORT_PRIMARY, new_sort_primary);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_exec(%s) Fail(%d, %s)", query, ret, errmsg);
		sqlite3_free(errmsg);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "UPDATE %s SET reverse_display_name_language=%d WHERE reverse_display_name_language = %d",
			CTS_TABLE_CONTACTS, CTSVC_SORT_SECONDARY, new_sort_secondary);
	ret = sqlite3_exec(db, query, NULL, NULL, &errmsg);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_exec(%s) Fail(%d, %s)", query, ret, errmsg);
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

	// person noti
#if 0
	if (CONTACTS_ERROR_NONE == ret) {
		int fd = open(CTSVC_NOTI_PERSON_CHANGED, O_TRUNC | O_RDWR);
		if (0 <= fd)
			close(fd);
	}
#endif
	ctsvc_server_db_close();

	return ret;
}

int ctsvc_server_insert_sdn_contact(const char *name, const char *number,
		int sim_slot_no)
{
	int ret;
	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	ret = ctsvc_server_db_open(&db);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_db_open() Fail(%d)", ret);

	snprintf(query, sizeof(query), "INSERT INTO %s(name, number, sim_slot_no) VALUES(?, ?, ?)",
			CTS_TABLE_SDN);

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_prepare_v2(%s) Fail(%s)", query, sqlite3_errmsg(db));
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}

	sqlite3_bind_text(stmt, 1, name, strlen(name), SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, number, strlen(number), SQLITE_STATIC);
	sqlite3_bind_int(stmt, 3, sim_slot_no);

	ret = sqlite3_step(stmt);
	if (SQLITE_DONE != ret) {
		CTS_ERR("sqlite3_step() Fail(%d)", ret);
		sqlite3_finalize(stmt);
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}
	sqlite3_finalize(stmt);
	ctsvc_server_db_close();

	return CONTACTS_ERROR_NONE;
}

int ctsvc_server_delete_sdn_contact(int sim_slot_no)
{
	int ret;
	sqlite3* db = NULL;
	sqlite3_stmt* stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	ret = ctsvc_server_db_open(&db);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "helper_db_open() Fail(%d)", ret);

	if (sim_slot_no < 0)
		snprintf(query, sizeof(query), "DELETE FROM %s", CTS_TABLE_SDN);
	else
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE sim_slot_no = %d", CTS_TABLE_SDN, sim_slot_no);

	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_prepare_v2(%s) Fail(%s)", query, sqlite3_errmsg(db));
		ctsvc_server_db_close();
		return CONTACTS_ERROR_DB;
	}
	ret = sqlite3_step(stmt);
	if (SQLITE_DONE != ret) {
		CTS_ERR("sqlite3_step() Fail(%d)", ret);
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

	ctsvc_db_set_status(CONTACTS_DB_STATUS_CHANGING_COLLATION);

	ret = ctsvc_server_db_open(&db);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_server_db_open() Fail(%d)", ret);
		ctsvc_db_set_status(CONTACTS_DB_STATUS_NORMAL);
		return ret;
	}

	ret = ctsvc_server_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_server_begin_trans() Fail(%d)", ret);
		ctsvc_server_db_close();
		ctsvc_db_set_status(CONTACTS_DB_STATUS_NORMAL);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT contact_id, sort_name, reverse_sort_name "
				"FROM "CTS_TABLE_CONTACTS" WHERE deleted = 0");
	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_prepare_v2(%s) Fail(%s)", query, sqlite3_errmsg(db));
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		ctsvc_db_set_status(CONTACTS_DB_STATUS_NORMAL);
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_CONTACTS" SET sortkey=?, reverse_sortkey=? "
				"WHERE contact_id = ?");
	ret = sqlite3_prepare_v2(db, query, strlen(query), &update_stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_prepare_v2(%s) Fail(%s)", query, sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		ctsvc_db_set_status(CONTACTS_DB_STATUS_NORMAL);
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
			CTS_ERR("sqlite3_step(%s) Fail(%d, %s)", query, ret, sqlite3_errmsg(db));
			sqlite3_finalize(stmt);
			sqlite3_finalize(update_stmt);
			ctsvc_server_end_trans(false);
			ctsvc_server_db_close();
			ctsvc_db_set_status(CONTACTS_DB_STATUS_NORMAL);
			return CONTACTS_ERROR_DB;
		}

		sqlite3_reset(update_stmt);

	}

	if (SQLITE_ROW != ret && SQLITE_DONE != ret) {
		CTS_ERR("sqlite3_step() Fail(%d)", ret);
		sqlite3_finalize(update_stmt);
		sqlite3_finalize(stmt);
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		ctsvc_db_set_status(CONTACTS_DB_STATUS_NORMAL);
		return CONTACTS_ERROR_DB;
	}

	sqlite3_finalize(update_stmt);
	sqlite3_finalize(stmt);

	ret = ctsvc_server_end_trans(true);
	if (CONTACTS_ERROR_NONE == ret) {
		int fd;
		fd = open(CTSVC_NOTI_CONTACT_CHANGED, O_TRUNC | O_RDWR);
		if (0 <= fd)
			close(fd);
		fd = open(CTSVC_NOTI_PERSON_CHANGED, O_TRUNC | O_RDWR);
		if (0 <= fd)
			close(fd);
	}
	ctsvc_server_db_close();

	ctsvc_db_set_status(CONTACTS_DB_STATUS_NORMAL);

	return ret;
}

int ctsvc_server_get_sim_id(const char *unique_id, int *sim_id)
{
	int ret;
	int id;
	char query[CTS_SQL_MAX_LEN] = {0};

	*sim_id = 0;
	RETVM_IF(unique_id == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "unique_id is NULL");

	snprintf(query, sizeof(query),
				"SELECT sim_id FROM "CTS_TABLE_SIM_INFO" "
								"WHERE unique_id = '%s'", unique_id);
	ret = ctsvc_query_get_first_int_result(query, &id);
	if (CONTACTS_ERROR_NONE != ret && CONTACTS_ERROR_NO_DATA != ret) {
		CTS_ERR("ctsvc_query_get_first_int_result() Fail(%d)", ret);
		return ret;
	}

	if (CONTACTS_ERROR_NO_DATA == ret) {
		snprintf(query, sizeof(query),
			"INSERT INTO "CTS_TABLE_SIM_INFO" (unique_id) VALUES('%s')", unique_id);
		ret = ctsvc_query_exec(query);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_query_exec Fail(%d)", ret);
			return ret;
		}
		id = ctsvc_db_get_last_insert_id();
	}
	CTS_DBG("id :%d, unique_id :%s", id, unique_id);
	*sim_id = id;

	return CONTACTS_ERROR_NONE;

}

static int __ctsvc_server_db_get_contact_data(sqlite3* db, int id, ctsvc_contact_s *contact)
{
	int ret;
	int datatype;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
				"SELECT datatype, id, data.contact_id, is_default, data1, data2, "
					"data3, data4, data5, data6, data7, data8, data9, data10, data11, data12 "
					"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
					"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
					"WHERE data.contact_id = %d  AND is_my_profile = 0 "
					"ORDER BY is_default DESC", id);
	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	RETVM_IF(SQLITE_OK != ret, ret, "DB error : sqlite3_prepare_v2() Fail(%d)", ret);

	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		CTS_ERR("sqlite3_step() Fail(%d)", ret);
		sqlite3_finalize(stmt);
		return ret;
	}

	do {
		datatype = ctsvc_stmt_get_int(stmt, 0);
		switch (datatype) {
		case CTSVC_DATA_NAME:
			ctsvc_get_data_info_name(stmt, (contacts_list_h)contact->name);
			break;
		case CTSVC_DATA_NICKNAME:
			ctsvc_get_data_info_nickname(stmt, (contacts_list_h)contact->nicknames);
			break;
		case CTSVC_DATA_NUMBER:
			ctsvc_get_data_info_number(stmt, (contacts_list_h)contact->numbers);
			break;
		case CTSVC_DATA_EMAIL:
			ctsvc_get_data_info_email(stmt, (contacts_list_h)contact->emails);
			break;
		case CTSVC_DATA_COMPANY:
			ctsvc_get_data_info_company(stmt, (contacts_list_h)contact->company);
			break;
		default:
			break;
		}
	} while (SQLITE_ROW == sqlite3_step(stmt));

	sqlite3_finalize(stmt);

	return SQLITE_DONE;
}

int ctsvc_server_update_sort_name()
{
	int ret = 0;
	sqlite3* db = NULL;
	cts_stmt stmt = NULL;
	cts_stmt update_stmt = NULL;
	cts_stmt search_name_stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	INFO("Start to update sort_name");

	ctsvc_db_set_status(CONTACTS_DB_STATUS_CHANGING_COLLATION);

	ret = ctsvc_server_db_open(&db);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_server_db_open() Fail(%d)", ret);
		ctsvc_db_set_status(CONTACTS_DB_STATUS_NORMAL);
		return ret;
	}

	ret = ctsvc_server_begin_trans();
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_server_begin_trans() Fail(%d)", ret);
		ctsvc_server_db_close();
		ctsvc_db_set_status(CONTACTS_DB_STATUS_NORMAL);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT contact_id "
				"FROM "CTS_TABLE_CONTACTS" WHERE deleted = 0");
	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_prepare_v2(%s) Fail(%s)", query, sqlite3_errmsg(db));
		ctsvc_server_end_trans(false);
		ctsvc_server_db_close();
		ctsvc_db_set_status(CONTACTS_DB_STATUS_NORMAL);
		return CONTACTS_ERROR_DB;
	}

	// Update sort_name, sortkey, display_name_language of contact table
	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_CONTACTS" "
				"SET sort_name = ?, reverse_sort_name = ?, sortkey = ?, reverse_sortkey = ?, "
					" display_name_language = ?,  reverse_display_name_language = ?, "
					" display_name_source = ? "
				"WHERE contact_id = ?");
	ret = sqlite3_prepare_v2(db, query, strlen(query), &update_stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_propare_v2(%s) Fail(%s)", query, sqlite3_errmsg(db));
		ret = CONTACTS_ERROR_DB;
		goto DATA_FREE;
	}

	// Update name of search_index table
	snprintf(query, sizeof(query),
			"UPDATE %s SET name=? WHERE contact_id = ?",
			CTS_TABLE_SEARCH_INDEX);
	ret = sqlite3_prepare_v2(db, query, strlen(query), &search_name_stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_propare_v2(%s) Fail(%s)", query, sqlite3_errmsg(db));
		ret = CONTACTS_ERROR_DB;
		goto DATA_FREE;
	}

	while (SQLITE_ROW == (ret = sqlite3_step(stmt))) {
		int contact_id = sqlite3_column_int(stmt, 0);
		char *search_name = NULL;

		// get_contact_info
		ctsvc_contact_s *contact = NULL;
		contacts_record_create(_contacts_contact._uri, (contacts_record_h*)&contact);
		ret = __ctsvc_server_db_get_contact_data(db, contact_id, contact);
		if (SQLITE_DONE != ret) {
			CTS_ERR("sqlite3_step(%s) Fail(%d, %s)", query, ret, sqlite3_errmsg(db));
			contacts_record_destroy((contacts_record_h)contact, true);
			ret = CONTACTS_ERROR_DB;
			goto DATA_FREE;
		}

		// update sort_name, sortkey, display_name_language(sort group)
		ctsvc_contact_make_display_name(contact);
		if (contact->sort_name)
			sqlite3_bind_text(update_stmt, 1, contact->sort_name, strlen(contact->sort_name), SQLITE_STATIC);
		if (contact->reverse_sort_name)
			sqlite3_bind_text(update_stmt, 2, contact->reverse_sort_name, strlen(contact->reverse_sort_name), SQLITE_STATIC);
		if (contact->sortkey)
			sqlite3_bind_text(update_stmt, 3, contact->sortkey, strlen(contact->sortkey), SQLITE_STATIC);
		if (contact->reverse_sortkey)
			sqlite3_bind_text(update_stmt, 4, contact->reverse_sortkey, strlen(contact->reverse_sortkey), SQLITE_STATIC);
		sqlite3_bind_int(update_stmt, 5, contact->display_name_language);
		sqlite3_bind_int(update_stmt, 6, contact->reverse_display_name_language);
		sqlite3_bind_int(update_stmt, 7, contact->display_source_type);
		sqlite3_bind_int(update_stmt, 8, contact_id);

		ret = sqlite3_step(update_stmt);
		if (SQLITE_DONE != ret) {
			CTS_ERR("sqlite3_step(%s) Fail(%d, %s)", query, ret, sqlite3_errmsg(db));
			contacts_record_destroy((contacts_record_h)contact, true);
			ret = CONTACTS_ERROR_DB;
			goto DATA_FREE;
		}
		sqlite3_reset(update_stmt);

		// update name valud of search_index table
		ctsvc_contact_make_search_name(contact, &search_name);
		if (search_name) {
			sqlite3_bind_text(search_name_stmt, 1, search_name, strlen(search_name), SQLITE_STATIC);
			sqlite3_bind_int(search_name_stmt, 2, contact_id);
			ret = sqlite3_step(search_name_stmt);
			free(search_name);
			if (SQLITE_DONE != ret) {
				CTS_ERR("sqlite3_step(%s) Fail(%d, %s)", query, ret, sqlite3_errmsg(db));
				contacts_record_destroy((contacts_record_h)contact, true);
				ret = CONTACTS_ERROR_DB;
				goto DATA_FREE;
			}
			sqlite3_reset(search_name_stmt);
		}

		contacts_record_destroy((contacts_record_h)contact, true);
	}

	if (SQLITE_ROW != ret && SQLITE_DONE != ret) {
		CTS_ERR("sqlite3_step() Fail(%d)", ret);
		ret = CONTACTS_ERROR_DB;
		goto DATA_FREE;
	}

DATA_FREE:
	if (stmt)
		sqlite3_finalize(stmt);
	if (update_stmt)
		sqlite3_finalize(update_stmt);
	if (search_name_stmt)
		sqlite3_finalize(search_name_stmt);

	// send notification
	if (CONTACTS_ERROR_DB != ret) {
		ret = ctsvc_server_end_trans(true);
		if (CONTACTS_ERROR_NONE == ret) {
			int fd;
			fd = open(CTSVC_NOTI_CONTACT_CHANGED, O_TRUNC | O_RDWR);
			if (0 <= fd)
				close(fd);
			fd = open(CTSVC_NOTI_PERSON_CHANGED, O_TRUNC | O_RDWR);
			if (0 <= fd)
				close(fd);
		}
	}
	else
		ctsvc_server_end_trans(false);

	ctsvc_server_db_close();
	ctsvc_db_set_status(CONTACTS_DB_STATUS_NORMAL);

	INFO("End to update sort_name");

	return ret;
}


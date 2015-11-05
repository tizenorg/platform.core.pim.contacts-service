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
#include <sqlite3.h>
#include <db-util.h>

#include "contacts.h"

#include "ctsvc_internal.h"
#include "ctsvc_db_schema.h"
#include "ctsvc_db_sqlite.h"
#include "ctsvc_server_setting.h"
#include "ctsvc_number_utils.h"
#include "ctsvc_localize_utils.h"
#include "ctsvc_server_update.h"

/*
 * It should be updated whenever DB schema including VIEW query is changed
 * You have to update user version schema.sql
 *			PRAGMA user_version = 100;
 */
#define CTSVC_SCHEMA_VERSION 102

#ifdef ENABLE_LOG_FEATURE
static int __ctsvc_server_find_person_id_of_phonelog(sqlite3 *__db, char *normal_num,
		char *minmatch, int person_id, int *find_number_type)
{
	int ret;
	int find_person_id = -1;
	char query[CTS_SQL_MAX_LEN] = {0};
	cts_stmt stmt = NULL;
	int id;
	int number_type = -1;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	int i = 0;

	*find_number_type = -1;

	ret = snprintf(query, sizeof(query),
			"SELECT person_id, data1 FROM "CTS_TABLE_CONTACTS", "CTS_TABLE_DATA" "
			"ON "CTS_TABLE_CONTACTS".contact_id = "CTS_TABLE_DATA".contact_id "
			"AND datatype = %d AND is_my_profile = 0 AND deleted = 0 "
			"WHERE data4 = ? AND _NUMBER_COMPARE_(data5, ?, NULL, NULL)",
			CTSVC_DATA_NUMBER);

	bind_text = g_slist_append(bind_text, strdup(minmatch));
	bind_text = g_slist_append(bind_text, strdup(normal_num));

	ret = sqlite3_prepare_v2(__db, query, sizeof(query), &stmt, NULL);
	if (stmt == NULL) {
		CTS_ERR("sqlite3_prepare_v2 fail(%d)", ret);
		if (bind_text) {
			for (cursor=bind_text;cursor;cursor=cursor->next,i++)
				free(cursor->data);
			g_slist_free(bind_text);
		}
		return CONTACTS_ERROR_DB;
	}

	if (bind_text) {
		for (cursor=bind_text,i=1;cursor;cursor=cursor->next,i++) {
			const char *text = cursor->data;
			if (text && *text)
				sqlite3_bind_text(stmt, i, text, strlen(text), SQLITE_STATIC);
		}
	}

	while ((ret = sqlite3_step(stmt))) {
		id = sqlite3_column_int(stmt, 0);
		number_type = sqlite3_column_int(stmt, 1);
		if (find_person_id <= 0 && id > 0) {
			find_person_id = id;	/* find first match person_id */
			*find_number_type = number_type;
			if (person_id <= 0)
				break;
		}

		if (id == person_id) {
			find_person_id = person_id;
			*find_number_type = number_type;
			break;
		}
	}
	sqlite3_finalize(stmt);

	if (bind_text) {
		for (cursor=bind_text;cursor;cursor=cursor->next,i++)
			free(cursor->data);
		g_slist_free(bind_text);
	}

	return find_person_id;
}
#endif /* ENABLE_LOG_FEATURE */

static void __ctsvc_server_number_info_update(sqlite3 *__db)
{
	int ret;
	cts_stmt stmt = NULL;
	cts_stmt update_stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

#ifdef ENABLE_LOG_FEATURE
	/* update number of phonelog table */
	ret = snprintf(query, sizeof(query),
			"SELECT id, number, person_id FROM "CTS_TABLE_PHONELOGS " "
			"WHERE log_type < %d", CONTACTS_PLOG_TYPE_EMAIL_RECEIVED);
	ret = sqlite3_prepare_v2(__db, query, strlen(query), &stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_prepare_v2() Fail(%s)", sqlite3_errmsg(__db));
		return;
	}

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_PHONELOGS" SET normal_num=?, clean_num=?, "
			"minmatch=?, person_id=?, number_type=? WHERE id = ?");
	ret = sqlite3_prepare_v2(__db, query, strlen(query), &update_stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_prepare_v2() Fail(%s)", sqlite3_errmsg(__db));
		sqlite3_finalize(stmt);
		return;
	}

	while (SQLITE_ROW == (ret = sqlite3_step(stmt))) {
		int phonelog_id = sqlite3_column_int(stmt, 0);
		char *number = (char*)sqlite3_column_text(stmt, 1);
		int person_id = sqlite3_column_int(stmt, 2);
		char clean_num[SAFE_STRLEN(number)+1];
		int find_person_id = -1;
		int number_type = -1;

		ret = ctsvc_clean_number(number, clean_num, sizeof(clean_num), true);
		if (0 < ret) {
			char normal_num[sizeof(clean_num) + 20];
			sqlite3_bind_text(update_stmt, 2, clean_num, strlen(clean_num), SQLITE_STATIC);
			ret = ctsvc_normalize_number(clean_num, normal_num, sizeof(normal_num), true);
			char minmatch[sizeof(normal_num) +1];
			if (0 < ret) {
				sqlite3_bind_text(update_stmt, 1, normal_num, strlen(normal_num), SQLITE_STATIC);
				ret = ctsvc_get_minmatch_number(normal_num, minmatch, sizeof(minmatch), ctsvc_get_phonenumber_min_match_digit());
				if (CONTACTS_ERROR_NONE == ret) {
					sqlite3_bind_text(update_stmt, 3, minmatch, strlen(minmatch), SQLITE_STATIC);
					find_person_id  = __ctsvc_server_find_person_id_of_phonelog(__db, normal_num, minmatch, person_id, &number_type);
				}

				if (0 < find_person_id)
					sqlite3_bind_int(update_stmt, 4, find_person_id);
				if (0 <= number_type)
					sqlite3_bind_int(update_stmt, 5, number_type);
			}
			sqlite3_bind_int(update_stmt, 6, phonelog_id);

			ret = sqlite3_step(update_stmt);
			if (SQLITE_DONE != ret) {
				CTS_ERR("sqlite3_step() Fail(%d, %s)", ret, sqlite3_errmsg(__db));
				break;
			}
			sqlite3_reset(update_stmt);
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_finalize(update_stmt);
#endif /* ENABLE_LOG_FEATURE */

	/* update number of data table */
	snprintf(query, sizeof(query),
			"SELECT id, data3 FROM "CTS_TABLE_DATA" "
			"WHERE datatype = %d", CTSVC_DATA_NUMBER);
	ret = sqlite3_prepare_v2(__db, query, strlen(query), &stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_prepare_v2() Fail(%s)", sqlite3_errmsg(__db));
		return;
	}

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_DATA" SET data4=?, data5=?, data6=?  WHERE id = ?");
	ret = sqlite3_prepare_v2(__db, query, strlen(query), &update_stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_prepare_v2() Fail(%s)", sqlite3_errmsg(__db));
		sqlite3_finalize(stmt);
		return;
	}

	while (SQLITE_ROW == (ret = sqlite3_step(stmt))) {
		int id = sqlite3_column_int(stmt, 0);
		char *number = (char*)sqlite3_column_text(stmt, 1);
		char clean_num[SAFE_STRLEN(number)+1];

		ret = ctsvc_clean_number(number, clean_num, sizeof(clean_num), true);
		if (0 < ret) {
			char normal_num[sizeof(clean_num)+20];
			sqlite3_bind_text(update_stmt, 3, clean_num, strlen(clean_num), SQLITE_STATIC);
			ret = ctsvc_normalize_number(clean_num, normal_num, sizeof(normal_num), true);
			char minmatch[sizeof(normal_num)+1];
			if (0 < ret) {
				sqlite3_bind_text(update_stmt, 2, normal_num, strlen(normal_num), SQLITE_STATIC);
				ret = ctsvc_get_minmatch_number(normal_num, minmatch, sizeof(minmatch), ctsvc_get_phonenumber_min_match_digit());
				if (CONTACTS_ERROR_NONE == ret)
					ctsvc_stmt_bind_text(update_stmt, 1, minmatch);
			}
			sqlite3_bind_int(update_stmt, 4, id);
			ret = sqlite3_step(update_stmt);
			if (SQLITE_DONE != ret) {
				CTS_ERR("sqlite3_step() Fail(%d, %s)", ret, sqlite3_errmsg(__db));
				break;
			}
			sqlite3_reset(update_stmt);
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_finalize(update_stmt);

	return;
}

static int __ctsvc_server_get_db_version(sqlite3 *db, int *version)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};
	cts_stmt stmt = NULL;

	*version = 0;

	snprintf(query, sizeof(query), "PRAGMA user_version;");
	ret = sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_prepare_v2() Fail(%s)", sqlite3_errmsg(db));
		return CONTACTS_ERROR_DB;
	}
	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		CTS_ERR("sqlite3_step() Fail(%s)", sqlite3_errmsg(db));
		sqlite3_finalize(stmt);
		return CONTACTS_ERROR_DB;
	}
	*version = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);
	return CONTACTS_ERROR_NONE;
}

int ctsvc_server_db_update(void)
{
	int ret;
	int old_version = 0;
	char *errmsg = NULL;
	sqlite3 *__db;
	char query[CTS_SQL_MIN_LEN] = {0};

	ret = db_util_open(CTSVC_DB_PATH, &__db, 0);
	RETVM_IF(ret != SQLITE_OK, CONTACTS_ERROR_DB,
			"db_util_open() Fail(%d)", ret);

	/* check DB schema version */
	__ctsvc_server_get_db_version(__db, &old_version);

	/* Tizen 2.3a Releae 0.9.114.7 ('14/5)  ----------------------------------- */
	if (old_version <= 100) {
		ret = sqlite3_exec(__db, "CREATE INDEX name_lookup_idx1 ON name_lookup(contact_id);", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("drop view view_person_contact_group Fail(%d) : %s", ret, errmsg);
			sqlite3_free(errmsg);
		}

		ret = sqlite3_exec(__db, "CREATE INDEX phone_lookup_idx1 ON phone_lookup(contact_id);", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("drop view view_person_contact_group Fail(%d) : %s", ret, errmsg);
			sqlite3_free(errmsg);
		}

		/* add view_activity_photos */
		ret = sqlite3_exec(__db, "DROP VIEW view_phonelog_number", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("drop view view_person_contact_group Fail(%d) : %s", ret, errmsg);
			sqlite3_free(errmsg);
		}

		/* add view_person_contact_group_assigned */
		/* add view_person_contact_group_not_assigned */
		/* change DB VIEW view_person_contact_group for performance */
		ret = sqlite3_exec(__db, "DROP VIEW "CTSVC_DB_VIEW_PERSON_GROUP, NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("drop view view_person_contact_group Fail(%d) : %s", ret, errmsg);
			sqlite3_free(errmsg);
		}

		/* change DB VIEW view_contact_group for performance */
		ret = sqlite3_exec(__db, "DROP VIEW "CTSVC_DB_VIEW_CONTACT_GROUP, NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("drop view view_contact_group Fail : %s", errmsg);
			sqlite3_free(errmsg);
		}

		/* for number compare */
		ret = sqlite3_exec(__db, "DROP VIEW "CTSVC_DB_VIEW_NUMBER, NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("drop view %s Fail(%d) : %s", CTSVC_DB_VIEW_NUMBER, ret, errmsg);
			sqlite3_free(errmsg);
		}

		ret = sqlite3_exec(__db, "DROP VIEW "CTSVC_DB_VIEW_SPEEDIDAL, NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("drop view %s Fail(%d) : %s", CTSVC_DB_VIEW_SPEEDIDAL, ret, errmsg);
			sqlite3_free(errmsg);
		}

		ret = sqlite3_exec(__db, "DROP VIEW "CTSVC_DB_VIEW_PERSON_NUMBER, NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("drop view %s Fail(%d) : %s", CTSVC_DB_VIEW_PERSON_NUMBER, ret, errmsg);
			sqlite3_free(errmsg);
		}

		ret = sqlite3_exec(__db, "DROP VIEW "CTSVC_DB_VIEW_CONTACT_NUMBER, NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("drop view %s Fail(%d) : %s", CTSVC_DB_VIEW_CONTACT_NUMBER, ret, errmsg);
			sqlite3_free(errmsg);
		}

		ret = sqlite3_exec(__db, "DROP VIEW "CTSVC_DB_VIEW_PERSON_PHONELOG, NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("drop view %s Fail(%d) : %s", CTSVC_DB_VIEW_PERSON_PHONELOG, ret, errmsg);
			sqlite3_free(errmsg);
		}

		ret = sqlite3_exec(__db, "ALTER TABLE "CTS_TABLE_PHONELOGS" ADD COLUMN clean_num TEXT", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("add phonelogs.clean_num Fail(%d) : %s", ret, errmsg);
			sqlite3_free(errmsg);
		}

		ret = sqlite3_exec(__db, "ALTER TABLE "CTS_TABLE_PHONELOGS" ADD COLUMN sim_id TEXT", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("add phonelogs.sim_id Fail(%d) : %s", ret, errmsg);
			sqlite3_free(errmsg);
		}

		ret = sqlite3_exec(__db, "ALTER TABLE "CTS_TABLE_PHONELOGS" ADD COLUMN number_type TEXT", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("add phonelogs.number_type Fail(%d) : %s", ret, errmsg);
			sqlite3_free(errmsg);
		}

		/* update phonelog, data number value */
		__ctsvc_server_number_info_update(__db);

		ret = sqlite3_exec(__db, "ALTER TABLE "CTS_TABLE_SDN" ADD COLUMN sim_slot_no TEXT", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("add sdn.sim_id Fail(%d) : %s", ret, errmsg);
			sqlite3_free(errmsg);
		}

		ret = sqlite3_exec(__db, "ALTER TABLE "CTS_TABLE_ADDRESSBOOKS" ADD COLUMN smack_label TEXT", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("add sdn.sim_id Fail(%d) : %s", ret, errmsg);
			sqlite3_free(errmsg);
		}
		ret = sqlite3_exec(__db, "UPDATE "CTS_TABLE_ADDRESSBOOKS" SET='org.tizen.contact' WHERE addressbook_id = 0", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("add sdn.sim_id Fail(%d) : %s", ret, errmsg);
			sqlite3_free(errmsg);
		}

		snprintf(query, sizeof(query),
				"CREATE TABLE IF NOT EXISTS "CTS_TABLE_SIM_INFO" ("
				"sim_id			INTEGER PRIMARY KEY AUTOINCREMENT,"
				"unique_id		TEXT NOT NULL, "
				"UNIQUE(unique_id))");
		ret = sqlite3_exec(__db, query, NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("create sim_info table(%d)", ret);
			sqlite3_free(errmsg);
		}

		ret = sqlite3_exec(__db, "DROP trigger trg_contacts_update", NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("drop trigger trg_contacts_update Fail(%d) : %s", ret, errmsg);
			sqlite3_free(errmsg);
		}
		ret = sqlite3_exec(__db,
				"CREATE TRIGGER trg_contacts_update AFTER UPDATE ON contacts "
				"WHEN new.deleted = 1 "
				" BEGIN "
				"	SELECT _DATA_DELETE_(data.id, data.datatype) FROM data WHERE contact_id = old.contact_id AND is_my_profile = 0;"
				"	DELETE FROM group_relations WHERE old.addressbook_id != -1 AND contact_id = old.contact_id; "
				"	DELETE FROM persons WHERE person_id = old.person_id AND link_count = 1; "
				"	UPDATE persons SET dirty=1 WHERE person_id = old.person_id AND link_count > 1; "
				"	DELETE FROM speeddials WHERE number_id IN (SELECT id FROM data WHERE data.contact_id = old.contact_id AND datatype = 8); "
				" END;",
				NULL, 0, &errmsg);
		if (SQLITE_OK != ret) {
			CTS_ERR("create trigger trg_contacts_update Fail(%d) : %s", ret, errmsg);
			sqlite3_free(errmsg);
		}

		old_version = 101;
	}

	if (old_version <= 101) {
		/* update phonelog, data number value */
		__ctsvc_server_number_info_update(__db);

		old_version = 102;
	}

	snprintf(query, sizeof(query),
			"PRAGMA user_version = %d", CTSVC_SCHEMA_VERSION);
	ret = sqlite3_exec(__db, query, NULL, 0, &errmsg);
	if (SQLITE_OK != ret) {
		CTS_ERR("sqlite3_exec() Fail(%s)", errmsg);
		sqlite3_free(errmsg);
	}

	db_util_close(__db);
	return CONTACTS_ERROR_NONE;
}


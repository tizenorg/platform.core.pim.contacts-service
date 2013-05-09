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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_normalize.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_query.h"
#include "ctsvc_db_plugin_name_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_setting.h"
#include "ctsvc_notification.h"

enum{
	CTSVC_NN_FIRST,
	CTSVC_NN_LAST,
	CTSVC_NN_MAX,
};

static inline void __ctsvc_make_name_lookup(int op_code, const char *name_first,
		const char *name_last, char **name_lookup)
{
	if (name_first && !name_last)
		*name_lookup = SAFE_STRDUP(name_first);
	else if (!name_first && name_last)
		*name_lookup = SAFE_STRDUP(name_last);
	else {
		if (CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST == op_code) {
			*name_lookup = calloc(1, SAFE_STRLEN(name_first) + SAFE_STRLEN(name_last) + 3);
			snprintf(*name_lookup, SAFE_STRLEN(name_first) + SAFE_STRLEN(name_last) + 3, "%s %c%s",
					SAFE_STR(name_first), 0x7E, SAFE_STR(name_last));
		}
		else {
			*name_lookup = calloc(1, SAFE_STRLEN(name_first) + SAFE_STRLEN(name_last) + 5);
			snprintf(*name_lookup, SAFE_STRLEN(name_first) + SAFE_STRLEN(name_last) + 5, "%s,%c %c%s",
					SAFE_STR(name_last), 0x7E, 0x7E, SAFE_STR(name_first));
		}
	}
}

static inline int __ctsvc_name_bind_stmt(cts_stmt stmt, ctsvc_name_s *name, int start_cnt)
{
	cts_stmt_bind_int(stmt, start_cnt, name->is_default);
	cts_stmt_bind_int(stmt, start_cnt+1, name->language_type);
	if (name->first)
		sqlite3_bind_text(stmt, start_cnt+2, name->first,
				strlen(name->first), SQLITE_STATIC);
	if (name->last)
		sqlite3_bind_text(stmt, start_cnt+3, name->last,
				strlen(name->last), SQLITE_STATIC);
	if (name->addition)
		sqlite3_bind_text(stmt, start_cnt+4, name->addition,
				strlen(name->addition), SQLITE_STATIC);
	if (name->prefix)
		sqlite3_bind_text(stmt, start_cnt+5, name->prefix,
				strlen(name->prefix), SQLITE_STATIC);
	if (name->suffix)
		sqlite3_bind_text(stmt, start_cnt+6, name->suffix,
				strlen(name->suffix), SQLITE_STATIC);
	if (name->phonetic_first)
		sqlite3_bind_text(stmt, start_cnt+7, name->phonetic_first,
				strlen(name->phonetic_first), SQLITE_STATIC);
	if (name->phonetic_middle)
		sqlite3_bind_text(stmt, start_cnt+8, name->phonetic_middle,
				strlen(name->phonetic_middle), SQLITE_STATIC);
	if (name->phonetic_last)
		sqlite3_bind_text(stmt, start_cnt+9, name->phonetic_last,
				strlen(name->phonetic_last), SQLITE_STATIC);
	if (name->lookup)
		sqlite3_bind_text(stmt, start_cnt+10, name->lookup,
				strlen(name->lookup), SQLITE_STATIC);
	if (name->reverse_lookup)
		sqlite3_bind_text(stmt, start_cnt+11, name->reverse_lookup,
				strlen(name->reverse_lookup), SQLITE_STATIC);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_normalize_name(ctsvc_name_s *src, char *dest[])
{
	int ret = CONTACTS_ERROR_NO_DATA;
	int language_type = 0;

	if (src->first) {
		ret = ctsvc_normalize_str(src->first, &dest[CTSVC_NN_FIRST]);
		RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "_cts_normalize_str() Failed(%d)", ret);
		language_type = ret;
	}

	if (src->last) {
		ret = ctsvc_normalize_str(src->last, &dest[CTSVC_NN_LAST]);
		RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "_cts_normalize_str() Failed(%d)", ret);
		if (language_type < ret)
			language_type = ret;
	}
	return language_type;
}

int ctsvc_db_name_insert(contacts_record_h record, int contact_id, bool is_my_profile, int *id)
{
	int ret, len = 0;
	cts_stmt stmt = NULL;
	ctsvc_name_s *name = (ctsvc_name_s*)record;
	char query[CTS_SQL_MAX_LEN]={0};
	char *normal_name[CTSVC_NN_MAX]={NULL};	//insert name search info
	char *temp_normal_first = NULL;
	char *temp_normal_last = NULL;

	RETV_IF(NULL == name, CONTACTS_ERROR_INVALID_PARAMETER);

	RETVM_IF(contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : contact_id(%d) is mandatory field to insert name record ", name->contact_id);
	RETVM_IF(0 < name->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : id(%d), This record is already inserted", name->id);

	if (name->first || name->last || name->addition || name->prefix || name->suffix
		|| name->phonetic_first || name->phonetic_middle || name->phonetic_last) {
		snprintf(query, sizeof(query),
			"INSERT INTO "CTS_TABLE_DATA"(contact_id, is_my_profile, datatype, is_default, data1, data2, data3, "
						"data4, data5, data6, data7, data8, data9, data10, data11, data12) "
						"VALUES(%d, %d, %d, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
				contact_id, is_my_profile, CTSVC_DATA_NAME);

		stmt = cts_query_prepare(query);
		RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

		ret = __ctsvc_normalize_name(name, normal_name);
		if (ret < CONTACTS_ERROR_NONE) {
			CTS_ERR("__ctsvc_normalize_name() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}

		switch (ret) {
		case CTSVC_LANG_KOREAN:
			temp_normal_first = calloc(1, SAFE_STRLEN(normal_name[CTSVC_NN_LAST]) +  SAFE_STRLEN(normal_name[CTSVC_NN_LAST]) + 1);
			if (normal_name[CTSVC_NN_LAST]) {
				len = snprintf(temp_normal_first, SAFE_STRLEN(normal_name[CTSVC_NN_LAST]) +  SAFE_STRLEN(normal_name[CTSVC_NN_LAST]) + 1,
					"%s", normal_name[CTSVC_NN_LAST]);
			}
			if (normal_name[CTSVC_NN_FIRST]) {
				snprintf(temp_normal_first+len, SAFE_STRLEN(normal_name[CTSVC_NN_LAST]) +  SAFE_STRLEN(normal_name[CTSVC_NN_LAST]) + 1 - len,
					"%s", normal_name[CTSVC_NN_FIRST]);
			}
			temp_normal_last = NULL;
			break;
		case CTSVC_LANG_ENGLISH:
		default:
			if (normal_name[CTSVC_NN_FIRST] && normal_name[CTSVC_NN_FIRST][0])
				temp_normal_first = SAFE_STRDUP(normal_name[CTSVC_NN_FIRST]);

			if (normal_name[CTSVC_NN_LAST] && normal_name[CTSVC_NN_LAST][0])
				temp_normal_last = SAFE_STRDUP(normal_name[CTSVC_NN_LAST]);

			break;
		}


		if (ctsvc_get_primary_sort() == ret)
			name->language_type = CTSVC_LANG_DEFAULT;
		else if (ctsvc_get_secondary_sort() == ret)
			name->language_type = CTSVC_LANG_SECONDARY;
		else
			name->language_type = ret;


		__ctsvc_make_name_lookup(CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST,
									temp_normal_first, temp_normal_last, &name->lookup);

		__ctsvc_make_name_lookup(CONTACTS_NAME_DISPLAY_ORDER_LASTFIRST,
									temp_normal_first, temp_normal_last, &name->reverse_lookup);

		free(temp_normal_first);
		free(temp_normal_last);
		free(normal_name[CTSVC_NN_FIRST]);
		free(normal_name[CTSVC_NN_LAST]);

		__ctsvc_name_bind_stmt(stmt, name, 1);

		ret = cts_stmt_step(stmt);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}

		//name->id = cts_db_get_last_insert_id();
		if (id)
			*id = cts_db_get_last_insert_id();
		name->contact_id = contact_id;
		cts_stmt_finalize(stmt);

		if (!is_my_profile)
			ctsvc_set_name_noti();
	}


	// update search index table
	return CONTACTS_ERROR_NONE;
}


int ctsvc_db_name_get_value_from_stmt(cts_stmt stmt, contacts_record_h *record, int start_count)
{
	int ret;
	char *temp;
	ctsvc_name_s *name;

	ret = contacts_record_create(_contacts_name._uri, (contacts_record_h *)&name);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create is failed(%d)", ret);

	name->id = ctsvc_stmt_get_int(stmt, start_count++);
	name->contact_id = ctsvc_stmt_get_int(stmt, start_count++);
	name->is_default = ctsvc_stmt_get_int(stmt, start_count++);
	name->language_type = ctsvc_stmt_get_int(stmt, start_count++);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	name->first = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	name->last = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	name->addition = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	name->prefix = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	name->suffix = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	name->phonetic_first = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	name->phonetic_middle = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	name->phonetic_last = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	name->lookup = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	name->reverse_lookup = SAFE_STRDUP(temp);

	*record = (contacts_record_h)name;

	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_name_update(contacts_record_h record, bool is_my_profile)
{
	int ret, len=0;
	int id;
	char* set = NULL;
	GSList *cursor = NULL;
	GSList *bind_text = NULL;
	ctsvc_name_s *name = (ctsvc_name_s*)record;
	char *tmp_first, *tmp_last;
	char *normal_name[CTSVC_NN_MAX] = {NULL};
	char *temp_normal_first = NULL;
	char *temp_normal_last = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	RETVM_IF(!name->id, CONTACTS_ERROR_INVALID_PARAMETER, "name of contact has no ID.");
	RETVM_IF(CTSVC_PROPERTY_FLAG_DIRTY != (name->base.property_flag & CTSVC_PROPERTY_FLAG_DIRTY), CONTACTS_ERROR_NONE, "No update");

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_DATA" WHERE id = %d", name->id);
	ret = ctsvc_query_get_first_int_result(query, &id);
	RETV_IF(ret != CONTACTS_ERROR_NONE, ret);

	tmp_first = name->first;
	tmp_last = name->last;

	ret = __ctsvc_normalize_name(name, normal_name);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("cts_normalize_name() Failed(%d)", ret);
		return ret;
	}

	switch (ret) {
	case CTSVC_LANG_KOREAN:
		temp_normal_first = calloc(1, SAFE_STRLEN(normal_name[CTSVC_NN_LAST]) +  SAFE_STRLEN(normal_name[CTSVC_NN_LAST]) + 1);
		if (normal_name[CTSVC_NN_LAST]) {
			len = snprintf(temp_normal_first, SAFE_STRLEN(normal_name[CTSVC_NN_LAST]) +  SAFE_STRLEN(normal_name[CTSVC_NN_LAST]) + 1,
				"%s", normal_name[CTSVC_NN_LAST]);
		}
		if (normal_name[CTSVC_NN_FIRST]) {
			snprintf(temp_normal_first+len, SAFE_STRLEN(normal_name[CTSVC_NN_LAST]) +  SAFE_STRLEN(normal_name[CTSVC_NN_LAST]) + 1 - len,
				"%s", normal_name[CTSVC_NN_FIRST]);
		}
		temp_normal_last = NULL;
		break;
	case CTSVC_LANG_ENGLISH:
	default:
		if (normal_name[CTSVC_NN_FIRST] && normal_name[CTSVC_NN_FIRST][0])
			temp_normal_first = normal_name[CTSVC_NN_FIRST];
		else
			name->first = NULL;

		if (normal_name[CTSVC_NN_LAST] && normal_name[CTSVC_NN_LAST][0])
			temp_normal_last = normal_name[CTSVC_NN_LAST];
		else
			name->last = NULL;
		break;
	}

	if (ctsvc_get_primary_sort() == ret)
		name->language_type = CTSVC_LANG_DEFAULT;
	else if (ctsvc_get_secondary_sort() == ret)
			name->language_type = CTSVC_LANG_SECONDARY;
	else
		name->language_type = ret;

	name->first = tmp_first;
	name->last = tmp_last;

	__ctsvc_make_name_lookup(CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST,
								temp_normal_first, temp_normal_last, &name->lookup);

	__ctsvc_make_name_lookup(CONTACTS_NAME_DISPLAY_ORDER_LASTFIRST,
								temp_normal_first, temp_normal_last, &name->reverse_lookup);

	free(normal_name[CTSVC_NN_FIRST]);
	free(normal_name[CTSVC_NN_LAST]);

	do {
		char query_set[CTS_SQL_MAX_LEN] = {0};
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_create_set_query(record, &set, &bind_text))) break;
		snprintf(query_set, sizeof(query_set), "%s, is_default=%d, data1=%d, data11=?, data12=?",
				set, name->is_default, name->language_type);
		bind_text = g_slist_append(bind_text, strdup(SAFE_STR(name->lookup)));
		bind_text = g_slist_append(bind_text, strdup(SAFE_STR(name->reverse_lookup)));
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_update_record_with_set_query(query_set, bind_text, CTS_TABLE_DATA, name->id))) break;

		if (!is_my_profile)
			ctsvc_set_name_noti();
	} while (0);

	CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s *)record);
	CONTACTS_FREE(set);
	if (bind_text) {
		for (cursor=bind_text;cursor;cursor=cursor->next)
			CONTACTS_FREE(cursor->data);
		g_slist_free(bind_text);
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_name_delete(int id, bool is_my_profile)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_DATA" WHERE id = %d", id);

	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Failed(%d)", ret);

	if (!is_my_profile)
		ctsvc_set_name_noti();

	return CONTACTS_ERROR_NONE;
}


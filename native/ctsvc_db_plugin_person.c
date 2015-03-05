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
#include "ctsvc_db_plugin_person_helper.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_utils.h"
#include "ctsvc_list.h"
#include "ctsvc_db_query.h"
#include "ctsvc_record.h"
#include "ctsvc_normalize.h"
#include "ctsvc_notification.h"

#ifdef _CONTACTS_IPC_SERVER
#include "ctsvc_server_change_subject.h"
#endif

static int __ctsvc_db_person_insert_record( contacts_record_h record, int *id );
static int __ctsvc_db_person_get_record( int id, contacts_record_h* out_record );
static int __ctsvc_db_person_update_record( contacts_record_h record );
static int __ctsvc_db_person_delete_record( int id );
static int __ctsvc_db_person_get_all_records( int offset, int limit, contacts_list_h* out_list );
static int __ctsvc_db_person_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list );
//static int __ctsvc_db_person_insert_records(const contacts_list_h in_list, int **ids);
//static int __ctsvc_db_person_update_records(const contacts_list_h in_list);
//static int __ctsvc_db_person_delete_records(int ids[], int count);

ctsvc_db_plugin_info_s ctsvc_db_plugin_person = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_person_insert_record,
	.get_record = __ctsvc_db_person_get_record,
	.update_record = __ctsvc_db_person_update_record,
	.delete_record = __ctsvc_db_person_delete_record,
	.get_all_records = __ctsvc_db_person_get_all_records,
	.get_records_with_query = __ctsvc_db_person_get_records_with_query,
	.insert_records = NULL,//__ctsvc_db_person_insert_records,
	.update_records = NULL,//__ctsvc_db_person_update_records,
	.delete_records = NULL,//__ctsvc_db_person_delete_records
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};

static int __ctsvc_db_person_insert_record( contacts_record_h record, int *id )
{
	CTS_ERR("Can not insert person record directly");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_db_person_get_record( int id, contacts_record_h* out_record )
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_record_h record;

	*out_record = NULL;

	snprintf(query, sizeof(query),
		"SELECT persons.person_id, "
				"%s, "
				"_NORMALIZE_INDEX_(%s), "
				"name_contact_id, "
				"persons.image_thumbnail_path, "
				"persons.ringtone_path, "
				"persons.vibration, "
				"persons.message_alert, "
				"status, "
				"link_count, "
				"addressbook_ids, "
				"persons.has_phonenumber, "
				"persons.has_email, "
				"EXISTS(SELECT person_id FROM "CTS_TABLE_FAVORITES" WHERE person_id=persons.person_id) is_favorite "
			"FROM "CTS_TABLE_PERSONS" "
			"LEFT JOIN "CTS_TABLE_CONTACTS" "
			"ON (name_contact_id = contacts.contact_id AND contacts.deleted = 0) "
			"WHERE persons.person_id = %d",
			ctsvc_get_display_column(), ctsvc_get_sort_name_column(), id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Failed(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if( 1 /*CTS_TRUE*/ != ret) {
		CTS_ERR("ctsvc_stmt_step() Failed(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}
	ret = ctsvc_db_person_create_record_from_stmt(stmt, &record);
	ctsvc_stmt_finalize(stmt);

	if(CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_db_person_create_record_from_stmt() Failed(%d)", ret);
		return ret;
	}

	*out_record = record;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_person_update_record( contacts_record_h record )
{
	int ret, i, len;
	int person_id;
	cts_stmt stmt = NULL;
	char *set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	char contact_query[CTS_SQL_MIN_LEN] = {0};
	char query[CTS_SQL_MIN_LEN] = {0};
	ctsvc_person_s *person = (ctsvc_person_s *)record;
	const char *display_name = NULL;
	int index_favorite = 0;

	RETV_IF(NULL == person, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(person->person_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT person_id FROM "CTS_TABLE_PERSONS" WHERE person_id = %d", person->person_id);
	ret = ctsvc_query_get_first_int_result(query, &person_id);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_query_get_first_int_result Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (ctsvc_record_check_property_flag((ctsvc_record_s *)person, _contacts_person.display_contact_id, CTSVC_PROPERTY_FLAG_DIRTY)) {
		// check name_contact_id validation
		char *temp;
		char check_query[CTS_SQL_MIN_LEN] = {0};
		snprintf(check_query, sizeof(check_query), "SELECT contact_id, %s FROM "CTS_TABLE_CONTACTS
				" WHERE person_id = %d AND contact_id = %d AND deleted = 0",
				ctsvc_get_display_column(), person->person_id, person->name_contact_id);
		ret = ctsvc_query_prepare(check_query, &stmt);
		if (NULL == stmt) {
			CTS_ERR("ctsvc_query_prepare() Failed(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}

		ret = ctsvc_stmt_step(stmt);
		if ( 1 != ret) {
			if ( CONTACTS_ERROR_NONE == ret) {
				CTS_ERR("Invalid parameter : the name_contact_id(%d) is not linked with person_id(%d)",
					person->name_contact_id, person->person_id);
				ctsvc_stmt_finalize(stmt);
				ctsvc_end_trans(false);
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}
			else {
				CTS_ERR("ctsvc_stmt_step() Failed(%d)", ret);
				ctsvc_stmt_finalize(stmt);
				ctsvc_end_trans(false);
				return ret;
			}
		}
		temp = ctsvc_stmt_get_text(stmt, 0);
		display_name = SAFE_STRDUP(temp);

		ctsvc_stmt_finalize(stmt);
	}

	// update favorite
	index_favorite = CTSVC_PROPERTY_PERSON_IS_FAVORITE & 0x000000FF;
	if (person->base.properties_flags &&
			CTSVC_PROPERTY_FLAG_DIRTY == person->base.properties_flags[index_favorite]) {
		ret = ctsvc_db_person_set_favorite(person->person_id, person->is_favorite, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_db_person_set_favorite() Failed(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
		person->base.properties_flags[index_favorite] = 0;
		ctsvc_set_contact_noti();
	}

	do {
		int ret = CONTACTS_ERROR_NONE;
		char query[CTS_SQL_MAX_LEN] = {0};
		char query_set[CTS_SQL_MIN_LEN] = {0, };
		GSList *cursor = NULL;

		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_create_set_query(record, &set, &bind_text))) break;
		if (NULL == set || '\0' == *set)
			break;
		len = snprintf(query_set, sizeof(query_set), "%s, changed_ver=%d", set, ctsvc_get_next_ver());

		snprintf(query, sizeof(query), "UPDATE %s SET %s WHERE person_id = %d", CTS_TABLE_PERSONS, query_set, person->person_id);

		ret = ctsvc_query_prepare(query, &stmt);
		if (NULL == stmt) {
			CTS_ERR("DB error : ctsvc_query_prepare() Failed(%d)", ret);
			break;
		}

		if (bind_text) {
			int i = 0;
			for (cursor=bind_text,i=1;cursor;cursor=cursor->next,i++) {
				const char *text = cursor->data;
				if (text && *text)
					ctsvc_stmt_bind_text(stmt, i, text);
			}
		}
		ret = ctsvc_stmt_step(stmt);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_stmt_step() Failed(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			break;
		}
		ctsvc_stmt_finalize(stmt);
	} while (0);

	if (CONTACTS_ERROR_NONE != ret) {
		ctsvc_end_trans(false);
		CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s *)record);
		CONTACTS_FREE(set);
		if (bind_text) {
			for (cursor=bind_text;cursor;cursor=cursor->next)
				CONTACTS_FREE(cursor->data);
			g_slist_free(bind_text);
		}
		return ret;
	}

	len = snprintf(contact_query, sizeof(contact_query), "UPDATE "CTS_TABLE_CONTACTS" SET changed_ver=%d ", ctsvc_get_next_ver());
	if (ctsvc_record_check_property_flag((ctsvc_record_s *)person, _contacts_person.ringtone_path, CTSVC_PROPERTY_FLAG_DIRTY))
		len += snprintf(contact_query + len, sizeof(contact_query) - len, ", ringtone_path=? ");
	if (ctsvc_record_check_property_flag((ctsvc_record_s *)person, _contacts_person.vibration, CTSVC_PROPERTY_FLAG_DIRTY))
		len += snprintf(contact_query + len, sizeof(contact_query) - len, ", vibration=? ");
	if (ctsvc_record_check_property_flag((ctsvc_record_s *)person, _contacts_person.message_alert, CTSVC_PROPERTY_FLAG_DIRTY))
		len += snprintf(contact_query + len, sizeof(contact_query) - len, ", message_alert=? ");
	snprintf(contact_query+len, sizeof(contact_query)-len, " WHERE person_id=%d AND deleted = 0", person->person_id);

	ret = ctsvc_query_prepare(contact_query, &stmt);
	if (NULL == stmt) {
		CTS_ERR("ctsvc_query_prepare() Failed(%d)", ret);
		ctsvc_end_trans(false);
		CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s *)record);
		CONTACTS_FREE(set);
		if (bind_text) {
			for (cursor=bind_text;cursor;cursor=cursor->next)
				CONTACTS_FREE(cursor->data);
			g_slist_free(bind_text);
		}
		return ret;
	}

	i = 1;
	if (ctsvc_record_check_property_flag((ctsvc_record_s *)person, _contacts_person.ringtone_path, CTSVC_PROPERTY_FLAG_DIRTY)) {
		if (person->ringtone_path)
			ctsvc_stmt_bind_text(stmt, i, person->ringtone_path);
		i++;
	}
	if (ctsvc_record_check_property_flag((ctsvc_record_s *)person, _contacts_person.vibration, CTSVC_PROPERTY_FLAG_DIRTY)) {
		if (person->vibration)
			ctsvc_stmt_bind_text(stmt, i, person->vibration);
		i++;
	}
	if (ctsvc_record_check_property_flag((ctsvc_record_s *)person, _contacts_person.message_alert, CTSVC_PROPERTY_FLAG_DIRTY)) {
		if (person->message_alert)
			ctsvc_stmt_bind_text(stmt, i, person->message_alert);
		i++;
	}

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_stmt_step() Failed(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s *)record);
		CONTACTS_FREE(set);
		if (bind_text) {
			for (cursor=bind_text;cursor;cursor=cursor->next)
				CONTACTS_FREE(cursor->data);
			g_slist_free(bind_text);
		}
		return ret;
	}
	ctsvc_stmt_finalize(stmt);

	CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s *)record);
	CONTACTS_FREE(set);
	if (bind_text) {
		for (cursor=bind_text;cursor;cursor=cursor->next)
			CONTACTS_FREE(cursor->data);
		g_slist_free(bind_text);
	}

	// update person display_name
	if (display_name) {
		char *temp = NULL;
		person->display_name = SAFE_STRDUP(display_name);
		ret = ctsvc_normalize_index(person->display_name, &temp);
		if (0 <= ret)
			person->display_name_index = strdup(temp);

		free(temp);
		// TODO : update name primary_default??
	}
	ctsvc_set_person_noti();
#ifdef _CONTACTS_IPC_SERVER
	ctsvc_change_subject_add_changed_person_id(CONTACTS_CHANGE_UPDATED, person->person_id);
#endif

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "DB error : ctsvc_end_trans() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_person_delete_record( int id )
{
	int ret, rel_changed;
	int person_id;
	char query[CTS_SQL_MAX_LEN] = {0};
	int version;
	int *addressbook_ids = NULL;
	int count;

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "DB error : ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT person_id FROM "CTS_TABLE_PERSONS" WHERE person_id = %d", id);
	ret = ctsvc_query_get_first_int_result(query, &person_id);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_query_get_first_int_result Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	version = ctsvc_get_next_ver();
	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_GROUPS" SET member_changed_ver=%d "
				"WHERE group_id IN (SELECT distinct group_id "
									"FROM "CTS_TABLE_CONTACTS" C, "CTS_TABLE_GROUP_RELATIONS" R "
									"ON C.contact_id=R.contact_id AND R.deleted = 0 AND C.deleted = 0 "
									"WHERE person_id = %d)",
				version, id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	rel_changed = ctsvc_db_change();

	ret = ctsvc_get_write_permitted_addressbook_ids(&addressbook_ids, &count);
	if (CONTACTS_ERROR_INTERNAL == ret) {
		CTS_ERR("ctsvc_get_write_permitted_addressbook_ids() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (addressbook_ids && count > 0) {
		int i;
		int len = snprintf(query, sizeof(query),
				"UPDATE "CTS_TABLE_CONTACTS" SET deleted = 1, person_id = 0, changed_ver = %d "
					"WHERE person_id = %d AND (",
				version, id);

		for (i=0;i<count;i++) {
			if(i == 0)
				len += snprintf(query+len, sizeof(query) + len, "addressbook_id = %d ", addressbook_ids[i]);
			else
				len += snprintf(query+len, sizeof(query) + len, "OR addressbook_id = %d ", addressbook_ids[i]);
		}
		len += snprintf(query+len, sizeof(query)-len, " ) ");

		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_query_exec() Failed(%d)", ret);
			ctsvc_end_trans(false);
			free(addressbook_ids);
			return ret;
		}
	}
	free(addressbook_ids);

	// access control logic should be enabled
	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_CONTACTS" SET person_id = 0, changed_ver = %d WHERE person_id = %d",
			version, id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	// images are deleted by db trigger callback function in ctsvc_db_image_delete_callback
	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_PERSONS" WHERE person_id = %d", id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_contact_noti();
	ctsvc_set_person_noti();
	if (rel_changed > 0)
		ctsvc_set_group_rel_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_person_get_all_records( int offset, int limit, contacts_list_h* out_list )
{
	int ret;
	int len;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;

	len = snprintf(query, sizeof(query),
		"SELECT person_id, "
				"%s, "
				"_NORMALIZE_INDEX_(%s), "
				"name_contact_id, "
				"image_thumbnail_path, "
				"ringtone_path, "
				"vibration, "
				"message_alert, "
				"status, "
				"link_count, "
				"addressbook_ids, "
				"has_phonenumber, "
				"has_email, "
				"is_favorite "
		"FROM "CTSVC_DB_VIEW_PERSON,
			ctsvc_get_display_column(), ctsvc_get_sort_name_column());

	len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s", ctsvc_get_sort_column());

	if (0 != limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Failed(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Failed(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}
		ret = ctsvc_db_person_create_record_from_stmt(stmt, &record);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : ctsvc_db_person_create_record_from_stmt() Failed(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_person_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list )
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_person_s *person;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s *)query;

	ret = ctsvc_db_make_get_records_query_stmt(s_query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_db_make_get_records_query_stmt fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Failed(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		contacts_record_create(_contacts_person._uri, &record);
		person = (ctsvc_person_s*)record;
		if (0 == s_query->projection_count)
			field_count = s_query->property_count;
		else
		{
			field_count = s_query->projection_count;

			if( CONTACTS_ERROR_NONE != ctsvc_record_set_projection_flags(record, s_query->projection, s_query->projection_count, s_query->property_count) )
			{
				ASSERT_NOT_REACHED("To set projection is failed.\n");
			}
		}

		for(i=0;i<field_count;i++) {
			char *temp;
			int property_id;
			if (0 == s_query->projection_count)
				property_id = s_query->properties[i].property_id;
			else
				property_id = s_query->projection[i];

			switch(property_id) {
			case CTSVC_PROPERTY_PERSON_ID:
				person->person_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_PERSON_DISPLAY_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				person->display_name = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX:
				temp = ctsvc_stmt_get_text(stmt, i);
				person->display_name_index = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID:
				person->name_contact_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_PERSON_RINGTONE:
				temp = ctsvc_stmt_get_text(stmt, i);
				person->ringtone_path = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL:
				temp = ctsvc_stmt_get_text(stmt, i);
				if (temp && *temp) {
					snprintf(full_path, sizeof(full_path), "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, temp);
					person->image_thumbnail_path = strdup(full_path);
				}
				break;
			case CTSVC_PROPERTY_PERSON_IS_FAVORITE:
				person->is_favorite = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER:
				person->has_phonenumber = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_PERSON_HAS_EMAIL:
				person->has_email = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_PERSON_LINK_COUNT:
				person->link_count = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS:
				temp = ctsvc_stmt_get_text(stmt, i);
				person->addressbook_ids = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_PERSON_VIBRATION:
				temp = ctsvc_stmt_get_text(stmt, i);
				person->vibration = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_PERSON_STATUS:
				temp = ctsvc_stmt_get_text(stmt, i);
				person->status = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_PERSON_MESSAGE_ALERT:
				temp = ctsvc_stmt_get_text(stmt, i);
				person->message_alert = SAFE_STRDUP(temp);
				break;
			default:
				break;
			}
		}
		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;

	return CONTACTS_ERROR_NONE;
}

#if 0
static int __ctsvc_db_person_insert_records(const contacts_list_h in_list, int **ids)
{
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_person_update_records(const contacts_list_h in_list)
{
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_person_delete_records(int ids[], int count)
{
	return CONTACTS_ERROR_NONE;
}
#endif

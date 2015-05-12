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
#include "ctsvc_db_access_control.h"
#include "ctsvc_db_plugin_person_helper.h"
#include "ctsvc_localize.h"
#include "ctsvc_localize_utils.h"
#include "ctsvc_normalize.h"
#include "ctsvc_db_init.h"
#include "ctsvc_utils.h"
#include "ctsvc_db_query.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"
#include "ctsvc_notify.h"

#ifdef _CONTACTS_IPC_SERVER
#include "ctsvc_server_change_subject.h"
#endif

int ctsvc_db_person_create_record_from_stmt_with_projection(cts_stmt stmt, unsigned int *projection, int projection_count, contacts_record_h *record)
{
	ctsvc_person_s *person;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	contacts_record_create(_contacts_person._uri, record);
	person = (ctsvc_person_s*)*record;

	int i;
	for (i=0;i<projection_count;i++) {
		char *temp;
		int property_id = projection[i];

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
		case CTSVC_PROPERTY_PERSON_VIBRATION:
			temp = ctsvc_stmt_get_text(stmt, i);
			person->vibration = SAFE_STRDUP(temp);
			break;
		case CTSVC_PROPERTY_PERSON_MESSAGE_ALERT:
			temp = ctsvc_stmt_get_text(stmt, i);
			person->message_alert = SAFE_STRDUP(temp);
			break;
		case CTSVC_PROPERTY_PERSON_STATUS:
			temp = ctsvc_stmt_get_text(stmt, i);
			person->status = SAFE_STRDUP(temp);
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
		case CTSVC_PROPERTY_PERSON_FAVORITE_PRIORITY:
			{
			// TODO: Fixme (BS)
				int value = ctsvc_stmt_get_int(stmt, i);
				value++; // fix warning
			}
			break;
			//	ASSERT_NOT_REACHED("Invalid parameter : property_id(0x%0x) is not supported in projection value(person)", property_id);
			// return CONTACTS_ERROR_INVALID_PARAMETER;
		default:
			ASSERT_NOT_REACHED("Invalid parameter : property_id(0x%0x) is not supported in value(person)", property_id);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}
	return CONTACTS_ERROR_NONE;
}


int ctsvc_db_person_create_record_from_stmt_with_query(cts_stmt stmt, contacts_query_h query, contacts_record_h *record)
{
	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	ctsvc_query_s *s_query = (ctsvc_query_s *)query;

	if (0 == s_query->projection_count) {
		unsigned int *projection = malloc(sizeof(unsigned int)*s_query->property_count);
		int i;
		for (i=0;i<s_query->property_count;i++) {
			projection[i] = s_query->properties[i].property_id;
		}

		int ret = ctsvc_db_person_create_record_from_stmt_with_projection(stmt, projection, s_query->property_count, record);

		free(projection);

		return ret;
	}
	else
		return ctsvc_db_person_create_record_from_stmt_with_projection(stmt, s_query->projection, s_query->projection_count, record);

}

int ctsvc_db_person_create_record_from_stmt(cts_stmt stmt, contacts_record_h *record)
{
	int ret;
	int i;
	char *temp;
	ctsvc_person_s *person;

	i = 0;
	ret = contacts_record_create(_contacts_person._uri, record);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create Fail(%d)", ret);
	person = (ctsvc_person_s *)*record;
	person->person_id = ctsvc_stmt_get_int(stmt, i++);

	temp = ctsvc_stmt_get_text(stmt, i++);
	person->display_name = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	person->display_name_index = SAFE_STRDUP(temp);
	person->name_contact_id = ctsvc_stmt_get_int(stmt, i++);

	temp = ctsvc_stmt_get_text(stmt, i++);
	if (temp && *temp) {
		char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		snprintf(full_path, sizeof(full_path), "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, temp);
		person->image_thumbnail_path = strdup(full_path);
	}

	temp = ctsvc_stmt_get_text(stmt, i++);
	person->ringtone_path = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	person->vibration = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	person->message_alert = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	person->status = SAFE_STRDUP(temp);

	person->link_count = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	person->addressbook_ids = SAFE_STRDUP(temp);

	person->has_phonenumber = ctsvc_stmt_get_int(stmt, i++);
	person->has_email = ctsvc_stmt_get_int(stmt, i++);
	person->is_favorite = ctsvc_stmt_get_int(stmt, i++);

	return CONTACTS_ERROR_NONE;
}

inline static const char* __ctsvc_get_image_filename(const char* src)
{
	const char* dir = CTSVC_CONTACT_IMG_FULL_LOCATION;
	int pos=0;
	while (dir[pos]==src[pos]) {
		pos++;
	}

	if ('/' == src[pos])
		return src + pos + 1;

	return src+pos;
}

int ctsvc_db_person_set_favorite(int person_id, bool set, bool propagate)
{
	int ret;
	double prio = 0.0;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	if (set) {
		snprintf(query, sizeof(query),
			"SELECT MAX(favorite_prio) FROM "CTS_TABLE_FAVORITES);

		ret = ctsvc_query_prepare(query, &stmt);
		RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

		ret = ctsvc_stmt_step(stmt);
		if (1 /*CTS_TRUE*/ == ret) {
			prio = ctsvc_stmt_get_dbl(stmt, 0);
		}
		else if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			return ret;
		}
		ctsvc_stmt_finalize(stmt);

		prio = prio + 1.0;
		snprintf(query, sizeof(query),
			"INSERT OR REPLACE INTO "CTS_TABLE_FAVORITES" values(%d, %f)", person_id, prio);
	}
	else {
		snprintf(query, sizeof(query),
			"DELETE FROM "CTS_TABLE_FAVORITES" WHERE person_id = %d", person_id);
	}

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec() Fail(%d)", ret);
		return ret;
	}

	if (propagate) {
		snprintf(query, sizeof(query),
			 "UPDATE "CTS_TABLE_CONTACTS" SET is_favorite=%d, "
			 "changed_time=%d, changed_ver=%d WHERE "
			 "person_id=%d AND deleted = 0",
			 set ? 1 : 0, (int)time(NULL), ctsvc_get_next_ver(),
			 person_id);
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_query_exec() Fail(%d)", ret);
			return ret;
		}
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_insert_person(contacts_record_h record)
{
	int ret, index;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};
	int version;
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;
	char *status = NULL;

	snprintf(query, sizeof(query),
		"SELECT status FROM %s "
		"WHERE contact_id=%d "
		"ORDER BY timestamp DESC LIMIT 1",
		CTS_TABLE_ACTIVITIES, contact->id);
	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	if (1 == ctsvc_stmt_step(stmt))
		status = SAFE_STRDUP(ctsvc_stmt_get_text(stmt, 0));
	ctsvc_stmt_finalize(stmt);

	version = ctsvc_get_next_ver();
	snprintf(query, sizeof(query),
		"INSERT INTO "CTS_TABLE_PERSONS"(name_contact_id, created_ver, changed_ver, "
			"has_phonenumber, has_email, ringtone_path, vibration, message_alert, status, "
			"image_thumbnail_path, link_count, addressbook_ids) "
			"VALUES(%d, %d, %d, %d, %d, ?, ?, ?, ?, ?, 1, '%d') ",
			contact->id, version, version,
			contact->has_phonenumber, contact->has_email, contact->addressbook_id);

	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		CTS_ERR("DB error : ctsvc_query_prepare() Fail(%d)", ret);
		free(status);
		return ret;
	}
	if (contact->ringtone_path)
		ctsvc_stmt_bind_text(stmt, 1, contact->ringtone_path);
	if (contact->vibration)
		ctsvc_stmt_bind_text(stmt, 2, contact->vibration);
	if (contact->message_alert)
		ctsvc_stmt_bind_text(stmt, 3, contact->message_alert);
	if (status)
		ctsvc_stmt_bind_text(stmt, 4, status);
	if (contact->image_thumbnail_path)
		ctsvc_stmt_bind_text(stmt, 5, __ctsvc_get_image_filename(contact->image_thumbnail_path));

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		free(status);
		return ret;
	}
	index = ctsvc_db_get_last_insert_id();

	ctsvc_stmt_finalize(stmt);

	snprintf(query, sizeof(query),
		"UPDATE "CTS_TABLE_DATA" SET is_primary_default = 1 "
			"WHERE is_default = 1 AND contact_id = %d  AND is_my_profile = 0", contact->id);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec Fail(%d)", ret);
		free(status);
		return ret;
	}

	// set favorite
	if (contact->is_favorite) {
		ret = ctsvc_db_person_set_favorite(index, contact->is_favorite, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_db_person_set_favorite() Fail(%d)", ret);
			return ret;
		}
	}

	free(status);
	ctsvc_set_person_noti();
#ifdef _CONTACTS_IPC_SERVER
	ctsvc_change_subject_add_changed_person_id(CONTACTS_CHANGE_INSERTED, index);
#endif

	return index;
}

static inline int __ctsvc_db_update_person_default(int person_id, int datatype)
{
	int ret, data_id;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};
	char *temp = NULL;
	char *image_thumbnail_path = NULL;

	snprintf(query, sizeof(query),
		"SELECT D.id FROM "CTS_TABLE_CONTACTS" C, "CTS_TABLE_DATA" D "
		"ON C.contact_id = D.contact_id AND C.deleted = 0 "
		"WHERE C.person_id=%d AND D.datatype=%d AND is_primary_default=1 AND D.is_my_profile = 0",
		person_id, datatype);

	ret = ctsvc_query_get_first_int_result(query, &data_id);
	if (CONTACTS_ERROR_NO_DATA == ret) {
		snprintf(query, sizeof(query),
			"SELECT D.id, D.data3 FROM "CTS_TABLE_CONTACTS" C, "CTS_TABLE_DATA" D "
			"ON C.contact_id = D.contact_id AND C.deleted = 0 "
			"WHERE C.person_id=%d AND D.datatype=%d AND D.is_default=1 AND D.is_my_profile = 0 ORDER BY D.id",
			person_id, datatype);

		ret = ctsvc_query_prepare(query, &stmt);
		RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

		ret = ctsvc_stmt_step(stmt);
		if (1 == ret) {
			data_id = ctsvc_stmt_get_int(stmt, 0);

			snprintf(query, sizeof(query),
					"UPDATE "CTS_TABLE_DATA" SET is_primary_default=1 WHERE id=%d"
					,data_id);

			ret = ctsvc_query_exec(query);
			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("ctsvc_query_exec Fail(%d)", ret);
				ctsvc_stmt_finalize(stmt);
				return ret;
			}
			temp = ctsvc_stmt_get_text(stmt, 1);
			image_thumbnail_path = SAFE_STRDUP(temp);
		}
		ctsvc_stmt_finalize(stmt);

		if (CTSVC_DATA_IMAGE == datatype) {
			if (image_thumbnail_path) {
				snprintf(query, sizeof(query),
						"UPDATE "CTS_TABLE_PERSONS" SET image_thumbnail_path=? WHERE person_id=%d", person_id);
				ret = ctsvc_query_prepare(query, &stmt);
				RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);
				ctsvc_stmt_bind_text(stmt, 1, image_thumbnail_path);
				ret = ctsvc_stmt_step(stmt);
				ctsvc_stmt_finalize(stmt);
				free(image_thumbnail_path);
				if (CONTACTS_ERROR_NONE != ret) {
					CTS_ERR("ctsvc_stmt_step Fail(%d)", ret);
					return ret;
				}
			}
		}
		else {
			free(image_thumbnail_path);
		}
	}

	return CONTACTS_ERROR_NONE;
}

static bool __ctsvc_get_has_column(int person_id, const char *culumn)
{
	int ret;
	int contact_count = 0;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query),
				"SELECT count(contact_id) FROM "CTS_TABLE_CONTACTS" "
				"WHERE person_id=%d AND %s=1 AND deleted = 0",
				person_id, culumn);

	ret = ctsvc_query_get_first_int_result(query, &contact_count);
	RETV_IF(CONTACTS_ERROR_NONE != ret, false);

	if (contact_count)
		return true;
	return false;
}

static int __ctsvc_get_thumbnail_contact_id(int person_id)
{
	int ret;
	int contact_id = 0;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT D.contact_id FROM "CTS_TABLE_CONTACTS" C, "CTS_TABLE_DATA" D "
			"ON C.contact_id = D.contact_id AND C.deleted = 0 "
			"WHERE C.person_id=%d AND D.datatype=%d AND is_primary_default=1 AND D.is_my_profile = 0",
			person_id, CTSVC_DATA_IMAGE);
	ret = ctsvc_query_get_first_int_result(query, &contact_id);
	RETV_IF(CONTACTS_ERROR_NONE != ret, -1);
	return contact_id;
}

int ctsvc_db_update_person(contacts_record_h record)
{
	int ret, i=1, len=0;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;
	bool has_phonenumber=false, has_email=false;
	int thumbnail_contact_id = 0;
	int person_id = 0;
	int is_favorite = 0;

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	__ctsvc_db_update_person_default(contact->person_id, CTSVC_DATA_NUMBER);
	__ctsvc_db_update_person_default(contact->person_id, CTSVC_DATA_EMAIL);
	__ctsvc_db_update_person_default(contact->person_id, CTSVC_DATA_IMAGE);

	has_phonenumber = __ctsvc_get_has_column(contact->person_id, "has_phonenumber");
	has_email = __ctsvc_get_has_column(contact->person_id, "has_email");
	thumbnail_contact_id = __ctsvc_get_thumbnail_contact_id(contact->person_id);

	len = snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_PERSONS" SET changed_ver=%d, has_phonenumber=%d, has_email=%d ",
			ctsvc_get_next_ver(), has_phonenumber, has_email);

	if (ctsvc_record_check_property_flag((ctsvc_record_s *)record, _contacts_contact.ringtone_path, CTSVC_PROPERTY_FLAG_DIRTY))
		len += snprintf(query+len, sizeof(query)-len, ", ringtone_path=?");
	if (ctsvc_record_check_property_flag((ctsvc_record_s *)record, _contacts_contact.vibration, CTSVC_PROPERTY_FLAG_DIRTY))
		len += snprintf(query+len, sizeof(query)-len, ", vibration=?");
	if (ctsvc_record_check_property_flag((ctsvc_record_s *)record, _contacts_contact.message_alert, CTSVC_PROPERTY_FLAG_DIRTY))
		len += snprintf(query+len, sizeof(query)-len, ", message_alert=?");
	if (ctsvc_record_check_property_flag((ctsvc_record_s *)record, _contacts_contact.image_thumbnail_path, CTSVC_PROPERTY_FLAG_DIRTY) &&
			(contact->id == thumbnail_contact_id || thumbnail_contact_id == -1))
		len += snprintf(query+len, sizeof(query)-len, ", image_thumbnail_path=?");

	snprintf(query+len, sizeof(query)-len,
			" WHERE person_id=%d", contact->person_id);

	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		CTS_ERR("ctsvc_query_prepare() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (ctsvc_record_check_property_flag((ctsvc_record_s *)record, _contacts_contact.ringtone_path, CTSVC_PROPERTY_FLAG_DIRTY)) {
		if (contact->ringtone_path)
			ctsvc_stmt_bind_text(stmt, i, contact->ringtone_path);
		i++;
	}
	if (ctsvc_record_check_property_flag((ctsvc_record_s *)record, _contacts_contact.vibration, CTSVC_PROPERTY_FLAG_DIRTY)) {
		if (contact->vibration)
			ctsvc_stmt_bind_text(stmt, i, contact->vibration);
		i++;
	}
	if (ctsvc_record_check_property_flag((ctsvc_record_s *)record, _contacts_contact.message_alert, CTSVC_PROPERTY_FLAG_DIRTY)) {
		if (contact->message_alert)
			ctsvc_stmt_bind_text(stmt, i, contact->message_alert);
		i++;
	}
	if (ctsvc_record_check_property_flag((ctsvc_record_s *)record, _contacts_contact.image_thumbnail_path, CTSVC_PROPERTY_FLAG_DIRTY) &&
			(contact->id == thumbnail_contact_id || thumbnail_contact_id == -1)) {
		if (contact->image_thumbnail_path)
			ctsvc_stmt_bind_text(stmt, i, contact->image_thumbnail_path);
		i++;
	}

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}
	ctsvc_stmt_finalize(stmt);

	// update favorite
	snprintf(query, sizeof(query),
			"SELECT is_favorite FROM "CTS_TABLE_CONTACTS" WHERE contact_id =%d ", contact->id);
	ret = ctsvc_query_get_first_int_result(query, &is_favorite);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_query_get_first_int_result() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query),
			"SELECT person_id FROM "CTS_TABLE_FAVORITES" WHERE person_id =%d ", contact->person_id);
	ret = ctsvc_query_get_first_int_result(query, &person_id);
	if (CONTACTS_ERROR_NO_DATA == ret && is_favorite) {
		ret = ctsvc_db_person_set_favorite(contact->person_id, true, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_db_person_set_favorite() Fail(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
	}
	else if (CONTACTS_ERROR_NONE == ret && false == is_favorite) {
		snprintf(query, sizeof(query),
			"SELECT person_id FROM "CTS_TABLE_CONTACTS" WHERE person_id =%d AND is_favorite = 1", contact->person_id);
		ret = ctsvc_query_get_first_int_result(query, &person_id);
		if (CONTACTS_ERROR_NO_DATA == ret) {
			ret = ctsvc_db_person_set_favorite(contact->person_id, false, false);
			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("ctsvc_db_person_set_favorite() Fail(%d)", ret);
				ctsvc_end_trans(false);
				return ret;
			}
		}
		else if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_query_get_first_int_result() Fail(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
	}
	else if (ret < CONTACTS_ERROR_NONE && CONTACTS_ERROR_NO_DATA != ret) {
		CTS_ERR("ctsvc_query_get_first_int_result() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_person_noti();

#ifdef _CONTACTS_IPC_SERVER
	ctsvc_change_subject_add_changed_person_id(CONTACTS_CHANGE_UPDATED, contact->person_id);
#endif

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

// This function will return group letter of the person
void ctsvc_db_normalize_str_callback(sqlite3_context * context,
		int argc, sqlite3_value ** argv)
{
	const char *display_name;
	int display_name_language = CTSVC_LANG_OTHERS;

	if (argc < 1) {
		sqlite3_result_null(context);
		return;
	}

	display_name_language = sqlite3_value_int(argv[1]);
	if (display_name_language == CTSVC_SORT_OTHERS || display_name_language == CTSVC_SORT_NUMBER) {
		sqlite3_result_text(context, "#", 1, SQLITE_TRANSIENT);
		return;
	}
	else {
		display_name = (const char *)sqlite3_value_text(argv[0]);
		if (display_name) {
			int ret;
			char *dest = NULL;
			ret = ctsvc_normalize_index(display_name, &dest);
			if (ret < CONTACTS_ERROR_NONE) {
				CTS_ERR("ctsvc_normalize_index() Fail(%d)", ret);
				sqlite3_result_null(context);
				return;
			}
			sqlite3_result_text(context, dest, strlen(dest), SQLITE_TRANSIENT);
			free(dest);
			return;
		}
	}

	sqlite3_result_null(context);
	return;
}


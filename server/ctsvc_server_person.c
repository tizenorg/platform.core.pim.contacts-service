/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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
#include "ctsvc_db_schema.h"
#include "ctsvc_db_sqlite.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_utils.h"
#include "ctsvc_notification.h"
#include "ctsvc_db_plugin_person_helper.h"
#include "ctsvc_db_plugin_contact_helper.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_notify.h"
#include "ctsvc_number_utils.h"
#include "ctsvc_server_setting.h"
#include "ctsvc_list.h"
#include "ctsvc_localize.h"

#ifdef ENABLE_LOG_FEATURE
#include "ctsvc_server_phonelog.h"
#endif /* ENABLE_LOG_FEATURE */

#ifdef _CONTACTS_IPC_SERVER
#include "ctsvc_server_change_subject.h"
#endif

#define CTSVC_COMP_NAME_LEN 4

enum {
	CTSVC_GET_PERSON_DEFAULT_NUMBER_VALUE,
	CTSVC_GET_PERSON_DEFAULT_EMAIL_VALUE,
	CTSVC_GET_PERSON_DEFAULT_IMAGE_VALUE,
};

static inline int __ctsvc_get_person_default_number_value(int id, contacts_record_h *record)
{
	int ret;
	cts_stmt stmt;
	ctsvc_number_s *number;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT data.id, data.is_default, data.data1, data.data2, data.data3, data.data4 "
			"FROM %s, %s ON data.is_primary_default=1 AND data.datatype=%d "
			"AND data.contact_id = contacts.contact_id AND contacts.deleted = 0 "
			"WHERE contacts.person_id = %d",
			CTS_TABLE_CONTACTS, CTS_TABLE_DATA, CTSVC_DATA_NUMBER, id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		ctsvc_stmt_finalize(stmt);
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}

	ret = contacts_record_create(_contacts_number._uri, (contacts_record_h*)&number);
	if (number) {
		char *temp;
		number->id = ctsvc_stmt_get_int(stmt, 0);
		number->is_default = ctsvc_stmt_get_int(stmt, 1);
		number->type = ctsvc_stmt_get_int(stmt, 2);
		temp = ctsvc_stmt_get_text(stmt, 3);
		number->label = SAFE_STRDUP(temp);
		temp = ctsvc_stmt_get_text(stmt, 4);
		number->number = SAFE_STRDUP(temp);
		temp = ctsvc_stmt_get_text(stmt, 5);
		number->lookup = SAFE_STRDUP(temp);

		*record = (contacts_record_h)number;
		ret = CONTACTS_ERROR_NONE;
	} else {
		ERR("contacts_record_create() Fail");
	}

	ctsvc_stmt_finalize(stmt);
	return ret;
}

static inline int __ctsvc_get_person_default_email_value(int id, contacts_record_h *record)
{
	int ret;
	cts_stmt stmt;
	ctsvc_email_s *email;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT data.id, data.is_default, data.data1, data.data2, data.data3 "
			"FROM %s, %s ON data.is_primary_default=1 AND data.datatype=%d "
			"AND data.contact_id = contacts.contact_id AND contacts.deleted = 0 "
			"WHERE contacts.person_id = %d",
			CTS_TABLE_CONTACTS, CTS_TABLE_DATA, CTSVC_DATA_EMAIL, id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		ctsvc_stmt_finalize(stmt);
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}

	ret = contacts_record_create(_contacts_email._uri, (contacts_record_h*)&email);
	if (email) {
		char *temp;
		email->id = ctsvc_stmt_get_int(stmt, 0);
		email->is_default = ctsvc_stmt_get_int(stmt, 1);
		email->type = ctsvc_stmt_get_int(stmt, 2);
		temp = ctsvc_stmt_get_text(stmt, 3);
		email->label = SAFE_STRDUP(temp);
		temp = ctsvc_stmt_get_text(stmt, 4);
		email->email_addr = SAFE_STRDUP(temp);

		*record = (contacts_record_h)email;
		ret = CONTACTS_ERROR_NONE;
	} else {
		ERR("contacts_record_create() Fail");
	}

	ctsvc_stmt_finalize(stmt);
	return ret;
}

static inline int __ctsvc_get_person_default_image_value(int id, contacts_record_h *record)
{
	int ret;
	cts_stmt stmt;
	ctsvc_image_s *image;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT data.id, data.is_default, data.data1, data.data2, data.data3 "
			"FROM "CTS_TABLE_CONTACTS", "CTS_TABLE_DATA" "
			"ON data.is_primary_default=1 AND data.datatype=%d AND data.is_my_profile = 0 "
			"AND data.contact_id = contacts.contact_id AND contacts.deleted = 0 "
			"WHERE contacts.person_id = %d",
			CTSVC_DATA_IMAGE, id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		ctsvc_stmt_finalize(stmt);
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}

	ret = contacts_record_create(_contacts_image._uri, (contacts_record_h*)&image);
	if (image) {
		char *temp;
		image->id = ctsvc_stmt_get_int(stmt, 0);
		image->is_default = ctsvc_stmt_get_int(stmt, 1);
		image->type = ctsvc_stmt_get_int(stmt, 2);
		temp = ctsvc_stmt_get_text(stmt, 3);
		image->label = SAFE_STRDUP(temp);
		temp = ctsvc_stmt_get_text(stmt, 4);
		image->path = SAFE_STRDUP(temp);

		*record = (contacts_record_h)image;
		ret = CONTACTS_ERROR_NONE;
	} else {
		ERR("contacts_record_create() Fail");
	}

	ctsvc_stmt_finalize(stmt);
	return ret;
}

static int __ctsvc_get_person_value(int op_code,
		int person_id, contacts_record_h *record)
{
	int ret;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);

	switch (op_code) {
	case CTSVC_GET_PERSON_DEFAULT_NUMBER_VALUE:
		ret = __ctsvc_get_person_default_number_value(person_id, record);
		RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "__ctsvc_get_person_default_number_value() Fail(%d)", ret);
		break;
	case CTSVC_GET_PERSON_DEFAULT_EMAIL_VALUE:
		ret = __ctsvc_get_person_default_email_value(person_id, record);
		RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "__ctsvc_get_person_default_email_value() Fail(%d)", ret);
		break;
	case CTSVC_GET_PERSON_DEFAULT_IMAGE_VALUE:
		ret = __ctsvc_get_person_default_image_value(person_id, record);
		RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "__ctsvc_get_person_default_image_value() Fail(%d)", ret);
		break;
	default:
		ERR("The op_code(%d) is not supported", op_code);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	if (NULL == *record)
		return CONTACTS_ERROR_NO_DATA;

	return ret;
}

static inline int __ctsvc_put_person_default_name(int person_id, int contact_id)
{
	int ret;
	int id;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT person_id FROM %s WHERE contact_id=%d AND deleted = 0",
			CTS_TABLE_CONTACTS, contact_id);

	ret = ctsvc_query_get_first_int_result(query, &id);
	RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "ctsvc_query_get_first_int_result() Fail(%d)", ret);

	if (id == person_id) {
		ret = ctsvc_begin_trans();
		RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_begin_trans() Fail(%d)", ret);
		snprintf(query, sizeof(query),
				"UPDATE %s SET name_contact_id=%d WHERE person_id=%d",
				CTS_TABLE_PERSONS, contact_id, person_id);

		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			ERR("ctsvc_query_exec() Fail(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		} else {
			ret = ctsvc_end_trans(true);
			if (ret < CONTACTS_ERROR_NONE) {
				ERR("ctsvc_end_trans() Fail(%d)", ret);
				return ret;
			} else {
				return CONTACTS_ERROR_NONE;
			}
		}
	} else {
		ERR("contact(%d) does not belong to person(%d), to person(%d)", contact_id, person_id, ret);
		ret = CONTACTS_ERROR_NO_DATA;
	}

	return ret;
}

static inline int __ctsvc_put_person_default_image(int person_id, int id)
{
	int ret;
	int is_default;
	int contact_id;
	cts_stmt stmt;
	char *image_path;
	char query[CTS_SQL_MAX_LEN] = {0};

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT D.is_default, D.contact_id, D.data3 "
			"FROM "CTS_TABLE_DATA" D, "CTS_TABLE_CONTACTS" C "
			"ON D.contact_id = C.contact_id AND C.deleted = 0 "
			"WHERE D.datatype=%d AND D.is_my_profile = 0 AND C.person_id=%d AND D.id=%d",
			CTSVC_DATA_IMAGE, person_id, id);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		ERR("ctsvc_query_prepare Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_stmt_step(stmt);
	if (1 != ret) {
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}

	is_default = ctsvc_stmt_get_int(stmt, 0);
	contact_id = ctsvc_stmt_get_int(stmt, 1);
	image_path = SAFE_STRDUP(ctsvc_stmt_get_text(stmt, 2));
	ctsvc_stmt_finalize(stmt);

	/* unset is_primary_default of all data of the person */
	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_DATA" SET is_primary_default=0 WHERE datatype=%d AND is_my_profile = 0 "
			"AND contact_id IN (SELECT contact_id FROM "CTS_TABLE_CONTACTS" "
			"WHERE person_id=%d AND deleted = 0) ",
			CTSVC_DATA_IMAGE, person_id);
	ret = ctsvc_query_exec(query);
	if (ret < CONTACTS_ERROR_NONE) {
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		free(image_path);
		ctsvc_end_trans(false);
		return ret;
	}

	/* unset is_default of all data of person if the data is not default */
	if (false == is_default) {
		snprintf(query, sizeof(query),
				"UPDATE "CTS_TABLE_DATA" SET is_default=0 WHERE datatype=%d  AND is_my_profile = 0 "
				"AND contact_id = (SELECT contact_id FROM "CTS_TABLE_DATA" WHERE id=%d) ",
				CTSVC_DATA_IMAGE, id);

		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			ERR("ctsvc_query_exec() Fail(%d)", ret);
			free(image_path);
			ctsvc_end_trans(false);
			return ret;
		}
	}

	/* set is_default, is _primary_default */
	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_DATA" SET is_primary_default=1, is_default=1 WHERE id=%d ", id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		free(image_path);
		ctsvc_end_trans(false);
		return ret;
	}

	/* update person's image_thumbnail_path */
	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_PERSONS" SET image_thumbnail_path=? WHERE person_id=%d ", person_id);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		free(image_path);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_stmt_bind_text(stmt, 1, image_path);
	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		free(image_path);
		ctsvc_end_trans(false);
		return ret;
	}
	ctsvc_stmt_finalize(stmt);

	/* update contact's image_thumbnail_path */
	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_CONTACTS" SET image_thumbnail_path=? WHERE contact_id=%d ", contact_id);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		free(image_path);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_stmt_bind_text(stmt, 1, image_path);
	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		free(image_path);
		ctsvc_end_trans(false);
		return ret;
	}
	ctsvc_stmt_finalize(stmt);

	free(image_path);

	ret = ctsvc_end_trans(true);
	return ret;
}

static inline int __ctsvc_put_person_default_data(int person_id, int id, int datatype)
{
	int ret;
	int is_default = 0;
	int contact_id = 0;
	int name_contact_id = 0;
	char query[CTS_SQL_MAX_LEN] = {0};
	cts_stmt stmt = NULL;
	contacts_display_name_source_type_e source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_INVALID;

	snprintf(query, sizeof(query),
			"SELECT D.is_default, D.contact_id FROM %s D, %s C "
			"ON (D.contact_id=C.contact_id AND C.deleted = 0) "
			"WHERE D.datatype=%d AND C.person_id=%d AND D.id=%d",
			CTS_TABLE_DATA, CTS_TABLE_CONTACTS, datatype, person_id, id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 != ret) {
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		return ret;
	}

	is_default = ctsvc_stmt_get_int(stmt, 0);
	contact_id = ctsvc_stmt_get_int(stmt, 1);

	ctsvc_stmt_finalize(stmt);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	/* unset is_primary_default of all data of the person */
	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_DATA" SET is_primary_default=0 WHERE datatype=%d AND is_my_profile = 0 "
			"AND contact_id IN (SELECT contact_id FROM "CTS_TABLE_CONTACTS" "
			"WHERE person_id=%d AND deleted = 0) ",
			datatype, person_id);

	ret = ctsvc_query_exec(query);
	if (ret < CONTACTS_ERROR_NONE) {
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	/* unset is_default of all data of person if the data is not default */
	if (false == is_default) {
		snprintf(query, sizeof(query),
				"UPDATE "CTS_TABLE_DATA" SET is_default=0 WHERE datatype=%d AND is_my_profile = 0 "
				"AND contact_id = (SELECT contact_id FROM "CTS_TABLE_DATA" WHERE id=%d) ",
				datatype, id);

		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			ERR("ctsvc_query_exec() Fail(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
	}

	/* set is_default, is _primary_default */
	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_DATA" SET is_primary_default=1, is_default=1 WHERE id=%d ", id);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (datatype == CTSVC_DATA_NUMBER)
		source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NUMBER;
	else if (datatype == CTSVC_DATA_EMAIL)
		source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_EMAIL;

	if (CONTACTS_DISPLAY_NAME_SOURCE_TYPE_INVALID != source_type)
		ctsvc_contact_update_display_name(contact_id, source_type);

	snprintf(query, sizeof(query),
			"SELECT name_contact_id FROM "CTS_TABLE_PERSONS" WHERE person_id = %d", person_id);
	ret = ctsvc_query_get_first_int_result(query, &name_contact_id);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_query_get_first_int_result() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (name_contact_id != contact_id) {
		int org_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_INVALID;
		snprintf(query, sizeof(query),
				"SELECT display_name_source FROM "CTS_TABLE_CONTACTS" WHERE contact_id = %d", name_contact_id);
		ret = ctsvc_query_get_first_int_result(query, &org_source_type);
		if (CONTACTS_ERROR_NONE != ret) {
			ERR("ctsvc_query_get_first_int_result() Fail(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}

		if (org_source_type <= source_type) {
			snprintf(query, sizeof(query),
					"UPDATE %s SET name_contact_id=%d WHERE person_id=%d",
					CTS_TABLE_PERSONS, contact_id, person_id);
			ret = ctsvc_query_exec(query);
			if (CONTACTS_ERROR_NONE != ret) {
				ERR("ctsvc_query_exec() Fail(%d)", ret);
				ctsvc_end_trans(false);
				return ret;
			}
		}
	}

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		ERR("ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	} else {
		return CONTACTS_ERROR_NONE;
	}
}

void ctsvc_db_person_delete_callback(sqlite3_context  *context,
		int argc, sqlite3_value **argv)
{
#ifdef _CONTACTS_IPC_SERVER
	int person_id;

	if (argc < 1) {
		sqlite3_result_null(context);
		return;
	}

	person_id = sqlite3_value_int(argv[0]);
	ctsvc_change_subject_add_changed_person_id(CONTACTS_CHANGE_DELETED, person_id);

#ifdef ENABLE_LOG_FEATURE
	/*
	 * update phonelog
	 * CASE : do not know the proper new person_id
	 */
	ctsvc_db_phone_log_update_person_id(NULL, person_id, -1, false);
#endif /* ENABLE_LOG_FEATURE */
	sqlite3_result_null(context);
	return;
#endif
}

static inline const char *__ctsvc_get_image_filename(const char *src)
{
	const char *dir = CTSVC_CONTACT_IMG_FULL_LOCATION;
	int pos = 0;
	while (dir[pos] == src[pos])
		pos++;

	if ('/' == src[pos])
		return src + pos + 1;

	return src+pos;
}

int ctsvc_person_aggregate(int person_id)
{
	int ret, len = 0;
	int version;
	int link_count;
	int id = 0;
	char *addressbook_ids = NULL;
	int addressbooks_len = 100;
	int name_contact_id = 0;
	int person_name_contact_id = 0;
	int display_name_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_INVALID;
	char query[CTS_SQL_MIN_LEN] = {0};
	char *ringtone_path = NULL;
	char *image_thumbnail_path = NULL;
	char *vibration = NULL;
	char *message_alert = NULL;
	char *status = NULL;
	const char *temp;
	cts_stmt stmt;
	ctsvc_person_s *person;
	bool person_is_favorite = false;

	/*
	 * person aggregation : person link/unlink, contact insert (auto link),
	 *  contact delete, garbage collection (addressbook delete)
	 * It should be get all contacts of person regardless of permission
	 * Get person info directly instead of contacts_db_get_record(_contacts_person._uri, person_id, (contacts_record_h*)&person);
	 */
	snprintf(query, sizeof(query),
			"SELECT person_id, "
			"name_contact_id, "
			"image_thumbnail_path, "
			"ringtone_path, "
			"vibration, "
			"message_alert "
			"FROM "CTS_TABLE_PERSONS" "
			"WHERE persons.person_id = %d", person_id);
	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	if (1 != ctsvc_stmt_step(stmt)) {
		ERR("ctsvc_stmt_step() Fail\n");
		return CONTACTS_ERROR_DB;
	}
	ret = contacts_record_create(_contacts_person._uri, (contacts_record_h*)&person);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("contacts_record_create() Fail\n");
		return CONTACTS_ERROR_INTERNAL;
	}
	person->person_id = ctsvc_stmt_get_int(stmt, 0);
	person->name_contact_id = ctsvc_stmt_get_int(stmt, 1);
	temp = ctsvc_stmt_get_text(stmt, 2);
	person->image_thumbnail_path = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, 3);
	person->ringtone_path = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, 4);
	person->vibration = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, 5);
	person->message_alert = SAFE_STRDUP(temp);
	ctsvc_stmt_finalize(stmt);

	/* check image_thumbnail_path */
	if (person->image_thumbnail_path) {
		temp = __ctsvc_get_image_filename(person->image_thumbnail_path);
		snprintf(query, sizeof(query),
				"SELECT D.id FROM "CTS_TABLE_CONTACTS" C, "CTS_TABLE_DATA" D "
				"WHERE C.person_id=%d AND C.contact_id=D.contact_id AND C.deleted = 0 "
				"AND D.datatype=%d AND D.is_primary_default = 1 AND D.data3='%s'",
				person->person_id, CTSVC_DATA_IMAGE, temp);
		ret = ctsvc_query_get_first_int_result(query, &id);
		if (ret == CONTACTS_ERROR_NONE)
			image_thumbnail_path = SAFE_STRDUP(temp);
	} else {
		image_thumbnail_path = NULL;
	}

	/* check name_contact_id */
	snprintf(query, sizeof(query),
			"SELECT contact_id FROM %s "
			"WHERE person_id=%d AND contact_id=%d AND deleted = 0",
			CTS_TABLE_CONTACTS, person->person_id, person->name_contact_id);

	ret = ctsvc_query_get_first_int_result(query, &id);
	if (ret == CONTACTS_ERROR_NONE) {
		name_contact_id = person->name_contact_id;
		person_name_contact_id = person->name_contact_id;
	} else {
		name_contact_id = 0;
		person_name_contact_id = 0;
	}

	/* get status of person */
	snprintf(query, sizeof(query),
			"SELECT a.status FROM %s c, %s a "
			"ON c.contact_id = a.contact_id AND c.deleted = 0 "
			"WHERE c.person_id=%d "
			"ORDER BY timestamp DESC LIMIT 1",
			CTS_TABLE_CONTACTS, CTS_TABLE_ACTIVITIES, person->person_id);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		free(image_thumbnail_path);
		contacts_record_destroy((contacts_record_h)person, true);
		return ret;
	}

	if (1 == ctsvc_stmt_step(stmt)) {
		temp = ctsvc_stmt_get_text(stmt, 0);
		status = SAFE_STRDUP(temp);
	}
	ctsvc_stmt_finalize(stmt);

	/* check ringtone_path */
	if (person->ringtone_path) {
		snprintf(query, sizeof(query),
				"SELECT C.contact_id FROM "CTS_TABLE_CONTACTS" C "
				"WHERE C.person_id=%d AND C.deleted = 0 "
				"AND C.ringtone_path = '%s'",
				person->person_id, person->ringtone_path);
		ret = ctsvc_query_get_first_int_result(query, &id);
		if (ret == CONTACTS_ERROR_NONE)
			ringtone_path = SAFE_STRDUP(person->ringtone_path);
	} else {
		ringtone_path = NULL;
	}

	/* check vibration */
	if (person->vibration) {
		snprintf(query, sizeof(query),
				"SELECT C.contact_idFROM "CTS_TABLE_CONTACTS" C "
				"WHERE C.person_id=%d AND C.deleted = 0 "
				"AND C.vibration = '%s'",
				person->person_id, person->vibration);
		ret = ctsvc_query_get_first_int_result(query, &id);
		if (ret == CONTACTS_ERROR_NONE)
			vibration = SAFE_STRDUP(person->vibration);
	} else {
		vibration = NULL;
	}

	/* check vibration */
	if (person->message_alert) {
		snprintf(query, sizeof(query),
				"SELECT C.contact_id FROM "CTS_TABLE_CONTACTS" C "
				"WHERE C.person_id=%d AND C.deleted = 0 "
				"AND C.message_alert = '%s'",
				person->person_id, person->message_alert);
		ret = ctsvc_query_get_first_int_result(query, &id);
		if (ret == CONTACTS_ERROR_NONE)
			message_alert = SAFE_STRDUP(person->message_alert);
	} else {
		message_alert = NULL;
	}
	contacts_record_destroy((contacts_record_h)person, true);

	snprintf(query, sizeof(query),
			"SELECT contact_id, contacts.addressbook_id, display_name_source, "
			"image_thumbnail_path, ringtone_path, vibration, message_alert, is_favorite "
			"FROM %s "
			"WHERE person_id = %d AND contacts.deleted = 0 "
			"ORDER BY contact_id",
			CTS_TABLE_CONTACTS, person_id);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		free(image_thumbnail_path);
		free(ringtone_path);
		free(vibration);
		free(message_alert);
		free(status);
		return ret;
	}

	link_count = 0;
	len = 0;
	while ((ret = ctsvc_stmt_step(stmt))) {
		const char *temp_str;
		int i = 0;
		int contact_id;
		int addressbook_id;
		int contact_display_name_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_INVALID;
		char *contact_ringtone_path = NULL;
		char *contact_image_thumbnail_path = NULL;
		char *contact_vibration = NULL;
		char *contact_message_alert = NULL;
		bool is_favorite = false;
		char addr[10] = {0};
		int addr_len = 0;

		contact_id = ctsvc_stmt_get_int(stmt, i++);
		addressbook_id = ctsvc_stmt_get_int(stmt, i++);
		contact_display_name_source_type = ctsvc_stmt_get_int(stmt, i++);
		temp = ctsvc_stmt_get_text(stmt, i++);
		if (temp)
			contact_image_thumbnail_path = strdup(temp);
		temp = ctsvc_stmt_get_text(stmt, i++);
		contact_ringtone_path = SAFE_STRDUP(temp);
		temp = ctsvc_stmt_get_text(stmt, i++);
		contact_vibration = SAFE_STRDUP(temp);
		temp = ctsvc_stmt_get_text(stmt, i++);
		contact_message_alert = SAFE_STRDUP(temp);
		is_favorite = ctsvc_stmt_get_int(stmt, i++);

		link_count++;

		if (display_name_source_type < contact_display_name_source_type) {
			display_name_source_type = contact_display_name_source_type;
			name_contact_id = contact_id;
		} else if (contact_display_name_source_type == display_name_source_type) {
			if (name_contact_id != person_name_contact_id && person_name_contact_id != 0)
				name_contact_id = person_name_contact_id;
			else if (person_name_contact_id == 0 && name_contact_id == 0)
				name_contact_id = contact_id;
		}

		addr_len = snprintf(addr, sizeof(addr), "%d%s", addressbook_id, ADDRESSBOOK_ID_DELIM);
		if (NULL == addressbook_ids)
			addressbook_ids = calloc(addressbooks_len+1, sizeof(char));
		else if (addressbooks_len <= strlen(addressbook_ids)+addr_len) {
			addressbooks_len = MAX(addressbooks_len*2, strlen(addressbook_ids)+addr_len+1);
			addressbook_ids = realloc(addressbook_ids, addressbooks_len);
		}

		len += snprintf(addressbook_ids + len, addressbooks_len -len, "%d%s", addressbook_id, ADDRESSBOOK_ID_DELIM);

		if (NULL == image_thumbnail_path && contact_image_thumbnail_path && *contact_image_thumbnail_path) {
			temp = __ctsvc_get_image_filename(contact_image_thumbnail_path);
			image_thumbnail_path = SAFE_STRDUP(temp);
			/* update data table : is_primary_default */
		}
		free(contact_image_thumbnail_path);

		temp_str = contact_ringtone_path;
		if (NULL == ringtone_path && temp_str && strlen(temp_str))
			ringtone_path = SAFE_STRDUP(temp_str);
		free(contact_ringtone_path);

		temp_str = contact_vibration;
		if (NULL == vibration && temp_str && strlen(temp_str))
			vibration = SAFE_STRDUP(temp_str);
		free(contact_vibration);

		temp_str = contact_message_alert;
		if (NULL == message_alert && temp_str && strlen(temp_str))
			message_alert = SAFE_STRDUP(temp_str);
		free(contact_message_alert);

		if (is_favorite)
			person_is_favorite = true;
	}
	ctsvc_stmt_finalize(stmt);
	version = ctsvc_get_next_ver();

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_PERSONS" SET dirty=0, name_contact_id = %d, changed_ver = %d, "
			"has_phonenumber = EXISTS(SELECT contact_id FROM "CTS_TABLE_CONTACTS" "
			"WHERE person_id = %d AND has_phonenumber = 1 AND deleted = 0), "
			"has_email = EXISTS(SELECT contact_id FROM "CTS_TABLE_CONTACTS" "
			"WHERE person_id = %d AND has_email = 1 AND deleted = 0), "
			"link_count = %d, addressbook_ids = ?, ringtone_path=?, vibration=?, message_alert=?, status=?, image_thumbnail_path=? "
			"WHERE person_id = %d ",
			name_contact_id, version, person_id,
			person_id, link_count, person_id);

	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		free(addressbook_ids);
		free(image_thumbnail_path);
		free(ringtone_path);
		free(vibration);
		free(message_alert);
		free(status);
		return ret;
	}

	if (addressbook_ids)
		ctsvc_stmt_bind_text(stmt, 1, addressbook_ids);
	if (ringtone_path)
		ctsvc_stmt_bind_text(stmt, 2, ringtone_path);
	if (vibration)
		ctsvc_stmt_bind_text(stmt, 3, vibration);
	if (message_alert)
		ctsvc_stmt_bind_text(stmt, 4, message_alert);
	if (status)
		ctsvc_stmt_bind_text(stmt, 5, status);
	if (image_thumbnail_path)
		ctsvc_stmt_bind_text(stmt, 6, image_thumbnail_path);

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		free(addressbook_ids);
		free(image_thumbnail_path);
		free(ringtone_path);
		free(vibration);
		free(message_alert);
		free(status);
		return ret;
	}

	ctsvc_stmt_finalize(stmt);

	free(addressbook_ids);
	free(image_thumbnail_path);
	free(ringtone_path);
	free(vibration);
	free(message_alert);
	free(status);

	if (false == person_is_favorite) {
		snprintf(query, sizeof(query),
				"DELETE FROM "CTS_TABLE_FAVORITES" WHERE person_id = %d", person_id);
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			ERR("ctsvc_query_exec() Fail(%d)", ret);
			return ret;
		}
	}

	ctsvc_set_person_noti();
#ifdef _CONTACTS_IPC_SERVER
	ctsvc_change_subject_add_changed_person_id(CONTACTS_CHANGE_UPDATED, person_id);
#endif

	return CONTACTS_ERROR_NONE;
}

static bool __ctsvc_get_person_favorite_info(int person_id, double *priority)
{
	int ret;
	cts_stmt stmt;
	char query[CTS_SQL_MIN_LEN] = {0};
	snprintf(query, sizeof(query),
			"SELECT favorite_prio FROM "CTS_TABLE_FAVORITES" WHERE person_id = %d", person_id);
	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 == ret) {
		*priority = ctsvc_stmt_get_dbl(stmt, 0);
		ctsvc_stmt_finalize(stmt);
		return true;
	}
	ctsvc_stmt_finalize(stmt);
	return false;
}

int ctsvc_person_link_person(int base_person_id, int person_id)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};
	int default_number_id = 0;
	int default_email_id = 0;
	int default_image_id = 0;
	contacts_record_h record = NULL;
	bool base_is_favorite = false;
	bool is_favorite = false;
	double favorite_prio = 0.0;

	RETVM_IF(base_person_id == person_id, CONTACTS_ERROR_INVALID_PARAMETER,
			"base_person_id(%d), person_id(%d)", base_person_id, person_id);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	ret = __ctsvc_get_person_value(CTSVC_GET_PERSON_DEFAULT_NUMBER_VALUE, base_person_id, &record);
	if (CONTACTS_ERROR_NONE != ret) {
		ret = __ctsvc_get_person_value(CTSVC_GET_PERSON_DEFAULT_NUMBER_VALUE, person_id, &record);
		if (CONTACTS_ERROR_NONE == ret) {
			contacts_record_get_int(record, CTSVC_PROPERTY_NUMBER_ID, &default_number_id);
			contacts_record_destroy(record, true);
		}
	} else {
		contacts_record_get_int(record, CTSVC_PROPERTY_NUMBER_ID, &default_number_id);
		contacts_record_destroy(record, true);
	}

	ret = __ctsvc_get_person_value(CTSVC_GET_PERSON_DEFAULT_EMAIL_VALUE, base_person_id, &record);
	if (CONTACTS_ERROR_NONE != ret) {
		ret = __ctsvc_get_person_value(CTSVC_GET_PERSON_DEFAULT_EMAIL_VALUE, person_id, &record);
		if (CONTACTS_ERROR_NONE == ret) {
			contacts_record_get_int(record, CTSVC_PROPERTY_EMAIL_ID, &default_email_id);
			contacts_record_destroy(record, true);
		}
	} else {
		contacts_record_get_int(record, CTSVC_PROPERTY_EMAIL_ID, &default_email_id);
		contacts_record_destroy(record, true);
	}

	ret = __ctsvc_get_person_value(CTSVC_GET_PERSON_DEFAULT_IMAGE_VALUE, base_person_id, &record);
	if (CONTACTS_ERROR_NONE != ret) {
		ret = __ctsvc_get_person_value(CTSVC_GET_PERSON_DEFAULT_IMAGE_VALUE, person_id, &record);
		if (CONTACTS_ERROR_NONE == ret) {
			contacts_record_get_int(record, CTSVC_PROPERTY_IMAGE_ID, &default_image_id);
			contacts_record_destroy(record, true);
		}
	} else {
		contacts_record_get_int(record, CTSVC_PROPERTY_IMAGE_ID, &default_image_id);
		contacts_record_destroy(record, true);
	}

	base_is_favorite = __ctsvc_get_person_favorite_info(base_person_id, &favorite_prio);
	if (false == base_is_favorite)
		is_favorite = __ctsvc_get_person_favorite_info(person_id, &favorite_prio);

	snprintf(query, sizeof(query),
			"UPDATE %s SET person_id = %d WHERE person_id = %d AND deleted = 0",
			CTS_TABLE_CONTACTS, base_person_id, person_id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_person_aggregate(base_person_id);

	if (default_number_id)
		__ctsvc_put_person_default_data(base_person_id, default_number_id, CTSVC_DATA_NUMBER);

	if (default_email_id)
		__ctsvc_put_person_default_data(base_person_id, default_email_id, CTSVC_DATA_EMAIL);

	if (default_image_id)
		__ctsvc_put_person_default_image(base_person_id, default_image_id);

#ifdef ENABLE_LOG_FEATURE
	/*
	 * update phonelog
	 * Updating phonelog person_id before deleting person
	 * Because, when deleting, ctsvc_db_person_delete_callback will be called
	 * the logic takes more time to find proper person_id (base_person_id)
	 */
	ctsvc_db_phone_log_update_person_id(NULL, person_id, base_person_id, true);
#endif /* ENABLE_LOG_FEATURE */

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE person_id = %d",
			CTS_TABLE_PERSONS, person_id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}
	/*
	 *#ifdef _CONTACTS_IPC_SERVER
	 *  It will be added in ctsvc_db_person_delete_callback
	 *  ctsvc_change_subject_add_changed_person_id(CONTACTS_CHANGE_DELETED, person_id);
	 *#endif
	 */

	if (is_favorite) {
		snprintf(query, sizeof(query),
				"INSERT INTO "CTS_TABLE_FAVORITES" values(%d, %f)", base_person_id, favorite_prio);
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			ERR("ctsvc_query_exec() Fail(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
	}

	ctsvc_set_person_noti();
	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		ERR("ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	} else {
		return CONTACTS_ERROR_NONE;
	}
}

static int __ctsvc_update_primary_default_data(int person_id)
{
	int ret;
	contacts_record_h record = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};
	cts_stmt stmt;

	ret = __ctsvc_get_person_value(CTSVC_GET_PERSON_DEFAULT_NUMBER_VALUE, person_id, &record);
	if (CONTACTS_ERROR_NONE != ret) {
		snprintf(query, sizeof(query),
				"SELECT contact_id "
				"FROM %s "
				"WHERE person_id = %d AND deleted = 0 "
				"ORDER BY contact_id",
				CTS_TABLE_CONTACTS, person_id);
		ret = ctsvc_query_prepare(query, &stmt);
		RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

		while (1 == ctsvc_stmt_step(stmt)) {
			int contact_id = ctsvc_stmt_get_int(stmt, 0);
			cts_stmt stmt_number;

			snprintf(query, sizeof(query),
					"SELECT id, is_default FROM %s "
					"WHERE contact_id = %d AND datatype = %d AND is_default = 1 AND is_my_profile = 0",
					CTS_TABLE_DATA, contact_id, CTSVC_DATA_NUMBER);

			ret = ctsvc_query_prepare(query, &stmt_number);
			if (NULL == stmt_number) {
				ERR("ctsvc_query_prepare() Fail(%d)", ret);
				ctsvc_stmt_finalize(stmt);
				return ret;
			}

			if (1 == ctsvc_stmt_step(stmt_number)) {
				int default_number_id = ctsvc_stmt_get_int(stmt_number, 0);
				__ctsvc_put_person_default_data(person_id, default_number_id, CTSVC_DATA_NUMBER);
				ctsvc_stmt_finalize(stmt_number);
				break;
			}
			ctsvc_stmt_finalize(stmt_number);
		}
		ctsvc_stmt_finalize(stmt);
	} else {
		contacts_record_destroy(record, true);
	}

	ret = __ctsvc_get_person_value(CTSVC_GET_PERSON_DEFAULT_EMAIL_VALUE, person_id, &record);
	if (CONTACTS_ERROR_NONE != ret) {
		snprintf(query, sizeof(query),
				"SELECT contact_id "
				"FROM %s "
				"WHERE person_id = %d AND deleted = 0 "
				"ORDER BY contact_id",
				CTS_TABLE_CONTACTS, person_id);
		ret = ctsvc_query_prepare(query, &stmt);
		RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

		while (1 == ctsvc_stmt_step(stmt)) {
			int contact_id = ctsvc_stmt_get_int(stmt, 0);
			cts_stmt stmt_email;

			snprintf(query, sizeof(query),
					"SELECT id, is_default FROM %s "
					"WHERE contact_id = %d AND datatype = %d AND is_default = 1 AND is_my_profile = 0",
					CTS_TABLE_DATA, contact_id, CTSVC_DATA_EMAIL);

			ret = ctsvc_query_prepare(query, &stmt_email);
			if (NULL == stmt_email) {
				ERR("ctsvc_query_prepare() Fail(%d)", ret);
				ctsvc_stmt_finalize(stmt);
				return ret;
			}

			if (1 == ctsvc_stmt_step(stmt_email)) {
				int default_email_id = ctsvc_stmt_get_int(stmt_email, 0);
				__ctsvc_put_person_default_data(person_id, default_email_id, CTSVC_DATA_EMAIL);
				ctsvc_stmt_finalize(stmt_email);
				break;
			}
			ctsvc_stmt_finalize(stmt_email);
		}
		ctsvc_stmt_finalize(stmt);
	} else {
		contacts_record_destroy(record, true);
	}

	ret = __ctsvc_get_person_value(CTSVC_GET_PERSON_DEFAULT_IMAGE_VALUE, person_id, &record);
	if (CONTACTS_ERROR_NONE != ret) {
		snprintf(query, sizeof(query),
				"SELECT contact_id "
				"FROM %s "
				"WHERE person_id = %d AND deleted = 0 "
				"ORDER BY contact_id",
				CTS_TABLE_CONTACTS, person_id);
		ret = ctsvc_query_prepare(query, &stmt);
		RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

		while (1 == ctsvc_stmt_step(stmt)) {
			int contact_id = ctsvc_stmt_get_int(stmt, 0);
			cts_stmt stmt_image;

			snprintf(query, sizeof(query),
					"SELECT id, is_default FROM %s "
					"WHERE contact_id = %d AND datatype = %d AND is_default = 1 AND is_my_profile = 0",
					CTS_TABLE_DATA, contact_id, CTSVC_DATA_IMAGE);

			ret = ctsvc_query_prepare(query, &stmt_image);
			if (NULL == stmt_image) {
				ERR("ctsvc_query_prepare() Fail(%d)", ret);
				ctsvc_stmt_finalize(stmt);
				return ret;
			}

			if (1 == ctsvc_stmt_step(stmt_image)) {
				int default_image_id = ctsvc_stmt_get_int(stmt_image, 0);
				__ctsvc_put_person_default_image(person_id, default_image_id);
				ctsvc_stmt_finalize(stmt_image);
				break;
			}
			ctsvc_stmt_finalize(stmt_image);
		}
		ctsvc_stmt_finalize(stmt);
	} else {
		contacts_record_destroy(record, true);
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_person_unlink_contact(int person_id, int contact_id, int *out_person_id)
{
	int ret;
	int id;
	int link_count = 0;
	char query[CTS_SQL_MIN_LEN] = {0};
	contacts_record_h record = NULL;
	bool is_favorite = false;
	double priority = 0.0;

	RETVM_IF(person_id <= 0 || contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"person_id(%d), person_id(%d)", person_id, person_id);

	if (out_person_id)
		*out_person_id = 0;

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT link_count FROM "CTS_TABLE_PERSONS" WHERE person_id=%d", person_id);
	ret = ctsvc_query_get_first_int_result(query, &link_count);
	if (CONTACTS_ERROR_NONE != ret) {
		ctsvc_end_trans(false);
		return ret;
	}

	if (link_count == 1) {
		ERR("This person(%d) has one contact(%d)", person_id, contact_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	ret = ctsvc_db_contact_get(contact_id, (contacts_record_h*)&record);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_db_contact_get() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	/* create new person */
	id = ctsvc_db_insert_person(record);
	if (id < CONTACTS_ERROR_NONE) {
		ERR("ctsvc_db_insert_person() Fail(%d)", id);
		ctsvc_end_trans(false);
		contacts_record_destroy(record, true);
		return id;
	}

	/* insert statistic info for new person */
	snprintf(query, sizeof(query),
			"INSERT INTO %s (person_id, usage_type, times_used) "
			"SELECT %d, usage_type, times_used FROM %s WHERE person_id = %d",
			CTS_TABLE_CONTACT_STAT, id, CTS_TABLE_CONTACT_STAT, person_id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		contacts_record_destroy(record, true);
		return ret;
	}

	is_favorite = __ctsvc_get_person_favorite_info(person_id, &priority);

	/* update person_id of unlinked contact */
	snprintf(query, sizeof(query),
			"UPDATE %s SET person_id = %d WHERE contact_id = %d",
			CTS_TABLE_CONTACTS, id, contact_id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		contacts_record_destroy(record, true);
		return ret;
	}

	/* update bsae person info */
	ret = ctsvc_person_aggregate(person_id);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_person_aggregate(%d) Fail(%d)", person_id, ret);
		ctsvc_end_trans(false);
		contacts_record_destroy(record, true);
		return ret;
	}

	if (is_favorite && ((ctsvc_contact_s*)record)->is_favorite) {
		snprintf(query, sizeof(query),
				"INSERT OR REPLACE INTO "CTS_TABLE_FAVORITES" values(%d, %f)", id, priority);
		ret = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != ret) {
			ERR("ctsvc_query_exec() Fail(%d)", ret);
			ctsvc_end_trans(false);
			contacts_record_destroy(record, true);
			return ret;
		}
	}
	contacts_record_destroy(record, true);

	__ctsvc_update_primary_default_data(person_id);

#ifdef ENABLE_LOG_FEATURE
	/* update phonelog */
	ctsvc_db_phone_log_update_person_id(NULL, person_id, id, false);
#endif /* ENABLE_LOG_FEATURE */

	if (out_person_id)
		*out_person_id = id;
	ctsvc_set_person_noti();
	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		ERR("ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	} else {
		return CONTACTS_ERROR_NONE;
	}
}

int ctsvc_person_do_garbage_collection(void)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "SELECT person_id FROM "CTS_TABLE_PERSONS" WHERE dirty=1");

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	while (1 /*CTS_TRUE*/ == ctsvc_stmt_step(stmt)) {
		int person_id;
		person_id = ctsvc_stmt_get_int(stmt, 0);
		ctsvc_person_aggregate(person_id);
#ifdef ENABLE_LOG_FEATURE
		/* update phonelog */
		ctsvc_db_phone_log_update_person_id(NULL, person_id, -1, false);
#endif /* ENABLE_LOG_FEATURE */
	}
	ctsvc_stmt_finalize(stmt);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_person_reset_usage(int person_id, contacts_usage_type_e type)
{
	int ret ;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETVM_IF(person_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "contact_id should be greater than 0");

	snprintf(query, sizeof(query),
			"UPDATE %s SET times_used = 0 WHERE person_id = %d AND usage_type = %d",
			CTS_TABLE_CONTACT_STAT, person_id, type);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		ERR("ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	} else {
		return CONTACTS_ERROR_NONE;
	}
}

int ctsvc_person_set_favorite_order(int person_id, int front_person_id, int back_person_id)
{
	int ret;
	double front_prio = 0.0;
	double back_prio = 0.0;
	double prio;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query), "SELECT favorite_prio FROM "CTS_TABLE_FAVORITES" WHERE person_id = ?");

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	ctsvc_stmt_bind_int(stmt, 1, front_person_id);
	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/  == ret)
		front_prio = ctsvc_stmt_get_dbl(stmt, 0);
	ctsvc_stmt_reset(stmt);
	ctsvc_stmt_bind_int(stmt, 1, back_person_id);
	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ == ret)
		back_prio = ctsvc_stmt_get_dbl(stmt, 0);
	ctsvc_stmt_finalize(stmt);

	RETVM_IF(0.0 == front_prio && 0.0 == back_prio, CONTACTS_ERROR_INVALID_PARAMETER,
			"The indexes for front and back are invalid.");

	if (0.0 == back_prio)
		prio = front_prio + 1;
	else
		prio = (front_prio + back_prio) / 2;

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query),
			"UPDATE %s SET favorite_prio = %f WHERE person_id = %d",
			CTS_TABLE_FAVORITES, prio, person_id);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret)	{
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_person_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		ERR("ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	} else {
		return CONTACTS_ERROR_NONE;
	}
}

int ctsvc_person_set_default_property(contacts_person_property_e property, int person_id,
		int id)
{
	int ret;

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, CONTACTS_ERROR_DB, "ctsvc_begin_trans() Fail(%d)", ret);

	switch (property) {
	case CONTACTS_PERSON_PROPERTY_NAME_CONTACT:
		ret = __ctsvc_put_person_default_name(person_id, id);		/* contact id */
		break;
	case CONTACTS_PERSON_PROPERTY_NUMBER:
		ret = __ctsvc_put_person_default_data(person_id, id, CTSVC_DATA_NUMBER);	/* number id */
		break;
	case CONTACTS_PERSON_PROPERTY_EMAIL:
		ret = __ctsvc_put_person_default_data(person_id, id, CTSVC_DATA_EMAIL);		/* email id */
		break;
	case CONTACTS_PERSON_PROPERTY_IMAGE:
		ret = __ctsvc_put_person_default_image(person_id, id);		/* image id */
		break;
	default:
		ret = CONTACTS_ERROR_INVALID_PARAMETER;
		break;
	}
	if (ret < CONTACTS_ERROR_NONE) {
		ERR("contacts_person_set_default_property() Fail(%d) : person property (%d)", ret, property);
		ctsvc_end_trans(false);
		return ret;
	}

#ifdef _CONTACTS_IPC_SERVER
	ctsvc_change_subject_add_changed_person_id(CONTACTS_CHANGE_UPDATED, person_id);
#endif
	ctsvc_set_person_noti();
	ret = ctsvc_end_trans(true);

	return ret;
}

int ctsvc_person_get_default_property(contacts_person_property_e property, int person_id,
		int *id)
{
	int ret = CONTACTS_ERROR_NONE;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETVM_IF(person_id <= 0 || id == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "id should be greater than 0");
	*id = 0;

	switch (property) {
	case CONTACTS_PERSON_PROPERTY_NAME_CONTACT:
		snprintf(query, sizeof(query),
				"SELECT name_contact_id FROM "CTS_TABLE_PERSONS" WHERE person_id = %d",
				person_id);
		break;
	case CONTACTS_PERSON_PROPERTY_NUMBER:
		snprintf(query, sizeof(query),
				"SELECT id FROM "CTS_TABLE_DATA" WHERE is_primary_default = 1 AND datatype = %d AND is_my_profile = 0 AND "
				"contact_id in (SELECT contact_id FROM "CTS_TABLE_CONTACTS" "
				"WHERE person_id = %d AND deleted = 0)",
				CTSVC_DATA_NUMBER, person_id);
		break;
	case CONTACTS_PERSON_PROPERTY_EMAIL:
		snprintf(query, sizeof(query),
				"SELECT id FROM "CTS_TABLE_DATA" WHERE is_primary_default = 1 AND datatype = %d AND is_my_profile = 0 AND "
				"contact_id in (SELECT contact_id FROM "CTS_TABLE_CONTACTS" "
				"WHERE person_id = %d AND deleted = 0)",
				CTSVC_DATA_EMAIL, person_id);
		break;
	case CONTACTS_PERSON_PROPERTY_IMAGE:
		snprintf(query, sizeof(query),
				"SELECT id FROM "CTS_TABLE_DATA" WHERE is_primary_default = 1 AND datatype = %d AND is_my_profile = 0 AND "
				"contact_id in (SELECT contact_id FROM "CTS_TABLE_CONTACTS" "
				"WHERE person_id = %d AND deleted = 0)",
				CTSVC_DATA_IMAGE, person_id);
		break;
	default:
		ret = CONTACTS_ERROR_INVALID_PARAMETER;
		break;
	}

	if (*query) {
		int result = 0;
		ret = ctsvc_query_get_first_int_result(query, &result);
		RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "ctsvc_query_get_first_int_result() Fail(%d)", ret);
		*id = result;
	}

	return ret;
}

static void __ctsvc_make_condition_names(contacts_record_h record, char **names,
		int *names_size, int *names_len)
{
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;
	char comp_name[CTSVC_COMP_NAME_LEN] = {0,};
	char *name = NULL;

	if (CTSVC_SORT_KOREAN == contact->display_name_language)
		name = contact->display_name;
	else
		name = contact->reverse_display_name;

	if (name && CTSVC_COMP_NAME_LEN <= strlen(name)) {
		if (*names_len)
			*names_len += SAFE_SNPRINTF(names, names_size, *names_len, " OR ");

		if (name == contact->display_name)
			*names_len += SAFE_SNPRINTF(names, names_size, *names_len, "display_name");
		else
			*names_len += SAFE_SNPRINTF(names, names_size, *names_len, "reverse_display_name");

		*names_len += SAFE_SNPRINTF(names, names_size, *names_len, " LIKE ");

		*names_len += SAFE_SNPRINTF(names, names_size, *names_len, "'");

		if (CTSVC_SORT_KOREAN == contact->display_name_language) { /*compare first name*/
			*names_len += SAFE_SNPRINTF(names, names_size, *names_len, "%");

			strncpy(comp_name, name + (strlen(name) -CTSVC_COMP_NAME_LEN), CTSVC_COMP_NAME_LEN);
			*names_len += SAFE_SNPRINTF(names, names_size, *names_len, comp_name);
		} else {
			strncpy(comp_name, name, CTSVC_COMP_NAME_LEN);
			*names_len += SAFE_SNPRINTF(names, names_size, *names_len, comp_name);

			*names_len += SAFE_SNPRINTF(names, names_size, *names_len, "%");
		}
		*names_len += SAFE_SNPRINTF(names, names_size, *names_len, "'");
	}

	DBG("names : %s", *names);
}


static void __ctsvc_make_condition_emails(contacts_record_h record, char **emails,
		int *emails_size, int *emails_len)
{
	GList *cursor = NULL;
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;
	ctsvc_email_s *email_data;
	int local_part_len = 0;
	char local_part[256] = {0,};

	if (NULL == contact->emails) {
		DBG("NULL == contact->emails");
		return;
	}

	for (cursor = contact->emails->records; cursor; cursor = cursor->next) {
		email_data = cursor->data;
		if (email_data && email_data->email_addr && email_data->email_addr[0]) {
			local_part_len = strcspn(email_data->email_addr, "@");
			if (local_part_len <= 0 || strlen(email_data->email_addr) <= local_part_len)
				continue;

			if (*emails_len)
				*emails_len += SAFE_SNPRINTF(emails, emails_size, *emails_len, " OR ");

			*emails_len += SAFE_SNPRINTF(emails, emails_size, *emails_len, "D.data3");

			*emails_len += SAFE_SNPRINTF(emails, emails_size, *emails_len, " LIKE ");

			*emails_len += SAFE_SNPRINTF(emails, emails_size, *emails_len, "'");

			strncpy(local_part, email_data->email_addr, local_part_len + 1);
			*emails_len += SAFE_SNPRINTF(emails, emails_size, *emails_len, local_part);

			*emails_len += SAFE_SNPRINTF(emails, emails_size, *emails_len, "%");

			*emails_len += SAFE_SNPRINTF(emails, emails_size, *emails_len, "'");
		}
	}
	DBG("emails : %s", *emails);
}

static void __ctsvc_make_condition_numbers(contacts_record_h record, char **numbers,
		int *numbers_size, int *numbers_len)
{
	int ret = CONTACTS_ERROR_NONE;
	GList *cursor = NULL;
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;
	ctsvc_number_s *number_data;

	if (NULL == contact->numbers) {
		DBG("NULL == contact->numbers");
		return;
	}

	for (cursor = contact->numbers->records; cursor; cursor = cursor->next) {
		number_data = cursor->data;
		if (number_data && number_data->number && number_data->number[0]) {
			int number_len = SAFE_STRLEN(number_data->number);
			char clean_num[number_len + 1];
			ret = ctsvc_clean_number(number_data->number, clean_num, sizeof(clean_num), true);
			if (ret <= 0)
				continue;

			char normal_num[sizeof(clean_num) + 20];
			ret = ctsvc_normalize_number(clean_num, normal_num, sizeof(normal_num), true);
			char minmatch[sizeof(normal_num) + 1];
			if (ret <= 0)
				continue;

			ret = ctsvc_get_minmatch_number(normal_num, minmatch, sizeof(minmatch),
				ctsvc_get_phonenumber_min_match_digit());

			if (CONTACTS_ERROR_NONE != ret)
				continue;

			if (*numbers_len)
				*numbers_len += SAFE_SNPRINTF(numbers, numbers_size, *numbers_len, ", ");

			*numbers_len += SAFE_SNPRINTF(numbers, numbers_size, *numbers_len, "'");

			*numbers_len += SAFE_SNPRINTF(numbers, numbers_size, *numbers_len, minmatch);

			*numbers_len += SAFE_SNPRINTF(numbers, numbers_size, *numbers_len, "'");
		}
	}
	DBG("numbers : %s", *numbers);
}

static int __ctsvc_find_aggregable_person(int person_id, GSList *contact_ids, contacts_list_h *out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	GSList *cursor = NULL;
	char *query = NULL;
	char *sub_query = NULL;
	char *cond_numbers = NULL;
	char *cond_emails = NULL;
	char *cond_names = NULL;
	int cond_numbers_size = CTS_SQL_MIN_LEN;
	int cond_emails_size = CTS_SQL_MIN_LEN;
	int cond_names_size = CTS_SQL_MIN_LEN;
	int cond_numbers_len = 0;
	int cond_emails_len = 0;
	int cond_names_len = 0;
	int query_size = 0;
	int sub_query_size = 0;
	int sub_query_len = 0;

	contacts_list_h list = NULL;
	cts_stmt stmt = NULL;

	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);

	cond_numbers = calloc(1, CTS_SQL_MIN_LEN);
	if (NULL == cond_numbers) {
		ERR("calloc() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	cond_emails = calloc(1, CTS_SQL_MIN_LEN);
	if (NULL == cond_emails) {
		ERR("calloc() Fail");
		free(cond_numbers);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	cond_names = calloc(1, CTS_SQL_MIN_LEN);
	if (NULL == cond_names) {
		ERR("calloc() Fail");
		free(cond_numbers);
		free(cond_emails);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	for (cursor = contact_ids; cursor; cursor = cursor->next) {
		contacts_record_h record = NULL;

		int contact_id = GPOINTER_TO_INT(cursor->data);
		ret = ctsvc_db_contact_get(contact_id, &record);
		if (CONTACTS_ERROR_NONE != ret) {
			WARN("ctsvc_db_contact_get() Fail(%d), contact_id = %d", ret, contact_id);
			continue;
		}

		__ctsvc_make_condition_numbers(record, &cond_numbers, &cond_numbers_size, &cond_numbers_len);
		__ctsvc_make_condition_emails(record, &cond_emails, &cond_emails_size, &cond_emails_len);
		__ctsvc_make_condition_names(record, &cond_names, &cond_names_size, &cond_names_len);

		contacts_record_destroy(record, true);
	}

	sub_query_size = CTS_SQL_MIN_LEN + cond_numbers_len + cond_emails_len + cond_names_len;
	sub_query = calloc(1, sub_query_size);
	if (NULL == sub_query) {
		ERR("calloc() Fail");
		free(cond_numbers);
		free(cond_emails);
		free(cond_names);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	if (cond_numbers_len) {
		sub_query_len += snprintf(sub_query, sub_query_size,
					"SELECT C.person_id FROM "CTS_TABLE_CONTACTS" C, "CTS_TABLE_DATA" D "
					"ON C.contact_id=D.contact_id AND D.datatype=%d AND C.deleted = 0 "
					"AND C.person_id <> %d AND D.is_my_profile = 0 "
					"WHERE D.data4 IN (%s)",
					CTSVC_DATA_NUMBER, person_id, cond_numbers);
	}

	if (cond_emails_len) {
		if (sub_query_len)
			sub_query_len += snprintf(sub_query + sub_query_len, sub_query_size -sub_query_len, " UNION ");

		sub_query_len += snprintf(sub_query + sub_query_len, sub_query_size -sub_query_len,
					"SELECT C.person_id FROM "CTS_TABLE_CONTACTS" C, "CTS_TABLE_DATA" D "
					"ON C.contact_id=D.contact_id AND D.datatype=%d AND C.deleted = 0 "
					"AND C.person_id <> %d AND D.is_my_profile = 0 "
					"WHERE %s",
					CTSVC_DATA_EMAIL, person_id, cond_emails);
	}

	if (cond_names_len) {
		if (sub_query_len)
			sub_query_len += snprintf(sub_query + sub_query_len, sub_query_size -sub_query_len, " UNION ");

		sub_query_len += snprintf(sub_query + sub_query_len, sub_query_size -sub_query_len,
					"SELECT person_id FROM "CTS_TABLE_CONTACTS" "
					"WHERE person_id <> %d AND (%s)",
					person_id, cond_names);
	}

	free(cond_numbers);
	free(cond_emails);
	free(cond_names);

	DBG("sub_query : %s", sub_query);

	query_size = CTS_SQL_MIN_LEN + sub_query_len;
	query = calloc(1, query_size);
	if (NULL == query) {
		ERR("calloc() Fail");
		free(sub_query);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	snprintf(query, query_size,
			"SELECT DISTINCT persons.person_id, "
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
			"WHERE persons.person_id IN (%s)",
			ctsvc_get_display_column(), ctsvc_get_sort_name_column(), sub_query);

	free(sub_query);

	ret = ctsvc_query_prepare(query, &stmt);
	free(query);
	if (NULL == stmt) {
		ERR("ctsvc_query_prepare fail(%d)", ret);
		return ret;
	}

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}
		ret = ctsvc_db_person_create_record_from_stmt(stmt, &record);
		if (CONTACTS_ERROR_NONE != ret) {
			ERR("ctsvc_db_person_create_record_from_stmt() Fail(%d)", ret);
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


int ctsvc_person_search_aggregable(int person_id, contacts_list_h *out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	cts_stmt stmt;
	char query[CTS_SQL_MIN_LEN] = {0};
	GSList *contact_ids = NULL;

	RETV_IF(person_id <= 0 || NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);

	snprintf(query, sizeof(query),
			"SELECT contact_id FROM "CTS_TABLE_CONTACTS" "
			"WHERE deleted = 0 AND person_id = %d", person_id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	while ((ret = ctsvc_stmt_step(stmt))) {
		int contact_id = 0;

		if (1 != ret) {
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			return ret;
		}
		contact_id = ctsvc_stmt_get_int(stmt, 0);

		contact_ids = g_slist_append(contact_ids, GINT_TO_POINTER(contact_id));
	}
	ctsvc_stmt_finalize(stmt);

	if (NULL == contact_ids) {
		ERR("Fail to get contacts by person_id");
		return CONTACTS_ERROR_DB;
	}

	ret =  __ctsvc_find_aggregable_person(person_id, contact_ids, out_list);

	g_slist_free(contact_ids);

	return ret;
}


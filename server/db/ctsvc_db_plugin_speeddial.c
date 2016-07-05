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
#include "ctsvc_db_utils.h"
#include "ctsvc_list.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_query.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"
#include "ctsvc_notify.h"


static int __ctsvc_db_speeddial_insert_record(contacts_record_h record, int *id)
{
	int ret;
	int number_id = 0;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_speeddial_s *speeddial = (ctsvc_speeddial_s*)record;

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	/* check number_id validation */
	snprintf(query, sizeof(query),
			"SELECT data.id FROM "CTS_TABLE_DATA", "CTS_TABLE_CONTACTS" "
			"ON "CTS_TABLE_DATA".contact_id = "CTS_TABLE_CONTACTS".contact_id "
			"AND contacts.deleted = 0  AND is_my_profile = 0 AND datatype = %d "
			"WHERE id = %d ",
			CONTACTS_DATA_TYPE_NUMBER, speeddial->number_id);
	ret = ctsvc_query_get_first_int_result(query, &number_id);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_get_first_int_result() Fail(%d) : number_id is invalid", ret);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	snprintf(query, sizeof(query),
			"INSERT INTO "CTS_TABLE_SPEEDDIALS"(speed_number, number_id) VALUES(%d, %d)",
			speeddial->dial_number, speeddial->number_id);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_change();
	if (0 < ret) {
		if (id)
			*id  = speeddial->dial_number;
		ctsvc_set_speed_noti();
	} else {
		/* LCOV_EXCL_START */
		ERR("already exist");
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
		return ret;
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_speeddial_value_set(cts_stmt stmt, contacts_record_h *record)
{
	int i;
	int ret;
	char *temp;
	ctsvc_speeddial_s *speeddial;

	ret = contacts_record_create(_contacts_speeddial._uri, record);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create Fail(%d)", ret);
	speeddial = (ctsvc_speeddial_s*)*record;

	i = 0;
	speeddial->person_id = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	speeddial->display_name = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	if (temp) {
		char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		snprintf(full_path, sizeof(full_path), "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, temp);
		speeddial->image_thumbnail_path = strdup(full_path);
	}

	speeddial->number_id = ctsvc_stmt_get_int(stmt, i++);
	speeddial->number_type = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	speeddial->label = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	speeddial->number = SAFE_STRDUP(temp);
	speeddial->dial_number = ctsvc_stmt_get_int(stmt, i++);
	speeddial->id = speeddial->dial_number; /* dial_number is an unique key */

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_speeddial_get_record(int id, contacts_record_h *out_record)
{
	int ret;
	cts_stmt stmt = NULL;
	contacts_record_h record;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT person_id, %s, image_thumbnail_path, number_id, "
			"type, label, number, speed_number  "
			"FROM "CTSVC_DB_VIEW_SPEEDIDAL " "
			"WHERE speed_number = %d",
			ctsvc_get_display_column(), id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = __ctsvc_db_speeddial_value_set(stmt, &record);

	ctsvc_stmt_finalize(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("__ctsvc_db_speeddial_value_set(ALL) Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	*out_record = record;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_speeddial_update_record(contacts_record_h record)
{
	int ret;
	int number_id;
	int speeddial_id;
	cts_stmt stmt;
	char query[CTS_SQL_MIN_LEN] = {0};
	ctsvc_speeddial_s *speeddial = (ctsvc_speeddial_s*)record;

	RETVM_IF(speeddial->dial_number < 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"dial number (%d)", speeddial->dial_number);
	RETVM_IF(speeddial->number_id < 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"number id (%d)", speeddial->number_id);
	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, CONTACTS_ERROR_DB, "ctsvc_begin_trans() Fail(%d)", ret);

	/* check number_id validation */
	snprintf(query, sizeof(query),
			"SELECT data.id FROM "CTS_TABLE_DATA", "CTS_TABLE_CONTACTS" "
			"ON "CTS_TABLE_DATA".contact_id = "CTS_TABLE_CONTACTS".contact_id "
			"AND contacts.deleted = 0  AND is_my_profile = 0 AND datatype = %d "
			"WHERE id = %d ",
			CONTACTS_DATA_TYPE_NUMBER, speeddial->number_id);
	ret = ctsvc_query_get_first_int_result(query, &number_id);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_get_first_int_result() Fail(%d) : number_id is invalid", ret);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	snprintf(query, sizeof(query),
			"SELECT speed_number FROM "CTS_TABLE_SPEEDDIALS" WHERE speed_number = %d", speeddial->dial_number);
	ret = ctsvc_query_get_first_int_result(query, &speeddial_id);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_get_first_int_result() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	snprintf(query, sizeof(query), "UPDATE "CTS_TABLE_SPEEDDIALS" "
			"SET number_id = %d WHERE speed_number = %d",
			speeddial->number_id, speeddial->dial_number);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_db_change();
	ctsvc_stmt_finalize(stmt);

	if (0 < ret) {
		ctsvc_set_speed_noti();
		ret = ctsvc_end_trans(true);
	} else {
		ctsvc_end_trans(false);
		ret = CONTACTS_ERROR_NO_DATA;
	}

	if (ret < CONTACTS_ERROR_NONE)
		return ret;
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_speeddial_delete_record(int id)
{
	int ret;
	int speeddial_id;
	cts_stmt stmt;
	char query[CTS_SQL_MIN_LEN] = {0};

	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, CONTACTS_ERROR_DB, "ctsvc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT speed_number FROM "CTS_TABLE_SPEEDDIALS" WHERE speed_number = %d", id);
	ret = ctsvc_query_get_first_int_result(query, &speeddial_id);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_get_first_int_result() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE speed_number = %d",
			CTS_TABLE_SPEEDDIALS, id);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_db_change();
	ctsvc_stmt_finalize(stmt);

	if (0 < ret) {
		ctsvc_set_speed_noti();
		ret = ctsvc_end_trans(true);
	} else {
		ctsvc_end_trans(false);
		ret = CONTACTS_ERROR_NO_DATA;
	}

	if (ret < CONTACTS_ERROR_NONE)
		return ret;
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_speeddial_get_all_records(int offset, int limit, contacts_list_h *out_list)
{
	int ret;
	int len;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;

	len = snprintf(query, sizeof(query),
			"SELECT person_id, %s, image_thumbnail_path, number_id, "
			"type, label, number, speed_number	"
			"FROM "CTSVC_DB_VIEW_SPEEDIDAL " ",	ctsvc_get_display_column());

	if (0 != limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
			/* LCOV_EXCL_STOP */
		}
		__ctsvc_db_speeddial_value_set(stmt, &record);

		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_speeddial_get_records_with_query(contacts_query_h query,
		int offset, int limit, contacts_list_h *out_list)
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_speeddial_s *speeddial;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s*)query;

	ret = ctsvc_db_make_get_records_query_stmt(s_query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_db_make_get_records_query_stmt Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
			/* LCOV_EXCL_STOP */
		}

		contacts_record_create(_contacts_speeddial._uri, &record);
		speeddial = (ctsvc_speeddial_s*)record;
		if (0 == s_query->projection_count) {
			field_count = s_query->property_count;
		} else {
			field_count = s_query->projection_count;

			int err = ctsvc_record_set_projection_flags(record, s_query->projection,
					s_query->projection_count, s_query->property_count);
			if (CONTACTS_ERROR_NONE != err)
				ASSERT_NOT_REACHED("ctsvc_record_set_projection_flags() Fail(%d)", err);
		}

		for (i = 0; i < field_count; i++) {
			char *temp;
			int property_id;
			if (0 == s_query->projection_count)
				property_id = s_query->properties[i].property_id;
			else
				property_id = s_query->projection[i];

			switch (property_id) {
			case CTSVC_PROPERTY_SPEEDDIAL_DIAL_NUMBER:
				speeddial->dial_number = ctsvc_stmt_get_int(stmt, i);
				speeddial->id = speeddial->dial_number; /* dial_number is an unique key */
				break;
			case CTSVC_PROPERTY_SPEEDDIAL_NUMBER_ID:
				speeddial->number_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_SPEEDDIAL_NUMBER:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(speeddial->number);
				speeddial->number = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_SPEEDDIAL_NUMBER_LABEL:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(speeddial->label);
				speeddial->label = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_SPEEDDIAL_NUMBER_TYPE:
				speeddial->number_type = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_SPEEDDIAL_PERSON_ID:
				speeddial->person_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_SPEEDDIAL_DISPLAY_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(speeddial->display_name);
				speeddial->display_name = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_SPEEDDIAL_IMAGE_THUMBNAIL:
				temp = ctsvc_stmt_get_text(stmt, i);
				if (temp) {
					snprintf(full_path, sizeof(full_path), "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, temp);
					free(speeddial->image_thumbnail_path);
					speeddial->image_thumbnail_path = strdup(full_path);
				}
				break;
			default:
				break;
			}
		}
		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;

	return CONTACTS_ERROR_NONE;
}

ctsvc_db_plugin_info_s ctsvc_db_plugin_speeddial = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_speeddial_insert_record,
	.get_record = __ctsvc_db_speeddial_get_record,
	.update_record = __ctsvc_db_speeddial_update_record,
	.delete_record = __ctsvc_db_speeddial_delete_record,
	.get_all_records = __ctsvc_db_speeddial_get_all_records,
	.get_records_with_query = __ctsvc_db_speeddial_get_records_with_query,
	.insert_records = NULL,
	.update_records = NULL,
	.delete_records = NULL,
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};


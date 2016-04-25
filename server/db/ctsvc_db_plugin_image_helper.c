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
#include "ctsvc_db_query.h"
#include "ctsvc_db_plugin_image_helper.h"
#include "ctsvc_db_plugin_contact_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_notify.h"
#include "ctsvc_db_utils.h"

int ctsvc_db_image_get_value_from_stmt(cts_stmt stmt, contacts_record_h *record,
		int start_count)
{
	int ret;
	char *temp;
	ctsvc_image_s *image;

	ret = contacts_record_create(_contacts_image._uri, (contacts_record_h*)&image);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create Fail(%d)", ret);

	image->id = ctsvc_stmt_get_int(stmt, start_count++);
	image->contact_id = ctsvc_stmt_get_int(stmt, start_count++);
	image->is_default = ctsvc_stmt_get_int(stmt, start_count++);
	image->type = ctsvc_stmt_get_int(stmt, start_count++);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	image->label = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	if (temp) {
		char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		snprintf(full_path, sizeof(full_path), "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, temp);
		image->path = strdup(full_path);
	}

	*record = (contacts_record_h)image;
	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_image_bind_stmt(cts_stmt stmt, ctsvc_image_s *image, int start_cnt)
{
	if (image->label)
		ctsvc_stmt_bind_text(stmt, start_cnt, image->label);
	if (image->path)
		ctsvc_stmt_bind_text(stmt, start_cnt+1, image->path);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_image_reset_default(int image_id, int contact_id)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_DATA" SET is_default=0, is_primary_default=0 WHERE id != %d AND contact_id = %d AND datatype=%d",
			image_id, contact_id, CONTACTS_DATA_TYPE_IMAGE);
	ret = ctsvc_query_exec(query);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_query_exec() Fail(%d)", ret);
	return ret;
}

int ctsvc_db_image_insert(contacts_record_h record, int contact_id, bool is_my_profile, int *id)
{
	int ret;
	int image_id;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	char image_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
	ctsvc_image_s *image = (ctsvc_image_s*)record;

	/* These check should be done in client side */
	RETV_IF(NULL == image->path, CONTACTS_ERROR_NONE);
	RETVM_IF(contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"contact_id(%d) is mandatory field to insert image record", image->contact_id);
	RETVM_IF(0 < image->id, CONTACTS_ERROR_INVALID_PARAMETER,
			"id(%d), This record is already inserted", image->id);

	ret = ctsvc_have_file_read_permission(image->path);
	RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "ctsvc_have_file_read_permission fail(%d)", ret);

	image_id = ctsvc_db_get_next_id(CTS_TABLE_DATA);
	ret = ctsvc_contact_add_image_file(contact_id, image_id, image->path, image_path, sizeof(image_path));
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_contact_add_image_file() Fail(%d)", ret);
		return ret;
	}
	free(image->path);
	image->path = strdup(image_path);

	snprintf(query, sizeof(query),
			"INSERT INTO "CTS_TABLE_DATA"(id, contact_id, is_my_profile, datatype, is_default, is_primary_default, data1, data2, data3) "
			"VALUES(%d, %d, %d, %d, %d, %d, %d, ?, ?)",
			image_id, contact_id, is_my_profile, CONTACTS_DATA_TYPE_IMAGE, image->is_default, image->is_default, image->type);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	__ctsvc_image_bind_stmt(stmt, image, 1);

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		return ret;
	}

	/* image->id = ctsvc_db_get_last_insert_id(); */
	if (id)
		*id = image_id;
	ctsvc_stmt_finalize(stmt);

	if (ctsvc_record_check_property_flag((ctsvc_record_s*)record, _contacts_image.is_default, CTSVC_PROPERTY_FLAG_DIRTY)) {
		if (image->is_default)
			__ctsvc_db_image_reset_default(image_id, contact_id);
	}

	if (false == is_my_profile)
		ctsvc_set_image_noti();

	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_image_update(contacts_record_h record, int contact_id, bool is_my_profile)
{
	int id;
	int ret = CONTACTS_ERROR_NONE;
	char *set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	ctsvc_image_s *image = (ctsvc_image_s*)record;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETVM_IF(0 == image->id, CONTACTS_ERROR_INVALID_PARAMETER, "image of contact has no ID.");
	RETVM_IF(CTSVC_PROPERTY_FLAG_DIRTY != (image->base.property_flag & CTSVC_PROPERTY_FLAG_DIRTY), CONTACTS_ERROR_NONE, "No update");

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_DATA" WHERE id = %d", image->id);
	ret = ctsvc_query_get_first_int_result(query, &id);
	RETV_IF(ret != CONTACTS_ERROR_NONE, ret);

	if (ctsvc_record_check_property_flag((ctsvc_record_s*)record, _contacts_image.path, CTSVC_PROPERTY_FLAG_DIRTY)) {
		char image_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		if (image->path) {
			ret = ctsvc_have_file_read_permission(image->path);
			if (ret != CONTACTS_ERROR_NONE) {
				ERR("ctsvc_have_file_read_permission Fail(%d)", ret);
				return ret;
			}
		}

		ret = ctsvc_contact_update_image_file(contact_id, image->id, image->path, image_path, sizeof(image_path));
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_contact_update_image_file() Fail(%d)", ret);

		if (*image_path) {
			free(image->path);
			image->path = strdup(image_path);
		}
	}

	if (ctsvc_record_check_property_flag((ctsvc_record_s*)record, _contacts_image.is_default, CTSVC_PROPERTY_FLAG_DIRTY)) {
		if (image->is_default)
			__ctsvc_db_image_reset_default(image->id, contact_id);
	}

	do {
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_create_set_query(record, &set, &bind_text))) break;
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_update_record_with_set_query(set, bind_text, CTS_TABLE_DATA, image->id))) break;
		if (false == is_my_profile)
			ctsvc_set_image_noti();
	} while (0);

	CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s*)record);
	free(set);

	if (bind_text) {
		for (cursor = bind_text; cursor; cursor = cursor->next) {
			free(cursor->data);
			cursor->data = NULL;
		}
		g_slist_free(bind_text);
	}
	return ret;
}

int ctsvc_db_image_delete(int id, bool is_my_profile)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_DATA" WHERE id = %d AND datatype = %d",
			id, CONTACTS_DATA_TYPE_IMAGE);

	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	if (false == is_my_profile)
		ctsvc_set_image_noti();

	return CONTACTS_ERROR_NONE;
}

/*
 * Whenever deleting image recode in data table, this funcion will be called
 * in order to delete the image file
 */
void ctsvc_db_image_delete_callback(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	int ret;
	const unsigned char *image_path;

	if (1 < argc) {
		sqlite3_result_null(context);
		return;
	}
	image_path = sqlite3_value_text(argv[0]);

	ret = ctsvc_contact_delete_image_file_with_path(image_path);
	WARN_IF(CONTACTS_ERROR_NONE != ret && CONTACTS_ERROR_NO_DATA != ret,
			"ctsvc_contact_delete_image_file_with_path() Fail(%d)", ret);

	return;
}

int ctsvc_db_image_set_primary_default(int contact_id, const char *image_thumbnail_path,
		bool is_primary_default)
{
	int ret;
	char *image_path = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETV_IF(contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == image_thumbnail_path, CONTACTS_ERROR_INVALID_PARAMETER);

	image_path = ctsvc_utils_get_image_path(image_thumbnail_path);

	if (NULL == image_path) {
		ERR("ctsvc_utils_get_image_path() Fail");
		return CONTACTS_ERROR_INTERNAL;
	}

	snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_DATA" SET is_primary_default = %d WHERE contact_id = %d AND datatype = %d AND data3 = %s",
			is_primary_default, contact_id, CONTACTS_DATA_TYPE_IMAGE, image_path);

	free(image_path);
	ret = ctsvc_query_exec(query);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_query_exec() Fail(%d)", ret);
	return ret;
}


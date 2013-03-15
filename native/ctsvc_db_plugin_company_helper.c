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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_utils.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_query.h"
#include "ctsvc_db_plugin_company_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"

#define CTS_LOGO_IMAGE_LOCATION "/opt/usr/data/contacts-svc/img/logo"

static int __ctsvc_company_add_logo_file(int parent_id, int company_id, char *src_img, char *dest_name, int dest_size)
{
	int ret;
	char *ext;
	char dest[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	RETVM_IF(NULL == src_img, CONTACTS_ERROR_INVALID_PARAMETER, "img_path is NULL");
	RETVM_IF(NULL == dest_name, CONTACTS_ERROR_INVALID_PARAMETER, "img_path is NULL");
	dest_name[0] = '\0';

	ext = strrchr(src_img, '.');
	if (NULL == ext || strchr(ext, '/'))
		ext = "";

	snprintf(dest, sizeof(dest), "%s/%d_%d%s",
			CTS_LOGO_IMAGE_LOCATION, parent_id, company_id, ext);

	ret = ctsvc_copy_image(src_img, dest);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "cts_copy_file() Failed(%d)", ret);

	snprintf(dest_name, dest_size, "%d_%d%s", parent_id, company_id, ext);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_company_delete_logo_file(int company_id)
{
	int ret;
	cts_stmt stmt;
	char *tmp_path;
	char query[CTS_SQL_MIN_LEN] = {0};
	snprintf(query, sizeof(query),
			"SELECT data8 FROM %s WHERE id = %d AND datatype = %d",
			CTS_TABLE_DATA, company_id, CTSVC_DATA_COMPANY);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("cts_query_prepare() Failed");
		cts_stmt_finalize(stmt);
		return CONTACTS_ERROR_DB;
	}

	ret = cts_stmt_step(stmt);
	if (1 /*CTS_TRUE*/  != ret) {
		CTS_DBG("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CONTACTS_ERROR_NO_DATA;
	}

	tmp_path = ctsvc_stmt_get_text(stmt, 0);
	if (tmp_path) {
		char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_LOGO_IMAGE_LOCATION, tmp_path);
		ret = unlink(full_path);
		WARN_IF(ret < 0, "unlink(%s) Failed(%d)", full_path, errno);
	}
	cts_stmt_finalize(stmt);

	return CONTACTS_ERROR_NONE;
}


static int __ctsvc_company_update_logo_file(int parent_id, int company_id, char *src_img, char *dest_name, int dest_size)
{
	int ret;
	dest_name[0] = '\0';
	ret = __ctsvc_company_delete_logo_file(company_id);
	RETVM_IF(CONTACTS_ERROR_NONE != ret && CONTACTS_ERROR_NO_DATA != ret,
			ret, "ccts_company_delete_logo_file() Failed(%d)", ret);

	if (src_img) {
		ret = __ctsvc_company_add_logo_file(parent_id, company_id, src_img, dest_name, dest_size);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_company_add_logo_file() Failed(%d)", ret);
	}
	return ret;
}

static int __ctsvc_company_bind_stmt(cts_stmt stmt, ctsvc_company_s *company, int start_cnt)
{
	cts_stmt_bind_int(stmt, start_cnt, company->is_default);
	cts_stmt_bind_int(stmt, start_cnt+1, company->type);
	if (company->label)
		sqlite3_bind_text(stmt, start_cnt+2, company->label,
			strlen(company->label), SQLITE_STATIC);
	if (company->name)
		sqlite3_bind_text(stmt, start_cnt+3, company->name,
			strlen(company->name), SQLITE_STATIC);
	if (company->department)
		sqlite3_bind_text(stmt, start_cnt+4, company->department,
				strlen(company->department), SQLITE_STATIC);
	if (company->job_title)
		sqlite3_bind_text(stmt, start_cnt+5, company->job_title,
				strlen(company->job_title), SQLITE_STATIC);
	if (company->role)
		sqlite3_bind_text(stmt, start_cnt+6, company->role,
				strlen(company->role), SQLITE_STATIC);
	if (company->assistant_name)
		sqlite3_bind_text(stmt, start_cnt+7, company->assistant_name,
			strlen(company->assistant_name), SQLITE_STATIC);

	// skip logo here

	if (company->location)
		sqlite3_bind_text(stmt, start_cnt+9, company->location,
			strlen(company->location), SQLITE_STATIC);
	if (company->description)
		sqlite3_bind_text(stmt, start_cnt+10, company->description,
			strlen(company->description), SQLITE_STATIC);
	if (company->phonetic_name)
		sqlite3_bind_text(stmt, start_cnt+11, company->phonetic_name,
			strlen(company->phonetic_name), SQLITE_STATIC);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_company_insert(contacts_record_h record, int contact_id, bool is_my_profile, int *id)
{
	int ret;
	int company_id = 0;
	cts_stmt stmt = NULL;
	ctsvc_company_s *company = (ctsvc_company_s*)record;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETVM_IF(contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : contact_id(%d) is mandatory field to insert company record ", company->contact_id);
	RETVM_IF(0 < company->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : id(%d), This record is already inserted", company->id);

	if (company->name || company->department || company->job_title || company->role
			|| company->assistant_name || company->logo || company->location || company->description
			|| company->phonetic_name) {

		ret = cts_db_get_next_id(CTS_TABLE_DATA);
		if (ret < CONTACTS_ERROR_NONE) {
			CTS_ERR("cts_db_get_next_id() Failed(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
		company_id = ret;

		snprintf(query, sizeof(query),
			"INSERT INTO "CTS_TABLE_DATA"(id, contact_id, is_my_profile, datatype, is_default, data1, data2, data3, data4, "
					"data5, data6, data7, data8, data9, data10, data11, data12) "
					"VALUES(%d, %d, %d, %d, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
					company_id, contact_id, is_my_profile, CTSVC_DATA_COMPANY);

		stmt = cts_query_prepare(query);
		RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

		__ctsvc_company_bind_stmt(stmt, company, 1);
		if (company->logo) {
			char image[CTS_SQL_MAX_LEN] = {0};
			ret = __ctsvc_company_add_logo_file(contact_id, company->id, company->logo, image, sizeof(image));
			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("__ctsvc_company_add_logo_file() Failed(%d)", ret);
				cts_stmt_finalize(stmt);
				return ret;
			}
			cts_stmt_bind_text(stmt, 9, image);
		}

		ret = cts_stmt_step(stmt);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
		cts_stmt_finalize(stmt);

		company->id = company_id;
		if (id)
			*id = company_id;

		if (!is_my_profile)
			ctsvc_set_company_noti();
	}
	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_company_get_value_from_stmt(cts_stmt stmt, contacts_record_h *record, int start_count)
{
	int ret;
	char *temp;
	ctsvc_company_s *company;

	ret = contacts_record_create(_contacts_company._uri, (contacts_record_h *)&company);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create is failed(%d)", ret);

	company->id = ctsvc_stmt_get_int(stmt, start_count++);
	company->contact_id = ctsvc_stmt_get_int(stmt, start_count++);
	company->is_default = ctsvc_stmt_get_int(stmt, start_count++);
	company->type = ctsvc_stmt_get_int(stmt, start_count++);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	company->label = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	company->name = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	company->department = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	company->job_title = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	company->role = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	company->assistant_name = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	if (temp) {
		char tmp_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		snprintf(tmp_path, sizeof(tmp_path), "%s/%s", CTS_LOGO_IMAGE_LOCATION, temp);
		company->logo = strdup(tmp_path);
	}
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	company->location = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	company->description = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, start_count++);
	company->phonetic_name = SAFE_STRDUP(temp);

	if (company->name || company->department || company->job_title || company->role
			|| company->assistant_name || company->logo || company->location || company->description
			|| company->phonetic_name)
		*record = (contacts_record_h)company;
	else {
		contacts_record_destroy((contacts_record_h)company, true);
		*record = NULL;
	}
	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_company_update(contacts_record_h record, int contact_id, bool is_my_profile)
{
	int id;
	int ret = CONTACTS_ERROR_NONE;
	char* set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_company_s *company =  (ctsvc_company_s*)record;

	RETVM_IF(!company->id, CONTACTS_ERROR_INVALID_PARAMETER, "company of contact has no ID.");
	RETVM_IF(CTSVC_PROPERTY_FLAG_DIRTY != (company->base.property_flag & CTSVC_PROPERTY_FLAG_DIRTY), CONTACTS_ERROR_NONE, "No update");

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_DATA" WHERE id = %d", company->id);
	ret = ctsvc_query_get_first_int_result(query, &id);
	RETV_IF(ret != CONTACTS_ERROR_NONE, ret);


	if (company->logo_changed) {
		char dest[CTS_SQL_MAX_LEN] = {0};
		__ctsvc_company_update_logo_file(contact_id, company->id, company->logo, dest, sizeof(dest));
		if (*dest) {
			free(company->logo);
			company->logo = strdup(dest);
		}
		company->logo_changed = false;
	}

	do {
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_create_set_query(record, &set, &bind_text))) break;
		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_update_record_with_set_query(set, bind_text, CTS_TABLE_DATA, company->id))) break;
		if (!is_my_profile)
			ctsvc_set_company_noti();
	} while (0);

	CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s *)record);
	CONTACTS_FREE(set);
	if (bind_text) {
		for (cursor=bind_text;cursor;cursor=cursor->next)
			CONTACTS_FREE(cursor->data);
		g_slist_free(bind_text);
	}

	return ret;
}

int ctsvc_db_company_delete(int id, bool is_my_profile)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_DATA" WHERE id = %d AND datatype = %d",
			id, CTSVC_DATA_COMPANY);

	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Failed(%d)", ret);
	if (!is_my_profile)
		ctsvc_set_company_noti();

	return CONTACTS_ERROR_NONE;
}

void ctsvc_db_data_company_delete_callback(sqlite3_context *context, int argc, sqlite3_value ** argv)
{
	int ret;
	const unsigned char* logo_path;

	if (argc > 1) {
		sqlite3_result_null(context);
		return;
	}
	logo_path = sqlite3_value_text(argv[0]);

	if (logo_path) {
		char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_LOGO_IMAGE_LOCATION, logo_path);
		ret = unlink(full_path);
		WARN_IF(ret < 0, "unlink(%s) Failed(%d)", full_path, errno);
	}

	return;
}



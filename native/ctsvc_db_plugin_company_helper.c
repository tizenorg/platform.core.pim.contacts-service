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

#include <ctype.h>
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
#include "ctsvc_db_access_control.h"
#include "ctsvc_notify.h"

static int __ctsvc_company_bind_stmt(cts_stmt stmt, ctsvc_company_s *company, int start_cnt)
{
	ctsvc_stmt_bind_int(stmt, start_cnt, company->is_default);
	ctsvc_stmt_bind_int(stmt, start_cnt+1, company->type);
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

		ret = ctsvc_db_get_next_id(CTS_TABLE_DATA);
		if (ret < CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_db_get_next_id() Fail(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
		company_id = ret;

		snprintf(query, sizeof(query),
			"INSERT INTO "CTS_TABLE_DATA"(id, contact_id, is_my_profile, datatype, is_default, data1, data2, data3, data4, "
					"data5, data6, data7, data8, data9, data10, data11, data12) "
					"VALUES(%d, %d, %d, %d, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
					company_id, contact_id, is_my_profile, CTSVC_DATA_COMPANY);

		ret = ctsvc_query_prepare(query, &stmt);
		RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

		__ctsvc_company_bind_stmt(stmt, company, 1);
		if (company->logo) {
			char image[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
			ret = ctsvc_have_file_read_permission(company->logo);
			if (ret != CONTACTS_ERROR_NONE) {
				CTS_ERR("ctsvc_have_file_read_permission Fail(%d)", ret);
				ctsvc_stmt_finalize(stmt);
				return ret;
			}
			ctsvc_utils_make_image_file_name(contact_id, company_id, company->logo, image, sizeof(image));
			ret = ctsvc_utils_copy_image(CTS_LOGO_IMAGE_LOCATION, company->logo, image);
			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("ctsvc_utils_copy_image() Fail(%d)", ret);
				ctsvc_stmt_finalize(stmt);
				return ret;
			}
			ctsvc_stmt_bind_text(stmt, 9, image);
		}

		ret = ctsvc_stmt_step(stmt);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			return ret;
		}
		ctsvc_stmt_finalize(stmt);

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
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create Fail(%d)", ret);

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
	int ret = CONTACTS_ERROR_NONE;
	char* set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_company_s *company = (ctsvc_company_s*)record;
	cts_stmt stmt = NULL;

	RETVM_IF(!company->id, CONTACTS_ERROR_INVALID_PARAMETER, "company of contact has no ID.");
	RETVM_IF(CTSVC_PROPERTY_FLAG_DIRTY != (company->base.property_flag & CTSVC_PROPERTY_FLAG_DIRTY), CONTACTS_ERROR_NONE, "No update");

	snprintf(query, sizeof(query),
			"SELECT id, data8 FROM "CTS_TABLE_DATA" WHERE id = %d", company->id);
	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (ret != 1) {
		ctsvc_stmt_finalize(stmt);
		if (ret == CONTACTS_ERROR_NONE)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}

	if (ctsvc_record_check_property_flag((ctsvc_record_s *)company, _contacts_company.logo, CTSVC_PROPERTY_FLAG_DIRTY)) {
		char *logo = ctsvc_stmt_get_text(stmt, 1);
		bool same = false;
		bool check_permission = false;

		// delete current logo image
		if (logo) {
			char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
			snprintf(full_path, sizeof(full_path), "%s/%s", CTS_LOGO_IMAGE_LOCATION, logo);
			if (company->logo && STRING_EQUAL == strcmp(company->logo, full_path)) {
				int index = _contacts_company.logo & 0x000000FF;
				((ctsvc_record_s *)record)->properties_flags[index] = 0;
				same = true;
			}
			else {
				if (company->logo) {
					ret = ctsvc_have_file_read_permission(company->logo);
					if (ret != CONTACTS_ERROR_NONE) {
						CTS_ERR("ctsvc_have_file_read_permission Fail(%d)", ret);
						ctsvc_stmt_finalize(stmt);
						return ret;
					}
					check_permission = true;
				}
				ret = unlink(full_path);
				if (ret < 0) {
					CTS_WARN("unlink Fail(%d)", errno);
				}
			}
		}

		// add new logo file
		if (!same && company->logo) {
			char dest[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
			if (false == check_permission) {
				ret = ctsvc_have_file_read_permission(company->logo);
				if (ret != CONTACTS_ERROR_NONE) {
					CTS_ERR("ctsvc_have_file_read_permission Fail(%d)", ret);
					ctsvc_stmt_finalize(stmt);
					return ret;
				}
			}
			ctsvc_utils_make_image_file_name(contact_id, company->id, company->logo, dest, sizeof(dest));
			ret = ctsvc_utils_copy_image(CTS_LOGO_IMAGE_LOCATION, company->logo, dest);
			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("cts_copy_file() Fail(%d)", ret);
				ctsvc_stmt_finalize(stmt);
				return ret;
			}
			free(company->logo);
			company->logo = strdup(dest);
		}
	}
	ctsvc_stmt_finalize(stmt);

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
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);
	if (!is_my_profile)
		ctsvc_set_company_noti();

	return CONTACTS_ERROR_NONE;
}

// Whenever deleting company record in data table, this function will be called
// in order to delete company logo image file
void ctsvc_db_company_delete_callback(sqlite3_context *context, int argc, sqlite3_value ** argv)
{
	int ret;
	const unsigned char* logo_path;

	if (1 < argc) {
		sqlite3_result_null(context);
		return;
	}
	logo_path = sqlite3_value_text(argv[0]);

	if (logo_path) {
		char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_LOGO_IMAGE_LOCATION, logo_path);
		ret = unlink(full_path);
		if (ret < 0) {
			CTS_WARN("unlink Fail(%d)", errno);
		}
	}

	return;
}



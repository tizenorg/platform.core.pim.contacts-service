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
#include "ctsvc_utils.h"
#include "ctsvc_list.h"
#include "ctsvc_db_query.h"
#include "ctsvc_db_init.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"

#define CTS_GROUP_IMAGE_LOCATION "/opt/usr/data/contacts-svc/img/group"

static int __ctsvc_db_group_insert_record( contacts_record_h record, int *id );
static int __ctsvc_db_group_get_record( int id, contacts_record_h* out_record );
static int __ctsvc_db_group_update_record( contacts_record_h record );
static int __ctsvc_db_group_delete_record( int id );
static int __ctsvc_db_group_get_all_records( int offset, int limit, contacts_list_h* out_list );
static int __ctsvc_db_group_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list );
//static int __ctsvc_db_group_insert_records(const contacts_list_h in_list, int **ids);
//static int __ctsvc_db_group_update_records(const contacts_list_h in_list);
//static int __ctsvc_db_group_delete_records( int ids[], int count);

ctsvc_db_plugin_info_s ctsvc_db_plugin_group = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_group_insert_record,
	.get_record = __ctsvc_db_group_get_record,
	.update_record = __ctsvc_db_group_update_record,
	.delete_record = __ctsvc_db_group_delete_record,
	.get_all_records = __ctsvc_db_group_get_all_records,
	.get_records_with_query = __ctsvc_db_group_get_records_with_query,
	.insert_records = NULL,//__ctsvc_db_group_insert_records,
	.update_records = NULL,//__ctsvc_db_group_update_records,
	.delete_records = NULL,//__ctsvc_db_group_delete_records
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};

static int __ctsvc_db_group_insert_record( contacts_record_h record, int *id )
{
	int ret;
	int ver;
	cts_stmt stmt;
	ctsvc_group_s *group = (ctsvc_group_s *)record;
	char query[CTS_SQL_MAX_LEN] = {0};
	char image[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(CTSVC_RECORD_GROUP != group->base.r_type, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : record is invalid type(%d)", group->base.r_type);
	RETVM_IF(NULL == group->name, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : The name of record is empty.");

	ret = ctsvc_begin_trans();
	if( ret < CONTACTS_ERROR_NONE ) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	group->id = cts_db_get_next_id(CTS_TABLE_GROUPS);
	if (id)
		*id = group->id;

	snprintf(query, sizeof(query),
			"INSERT INTO "CTS_TABLE_GROUPS"(group_id, addressbook_id, group_name, created_ver, changed_ver, ringtone_path, "
						"vibration, image_thumbnail_path, system_id, is_read_only) "
			"VALUES(%d, %d, ?, ?, ?, ?, ?, ?, ?, %d)",
			group->id, group->addressbook_id, group->is_read_only);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("DB error : cts_query_prepare() Failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	cts_stmt_bind_text(stmt, 1, group->name);

	ver = ctsvc_get_next_ver();

	cts_stmt_bind_int(stmt, 2, ver);
	cts_stmt_bind_int(stmt, 3, ver);

	if (group->ringtone_path)
		cts_stmt_bind_text(stmt, 4, group->ringtone_path);
	if (group->vibration)
		cts_stmt_bind_text(stmt, 5, group->vibration);

	if(group->image_thumbnail_path) {
		ret = ctsvc_change_image(CTS_GROUP_IMAGE_LOCATION, group->id, group->image_thumbnail_path,
			image, sizeof(image));
		if(CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : ctsvc_change_image() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			ctsvc_end_trans(false);
			return ret;
		}
		free(group->image_thumbnail_path);
		group->image_thumbnail_path = strdup(image);
		cts_stmt_bind_text(stmt, 6, group->image_thumbnail_path);
	}

	if (group->system_id)
		cts_stmt_bind_text(stmt, 7, group->system_id);

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_group_noti();

	cts_stmt_finalize(stmt);

	ret = ctsvc_end_trans(true);
	if(ret < CONTACTS_ERROR_NONE ) {
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_group_update_record( contacts_record_h record )
{
	int ret;
	ctsvc_group_s *group = (ctsvc_group_s *)record;
	char image[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
	char query[CTS_SQL_MIN_LEN] = {0};
	cts_stmt stmt;
	int len;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(CTSVC_RECORD_GROUP != group->base.r_type, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : group is invalid type(%d)", group->base.r_type);
	RETVM_IF(NULL == group->name, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : The name of group is empty.");

	len = snprintf(query, sizeof(query),
		"UPDATE "CTS_TABLE_GROUPS" SET group_name=?, changed_ver=?, ringtone_path=?, vibration=? ");

	if (group->image_thumbnail_changed)
		len += snprintf(query+len, sizeof(query)-len, ", image_thumbnail_path = ? ");

	snprintf(query+len, sizeof(query)-len, "WHERE group_id=%d", group->id);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("DB error : cts_query_prepare() Failed");
		return CONTACTS_ERROR_DB;
	}

	ret = ctsvc_begin_trans();
	if( ret < CONTACTS_ERROR_NONE ) {
		CTS_ERR("DB error : ctsvc_begin_trans() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}

	cts_stmt_bind_text(stmt, 1, group->name);
	cts_stmt_bind_int(stmt, 2, ctsvc_get_next_ver());

	if (group->ringtone_path)
		cts_stmt_bind_text(stmt, 3, group->ringtone_path);

	if (group->vibration)
		cts_stmt_bind_text(stmt, 4, group->vibration);

	if (group->image_thumbnail_changed) {
		ret = ctsvc_change_image(CTS_GROUP_IMAGE_LOCATION, group->id, group->image_thumbnail_path,
			image, sizeof(image));
		if(CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : ctsvc_change_image() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			ctsvc_end_trans(false);
			return ret;
		}
		if (*image) {
			free(group->image_thumbnail_path);
			group->image_thumbnail_path = strdup(image);
			cts_stmt_bind_text(stmt, 5, image);
		}
	}

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	ctsvc_set_group_noti();

	ret = ctsvc_end_trans(true);
	if(ret < CONTACTS_ERROR_NONE ) {
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_group_delete_record( int index )
{
	int ret;
	int addressbook_id;
	char  query[CTS_SQL_MAX_LEN] = {0};

	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE > ret, ret, "DB error : ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT addressbook_id FROM %s WHERE group_id = %d",
			CTS_TABLE_GROUPS, index);

	ret = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if ( ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : The index(%d) is Invalid(%d)", index, addressbook_id);
		ctsvc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE group_id=%d AND is_read_only=0",
			CTS_TABLE_GROUPS, index);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = cts_db_change();
	if (ret <= 0) {
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_NO_DATA;
	}

	snprintf(query, sizeof(query), "INSERT INTO %s VALUES(%d, %d, %d)",
			CTS_TABLE_GROUP_DELETEDS, index, addressbook_id, ctsvc_get_next_ver());
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_change_image(CTS_GROUP_IMAGE_LOCATION, index, NULL, NULL, 0);
	if (ret < 0) {
		CTS_ERR("DB error : ctsvc_change_image() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_group_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_group_value_set(cts_stmt stmt, contacts_record_h *record)
{
	int i;
	int ret;
	char *temp;
	ctsvc_group_s *group;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	ret = contacts_record_create(_contacts_group._uri, record);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create is failed(%d)", ret);
	group = (ctsvc_group_s*)*record;

	i = 0;
	group->id = ctsvc_stmt_get_int(stmt, i++);
	group->addressbook_id = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	group->name = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	group->system_id = SAFE_STRDUP(temp);
	group->is_read_only = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	group->ringtone_path = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	group->vibration = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	if (temp) {
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_GROUP_IMAGE_LOCATION, temp);
		group->image_thumbnail_path = strdup(full_path);
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_group_get_record( int id, contacts_record_h *out_record )
{
	int ret;
	int len;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_record_h record;

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	len = snprintf(query, sizeof(query),
			"SELECT group_id, addressbook_id, group_name, system_id, is_read_only, "
				"ringtone_path, vibration, image_thumbnail_path "
				"FROM "CTS_TABLE_GROUPS" WHERE group_id = %d", id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CONTACTS_ERROR_NO_DATA;
	}

	ret = __ctsvc_db_group_value_set(stmt, &record);

	cts_stmt_finalize(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_group_value_set(ALL) Failed(%d)", ret);
		return ret;
	}
	*out_record = record;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_group_get_all_records( int offset, int limit, contacts_list_h* out_list )
{
	int ret;
	int len;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;

	len = snprintf(query, sizeof(query),
			"SELECT group_id, addressbook_id, group_name, system_id, is_read_only, "
				"ringtone_path, vibration, image_thumbnail_path "
				"FROM "CTS_TABLE_GROUPS" ORDER BY addressbook_id, group_name COLLATE NOCASE");

	if (0 < limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB , "DB error : cts_query_prepare() Failed");

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}
		__ctsvc_db_group_value_set(stmt, &record);

		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_group_get_records_with_query( contacts_query_h query,
	int offset, int limit, contacts_list_h* out_list )
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_group_s *group;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s *)query;

	ret = ctsvc_db_make_get_records_query_stmt(s_query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_db_make_get_records_query_stmt fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		contacts_record_create(_contacts_group._uri, &record);
		group = (ctsvc_group_s*)record;
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
			case CTSVC_PROPERTY_GROUP_ID:
				group->id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_GROUP_ADDRESSBOOK_ID:
				group->addressbook_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_GROUP_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				group->name = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_GROUP_RINGTONE:
				temp = ctsvc_stmt_get_text(stmt, i);
				group->ringtone_path = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_GROUP_IMAGE:
				temp = ctsvc_stmt_get_text(stmt, i);
				if (temp) {
					snprintf(full_path, sizeof(full_path), "%s/%s", CTS_GROUP_IMAGE_LOCATION, temp);
					group->image_thumbnail_path = strdup(full_path);
				}
				break;
			case CTSVC_PROPERTY_GROUP_VIBRATION:
				temp = ctsvc_stmt_get_text(stmt, i);
				group->vibration = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_GROUP_SYSTEM_ID:
				temp = ctsvc_stmt_get_text(stmt, i);
				group->system_id = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_GROUP_IS_READ_ONLY:
				group->is_read_only = ctsvc_stmt_get_int(stmt, i);
				break;
			default:
				break;
			}
		}
		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;

	return CONTACTS_ERROR_NONE;
}
//static int __ctsvc_db_group_insert_records(const contacts_list_h in_list, int **ids) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_group_update_records(const contacts_list_h in_list) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_group_delete_records( int ids[], int count) { return CONTACTS_ERROR_NONE; }



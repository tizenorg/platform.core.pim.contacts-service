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
#include <sys/types.h>
#include <fcntl.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_schema.h"
#include "ctsvc_db_init.h"
#include "ctsvc_utils.h"
#include "ctsvc_list.h"
#include "ctsvc_db_plugin_person_helper.h"
#include "ctsvc_db_plugin_contact_helper.h"
#include "ctsvc_record.h"
#include "ctsvc_db_query.h"
#include "ctsvc_notification.h"

static int __ctsvc_db_simple_contact_insert_record( contacts_record_h record, int *id );
static int __ctsvc_db_simple_contact_get_record( int id, contacts_record_h* out_record );
static int __ctsvc_db_simple_contact_update_record( contacts_record_h record );
static int __ctsvc_db_simple_contact_delete_record( int id );
static int __ctsvc_db_simple_contact_get_all_records( int offset, int limit, contacts_list_h* out_list );
static int __ctsvc_db_simple_contact_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list );

ctsvc_db_plugin_info_s ctsvc_db_plugin_simple_contact = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_simple_contact_insert_record,
	.get_record = __ctsvc_db_simple_contact_get_record,
	.update_record = __ctsvc_db_simple_contact_update_record,
	.delete_record = __ctsvc_db_simple_contact_delete_record,
	.get_all_records = __ctsvc_db_simple_contact_get_all_records,
	.get_records_with_query = __ctsvc_db_simple_contact_get_records_with_query,
	.insert_records = NULL,
	.update_records = NULL,
	.delete_records = NULL,
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};

static int __ctsvc_db_simple_contact_value_set(cts_stmt stmt, contacts_record_h *record)
{
	int i;
	int ret;
	char *temp;
	ctsvc_simple_contact_s *contact;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	ret = contacts_record_create(_contacts_simple_contact._uri, record);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create is failed(%d)", ret);
	contact = (ctsvc_simple_contact_s*)*record;

	i = 0;
	contact->contact_id = ctsvc_stmt_get_int(stmt, i++);
	contact->addressbook_id = ctsvc_stmt_get_int(stmt, i++);
	contact->person_id = ctsvc_stmt_get_int(stmt, i++);
	contact->changed_time = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	contact->display_name = SAFE_STRDUP(temp);
	contact->display_source_type = ctsvc_stmt_get_int(stmt, i++);

	temp = ctsvc_stmt_get_text(stmt, i++);
	if (temp) {
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_IMG_FULL_LOCATION, temp);
		contact->image_thumbnail_path = strdup(full_path);
	}

	temp = ctsvc_stmt_get_text(stmt, i++);
	contact->ringtone_path = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	contact->vibration = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	contact->uid = SAFE_STRDUP(temp);
	contact->is_favorite = ctsvc_stmt_get_int(stmt, i++);
	contact->has_phonenumber = ctsvc_stmt_get_int(stmt, i++);
	contact->has_email = ctsvc_stmt_get_int(stmt, i++);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_simple_contact_get_record( int id, contacts_record_h* out_record )
{
	int ret;
	int len;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_record_h record;

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	len = snprintf(query, sizeof(query),
				"SELECT contact_id, addressbook_id, person_id, changed_time, %s, "
					"display_name_source, image_thumbnail_path, "
					"ringtone_path, vibration, uid, is_favorite, has_phonenumber, has_email "
					"FROM "CTS_TABLE_CONTACTS" WHERE contact_id = %d AND deleted = 0",
					ctsvc_get_display_column(), id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CONTACTS_ERROR_NO_DATA;
	}

	ret = __ctsvc_db_simple_contact_value_set(stmt, &record);

	cts_stmt_finalize(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_simple_contact_value_set(ALL) Failed(%d)", ret);
		return ret;
	}

	*out_record = record;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_simple_contact_get_default_image_id(int contact_id)
{
	int ret = 0;
	int image_id = 0;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT id FROM "CTS_TABLE_DATA" WHERE datatype=%d AND contact_id=%d AND is_default=1",
			CTSVC_DATA_IMAGE, contact_id);

	ret = ctsvc_query_get_first_int_result(query, &image_id);
	if (CONTACTS_ERROR_NONE != ret)
		return 0;
	return image_id;
}

static int __ctsvc_db_simple_contact_update_record( contacts_record_h record )
{
	int ret;
	int len;
	int i;
	int id;
	char query[CTS_SQL_MAX_LEN] = {0};
	char image[CTS_SQL_MAX_LEN] = {0};
	ctsvc_simple_contact_s *contact = (ctsvc_simple_contact_s*)record;
	cts_stmt stmt;

	// These check should be done in client side
	RETVM_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER,
					"Invalid parameter : contact is NULL");
	RETVM_IF(contact->addressbook_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : addressbook_id(%d) is mandatory field to insert contact record ",
				contact->addressbook_id);
	RETVM_IF(contact->contact_id < 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : ide(%d), This record is already inserted", contact->contact_id);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT contact_id FROM "CTS_TABLE_CONTACTS" "
				"WHERE contact_id = %d AND deleted = 0", contact->contact_id);
	ret = ctsvc_query_get_first_int_result(query, &id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("Invalid Parameter : contact_id (%d) is not exist", contact->contact_id);
		ctsvc_end_trans(false);
		return ret;
	}

	len = snprintf(query, sizeof(query),
				"UPDATE "CTS_TABLE_CONTACTS" SET changed_ver=%d, changed_time=%d ",
						ctsvc_get_next_ver(), (int)time(NULL));

	if (contact->uid_changed)
		len += snprintf(query+len, sizeof(query)-len, ", uid=?");

	if (contact->ringtone_changed)
		len += snprintf(query+len, sizeof(query)-len, ", ringtone_path=?");

	if (contact->vibration_changed)
			len += snprintf(query+len, sizeof(query)-len, ", vibration=?");

	if (contact->image_thumbnail_changed)
		len += snprintf(query+len, sizeof(query)-len, ", image_thumbnail_path=?");

	len += snprintf(query+len, sizeof(query)-len, " WHERE contact_id=%d", contact->contact_id);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("cts_query_prepare() Failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	i = 1;
	if (contact->uid_changed) {
		if (contact->uid)
			cts_stmt_bind_text(stmt, i, contact->uid);
		i++;
	}

	if (contact->ringtone_changed) {
		if (contact->ringtone_path)
			cts_stmt_bind_text(stmt, i, contact->ringtone_path);
		i++;
	}

	if (contact->vibration_changed) {
		if (contact->vibration)
			cts_stmt_bind_text(stmt, i, contact->vibration);
		i++;
	}

	//////////////////////////////////////////////////////////////////////
	// This code will be removed
	if (contact->image_thumbnail_changed) {
		int img_id;
		image[0] = '\0';
		img_id = __ctsvc_db_simple_contact_get_default_image_id(contact->contact_id);

		if (0 == img_id) {
			img_id = cts_db_get_next_id(CTS_TABLE_DATA);
			ret = ctsvc_contact_add_image_file(contact->contact_id, img_id, contact->image_thumbnail_path,
					image, sizeof(image));
		}
		else  {
			ret = ctsvc_contact_update_image_file(contact->contact_id, img_id,
					contact->image_thumbnail_path, image, sizeof(image));
		}
		if (*image) {
			free(contact->image_thumbnail_path);
			contact->image_thumbnail_path = strdup(image);
			if (CONTACTS_ERROR_NONE == ret && contact->image_thumbnail_path)
				cts_stmt_bind_text(stmt, i, contact->image_thumbnail_path);
		}
		i++;
	}
	//////////////////////////////////////////////////////////////////////

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	ctsvc_set_contact_noti();
	//ctsvc_update_person(contact);
	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_end_trans() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_simple_contact_delete_record( int id )
{
	return ctsvc_db_contact_delete(id);
}

static int __ctsvc_db_simple_contact_get_all_records( int offset, int limit,
	contacts_list_h* out_list )
{
	int ret;
	int len;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;

	len = snprintf(query, sizeof(query),
			"SELECT contact_id, addressbook_id, person_id, changed_time, "
				"%s, display_name_source, image_thumbnail_path, "
				"ringtone_path, vibration, uid, is_favorite, has_phonenumber, has_email "
				"FROM "CTS_TABLE_CONTACTS" WHERE deleted = 0", ctsvc_get_display_column());

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
		__ctsvc_db_simple_contact_value_set(stmt, &record);

		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_simple_contact_get_records_with_query( contacts_query_h query,
	int offset, int limit, contacts_list_h* out_list )
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_simple_contact_s *contact;
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

		contacts_record_create(_contacts_simple_contact._uri, &record);
		contact = (ctsvc_simple_contact_s*)record;
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
			case CTSVC_PROPERTY_CONTACT_ID:
				contact->contact_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_CONTACT_DISPLAY_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				contact->display_name = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID:
				contact->display_source_type = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID:
				contact->addressbook_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_CONTACT_RINGTONE:
				temp = ctsvc_stmt_get_text(stmt, i);
				contact->ringtone_path = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL:
				temp = ctsvc_stmt_get_text(stmt, i);
				if (temp && *temp) {
					snprintf(full_path, sizeof(full_path), "%s/%s", CTS_IMG_FULL_LOCATION, temp);
					contact->image_thumbnail_path = strdup(full_path);
				}
				break;
			case CTSVC_PROPERTY_CONTACT_IS_FAVORITE:
				contact->is_favorite = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_CONTACT_HAS_PHONENUMBER:
				contact->has_phonenumber = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_CONTACT_HAS_EMAIL:
				contact->has_email = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_CONTACT_PERSON_ID:
				contact->person_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_CONTACT_UID:
				temp = ctsvc_stmt_get_text(stmt, i);
				contact->uid = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_CONTACT_VIBRATION:
				temp = ctsvc_stmt_get_text(stmt, i);
				contact->vibration = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_CONTACT_CHANGED_TIME:
				contact->changed_time = ctsvc_stmt_get_int(stmt, i);
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

static int __ctsvc_db_simple_contact_insert_record( contacts_record_h record, int *id)
{
	int version;
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};
	char image[CTS_SQL_MAX_LEN] = {0};
	ctsvc_simple_contact_s *contact = (ctsvc_simple_contact_s*)record;
	cts_stmt stmt;

	// These check should be done in client side
	RETVM_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER,
					"Invalid parameter : contact is NULL");
	RETVM_IF(contact->addressbook_id < 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : addressbook_id(%d) is mandatory field to insert contact record ", contact->addressbook_id);
//	RETVM_IF(0 < contact->contact_id, CONTACTS_ERROR_INVALID_PARAMETER,
//				"Invalid parameter : ide(%d), This record is already inserted", contact->contact_id);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	ret = cts_db_get_next_id(CTS_TABLE_CONTACTS);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("cts_db_get_next_id() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}
	contact->contact_id = ret;
	if (id)
		*id = ret;

	if (contact->image_thumbnail_path) {
		int image_id;
		image[0] = '\0';
		image_id = __ctsvc_db_simple_contact_get_default_image_id(contact->contact_id);
		ret = ctsvc_contact_add_image_file(contact->contact_id, image_id, contact->image_thumbnail_path,
				image, sizeof(image));
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_add_image_file(NORMAL) Failed(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
		free(contact->image_thumbnail_path);
		contact->image_thumbnail_path = strdup(image);
	}

	version = ctsvc_get_next_ver();

	ret = ctsvc_db_insert_person((contacts_record_h)contact);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_db_insert_person() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}
	contact->person_id = ret;

	snprintf(query, sizeof(query),
		"INSERT INTO "CTS_TABLE_CONTACTS"(contact_id, person_id, addressbook_id, is_favorite, "
			"created_ver, changed_ver, changed_time, "
			"uid, ringtone_path, vibration, image_thumbnail_path) "
			"VALUES(%d, %d, %d, %d, %d, %d, %d, ?, ?, ?, ?)",
			contact->contact_id, contact->person_id, contact->addressbook_id, contact->is_favorite,
			version, version, (int)time(NULL));

	stmt = cts_query_prepare(query);
	if(NULL == stmt){
		CTS_ERR("cts_query_prepare() Failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	if (contact->uid)
		cts_stmt_bind_text(stmt, 1, contact->uid);
	if (contact->ringtone_path)
		cts_stmt_bind_text(stmt, 2, contact->ringtone_path);
	if (contact->vibration)
		cts_stmt_bind_text(stmt, 3, contact->vibration);
	if (contact->image_thumbnail_path)
		cts_stmt_bind_text(stmt, 4, contact->image_thumbnail_path);

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	ctsvc_set_contact_noti();

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_svc_end_trans() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

//static int __ctsvc_db_simple_contact_insert_records(const contacts_list_h in_list, int **ids) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_simple_contact_update_records(const contacts_list_h in_list) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_simple_contact_delete_records(int ids[], int count) { return CONTACTS_ERROR_NONE; }

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
#include "ctsvc_db_access_control.h"
#include "ctsvc_notify.h"

static int __ctsvc_db_simple_contact_get_record( int id, contacts_record_h* out_record );
static int __ctsvc_db_simple_contact_get_all_records( int offset, int limit, contacts_list_h* out_list );
static int __ctsvc_db_simple_contact_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list );

ctsvc_db_plugin_info_s ctsvc_db_plugin_simple_contact = {
	.is_query_only = false,
	.insert_record = NULL,
	.get_record = __ctsvc_db_simple_contact_get_record,
	.update_record = NULL,
	.delete_record = NULL,
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
		snprintf(full_path, sizeof(full_path), "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, temp);
		contact->image_thumbnail_path = strdup(full_path);
	}

	temp = ctsvc_stmt_get_text(stmt, i++);
	contact->ringtone_path = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	contact->vibration = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	contact->message_alert = SAFE_STRDUP(temp);
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
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_record_h record;

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	snprintf(query, sizeof(query),
				"SELECT contact_id, addressbook_id, person_id, changed_time, %s, "
					"display_name_source, image_thumbnail_path, "
					"ringtone_path, vibration, message_alert, uid, is_favorite, has_phonenumber, has_email "
					"FROM "CTS_TABLE_CONTACTS" WHERE contact_id = %d AND deleted = 0",
					ctsvc_get_display_column(), id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Failed(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		CTS_ERR("ctsvc_stmt_step() Failed(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}

	ret = __ctsvc_db_simple_contact_value_set(stmt, &record);

	ctsvc_stmt_finalize(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_simple_contact_value_set(ALL) Failed(%d)", ret);
		return ret;
	}

	*out_record = record;

	return CONTACTS_ERROR_NONE;
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
				"ringtone_path, vibration, message_alert, uid, is_favorite, has_phonenumber, has_email "
				"FROM "CTS_TABLE_CONTACTS" WHERE deleted = 0", ctsvc_get_display_column());

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
		__ctsvc_db_simple_contact_value_set(stmt, &record);

		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
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
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Failed(%d)", ret);
			ctsvc_stmt_finalize(stmt);
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
					snprintf(full_path, sizeof(full_path), "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, temp);
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
			case CTSVC_PROPERTY_CONTACT_MESSAGE_ALERT:
				temp = ctsvc_stmt_get_text(stmt, i);
				contact->message_alert = SAFE_STRDUP(temp);
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
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;

	return CONTACTS_ERROR_NONE;
}

//static int __ctsvc_db_simple_contact_insert_records(const contacts_list_h in_list, int **ids) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_simple_contact_update_records(const contacts_list_h in_list) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_simple_contact_delete_records(int ids[], int count) { return CONTACTS_ERROR_NONE; }

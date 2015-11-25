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
#include <glib.h>

#include "contacts.h"

#include "ctsvc_internal.h"
#include "ctsvc_db_schema.h"
#include "ctsvc_db_sqlite.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_utils.h"
#include "ctsvc_view.h"
#include "ctsvc_notification.h"
#include "ctsvc_db_access_control.h"

extern ctsvc_db_plugin_info_s ctsvc_db_plugin_addressbook;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_contact;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_my_profile;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_simple_contact;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_group;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_person;
#ifdef ENABLE_LOG_FEATURE
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_phonelog;
#endif /* ENABLE_LOG_FEATURE */
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_speeddial;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_sdn;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_activity;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_activity_photo;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_address;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_company;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_email;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_event;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_grouprelation;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_relationship;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_image;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_messenger;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_name;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_nickname;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_note;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_number;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_url;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_extension;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_profile;
extern ctsvc_db_plugin_info_s ctsvc_db_plugin_sip;


#ifdef _CONTACTS_NATIVE
static int __ctsvc_db_view_ref_count = 0;
#endif

static GHashTable *__ctsvc_db_view_hash_table = NULL;

#ifdef _CONTACTS_IPC_SERVER
static bool __ctsvc_db_view_already_created = false;
#endif

typedef struct {
	char *view_uri;
	const char * const table_name;
	int read_permission;
	int write_permission;
	bool need_ab_access_control;
} db_table_info_s;

static const db_table_info_s __db_tables[] = {
	{CTSVC_VIEW_URI_ADDRESSBOOK,	CTS_TABLE_ADDRESSBOOKS, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_GROUP,			CTS_TABLE_GROUPS, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_PERSON,			CTSVC_DB_VIEW_PERSON, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_SIMPLE_CONTACT, CTSVC_DB_VIEW_CONTACT, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_CONTACT,		CTSVC_DB_VIEW_CONTACT, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_MY_PROFILE,	CTSVC_DB_VIEW_MY_PROFILE, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_ACTIVITY,		CTSVC_DB_VIEW_ACTIVITY, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_ACTIVITY_PHOTO, CTSVC_DB_VIEW_ACTIVITY_PHOTOS, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_PHONELOG,		CTS_TABLE_PHONELOGS, CTSVC_PERMISSION_PHONELOG_READ, CTSVC_PERMISSION_PHONELOG_WRITE, false},
	{CTSVC_VIEW_URI_SPEEDDIAL,		CTSVC_DB_VIEW_SPEEDIDAL, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, false},
	{CTSVC_VIEW_URI_SDN,				CTS_TABLE_SDN, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, false},

	{CTSVC_VIEW_URI_NAME,			CTSVC_DB_VIEW_NAME, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_COMPANY,		CTSVC_DB_VIEW_COMPANY, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_NUMBER,			CTSVC_DB_VIEW_NUMBER, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_EMAIL,			CTSVC_DB_VIEW_EMAIL, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_URL,			CTSVC_DB_VIEW_URL, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_ADDRESS,		CTSVC_DB_VIEW_ADDRESS, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_PROFILE,		CTSVC_DB_VIEW_PROFILE, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_RELATIONSHIP,	CTSVC_DB_VIEW_RELATIONSHIP, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_IMAGE,			CTSVC_DB_VIEW_IMAGE, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_NOTE,			CTSVC_DB_VIEW_NOTE, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_NICKNAME,		CTSVC_DB_VIEW_NICKNAME, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_EVENT,			CTSVC_DB_VIEW_EVENT, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_MESSENGER,		CTSVC_DB_VIEW_MESSENGER, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_GROUP_RELATION, CTSVC_DB_VIEW_GROUP_RELATION, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_EXTENSION,		CTSVC_DB_VIEW_EXTENSION, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},
	{CTSVC_VIEW_URI_SIP,			CTSVC_DB_VIEW_SIP, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_WRITE, true},

	/* Do not support get_all_records, get_records_with_query, get_count, get_count_with_query */
	/*
	 * {CTSVC_VIEW_URI_GROUPS_UPDATED_INFO, CTSVC_DB_VIEW_GROUPS_UPDATED_INFO, false},
	 * {CTSVC_VIEW_URI_GROUPS_MEMBER_UPDATED_INFO, CTSVC_DB_VIEW_GROUPS_MEMBER_UPDATED_INFO, false},
	 * {CTSVC_VIEW_URI_CONTACTS_UPDATED_INFO, CTSVC_DB_VIEW_CONTACTS_UPDATED_INFO, false},
	 * {CTSVC_VIEW_URI_MY_PROFILE_UPDATED_INFO, NULL, false},
	 * {CTSVC_VIEW_URI_GROUPRELS_UPDATED_INFO, NULL, false},
	 */

	{CTSVC_VIEW_URI_READ_ONLY_PERSON_CONTACT, CTSVC_DB_VIEW_PERSON_CONTACT, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_NONE, true},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_NUMBER, CTSVC_DB_VIEW_PERSON_NUMBER, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_NONE, true},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_EMAIL, CTSVC_DB_VIEW_PERSON_EMAIL, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_NONE, true},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP, CTSVC_DB_VIEW_PERSON_GROUP, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_NONE, true},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_ASSIGNED, CTSVC_DB_VIEW_PERSON_GROUP_ASSIGNED, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_NONE, true},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_NOT_ASSIGNED, CTSVC_DB_VIEW_PERSON_GROUP_NOT_ASSIGNED, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_NONE, true},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_PHONELOG, CTSVC_DB_VIEW_PERSON_PHONELOG, CTSVC_PERMISSION_CONTACT_READ|CTSVC_PERMISSION_PHONELOG_READ, CTSVC_PERMISSION_CONTACT_NONE, false},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_USAGE, CTSVC_DB_VIEW_PERSON_USAGE, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_NONE, true},

	{CTSVC_VIEW_URI_READ_ONLY_CONTACT_NUMBER, CTSVC_DB_VIEW_CONTACT_NUMBER, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_NONE, true},
	{CTSVC_VIEW_URI_READ_ONLY_CONTACT_EMAIL, CTSVC_DB_VIEW_CONTACT_EMAIL, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_NONE, true},
	{CTSVC_VIEW_URI_READ_ONLY_CONTACT_GROUP, CTSVC_DB_VIEW_CONTACT_GROUP, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_NONE, true},
	{CTSVC_VIEW_URI_READ_ONLY_CONTACT_ACTIVITY, CTSVC_DB_VIEW_CONTACT_ACTIVITY, CTSVC_PERMISSION_CONTACT_READ, CTSVC_PERMISSION_CONTACT_NONE, true},

	{CTSVC_VIEW_URI_READ_ONLY_PHONELOG_STAT, CTS_TABLE_PHONELOG_STAT, CTSVC_PERMISSION_PHONELOG_READ, CTSVC_PERMISSION_CONTACT_NONE, false},

};

/* this function is called in mutex lock */
int ctsvc_db_plugin_init()
{
	int i;

#ifdef _CONTACTS_NATIVE
	__ctsvc_db_view_ref_count++;
#endif

	if (__ctsvc_db_view_hash_table)
		return CONTACTS_ERROR_NONE;

	__ctsvc_db_view_hash_table = g_hash_table_new(g_str_hash, g_str_equal);

	if (__ctsvc_db_view_hash_table) {
		int count = sizeof(__db_tables) /sizeof(db_table_info_s);
		for (i = 0; i < count; i++)
			g_hash_table_insert(__ctsvc_db_view_hash_table, __db_tables[i].view_uri, GINT_TO_POINTER(&(__db_tables[i])));
	}
	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_plugin_deinit()
{
#ifdef _CONTACTS_NATIVE
	__ctsvc_db_view_ref_count--;
	if (__ctsvc_db_view_ref_count != 0)
		return;
#endif

	if (NULL == __ctsvc_db_view_hash_table)
		return CONTACTS_ERROR_NONE;

	g_hash_table_destroy(__ctsvc_db_view_hash_table);
	__ctsvc_db_view_hash_table = NULL;

	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_get_table_name(const char *view_uri, const char **out_table)
{
	db_table_info_s *db_view_info = NULL;

	RETV_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_table, CONTACTS_ERROR_INVALID_PARAMETER);

	if (__ctsvc_db_view_hash_table) {
		db_view_info = g_hash_table_lookup(__ctsvc_db_view_hash_table, view_uri);
		if (db_view_info) {
			*out_table = db_view_info->table_name;
			return CONTACTS_ERROR_NONE;
		}
	} else {
		ERR("Please check contact_connect()");
	}

	return CONTACTS_ERROR_INVALID_PARAMETER;
}

int ctsvc_required_read_permission(const char *view_uri)
{
	db_table_info_s *db_view_info = NULL;

	RETV_IF(NULL == view_uri, CTSVC_PERMISSION_CONTACT_NONE);

	if (__ctsvc_db_view_hash_table) {
		db_view_info = g_hash_table_lookup(__ctsvc_db_view_hash_table, view_uri);
		if (db_view_info)
			return db_view_info->read_permission;
	} else {
		ERR("Please check contact_connect()");
	}

	return CTSVC_PERMISSION_CONTACT_NONE;
}

int ctsvc_required_write_permission(const char *view_uri)
{
	db_table_info_s *db_view_info = NULL;

	RETV_IF(NULL == view_uri, CTSVC_PERMISSION_CONTACT_NONE);

	if (__ctsvc_db_view_hash_table) {
		db_view_info = g_hash_table_lookup(__ctsvc_db_view_hash_table, view_uri);
		if (db_view_info)
			return db_view_info->write_permission;
	} else {
		ERR("Please check contact_connect()");
	}

	return CTSVC_PERMISSION_CONTACT_NONE;
}

bool ctsvc_should_ab_access_control(const char *view_uri)
{
	db_table_info_s *db_view_info = NULL;

	RETV_IF(NULL == view_uri, false);

	if (__ctsvc_db_view_hash_table) {
		db_view_info = g_hash_table_lookup(__ctsvc_db_view_hash_table, view_uri);
		if (db_view_info)
			return db_view_info->need_ab_access_control;
	} else {
		ERR("Please check contact_connect()");
	}

	return false;
}

ctsvc_db_plugin_info_s *ctsvc_db_get_plugin_info(ctsvc_record_type_e type)
{
	switch ((int)type) {
	case CTSVC_RECORD_ADDRESSBOOK:
		return &ctsvc_db_plugin_addressbook;
	case CTSVC_RECORD_GROUP:
		return &ctsvc_db_plugin_group;
	case CTSVC_RECORD_PERSON:
		return &ctsvc_db_plugin_person;
	case CTSVC_RECORD_CONTACT:
		return &ctsvc_db_plugin_contact;
	case CTSVC_RECORD_MY_PROFILE:
		return &ctsvc_db_plugin_my_profile;
	case CTSVC_RECORD_SIMPLE_CONTACT:
		return &ctsvc_db_plugin_simple_contact;
	case CTSVC_RECORD_NAME:
		return &ctsvc_db_plugin_name;
	case CTSVC_RECORD_COMPANY:
		return &ctsvc_db_plugin_company;
	case CTSVC_RECORD_NOTE:
		return &ctsvc_db_plugin_note;
	case CTSVC_RECORD_NUMBER:
		return &ctsvc_db_plugin_number;
	case CTSVC_RECORD_EMAIL:
		return &ctsvc_db_plugin_email;
	case CTSVC_RECORD_URL:
		return &ctsvc_db_plugin_url;
	case CTSVC_RECORD_EVENT:
		return &ctsvc_db_plugin_event;
	case CTSVC_RECORD_NICKNAME:
		return &ctsvc_db_plugin_nickname;
	case CTSVC_RECORD_ADDRESS:
		return &ctsvc_db_plugin_address;
	case CTSVC_RECORD_MESSENGER:
		return &ctsvc_db_plugin_messenger;
	case CTSVC_RECORD_GROUP_RELATION:
		return &ctsvc_db_plugin_grouprelation;
	case CTSVC_RECORD_ACTIVITY:
		return &ctsvc_db_plugin_activity;
	case CTSVC_RECORD_ACTIVITY_PHOTO:
		return &ctsvc_db_plugin_activity_photo;
	case CTSVC_RECORD_PROFILE:
		return &ctsvc_db_plugin_profile;
	case CTSVC_RECORD_RELATIONSHIP:
		return &ctsvc_db_plugin_relationship;
	case CTSVC_RECORD_IMAGE:
		return &ctsvc_db_plugin_image;
	case CTSVC_RECORD_EXTENSION:
		return &ctsvc_db_plugin_extension;
#ifdef ENABLE_LOG_FEATURE
	case CTSVC_RECORD_PHONELOG:
		return &ctsvc_db_plugin_phonelog;
#endif /* ENABLE_LOG_FEATURE */
	case CTSVC_RECORD_SPEEDDIAL:
		return &ctsvc_db_plugin_speeddial;
	case CTSVC_RECORD_SDN:
		return &ctsvc_db_plugin_sdn;
	case CTSVC_RECORD_SIP:
		return &ctsvc_db_plugin_sip;
	case CTSVC_RECORD_UPDATED_INFO:
	case CTSVC_RECORD_RESULT:
	default:
		return NULL;
	}
}

#ifdef _CONTACTS_IPC_SERVER
static int __ctsvc_db_create_views()
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};

	if (__ctsvc_db_view_already_created)
		return CONTACTS_ERROR_NONE;

	/* CTSVC_DB_VIEW_CONTACT */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_CONTACT" AS "
			"SELECT * FROM "CTS_TABLE_CONTACTS" WHERE deleted = 0");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_execs() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_MY_PROFILE */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_MY_PROFILE" AS "
			"SELECT * FROM "CTS_TABLE_MY_PROFILES" WHERE deleted = 0");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_PERSON */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_PERSON" AS "
			"SELECT persons.person_id, "
			"display_name, reverse_display_name, "
			"display_name_language, "
			"reverse_display_name_language, "
			"sort_name, reverse_sort_name, "
			"sortkey, reverse_sortkey, "
			"name_contact_id, "
			"persons.ringtone_path, "
			"persons.image_thumbnail_path, "
			"persons.vibration, "
			"persons.message_alert, "
			"status, "
			"link_count, "
			"addressbook_ids, "
			"persons.has_phonenumber, "
			"persons.has_email, "
			"EXISTS(SELECT person_id FROM "CTS_TABLE_FAVORITES" WHERE person_id=persons.person_id) is_favorite, "
			"(SELECT favorite_prio FROM "CTS_TABLE_FAVORITES" WHERE person_id=persons.person_id) favorite_prio "
			"FROM "CTS_TABLE_CONTACTS", "CTS_TABLE_PERSONS" "
			"ON (name_contact_id = contacts.contact_id) ");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_NAME */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_NAME" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"data2, "
			"data3, "
			"data4, "
			"data5, "
			"data6, "
			"data7, "
			"data8, "
			"data9 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_NAME);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_NUMBER */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_NUMBER" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"is_default, "
			"data1, "
			"data2, "
			"data3, "
			"data4, "
			"data5, "
			"data6 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_NUMBER);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_EMAIL */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_EMAIL" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"is_default, "
			"data1, "
			"data2, "
			"data3 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_EMAIL);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_ADDRESS */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_ADDRESS" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"is_default, "
			"data1, "
			"data2, "
			"data3, "
			"data4, "
			"data5, "
			"data6, "
			"data7, "
			"data8, "
			"data9 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_POSTAL);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_URL */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_URL" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"data1, "
			"data2, "
			"data3 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_URL);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_EVENT */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_EVENT" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"data1, "
			"data2, "
			"data3, "
			"data4, "
			"data5 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_EVENT);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_GROUP_RELATION */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_GROUP_RELATION" AS "
			"SELECT "CTS_TABLE_GROUP_RELATIONS".group_id, "
			"contact_id, "
			"addressbook_id, "
			"group_name "
			"FROM "CTS_TABLE_GROUP_RELATIONS", "CTS_TABLE_GROUPS" "
			"ON "CTS_TABLE_GROUP_RELATIONS".group_id = "CTS_TABLE_GROUPS".group_id AND deleted = 0 "
			"ORDER BY group_prio");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_RELATIONSHIP */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_RELATIONSHIP" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"data1, "
			"data2, "
			"data3  "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_RELATIONSHIP);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_IMAGE */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_IMAGE" AS "
			"SELECT id, "
			"is_default, "
			"data.contact_id, "
			"addressbook_id, "
			"data1, "
			"data2, "
			"data3 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_IMAGE);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_COMPANY */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_COMPANY" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"data1, "
			"data2, "
			"data3, "
			"data4, "
			"data5, "
			"data6, "
			"data7, "
			"data8, "
			"data9, "
			"data10, "
			"data11 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_COMPANY);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_NICKNAME */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_NICKNAME" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"data3 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_NICKNAME);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_MESSENGER */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_MESSENGER" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"data1, "
			"data2, "
			"data3 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_MESSENGER);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_NOTE */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_NOTE" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"data3 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_NOTE);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_PROFILE */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_PROFILE" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"data1, "
			"data2, "
			"data3, "
			"data4, "
			"data5, "
			"data6, "
			"data7, "
			"data8, "
			"data9, "
			"data10, "
			"data11 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_PROFILE);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_EXTENSION */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_EXTENSION" AS "
			"SELECT id, "
			"data.contact_id, "
			"addressbook_id, "
			"data1, "
			"data2, "
			"data3, "
			"data4, "
			"data5, "
			"data6, "
			"data7, "
			"data8, "
			"data9, "
			"data10, "
			"data11, "
			"data12 "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
			CTSVC_DATA_EXTENSION);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_ACTIVITY */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_ACTIVITY" AS "
			"SELECT id, "
			"activities.contact_id, "
			"addressbook_id, "
			"source_name, "
			"status, "
			"timestamp, "
			"service_operation, "
			"uri "
			"FROM "CTS_TABLE_ACTIVITIES", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_ACTIVITIES".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"ORDER BY timestamp DESC");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_ACTIVITY_PHOTOS */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_ACTIVITY_PHOTOS" AS "
			"SELECT "CTS_TABLE_ACTIVITY_PHOTOS".id, "
			"activity_id, "
			"photo_url, "
			"sort_index "
			"FROM "CTS_TABLE_ACTIVITY_PHOTOS", "CTSVC_DB_VIEW_ACTIVITY" "
			"ON "CTS_TABLE_ACTIVITY_PHOTOS".activity_id = "CTSVC_DB_VIEW_ACTIVITY".id");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_SPEEDIDAL */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_SPEEDIDAL" AS "
			"SELECT persons.person_id, "
			"name_contacts.display_name, name_contacts.reverse_display_name, "
			"name_contacts.display_name_language, "
			"name_contacts.reverse_display_name_language, "
			"name_contacts.sort_name, name_contacts.reverse_sort_name, "
			"name_contacts.sortkey, name_contacts.reverse_sortkey, "
			"persons.image_thumbnail_path, "
			"data.id number_id, "
			"data.data1 type, "
			"data.data2 label, "
			"data.data3 number, "
			"data.data4 minmatch, "
			"data.data5 normalized_number, "
			"data.data6 cleaned_number, "
			"speeddials.speed_number  "
			"FROM "CTS_TABLE_PERSONS", "CTS_TABLE_CONTACTS" AS name_contacts, "
			CTSVC_DB_VIEW_CONTACT" AS temp_contacts, "
			CTS_TABLE_DATA", "CTS_TABLE_SPEEDDIALS" "
			"ON (persons.name_contact_id = name_contacts.contact_id "
			"AND persons.person_id = temp_contacts.person_id "
			"AND temp_contacts.contact_id = data.contact_id "
			"AND data.id = speeddials.number_id) "
			"ORDER BY speeddials.speed_number");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_PERSON_CONTACT */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_PERSON_CONTACT" AS "
			"SELECT * FROM "CTSVC_DB_VIEW_PERSON" "
			"JOIN (SELECT contact_id, "
			"addressbook_id, "
			"person_id person_id_in_contact "
			"FROM "CTSVC_DB_VIEW_CONTACT") temp_contacts "
			"JOIN (SELECT addressbook_id addressbook_id_in_addressbooks, addressbook_name, mode addressbook_mode "
			"FROM "CTS_TABLE_ADDRESSBOOKS") temp_addressbooks "
			"ON temp_contacts.person_id_in_contact = "CTSVC_DB_VIEW_PERSON".person_id "
			"AND addressbook_id = temp_addressbooks.addressbook_id_in_addressbooks");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_PERSON_NUMBER */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_PERSON_NUMBER" AS "
			"SELECT * FROM "CTSVC_DB_VIEW_PERSON_CONTACT" "
			"JOIN (SELECT id number_id, "
			"contact_id, "
			"data1 type, "
			"is_primary_default, "
			"data2 label, "
			"data3 number, "
			"data4 minmatch, "
			"data5 normalized_number, "
			"data6 cleaned_number "
			"FROM "CTS_TABLE_DATA" WHERE datatype = %d AND is_my_profile = 0) temp_data "
			"ON temp_data.contact_id = "CTSVC_DB_VIEW_PERSON_CONTACT".contact_id",
			CTSVC_DATA_NUMBER);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_PERSON_EMAIL */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_PERSON_EMAIL" AS "
			"SELECT * FROM "CTSVC_DB_VIEW_PERSON_CONTACT" "
			"JOIN (SELECT id email_id, "
			"contact_id, "
			"data1 type, "
			"is_primary_default, "
			"data2 label, "
			"data3 email "
			"FROM "CTS_TABLE_DATA" WHERE datatype = %d AND is_my_profile = 0) temp_data "
			"ON temp_data.contact_id = "CTSVC_DB_VIEW_PERSON_CONTACT".contact_id",
			CTSVC_DATA_EMAIL);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

#ifdef ENABLE_LOG_FEATURE
	/* CTSVC_DB_VIEW_PERSON_PHONELOG */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_PERSON_PHONELOG" AS "
			"SELECT L.id phonelog_id, "
			"C.display_name, "
			"C.reverse_display_name, "
			"C.display_name_language, "
			"C.reverse_display_name_language, "
			"C.sort_name, C.reverse_sort_name, "
			"C.sortkey, C.reverse_sortkey, "
			"P.image_thumbnail_path, "
			"L.number address, "
			"L.normal_num, "
			"L.minmatch, "
			"L.clean_num, "
			"L.log_type, "
			"L.log_time, "
			"L.data1, "
			"L.data2, "
			"L.sim_id, "
			"L.person_id id, "
			"L.number_type address_type "
			"FROM "CTS_TABLE_PHONELOGS" L "
			"LEFT JOIN "CTS_TABLE_PERSONS" P "
			"ON P.person_id = L.person_id "
			"LEFT JOIN "CTS_TABLE_CONTACTS" C "
			"ON P.name_contact_id = C.contact_id AND C.deleted = 0 "
			"ORDER BY L.log_time DESC");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);
#endif /* ENABLE_LOG_FEATURE */

	/* CTSVC_DB_VIEW_PERSON_USAGE */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_PERSON_USAGE" AS "
			"SELECT * FROM "CTSVC_DB_VIEW_PERSON" "
			"LEFT JOIN (SELECT usage_type, "
			"person_id, "
			"times_used "
			"FROM "CTS_TABLE_CONTACT_STAT") usage "
			"ON usage.person_id = "CTSVC_DB_VIEW_PERSON".person_id "
			"ORDER BY usage.times_used");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_PERSON_GROUP */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_PERSON_GROUP" AS "
			"SELECT view_person_contact.*, groups.group_id, group_prio "
			"FROM "CTSVC_DB_VIEW_PERSON_CONTACT" "
			"LEFT JOIN "CTS_TABLE_GROUP_RELATIONS" "
			"ON "CTS_TABLE_GROUP_RELATIONS".deleted = 0 AND "
			CTS_TABLE_GROUP_RELATIONS".contact_id = "CTSVC_DB_VIEW_PERSON_CONTACT".contact_id "
			"LEFT JOIN "CTS_TABLE_GROUPS" "
			"ON "CTS_TABLE_GROUP_RELATIONS".group_id = "CTS_TABLE_GROUPS".group_id "
			"ORDER BY group_prio");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_PERSON_GROUP_NOT_ASSIGNED */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_PERSON_GROUP_NOT_ASSIGNED" AS "
			"SELECT * FROM "CTSVC_DB_VIEW_PERSON_CONTACT" "
			"WHERE contact_id NOT IN (select contact_id FROM "CTS_TABLE_GROUP_RELATIONS" WHERE deleted = 0)");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_PERSON_GROUP_ASSIGNED */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_PERSON_GROUP_ASSIGNED" AS "
			"SELECT "CTSVC_DB_VIEW_PERSON_CONTACT".*, groups.group_id, group_prio "
			"FROM "CTSVC_DB_VIEW_PERSON_CONTACT", "
			CTS_TABLE_GROUP_RELATIONS", "CTS_TABLE_GROUPS" "
			"ON "
			CTS_TABLE_GROUP_RELATIONS".contact_id = "CTSVC_DB_VIEW_PERSON_CONTACT".contact_id AND "
			CTS_TABLE_GROUP_RELATIONS".group_id = "CTS_TABLE_GROUPS".group_id AND "
			CTS_TABLE_GROUP_RELATIONS".deleted = 0 "
			"ORDER BY group_prio");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_CONTACT_NUMBER */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_CONTACT_NUMBER" AS "
			"SELECT * FROM "CTSVC_DB_VIEW_CONTACT" "
			"JOIN (SELECT id number_id, "
			"contact_id, "
			"data1 type, "
			"is_default, "
			"data2 label, "
			"data3 number, "
			"data4 minmatch, "
			"data5 normalized_number, "
			"data6 cleaned_number "
			"FROM "CTS_TABLE_DATA" WHERE datatype = %d AND is_my_profile = 0) temp_data "
			"ON temp_data.contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id",
			CTSVC_DATA_NUMBER);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_CONTACT_EMAIL */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_CONTACT_EMAIL" AS "
			"SELECT * FROM "CTSVC_DB_VIEW_CONTACT" "
			"JOIN (SELECT id email_id, "
			"contact_id, "
			"data1 type, "
			"is_default, "
			"data2 label, "
			"data3 email "
			"FROM "CTS_TABLE_DATA" WHERE datatype = %d AND is_my_profile = 0) temp_data "
			"ON temp_data.contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id",
			CTSVC_DATA_EMAIL);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_CONTACT_GROUP */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_CONTACT_GROUP" AS "
			"SELECT C.*, groups.group_id, group_name "
			"FROM "CTSVC_DB_VIEW_CONTACT" C "
			"LEFT JOIN "CTS_TABLE_GROUP_RELATIONS" "
			"ON "CTS_TABLE_GROUP_RELATIONS".deleted = 0 AND "
			CTS_TABLE_GROUP_RELATIONS".contact_id = C.contact_id "
			"LEFT JOIN "CTS_TABLE_GROUPS" "
			"ON "CTS_TABLE_GROUP_RELATIONS".group_id = "CTS_TABLE_GROUPS".group_id");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_CONTACT_ACTIVITY */
	snprintf(query, sizeof(query),
			"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_CONTACT_ACTIVITY" AS "
			"SELECT A.contact_id, "
			"A.display_name, "
			"A.display_name_source, "
			"A.reverse_display_name, "
			"A.display_name_language, "
			"A.reverse_display_name_language, "
			"A.sort_name, A.reverse_sort_name, "
			"A.sortkey, A.reverse_sortkey, "
			"A.addressbook_id, "
			"AB.account_id, "
			"A.person_id, "
			"A.ringtone_path, "
			"A.image_thumbnail_path, "
			"AC.id activity_id, "
			"AC.source_name, "
			"AC.status, "
			"AC.timestamp, "
			"AC.service_operation, "
			"AC.uri "
			"FROM "CTSVC_DB_VIEW_CONTACT" A, "CTS_TABLE_ACTIVITIES" AC, "CTS_TABLE_ADDRESSBOOKS" AB "
			"ON A.contact_id = AC.contact_id "
			"AND A.addressbook_id = AB.addressbook_id "
			"ORDER BY timestamp DESC");
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_query_exec() Fail(%d)", ret);

	/* CTSVC_DB_VIEW_SIP */
	snprintf(query, sizeof(query),
		"CREATE VIEW IF NOT EXISTS "CTSVC_DB_VIEW_SIP" AS "
			"SELECT id, "
					"data.contact_id, "
					"data1 address, "
					"data2 type, "
					"data3 label "
			"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
			"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
			"WHERE datatype = %d AND is_my_profile = 0 ",
				CTSVC_DATA_SIP);
	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "DB error : ctsvc_query_exec() Fail(%d)", ret);

	__ctsvc_db_view_already_created = true;

	return CONTACTS_ERROR_NONE;
}

#endif

int ctsvc_db_init()
{
	int ret = CONTACTS_ERROR_NONE;
	ret = ctsvc_db_open();
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_db_open() Fail(%d)", ret);
		return ret;
	}

#ifdef _CONTACTS_IPC_SERVER
	ret = __ctsvc_db_create_views();
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("__ctsvc_db_create_views() Fail(%d)", ret);
		return ret;
	}
#endif

	return ret;
}

int ctsvc_db_deinit()
{
	int ret = CONTACTS_ERROR_NONE;
	ret = ctsvc_db_close();
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_db_close() Fail(%d)", ret);
		return ret;
	}

	return CONTACTS_ERROR_NONE;
}



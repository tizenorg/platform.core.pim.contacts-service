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
#include <unistd.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_schema.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_query.h"
#include "ctsvc_utils.h"
#include "ctsvc_record.h"
#include "ctsvc_normalize.h"
#include "ctsvc_list.h"
#include "ctsvc_setting.h"
#include "ctsvc_localize_ch.h"
#include "ctsvc_group.h"
#include "ctsvc_notification.h"
#include "ctsvc_localize.h"
#include "ctsvc_person.h"

#include "ctsvc_db_plugin_contact_helper.h"
#include "ctsvc_db_plugin_person_helper.h"

#define CTSVC_MY_IMAGE_LOCATION "/opt/usr/data/contacts-svc/img/my"

#define CTSVC_CONTACT_DISPLAY_NAME_MAX_LEN 1024
#define CTSVC_CONTACT_SEARCH_DATA_MAX_LEN 1024
#define CTSVC_CONTACT_INITIAL_DATA_MAX_LEN 128

static int __ctsvc_db_contact_insert_record( contacts_record_h record, int *id );
static int __ctsvc_db_contact_get_record( int id, contacts_record_h* out_record );
static int __ctsvc_db_contact_update_record( contacts_record_h record );
static int __ctsvc_db_contact_delete_record( int id );
static int __ctsvc_db_contact_replace_record( contacts_record_h record, int id );

static int __ctsvc_db_contact_get_all_records( int offset, int limit, contacts_list_h* out_list );
static int __ctsvc_db_contact_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list );
//static int __ctsvc_db_contact_insert_records(const contacts_list_h in_list, int **ids);
//static int __ctsvc_db_contact_update_records(const contacts_list_h in_list);
//static int __ctsvc_db_contact_delete_records(int ids[], int count);

ctsvc_db_plugin_info_s ctsvc_db_plugin_contact = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_contact_insert_record,
	.get_record = __ctsvc_db_contact_get_record,
	.update_record = __ctsvc_db_contact_update_record,
	.delete_record = __ctsvc_db_contact_delete_record,
	.get_all_records = __ctsvc_db_contact_get_all_records,
	.get_records_with_query = __ctsvc_db_contact_get_records_with_query,
	.insert_records = NULL,//__ctsvc_db_contact_insert_records,
	.update_records = NULL,//__ctsvc_db_contact_update_records,
	.delete_records = NULL,//__ctsvc_db_contact_delete_records
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = __ctsvc_db_contact_replace_record,
	.replace_records = NULL,
};

static int __ctsvc_db_get_contact_base_info(int id, ctsvc_contact_s *contact)
{
	int ret, len;
	int i;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	char *temp;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

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

	i = 0;
	contact->id = ctsvc_stmt_get_int(stmt, i++);
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
#if 0
	contact->sync_data1 = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	contact->sync_data1 = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	contact->sync_data1 = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	contact->sync_data1 = SAFE_STRDUP(temp);
#endif
	cts_stmt_finalize(stmt);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_data(int id, ctsvc_contact_s *contact)
{
	int ret, len;
	int datatype;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	len = snprintf(query, sizeof(query),
				"SELECT datatype, id, data.contact_id, is_default, data1, data2, "
					"data3, data4, data5, data6, data7, data8, data9, data10, data11, data12 "
					"FROM "CTS_TABLE_DATA", "CTSVC_DB_VIEW_CONTACT" "
					"ON "CTS_TABLE_DATA".contact_id = "CTSVC_DB_VIEW_CONTACT".contact_id "
					"WHERE data.contact_id = %d  AND is_my_profile = 0", id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (1 /*CTS_TRUE */!= ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}

	do {
		datatype = ctsvc_stmt_get_int(stmt, 0);
		switch (datatype) {
		case CTSVC_DATA_NAME:
			ctsvc_get_data_info_name(stmt, (contacts_list_h)contact->name);
			break;
		case CTSVC_DATA_EVENT:
			ctsvc_get_data_info_event(stmt, (contacts_list_h)contact->events);
			break;
		case CTSVC_DATA_MESSENGER:
			ctsvc_get_data_info_messenger(stmt, (contacts_list_h)contact->messengers);
			break;
		case CTSVC_DATA_POSTAL:
			ctsvc_get_data_info_address(stmt, (contacts_list_h)contact->postal_addrs);
			break;
		case CTSVC_DATA_URL:
			ctsvc_get_data_info_url(stmt, (contacts_list_h)contact->urls);
			break;
		case CTSVC_DATA_NICKNAME:
			ctsvc_get_data_info_nickname(stmt, (contacts_list_h)contact->nicknames);
			break;
		case CTSVC_DATA_NUMBER:
			ctsvc_get_data_info_number(stmt, (contacts_list_h)contact->numbers);
			break;
		case CTSVC_DATA_EMAIL:
			ctsvc_get_data_info_email(stmt, (contacts_list_h)contact->emails);
			break;
		case CTSVC_DATA_PROFILE:
			ctsvc_get_data_info_profile(stmt, (contacts_list_h)contact->profiles);
			break;
		case CTSVC_DATA_RELATIONSHIP:
			ctsvc_get_data_info_relationship(stmt, (contacts_list_h)contact->relationships);
			break;
		case CTSVC_DATA_IMAGE:
			ctsvc_get_data_info_image(stmt, (contacts_list_h)contact->images);
			break;
		case CTSVC_DATA_COMPANY:
			ctsvc_get_data_info_company(stmt, (contacts_list_h)contact->company);
			break;
		case CTSVC_DATA_NOTE:
			ctsvc_get_data_info_note(stmt, (contacts_list_h)contact->note);
			break;
		case CTSVC_DATA_EXTENSION:
			ctsvc_get_data_info_extension(stmt, (contacts_list_h)contact->extensions);
			break;
		default:
			CTS_ERR("Intenal : Not supported data type (%d)", datatype);
			break;
		}

	}while(1 /*CTS_TRUE*/ == cts_stmt_step(stmt));

	cts_stmt_finalize(stmt);

	return CONTACTS_ERROR_NONE;

}

static inline int __ctsvc_get_contact_grouprel(int contact_id, ctsvc_contact_s *contact)
{
	CTS_FN_CALL;
	ctsvc_group_relation_s *grouprel;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	char *temp;

	snprintf(query, sizeof(query),
		"SELECT group_id, contact_id, group_name "
			" FROM "CTSVC_DB_VIEW_GROUP_RELATION" WHERE contact_id = %d", contact_id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	while (1 /*CTS_TRUE */ == cts_stmt_step(stmt)) {
		contacts_record_create(_contacts_group_relation._uri, (contacts_record_h*)&grouprel);

		if (grouprel) {
			grouprel->group_id = ctsvc_stmt_get_int(stmt, 0);
			grouprel->id = grouprel->group_id;
			grouprel->contact_id = ctsvc_stmt_get_int(stmt, 1);
			temp = ctsvc_stmt_get_text(stmt, 2);
			grouprel->group_name = SAFE_STRDUP(temp);

			ctsvc_list_prepend((contacts_list_h)contact->grouprelations, (contacts_record_h)grouprel);
		}
	}

	cts_stmt_finalize(stmt);
	ctsvc_list_reverse((contacts_list_h)contact->grouprelations);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_contact_get_record( int id, contacts_record_h* out_record )
{
	int ret;
	contacts_record_h record;
	ctsvc_contact_s *contact;

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	contacts_record_create(_contacts_contact._uri, &record);
	contact = (ctsvc_contact_s *)record;
	ret = __ctsvc_db_get_contact_base_info(id, contact);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_get_main_contacts_info(ALL) Failed(%d)", ret);
		contacts_record_destroy(record, true);
		return ret;
	}

	ret = __ctsvc_db_get_data(id, contact);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_get_data_info Failed(%d)", ret);
		contacts_record_destroy(record, true);
		return ret;
	}

	ret = __ctsvc_get_contact_grouprel(id, contact);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_get_group_relations Failed(%d)", ret);
		contacts_record_destroy(record, true);
		return ret;
	}

	*out_record = record;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_contact_delete_record( int id )
{
	CTS_FN_CALL;
	int ret, rel_changed;
	int addressbook_id;
	int person_id;
	int link_count = 0;
	char query[CTS_SQL_MAX_LEN] = {0};
	cts_stmt stmt = NULL;

	snprintf(query, sizeof(query),
		"SELECT addressbook_id, person_id "
		"FROM "CTS_TABLE_CONTACTS" WHERE contact_id = %d AND deleted = 0", id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB , "DB error : cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (1 != ret) {
		CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	addressbook_id = ctsvc_stmt_get_int(stmt, 0);
	person_id = ctsvc_stmt_get_int(stmt, 1);
	CTS_DBG("addressbook_id : %d, person_id : %d", addressbook_id, person_id);
	cts_stmt_finalize(stmt);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "DB error : ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"UPDATE %s SET member_changed_ver=%d "
				"WHERE group_id IN (SELECT group_id FROM %s WHERE contact_id = %d AND deleted = 0) ",
				CTS_TABLE_GROUPS, ctsvc_get_next_ver(), CTS_TABLE_GROUP_RELATIONS, id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	rel_changed = cts_db_change();
	// images are deleted by db trigger callback function in ctsvc_db_contact_delete_callback
	snprintf(query, sizeof(query), "DELETE FROM %s WHERE contact_id = %d",
			CTS_TABLE_CONTACTS, id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "SELECT link_count FROM "CTS_TABLE_PERSONS" WHERE person_id = %d", person_id);
	ret = ctsvc_query_get_first_int_result(query, &link_count);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_query_get_first_int_result() Failed(%d)", ret);
	// set dirty bit to person by trigger : person will be aggregated in ctsvc_person_aggregate
	if (1 < link_count)
		ctsvc_person_aggregate(person_id);
	else
		ctsvc_set_person_noti();

	ctsvc_set_contact_noti();
	if (rel_changed > 0)
		ctsvc_set_group_rel_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
	{
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_safe_strcmp(char *s1, char *s2)
{
	if (NULL == s1 || NULL == s2)
		return !(s1 == s2);
	else
		return strcmp(s1, s2);
}

static inline void __ctsvc_update_contact_display_name(ctsvc_contact_s *contact)
{
	char *temp_display_name = NULL;
	char *display = NULL;
	char *sortkey = NULL;
	GList *cur;
	int ret, len=0, display_len=0;

	ctsvc_name_s *name = NULL;
	if ( contact->name->count > 0 && contact->name->records != NULL && contact->name->records->data != NULL )
	{
		name = (ctsvc_name_s *)contact->name->records->data;
	}

	if ( name && ( name->first || name->last) ) {
		if (name->first && name->last){
			temp_display_name = calloc(1, SAFE_STRLEN(name->first) + SAFE_STRLEN(name->first) + 2 );
			snprintf(temp_display_name, SAFE_STRLEN(name->first) + SAFE_STRLEN(name->first) + 2, "%s %s",name->first,name->last);
		}
		else if (name->first)
			temp_display_name = strdup(name->first);
		else
			temp_display_name = strdup(name->last);

		if (0 != __ctsvc_safe_strcmp(contact->display_name, temp_display_name)
				|| CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NAME != contact->display_source_type) {
			contact->display_name_changed = true;

			// make display name
			display_len = SAFE_STRLEN(name->first)
								+ SAFE_STRLEN(name->addition)
								+ SAFE_STRLEN(name->last)
								+ SAFE_STRLEN(name->suffix) + 5;

			display = calloc(1, display_len);

			if(name->first)
				len += snprintf(display + len, display_len - len, "%s", name->first);

			if(name->addition) {
				if (*display)
					len += snprintf(display + len, display_len - len, " ");
				len += snprintf(display + len, display_len - len, "%s", name->addition);
			}

			if(name->last) {
				if (*display)
					len += snprintf(display + len, display_len - len, " ");
				len += snprintf(display + len, display_len - len, "%s", name->last);
			}

			if(name->suffix) {
				if (*display)
					len += snprintf(display + len, display_len - len, " ");
				len += snprintf(display + len, display_len - len, "%s", name->suffix);
			}
			contact->display_name = display;

			display = calloc(1, display_len);
			// make reverse_display_name
			len = 0;
			if(name->last) {
				len += snprintf(display + len, display_len - len, "%s", name->last);

				if(name->first || name->addition)
					len += snprintf(display + len, display_len - len, ",");
			}

			if(name->first) {
				if (*display)
					len += snprintf(display + len, display_len - len, " ");
				len += snprintf(display + len, display_len - len, "%s", name->first);
			}

			if(name->addition) {
				if (*display)
					len += snprintf(display + len, display_len - len, " ");
				len += snprintf(display + len, display_len - len, "%s", name->addition);
			}

			if(name->suffix) {
				if (*display)
					len += snprintf(display + len, sizeof(display) - len, " ");
				len += snprintf(display + len, sizeof(display) - len, "%s", name->suffix);
			}


			contact->reverse_display_name = display;

			ret = ctsvc_check_language_type(contact->display_name);
			if (0 <= ret) {
				if (ctsvc_get_default_language() == ret)
					contact->display_name_language = CTSVC_LANG_DEFAULT;
				else if (ctsvc_get_secondary_language() == ret)
					contact->display_name_language = CTSVC_LANG_SECONDARY;
				else
					contact->display_name_language = ret;
			}

			ret = ctsvc_collation_str(contact->display_name, &sortkey);
			if (CONTACTS_ERROR_NONE == ret)
				contact->sortkey = sortkey;
			else
				free(sortkey);
			sortkey = NULL;

			ret = ctsvc_collation_str(contact->reverse_display_name, &sortkey);
			if (CONTACTS_ERROR_NONE == ret)
				contact->reverse_sortkey = sortkey;
			else
				free(sortkey);
			contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NAME;
		}

		free(temp_display_name);
	}

	else {
		if (contact->company && contact->company->records) {
			for (cur=contact->company->records;cur;cur=cur->next) {
				ctsvc_company_s *company = (ctsvc_company_s *)cur->data;
				if (company && company->name) {
					if (__ctsvc_safe_strcmp(contact->display_name, company->name)
						|| CONTACTS_DISPLAY_NAME_SOURCE_TYPE_COMPANY != contact->display_source_type) {
						contact->display_name_changed = true;
						contact->display_name = SAFE_STRDUP(company->name);
						contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_COMPANY;
						break;
					}
				}
			}
		}
		else if (contact->nicknames && contact->nicknames->records) {
			for (cur=contact->nicknames->records;cur;cur=cur->next) {
				ctsvc_nickname_s *nickname = (ctsvc_nickname_s *)cur->data;
				if (nickname && nickname->nickname) {
					if (__ctsvc_safe_strcmp(contact->display_name, nickname->nickname)
						|| CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NICKNAME != contact->display_source_type) {
						contact->display_name = SAFE_STRDUP(nickname->nickname);
						contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NICKNAME;
						break;
					}
				}
			}
		}
		else if (contact->numbers && contact->numbers->records) {
			for (cur=contact->numbers->records;cur;cur=cur->next) {
				ctsvc_number_s *number = (ctsvc_number_s *)cur->data;
				if (number && number->number) {
					if (__ctsvc_safe_strcmp(contact->display_name, number->number)
							|| CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NUMBER != contact->display_source_type) {
						contact->display_name_changed = true;
						contact->display_name = SAFE_STRDUP(number->number);
						contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NUMBER;
						break;
					}
				}
			}
		}
		else if (contact->emails && contact->emails->records) {
			for (cur=contact->emails->records;cur;cur=cur->next) {
				ctsvc_email_s *email = (ctsvc_email_s*)cur->data;
				if (email && email->email_addr) {
					if (__ctsvc_safe_strcmp(contact->display_name, email->email_addr)
							|| CONTACTS_DISPLAY_NAME_SOURCE_TYPE_EMAIL != contact->display_source_type) {
						contact->display_name_changed = true;
						contact->display_name = SAFE_STRDUP(email->email_addr);
						contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_EMAIL;
						break;
					}
				}
			}
		}

		if (contact->display_name_changed) {
			contact->reverse_display_name = SAFE_STRDUP(contact->display_name);

			ret = ctsvc_check_language_type(contact->display_name);
			if (0 <= ret) {
				if (ctsvc_get_default_language() == ret)
					contact->display_name_language = CTSVC_LANG_DEFAULT;
				else if (ctsvc_get_secondary_language() == ret)
					contact->display_name_language = CTSVC_LANG_SECONDARY;
				else
					contact->display_name_language = ret;
			}

			ret = ctsvc_collation_str(contact->display_name, &sortkey);
			if (CONTACTS_ERROR_NONE == ret) {
				contact->sortkey = sortkey;
				contact->reverse_sortkey = strdup(sortkey);
			}
			else
				free(sortkey);
		}
	}
	return;
}

static inline int __ctsvc_update_contact_data(ctsvc_contact_s *contact)
{
	int ret;

	if (contact->name) {
		ret = ctsvc_contact_update_data_name((contacts_list_h)contact->name, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_name() Failed(%d)", ret);
			return ret;
		}
	}

	if (contact->company) {
		ret = ctsvc_contact_update_data_company((contacts_list_h)contact->company, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_company() Failed(%d)", ret);
			return ret;
		}
	}

	if (contact->note) {
		ret = ctsvc_contact_update_data_note((contacts_list_h)contact->note, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_note() Failed(%d)", ret);
			return ret;
		}
	}

	if (contact->events) {
		ret = ctsvc_contact_update_data_event((contacts_list_h)contact->events, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_events() Failed(%d)", ret);
			return ret;
		}
	}

	if (contact->messengers) {
		ret = ctsvc_contact_update_data_messenger((contacts_list_h)contact->messengers, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_messengers() Failed(%d)", ret);
			return ret;
		}
	}

	if (contact->postal_addrs) {
		ret = ctsvc_contact_update_data_address((contacts_list_h)contact->postal_addrs, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_address() Failed(%d)", ret);
			return ret;
		}
	}

	if (contact->urls) {
		ret = ctsvc_contact_update_data_url((contacts_list_h)contact->urls, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_url() Failed(%d)", ret);
			return ret;
		}
	}

	if (contact->nicknames) {
		ret = ctsvc_contact_update_data_nickname((contacts_list_h)contact->nicknames, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_nickname() Failed(%d)", ret);
			return ret;
		}
	}

	if (contact->numbers) {
		bool had_phonenumber;
		ret = ctsvc_contact_update_data_number((contacts_list_h)contact->numbers, contact->id, false, &had_phonenumber);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_number() Failed(%d)", ret);
			return ret;
		}
		contact->has_phonenumber = had_phonenumber;
	}

	if (contact->emails) {
		bool had_email;
		ret = ctsvc_contact_update_data_email((contacts_list_h)contact->emails, contact->id, false, &had_email);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_email() Failed(%d)", ret);
			return ret;
		}
		contact->has_email = had_email;
	}

	if (contact->profiles) {
		ret = ctsvc_contact_update_data_profile((contacts_list_h)contact->profiles, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_profile() Failed(%d)", ret);
			return ret;
		}
	}

	if (contact->relationships) {
		ret = ctsvc_contact_update_data_relationship((contacts_list_h)contact->relationships, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_relationship() Failed(%d)", ret);
			return ret;
		}
	}

	if (contact->images) {
		ret = ctsvc_contact_update_data_image((contacts_list_h)contact->images, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_image() Failed(%d)", ret);
			return ret;
		}
	}

	if (contact->extensions) {
		ret = ctsvc_contact_update_data_extension((contacts_list_h)contact->extensions, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_extension() Failed(%d)", ret);
			return ret;
		}
	}

	return CONTACTS_ERROR_NONE;
}

static void __ctsvc_contact_check_default_data(ctsvc_contact_s *contact)
{
	if (contact->numbers)
		contact->has_phonenumber = ctsvc_contact_check_default_number((contacts_list_h)contact->numbers);
	if (contact->emails)
		contact->has_email = ctsvc_contact_check_default_email((contacts_list_h)contact->emails);
	if (contact->images)
		ctsvc_contact_check_default_image((contacts_list_h)contact->images);
}

static inline int __ctsvc_update_contact_grouprel(int contact_id, contacts_list_h group_list)
{
	CTS_FN_CALL;
	ctsvc_group_relation_s *grouprel;
	ctsvc_list_s *list = (ctsvc_list_s*)group_list;
	int rel_changed = 0;
	unsigned int count;
	int ret;
	GList *cursor;

	RETV_IF(NULL == group_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		grouprel = (ctsvc_group_relation_s *)cursor->data;
		ret = ctsvc_group_remove_contact_in_transaction(grouprel->group_id, contact_id);
		if (0 < ret)
			rel_changed += ret;
	}

	ret = contacts_list_get_count(group_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(group_list);
	do {
		contacts_list_get_current_record_p(group_list, (contacts_record_h*)&grouprel);
		if (NULL == grouprel)
			continue;

		if (grouprel->group_id) {
			ret = ctsvc_group_add_contact_in_transaction(grouprel->group_id, contact_id);
			RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_db_group_set_relation() Failed(%d)", ret);
			if (0 < ret)
				rel_changed += ret;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(group_list));

	if (rel_changed)
		return rel_changed;
	else
		return CONTACTS_ERROR_NONE;
}

static inline void __ctsvc_contact_get_initial(char *src, char *dest, int dest_size, bool pinyin )
{
	int i, j=0;
	bool bFirst = true;
	int len = strlen(src);
	for(i = 0; i < len && j < (dest_size-1); i++)
	{
		if (src[i] == ' ') {
			bFirst=true;
		} else if (bFirst) {
			dest[j++] = src[i];
			if (!pinyin)
				dest[j++] = ' ';
			bFirst = false;
		}
	}
	CTS_DBG("src(%s) dest(%s)", src, dest);
}

static inline void __ctsvc_remove_space(char *src, char *dest, int dest_size)
{
	int len = strlen(src);
	int i, j=0;

	for(i=0; i < len && i < dest_size; i++) {
		if (src[i] && src[i] != ' ') {
			dest[j] = src[i];
			j++;
		}
	}
	dest[j] = '\0';
}

static inline int __ctsvc_contact_make_search_name(ctsvc_contact_s *contact, char **search_name) {
	char *name = NULL;
	int buf_size, ret;

	if (contact->display_name) {
		if (ctsvc_has_chinese(contact->display_name)) {
			pinyin_name_s *pinyinname;
			int size, i;

			ret = ctsvc_convert_chinese_to_pinyin(contact->display_name, &pinyinname, &size);

			if (CONTACTS_ERROR_NONE == ret) {
				char *name_nospace = calloc(1, CHINESE_PINYIN_SPELL_MAX_LEN);
				char *temp_name = NULL;

				name = calloc(1, strlen(contact->display_name)* 5);

				ctsvc_normalize_str(contact->display_name, name, strlen(contact->display_name)* 5);

				for(i=0; i<size; i++) {
					__ctsvc_remove_space(pinyinname[i].pinyin_name, name_nospace, CHINESE_PINYIN_SPELL_MAX_LEN*(CHINESE_PINYIN_MAX_LEN+1));

					buf_size = SAFE_STRLEN(name)
										+ SAFE_STRLEN(pinyinname[i].pinyin_name)
										+ SAFE_STRLEN(name_nospace) + 4;
					temp_name = calloc(1, buf_size);
					snprintf(temp_name, buf_size, "%s %s %s %s",
							name, pinyinname[i].pinyin_name, name_nospace, pinyinname[i].pinyin_initial);

					free(name);
					name = temp_name;
				}

				free(name_nospace);
				free(pinyinname);
			}
			else {
				char initial[CTSVC_CONTACT_INITIAL_DATA_MAX_LEN] = {0,};
				char *normalized_display_name = NULL;

				normalized_display_name = calloc(1, strlen(contact->display_name)* 5);

				ctsvc_normalize_str(contact->display_name, normalized_display_name, strlen(contact->display_name)* 5);
				__ctsvc_contact_get_initial(contact->display_name, initial, sizeof(initial), false);
				buf_size = SAFE_STRLEN(normalized_display_name) + strlen(initial) + 2;
				name = calloc(1, buf_size);
				snprintf(name, buf_size, "%s %s", normalized_display_name, initial);

				free(normalized_display_name);
			}
		}
		else if (CTSVC_LANG_KOREAN == ctsvc_check_language_type(contact->display_name)) {
			char *temp_name = NULL;
			char *chosung = calloc(1, strlen(contact->display_name) + 1);
			int count, i, j;
			int full_len, chosung_len;
			int total_len = strlen(contact->display_name);

			count = ctsvc_get_chosung(contact->display_name, chosung, strlen(contact->display_name) + 1 );

			name = calloc(1, strlen(contact->display_name)* 5);

			ctsvc_normalize_str(contact->display_name, name, strlen(contact->display_name)* 5);

			if (count > 0) {
				for(i=0, j=0; i < total_len; i+=full_len, j+=chosung_len) {
					full_len = ctsvc_check_utf8(contact->display_name[i]);
					chosung_len = ctsvc_check_utf8(chosung[j]);

					buf_size = SAFE_STRLEN(name) + SAFE_STRLEN(&contact->display_name[i]) + SAFE_STRLEN(&chosung[j]) + 3;
					temp_name = calloc(1, buf_size);

					snprintf(temp_name, buf_size, "%s %s %s", name, &contact->display_name[i], &chosung[j]);

					free(name);
					name = temp_name;
				}
			}
			free(chosung);
		}
		else {
			char initial[CTSVC_CONTACT_INITIAL_DATA_MAX_LEN] = {0,};
			char *normalized_display_name=NULL;

			normalized_display_name = calloc(1, strlen(contact->display_name)* 5);

			ctsvc_normalize_str(contact->display_name, normalized_display_name, strlen(contact->display_name)* 5);
			__ctsvc_contact_get_initial(contact->display_name, initial, sizeof(initial), false);
			buf_size = SAFE_STRLEN(normalized_display_name) + strlen(initial) + 2;
			name = calloc(1, buf_size);
			snprintf(name, buf_size, "%s %s", normalized_display_name, initial);

			free(normalized_display_name);
		}
	}
	*search_name = name;
	return CONTACTS_ERROR_NONE;
}


static inline int __ctsvc_contact_make_search_data(int contact_id, char **search_name,
		char **search_number, char **search_data)
{
	int ret, len = 0;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_contact_s *contact;

	char *number = NULL;
	char *data = NULL;
	char *temp_number=NULL;
	char *temp_data=NULL;
	int buf_size=0;

	ret = contacts_db_get_record(_contacts_contact._uri, contact_id, (contacts_record_h*)&contact);
	if (CONTACTS_ERROR_NO_DATA == ret) {
		int r;
		snprintf(query, sizeof(query), "DELETE FROM %s WHERE contact_id = %d",
				CTS_TABLE_SEARCH_INDEX, contact_id);
		r = ctsvc_query_exec(query);
		if (CONTACTS_ERROR_NONE != r) {
			CTS_ERR("ctsvc_query_exec() Failed(%d)", r);
			return r;
		}
		return ret;
	}
	else if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("contacts_db_get_record() Failed(%d)", ret);
		return ret;
	}

	__ctsvc_contact_make_search_name(contact, search_name);

	if (contact->numbers) {
		contacts_list_h number_list = (contacts_list_h)contact->numbers;
		ctsvc_number_s *number_record;
		contacts_list_first(number_list);
		len = 0;
		do {
			contacts_list_get_current_record_p(number_list, (contacts_record_h*)&number_record);
			if (NULL != number_record) {
				buf_size = SAFE_STRLEN(number) + SAFE_STRLEN(number_record->lookup) + 3;
				temp_number = calloc(1, buf_size);
				if (number)
					snprintf(temp_number, buf_size, "%s %s ", SAFE_STR(number), number_record->lookup);
				else
					snprintf(temp_number, buf_size, "%s ", number_record->lookup);
				free(number);
				number = temp_number;
			}
		}while(CONTACTS_ERROR_NONE == contacts_list_next(number_list));
	}

	len = 0;
	if (contact->emails) {
		contacts_list_h email_list = (contacts_list_h)contact->emails;
		ctsvc_email_s *email;
		contacts_list_first(email_list);
		do {
			contacts_list_get_current_record_p(email_list, (contacts_record_h*)&email);
			if (NULL != email) {
				buf_size = SAFE_STRLEN(data) + SAFE_STRLEN(email->email_addr) + 3;
				temp_data = calloc(1, buf_size);
				if (data)
					snprintf(temp_data, buf_size, "%s %s ",data, email->email_addr);
				else
					snprintf(temp_data, buf_size, "%s ",email->email_addr);
				free(data);
				data = temp_data;
			}
		}while(CONTACTS_ERROR_NONE == contacts_list_next(email_list));
	}

	if (contact->nicknames) {
		contacts_list_h nickname_list = (contacts_list_h)contact->nicknames;
		ctsvc_nickname_s *nickname;
		contacts_list_first(nickname_list);
		do {
			contacts_list_get_current_record_p(nickname_list, (contacts_record_h*)&nickname);
			if (NULL != nickname) {
				buf_size = SAFE_STRLEN(data) + SAFE_STRLEN(nickname->nickname) + 3;
				temp_data = calloc(1, buf_size);
				if (data)
					snprintf(temp_data, buf_size, "%s %s ", data, nickname->nickname);
				else
					snprintf(temp_data+len, buf_size, "%s ", nickname->nickname);
				free(data);
				data = temp_data;
			}
		}while(CONTACTS_ERROR_NONE == contacts_list_next(nickname_list));
	}

	if (contact->postal_addrs) {
		contacts_list_h address_list = (contacts_list_h)contact->postal_addrs;
		ctsvc_address_s *address;
		contacts_list_first(address_list);
		do {
			contacts_list_get_current_record_p(address_list, (contacts_record_h*)&address);
			if (NULL != address) {
				len =0;
				buf_size = SAFE_STRLEN(data)
							+ SAFE_STRLEN(address->country)
							+ SAFE_STRLEN(address->pobox)
							+ SAFE_STRLEN(address->postalcode)
							+ SAFE_STRLEN(address->region)
							+ SAFE_STRLEN(address->locality)
							+ SAFE_STRLEN(address->street)
							+ SAFE_STRLEN(address->extended) + 9;
				temp_data = calloc(1, buf_size);
				if(data)
					len += snprintf(temp_data + len, buf_size - len, "%s ", data);
				if (address->country)
					len += snprintf(temp_data + len, buf_size - len, "%s ", address->country);
				if (address->pobox)
					len += snprintf(temp_data + len, buf_size - len, "%s ", address->pobox);
				if (address->postalcode)
					len += snprintf(temp_data + len, buf_size - len, "%s ", address->postalcode);
				if (address->region)
					len += snprintf(temp_data + len, buf_size - len, "%s ", address->region);
				if (address->locality)
					len += snprintf(temp_data + len, buf_size - len, "%s ", address->locality);
				if (address->street)
					len += snprintf(temp_data + len, buf_size - len, "%s ", address->street);
				if (address->extended)
					len += snprintf(temp_data + len, buf_size - len, "%s ", address->extended);
				free(data);
				data = temp_data;
			}
		}while(CONTACTS_ERROR_NONE == contacts_list_next(address_list));
	}

	contacts_record_destroy((contacts_record_h)contact, true);

	*search_number = number;
	*search_data = data;
	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_update_contact_search_data(int contact_id)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	char *search_name = NULL;
	char *search_number = NULL;
	char *search_data = NULL;

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"UPDATE %s SET name=?, number=?, data=? "
			"WHERE contact_id = %d",
			CTS_TABLE_SEARCH_INDEX, contact_id);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("cts_query_prepare() Failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	ret = __ctsvc_contact_make_search_data(contact_id, &search_name, &search_number, &search_data);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_make_contact_search_data() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}

	if (search_name)
		cts_stmt_bind_text(stmt, 1, search_name);
	if(search_number)
		cts_stmt_bind_text(stmt, 2, search_number);
	if(search_data)
		cts_stmt_bind_text(stmt, 3, search_data);

	ret = cts_stmt_step(stmt);

	free(search_name);
	free(search_number);
	free(search_data);

	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_end_trans() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_contact_update_record( contacts_record_h record )
{
	int i, ret, len;
	int rel_changed = 0;
	int count;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;
	cts_stmt stmt;

	snprintf(query, sizeof(query),
		"SELECT count(contact_id) FROM "CTS_TABLE_CONTACTS" "
		"WHERE contact_id = %d AND deleted = 0", contact->id);
	ret = ctsvc_query_get_first_int_result(query, &count);
	RETVM_IF(1 != count, CONTACTS_ERROR_NO_DATA,
			"The index(%d) is Invalid. %d Record(s) is(are) found", contact->id, ret);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	__ctsvc_update_contact_display_name(contact);
	__ctsvc_contact_check_default_data(contact);

	//update data
	ret = __ctsvc_update_contact_data(contact);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_update_contact_data() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (contact->grouprelations) {
		rel_changed = __ctsvc_update_contact_grouprel(contact->id, (contacts_list_h)contact->grouprelations);
		if (rel_changed < CONTACTS_ERROR_NONE) {
			CTS_ERR("cts_update_contact_grouprel() Failed(%d)", rel_changed);
			ctsvc_end_trans(false);
			return rel_changed;
		}
	}

	//////////////////////////////////////////////////////////////////////
	// this code will be removed.
	free(contact->image_thumbnail_path);
	contact->image_thumbnail_path = NULL;
	contact->image_thumbnail_changed = false;

	if (contact->images) {
		int ret = CONTACTS_ERROR_NONE;
		contacts_record_h record = NULL;
		unsigned int count = 0;
		ctsvc_list_s *list = (ctsvc_list_s*)contact->images;
		ctsvc_image_s *image;
		GList *cursor;

		for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
			image = (ctsvc_image_s *)cursor->data;
			contact->image_thumbnail_path = NULL;
		}

		contacts_list_get_count((contacts_list_h)contact->images, &count);
		if (count) {
			contacts_list_first((contacts_list_h)contact->images);
			ret = contacts_list_get_current_record_p((contacts_list_h)contact->images, &record);

			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("contacts_list_get_current_record_p() Failed(%d)", ret);
				ctsvc_end_trans(false);
				return CONTACTS_ERROR_DB;
			}
			image = (ctsvc_image_s*)record;

			if ( image->path && *image->path && strstr(image->path, CTS_IMG_FULL_LOCATION) != NULL)
				contact->image_thumbnail_path = SAFE_STRDUP( image->path + strlen(CTS_IMG_FULL_LOCATION) + 1);
			else
				contact->image_thumbnail_path = SAFE_STRDUP(image->path);

		}
		contact->image_thumbnail_changed = true;
	}
	// this code will be removed.
	//////////////////////////////////////////////////////////////////////

	len = snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_CONTACTS" SET changed_ver=%d, changed_time=%d, "
					"has_phonenumber=%d, has_email=%d ",
					ctsvc_get_next_ver(), (int)time(NULL),
					contact->has_phonenumber, contact->has_email);

	if (contact->display_name_changed)
		len += snprintf(query+len, sizeof(query)-len, ", display_name=?, "
					"reverse_display_name=?, display_name_source=?, display_name_language=?, "
					"sortkey=?, reverse_sortkey=?");

	if (contact->uid_changed)
		len += snprintf(query+len, sizeof(query)-len, ", uid=?");

	if (contact->ringtone_changed)
		len += snprintf(query+len, sizeof(query)-len, ", ringtone_path=?");

	if (contact->vibration_changed)
		len += snprintf(query+len, sizeof(query)-len, ", vibration=?");

	if (contact->image_thumbnail_changed)
		len += snprintf(query+len, sizeof(query)-len, ", image_thumbnail_path=?");

	snprintf(query+len, sizeof(query)-len, " WHERE contact_id=%d", contact->id);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("cts_query_prepare() Failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	i=1;
	if (contact->display_name_changed) {
		cts_stmt_bind_text(stmt, i++, contact->display_name);
		cts_stmt_bind_text(stmt, i++, contact->reverse_display_name);
		cts_stmt_bind_int(stmt, i++, contact->display_source_type);
		cts_stmt_bind_int(stmt, i++, contact->display_name_language);
		cts_stmt_bind_text(stmt, i++, contact->sortkey);
		cts_stmt_bind_text(stmt, i++, contact->reverse_sortkey);
	}
	if (contact->uid_changed) {
		if(contact->uid)
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

	if (contact->image_thumbnail_changed ) {
		if (contact->image_thumbnail_path) {
			cts_stmt_bind_text(stmt, i, contact->image_thumbnail_path);
		}
		i++;
	}

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	ctsvc_set_contact_noti();
	if (0 < rel_changed)
		ctsvc_set_group_rel_noti();

	__ctsvc_update_contact_search_data(contact->id);
	ctsvc_db_update_person((contacts_record_h)contact);

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
	{
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_contact_get_all_records( int offset, int limit, contacts_list_h* out_list )
{
	int ret;
	int len;
	int contact_id;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;

	len = snprintf(query, sizeof(query),
			"SELECT contact_id FROM "CTS_TABLE_CONTACTS" WHERE deleted = 0");

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
		contact_id = ctsvc_stmt_get_int(stmt, 0);
		ret = contacts_db_get_record(_contacts_contact._uri, contact_id, &record);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : contacts_db_get_record() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return CONTACTS_ERROR_NO_DATA;
		}
		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_contact_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list )
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_contact_s *contact;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
	bool had_contact_id = false;
	int contact_id = 0;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s *)query;

	if (s_query->projection) {
		for (i=0;i<s_query->projection_count;i++) {
			if (s_query->projection[i] == CTSVC_PROPERTY_CONTACT_ID) {
				had_contact_id = true;
				break;
			}
		}
	}
	else
		had_contact_id = true;

	if (!had_contact_id) {
		s_query->projection = realloc(s_query->projection, s_query->projection_count+1);
		s_query->projection[s_query->projection_count] = CTSVC_PROPERTY_CONTACT_ID;
		s_query->projection_count++;
	}

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

		contacts_record_create(_contacts_contact._uri, &record);
		contact = (ctsvc_contact_s*)record;
		if (0 == s_query->projection_count)
			field_count = s_query->property_count;
		else
			field_count = s_query->projection_count;

		for(i=0;i<field_count;i++) {
			char *temp;
			int property_id;
			if (0 == s_query->projection_count)
				property_id = s_query->properties[i].property_id;
			else
				property_id = s_query->projection[i];

			switch(property_id) {
			case CTSVC_PROPERTY_CONTACT_ID:
				contact_id = ctsvc_stmt_get_int(stmt, i);
				if (had_contact_id)
					contact->id = contact_id;
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
		ret = __ctsvc_db_get_data(contact_id, contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_get_data_info Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		ret = __ctsvc_get_contact_grouprel(contact_id, contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_get_group_relations Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;

	return CONTACTS_ERROR_NONE;
}

//static int __ctsvc_db_contact_insert_records(const contacts_list_h in_list, int **ids) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_contact_update_records(const contacts_list_h in_list) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_contact_delete_records(int ids[], int count) { return CONTACTS_ERROR_NONE; }

static int __ctsvc_contact_insert_data(ctsvc_contact_s *contact)
{
	int ret;

	DBG("B ctsvc_contact_insert_data_name");
	//Insert the name
	if (contact->name) {
		ret = ctsvc_contact_insert_data_name((contacts_list_h)contact->name, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_name() Failed(%d)", ret);
			return ret;
		}
	}
	DBG("A ctsvc_contact_insert_data_name");

	//Insert the company
	if (contact->company) {
		ret = ctsvc_contact_insert_data_company((contacts_list_h)contact->company, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_contact_data_company() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the events
	if (contact->events) {
		ret = ctsvc_contact_insert_data_event((contacts_list_h)contact->events, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_contact_data_event() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the messengers
	if (contact->messengers) {
		ret = ctsvc_contact_insert_data_messenger((contacts_list_h)contact->messengers, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_contact_data_messenger() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the postals
	if (contact->postal_addrs) {
		ret = ctsvc_contact_insert_data_address((contacts_list_h)contact->postal_addrs, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_contact_data_postal() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the Web addrs
	if (contact->urls) {
		ret = ctsvc_contact_insert_data_url((contacts_list_h)contact->urls, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_contact_data_web() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the Nick names
	if (contact->nicknames) {
		ret = ctsvc_contact_insert_data_nickname((contacts_list_h)contact->nicknames, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_contact_data_nickname() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the numbers
	if (contact->numbers) {
		ret = ctsvc_contact_insert_data_number((contacts_list_h)contact->numbers, contact->id, false);
		if (ret < CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_contact_insert_data_number() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the emails
	if (contact->emails) {
		ret = ctsvc_contact_insert_data_email((contacts_list_h)contact->emails, contact->id, false);
		if (ret < CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_insert_contact_data_email() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the profile values
	if (contact->profiles) {
		ret = ctsvc_contact_insert_data_profile((contacts_list_h)contact->profiles, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_contact_data_profile() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the relationship values
	if (contact->relationships) {
		ret = ctsvc_contact_insert_data_relationship((contacts_list_h)contact->relationships, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_relationship() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the image values
	if (contact->images) {
		ret = ctsvc_contact_insert_data_image((contacts_list_h)contact->images, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_image() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the note values
	if (contact->note) {
		ret = ctsvc_contact_insert_data_note((contacts_list_h)contact->note, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_note() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the extensions values
	if (contact->extensions) {
		ret = ctsvc_contact_insert_data_extension((contacts_list_h)contact->extensions, contact->id, false);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_extension() Failed(%d)", ret);
			return ret;
		}
	}

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_contact_insert_search_data(int contact_id)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	char *search_name = NULL;
	char *search_number = NULL;
	char *search_data = NULL;

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE contact_id = %d",
			CTS_TABLE_SEARCH_INDEX, contact_id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec() Failed(%d)", ret);
		return ret;
	}

	snprintf(query, sizeof(query),
			"INSERT INTO %s(contact_id, name, number, data) "
			"VALUES(%d, ?, ?, ?)",
			CTS_TABLE_SEARCH_INDEX, contact_id);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("cts_query_prepare() Failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	ret = __ctsvc_contact_make_search_data(contact_id, &search_name, &search_number, &search_data);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_contact_make_search_data() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}

	if(search_name)
		cts_stmt_bind_text(stmt, 1, search_name);
	if(search_number)
		cts_stmt_bind_text(stmt, 2, search_number);
	if(search_data)
		cts_stmt_bind_text(stmt, 3, search_data);

	ret = cts_stmt_step(stmt);

	free(search_name);
	free(search_number);
	free(search_data);

	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_end_trans() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_contact_insert_grouprel(int contact_id, contacts_list_h group_list)
{
	CTS_FN_CALL;
	ctsvc_group_relation_s *grouprel;
	int rel_changed = 0;
	unsigned int count;
	int ret;

	RETV_IF(NULL == group_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(group_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(group_list);
	do {
		contacts_list_get_current_record_p(group_list, (contacts_record_h*)&grouprel);
		if (NULL == grouprel || grouprel->base.deleted)
			continue;

		if (grouprel->group_id) {
			int ret = ctsvc_group_add_contact_in_transaction(grouprel->group_id, contact_id);
			RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_db_group_set_relation() Failed(%d)", ret);
			if (0 < ret)
				rel_changed += ret;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(group_list));

	if (rel_changed)
		return rel_changed;
	else
		return CONTACTS_ERROR_NONE;
}

inline static int __ctsvc_find_person_to_link_with_number(const char *normalized_number, int *person_id)
{
	int ret;

	char query[CTS_SQL_MIN_LEN] = {0};
	char normal_num[CTSVC_NUMBER_MAX_LEN] = {0};
	char clean_num[CTSVC_NUMBER_MAX_LEN] = {0};

	ret = ctsvc_clean_number(normalized_number, clean_num, sizeof(clean_num));
	if (0 < ret) {
		ret = ctsvc_normalize_number(clean_num, normal_num, CTSVC_NUMBER_MAX_LEN);
		snprintf(query, sizeof(query),
				"SELECT C.person_id FROM "CTS_TABLE_CONTACTS" C, "CTS_TABLE_DATA" D "
				"ON C.contact_id=D.contact_id AND D.datatype=%d AND C.deleted = 0 "
				"WHERE D.data4='%s' AND D.is_my_profile = 0",
				CTSVC_DATA_NUMBER, normal_num);
		ret = ctsvc_query_get_first_int_result(query, person_id);
		CTS_DBG("%s", query);
		CTS_DBG("result ret(%d) person_id(%d)", ret, *person_id);
		return ret;
	}

	return CONTACTS_ERROR_INVALID_PARAMETER;
}

inline static int __ctsvc_find_person_to_link_with_email(const char *email_addr, int *person_id)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query),
			"SELECT C.person_id FROM "CTS_TABLE_CONTACTS" C, "CTS_TABLE_DATA" D "
			"ON C.contact_id=D.contact_id AND D.datatype=%d AND C.deleted = 0 "
			"WHERE D.data3='%s' AND D.is_my_profile = 0",
			CTSVC_DATA_EMAIL, email_addr);
	ret = ctsvc_query_get_first_int_result(query, person_id);
	CTS_DBG("%s", query);
	CTS_DBG("result ret(%d) person_id(%d)", ret, *person_id);

	return ret;
}


inline static int __ctsvc_find_person_to_link(contacts_record_h record, int *person_id)
{
	int ret;
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;
	ctsvc_number_s *number_data;
	ctsvc_email_s *email_data;
	GList *cursor;

	for(cursor = contact->numbers->records;cursor;cursor=cursor->next) {
		number_data = (ctsvc_number_s *)cursor->data;
		if (number_data && number_data->number && number_data->number[0]){
			ret = __ctsvc_find_person_to_link_with_number(number_data->number, person_id);

			if (ret == CONTACTS_ERROR_NONE && *person_id > 0)
				return ret;
		}
	}

	for(cursor = contact->emails->records;cursor;cursor=cursor->next) {
		email_data = (ctsvc_email_s *)cursor->data;
		if (email_data && email_data->email_addr && email_data->email_addr[0]){
			ret = __ctsvc_find_person_to_link_with_email(email_data->email_addr, person_id);

			if (ret == CONTACTS_ERROR_NONE && *person_id > 0)
				return ret;
		}
	}

	return CONTACTS_ERROR_NO_DATA;
}


static int __ctsvc_db_contact_insert_record( contacts_record_h record, int *id)
{
	int version;
	int ret, person_id = 0;
	char query[CTS_SQL_MAX_LEN] = {0};
	bool auto_link_enabled = true;

	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;
	int rel_changed = 0;
	cts_stmt stmt;

	// These check should be done in client side
//	RETVM_IF(contact->deleted, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : deleted contact record");
	RETVM_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER,
					"Invalid parameter : contact is NULL");
	RETVM_IF(contact->addressbook_id < 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : addressbook_id(%d) is mandatory field to insert contact record ", contact->addressbook_id);
	RETVM_IF(0 < contact->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : id(%d), This record is already inserted", contact->id);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	ret = cts_db_get_next_id(CTS_TABLE_CONTACTS);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("cts_db_get_next_id() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}
	contact->id = ret;
	if (id)
		*id = ret;

	DBG("B ctsvc_make_contact_display_name");
	ctsvc_make_contact_display_name(contact);
	DBG("A ctsvc_make_contact_display_name");
	__ctsvc_contact_check_default_data(contact);

	//Insert Data
	ret = __ctsvc_contact_insert_data(contact);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_insert_contact_data() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////
	// this code will be removed.
	free(contact->image_thumbnail_path);
	contact->image_thumbnail_path = NULL;

	if (contact->images) {
		ctsvc_image_s *image;
		unsigned int count = 0;

		contacts_list_get_count((contacts_list_h)contact->images, &count);

		while (count) {
			contacts_list_first((contacts_list_h)contact->images);
			ret = contacts_list_get_current_record_p((contacts_list_h)contact->images, (contacts_record_h*)&image);
			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("contacts_list_get_current_record_p() Failed(%d)", ret);
				ctsvc_end_trans(false);
				return CONTACTS_ERROR_DB;
			}

			if (image->path && image->is_default) {
				contact->image_thumbnail_path = strdup(image->path);
				break;
			}
			count--;
		}
	}
	// this code will be removed.
	//////////////////////////////////////////////////////////////////////

	version = ctsvc_get_next_ver();

	if (auto_link_enabled) {
		ret = __ctsvc_find_person_to_link((contacts_record_h)contact, &person_id);
		CTS_DBG("__ctsvc_find_person_to_link return %d , person_id(%d)", ret, person_id);
		if (ret == CONTACTS_ERROR_NONE && person_id > 0) {
			contact->person_id = person_id;
		}
		else {
			ret = ctsvc_db_insert_person((contacts_record_h)contact);
			CTS_DBG("ctsvc_db_insert_person return %d, person_id(%d)", ret, person_id);
			if (ret < CONTACTS_ERROR_NONE) {
				CTS_ERR("ctsvc_db_insert_person() Failed(%d)", ret);
				ctsvc_end_trans(false);
				return ret;
			}
			contact->person_id = ret;
		}
	}
	else {
		ret = ctsvc_db_insert_person((contacts_record_h)contact);
		CTS_DBG("ctsvc_db_insert_person return %d, person_id(%d)", ret, person_id);
		if (ret < CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_db_insert_person() Failed(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
		contact->person_id = ret;
	}

	snprintf(query, sizeof(query),
		"INSERT INTO "CTS_TABLE_CONTACTS"(contact_id, person_id, addressbook_id, is_restricted, "
			"created_ver, changed_ver, changed_time, has_phonenumber, has_email, "
			"display_name, reverse_display_name, display_name_source, display_name_language, "
			"sortkey, reverse_sortkey, "
			"uid, ringtone_path, vibration, image_thumbnail_path) "
			"VALUES(%d, %d, %d, %d, %d, %d, %d, %d, %d, ?, ?, %d, %d, ?, ?, ?, ?, ?, ?)",
			contact->id, contact->person_id, contact->addressbook_id, contact->is_restricted,
			version, version, (int)time(NULL), contact->has_phonenumber, contact->has_email,
			contact->display_source_type, contact->display_name_language);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("cts_query_prepare() Failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	if (contact->display_name)
		cts_stmt_bind_text(stmt, 1, contact->display_name);
	if (contact->reverse_display_name)
		cts_stmt_bind_text(stmt, 2, contact->reverse_display_name);
	if (contact->sortkey)
		cts_stmt_bind_text(stmt, 3, contact->sortkey);
	if (contact->reverse_sortkey)
		cts_stmt_bind_text(stmt, 4, contact->reverse_sortkey);
	if (contact->uid)
		cts_stmt_bind_text(stmt, 5, contact->uid);
	if (contact->ringtone_path)
		cts_stmt_bind_text(stmt, 6, contact->ringtone_path);
	if (contact->vibration)
		cts_stmt_bind_text(stmt, 7, contact->vibration);
	if (contact->image_thumbnail_path)
		cts_stmt_bind_text(stmt, 8, contact->image_thumbnail_path);

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	//Insert group Info
	if (contact->grouprelations) {
		rel_changed = __ctsvc_contact_insert_grouprel(contact->id, (contacts_list_h)contact->grouprelations);
		if (rel_changed < CONTACTS_ERROR_NONE) {
			CTS_ERR("__ctsvc_contact_insert_grouprel() Failed(%d)", rel_changed);
			ctsvc_end_trans(false);
			return rel_changed;
		}
	}

	DBG("B __ctsvc_contact_insert_search_data");
	__ctsvc_contact_insert_search_data(contact->id);
	DBG("A __ctsvc_contact_insert_search_data");

	if (rel_changed)
		ctsvc_set_group_rel_noti();
	ctsvc_set_contact_noti();

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_svc_end_trans() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_contact_replace_record( contacts_record_h record, int contact_id )
{
	CTS_FN_CALL;
	int ret, len;
	int rel_changed = 0;
	int count;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;
	cts_stmt stmt;

	snprintf(query, sizeof(query),
		"SELECT count(contact_id) FROM "CTS_TABLE_CONTACTS" "
		"WHERE contact_id = %d AND deleted = 0", contact_id);
	ret = ctsvc_query_get_first_int_result(query, &count);
	RETVM_IF(1 != count, CONTACTS_ERROR_NO_DATA,
			"The index(%d) is Invalid. %d Record(s) is(are) not found", contact->id, ret);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	contact->id = contact_id;
	__ctsvc_update_contact_display_name(contact);
	__ctsvc_contact_check_default_data(contact);

	//remove current child data
	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_DATA" WHERE contact_id = %d", contact_id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ret = __ctsvc_contact_insert_data(contact);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_contact_insert_data() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	//remove current child data
	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_GROUP_RELATIONS" WHERE contact_id = %d", contact_id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (contact->grouprelations) {
		rel_changed = __ctsvc_contact_insert_grouprel(contact_id, (contacts_list_h)contact->grouprelations);
		if (rel_changed < CONTACTS_ERROR_NONE) {
			CTS_ERR("__ctsvc_contact_insert_grouprel() Failed(%d)", rel_changed);
			ctsvc_end_trans(false);
			return rel_changed;
		}
	}

	//////////////////////////////////////////////////////////////////////
	// this code will be removed.
	free(contact->image_thumbnail_path);
	contact->image_thumbnail_path = NULL;
	contact->image_thumbnail_changed = false;

	if (contact->images) {
		int ret = CONTACTS_ERROR_NONE;
		contacts_record_h record = NULL;
		unsigned int count = 0;
		ctsvc_list_s *list = (ctsvc_list_s*)contact->images;
		ctsvc_image_s *image;
		GList *cursor;

		for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
			image = (ctsvc_image_s *)cursor->data;
			contact->image_thumbnail_path = NULL;
		}

		contacts_list_get_count((contacts_list_h)contact->images, &count);
		if (count) {
			contacts_list_first((contacts_list_h)contact->images);
			ret = contacts_list_get_current_record_p((contacts_list_h)contact->images, &record);

			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("contacts_list_get_current_record_p() Failed(%d)", ret);
				ctsvc_end_trans(false);
				return CONTACTS_ERROR_DB;
			}
			image = (ctsvc_image_s*)record;

			if ( image->path && *image->path && strstr(image->path, CTS_IMG_FULL_LOCATION) != NULL)
				contact->image_thumbnail_path = SAFE_STRDUP( image->path + strlen(CTS_IMG_FULL_LOCATION) + 1);
			else
				contact->image_thumbnail_path = SAFE_STRDUP(image->path);

		}
		contact->image_thumbnail_changed = true;
	}
	// this code will be removed.
	//////////////////////////////////////////////////////////////////////

	len = snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_CONTACTS" SET changed_ver=%d, changed_time=%d, "
					"has_phonenumber=%d, has_email=%d , display_name=?, "
					"reverse_display_name=?, display_name_source=%d, display_name_language=%d, "
					"sortkey=?, reverse_sortkey=?, uid=?, ringtone_path=?, vibration=?, "
					"image_thumbnail_path=? WHERE contact_id=%d",
					ctsvc_get_next_ver(), (int)time(NULL),
					contact->has_phonenumber, contact->has_email,
					contact->display_source_type, contact->display_name_language,
					contact->id);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("cts_query_prepare() Failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	if (contact->display_name)
		cts_stmt_bind_text(stmt, 1, contact->display_name);
	if (contact->reverse_display_name)
		cts_stmt_bind_text(stmt, 2, contact->reverse_display_name);
	if (contact->sortkey)
		cts_stmt_bind_text(stmt, 3, contact->sortkey);
	if (contact->reverse_sortkey)
		cts_stmt_bind_text(stmt, 4, contact->reverse_sortkey);
	if (contact->uid)
		cts_stmt_bind_text(stmt, 5, contact->uid);
	if (contact->ringtone_path)
		cts_stmt_bind_text(stmt, 6, contact->ringtone_path);
	if (contact->vibration)
		cts_stmt_bind_text(stmt, 7, contact->vibration);
	if (contact->image_thumbnail_path)
		cts_stmt_bind_text(stmt, 8, contact->image_thumbnail_path);

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	ctsvc_set_contact_noti();
	if (0 < rel_changed)
		ctsvc_set_group_rel_noti();

	__ctsvc_update_contact_search_data(contact->id);
	ctsvc_db_update_person((contacts_record_h)contact);

	ret = ctsvc_end_trans(true);

	if (ret < CONTACTS_ERROR_NONE)
		return ret;
	else
		return CONTACTS_ERROR_NONE;
}


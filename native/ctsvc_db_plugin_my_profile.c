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

#include "ctsvc_db_plugin_contact_helper.h"

#define CTSVC_MY_PROFILE_DISPLAY_NAME_MAX_LEN 1024

static int __ctsvc_db_my_profile_insert_record( contacts_record_h record, int *id );
static int __ctsvc_db_my_profile_get_record( int id, contacts_record_h* out_record );
static int __ctsvc_db_my_profile_update_record( contacts_record_h record );
static int __ctsvc_db_my_profile_delete_record( int id );

static int __ctsvc_db_my_profile_get_all_records( int offset, int limit, contacts_list_h* out_list );
static int __ctsvc_db_my_profile_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list );

ctsvc_db_plugin_info_s ctsvc_db_plugin_my_profile = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_my_profile_insert_record,
	.get_record = __ctsvc_db_my_profile_get_record,
	.update_record = __ctsvc_db_my_profile_update_record,
	.delete_record = __ctsvc_db_my_profile_delete_record,
	.get_all_records = __ctsvc_db_my_profile_get_all_records,
	.get_records_with_query = __ctsvc_db_my_profile_get_records_with_query,
	.insert_records = NULL,
	.update_records = NULL,
	.delete_records = NULL,
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};

static int __ctsvc_db_get_my_profile_base_info(int id, ctsvc_my_profile_s *my_profile)
{
	int ret, len;
	int i;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	char *temp;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	len = snprintf(query, sizeof(query),
			"SELECT my_profile_id, addressbook_id, changed_time, %s, image_thumbnail_path, uid "
				"FROM "CTS_TABLE_MY_PROFILES" WHERE my_profile_id = %d",
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
	my_profile->id = ctsvc_stmt_get_int(stmt, i++);
	my_profile->addressbook_id = ctsvc_stmt_get_int(stmt, i++);
	my_profile->changed_time = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	my_profile->display_name = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	if (temp) {
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_IMG_FULL_LOCATION, temp);
		my_profile->image_thumbnail_path = strdup(full_path);
	}
	temp = ctsvc_stmt_get_text(stmt, i++);
	my_profile->uid = SAFE_STRDUP(temp);
	cts_stmt_finalize(stmt);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_my_profile_get_data(int id, ctsvc_my_profile_s *my_profile)
{
	int ret, len;
	int datatype;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	len = snprintf(query, sizeof(query),
				"SELECT datatype, id, contact_id, is_default, data1, data2, "
					"data3, data4, data5, data6, data7, data8, data9, data10 "
					"FROM "CTS_TABLE_DATA" WHERE contact_id = %d AND is_my_profile = 1", id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB, "DB error : cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (1 /*CTS_TRUE */!= ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CONTACTS_ERROR_NO_DATA;
	}

	do {
		datatype = ctsvc_stmt_get_int(stmt, 0);
		switch (datatype) {
		case CTSVC_DATA_NAME:
			ctsvc_get_data_info_name(stmt, (contacts_list_h)my_profile->name);
			break;
		case CTSVC_DATA_EVENT:
			ctsvc_get_data_info_event(stmt, (contacts_list_h)my_profile->events);
			break;
		case CTSVC_DATA_MESSENGER:
			ctsvc_get_data_info_messenger(stmt, (contacts_list_h)my_profile->messengers);
			break;
		case CTSVC_DATA_POSTAL:
			ctsvc_get_data_info_address(stmt, (contacts_list_h)my_profile->postal_addrs);
			break;
		case CTSVC_DATA_URL:
			ctsvc_get_data_info_url(stmt, (contacts_list_h)my_profile->urls);
			break;
		case CTSVC_DATA_NICKNAME:
			ctsvc_get_data_info_nickname(stmt, (contacts_list_h)my_profile->nicknames);
			break;
		case CTSVC_DATA_NUMBER:
			ctsvc_get_data_info_number(stmt, (contacts_list_h)my_profile->numbers);
			break;
		case CTSVC_DATA_EMAIL:
			ctsvc_get_data_info_email(stmt, (contacts_list_h)my_profile->emails);
			break;
		case CTSVC_DATA_PROFILE:
			ctsvc_get_data_info_profile(stmt, (contacts_list_h)my_profile->profiles);
			break;
		case CTSVC_DATA_RELATIONSHIP:
			ctsvc_get_data_info_relationship(stmt, (contacts_list_h)my_profile->relationships);
			break;
		case CTSVC_DATA_IMAGE:
			ctsvc_get_data_info_image(stmt, (contacts_list_h)my_profile->images);
			break;
		case CTSVC_DATA_COMPANY:
			ctsvc_get_data_info_company(stmt, (contacts_list_h)my_profile->company);
			break;
		case CTSVC_DATA_NOTE:
			ctsvc_get_data_info_note(stmt, (contacts_list_h)my_profile->note);
			break;
		case CTSVC_DATA_EXTENSION:
			ctsvc_get_data_info_extension(stmt, (contacts_list_h)my_profile->extensions);
			break;
		default:
			CTS_ERR("Intenal : Not supported data type (%d)", datatype);
			break;
		}

	}while(1 /*CTS_TRUE*/ == cts_stmt_step(stmt));

	cts_stmt_finalize(stmt);

	return CONTACTS_ERROR_NONE;

}

static int __ctsvc_db_my_profile_get_record( int id, contacts_record_h* out_record )
{
	int ret;
	contacts_record_h record;
	ctsvc_my_profile_s *my_profile;

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	contacts_record_create(_contacts_my_profile._uri, &record);
	my_profile = (ctsvc_my_profile_s *)record;
	ret = __ctsvc_db_get_my_profile_base_info(id, my_profile);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_get_main_contacts_info(ALL) Failed(%d)", ret);
		contacts_record_destroy(record, true);
		return ret;
	}

	ret = __ctsvc_db_my_profile_get_data(id, my_profile);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_get_data_info Failed(%d)", ret);
		contacts_record_destroy(record, true);
		return ret;
	}

	*out_record = record;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_my_profile_delete_record( int id )
{
	CTS_FN_CALL;
	int ret;
	int addressbook_id;
	char query[CTS_SQL_MAX_LEN] = {0};

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "DB error : ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
		"SELECT addressbook_id FROM "CTSVC_DB_VIEW_MY_PROFILE" WHERE my_profile_id = %d", id);
	ret  = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_get_first_int_result Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "UPDATE "CTS_TABLE_MY_PROFILES" "
					"SET deleted = 1, changed_ver = %d WHERE my_profile_id = %d", ctsvc_get_next_ver(), id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_my_profile_noti();

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

static inline int __ctsvc_my_profile_update_data(ctsvc_my_profile_s *my_profile)
{
	int ret;

	if (my_profile->name) {
		ret = ctsvc_contact_update_data_name((contacts_list_h)my_profile->name, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_name() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->company) {
		ret = ctsvc_contact_update_data_company((contacts_list_h)my_profile->company, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_company() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->note) {
		ret = ctsvc_contact_update_data_note((contacts_list_h)my_profile->note, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_note() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->events) {
		ret = ctsvc_contact_update_data_event((contacts_list_h)my_profile->events, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_events() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->messengers) {
		ret = ctsvc_contact_update_data_messenger((contacts_list_h)my_profile->messengers, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_messengers() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->postal_addrs) {
		ret = ctsvc_contact_update_data_address((contacts_list_h)my_profile->postal_addrs, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_address() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->urls) {
		ret = ctsvc_contact_update_data_url((contacts_list_h)my_profile->urls, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_url() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->nicknames) {
		ret = ctsvc_contact_update_data_nickname((contacts_list_h)my_profile->nicknames, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_nickname() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->numbers) {
		bool had_phonenumber;
		ret = ctsvc_contact_update_data_number((contacts_list_h)my_profile->numbers, my_profile->id, true, &had_phonenumber);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_number() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->emails) {
		bool had_email;
		ret = ctsvc_contact_update_data_email((contacts_list_h)my_profile->emails, my_profile->id, true, &had_email);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_email() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->profiles) {
		ret = ctsvc_contact_update_data_profile((contacts_list_h)my_profile->profiles, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_profile() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->relationships) {
		ret = ctsvc_contact_update_data_relationship((contacts_list_h)my_profile->relationships, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_relationship() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->images) {
		ret = ctsvc_contact_update_data_image((contacts_list_h)my_profile->images, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_image() Failed(%d)", ret);
			return ret;
		}
	}

	if (my_profile->extensions) {
		ret = ctsvc_contact_update_data_extension((contacts_list_h)my_profile->extensions, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_extension() Failed(%d)", ret);
			return ret;
		}
	}

	return CONTACTS_ERROR_NONE;
}

static void __ctsvc_my_profile_check_default_data(ctsvc_my_profile_s *my_profile)
{
	ctsvc_contact_check_default_number((contacts_list_h)my_profile->numbers);
	ctsvc_contact_check_default_email((contacts_list_h)my_profile->emails);
	ctsvc_contact_check_default_image((contacts_list_h)my_profile->images);
}

static void __ctsvc_make_my_profile_display_name(ctsvc_my_profile_s *my_profile)
{
	char *display = NULL;
	GList *cur;
	int len, display_len;

	ctsvc_name_s *name = NULL;

	free(my_profile->display_name);
	my_profile->display_name = NULL;

	free(my_profile->reverse_display_name);
	my_profile->reverse_display_name = NULL;

	if (my_profile->name->count > 0 && my_profile->name->records != NULL && my_profile->name->records->data != NULL) {
		name = (ctsvc_name_s *)my_profile->name->records->data;
	}

	if (name && (name->first || name->last)) {
		// make display name
		display_len = SAFE_STRLEN(name->prefix)
						+ SAFE_STRLEN(name->first)
						+ SAFE_STRLEN(name->addition)
						+ SAFE_STRLEN(name->last)
						+ SAFE_STRLEN(name->suffix) + 5;
		display = calloc(1, display_len);
		len=0;

		if (name->prefix)
			len += snprintf(display + len, display_len - len, "%s", name->prefix);

		if (name->first) {
			if (*display)
				len += snprintf(display + len, display_len - len, " ");
			len += snprintf(display + len, display_len - len, "%s", name->first);
		}

		if (name->addition) {
			if (*display)
				len += snprintf(display + len, display_len - len, " ");
			len += snprintf(display + len, display_len - len, "%s", name->addition);
		}

		if (name->last) {
			if (*display)
				len += snprintf(display + len, display_len - len, " ");
			len += snprintf(display + len, display_len - len, "%s", name->last);
		}

		if (name->suffix) {
			if (*display)
				len += snprintf(display + len, display_len - len, " ");
			len += snprintf(display + len, display_len - len, "%s", name->suffix);
		}

		my_profile->display_name = display;

		// make reverse_display_name
		display = calloc(1, display_len);
		len = 0;

		if (name->prefix)
			len += snprintf(display + len, display_len - len, "%s", name->prefix);

		if (name->last) {
			if (*display)
				len += snprintf(display + len, display_len - len, " ");

			len += snprintf(display + len, display_len - len, "%s", name->last);

			if(name->first || name->addition)
				len += snprintf(display + len, display_len - len, ",");
		}

		if (name->first) {
			if (*display)
				len += snprintf(display + len, display_len - len, " ");
			len += snprintf(display + len, display_len - len, "%s", name->first);
		}

		if (name->addition) {
			if (*display)
				len += snprintf(display + len, display_len - len, " ");
			len += snprintf(display + len, display_len - len, "%s", name->addition);
		}

		if (name->suffix) {
			if (*display)
				len += snprintf(display + len, display_len - len, " ");
			len += snprintf(display + len, display_len - len, "%s", name->suffix);
		}

		my_profile->reverse_display_name = display;
	}
	else {
		if (my_profile->company && my_profile->company->records) {
			for (cur=my_profile->company->records;cur;cur=cur->next) {
				ctsvc_company_s *company = (ctsvc_company_s *)cur->data;
				if (company && company->name) {
					my_profile->display_name_changed = true;
					my_profile->display_name = SAFE_STRDUP(company->name);
					break;
				}
			}
		}

		if (!my_profile->display_name_changed && my_profile->nicknames && my_profile->nicknames->records) {
			for (cur=my_profile->nicknames->records;cur;cur=cur->next) {
				ctsvc_nickname_s *nickname = (ctsvc_nickname_s *)cur->data;
				if (nickname && nickname->nickname) {
					my_profile->display_name_changed = true;
					my_profile->display_name = SAFE_STRDUP(nickname->nickname);
					break;
				}
			}
		}

		if (!my_profile->display_name_changed && my_profile->numbers && my_profile->numbers->records) {
			for (cur=my_profile->numbers->records;cur;cur=cur->next) {
				ctsvc_number_s *number = (ctsvc_number_s *)cur->data;
				if (number && number->number) {
					my_profile->display_name_changed = true;
					my_profile->display_name = SAFE_STRDUP(number->number);
					break;
				}
			}
		}

		if (!my_profile->display_name_changed && my_profile->emails && my_profile->emails->records) {
			for (cur=my_profile->emails->records;cur;cur=cur->next) {
				ctsvc_email_s *email = (ctsvc_email_s *)cur->data;
				if (email && email->email_addr) {
					my_profile->display_name_changed = true;
					my_profile->display_name = SAFE_STRDUP(email->email_addr);
					break;
				}
			}
		}

		if (my_profile->display_name_changed) {
			my_profile->reverse_display_name = SAFE_STRDUP(my_profile->display_name);
		}
	}
	return;
}


static int __ctsvc_db_my_profile_update_record( contacts_record_h record )
{
	int ret, len;
	int id;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_my_profile_s *my_profile = (ctsvc_my_profile_s*)record;
	cts_stmt stmt;

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
		"SELECT my_profile_id FROM "CTSVC_DB_VIEW_MY_PROFILE" WHERE my_profile_id = %d", my_profile->id);
	ret = ctsvc_query_get_first_int_result(query, &id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("The index(%d) is Invalid. %d Record(s) is(are) found", my_profile->id, ret);
		ctsvc_end_trans(false);
		return ret;
	}

	__ctsvc_make_my_profile_display_name(my_profile);
	__ctsvc_my_profile_check_default_data(my_profile);

	//update data
	ret = __ctsvc_my_profile_update_data(my_profile);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_my_profile_update_data() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////
	// this code will be removed.
	free(my_profile->image_thumbnail_path);
	my_profile->image_thumbnail_path = NULL;

	if (my_profile->images) {
		int ret = CONTACTS_ERROR_NONE;
		contacts_record_h record = NULL;
		unsigned int count = 0;
		ctsvc_list_s *list = (ctsvc_list_s*)my_profile->images;
		ctsvc_image_s *image;
		GList *cursor;

		for(cursor=list->deleted_records;cursor;cursor=cursor->next) {
			image = (ctsvc_image_s *)cursor->data;
			my_profile->image_thumbnail_path = NULL;
		}

		contacts_list_get_count((contacts_list_h)my_profile->images, &count);
		if (count) {
			contacts_list_first((contacts_list_h)my_profile->images);
			ret = contacts_list_get_current_record_p((contacts_list_h)my_profile->images, &record);

			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("contacts_list_get_current_record_p() Failed(%d)", ret);
				ctsvc_end_trans(false);
				return CONTACTS_ERROR_DB;
			}
			image = (ctsvc_image_s*)record;

			if ( image->path && *image->path && strstr(image->path, CTS_IMG_FULL_LOCATION) != NULL)
				my_profile->image_thumbnail_path = SAFE_STRDUP( image->path + strlen(CTS_IMG_FULL_LOCATION) + 1);
			else
				my_profile->image_thumbnail_path = SAFE_STRDUP(image->path);

		}
	}
	// this code will be removed.
	//////////////////////////////////////////////////////////////////////

	len = snprintf(query, sizeof(query),
			"UPDATE "CTS_TABLE_MY_PROFILES" SET changed_ver=%d, changed_time=%d, "
				"display_name=?, reverse_display_name=?, uid=?, image_thumbnail_path=?",
				ctsvc_get_next_ver(), (int)time(NULL));
	snprintf(query+len, sizeof(query)-len, " WHERE my_profile_id=%d", my_profile->id);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("cts_query_prepare() Failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	cts_stmt_bind_text(stmt, 1, my_profile->display_name);
	cts_stmt_bind_text(stmt, 2, my_profile->reverse_display_name);
	if (my_profile->uid)
		cts_stmt_bind_text(stmt, 3, my_profile->uid);
	if (my_profile->image_thumbnail_path)
		cts_stmt_bind_text(stmt, 4, my_profile->image_thumbnail_path);

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	ctsvc_set_my_profile_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
	{
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_my_profile_get_all_records( int offset, int limit, contacts_list_h* out_list )
{
	int ret;
	int len;
	int my_profile_id;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;

	len = snprintf(query, sizeof(query),
			"SELECT my_profile_id FROM "CTSVC_DB_VIEW_MY_PROFILE);

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
		my_profile_id = ctsvc_stmt_get_int(stmt, 0);
		ret = contacts_db_get_record(_contacts_my_profile._uri, my_profile_id, &record);
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

static int __ctsvc_db_my_profile_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list )
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_my_profile_s *my_profile;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
	bool had_my_profile_id = false;
	int my_profile_id = 0;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s *)query;

	if (s_query->projection) {
		for (i=0;i<s_query->projection_count;i++) {
			if (s_query->projection[i] == CTSVC_PROPERTY_MY_PROFILE_ID) {
				had_my_profile_id = true;
				break;
			}
		}
	}
	else
		had_my_profile_id = true;

	if (!had_my_profile_id) {
		s_query->projection = realloc(s_query->projection, s_query->projection_count+1);
		s_query->projection[s_query->projection_count] = CTSVC_PROPERTY_MY_PROFILE_ID;
		s_query->projection_count++;
	}

	ret = ctsvc_db_make_get_records_query_stmt(s_query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_db_make_get_records_query_stmt fail(%d)", ret);

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE == ret) {
		CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CONTACTS_ERROR_NO_DATA;
	}

	contacts_list_create(&list);
	do {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		contacts_record_create(_contacts_my_profile._uri, &record);
		my_profile = (ctsvc_my_profile_s*)record;
		if (0 == s_query->projection_count)
			field_count = s_query->property_count;
		else {
			field_count = s_query->projection_count;
			ret = ctsvc_record_set_projection_flags(record, s_query->projection,
					s_query->projection_count, s_query->property_count);

			if (CONTACTS_ERROR_NONE != ret)
				ASSERT_NOT_REACHED("To set projection is failed.\n");
		}

		for(i=0;i<field_count;i++) {
			char *temp;
			int property_id;
			if (0 == s_query->projection_count)
				property_id = s_query->properties[i].property_id;
			else
				property_id = s_query->projection[i];

			switch(property_id) {
			case CTSVC_PROPERTY_MY_PROFILE_ID:
				my_profile_id = ctsvc_stmt_get_int(stmt, i);
				if (had_my_profile_id)
					my_profile->id = my_profile_id;
				break;
			case CTSVC_PROPERTY_MY_PROFILE_DISPLAY_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				my_profile->display_name = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_MY_PROFILE_ADDRESSBOOK_ID:
				my_profile->addressbook_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_MY_PROFILE_IMAGE_THUMBNAIL:
				temp = ctsvc_stmt_get_text(stmt, i);
				if (temp) {
					snprintf(full_path, sizeof(full_path), "%s/%s", CTS_IMG_FULL_LOCATION, temp);
					my_profile->image_thumbnail_path = strdup(full_path);
				}
				break;
			case CTSVC_PROPERTY_MY_PROFILE_CHANGED_TIME:
				my_profile->changed_time = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_MY_PROFILE_UID:
				temp = ctsvc_stmt_get_text(stmt, i);
				my_profile->uid = SAFE_STRDUP(temp);
				break;
			default:
				break;
			}
		}
		ret = __ctsvc_db_my_profile_get_data(my_profile_id, my_profile);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_get_data_info Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		ctsvc_list_prepend(list, record);
	} while ((ret = cts_stmt_step(stmt)));
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;

	return CONTACTS_ERROR_NONE;
}

//static int __ctsvc_db_my_profile_insert_records(const contacts_list_h in_list, int **ids) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_my_profile_update_records(const contacts_list_h in_list) { return CONTACTS_ERROR_NONE; }
//static int __ctsvc_db_my_profile_delete_records(int ids[], int count) { return CONTACTS_ERROR_NONE; }

static int __ctsvc_my_profile_insert_data(ctsvc_my_profile_s *contact)
{
	int ret;

	//Insert the name
	if (contact->name) {
		ret = ctsvc_contact_insert_data_name((contacts_list_h)contact->name, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_name() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the company
	if (contact->company) {
		ret = ctsvc_contact_insert_data_company((contacts_list_h)contact->company, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_company() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the events
	if (contact->events) {
		ret = ctsvc_contact_insert_data_event((contacts_list_h)contact->events, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_event() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the messengers
	if (contact->messengers) {
		ret = ctsvc_contact_insert_data_messenger((contacts_list_h)contact->messengers, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_messenger() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the postals
	if (contact->postal_addrs) {
		ret = ctsvc_contact_insert_data_address((contacts_list_h)contact->postal_addrs, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_postal() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the Web addrs
	if (contact->urls) {
		ret = ctsvc_contact_insert_data_url((contacts_list_h)contact->urls, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_web() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the Nick names
	if (contact->nicknames) {
		ret = ctsvc_contact_insert_data_nickname((contacts_list_h)contact->nicknames, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_nickname() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the numbers
	if (contact->numbers) {
		ret = ctsvc_contact_insert_data_number((contacts_list_h)contact->numbers, contact->id, true);
		if (ret < CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_contact_insert_data_number() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the emails
	if (contact->emails) {
		ret = ctsvc_contact_insert_data_email((contacts_list_h)contact->emails, contact->id, true);
		if (ret < CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_insert_my_profile_data_email() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the profile values
	if (contact->profiles) {
		ret = ctsvc_contact_insert_data_profile((contacts_list_h)contact->profiles, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_profile() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the relationship values
	if (contact->relationships) {
		ret = ctsvc_contact_insert_data_relationship((contacts_list_h)contact->relationships, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_relationship() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the image values
	if (contact->images) {
		ret = ctsvc_contact_insert_data_image((contacts_list_h)contact->images, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_image() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the note values
	if (contact->note) {
		ret = ctsvc_contact_insert_data_note((contacts_list_h)contact->note, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_note() Failed(%d)", ret);
			return ret;
		}
	}

	//Insert the extensions values
	if (contact->extensions) {
		ret = ctsvc_contact_insert_data_extension((contacts_list_h)contact->extensions, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_extension() Failed(%d)", ret);
			return ret;
		}
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_my_profile_insert_record( contacts_record_h record, int *id)
{
	CTS_FN_CALL;
	int ret;
	int version;
	char query[CTS_SQL_MAX_LEN] = {0};

	ctsvc_my_profile_s *my_profile = (ctsvc_my_profile_s*)record;
	cts_stmt stmt;

	// These check should be done in client side
	RETVM_IF(NULL == my_profile, CONTACTS_ERROR_INVALID_PARAMETER,
					"Invalid parameter : my_profile is NULL");
	RETVM_IF(my_profile->addressbook_id < 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : addressbook_id(%d) is mandatory field to insert my_profile record ", my_profile->addressbook_id);
	RETVM_IF(0 < my_profile->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : id(%d), This record is already inserted", my_profile->id);

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_MY_PROFILES" WHERE addressbook_id = %d AND deleted = 1", my_profile->addressbook_id);
	ret = ctsvc_query_exec(query);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "Delete deleted my_profile of addressbook(%d) failed", my_profile->addressbook_id);

	ret = cts_db_get_next_id(CTS_TABLE_MY_PROFILES);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("cts_db_get_next_id() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}
	my_profile->id = ret;
	if (id)
		*id = ret;

	__ctsvc_make_my_profile_display_name(my_profile);
	__ctsvc_my_profile_check_default_data(my_profile);

	//Insert Data
	ret = __ctsvc_my_profile_insert_data(my_profile);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_insert_my_profile_data() Failed(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////
	// this code will be removed.
	free(my_profile->image_thumbnail_path);
	my_profile->image_thumbnail_path = NULL;

	if (my_profile->images) {
		ctsvc_image_s *image;
		unsigned int count = 0;

		contacts_list_get_count((contacts_list_h)my_profile->images, &count);

		while (count) {
			contacts_list_first((contacts_list_h)my_profile->images);
			ret = contacts_list_get_current_record_p((contacts_list_h)my_profile->images, (contacts_record_h*)&image);
			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("contacts_list_get_current_record_p() Failed(%d)", ret);
				ctsvc_end_trans(false);
				return CONTACTS_ERROR_DB;
			}

			if (image->path && image->is_default) {
				my_profile->image_thumbnail_path = strdup(image->path);
				break;
			}
			count--;
		}
	}
	// this code will be removed.
	//////////////////////////////////////////////////////////////////////

	version = ctsvc_get_next_ver();

	snprintf(query, sizeof(query),
		"INSERT INTO "CTS_TABLE_MY_PROFILES"(my_profile_id, addressbook_id, "
			"created_ver, changed_ver, changed_time, "
			"display_name, reverse_display_name, uid, image_thumbnail_path) "
			"VALUES(%d, %d, %d, %d, %d, ?, ?, ?, ?)",
			my_profile->id, my_profile->addressbook_id, version, version, (int)time(NULL));

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("cts_query_prepare() Failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	if (my_profile->display_name)
		cts_stmt_bind_text(stmt, 1, my_profile->display_name);
	if (my_profile->reverse_display_name)
		cts_stmt_bind_text(stmt, 2, my_profile->reverse_display_name);
	if (my_profile->uid)
		cts_stmt_bind_text(stmt, 3, my_profile->uid);
	if (my_profile->image_thumbnail_path)
		cts_stmt_bind_text(stmt, 4, my_profile->image_thumbnail_path);

	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	ctsvc_set_my_profile_noti();

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_svc_end_trans() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}


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
#include <vconf.h>
#include <vconf-keys.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_schema.h"
#include "ctsvc_db_init.h"
#include "ctsvc_utils.h"
#include "ctsvc_record.h"
#include "ctsvc_normalize.h"
#include "ctsvc_setting.h"
#include "ctsvc_localize.h"
#include "ctsvc_localize_ch.h"
#include "ctsvc_notification.h"
#include "ctsvc_localize.h"

#include "ctsvc_db_plugin_contact_helper.h"
#include "ctsvc_db_plugin_company_helper.h"
#include "ctsvc_db_plugin_name_helper.h"
#include "ctsvc_db_plugin_number_helper.h"
#include "ctsvc_db_plugin_email_helper.h"
#include "ctsvc_db_plugin_event_helper.h"
#include "ctsvc_db_plugin_url_helper.h"
#include "ctsvc_db_plugin_note_helper.h"
#include "ctsvc_db_plugin_profile_helper.h"
#include "ctsvc_db_plugin_address_helper.h"
#include "ctsvc_db_plugin_nickname_helper.h"
#include "ctsvc_db_plugin_messenger_helper.h"
#include "ctsvc_db_plugin_relationship_helper.h"
#include "ctsvc_db_plugin_image_helper.h"
#include "ctsvc_db_plugin_extension_helper.h"

//#include "ctsvc_db_plugin_grouprelation_helper.h"

#include "ctsvc_group.h"

#define CTSVC_MY_IMAGE_LOCATION "/opt/usr/data/contacts-svc/img/my"

int ctsvc_contact_add_image_file(ctsvc_img_e image_type, int parent_id, int img_id,
		char *src_img, char *dest_name, int dest_size)
{
	int ret;
	char *ext;
	char dest[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	RETVM_IF(NULL == src_img, CONTACTS_ERROR_INVALID_PARAMETER, "image_thumbnail_path is NULL");

	ext = strrchr(src_img, '.');
	if (NULL == ext || strchr(ext, '/'))
		ext = "";

	snprintf(dest, sizeof(dest), "%s/%d_%d-%d%s", CTS_IMG_FULL_LOCATION, parent_id, img_id, image_type, ext);

	ret = ctsvc_copy_image(src_img, dest);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "cts_copy_file() Failed(%d)", ret);

	snprintf(dest_name, dest_size, "%d_%d-%d%s", parent_id, img_id, image_type, ext);
	return CONTACTS_ERROR_NONE;
}

static inline const char* __ctsvc_get_image_column_name(ctsvc_img_e image_type)
{
	switch(image_type)
	{
	case CTSVC_IMG_NORMAL:
		return "image_thumbnail_path";
	default:
		CTS_ERR("Invalid parameter : The image_type(%d) is not supported", image_type);
		return NULL;
	}
}

static int __ctsvc_contact_delete_image_file(ctsvc_img_e image_type, int image_id)
{
	int ret;
	cts_stmt stmt;
	char *tmp_path;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query), "SELECT data3 FROM %s WHERE id = %d", CTS_TABLE_DATA, image_id);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("DB error: cts_query_prepare() Failed");
		return CONTACTS_ERROR_DB;
	}

	ret = cts_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		CTS_ERR("DB error: cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CONTACTS_ERROR_NO_DATA;
	}

	tmp_path = ctsvc_stmt_get_text(stmt, 0);
	if (tmp_path) {
		char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_IMG_FULL_LOCATION, tmp_path);
		ret = unlink(full_path);
		WARN_IF (ret < 0, "unlink(%s) Failed(%d)", full_path, errno);
	}
	cts_stmt_finalize(stmt);
	return CONTACTS_ERROR_NONE;
}

int ctsvc_contact_update_image_file(int image_type, int parent_id, int img_id,
		char *src_img, char *dest_name, int dest_size)
{
	int ret;
	if (src_img && strstr(src_img, CTS_IMG_FULL_LOCATION) != NULL) {
		snprintf(dest_name, dest_size, "%s", src_img + strlen(CTS_IMG_FULL_LOCATION) + 1);
		return CONTACTS_ERROR_NONE;
	}

	ret = __ctsvc_contact_delete_image_file(image_type, img_id);
	WARN_IF(CONTACTS_ERROR_NONE != ret && CONTACTS_ERROR_NO_DATA != ret,
			"__ctsvc_contact_delete_image_file() Failed(%d)", ret);

	if (src_img) {
		ret = ctsvc_contact_add_image_file(image_type, parent_id, img_id, src_img, dest_name, dest_size);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_contact_add_image_file() Failed(%d)", ret);
	}

	return ret;
}

int ctsvc_db_contact_update_changed_time(int contact_id)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	snprintf(query, sizeof(query),
			"UPDATE %s SET changed_ver=%d, changed_time=%d WHERE contact_id=%d AND deleted = 0",
			CTS_TABLE_CONTACTS, ctsvc_get_next_ver(), (int)time(NULL), contact_id);

	ret = ctsvc_query_exec(query);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "cts_query_exec() Failed(%d)", ret);

	ctsvc_set_contact_noti();

	return CONTACTS_ERROR_NONE;
}

int ctsvc_contact_delete_image_file_with_path(const unsigned char* image_path)
{
	int ret;

	if (image_path) {
		char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_IMG_FULL_LOCATION, image_path);
		ret = unlink(full_path);
		WARN_IF(ret < 0, "unlink(%s) Failed(%d)", full_path, errno);
	}

	return CONTACTS_ERROR_NONE;
}

void ctsvc_make_contact_display_name(ctsvc_contact_s *contact)
{
	char *display = NULL;
	char *sortkey = NULL;
	char *phonetic = NULL;
	GList *cur;
	int ret, len, display_len, temp_len = 0;

	ctsvc_name_s *name = NULL;

	contact->display_name_language = CTSVC_SORT_OTHERS;

	if ( contact->name->count > 0 && contact->name->records != NULL && contact->name->records->data != NULL )
	{
		name = (ctsvc_name_s *)contact->name->records->data;
	}

	if ( name && ( name->first || name->last) ) {

		// make display name
		display_len = SAFE_STRLEN(name->prefix)
						+ SAFE_STRLEN(name->first)
						+ SAFE_STRLEN(name->addition)
						+ SAFE_STRLEN(name->last)
						+ SAFE_STRLEN(name->suffix) + 5;
		display = calloc(1, display_len);
		len=0;

		if(name->prefix)
			len += snprintf(display + len, display_len - len, "%s", name->prefix);

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

		// make reverse_display_name
		display = calloc(1, display_len);
		len = 0;

		if(name->prefix)
			len += snprintf(display + len, display_len - len, "%s", name->prefix);

		if(name->last) {
			if (*display)
				len += snprintf(display + len, display_len - len, " ");

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
				len += snprintf(display + len, display_len - len, " ");
			len += snprintf(display + len, display_len - len, "%s", name->suffix);
		}

		contact->display_name_changed = true;
		contact->reverse_display_name = display;
		contact->sort_name = SAFE_STRDUP(contact->display_name);
		contact->reverse_sort_name = SAFE_STRDUP(contact->reverse_display_name);
		contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NAME;
	}
	else {
		if (contact->company && contact->company->records) {
			for (cur=contact->company->records;cur;cur=cur->next) {
				ctsvc_company_s *company = (ctsvc_company_s *)cur->data;
				if (company && company->name) {
					contact->display_name_changed = true;
					contact->display_name = SAFE_STRDUP(company->name);
					contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_COMPANY;
					break;
				}
			}
		}

		if (!contact->display_name_changed && contact->nicknames && contact->nicknames->records) {
			for (cur=contact->nicknames->records;cur;cur=cur->next) {
				ctsvc_nickname_s *nickname = (ctsvc_nickname_s *)cur->data;
				if (nickname && nickname->nickname) {
					contact->display_name_changed = true;
					contact->display_name = SAFE_STRDUP(nickname->nickname);
					contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NICKNAME;
					break;
				}
			}
		}

		if (!contact->display_name_changed && contact->numbers && contact->numbers->records) {
			for (cur=contact->numbers->records;cur;cur=cur->next) {
				ctsvc_number_s *number = (ctsvc_number_s *)cur->data;
				if (number && number->number) {
					contact->display_name_changed = true;
					contact->display_name = SAFE_STRDUP(number->number);
					contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NUMBER;
					break;
				}
			}
		}

		if (!contact->display_name_changed && contact->emails && contact->emails->records) {
			for (cur=contact->emails->records;cur;cur=cur->next) {
				ctsvc_email_s *email = (ctsvc_email_s *)cur->data;
				if (email && email->email_addr) {
					contact->display_name_changed = true;
					contact->display_name = SAFE_STRDUP(email->email_addr);
					contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_EMAIL;
					break;
				}
			}
		}

		if (contact->display_name_changed) {
			contact->reverse_display_name = SAFE_STRDUP(contact->display_name);
			contact->sort_name = SAFE_STRDUP(contact->display_name);
			contact->reverse_sort_name = SAFE_STRDUP(contact->display_name);
		}
	}

	if (contact->display_name_changed) {
		ret = ctsvc_get_name_sort_type(contact->display_name);
		WARN_IF( ret < 0, "ctsvc_check_language_type Failed(%d)", ret);

		switch (ret)
		{
		case CTSVC_SORT_CJK:
			if (contact->display_source_type == CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NAME) {
				len = SAFE_STRLEN(name->phonetic_first) + SAFE_STRLEN(name->phonetic_last) + SAFE_STRLEN(name->phonetic_middle);
				if (len > 0) {
					len += 3; // for space and null string
					phonetic = calloc(1, len);
					if (name->phonetic_first)
						temp_len += snprintf(phonetic, len, "%s", name->phonetic_first);
					if (name->phonetic_middle) {
						if (temp_len)
							temp_len += snprintf(phonetic + temp_len, len - temp_len, " ");
						temp_len += snprintf(phonetic + temp_len, len - temp_len, "%s", name->phonetic_middle);
					}
					if (name->phonetic_last) {
						if (temp_len)
							temp_len += snprintf(phonetic + temp_len, len - temp_len, " ");
						temp_len += snprintf(phonetic + temp_len, len - temp_len, "%s", name->phonetic_last);
					}
				}
			}

			if (phonetic && ctsvc_get_name_sort_type(phonetic) == CTSVC_SORT_JAPANESE) {
				ret = CTSVC_SORT_JAPANESE;
				FREEandSTRDUP(contact->sort_name, phonetic);
				FREEandSTRDUP(contact->reverse_sort_name, phonetic);
			}
			else {
				{
					pinyin_name_s *pinyinname = NULL;
					int size;

					if (ctsvc_convert_chinese_to_pinyin(contact->display_name, &pinyinname, &size) == CONTACTS_ERROR_NONE) {
						FREEandSTRDUP(contact->sort_name, pinyinname[0].pinyin_name);
						FREEandSTRDUP(contact->reverse_sort_name, pinyinname[0].pinyin_name);
						free(pinyinname);
					}
					ret = CTSVC_SORT_WESTERN;
				}
			}
			free(phonetic);
			break;
		case CTSVC_SORT_JAPANESE:
			{
				ctsvc_convert_japanese_to_hiragana(contact->display_name, &contact->sort_name);
				ctsvc_convert_japanese_to_hiragana(contact->reverse_display_name, &contact->reverse_sort_name);
				break;
			}
		}

		if (ctsvc_get_default_language() == ret)
			contact->display_name_language = CTSVC_SORT_PRIMARY;
		else if (ctsvc_get_secondary_language() == ret)
			contact->display_name_language = CTSVC_SORT_SECONDARY;
		else
			contact->display_name_language = ret;

		ret = ctsvc_collation_str(contact->sort_name, &sortkey);
		if (CONTACTS_ERROR_NONE == ret)
			contact->sortkey = sortkey;
		else
			free(sortkey);
		sortkey = NULL;

		ret = ctsvc_collation_str(contact->reverse_sort_name, &sortkey);
		if (CONTACTS_ERROR_NONE == ret)
			contact->reverse_sortkey = sortkey;
		else
			free(sortkey);
	}

	return;
}

int ctsvc_get_data_info_name(cts_stmt stmt, contacts_list_h name_list)
{
	int ret;
	unsigned int count;
	contacts_record_h record;

	ret = contacts_list_get_count(name_list, &count);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_list_get_count is failed(%d)", ret);
	RETVM_IF (1 < count, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : already had name");

	ctsvc_db_name_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(name_list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_event(cts_stmt stmt, contacts_list_h list)
{
	contacts_record_h record;

	ctsvc_db_event_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_number(cts_stmt stmt, contacts_list_h number_list)
{
	contacts_record_h record;

	ctsvc_db_number_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(number_list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_email(cts_stmt stmt, contacts_list_h list)
{
	contacts_record_h record;

	ctsvc_db_email_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_address(cts_stmt stmt, contacts_list_h list)
{
	contacts_record_h record;

	ctsvc_db_address_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_messenger(cts_stmt stmt, contacts_list_h list)
{
	contacts_record_h record;

	ctsvc_db_messenger_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_note(cts_stmt stmt, contacts_list_h list)
{
	contacts_record_h record;

	ctsvc_db_note_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_company(cts_stmt stmt, contacts_list_h list)
{
	contacts_record_h record;

	ctsvc_db_company_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_profile(cts_stmt stmt, contacts_list_h list)
{
	contacts_record_h record;

	ctsvc_db_profile_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_relationship(cts_stmt stmt, contacts_list_h list)
{
	contacts_record_h record;

	ctsvc_db_relationship_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_image(cts_stmt stmt, contacts_list_h list)
{
	contacts_record_h record;

	ctsvc_db_image_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_url(cts_stmt stmt, contacts_list_h list)
{
	contacts_record_h record;

	ctsvc_db_url_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_nickname(cts_stmt stmt, contacts_list_h list)
{
	contacts_record_h record;

	ctsvc_db_nickname_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(list, record);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_data_info_extension(cts_stmt stmt, contacts_list_h list)
{
	contacts_record_h record;

	ctsvc_db_extension_get_value_from_stmt(stmt, &record, 1);
	contacts_list_add(list, record);

	return CONTACTS_ERROR_NONE;
}

bool ctsvc_contact_check_default_number(contacts_list_h number_list)
{
	bool has_default = false;
	ctsvc_number_s* number;
	unsigned int count;
	int ret;

	RETV_IF(NULL == number_list, false);

	ret = contacts_list_get_count(number_list, &count);
	if(CONTACTS_ERROR_NONE !=ret || 0 == count)
		return false;

	contacts_list_first(number_list);
	do {
		contacts_list_get_current_record_p(number_list, (contacts_record_h*)&number);
		if (NULL != number && number->number && *number->number) {
			if (number->is_default && false == has_default)
				has_default = true;
			else if (has_default)
				number->is_default = false;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(number_list));

	if (false == has_default) {
		contacts_list_first(number_list);
		do {
			contacts_list_get_current_record_p(number_list, (contacts_record_h*)&number);
			if (NULL != number && number->number && *number->number) {
				number->is_default = true;
				has_default = true;
				break;
			}
		}while(CONTACTS_ERROR_NONE == contacts_list_next(number_list));
	}
	return has_default;
}

bool ctsvc_contact_check_default_email(contacts_list_h email_list)
{
	bool has_default = false;
	ctsvc_email_s* email;
	unsigned int count;
	int ret;

	RETV_IF(NULL == email_list, false);

	ret = contacts_list_get_count(email_list, &count);
	if(CONTACTS_ERROR_NONE !=ret || 0 == count)
		return false;

	contacts_list_first(email_list);
	do {
		contacts_list_get_current_record_p(email_list, (contacts_record_h*)&email);
		if (NULL != email && email->email_addr && *email->email_addr) {
			if (email->is_default && false == has_default)
				has_default = true;
			else if (has_default)
				email->is_default = false;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(email_list));

	if (false == has_default) {
		contacts_list_first(email_list);
		do {
			contacts_list_get_current_record_p(email_list, (contacts_record_h*)&email);
			if (NULL != email && email->email_addr && *email->email_addr) {
				email->is_default = true;
				has_default = true;
				break;
			}
		}while(CONTACTS_ERROR_NONE == contacts_list_next(email_list));
	}
	return has_default;
}

bool ctsvc_contact_check_default_image(contacts_list_h image_list)
{
	bool has_default = false;
	ctsvc_image_s* image;
	unsigned int count;
	int ret;

	RETV_IF(NULL == image_list, false);

	ret = contacts_list_get_count(image_list, &count);
	if (CONTACTS_ERROR_NONE !=ret || 0 == count) {
		CTS_DBG("list get count failed (%d)", count);
		return false;
	}

	contacts_list_first(image_list);
	do {
		contacts_list_get_current_record_p(image_list, (contacts_record_h*)&image);
		if (NULL != image && image->path && *image->path) {
			if (image->is_default && false == has_default)
				has_default = true;
			else if (has_default)
				image->is_default = false;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(image_list));

	if (false == has_default) {
		contacts_list_first(image_list);
		do {
			contacts_list_get_current_record_p(image_list, (contacts_record_h*)&image);
			if (NULL != image && image->path && *image->path) {
				image->is_default = true;
				has_default = true;
				break;
			}
		}while(CONTACTS_ERROR_NONE == contacts_list_next(image_list));
	}
	return CONTACTS_ERROR_NONE;
}

int ctsvc_contact_update_data_name(contacts_list_h name_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	ctsvc_name_s *name;
	ctsvc_list_s *list = (ctsvc_list_s*)name_list;
	GList *cursor;
	RETV_IF(NULL == name_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		name = (ctsvc_name_s *)cursor->data;
		ctsvc_db_name_delete(name->id, is_my_profile);
	}

	contacts_list_first(name_list);
	contacts_list_get_current_record_p(name_list, &record);
	if (record) {
		name = (ctsvc_name_s*)record;
		if (0 < name->id) {
			if (name->first || name->last || name->addition || name->prefix || name->suffix
					|| name->phonetic_first || name->phonetic_middle || name->phonetic_last)
				ret = ctsvc_db_name_update(record, is_my_profile);
			else
				ret = ctsvc_db_name_delete(name->id, is_my_profile);
		}
		else
			ret = ctsvc_db_name_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret)
			CTS_ERR("DB error : return(%d)", ret);
	}

	return ret;
}

int ctsvc_contact_update_data_company(contacts_list_h company_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)company_list;
	ctsvc_company_s *company;
	GList *cursor;

	RETV_IF(NULL == company_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		company = (ctsvc_company_s *)cursor->data;
		ctsvc_db_company_delete(company->id, is_my_profile);
	}

	ret = contacts_list_get_count(company_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(company_list);
	do {
		contacts_list_get_current_record_p(company_list, &record);
		company = (ctsvc_company_s*)record;
		if (0 < company->id) {
			if (company->name || company->department || company->job_title || company->role
				|| company->assistant_name || company->logo || company->location || company->description
				|| company->phonetic_name)
				ret = ctsvc_db_company_update(record, contact_id, is_my_profile);
			else
				ret = ctsvc_db_company_delete(company->id, is_my_profile);
		}
		else
			ret = ctsvc_db_company_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : return (%d)", ret);
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(company_list));

	return ret;
}

int ctsvc_contact_update_data_note(contacts_list_h note_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)note_list;
	ctsvc_note_s *note;
	GList *cursor;

	RETV_IF(NULL == note_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		note = (ctsvc_note_s *)cursor->data;
		ctsvc_db_note_delete(note->id, is_my_profile);
	}

	ret = contacts_list_get_count(note_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(note_list);
	do {
		contacts_list_get_current_record_p(note_list, &record);
		note = (ctsvc_note_s*)record;
		if (0 < note->id) {
			if (note->note)
				ret = ctsvc_db_note_update(record, is_my_profile);
			else
				ret = ctsvc_db_note_delete(note->id, is_my_profile);
		}
		else
			ret = ctsvc_db_note_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : return (%d)", ret);
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(note_list));

	return ret;
}

int ctsvc_contact_update_data_event(contacts_list_h event_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)event_list;
	ctsvc_event_s *event;
	GList *cursor;

	RETV_IF(NULL == event_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		event = (ctsvc_event_s *)cursor->data;
		ctsvc_db_event_delete(event->id, is_my_profile);
	}

	ret = contacts_list_get_count(event_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(event_list);
	do {
		contacts_list_get_current_record_p(event_list, &record);
		event = (ctsvc_event_s*)record;
		if (0 < event->id) {
			if (event->date > 0)
				ret = ctsvc_db_event_update(record, is_my_profile);
			else
				ret = ctsvc_db_event_delete(event->id, is_my_profile);
		}
		else
			ret = ctsvc_db_event_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : return (%d)", ret);
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(event_list));

	return ret;
}

int ctsvc_contact_update_data_messenger(contacts_list_h messenger_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)messenger_list;
	ctsvc_messenger_s *messenger;
	GList *cursor;

	RETV_IF(NULL == messenger_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		messenger = (ctsvc_messenger_s *)cursor->data;
		ctsvc_db_messenger_delete(messenger->id, is_my_profile);
	}

	ret = contacts_list_get_count(messenger_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(messenger_list);
	do {
		contacts_list_get_current_record_p(messenger_list, &record);
		messenger = (ctsvc_messenger_s*)record;
		if (0 < messenger->id) {
			if (messenger->im_id)
				ret = ctsvc_db_messenger_update(record, is_my_profile);
			else
				ret = ctsvc_db_messenger_delete(messenger->id, is_my_profile);
		}
		else
			ret = ctsvc_db_messenger_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : return (%d)", ret);
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(messenger_list));

	return ret;
}

int ctsvc_contact_update_data_address(contacts_list_h address_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)address_list;
	ctsvc_address_s *address;
	GList *cursor;

	RETV_IF(NULL == address_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		address = (ctsvc_address_s *)cursor->data;
		ctsvc_db_address_delete(address->id, is_my_profile);
	}

	ret = contacts_list_get_count(address_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(address_list);
	do {
		contacts_list_get_current_record_p(address_list, &record);
		address = (ctsvc_address_s*)record;
		if (0 < address->id) {
			if (address->pobox || address->postalcode || address->region || address->locality
				|| address->street || address->extended || address->country)
				ret = ctsvc_db_address_update(record, is_my_profile);
			else
				ret = ctsvc_db_address_delete(address->id, is_my_profile);
		}
		else
			ret = ctsvc_db_address_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : return (%d)", ret);
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(address_list));

	return ret;
}

int ctsvc_contact_update_data_url(contacts_list_h url_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)url_list;
	ctsvc_url_s *url;
	GList *cursor;

	RETV_IF(NULL == url_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		url = (ctsvc_url_s *)cursor->data;
		ctsvc_db_url_delete(url->id, is_my_profile);
	}

	ret = contacts_list_get_count(url_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(url_list);
	do {
		contacts_list_get_current_record_p(url_list, &record);
		url = (ctsvc_url_s*)record;
		if (0 < url->id) {
			if (url->url)
				ret = ctsvc_db_url_update(record, is_my_profile);
			else
				ret = ctsvc_db_url_delete(url->id, is_my_profile);
		}
		else
			ret = ctsvc_db_url_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : return (%d)", ret);
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(url_list));

	return ret;
}

int ctsvc_contact_update_data_profile(contacts_list_h profile_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)profile_list;
	ctsvc_profile_s *profile;
	GList *cursor;

	RETV_IF(NULL == profile_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		profile = (ctsvc_profile_s *)cursor->data;
		ctsvc_db_profile_delete(profile->id, is_my_profile);
	}
	ret = contacts_list_get_count(profile_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(profile_list);
	do {
		contacts_list_get_current_record_p(profile_list, &record);
		profile = (ctsvc_profile_s*)record;
		if (0 < profile->id) {
			if (profile->appsvc_operation)
				ret = ctsvc_db_profile_update(record, is_my_profile);
			else
				ret = ctsvc_db_profile_delete(profile->id, is_my_profile);
		}
		else
			ret = ctsvc_db_profile_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : return (%d)", ret);
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(profile_list));

	return ret;
}

int ctsvc_contact_update_data_relationship(contacts_list_h relationship_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)relationship_list;
	ctsvc_relationship_s *relationship;
	GList *cursor;

	RETV_IF(NULL == relationship_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		relationship = (ctsvc_relationship_s *)cursor->data;
		ctsvc_db_relationship_delete(relationship->id, is_my_profile);
	}

	ret = contacts_list_get_count(relationship_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(relationship_list);
	do {
		contacts_list_get_current_record_p(relationship_list, &record);
		relationship = (ctsvc_relationship_s*)record;
		if (0 < relationship->id) {
			if (relationship->name)
				ret = ctsvc_db_relationship_update(record, is_my_profile);
			else
				ret = ctsvc_db_relationship_delete(relationship->id, is_my_profile);
		}
		else
			ret = ctsvc_db_relationship_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : return (%d)", ret);
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(relationship_list));

	return ret;
}

int ctsvc_contact_update_data_image(contacts_list_h image_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)image_list;
	ctsvc_image_s *image;
	GList *cursor;

	RETV_IF(NULL == image_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		image = (ctsvc_image_s *)cursor->data;
		ctsvc_db_image_delete(image->id, is_my_profile);
	}

	ret = contacts_list_get_count(image_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(image_list);
	do {
		contacts_list_get_current_record_p(image_list, &record);
		image = (ctsvc_image_s*)record;
		if (image->is_changed) {
			if (0 < image->id) {
				if (image->path)
					ret = ctsvc_db_image_update(record, contact_id, is_my_profile);
				else
					ret = ctsvc_db_image_delete(image->id, is_my_profile);
			}
			else
				ret = ctsvc_db_image_insert(record, contact_id, is_my_profile, NULL);
		}
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : return (%d)", ret);
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(image_list));

	return ret;
}

int ctsvc_contact_update_data_nickname(contacts_list_h nickname_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)nickname_list;
	ctsvc_nickname_s *nickname;
	GList *cursor;

	RETV_IF(NULL == nickname_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		nickname = (ctsvc_nickname_s *)cursor->data;
		ctsvc_db_nickname_delete(nickname->id, is_my_profile);
	}

	ret = contacts_list_get_count(nickname_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(nickname_list);
	do {
		contacts_list_get_current_record_p(nickname_list, &record);
		nickname = (ctsvc_nickname_s*)record;
		if (0 < nickname->id) {
			if (nickname->nickname)
				ret = ctsvc_db_nickname_update(record, is_my_profile);
			else
				ret = ctsvc_db_nickname_delete(nickname->id, is_my_profile);
		}
		else
			ret = ctsvc_db_nickname_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : return (%d)", ret);
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(nickname_list));

	return ret;
}

int ctsvc_contact_update_data_extension(contacts_list_h extension_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)extension_list;
	ctsvc_extension_s *extension;
	GList *cursor;

	RETV_IF(NULL == extension_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		extension = (ctsvc_extension_s *)cursor->data;
		ctsvc_db_extension_delete(extension->id, is_my_profile);
	}

	ret = contacts_list_get_count(extension_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(extension_list);
	do {
		contacts_list_get_current_record_p(extension_list, &record);
		extension = (ctsvc_extension_s*)record;
		if (0 < extension->id) {
			if (extension->data2 || extension->data3 || extension->data4 || extension->data5
				|| extension->data6 || extension->data7 || extension->data8 || extension->data9
				|| extension->data10 || extension->data11 || extension->data12)
				ret = ctsvc_db_extension_update(record);
			else
				ret = ctsvc_db_extension_delete(extension->id, is_my_profile);
		}
		else
			ret = ctsvc_db_extension_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : return (%d)", ret);
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(extension_list));

	return ret;
}

int ctsvc_contact_update_data_number(contacts_list_h number_list,
	int contact_id, bool is_my_profile, bool *had_phonenumber)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)number_list;
	ctsvc_number_s *number;
	GList *cursor;
	bool had = false;
	*had_phonenumber = false;

	RETV_IF(NULL == number_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		number = (ctsvc_number_s *)cursor->data;
		ctsvc_db_number_delete(number->id, is_my_profile);
	}

	ret = contacts_list_get_count(number_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(number_list);
	do {
		contacts_list_get_current_record_p(number_list, &record);
		number = (ctsvc_number_s*)record;
		if (number->number) {
			if (0 < number->id) {
				if (number->number)
					ret = ctsvc_db_number_update(record, is_my_profile);
				else
					ret = ctsvc_db_number_delete(number->id, is_my_profile);
			}
			else
				ret = ctsvc_db_number_insert(record, contact_id, is_my_profile, NULL);
			if (CONTACTS_ERROR_DB == ret){
				CTS_ERR("DB error : return (%d)", ret);
				break;
			}
			had = true;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(number_list));

	*had_phonenumber = had;
	return ret;
}

int ctsvc_contact_update_data_email(contacts_list_h email_list,
	int contact_id, bool is_my_profile, bool *had_email)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;
	ctsvc_list_s *list = (ctsvc_list_s*)email_list;
	ctsvc_email_s *email;
	GList *cursor;
	bool had = false;
	*had_email = false;

	RETV_IF(NULL == email_list, CONTACTS_ERROR_INVALID_PARAMETER);

	for(cursor = list->deleted_records;cursor;cursor=cursor->next) {
		email = (ctsvc_email_s *)cursor->data;
		ctsvc_db_email_delete(email->id, is_my_profile);
	}

	ret = contacts_list_get_count(email_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(email_list);
	do {
		contacts_list_get_current_record_p(email_list, &record);
		email = (ctsvc_email_s*)record;
		if (email->email_addr) {
			if (0 < email->id) {
				if (email->email_addr)
					ret = ctsvc_db_email_update(record, is_my_profile);
				else
					ret = ctsvc_db_email_delete(email->id, is_my_profile);
			}
			else
				ret = ctsvc_db_email_insert(record, contact_id, is_my_profile, NULL);
			if (CONTACTS_ERROR_DB == ret){
				CTS_ERR("DB error : return (%d)", ret);
				break;
			}
			had = true;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(email_list));

	*had_email = had;
	return ret;
}

int ctsvc_contact_insert_data_name(contacts_list_h name_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	RETV_IF(NULL == name_list, CONTACTS_ERROR_INVALID_PARAMETER);

	contacts_list_first(name_list);
	contacts_list_get_current_record_p(name_list, &record);
	if (record) {
		ret = ctsvc_db_name_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_name_insert");
		}
	}
	return ret;
}

int ctsvc_contact_insert_data_number(contacts_list_h number_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	unsigned int count = 0;

	RETV_IF(NULL == number_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(number_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(number_list);
	do {
		contacts_list_get_current_record_p(number_list, &record);
		ret = ctsvc_db_number_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_number_insert");
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(number_list));

	return ret;
}

int ctsvc_contact_insert_data_email(contacts_list_h email_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	unsigned int count = 0;

	RETV_IF(NULL == email_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(email_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(email_list);
	do {
		contacts_list_get_current_record_p(email_list, &record);
		ret = ctsvc_db_email_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_email_insert");
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(email_list));

	return ret;
}

int ctsvc_contact_insert_data_profile(contacts_list_h profile_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	unsigned int count = 0;

	RETV_IF(NULL == profile_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(profile_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(profile_list);
	do {
		contacts_list_get_current_record_p(profile_list, &record);
		ret = ctsvc_db_profile_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_profile_insert");
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(profile_list));

	return ret;
}

int ctsvc_contact_insert_data_company(contacts_list_h company_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;

	RETV_IF(NULL == company_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(company_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(company_list);
	do {
		contacts_list_get_current_record_p(company_list, &record);
		ret = ctsvc_db_company_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_company_insert");
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(company_list));

	return ret;
}

int ctsvc_contact_insert_data_note(contacts_list_h note_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	unsigned int count = 0;

	RETV_IF(NULL == note_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(note_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(note_list);
	do {
		contacts_list_get_current_record_p(note_list, &record);
		if (record) {
			ret = ctsvc_db_note_insert(record, contact_id, is_my_profile, NULL);
			if (CONTACTS_ERROR_DB == ret){
				CTS_ERR("DB error : ctsvc_db_note_insert");
				break;
			}
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(note_list));
	return ret;
}

int ctsvc_contact_insert_data_event(contacts_list_h event_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	unsigned int count = 0;

	RETV_IF(NULL == event_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(event_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(event_list);
	do {
		contacts_list_get_current_record_p(event_list, &record);
		ret = ctsvc_db_event_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_event_insert");
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(event_list));

	return ret;
}

int ctsvc_contact_insert_data_messenger(contacts_list_h messenger_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	unsigned int count = 0;

	RETV_IF(NULL == messenger_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(messenger_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(messenger_list);
	do {
		contacts_list_get_current_record_p(messenger_list, &record);
		ret = ctsvc_db_messenger_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_messenger_insert");
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(messenger_list));

	return ret;
}

int ctsvc_contact_insert_data_address(contacts_list_h address_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	unsigned int count = 0;

	RETV_IF(NULL == address_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(address_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(address_list);
	do {
		contacts_list_get_current_record_p(address_list, &record);
		ret = ctsvc_db_address_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_address_insert");
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(address_list));

	return ret;
}

int ctsvc_contact_insert_data_url(contacts_list_h url_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	unsigned int count = 0;

	RETV_IF(NULL == url_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(url_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(url_list);
	do {
		contacts_list_get_current_record_p(url_list, &record);
		ret = ctsvc_db_url_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_url_insert");
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(url_list));

	return ret;
}

int ctsvc_contact_insert_data_nickname(contacts_list_h nickname_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	unsigned int count = 0;

	RETV_IF(NULL == nickname_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(nickname_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(nickname_list);
	do {
		contacts_list_get_current_record_p(nickname_list, &record);
		ret = ctsvc_db_nickname_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_nickname_insert");
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(nickname_list));

	return ret;
}

int ctsvc_contact_insert_data_relationship(contacts_list_h relationship_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	unsigned int count = 0;

	RETV_IF(NULL == relationship_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(relationship_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(relationship_list);
	do {
		contacts_list_get_current_record_p(relationship_list, &record);
		ret = ctsvc_db_relationship_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_relationship_insert");
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(relationship_list));

	return ret;
}

int ctsvc_contact_insert_data_image(contacts_list_h image_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	unsigned int count = 0;

	RETV_IF(NULL == image_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(image_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(image_list);
	do {
		contacts_list_get_current_record_p(image_list, &record);
		ret = ctsvc_db_image_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_image_insert");
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(image_list));

	return ret;
}

int ctsvc_contact_insert_data_extension(contacts_list_h extension_list, int contact_id, bool is_my_profile)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record;
	unsigned int count = 0;

	RETV_IF(NULL == extension_list, CONTACTS_ERROR_INVALID_PARAMETER);
	ret = contacts_list_get_count(extension_list, &count);
	if(0 == count)
		return CONTACTS_ERROR_NONE;

	contacts_list_first(extension_list);
	do {
		contacts_list_get_current_record_p(extension_list, &record);
		ret = ctsvc_db_extension_insert(record, contact_id, is_my_profile, NULL);
		if (CONTACTS_ERROR_DB == ret){
			CTS_ERR("DB error : ctsvc_db_extension_insert");
			break;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(extension_list));

	return ret;
}


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
#include "ctsvc_localize_utils.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_notify.h"

#include "ctsvc_db_plugin_contact_helper.h"

#define CTSVC_MY_PROFILE_DISPLAY_NAME_MAX_LEN 1024

static int __ctsvc_db_my_profile_insert_record(contacts_record_h record, int *id);
static int __ctsvc_db_my_profile_get_record(int id, contacts_record_h* out_record);
static int __ctsvc_db_my_profile_update_record(contacts_record_h record);
static int __ctsvc_db_my_profile_delete_record(int id);

static int __ctsvc_db_my_profile_get_all_records(int offset, int limit, contacts_list_h* out_list);
static int __ctsvc_db_my_profile_get_records_with_query(contacts_query_h query, int offset, int limit, contacts_list_h* out_list);

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
	int ret;
	int i;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	char *temp;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	snprintf(query, sizeof(query),
			"SELECT my_profile_id, addressbook_id, changed_time, %s, image_thumbnail_path, uid "
				"FROM "CTS_TABLE_MY_PROFILES" WHERE my_profile_id = %d AND deleted = 0",
				ctsvc_get_display_column(), id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		CTS_ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
	}

	i = 0;
	my_profile->id = ctsvc_stmt_get_int(stmt, i++);
	my_profile->addressbook_id = ctsvc_stmt_get_int(stmt, i++);
	my_profile->changed_time = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	my_profile->display_name = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	if (temp) {
		snprintf(full_path, sizeof(full_path), "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, temp);
		my_profile->image_thumbnail_path = strdup(full_path);
	}
	temp = ctsvc_stmt_get_text(stmt, i++);
	my_profile->uid = SAFE_STRDUP(temp);
	ctsvc_stmt_finalize(stmt);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_my_profile_get_data(int id, ctsvc_my_profile_s *my_profile)
{
	int ret;
	int datatype;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query),
				"SELECT datatype, id, contact_id, is_default, data1, data2, "
					"data3, data4, data5, data6, data7, data8, data9, data10, data11, data12 "
					"FROM "CTS_TABLE_DATA" WHERE contact_id = %d AND is_my_profile = 1", id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE */!= ret) {
		CTS_ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		return ret;
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

	} while (1 /*CTS_TRUE*/ == ctsvc_stmt_step(stmt));

	ctsvc_stmt_finalize(stmt);

	return CONTACTS_ERROR_NONE;

}

static int __ctsvc_db_my_profile_get_record(int id, contacts_record_h* out_record)
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
		CTS_ERR("cts_get_main_contacts_info(ALL) Fail(%d)", ret);
		contacts_record_destroy(record, true);
		return ret;
	}

	ret = __ctsvc_db_my_profile_get_data(id, my_profile);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_my_profile_get_data Fail(%d)", ret);
		contacts_record_destroy(record, true);
		return ret;
	}

	*out_record = record;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_my_profile_delete_record(int id)
{
	CTS_FN_CALL;
	int ret;
	int addressbook_id;
	char query[CTS_SQL_MAX_LEN] = {0};

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "DB error : ctsvc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query),
		"SELECT addressbook_id FROM "CTSVC_DB_VIEW_MY_PROFILE" WHERE my_profile_id = %d", id);
	ret  = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_get_first_int_result Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (false == ctsvc_have_ab_write_permission(addressbook_id)) {
		CTS_ERR("Does not have permission to delete this contact");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	snprintf(query, sizeof(query), "UPDATE "CTS_TABLE_MY_PROFILES" "
					"SET deleted = 1, changed_ver = %d WHERE my_profile_id = %d", ctsvc_get_next_ver(), id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	ctsvc_set_my_profile_noti();

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	}
	else
		return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_my_profile_update_data(ctsvc_my_profile_s *my_profile)
{
	int ret;

	if (my_profile->name) {
		ret = ctsvc_contact_update_data_name((contacts_list_h)my_profile->name, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_name() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->company) {
		ret = ctsvc_contact_update_data_company((contacts_list_h)my_profile->company, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_company() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->note) {
		ret = ctsvc_contact_update_data_note((contacts_list_h)my_profile->note, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_note() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->events) {
		ret = ctsvc_contact_update_data_event((contacts_list_h)my_profile->events, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_events() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->messengers) {
		ret = ctsvc_contact_update_data_messenger((contacts_list_h)my_profile->messengers, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_messengers() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->postal_addrs) {
		ret = ctsvc_contact_update_data_address((contacts_list_h)my_profile->postal_addrs, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_address() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->urls) {
		ret = ctsvc_contact_update_data_url((contacts_list_h)my_profile->urls, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_url() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->nicknames) {
		ret = ctsvc_contact_update_data_nickname((contacts_list_h)my_profile->nicknames, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_nickname() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->numbers) {
		bool had_phonenumber;
		ret = ctsvc_contact_update_data_number((contacts_list_h)my_profile->numbers, my_profile->id, true, &had_phonenumber);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_number() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->emails) {
		bool had_email;
		ret = ctsvc_contact_update_data_email((contacts_list_h)my_profile->emails, my_profile->id, true, &had_email);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_email() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->profiles) {
		ret = ctsvc_contact_update_data_profile((contacts_list_h)my_profile->profiles, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_profile() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->relationships) {
		ret = ctsvc_contact_update_data_relationship((contacts_list_h)my_profile->relationships, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_relationship() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->images) {
		ret = ctsvc_contact_update_data_image((contacts_list_h)my_profile->images, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_image() Fail(%d)", ret);
			return ret;
		}
	}

	if (my_profile->extensions) {
		ret = ctsvc_contact_update_data_extension((contacts_list_h)my_profile->extensions, my_profile->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_update_data_extension() Fail(%d)", ret);
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
	ctsvc_contact_check_default_address((contacts_list_h)my_profile->postal_addrs);
}

static void __ctsvc_make_my_profile_display_name(ctsvc_my_profile_s *my_profile)
{
	ctsvc_name_s *name = NULL;

	free(my_profile->display_name);
	my_profile->display_name = NULL;

	free(my_profile->reverse_display_name);
	my_profile->reverse_display_name = NULL;

	if (0 < my_profile->name->count && my_profile->name->records != NULL && my_profile->name->records->data != NULL) {
		name = (ctsvc_name_s *)my_profile->name->records->data;
	}

	if (name && (name->first || name->last  || name->prefix || name->addition || name->suffix)) {
		char *display = NULL;
		int len, display_len;
		int reverse_lang_type = -1;
		int temp_display_len;
		char *temp_display = NULL;

		///////////////////////////////////////////////
		// Make reverse display name (Last name first)
		// Default         : Prefix Last, First Middle(addition), Suffix
		// Korean, Chinese : Prefix LastFirstMiddleSuffix
		// Japanese        : Prefix Last Middle First Suffix
		// reverse sort name does not include prefix
		//    But, if there is only prefix, reverse sort_name is prefix
		//////////////////////////////////////////////
		temp_display_len = SAFE_STRLEN(name->first)
						+ SAFE_STRLEN(name->addition)
						+ SAFE_STRLEN(name->last)
						+ SAFE_STRLEN(name->suffix);
		if (0 < temp_display_len) {
			temp_display_len += 7;
			temp_display = calloc(1, temp_display_len);
			len=0;

			if (name->last) {
				len += snprintf(temp_display + len, temp_display_len - len, "%s", name->last);

				if (reverse_lang_type < 0) {
					reverse_lang_type = ctsvc_check_language_type(temp_display);
				}

				if (reverse_lang_type != CTSVC_LANG_KOREAN &&
					reverse_lang_type != CTSVC_LANG_CHINESE &&
					reverse_lang_type != CTSVC_LANG_JAPANESE) {
					if (name->first || name->addition)
						len += snprintf(temp_display + len, temp_display_len - len, ",");
				}
			}

			if (reverse_lang_type < 0) {
				if (*temp_display) {
					reverse_lang_type = ctsvc_check_language_type(temp_display);
				}
				else if (name->first) {
					reverse_lang_type = ctsvc_check_language_type(name->first);
				}
				else if (name->addition) {
					reverse_lang_type = ctsvc_check_language_type(name->addition);
				}
			}

			if (reverse_lang_type == CTSVC_LANG_JAPANESE) {
				// make temp_display name Prefix - Last - Middle - First - Suffix
				if (name->addition) {
					if (*temp_display)
						len += snprintf(temp_display + len, temp_display_len - len, " ");
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->addition);
				}

				if (name->first) {
					if (*temp_display)
						len += snprintf(temp_display + len, temp_display_len - len, " ");
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->first);
				}
			}
			else {
				if (name->first) {
					if (*temp_display) {
						if (reverse_lang_type < 0) {
							reverse_lang_type = ctsvc_check_language_type(temp_display);
						}

						if (reverse_lang_type != CTSVC_LANG_KOREAN &&
								reverse_lang_type != CTSVC_LANG_CHINESE)
							len += snprintf(temp_display + len, temp_display_len - len, " ");
					}
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->first);
				}

				if (name->addition) {
					if (*temp_display) {
						if (reverse_lang_type < 0) {
							reverse_lang_type = ctsvc_check_language_type(temp_display);
						}

						if (reverse_lang_type != CTSVC_LANG_KOREAN &&
								reverse_lang_type != CTSVC_LANG_CHINESE)
							len += snprintf(temp_display + len, temp_display_len - len, " ");
					}
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->addition);
				}
			}

			if (name->suffix) {
				if (*temp_display) {
					if (reverse_lang_type < 0) {
						reverse_lang_type = ctsvc_check_language_type(temp_display);
					}

					if (reverse_lang_type == CTSVC_LANG_JAPANESE)
						len += snprintf(temp_display + len, temp_display_len - len, " ");
					else if (reverse_lang_type != CTSVC_LANG_KOREAN &&
									reverse_lang_type != CTSVC_LANG_CHINESE)
						len += snprintf(temp_display + len, temp_display_len - len, ", ");
				}
				len += snprintf(temp_display + len, temp_display_len - len, "%s", name->suffix);
			}
		}

		if (name->prefix && temp_display) {
			display_len = SAFE_STRLEN(name->prefix) + temp_display_len + 2;
			display = calloc(1, display_len);
			snprintf(display, display_len, "%s %s", name->prefix, temp_display);
			my_profile->reverse_display_name = display;
			free(temp_display);
		}
		else if (temp_display) {
			my_profile->reverse_display_name = temp_display;
		}
		else if (name->prefix) {
			my_profile->reverse_display_name = strdup(name->prefix);
		}

		///////////////////////////////////////////////
		// Make display name (First name first)
		// Default         : Prefix First Middle Last, Suffix
		// Korean, Chinese : Prefix LastFirstMiddleSuffix (Same as reverse display name)
		// Japanese        : Prefix First Middle Last Suffix
		// sort name does not include prefix
		//    But, if there is only prefix, sort_name is prefix
		//////////////////////////////////////////////
		if (reverse_lang_type == CTSVC_LANG_KOREAN ||
			reverse_lang_type == CTSVC_LANG_CHINESE)
			my_profile->display_name = SAFE_STRDUP(my_profile->reverse_display_name);
		else {
			int lang_type = -1;
			temp_display = NULL;
			temp_display_len = SAFE_STRLEN(name->first)
								+ SAFE_STRLEN(name->addition)
								+ SAFE_STRLEN(name->last)
								+ SAFE_STRLEN(name->suffix);
			if (0 < temp_display_len) {
				temp_display_len += 6;
				// make reverse_temp_display_name
				temp_display = calloc(1, temp_display_len);
				len = 0;

				if (name->first) {
					if (*temp_display)
						len += snprintf(temp_display + len, temp_display_len - len, " ");
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->first);
				}

				if (name->addition) {
					if (*temp_display)
						len += snprintf(temp_display + len, temp_display_len - len, " ");
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->addition);
				}

				if (name->last) {
					if (*temp_display)
						len += snprintf(temp_display + len, temp_display_len - len, " ");
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->last);
				}

				if (name->suffix) {
					if (*temp_display) {
						lang_type = ctsvc_check_language_type(temp_display);
						if (lang_type == CTSVC_LANG_JAPANESE)
							len += snprintf(temp_display + len, temp_display_len - len, " ");
						else
							len += snprintf(temp_display + len, temp_display_len - len, ", ");
					}
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->suffix);
				}

			}
			if (name->prefix && temp_display) {
				display_len = SAFE_STRLEN(name->prefix) + temp_display_len + 2;
				display = calloc(1, display_len);
				snprintf(display, display_len, "%s %s", name->prefix, temp_display);
				my_profile->display_name = display;
				free(temp_display);
			}
			else if (temp_display) {
				my_profile->display_name = temp_display;
			}
			else if (name->prefix) {
				my_profile->display_name = strdup(name->prefix);
			}
		}

		ctsvc_record_set_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.display_name, CTSVC_PROPERTY_FLAG_DIRTY);
	}
	else {
		GList *cur;
		if (my_profile->company && my_profile->company->records) {
			for (cur=my_profile->company->records;cur;cur=cur->next) {
				ctsvc_company_s *company = (ctsvc_company_s *)cur->data;
				if (company && company->name) {
					ctsvc_record_set_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.display_name, CTSVC_PROPERTY_FLAG_DIRTY);
					my_profile->display_name = SAFE_STRDUP(company->name);
					break;
				}
			}
		}

		if (!ctsvc_record_check_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.display_name, CTSVC_PROPERTY_FLAG_DIRTY) &&
				my_profile->nicknames && my_profile->nicknames->records) {
			for (cur=my_profile->nicknames->records;cur;cur=cur->next) {
				ctsvc_nickname_s *nickname = (ctsvc_nickname_s *)cur->data;
				if (nickname && nickname->nickname) {
					ctsvc_record_set_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.display_name, CTSVC_PROPERTY_FLAG_DIRTY);
					my_profile->display_name = SAFE_STRDUP(nickname->nickname);
					break;
				}
			}
		}

		if (!ctsvc_record_check_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.display_name, CTSVC_PROPERTY_FLAG_DIRTY) &&
				my_profile->numbers && my_profile->numbers->records) {
			for (cur=my_profile->numbers->records;cur;cur=cur->next) {
				ctsvc_number_s *number = (ctsvc_number_s *)cur->data;
				if (number && number->number) {
					ctsvc_record_set_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.display_name, CTSVC_PROPERTY_FLAG_DIRTY);
					my_profile->display_name = SAFE_STRDUP(number->number);
					break;
				}
			}
		}

		if (!ctsvc_record_check_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.display_name, CTSVC_PROPERTY_FLAG_DIRTY) &&
				my_profile->emails && my_profile->emails->records) {
			for (cur=my_profile->emails->records;cur;cur=cur->next) {
				ctsvc_email_s *email = (ctsvc_email_s *)cur->data;
				if (email && email->email_addr) {
					ctsvc_record_set_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.display_name, CTSVC_PROPERTY_FLAG_DIRTY);
					my_profile->display_name = SAFE_STRDUP(email->email_addr);
					break;
				}
			}
		}

		if (ctsvc_record_check_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.display_name, CTSVC_PROPERTY_FLAG_DIRTY)) {
			my_profile->reverse_display_name = SAFE_STRDUP(my_profile->display_name);
		}
		else {
			// Update as NULL
			ctsvc_record_set_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.display_name, CTSVC_PROPERTY_FLAG_DIRTY);
		}
	}
	return;
}


static int __ctsvc_db_my_profile_update_record(contacts_record_h record)
{
	int id;
	int ret;
	char *set = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_my_profile_s *my_profile = (ctsvc_my_profile_s*)record;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query),
		"SELECT my_profile_id FROM "CTSVC_DB_VIEW_MY_PROFILE" WHERE my_profile_id = %d", my_profile->id);
	ret = ctsvc_query_get_first_int_result(query, &id);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("The index(%d) is Invalid. %d Record(s) is(are) found", my_profile->id, ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (false == ctsvc_have_ab_write_permission(my_profile->addressbook_id)) {
		CTS_ERR("ctsvc_have_ab_write_permission fail : does not have permission(addressbook_id : %d)",
					my_profile->addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	__ctsvc_make_my_profile_display_name(my_profile);
	__ctsvc_my_profile_check_default_data(my_profile);

	//update data
	ret = __ctsvc_my_profile_update_data(my_profile);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_my_profile_update_data() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////
	// this code will be removed.
	if (my_profile->images) {
		int ret = CONTACTS_ERROR_NONE;
		contacts_record_h record_image = NULL;
		int count = 0;
		ctsvc_image_s *image;

		contacts_list_get_count((contacts_list_h)my_profile->images, &count);
		if (count) {
			contacts_list_first((contacts_list_h)my_profile->images);
			ret = contacts_list_get_current_record_p((contacts_list_h)my_profile->images, &record_image);

			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("contacts_list_get_current_record_p() Fail(%d)", ret);
				ctsvc_end_trans(false);
				return CONTACTS_ERROR_DB;
			}

			image = (ctsvc_image_s*)record_image;
			if ((NULL == my_profile->image_thumbnail_path && NULL != image->path) ||
					(NULL != my_profile->image_thumbnail_path && NULL == image->path) ||
					(my_profile->image_thumbnail_path && image->path && STRING_EQUAL != strcmp(my_profile->image_thumbnail_path, image->path))) {
				ctsvc_record_set_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.image_thumbnail_path, CTSVC_PROPERTY_FLAG_DIRTY);

				if (ctsvc_contact_check_image_location(image->path))
					my_profile->image_thumbnail_path = SAFE_STRDUP(image->path + strlen(CTSVC_CONTACT_IMG_FULL_LOCATION) + 1);
				else
					my_profile->image_thumbnail_path = SAFE_STRDUP(image->path);
			}
		}
		else if (my_profile->image_thumbnail_path) {
			free(my_profile->image_thumbnail_path);
			my_profile->image_thumbnail_path = NULL;
			ctsvc_record_set_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.image_thumbnail_path, CTSVC_PROPERTY_FLAG_DIRTY);
		}
	}
	// this code will be removed.
	//////////////////////////////////////////////////////////////////////

	do {
		int len = 0;
		int version;
		char query[CTS_SQL_MAX_LEN] = {0};
		char query_set[CTS_SQL_MIN_LEN] = {0, };
		cts_stmt stmt = NULL;

		version = ctsvc_get_next_ver();

		ret = ctsvc_db_create_set_query(record, &set, &bind_text);
		WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_db_create_set_query() Fail(%d)", ret);

		if (set && *set)
			len = snprintf(query_set, sizeof(query_set), "%s, ", set);
		len += snprintf(query_set+len, sizeof(query_set)-len, " changed_ver=%d, changed_time=%d", version, (int)time(NULL));

		if (ctsvc_record_check_property_flag((ctsvc_record_s *)my_profile, _contacts_my_profile.display_name, CTSVC_PROPERTY_FLAG_DIRTY)) {
			len += snprintf(query_set+len, sizeof(query_set)-len,
					", display_name=?, reverse_display_name=?");
			bind_text = g_slist_append(bind_text, strdup(SAFE_STR(my_profile->display_name)));
			bind_text = g_slist_append(bind_text, strdup(SAFE_STR(my_profile->reverse_display_name)));
		}

		snprintf(query, sizeof(query), "UPDATE %s SET %s WHERE my_profile_id = %d", CTS_TABLE_MY_PROFILES, query_set, my_profile->id);

		ret = ctsvc_query_prepare(query, &stmt);
		if (NULL == stmt) {
			CTS_ERR("DB error : ctsvc_query_prepare() Fail(%d)", ret);
			break;
		}

		if (bind_text) {
			int i = 0;
			for (cursor=bind_text,i=1;cursor;cursor=cursor->next,i++) {
				const char *text = cursor->data;
				if (*text)
					ctsvc_stmt_bind_text(stmt, i, text);
			}
		}

		ret = ctsvc_stmt_step(stmt);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			break;
		}
		ctsvc_stmt_finalize(stmt);
	} while (0);

	CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s *)record);
	CONTACTS_FREE(set);
	if (bind_text) {
		for (cursor=bind_text;cursor;cursor=cursor->next)
			CONTACTS_FREE(cursor->data);
		g_slist_free(bind_text);
	}

	ctsvc_set_my_profile_noti();

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "DB error : ctsvc_end_trans() Fail(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_my_profile_get_all_records(int offset, int limit, contacts_list_h* out_list)
{
	int ret;
	int len;
	int my_profile_id;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;

	len = snprintf(query, sizeof(query),
			"SELECT my_profile_id FROM "CTSVC_DB_VIEW_MY_PROFILE);

	if (0 != limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}
		my_profile_id = ctsvc_stmt_get_int(stmt, 0);
		ret = contacts_db_get_record(_contacts_my_profile._uri, my_profile_id, &record);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("DB error : contacts_db_get_record() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}
		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_my_profile_get_records_with_query(contacts_query_h query, int offset, int limit, contacts_list_h* out_list)
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

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
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
				ASSERT_NOT_REACHED("To set projection is Fail.\n");
		}

		for (i=0;i<field_count;i++) {
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
					snprintf(full_path, sizeof(full_path), "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, temp);
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
			CTS_ERR("__ctsvc_db_my_profile_get_data Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
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
			CTS_ERR("ctsvc_contact_insert_data_name() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the company
	if (contact->company) {
		ret = ctsvc_contact_insert_data_company((contacts_list_h)contact->company, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_company() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the events
	if (contact->events) {
		ret = ctsvc_contact_insert_data_event((contacts_list_h)contact->events, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_event() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the messengers
	if (contact->messengers) {
		ret = ctsvc_contact_insert_data_messenger((contacts_list_h)contact->messengers, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_messenger() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the postals
	if (contact->postal_addrs) {
		ret = ctsvc_contact_insert_data_address((contacts_list_h)contact->postal_addrs, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_postal() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the Web addrs
	if (contact->urls) {
		ret = ctsvc_contact_insert_data_url((contacts_list_h)contact->urls, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_web() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the Nick names
	if (contact->nicknames) {
		ret = ctsvc_contact_insert_data_nickname((contacts_list_h)contact->nicknames, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_nickname() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the numbers
	if (contact->numbers) {
		ret = ctsvc_contact_insert_data_number((contacts_list_h)contact->numbers, contact->id, true);
		if (ret < CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_contact_insert_data_number() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the emails
	if (contact->emails) {
		ret = ctsvc_contact_insert_data_email((contacts_list_h)contact->emails, contact->id, true);
		if (ret < CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_insert_my_profile_data_email() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the profile values
	if (contact->profiles) {
		ret = ctsvc_contact_insert_data_profile((contacts_list_h)contact->profiles, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_insert_my_profile_data_profile() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the relationship values
	if (contact->relationships) {
		ret = ctsvc_contact_insert_data_relationship((contacts_list_h)contact->relationships, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_relationship() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the image values
	if (contact->images) {
		ret = ctsvc_contact_insert_data_image((contacts_list_h)contact->images, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_image() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the note values
	if (contact->note) {
		ret = ctsvc_contact_insert_data_note((contacts_list_h)contact->note, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_note() Fail(%d)", ret);
			return ret;
		}
	}

	//Insert the extensions values
	if (contact->extensions) {
		ret = ctsvc_contact_insert_data_extension((contacts_list_h)contact->extensions, contact->id, true);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_contact_insert_data_extension() Fail(%d)", ret);
			return ret;
		}
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_my_profile_insert_record(contacts_record_h record, int *id)
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
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	if (false == ctsvc_have_ab_write_permission(my_profile->addressbook_id)) {
		CTS_ERR("ctsvc_have_ab_write_permission fail : does not have permission(addressbook_id : %d)",
					my_profile->addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_MY_PROFILES" WHERE addressbook_id = %d AND deleted = 1", my_profile->addressbook_id);
	ret = ctsvc_query_exec(query);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "Delete deleted my_profile of addressbook(%d) Fail", my_profile->addressbook_id);

	ret = ctsvc_db_get_next_id(CTS_TABLE_MY_PROFILES);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_db_get_next_id() Fail(%d)", ret);
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
		CTS_ERR("cts_insert_my_profile_data() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	//////////////////////////////////////////////////////////////////////
	// this code will be removed.
	free(my_profile->image_thumbnail_path);
	my_profile->image_thumbnail_path = NULL;

	if (my_profile->images) {
		ctsvc_image_s *image;
		int count = 0;

		contacts_list_get_count((contacts_list_h)my_profile->images, &count);

		while (count) {
			contacts_list_first((contacts_list_h)my_profile->images);
			ret = contacts_list_get_current_record_p((contacts_list_h)my_profile->images, (contacts_record_h*)&image);
			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("contacts_list_get_current_record_p() Fail(%d)", ret);
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

	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		CTS_ERR("ctsvc_query_prepare() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
	}

	if (my_profile->display_name)
		ctsvc_stmt_bind_text(stmt, 1, my_profile->display_name);
	if (my_profile->reverse_display_name)
		ctsvc_stmt_bind_text(stmt, 2, my_profile->reverse_display_name);
	if (my_profile->uid)
		ctsvc_stmt_bind_text(stmt, 3, my_profile->uid);
	if (my_profile->image_thumbnail_path)
		ctsvc_stmt_bind_text(stmt, 4, my_profile->image_thumbnail_path);

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
	}
	ctsvc_stmt_finalize(stmt);

	ctsvc_set_my_profile_noti();

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_svc_end_trans() Fail(%d)", ret);

	return CONTACTS_ERROR_NONE;
}


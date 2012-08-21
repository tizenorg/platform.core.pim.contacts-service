/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjae Shin <yj99.shin@samsung.com>
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
#include "cts-internal.h"
#include "cts-normalize.h"
#include "cts-utils.h"
#include "cts-list.h"

static inline CTSvalue* cts_iter_get_info_contact(cts_stmt stmt, int type)
{
	int i, lang;
	char *temp;
	contact_list *result;

	result = (contact_list *)contacts_svc_value_new(CTS_VALUE_LIST_CONTACT);
	retvm_if(NULL == result, NULL, "contacts_svc_value_new() Failed");

	i = 0;
	result->person_id = cts_stmt_get_int(stmt, i++);
	lang = cts_stmt_get_int(stmt, i++);
	temp = cts_stmt_get_text(stmt, i++);
	result->first = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, i++);
	result->last = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, i++);
	result->display = SAFE_STRDUP(temp);
	result->addrbook_id = cts_stmt_get_int(stmt, i++);
	temp = cts_stmt_get_text(stmt, i++);
	if (temp) {
		char full_path[CTS_IMG_PATH_SIZE_MAX];
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_IMAGE_LOCATION, temp);
		result->img_path = strdup(full_path);
	}
	result->contact_id = cts_stmt_get_int(stmt, i++);

	if (CTS_LANG_DEFAULT == lang)
		lang = cts_get_default_language();

	if (NULL == result->display && result->first && result->last
			&& CTS_LANG_ENGLISH == lang) {
		char display[CTS_SQL_MAX_LEN];
		if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			snprintf(display, sizeof(display), "%s %s", result->first, result->last);
		else
			snprintf(display, sizeof(display), "%s, %s", result->last, result->first);

		result->display = strdup(display);
	}
	if (CTS_ITER_CONTACTS_WITH_NAME != type) {
		temp = cts_stmt_get_text(stmt, i++);
		result->normalize = SAFE_STRDUP(temp);
	}

	return (CTSvalue *)result;
}

static inline CTSvalue* cts_iter_get_info_osp(cts_stmt stmt)
{
	int i, lang;
	char *temp;
	osp_list *result;

	result = (osp_list *)contacts_svc_value_new(CTS_VALUE_LIST_OSP);
	retvm_if(NULL == result, NULL, "contacts_svc_value_new() Failed");

	i = 0;
	result->person_id = cts_stmt_get_int(stmt, i++);
	lang = cts_stmt_get_int(stmt, i++);
	temp = cts_stmt_get_text(stmt, i++);
	result->first = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, i++);
	result->last = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, i++);
	result->display = SAFE_STRDUP(temp);
	result->addrbook_id = cts_stmt_get_int(stmt, i++);
	temp = cts_stmt_get_text(stmt, i++);
	if (temp) {
		char full_path[CTS_IMG_PATH_SIZE_MAX];
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_IMAGE_LOCATION, temp);
		result->img_path = strdup(full_path);
	}
	result->contact_id = cts_stmt_get_int(stmt, i++);
	result->def_num_type = cts_stmt_get_int(stmt, i++);
	temp = cts_stmt_get_text(stmt, i++);
	result->def_num = SAFE_STRDUP(temp);
	result->def_email_type = cts_stmt_get_int(stmt, i++);
	temp = cts_stmt_get_text(stmt, i++);
	result->def_email = SAFE_STRDUP(temp);

	if (CTS_LANG_DEFAULT == lang)
		lang = cts_get_default_language();

	if (NULL == result->display && result->first && result->last
			&& CTS_LANG_ENGLISH == lang) {
		char display[CTS_SQL_MAX_LEN];
		if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			snprintf(display, sizeof(display), "%s %s", result->first, result->last);
		else
			snprintf(display, sizeof(display), "%s, %s", result->last, result->first);

		result->display = strdup(display);
	}
	temp = cts_stmt_get_text(stmt, i++);
	result->normalize = SAFE_STRDUP(temp);

	return (CTSvalue *)result;
}

static inline CTSvalue* cts_iter_get_info_number_email(cts_stmt stmt, int type)
{
	int i, lang;
	char *temp;
	contact_list *result;

	result = (contact_list *)contacts_svc_value_new(CTS_VALUE_LIST_CONTACT);
	retvm_if(NULL == result, NULL, "contacts_svc_value_new() Failed");
	if (CTS_ITER_EMAILINFOS_WITH_EMAIL == type)
		result->v_type = CTS_VALUE_LIST_EMAILINFO;
	else if (CTS_ITER_NUMBERS_EMAILS == type)
		result->v_type = CTS_VALUE_LIST_NUMS_EMAILS;
	else
		result->v_type = CTS_VALUE_LIST_NUMBERINFO;

	i = 0;
	result->person_id = cts_stmt_get_int(stmt, i++);
	lang = cts_stmt_get_int(stmt, i++);
	temp = cts_stmt_get_text(stmt, i++);
	result->first = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, i++);
	result->last = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, i++);
	result->display = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, i++);
	result->connect = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, i++);
	if (temp) {
		char full_path[CTS_IMG_PATH_SIZE_MAX];
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_IMAGE_LOCATION, temp);
		result->img_path = strdup(full_path);
	}
	result->contact_id = cts_stmt_get_int(stmt, i++);
	if (CTS_ITER_NUMBERS_EMAILS == type) {
		result->addrbook_id = cts_stmt_get_int(stmt, i++);
		temp = cts_stmt_get_text(stmt, i++);
		result->normalize = SAFE_STRDUP(temp);
	}

	if (CTS_LANG_DEFAULT == lang)
		lang = cts_get_default_language();

	if (NULL == result->display && result->first && result->last
			&& CTS_LANG_ENGLISH == lang) {
		char display[CTS_SQL_MAX_LEN];
		if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			snprintf(display, sizeof(display), "%s %s", result->first, result->last);
		else
			snprintf(display, sizeof(display), "%s, %s", result->last, result->first);

		result->display = strdup(display);
	}

	return (CTSvalue *)result;
}

static inline CTSvalue* cts_iter_get_info_sdn(cts_stmt stmt, int type)
{
	char *temp;
	sdn_list *result;

	result = (sdn_list *)contacts_svc_value_new(CTS_VALUE_LIST_SDN);
	retvm_if(NULL == result, NULL, "contacts_svc_value_new() Failed");

	temp = cts_stmt_get_text(stmt, 0);
	result->name = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, 1);
	result->number = SAFE_STRDUP(temp);

	return (CTSvalue *)result;
}

static inline CTSvalue* cts_iter_get_info_change(updated_record *cursor)
{
	change_list *result;

	result = (change_list *)contacts_svc_value_new(CTS_VALUE_LIST_CHANGE);
	retvm_if(NULL == result, NULL, "contacts_svc_value_new() Failed");

	result->changed_type = cursor->type;
	result->id = cursor->id;
	result->changed_ver = cursor->ver;
	result->addressbook_id = cursor->addressbook_id;

	return (CTSvalue *)result;
}

static inline CTSvalue* cts_iter_get_info_plog(int type, cts_stmt stmt)
{
	int lang, cnt=0;
	char *temp;
	plog_list *result;

	result = (plog_list *)contacts_svc_value_new(CTS_VALUE_LIST_PLOG);
	retvm_if(NULL == result, NULL, "contacts_svc_value_new() Failed");

	switch (type)
	{
	case CTS_ITER_GROUPING_PLOG:
		result->id = cts_stmt_get_int(stmt, cnt++);
		lang = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->first = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->last = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->display = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		if (temp) {
			char full_path[CTS_IMG_PATH_SIZE_MAX];
			snprintf(full_path, sizeof(full_path), "%s/%s", CTS_IMAGE_LOCATION, temp);
			result->img_path = strdup(full_path);
		}
		if (CTS_LANG_DEFAULT == lang)
			lang = cts_get_default_language();

		if (NULL == result->display && result->first && result->last
				&& CTS_LANG_ENGLISH == lang) {
			char display[CTS_SQL_MAX_LEN];
			if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
				snprintf(display, sizeof(display), "%s %s", result->first, result->last);
			else
				snprintf(display, sizeof(display), "%s, %s", result->last, result->first);

			result->display = strdup(display);
		}

		temp = cts_stmt_get_text(stmt, cnt++);
		result->number = SAFE_STRDUP(temp);
		result->log_type = cts_stmt_get_int(stmt, cnt++);
		result->log_time = cts_stmt_get_int(stmt, cnt++);
		result->extra_data1 = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->extra_data2 = SAFE_STRDUP(temp);
		result->related_id = cts_stmt_get_int(stmt, cnt++);
		result->num_type = cts_stmt_get_int(stmt, cnt++);
		break;
	case CTS_ITER_PLOGS_OF_NUMBER:
		result->id = cts_stmt_get_int(stmt, cnt++);
		result->log_type = cts_stmt_get_int(stmt, cnt++);
		result->log_time = cts_stmt_get_int(stmt, cnt++);
		result->extra_data1 = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->extra_data2 = SAFE_STRDUP(temp);
		result->related_id = cts_stmt_get_int(stmt, cnt++);
		break;
	case CTS_ITER_PLOGS_OF_PERSON_ID:
		result->id = cts_stmt_get_int(stmt, cnt++);
		result->log_type = cts_stmt_get_int(stmt, cnt++);
		result->log_time = cts_stmt_get_int(stmt, cnt++);
		result->extra_data1 = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->extra_data2 = SAFE_STRDUP(temp);
		result->related_id = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->number = SAFE_STRDUP(temp);
		break;
	default:
		ERR("Invalid parameter : The type(%d) is unknown type", type);
		contacts_svc_value_free((CTSvalue*)result);
		return NULL;
	}
	return (CTSvalue *)result;
}

static inline CTSvalue* cts_iter_get_info_custom_num_type(cts_stmt stmt)
{
	numtype_list *result;

	result = (numtype_list *)contacts_svc_value_new(CTS_VALUE_LIST_CUSTOM_NUM_TYPE);
	retvm_if(NULL == result, NULL, "contacts_svc_value_new() Failed");

	result->id = cts_stmt_get_int(stmt, 0);
	result->name = SAFE_STRDUP(cts_stmt_get_text(stmt, 1));

	return (CTSvalue *)result;
}

static inline CTSvalue* cts_iter_get_info_addrbook(cts_stmt stmt)
{
	cts_addrbook *result;

	result = (cts_addrbook *)contacts_svc_value_new(CTS_VALUE_LIST_ADDRBOOK);
	retvm_if(NULL == result, NULL, "contacts_svc_value_new() Failed");

	cts_stmt_get_addressbook(stmt, result);

	return (CTSvalue *)result;
}

static inline CTSvalue* cts_iter_get_info_group(cts_stmt stmt)
{
	cts_group *result;

	result = (cts_group *)contacts_svc_value_new(CTS_VALUE_LIST_GROUP);
	retvm_if(NULL == result, NULL, "contacts_svc_value_new() Failed");

	result->id = cts_stmt_get_int(stmt, 0);
	result->addrbook_id = cts_stmt_get_int(stmt, 1);
	result->name = SAFE_STRDUP(cts_stmt_get_text(stmt, 2));
	result->ringtone_path = SAFE_STRDUP(cts_stmt_get_text(stmt, 3));
	result->img_loaded = false; //It will load at cts_value_get_str_group()

	return (CTSvalue *)result;
}

static inline CTSvalue* cts_iter_get_info_shortcut(int type, cts_stmt stmt)
{
	int i, lang;
	char *temp;
	shortcut_list *result;

	result = (shortcut_list *)contacts_svc_value_new(CTS_VALUE_LIST_SHORTCUT);
	retvm_if(NULL == result, NULL, "contacts_svc_value_new() Failed");

	i = 0;
	result->contact_id = cts_stmt_get_int(stmt, i++);
	lang = cts_stmt_get_int(stmt, i++);
	temp = cts_stmt_get_text(stmt, i++);
	result->first = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, i++);
	result->last = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, i++);
	result->display = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, i++);
	if (temp) {
		char full_path[CTS_IMG_PATH_SIZE_MAX];
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_IMAGE_LOCATION, temp);
		result->img_path = strdup(full_path);
	}
	result->id = cts_stmt_get_int(stmt, i++);
	if (CTS_LANG_DEFAULT == lang)
		lang = cts_get_default_language();

	if (NULL == result->display && result->first && result->last
			&& CTS_LANG_ENGLISH == lang) {
		char display[CTS_SQL_MAX_LEN];
		if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			snprintf(display, sizeof(display), "%s %s", result->first, result->last);
		else
			snprintf(display, sizeof(display), "%s, %s", result->last, result->first);

		result->display = strdup(display);
	}

	if (CTS_ITER_ALL_CONTACT_FAVORITE != type) {
		result->num_type = cts_stmt_get_int(stmt, i++);
		temp = cts_stmt_get_text(stmt, i++);
		result->number = SAFE_STRDUP(temp);

		if (CTS_ITER_ALL_SPEEDDIAL == type)
			result->speeddial = cts_stmt_get_int(stmt, i++);
	}

	return (CTSvalue *)result;
}

API CTSvalue* contacts_svc_iter_get_info(CTSiter *iter)
{
	CTSvalue *result;

	retvm_if(NULL == iter, NULL, "iter is NULL");

	switch (iter->i_type)
	{
	case CTS_ITER_CONTACTS:
	case CTS_ITER_CONTACTS_WITH_NAME:
		result = cts_iter_get_info_contact(iter->stmt, iter->i_type);
		retvm_if(NULL == result, NULL, "cts_iter_get_info_contact() Failed");
		break;
	case CTS_ITER_NUMBERINFOS:
	case CTS_ITER_EMAILINFOS_WITH_EMAIL:
	case CTS_ITER_NUMBERS_EMAILS:
		result = cts_iter_get_info_number_email(iter->stmt, iter->i_type);
		retvm_if(NULL == result, NULL, "cts_iter_get_info_number() Failed");
		break;
	case CTS_ITER_ALL_SDN:
		result = cts_iter_get_info_sdn(iter->stmt, iter->i_type);
		retvm_if(NULL == result, NULL, "cts_iter_get_info_number() Failed");
		break;
	case CTS_ITER_UPDATED_INFO_AFTER_VER:
		result = cts_iter_get_info_change(iter->info->cursor);
		retvm_if(NULL == result, NULL, "cts_iter_get_info_change() Failed");
		break;
	case CTS_ITER_GROUPING_PLOG:
	case CTS_ITER_PLOGS_OF_NUMBER:
	case CTS_ITER_PLOGS_OF_PERSON_ID:
		result = cts_iter_get_info_plog(iter->i_type, iter->stmt);
		retvm_if(NULL == result, NULL, "cts_iter_get_info_plog() Failed");
		break;
	case CTS_ITER_ALL_CUSTOM_NUM_TYPE:
		result = cts_iter_get_info_custom_num_type(iter->stmt);
		retvm_if(NULL == result, NULL, "cts_iter_get_info_custom_num_type() Failed");
		break;
	case CTS_ITER_ADDRESSBOOKS:
		result = cts_iter_get_info_addrbook(iter->stmt);
		retvm_if(NULL == result, NULL, "cts_iter_get_info_addrbook() Failed");
		break;
	case CTS_ITER_GROUPS:
		result = cts_iter_get_info_group(iter->stmt);
		retvm_if(NULL == result, NULL, "cts_iter_get_info_group() Failed");
		break;
	case CTS_ITER_ALL_NUM_FAVORITE:
	case CTS_ITER_ALL_CONTACT_FAVORITE:
	case CTS_ITER_ALL_SPEEDDIAL:
		result = cts_iter_get_info_shortcut(iter->i_type, iter->stmt);
		retvm_if(NULL == result, NULL, "cts_iter_get_info_shortcut() Failed");
		break;
	case CTS_ITER_PLOGNUMBERS_WITH_NUM:
		result = (CTSvalue*)(SAFE_STRDUP(cts_stmt_get_text(iter->stmt, 0)));
		break;
	case CTS_ITER_OSP:
		result = cts_iter_get_info_osp(iter->stmt);
		break;
	default:
		ERR("Invalid parameter : The iter(%d) has unknown type", iter->i_type);
		return NULL;
	}

	return result;
}


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
#include <time.h>

#include "cts-internal.h"
#include "cts-schema.h"
#include "cts-sqlite.h"
#include "cts-utils.h"
#include "cts-group.h"
#include "cts-types.h"
#include "cts-normalize.h"
#include "cts-contact.h"

static const int CTS_UPDATE_ID_LOC = 11;

static inline int cts_insert_contact_data_number(cts_stmt stmt,
		GSList* number_list)
{
	CTS_FN_CALL;

	int ret, cnt, default_num=0, mobile_num=0;
	GSList *number_repeat = number_list;
	cts_number *number_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == number_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (number_data = (cts_number *)number_repeat->data)
				&& !number_data->deleted && number_data->number)
		{
			const char *normal_num;
			char clean_num[CTS_NUMBER_MAX_LEN];

			cnt = 1;
			cts_stmt_bind_int(stmt, cnt++, CTS_DATA_NUMBER);
			cts_stmt_bind_int(stmt, cnt++, number_data->type);
			ret = cts_clean_number(number_data->number, clean_num, sizeof(clean_num));
			if (ret <= 0) {
				ERR("Number(%s) is invalid", number_data->number);
				number_repeat = g_slist_next(number_repeat);
				continue;
			}

			cts_stmt_bind_text(stmt, cnt++, clean_num);
			normal_num = cts_normalize_number(clean_num);
			cts_stmt_bind_text(stmt, cnt++, normal_num);

			ret = cts_stmt_step(stmt);
			retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
			number_data->id = cts_db_get_last_insert_id();

			if (number_data->is_default)
				default_num = number_data->id;
			else if (!default_num && CTS_NUM_TYPE_CELL & number_data->type && !mobile_num)
				mobile_num = number_data->id;
			cts_stmt_reset(stmt);
		}
		number_repeat = g_slist_next(number_repeat);
	}while(number_repeat);

	if (default_num)
		return default_num;
	else
		return mobile_num;
}

static inline int cts_insert_contact_data_email(cts_stmt stmt,
		GSList* email_list)
{
	CTS_FN_CALL;

	int ret, cnt, default_email=0;
	GSList *email_repeat = email_list;
	cts_email *email_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == email_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (email_data = (cts_email *)email_repeat->data)
				&& !email_data->deleted && email_data->email_addr)
		{
			cnt = 1;
			cts_stmt_bind_int(stmt, cnt++, CTS_DATA_EMAIL);
			cts_stmt_bind_int(stmt, cnt++, email_data->type);
			cts_stmt_bind_text(stmt, cnt++, email_data->email_addr);

			ret = cts_stmt_step(stmt);
			retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
			email_data->id = cts_db_get_last_insert_id();

			if (email_data->is_default)
				default_email = email_data->id;
			cts_stmt_reset(stmt);
		}
		email_repeat = g_slist_next(email_repeat);
	}while(email_repeat);

	return default_email;
}

static inline int cts_insert_contact_grouprel(int acc_id, int index,
		GSList* group_list)
{
	CTS_FN_CALL;

	GSList *group_repeat = group_list;
	cts_group *group_data;

	retv_if(NULL == group_list, CTS_ERR_ARG_NULL);

	do
	{
		group_data = group_repeat->data;
		group_repeat = group_repeat->next;

		if (NULL == group_data || group_data->deleted)
			continue;

		if (group_data->vcard_group) {
			int found_id;
			found_id = contacts_svc_find_group(acc_id, group_data->vcard_group);
			if (0 < found_id)
				group_data->id = found_id;
			else if (found_id == CTS_ERR_DB_RECORD_NOT_FOUND) {
				CTSvalue *group;
				group = contacts_svc_value_new(CTS_VALUE_GROUP);

				contacts_svc_value_set_str(group, CTS_GROUP_VAL_NAME_STR, group_data->vcard_group);
				found_id = contacts_svc_insert_group(acc_id, group);
				if (found_id < CTS_SUCCESS)
					ERR("contacts_svc_insert_group() Failed\n");
				else
					group_data->id = found_id;

				contacts_svc_value_free(group);
			}
		}
		if (group_data->id) {
			int ret = cts_group_set_relation(group_data->id, index, acc_id);
			retvm_if(ret < CTS_SUCCESS, ret, "cts_group_set_relation() Failed(%d)", ret);
		}
	}while(group_repeat);

	return CTS_SUCCESS;
}

static inline int cts_insert_contact_data_event(cts_stmt stmt,
		GSList* event_list)
{
	CTS_FN_CALL;

	GSList *event_repeat = event_list;
	cts_event *event_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == event_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (event_data = (cts_event *)event_repeat->data)
				&& !event_data->deleted && event_data->date)
		{
			cts_stmt_bind_int(stmt, 1, CTS_DATA_EVENT);
			cts_stmt_bind_event(stmt, 2, event_data);

			int ret = cts_stmt_step(stmt);
			retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
			cts_stmt_reset(stmt);
		}
		event_repeat = g_slist_next(event_repeat);
	}while(event_repeat);

	return CTS_SUCCESS;
}


static inline int cts_insert_contact_data_messenger(cts_stmt stmt,
		GSList* messenger_list)
{
	CTS_FN_CALL;

	GSList *messenger_repeat = messenger_list;
	cts_messenger *messenger_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == messenger_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (messenger_data = (cts_messenger *)messenger_repeat->data)
				&& !messenger_data->deleted && messenger_data->im_id)
		{
			cts_stmt_bind_int(stmt, 1, CTS_DATA_MESSENGER);
			cts_stmt_bind_messenger(stmt, 2, messenger_data);

			int ret = cts_stmt_step(stmt);
			retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
			cts_stmt_reset(stmt);
		}
		messenger_repeat = g_slist_next(messenger_repeat);
	}while(messenger_repeat);

	return CTS_SUCCESS;
}

static inline int cts_insert_contact_data_postal(cts_stmt stmt,
		GSList* postal_list)
{
	CTS_FN_CALL;

	GSList *postal_repeat = postal_list;
	cts_postal *postal_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == postal_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (postal_data = (cts_postal *)postal_repeat->data)
				&& !postal_data->deleted
				&& (postal_data->country || postal_data->pobox
					|| postal_data->postalcode || postal_data->region
					|| postal_data->locality || postal_data->street || postal_data->extended))
		{
			cts_stmt_bind_int(stmt, 1, CTS_DATA_POSTAL);
			cts_stmt_bind_postal(stmt, 2, postal_data);

			int ret = cts_stmt_step(stmt);
			retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
			cts_stmt_reset(stmt);
		}
		postal_repeat = g_slist_next(postal_repeat);
	}while(postal_repeat);

	return CTS_SUCCESS;
}

static inline int cts_insert_contact_data_web(cts_stmt stmt,
		GSList* web_list)
{
	CTS_FN_CALL;

	GSList *web_repeat = web_list;
	cts_web *web_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == web_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (web_data = (cts_web *)web_repeat->data)
				&& !web_data->deleted && web_data->url)
		{
			cts_stmt_bind_int(stmt, 1, CTS_DATA_WEB);
			cts_stmt_bind_web(stmt, 2, web_data);

			int ret = cts_stmt_step(stmt);
			retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
			cts_stmt_reset(stmt);
		}
		web_repeat = g_slist_next(web_repeat);
	}while(web_repeat);

	return CTS_SUCCESS;
}

static inline int cts_insert_contact_data_nick(cts_stmt stmt,
		GSList* nick_list)
{
	CTS_FN_CALL;
	GSList *nick_repeat = nick_list;
	cts_nickname *nick_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == nick_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (nick_data = (cts_nickname *)nick_repeat->data)
				&& !nick_data->deleted && nick_data->nick)
		{
			cts_stmt_bind_int(stmt, 1, CTS_DATA_NICKNAME);
			cts_stmt_bind_text(stmt, 3, nick_data->nick);

			int ret = cts_stmt_step(stmt);
			retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
			cts_stmt_reset(stmt);
		}
		nick_repeat = g_slist_next(nick_repeat);
	}while(nick_repeat);

	return CTS_SUCCESS;
}

static inline int cts_insert_contact_data_extend(cts_stmt stmt,
		GSList* extend_list)
{
	CTS_FN_CALL;

	GSList *extend_repeat = extend_list;
	cts_extend *extend_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == extend_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (extend_data = (cts_extend *)extend_repeat->data)
				&& !extend_data->deleted)
		{
			cts_stmt_bind_int(stmt, 1, extend_data->type);
			cts_stmt_bind_extend(stmt, 2, extend_data);

			int ret = cts_stmt_step(stmt);
			retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
			cts_stmt_reset(stmt);
		}
		extend_repeat = g_slist_next(extend_repeat);
	}while(extend_repeat);

	return CTS_SUCCESS;
}

static inline int cts_make_name_lookup(int op_code, cts_name *name,
		char *name_lookup, int name_lookup_size)
{
	if (name->display) {
		snprintf(name_lookup, name_lookup_size, "%s", name->display);
		return CTS_SUCCESS;
	}

	if (CTS_ORDER_NAME_FIRSTLAST == op_code)
		snprintf(name_lookup, name_lookup_size, "%s %c%s",
				SAFE_STR(name->first), 0x1, SAFE_STR(name->last));
	else
		snprintf(name_lookup, name_lookup_size, "%s,%c %c%s",
				SAFE_STR(name->last), 0x1, 0x1, SAFE_STR(name->first));

	return CTS_SUCCESS;
}

static inline int cts_insert_contact_data_name(cts_stmt stmt,
		cts_name *name)
{
	int ret;
	int cnt = 2;
	cts_name normalize_name={0};
	char *tmp_display, *tmp_first, *tmp_last;
	char display[CTS_SQL_MAX_LEN]={0};
	char lookup[CTS_SQL_MAX_LEN]={0};
	char reverse_lookup[CTS_SQL_MAX_LEN]={0};

	//insert name search info
	char normal_name[CTS_NN_MAX][CTS_SQL_MAX_LEN]={{0},{0},{0}};

	tmp_display = name->display;
	tmp_first = name->first;
	tmp_last = name->last;

	if (name->display) {
		ret = cts_normalize_name(name, normal_name, true);
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_name() Failed(%d)", ret);
		if (normal_name[CTS_NN_FIRST][0])
			normalize_name.display = normal_name[CTS_NN_FIRST];
		else
			name->display = NULL;
	}

	if (NULL == name->display) {
		ret = cts_normalize_name(name, normal_name, false);
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_name() Failed(%d)", ret);

		switch (ret)
		{
		case CTS_LANG_KOREAN:
			snprintf(display, sizeof(display), "%s%s",
					SAFE_STR(name->last), SAFE_STR(name->first));

			strncat(normal_name[CTS_NN_LAST], normal_name[CTS_NN_FIRST],
					sizeof(normal_name[CTS_NN_LAST]));

			name->display = display;
			normalize_name.display = normal_name[CTS_NN_LAST];
			break;
		case CTS_LANG_ENGLISH:
		default:
			if (normal_name[CTS_NN_FIRST][0])
				normalize_name.first = normal_name[CTS_NN_FIRST];
			else
				name->first = NULL;

			if (normal_name[CTS_NN_LAST][0])
				normalize_name.last = normal_name[CTS_NN_LAST];
			else
				name->last = NULL;

			break;
		}
	}

	if (cts_get_default_language() == ret)
		cts_stmt_bind_int(stmt, cnt, CTS_LANG_DEFAULT);
	else
		cts_stmt_bind_int(stmt, cnt, ret);
	cnt = cts_stmt_bind_name(stmt, cnt, name);

	name->display = tmp_display;
	name->first = tmp_first;
	name->last = tmp_last;

	ret = cts_make_name_lookup(CTS_ORDER_NAME_FIRSTLAST, &normalize_name,
										lookup, sizeof(lookup));
	retvm_if(CTS_SUCCESS != ret, ret, "cts_make_name_lookup() Failed(%d)", ret);

	ret = cts_make_name_lookup(CTS_ORDER_NAME_LASTFIRST, &normalize_name,
										reverse_lookup, sizeof(reverse_lookup));
	retvm_if(CTS_SUCCESS != ret, ret, "cts_make_name_lookup() Failed(%d)", ret);

	CTS_DBG("lookup=%s(%d), reverse_lookup=%s(%d)",
			lookup, strlen(lookup), reverse_lookup, strlen(reverse_lookup));

	cts_stmt_bind_text(stmt, cnt++, lookup);
	cts_stmt_bind_text(stmt, cnt++, reverse_lookup);
	cts_stmt_bind_text(stmt, cnt++, normal_name[CTS_NN_SORTKEY]);

	return cnt;
}

static int cts_insert_contact_data(int field, contact_t *contact)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query), "INSERT INTO %s(contact_id, datatype, "
			"data1, data2, data3, data4, data5, data6, data7, data8, data9, data10) "
			"VALUES(%d, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
			CTS_TABLE_DATA, contact->base->id);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	//Insert the name
	if ((field & CTS_DATA_FIELD_NAME))
	{
		cts_stmt_bind_int(stmt, 1, CTS_DATA_NAME);

		if (contact->name) {
			ret = cts_insert_contact_data_name(stmt, contact->name);
			if (ret < CTS_SUCCESS)
			{
				ERR("cts_insert_contact_data_name() Failed(%d)", ret);
				cts_stmt_finalize(stmt);
				return ret;
			}
		}

		ret = cts_stmt_step(stmt);
		if (CTS_SUCCESS != ret) {
			ERR("cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
		cts_stmt_reset(stmt);
	}

	//Insert the company
	if (contact->company && (field & CTS_DATA_FIELD_COMPANY))
	{
		cts_company *com = contact->company;
		if (com->name || com->department || com->jot_title || com->role || com->assistant_name) {
			cts_stmt_bind_int(stmt, 1, CTS_DATA_COMPANY);
			cts_stmt_bind_company(stmt, 2, com);

			ret = cts_stmt_step(stmt);
			if (CTS_SUCCESS != ret) {
				ERR("cts_stmt_step() Failed(%d)", ret);
				cts_stmt_finalize(stmt);
				return ret;
			}
			cts_stmt_reset(stmt);
		}
	}

	//Insert the events
	if (contact->events && (field & CTS_DATA_FIELD_EVENT))
	{
		ret = cts_insert_contact_data_event(stmt, contact->events);

		if (CTS_SUCCESS != ret)
		{
			ERR("cts_insert_contact_data_event() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}

	//Insert the messengers
	if (contact->messengers && (field & CTS_DATA_FIELD_MESSENGER))
	{
		ret = cts_insert_contact_data_messenger(stmt, contact->messengers);

		if (CTS_SUCCESS != ret)
		{
			ERR("cts_insert_contact_data_messenger() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}

	//Insert the postals
	if (contact->postal_addrs && (field & CTS_DATA_FIELD_POSTAL))
	{
		ret = cts_insert_contact_data_postal(stmt, contact->postal_addrs);

		if (CTS_SUCCESS != ret)
		{
			ERR("cts_insert_contact_data_postal() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}

	//Insert the Web addrs
	if (contact->web_addrs && (field & CTS_DATA_FIELD_WEB))
	{
		ret = cts_insert_contact_data_web(stmt, contact->web_addrs);

		if (CTS_SUCCESS != ret)
		{
			ERR("cts_insert_contact_data_web() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}

	//Insert the Nick names
	if (contact->nicknames && (field & CTS_DATA_FIELD_NICKNAME))
	{
		ret = cts_insert_contact_data_nick(stmt, contact->nicknames);

		if (CTS_SUCCESS != ret)
		{
			ERR("cts_insert_contact_data_nick() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}

	//Insert the numbers
	if (contact->numbers && (field & CTS_DATA_FIELD_NUMBER))
	{
		ret = cts_insert_contact_data_number(stmt, contact->numbers);

		if (ret < CTS_SUCCESS)
		{
			ERR("cts_insert_contact_data_number() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
		contact->default_num = ret;
	}

	//Insert the emails
	if (contact->emails && (field & CTS_DATA_FIELD_EMAIL))
	{
		ret = cts_insert_contact_data_email(stmt, contact->emails);

		if (ret < CTS_SUCCESS) {
			ERR("cts_insert_contact_data_email() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
		contact->default_email = ret;
	}


	//Insert the extended values
	if (contact->extended_values && (field & CTS_DATA_FIELD_EXTEND_ALL))
	{
		ret = cts_insert_contact_data_extend(stmt, contact->extended_values);

		if (CTS_SUCCESS != ret)
		{
			ERR("cts_insert_contact_data_extend() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}

	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}

static int cts_set_first_id_for_default(contact_t *contact)
{
	GSList *cur;

	if (!contact->default_num)
	{
		for (cur = contact->numbers;cur;cur = cur->next) {
			if(cur->data) {
				cts_number *num = cur->data;
				if(!num->deleted && num->id) {
					contact->default_num = num->id;
					break;
				}
			}
		}
	}

	if (!contact->default_email)
	{
		for (cur = contact->emails;cur;cur = cur->next) {
			if(cur->data) {
				cts_email *email = cur->data;
				if(!email->deleted && email->id) {
					contact->default_email = email->id;
					break;
				}
			}
		}
	}

	return CTS_SUCCESS;
}

static inline char* cts_contact_get_valid_first_number_or_email(GSList *numbers, GSList *emails)
{
	GSList *cur;

	for (cur=numbers;cur;cur=cur->next) {
		cts_number *num = cur->data;
		if (num && !num->deleted && num->number) {
			return num->number;
		}
	}

	for (cur=emails;cur;cur=cur->next) {
		cts_email *email = cur->data;
		if (email && !email->deleted && email->email_addr) {
			return email->email_addr;
		}
	}
	return NULL;
}

static inline void cts_insert_contact_handle_no_name(contact_t *contact)
{
	if (NULL == contact->name) {
		contact->name = calloc(1, sizeof(cts_name));
		contact->name->embedded = true;
	}

	if (contact->name->display || contact->name->first || contact->name->last)
		return;

	if (contact->company && contact->company->name)
		contact->name->display = strdup(contact->company->name);
	else {
		char *temp;

		temp = cts_contact_get_valid_first_number_or_email(contact->numbers, contact->emails);
		contact->name->display = SAFE_STRDUP(temp);
	}
	return;
}

static inline int cts_safe_strcmp(char *s1, char *s2)
{
	if (NULL == s1 || NULL == s2)
		return !(s1 == s2);
	else
		return strcmp(s1, s2);
}


static inline void cts_contact_remove_dup_data_number(GSList *numbers)
{
	GSList *cur1, *cur2;

	cts_number *num1, *num2;
	for (cur1=numbers;cur1;cur1=cur1->next) {
		num1 = cur1->data;
		if (NULL == num1 || num1->deleted)
			continue;
		for (cur2=numbers;cur2;cur2=cur2->next) {
			num2 = cur2->data;
			if(NULL == num2 || num1 == num2)
				continue;
			if (!num2->deleted && num1->type == num2->type &&
					!cts_safe_strcmp(num1->number, num2->number)) {
				num1->is_default |= num2->is_default;
				num1->is_favorite |= num2->is_favorite;
				num2->deleted = true;
			}
		}
	}
}


static inline void cts_contact_remove_dup_data_email(GSList *emails)
{
	GSList *cur1, *cur2;
	cts_email *email1, *email2;

	for (cur1=emails;cur1;cur1=cur1->next) {
		email1 = cur1->data;
		if (NULL == email1 || email1->deleted)
			continue;
		for (cur2=emails;cur2;cur2=cur2->next) {
			email2 = cur2->data;
			if(NULL == email2 || email1 == email2)
				continue;
			if (!email2->deleted && email1->type == email2->type &&
					!cts_safe_strcmp(email1->email_addr, email2->email_addr)) {
				email1->is_default |= email2->is_default;
				email2->deleted = true;
			}
		}
	}
}

static inline void cts_contact_remove_dup_data_event(GSList *events)
{
	GSList *cur1, *cur2;
	cts_event *event1, *event2;

	for (cur1=events;cur1;cur1=cur1->next) {
		event1 = cur1->data;
		if (NULL == event1 || event1->deleted)
			continue;
		for (cur2=events;cur2;cur2=cur2->next) {
			event2 = cur2->data;
			if(NULL == event2 || event1 == event2)
				continue;
			if (!event2->deleted && event1->type == event2->type &&
					event1->date == event2->date) {
				event2->deleted = true;
			}
		}
	}
}


static inline void cts_contact_remove_dup_data_IM(GSList *IMs)
{
	GSList *cur1, *cur2;
	cts_messenger *im1, *im2;

	for (cur1=IMs;cur1;cur1=cur1->next) {
		im1 = cur1->data;
		if (NULL == im1 || im1->deleted)
			continue;
		for (cur2=IMs;cur2;cur2=cur2->next) {
			im2 = cur2->data;
			if(NULL == im2 || im1 == im2)
				continue;
			if (!im2->deleted && im1->type == im2->type &&
					!cts_safe_strcmp(im1->im_id, im2->im_id)) {
				im2->deleted = true;
			}
		}
	}
}


static inline void cts_contact_remove_dup_data_web(GSList *webs)
{
	GSList *cur1, *cur2;
	cts_web *web1, *web2;

	for (cur1=webs;cur1;cur1=cur1->next) {
		web1 = cur1->data;
		if (NULL == web1 || web1->deleted)
			continue;
		for (cur2=webs;cur2;cur2=cur2->next) {
			web2 = cur2->data;
			if(NULL == web2 || web1 == web2)
				continue;
			if (!web2->deleted && web1->type == web2->type &&
					!cts_safe_strcmp(web1->url, web2->url)) {
				web2->deleted = true;
			}
		}
	}
}


static inline void cts_contact_remove_dup_data_nick(GSList *nicks)
{
	GSList *cur1, *cur2;
	cts_nickname *nick1, *nick2;

	for (cur1=nicks;cur1;cur1=cur1->next) {
		nick1 = cur1->data;
		if (NULL == nick1 || nick1->deleted)
			continue;
		for (cur2=nicks;cur2;cur2=cur2->next) {
			nick2 = cur2->data;
			if(NULL == nick2 || nick1 == nick2)
				continue;
			if (!nick2->deleted && !cts_safe_strcmp(nick1->nick, nick2->nick)) {
				nick2->deleted = true;
			}
		}
	}
}


static inline void cts_contact_remove_dup_data_postal(GSList *postals)
{
	GSList *cur1, *cur2;
	cts_postal *addr1, *addr2;

	for (cur1=postals;cur1;cur1=cur1->next) {
		addr1 = cur1->data;
		if (NULL == addr1 || addr1->deleted)
			continue;
		for (cur2=postals;cur2;cur2=cur2->next) {
			addr2 = cur2->data;
			if(NULL == addr2 || addr1 == addr2)
				continue;
			if (!addr2->deleted && addr1->type == addr2->type &&
					!cts_safe_strcmp(addr1->pobox, addr2->pobox) &&
					!cts_safe_strcmp(addr1->postalcode, addr2->postalcode) &&
					!cts_safe_strcmp(addr1->region, addr2->region) &&
					!cts_safe_strcmp(addr1->locality, addr2->locality) &&
					!cts_safe_strcmp(addr1->street, addr2->street) &&
					!cts_safe_strcmp(addr1->extended, addr2->extended) &&
					!cts_safe_strcmp(addr1->country, addr2->country)) {
				addr2->deleted = true;
			}
		}
	}
}


static void cts_contact_remove_dup_data(contact_t *contact)
{
	if (contact->numbers)
		cts_contact_remove_dup_data_number(contact->numbers);

	if (contact->emails)
		cts_contact_remove_dup_data_email(contact->emails);

	if (contact->events)
		cts_contact_remove_dup_data_event(contact->events);

	if (contact->messengers)
		cts_contact_remove_dup_data_IM(contact->messengers);

	if (contact->web_addrs)
		cts_contact_remove_dup_data_web(contact->web_addrs);

	if (contact->nicknames)
		cts_contact_remove_dup_data_nick(contact->nicknames);

	if (contact->postal_addrs)
		cts_contact_remove_dup_data_postal(contact->postal_addrs);

}

static inline int cts_insert_contact(int addressbook_id, contact_t *contact)
{
	int ret;
	char *img_path;
	char query[CTS_SQL_MAX_LEN] = {0};

	retv_if(NULL == contact, CTS_ERR_ARG_NULL);

	cts_insert_contact_handle_no_name(contact);
	cts_contact_remove_dup_data(contact);

	//Insert Data
	ret = cts_insert_contact_data(CTS_DATA_FIELD_ALL, contact);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_insert_contact_data() Failed(%d)", ret);

	//Insert group Info
	if (contact->grouprelations)
	{
		ret = cts_insert_contact_grouprel(addressbook_id, contact->base->id,
				contact->grouprelations);
		retvm_if(ret < CTS_SUCCESS, ret, "cts_insert_contact_grouprel() Failed(%d)", ret);
	}

	// default setting
	if (!contact->default_num || !contact->default_email)
		ret = cts_set_first_id_for_default(contact);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_set_first_id_for_default() Failed(%d)", ret);

	//insert contact table
	int input_ver = cts_get_next_ver();

	snprintf(query, sizeof(query),
			"INSERT INTO %s(contact_id, addrbook_id, created_ver, changed_ver, "
			"changed_time, default_num, default_email, uid, ringtone, note, image0, image1) "
			"VALUES(%d, %d, %d, %d, %d, %d, %d, ?, ?, ?, ?, ?)",
			CTS_TABLE_CONTACTS, contact->base->id, addressbook_id, input_ver,
			input_ver, (int)time(NULL), contact->default_num, contact->default_email);

	cts_stmt stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	if (contact->base->uid)
		cts_stmt_bind_text(stmt, 1, contact->base->uid);
	if (contact->base->ringtone_path)
		cts_stmt_bind_text(stmt, 2, contact->base->ringtone_path);
	if (contact->base->note)
		cts_stmt_bind_text(stmt, 3, contact->base->note);

	if (contact->base->img_path) {
		ret = cts_add_image_file(CTS_IMG_NORMAL, contact->base->id, contact->base->img_path, &img_path);
		if (CTS_SUCCESS != ret) {
			ERR("cts_add_image_file(NORMAL) Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
		cts_stmt_bind_text(stmt, 4, img_path);
	}
	else if (contact->base->vcard_img_path) {
		ret = cts_add_image_file(CTS_IMG_NORMAL, contact->base->id, contact->base->vcard_img_path, &img_path);
		if (CTS_SUCCESS == ret)
			cts_stmt_bind_text(stmt, 4, img_path);
	}

	ret = cts_add_image_file(CTS_IMG_FULL, contact->base->id, contact->base->full_img_path, &img_path);
	if (CTS_SUCCESS == ret)
		cts_stmt_bind_text(stmt, 5, img_path);

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret) {
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}
	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}


API int contacts_svc_insert_contact(int addressbook_id, CTSstruct* contact)
{
	int ret;
	contact_t *record;

	CTS_FN_CALL;

	retv_if(NULL == contact, CTS_ERR_ARG_NULL);
	retvm_if(CTS_STRUCT_CONTACT != contact->s_type, CTS_ERR_ARG_INVALID,
			"The contact(%d) must be type of CTS_STRUCT_CONTACT.", contact->s_type);

	CTS_START_TIME_CHECK;

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	record = (contact_t *)contact;

	ret = cts_db_get_next_id(CTS_TABLE_CONTACTS);
	if (ret < CTS_SUCCESS)
	{
		ERR("cts_db_get_next_id() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	CTS_DBG("index = %d.", ret);

	if (!record->base) {
		record->base = (cts_ct_base*)contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);

		if (NULL == record->base)
		{
			ERR("contacts_svc_value_new() Failed");
			contacts_svc_end_trans(false);
			return CTS_ERR_OUT_OF_MEMORY;
		}
	}

	record->base->id = ret;
	record->base->addrbook_id = addressbook_id;
	ret = cts_insert_contact(addressbook_id, record);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_insert_contact() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	cts_set_contact_noti();
	ret = contacts_svc_end_trans(true);
	retvm_if(ret < CTS_SUCCESS, ret, "contacts_svc_end_trans() Failed(%d)", ret);

	CTS_END_TIME_CHECK();
	return record->base->id;
}

API int contacts_svc_delete_contact(int index)
{
	CTS_FN_CALL;
	int ret, addrbook_id;
	char query[CTS_SQL_MAX_LEN] = {0};

	CTS_START_TIME_CHECK;

	snprintf(query, sizeof(query),
			"SELECT addrbook_id FROM %s WHERE contact_id = %d",
			CTS_TABLE_CONTACTS, index);
	addrbook_id = cts_query_get_first_int_result(query);
	retvm_if(addrbook_id < CTS_SUCCESS, addrbook_id,
			"The index(%d) is Invalid(%d)", index, addrbook_id);

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	ret = cts_delete_image_file(CTS_IMG_NORMAL, index);
	if (CTS_SUCCESS != ret && CTS_ERR_DB_RECORD_NOT_FOUND != ret) {
		ERR("cts_delete_image_file(NORMAL) Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}
	ret = cts_delete_image_file(CTS_IMG_FULL, index);
	warn_if(CTS_SUCCESS != ret && CTS_ERR_DB_RECORD_NOT_FOUND != ret,
			"cts_delete_image_file(FULL) Failed(%d)", ret);

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE contact_id = %d",
			CTS_TABLE_CONTACTS, index);
	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "INSERT INTO %s VALUES(%d, %d, %d)",
			CTS_TABLE_DELETEDS, index, addrbook_id, cts_get_next_ver());
	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	cts_set_contact_noti();

	ret = contacts_svc_end_trans(true);
	CTS_END_TIME_CHECK();
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

static inline int cts_delete_record_by_id(const char *table, int id)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	retv_if(NULL == table, CTS_ERR_ARG_NULL);

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE id=%d", table, id);

	ret = cts_query_exec(query);
	retvm_if(ret, ret, "cts_query_exec() Failed(%d)", ret);

	return CTS_SUCCESS;
}

static inline int cts_update_contact_data_number(
		cts_stmt stmt, int contact_id, GSList* number_list)
{
	CTS_FN_CALL;

	int ret, default_num=0, mobile_num=0;
	GSList *added_numbers=NULL, *number_repeat = number_list;
	cts_number *number_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == number_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (number_data = (cts_number *)number_repeat->data))
		{
			if (!number_data->id){
				if (!number_data->deleted)
					added_numbers = g_slist_append(added_numbers, number_data);
				number_repeat = g_slist_next(number_repeat);
				continue;
			}
			if (number_data->deleted || NULL == number_data->number) {
				ret = cts_delete_record_by_id(CTS_TABLE_DATA, number_data->id);
				retvm_if(ret, ret, "cts_delete_record_by_id() Failed(%d)", ret);
			}
			else
			{
				int cnt = 1;
				const char *normal_num;
				char clean_num[CTS_NUMBER_MAX_LEN];

				cts_stmt_bind_int(stmt, cnt++, number_data->type);

				ret = cts_clean_number(number_data->number, clean_num, sizeof(clean_num));
				if (ret <= 0) {
					ERR("Number(%s) is invalid", number_data->number);
					number_repeat = g_slist_next(number_repeat);
					continue;
				}
				cts_stmt_bind_text(stmt, cnt++, clean_num);

				normal_num = cts_normalize_number(clean_num);
				cts_stmt_bind_text(stmt, cnt++, normal_num);
				cts_stmt_bind_int(stmt, CTS_UPDATE_ID_LOC, number_data->id);

				ret = cts_stmt_step(stmt);
				retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);

				if (number_data->is_default)
					default_num = number_data->id;
				else if (!default_num && CTS_NUM_TYPE_CELL & number_data->type && !mobile_num)
					mobile_num = number_data->id;
				cts_stmt_reset(stmt);
			}
		}
		number_repeat = g_slist_next(number_repeat);
	}while(number_repeat);

	if (added_numbers) {
		contact_t temp;
		cts_ct_base base;
		temp.base = &base;
		temp.base->id = contact_id;
		temp.numbers = added_numbers;

		ret = cts_insert_contact_data(CTS_DATA_FIELD_NUMBER, &temp);
		if (temp.default_num) {
			number_repeat = added_numbers;
			while (number_repeat) {
				if (NULL != (number_data = (cts_number *)number_repeat->data)) {
					if (number_data->is_default) {
						default_num = temp.default_num;
						break;
					}
				}
				number_repeat = number_repeat->next;
			}
			if(!default_num)
				mobile_num = temp.default_num;
		}
		g_slist_free(added_numbers);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_insert_contact_data() Failed(%d)", ret);
	}

	if (default_num)
		return default_num;
	else
		return mobile_num;
}

static inline int cts_update_contact_data_email(
		cts_stmt stmt, int contact_id, GSList* email_list)
{
	CTS_FN_CALL;

	int ret, default_email=0;
	GSList *added_emails=NULL, *email_repeat = email_list;
	cts_email *email_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == email_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (email_data = (cts_email *)email_repeat->data))
		{
			if (!email_data->id){
				if (!email_data->deleted)
					added_emails = g_slist_append(added_emails, email_data);
				email_repeat = g_slist_next(email_repeat);
				continue;
			}
			if (email_data->deleted || NULL == email_data->email_addr) {
				ret = cts_delete_record_by_id(CTS_TABLE_DATA, email_data->id);
				retvm_if(ret, ret, "cts_delete_record_by_id() Failed(%d)", ret);
			}
			else
			{
				int cnt = 1;

				cts_stmt_bind_int(stmt, cnt++, email_data->type);
				cts_stmt_bind_text(stmt, cnt++, email_data->email_addr);
				cts_stmt_bind_int(stmt, CTS_UPDATE_ID_LOC, email_data->id);

				ret = cts_stmt_step(stmt);
				retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);

				if (email_data->is_default)
					default_email = email_data->id;
				cts_stmt_reset(stmt);
			}
		}
		email_repeat = g_slist_next(email_repeat);
	}while(email_repeat);

	if (added_emails) {
		contact_t temp;
		cts_ct_base base;
		temp.base = &base;
		temp.base->id = contact_id;
		temp.emails = added_emails;

		ret = cts_insert_contact_data(CTS_DATA_FIELD_EMAIL, &temp);
		if (temp.default_email) {
			email_repeat = added_emails;
			while (email_repeat) {
				if (NULL != (email_data = (cts_email *)email_repeat->data)) {
					if (email_data->is_default) {
						default_email = temp.default_email;
						break;
					}
				}
				email_repeat = email_repeat->next;
			}
		}
		g_slist_free(added_emails);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_insert_contact_data() Failed(%d)", ret);
	}

	return default_email;
}

static inline int cts_update_contact_grouprel(cts_ct_base *base_info,
		GSList* group_list)
{
	CTS_FN_CALL;

	int ret;
	GSList *group_repeat = group_list;
	cts_group *group_data;

	retv_if(NULL == group_list, CTS_ERR_ARG_NULL);

	do
	{
		group_data = group_repeat->data;
		group_repeat = group_repeat->next;

		if (NULL == group_data)
			continue;

		if (group_data->deleted) {
			ret = cts_group_unset_relation(group_data->id, base_info->id);
			retvm_if(ret, ret, "cts_group_unset_relation() Failed(%d)", ret);
		}
		else {
			if (group_data->vcard_group) {
				int found_id;
				found_id = contacts_svc_find_group(base_info->addrbook_id, group_data->vcard_group);
				if (0 < found_id)
					group_data->id = found_id;
				else if (found_id == CTS_ERR_DB_RECORD_NOT_FOUND) {
					CTSvalue *group;
					group = contacts_svc_value_new(CTS_VALUE_GROUP);

					contacts_svc_value_set_str(group, CTS_GROUP_VAL_NAME_STR, group_data->vcard_group);
					found_id = contacts_svc_insert_group(base_info->addrbook_id, group);
					if (found_id < CTS_SUCCESS)
						ERR("contacts_svc_insert_group() Failed\n");
					else
						group_data->id = found_id;

					contacts_svc_value_free(group);
				}
			}
			if (group_data->id) {
				ret = cts_group_set_relation(group_data->id, base_info->id,
						base_info->addrbook_id);
				retvm_if(ret < CTS_SUCCESS, ret, "cts_group_set_relation() Failed(%d)", ret);
			}
		}
	}while(group_repeat);

	return CTS_SUCCESS;
}

static inline int cts_update_contact_data_event(
		cts_stmt stmt, int contact_id, GSList* event_list)
{
	CTS_FN_CALL;

	int ret;
	GSList *added_event=NULL, *event_repeat = event_list;
	cts_event *event_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == event_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (event_data = (cts_event *)event_repeat->data))
		{
			if (!event_data->id){
				if (!event_data->deleted)
					added_event = g_slist_append(added_event, event_data);
				event_repeat = g_slist_next(event_repeat);
				continue;
			}
			if (event_data->deleted) {
				ret = cts_delete_record_by_id(CTS_TABLE_DATA, event_data->id);
				retvm_if(ret, ret, "cts_delete_record_by_id() Failed(%d)", ret);
			}
			else
			{
				cts_stmt_bind_event(stmt, 1, event_data);
				cts_stmt_bind_int(stmt, CTS_UPDATE_ID_LOC, event_data->id);

				ret = cts_stmt_step(stmt);
				retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
				cts_stmt_reset(stmt);
			}
		}
		event_repeat = g_slist_next(event_repeat);
	}while(event_repeat);

	if (added_event) {
		contact_t temp;
		cts_ct_base base;
		temp.base = &base;
		temp.base->id = contact_id;
		temp.events = added_event;

		ret = cts_insert_contact_data(CTS_DATA_FIELD_EVENT, &temp);
		g_slist_free(added_event);
		retvm_if(CTS_SUCCESS != ret, ret,
				"cts_insert_contact_data() Failed(%d)", ret);
	}
	return CTS_SUCCESS;
}

static inline int cts_update_contact_data_messenger(
		cts_stmt stmt, int contact_id, GSList* messenger_list)
{
	CTS_FN_CALL;

	int ret;
	GSList *added_messenger=NULL, *messenger_repeat = messenger_list;
	cts_messenger *messenger_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == messenger_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (messenger_data = (cts_messenger *)messenger_repeat->data))
		{
			if (!messenger_data->id){
				if (!messenger_data->deleted)
					added_messenger = g_slist_append(added_messenger, messenger_data);
				messenger_repeat = g_slist_next(messenger_repeat);
				continue;
			}
			if (messenger_data->deleted || NULL == messenger_data->im_id)
			{
				ret = cts_delete_record_by_id(CTS_TABLE_DATA, messenger_data->id);
				retvm_if(ret, ret, "cts_delete_record_by_id() Failed(%d)", ret);
			}
			else
			{
				cts_stmt_bind_messenger(stmt, 1, messenger_data);
				cts_stmt_bind_int(stmt, CTS_UPDATE_ID_LOC, messenger_data->id);

				ret = cts_stmt_step(stmt);
				retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
				cts_stmt_reset(stmt);
			}
		}
		messenger_repeat = g_slist_next(messenger_repeat);
	}while(messenger_repeat);

	if (added_messenger) {
		contact_t temp;
		cts_ct_base base;
		temp.base = &base;
		temp.base->id = contact_id;
		temp.messengers = added_messenger;

		ret = cts_insert_contact_data(CTS_DATA_FIELD_MESSENGER, &temp);
		g_slist_free(added_messenger);
		retvm_if(CTS_SUCCESS != ret, ret,
				"cts_insert_contact_data() Failed(%d)", ret);
	}
	return CTS_SUCCESS;
}

static inline int cts_update_contact_data_postal(
		cts_stmt stmt, int contact_id, GSList* postal_list)
{
	CTS_FN_CALL;

	int ret;
	GSList *added_postal=NULL, *postal_repeat = postal_list;
	cts_postal *postal_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == postal_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (postal_data = (cts_postal *)postal_repeat->data))
		{
			if (!postal_data->id){
				if (!postal_data->deleted)
					added_postal = g_slist_append(added_postal, postal_data);
				postal_repeat = g_slist_next(postal_repeat);
				continue;
			}
			if (postal_data->deleted || !(postal_data->country || postal_data->pobox
						|| postal_data->postalcode || postal_data->region
						|| postal_data->locality || postal_data->street || postal_data->extended))
			{
				ret = cts_delete_record_by_id(CTS_TABLE_DATA, postal_data->id);
				retvm_if(ret, ret, "cts_delete_record_by_id() Failed(%d)", ret);
			}
			else
			{
				cts_stmt_bind_postal(stmt, 1, postal_data);
				cts_stmt_bind_int(stmt, CTS_UPDATE_ID_LOC, postal_data->id);

				ret = cts_stmt_step(stmt);
				retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
				cts_stmt_reset(stmt);
			}
		}
		postal_repeat = g_slist_next(postal_repeat);
	}while(postal_repeat);

	if (added_postal) {
		contact_t temp;
		cts_ct_base base;
		temp.base = &base;
		temp.base->id = contact_id;
		temp.postal_addrs = added_postal;

		ret = cts_insert_contact_data(CTS_DATA_FIELD_POSTAL, &temp);
		g_slist_free(added_postal);
		retvm_if(CTS_SUCCESS != ret, ret,
				"cts_insert_contact_data() Failed(%d)", ret);
	}
	return CTS_SUCCESS;
}

static inline int cts_update_contact_data_web(
		cts_stmt stmt, int contact_id, GSList* web_list)
{
	CTS_FN_CALL;

	int ret;
	GSList *added_web=NULL, *web_repeat = web_list;
	cts_web *web_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == web_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (web_data = (cts_web *)web_repeat->data))
		{
			if (!web_data->id){
				if (!web_data->deleted)
					added_web = g_slist_append(added_web, web_data);
				web_repeat = g_slist_next(web_repeat);
				continue;
			}
			if (web_data->deleted || NULL == web_data->url)
			{
				ret = cts_delete_record_by_id(CTS_TABLE_DATA, web_data->id);
				retvm_if(ret, ret, "cts_delete_record_by_id() Failed(%d)", ret);
			}
			else
			{
				cts_stmt_bind_web(stmt, 1, web_data);
				cts_stmt_bind_int(stmt, CTS_UPDATE_ID_LOC, web_data->id);

				ret = cts_stmt_step(stmt);
				retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
				cts_stmt_reset(stmt);
			}
		}
		web_repeat = g_slist_next(web_repeat);
	}while(web_repeat);

	if (added_web) {
		contact_t temp;
		cts_ct_base base;
		temp.base = &base;
		temp.base->id = contact_id;
		temp.web_addrs = added_web;

		ret = cts_insert_contact_data(CTS_DATA_FIELD_WEB, &temp);
		g_slist_free(added_web);
		retvm_if(CTS_SUCCESS != ret, ret,
				"cts_insert_contact_data() Failed(%d)", ret);
	}
	return CTS_SUCCESS;
}

static inline int cts_update_contact_data_nick(
		cts_stmt stmt, int contact_id, GSList* nick_list)
{
	CTS_FN_CALL;

	int ret;
	GSList *added_nick=NULL, *nick_repeat = nick_list;
	cts_nickname *nick_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == nick_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (nick_data = (cts_nickname *)nick_repeat->data))
		{
			if (!nick_data->id){
				if (!nick_data->deleted)
					added_nick = g_slist_append(added_nick, nick_data);
				nick_repeat = g_slist_next(nick_repeat);
				continue;
			}
			if (nick_data->deleted || NULL == nick_data->nick)
			{
				ret = cts_delete_record_by_id(CTS_TABLE_DATA, nick_data->id);
				retvm_if(ret, ret, "cts_delete_record_by_id() Failed(%d)", ret);
			}
			else
			{
				cts_stmt_bind_text(stmt, 2, nick_data->nick);
				cts_stmt_bind_int(stmt, CTS_UPDATE_ID_LOC, nick_data->id);

				ret = cts_stmt_step(stmt);
				retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
				cts_stmt_reset(stmt);
			}
		}
		nick_repeat = g_slist_next(nick_repeat);
	}while(nick_repeat);

	if (added_nick) {
		contact_t temp;
		cts_ct_base base;
		temp.base = &base;
		temp.base->id = contact_id;
		temp.nicknames = added_nick;

		ret = cts_insert_contact_data(CTS_DATA_FIELD_NICKNAME, &temp);
		g_slist_free(added_nick);
		retvm_if(CTS_SUCCESS != ret, ret,
				"cts_insert_contact_data() Failed(%d)", ret);
	}
	return CTS_SUCCESS;
}

static inline int cts_update_contact_data_extend(
		cts_stmt stmt, int contact_id, GSList* extend_list)
{
	CTS_FN_CALL;

	int ret;
	GSList *added_extend=NULL, *extend_repeat = extend_list;
	cts_extend *extend_data;

	retv_if(NULL == stmt, CTS_ERR_ARG_NULL);
	retv_if(NULL == extend_list, CTS_ERR_ARG_NULL);

	do
	{
		if (NULL != (extend_data = (cts_extend *)extend_repeat->data))
		{
			if (!extend_data->id){
				if (!extend_data->deleted)
					added_extend = g_slist_append(added_extend, extend_data);
				extend_repeat = g_slist_next(extend_repeat);
				continue;
			}
			if (extend_data->deleted)
			{
				ret = cts_delete_record_by_id(CTS_TABLE_DATA, extend_data->id);
				retvm_if(ret, ret, "cts_delete_record_by_id() Failed(%d)", ret);
			}
			else
			{
				cts_stmt_bind_extend(stmt, 1, extend_data);
				cts_stmt_bind_int(stmt, CTS_UPDATE_ID_LOC, extend_data->id);

				ret = cts_stmt_step(stmt);
				retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);
				cts_stmt_reset(stmt);
			}
		}
		extend_repeat = g_slist_next(extend_repeat);
	}while(extend_repeat);

	if (added_extend) {
		contact_t temp;
		cts_ct_base base;
		temp.base = &base;
		temp.base->id = contact_id;
		temp.extended_values = added_extend;

		ret = cts_insert_contact_data(CTS_DATA_FIELD_EXTEND_ALL, &temp);
		g_slist_free(added_extend);
		retvm_if(CTS_SUCCESS != ret, ret,
				"cts_insert_contact_data() Failed(%d)", ret);
	}
	return CTS_SUCCESS;
}

static inline int cts_update_contact_data_name(cts_stmt stmt,
		cts_name *name)
{
	int ret, cnt=1;
	cts_name normalize_name={0};
	char *tmp_display, *tmp_first, *tmp_last;
	char display[CTS_SQL_MAX_LEN]={0};
	char lookup[CTS_SQL_MAX_LEN]={0};
	char reverse_lookup[CTS_SQL_MAX_LEN]={0};

	retvm_if(!name->id, CTS_ERR_ARG_INVALID, "name of contact has no ID.");
	cts_stmt_bind_int(stmt, CTS_UPDATE_ID_LOC, name->id);

	//Update name search info
	char normal_name[CTS_NN_MAX][CTS_SQL_MAX_LEN]={{0},{0},{0}};

	tmp_display = name->display;
	tmp_first = name->first;
	tmp_last = name->last;

	if (name->display) {
		ret = cts_normalize_name(name, normal_name, true);
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_name() Failed(%d)", ret);
		if (normal_name[CTS_NN_FIRST][0])
			normalize_name.display = normal_name[CTS_NN_FIRST];
		else
			name->display = NULL;
	}

	if (NULL == name->display) {
		ret = cts_normalize_name(name, normal_name, false);
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_name() Failed(%d)", ret);

		switch (ret)
		{
		case CTS_LANG_KOREAN:
			snprintf(display, sizeof(display), "%s%s",
					SAFE_STR(name->last), SAFE_STR(name->first));

			strncat(normal_name[CTS_NN_LAST], normal_name[CTS_NN_FIRST],
					sizeof(normal_name[CTS_NN_LAST]));

			name->display = display;
			normalize_name.display = normal_name[CTS_NN_LAST];
			break;
		case CTS_LANG_ENGLISH:
		default:
			if (normal_name[CTS_NN_FIRST][0])
				normalize_name.first = normal_name[CTS_NN_FIRST];
			else
				name->first = NULL;

			if (normal_name[CTS_NN_LAST][0])
				normalize_name.last = normal_name[CTS_NN_LAST];
			else
				name->last = NULL;

			break;
		}
	}

	if (cts_get_default_language() == ret)
		cts_stmt_bind_int(stmt, cnt, CTS_LANG_DEFAULT);
	else
		cts_stmt_bind_int(stmt, cnt, ret);
	cnt = cts_stmt_bind_name(stmt, cnt, name);

	name->display = tmp_display;
	name->first = tmp_first;
	name->last = tmp_last;

	ret = cts_make_name_lookup(CTS_ORDER_NAME_FIRSTLAST, &normalize_name,
										lookup, sizeof(lookup));
	retvm_if(CTS_SUCCESS != ret, ret, "cts_make_name_lookup() Failed(%d)", ret);

	ret = cts_make_name_lookup(CTS_ORDER_NAME_LASTFIRST, &normalize_name,
										reverse_lookup, sizeof(reverse_lookup));
	retvm_if(CTS_SUCCESS != ret, ret, "cts_make_name_lookup() Failed(%d)", ret);

	CTS_DBG("lookup=%s(%d), reverse_lookup=%s(%d)",
			lookup, strlen(lookup), reverse_lookup, strlen(reverse_lookup));

	cts_stmt_bind_text(stmt, cnt++, lookup);
	cts_stmt_bind_text(stmt, cnt++, reverse_lookup);
	cts_stmt_bind_text(stmt, cnt, normal_name[CTS_NN_SORTKEY]);

	ret = cts_stmt_step(stmt);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_stmt_step() Failed(%d)", ret);

	return CTS_SUCCESS;
}

static inline int cts_update_contact_data(contact_t *contact)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	// Use stmt query for DATA table
	snprintf(query, sizeof(query), "UPDATE %s SET data1=?, data2=?, data3=?, data4=?,"
			"data5=?, data6=?, data7=?, data8=?, data9=?, data10=? WHERE id=?",
			CTS_TABLE_DATA);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	//Update the name
	if (contact->name && contact->name->is_changed)
	{
		ret = cts_update_contact_data_name(stmt, contact->name);
		if (CTS_SUCCESS != ret) {
			ERR("cts_update_contact_data_name() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
		cts_stmt_reset(stmt);
	}

	//Update the company
	if (contact->company)
	{
		cts_company *com = contact->company;
		if (!com->id) {
			ret = cts_insert_contact_data(CTS_DATA_FIELD_COMPANY, contact);
			retvm_if(CTS_SUCCESS != ret, ret,
					"cts_insert_contact_data(CTS_DATA_FIELD_COMPANY) Failed(%d)", ret);
		}
		else {
			if (com->name || com->department || com->jot_title || com->role || com->assistant_name) {
				cts_stmt_bind_company(stmt, 1, com);
				cts_stmt_bind_int(stmt, CTS_UPDATE_ID_LOC, com->id);

				ret = cts_stmt_step(stmt);
				if (CTS_SUCCESS != ret)
				{
					ERR("cts_stmt_step() Failed(%d)", ret);
					cts_stmt_finalize(stmt);
					return ret;
				}
				cts_stmt_reset(stmt);
			}
			else {
				ret = cts_delete_record_by_id(CTS_TABLE_DATA, com->id);
				retvm_if(ret, ret, "cts_delete_record_by_id() Failed(%d)", ret);
			}
		}
	}

	//Update the events
	if (contact->events)
	{
		ret = cts_update_contact_data_event(stmt, contact->base->id, contact->events);

		if (CTS_SUCCESS != ret)
		{
			ERR("cts_update_contact_data_event() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}

	//Update the messengers
	if (contact->messengers)
	{
		ret = cts_update_contact_data_messenger(stmt, contact->base->id,
				contact->messengers);

		if (CTS_SUCCESS != ret)
		{
			ERR("cts_update_contact_data_messenger() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}

	//Update the postals
	if (contact->postal_addrs)
	{
		ret = cts_update_contact_data_postal(stmt, contact->base->id,
				contact->postal_addrs);

		if (CTS_SUCCESS != ret)
		{
			ERR("cts_update_contact_data_postal() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}

	//Update the Web addrs
	if (contact->web_addrs)
	{
		ret = cts_update_contact_data_web(stmt, contact->base->id, contact->web_addrs);

		if (CTS_SUCCESS != ret)
		{
			ERR("cts_update_contact_data_web() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}

	//Update the Nick Names
	if (contact->nicknames)
	{
		ret = cts_update_contact_data_nick(stmt, contact->base->id, contact->nicknames);

		if (CTS_SUCCESS != ret)
		{
			ERR("cts_update_contact_data_nick() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}

	//Update the Numbers
	if (contact->numbers)
	{
		ret = cts_update_contact_data_number(stmt, contact->base->id, contact->numbers);

		if (ret < CTS_SUCCESS)
		{
			ERR("cts_update_contact_data_number() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
		contact->default_num = ret;
	}

	//Update the Emails
	if (contact->emails)
	{
		ret = cts_update_contact_data_email(stmt, contact->base->id, contact->emails);

		if (ret < CTS_SUCCESS)
		{
			ERR("cts_update_contact_data_email() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
		contact->default_email = ret;
	}

	//Update the extended values
	if (contact->extended_values)
	{
		ret = cts_update_contact_data_extend(stmt, contact->base->id,
				contact->extended_values);

		if (CTS_SUCCESS != ret)
		{
			ERR("cts_update_contact_data_extend() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			return ret;
		}
	}

	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}


static inline void cts_update_contact_handle_no_name(contact_t *contact)
{
	if (NULL == contact->name) {
		contact->name = calloc(1, sizeof(cts_name));
		contact->name->embedded = true;
	} else if (contact->name->first || contact->name->last) {
		return;
	}

	if (contact->name->display) {
		free(contact->name->display);
		contact->name->display = NULL;
	}
	contact->name->is_changed = true;

	if (contact->company && contact->company->name) {
		contact->name->display = strdup(contact->company->name);
	} else {
		char *temp;

		temp = cts_contact_get_valid_first_number_or_email(contact->numbers, contact->emails);
		contact->name->display = SAFE_STRDUP(temp);
	}
}


static inline int cts_update_contact(contact_t *contact)
{
	int i, ret, len;
	char query[CTS_SQL_MAX_LEN] = {0};
	char *img_path;

	snprintf(query, sizeof(query),
			"SELECT count(contact_id) FROM %s WHERE contact_id = %d",
			CTS_TABLE_CONTACTS, contact->base->id);
	ret = cts_query_get_first_int_result(query);
	retvm_if(1 != ret, CTS_ERR_DB_RECORD_NOT_FOUND,
			"The index(%d) is Invalid. %d Record(s) is(are) found", contact->base->id, ret);

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	cts_update_contact_handle_no_name(contact);
	cts_contact_remove_dup_data(contact);

	//update data
	ret = cts_update_contact_data(contact);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_update_contact_data() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	//update group relation Info
	if (contact->grouprelations)
	{
		ret = cts_update_contact_grouprel(contact->base, contact->grouprelations);
		if (ret < CTS_SUCCESS)
		{
			ERR("cts_update_contact_grouprel() Failed(%d)", ret);
			contacts_svc_end_trans(false);
			return ret;
		}
	}

	// default setting
	if (!contact->default_num || !contact->default_email) {
		ret = cts_set_first_id_for_default(contact);
		if (CTS_SUCCESS != ret) {
			ERR("cts_set_first_id_for_default() Failed(%d)", ret);
			contacts_svc_end_trans(false);
			return ret;
		}
	}
	len = snprintf(query, sizeof(query),
			"UPDATE %s SET changed_ver=%d, changed_time=%d, "
			"default_num=%d, default_email=%d",
			CTS_TABLE_CONTACTS, cts_get_next_ver(), (int)time(NULL),
			contact->default_num, contact->default_email);

	if (contact->base->uid_changed)
		len += snprintf(query+len, sizeof(query)-len, ", uid=?");

	if (contact->base->ringtone_changed)
		len += snprintf(query+len, sizeof(query)-len, ", ringtone=?");

	if (contact->base->note_changed)
		len += snprintf(query+len, sizeof(query)-len, ", note=?");

	if (contact->base->img_changed ||
			(NULL == contact->base->img_path && contact->base->vcard_img_path))
		len += snprintf(query+len, sizeof(query)-len, ", image0=?");

	if (contact->base->full_img_changed)
		len += snprintf(query+len, sizeof(query)-len, ", image1=?");

	snprintf(query+len, sizeof(query)-len,
			" WHERE contact_id=%d", contact->base->id);

	cts_stmt stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		ERR("cts_query_prepare() Failed");
		contacts_svc_end_trans(false);
		return CTS_ERR_DB_FAILED;
	}

	i=1;
	if (contact->base->uid_changed) {
		if (contact->base->uid)
			cts_stmt_bind_text(stmt, i, contact->base->uid);
		i++;
	}
	if (contact->base->ringtone_changed) {
		if (contact->base->ringtone_path)
			cts_stmt_bind_text(stmt, i, contact->base->ringtone_path);
		i++;
	}
	if (contact->base->note_changed) {
		if (contact->base->note)
			cts_stmt_bind_text(stmt, i, contact->base->note);
		i++;
	}

	if (contact->base->img_changed ||
			(NULL == contact->base->img_path && contact->base->vcard_img_path)) {
		if (contact->base->img_path) {
			ret = cts_update_image_file(CTS_IMG_NORMAL, contact->base->id, contact->base->img_path, &img_path);
			if (CTS_SUCCESS != ret) {
				ERR("cts_update_image_file() Failed(%d)", ret);
				cts_stmt_finalize(stmt);
				contacts_svc_end_trans(false);
				return ret;
			}
			if (img_path)
				cts_stmt_bind_text(stmt, i, img_path);
			i++;
		} else {
			if (contact->base->vcard_img_path) {
				ret = cts_update_image_file(CTS_IMG_NORMAL, contact->base->id, contact->base->vcard_img_path, &img_path);
				if (CTS_SUCCESS == ret && img_path)
					cts_stmt_bind_text(stmt, i, img_path);
				i++;
			} else {
				ret = cts_delete_image_file(CTS_IMG_NORMAL, contact->base->id);
				if (CTS_SUCCESS != ret) {
					ERR("cts_delete_image_file() Failed(%d)", ret);
					cts_stmt_finalize(stmt);
					contacts_svc_end_trans(false);
					return ret;
				}
			}
		}
	}

	if (contact->base->full_img_changed) {
		ret = cts_update_image_file(CTS_IMG_FULL, contact->base->id, contact->base->full_img_path, &img_path);
		if (CTS_SUCCESS == ret && img_path)
			cts_stmt_bind_text(stmt, i, img_path);
		i++;
	}

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		contacts_svc_end_trans(false);
		return ret;
	}
	cts_stmt_finalize(stmt);

	cts_set_contact_noti();

	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

API int contacts_svc_update_contact(CTSstruct* contact)
{
	CTS_FN_CALL;
	int ret;

	retv_if(NULL == contact, CTS_ERR_ARG_NULL);
	retvm_if(CTS_STRUCT_CONTACT != contact->s_type, CTS_ERR_ARG_INVALID,
			"The contact(%d) must be type of CTS_STRUCT_CONTACT.", contact->s_type);

	CTS_START_TIME_CHECK;

	ret = cts_update_contact((contact_t *)contact);

	CTS_END_TIME_CHECK();
	return ret;
}

static inline int cts_put_base_val(int op_code, int contact_id,
		cts_ct_base *value)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MIN_LEN] = {0};

	retvm_if(CTS_VALUE_CONTACT_BASE_INFO != value->v_type, CTS_ERR_ARG_INVALID,
			"value has unknown type");
	switch (op_code) {
	case CTS_PUT_VAL_REPLACE_RINGTONE:
		snprintf(query, sizeof(query),
				"UPDATE %s SET changed_ver=%d, changed_time=%d, "
				"ringtone=? WHERE contact_id=%d",
				CTS_TABLE_CONTACTS, cts_get_next_ver(), (int)time(NULL),
				contact_id);

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

		if (value->ringtone_path)
			cts_stmt_bind_text(stmt, 1, value->ringtone_path);
		break;
	case CTS_PUT_VAL_REPLACE_NOTE:
		snprintf(query, sizeof(query),
				"UPDATE %s SET changed_ver=%d, changed_time=%d, "
				"note=? WHERE contact_id=%d",
				CTS_TABLE_CONTACTS, cts_get_next_ver(), (int)time(NULL),
				contact_id);

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

		if (value->note)
			cts_stmt_bind_text(stmt, 1, value->note);
		break;
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}
	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}

static inline int cts_put_number_val(int contact_id,
		cts_number *number)
{
	int ret, default_id;
	cts_stmt stmt = NULL;
	char clean_num[CTS_NUMBER_MAX_LEN], query[CTS_SQL_MAX_LEN] = {0};
	const char *normal_num;

	retv_if(NULL == number->number, CTS_ERR_ARG_NULL);
	retvm_if(CTS_VALUE_NUMBER != number->v_type, CTS_ERR_ARG_INVALID,
			"value has unknown type");

	ret = cts_clean_number(number->number, clean_num, sizeof(clean_num));
	retvm_if(ret <= 0, CTS_ERR_ARG_INVALID, "Number(%s) is invalid", number->number);

	snprintf(query, sizeof(query),
			"INSERT INTO %s(contact_id, datatype, data1, data2, data3) VALUES(%d, %d, %d, ?, ?)",
			CTS_TABLE_DATA, contact_id, CTS_DATA_NUMBER, number->type);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	cts_stmt_bind_text(stmt, 1, clean_num);

	normal_num = cts_normalize_number(clean_num);
	cts_stmt_bind_text(stmt, 2, normal_num);

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret) {
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}
	cts_stmt_finalize(stmt);

	if (number->is_default) {
		default_id = cts_db_get_last_insert_id();
		snprintf(query, sizeof(query),
				"UPDATE %s SET changed_ver=%d, changed_time=%d, default_num=%d "
				"WHERE contact_id=%d",
				CTS_TABLE_CONTACTS, cts_get_next_ver(), (int)time(NULL),
				default_id, contact_id);
	} else {
		snprintf(query, sizeof(query),
				"UPDATE %s SET changed_ver=%d, changed_time=%d WHERE contact_id=%d",
				CTS_TABLE_CONTACTS, cts_get_next_ver(), (int)time(NULL),
				contact_id);
	}

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		return ret;
	}

	return CTS_SUCCESS;
}

static inline int cts_put_email_val(int contact_id,
		cts_email *email)
{
	int ret, default_id;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	retv_if(NULL == email->email_addr, CTS_ERR_ARG_NULL);
	retvm_if(CTS_VALUE_EMAIL != email->v_type, CTS_ERR_ARG_INVALID,
			"value has unknown type");

	snprintf(query, sizeof(query),
			"INSERT INTO %s(contact_id, datatype, data1, data2) VALUES(%d, %d, %d, ?)",
			CTS_TABLE_DATA, contact_id, CTS_DATA_EMAIL, email->type);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	if (email->email_addr)
		cts_stmt_bind_text(stmt, 1, email->email_addr);

	ret = cts_stmt_step(stmt);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}
	cts_stmt_finalize(stmt);

	if (email->is_default) {
		default_id = cts_db_get_last_insert_id();
		snprintf(query, sizeof(query),
				"UPDATE %s SET changed_ver=%d, changed_time=%d, default_email=%d "
				"WHERE contact_id=%d",
				CTS_TABLE_CONTACTS, cts_get_next_ver(), (int)time(NULL),
				default_id, contact_id);
	} else {
		snprintf(query, sizeof(query),
				"UPDATE %s SET changed_ver=%d, changed_time=%d WHERE contact_id=%d",
				CTS_TABLE_CONTACTS, cts_get_next_ver(), (int)time(NULL),
				contact_id);
	}

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret) {
		ERR("cts_query_exec() Failed(%d)", ret);
		return ret;
	}

	return CTS_SUCCESS;
}

API int contacts_svc_put_contact_value(cts_put_contact_val_op op_code,
		int contact_id, CTSvalue* value)
{
	CTS_FN_CALL;

	int ret;

	retv_if(NULL == value, CTS_ERR_ARG_NULL);
	CTS_START_TIME_CHECK;

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	switch (op_code)
	{
	case CTS_PUT_VAL_REPLACE_RINGTONE:
	case CTS_PUT_VAL_REPLACE_NOTE:
		ret = cts_put_base_val(op_code, contact_id, (cts_ct_base *)value);
		if (CTS_SUCCESS != ret)
		{
			ERR("cts_update_contact_val_ringtone() Failed(%d)", ret);
			contacts_svc_end_trans(false);
			return ret;
		}
		break;
	case CTS_PUT_VAL_ADD_NUMBER:
		ret = cts_put_number_val(contact_id, (cts_number *)value);
		if (CTS_SUCCESS != ret)
		{
			ERR("cts_put_number_val() Failed(%d)", ret);
			contacts_svc_end_trans(false);
			return ret;
		}
		break;
	case CTS_PUT_VAL_ADD_EMAIL:
		ret = cts_put_email_val(contact_id, (cts_email *)value);
		if (CTS_SUCCESS != ret)
		{
			ERR("cts_put_email_val() Failed(%d)", ret);
			contacts_svc_end_trans(false);
			return ret;
		}
		break;
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
		contacts_svc_end_trans(false);
		return CTS_ERR_ARG_INVALID;
	}

	cts_set_contact_noti();

	ret = contacts_svc_end_trans(true);
	CTS_END_TIME_CHECK();
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

#define CTS_MAIN_CTS_GET_RINGTON (1<<1)
#define CTS_MAIN_CTS_GET_NOTE (1<<2)
#define CTS_MAIN_CTS_GET_DEFAULT_NUM (1<<3)
#define CTS_MAIN_CTS_GET_DEFAULT_EMAIL (1<<4)
#define CTS_MAIN_CTS_GET_FAVOR (1<<5)
#define CTS_MAIN_CTS_GET_IMG (1<<6)
#define CTS_MAIN_CTS_GET_ALL (1<<1|1<<2|1<<3|1<<4|1<<5|1<<6)

static int cts_get_main_contacts_info(int op_code, int index, contact_t *contact)
{
	int ret, len;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	char *temp;

	len = snprintf(query, sizeof(query), "SELECT ");

	len += snprintf(query+len, sizeof(query)-len,
			"contact_id, addrbook_id, changed_time");

	if (op_code & CTS_MAIN_CTS_GET_RINGTON)
		len += snprintf(query+len, sizeof(query)-len, ", ringtone");
	if (op_code & CTS_MAIN_CTS_GET_NOTE)
		len += snprintf(query+len, sizeof(query)-len, ", note");
	if (op_code & CTS_MAIN_CTS_GET_DEFAULT_NUM)
		len += snprintf(query+len, sizeof(query)-len, ", default_num");
	if (op_code & CTS_MAIN_CTS_GET_DEFAULT_EMAIL)
		len += snprintf(query+len, sizeof(query)-len, ", default_email");
	if (op_code & CTS_MAIN_CTS_GET_FAVOR)
		len += snprintf(query+len, sizeof(query)-len, ", is_favorite");
	if (op_code & CTS_MAIN_CTS_GET_IMG) {
		len += snprintf(query+len, sizeof(query)-len, ", image0");
		len += snprintf(query+len, sizeof(query)-len, ", image1");
	}

	snprintf(query+len, sizeof(query)-len,
			" FROM %s WHERE contact_id = %d", CTS_TABLE_CONTACTS, index);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}
	int count=0;

	contact->base = (cts_ct_base *)contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);
	if (NULL == contact->base) {
		cts_stmt_finalize(stmt);
		ERR("contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO) Failed");
		return CTS_ERR_OUT_OF_MEMORY;
	}

	contact->base->id = cts_stmt_get_int(stmt, count++);
	contact->base->addrbook_id = cts_stmt_get_int(stmt, count++);
	contact->base->changed_time = cts_stmt_get_int(stmt, count++);

	if (op_code & CTS_MAIN_CTS_GET_RINGTON)
	{
		contact->base->embedded = true;
		temp = cts_stmt_get_text(stmt, count++);
		if (temp && CTS_SUCCESS == cts_exist_file(temp))
			contact->base->ringtone_path = strdup(temp);
		else
			contact->base->ringtone_path = NULL;
	}
	if (op_code & CTS_MAIN_CTS_GET_NOTE)
	{
		contact->base->embedded = true;
		temp = cts_stmt_get_text(stmt, count++);
		contact->base->note = SAFE_STRDUP(temp);
	}
	if (op_code & CTS_MAIN_CTS_GET_DEFAULT_NUM)
		contact->default_num = cts_stmt_get_int(stmt, count++);
	if (op_code & CTS_MAIN_CTS_GET_DEFAULT_EMAIL)
		contact->default_email = cts_stmt_get_int(stmt, count++);
	if (op_code & CTS_MAIN_CTS_GET_FAVOR)
		contact->base->is_favorite = cts_stmt_get_int(stmt, count++);

	if (op_code & CTS_MAIN_CTS_GET_IMG) {
		contact->base->embedded = true;
		temp = cts_stmt_get_text(stmt, count++);
		contact->base->img_path = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, count++);
		contact->base->full_img_path = SAFE_STRDUP(temp);
	}

	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}

static inline int cts_get_data_info_number(cts_stmt stmt, contact_t *contact)
{
	cts_number *result;

	result = (cts_number *)contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (result) {
		int cnt = 1;
		result->embedded = true;
		cnt = cts_stmt_get_number(stmt, result, cnt);

		if (result->id == contact->default_num)
			result->is_default = true;

		result->is_favorite = cts_stmt_get_int(stmt, cnt);
		contact->numbers = g_slist_append(contact->numbers, result);
	}
	return CTS_SUCCESS;
}

static inline int cts_get_data_info_email(cts_stmt stmt, contact_t *contact)
{
	cts_email *result;

	result = (cts_email *)contacts_svc_value_new(CTS_VALUE_EMAIL);
	if (result) {
		result->embedded = true;
		cts_stmt_get_email(stmt, result, 1);

		if (result->id == contact->default_email)
			result->is_default = true;

		contact->emails = g_slist_append(contact->emails, result);
	}
	return CTS_SUCCESS;
}

static inline cts_name* cts_get_data_info_name(cts_stmt stmt)
{
	cts_name *result;

	result = (cts_name *)contacts_svc_value_new(CTS_VALUE_NAME);
	if (result) {
		result->embedded = true;
		cts_stmt_get_name(stmt, result, 1);
	}
	return result;
}

static inline int cts_get_data_info_event(cts_stmt stmt, contact_t *contact)
{
	int cnt=1;
	cts_event *result;

	result = (cts_event *)contacts_svc_value_new(CTS_VALUE_EVENT);
	if (result) {
		result->embedded = true;
		result->id = cts_stmt_get_int(stmt, cnt++);
		result->type = cts_stmt_get_int(stmt, cnt++);
		result->date = cts_stmt_get_int(stmt, cnt++);

		contact->events = g_slist_append(contact->events, result);
	}
	return CTS_SUCCESS;
}

static inline int cts_get_data_info_messenger(cts_stmt stmt, contact_t *contact)
{
	int cnt=1;
	cts_messenger *result;

	result = (cts_messenger *)contacts_svc_value_new(CTS_VALUE_MESSENGER);
	if (result) {
		char *temp;
		result->embedded = true;
		result->id = cts_stmt_get_int(stmt, cnt++);
		result->type = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->im_id = SAFE_STRDUP(temp);

		contact->messengers = g_slist_append(contact->messengers, result);
	}
	return CTS_SUCCESS;
}

static inline int cts_get_data_info_postal(cts_stmt stmt, contact_t *contact)
{
	int cnt=1;
	cts_postal *result;

	result = (cts_postal *)contacts_svc_value_new(CTS_VALUE_POSTAL);
	if (result) {
		char *temp;
		result->embedded = true;
		result->id = cts_stmt_get_int(stmt, cnt++);
		result->type = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->pobox= SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->postalcode = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->region= SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->locality = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->street = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->extended = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->country = SAFE_STRDUP(temp);

		contact->postal_addrs = g_slist_append(contact->postal_addrs, result);
	}
	return CTS_SUCCESS;
}

static inline int cts_get_data_info_web(cts_stmt stmt, contact_t *contact)
{
	int cnt=1;
	cts_web *result;

	result = (cts_web *)contacts_svc_value_new(CTS_VALUE_WEB);
	if (result) {
		char *temp;
		result->embedded = true;
		result->id = cts_stmt_get_int(stmt, cnt++);
		result->type = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->url = SAFE_STRDUP(temp);

		contact->web_addrs = g_slist_append(contact->web_addrs, result);
	}
	return CTS_SUCCESS;
}

static inline int cts_get_data_info_nick(cts_stmt stmt, contact_t *contact)
{
	int cnt=1;
	cts_nickname *result;

	result = (cts_nickname *)contacts_svc_value_new(CTS_VALUE_NICKNAME);
	if (result) {
		char *temp;
		result->embedded = true;
		result->id = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt+1);
		result->nick = SAFE_STRDUP(temp);

		contact->nicknames = g_slist_append(contact->nicknames, result);
	}
	return CTS_SUCCESS;
}

static inline cts_company* cts_get_data_info_company(cts_stmt stmt)
{
	int cnt=1;
	cts_company *result;

	result = (cts_company *)contacts_svc_value_new(CTS_VALUE_COMPANY);
	retvm_if(NULL == result, NULL, "contacts_svc_value_new() Failed");

	char *temp;
	result->embedded = true;
	result->id = cts_stmt_get_int(stmt, cnt++);
	cnt++;
	temp = cts_stmt_get_text(stmt, cnt++);
	result->name = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, cnt++);
	result->department = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, cnt++);
	result->jot_title = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, cnt++);
	result->role = SAFE_STRDUP(temp);
	temp = cts_stmt_get_text(stmt, cnt++);
	result->assistant_name = SAFE_STRDUP(temp);

	if (result->name || result->department || result->jot_title || result->role ||  result->assistant_name)
		return result;
	else {
		contacts_svc_value_free((CTSvalue *)result);
		return NULL;
	}
}

static cts_extend* cts_make_extend_data(cts_stmt stmt, int type, int cnt)
{
	cts_extend *result;
	result = (cts_extend *)contacts_svc_value_new(CTS_VALUE_EXTEND);
	if (result)
	{
		char *temp;
		result->type = type;
		result->id = cts_stmt_get_int(stmt, cnt++);
		result->data1 = cts_stmt_get_int(stmt, cnt++);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data2= SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data3 = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data4= SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data5 = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data6 = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data7 = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data8 = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data9 = SAFE_STRDUP(temp);
		temp = cts_stmt_get_text(stmt, cnt++);
		result->data10 = SAFE_STRDUP(temp);
	}
	return result;
}

static inline int cts_get_data_info_extend(cts_stmt stmt, int type,
		contact_t *contact)
{
	cts_extend *result;

	result = cts_make_extend_data(stmt, type, 1);
	if (result) {
		result->embedded = true;
		contact->extended_values = g_slist_append(contact->extended_values, result);
	}
	else
		return CTS_ERR_OUT_OF_MEMORY;

	return CTS_SUCCESS;
}

enum{
	CTS_GET_DATA_BY_CONTACT_ID,
	CTS_GET_DATA_BY_ID
};

static int cts_get_data_info(int op_code, int field, int index, contact_t *contact)
{
	int ret, datatype, len;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	switch (op_code)
	{
	case CTS_GET_DATA_BY_CONTACT_ID:
		len = snprintf(query, sizeof(query), "SELECT datatype, id, data1, data2,"
				"data3, data4, data5, data6, data7, data8, data9, data10 "
				"FROM %s WHERE contact_id = %d", CTS_TABLE_DATA, index);
		break;
	case CTS_GET_DATA_BY_ID:
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	if (CTS_DATA_FIELD_ALL != field && CTS_DATA_FIELD_EXTEND_ALL != field)
	{
		bool first= true;
		len += snprintf(query+len, sizeof(query)-len, " AND datatype IN (");

		if (field & CTS_DATA_FIELD_NAME) {
			first=false;
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_NAME);
		}
		if (field & CTS_DATA_FIELD_EVENT) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_EVENT);
		}
		if (field & CTS_DATA_FIELD_MESSENGER) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_MESSENGER);
		}
		if (field & CTS_DATA_FIELD_POSTAL) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_POSTAL);
		}
		if (field & CTS_DATA_FIELD_WEB) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_WEB);
		}
		if (field & CTS_DATA_FIELD_NICKNAME) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_NICKNAME);
		}
		if (field & CTS_DATA_FIELD_COMPANY) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_COMPANY);
		}
		if (field & CTS_DATA_FIELD_NUMBER) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_NUMBER);
		}
		if (field & CTS_DATA_FIELD_EMAIL) {
			if (first)
				first=false;
			else
				len += snprintf(query+len, sizeof(query)-len, ", ");
			len += snprintf(query+len, sizeof(query)-len, "%d", CTS_DATA_EMAIL);
		}

		len += snprintf(query+len, sizeof(query)-len, ")");
	}

	if (CTS_DATA_FIELD_ALL != field && field & CTS_DATA_FIELD_EXTEND_ALL) {
		len += snprintf(query+len, sizeof(query)-len, " AND datatype>=%d",
				CTS_DATA_EXTEND_START);
	}

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	do {
		datatype = cts_stmt_get_int(stmt, 0);

		switch (datatype)
		{
		case CTS_DATA_NAME:
			if (contact->name)
				ERR("name already Exist");
			else
				contact->name = cts_get_data_info_name(stmt);
			break;
		case CTS_DATA_EVENT:
			cts_get_data_info_event(stmt, contact);
			break;
		case CTS_DATA_MESSENGER:
			cts_get_data_info_messenger(stmt, contact);
			break;
		case CTS_DATA_POSTAL:
			cts_get_data_info_postal(stmt, contact);
			break;
		case CTS_DATA_WEB:
			cts_get_data_info_web(stmt, contact);
			break;
		case CTS_DATA_NICKNAME:
			cts_get_data_info_nick(stmt, contact);
			break;
		case CTS_DATA_NUMBER:
			cts_get_data_info_number(stmt, contact);
			break;
		case CTS_DATA_EMAIL:
			cts_get_data_info_email(stmt, contact);
			break;
		case CTS_DATA_COMPANY:
			if (contact->company)
				ERR("company already Exist");
			else
				contact->company = cts_get_data_info_company(stmt);
			break;
		default:
			if (CTS_DATA_EXTEND_START <= datatype) {
				cts_get_data_info_extend(stmt, datatype, contact);
				break;
			}
			ERR("Unknown data type(%d)", datatype);
			continue;
		}
	}while(CTS_TRUE == cts_stmt_step(stmt));

	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}

static inline int cts_get_groups_info(int index, contact_t *contact)
{
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	GSList *result_list=NULL;

	snprintf(query, sizeof(query), "SELECT group_id, addrbook_id,"
			" group_name"
			" FROM %s WHERE group_id IN (SELECT group_id"
			" FROM %s WHERE contact_id = %d)"
			" ORDER BY group_name COLLATE NOCASE",
			CTS_TABLE_GROUPS, CTS_TABLE_GROUPING_INFO, index);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	while (CTS_TRUE == cts_stmt_step(stmt))
	{
		cts_group *group_info;
		group_info = (cts_group *)contacts_svc_value_new(CTS_VALUE_GROUP_RELATION);

		if (group_info)
		{
			group_info->id = cts_stmt_get_int(stmt, 0);
			group_info->addrbook_id = cts_stmt_get_int(stmt, 1);
			group_info->embedded = true;
			group_info->name = SAFE_STRDUP(cts_stmt_get_text(stmt, 2));

			result_list = g_slist_append(result_list, group_info);
		}
	}

	cts_stmt_finalize(stmt);
	contact->grouprelations = result_list;

	return CTS_SUCCESS;

}

static inline int cts_get_number_value(int op_code, int id, CTSvalue **value)
{
	int ret;
	cts_stmt stmt;
	cts_number *number;
	char query[CTS_SQL_MAX_LEN] = {0};

	if (CTS_GET_DEFAULT_NUMBER_VALUE == op_code) {
		snprintf(query, sizeof(query),
				"SELECT B.id, B.data1, B.data2 FROM %s A, %s B "
				"WHERE A.contact_id = %d AND B.id=A.default_num AND B.datatype = %d",
				CTS_TABLE_CONTACTS, CTS_TABLE_DATA, id, CTS_DATA_NUMBER);
	}
	else if (CTS_GET_NUMBER_VALUE == op_code) {
		snprintf(query, sizeof(query),
				"SELECT id, data1, data2, contact_id FROM %s "
				"WHERE id = %d AND datatype = %d",
				CTS_TABLE_DATA, id, CTS_DATA_NUMBER);
	}
	else {
		ERR("Invalid op_code(%d)", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	number = (cts_number *)contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number) {
		ret = CTS_SUCCESS;
		number->v_type = CTS_VALUE_RDONLY_NUMBER;
		number->embedded = true;
		cts_stmt_get_number(stmt, number, 0);

		if (CTS_GET_DEFAULT_NUMBER_VALUE == op_code)
			number->is_default = true;
		else
			ret = cts_stmt_get_int(stmt, 3);

		*value = (CTSvalue*) number;

		cts_stmt_finalize(stmt);
		return ret;
	}
	else {
		ERR("contacts_svc_value_new() Failed");
		cts_stmt_finalize(stmt);
		return CTS_ERR_OUT_OF_MEMORY;
	}
}

static inline int cts_get_email_value(int op_code, int id, CTSvalue **value)
{
	int ret;
	cts_stmt stmt;
	cts_email *email;
	char query[CTS_SQL_MAX_LEN] = {0};

	if (CTS_GET_DEFAULT_EMAIL_VALUE == op_code) {
		snprintf(query, sizeof(query),
				"SELECT B.id, B.data1, B.data2 FROM %s A, %s B "
				"WHERE A.contact_id = %d AND B.id=A.default_email AND B.datatype = %d",
				CTS_TABLE_CONTACTS, CTS_TABLE_DATA, id, CTS_DATA_EMAIL);
	}
	else if (CTS_GET_EMAIL_VALUE == op_code) {
		snprintf(query, sizeof(query),
				"SELECT id, data1, data2, contact_id FROM %s "
				"WHERE id = %d AND datatype = %d",
				CTS_TABLE_DATA, id, CTS_DATA_EMAIL);
	}
	else {
		ERR("Invalid op_code(%d)", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	email = (cts_email *)contacts_svc_value_new(CTS_VALUE_EMAIL);
	if (email)
	{
		ret = CTS_SUCCESS;
		email->v_type = CTS_VALUE_RDONLY_EMAIL;
		email->embedded = true;
		cts_stmt_get_email(stmt, email, 0);

		if (CTS_GET_DEFAULT_EMAIL_VALUE == op_code)
			email->is_default = true;
		else
			ret = cts_stmt_get_int(stmt, 3);

		*value = (CTSvalue*) email;

		cts_stmt_finalize(stmt);
		return ret;
	}
	else
	{
		ERR("contacts_svc_value_new() Failed");
		cts_stmt_finalize(stmt);
		return CTS_ERR_OUT_OF_MEMORY;
	}
}

static inline int cts_get_extend_data(int type, int id, CTSvalue **value)
{
	int ret;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query), "SELECT id, data1, data2,"
			"data3, data4, data5, data6, data7, data8, data9, data10 "
			"FROM %s WHERE datatype = %d AND contact_id = %d", CTS_TABLE_DATA, type, id);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	*value = (CTSvalue *)cts_make_extend_data(stmt, type, 0);
	cts_stmt_finalize(stmt);

	retvm_if(NULL == *value, CTS_ERR_OUT_OF_MEMORY, "cts_make_extend_data() return NULL");

	return CTS_SUCCESS;
}

API int contacts_svc_get_contact_value(cts_get_contact_val_op op_code,
		int id, CTSvalue **value)
{
	int ret;
	contact_t temp={0};

	retv_if(NULL == value, CTS_ERR_ARG_NULL);
	CTS_START_TIME_CHECK;

	if ((int)CTS_DATA_EXTEND_START <= op_code) {
		ret = cts_get_extend_data(op_code, id, value);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_get_extend_data() Failed(%d)", ret);
	}
	else {
		switch (op_code)
		{
		case CTS_GET_NAME_VALUE:
			ret = cts_get_data_info(CTS_GET_DATA_BY_CONTACT_ID,
					CTS_DATA_FIELD_NAME, id, &temp);
			retvm_if(CTS_SUCCESS != ret, ret,
					"cts_get_data_info(CTS_GET_DATA_BY_CONTACT_ID) Failed(%d)", ret);
			if (temp.name) {
				temp.name->v_type = CTS_VALUE_RDONLY_NAME;
				*value = (CTSvalue *)temp.name;
			}else
				*value = NULL;
			break;
		case CTS_GET_DEFAULT_NUMBER_VALUE:
		case CTS_GET_NUMBER_VALUE:
			ret = cts_get_number_value(op_code, id, value);
			retvm_if(ret < CTS_SUCCESS, ret,
					"cts_get_number_value() Failed(%d)", ret);
			break;
		case CTS_GET_DEFAULT_EMAIL_VALUE:
		case CTS_GET_EMAIL_VALUE:
			ret = cts_get_email_value(op_code, id, value);
			retvm_if(ret < CTS_SUCCESS, ret, "cts_get_email_value() Failed(%d)", ret);
			break;
		case CTS_GET_COMPANY_VALUE:
			ret = cts_get_data_info(CTS_GET_DATA_BY_CONTACT_ID,
					CTS_DATA_FIELD_COMPANY, id, &temp);
			retvm_if(CTS_SUCCESS != ret, ret,
					"cts_get_data_info(CTS_DATA_FIELD_COMPANY) Failed(%d)", ret);
			if (temp.company) {
				temp.company->v_type = CTS_VALUE_RDONLY_COMPANY;
				*value = (CTSvalue *)temp.company;
			}else
				*value = NULL;
			break;
		default:
			ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
			return CTS_ERR_ARG_INVALID;
		}
	}
	if (NULL == *value) return CTS_ERR_NO_DATA;

	CTS_END_TIME_CHECK();
	return ret;
}

API int contacts_svc_get_contact(int index, CTSstruct **contact)
{
	int ret;
	contact_t *record;

	retv_if(NULL == contact, CTS_ERR_ARG_NULL);
	CTS_START_TIME_CHECK;

	record = (contact_t *)contacts_svc_struct_new(CTS_STRUCT_CONTACT);

	ret = cts_get_main_contacts_info(CTS_MAIN_CTS_GET_ALL, index, record);

	if (CTS_SUCCESS != ret) {
		ERR("cts_get_main_contacts_info(ALL) Failed(%d)", ret);
		goto CTS_RETURN_ERROR;
	}

	ret = cts_get_data_info(CTS_GET_DATA_BY_CONTACT_ID,
			CTS_DATA_FIELD_ALL, index, record);
	if (CTS_SUCCESS != ret) {
		ERR("cts_get_data_info(CTS_GET_DATA_BY_CONTACT_ID) Failed(%d)", ret);
		goto CTS_RETURN_ERROR;
	}

	ret = cts_get_groups_info(index, record);
	if (CTS_SUCCESS != ret) {
		ERR("cts_get_group_info(CTS_GET_DATA_BY_CONTACT_ID) Failed(%d)", ret);
		goto CTS_RETURN_ERROR;
	}

	*contact = (CTSstruct *)record;

	CTS_END_TIME_CHECK();
	return CTS_SUCCESS;

CTS_RETURN_ERROR:
	contacts_svc_struct_free((CTSstruct *)record);
	return ret;
}

API int contacts_svc_find_contact_by(cts_find_op op_code,
		const char *user_data)
{
	int ret;
	const char *temp;
	char query[CTS_SQL_MAX_LEN] = {0};
	char normalized_val[CTS_SQL_MIN_LEN];

	CTS_START_TIME_CHECK;
	retv_if(NULL == user_data, CTS_ERR_ARG_NULL);

	switch (op_code)
	{
	case CTS_FIND_BY_NUMBER:
		ret = cts_clean_number(user_data, normalized_val, sizeof(normalized_val));
		retvm_if(ret <= 0, CTS_ERR_ARG_INVALID, "Number(%s) is invalid", user_data);

		temp = cts_normalize_number(normalized_val);
		snprintf(query, sizeof(query), "SELECT contact_id "
				"FROM %s WHERE datatype = %d AND data3 = '%s' LIMIT 1",
				CTS_TABLE_DATA, CTS_DATA_NUMBER, temp);
		ret = cts_query_get_first_int_result(query);
		break;
	case CTS_FIND_BY_EMAIL:
		snprintf(query, sizeof(query), "SELECT contact_id "
				"FROM %s WHERE datatype = %d AND data2 = '%s' LIMIT 1",
				CTS_TABLE_DATA, CTS_DATA_EMAIL, user_data);
		ret = cts_query_get_first_int_result(query);
		break;
	case CTS_FIND_BY_NAME:
		ret = cts_normalize_str(user_data, normalized_val, sizeof(normalized_val));
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_str() Failed(%d)", ret);

		if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			temp = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
		else
			temp = CTS_SCHEMA_DATA_NAME_LOOKUP;

		snprintf(query, sizeof(query), "SELECT contact_id FROM %s "
				"WHERE %s LIKE '%%%s%%' LIMIT 1",
				CTS_TABLE_DATA, temp, normalized_val);

		ret = cts_query_get_first_int_result(query);
		break;
	case CTS_FIND_BY_UID:
		snprintf(query, sizeof(query), "SELECT contact_id "
				"FROM %s WHERE uid = '%s' LIMIT 1", CTS_TABLE_CONTACTS, user_data);
		ret = cts_query_get_first_int_result(query);
		break;
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	CTS_END_TIME_CHECK();
	return ret;
}

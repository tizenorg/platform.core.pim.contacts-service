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
#include <unistd.h>
#include <errno.h>

#include "cts-internal.h"
#include "cts-list.h"
#include "cts-utils.h"

static contact_list *contact_list_mempool=NULL;
static plog_list *plog_list_mempool=NULL;
static change_list *change_list_mempool=NULL;
static numtype_list *numtype_list_mempool=NULL;
static shortcut_list *favorite_list_mempool=NULL;
static cts_group *group_list_mempool=NULL;
static cts_addrbook *addrbook_list_mempool=NULL;
static sdn_list *sdn_list_mempool=NULL;

API CTSstruct* contacts_svc_struct_new(cts_struct_type type)
{
	CTSstruct* ret_val;
	switch (type)
	{
	case CTS_STRUCT_CONTACT:
		ret_val = (CTSstruct*)calloc(1, sizeof(contact_t));
		if (ret_val) ret_val->s_type = CTS_STRUCT_CONTACT;
		return ret_val;
	default:
		ERR("your type is Not supported");
		return NULL;
	}
}

static void cts_number_free(gpointer data, gpointer user_data)
{
	if (NULL == data || !((cts_number*)data)->embedded)
		return;

	free(((cts_number*)data)->number);
	free(((cts_number*)data)->added_type);
	free(data);
}
static void cts_email_free(gpointer data, gpointer user_data)
{
	if (NULL == data || !((cts_email*)data)->embedded)
		return;

	free(((cts_email*)data)->email_addr);
	free(data);
}
static void cts_group_free(gpointer data, gpointer user_data)
{
	cts_group* data0 = (cts_group*)data;

	if (NULL == data || !data0->embedded)
		return;

	free(data0->name);
	free(data0->ringtone_path);
	free(data0->vcard_group);
	free(data);
}
static void cts_event_free(gpointer data, gpointer user_data)
{
	if (NULL == data || !((cts_event*)data)->embedded)
		return;

	free(data);
}
static void cts_messenger_free(gpointer data, gpointer user_data)
{
	cts_messenger *data0 = (cts_messenger *)data;

	if (NULL == data0 || !data0->embedded)
		return;

	free(data0->im_id);
	free(data0->svc_name);
	free(data0->svc_op);
	free(data);
}
static void cts_postal_free(gpointer data, gpointer user_data)
{
	cts_postal *data0 = (cts_postal *)data;

	if (NULL == data0 || !data0->embedded)
		return;

	free(data0->pobox);
	free(data0->postalcode);
	free(data0->region);
	free(data0->locality);
	free(data0->street);
	free(data0->extended);
	free(data0->country);
	free(data);
}
static void cts_web_free(gpointer data, gpointer user_data)
{
	if (NULL == data || !((cts_web*)data)->embedded)
		return;

	free(((cts_web*)data)->url);
	free(data);
}
static void cts_nickname_free(gpointer data, gpointer user_data)
{
	if (NULL == data || !((cts_nickname*)data)->embedded)
		return;

	free(((cts_nickname*)data)->nick);
	free(data);
}

static void cts_extend_free(gpointer data, gpointer user_data)
{
	cts_extend *data0 = (cts_extend *)data;
	if (NULL == data0 || !data0->embedded)
		return;

	free(data0->data2);
	free(data0->data3);
	free(data0->data4);
	free(data0->data5);
	free(data0->data6);
	free(data0->data7);
	free(data0->data8);
	free(data0->data9);
	free(data0->data10);
	free(data);
}

static inline void cts_name_free(cts_name *name)
{
	if (!name->embedded)
		return;

	free(name->first);
	free(name->last);
	free(name->addition);
	free(name->display);
	free(name->prefix);
	free(name->suffix);
	free(name);
}

static inline void cts_company_free(cts_company *company)
{
	if (!company->embedded)
		return;

	free(company->name);
	free(company->department);
	free(company->jot_title);
	free(company->role);
	free(company->assistant_name);
	free(company);
}

static inline void cts_contact_free(contact_t *contact)
{
	if (contact->base && contact->base->embedded) {
		free(contact->base->uid);
		free(contact->base->img_path);
		free(contact->base->full_img_path);
		free(contact->base->ringtone_path);
		free(contact->base->note);

		if (contact->base->vcard_img_path) {
			unlink(contact->base->vcard_img_path);
			free(contact->base->vcard_img_path);
		}

		free(contact->base);
	}

	if (contact->name)
		cts_name_free(contact->name);

	if (contact->company)
		cts_company_free(contact->company);

	if (contact->numbers) {
		g_slist_foreach(contact->numbers, cts_number_free, NULL);
		g_slist_free(contact->numbers);
	}

	if (contact->emails) {
		g_slist_foreach(contact->emails, cts_email_free, NULL);
		g_slist_free(contact->emails);
	}

	if (contact->grouprelations) {
		g_slist_foreach(contact->grouprelations, cts_group_free, NULL);
		g_slist_free(contact->grouprelations);
	}

	if (contact->events) {
		g_slist_foreach(contact->events, cts_event_free, NULL);
		g_slist_free(contact->events);
	}

	if (contact->messengers) {
		g_slist_foreach(contact->messengers, cts_messenger_free, NULL);
		g_slist_free(contact->messengers);
	}

	if (contact->postal_addrs) {
		g_slist_foreach(contact->postal_addrs, cts_postal_free, NULL);
		g_slist_free(contact->postal_addrs);
	}

	if (contact->web_addrs) {
		g_slist_foreach(contact->web_addrs, cts_web_free, NULL);
		g_slist_free(contact->web_addrs);
	}

	if (contact->nicknames) {
		g_slist_foreach(contact->nicknames, cts_nickname_free, NULL);
		g_slist_free(contact->nicknames);
	}

	if (contact->extended_values) {
		g_slist_foreach(contact->extended_values, cts_extend_free, NULL);
		g_slist_free(contact->extended_values);
	}
}

API int contacts_svc_struct_free(CTSstruct* structure)
{
	retv_if(NULL == structure, CTS_ERR_ARG_NULL);

	switch (structure->s_type)
	{
	case CTS_STRUCT_CONTACT:
		cts_contact_free((contact_t *)structure);
		free(structure);
		break;
	default:
		ERR("The structure type(%d) is Not valid", structure->s_type);
		return CTS_ERR_ARG_INVALID;
	}
	return CTS_SUCCESS;
}

API int contacts_svc_struct_get_list(CTSstruct *contact,
		cts_struct_field field, GSList** retlist)
{
	contact_t *record = (contact_t *)contact;

	retv_if(NULL == contact, CTS_ERR_ARG_NULL);
	retv_if(NULL == retlist, CTS_ERR_ARG_NULL);
	retvm_if(CTS_STRUCT_CONTACT != contact->s_type, CTS_ERR_ARG_INVALID,
			"The contact(%d) must be type of CTS_STRUCT_CONTACT.", contact->s_type);

	switch (field)
	{
	case CTS_CF_NUMBER_LIST:
		*retlist = record->numbers;
		break;
	case CTS_CF_EMAIL_LIST:
		*retlist = record->emails;
		break;
	case CTS_CF_GROUPREL_LIST:
		*retlist = record->grouprelations;
		break;
	case CTS_CF_EVENT_LIST:
		*retlist = record->events;
		break;
	case CTS_CF_MESSENGER_LIST:
		*retlist = record->messengers;
		break;
	case CTS_CF_POSTAL_ADDR_LIST:
		*retlist = record->postal_addrs;
		break;
	case CTS_CF_WEB_ADDR_LIST:
		*retlist = record->web_addrs;
		break;
	case CTS_CF_NICKNAME_LIST:
		*retlist = record->nicknames;
		break;
	default:
		ERR("The parameter(field) is invalid"
				"You MUST be (CTS_CF_VALUE_MAX < field < CTS_CF_FIELD_MAX).");
		return CTS_ERR_ARG_INVALID;
	}

	if (NULL == *retlist) return CTS_ERR_NO_DATA;

	return CTS_SUCCESS;
}

static cts_extend* cts_extend_slist_search(int type, GSList *list)
{
	cts_extend *tmp_extend;
	GSList *tmp_gslist=list;
	while (tmp_gslist)
	{
		tmp_extend = tmp_gslist->data;
		retvm_if(CTS_VALUE_EXTEND != tmp_extend->v_type, NULL,
				"List has other type");
		if (tmp_extend->type == type) return tmp_extend;

		tmp_gslist = tmp_gslist->next;
	}
	return NULL;
}

static inline int cts_contact_get_value(contact_t *contact,
		cts_struct_field field, CTSvalue** retval)
{

	switch (field)
	{
	case CTS_CF_NAME_VALUE:
		*retval = (CTSvalue *)contact->name;
		break;
	case CTS_CF_BASE_INFO_VALUE:
		*retval = (CTSvalue *)contact->base;
		break;
	case CTS_CF_COMPANY_VALUE:
		*retval = (CTSvalue *)contact->company;
		break;
	default:
		if ((int)CTS_DATA_EXTEND_START <= field) {
			*retval = (CTSvalue *)cts_extend_slist_search(field,
					contact->extended_values);
			return CTS_SUCCESS;
		}
		ERR("The parameter(field:%d) is not interpreted", field);
		return CTS_ERR_ARG_INVALID;
	}
	return CTS_SUCCESS;
}

API int contacts_svc_struct_get_value(CTSstruct *structure,
		cts_struct_field field, CTSvalue **retval)
{
	int ret;

	retv_if(NULL == structure, CTS_ERR_ARG_NULL);
	retv_if(NULL == retval, CTS_ERR_ARG_NULL);

	switch (structure->s_type)
	{
	case CTS_STRUCT_CONTACT:
		ret = cts_contact_get_value((contact_t *)structure, field, retval);
		if (CTS_SUCCESS != ret)
			return ret;
		break;
	default:
		ERR("The structure type(%d) is Not valid", structure->s_type);
		return CTS_ERR_ARG_INVALID;
	}

	if (NULL == *retval) return CTS_ERR_NO_DATA;
	return CTS_SUCCESS;
}

#define CTS_REMOVE_GSLIST_ITEM(type, loc) \
	do { \
		cts_##type##_free(tmp_##type, NULL); \
		if (prev) { \
			prev->next = tmp_gslist->next; \
			g_slist_free_1(tmp_gslist); \
			tmp_gslist = prev->next; \
		} \
		else { \
			contact->loc = tmp_gslist->next; \
			g_slist_free_1(tmp_gslist); \
			tmp_gslist = contact->loc; \
		} \
	}while(false)

static inline int cts_struct_store_num_list(contact_t *contact, GSList* list)
{
	cts_number *tmp_number;

	GSList *new_gslist=NULL, *tmp_gslist=list, *prev=NULL;
	if (contact->numbers && tmp_gslist == contact->numbers)
	{
		while (tmp_gslist)
		{
			tmp_number = tmp_gslist->data;
			if (tmp_number)
			{
				retvm_if(CTS_VALUE_NUMBER != tmp_number->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");

				if (!tmp_number->id && tmp_number->deleted)
				{
					CTS_REMOVE_GSLIST_ITEM(number, numbers);
					continue;
				}

				if (!tmp_number->embedded)
				{
					tmp_number->embedded = true;
					tmp_number->number = SAFE_STRDUP(tmp_number->number);
				}
			}
			prev = tmp_gslist;
			tmp_gslist = tmp_gslist->next;
		}
	}
	else
	{
		while (tmp_gslist)
		{
			tmp_number = tmp_gslist->data;
			if (tmp_number)
			{
				retvm_if(tmp_number && CTS_VALUE_NUMBER != tmp_number->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");
				if (!tmp_number->embedded)
				{
					tmp_number->embedded = true;
					tmp_number->number = SAFE_STRDUP(tmp_number->number);
					new_gslist = g_slist_append(new_gslist, tmp_number);
				}
			}
			tmp_gslist = tmp_gslist->next;
		}
		contact->numbers = g_slist_concat(contact->numbers, new_gslist);
	}
	return CTS_SUCCESS;
}

static inline int cts_struct_store_email_list(contact_t *contact, GSList* list)
{
	cts_email *tmp_email;

	GSList *new_gslist=NULL, *tmp_gslist=list, *prev=NULL;
	if (contact->emails && tmp_gslist == contact->emails)
	{
		while (tmp_gslist)
		{
			tmp_email = tmp_gslist->data;
			if (tmp_email)
			{
				retvm_if(CTS_VALUE_EMAIL != tmp_email->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");

				if (!tmp_email->id && tmp_email->deleted) {
					CTS_REMOVE_GSLIST_ITEM(email, emails);
					continue;
				}

				if (!tmp_email->embedded)
				{
					tmp_email->embedded = true;
					tmp_email->email_addr = SAFE_STRDUP(tmp_email->email_addr);
				}
			}
			prev = tmp_gslist;
			tmp_gslist = tmp_gslist->next;
		}
	}
	else
	{
		while (tmp_gslist)
		{
			tmp_email = tmp_gslist->data;
			if (tmp_email)
			{
				retvm_if(CTS_VALUE_EMAIL != tmp_email->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");
				if (!tmp_email->embedded)
				{
					tmp_email->embedded = true;
					tmp_email->email_addr = SAFE_STRDUP(tmp_email->email_addr);
					new_gslist = g_slist_append(new_gslist, tmp_email);
				}
			}
			tmp_gslist = tmp_gslist->next;
		}
		contact->emails = g_slist_concat(contact->emails, new_gslist);
	}
	return CTS_SUCCESS;
}

static inline int cts_struct_store_grouprel_list(contact_t *contact, GSList* list)
{
	cts_group *tmp_group;

	GSList *new_gslist=NULL, *tmp_gslist=list, *prev=NULL;
	if (contact->grouprelations && tmp_gslist == contact->grouprelations)
	{
		while (tmp_gslist)
		{
			tmp_group = tmp_gslist->data;
			if (tmp_group)
			{
				retvm_if(CTS_VALUE_GROUP_RELATION != tmp_group->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");

				if (!tmp_group->name && tmp_group->deleted) {
					CTS_REMOVE_GSLIST_ITEM(group, grouprelations);
					continue;
				}

				tmp_group->embedded = true;
			}
			prev = tmp_gslist;
			tmp_gslist = tmp_gslist->next;
		}
	}
	else
	{
		while (tmp_gslist)
		{
			tmp_group = tmp_gslist->data;
			if (tmp_group)
			{
				retvm_if(CTS_VALUE_GROUP_RELATION != tmp_group->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");
				if (!tmp_group->embedded)
				{
					tmp_group->embedded = true;
					new_gslist = g_slist_append(new_gslist, tmp_group);
				}
			}
			tmp_gslist = tmp_gslist->next;
		}
		contact->grouprelations = g_slist_concat(contact->grouprelations, new_gslist);
	}
	return CTS_SUCCESS;
}

static inline int cts_struct_store_event_list(contact_t *contact, GSList* list)
{
	cts_event *tmp_event;

	GSList *new_gslist=NULL, *tmp_gslist=list, *prev=NULL;
	if (contact->events && tmp_gslist == contact->events)
	{
		while (tmp_gslist)
		{
			tmp_event = tmp_gslist->data;
			if (tmp_event)
			{
				retvm_if(CTS_VALUE_EVENT != tmp_event->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");

				if (!tmp_event->id && tmp_event->deleted) {
					CTS_REMOVE_GSLIST_ITEM(event, events);
					continue;
				}

				tmp_event->embedded = true;
			}
			prev = tmp_gslist;
			tmp_gslist = tmp_gslist->next;
		}
	}
	else
	{
		while (tmp_gslist)
		{
			tmp_event = tmp_gslist->data;
			if (tmp_event)
			{
				retvm_if(CTS_VALUE_EVENT != tmp_event->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");
				if (!tmp_event->embedded)
				{
					tmp_event->embedded = true;
					new_gslist = g_slist_append(new_gslist, tmp_event);
				}
			}
			tmp_gslist = tmp_gslist->next;
		}
		contact->events = g_slist_concat(contact->events, new_gslist);
	}
	return CTS_SUCCESS;
}

static inline int cts_struct_store_messenger_list(contact_t *contact, GSList* list)
{
	cts_messenger *tmp_messenger;

	GSList *new_gslist=NULL, *tmp_gslist=list, *prev=NULL;
	if (contact->messengers && tmp_gslist == contact->messengers)
	{
		while (tmp_gslist)
		{
			tmp_messenger = tmp_gslist->data;
			if (tmp_messenger)
			{
				retvm_if(CTS_VALUE_MESSENGER != tmp_messenger->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");

				if (!tmp_messenger->id && tmp_messenger->deleted) {
					CTS_REMOVE_GSLIST_ITEM(messenger, messengers);
					continue;
				}

				if (!tmp_messenger->embedded) {
					tmp_messenger->embedded = true;
					tmp_messenger->im_id = SAFE_STRDUP(tmp_messenger->im_id);
					tmp_messenger->svc_name = SAFE_STRDUP(tmp_messenger->svc_name);
					tmp_messenger->svc_op = SAFE_STRDUP(tmp_messenger->svc_op);
				}
			}
			prev = tmp_gslist;
			tmp_gslist = tmp_gslist->next;
		}
	}
	else
	{
		while (tmp_gslist)
		{
			tmp_messenger = tmp_gslist->data;
			if (tmp_messenger)
			{
				retvm_if(CTS_VALUE_MESSENGER != tmp_messenger->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");
				if (!tmp_messenger->embedded)
				{
					tmp_messenger->embedded = true;
					tmp_messenger->im_id = SAFE_STRDUP(tmp_messenger->im_id);
					tmp_messenger->svc_name = SAFE_STRDUP(tmp_messenger->svc_name);
					tmp_messenger->svc_op = SAFE_STRDUP(tmp_messenger->svc_op);
					new_gslist = g_slist_append(new_gslist, tmp_messenger);
				}
			}
			tmp_gslist = tmp_gslist->next;
		}
		contact->messengers = g_slist_concat(contact->messengers, new_gslist);
	}
	return CTS_SUCCESS;
}

static inline int cts_struct_store_postal_list(contact_t *contact, GSList* list)
{
	cts_postal *tmp_postal;

	GSList *new_gslist=NULL, *tmp_gslist=list, *prev=NULL;
	if (contact->postal_addrs && tmp_gslist == contact->postal_addrs)
	{
		while (tmp_gslist)
		{
			tmp_postal = tmp_gslist->data;
			if (tmp_postal)
			{
				retvm_if(CTS_VALUE_POSTAL != tmp_postal->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");

				if (!tmp_postal->id && tmp_postal->deleted) {
					CTS_REMOVE_GSLIST_ITEM(postal, postal_addrs);
					continue;
				}

				if (!tmp_postal->embedded) {
					tmp_postal->embedded = true;
					tmp_postal->pobox = SAFE_STRDUP(tmp_postal->pobox);
					tmp_postal->postalcode = SAFE_STRDUP(tmp_postal->postalcode);
					tmp_postal->region = SAFE_STRDUP(tmp_postal->region);
					tmp_postal->locality = SAFE_STRDUP(tmp_postal->locality);
					tmp_postal->street = SAFE_STRDUP(tmp_postal->street);
					tmp_postal->extended = SAFE_STRDUP(tmp_postal->extended);
					tmp_postal->country = SAFE_STRDUP(tmp_postal->country);
				}
			}
			prev = tmp_gslist;
			tmp_gslist = tmp_gslist->next;
		}
	}
	else
	{
		//retvm_if(NULL != contact->postal_addrs, CTS_ERR_ARG_INVALID, "New list can be stored when struct has no list");
		while (tmp_gslist)
		{
			tmp_postal = tmp_gslist->data;
			if (tmp_postal) {
				retvm_if(tmp_postal && CTS_VALUE_POSTAL != tmp_postal->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");
				if (!tmp_postal->embedded) {
					tmp_postal->embedded = true;
					tmp_postal->pobox = SAFE_STRDUP(tmp_postal->pobox);
					tmp_postal->postalcode = SAFE_STRDUP(tmp_postal->postalcode);
					tmp_postal->region = SAFE_STRDUP(tmp_postal->region);
					tmp_postal->locality = SAFE_STRDUP(tmp_postal->locality);
					tmp_postal->street = SAFE_STRDUP(tmp_postal->street);
					tmp_postal->extended = SAFE_STRDUP(tmp_postal->extended);
					tmp_postal->country = SAFE_STRDUP(tmp_postal->country);
					new_gslist = g_slist_append(new_gslist, tmp_postal);
				}
			}
			tmp_gslist = tmp_gslist->next;
		}
		contact->postal_addrs = g_slist_concat(contact->postal_addrs, new_gslist);
	}
	return CTS_SUCCESS;
}

static inline int cts_struct_store_web_list(contact_t *contact, GSList* list)
{
	cts_web *tmp_web;

	GSList *new_gslist=NULL, *tmp_gslist=list, *prev=NULL;
	if (contact->web_addrs && tmp_gslist == contact->web_addrs)
	{
		while (tmp_gslist)
		{
			tmp_web = tmp_gslist->data;
			if (tmp_web)
			{
				retvm_if(CTS_VALUE_WEB != tmp_web->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");

				if (!tmp_web->id && tmp_web->deleted) {
					CTS_REMOVE_GSLIST_ITEM(web, web_addrs);
					continue;
				}

				if (!tmp_web->embedded) {
					tmp_web->embedded = true;
					tmp_web->url = SAFE_STRDUP(tmp_web->url);
				}
			}
			prev = tmp_gslist;
			tmp_gslist = tmp_gslist->next;
		}
	}
	else
	{
		while (tmp_gslist)
		{
			tmp_web = tmp_gslist->data;
			if (tmp_web)
			{
				retvm_if(tmp_web && CTS_VALUE_WEB != tmp_web->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");
				if (!tmp_web->embedded) {
					tmp_web->embedded = true;
					tmp_web->url = SAFE_STRDUP(tmp_web->url);
					new_gslist = g_slist_append(new_gslist, tmp_web);
				}
			}
			tmp_gslist = tmp_gslist->next;
		}
		contact->web_addrs = g_slist_concat(contact->web_addrs, new_gslist);
	}
	return CTS_SUCCESS;
}

static inline int cts_struct_store_nickname_list(contact_t *contact, GSList* list)
{
	cts_nickname *tmp_nickname;

	GSList *new_gslist=NULL, *tmp_gslist=list, *prev=NULL;
	if (contact->nicknames && tmp_gslist == contact->nicknames)
	{
		while (tmp_gslist)
		{
			tmp_nickname = tmp_gslist->data;
			if (tmp_nickname)
			{
				retvm_if(CTS_VALUE_NICKNAME != tmp_nickname->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");

				if (!tmp_nickname->id && tmp_nickname->deleted) {
					CTS_REMOVE_GSLIST_ITEM(nickname, nicknames);
					continue;
				}

				if (!tmp_nickname->embedded) {
					tmp_nickname->embedded = true;
					tmp_nickname->nick = SAFE_STRDUP(tmp_nickname->nick);
				}
			}
			prev = tmp_gslist;
			tmp_gslist = tmp_gslist->next;
		}
	}
	else
	{
		//retvm_if(NULL != contact->web_addrs, CTS_ERR_ARG_INVALID, "New list can be stored when struct has no list");
		while (tmp_gslist)
		{
			tmp_nickname = tmp_gslist->data;
			if (tmp_nickname) {
				retvm_if(tmp_nickname && CTS_VALUE_NICKNAME != tmp_nickname->v_type, CTS_ERR_ARG_INVALID,
						"List has other type");
				if (!tmp_nickname->embedded)
				{
					tmp_nickname->embedded = true;
					tmp_nickname->nick = SAFE_STRDUP(tmp_nickname->nick);
					new_gslist = g_slist_append(new_gslist, tmp_nickname);
				}
			}
			tmp_gslist = tmp_gslist->next;
		}
		contact->nicknames = g_slist_concat(contact->nicknames, new_gslist);
	}
	return CTS_SUCCESS;
}

API int contacts_svc_struct_store_list(CTSstruct *contact,
		cts_struct_field field, GSList *list)
{
	int ret;

	retv_if(NULL == contact, CTS_ERR_ARG_NULL);
	retv_if(NULL == list, CTS_ERR_ARG_NULL);
	retvm_if(CTS_STRUCT_CONTACT != contact->s_type, CTS_ERR_ARG_INVALID,
			"The contact(%d) must be type of CTS_STRUCT_CONTACT.", contact->s_type);

	switch (field)
	{
	case CTS_CF_NUMBER_LIST:
		ret = cts_struct_store_num_list((contact_t *)contact, list);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_struct_store_num_list() Failed(%d)",ret);
		break;
	case CTS_CF_EMAIL_LIST:
		ret = cts_struct_store_email_list((contact_t *)contact, list);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_struct_store_email_list() Failed(%d)",ret);
		break;
	case CTS_CF_GROUPREL_LIST:
		ret = cts_struct_store_grouprel_list((contact_t *)contact, list);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_struct_store_grouprel_list() Failed(%d)",ret);
		break;
	case CTS_CF_EVENT_LIST:
		ret = cts_struct_store_event_list((contact_t *)contact, list);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_struct_store_event_list() Failed(%d)",ret);
		break;
	case CTS_CF_MESSENGER_LIST:
		ret = cts_struct_store_messenger_list((contact_t *)contact, list);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_struct_store_messenger_list() Failed(%d)",ret);
		break;
	case CTS_CF_POSTAL_ADDR_LIST:
		ret = cts_struct_store_postal_list((contact_t *)contact, list);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_struct_store_postal_list() Failed(%d)",ret);
		break;
	case CTS_CF_WEB_ADDR_LIST:
		ret = cts_struct_store_web_list((contact_t *)contact, list);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_struct_store_web_list() Failed(%d)",ret);
		break;
	case CTS_CF_NICKNAME_LIST:
		ret = cts_struct_store_nickname_list((contact_t *)contact, list);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_struct_store_nickname_list() Failed(%d)",ret);
		break;
	default:
		ERR("The parameter(field) is invalid"
				"You MUST be (CTS_CF_VALUE_MAX < field < CTS_CF_FIELD_MAX).");
		return CTS_ERR_ARG_INVALID;
	}
	return CTS_SUCCESS;
}

static inline void cts_contact_store_name(contact_t *contact, cts_name *value)
{
	if (contact->name)
	{
		if (value->is_changed) {
			FREEandSTRDUP(contact->name->first, value->first);
			FREEandSTRDUP(contact->name->last, value->last);
			FREEandSTRDUP(contact->name->addition, value->addition);
			FREEandSTRDUP(contact->name->display, value->display);
			FREEandSTRDUP(contact->name->prefix, value->prefix);
			FREEandSTRDUP(contact->name->suffix, value->suffix);
			contact->name->is_changed = true;
		}
	}
	else
	{
		//contact->name = (cts_name *)contacts_svc_value_new(CTS_VALUE_NAME);
		contact->name = value;
		contact->name->embedded = true;
		contact->name->first = SAFE_STRDUP(value->first);
		contact->name->last = SAFE_STRDUP(value->last);
		contact->name->addition = SAFE_STRDUP(value->addition);
		contact->name->display = SAFE_STRDUP(value->display);
		contact->name->prefix = SAFE_STRDUP(value->prefix);
		contact->name->suffix = SAFE_STRDUP(value->suffix);
	}
}

static inline void cts_contact_store_base(contact_t *contact, cts_ct_base *value)
{
	if (contact->base)
	{
		if (value->uid_changed) {
			FREEandSTRDUP(contact->base->uid, value->uid);
			contact->base->uid_changed = true;
		}
		if (value->img_changed) {
			FREEandSTRDUP(contact->base->img_path, value->img_path);
			contact->base->img_changed = true;
		}
		if (value->full_img_changed) {
			FREEandSTRDUP(contact->base->full_img_path, value->full_img_path);
			contact->base->full_img_changed = true;
		}
		if (value->ringtone_changed) {
			FREEandSTRDUP(contact->base->ringtone_path, value->ringtone_path);
			contact->base->ringtone_changed = true;
		}
		if (value->note_changed) {
			FREEandSTRDUP(contact->base->note, value->note);
			contact->base->note_changed = true;
		}
	}
	else
	{
		contact->base = value;
		contact->base->embedded = true;
		contact->base->uid = SAFE_STRDUP(value->uid);
		contact->base->img_path = SAFE_STRDUP(value->img_path);
		contact->base->full_img_path = SAFE_STRDUP(value->full_img_path);
		contact->base->ringtone_path = SAFE_STRDUP(value->ringtone_path);
		contact->base->note = SAFE_STRDUP(value->note);
	}
}

static inline void cts_contact_store_company(contact_t *contact, cts_company *value)
{
	if (contact->company)
	{
		FREEandSTRDUP(contact->company->name, value->name);
		FREEandSTRDUP(contact->company->department, value->department);
		FREEandSTRDUP(contact->company->jot_title, value->jot_title);
		FREEandSTRDUP(contact->company->role, value->role);
		FREEandSTRDUP(contact->company->assistant_name, value->assistant_name);
	}
	else
	{
		//contact->company = (cts_company *)contacts_svc_value_new(CTS_VALUE_COMPANY);
		contact->company = value;
		contact->company->embedded = true;
		contact->company->name = SAFE_STRDUP(value->name);
		contact->company->department = SAFE_STRDUP(value->department);
		contact->company->jot_title = SAFE_STRDUP(value->jot_title);
		contact->company->role = SAFE_STRDUP(value->role);
		contact->company->assistant_name = SAFE_STRDUP(value->assistant_name);
	}
}

static inline int cts_contact_store_extend(contact_t *contact,
		int type, cts_extend *value)
{
	cts_extend *stored_extend;

	stored_extend = cts_extend_slist_search(type, contact->extended_values);
	if (NULL == stored_extend)
	{
		retvm_if(value->embedded, CTS_ERR_ARG_INVALID, "This Value is already stored");
		value->embedded = true;
		value->type = type;
		contact->extended_values = g_slist_append(contact->extended_values, value);
		value->data2 = SAFE_STRDUP(value->data2);
		value->data3 = SAFE_STRDUP(value->data3);
		value->data4 = SAFE_STRDUP(value->data4);
		value->data5 = SAFE_STRDUP(value->data5);
		value->data6 = SAFE_STRDUP(value->data6);
		value->data7 = SAFE_STRDUP(value->data7);
		value->data8 = SAFE_STRDUP(value->data8);
		value->data9 = SAFE_STRDUP(value->data9);
		value->data10 = SAFE_STRDUP(value->data10);
	}
	else
	{
		retvm_if(stored_extend == value, CTS_SUCCESS, "This value is already stored");

		FREEandSTRDUP(stored_extend->data2, value->data2);
		FREEandSTRDUP(stored_extend->data3, value->data3);
		FREEandSTRDUP(stored_extend->data4, value->data4);
		FREEandSTRDUP(stored_extend->data5, value->data5);
		FREEandSTRDUP(stored_extend->data6, value->data6);
		FREEandSTRDUP(stored_extend->data7, value->data7);
		FREEandSTRDUP(stored_extend->data8, value->data8);
		FREEandSTRDUP(stored_extend->data9, value->data9);
		FREEandSTRDUP(stored_extend->data10, value->data10);
	}

	return CTS_SUCCESS;
}

API int contacts_svc_struct_store_value(CTSstruct *contact,
		cts_struct_field field, CTSvalue *value)
{
	contact_t *record = (contact_t *)contact;

	retv_if(NULL == contact, CTS_ERR_ARG_NULL);
	retv_if(NULL == value, CTS_ERR_ARG_NULL);
	retvm_if(CTS_STRUCT_CONTACT != contact->s_type, CTS_ERR_ARG_INVALID,
			"The contact(%d) must be type of CTS_STRUCT_CONTACT.", contact->s_type);
	CTS_DBG("contact type = %d, field = %d, value type = %d",
			contact->s_type, field, value->v_type);

	switch (field)
	{
	case CTS_CF_NAME_VALUE:
		retvm_if(CTS_VALUE_NAME != value->v_type, CTS_ERR_ARG_INVALID,
				"The value must be a CTS_VALUE_NAME for field(CTS_CF_NAME_VALUE).");
		if (record->name != (cts_name *)value)
			cts_contact_store_name(record, (cts_name *)value);
		break;
	case CTS_CF_BASE_INFO_VALUE:
		retvm_if(CTS_VALUE_CONTACT_BASE_INFO != value->v_type, CTS_ERR_ARG_INVALID,
				"The value must be a CTS_VALUE_CONTACT_BASE_INFO for field(CTS_CF_IMAGE_PATH_STR).");
		if (record->base != (cts_ct_base *)value)
			cts_contact_store_base(record, (cts_ct_base*)value);
		break;
	case CTS_CF_COMPANY_VALUE:
		retvm_if(CTS_VALUE_COMPANY != value->v_type, CTS_ERR_ARG_INVALID,
				"The value must be a CTS_VALUE_COMPANY for field(CTS_CF_COMPANY_VALUE).");
		if (record->company != (cts_company *)value)
			cts_contact_store_company(record, (cts_company*)value);
		break;
	default:
		if (CTS_VALUE_EXTEND == value->v_type && (int)CTS_DATA_EXTEND_START <= field)
			return cts_contact_store_extend(record, field, (cts_extend*)value);
		ERR("The parameter(field:%d) is invalid"
				"You MUST be (CTS_CF_NONE < field < CTS_CF_VALUE_MAX).", field);
		return CTS_ERR_ARG_INVALID;
	}

	return CTS_SUCCESS;
}

API CTSvalue* contacts_svc_value_new(cts_value_type type)
{
	CTSvalue* ret_val;
	switch ((int)type)
	{
	case CTS_VALUE_BASIC:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_basic));
		break;
	case CTS_VALUE_CONTACT_BASE_INFO:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_ct_base));
		break;
	case CTS_VALUE_NAME:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_name));
		break;
	case CTS_VALUE_EMAIL:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_email));
		break;
	case CTS_VALUE_NUMBER:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_number));
		break;
	case CTS_VALUE_WEB:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_web));
		break;
	case CTS_VALUE_POSTAL:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_postal));
		break;
	case CTS_VALUE_EVENT:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_event));
		break;
	case CTS_VALUE_MESSENGER:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_messenger));
		if (ret_val) ret_val->v_type = CTS_VALUE_MESSENGER;
		break;
	case CTS_VALUE_NICKNAME:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_nickname));
		break;
	case CTS_VALUE_GROUP_RELATION:
	case CTS_VALUE_GROUP:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_group));
		break;
	case CTS_VALUE_COMPANY:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_company));
		break;
	case CTS_VALUE_PHONELOG:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_plog));
		break;
	case CTS_VALUE_EXTEND:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_extend));
		break;
	case CTS_VALUE_ADDRESSBOOK:
		ret_val = (CTSvalue*)calloc(1, sizeof(cts_addrbook));
		break;
	case CTS_VALUE_LIST_CONTACT:
		if (contact_list_mempool) {
			memset(contact_list_mempool, 0x00, sizeof(contact_list));
			ret_val = (CTSvalue*)contact_list_mempool;
			contact_list_mempool = NULL;
		}
		else
			ret_val = (CTSvalue*)calloc(1, sizeof(contact_list));
		break;
	case CTS_VALUE_LIST_PLOG:
		if (plog_list_mempool) {
			memset(plog_list_mempool, 0x00, sizeof(plog_list));
			ret_val = (CTSvalue*)plog_list_mempool;
			plog_list_mempool = NULL;
		}
		else
			ret_val = (CTSvalue*)calloc(1, sizeof(plog_list));
		break;
	case CTS_VALUE_LIST_CUSTOM_NUM_TYPE:
		if (numtype_list_mempool) {
			memset(numtype_list_mempool, 0x00, sizeof(numtype_list));
			ret_val = (CTSvalue*)numtype_list_mempool;
			numtype_list_mempool = NULL;
		}
		else
			ret_val = (CTSvalue*)calloc(1, sizeof(numtype_list));
		break;
	case CTS_VALUE_LIST_CHANGE:
		if (change_list_mempool) {
			memset(change_list_mempool, 0x00, sizeof(change_list));
			ret_val = (CTSvalue*)change_list_mempool;
			change_list_mempool = NULL;
		}
		else
			ret_val = (CTSvalue*)calloc(1, sizeof(change_list));
		break;
	case CTS_VALUE_LIST_ADDRBOOK:
		if (addrbook_list_mempool) {
			memset(addrbook_list_mempool, 0x00, sizeof(cts_addrbook));
			ret_val = (CTSvalue*)addrbook_list_mempool;
			addrbook_list_mempool = NULL;
		}
		else
			ret_val = (CTSvalue*)calloc(1, sizeof(cts_addrbook));
		break;
	case CTS_VALUE_LIST_GROUP:
		if (group_list_mempool) {
			memset(group_list_mempool, 0x00, sizeof(cts_group));
			ret_val = (CTSvalue*)group_list_mempool;
			group_list_mempool = NULL;
		}
		else
			ret_val = (CTSvalue*)calloc(1, sizeof(cts_group));
		break;
	case CTS_VALUE_LIST_SHORTCUT:
		if (favorite_list_mempool) {
			memset(favorite_list_mempool, 0x00, sizeof(shortcut_list));
			ret_val = (CTSvalue*)favorite_list_mempool;
			favorite_list_mempool = NULL;
		}
		else
			ret_val = (CTSvalue*)calloc(1, sizeof(shortcut_list));
		break;
	case CTS_VALUE_LIST_SDN:
		if (sdn_list_mempool) {
			memset(sdn_list_mempool, 0x00, sizeof(sdn_list));
			ret_val = (CTSvalue*)sdn_list_mempool;
			sdn_list_mempool = NULL;
		}
		else
			ret_val = (CTSvalue*)calloc(1, sizeof(sdn_list));
		break;
	default:
		ERR("your type is Not supported");
		return NULL;
	}

	if (ret_val)
		ret_val->v_type = type;
	else
		ERR("calloc() Failed(%d)", errno);

	return ret_val;
}

static inline void cts_internal_value_info_free(CTSvalue *value)
{
	plog_list *plog;
	cts_plog *log;
	numtype_list *numtype;
	contact_list *contact;
	change_list *change;
	shortcut_list *favorite;
	cts_group *group;
	cts_addrbook *ab;
	sdn_list *sdn;

	switch (value->v_type)
	{
	case CTS_VALUE_LIST_CONTACT:
	case CTS_VALUE_LIST_NUMBERINFO:
	case CTS_VALUE_LIST_EMAILINFO:
		contact = (contact_list *)value;
		free(contact->img_path);
		free(contact->first);
		free(contact->last);
		free(contact->display);
		free(contact->connect);
		free(contact->normalize);

		if (!contact_list_mempool) {
			contact_list_mempool = contact;
		}
		else
			if (contact_list_mempool != contact)
				free(contact);
		break;
	case CTS_VALUE_LIST_PLOG:
		plog = (plog_list *)value;
		free(plog->first);
		free(plog->last);
		free(plog->display);
		free(plog->img_path);

		if (!plog_list_mempool) {
			plog_list_mempool = plog;
		}
		else
			if (plog_list_mempool != plog)
				free(plog);
		break;
	case CTS_VALUE_LIST_CUSTOM_NUM_TYPE:
		numtype = (numtype_list *)value;
		free(numtype->name);
		if (!numtype_list_mempool) {
			numtype_list_mempool = numtype;
		}
		else
			if (numtype_list_mempool != numtype)
				free(numtype);
		break;
	case CTS_VALUE_LIST_CHANGE:
		change = (change_list *)value;
		if (!change_list_mempool) {
			change_list_mempool = change;
		}
		else
			if (change_list_mempool != change)
				free(change);
		break;
	case CTS_VALUE_LIST_GROUP:
		group = (cts_group *)value;
		free(group->name);

		if (!group_list_mempool) {
			group_list_mempool = group;
		}
		else
			if (group_list_mempool != group)
				free(group);
		break;
	case CTS_VALUE_LIST_ADDRBOOK:
		ab = (cts_addrbook *)value;
		free(ab->name);

		if (!addrbook_list_mempool) {
			addrbook_list_mempool = ab;
		}
		else
			if (addrbook_list_mempool != ab)
				free(ab);
		break;
	case CTS_VALUE_LIST_SHORTCUT:
		favorite = (shortcut_list *)value;
		free(favorite->first);
		free(favorite->last);
		free(favorite->display);
		free(favorite->number);
		free(favorite->img_path);

		if (!favorite_list_mempool) {
			favorite_list_mempool = favorite;
		}
		else
			if (favorite_list_mempool != favorite)
				free(favorite);
		break;
	case CTS_VALUE_LIST_SDN:
		sdn = (sdn_list *)value;
		free(sdn->name);
		free(sdn->number);

		if (!sdn_list_mempool) {
			sdn_list_mempool = sdn;
		}
		else
			if (sdn_list_mempool != sdn)
				free(sdn);
		break;
	case CTS_VALUE_RDONLY_NAME:
		cts_name_free((cts_name *)value);
		break;
	case CTS_VALUE_RDONLY_NUMBER:
		cts_number_free(value, NULL);
		break;
	case CTS_VALUE_RDONLY_EMAIL:
		cts_email_free(value, NULL);
		break;
	case CTS_VALUE_RDONLY_COMPANY:
		cts_company_free((cts_company *)value);
		break;
	case CTS_VALUE_RDONLY_PLOG:
		log = (cts_plog *)value;
		free(log->number);
		free(log->extra_data2);
		free(log);
		break;
	default:
		ERR("The type of value is unknown type(%d)", value->v_type);
		return;
	}
}

API int contacts_svc_value_free(CTSvalue *value)
{
	retv_if(NULL == value, CTS_ERR_ARG_NULL);

	if (CTS_VALUE_LIST_CONTACT <= value->v_type)
		cts_internal_value_info_free(value);
	else {
		switch (value->v_type) {
		case CTS_VALUE_GROUP:
			if (value->embedded) {
				free(((cts_group *)value)->name);
				free(((cts_group *)value)->ringtone_path);
			}
			break;
		case CTS_VALUE_ADDRESSBOOK:
			if (value->embedded) {
				free(((cts_addrbook *)value)->name);
			}
			break;
		default:
			if (value->embedded) {
				DBG("This is the value of struct. It is really freed with the struct.");
				return CTS_SUCCESS;
			}
			break;
		}
		free(value);
	}

	return CTS_SUCCESS;
}

API int contacts_svc_value_get_type(CTSvalue *value)
{
	retv_if(NULL == value, CTS_ERR_ARG_NULL);

	return value->v_type;
}

static inline int cts_value_get_int_base(cts_ct_base *value, int field)
{
	int ret = 0;

	switch (field)
	{
	case CTS_BASE_VAL_ID_INT:
		ret = value->id;
		break;
	case CTS_BASE_VAL_CHANGED_TIME_INT:
		ret = value->changed_time;
		break;
	case CTS_BASE_VAL_ADDRESSBOOK_ID_INT:
		ret = value->addrbook_id;
		break;
	default:
		ERR("The field(%d) is not supported in value(Base_info)", field);
		break;
	}
	return ret;
}

static inline int cts_value_get_int_plog_list(plog_list *value, int field)
{
	int ret = 0;

	switch (field)
	{
	case CTS_LIST_PLOG_ID_INT:
		ret = value->id;
		break;
	case CTS_LIST_PLOG_RELATED_ID_INT:
		ret = value->related_id;
		break;
	case CTS_LIST_PLOG_NUM_TYPE_INT:
		ret = value->num_type;
		break;
	case CTS_LIST_PLOG_LOG_TIME_INT:
		ret = value->log_time;
		break;
	case CTS_LIST_PLOG_LOG_TYPE_INT:
		ret = value->log_type;
		break;
	case CTS_LIST_PLOG_DURATION_INT:
	case CTS_LIST_PLOG_MSGID_INT:
		ret = value->extra_data1;
		break;
	default:
		ERR("The field(%d) is not supported in value(plog list)", field);
		break;
	}
	return ret;
}

static inline int cts_value_get_int_plog(cts_plog *value, int field)
{
	int ret = 0;

	switch (field)
	{
	case CTS_PLOG_VAL_ID_INT:
		ret = value->id;
		break;
	case CTS_PLOG_VAL_RELATED_ID_INT:
		ret = value->related_id;
		break;
	case CTS_PLOG_VAL_LOG_TIME_INT:
		ret = value->log_time;
		break;
	case CTS_PLOG_VAL_LOG_TYPE_INT:
		ret = value->log_type;
		break;
	case CTS_PLOG_VAL_DURATION_INT:
	case CTS_PLOG_VAL_MSGID_INT:
		ret = value->extra_data1;
		break;
	default:
		ERR("The field(%d) is not supported in value(plog)", field);
		break;
	}
	return ret;
}

static inline int cts_value_get_int_change_list(change_list *value, int field)
{
	int ret = 0;

	switch (field)
	{
	case CTS_LIST_CHANGE_ID_INT:
		ret = value->id;
		break;
	case CTS_LIST_CHANGE_TYPE_INT:
		ret = value->changed_type;
		break;
	case CTS_LIST_CHANGE_VER_INT:
		ret = value->changed_ver;
		break;
	default:
		ERR("The field(%d) is not supported in value(change list)", field);
		break;
	}
	return ret;
}

static inline int cts_value_get_int_shortcut_list(shortcut_list *value, int field)
{
	int ret = 0;

	switch (field)
	{
	case CTS_LIST_SHORTCUT_ID_INT:
		ret = value->id;
		break;
	case CTS_LIST_SHORTCUT_CONTACT_ID_INT:
		ret = value->contact_id;
		break;
	case CTS_LIST_SHORTCUT_NUMBER_TYPE_INT:
		ret = value->num_type;
		break;
	case CTS_LIST_SHORTCUT_SPEEDDIAL_INT:
		ret = value->speeddial;
		break;
	default:
		ERR("The field(%d) is not supported in value(shorcut list)", field);
		break;
	}
	return ret;
}

static inline int cts_value_get_int_addrbook(cts_addrbook *value, int field)
{
	int ret = 0;

	switch (field)
	{
	case CTS_ADDRESSBOOK_VAL_ID_INT:
		ret = value->id;
		break;
	case CTS_ADDRESSBOOK_VAL_ACC_ID_INT:
		ret = value->acc_id;
		break;
	case CTS_ADDRESSBOOK_VAL_ACC_TYPE_INT:
		ret = value->acc_type;
		break;
	case CTS_ADDRESSBOOK_VAL_MODE_INT:
		ret = value->mode;
		break;
	default:
		ERR("The field(%d) is not supported in value(addressbook)", field);
		break;
	}
	return ret;
}

API int contacts_svc_value_get_int(CTSvalue *value, int field)
{
	int ret = 0;
	retvm_if(NULL == value, 0, "The Parameter(value) is NULL");

	switch (value->v_type)
	{
	case CTS_VALUE_BASIC:
		retvm_if(CTS_BASIC_VAL_INT != ((cts_basic*)value)->type, 0,
				"The type of Basic_value is not integer");
		ret = ((cts_basic*)value)->val.i;
		break;
	case CTS_VALUE_CONTACT_BASE_INFO:
		ret = cts_value_get_int_base((cts_ct_base *)value, field);
		break;
	case CTS_VALUE_EXTEND:
		if (CTS_EXTEND_VAL_DATA1_INT == field)
			ret = ((cts_extend*)value)->data1;
		else
			ERR("The field(%d) is not supported in value(Extend)", field);
		break;
	case CTS_VALUE_RDONLY_NUMBER:
	case CTS_VALUE_NUMBER:
		if (CTS_NUM_VAL_ID_INT == field)
			ret = ((cts_number*)value)->id;
		else if (CTS_NUM_VAL_TYPE_INT == field)
			ret = ((cts_number*)value)->type;
		else
			ERR("The field(%d) is not supported in value(Number)", field);
		break;
	case CTS_VALUE_RDONLY_EMAIL:
	case CTS_VALUE_EMAIL:
		if (CTS_EMAIL_VAL_ID_INT == field)
			ret = ((cts_email*)value)->id;
		else if (CTS_EMAIL_VAL_TYPE_INT == field)
			ret = ((cts_email*)value)->type;
		else
			ERR("The field(%d) is not supported in value(Email)", field);
		break;
	case CTS_VALUE_LIST_PLOG:
		ret = cts_value_get_int_plog_list((plog_list *)value, field);
		break;
	case CTS_VALUE_RDONLY_PLOG:
		ret = cts_value_get_int_plog((cts_plog *)value, field);
		break;
	case CTS_VALUE_LIST_CONTACT:
	case CTS_VALUE_LIST_NUMS_EMAILS:
		if (CTS_LIST_CONTACT_ID_INT == field)
			ret = ((contact_list *)value)->id;
		else if (CTS_LIST_CONTACT_ADDRESSBOOK_ID_INT == field)
			ret = ((contact_list *)value)->acc_id;
		else
			ERR("The field(%d) is not supported in value(contact_list)", field);
		break;
	case CTS_VALUE_ADDRESSBOOK:
	case CTS_VALUE_LIST_ADDRBOOK:
		ret = cts_value_get_int_addrbook((cts_addrbook *)value, field);
		break;
	case CTS_VALUE_LIST_NUMBERINFO:
	case CTS_VALUE_LIST_EMAILINFO: // CTS_LIST_EMAIL_CONTACT_ID_INT is same to CTS_LIST_NUM_CONTACT_ID_INT
		retvm_if(CTS_LIST_NUM_CONTACT_ID_INT != field, 0,
				"The field(%d) is not supported in value(Number list)", field);
		ret = ((contact_list*)value)->id;
		break;
	case CTS_VALUE_LIST_CUSTOM_NUM_TYPE:
		if (CTS_LIST_CUSTOM_NUM_TYPE_ID_INT == field)
			ret = ((numtype_list*)value)->id;
		else
			ERR("Not supported field(%d)", field);
		break;
	case CTS_VALUE_LIST_GROUP:
		if (CTS_LIST_GROUP_ID_INT == field)
			ret = ((cts_group *)value)->id;
		else if (CTS_LIST_GROUP_ADDRESSBOOK_ID_INT == field)
			ret = ((cts_group *)value)->addrbook_id;
		else
			ERR("Not supported field(%d)", field);
		break;
	case CTS_VALUE_LIST_CHANGE:
		ret = cts_value_get_int_change_list((change_list *)value, field);
		break;
	case CTS_VALUE_LIST_SHORTCUT:
		ret = cts_value_get_int_shortcut_list((shortcut_list *)value, field);
		break;
	case CTS_VALUE_MESSENGER:
		if (CTS_MESSENGER_VAL_TYPE_INT == field)
			ret = ((cts_messenger*)value)->type;
		else
			ERR("Not supported field(%d)", field);
		break;
	case CTS_VALUE_GROUP_RELATION:
		if (CTS_GROUPREL_VAL_ID_INT == field)
			ret = ((cts_group*)value)->id;
		else
			ERR("Not supported field(%d)", field);
		break;
	case CTS_VALUE_GROUP:
		if (CTS_GROUP_VAL_ID_INT == field)
			ret = ((cts_group*)value)->id;
		if (CTS_GROUP_VAL_ADDRESSBOOK_ID_INT == field)
			ret = ((cts_group*)value)->addrbook_id;
		else
			ERR("Not supported field(%d)", field);
		break;
	case CTS_VALUE_WEB:
		if (CTS_WEB_VAL_TYPE_INT == field)
			ret = ((cts_web*)value)->type;
		else
			ERR("Not supported field(%d)", field);
		break;
	case CTS_VALUE_POSTAL:
		if (CTS_POSTAL_VAL_TYPE_INT == field)
			ret = ((cts_postal*)value)->type;
		else
			ERR("Not supported field(%d)", field);
		break;
	case CTS_VALUE_EVENT:
		if (CTS_EVENT_VAL_TYPE_INT == field)
			ret = ((cts_event *)value)->type;
		else if (CTS_EVENT_VAL_DATE_INT == field)
			ret = ((cts_event *)value)->date;
		else
			ERR("Not supported field(%d)", field);
		break;
	case CTS_VALUE_PHONELOG:
		/* phonelog value is write only */
	case CTS_VALUE_COMPANY:
		/* company value doesn't have interger value */
	case CTS_VALUE_NAME:
		/* name value doesn't have interger value */
	default:
		ERR("The value has unsupported type");
		break;
	}
	return ret;
}

double contacts_svc_value_get_dbl(CTSvalue *value, int field)
{
	retv_if(NULL == value, CTS_ERR_ARG_NULL);

	switch (value->v_type)
	{
	case CTS_VALUE_BASIC:
		retvm_if(CTS_BASIC_VAL_DBL != ((cts_basic*)value)->type, 0.0,
				"The type of value is not double");
		return ((cts_basic*)value)->val.d;
	case CTS_VALUE_NAME:
	case CTS_VALUE_EMAIL:
	case CTS_VALUE_NUMBER:
	case CTS_VALUE_WEB:
	case CTS_VALUE_POSTAL:
	case CTS_VALUE_EVENT:
	case CTS_VALUE_MESSENGER:
	case CTS_VALUE_GROUP_RELATION:
	case CTS_VALUE_COMPANY:
	default:
		ERR("The value has unsupported type");
		return CTS_ERR_ARG_INVALID;
	}

}

API bool contacts_svc_value_get_bool(CTSvalue *value, int field)
{
	retvm_if(NULL == value, false, "The Parameter(value) is NULL");

	switch (value->v_type)
	{
	case CTS_VALUE_CONTACT_BASE_INFO:
		if (CTS_BASE_VAL_FAVORITE_BOOL == field) {
			return ((cts_ct_base*)value)->is_favorite;
		}
		else {
			ERR("The field(%d) is not supported in value(BASE_INFO)", field);
			return false;
		}
	case CTS_VALUE_RDONLY_NUMBER:
	case CTS_VALUE_NUMBER:
		if (CTS_NUM_VAL_DEFAULT_BOOL == field) {
			return ((cts_number*)value)->is_default;
		}
		else if (CTS_NUM_VAL_DELETE_BOOL == field) {
			return value->deleted;
		}
		else if (CTS_NUM_VAL_FAVORITE_BOOL == field) {
			return ((cts_number*)value)->is_favorite;
		}
		else {
			ERR("The field(%d) is not supported in value(Number)", field);
			return false;
		}
	case CTS_VALUE_RDONLY_EMAIL:
	case CTS_VALUE_EMAIL:
		if (CTS_EMAIL_VAL_DEFAULT_BOOL == field) {
			return ((cts_email*)value)->is_default;
		}
		else if (CTS_EMAIL_VAL_DELETE_BOOL == field) {
			return value->deleted;
		}
		else {
			ERR("The field(%d) is not supported in value(Email)", field);
			return false;
		}
	case CTS_VALUE_GROUP_RELATION:
		if (CTS_GROUPREL_VAL_DELETE_BOOL == field) {
			return value->deleted;
		}
		else {
			ERR("The field(%d) is not supported in value(Group)", field);
			return false;
		}
	case CTS_VALUE_EVENT:
		if (CTS_EVENT_VAL_DELETE_BOOL == field) {
			return value->deleted;
		}
		else {
			ERR("The field(%d) is not supported in value(Event)", field);
			return false;
		}
	case CTS_VALUE_MESSENGER:
		if (CTS_MESSENGER_VAL_DELETE_BOOL == field) {
			return value->deleted;
		}
		else {
			ERR("The field(%d) is not supported in value(Messenger)", field);
			return false;
		}
	case CTS_VALUE_POSTAL:
		if (CTS_POSTAL_VAL_DELETE_BOOL == field) {
			return value->deleted;
		}
		else if (CTS_POSTAL_VAL_DEFAULT_BOOL == field) {
			return ((cts_postal*)value)->is_default;;
		}
		else {
			ERR("The field(%d) is not supported in value(Postal)", field);
			return false;
		}
	case CTS_VALUE_WEB:
		if (CTS_WEB_VAL_DELETE_BOOL == field) {
			return value->deleted;
		}
		else {
			ERR("The field(%d) is not supported in value(Web)", field);
			return false;
		}
	case CTS_VALUE_NICKNAME:
		if (CTS_NICKNAME_VAL_DELETE_BOOL == field) {
			return value->deleted;
		}
		else {
			ERR("The field(%d) is not supported in value(Web)", field);
			return false;
		}
	case CTS_VALUE_EXTEND:
		if (CTS_EXTEND_VAL_DELETE_BOOL == field) {
			return value->deleted;
		}
		else {
			ERR("The field(%d) is not supported in value(Extend)", field);
			return false;
		}
	case CTS_VALUE_BASIC:
		retvm_if(CTS_BASIC_VAL_BOOL != ((cts_basic*)value)->type, false,
				"The type of value is not boolean");
		return ((cts_basic*)value)->val.b;
	case CTS_VALUE_PHONELOG:
		/* phonelog value is write only */
	case CTS_VALUE_LIST_CONTACT:
		/* contact list value doesn't have boolean value */
	case CTS_VALUE_LIST_PLOG:
		/* plog list value doesn't have boolean value */
	case CTS_VALUE_LIST_CUSTOM_NUM_TYPE:
		/* custom number type list value doesn't have boolean value */
	case CTS_VALUE_LIST_CHANGE:
		/* Change list value doesn't have boolean value */
	case CTS_VALUE_NAME:
		/* name value doesn't have boolean value */
	case CTS_VALUE_COMPANY:
		/* company value doesn't have boolean value */
	default:
		ERR("The value has unsupported type");
		return false;
	}
}

static inline char* cts_value_get_str_name(int op_code,
		cts_name *value, int field)
{
	char *ret_val;

	switch (field)
	{
	case CTS_NAME_VAL_FIRST_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->first);
		break;
	case CTS_NAME_VAL_LAST_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->last);
		break;
	case CTS_NAME_VAL_DISPLAY_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->display);
		break;
	case CTS_NAME_VAL_ADDITION_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->addition);
		break;
	case CTS_NAME_VAL_PREFIX_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->prefix);
		break;
	case CTS_NAME_VAL_SUFFIX_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->suffix);
		break;
	default:
		ERR("The parameter(field:%d) is not interpreted", field);
		ret_val = NULL;
		break;
	}
	return ret_val;
}

static inline char* cts_value_get_str_extend(int op_code,
		cts_extend *value, int field)
{
	char *ret_val;

	switch (field)
	{
	case CTS_EXTEND_VAL_DATA2_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->data2);
		break;
	case CTS_EXTEND_VAL_DATA3_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->data3);
		break;
	case CTS_EXTEND_VAL_DATA4_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->data4);
		break;
	case CTS_EXTEND_VAL_DATA5_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->data5);
		break;
	case CTS_EXTEND_VAL_DATA6_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->data6);
		break;
	case CTS_EXTEND_VAL_DATA7_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->data7);
		break;
	case CTS_EXTEND_VAL_DATA8_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->data8);
		break;
	case CTS_EXTEND_VAL_DATA9_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->data9);
		break;
	case CTS_EXTEND_VAL_DATA10_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->data10);
		break;
	default:
		ERR("The parameter(field:%d) is not interpreted", field);
		ret_val = NULL;
		break;
	}
	return ret_val;
}

static inline char* cts_value_get_str_base(int op_code,
		cts_ct_base *value, int field)
{
	char *ret_val;

	switch (field)
	{
	case CTS_BASE_VAL_IMG_PATH_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->img_path);
		if (NULL == ret_val && value->vcard_img_path) {
			if (CTS_HANDLE_STR_STEAL == op_code)
				ret_val = strdup(value->vcard_img_path);
			else
				ret_val = value->vcard_img_path;
		}
		break;
	case CTS_BASE_VAL_RINGTONE_PATH_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->ringtone_path);
		break;
	case CTS_BASE_VAL_NOTE_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->note);
		break;
	case CTS_BASE_VAL_UID_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->uid);
		break;
	case CTS_BASE_VAL_FULL_IMG_PATH_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->full_img_path);
		break;
	default:
		ERR("The parameter(field:%d) is not interpreted", field);
		ret_val = NULL;
		break;
	}
	return ret_val;
}

static inline char* cts_value_get_str_contact_list(int op_code,
		contact_list *value, int field)
{
	char *ret_val;
	switch (field)
	{
	case CTS_LIST_CONTACT_FIRST_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->first);
		break;
	case CTS_LIST_CONTACT_LAST_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->last);
		break;
	case CTS_LIST_CONTACT_DISPLAY_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->display);
		break;
	case CTS_LIST_CONTACT_IMG_PATH_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->img_path);
		break;
	case CTS_LIST_CONTACT_NUM_OR_EMAIL_STR:
		if (CTS_VALUE_LIST_NUMS_EMAILS == value->v_type) {
			HANDLE_STEAL_STRING(op_code, ret_val, value->connect);
		} else {
			ERR("The parameter(field:%d, value type = %d) is not interpreted",
				field, value->v_type);
			ret_val = NULL;
		}
		break;
	case CTS_LIST_CONTACT_NORMALIZED_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->normalize);
		break;
	default:
		ERR("The parameter(field:%d) is not interpreted", field);
		ret_val = NULL;
		break;
	}
	return ret_val;
}

static inline char* cts_value_get_str_num_email_list(int op_code,
		contact_list *value, int field)
{
	char *ret_val;
	switch (field)
	{
	case CTS_LIST_NUM_CONTACT_FIRST_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->first);
		break;
	case CTS_LIST_NUM_CONTACT_LAST_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->last);
		break;
	case CTS_LIST_NUM_CONTACT_DISPLAY_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->display);
		break;
	case CTS_LIST_NUM_CONTACT_IMG_PATH_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->img_path);
		break;
	case CTS_LIST_NUM_NUMBER_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->connect);
		break;
	default:
		ERR("The parameter(field:%d) is not interpreted", field);
		ret_val = NULL;
		break;
	}
	return ret_val;
}

static inline char* cts_value_get_str_favorite_list(int op_code,
		shortcut_list *value, int field)
{
	char *ret_val;
	switch (field)
	{
	case CTS_LIST_SHORTCUT_FIRST_NAME_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->first);
		break;
	case CTS_LIST_SHORTCUT_LAST_NAME_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->last);
		break;
	case CTS_LIST_SHORTCUT_DISPLAY_NAME_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->display);
		break;
	case CTS_LIST_SHORTCUT_IMG_PATH_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->img_path);
		break;
	case CTS_LIST_SHORTCUT_NUMBER_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->number);
		break;
	default:
		ERR("The parameter(field:%d) is not interpreted", field);
		ret_val = NULL;
		break;
	}
	return ret_val;
}

static inline char* cts_value_get_str_plog_list(int op_code,
		plog_list *value, int field)
{
	char *ret_val;
	switch (field)
	{
	case CTS_LIST_PLOG_FIRST_NAME_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->first);
		break;
	case CTS_LIST_PLOG_LAST_NAME_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->last);
		break;
	case CTS_LIST_PLOG_DISPLAY_NAME_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->display);
		break;
	case CTS_LIST_PLOG_NUMBER_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->number);
		break;
	case CTS_LIST_PLOG_IMG_PATH_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->img_path);
		break;
	case CTS_LIST_PLOG_SHORTMSG_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->extra_data2);
		break;
	default:
		ERR("The parameter(field:%d) is not interpreted", field);
		ret_val = NULL;
		break;
	}
	return ret_val;
}

static inline char* cts_value_get_str_postal(int op_code,
		cts_postal *value, int field)
{
	char *ret_val;
	switch (field)
	{
	case CTS_POSTAL_VAL_POBOX_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->pobox);
		break;
	case CTS_POSTAL_VAL_POSTALCODE_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->postalcode);
		break;
	case CTS_POSTAL_VAL_REGION_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->region);
		break;
	case CTS_POSTAL_VAL_LOCALITY_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->locality);
		break;
	case CTS_POSTAL_VAL_STREET_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->street);
		break;
	case CTS_POSTAL_VAL_EXTENDED_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->extended);
		break;
	case CTS_POSTAL_VAL_COUNTRY_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->country);
		break;
	default:
		ERR("The parameter(field:%d) is not interpreted", field);
		ret_val = NULL;
		break;
	}
	return ret_val;
}

static inline char* cts_value_get_str_company(int op_code,
		cts_company *value, int field)
{
	char *ret_val;
	switch (field)
	{
	case CTS_COMPANY_VAL_NAME_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->name);
		break;
	case CTS_COMPANY_VAL_DEPARTMENT_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->department);
		break;
	case CTS_COMPANY_VAL_JOB_TITLE_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->jot_title);
		break;
	case CTS_COMPANY_VAL_ROLE_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->role);
		break;
	case CTS_COMPANY_VAL_ASSISTANT_NAME_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->assistant_name);
		break;
	default:
		ERR("The parameter(field:%d) is not interpreted", field);
		ret_val = NULL;
		break;
	}
	return ret_val;
}


static inline char* cts_value_get_str_im(int op_code,
		cts_messenger *value, int field)
{
	char *ret_val;
	switch (field)
	{
	case CTS_MESSENGER_VAL_IM_ID_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->im_id);
		break;
	case CTS_MESSENGER_VAL_SERVICE_NAME_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->svc_name);
		break;
	case CTS_MESSENGER_VAL_SERVICE_OP_STR:
		HANDLE_STEAL_STRING(op_code, ret_val, value->svc_op);
		break;
	default:
		ERR("The parameter(field:%d) is not interpreted", field);
		ret_val = NULL;
		break;
	}
	return ret_val;
}

static char* cts_value_handle_str(int op_code, CTSvalue *value, int field)
{
	char *ret_val;
	retvm_if(NULL == value, NULL, "The Parameter(value) is NULL");

	switch (value->v_type)
	{
	case CTS_VALUE_BASIC:
		retvm_if(CTS_BASIC_VAL_STR != ((cts_basic *)value)->type, NULL,
				"The type of value is not string");
		HANDLE_STEAL_STRING(op_code, ret_val, ((cts_basic *)value)->val.s);
		break;
	case CTS_VALUE_CONTACT_BASE_INFO:
		ret_val = cts_value_get_str_base(op_code, (cts_ct_base *)value, field);
		break;
	case CTS_VALUE_POSTAL:
		ret_val = cts_value_get_str_postal(op_code, (cts_postal *)value, field);
		break;
	case CTS_VALUE_COMPANY:
	case CTS_VALUE_RDONLY_COMPANY:
		ret_val = cts_value_get_str_company(op_code, (cts_company *)value, field);
		break;
	case CTS_VALUE_NAME:
	case CTS_VALUE_RDONLY_NAME:
		ret_val = cts_value_get_str_name(op_code, (cts_name *)value, field);
		break;
	case CTS_VALUE_EXTEND:
		ret_val = cts_value_get_str_extend(op_code, (cts_extend *)value, field);
		break;
	case CTS_VALUE_LIST_CONTACT:
	case CTS_VALUE_LIST_NUMS_EMAILS:
		ret_val = cts_value_get_str_contact_list(op_code, (contact_list *)value, field);
		break;
	case CTS_VALUE_LIST_NUMBERINFO:
	case CTS_VALUE_LIST_EMAILINFO:
		ret_val = cts_value_get_str_num_email_list(op_code, (contact_list *)value, field);
		break;
	case CTS_VALUE_LIST_SHORTCUT:
		ret_val = cts_value_get_str_favorite_list(op_code, (shortcut_list *)value, field);
		break;
	case CTS_VALUE_LIST_PLOG:
		ret_val = cts_value_get_str_plog_list(op_code, (plog_list *)value, field);
		break;
	case CTS_VALUE_MESSENGER:
		ret_val = cts_value_get_str_im(op_code, (cts_messenger *)value, field);
		break;
	case CTS_VALUE_RDONLY_PLOG:
		if (CTS_PLOG_VAL_NUMBER_STR == field) {
			HANDLE_STEAL_STRING(op_code, ret_val, ((cts_plog *)value)->number);
		} else if (CTS_PLOG_VAL_SHORTMSG_STR == field) {
			HANDLE_STEAL_STRING(op_code, ret_val, ((cts_plog *)value)->extra_data2);
		} else {
			ERR("Not supported field");
			return NULL;
		}
		break;
	case CTS_VALUE_NUMBER:
	case CTS_VALUE_RDONLY_NUMBER:
		retvm_if(CTS_NUM_VAL_NUMBER_STR != field, NULL,
				"This field(%d) is not supported in value(Number)", field);
		HANDLE_STEAL_STRING(op_code, ret_val, ((cts_number *)value)->number);
		break;
	case CTS_VALUE_EMAIL:
	case CTS_VALUE_RDONLY_EMAIL:
		retvm_if(CTS_EMAIL_VAL_ADDR_STR != field, NULL,
				"This field(%d) is not supported in value(Email)", field);
		HANDLE_STEAL_STRING(op_code, ret_val, ((cts_email *)value)->email_addr);
		break;
	case CTS_VALUE_ADDRESSBOOK:
	case CTS_VALUE_LIST_ADDRBOOK:
		retvm_if(CTS_ADDRESSBOOK_VAL_NAME_STR != field, NULL,
				"This field(%d) is not supported in value(addressbook)", field);
		HANDLE_STEAL_STRING(op_code, ret_val, ((cts_addrbook *)value)->name);
		break;
	case CTS_VALUE_GROUP_RELATION:
		if (CTS_GROUPREL_VAL_NAME_STR == field) {
			HANDLE_STEAL_STRING(op_code, ret_val, ((cts_group *)value)->name);
		}
		else if (CTS_GROUPREL_VAL_RINGTONE_STR == field) {
			HANDLE_STEAL_STRING(op_code, ret_val, ((cts_group *)value)->ringtone_path);
		}
		else {
			ERR("Not supported field(%d)", field);
			ret_val = NULL;
		}
		break;
	case CTS_VALUE_WEB:
		if (CTS_WEB_VAL_ADDR_STR == field) {
			HANDLE_STEAL_STRING(op_code, ret_val, ((cts_web *)value)->url);
		}
		else {
			ERR("Not supported field(%d)", field);
			ret_val = NULL;
		}
		break;
	case CTS_VALUE_NICKNAME:
		if (CTS_NICKNAME_VAL_NAME_STR == field) {
			HANDLE_STEAL_STRING(op_code, ret_val, ((cts_nickname *)value)->nick);
		}
		else {
			ERR("Not supported field(%d)", field);
			ret_val = NULL;
		}
		break;
	case CTS_VALUE_GROUP:
		if (CTS_GROUP_VAL_NAME_STR == field) {
			HANDLE_STEAL_STRING(op_code, ret_val, ((cts_group *)value)->name);
		}
		else if (CTS_GROUP_VAL_RINGTONE_STR == field) {
			HANDLE_STEAL_STRING(op_code, ret_val, ((cts_group *)value)->ringtone_path);
		}
		else {
			ERR("Not supported field(%d)", field);
			ret_val = NULL;
		}
		break;
	case CTS_VALUE_LIST_GROUP:
		if (CTS_LIST_GROUP_NAME_STR == field) {
			HANDLE_STEAL_STRING(op_code, ret_val, ((cts_group *)value)->name);
		}
		else {
			ERR("Not supported field(%d)", field);
			ret_val = NULL;
		}
		break;
	case CTS_VALUE_LIST_CUSTOM_NUM_TYPE:
		if (CTS_LIST_CUSTOM_NUM_TYPE_NAME_STR == field) {
			HANDLE_STEAL_STRING(op_code, ret_val, ((numtype_list *)value)->name);
		} else {
			ERR("Not supported field(%d)", field);
			ret_val = NULL;
		}
		break;
	case CTS_VALUE_LIST_SDN:
		if (CTS_LIST_SDN_NAME_STR == field) {
			HANDLE_STEAL_STRING(op_code, ret_val, ((sdn_list *)value)->name);
		}
		else if (CTS_LIST_SDN_NUMBER_STR == field) {
			HANDLE_STEAL_STRING(op_code, ret_val, ((sdn_list *)value)->number);
		}
		else {
			ERR("Not supported field(%d)", field);
			ret_val = NULL;
		}
		break;
	case CTS_VALUE_PHONELOG:
		/* phonelog value is write only */
	case CTS_VALUE_LIST_CHANGE:
		/* Change list value doesn't have string value */
	case CTS_VALUE_EVENT:
		/* evet value doesn't have string value */
	default:
		ERR("The value has unsupported type");
		ret_val = NULL;
		break;
	}
	return ret_val;
}

API const char* contacts_svc_value_get_str(CTSvalue *value, int field)
{
	return cts_value_handle_str(CTS_HANDLE_STR_GET, value, field);
}

API char* contacts_svc_value_steal_str(CTSvalue *value, int field)
{
	return cts_value_handle_str(CTS_HANDLE_STR_STEAL, value, field);
}

static inline int cts_value_set_int_plog(cts_plog *value, int field, int intval)
{
	switch (field)
	{
	case CTS_PLOG_VAL_LOG_TIME_INT:
		value->log_time = intval;
		break;
	case CTS_PLOG_VAL_LOG_TYPE_INT:
		value->log_type = intval;
		break;
	case CTS_PLOG_VAL_DURATION_INT:
	case CTS_PLOG_VAL_MSGID_INT:
		value->extra_data1 = intval;
		break;
	case CTS_PLOG_VAL_RELATED_ID_INT:
		value->related_id = intval;
		break;
	default:
		ERR("The field(%d) is not supported in value(plog)", field);
		return CTS_ERR_ARG_INVALID;
	}
	return CTS_SUCCESS;
}

static inline int cts_value_set_int_addrbook(cts_addrbook *value,
		int field, int intval)
{
	switch (field)
	{
	case CTS_ADDRESSBOOK_VAL_ACC_ID_INT:
		value->acc_id = intval;
		break;
	case CTS_ADDRESSBOOK_VAL_ACC_TYPE_INT:
		value->acc_type = intval;
		break;
	case CTS_ADDRESSBOOK_VAL_MODE_INT:
		value->mode = intval;
		break;
	default:
		ERR("The field(%d) is not supported in value(addressbook)", field);
		return CTS_ERR_ARG_INVALID;
	}
	return CTS_SUCCESS;
}

API int contacts_svc_value_set_int(CTSvalue *value, int field, int intval)
{
	retv_if(NULL == value, CTS_ERR_ARG_NULL);

	switch (value->v_type)
	{
	case CTS_VALUE_BASIC:
		((cts_basic*)value)->type = CTS_BASIC_VAL_INT;
		((cts_basic*)value)->val.i = intval;
		break;
	case CTS_VALUE_EXTEND:
		retvm_if(CTS_EXTEND_VAL_DATA1_INT != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		((cts_extend *)value)->data1 = intval;
		break;
	case CTS_VALUE_EMAIL:
	case CTS_VALUE_NUMBER:
		retvm_if(CTS_NUM_VAL_TYPE_INT != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		((cts_number *)value)->type = intval;
		break;
	case CTS_VALUE_PHONELOG:
		return cts_value_set_int_plog((cts_plog *)value, field, intval);
	case CTS_VALUE_GROUP_RELATION:
		retvm_if(CTS_GROUPREL_VAL_ID_INT != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		retvm_if(value->embedded, CTS_ERR_ARG_INVALID,
				"The field is only used for creating");
		((cts_group *)value)->id = intval;
		break;
	case CTS_VALUE_GROUP:
		retvm_if(CTS_GROUP_VAL_ADDRESSBOOK_ID_INT != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		retvm_if(!value->embedded, CTS_ERR_ARG_INVALID,
				"The field is only used for updating");
		((cts_group *)value)->addrbook_id = intval;
		break;
	case CTS_VALUE_MESSENGER:
		retvm_if(CTS_MESSENGER_VAL_TYPE_INT != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		((cts_messenger *)value)->type = intval;
		break;
	case CTS_VALUE_WEB:
		retvm_if(CTS_WEB_VAL_TYPE_INT != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		((cts_web *)value)->type = intval;
		break;
	case CTS_VALUE_EVENT:
		if (CTS_EVENT_VAL_TYPE_INT == field)
			((cts_event *)value)->type = intval;
		else if (CTS_EVENT_VAL_DATE_INT == field)
			((cts_event *)value)->date = intval;
		else
		{
			ERR("Not supported field");
			return CTS_ERR_ARG_INVALID;
		}
		break;
	case CTS_VALUE_POSTAL:
		retvm_if(CTS_POSTAL_VAL_TYPE_INT != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		((cts_postal *)value)->type = intval;
		break;
	case CTS_VALUE_ADDRESSBOOK:
		return cts_value_set_int_addrbook((cts_addrbook *)value, field, intval);
	case CTS_VALUE_COMPANY:
		/* company value doesn't have integer value */
	case CTS_VALUE_NAME:
		/* name value doesn't have integer value */
	case CTS_VALUE_CONTACT_BASE_INFO:
		/* base_info value doesn't have integer value for set */
	default:
		ERR("The value has unsupported type");
		return CTS_ERR_ARG_INVALID;
	}

	return CTS_SUCCESS;
}

int contacts_svc_value_set_dbl(CTSvalue *value, int field, double dblval)
{
	retv_if(NULL == value, CTS_ERR_ARG_NULL);

	switch (value->v_type)
	{
	case CTS_VALUE_BASIC:
		((cts_basic*)value)->type = CTS_BASIC_VAL_DBL;
		((cts_basic*)value)->val.d = dblval;
		break;
	case CTS_VALUE_EMAIL:
	case CTS_VALUE_NUMBER:
	case CTS_VALUE_WEB:
	case CTS_VALUE_POSTAL:
	case CTS_VALUE_EVENT:
	case CTS_VALUE_MESSENGER:
	case CTS_VALUE_GROUP_RELATION:
	case CTS_VALUE_COMPANY:
	case CTS_VALUE_NAME:
	case CTS_VALUE_CONTACT_BASE_INFO:
	default:
		ERR("The value has unsupported type");
		return CTS_ERR_ARG_INVALID;
	}

	return CTS_SUCCESS;
}

API int contacts_svc_value_set_bool(CTSvalue *value,
		int field, bool boolval)
{
	retv_if(NULL == value, CTS_ERR_ARG_NULL);

	switch (value->v_type)
	{
	case CTS_VALUE_CONTACT_BASE_INFO:
		if (CTS_BASE_VAL_FAVORITE_BOOL == field)
			((cts_ct_base*)value)->is_favorite = boolval;
		else {
			ERR("The field(%d) is not supported in value(BASE_INFO)", field);
			return CTS_ERR_ARG_INVALID;
		}
		break;
	case CTS_VALUE_NUMBER:
		if (CTS_NUM_VAL_DEFAULT_BOOL == field)
			((cts_number *)value)->is_default = boolval;
		else if (CTS_NUM_VAL_FAVORITE_BOOL == field)
			((cts_number *)value)->is_favorite = boolval;
		else if (CTS_NUM_VAL_DELETE_BOOL == field) {
			retvm_if(false == value->embedded, CTS_ERR_ARG_INVALID,
					"The field is only used for updating");
			value->deleted = boolval;
		}
		else {
			ERR("Not supported field");
			return CTS_ERR_ARG_INVALID;
		}
		break;
	case CTS_VALUE_EMAIL:
		if (CTS_EMAIL_VAL_DEFAULT_BOOL == field)
			((cts_email *)value)->is_default = boolval;
		else if (CTS_EMAIL_VAL_DELETE_BOOL == field) {
			retvm_if(false == value->embedded, CTS_ERR_ARG_INVALID,
					"The field is only used for updating");
			value->deleted = boolval;
		}
		else {
			ERR("Not supported field");
			return CTS_ERR_ARG_INVALID;
		}
		break;
	case CTS_VALUE_POSTAL:
		if (CTS_POSTAL_VAL_DEFAULT_BOOL == field)
			((cts_postal *)value)->is_default = boolval;
		else if (CTS_POSTAL_VAL_DELETE_BOOL == field) {
			retvm_if(false == value->embedded, CTS_ERR_ARG_INVALID,
					"The field is only used for updating");
			value->deleted = boolval;
		}
		else {
			ERR("Not supported field");
			return CTS_ERR_ARG_INVALID;
		}
		break;
	case CTS_VALUE_GROUP_RELATION:
		retvm_if(CTS_GROUPREL_VAL_DELETE_BOOL != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		retvm_if(false == value->embedded, CTS_ERR_ARG_INVALID,
				"The field is only used for updating");
		value->deleted = boolval;
		break;
	case CTS_VALUE_EVENT:
		retvm_if(CTS_EVENT_VAL_DELETE_BOOL != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		retvm_if(false == value->embedded, CTS_ERR_ARG_INVALID,
				"The field is only used for updating");
		value->deleted = boolval;
		break;
	case CTS_VALUE_MESSENGER:
		retvm_if(CTS_MESSENGER_VAL_DELETE_BOOL != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		retvm_if(false == value->embedded, CTS_ERR_ARG_INVALID,
				"The field is only used for updating");
		value->deleted = boolval;
		break;
	case CTS_VALUE_WEB:
		retvm_if(CTS_WEB_VAL_DELETE_BOOL != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		retvm_if(false == value->embedded, CTS_ERR_ARG_INVALID,
				"The field is only used for updating");
		value->deleted = boolval;
		break;
	case CTS_VALUE_NICKNAME:
		retvm_if(CTS_NICKNAME_VAL_DELETE_BOOL != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		retvm_if(false == value->embedded, CTS_ERR_ARG_INVALID,
				"The field is only used for updating");
		value->deleted = boolval;
		break;
	case CTS_VALUE_EXTEND:
		retvm_if(CTS_EXTEND_VAL_DELETE_BOOL != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		retvm_if(false == value->embedded, CTS_ERR_ARG_INVALID,
				"The field is only used for updating");
		value->deleted = boolval;
		break;
	case CTS_VALUE_BASIC:
		((cts_basic*)value)->type = CTS_BASIC_VAL_BOOL;
		((cts_basic*)value)->val.b = boolval;
		break;
	case CTS_VALUE_COMPANY:
		/* company value doesn't have boolean value */
	case CTS_VALUE_NAME:
		/* name value doesn't have boolean value */
	default:
		ERR("The value has unsupported type");
		return CTS_ERR_ARG_INVALID;
	}

	return CTS_SUCCESS;
}

static inline int cts_value_set_str_base(cts_ct_base *base, int field, char *strval)
{
	switch (field)
	{
	case CTS_BASE_VAL_IMG_PATH_STR:
		if (base->embedded)
			FREEandSTRDUP(base->img_path, strval);
		else
			base->img_path = strval;
		base->img_changed = true;
		break;
	case CTS_BASE_VAL_RINGTONE_PATH_STR:
		if (base->embedded)
			FREEandSTRDUP(base->ringtone_path, strval);
		else
			base->ringtone_path = strval;
		base->ringtone_changed = true;
		break;
	case CTS_BASE_VAL_NOTE_STR:
		if (base->embedded)
			FREEandSTRDUP(base->note, strval);
		else
			base->note = strval;
		base->note_changed = true;
		break;
	case CTS_BASE_VAL_UID_STR:
		if (base->embedded)
			FREEandSTRDUP(base->uid, strval);
		else
			base->uid = strval;
		base->uid_changed = true;
		break;
	case CTS_BASE_VAL_FULL_IMG_PATH_STR:
		if (base->embedded)
			FREEandSTRDUP(base->full_img_path, strval);
		else
			base->full_img_path = strval;
		base->full_img_changed = true;
		break;
	default:
		ERR("Not supported field");
		return CTS_ERR_ARG_INVALID;
	}
	return CTS_SUCCESS;
}

static inline int cts_value_set_str_name(cts_name *name, int field, char *strval)
{
	switch (field)
	{
	case CTS_NAME_VAL_FIRST_STR:
		if (name->embedded) {
			FREEandSTRDUP(name->first, strval);
		}
		else
			name->first = strval;
		break;
	case CTS_NAME_VAL_LAST_STR:
		if (name->embedded) {
			FREEandSTRDUP(name->last, strval);
		}
		else
			name->last = strval;
		break;
	case CTS_NAME_VAL_ADDITION_STR:
		if (name->embedded) {
			FREEandSTRDUP(name->addition, strval);
		}
		else
			name->addition = strval;
		break;
	case CTS_NAME_VAL_DISPLAY_STR:
		if (name->embedded) {
			FREEandSTRDUP(name->display, strval);
		}
		else
			name->display = strval;
		break;
	case CTS_NAME_VAL_PREFIX_STR:
		if (name->embedded) {
			FREEandSTRDUP(name->prefix, strval);
		}
		else
			name->prefix = strval;
		break;
	case CTS_NAME_VAL_SUFFIX_STR:
		if (name->embedded) {
			FREEandSTRDUP(name->suffix, strval);
		}
		else
			name->suffix = strval;
		break;
	default:
		ERR("Not supported field");
		return CTS_ERR_ARG_INVALID;
	}
	name->is_changed = true;
	return CTS_SUCCESS;
}

static inline int cts_value_set_str_postal(cts_postal *postal, int field, char *strval)
{
	switch (field)
	{
	case CTS_POSTAL_VAL_POBOX_STR:
		if (postal->embedded) {
			FREEandSTRDUP(postal->pobox, strval);
		}
		else
			postal->pobox = strval;
		break;
	case CTS_POSTAL_VAL_POSTALCODE_STR:
		if (postal->embedded) {
			FREEandSTRDUP(postal->postalcode, strval);
		}
		else
			postal->postalcode = strval;
		break;
	case CTS_POSTAL_VAL_REGION_STR:
		if (postal->embedded) {
			FREEandSTRDUP(postal->region, strval);
		}
		else
			postal->region = strval;
		break;
	case CTS_POSTAL_VAL_LOCALITY_STR:
		if (postal->embedded) {
			FREEandSTRDUP(postal->locality, strval);
		}
		else
			postal->locality = strval;
		break;
	case CTS_POSTAL_VAL_STREET_STR:
		if (postal->embedded) {
			FREEandSTRDUP(postal->street, strval);
		}
		else
			postal->street = strval;
		break;
	case CTS_POSTAL_VAL_EXTENDED_STR:
		if (postal->embedded) {
			FREEandSTRDUP(postal->extended, strval);
		}
		else
			postal->extended = strval;
		break;
	case CTS_POSTAL_VAL_COUNTRY_STR:
		if (postal->embedded) {
			FREEandSTRDUP(postal->country, strval);
		}
		else
			postal->country = strval;
		break;
	default:
		ERR("Not supported field");
		return CTS_ERR_ARG_INVALID;
	}
	return CTS_SUCCESS;
}

static inline int cts_value_set_str_company(
		cts_company *com, int field, char *strval)
{
	switch (field)
	{
	case CTS_COMPANY_VAL_NAME_STR:
		if (com->embedded) {
			FREEandSTRDUP(com->name, strval);
		}
		else
			com->name = strval;
		break;
	case CTS_COMPANY_VAL_DEPARTMENT_STR:
		if (com->embedded) {
			FREEandSTRDUP(com->department, strval);
		}
		else
			com->department = strval;
		break;
	case CTS_COMPANY_VAL_JOB_TITLE_STR:
		if (com->embedded) {
			FREEandSTRDUP(com->jot_title, strval);
		}
		else
			com->jot_title = strval;
		break;
	case CTS_COMPANY_VAL_ROLE_STR:
		if (com->embedded) {
			FREEandSTRDUP(com->role, strval);
		}
		else
			com->role = strval;
		break;
	case CTS_COMPANY_VAL_ASSISTANT_NAME_STR:
		if (com->embedded) {
			FREEandSTRDUP(com->assistant_name, strval);
		}
		else
			com->assistant_name = strval;
		break;
	default:
		ERR("Not supported field");
		return CTS_ERR_ARG_INVALID;
	}
	return CTS_SUCCESS;
}

static inline int cts_value_set_str_group(
		cts_group *group, int field, char *strval)
{
	switch (field)
	{
	case CTS_GROUP_VAL_NAME_STR:
		if (group->embedded) {
			FREEandSTRDUP(group->name, strval);
		}
		else
			group->name = strval;
		break;
	case CTS_GROUP_VAL_RINGTONE_STR:
		if (group->embedded) {
			FREEandSTRDUP(group->ringtone_path, strval);
		}
		else
			group->ringtone_path = strval;
		break;
	default:
		ERR("Not supported field");
		return CTS_ERR_ARG_INVALID;
	}
	return CTS_SUCCESS;
}

static inline int cts_value_set_str_extend(cts_extend *extend, int field, char *strval)
{
	switch (field)
	{
	case CTS_EXTEND_VAL_DATA2_STR:
		if (extend->embedded) {
			FREEandSTRDUP(extend->data2, strval);
		}
		else
			extend->data2 = strval;
		break;
	case CTS_EXTEND_VAL_DATA3_STR:
		if (extend->embedded) {
			FREEandSTRDUP(extend->data3, strval);
		}
		else
			extend->data3 = strval;
		break;
	case CTS_EXTEND_VAL_DATA4_STR:
		if (extend->embedded) {
			FREEandSTRDUP(extend->data4, strval);
		}
		else
			extend->data4 = strval;
		break;
	case CTS_EXTEND_VAL_DATA5_STR:
		if (extend->embedded) {
			FREEandSTRDUP(extend->data5, strval);
		}
		else
			extend->data5 = strval;
		break;
	case CTS_EXTEND_VAL_DATA6_STR:
		if (extend->embedded) {
			FREEandSTRDUP(extend->data6, strval);
		}
		else
			extend->data6 = strval;
		break;
	case CTS_EXTEND_VAL_DATA7_STR:
		if (extend->embedded) {
			FREEandSTRDUP(extend->data7, strval);
		}
		else
			extend->data7 = strval;
		break;
	case CTS_EXTEND_VAL_DATA8_STR:
		if (extend->embedded) {
			FREEandSTRDUP(extend->data8, strval);
		}
		else
			extend->data8 = strval;
		break;
	case CTS_EXTEND_VAL_DATA9_STR:
		if (extend->embedded) {
			FREEandSTRDUP(extend->data9, strval);
		}
		else
			extend->data9 = strval;
		break;

	case CTS_EXTEND_VAL_DATA10_STR:
		if (extend->embedded) {
			FREEandSTRDUP(extend->data10, strval);
		}
		else
			extend->data10 = strval;
		break;
	default:
		ERR("Not supported field");
		return CTS_ERR_ARG_INVALID;
	}
	return CTS_SUCCESS;
}


static inline int cts_value_set_str_im(cts_messenger *im, int field, char *strval)
{
	switch (field)
	{
	case CTS_MESSENGER_VAL_IM_ID_STR:
		if (im->embedded)
			FREEandSTRDUP(im->im_id, strval);
		else
			im->im_id = strval;
		break;
	case CTS_MESSENGER_VAL_SERVICE_NAME_STR:
		if (im->embedded)
			FREEandSTRDUP(im->svc_name, strval);
		else
			im->svc_name = strval;
		break;
	case CTS_MESSENGER_VAL_SERVICE_OP_STR:
		if (im->embedded)
			FREEandSTRDUP(im->svc_op, strval);
		else
			im->svc_op = strval;
		break;
	default:
		ERR("Not supported field");
		return CTS_ERR_ARG_INVALID;
	}
	return CTS_SUCCESS;
}


API int contacts_svc_value_set_str(CTSvalue *value, int field, const char *strval)
{
	char *str;

	retv_if(NULL == value, CTS_ERR_ARG_NULL);

	if (strval && *strval)
		str = (char *)strval;
	else
		str = NULL;

	switch (value->v_type)
	{
	case CTS_VALUE_BASIC:
		((cts_basic*)value)->type = CTS_BASIC_VAL_STR;
		if (value->embedded)
			FREEandSTRDUP(((cts_basic*)value)->val.s, str);
		else
			((cts_basic*)value)->val.s = str;
		break;
	case CTS_VALUE_CONTACT_BASE_INFO:
		return cts_value_set_str_base((cts_ct_base *)value, field, str);
	case CTS_VALUE_NAME:
		return cts_value_set_str_name((cts_name *)value, field, str);
	case CTS_VALUE_POSTAL:
		return cts_value_set_str_postal((cts_postal *)value, field, str);
	case CTS_VALUE_COMPANY:
		return cts_value_set_str_company((cts_company *)value, field, str);
	case CTS_VALUE_GROUP:
		return cts_value_set_str_group((cts_group *)value, field, str);
	case CTS_VALUE_EXTEND:
		return cts_value_set_str_extend((cts_extend *)value, field, str);
	case CTS_VALUE_MESSENGER:
		return cts_value_set_str_im((cts_messenger *)value, field, str);
	case CTS_VALUE_NUMBER:
		retvm_if(CTS_NUM_VAL_NUMBER_STR != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		if (value->embedded)
			FREEandSTRDUP(((cts_number*)value)->number, str);
		else
			((cts_number *)value)->number = str;
		break;
	case CTS_VALUE_EMAIL:
		retvm_if(CTS_EMAIL_VAL_ADDR_STR != field, CTS_ERR_ARG_INVALID, "Not supported field");
		if (value->embedded)
			FREEandSTRDUP(((cts_email*)value)->email_addr, str);
		else
			((cts_email *)value)->email_addr = str;
		break;
	case CTS_VALUE_GROUP_RELATION:
		retvm_if(CTS_GROUPREL_VAL_NAME_STR != field, CTS_ERR_ARG_INVALID,
				"Not supported field(%d) for CTS_VALUE_GROUP_RELATION", field);
		retvm_if(value->embedded, CTS_ERR_ARG_INVALID,
				"CTS_GROUPREL_VAL_NAME_STR is readonly");
		((cts_group *)value)->name = str;
		break;
	case CTS_VALUE_PHONELOG:  /* phonelog value never be embedded*/
		if (CTS_PLOG_VAL_NUMBER_STR == field)
			((cts_plog *)value)->number = str;
		else if (CTS_PLOG_VAL_SHORTMSG_STR == field)
			((cts_plog *)value)->extra_data2 = str;
		else
		{
			ERR("Not supported field");
			return CTS_ERR_ARG_INVALID;
		}
		break;
	case CTS_VALUE_WEB:
		retvm_if(CTS_WEB_VAL_ADDR_STR != field, CTS_ERR_ARG_INVALID, "Not supported field");
		if (value->embedded)
			FREEandSTRDUP(((cts_web *)value)->url, str);
		else
			((cts_web *)value)->url = str;
		break;
	case CTS_VALUE_NICKNAME:
		retvm_if(CTS_NICKNAME_VAL_NAME_STR != field, CTS_ERR_ARG_INVALID, "Not supported field");
		if (value->embedded)
			FREEandSTRDUP(((cts_nickname *)value)->nick, str);
		else
			((cts_nickname *)value)->nick = str;
		break;
	case CTS_VALUE_ADDRESSBOOK:
		retvm_if(CTS_ADDRESSBOOK_VAL_NAME_STR != field, CTS_ERR_ARG_INVALID,
				"Not supported field");
		if (value->embedded)
			FREEandSTRDUP(((cts_addrbook *)value)->name, str);
		else
			((cts_addrbook *)value)->name = str;
		break;
	case CTS_VALUE_EVENT:
		/* evet value doesn't have string value */
	default:
		ERR("The value has unsupported type");
		return CTS_ERR_ARG_INVALID;
	}

	return CTS_SUCCESS;
}

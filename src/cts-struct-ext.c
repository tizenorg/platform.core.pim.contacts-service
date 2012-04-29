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
#include "cts-utils.h"
#include "cts-struct.h"
#include "cts-struct-ext.h"

#define CTS_VCARD_MOVE_TO(orig, src, check) \
	do{ \
		if (NULL == orig && NULL != src) { \
			orig = src; \
			check = true; \
			src = NULL; \
		} \
	}while(false)

static inline int cts_merge_vcard_base(cts_ct_base *orig, cts_ct_base *addition)
{
	int ret;
	char dest[CTS_IMG_PATH_SIZE_MAX];

	retvm_if(NULL == addition, CTS_ERR_ARG_INVALID, "Invalid addition(%p)", addition);

	if (NULL == orig->img_path) {
		if (orig->id && addition->img_path) {
			ret = snprintf(dest, sizeof(dest), "%s/%d-%d.", CTS_IMAGE_LOCATION,
					orig->id, CTS_IMG_NORMAL);
			if (0 != strncmp(dest, addition->img_path, ret)) {
				orig->img_path = addition->img_path;
				orig->img_changed = true;
				addition->img_path = NULL;
			}
		} else {
			orig->img_path = addition->img_path;
			orig->img_changed = true;
			addition->img_path = NULL;
		}
	}

	if (NULL == orig->full_img_path) {
		if (orig->id && addition->full_img_path) {
			ret = snprintf(dest, sizeof(dest), "%s/%d-%d.", CTS_IMAGE_LOCATION,
					orig->id, CTS_IMG_FULL);
			if (0 != strncmp(dest, addition->full_img_path, ret)) {
				orig->full_img_path = addition->full_img_path;
				orig->full_img_changed = true;
				addition->full_img_path = NULL;
			}
		} else {
			orig->full_img_path = addition->full_img_path;
			orig->full_img_changed = true;
			addition->full_img_path = NULL;
		}
	}

	CTS_VCARD_MOVE_TO(orig->uid, addition->uid, orig->uid_changed);
	CTS_VCARD_MOVE_TO(orig->note, addition->note, orig->note_changed);
	CTS_VCARD_MOVE_TO(orig->ringtone_path, addition->ringtone_path, orig->ringtone_changed);
	if (NULL == orig->vcard_img_path) {
		orig->vcard_img_path = addition->vcard_img_path;
		addition->vcard_img_path = NULL;
	}

	return CTS_SUCCESS;
}

static inline int cts_merge_vcard_name(cts_name *orig, cts_name *addition)
{
	retvm_if(NULL == addition, CTS_ERR_ARG_INVALID, "Invalid addition(%p)", addition);

	if (NULL == orig->first && NULL == orig->last) {
		CTS_VCARD_MOVE_TO(orig->first, addition->first, orig->is_changed);
		CTS_VCARD_MOVE_TO(orig->last, addition->last, orig->is_changed);
	}
	CTS_VCARD_MOVE_TO(orig->addition, addition->addition, orig->is_changed);
	CTS_VCARD_MOVE_TO(orig->display, addition->display, orig->is_changed);
	CTS_VCARD_MOVE_TO(orig->prefix, addition->prefix, orig->is_changed);
	CTS_VCARD_MOVE_TO(orig->suffix, addition->suffix, orig->is_changed);

	return CTS_SUCCESS;
}

static inline GSList* cts_merge_vcard_numbers(GSList *orig, GSList *addition)
{
	GSList *i, *j;
	for (i=addition;i;i=i->next) {
		cts_number *addition = i->data;

		if (NULL == addition->number) continue;

		for (j=orig;j;j=j->next) {
			cts_number *org = j->data;
			if (org->deleted) continue;
			if (addition->number && org->number
					&& 0 == strcmp(addition->number, org->number))
				break;
		}
		if (NULL == j) {
			orig = g_slist_append(orig, addition);
			i->data = NULL;
		}
	}

	return orig;
}

static inline GSList* cts_merge_vcard_emails(GSList *orig, GSList *addition)
{
	GSList *i, *j;
	for (i=addition;i;i=i->next) {
		cts_email *addition = i->data;

		if (NULL == addition->email_addr) continue;

		for (j=orig;j;j=j->next) {
			cts_email *org = j->data;
			if (org->deleted) continue;
			if (addition->email_addr && org->email_addr
					&& 0 == strcmp(addition->email_addr, org->email_addr))
				break;
		}
		if (NULL == j) {
			orig = g_slist_append(orig, addition);
			i->data = NULL;
		}
	}

	return orig;
}

static inline GSList* cts_merge_vcard_events(GSList *orig, GSList *addition)
{
	GSList *i, *j;
	for (i=addition;i;i=i->next) {
		cts_event *addition = i->data;
		for (j=orig;j;j=j->next) {
			cts_event *org = j->data;
			if (org->deleted) continue;
			if (addition->date == org->date)
				break;
		}
		if (NULL == j) {
			orig = g_slist_append(orig, addition);
			i->data = NULL;
		}
	}

	return orig;
}

static inline GSList* cts_merge_vcard_postals(GSList *orig, GSList *addition)
{
	GSList *i, *j;
	for (i=addition;i;i=i->next) {
		cts_postal *addition = i->data;
		for (j=orig;j;j=j->next) {
			cts_postal *org = j->data;
			if (org->deleted) continue;
			char *s1, *s2;
			s1 = addition->pobox;
			s2 = org->pobox;
			if (s1 == s2 || (s1 && s2 && 0 == strcmp(s1, s2))) {
				s1 = addition->postalcode;
				s2 = org->postalcode;
				if (s1 == s2 || (s1 && s2 && 0 == strcmp(s1, s2))) {
					s1 = addition->region;
					s2 = org->region;
					if (s1 == s2 || (s1 && s2 && 0 == strcmp(s1, s2))) {
						s1 = addition->locality;
						s2 = org->locality;
						if (s1 == s2 || (s1 && s2 && 0 == strcmp(s1, s2))) {
							s1 = addition->street;
							s2 = org->street;
							if (s1 == s2 || (s1 && s2 && 0 == strcmp(s1, s2))) {
								s1 = addition->extended;
								s2 = org->extended;
								if (s1 == s2 || (s1 && s2 && 0 == strcmp(s1, s2))) {
									s1 = addition->country;
									s2 = org->country;
									if (s1 == s2 || (s1 && s2 && 0 == strcmp(s1, s2))) {
										break;
									}
								}
							}
						}
					}
				}
			}
		}
		if (NULL == j) {
			orig = g_slist_append(orig, addition);
			i->data = NULL;
		}
	}

	return orig;
}

static inline GSList* cts_merge_vcard_webs(GSList *orig, GSList *addition)
{
	GSList *i, *j;
	for (i=addition;i;i=i->next) {
		cts_web *addition = i->data;

		if (NULL == addition->url) continue;

		for (j=orig;j;j=j->next) {
			cts_web *org = j->data;
			if (org->deleted) continue;
			if (addition->url && org->url
					&& 0 == strcmp(addition->url, org->url))
				break;
		}
		if (NULL == j) {
			orig = g_slist_append(orig, addition);
			i->data = NULL;
		}
	}

	return orig;
}

static inline GSList* cts_merge_vcard_nicknames(GSList *orig, GSList *addition)
{
	GSList *i, *j;
	for (i=addition;i;i=i->next) {
		cts_nickname *addition = i->data;

		if (NULL == addition->nick) continue;

		for (j=orig;j;j=j->next) {
			cts_nickname *org = j->data;
			if (org->deleted) continue;
			if (addition->nick && org->nick
					&& 0 == strcmp(addition->nick, org->nick))
				break;
		}
		if (NULL == j) {
			orig = g_slist_append(orig, addition);
			i->data = NULL;
		}
	}

	return orig;
}

static inline GSList* cts_merge_vcard_extends(GSList *orig, GSList *addition)
{
	GSList *d, *t;
	cts_extend *orig_val, *addition_val;

	for (d=addition;d;d=d->next) {
		addition_val = d->data;

		for (t=orig;t;t=t->next) {
			orig_val = t->data;
			if (orig_val->deleted) continue;
			if (addition_val->type == orig_val->type)
				break;
		}
		if (NULL == t) {
			orig = g_slist_append(orig, addition_val);
			d->data = NULL;
		}
	}

	return orig;
}

static inline int cts_merge_vcard_company(cts_company *orig, cts_company *addition)
{
	bool temp;

	retvm_if(NULL == addition, CTS_ERR_ARG_INVALID, "Invalid addition(%p)", addition);

	CTS_VCARD_MOVE_TO(orig->name, addition->name, temp);
	CTS_VCARD_MOVE_TO(orig->department, addition->department, temp);
	CTS_VCARD_MOVE_TO(orig->jot_title, addition->jot_title, temp);
	CTS_VCARD_MOVE_TO(orig->role, addition->role, temp);
	CTS_VCARD_MOVE_TO(orig->assistant_name, addition->assistant_name, temp);

	return CTS_SUCCESS;
}

static inline GSList* cts_merge_vcard_grouprel(GSList *orig, GSList *addition)
{
	GSList *i, *j;
	for (i=addition;i;i=i->next) {
		cts_group *addition = i->data;

		if (0 == addition->id) continue;

		for (j=orig;j;j=j->next) {
			cts_group *org = j->data;
			if (org->deleted) continue;
			if (addition->id == org->id)
				break;
		}
		if (NULL == j) {
			orig = g_slist_append(orig, addition);
			i->data = NULL;
		}
	}

	return orig;
}

API int contacts_svc_struct_merge(CTSstruct *s1, CTSstruct *s2)
{
	contact_t *orig, *addition;

	retv_if(NULL == s1, CTS_ERR_ARG_NULL);
	retv_if(NULL == s2, CTS_ERR_ARG_NULL);

	orig = (contact_t *)s1;
	addition = (contact_t *)s2;

	if (orig->base) {
		cts_merge_vcard_base(orig->base, addition->base);
	} else {
		orig->base = addition->base;
		addition->base = NULL;
	}

	if (orig->name) {
		cts_merge_vcard_name(orig->name, addition->name);
	} else {
		orig->name = addition->name;
		addition->name = NULL;
	}

	if (orig->numbers) {
		orig->numbers =
			cts_merge_vcard_numbers(orig->numbers, addition->numbers);
	} else {
		orig->numbers = addition->numbers;
		addition->numbers = NULL;
	}

	if (orig->emails) {
		orig->emails =
			cts_merge_vcard_emails(orig->emails, addition->emails);
	} else {
		orig->emails = addition->emails;
		addition->emails = NULL;
	}
	//orig->grouprelations does not support.

	if (orig->events) {
		orig->events =
			cts_merge_vcard_events(orig->events, addition->events);
	} else {
		orig->events = addition->events;
		addition->events = NULL;
	}
	//orig->messengers does not support.

	if (orig->postal_addrs) {
		orig->postal_addrs =
			cts_merge_vcard_postals(orig->postal_addrs, addition->postal_addrs);
	} else {
		orig->postal_addrs = addition->postal_addrs;
		addition->postal_addrs = NULL;
	}

	if (orig->web_addrs) {
		orig->web_addrs =
			cts_merge_vcard_webs(orig->web_addrs, addition->web_addrs);
	} else {
		orig->web_addrs = addition->web_addrs;
		addition->web_addrs = NULL;
	}

	if (orig->nicknames) {
		orig->nicknames =
			cts_merge_vcard_nicknames(orig->nicknames, addition->nicknames);
	} else {
		orig->nicknames = addition->nicknames;
		addition->nicknames = NULL;
	}

	if (orig->company) {
		cts_merge_vcard_company(orig->company, addition->company);
	} else {
		orig->company = addition->company;
		addition->company = NULL;
	}

	if (orig->grouprelations) {
		cts_merge_vcard_grouprel(orig->grouprelations, addition->grouprelations);
	} else {
		orig->grouprelations = addition->grouprelations;
		addition->grouprelations = NULL;
	}

	if (orig->extended_values) {
		orig->extended_values =
			cts_merge_vcard_extends(orig->extended_values, addition->extended_values);
	} else {
		orig->extended_values = addition->extended_values;
		addition->extended_values = NULL;
	}

	return CTS_SUCCESS;
}

static inline cts_ct_base* cts_struct_dup_base(const cts_ct_base *src)
{
	cts_ct_base *result = NULL;
	if (src) {
		result = calloc(1, sizeof(cts_ct_base));
		retvm_if(NULL == result, NULL, "calloc() Failed");

		memcpy(result, src, sizeof(cts_ct_base));

		if (src->uid)
			result->uid = strdup(src->uid);
		if (src->img_path)
			result->img_path = strdup(src->img_path);
		if (src->full_img_path)
			result->full_img_path = strdup(src->full_img_path);
		if (src->ringtone_path)
			result->ringtone_path = strdup(src->ringtone_path);
		if (src->note)
			result->note = strdup(src->note);
		if (src->vcard_img_path)
			result->vcard_img_path = strdup(src->vcard_img_path);
	}

	return result;
}

static inline cts_name* cts_struct_dup_name(const cts_name *src)
{
	cts_name *result = NULL;
	if (src) {
		result = calloc(1, sizeof(cts_name));
		retvm_if(NULL == result, NULL, "calloc() Failed");

		memcpy(result, src, sizeof(cts_name));

		if (src->first)
			result->first = strdup(src->first);
		if (src->last)
			result->last = strdup(src->last);
		if (src->addition)
			result->addition = strdup(src->addition);
		if (src->display)
			result->display = strdup(src->display);
		if (src->prefix)
			result->prefix = strdup(src->prefix);
		if (src->suffix)
			result->suffix = strdup(src->suffix);
	}

	return result;
}

static inline cts_number* cts_struct_dup_number(const cts_number *src)
{
	cts_number *result = NULL;
	if (src) {
		result = calloc(1, sizeof(cts_number));
		retvm_if(NULL == result, NULL, "calloc() Failed");

		memcpy(result, src, sizeof(cts_number));

		if (src->number)
			result->number = strdup(src->number);
	}

	return result;
}

static inline cts_email* cts_struct_dup_email(const cts_email *src)
{
	cts_email *result = NULL;
	if (src) {
		result = calloc(1, sizeof(cts_email));
		retvm_if(NULL == result, NULL, "calloc() Failed");

		memcpy(result, src, sizeof(cts_email));

		if (src->email_addr)
			result->email_addr = strdup(src->email_addr);
	}

	return result;
}

static inline cts_web* cts_struct_dup_web(const cts_web *src)
{
	cts_web *result = NULL;
	if (src) {
		result = calloc(1, sizeof(cts_web));
		retvm_if(NULL == result, NULL, "calloc() Failed");

		memcpy(result, src, sizeof(cts_web));

		if (src->url)
			result->url = strdup(src->url);
	}

	return result;
}

static inline cts_postal* cts_struct_dup_postal(const cts_postal *src)
{
	cts_postal *result = NULL;
	if (src) {
		result = calloc(1, sizeof(cts_postal));
		retvm_if(NULL == result, NULL, "calloc() Failed");

		memcpy(result, src, sizeof(cts_postal));

		if (src->pobox)
			result->pobox = strdup(src->pobox);
		if (src->postalcode)
			result->postalcode = strdup(src->postalcode);
		if (src->region)
			result->region = strdup(src->region);
		if (src->locality)
			result->locality = strdup(src->locality);
		if (src->street)
			result->street = strdup(src->street);
		if (src->extended)
			result->extended = strdup(src->extended);
		if (src->country)
			result->country = strdup(src->country);
	}

	return result;
}

static inline cts_event* cts_struct_dup_event(const cts_event *src)
{
	cts_event *result = NULL;
	if (src) {
		result = calloc(1, sizeof(cts_event));
		retvm_if(NULL == result, NULL, "calloc() Failed");

		memcpy(result, src, sizeof(cts_event));
	}

	return result;
}

static inline cts_messenger* cts_struct_dup_messenger(const cts_messenger *src)
{
	cts_messenger *result = NULL;
	if (src) {
		result = calloc(1, sizeof(cts_messenger));
		retvm_if(NULL == result, NULL, "calloc() Failed");

		memcpy(result, src, sizeof(cts_messenger));

		if (src->im_id)
			result->im_id = strdup(src->im_id);
		if (src->svc_name)
			result->svc_name = strdup(src->svc_name);
		if (src->svc_op)
			result->svc_op = strdup(src->svc_op);
	}

	return result;
}

static inline cts_group* cts_struct_dup_grouprel(const cts_group *src)
{
	cts_group *result = NULL;
	if (src) {
		result = calloc(1, sizeof(cts_group));
		retvm_if(NULL == result, NULL, "calloc() Failed");

		memcpy(result, src, sizeof(cts_group));

		if (src->name)
			result->name = strdup(src->name);
		if (src->ringtone_path)
			result->ringtone_path = strdup(src->ringtone_path);
	}

	return result;
}

static inline cts_extend* cts_struct_dup_extend(const cts_extend *src)
{
	cts_extend *result = NULL;
	if (src) {
		result = calloc(1, sizeof(cts_extend));
		retvm_if(NULL == result, NULL, "calloc() Failed");

		memcpy(result, src, sizeof(cts_extend));

		if (src->data2)
			result->data2 = strdup(src->data2);
		if (src->data3)
			result->data3 = strdup(src->data3);
		if (src->data4)
			result->data4 = strdup(src->data4);
		if (src->data5)
			result->data5 = strdup(src->data5);
		if (src->data6)
			result->data6 = strdup(src->data6);
		if (src->data7)
			result->data7 = strdup(src->data7);
		if (src->data8)
			result->data8 = strdup(src->data8);
		if (src->data9)
			result->data9 = strdup(src->data9);
		if (src->data10)
			result->data10 = strdup(src->data10);
	}

	return result;
}

static inline cts_nickname* cts_struct_dup_nick(const cts_nickname *src)
{
	cts_nickname *result = NULL;
	if (src) {
		result = calloc(1, sizeof(cts_nickname));
		retvm_if(NULL == result, NULL, "calloc() Failed");

		memcpy(result, src, sizeof(cts_nickname));

		if (src->nick)
			result->nick = strdup(src->nick);
	}

	return result;
}

static inline GSList* cts_struct_dup_list(int type, GSList *src)
{
	GSList *cur, *result = NULL;
	if (src) {
		result = g_slist_copy(src);

		for (cur=result;cur;cur=cur->next) {
			switch (type) {
			case CTS_VALUE_NUMBER:
				cur->data = cts_struct_dup_number(cur->data);
				break;
			case CTS_VALUE_EMAIL:
				cur->data = cts_struct_dup_email(cur->data);
				break;
			case CTS_VALUE_WEB:
				cur->data = cts_struct_dup_web(cur->data);
				break;
			case CTS_VALUE_POSTAL:
				cur->data = cts_struct_dup_postal(cur->data);
				break;
			case CTS_VALUE_EVENT:
				cur->data = cts_struct_dup_event(cur->data);
				break;
			case CTS_VALUE_MESSENGER:
				cur->data = cts_struct_dup_messenger(cur->data);
				break;
			case CTS_VALUE_GROUP_RELATION:
				cur->data = cts_struct_dup_grouprel(cur->data);
				break;
			case CTS_VALUE_EXTEND:
				cur->data = cts_struct_dup_extend(cur->data);
				break;
			case CTS_VALUE_NICKNAME:
				cur->data = cts_struct_dup_nick(cur->data);
				break;
			default:
				ERR("invalid type(%d)", type);
				break;
			}
		}
	}

	return result;
}

static inline cts_company* cts_struct_dup_company(const cts_company *src)
{
	cts_company *result = NULL;
	if (src) {
		result = calloc(1, sizeof(cts_company));
		retvm_if(NULL == result, NULL, "calloc() Failed");

		memcpy(result, src, sizeof(cts_company));

		if (src->name)
			result->name = strdup(src->name);
		if (src->department)
			result->department = strdup(src->department);
		if (src->jot_title)
			result->jot_title = strdup(src->jot_title);
		if (src->role)
			result->role = strdup(src->role);
		if (src->assistant_name)
			result->assistant_name = strdup(src->assistant_name);
	}

	return result;
}

API CTSstruct* contacts_svc_struct_duplicate(const CTSstruct *contact)
{
	contact_t *src, *result = NULL;

	retvm_if(NULL == contact, NULL, "contact is NULL");

	src = (contact_t *)contact;
	result = (contact_t *)contacts_svc_struct_new(CTS_STRUCT_CONTACT);
	retvm_if(NULL == result, NULL, "contacts_svc_struct_new() Failed");

	result->base = cts_struct_dup_base(src->base);
	result->name = cts_struct_dup_name(src->name);
	result->numbers = cts_struct_dup_list(CTS_VALUE_NUMBER, src->numbers);
	result->emails = cts_struct_dup_list(CTS_VALUE_EMAIL, src->emails);
	result->web_addrs = cts_struct_dup_list(CTS_VALUE_WEB, src->web_addrs);
	result->postal_addrs = cts_struct_dup_list(CTS_VALUE_POSTAL, src->postal_addrs);
	result->events = cts_struct_dup_list(CTS_VALUE_EVENT, src->events);
	result->messengers = cts_struct_dup_list(CTS_VALUE_MESSENGER, src->messengers);
	result->grouprelations = cts_struct_dup_list(CTS_VALUE_GROUP_RELATION, src->grouprelations);
	result->company = cts_struct_dup_company(src->company);
	result->extended_values = cts_struct_dup_list(CTS_VALUE_EXTEND, src->extended_values);
	result->nicknames = cts_struct_dup_list(CTS_VALUE_NICKNAME, src->nicknames);

	result->default_num = src->default_num;
	result->default_email = src->default_email;

	return (CTSstruct *)result;
}


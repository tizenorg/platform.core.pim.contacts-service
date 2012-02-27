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
#include <errno.h>

#include "cts-internal.h"
#include "cts-types.h"
#include "cts-contact.h"
#include "cts-vcard.h"
#include "cts-utils.h"
#include "cts-sqlite.h"
#include "cts-vcard-file.h"
#include "cts-struct-ext.h"

API int contacts_svc_get_vcard_from_contact(
		const CTSstruct *contact, char **vcard_stream)
{
	return cts_vcard_make(contact, vcard_stream, CTS_VCARD_CONTENT_BASIC);
}

API int contacts_svc_get_contact_from_vcard(
		const char *vcard_stream, CTSstruct **contact)
{
	int ret;

	ret = cts_vcard_parse(vcard_stream, contact, CTS_VCARD_CONTENT_BASIC);
	retvm_if(ret, ret, "cts_vcard_parse() Failed(%d)", ret);

	return CTS_SUCCESS;
}

static inline void cts_remove_name(cts_name *name)
{
	name->is_changed = true;
	if (name->first) {
		free(name->first);
		name->first = NULL;
	}
	if (name->last) {
		free(name->last);
		name->last = NULL;
	}
	if (name->addition) {
		free(name->addition);
		name->addition = NULL;
	}
	if (name->display) {
		free(name->display);
		name->display = NULL;
	}
	if (name->prefix) {
		free(name->prefix);
		name->prefix = NULL;
	}
	if (name->suffix) {
		free(name->suffix);
		name->suffix = NULL;
	}
}

static inline void cts_remove_company(cts_company *company)
{
	if (company->name) {
		free(company->name);
		company->name = NULL;
	}
	if (company->department) {
		free(company->department);
		company->department = NULL;
	}
	if (company->jot_title) {
		free(company->jot_title);
		company->jot_title = NULL;
	}
	if (company->role) {
		free(company->role);
		company->role = NULL;
	}
}

static inline void cts_remove_base(cts_ct_base *base)
{
	if (base->img_path) {
		free(base->img_path);
		base->img_path = NULL;
		base->img_changed = true;
	}
	if (base->full_img_path) {
		free(base->full_img_path);
		base->full_img_path = NULL;
		base->full_img_changed = true;
	}
	if (base->note) {
		free(base->note);
		base->note = NULL;
		base->note_changed = true;
	}
}

static void cts_remove_number(gpointer data, gpointer user_data)
{
	((cts_number*)data)->deleted = true;
}

void cts_remove_email(gpointer data, gpointer user_data)
{
	((cts_email*)data)->deleted = true;
}

void cts_remove_event(gpointer data, gpointer user_data)
{
	((cts_event*)data)->deleted = true;
}

void cts_remove_postal(gpointer data, gpointer user_data)
{
	((cts_postal*)data)->deleted = true;
}

void cts_remove_web(gpointer data, gpointer user_data)
{
	((cts_web*)data)->deleted = true;
}

void cts_remove_nick(gpointer data, gpointer user_data)
{
	((cts_nickname*)data)->deleted = true;
}

void cts_remove_grouprel(gpointer data, gpointer user_data)
{
	((cts_group*)data)->deleted = true;
}

/*
	void cts_remove_extend(gpointer data, gpointer user_data)
	{
	cts_extend *extend = data;
	if(0000 == extend->type) extend->deleted = true;
	}
	*/

static inline void cts_contact_remove_vcard_field(contact_t *contact, int flags)
{
	if (contact->name)
		cts_remove_name(contact->name);
	if (contact->company)
		cts_remove_company(contact->company);
	if (contact->base)
		cts_remove_base(contact->base);

	g_slist_foreach(contact->numbers, cts_remove_number, NULL);
	g_slist_foreach(contact->emails, cts_remove_email, NULL);
	g_slist_foreach(contact->events, cts_remove_event, NULL);
	g_slist_foreach(contact->postal_addrs, cts_remove_postal, NULL);
	g_slist_foreach(contact->web_addrs, cts_remove_web, NULL);
	g_slist_foreach(contact->nicknames, cts_remove_nick, NULL);
	if (flags & CTS_VCARD_CONTENT_X_SLP_GROUP)
		g_slist_foreach(contact->grouprelations, cts_remove_grouprel, NULL);
	//g_slist_foreach(contact->extended_values, cts_remove_extend, NULL);
}

API int contacts_svc_insert_vcard(int addressbook_id, const char* a_vcard_stream)
{
	int ret;
	CTSstruct *vcard_ct;

	retv_if(NULL == a_vcard_stream, CTS_ERR_ARG_NULL);

	ret = cts_vcard_parse(a_vcard_stream, &vcard_ct, CTS_VCARD_CONTENT_BASIC);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_vcard_parse() Failed(%d)", ret);

	ret = contacts_svc_insert_contact(addressbook_id, vcard_ct);
	warn_if(ret < CTS_SUCCESS, "contacts_svc_insert_contact() Failed(%d)", ret);

	contacts_svc_struct_free(vcard_ct);

	return ret;
}

API int contacts_svc_replace_by_vcard(int contact_id, const char* a_vcard_stream)
{
	int ret;
	CTSstruct *vcard_ct, *contact=NULL;

	retv_if(NULL == a_vcard_stream, CTS_ERR_ARG_NULL);

	ret = contacts_svc_get_contact(contact_id, &contact);
	retvm_if(CTS_SUCCESS != ret, ret, "contacts_svc_get_contact() Failed(%d)", ret);

	ret = cts_vcard_parse(a_vcard_stream, &vcard_ct, CTS_VCARD_CONTENT_BASIC);
	if (CTS_SUCCESS != ret) {
		if (contact) contacts_svc_struct_free(contact);
		ERR("cts_vcard_parse() Failed(%d)", ret);
		return ret;
	}

	cts_contact_remove_vcard_field((contact_t *)contact, CTS_VCARD_CONTENT_BASIC);
	ret = contacts_svc_struct_merge(contact, vcard_ct);
	if (CTS_SUCCESS == ret) {
		ret = contacts_svc_update_contact(contact);
		warn_if(CTS_SUCCESS != ret, "contacts_svc_update_contact() Failed(%d)", ret);
	} else {
		ERR("contacts_svc_struct_merge() Failed(%d)", ret);
	}

	contacts_svc_struct_free(contact);
	contacts_svc_struct_free(vcard_ct);

	return ret;
}

#define CTS_VCARD_MAX_SIZE 1024*1024

API int contacts_svc_vcard_foreach(const char *vcard_file_name,
		int (*fn)(const char *a_vcard_stream, void *data), void *data)
{
	FILE *file;
	int buf_size, len;
	char *stream;
	char line[1024];

	retv_if(NULL == vcard_file_name, CTS_ERR_ARG_NULL);
	retv_if(NULL == fn, CTS_ERR_ARG_NULL);

	file = fopen(vcard_file_name, "r");
	retvm_if(NULL == file, CTS_ERR_FAIL, "fopen() Failed(%d)", errno);

	len = 0;
	buf_size = CTS_VCARD_MAX_SIZE;
	stream = malloc(CTS_VCARD_MAX_SIZE);
	retvm_if(NULL == stream, CTS_ERR_OUT_OF_MEMORY, "malloc() Failed");

	while (fgets(line, sizeof(line), file)) {
		if (0 == len)
			if (strncmp(line, "BEGIN:VCARD", sizeof("BEGIN:VCARD")-1))
				continue;

		if (len + sizeof(line) < buf_size)
			len += snprintf(stream + len, buf_size - len, "%s", line);
		else {
			char *new_stream;
			buf_size += sizeof(line) * 2;
			new_stream = realloc(stream, buf_size);
			if (new_stream)
				stream = new_stream;
			else {
				free(stream);
				fclose(file);
				return CTS_ERR_OUT_OF_MEMORY;
			}

			len += snprintf(stream + len, buf_size - len, "%s", line);
		}

		if (0 == strncmp(line, "END:VCARD", 9)) {
			if (fn)
				if (fn(stream, data)) {
					free(stream);
					fclose(file);
					return CTS_ERR_FINISH_ITER;
				}
			len = 0;
		}
	}

	free(stream);
	fclose(file);
	return CTS_SUCCESS;
}

API int contacts_svc_vcard_count(const char *vcard_file_name)
{
	FILE *file;
	int cnt;
	char line[1024];

	retv_if(NULL == vcard_file_name, CTS_ERR_ARG_NULL);

	file = fopen(vcard_file_name, "r");
	retvm_if(NULL == file, CTS_ERR_FAIL, "fopen() Failed(%d)", errno);

	cnt = 0;
	while (fgets(line, sizeof(line), file)) {
		if (0 == strncmp(line, "END:VCARD", 9))
			cnt++;
	}
	fclose(file);

	return cnt;
}

static inline char* cts_new_strcpy(char *dest, const char *src, int size)
{
	int i;
	for (i=0;i < size && src[i];i++)
		dest[i] = src[i];
	dest[i] = '\0';

	return &dest[i];
}

API char* contacts_svc_vcard_put_content(const char *vcard_stream,
		const char *content_type, const char *content_value)
{
	int i, org_len, new_len;
	char *new_stream, *cur;
	const char *end_content = "END:VCARD";

	retvm_if(NULL == vcard_stream, NULL, "vcard_stream is NULL");
	retvm_if(NULL == content_type, NULL, "content_type is NULL");
	retvm_if(NULL == content_value, NULL, "content_value is NULL");

	org_len = strlen(vcard_stream);
	new_len = org_len + strlen(content_type) + strlen(content_value) + 8;

	new_stream = malloc(new_len);
	retvm_if(NULL == new_stream, NULL, "malloc() Failed");

	memcpy(new_stream, vcard_stream, org_len);

	i = 1;
	for (cur = new_stream + new_len - 1 ;cur;cur--) {
		if (end_content[9-i] == *cur) {
			if (9 == i) break;
			i++;
			continue;
		} else {
			i = 1;
		}
	}
	if (9 != i) {
		ERR("vcard_stream is invalid(%s)", vcard_stream);
		free(new_stream);
		return NULL;
	}

	cur += snprintf(cur, new_len - (cur - new_stream), "%s:", content_type);
	cur = cts_new_strcpy(cur, content_value, new_len - (cur - new_stream));
	snprintf(cur, new_len - (cur - new_stream), "\r\n%s\r\n", end_content);

	return new_stream;
}

API int contacts_svc_vcard_get_content(const char *vcard_stream,
		const char *content_type, int (*fn)(const char *content_value, void *data), void *data)
{
	int len, buf_size, type_len;
	char *cursor, *found, *value;

	retv_if(NULL == vcard_stream, CTS_ERR_ARG_NULL);
	retv_if(NULL == content_type, CTS_ERR_ARG_NULL);
	retv_if(NULL == fn, CTS_ERR_ARG_NULL);

	type_len = strlen(content_type);
	value = malloc(1024);
	retvm_if(NULL == value, CTS_ERR_OUT_OF_MEMORY, "malloc() Failed");
	buf_size = 1024;

	while ((found = strstr(vcard_stream, content_type))) {
		if ((found != vcard_stream && '\n' != *(found-1))
				&& ':' != *(found+type_len+1))
			continue;

		cursor = found;
		while (*cursor) {
			if ('\r' == *cursor) cursor++;
			if ('\n' == *cursor) {
				if (' ' != *(cursor+1))
					break;
			}

			cursor++;
		}
		len = cursor - found;
		if (len < buf_size)
			memcpy(value, found, len);
		else {
			value = realloc(value, len + 1);
			if (value) {
				buf_size = len + 1;
				memcpy(value, found, len);
			}else {
				vcard_stream = found + type_len;
				continue;
			}
		}
		value[len] = '\0';
		if (fn)
			if (fn(value+type_len+1, data)) {
				free(value);
				return CTS_ERR_FINISH_ITER;
			}
		vcard_stream = found + type_len;
	}

	free(value);
	return CTS_SUCCESS;
}

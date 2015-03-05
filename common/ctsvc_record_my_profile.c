/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Dohyung Jin <dh.jin@samsung.com>
 *                 Jongwon Lee <gogosing.lee@samsung.com>
 *                 Donghee Ye <donghee.ye@samsung.com>
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
#include "ctsvc_list.h"
#include "ctsvc_record.h"
#include "ctsvc_view.h"

static int __ctsvc_my_profile_create(contacts_record_h *out_record);
static int __ctsvc_my_profile_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_my_profile_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_my_profile_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_my_profile_get_str(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_my_profile_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_my_profile_set_int(contacts_record_h record, unsigned int property_id, int value);
static int __ctsvc_my_profile_set_str(contacts_record_h record, unsigned int property_id, const char* str );
static int __ctsvc_my_profile_clone_child_record_list(contacts_record_h record, unsigned int property_id, contacts_list_h* out_list );
static int __ctsvc_my_profile_get_child_record_at_p(contacts_record_h record, unsigned int property_id, int index, contacts_record_h* out_record );
static int __ctsvc_my_profile_get_child_record_count(contacts_record_h record, unsigned int property_id, int *count );
static int __ctsvc_my_profile_add_child_record(contacts_record_h record, unsigned int property_id, contacts_record_h child_record );
static int __ctsvc_my_profile_remove_child_record(contacts_record_h record, unsigned int property_id, contacts_record_h child_record );

ctsvc_record_plugin_cb_s my_profile_plugin_cbs = {
	.create = __ctsvc_my_profile_create,
	.destroy = __ctsvc_my_profile_destroy,
	.clone = __ctsvc_my_profile_clone,
	.get_str = __ctsvc_my_profile_get_str,
	.get_str_p = __ctsvc_my_profile_get_str_p,
	.get_int = __ctsvc_my_profile_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_my_profile_set_str,
	.set_int = __ctsvc_my_profile_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = __ctsvc_my_profile_add_child_record,
	.remove_child_record = __ctsvc_my_profile_remove_child_record,
	.get_child_record_count = __ctsvc_my_profile_get_child_record_count,
	.get_child_record_at_p = __ctsvc_my_profile_get_child_record_at_p,
	.clone_child_record_list = __ctsvc_my_profile_clone_child_record_list,
};

static int __ctsvc_my_profile_create(contacts_record_h *out_record)
{
	ctsvc_my_profile_s *my_profile;

	my_profile = calloc(1, sizeof(ctsvc_my_profile_s));
	RETVM_IF(NULL == my_profile, CONTACTS_ERROR_OUT_OF_MEMORY,
			"Out of memory : calloc is failed");

	my_profile->name = calloc(1, sizeof(ctsvc_list_s));
	my_profile->name->l_type = CTSVC_RECORD_NAME;

	my_profile->company = calloc(1, sizeof(ctsvc_list_s));
	my_profile->company->l_type = CTSVC_RECORD_COMPANY;

	my_profile->note = calloc(1, sizeof(ctsvc_list_s));
	my_profile->note->l_type = CTSVC_RECORD_NOTE;

	my_profile->numbers = calloc(1, sizeof(ctsvc_list_s));
	my_profile->numbers->l_type = CTSVC_RECORD_NUMBER;

	my_profile->emails = calloc(1, sizeof(ctsvc_list_s));
	my_profile->emails->l_type = CTSVC_RECORD_EMAIL;

	my_profile->events = calloc(1, sizeof(ctsvc_list_s));
	my_profile->events->l_type = CTSVC_RECORD_EVENT;

	my_profile->messengers = calloc(1, sizeof(ctsvc_list_s));
	my_profile->messengers->l_type = CTSVC_RECORD_MESSENGER;

	my_profile->postal_addrs = calloc(1, sizeof(ctsvc_list_s));
	my_profile->postal_addrs->l_type = CTSVC_RECORD_ADDRESS;

	my_profile->urls = calloc(1, sizeof(ctsvc_list_s));
	my_profile->urls->l_type = CTSVC_RECORD_URL;

	my_profile->nicknames = calloc(1, sizeof(ctsvc_list_s));
	my_profile->nicknames->l_type = CTSVC_RECORD_NICKNAME;

	my_profile->profiles = calloc(1, sizeof(ctsvc_list_s));
	my_profile->profiles->l_type = CTSVC_RECORD_PROFILE;

	my_profile->relationships = calloc(1, sizeof(ctsvc_list_s));
	my_profile->relationships->l_type = CTSVC_RECORD_RELATIONSHIP;

	my_profile->images = calloc(1, sizeof(ctsvc_list_s));
	my_profile->images->l_type = CTSVC_RECORD_IMAGE;

	my_profile->extensions = calloc(1, sizeof(ctsvc_list_s));
	my_profile->extensions->l_type = CTSVC_RECORD_EXTENSION;

	*out_record = (contacts_record_h)my_profile;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_my_profile_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_my_profile_s *my_profile = (ctsvc_my_profile_s*)record;
	my_profile->base.plugin_cbs = NULL;	// help to find double destroy bug (refer to the contacts_record_destroy)
	free(my_profile->base.properties_flags);

	free(my_profile->display_name);
	free(my_profile->reverse_display_name);
	free(my_profile->uid);
	free(my_profile->image_thumbnail_path);

	contacts_list_destroy((contacts_list_h)my_profile->name, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->company, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->note, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->numbers, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->emails, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->events, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->messengers, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->postal_addrs, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->urls, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->nicknames, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->profiles, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->relationships, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->images, delete_child);

	contacts_list_destroy((contacts_list_h)my_profile->extensions, delete_child);

	free(my_profile);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_my_profile_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_my_profile_s *my_profile = (ctsvc_my_profile_s *)record;

	switch(property_id) {
	case CTSVC_PROPERTY_MY_PROFILE_ID:
		*out = my_profile->id;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_ADDRESSBOOK_ID:
		*out = my_profile->addressbook_id;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_CHANGED_TIME:
		*out = my_profile->changed_time;
		break;
	default:
		CTS_ERR("Invalid parameter : property_id(%d) is not supported in value(my_profile)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_my_profile_set_int(contacts_record_h record, unsigned int property_id, int value)
{
	ctsvc_my_profile_s *my_profile = (ctsvc_my_profile_s *)record;

	switch(property_id) {
	case CTSVC_PROPERTY_MY_PROFILE_ID:
		my_profile->id = value;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_CHANGED_TIME:
		my_profile->changed_time = value;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_ADDRESSBOOK_ID:
		RETVM_IF(my_profile->id > 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : property_id(%d) is a read-only value (my_profile)", property_id);
		my_profile->addressbook_id = value;
		break;
	default:
		CTS_ERR("Invalid parameter : property_id(%d) is not supported in valuecontact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_my_profile_get_str_real(contacts_record_h record, unsigned int property_id, char** out_str, bool copy )
{
	ctsvc_my_profile_s *my_profile = (ctsvc_my_profile_s*)record;
	switch(property_id) {
	case CTSVC_PROPERTY_MY_PROFILE_DISPLAY_NAME:
		*out_str = GET_STR(copy, my_profile->display_name);
		break;
	case CTSVC_PROPERTY_MY_PROFILE_IMAGE_THUMBNAIL:
		*out_str = GET_STR(copy, my_profile->image_thumbnail_path);
		break;
	case CTSVC_PROPERTY_MY_PROFILE_UID:
		*out_str = GET_STR(copy, my_profile->uid);
		break;
	default :
		CTS_ERR("Invalid parameter : property_id(%d) is not supported in value(my_profile)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_my_profile_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_my_profile_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_my_profile_get_str(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_my_profile_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_my_profile_get_record_list_p(contacts_record_h record,
		unsigned int property_id, contacts_list_h *list)
{
	ctsvc_my_profile_s *contact = (ctsvc_my_profile_s *)record;

	switch(property_id) {
	case CTSVC_PROPERTY_MY_PROFILE_NAME:
		*list = (contacts_list_h)contact->name;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_COMPANY:
		*list = (contacts_list_h)contact->company;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_NOTE:
		*list = (contacts_list_h)contact->note;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_NUMBER:
		*list = (contacts_list_h)contact->numbers;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_EMAIL:
		*list = (contacts_list_h)contact->emails;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_EVENT:
		*list = (contacts_list_h)contact->events;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_MESSENGER:
		*list = (contacts_list_h)contact->messengers;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_ADDRESS:
		*list = (contacts_list_h)contact->postal_addrs;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_URL:
		*list = (contacts_list_h)contact->urls;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_NICKNAME:
		*list = (contacts_list_h)contact->nicknames;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_PROFILE:
		*list = (contacts_list_h)contact->profiles;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_RELATIONSHIP:
		*list = (contacts_list_h)contact->relationships;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_IMAGE:
		*list = (contacts_list_h)contact->images;
		break;
	case CTSVC_PROPERTY_MY_PROFILE_EXTENSION:
		*list = (contacts_list_h)contact->extensions;
		break;
	default :
		CTS_ERR("Invalid parameter : property_id(%d) is not supported in value(contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_my_profile_get_child_record_count(contacts_record_h record,
		unsigned int property_id, int *count )
{
	int ret;
	contacts_list_h list = NULL;

	*count = 0;
	ret = __ctsvc_my_profile_get_record_list_p(record, property_id, &list);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		return ret;

	if(list)
		contacts_list_get_count(list, count);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_my_profile_get_child_record_at_p(contacts_record_h record,
		unsigned int property_id, int index, contacts_record_h* out_record )
{
	int ret;
	int count;
	contacts_list_h list = NULL;

	ret = __ctsvc_my_profile_get_record_list_p(record, property_id, &list);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		return ret;

	contacts_list_get_count(list, &count);
	if (count < index) {
		CTS_ERR("The index(%d) is greather than total length(%d)", index, count);
		*out_record = NULL;
		return CONTACTS_ERROR_NO_DATA;
	}
	else
		return ctsvc_list_get_nth_record_p(list, index, out_record);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_my_profile_clone_child_record_list(contacts_record_h record,
		unsigned int property_id, contacts_list_h* out_list )
{
	int ret;
	int count;
	contacts_list_h list = NULL;

	ret = __ctsvc_my_profile_get_record_list_p(record, property_id, &list);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		return ret;

	contacts_list_get_count(list, &count);
	if (count <= 0) {
		*out_list = NULL;
		return CONTACTS_ERROR_NO_DATA;
	}
	ctsvc_list_clone(list, out_list);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_my_profile_reset_child_record_id(contacts_record_h child_record)
{
	ctsvc_record_s *record = (ctsvc_record_s*)child_record;

	switch(record->r_type) {
	case CTSVC_RECORD_NAME:
		((ctsvc_name_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_COMPANY:
		((ctsvc_company_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_NOTE:
		((ctsvc_note_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_NUMBER:
		((ctsvc_number_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_EMAIL:
		((ctsvc_email_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_URL:
		((ctsvc_url_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_EVENT:
		((ctsvc_event_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_NICKNAME:
		((ctsvc_nickname_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_ADDRESS:
		((ctsvc_address_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_MESSENGER:
		((ctsvc_messenger_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_GROUP_RELATION:
		((ctsvc_group_relation_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_ACTIVITY:
		((ctsvc_activity_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_PROFILE:
		((ctsvc_profile_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_RELATIONSHIP:
		((ctsvc_relationship_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_IMAGE:
		((ctsvc_image_s *)record)->id = 0;
		break;
	case CTSVC_RECORD_EXTENSION:
		((ctsvc_extension_s *)record)->id = 0;
		break;
	default :
		CTS_ERR("Invalid parameter : record(%d) is not child of contact", record->r_type);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}


static int __ctsvc_my_profile_add_child_record(contacts_record_h record,
		unsigned int property_id, contacts_record_h child_record )
{
	int ret;
	contacts_list_h list = NULL;
	ctsvc_record_s *s_record = (ctsvc_record_s *)child_record;

	ret = __ctsvc_my_profile_get_record_list_p(record, property_id, &list);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		return ret;

	if (CTSVC_RECORD_NAME == s_record->r_type && 1 == ((ctsvc_list_s *)list)->count) {
		CTS_ERR("This type(%d) of child_record can not be added anymore", s_record->r_type);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	if (CTSVC_RECORD_IMAGE == s_record->r_type && 1 == ((ctsvc_list_s *)list)->count) {
		CTS_ERR("This type(%d) of child_record can not be added anymore", s_record->r_type);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	ret = __ctsvc_my_profile_reset_child_record_id(child_record);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		return ret;

	ctsvc_list_add_child(list, child_record);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_my_profile_get_child_record_id(contacts_record_h child_record)
{
	ctsvc_record_s *record = (ctsvc_record_s*)child_record;

	switch(record->r_type) {
	case CTSVC_RECORD_NAME:
		return ((ctsvc_name_s *)record)->id;
	case CTSVC_RECORD_COMPANY:
		return ((ctsvc_company_s *)record)->id;
	case CTSVC_RECORD_NOTE:
		return ((ctsvc_note_s *)record)->id;
	case CTSVC_RECORD_NUMBER:
		return ((ctsvc_number_s *)record)->id;
	case CTSVC_RECORD_EMAIL:
		return ((ctsvc_email_s *)record)->id;
	case CTSVC_RECORD_URL:
		return ((ctsvc_url_s *)record)->id;
	case CTSVC_RECORD_EVENT:
		return ((ctsvc_event_s *)record)->id;
	case CTSVC_RECORD_NICKNAME:
		return ((ctsvc_nickname_s *)record)->id;
	case CTSVC_RECORD_ADDRESS:
		return ((ctsvc_address_s *)record)->id;
	case CTSVC_RECORD_MESSENGER:
		return ((ctsvc_messenger_s *)record)->id;
	case CTSVC_RECORD_GROUP_RELATION:
		return ((ctsvc_group_relation_s *)record)->id;
	case CTSVC_RECORD_ACTIVITY:
		return ((ctsvc_activity_s *)record)->id;
	case CTSVC_RECORD_PROFILE:
		return ((ctsvc_profile_s *)record)->id;
	case CTSVC_RECORD_RELATIONSHIP:
		return ((ctsvc_relationship_s *)record)->id;
	case CTSVC_RECORD_IMAGE:
		return ((ctsvc_image_s *)record)->id;
	case CTSVC_RECORD_EXTENSION:
		return ((ctsvc_extension_s *)record)->id;
	default :
		CTS_ERR("Invalid parameter : record(%d) is not child of contact", record->r_type);
		return 0;
	}
	return 0;
}


static int __ctsvc_my_profile_remove_child_record(contacts_record_h record,
		unsigned int property_id, contacts_record_h child_record )
{
	int id;
	int ret;
	contacts_list_h list = NULL;

	ret = __ctsvc_my_profile_get_record_list_p(record, property_id, &list);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		return ret;

	id = __ctsvc_my_profile_get_child_record_id(child_record);
	ctsvc_list_remove_child(list, child_record, (id?true:false));

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_my_profile_set_str(contacts_record_h record, unsigned int property_id, const char* str )
{
	ctsvc_my_profile_s *my_profile = (ctsvc_my_profile_s *)record;

	switch(property_id) {
	case CTSVC_PROPERTY_MY_PROFILE_DISPLAY_NAME:
		FREEandSTRDUP(my_profile->display_name, str);
		break;
	case CTSVC_PROPERTY_MY_PROFILE_IMAGE_THUMBNAIL:
		FREEandSTRDUP(my_profile->image_thumbnail_path, str);
		break;
	case CTSVC_PROPERTY_MY_PROFILE_UID:
		FREEandSTRDUP(my_profile->uid, str);
		break;
	default :
		CTS_ERR("Invalid parameter : property_id(%d) is not supported in value(my_profile)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_my_profile_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_my_profile_s *out_data = NULL;
	ctsvc_my_profile_s *src_data = NULL;

	src_data = (ctsvc_my_profile_s*)record;
	out_data = calloc(1, sizeof(ctsvc_my_profile_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY,
			"Out of memeory : calloc(ctsvc_my_profile_s) Failed(%d)", CONTACTS_ERROR_OUT_OF_MEMORY);

	out_data->id = src_data->id;
	out_data->addressbook_id = src_data->addressbook_id;
	out_data->changed_time = src_data->changed_time;

	out_data->display_name = SAFE_STRDUP(src_data->display_name);
	out_data->reverse_display_name = SAFE_STRDUP(src_data->reverse_display_name);
	out_data->uid = SAFE_STRDUP(src_data->uid);
	out_data->image_thumbnail_path = SAFE_STRDUP(src_data->image_thumbnail_path);

	ctsvc_list_clone((contacts_list_h)src_data->name, (contacts_list_h*)&out_data->name);
	out_data->name->l_type = CTSVC_RECORD_NAME;

	ctsvc_list_clone((contacts_list_h)src_data->company, (contacts_list_h*)&out_data->company);
	out_data->company->l_type = CTSVC_RECORD_COMPANY;

	ctsvc_list_clone((contacts_list_h)src_data->note, (contacts_list_h*)&out_data->note);
	out_data->note->l_type = CTSVC_RECORD_NOTE;

	ctsvc_list_clone((contacts_list_h)src_data->numbers, (contacts_list_h*)&out_data->numbers);
	out_data->numbers->l_type = CTSVC_RECORD_NUMBER;

	ctsvc_list_clone((contacts_list_h)src_data->emails, (contacts_list_h*)&out_data->emails);
	out_data->emails->l_type = CTSVC_RECORD_EMAIL;

	ctsvc_list_clone((contacts_list_h)src_data->events, (contacts_list_h*)&out_data->events);
	out_data->events->l_type = CTSVC_RECORD_EVENT;

	ctsvc_list_clone((contacts_list_h)src_data->messengers, (contacts_list_h*)&out_data->messengers);
	out_data->messengers->l_type = CTSVC_RECORD_MESSENGER;

	ctsvc_list_clone((contacts_list_h)src_data->postal_addrs, (contacts_list_h*)&out_data->postal_addrs);
	out_data->postal_addrs->l_type = CTSVC_RECORD_ADDRESS;

	ctsvc_list_clone((contacts_list_h)src_data->urls, (contacts_list_h*)&out_data->urls);
	out_data->urls->l_type = CTSVC_RECORD_URL;

	ctsvc_list_clone((contacts_list_h)src_data->nicknames, (contacts_list_h*)&out_data->nicknames);
	out_data->nicknames->l_type = CTSVC_RECORD_NICKNAME;

	ctsvc_list_clone((contacts_list_h)src_data->profiles, (contacts_list_h*)&out_data->profiles);
	out_data->profiles->l_type = CTSVC_RECORD_PROFILE;

	ctsvc_list_clone((contacts_list_h)src_data->relationships, (contacts_list_h*)&out_data->relationships);
	out_data->relationships->l_type = CTSVC_RECORD_RELATIONSHIP;

	ctsvc_list_clone((contacts_list_h)src_data->images, (contacts_list_h*)&out_data->images);
	out_data->images->l_type = CTSVC_RECORD_IMAGE;

	ctsvc_list_clone((contacts_list_h)src_data->extensions, (contacts_list_h*)&out_data->extensions);
	out_data->extensions->l_type = CTSVC_RECORD_EXTENSION;

	CTSVC_RECORD_COPY_BASE(&(out_data->base), &(src_data->base));

	*out_record = (contacts_record_h)out_data;

	return CONTACTS_ERROR_NONE;
}


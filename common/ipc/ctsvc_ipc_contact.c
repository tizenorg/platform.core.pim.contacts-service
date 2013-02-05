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

#include "ctsvc_internal.h"
#include "ctsvc_ipc_marshal.h"

static int __ctsvc_ipc_unmarshal_contact(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record);
static int __ctsvc_ipc_marshal_contact(const contacts_record_h record, pims_ipc_data_h ipc_data);
static int __ctsvc_ipc_marshal_contact_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id);

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_contact_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_contact,
	.marshal_record = __ctsvc_ipc_marshal_contact,
	.get_primary_id = __ctsvc_ipc_marshal_contact_get_primary_id
};

static int __ctsvc_ipc_unmarshal_contact(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record)
{
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(record==NULL,CONTACTS_ERROR_NO_DATA);

	ctsvc_contact_s* pcontact = (ctsvc_contact_s*) record;

	do {
		if (ctsvc_ipc_unmarshal_bool(ipc_data, &pcontact->is_favorite) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->person_id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->changed_time) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->link_mode) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->addressbook_id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_bool(ipc_data, &pcontact->has_phonenumber) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_bool(ipc_data, &pcontact->has_email) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->display_name) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->reverse_display_name) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->display_source_type) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->display_name_language) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->reverse_display_name_language) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->sort_name) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->reverse_sort_name) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->sortkey) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->reverse_sortkey) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->uid) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->image_thumbnail_path) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->ringtone_path) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->vibration) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->message_alert) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->changed_ver) != CONTACTS_ERROR_NONE) break;

		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->name) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->note) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->company) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->numbers) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->emails) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->grouprelations) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->events) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->messengers) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->postal_addrs) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->urls) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->nicknames) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->profiles) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->relationships) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->images) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pcontact->extensions) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->is_unknown) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while(0);

	CTS_ERR("_ctsvc_ipc_unmarshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_contact(const contacts_record_h record, pims_ipc_data_h ipc_data)
{
	ctsvc_contact_s* pcontact = (ctsvc_contact_s*)record;
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(pcontact==NULL,CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_bool((pcontact->is_favorite),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((pcontact->id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((pcontact->person_id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((pcontact->changed_time),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((pcontact->link_mode),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((pcontact->addressbook_id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_bool((pcontact->has_phonenumber),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_bool((pcontact->has_email),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->display_name),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->reverse_display_name),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((pcontact->display_source_type),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((pcontact->display_name_language),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((pcontact->reverse_display_name_language),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->sort_name),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->reverse_sort_name),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->sortkey),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->reverse_sortkey),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->uid),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->image_thumbnail_path),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->ringtone_path),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->vibration),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->message_alert),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((pcontact->changed_ver),ipc_data) != CONTACTS_ERROR_NONE) break;

		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->name, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->note, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->company, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->numbers, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->emails, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->grouprelations, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->events, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->messengers, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->postal_addrs, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->urls, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->nicknames, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->profiles, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->relationships, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->images, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list( (contacts_list_h)pcontact->extensions, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int(pcontact->is_unknown, ipc_data) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while(0);

	CTS_ERR("_ctsvc_ipc_marshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_contact_get_primary_id(const contacts_record_h record, unsigned int *property_id, int *id)
{
	*property_id = CTSVC_PROPERTY_CONTACT_ID;
	return contacts_record_get_int(record, *property_id, id );
}

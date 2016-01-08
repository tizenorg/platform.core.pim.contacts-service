/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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
#include "contacts_record.h"


static int __ctsvc_ipc_unmarshal_person(pims_ipc_data_h ipc_data, const char *view_uri,
		contacts_record_h record)
{
	ctsvc_person_s * person_p = (ctsvc_person_s*)record;

	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_NO_DATA);
	RETV_IF(NULL == record, CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_unmarshal_bool(ipc_data, &person_p->is_favorite) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_bool(ipc_data, &person_p->has_phonenumber) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_bool(ipc_data, &person_p->has_email) != CONTACTS_ERROR_NONE)
			break;

		if (ctsvc_ipc_unmarshal_int(ipc_data, &person_p->person_id) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &person_p->name_contact_id) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &person_p->display_name) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &person_p->display_name_index) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &person_p->image_thumbnail_path) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &person_p->ringtone_path) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &person_p->vibration) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &person_p->message_alert) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &person_p->status) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &person_p->link_count) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &person_p->addressbook_ids) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_snippet(ipc_data, &person_p->snippet) != CONTACTS_ERROR_NONE)
			 break;

		return CONTACTS_ERROR_NONE;
	} while (0);

	ERR("__ctsvc_ipc_unmarshal_person() Fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_person(const contacts_record_h record,
		pims_ipc_data_h ipc_data)
{
	ctsvc_person_s *person_p = (ctsvc_person_s*)record;

	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_NO_DATA);
	RETV_IF(person_p == NULL, CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_bool((person_p->is_favorite), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_bool((person_p->has_phonenumber), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_bool((person_p->has_email), ipc_data) != CONTACTS_ERROR_NONE)
			break;

		if (ctsvc_ipc_marshal_int((person_p->person_id), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_int((person_p->name_contact_id), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((person_p->display_name), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((person_p->display_name_index), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((person_p->image_thumbnail_path), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((person_p->ringtone_path), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((person_p->vibration), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((person_p->message_alert), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((person_p->status), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_int((person_p->link_count), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((person_p->addressbook_ids), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_snippet(&person_p->snippet,ipc_data) != CONTACTS_ERROR_NONE)
			break;

		return CONTACTS_ERROR_NONE;
	} while (0);

	ERR("_ctsvc_ipc_marshal() Fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_person_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_person,
	.marshal_record = __ctsvc_ipc_marshal_person
};


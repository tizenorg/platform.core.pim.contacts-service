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


static int __ctsvc_ipc_unmarshal_simple_contact(pims_ipc_data_h ipc_data,
		const char *view_uri, contacts_record_h record)
{
	ctsvc_simple_contact_s *pcontact = (ctsvc_simple_contact_s*)record;

	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_NO_DATA);
	RETV_IF(NULL == record, CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_unmarshal_bool(ipc_data, &pcontact->is_favorite) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->changed_time) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_bool(ipc_data, &pcontact->has_phonenumber) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_bool(ipc_data, &pcontact->has_email) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->person_id) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->contact_id) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->addressbook_id) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->image_thumbnail_path) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->ringtone_path) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->vibration) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->message_alert) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->display_name) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pcontact->uid) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pcontact->display_source_type) != CONTACTS_ERROR_NONE)
			break;

		return CONTACTS_ERROR_NONE;
	} while (0);

	ERR("__ctsvc_ipc_unmarshal_simple_contact() Fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_simple_contact(const contacts_record_h record,
		pims_ipc_data_h ipc_data)
{
	ctsvc_simple_contact_s *pcontact = (ctsvc_simple_contact_s*)record;

	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_NO_DATA);
	RETV_IF(NULL == pcontact, CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_bool((pcontact->is_favorite), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_int((pcontact->changed_time), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_bool((pcontact->has_phonenumber), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_bool((pcontact->has_email), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_int((pcontact->person_id), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_int((pcontact->contact_id), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_int((pcontact->addressbook_id), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((pcontact->image_thumbnail_path), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((pcontact->ringtone_path), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((pcontact->vibration), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((pcontact->message_alert), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((pcontact->display_name), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((pcontact->uid), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_int((pcontact->display_source_type), ipc_data) != CONTACTS_ERROR_NONE)
			break;

		return CONTACTS_ERROR_NONE;
	} while (0);

	ERR("_ctsvc_ipc_marshal() Fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_simple_contact_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_simple_contact,
	.marshal_record = __ctsvc_ipc_marshal_simple_contact
};


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


static int __ctsvc_ipc_unmarshal_address(pims_ipc_data_h ipc_data,
		const char *view_uri, contacts_record_h record)
{
	ctsvc_address_s *address_p = (ctsvc_address_s*)record;

	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_NO_DATA);
	RETV_IF(NULL == record, CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_unmarshal_bool(ipc_data, &address_p->is_default) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &address_p->id) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &address_p->contact_id) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &address_p->type) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &address_p->label) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &address_p->pobox) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &address_p->postalcode) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &address_p->region) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &address_p->locality) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &address_p->street) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &address_p->extended) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &address_p->country) != CONTACTS_ERROR_NONE)
			break;

		return CONTACTS_ERROR_NONE;
	} while (0);

	ERR("__ctsvc_ipc_unmarshal_address() Fail");

	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_address(const contacts_record_h record,
		pims_ipc_data_h ipc_data)
{
	ctsvc_address_s *address_p = (ctsvc_address_s*)record;

	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_NO_DATA);
	RETV_IF(NULL == address_p, CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_bool((address_p->is_default), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_int((address_p->id), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_int((address_p->contact_id), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_int((address_p->type), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((address_p->label), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((address_p->pobox), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((address_p->postalcode), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((address_p->region), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((address_p->locality), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((address_p->street), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((address_p->extended), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((address_p->country), ipc_data) != CONTACTS_ERROR_NONE)
			break;

		return CONTACTS_ERROR_NONE;
	} while (0);

	ERR("_ctsvc_ipc_marshal() Fail");

	return CONTACTS_ERROR_INVALID_PARAMETER;
}

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_address_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_address,
	.marshal_record = __ctsvc_ipc_marshal_address
};


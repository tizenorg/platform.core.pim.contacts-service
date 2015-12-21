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


static int __ctsvc_ipc_unmarshal_activity(pims_ipc_data_h ipc_data, const char *view_uri,
		contacts_record_h record)
{
	ctsvc_activity_s *activity_p = (ctsvc_activity_s*)record;

	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INTERNAL);
	RETV_IF(NULL == record, CONTACTS_ERROR_INTERNAL);

	do {
		if (ctsvc_ipc_unmarshal_int(ipc_data, &activity_p->id) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &activity_p->contact_id) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &activity_p->source_name) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &activity_p->status) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &activity_p->timestamp) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &activity_p->service_operation) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &activity_p->uri) != CONTACTS_ERROR_NONE)
			break;

		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&activity_p->photos) != CONTACTS_ERROR_NONE)
			break;

		return CONTACTS_ERROR_NONE;

	} while (0);

	ERR("__ctsvc_ipc_unmarshal_activity() Fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_activity(const contacts_record_h record,
		pims_ipc_data_h ipc_data)
{
	ctsvc_activity_s *activity_p = (ctsvc_activity_s*)record;

	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_NO_DATA);
	RETV_IF(activity_p == NULL, CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_int((activity_p->id), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_int((activity_p->contact_id), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((activity_p->source_name), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((activity_p->status), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_int((activity_p->timestamp), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((activity_p->service_operation), ipc_data) != CONTACTS_ERROR_NONE)
			break;
		if (ctsvc_ipc_marshal_string((activity_p->uri), ipc_data) != CONTACTS_ERROR_NONE)
			break;

		if (ctsvc_ipc_marshal_list((contacts_list_h)activity_p->photos, ipc_data) != CONTACTS_ERROR_NONE)
			break;

		return CONTACTS_ERROR_NONE;
	} while (0);

	ERR("_ctsvc_ipc_marshal() Fail");

	return CONTACTS_ERROR_INVALID_PARAMETER;

}

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_activity_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_activity,
	.marshal_record = __ctsvc_ipc_marshal_activity
};


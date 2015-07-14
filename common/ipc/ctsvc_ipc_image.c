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
#include "contacts_record.h"

static int __ctsvc_ipc_unmarshal_image(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record);
static int __ctsvc_ipc_marshal_image(const contacts_record_h record, pims_ipc_data_h ipc_data);

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_image_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_image,
	.marshal_record = __ctsvc_ipc_marshal_image
};


static int __ctsvc_ipc_unmarshal_image(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record)
{
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(record==NULL,CONTACTS_ERROR_NO_DATA);

	ctsvc_image_s* image_p = (ctsvc_image_s*) record;

	do {
		if (ctsvc_ipc_unmarshal_int(ipc_data, &image_p->id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &image_p->contact_id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &image_p->type) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &image_p->label) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &image_p->path) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_bool(ipc_data, &image_p->is_default) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while (0);

	CTS_ERR("_ctsvc_ipc_unmarshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_image(const contacts_record_h record, pims_ipc_data_h ipc_data)
{
	ctsvc_image_s* image_p = (ctsvc_image_s*)record;
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);
	RETV_IF(image_p==NULL,CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_int((image_p->id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((image_p->contact_id),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((image_p->type),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((image_p->label),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((image_p->path),ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_bool((image_p->is_default),ipc_data) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while (0);

	CTS_ERR("_ctsvc_ipc_marshal fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

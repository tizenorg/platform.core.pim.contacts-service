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

#include <glib.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_client_ipc.h"
#include <pims-ipc-data.h>
#include "ctsvc_ipc_marshal.h"

API int contacts_group_add_contact(int group_id, int contact_id)
{
	int ret = CONTACTS_ERROR_NONE;

	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(group_id <= 0 || contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,"id should be greater than 0");
	RETVM_IF(ctsvc_get_ipc_handle() == NULL,CONTACTS_ERROR_IPC,"contacts not connected");

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_int( group_id, indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int( contact_id, indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_GROUP_MODULE, CTSVC_IPC_SERVER_GROUP_ADD_CONTACT, indata, &outdata) != 0)
	{
		CTS_ERR("pims_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check result
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_group_remove_contact(int group_id, int contact_id)
{
	int ret = CONTACTS_ERROR_NONE;

	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(group_id <= 0 || contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,"id should be greater than 0");
	RETVM_IF(ctsvc_get_ipc_handle() == NULL,CONTACTS_ERROR_IPC,"contacts not connected");

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_int( group_id, indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int( contact_id, indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_GROUP_MODULE, CTSVC_IPC_SERVER_GROUP_REMOVE_CONTACT, indata, &outdata) != 0)
	{
		CTS_ERR("pims_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check result
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}


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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_client_ipc.h"
#include <pims-ipc-data.h>

API int contacts_sim_insert(contacts_record_h record, int *contact_id)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;



	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_record( record, indata );
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_SIM_MODULE, CTSVC_IPC_SERVER_SIM_INSERT_CONTACT, indata, &outdata) != 0)
	{
		pims_ipc_data_destroy(indata);
		CTS_ERR("ctsvc_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata)
	{
		// check result
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);

		pims_ipc_data_destroy(outdata);
	}

	pims_ipc_data_destroy(indata);

	return ret;
}

API int contacts_sim_update(contacts_record_h record)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;



	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_record( record, indata );
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_SIM_MODULE, CTSVC_IPC_SERVER_SIM_UPDATE_CONTACT, indata, &outdata) != 0)
	{
		pims_ipc_data_destroy(indata);
		CTS_ERR("ctsvc_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata)
	{
		// check result
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);

		pims_ipc_data_destroy(outdata);
	}

	pims_ipc_data_destroy(indata);

	return ret;
}

API int contacts_sim_delete(int person_id)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(person_id <= 0,CONTACTS_ERROR_INVALID_PARAMETER,"id should be greater than 0");


	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_int( person_id, indata );
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_SIM_MODULE, CTSVC_IPC_SERVER_SIM_DELETE_CONTACT, indata, &outdata) != 0)
	{
		pims_ipc_data_destroy(indata);
		CTS_ERR("ctsvc_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata)
	{
		// check result
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);

		pims_ipc_data_destroy(outdata);
	}

	pims_ipc_data_destroy(indata);

	return ret;
}

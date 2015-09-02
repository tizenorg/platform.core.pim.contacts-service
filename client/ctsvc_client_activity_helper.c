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

#include <glib.h>
#include <pims-ipc-data.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_client_ipc.h"

static const char CONTACTS_READ_PRIVILEGE_ID[] =  "http://tizen.org/privilege/contact.read";
static const char CONTACTS_WRITE_PRIVILEGE_ID[] =  "http://tizen.org/privilege/contact.write";
static const char PHONELOG_READ_PRIVILEGE_ID[] =  "http://tizen.org/privilege/callhistory.read";
static const char PHONELOG_WRITE_PRIVILEGE_ID[] =  "http://tizen.org/privilege/callhistory.write";


int ctsvc_client_activity_delete_by_contact_id(contacts_h contact, int contact_id)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER, "contact is NULL");
	RETVM_IF(contact_id <= 0,CONTACTS_ERROR_INVALID_PARAMETER,"id should be greater than 0");

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	ret = ctsvc_ipc_marshal_int(contact_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_ACTIVITY_MODULE, CTSVC_IPC_SERVER_ACTIVITY_DELETE_BY_CONTACT_ID, indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		// check result
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);
		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			ctsvc_client_ipc_set_change_version(contact, transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_activity_delete_by_account_id(contacts_h contact, int account_id)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER, "contact is NULL");
	RETVM_IF(account_id <= 0,CONTACTS_ERROR_INVALID_PARAMETER,"id should be greater than 0");

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	ret = ctsvc_ipc_marshal_int(account_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_ACTIVITY_MODULE, CTSVC_IPC_SERVER_ACTIVITY_DELETE_BY_ACCOUNT_ID, indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		// check result
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);
		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			ctsvc_client_ipc_set_change_version(contact, transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}


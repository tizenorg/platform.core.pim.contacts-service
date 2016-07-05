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

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"id should be greater than 0");

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		/* LCOV_EXCL_START */
		ERR("pims_ipc_data_create() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_int(contact_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	ret = ctsvc_ipc_call(CTSVC_IPC_ACTIVITY_MODULE,
			CTSVC_IPC_SERVER_ACTIVITY_DELETE_BY_CONTACT_ID, indata, &outdata);
	if (0 != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		int transaction_ver = 0;

		/* check result */
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &transaction_ver)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
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

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(account_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"id should be greater than 0");

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		/* LCOV_EXCL_START */
		ERR("pims_ipc_data_create() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_int(account_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail");
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	ret = ctsvc_ipc_call(CTSVC_IPC_ACTIVITY_MODULE,
			CTSVC_IPC_SERVER_ACTIVITY_DELETE_BY_ACCOUNT_ID, indata, &outdata);
	if (0 != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		int transaction_ver = 0;
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &transaction_ver)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
			ctsvc_client_ipc_set_change_version(contact, transaction_ver);
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}


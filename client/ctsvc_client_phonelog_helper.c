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

#include <pims-ipc-data.h>

#include "contacts.h"
#include "contacts_phone_log_internal.h"
#include "ctsvc_internal.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_client_ipc.h"
#include "ctsvc_ipc_marshal.h"


static const char CONTACTS_READ_PRIVILEGE_ID[] =  "http://tizen.org/privilege/contact.read";
static const char CONTACTS_WRITE_PRIVILEGE_ID[] =  "http://tizen.org/privilege/contact.write";
static const char PHONELOG_READ_PRIVILEGE_ID[] =  "http://tizen.org/privilege/callhistory.read";
static const char PHONELOG_WRITE_PRIVILEGE_ID[] =  "http://tizen.org/privilege/callhistory.write";

int ctsvc_client_phone_log_reset_statistics(contacts_h contact)
{
#ifndef ENABLE_LOG_FEATURE
	return CONTACTS_ERROR_NOT_SUPPORTED;
#endif /* ENABLE_LOG_FEATURE */

	int ret = CONTACTS_ERROR_NONE;

	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER, "contact is NULL");

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

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_PHONELOG_MODULE, CTSVC_IPC_SERVER_PHONELOG_RESET_STATISTICS, indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
		}

		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &transaction_ver)) {
				CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
			}
			ctsvc_client_ipc_set_change_version(contact, transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;

}

int ctsvc_client_phone_log_delete(contacts_h contact, contacts_phone_log_delete_e op, va_list args)
{
#ifndef ENABLE_LOG_FEATURE
	return CONTACTS_ERROR_NOT_SUPPORTED;
#endif /* ENABLE_LOG_FEATURE */

	int ret = CONTACTS_ERROR_NONE;

	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;
	char *number = NULL;
	int extra_data1;

	RETVM_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER, "contact is NULL");

	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		pims_ipc_data_destroy(indata);
		return ret;
	}

	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	ret = ctsvc_ipc_marshal_int(op, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_ipc_marshal_int fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	switch(op) {
	case CONTACTS_PHONE_LOG_DELETE_BY_ADDRESS:
		number = va_arg(args, char *);
		if (NULL == number) {
			pims_ipc_data_destroy(indata);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		ret = ctsvc_ipc_marshal_string(number, indata);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_marshal_string fail");
			pims_ipc_data_destroy(indata);
			return ret;
		}
		break;
	case CONTACTS_PHONE_LOG_DELETE_BY_MESSAGE_EXTRA_DATA1:
	case CONTACTS_PHONE_LOG_DELETE_BY_EMAIL_EXTRA_DATA1:
		extra_data1 = va_arg(args, int);
		ret = ctsvc_ipc_marshal_int(extra_data1, indata);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_marshal_int fail");
			pims_ipc_data_destroy(indata);
			return ret;
		}
		break;
	default:
		CTS_ERR("Invalid parameter : operation is not proper (%d)", ret);
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	if (ctsvc_ipc_call(CTSVC_IPC_PHONELOG_MODULE,
			CTSVC_IPC_SERVER_PHONELOG_DELETE, indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
		}
		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &transaction_ver)) {
				CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
			}
			ctsvc_client_ipc_set_change_version(contact, transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}


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
#include "ctsvc_client_ipc.h"
#include "ctsvc_ipc_marshal.h"

static const char CONTACTS_READ_PRIVILEGE_ID[] =  "http://tizen.org/privilege/contact.read";
static const char CONTACTS_WRITE_PRIVILEGE_ID[] =  "http://tizen.org/privilege/contact.write";
static const char PHONELOG_READ_PRIVILEGE_ID[] =  "http://tizen.org/privilege/callhistory.read";
static const char PHONELOG_WRITE_PRIVILEGE_ID[] =  "http://tizen.org/privilege/callhistory.write";

int ctsvc_client_person_link_person(contacts_h contact, int base_person_id, int person_id)
{
	int ret = CONTACTS_ERROR_NONE;

	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(base_person_id <= 0 || person_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"id should be greater than 0");

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		ERR("pims_ipc_data_create() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}
	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	bool success = false;
	do {
		if (ctsvc_ipc_marshal_int(base_person_id, indata) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int(person_id, indata) != CONTACTS_ERROR_NONE) break;

		success = true;
	} while (0);

	if (success == false) {
		ERR("ctsvc_ipc_marshal_int() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	/*
	   ret = ctsvc_ipc_marshal_int(base_person_id, indata);
	   if (ret != CONTACTS_ERROR_NONE) {
	   ERR("marshal fail");
	   return ret;
	   }
	   ret = ctsvc_ipc_marshal_int(person_id, indata);
	   if (ret != CONTACTS_ERROR_NONE) {
	   ERR("marshal fail");
	   return ret;
	   }
	   */

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_PERSON_MODULE, CTSVC_IPC_SERVER_PERSON_LINK_PERSON, indata, &outdata) != 0) {
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
		}

		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &transaction_ver)) {
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
			}
			ctsvc_client_ipc_set_change_version(contact, transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_person_unlink_contact(contacts_h contact, int person_id, int contact_id, int *unlinked_person_id)
{
	int ret = CONTACTS_ERROR_NONE;

	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(person_id <= 0 || contact_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"id should be greater than 0");

	if (unlinked_person_id)
		*unlinked_person_id = 0;

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		ERR("pims_ipc_data_create() Fail");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	ret = ctsvc_ipc_marshal_int(person_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(contact_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_PERSON_MODULE, CTSVC_IPC_SERVER_PERSON_UNLINK_CONTACT, indata, &outdata) != 0) {
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
		}

		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &transaction_ver)) {
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
			}
			ctsvc_client_ipc_set_change_version(contact, transaction_ver);

			if (unlinked_person_id) {
				if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, unlinked_person_id)) {
					ERR("ctsvc_ipc_unmarshal_int() Fail");
					pims_ipc_data_destroy(outdata);
					return CONTACTS_ERROR_IPC;
				}
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_person_reset_usage(contacts_h contact, int person_id, contacts_usage_type_e type)
{
	int ret = CONTACTS_ERROR_NONE;

	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(person_id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"contact_id should be greater than 0");

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		ERR("pims_ipc_data_create() Fail");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}


	ret = ctsvc_ipc_marshal_int(person_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int((int)type, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_PERSON_MODULE, CTSVC_IPC_SERVER_PERSON_RESET_USAGE, indata, &outdata) != 0) {
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
		}

		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &transaction_ver)) {
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
			}
			ctsvc_client_ipc_set_change_version(contact, transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_person_set_favorite_order(contacts_h contact, int person_id, int previous_person_id, int next_person_id)
{
	int ret = CONTACTS_ERROR_NONE;

	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(person_id <= 0 || previous_person_id < 0 || next_person_id < 0,
			CONTACTS_ERROR_INVALID_PARAMETER, "id should be greater than 0");

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		ERR("pims_ipc_data_create() Fail");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	ret = ctsvc_ipc_marshal_int(person_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(previous_person_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(next_person_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	/* ipc call */
	ret = ctsvc_ipc_call(CTSVC_IPC_PERSON_MODULE,
			CTSVC_IPC_SERVER_PERSON_SET_FAVORITE_ORDER, indata, &outdata);
	if (0 != ret) {
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
		}

		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &transaction_ver)) {
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
			}
			ctsvc_client_ipc_set_change_version(contact, transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;

}

int ctsvc_client_person_set_default_property(contacts_h contact, contacts_person_property_e property,
		int person_id, int id)
{
	int ret = CONTACTS_ERROR_NONE;

	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(person_id <= 0 || id <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"id should be greater than 0");

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		ERR("pims_ipc_data_create() Fail");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	ret = ctsvc_ipc_marshal_int(person_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_unsigned_int(property, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_unsigned_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_PERSON_MODULE, CTSVC_IPC_SERVER_PERSON_SET_DEFAULT_PROPERTY, indata, &outdata) != 0) {
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
		}
		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &transaction_ver)) {
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
			}
			ctsvc_client_ipc_set_change_version(contact, transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_person_get_default_property(contacts_h contact, contacts_person_property_e property,
		int person_id, int *id)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(person_id <= 0 || id == NULL, CONTACTS_ERROR_INVALID_PARAMETER,
			"id should be greater than 0");
	*id = 0;

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		ERR("pims_ipc_data_create() Fail");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	ret = ctsvc_ipc_marshal_int(person_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	ret = ctsvc_ipc_marshal_unsigned_int(property, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		ERR("ctsvc_ipc_marshal_unsigned_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_PERSON_MODULE, CTSVC_IPC_SERVER_PERSON_GET_DEFAULT_PROPERTY,
				indata, &outdata) != 0) {
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
		}
		if (ret == CONTACTS_ERROR_NONE && id) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, id)) {
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}


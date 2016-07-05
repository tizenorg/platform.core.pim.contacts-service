/*
 * Contacts Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <stdlib.h>
#include <pims-ipc-svc.h>
#include "contacts.h"

#include "ctsvc_handle.h"
#include "ctsvc_server_service.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_query.h"
#include "ctsvc_db_access_control.h"

#include "ctsvc_ipc_marshal.h"
#include "ctsvc_internal.h"
#include "ctsvc_ipc_server.h"
#include "ctsvc_db_utils.h"
#include "ctsvc_server_utils.h"

void ctsvc_ipc_server_connect(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("There is no indata Fail");
		ret = CONTACTS_ERROR_SYSTEM;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_connect();

	if (CONTACTS_ERROR_NONE == ret) {
		char *smack = NULL;
		if (0 != pims_ipc_svc_get_smack_label(ipc, &smack))
			ERR("pims_ipc_svc_get_smack_label() Fail()");
		ctsvc_set_client_access_info(ipc, smack);
		free(smack);
	}

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
}

void ctsvc_ipc_server_disconnect(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			ret = CONTACTS_ERROR_IPC;
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	}

	ret = ctsvc_disconnect();

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_trim_memory();
	ctsvc_server_start_timeout();
}

void ctsvc_ipc_server_check_permission(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int permission;
	bool result = false;

	if (NULL == indata) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_INVALID_PARAMETER;
		ERR("check permission Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_unmarshal_int(indata, &permission);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_unmarshal_int() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	result = ctsvc_have_permission(ipc, permission);

ERROR_RETURN:
	*outdata = pims_ipc_data_create(0);
	if (NULL == *outdata) {
		/* LCOV_EXCL_START */
		ERR("pims_ipc_data_create() Fail");
		ctsvc_server_start_timeout();
		return;
		/* LCOV_EXCL_STOP */
	}

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
		/* LCOV_EXCL_START */
		pims_ipc_data_destroy(*outdata);
		*outdata = NULL;
		ERR("ctsvc_ipc_marshal_int() Fail");
		ctsvc_server_start_timeout();
		return;
		/* LCOV_EXCL_STOP */
	}

	if (ret == CONTACTS_ERROR_NONE) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_bool(result, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_bool() Fail");
			ctsvc_server_start_timeout();
			return;
			/* LCOV_EXCL_STOP */
		}
	}
	ctsvc_server_start_timeout();
}

void ctsvc_ipc_server_db_insert_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	int id = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			ret = CONTACTS_ERROR_IPC;
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_ipc_unmarshal_record(indata, &record);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			record = NULL;
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(((ctsvc_record_s*)record)->view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_insert_record(record, &id);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("ctsvc_ipc_marshal_int() Fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
			if (ctsvc_ipc_marshal_int(id, *outdata) != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				pims_ipc_data_destroy(*outdata);
				ERR("ctsvc_ipc_marshal_int() Fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}

DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_record_destroy(record, true);
	ctsvc_server_trim_memory();
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_get_record(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char *view_uri = NULL;
	int id = 0;
	contacts_record_h record = NULL;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			ret = CONTACTS_ERROR_IPC;
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &view_uri);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_string Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &id);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_get_record(view_uri, id, &record);

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			if (ctsvc_ipc_marshal_record(record, *outdata) != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("ctsvc_ipc_marshal_record() Fail");
				goto DATA_FREE;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_record_destroy(record, true);
	free(view_uri);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_update_record(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			ret = CONTACTS_ERROR_IPC;
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_record(indata, &record);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(((ctsvc_record_s*)record)->view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_update_record(record);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("ctsvc_ipc_marshal_int() Fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		}
	}

	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_record_destroy(record, true);
	ctsvc_server_trim_memory();
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_delete_record(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char *view_uri = NULL;
	int id = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			ret = CONTACTS_ERROR_IPC;
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &view_uri);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &id);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_delete_record(view_uri, id);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("ctsvc_ipc_marshal_int() Fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		}
	}

	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}

DATA_FREE:
	ctsvc_handle_destroy(contact);
	free(view_uri);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_replace_record(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	int id = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			ret = CONTACTS_ERROR_IPC;
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_ipc_unmarshal_record(indata, &record);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			record = NULL;
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &id);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_replace_record Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(((ctsvc_record_s*)record)->view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}


	ret = ctsvc_db_replace_record(record, id);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("ctsvc_ipc_marshal_int() Fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}

DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_record_destroy(record, true);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_get_all_records(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char *view_uri = NULL;
	int offset = 0;
	int limit = 0;
	contacts_list_h list = NULL;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			ret = CONTACTS_ERROR_IPC;
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_ipc_unmarshal_string(indata, &view_uri);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &offset);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &limit);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_get_all_records(view_uri, offset, limit, &list);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_list(list, *outdata);

			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				goto DATA_FREE;
				/* LCOV_EXCL_STOP */
			}
		}

	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}

	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_list_destroy(list, true);
	free(view_uri);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_get_records_with_query(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_query_h query = NULL;
	int offset = 0;
	int limit = 0;
	contacts_list_h list = NULL;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_ipc_unmarshal_query(indata, &query);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &offset);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &limit);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(((ctsvc_query_s*)query)->view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_get_records_with_query(query, offset, limit, &list);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_list(list, *outdata);

			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				goto DATA_FREE;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_list_destroy(list, true);
	contacts_query_destroy(query);
	ctsvc_server_start_timeout();
	return;
}


void ctsvc_ipc_server_db_get_count(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char *view_uri = NULL;
	int count = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_ipc_unmarshal_string(indata, &view_uri);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_get_count(view_uri, &count);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_int(count, *outdata);

			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				goto DATA_FREE;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	free(view_uri);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_get_count_with_query(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_query_h query = NULL;
	int count = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_ipc_unmarshal_query(indata, &query);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(((ctsvc_query_s*)query)->view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_get_count_with_query(query, &count);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_int(count, *outdata);

			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				goto DATA_FREE;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_query_destroy(query);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_insert_records(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_list_h list = NULL;
	int id_count = 0;
	int *ids = NULL;
	int i = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_ipc_unmarshal_list(indata, &list);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_list Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (list) {
		contacts_record_h record = NULL;
		contacts_list_first(list);
		do {
			ret = contacts_list_get_current_record_p(list, &record);
			if (CONTACTS_ERROR_NONE != ret) {
				/* LCOV_EXCL_START */
				ERR("contacts_list_get_current_record_p() Fail(%d)", ret);
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}

			if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(((ctsvc_record_s*)record)->view_uri))) {
				/* LCOV_EXCL_START */
				ret = CONTACTS_ERROR_PERMISSION_DENIED;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		} while (CONTACTS_ERROR_NONE == contacts_list_next(list));
		contacts_list_first(list);
	}

	ret = ctsvc_db_insert_records(list, &ids, &id_count);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("ctsvc_ipc_marshal_int() Fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
			/* marshal : id_count+property_id+[ids]*id_count */
			/* id_count */
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(id_count, *outdata)) {
				/* LCOV_EXCL_START */
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("ctsvc_ipc_marshal_int() Fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}

			for (i = 0; i < id_count; i++) {
				/* marshal ids */
				if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ids[i], *outdata)) {
					/* LCOV_EXCL_START */
					pims_ipc_data_destroy(*outdata);
					*outdata = NULL;
					ERR("ctsvc_ipc_marshal_int() Fail");
					ret = CONTACTS_ERROR_OUT_OF_MEMORY;
					goto ERROR_RETURN;
					/* LCOV_EXCL_STOP */
				}
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_list_destroy(list, true);
	free(ids);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_update_records(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_list_h list = NULL;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_ipc_unmarshal_list(indata, &list);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_list Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (list) {
		contacts_record_h record = NULL;
		contacts_list_first(list);
		do {
			ret = contacts_list_get_current_record_p(list, &record);
			if (CONTACTS_ERROR_NONE != ret) {
				/* LCOV_EXCL_START */
				ERR("contacts_list_get_current_record_p() Fail(%d)", ret);
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}

			if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(((ctsvc_record_s*)record)->view_uri))) {
				/* LCOV_EXCL_START */
				ret = CONTACTS_ERROR_PERMISSION_DENIED;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		} while (CONTACTS_ERROR_NONE == contacts_list_next(list));
		contacts_list_first(list);
	}

	ret = ctsvc_db_update_records(list);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("ctsvc_ipc_marshal_int() Fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_list_destroy(list, true);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_delete_records(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int count = 0;
	int *ids = NULL;
	char *uri = NULL;
	int i = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_ipc_unmarshal_string(indata, &uri);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_string Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &count);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		if (count <= 0) {
			/* LCOV_EXCL_START */
			ret = CONTACTS_ERROR_INVALID_PARAMETER;
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ids = (int*)malloc(sizeof(int)*count);
		for (i = 0; i < count; i++) {
			ret = ctsvc_ipc_unmarshal_int(indata, &ids[i]);
			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_delete_records(uri, ids, count);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata)
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
		goto DATA_FREE;
		/* LCOV_EXCL_STOP */

		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("ctsvc_ipc_marshal_int() Fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}

	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	free(uri);
	free(ids);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_replace_records(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_list_h list = NULL;
	int count = 0;
	int *ids = NULL;
	int i = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_ipc_unmarshal_list(indata, &list);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_list Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_ipc_unmarshal_int(indata, &count);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		if (count <= 0) {
			/* LCOV_EXCL_START */
			ret = CONTACTS_ERROR_INVALID_PARAMETER;
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ids = (int*)malloc(sizeof(int)*count);
		for (i = 0; i < count; i++) {
			ret = ctsvc_ipc_unmarshal_int(indata, &ids[i]);
			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_repalce_records Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (list) {
		contacts_record_h record = NULL;
		contacts_list_first(list);
		do {
			ret = contacts_list_get_current_record_p(list, &record);
			if (CONTACTS_ERROR_NONE != ret) {
				/* LCOV_EXCL_START */
				ERR("contacts_list_get_current_record_p() Fail(%d)", ret);
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}

			if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(((ctsvc_record_s*)record)->view_uri))) {
				/* LCOV_EXCL_START */
				ret = CONTACTS_ERROR_PERMISSION_DENIED;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		} while (CONTACTS_ERROR_NONE == contacts_list_next(list));
		contacts_list_first(list);
	}

	ret = ctsvc_db_replace_records(list, ids, count);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ERR("ctsvc_ipc_marshal_int() Fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_list_destroy(list, true);
	free(ids);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_get_changes_by_version(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char *view_uri = NULL;
	int address_book_id = 0;
	int contacts_db_version = 0;
	contacts_list_h record_list = NULL;
	int current_contacts_db_version = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}

		ret = ctsvc_ipc_unmarshal_string(indata, &view_uri);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_string Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &address_book_id);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &contacts_db_version);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_READ)) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_get_changes_by_version(view_uri, address_book_id, contacts_db_version,
			&record_list, &current_contacts_db_version);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_list(record_list, *outdata);
			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_marshal_list Fail");
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
			ret = ctsvc_ipc_marshal_int(current_contacts_db_version, *outdata);
			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_marshal_int() Fail");
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_list_destroy(record_list, true);
	free(view_uri);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_get_current_version(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int contacts_db_version = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_READ) &&
			!ctsvc_have_permission(ipc, CTSVC_PERMISSION_PHONELOG_READ)) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_get_current_version(&contacts_db_version);

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_int(contacts_db_version, *outdata);
			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}

DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_search_records_for_snippet(pims_ipc_h ipc,
		pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char *view_uri = NULL;
	char *keyword = NULL;
	int offset = 0;
	int limit = 0;
	contacts_list_h list = NULL;
	contacts_h contact = NULL;
	char *start_match = NULL;
	char *end_match = NULL;
	int token_number = 0;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &view_uri);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &keyword);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &offset);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &limit);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &start_match);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &end_match);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &token_number);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_search_records_for_snippet(view_uri, keyword, offset, limit,
			start_match, end_match, token_number, &list);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_list(list, *outdata);

			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				goto DATA_FREE;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_list_destroy(list, true);
	free(view_uri);
	free(keyword);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_search_records_with_range_for_snippet(pims_ipc_h ipc,
		pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char *view_uri = NULL;
	char *keyword = NULL;
	int offset = 0;
	int limit = 0;
	int range = 0;
	contacts_list_h list = NULL;
	contacts_h contact = NULL;
	char *start_match = NULL;
	char *end_match = NULL;
	int token_number = 0;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &view_uri);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &keyword);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &offset);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &limit);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &range);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &start_match);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &end_match);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &token_number);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_START */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_search_records_with_range_for_snippet(view_uri, keyword, offset,
			limit, range, start_match, end_match, token_number, &list);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_list(list, *outdata);

			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				goto DATA_FREE;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}

DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_list_destroy(list, true);
	free(view_uri);
	free(keyword);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_search_records_with_query_for_snippet(pims_ipc_h ipc,
		pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_query_h query = NULL;
	char *keyword = NULL;
	int offset = 0;
	int limit = 0;
	contacts_list_h list = NULL;
	contacts_h contact = NULL;
	char *start_match = NULL;
	char *end_match = NULL;
	int token_number = 0;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_query(indata, &query);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &keyword);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &offset);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &limit);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &start_match);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &end_match);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &token_number);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(((ctsvc_query_s*)query)->view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_search_records_with_query_for_snippet(query, keyword, offset, limit,
			start_match, end_match, token_number, &list);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_list(list, *outdata);

			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_marshal_list Fail");
				goto DATA_FREE;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_list_destroy(list, true);
	contacts_query_destroy(query);
	free(keyword);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_search_records(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char *view_uri = NULL;
	char *keyword = NULL;
	int offset = 0;
	int limit = 0;
	contacts_list_h list = NULL;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &view_uri);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &keyword);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &offset);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &limit);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_search_records(view_uri, keyword, offset, limit, &list);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_list(list, *outdata);

			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				goto DATA_FREE;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_list_destroy(list, true);
	free(view_uri);
	free(keyword);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_search_records_with_range(pims_ipc_h ipc,
		pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char *view_uri = NULL;
	char *keyword = NULL;
	int offset = 0;
	int limit = 0;
	int range = 0;
	contacts_list_h list = NULL;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &view_uri);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &keyword);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &offset);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &limit);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &range);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_search_records_with_range(view_uri, keyword, offset, limit, range,
			&list);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_list(list, *outdata);

			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				goto DATA_FREE;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}

DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_list_destroy(list, true);
	free(view_uri);
	free(keyword);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_search_records_with_query(pims_ipc_h ipc,
		pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_query_h query = NULL;
	char *keyword = NULL;
	int offset = 0;
	int limit = 0;
	contacts_list_h list = NULL;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_query(indata, &query);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_string(indata, &keyword);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &offset);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &limit);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}


	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(((ctsvc_query_s*)query)->view_uri))) {
		/* LCOV_EXCL_START */
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
		/* LCOV_EXCL_STOP */
	}


	ret = ctsvc_db_search_records_with_query(query, keyword, offset, limit, &list);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			DBG("no data");
		} else if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_list(list, *outdata);

			if (ret != CONTACTS_ERROR_NONE) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_marshal_list Fail");
				goto DATA_FREE;
				/* LCOV_EXCL_STOP */
			}
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	contacts_list_destroy(list, true);
	contacts_query_destroy(query);
	free(keyword);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_server_db_get_status(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_db_status_e status;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_handle() Fail(%d)", ret);
			goto ERROR_RETURN;
			/* LCOV_EXCL_STOP */
		}
	}

	ret = ctsvc_db_get_status(&status);

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			/* LCOV_EXCL_START */
			ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(status, *outdata)) {
			/* LCOV_EXCL_START */
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
			/* LCOV_EXCL_STOP */
		}
	} else {
		/* LCOV_EXCL_START */
		ERR("outdata is NULL");
		/* LCOV_EXCL_STOP */
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}


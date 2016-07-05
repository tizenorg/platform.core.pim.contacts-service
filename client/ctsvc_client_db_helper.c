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
#include <pims-ipc.h>
#include <pims-ipc-data.h>

#include "contacts.h"

#include "ctsvc_internal.h"
#include "ctsvc_list.h"
#include "ctsvc_record.h"
#include "ctsvc_inotify.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_view.h"
#include "ctsvc_client_ipc.h"
#include "ctsvc_mutex.h"
#include "ctsvc_handle.h"
#include "ctsvc_client_db_helper.h"

#define CTSVC_DEFAULT_START_MATCH "["
#define CTSVC_DEFAULT_END_MATCH "]"

int ctsvc_client_db_insert_record(contacts_h contact, contacts_record_h record, int *id)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	if (id)
		*id = 0;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		/* LCOV_EXCL_START */
		ERR("pims_ipc_data_create() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_record(record, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_record() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_INSERT_RECORD, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &transaction_ver)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
			ctsvc_client_ipc_set_change_version(contact, transaction_ver);

			if (id) {
				if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, id)) {
					/* LCOV_EXCL_START */
					ERR("ctsvc_ipc_unmarshal_int() Fail");
					pims_ipc_data_destroy(outdata);
					return CONTACTS_ERROR_IPC;
					/* LCOV_EXCL_STOP */
				}
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_get_record(contacts_h contact, const char *view_uri, int id, contacts_record_h *out_record)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(id < 0, CONTACTS_ERROR_INVALID_PARAMETER, "id < 0");
	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		/* LCOV_EXCL_START */
		ERR("pims_ipc_data_create() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_string(view_uri, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_RECORD, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_record(outdata, out_record)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_record() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}
		pims_ipc_data_destroy(outdata);
	}
	return ret;
}

int ctsvc_client_db_update_record(contacts_h contact, contacts_record_h record)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);

	/* make indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		/* LCOV_EXCL_START */
		ERR("pims_ipc_data_create() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_handle() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_record(record, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_record() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_UPDATE_RECORD, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE == ret) {
			CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s*)record);
			int transaction_ver = 0;
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

int ctsvc_client_db_delete_record(contacts_h contact, const char *view_uri, int id)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(id <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "id <= 0");

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

	ret = ctsvc_ipc_marshal_string(view_uri, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_DELETE_RECORD, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
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

int ctsvc_client_db_replace_record(contacts_h contact, contacts_record_h record, int id)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);

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

	ret = ctsvc_ipc_marshal_record(record, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_record() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE,
				CTSVC_IPC_SERVER_DB_REPLACE_RECORD, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
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

int ctsvc_client_db_get_all_records(contacts_h contact, const char *view_uri, int offset, int limit, contacts_list_h *out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;
	RETV_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER);

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

	ret = ctsvc_ipc_marshal_string(view_uri, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(offset, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(limit, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_ALL_RECORDS, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (ret == CONTACTS_ERROR_NONE) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_list(outdata, out_list)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_list() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}
		pims_ipc_data_destroy(outdata);
	}
	return ret;
}

int ctsvc_client_db_get_records_with_query(contacts_h contact, contacts_query_h query, int offset, int limit, contacts_list_h *out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;
	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);

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

	ret = ctsvc_ipc_marshal_query(query, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_query() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_int(offset, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(limit, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_RECORDS_WITH_QUERY, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_list(outdata, out_list)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_list() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}


int ctsvc_client_db_get_count(contacts_h contact, const char *view_uri, int *out_count)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_count, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_count = 0;

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

	ret = ctsvc_ipc_marshal_string(view_uri, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_COUNT, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, out_count)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_get_count_with_query(contacts_h contact, contacts_query_h query, int *out_count)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_count, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_count = 0;

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

	ret = ctsvc_ipc_marshal_query(query, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_query() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_COUNT_WITH_QUERY, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, out_count)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_insert_records(contacts_h contact, contacts_list_h list, int **ids, int *count)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	if (ids)
		*ids = NULL;
	if (count)
		*count = 0;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);

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

	ret = ctsvc_ipc_marshal_list(list, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_list() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_INSERT_RECORDS,
				indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &transaction_ver)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
			ctsvc_client_ipc_set_change_version(contact, transaction_ver);

			if (ids && count) {
				int i = 0;
				int *id = NULL;
				int c;

				if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &c)) {
					/* LCOV_EXCL_START */
					ERR("ctsvc_ipc_unmarshal_int() Fail");
					pims_ipc_data_destroy(outdata);
					return CONTACTS_ERROR_IPC;
					/* LCOV_EXCL_STOP */
				}
				id = calloc(c, sizeof(int));
				if (NULL == id) {
					/* LCOV_EXCL_START */
					ERR("calloc() Fail");
					pims_ipc_data_destroy(outdata);
					return CONTACTS_ERROR_OUT_OF_MEMORY;
					/* LCOV_EXCL_STOP */
				}

				for (i = 0; i < c; i++) {
					if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &(id[i]))) {
						/* LCOV_EXCL_START */
						ERR("ctsvc_ipc_unmarshal_int() Fail");
						pims_ipc_data_destroy(outdata);
						free(id);
						return CONTACTS_ERROR_IPC;
						/* LCOV_EXCL_STOP */
					}
				}
				*ids = id;
				*count = c;
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_update_records(contacts_h contact, contacts_list_h list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);

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

	ret = ctsvc_ipc_marshal_list(list, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_list() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_UPDATE_RECORDS,
				indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
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

int ctsvc_client_db_delete_records(contacts_h contact, const char *view_uri, int ids[], int count)
{
	int i;
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER);

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

	ret = ctsvc_ipc_marshal_string(view_uri, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_int(count, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	for (i = 0; i < count; i++) {
		ret = ctsvc_ipc_marshal_int(ids[i], indata);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
			pims_ipc_data_destroy(indata);
			return ret;
			/* LCOV_EXCL_STOP */
		}
	}

	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_DELETE_RECORDS,
				indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
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

int ctsvc_client_db_replace_records(contacts_h contact, contacts_list_h list, int ids[], int count)
{
	int i;
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(0 == count, CONTACTS_ERROR_INVALID_PARAMETER);

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

	ret = ctsvc_ipc_marshal_list(list, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_list() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_ipc_marshal_int(count, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	for (i = 0; i < count; i++) {
		ret = ctsvc_ipc_marshal_int(ids[i], indata);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
			pims_ipc_data_destroy(indata);
			return ret;
			/* LCOV_EXCL_STOP */
		}
	}

	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_REPLACE_RECORDS,
				indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
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

int ctsvc_client_db_get_changes_by_version(contacts_h contact, const char *view_uri, int addressbook_id, int contacts_db_version, contacts_list_h *record_list, int *current_contacts_db_version)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == record_list, CONTACTS_ERROR_INVALID_PARAMETER);

	*record_list = NULL;
	RETV_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == current_contacts_db_version, CONTACTS_ERROR_INVALID_PARAMETER);

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

	ret = ctsvc_ipc_marshal_string(view_uri, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(addressbook_id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(contacts_db_version, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_CHANGES_BY_VERSION, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (ret == CONTACTS_ERROR_NONE) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_list(outdata, record_list)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_list() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, current_contacts_db_version)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_get_current_version(contacts_h contact, int *contacts_db_version)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == contacts_db_version, CONTACTS_ERROR_INVALID_PARAMETER);

	*contacts_db_version = 0;

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

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_CURRENT_VERSION, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, contacts_db_version)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_search_records_for_snippet(contacts_h contact,
		const char *view_uri,
		const char *keyword,
		int offset,
		int limit,
		const char *start_match,
		const char *end_match,
		int token_number,
		contacts_list_h *out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;

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
	ret = ctsvc_ipc_marshal_string(view_uri, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_string(keyword, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(offset, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(limit, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_string(start_match ? start_match : CTSVC_DEFAULT_START_MATCH, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_string(end_match ? end_match : CTSVC_DEFAULT_END_MATCH, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(token_number, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE,
				CTSVC_IPC_SERVER_DB_SEARCH_RECORDS_FOR_SNIPPET,
				indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_list(outdata, out_list)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_list() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_search_records_with_range_for_snippet(contacts_h contact,
		const char *view_uri,
		const char *keyword,
		int offset,
		int limit,
		int range,
		const char *start_match,
		const char *end_match,
		int token_number,
		contacts_list_h *out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(0 == range, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;

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

	ret = ctsvc_ipc_marshal_string(view_uri, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_string(keyword, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(offset, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(limit, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(range, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_string(start_match ? start_match : CTSVC_DEFAULT_START_MATCH, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_string(end_match ? end_match : CTSVC_DEFAULT_END_MATCH, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(token_number, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (0 != ctsvc_ipc_call(CTSVC_IPC_DB_MODULE,
				CTSVC_IPC_SERVER_DB_SEARCH_RECORDS_WITH_RANGE_FOR_SNIPPET,
				indata, &outdata)) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_list(outdata, out_list)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_list() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_search_records_with_query_for_snippet(contacts_h contact,
		contacts_query_h query,
		const char *keyword,
		int offset,
		int limit,
		const char *start_match,
		const char *end_match,
		int token_number,
		contacts_list_h *out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;

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
	ret = ctsvc_ipc_marshal_query(query, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_query() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_string(keyword, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(offset, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(limit, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_string(start_match ? start_match : CTSVC_DEFAULT_START_MATCH, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_string(end_match ? end_match : CTSVC_DEFAULT_END_MATCH, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(token_number, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE,
				CTSVC_IPC_SERVER_DB_SEARCH_RECORDS_WITH_QUERY_FOR_SNIPPET,
				indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_list(outdata, out_list)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_list() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_search_records(contacts_h contact, const char *view_uri, const char *keyword,
		int offset, int limit, contacts_list_h *out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;

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
	ret = ctsvc_ipc_marshal_string(view_uri, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_string(keyword, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(offset, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(limit, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_SEARCH_RECORDS, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_list(outdata, out_list)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_list() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_search_records_with_range(contacts_h contact, const char *view_uri, const char *keyword,
		int offset, int limit, int range, contacts_list_h *out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(0 == range, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;

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

	ret = ctsvc_ipc_marshal_string(view_uri, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_string(keyword, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(offset, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(limit, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(range, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (0 != ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_SEARCH_RECORDS_WITH_RANGE, indata, &outdata)) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_list(outdata, out_list)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_list() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_search_records_with_query(contacts_h contact, contacts_query_h query, const char *keyword,
		int offset, int limit, contacts_list_h *out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;

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
	ret = ctsvc_ipc_marshal_query(query, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_query() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_string(keyword, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_string() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(offset, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	ret = ctsvc_ipc_marshal_int(limit, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	/* ipc call */
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_SEARCH_RECORDS_WITH_QUERY, indata, &outdata) != 0) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}

		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_list(outdata, out_list)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_list() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_get_last_change_version(contacts_h contact, int *last_version)
{
	int ret = CONTACTS_ERROR_NONE;
	bool result = false;

	RETV_IF(NULL == last_version, CONTACTS_ERROR_INVALID_PARAMETER);
	*last_version = 0;

	ret = ctsvc_ipc_client_check_permission(CTSVC_PERMISSION_CONTACT_READ, &result);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission Fail(%d)", ret);
	if (result == false) {
		ret = ctsvc_ipc_client_check_permission(CTSVC_PERMISSION_PHONELOG_READ, &result);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission Fail(%d)", ret);
		RETVM_IF(result == false, CONTACTS_ERROR_PERMISSION_DENIED, "Permission denied");
	}

	*last_version = ctsvc_client_ipc_get_change_version(contact);
	return ret;
}

typedef struct {
	contacts_db_status_changed_cb cb;
	void *user_data;
} status_callback_info_s;

static GSList *__status_change_subscribe_list = NULL;

static void __ctsvc_client_db_free_cb_info(status_callback_info_s *cb_info)
{
	if (NULL == cb_info)
		return;
	free(cb_info);
}

static void __ctsvc_db_status_subscriber_callback(pims_ipc_h ipc, pims_ipc_data_h data,
		void *user_data)
{
	int ret;
	int status = -1;
	GSList *l;

	if (data) {
		ret = ctsvc_ipc_unmarshal_int(data, &status);
		WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_ipc_unmarshal_int() Fail(%d)", ret);
	}

	for (l = __status_change_subscribe_list; l; l = l->next) {
		status_callback_info_s *cb_info = l->data;
		/* TODO: Fixme - check zone_name */
		if (cb_info->cb)
			cb_info->cb(status, cb_info->user_data);
	}
}

int ctsvc_client_db_get_status(contacts_h contact, contacts_db_status_e *status)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h outdata = NULL;
	pims_ipc_data_h indata = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == status, CONTACTS_ERROR_INVALID_PARAMETER);
	*status = 0;

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

	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_STATUS, indata, &outdata) != 0) {
		pims_ipc_data_destroy(indata);
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_call() Fail");
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}
	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			/* LCOV_EXCL_STOP */
		}
		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, (int*)status)) {
				/* LCOV_EXCL_START */
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				/* LCOV_EXCL_STOP */
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_client_db_add_status_changed_cb(contacts_h contact,
		contacts_db_status_changed_cb cb, void *user_data)
{
	int ret;
	status_callback_info_s *cb_info = NULL;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = ctsvc_ipc_create_for_change_subscription();
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_create_for_change_subscription() Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	if (pims_ipc_subscribe(ctsvc_ipc_get_handle_for_change_subsciption(),
				CTSVC_IPC_SUBSCRIBE_MODULE, CTSVC_IPC_SERVER_DB_STATUS_CHANGED,
				__ctsvc_db_status_subscriber_callback, NULL) != 0) {
		/* LCOV_EXCL_START */
		ERR("pims_ipc_subscribe error\n");
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return CONTACTS_ERROR_IPC;
		/* LCOV_EXCL_STOP */
	}

	cb_info = calloc(1, sizeof(status_callback_info_s));
	if (NULL == cb_info) {
		/* LCOV_EXCL_START */
		ERR("calloc() Fail");
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		/* LCOV_EXCL_STOP */
	}
	cb_info->user_data = user_data;
	cb_info->cb = cb;
	__status_change_subscribe_list = g_slist_append(__status_change_subscribe_list, cb_info);

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}

int ctsvc_client_db_remove_status_changed_cb(contacts_h contact,
		contacts_db_status_changed_cb cb, void *user_data)
{
	int ret;
	GSList *l;

	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = ctsvc_ipc_destroy_for_change_subscription(false);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_ipc_destroy_for_change_subscription() Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	for (l = __status_change_subscribe_list; l; l = l->next) {
		status_callback_info_s *cb_info = l->data;
		if (cb == cb_info->cb && user_data == cb_info->user_data) {
			__status_change_subscribe_list = g_slist_remove(__status_change_subscribe_list, cb_info);
			__ctsvc_client_db_free_cb_info(cb_info);
			break;
		}
	}

	if (g_slist_length(__status_change_subscribe_list) == 0) {
		pims_ipc_unsubscribe(ctsvc_ipc_get_handle_for_change_subsciption(),
				CTSVC_IPC_SUBSCRIBE_MODULE, CTSVC_IPC_SERVER_DB_STATUS_CHANGED);
		g_slist_free(__status_change_subscribe_list);
		__status_change_subscribe_list = NULL;
	}

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}

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

#include "ctsvc_service.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_query.h"
#include "ctsvc_db_access_control.h"

#include "ctsvc_ipc_marshal.h"
#include "ctsvc_internal.h"
#include "ctsvc_ipc_server.h"
#include "ctsvc_utils.h"
#include "ctsvc_server_utils.h"

void ctsvc_ipc_server_connect(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;

	ret = contacts_connect();

	if (CONTACTS_ERROR_NONE == ret) {
		char *smack = NULL;
		if (0 != pims_ipc_svc_get_smack_label(ipc, &smack))
			CTS_ERR("pims_ipc_svc_get_smack_label() Fail()");
		ctsvc_set_client_access_info(ipc, smack);
		free(smack);
	}

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (!*outdata) {
			CTS_ERR("pims_ipc_data_create fail");
			return;
		}

		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			return;
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
}

void ctsvc_ipc_server_disconnect(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;

	ret = contacts_disconnect();

	// related data will be freed in __ctsvc_client_disconnected_cb
//	if (ret == CONTACTS_ERROR_NONE)
//		ctsvc_unset_client_access_info();

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			return;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			return;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
	return;
}

void ctsvc_ipc_server_check_permission(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int permission;
	bool result;

	if (NULL == indata) {
		ret = CONTACTS_ERROR_INVALID_PARAMETER;
		CTS_ERR("check permission fail.");
		goto ERROR_RETURN;
	}

	ret = ctsvc_ipc_unmarshal_int(indata, &permission);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_ipc_unmarshal_int fail");
		goto ERROR_RETURN;
	}

	result = ctsvc_have_permission(ipc, permission);

ERROR_RETURN:
	*outdata = pims_ipc_data_create(0);
	if (!*outdata) {
		CTS_ERR("pims_ipc_data_create fail");
		return;
	}

	if (pims_ipc_data_put(*outdata, (void*)&ret, sizeof(int)) != 0) {
		pims_ipc_data_destroy(*outdata);
		*outdata = NULL;
		CTS_ERR("pims_ipc_data_put fail (return value)");
		return;
	}

	if (ret == CONTACTS_ERROR_NONE) {
		if (pims_ipc_data_put(*outdata, (void*)&result, sizeof(bool)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail (id)");
			return;
		}
	}
}

void ctsvc_ipc_server_db_insert_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	int id = 0;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_record(indata,&record);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			record = NULL;
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(((ctsvc_record_s *)record)->view_uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_insert_record(record, &id);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
			if (ctsvc_ipc_marshal_int(id,*outdata) != CONTACTS_ERROR_NONE)
			{
				pims_ipc_data_destroy(*outdata);
				CTS_ERR("ctsvc_ipc_marshal fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}

DATA_FREE:
	if (record)
	{
		contacts_record_destroy(record,true);
	}
	return;
}

void ctsvc_ipc_server_db_get_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char* view_uri = NULL;
	int id = 0;
	contacts_record_h record = NULL;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_string(indata,&view_uri);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_string fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&id);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(view_uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_get_record(view_uri,id,&record);

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if ( CONTACTS_ERROR_NO_DATA == ret )
		{
			CTS_DBG("no data");
		}
		else if( CONTACTS_ERROR_NONE == ret )
		{
			if( ctsvc_ipc_marshal_record(record, *outdata) != CONTACTS_ERROR_NONE )
			{
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_record fail");
				goto DATA_FREE;
			}
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	if (record)
	{
		contacts_record_destroy(record,true);
	}
	CONTACTS_FREE(view_uri);
	return;
}

void ctsvc_ipc_server_db_update_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_record(indata,&record);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(((ctsvc_record_s *)record)->view_uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_update_record(record);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
		}
	}

	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	if (record)
	{
		contacts_record_destroy(record,true);
	}
	return;
}

void ctsvc_ipc_server_db_delete_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char* view_uri = NULL;
	int id = 0;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_string(indata,&view_uri);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&id);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(view_uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_delete_record(view_uri,id);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
		}
	}

	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}

DATA_FREE:

	CONTACTS_FREE(view_uri);
	return;
}

void ctsvc_ipc_server_db_replace_record(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_record_h record = NULL;
	int id = 0;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_record(indata, &record);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			record = NULL;
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_server_db_replace_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(((ctsvc_record_s *)record)->view_uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}


	ret = contacts_db_replace_record(record, id);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (!*outdata) {
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata, (void*)&ret, sizeof(int)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (!*outdata) {
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata, (void*)&ret, sizeof(int)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}

DATA_FREE:
	if (record)
		contacts_record_destroy(record, true);

	return;
}

void ctsvc_ipc_server_db_get_all_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char* view_uri = NULL;
	int offset = 0;
	int limit = 0;
	contacts_list_h list = NULL;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_string(indata,&view_uri);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&offset);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&limit);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(view_uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_get_all_records(view_uri,offset,limit,&list);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if ( CONTACTS_ERROR_NO_DATA == ret )
		{
			CTS_DBG("no data");
		}
		else if( CONTACTS_ERROR_NONE == ret )
		{
			ret = ctsvc_ipc_marshal_list(list,*outdata);

			if (ret != CONTACTS_ERROR_NONE)
			{
				CTS_ERR("ctsvc_ipc_unmarshal_int fail");
				goto DATA_FREE;
			}
		}

	}
	else
	{
		CTS_ERR("outdata is NULL");
	}

	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:

	if (list)
	{
		contacts_list_destroy(list,true);
	}
	CONTACTS_FREE(view_uri);
	return;
}

void ctsvc_ipc_server_db_get_records_with_query(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_query_h query = NULL;
	int offset = 0;
	int limit = 0;
	contacts_list_h list = NULL;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_query(indata,&query);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&offset);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&limit);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(((ctsvc_query_s *)query)->view_uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_get_records_with_query(query,offset,limit,&list);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if ( CONTACTS_ERROR_NO_DATA == ret )
		{
			CTS_DBG("no data");
		}
		else if( CONTACTS_ERROR_NONE == ret )
		{
			ret = ctsvc_ipc_marshal_list(list,*outdata);

			if (ret != CONTACTS_ERROR_NONE)
			{
				CTS_ERR("ctsvc_ipc_unmarshal_int fail");
				goto DATA_FREE;
			}
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:

	if (list)
	{
		contacts_list_destroy(list,true);
	}
	if (query)
	{
		contacts_query_destroy(query);
	}
	return;
}


void ctsvc_ipc_server_db_get_count(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char* view_uri = NULL;
	int count = 0;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_string(indata,&view_uri);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(view_uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_get_count(view_uri,&count);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if ( CONTACTS_ERROR_NO_DATA == ret )
		{
			CTS_DBG("no data");
		}
		else if( CONTACTS_ERROR_NONE == ret )
		{
			ret = ctsvc_ipc_marshal_int(count,*outdata);

			if (ret != CONTACTS_ERROR_NONE)
			{
				CTS_ERR("ctsvc_ipc_unmarshal_int fail");
				goto DATA_FREE;
			}
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	CONTACTS_FREE(view_uri);
	return;
}

void ctsvc_ipc_server_db_get_count_with_query(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_query_h query = NULL;
	int count = 0;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_query(indata,&query);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(((ctsvc_query_s *)query)->view_uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_get_count_with_query(query,&count);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if ( CONTACTS_ERROR_NO_DATA == ret )
		{
			CTS_DBG("no data");
		}
		else if( CONTACTS_ERROR_NONE == ret )
		{
			ret = ctsvc_ipc_marshal_int(count,*outdata);

			if (ret != CONTACTS_ERROR_NONE)
			{
				CTS_ERR("ctsvc_ipc_unmarshal_int fail");
				goto DATA_FREE;
			}
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	if (query)
	{
		contacts_query_destroy(query);
	}
	return;
}

void ctsvc_ipc_server_db_insert_records(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_list_h list = NULL;
	unsigned int id_count = 0;
	int *ids = NULL;
	int i=0;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_list(indata,&list);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_list fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (list) {
		contacts_record_h record = NULL;
		contacts_list_first(list);
		do {
			ret = contacts_list_get_current_record_p(list, &record);
			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("contacts_list_get_current_record_p is faild(%d)", ret);
				goto ERROR_RETURN;
			}

			if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(((ctsvc_record_s *)record)->view_uri))) {
				ret = CONTACTS_ERROR_PERMISSION_DENIED;
				goto ERROR_RETURN;
			}
		} while (CONTACTS_ERROR_NONE == contacts_list_next(list));
		contacts_list_first(list);
	}

	ret = ctsvc_db_insert_records(list, &ids, &id_count);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if(ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
			// marshal : id_count+property_id+[ids]*id_count
			// id_count
			if (pims_ipc_data_put(*outdata,(void*)&id_count,sizeof(int)) != 0)
			{
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("pims_ipc_data_put fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}

			for(i=0;i<id_count;i++)
			{
				// marshal ids
				if (pims_ipc_data_put(*outdata,(void*)&ids[i],sizeof(int)) != 0)
				{
					pims_ipc_data_destroy(*outdata);
					*outdata = NULL;
					CTS_ERR("pims_ipc_data_put fail");
					ret = CONTACTS_ERROR_OUT_OF_MEMORY;
					goto ERROR_RETURN;
				}
			}
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	if (list)
	{
		contacts_list_destroy(list,true);
	}
	CONTACTS_FREE(ids);
	return;
}

void ctsvc_ipc_server_db_update_records(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_list_h list = NULL;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_list(indata,&list);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_list fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (list) {
		contacts_record_h record = NULL;
		contacts_list_first(list);
		do {
			ret = contacts_list_get_current_record_p(list, &record);
			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("contacts_list_get_current_record_p is faild(%d)", ret);
				goto ERROR_RETURN;
			}

			if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(((ctsvc_record_s *)record)->view_uri))) {
				ret = CONTACTS_ERROR_PERMISSION_DENIED;
				goto ERROR_RETURN;
			}
		} while (CONTACTS_ERROR_NONE == contacts_list_next(list));
		contacts_list_first(list);
	}

	ret = contacts_db_update_records(list);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	if (list)
	{
		contacts_list_destroy(list,true);
	}
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

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_string(indata,&uri);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_string fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&count);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		if (count <=0)
		{
			ret = CONTACTS_ERROR_INVALID_PARAMETER;
			goto ERROR_RETURN;
		}
		ids = (int*)malloc(sizeof(int)*count);
		for(i=0;i<count;i++)
		{
			ret = ctsvc_ipc_unmarshal_int(indata,&ids[i]);
			if (ret != CONTACTS_ERROR_NONE)
			{
				CTS_ERR("ctsvc_ipc_unmarshal_int fail");
				goto ERROR_RETURN;
			}
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_delete_records(uri, ids, count);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}

	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	CONTACTS_FREE(uri);
	CONTACTS_FREE(ids);
	return;
}

void ctsvc_ipc_server_db_replace_records(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_list_h list = NULL;
	int count = 0;
	int *ids = NULL;
	int i=0;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_list(indata, &list);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_list fail");
			goto ERROR_RETURN;
		}

		ret = ctsvc_ipc_unmarshal_int(indata, &count);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}

		if (count <=0) {
			ret = CONTACTS_ERROR_INVALID_PARAMETER;
			goto ERROR_RETURN;
		}

		ids = (int*)malloc(sizeof(int)*count);
		for(i=0;i<count;i++) {
			ret = ctsvc_ipc_unmarshal_int(indata, &ids[i]);
			if (ret != CONTACTS_ERROR_NONE) {
				CTS_ERR("ctsvc_ipc_unmarshal_int fail");
				goto ERROR_RETURN;
			}
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_server_db_repalce_records fail");
		goto ERROR_RETURN;
	}

	if (list) {
		contacts_record_h record = NULL;
		contacts_list_first(list);
		do {
			ret = contacts_list_get_current_record_p(list, &record);
			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("contacts_list_get_current_record_p is faild(%d)", ret);
				goto ERROR_RETURN;
			}

			if (!ctsvc_have_permission(ipc, ctsvc_required_write_permission(((ctsvc_record_s *)record)->view_uri))) {
				ret = CONTACTS_ERROR_PERMISSION_DENIED;
				goto ERROR_RETURN;
			}
		} while (CONTACTS_ERROR_NONE == contacts_list_next(list));
		contacts_list_first(list);
	}

	ret = ctsvc_db_replace_records(list, ids, count);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (!*outdata) {
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata, (void*)&ret, sizeof(int)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int fail");
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (!*outdata) {
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	if (list) {
		contacts_list_destroy(list,true);
	}
	CONTACTS_FREE(ids);
	return;
}

void ctsvc_ipc_server_db_get_changes_by_version(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char* view_uri = NULL;
	int address_book_id = 0;
	int contacts_db_version = 0;
	contacts_list_h record_list = NULL;
	int current_contacts_db_version = 0;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_string(indata,&view_uri);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_string fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &address_book_id);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&contacts_db_version);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_READ)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_get_changes_by_version(view_uri, address_book_id,contacts_db_version,&record_list,&current_contacts_db_version);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if ( CONTACTS_ERROR_NO_DATA == ret )
		{
			CTS_DBG("no data");
		}
		else if( CONTACTS_ERROR_NONE == ret )
		{
			ret = ctsvc_ipc_marshal_list(record_list,*outdata);
			if (ret != CONTACTS_ERROR_NONE)
			{
				CTS_ERR("ctsvc_ipc_marshal_list fail");
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
			ret = ctsvc_ipc_marshal_int(current_contacts_db_version,*outdata);
			if (ret != CONTACTS_ERROR_NONE)
			{
				CTS_ERR("ctsvc_ipc_marshal_int fail");
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				ret = CONTACTS_ERROR_OUT_OF_MEMORY;
				goto ERROR_RETURN;
			}
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	if (record_list)
	{
		contacts_list_destroy(record_list,true);
	}
	CONTACTS_FREE(view_uri);
	return;
}

void ctsvc_ipc_server_db_get_current_version(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int contacts_db_version = 0;

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_READ) &&
			!ctsvc_have_permission(ipc, CTSVC_PERMISSION_PHONELOG_READ)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_get_current_version(&contacts_db_version);

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			return;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			return;
		}

		if ( CONTACTS_ERROR_NO_DATA == ret )
		{
			CTS_DBG("no data");
		}
		else if( CONTACTS_ERROR_NONE == ret )
		{
			ret = ctsvc_ipc_marshal_int(contacts_db_version,*outdata);
			if (ret != CONTACTS_ERROR_NONE)
			{
				CTS_ERR("ctsvc_ipc_marshal_int fail");
				return;
			}
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
	return;
}

void ctsvc_ipc_server_db_search_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char* view_uri = NULL;
	char* keyword = NULL;
	int offset = 0;
	int limit = 0;
	contacts_list_h list = NULL;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_string(indata,&view_uri);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_string(indata,&keyword);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&offset);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&limit);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(view_uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_search_records(view_uri, keyword, offset,limit,&list);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if ( CONTACTS_ERROR_NO_DATA == ret )
		{
			CTS_DBG("no data");
		}
		else if( CONTACTS_ERROR_NONE == ret )
		{
			ret = ctsvc_ipc_marshal_list(list,*outdata);

			if (ret != CONTACTS_ERROR_NONE)
			{
				CTS_ERR("ctsvc_ipc_unmarshal_int fail");
				goto DATA_FREE;
			}
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:

	if (list)
	{
		contacts_list_destroy(list,true);
	}
	CONTACTS_FREE(view_uri);
	CONTACTS_FREE(keyword);
	return;
}

void ctsvc_ipc_server_db_search_records_with_range(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	char* view_uri = NULL;
	char* keyword = NULL;
	int offset = 0;
	int limit = 0;
	int range = 0;
	contacts_list_h list = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_string(indata,&view_uri);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_string(indata,&keyword);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&offset);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&limit);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&range);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(view_uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = contacts_db_search_records_with_range(view_uri, keyword, offset,limit,range, &list);

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (!*outdata) {
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if (CONTACTS_ERROR_NO_DATA == ret) {
			CTS_DBG("no data");
		}
		else if(CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_marshal_list(list,*outdata);

			if (ret != CONTACTS_ERROR_NONE) {
				CTS_ERR("ctsvc_ipc_unmarshal_int fail");
				goto DATA_FREE;
			}
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (!*outdata) {
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}

DATA_FREE:

	if (list)
		contacts_list_destroy(list,true);
	free(view_uri);
	free(keyword);
	return;
}

void ctsvc_ipc_server_db_search_records_with_query(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_query_h query = NULL;
	char* keyword = NULL;
	int offset = 0;
	int limit = 0;
	contacts_list_h list = NULL;

	if (indata)
	{
		ret = ctsvc_ipc_unmarshal_query(indata,&query);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_string(indata,&keyword);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_record fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&offset);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata,&limit);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_ipc_unmarshal_int fail");
			goto ERROR_RETURN;
		}
	}
	else
	{
		CTS_ERR("ctsvc_ipc_server_db_insert_record fail");
		goto ERROR_RETURN;
	}


	if (!ctsvc_have_permission(ipc, ctsvc_required_read_permission(((ctsvc_query_s *)query)->view_uri))) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}


	ret = contacts_db_search_records_with_query(query, keyword, offset,limit,&list);

	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}

		if ( CONTACTS_ERROR_NO_DATA == ret )
		{
			CTS_DBG("no data");
		}
		else if( CONTACTS_ERROR_NONE == ret )
		{
			ret = ctsvc_ipc_marshal_list(list,*outdata);

			if (ret != CONTACTS_ERROR_NONE)
			{
				CTS_ERR("ctsvc_ipc_unmarshal_int fail");
				goto DATA_FREE;
			}
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
	goto DATA_FREE;

ERROR_RETURN:
	if (outdata)
	{
		*outdata = pims_ipc_data_create(0);
		if (!*outdata)
		{
			CTS_ERR("pims_ipc_data_create fail");
			goto DATA_FREE;
		}
		if (pims_ipc_data_put(*outdata,(void*)&ret,sizeof(int)) != 0)
		{
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("pims_ipc_data_put fail");
			goto DATA_FREE;
		}
	}
	else
	{
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:

	if (list)
	{
		contacts_list_destroy(list,true);
	}
	if (query)
	{
		contacts_query_destroy(query);
	}
	CONTACTS_FREE(keyword);

	return;
}

void ctsvc_ipc_server_db_get_status(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_db_status_e status;

	RETM_IF(outdata == NULL, "outdata is NULL");

	*outdata = pims_ipc_data_create(0);
	if (!*outdata) {
		CTS_ERR("pims_ipc_data_create fail");
		return;
	}

	ret = contacts_db_get_status(&status);
	if (pims_ipc_data_put(*outdata, (void*)&ret, sizeof(int)) != 0) {
		pims_ipc_data_destroy(*outdata);
		*outdata = NULL;
		CTS_ERR("pims_ipc_data_put fail (return value)");
		return;
	}

	if (pims_ipc_data_put(*outdata, (void*)&status, sizeof(int)) != 0) {
		pims_ipc_data_destroy(*outdata);
		*outdata = NULL;
		CTS_ERR("pims_ipc_data_put fail (id)");
		return;
	}

	return;
}


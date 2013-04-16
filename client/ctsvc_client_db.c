/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Dohyung Jin <dh.jin@samsung.com>
 *                 Jongwon Lee <gogosing.lee@samsung.com>
 *                 Donghee Ye <donghee.ye@samsung.com>
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
#include "ctsvc_list.h"
#include "ctsvc_record.h"
#include "ctsvc_query.h"
#include "ctsvc_inotify.h"

#include "ctsvc_internal.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_view.h"

#include "ctsvc_client_ipc.h"
#include <pims-ipc-data.h>

#include "ctsvc_inotify.h"

typedef struct {
	void *callback;
	void *user_data;
}ctsvc_ipc_async_userdata_s;

static void __ctsvc_ipc_client_insert_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata);
static void __ctsvc_ipc_client_update_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata);
static void __ctsvc_ipc_client_delete_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata);

void __ctsvc_ipc_client_insert_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata)
{
	ctsvc_ipc_async_userdata_s *sync_data = (ctsvc_ipc_async_userdata_s *)userdata;
	int ret = CONTACTS_ERROR_NONE;
	int *ids = NULL;
	unsigned int count = 0;

	if (data_out)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(data_out,&size);

		if (ret == CONTACTS_ERROR_NONE) {
			int i=0;
			unsigned int size = 0;
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(data_out, &size);
			ctsvc_client_ipc_set_change_version(transaction_ver);

			count = *(unsigned int*) pims_ipc_data_get(data_out,&size);
			ids = calloc(count, sizeof(int));
			for(i=0;i<count;i++)
			{
				ids[i] = *(int*) pims_ipc_data_get(data_out,&size);
			}
		}
	}

	if (sync_data->callback)
	{
		contacts_db_insert_result_cb callback = sync_data->callback;
		callback(ret, ids, count, sync_data->user_data);
	}
	free(ids);

	ctsvc_inotify_call_blocked_callback();

	CONTACTS_FREE(sync_data);

	return ;
}

void __ctsvc_ipc_client_update_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata)
{
	ctsvc_ipc_async_userdata_s *sync_data = (ctsvc_ipc_async_userdata_s *)userdata;
	int ret = CONTACTS_ERROR_NONE;

	if (data_out)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(data_out,&size);

		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(data_out, &size);
			ctsvc_client_ipc_set_change_version(transaction_ver);
		}
	}

	if (sync_data->callback)
	{
		contacts_db_result_cb callback = sync_data->callback;
		callback(ret, sync_data->user_data);
	}

	ctsvc_inotify_call_blocked_callback();

	CONTACTS_FREE(sync_data);

	return ;
}
void __ctsvc_ipc_client_delete_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata)
{
	ctsvc_ipc_async_userdata_s *sync_data = (ctsvc_ipc_async_userdata_s *)userdata;
	int ret = CONTACTS_ERROR_NONE;

	if (data_out)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(data_out,&size);
		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(data_out, &size);
			ctsvc_client_ipc_set_change_version(transaction_ver);
		}
	}

	if (sync_data->callback)
	{
		contacts_db_result_cb callback = sync_data->callback;
		callback(ret, sync_data->user_data);
	}

	ctsvc_inotify_call_blocked_callback();

	CONTACTS_FREE(sync_data);

	return ;
}

API int contacts_db_insert_record( contacts_record_h record, int *id )
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	if (id)
		*id = 0;

	RETVM_IF(record==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"record is NULL");

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_record(record,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_INSERT_RECORD, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			ctsvc_client_ipc_set_change_version(transaction_ver);

			if (id)
				*id = *(int*)pims_ipc_data_get(outdata,&size);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int	contacts_db_get_record( const char* view_uri, int id, contacts_record_h* out_record )
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(view_uri==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"view_uri is NULL");
	RETVM_IF(id<0,CONTACTS_ERROR_INVALID_PARAMETER,"id<0");
	RETVM_IF(out_record==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"record is NULL");
	*out_record = NULL;

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_string(view_uri,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(id,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_RECORD, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE)
		{
			ret = ctsvc_ipc_unmarshal_record(outdata,out_record);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_update_record( contacts_record_h record )
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(record==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"record is NULL");


	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_record(record,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_UPDATE_RECORD, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
		if (CONTACTS_ERROR_NONE == ret) {
			CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s *)record);
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			ctsvc_client_ipc_set_change_version(transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_delete_record( const char* view_uri, int id )
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(view_uri==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"view_uri is NULL");
	RETVM_IF(id<=0,CONTACTS_ERROR_INVALID_PARAMETER,"id <= 0");


	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_string(view_uri,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(id,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_DELETE_RECORD, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			ctsvc_client_ipc_set_change_version(transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_replace_record( contacts_record_h record, int id )
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : record is NULL");

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_record(record, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE,
				CTSVC_IPC_SERVER_DB_REPLACE_RECORD, indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			ctsvc_client_ipc_set_change_version(transaction_ver);
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_get_all_records( const char* view_uri, int offset, int limit, contacts_list_h* out_list )
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(out_list==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"list is NULL");
	RETVM_IF(view_uri==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"view_uri is NULL");
	*out_list = NULL;

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_string(view_uri,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(offset,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(limit,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_ALL_RECORDS, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE)
		{
			ret = ctsvc_ipc_unmarshal_list(outdata,out_list);
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* out_list )
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(query==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"query is NULL");
	RETVM_IF(out_list==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"list is NULL");
	*out_list = NULL;

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_query(query,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(offset,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(limit,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_RECORDS_WITH_QUERY, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE)
		{
			ret = ctsvc_ipc_unmarshal_list(outdata,out_list);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}


API int contacts_db_get_count( const char* view_uri, int *out_count )
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(view_uri==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"view_uri is NULL");
	RETVM_IF(out_count==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"count pointer is NULL");
	*out_count = 0;

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_string(view_uri,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_COUNT, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE)
		{
			ret = ctsvc_ipc_unmarshal_int(outdata,out_count);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_get_count_with_query( contacts_query_h query, int *out_count )
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(query==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"record is NULL");
	RETVM_IF(out_count==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"count pointer is NULL");
	*out_count = 0;

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_query(query,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_COUNT_WITH_QUERY, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE)
		{
			ret = ctsvc_ipc_unmarshal_int(outdata,out_count);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_insert_records_async(const contacts_list_h list, contacts_db_insert_result_cb callback, void *user_data)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	ctsvc_ipc_async_userdata_s *async_data = NULL;

	RETVM_IF(list==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"list is NULL");

	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_list(list,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	async_data = (ctsvc_ipc_async_userdata_s*)malloc(sizeof(ctsvc_ipc_async_userdata_s));
	if (async_data == NULL)
	{
		CTS_ERR("malloc fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		pims_ipc_data_destroy(indata);
		return ret;
	}
	async_data->callback = callback;
	async_data->user_data = user_data;

	if (ctsvc_ipc_call_async(CTSVC_IPC_DB_MODULE,CTSVC_IPC_SERVER_DB_INSERT_RECORDS,
				indata,__ctsvc_ipc_client_insert_records_cb,async_data) != 0)
	{
		CONTACTS_FREE(async_data);
		CTS_ERR("ctsvc_ipc_call_async failed");
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	return ret;
}

API int contacts_db_update_records_async(const contacts_list_h list, contacts_db_result_cb callback, void *user_data)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	ctsvc_ipc_async_userdata_s *async_data = NULL;

	RETVM_IF(list==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"record is NULL");

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_list(list,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	async_data = (ctsvc_ipc_async_userdata_s*)malloc(sizeof(ctsvc_ipc_async_userdata_s));
	if (async_data == NULL)
	{
		CTS_ERR("malloc fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		pims_ipc_data_destroy(indata);
		return ret;
	}
	async_data->callback = callback;
	async_data->user_data = user_data;

	if (ctsvc_ipc_call_async(CTSVC_IPC_DB_MODULE,CTSVC_IPC_SERVER_DB_UPDATE_RECORDS,
				indata,__ctsvc_ipc_client_update_records_cb,async_data) != 0)
	{
		CONTACTS_FREE(async_data);
		CTS_ERR("ctsvc_ipc_call_async failed");
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	return ret;
}

API int contacts_db_delete_records_async(const char* view_uri, int ids[], int count, contacts_db_result_cb callback, void *user_data)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	int i = 0;
	ctsvc_ipc_async_userdata_s *async_data = NULL;

	RETVM_IF(view_uri==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"view_uri is NULL");
	RETVM_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(0 == count, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_string(view_uri,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(count,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	for (i=0;i<count;i++)
	{
		ret = ctsvc_ipc_marshal_int(ids[i],indata);
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("marshal fail");
			return ret;
		}
	}

	async_data = (ctsvc_ipc_async_userdata_s*)malloc(sizeof(ctsvc_ipc_async_userdata_s));
	if (async_data == NULL)
	{
		CTS_ERR("malloc fail!");
		pims_ipc_data_destroy(indata);
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	async_data->callback = callback;
	async_data->user_data = user_data;
	if (ctsvc_ipc_call_async(CTSVC_IPC_DB_MODULE,CTSVC_IPC_SERVER_DB_DELETE_RECORDS,
				indata,__ctsvc_ipc_client_delete_records_cb,async_data) != 0)
	{
		CONTACTS_FREE(async_data);
		CTS_ERR("ctsvc_ipc_call_async failed");
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	return ret;
}

void __ctsvc_ipc_client_replace_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata)
{
	ctsvc_ipc_async_userdata_s *async_data = (ctsvc_ipc_async_userdata_s *)userdata;
	int ret = CONTACTS_ERROR_NONE;

	if (data_out) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(data_out,&size);

		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(data_out, &size);
			ctsvc_client_ipc_set_change_version(transaction_ver);
		}
	}

	if (async_data->callback) {
		contacts_db_result_cb callback = async_data->callback;
		callback(ret, async_data->user_data);
	}

	ctsvc_inotify_call_blocked_callback();

	free(async_data);

	return ;
}

API int contacts_db_replace_records_async( contacts_list_h list, int ids[], unsigned int count,
		contacts_db_result_cb callback, void *user_data )
{
	int i;
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	ctsvc_ipc_async_userdata_s *async_data = NULL;

	RETVM_IF(NULL == list,CONTACTS_ERROR_INVALID_PARAMETER, "list is NULL");
	RETVM_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(0 == count, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_list(list, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		return ret;
	}

	ret = ctsvc_ipc_marshal_unsigned_int(count, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		return ret;
	}

	for (i=0;i<count;i++) {
		ret = ctsvc_ipc_marshal_int(ids[i],indata);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("marshal fail");
			return ret;
		}
	}

	async_data = (ctsvc_ipc_async_userdata_s*)malloc(sizeof(ctsvc_ipc_async_userdata_s));
	async_data->callback = callback;
	async_data->user_data = user_data;

	if (ctsvc_ipc_call_async(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_REPLACE_RECORDS,
				indata, __ctsvc_ipc_client_replace_records_cb, async_data) != 0) {
		CONTACTS_FREE(async_data);
		CTS_ERR("ctsvc_ipc_call_async failed");
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	return ret;
}

API int contacts_db_insert_records( contacts_list_h list, int **ids, unsigned int *count)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	if (ids)
		*ids = NULL;
	if (count)
		*count = 0;

	RETVM_IF(list==NULL,CONTACTS_ERROR_INVALID_PARAMETER, "list is NULL");
	RETVM_IF(ctsvc_get_ipc_handle()==NULL,CONTACTS_ERROR_IPC, "contacts not connected");

	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_list(list,indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_INSERT_RECORDS,
				indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call_async failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			ctsvc_client_ipc_set_change_version(transaction_ver);

			if (ids && count) {
				int i = 0;
				int *id = NULL;
				unsigned int c;
				c = *(unsigned int*)pims_ipc_data_get(outdata, &size);
				id = calloc(c, sizeof(int));
				for(i=0;i<c;i++)
					id[i] = *(int*) pims_ipc_data_get(outdata, &size);
				*ids = id;
				*count = c;
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_update_records( contacts_list_h list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER, "record is NULL");
	RETVM_IF(ctsvc_get_ipc_handle() == NULL, CONTACTS_ERROR_IPC, "contacts not connected");

	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_list(list,indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_UPDATE_RECORDS,
				indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			ctsvc_client_ipc_set_change_version(transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_delete_records(const char* view_uri, int ids[], int count)
{
	int i;
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(view_uri == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "view_uri is NULL");
	RETVM_IF(ctsvc_get_ipc_handle() == NULL, CONTACTS_ERROR_IPC, "contacts not connected");

	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_string(view_uri,indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	ret = ctsvc_ipc_marshal_int(count,indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	for (i=0;i<count;i++) {
		ret = ctsvc_ipc_marshal_int(ids[i],indata);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("marshal fail");
			pims_ipc_data_destroy(indata);
			return ret;
		}
	}

	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_DELETE_RECORDS,
				indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			ctsvc_client_ipc_set_change_version(transaction_ver);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_replace_records( contacts_list_h list, int ids[], unsigned int count )
{
	int i;
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(NULL == list,CONTACTS_ERROR_INVALID_PARAMETER, "list is NULL");
	RETVM_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(0 == count, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_list(list, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		return ret;
	}

	ret = ctsvc_ipc_marshal_unsigned_int(count, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		return ret;
	}

	for (i=0;i<count;i++) {
		ret = ctsvc_ipc_marshal_int(ids[i], indata);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("marshal fail");
			return ret;
		}
	}

	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_REPLACE_RECORDS,
				indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
		if (CONTACTS_ERROR_NONE == ret) {
			int transaction_ver = 0;
			transaction_ver = *(int*)pims_ipc_data_get(outdata,&size);
			ctsvc_client_ipc_set_change_version(transaction_ver);
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_get_changes_by_version(const char* view_uri, int addressbook_id, int contacts_db_version, contacts_list_h* record_list, int* current_contacts_db_version )
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(view_uri==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"view_uri is NULL");
	RETVM_IF(record_list==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"record_list is NULL");
	RETVM_IF(current_contacts_db_version==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"current_contacts_db_version is NULL");
	*record_list = NULL;

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_string(view_uri,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(addressbook_id,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(contacts_db_version,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_CHANGES_BY_VERSION, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE)
		{
			ret = ctsvc_ipc_unmarshal_list(outdata,record_list);

			if (ret == CONTACTS_ERROR_NONE)
			{
				ret = ctsvc_ipc_unmarshal_int(outdata,current_contacts_db_version);
			}
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_get_current_version(int* contacts_db_version)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(contacts_db_version==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"contacts_db_version is null");
	*contacts_db_version = 0;

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_CURRENT_VERSION, NULL, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
		if (ret == CONTACTS_ERROR_NONE)
		{
			ret = ctsvc_ipc_unmarshal_int(outdata,contacts_db_version);
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_search_records(const char* view_uri, const char *keyword,
		int offset, int limit, contacts_list_h* out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(out_list == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "list is NULL");
	RETVM_IF(ctsvc_get_ipc_handle() == NULL, CONTACTS_ERROR_IPC, "contacts not connected");
	*out_list = NULL;

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail !");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_string(view_uri,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_string(keyword,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(offset,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(limit,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_SEARCH_RECORDS, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE)
		{
			ret = ctsvc_ipc_unmarshal_list(outdata,out_list);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_search_records_with_query(contacts_query_h query, const char *keyword,
		int offset, int limit, contacts_list_h* out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(query==NULL, CONTACTS_ERROR_INVALID_PARAMETER, "query is NULL");
	RETVM_IF(out_list==NULL, CONTACTS_ERROR_INVALID_PARAMETER, "list is NULL");
	RETVM_IF(ctsvc_get_ipc_handle() == NULL, CONTACTS_ERROR_IPC, "contacts not connected");
	*out_list = NULL;

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail !");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	ret = ctsvc_ipc_marshal_query(query,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_string(keyword,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(offset,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(limit,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_SEARCH_RECORDS_WITH_QUERY, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	if (outdata)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE)
		{
			ret = ctsvc_ipc_unmarshal_list(outdata,out_list);
		}

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_get_last_change_version(int* last_version)
{
	int ret = CONTACTS_ERROR_NONE;

	RETVM_IF(NULL == last_version, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	*last_version = ctsvc_client_ipc_get_change_version();
	return ret;
}


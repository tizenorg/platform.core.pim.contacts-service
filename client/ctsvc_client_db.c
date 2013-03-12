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
	contacts_list_h list;
}ctsvc_ipc_async_userdata_s;

static void __ctsvc_ipc_client_insert_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata);
static void __ctsvc_ipc_client_update_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata);
static void __ctsvc_ipc_client_delete_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata);

void __ctsvc_ipc_client_insert_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata)
{
	ctsvc_ipc_async_userdata_s *sync_data = (ctsvc_ipc_async_userdata_s *)userdata;
	int ret = CONTACTS_ERROR_NONE;
	contacts_list_h list = sync_data->list;
	int *ids = NULL;
	unsigned int count = 0;

	if (data_out)
	{
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(data_out,&size);

		if (ret == CONTACTS_ERROR_NONE && list != NULL)
		{
			int i=0;
			unsigned int size = 0;

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
		CTS_ERR("pims_ipc_call failed");
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
		CTS_ERR("pims_ipc_call failed");
		pims_ipc_data_destroy(indata);
		*out_record = NULL;
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
		CTS_ERR("pims_ipc_call failed");
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
		CTS_ERR("pims_ipc_call failed");
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

		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_replace_record( contacts_record_h record, int id )
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid paramter : record is NULL");

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
		CTS_ERR("pims_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		// check outdata
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
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
		CTS_ERR("pims_ipc_call failed");
		pims_ipc_data_destroy(indata);
		*out_list = NULL;
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
		CTS_ERR("pims_ipc_call failed");
		pims_ipc_data_destroy(indata);
		*out_list = NULL;
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
		CTS_ERR("pims_ipc_call failed");
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
		CTS_ERR("pims_ipc_call failed");
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
	pims_ipc_data_h outdata = NULL;
	contacts_list_h clone_list = NULL;
	ctsvc_ipc_async_userdata_s *async_data = NULL;

	RETVM_IF(list==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"list is NULL");


	ret = ctsvc_list_clone(list, &clone_list);
	RETV_IF(CONTACTS_ERROR_NONE != ret, ret);

	// make indata
	indata = pims_ipc_data_create(0);
	if (indata == NULL)
	{
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		contacts_list_destroy(clone_list, true);
		return ret;
	}
	ret = ctsvc_ipc_marshal_list(clone_list,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		contacts_list_destroy(clone_list, true);
		return ret;
	}

	if (callback == NULL)
	{
		if (ctsvc_ipc_call( CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_INSERT_RECORDS,
					indata,&outdata) != 0)
		{
			CTS_ERR("pims_ipc_call_async failed");
			contacts_list_destroy(clone_list, true);
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
				goto SET_DATA;
			}

			pims_ipc_data_destroy(outdata);
		}

		contacts_list_destroy(clone_list, true);
		return ret;
	}

	async_data = (ctsvc_ipc_async_userdata_s*)malloc(sizeof(ctsvc_ipc_async_userdata_s));
	if (async_data == NULL)
	{
		CTS_ERR("malloc fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		contacts_list_destroy(clone_list, true);
		return ret;
	}
	async_data->callback = callback;
	async_data->user_data = user_data;
	async_data->list = clone_list;
	if (ctsvc_ipc_call_async(CTSVC_IPC_DB_MODULE,CTSVC_IPC_SERVER_DB_INSERT_RECORDS,
				indata,__ctsvc_ipc_client_insert_records_cb,async_data) != 0)
	{
		CONTACTS_FREE(async_data);
		CTS_ERR("pims_ipc_call_async failed");
		contacts_list_destroy(clone_list, true);
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}
	contacts_list_destroy(clone_list, true);

	return ret;

SET_DATA:
	if (outdata)
	{
/*
		int count = 0;
		int id = 0;
		unsigned int property_id = 0;
		int i=0;
		unsigned int size = 0;

		contacts_list_first(list);
		count = *(int*) pims_ipc_data_get(outdata,&size);
		property_id = *(unsigned int*) pims_ipc_data_get(outdata,&size);
		for(i=0;i<count;i++)
		{
			contacts_record_h record = NULL;
			if (contacts_list_get_current_record_p(list,&record) != CONTACTS_ERROR_NONE)
			{
				CTS_ERR("contacts_list_get_current_record_p fail");
				pims_ipc_data_destroy(outdata);
				return ret;
			}
			id = *(int*) pims_ipc_data_get(outdata,&size);
			ctsvc_record_set_int(record,property_id,id);
		}
*/
		pims_ipc_data_destroy(outdata);
	}
	contacts_list_destroy(clone_list, true);

	return ret;
}

API int contacts_db_update_records_async(const contacts_list_h list, contacts_db_result_cb callback, void *user_data)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;
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

	if (callback == NULL)
	{
		// ipc call
		if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_UPDATE_RECORDS,
				indata, &outdata) != 0)
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
			// check outdata
			unsigned int size = 0;
			ret = *(int*) pims_ipc_data_get(outdata,&size);

			pims_ipc_data_destroy(outdata);
		}

		return ret;
	}

	async_data = (ctsvc_ipc_async_userdata_s*)malloc(sizeof(ctsvc_ipc_async_userdata_s));
	if (async_data == NULL)
	{
		CTS_ERR("malloc fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}
	async_data->callback = callback;
	async_data->user_data = user_data;
	async_data->list = list;
	if (ctsvc_ipc_call_async(CTSVC_IPC_DB_MODULE,CTSVC_IPC_SERVER_DB_UPDATE_RECORDS,
				indata,__ctsvc_ipc_client_update_records_cb,async_data) != 0)
	{
		CONTACTS_FREE(async_data);
		CTS_ERR("pims_ipc_call_async failed");
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
	pims_ipc_data_h outdata = NULL;
	int i = 0;
	ctsvc_ipc_async_userdata_s *async_data = NULL;

	RETVM_IF(view_uri==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"view_uri is NULL");
	RETVM_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid paramter");
	RETVM_IF(0 == count, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid paramter");

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

	if (callback == NULL)
	{
		// ipc call
		if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_DELETE_RECORDS,
				indata, &outdata) != 0)
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
			// check outdata
			unsigned int size = 0;
			ret = *(int*) pims_ipc_data_get(outdata,&size);

			pims_ipc_data_destroy(outdata);
		}

		return ret;
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
		CTS_ERR("pims_ipc_call_async failed");
		return CONTACTS_ERROR_IPC;
	}

	if (indata)
	{
		pims_ipc_data_destroy(indata);
	}

	return ret;
}

void __ctsvc_ipc_client_replace_records_cb(pims_ipc_h ipc, pims_ipc_data_h data_out, void *userdata)
{
	ctsvc_ipc_async_userdata_s *async_data = (ctsvc_ipc_async_userdata_s *)userdata;
	int ret = CONTACTS_ERROR_NONE;

	if (data_out) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(data_out,&size);
		//pims_ipc_data_destroy(data_out);
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
	pims_ipc_data_h outdata = NULL;
	ctsvc_ipc_async_userdata_s *async_data = NULL;

	RETVM_IF(NULL == list,CONTACTS_ERROR_INVALID_PARAMETER, "list is NULL");
	RETVM_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid paramter");
	RETVM_IF(0 == count, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid paramter");

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

	if (callback == NULL) {
		if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_REPLACE_RECORDS,
					indata, &outdata) != 0) {
			CTS_ERR("pims_ipc_call failed");
			return CONTACTS_ERROR_IPC;
		}

		pims_ipc_data_destroy(indata);

		if (outdata) {
			unsigned int size = 0;
			ret = *(int*) pims_ipc_data_get(outdata,&size);
			pims_ipc_data_destroy(outdata);
		}

		return ret;
	}

	async_data = (ctsvc_ipc_async_userdata_s*)malloc(sizeof(ctsvc_ipc_async_userdata_s));
	async_data->callback = callback;
	async_data->user_data = user_data;
	async_data->list = list;
	if (ctsvc_ipc_call_async(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_REPLACE_RECORDS,
				indata, __ctsvc_ipc_client_replace_records_cb, async_data) != 0) {
		CONTACTS_FREE(async_data);
		CTS_ERR("pims_ipc_call_async failed");
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
		CTS_ERR("pims_ipc_call_async failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE) {
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
		CTS_ERR("pims_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

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
		CTS_ERR("pims_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

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
	RETVM_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid paramter");
	RETVM_IF(0 == count, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid paramter");

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
		CTS_ERR("pims_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);
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
		CTS_ERR("pims_ipc_call failed");
		pims_ipc_data_destroy(indata);
		*record_list = NULL;
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

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_CURRENT_VERSION, NULL, &outdata) != 0)
	{
		CTS_ERR("pims_ipc_call failed");
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
		CTS_ERR("pims_ipc_call failed");
		pims_ipc_data_destroy(indata);
		*out_list = NULL;
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
		CTS_ERR("pims_ipc_call failed");
		pims_ipc_data_destroy(indata);
		*out_list = NULL;
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


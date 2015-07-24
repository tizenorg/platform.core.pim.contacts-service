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
#include <pims-ipc.h>
#include <pims-ipc-data.h>

#include "contacts.h"
#include "contacts_internal.h"

#include "ctsvc_internal.h"
#include "ctsvc_list.h"
#include "ctsvc_record.h"
#include "ctsvc_query.h"
#include "ctsvc_inotify.h"

#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_view.h"

#include "ctsvc_client_ipc.h"

#include "ctsvc_mutex.h"


API int contacts_db_insert_record(contacts_record_h record, int *id)
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
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_INSERT_RECORD, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

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

API int contacts_db_get_record(const char* view_uri, int id, contacts_record_h* out_record)
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
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(id,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_RECORD, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

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

API int contacts_db_update_record(contacts_record_h record)
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
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_UPDATE_RECORD, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

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

API int contacts_db_delete_record(const char* view_uri, int id)
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
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(id,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_DELETE_RECORD, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

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

API int contacts_db_replace_record(contacts_record_h record, int id)
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
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(id, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE,
				CTSVC_IPC_SERVER_DB_REPLACE_RECORD, indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call Fail");
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

API int contacts_db_get_all_records(const char* view_uri, int offset, int limit, contacts_list_h* out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(out_list==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"list is NULL");
	*out_list = NULL;
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
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(offset,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(limit,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_ALL_RECORDS, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

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

API int contacts_db_get_records_with_query(contacts_query_h query, int offset, int limit, contacts_list_h* out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(out_list==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"list is NULL");
	*out_list = NULL;
	RETVM_IF(query==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"query is NULL");

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
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(offset,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
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
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

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

API int contacts_db_get_count(const char* view_uri, int *out_count)
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
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_COUNT, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

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

API int contacts_db_get_count_with_query(contacts_query_h query, int *out_count)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(query==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"record is NULL");
	RETVM_IF(out_count==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"count pointer is NULL");
	*out_count = 0;

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
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_COUNT_WITH_QUERY, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

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

API int contacts_db_insert_records(contacts_list_h list, int **ids, int *count)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	if (ids)
		*ids = NULL;
	if (count)
		*count = 0;

	RETVM_IF(list==NULL,CONTACTS_ERROR_INVALID_PARAMETER, "list is NULL");

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
		CTS_ERR("ctsvc_ipc_call Fail");
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
				int c;
				c = *(int*)pims_ipc_data_get(outdata, &size);
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

API int contacts_db_update_records(contacts_list_h list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER, "list is NULL");

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
		CTS_ERR("ctsvc_ipc_call Fail");
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
		CTS_ERR("ctsvc_ipc_call Fail");
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

API int contacts_db_replace_records(contacts_list_h list, int ids[], int count)
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
		pims_ipc_data_destroy(indata);
		return ret;
	}

	ret = ctsvc_ipc_marshal_int(count, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	for (i=0;i<count;i++) {
		ret = ctsvc_ipc_marshal_int(ids[i], indata);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("marshal fail");
			pims_ipc_data_destroy(indata);
			return ret;
		}
	}

	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_REPLACE_RECORDS,
				indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call Fail");
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

API int contacts_db_get_changes_by_version(const char* view_uri, int addressbook_id,
		int contacts_db_version, contacts_list_h* record_list, int* current_contacts_db_version)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(record_list==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"record_list is NULL");
	*record_list = NULL;
	RETVM_IF(view_uri==NULL,CONTACTS_ERROR_INVALID_PARAMETER,"view_uri is NULL");
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
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(addressbook_id,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(contacts_db_version,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_CHANGES_BY_VERSION, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

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
		CTS_ERR("ctsvc_ipc_call Fail");
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
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_string(keyword,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(offset,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_int(limit,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_SEARCH_RECORDS, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

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

API int contacts_db_search_records_with_range(const char* view_uri, const char *keyword,
		int offset, int limit, int range, contacts_list_h* out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(out_list == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "list is NULL");
	*out_list = NULL;
	RETVM_IF(range == 0, CONTACTS_ERROR_INVALID_PARAMETER, "range is 0");

	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail !");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	ret = ctsvc_ipc_marshal_string(view_uri, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		goto DATA_FREE;
	}
	ret = ctsvc_ipc_marshal_string(keyword, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		goto DATA_FREE;
	}
	ret = ctsvc_ipc_marshal_int(offset, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		goto DATA_FREE;
	}
	ret = ctsvc_ipc_marshal_int(limit, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		goto DATA_FREE;
	}
	ret = ctsvc_ipc_marshal_int(range, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		goto DATA_FREE;
	}

	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_SEARCH_RECORDS_WITH_RANGE, indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE)
			ret = ctsvc_ipc_unmarshal_list(outdata,out_list);
		pims_ipc_data_destroy(outdata);
	}

	return ret;

DATA_FREE:
	pims_ipc_data_destroy(indata);
	return ret;
}

API int contacts_db_search_records_with_query(contacts_query_h query, const char *keyword,
		int offset, int limit, contacts_list_h* out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(out_list==NULL, CONTACTS_ERROR_INVALID_PARAMETER, "list is NULL");
	*out_list = NULL;
	RETVM_IF(query==NULL, CONTACTS_ERROR_INVALID_PARAMETER, "query is NULL");

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
		pims_ipc_data_destroy(indata);
		return ret;
	}
	ret = ctsvc_ipc_marshal_string(keyword,indata);
	if (ret != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
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
		pims_ipc_data_destroy(indata);
		return ret;
	}

	// ipc call
	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_SEARCH_RECORDS_WITH_QUERY, indata, &outdata) != 0)
	{
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

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
	bool result = false;

	RETVM_IF(NULL == last_version, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	*last_version = 0;

	ret = ctsvc_ipc_client_check_permission(CTSVC_PERMISSION_CONTACT_READ, &result);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission fail (%d)", ret);
	if (result == false) {
		ret = ctsvc_ipc_client_check_permission(CTSVC_PERMISSION_PHONELOG_READ, &result);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission fail (%d)", ret);
		RETVM_IF(result == false, CONTACTS_ERROR_PERMISSION_DENIED, "Permission denied");
	}

	*last_version = ctsvc_client_ipc_get_change_version();
	return ret;
}

typedef struct
{
	contacts_db_status_changed_cb cb;
	void *user_data;
}status_callback_info_s;

static GSList *__status_change_subscribe_list = NULL;

static void __ctsvc_db_status_subscriber_callback(pims_ipc_h ipc, pims_ipc_data_h data, void *user_data)
{
	int status = -1;
	unsigned int size = 0;
	GSList *l;

	if (data)
		status = *(int*)pims_ipc_data_get(data, &size);

	for (l = __status_change_subscribe_list;l;l=l->next) {
		status_callback_info_s *cb_info = l->data;
		if (cb_info->cb)
			cb_info->cb(status, cb_info->user_data);
	}
}

API int contacts_db_get_status(contacts_db_status_e *status)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(status == NULL, CONTACTS_ERROR_INVALID_PARAMETER,"The out param is NULL");
	*status = 0;

	if (ctsvc_ipc_call(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_STATUS, NULL, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call Fail");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);
		if (CONTACTS_ERROR_NONE == ret) {
			*status = *(int*) pims_ipc_data_get(outdata, &size);
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_db_add_status_changed_cb(
		contacts_db_status_changed_cb cb, void* user_data)
{
	status_callback_info_s *cb_info = NULL;
	RETVM_IF(NULL == cb, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : callback is null");

	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	if (pims_ipc_subscribe(ctsvc_ipc_get_handle_for_change_subsciption(),
				CTSVC_IPC_SUBSCRIBE_MODULE, CTSVC_IPC_SERVER_DB_STATUS_CHANGED,
				__ctsvc_db_status_subscriber_callback, NULL) != 0) {
		CTS_ERR("pims_ipc_subscribe error\n");
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return CONTACTS_ERROR_IPC;
	}

	cb_info = calloc(1, sizeof(status_callback_info_s));
	cb_info->user_data = user_data;
	cb_info->cb = cb;
	__status_change_subscribe_list = g_slist_append(__status_change_subscribe_list, cb_info);

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}

API int contacts_db_remove_status_changed_cb(
		contacts_db_status_changed_cb cb, void* user_data)
{
	GSList *l;

	RETVM_IF(NULL == cb, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : callback is null");

	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	for(l = __status_change_subscribe_list;l;l=l->next) {
		status_callback_info_s *cb_info = l->data;
		if (cb == cb_info->cb && user_data == cb_info->user_data) {
			__status_change_subscribe_list = g_slist_remove(__status_change_subscribe_list, cb_info);
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


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

#include <glib.h>
#include <pims-ipc-data.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_client_ipc.h"

API int contacts_setting_get_name_display_order(contacts_name_display_order_e *name_display_order)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(name_display_order == NULL, CONTACTS_ERROR_INVALID_PARAMETER,"The out param is NULL");
	*name_display_order = 0;
	RETVM_IF(ctsvc_get_ipc_handle() == NULL,CONTACTS_ERROR_IPC,"contacts not connected");

	if (ctsvc_ipc_call(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_GET_NAME_DISPLAY_ORDER, NULL, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);
		if (CONTACTS_ERROR_NONE == ret) {
			*name_display_order = *(int*) pims_ipc_data_get(outdata, &size);
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_setting_get_name_sorting_order(contacts_name_sorting_order_e *name_sorting_order)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(name_sorting_order == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "The out param is NULL");
	*name_sorting_order = 0;
	RETVM_IF(ctsvc_get_ipc_handle() == NULL,CONTACTS_ERROR_IPC,"contacts not connected");

	if (ctsvc_ipc_call(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_GET_NAME_SORTING_ORDER, NULL, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);
		if (CONTACTS_ERROR_NONE == ret) {
			*name_sorting_order = *(int*) pims_ipc_data_get(outdata, &size);
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_setting_set_name_display_order(contacts_name_display_order_e name_display_order)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(name_display_order != CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST
			&& name_display_order != CONTACTS_NAME_DISPLAY_ORDER_LASTFIRST,
			CONTACTS_ERROR_INVALID_PARAMETER, "name display order is invalid : %d", name_display_order);
	RETVM_IF(ctsvc_get_ipc_handle() == NULL,CONTACTS_ERROR_IPC,"contacts not connected");

	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_int(name_display_order, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	if (ctsvc_ipc_call(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_SET_NAME_DISPLAY_ORDER, indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}
	pims_ipc_data_destroy(indata);

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_setting_set_name_sorting_order(contacts_name_sorting_order_e name_sorint_order)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(name_sorint_order != CONTACTS_NAME_SORTING_ORDER_FIRSTLAST
			&& name_sorint_order != CONTACTS_NAME_SORTING_ORDER_LASTFIRST,
			CONTACTS_ERROR_INVALID_PARAMETER, "name sorting order is invalid : %d", name_sorint_order);
	RETVM_IF(ctsvc_get_ipc_handle() == NULL,CONTACTS_ERROR_IPC,"contacts not connected");

	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail!");
		ret = CONTACTS_ERROR_OUT_OF_MEMORY;
		return ret;
	}

	ret = ctsvc_ipc_marshal_int(name_sorint_order, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	if (ctsvc_ipc_call(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_SET_NAME_SORTING_ORDER, indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}
	pims_ipc_data_destroy(indata);

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}


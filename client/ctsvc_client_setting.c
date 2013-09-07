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
#include "ctsvc_notify.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_client_ipc.h"
#include "ctsvc_mutex.h"

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

typedef struct
{
	contacts_setting_name_display_order_changed_cb cb;
	void *user_data;
}ctsvc_name_display_order_changed_cb_info_s;

static GSList *__setting_name_display_order_subscribe_list = NULL;

static void __ctsvc_setting_name_display_order_subscriber_callback(pims_ipc_h ipc, pims_ipc_data_h data, void *user_data)
{
	int value = -1;
	unsigned int size = 0;
	if (data)
		value = *(int*)pims_ipc_data_get(data, &size);

	if (__setting_name_display_order_subscribe_list) {
		GSList *l;
		for (l = __setting_name_display_order_subscribe_list;l;l=l->next) {
			ctsvc_name_display_order_changed_cb_info_s *cb_info = l->data;
			if (cb_info->cb)
				cb_info->cb((contacts_name_display_order_e)value, cb_info->user_data);
		}
	}
}

API int contacts_setting_add_name_display_order_changed_cb(
	contacts_setting_name_display_order_changed_cb cb, void* user_data)
{
	GSList *l;
	ctsvc_name_display_order_changed_cb_info_s *cb_info;

	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	if (!__setting_name_display_order_subscribe_list) {
		if (pims_ipc_subscribe(ctsvc_ipc_get_handle_for_change_subsciption(),
					CTSVC_IPC_SUBSCRIBE_MODULE, CTSVC_SETTING_DISPLAY_ORDER_CHANGED,
					__ctsvc_setting_name_display_order_subscriber_callback, NULL) != 0) {
			CTS_ERR("pims_ipc_subscribe error\n");
			ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
			return CONTACTS_ERROR_IPC;
		}
	}

	for (l = __setting_name_display_order_subscribe_list;l;l=l->next) {
		ctsvc_name_display_order_changed_cb_info_s *cb_info = l->data;
		if (cb_info->cb == cb && cb_info->user_data == user_data) {
			CTS_ERR("The same callback(%s) is already exist");
			return CONTACTS_ERROR_SYSTEM;
		}
	}

	cb_info = calloc(1, sizeof(ctsvc_name_display_order_changed_cb_info_s));
	cb_info->user_data = user_data;
	cb_info->cb = cb;
	__setting_name_display_order_subscribe_list = g_slist_append(__setting_name_display_order_subscribe_list, cb_info);

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}

API int contacts_setting_remove_name_display_order_changed_cb(
	contacts_setting_name_display_order_changed_cb cb, void* user_data)
{
	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	if (__setting_name_display_order_subscribe_list) {
		GSList *l;
		for(l = __setting_name_display_order_subscribe_list;l;l=l->next) {
			ctsvc_name_display_order_changed_cb_info_s *cb_info = l->data;
			if (cb == cb_info->cb && user_data == cb_info->user_data) {
				__setting_name_display_order_subscribe_list = g_slist_remove(__setting_name_display_order_subscribe_list, cb_info);
				free(cb_info);
				break;
			}
		}
		if (g_slist_length(__setting_name_display_order_subscribe_list) == 0) {
			pims_ipc_unsubscribe(ctsvc_ipc_get_handle_for_change_subsciption(),
					CTSVC_IPC_SUBSCRIBE_MODULE, CTSVC_SETTING_DISPLAY_ORDER_CHANGED);
			g_slist_free(__setting_name_display_order_subscribe_list);
			__setting_name_display_order_subscribe_list = NULL;
		}
	}

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}

typedef struct
{
	contacts_setting_name_sorting_order_changed_cb cb;
	void *user_data;
}ctsvc_name_sorting_order_changed_cb_info_s;

static GSList *__setting_name_sorting_order_subscribe_list = NULL;

static void __ctsvc_setting_name_sorting_order_subscriber_callback(pims_ipc_h ipc, pims_ipc_data_h data, void *user_data)
{
	int value = -1;
	unsigned int size = 0;
	if (data)
		value = *(int*)pims_ipc_data_get(data, &size);

	if (__setting_name_sorting_order_subscribe_list) {
		GSList *l;
		for (l = __setting_name_sorting_order_subscribe_list;l;l=l->next) {
			ctsvc_name_sorting_order_changed_cb_info_s *cb_info = l->data;
			if (cb_info->cb)
				cb_info->cb((contacts_name_sorting_order_e)value, cb_info->user_data);
		}
	}
}

API int contacts_setting_add_name_sorting_order_changed_cb(
	contacts_setting_name_sorting_order_changed_cb cb, void* user_data)
{
	GSList *l;
	ctsvc_name_sorting_order_changed_cb_info_s *cb_info;

	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	if (!__setting_name_sorting_order_subscribe_list) {
		if (pims_ipc_subscribe(ctsvc_ipc_get_handle_for_change_subsciption(),
					CTSVC_IPC_SUBSCRIBE_MODULE, CTSVC_SETTING_SORTING_ORDER_CHANGED,
					__ctsvc_setting_name_sorting_order_subscriber_callback, NULL) != 0) {
			CTS_ERR("pims_ipc_subscribe error\n");
			ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
			return CONTACTS_ERROR_IPC;
		}
	}

	for (l = __setting_name_sorting_order_subscribe_list;l;l=l->next) {
		ctsvc_name_sorting_order_changed_cb_info_s *cb_info = l->data;
		if (cb_info->cb == cb && cb_info->user_data == user_data) {
			CTS_ERR("The same callback(%s) is already exist");
			return CONTACTS_ERROR_SYSTEM;
		}
	}

	cb_info = calloc(1, sizeof(ctsvc_name_sorting_order_changed_cb_info_s));
	cb_info->user_data = user_data;
	cb_info->cb = cb;
	__setting_name_sorting_order_subscribe_list = g_slist_append(__setting_name_sorting_order_subscribe_list, cb_info);

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}

API int contacts_setting_remove_name_sorting_order_changed_cb(
	contacts_setting_name_sorting_order_changed_cb cb, void* user_data)
{
	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	if (__setting_name_sorting_order_subscribe_list) {
		GSList *l;
		for(l = __setting_name_sorting_order_subscribe_list;l;l=l->next) {
			ctsvc_name_sorting_order_changed_cb_info_s *cb_info = l->data;
			if (cb == cb_info->cb && user_data == cb_info->user_data) {
				__setting_name_sorting_order_subscribe_list = g_slist_remove(__setting_name_sorting_order_subscribe_list, cb_info);
				free(cb_info);
				break;
			}
		}
		if (g_slist_length(__setting_name_sorting_order_subscribe_list) == 0) {
			pims_ipc_unsubscribe(ctsvc_ipc_get_handle_for_change_subsciption(),
					CTSVC_IPC_SUBSCRIBE_MODULE, CTSVC_SETTING_SORTING_ORDER_CHANGED);
			g_slist_free(__setting_name_sorting_order_subscribe_list);
			__setting_name_sorting_order_subscribe_list = NULL;
		}
	}

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}


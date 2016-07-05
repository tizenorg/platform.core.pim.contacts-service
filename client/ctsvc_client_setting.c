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
#include <pims-ipc-data.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_notify.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_client_ipc.h"
#include "ctsvc_mutex.h"

typedef struct {
	contacts_setting_name_display_order_changed_cb cb;
	void *user_data;
} ctsvc_name_display_order_changed_cb_info_s;

typedef struct {
	contacts_setting_name_sorting_order_changed_cb cb;
	void *user_data;
} ctsvc_name_sorting_order_changed_cb_info_s;

static GSList *__setting_name_display_order_subscribe_list = NULL;
static GSList *__setting_name_sorting_order_subscribe_list = NULL;

API int contacts_setting_get_name_display_order(
		contacts_name_display_order_e *name_display_order)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(name_display_order == NULL, CONTACTS_ERROR_INVALID_PARAMETER);
	*name_display_order = 0;

	if (ctsvc_ipc_call(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_GET_NAME_DISPLAY_ORDER,
				NULL, &outdata) != 0) {
		ERR("ctsvc_ipc_call Fail");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
		}

		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, (int*)name_display_order)) {
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_setting_get_name_sorting_order(
		contacts_name_sorting_order_e *name_sorting_order)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(name_sorting_order == NULL, CONTACTS_ERROR_INVALID_PARAMETER);
	*name_sorting_order = 0;

	if (ctsvc_ipc_call(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_GET_NAME_SORTING_ORDER,
				NULL, &outdata) != 0) {
		//LCOV_EXCL_START
		ERR("ctsvc_ipc_call Fail");
		return CONTACTS_ERROR_IPC;
		//LCOV_EXCL_STOP
	}

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			//LCOV_EXCL_START
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			//LCOV_EXCL_STOP
		}
		if (CONTACTS_ERROR_NONE == ret) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, (int*)name_sorting_order)) {
				//LCOV_EXCL_START
				ERR("ctsvc_ipc_unmarshal_int() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
				//LCOV_EXCL_STOP
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_setting_set_name_display_order(
		contacts_name_display_order_e name_display_order)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(name_display_order != CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST
			&& name_display_order != CONTACTS_NAME_DISPLAY_ORDER_LASTFIRST,
			CONTACTS_ERROR_INVALID_PARAMETER,
			"name display order is invalid : %d", name_display_order);

	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		//LCOV_EXCL_START
		ERR("pims_ipc_data_create() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		//LCOV_EXCL_STOP
	}

	ret = ctsvc_ipc_marshal_int(name_display_order, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		//LCOV_EXCL_START
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		//LCOV_EXCL_STOP
	}

	if (ctsvc_ipc_call(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_SET_NAME_DISPLAY_ORDER,
				indata, &outdata) != 0) {
		//LCOV_EXCL_START
		ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		//LCOV_EXCL_STOP
	}
	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			//LCOV_EXCL_START
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			//LCOV_EXCL_STOP
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

API int contacts_setting_set_name_sorting_order(
		contacts_name_sorting_order_e name_sorint_order)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(name_sorint_order != CONTACTS_NAME_SORTING_ORDER_FIRSTLAST
			&& name_sorint_order != CONTACTS_NAME_SORTING_ORDER_LASTFIRST,
			CONTACTS_ERROR_INVALID_PARAMETER,
			"name sorting order is invalid : %d", name_sorint_order);

	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		//LCOV_EXCL_START
		ERR("pims_ipc_data_create() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		//LCOV_EXCL_STOP
	}

	ret = ctsvc_ipc_marshal_int(name_sorint_order, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		//LCOV_EXCL_START
		ERR("ctsvc_ipc_marshal_int() Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
		//LCOV_EXCL_STOP
	}

	if (ctsvc_ipc_call(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_SET_NAME_SORTING_ORDER,
				indata, &outdata) != 0) {
		//LCOV_EXCL_START
		ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
		//LCOV_EXCL_STOP
	}
	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			//LCOV_EXCL_START
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
			//LCOV_EXCL_STOP
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

static void _csetting_name_display_order_subscribe_cb(pims_ipc_h ipc,
		pims_ipc_data_h data, void *user_data)
{
	int ret;
	int value = -1;

	if (data) {
		ret = ctsvc_ipc_unmarshal_int(data, &value);
		WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_ipc_unmarshal_int() Fail(%d)", ret);
	}
	if (__setting_name_display_order_subscribe_list) {
		GSList *l;
		for (l = __setting_name_display_order_subscribe_list; l; l = l->next) {
			ctsvc_name_display_order_changed_cb_info_s *cb_info = l->data;
			if (cb_info->cb)
				cb_info->cb((contacts_name_display_order_e)value, cb_info->user_data);
		}
	}
}

static void _csetting_name_sorting_order_subscribe_cb(pims_ipc_h ipc,
		pims_ipc_data_h data, void *user_data)
{
	int ret;
	int value = -1;

	if (data) {
		ret = ctsvc_ipc_unmarshal_int(data, &value);
		WARN_IF(CONTACTS_ERROR_NONE != ret, "() Fail(%d)", ret);
	}

	if (__setting_name_sorting_order_subscribe_list) {
		GSList *l;
		for (l = __setting_name_sorting_order_subscribe_list; l; l = l->next) {
			ctsvc_name_sorting_order_changed_cb_info_s *cb_info = l->data;
			if (cb_info->cb)
				cb_info->cb((contacts_name_sorting_order_e)value, cb_info->user_data);
		}
	}
}

//LCOV_EXCL_START
int ctsvc_setting_recover_for_change_subscription()
{
	GSList *it;

	for (it = __setting_name_display_order_subscribe_list; it; it = it->next) {
		ctsvc_name_display_order_changed_cb_info_s *cb_info = it->data;
		if (cb_info->cb) {
			if (pims_ipc_subscribe(ctsvc_ipc_get_handle_for_change_subsciption(),
						CTSVC_IPC_SUBSCRIBE_MODULE, CTSVC_SETTING_DISPLAY_ORDER_CHANGED,
						_csetting_name_display_order_subscribe_cb, NULL) != 0) {
				ERR("pims_ipc_subscribe() Fail");
				return CONTACTS_ERROR_IPC;
			}
			break;
		}
	}

	for (it = __setting_name_sorting_order_subscribe_list; it; it = it->next) {
		ctsvc_name_sorting_order_changed_cb_info_s *cb_info = it->data;
		if (cb_info->cb) {
			if (pims_ipc_subscribe(ctsvc_ipc_get_handle_for_change_subsciption(),
						CTSVC_IPC_SUBSCRIBE_MODULE, CTSVC_SETTING_SORTING_ORDER_CHANGED,
						_csetting_name_sorting_order_subscribe_cb, NULL) != 0) {
				//LCOV_EXCL_START
				ERR("pims_ipc_subscribe() Fail");
				return CONTACTS_ERROR_IPC;
				//LCOV_EXCL_STOP
			}
			break;
		}
	}

	return CONTACTS_ERROR_NONE;
}
//LCOV_EXCL_STOP

API int contacts_setting_add_name_display_order_changed_cb(
		contacts_setting_name_display_order_changed_cb cb, void *user_data)
{
	GSList *l;
	int ret;
	bool result = false;
	ctsvc_name_display_order_changed_cb_info_s *cb_info;

	RETV_IF(cb == NULL, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = ctsvc_ipc_client_check_permission(CTSVC_PERMISSION_CONTACT_READ, &result);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission() Fail(%d)", ret);
	RETVM_IF(result == false, CONTACTS_ERROR_PERMISSION_DENIED, "Permission denied (contact read)");

	ret = ctsvc_ipc_create_for_change_subscription();
	if (CONTACTS_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("ctsvc_ipc_create_for_change_subscription() Fail(%d)", ret);
		return ret;
		//LCOV_EXCL_STOP
	}

	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	if (NULL == __setting_name_display_order_subscribe_list) {
		if (pims_ipc_subscribe(ctsvc_ipc_get_handle_for_change_subsciption(),
					CTSVC_IPC_SUBSCRIBE_MODULE, CTSVC_SETTING_DISPLAY_ORDER_CHANGED,
					_csetting_name_display_order_subscribe_cb, NULL) != 0) {
			//LCOV_EXCL_START
			ERR("pims_ipc_subscribe() Fail");
			ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
			return CONTACTS_ERROR_IPC;
			//LCOV_EXCL_STOP
		}
	}

	for (l = __setting_name_display_order_subscribe_list; l; l = l->next) {
		ctsvc_name_display_order_changed_cb_info_s *cb_info = l->data;
		if (cb_info->cb == cb && cb_info->user_data == user_data) {
			ERR("The same callback(%x) is already exist", cb);
			ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}

	cb_info = calloc(1, sizeof(ctsvc_name_display_order_changed_cb_info_s));
	if (NULL == cb_info) {
		//LCOV_EXCL_START
		ERR("calloc() Fail");
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		//LCOV_EXCL_STOP
	}
	cb_info->user_data = user_data;
	cb_info->cb = cb;
	__setting_name_display_order_subscribe_list = g_slist_append(__setting_name_display_order_subscribe_list, cb_info);

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}

API int contacts_setting_remove_name_display_order_changed_cb(
		contacts_setting_name_display_order_changed_cb cb, void *user_data)
{
	int ret;

	RETV_IF(cb == NULL, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = ctsvc_ipc_destroy_for_change_subscription(false);
	if (CONTACTS_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("ctsvc_ipc_destroy_for_change_subscription() Fail(%d)", ret);
		return ret;
		//LCOV_EXCL_STOP
	}

	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	if (__setting_name_display_order_subscribe_list) {
		GSList *l;
		for (l = __setting_name_display_order_subscribe_list; l; l = l->next) {
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

API int contacts_setting_add_name_sorting_order_changed_cb(
		contacts_setting_name_sorting_order_changed_cb cb, void *user_data)
{
	GSList *l;
	int ret;
	bool result = false;
	ctsvc_name_sorting_order_changed_cb_info_s *cb_info;

	RETV_IF(cb == NULL, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = ctsvc_ipc_client_check_permission(CTSVC_PERMISSION_CONTACT_READ, &result);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission Fail(%d)", ret);
	RETVM_IF(result == false, CONTACTS_ERROR_PERMISSION_DENIED, "Permission denied (contact read)");

	ret = ctsvc_ipc_create_for_change_subscription();
	if (CONTACTS_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("ctsvc_ipc_create_for_change_subscription() Fail(%d)", ret);
		return ret;
		//LCOV_EXCL_STOP
	}

	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	if (NULL == __setting_name_sorting_order_subscribe_list) {
		if (pims_ipc_subscribe(ctsvc_ipc_get_handle_for_change_subsciption(),
					CTSVC_IPC_SUBSCRIBE_MODULE, CTSVC_SETTING_SORTING_ORDER_CHANGED,
					_csetting_name_sorting_order_subscribe_cb, NULL) != 0) {
			//LCOV_EXCL_START
			ERR("pims_ipc_subscribe() Fail");
			ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
			return CONTACTS_ERROR_IPC;
			//LCOV_EXCL_STOP
		}
	}

	for (l = __setting_name_sorting_order_subscribe_list; l; l = l->next) {
		ctsvc_name_sorting_order_changed_cb_info_s *cb_info = l->data;
		if (cb_info->cb == cb && cb_info->user_data == user_data) {
			//LCOV_EXCL_START
			ERR("The same callback(%x) is already exist", cb);
			ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
			return CONTACTS_ERROR_INVALID_PARAMETER;
			//LCOV_EXCL_STOP
		}
	}

	cb_info = calloc(1, sizeof(ctsvc_name_sorting_order_changed_cb_info_s));
	if (NULL == cb_info) {
		//LCOV_EXCL_START
		ERR("calloc() Fail");
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		//LCOV_EXCL_STOP
	}
	cb_info->user_data = user_data;
	cb_info->cb = cb;
	__setting_name_sorting_order_subscribe_list = g_slist_append(__setting_name_sorting_order_subscribe_list, cb_info);

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}

API int contacts_setting_remove_name_sorting_order_changed_cb(
		contacts_setting_name_sorting_order_changed_cb cb, void *user_data)
{
	int ret;
	RETV_IF(cb == NULL, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = ctsvc_ipc_destroy_for_change_subscription(false);
	if (CONTACTS_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("ctsvc_ipc_destroy_for_change_subscription() Fail(%d)", ret);
		return ret;
		//LCOV_EXCL_STOP
	}

	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	if (__setting_name_sorting_order_subscribe_list) {
		GSList *l;
		for (l = __setting_name_sorting_order_subscribe_list; l; l = l->next) {
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


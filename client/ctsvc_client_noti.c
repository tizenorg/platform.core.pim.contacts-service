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

#include <stdio.h>
#include <unistd.h>
#include <pims-ipc.h>
#include <pims-ipc-svc.h>
#include <pims-ipc-data.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_mutex.h"
#include "ctsvc_client_ipc.h"
#include "ctsvc_client_setting.h"
#include "contacts_extension.h"

typedef struct {
	contacts_db_change_cb_with_info cb;
	void *user_data;
} db_callback_info_s;

typedef struct {
	char *view_uri;
	GSList *callbacks;
} subscribe_info_s;

static int __ipc_pubsub_ref = 0;
static pims_ipc_h __ipc = NULL;
static GSList *__db_change_subscribe_list = NULL;

static void _cnoti_subscribe_cb(pims_ipc_h ipc, pims_ipc_data_h data,
		void *user_data)
{
	int ret;
	char *str = NULL;
	subscribe_info_s *info = user_data;

	if (data) {
		ret = ctsvc_ipc_unmarshal_string(data, &str);
		WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_ipc_unmarshal_string() Fail(%d)", ret);
	}

	if (NULL == str) {
		//LCOV_EXCL_START
		ERR("There is no changed data");
		free(str);
		return;
		//LCOV_EXCL_STOP
	}

	if (info) {
		GSList *l;
		for (l = info->callbacks; l; l = l->next) {
			db_callback_info_s *cb_info = l->data;
			if (cb_info->cb)
				cb_info->cb(info->view_uri, str, cb_info->user_data);
		}
	}
	free(str);
}

//LCOV_EXCL_START
int _cnoti_recover_for_change_subscription()
{
	GSList *it = NULL;
	subscribe_info_s *info = NULL;

	for (it = __db_change_subscribe_list; it; it = it->next) {
		if (NULL == it->data)
			continue;

		info = it->data;
		if (info->view_uri) {
			if (pims_ipc_subscribe(__ipc, CTSVC_IPC_SUBSCRIBE_MODULE, info->view_uri,
						_cnoti_subscribe_cb, (void*)info) != 0) {
				ERR("pims_ipc_subscribe() Fail");
				return CONTACTS_ERROR_IPC;
			}
		}
	}

	return CONTACTS_ERROR_NONE;
}
//LCOV_EXCL_STOP


/* This API should be called in CTS_MUTEX_PIMS_IPC_PUBSUB mutex */
pims_ipc_h ctsvc_ipc_get_handle_for_change_subsciption()
{
	return __ipc;
}

int ctsvc_ipc_create_for_change_subscription()
{
	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	if (0 < __ipc_pubsub_ref) {
		__ipc_pubsub_ref++;
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return CONTACTS_ERROR_NONE;
	}

	if (NULL == __ipc) {
		char sock_file[CTSVC_PATH_MAX_LEN] = {0};
		snprintf(sock_file, sizeof(sock_file), CTSVC_SOCK_PATH"/.%s_for_subscribe",
				getuid(), CTSVC_IPC_SERVICE);
		__ipc = pims_ipc_create_for_subscribe(sock_file);
		if (NULL == __ipc) {
			//LCOV_EXCL_START
			ERR("pims_ipc_create_for_subscribe() Fail");
			ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
			return CONTACTS_ERROR_IPC;
			//LCOV_EXCL_STOP
		}
	}
	__ipc_pubsub_ref++;
	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}

//LCOV_EXCL_START
int ctsvc_ipc_recover_for_change_subscription()
{
	int ret;
	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	if (__ipc_pubsub_ref <= 0) {
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return CONTACTS_ERROR_NONE;
	}

	pims_ipc_destroy_for_subscribe(__ipc);
	char sock_file[CTSVC_PATH_MAX_LEN] = {0};
	snprintf(sock_file, sizeof(sock_file), CTSVC_SOCK_PATH"/.%s_for_subscribe",
			getuid(), CTSVC_IPC_SERVICE);
	__ipc = pims_ipc_create_for_subscribe(sock_file);
	if (NULL == __ipc) {
		ERR("pims_ipc_create_for_subscribe() Fail");
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return CONTACTS_ERROR_IPC;
	}
	ret = ctsvc_setting_recover_for_change_subscription();
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_setting_recover_for_change_subscription() Fail(%d)", ret);
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return ret;
	}
	ret = _cnoti_recover_for_change_subscription();
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("__ctsvc_db_recover_changed_cb_with_info() Fail(%d)", ret);
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return ret;
	}

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}
//LCOV_EXCL_STOP

int ctsvc_ipc_destroy_for_change_subscription(bool is_disconnect)
{
	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	if (is_disconnect) {
		if (0 < __ipc_pubsub_ref) {
			pims_ipc_destroy_for_subscribe(__ipc);
			__ipc = NULL;
			__ipc_pubsub_ref = 0;
		}
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return CONTACTS_ERROR_NONE;
	}

	if (1 == __ipc_pubsub_ref) {
		pims_ipc_destroy_for_subscribe(__ipc);
		__ipc = NULL;
	} else if (1 < __ipc_pubsub_ref) {
		DBG("ctsvc pubsub ipc ref count : %d", __ipc_pubsub_ref);
	} else {
		DBG("System : please call connection APIs, connection count is (%d)",
				__ipc_pubsub_ref);
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	__ipc_pubsub_ref--;

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}

API int contacts_db_add_changed_cb_with_info(const char *view_uri,
		contacts_db_change_cb_with_info cb, void *user_data)
{
	GSList *it = NULL;
	subscribe_info_s *info = NULL;
	db_callback_info_s *cb_info;
	bool result = false;
	int ret;

	RETV_IF(view_uri == NULL, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(cb == NULL, CONTACTS_ERROR_INVALID_PARAMETER);

	if (STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_PHONELOG, strlen(CTSVC_VIEW_URI_PHONELOG))) {
		ret = ctsvc_ipc_client_check_permission(CTSVC_PERMISSION_PHONELOG_READ, &result);
		if (CONTACTS_ERROR_NONE != ret) {
			//LCOV_EXCL_START
			ERR("ctsvc_ipc_client_check_permission() Fail(%d)", ret);
			return ret;
			//LCOV_EXCL_STOP
		}

		RETVM_IF(result == false, CONTACTS_ERROR_PERMISSION_DENIED,
				"Permission denied (phonelog read)");
	} else if (STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_PERSON, strlen(CTSVC_VIEW_URI_PERSON))) {
		ret = ctsvc_ipc_client_check_permission(CTSVC_PERMISSION_CONTACT_READ, &result);
		if (CONTACTS_ERROR_NONE != ret) {
			//LCOV_EXCL_START
			ERR("ctsvc_ipc_client_check_permission() Fail(%d)", ret);
			return ret;
			//LCOV_EXCL_STOP
		}

		RETVM_IF(result == false, CONTACTS_ERROR_PERMISSION_DENIED,
				"Permission denied (contact read)");
	} else {
		//LCOV_EXCL_START
		ERR("We support this API for only %s and %s", CTSVC_VIEW_URI_PERSON,
				CTSVC_VIEW_URI_PHONELOG);
		return CONTACTS_ERROR_INVALID_PARAMETER;
		//LCOV_EXCL_STOP
	}

	ret = ctsvc_ipc_create_for_change_subscription();
	if (CONTACTS_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("ctsvc_ipc_create_for_change_subscription() Fail(%d)", ret);
		return ret;
		//LCOV_EXCL_STOP
	}

	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	for (it = __db_change_subscribe_list; it; it = it->next) {
		if (NULL == it->data) continue;

		info = it->data;
		if (STRING_EQUAL == strcmp(info->view_uri, view_uri))
			break;
		else
			info = NULL;
	}

	if (NULL == info) {
		info = calloc(1, sizeof(subscribe_info_s));
		if (NULL == info) {
			//LCOV_EXCL_START
			ERR("calloc() Fail");
			ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
			return CONTACTS_ERROR_OUT_OF_MEMORY;
			//LCOV_EXCL_STOP
		}

		if (pims_ipc_subscribe(__ipc, CTSVC_IPC_SUBSCRIBE_MODULE, (char*)view_uri,
					_cnoti_subscribe_cb, (void*)info) != 0) {
			//LCOV_EXCL_START
			ERR("pims_ipc_subscribe() Fail");
			free(info);
			ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
			return CONTACTS_ERROR_IPC;
			//LCOV_EXCL_STOP
		}
		info->view_uri = strdup(view_uri);
		__db_change_subscribe_list = g_slist_append(__db_change_subscribe_list, info);
	} else {
		GSList *l;
		for (l = info->callbacks; l; l = l->next) {
			db_callback_info_s *cb_info = l->data;
			if (cb_info->cb == cb && cb_info->user_data == user_data) {
				//LCOV_EXCL_START
				ERR("The same callback(%s) is already exist", view_uri);
				ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
				return CONTACTS_ERROR_INVALID_PARAMETER;
				//LCOV_EXCL_STOP
			}
		}
	}

	cb_info = calloc(1, sizeof(db_callback_info_s));
	if (NULL == cb_info) {
		//LCOV_EXCL_START
		ERR("calloc() Fail");
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		//LCOV_EXCL_STOP
	}
	cb_info->user_data = user_data;
	cb_info->cb = cb;
	info->callbacks = g_slist_append(info->callbacks, cb_info);

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}

API int contacts_db_remove_changed_cb_with_info(const char *view_uri,
		contacts_db_change_cb_with_info cb, void *user_data)
{
	int ret;
	GSList *it = NULL;
	subscribe_info_s *info = NULL;

	RETV_IF(view_uri == NULL, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(cb == NULL, CONTACTS_ERROR_INVALID_PARAMETER);

	if (STRING_EQUAL != strcmp(view_uri, CTSVC_VIEW_URI_PHONELOG)
			&& STRING_EQUAL != strcmp(view_uri, CTSVC_VIEW_URI_PERSON)) {
		//LCOV_EXCL_START
		ERR("We support this API for only %s and %s", CTSVC_VIEW_URI_PERSON,
				CTSVC_VIEW_URI_PHONELOG);
		return CONTACTS_ERROR_INVALID_PARAMETER;
		//LCOV_EXCL_STOP
	}

	ret = ctsvc_ipc_destroy_for_change_subscription(false);
	if (CONTACTS_ERROR_NONE != ret) {
		//LCOV_EXCL_START
		ERR("ctsvc_ipc_destroy_for_change_subscription() Fail(%d)", ret);
		return ret;
		//LCOV_EXCL_STOP
	}

	ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_PUBSUB);

	for (it = __db_change_subscribe_list; it; it = it->next) {
		if (NULL == it->data)
			continue;

		info = it->data;
		if (STRING_EQUAL == strcmp(info->view_uri, view_uri))
			break;
		else
			info = NULL;
	}

	if (info) {
		GSList *l;
		for (l = info->callbacks; l; l = l->next) {
			db_callback_info_s *cb_info = l->data;
			if (cb == cb_info->cb && user_data == cb_info->user_data) {
				info->callbacks = g_slist_remove(info->callbacks, cb_info);
				free(cb_info);
				break;
			}
		}
		if (g_slist_length(info->callbacks) == 0) {
			pims_ipc_unsubscribe(__ipc, CTSVC_IPC_SUBSCRIBE_MODULE, info->view_uri);
			__db_change_subscribe_list = g_slist_remove(__db_change_subscribe_list, info);
			free(info->view_uri);
			free(info);
		}
	}

	ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_PUBSUB);
	return CONTACTS_ERROR_NONE;
}

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
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <pims-ipc-data.h>
#include <pthread.h>

#include "ctsvc_client_ipc.h"
#include "ctsvc_client_utils.h"
#include "ctsvc_client_service_helper.h"

#include "ctsvc_internal.h"
#include "ctsvc_list.h"
#include "ctsvc_record.h"
#include "ctsvc_inotify.h"

#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_view.h"
#include "ctsvc_mutex.h"
#include "ctsvc_handle.h"

struct ctsvc_ipc_s {
	pims_ipc_h ipc;
	GList *list_handle;
};

static pthread_mutex_t _ctsvc_mutex_disconnected = PTHREAD_MUTEX_INITIALIZER;
static GHashTable *_ctsvc_ipc_table = NULL;
static bool _ctsvc_ipc_disconnected = false;

static pims_ipc_h _ctsvc_get_ipc_handle()
{
	struct ctsvc_ipc_s *ipc_data = NULL;
	char ipc_key[CTSVC_STR_SHORT_LEN] = {0};
	RETVM_IF(NULL == _ctsvc_ipc_table, NULL, "contacts not connected");

	snprintf(ipc_key, sizeof(ipc_key), "%u", ctsvc_client_get_tid());
	ipc_data = g_hash_table_lookup(_ctsvc_ipc_table, ipc_key);

	if (NULL == ipc_data) {
		snprintf(ipc_key, sizeof(ipc_key), "%u", ctsvc_client_get_pid());
		ipc_data = g_hash_table_lookup(_ctsvc_ipc_table, ipc_key);
	}

	RETVM_IF(NULL == ipc_data, NULL, "g_hash_table_lookup(%s) Fail", ipc_key);

	return ipc_data->ipc;
}

bool ctsvc_ipc_is_busy()
{
	bool ret = false;

	pims_ipc_h ipc = _ctsvc_get_ipc_handle();
	if (NULL == ipc) {
		CTS_ERR("_ctsvc_get_ipc_handle() return NULL");
		return false;
	}

	ret = pims_ipc_is_call_in_progress(ipc);
	if (ret)
		CTS_ERR("global ipc channel is busy.");

	return ret;
}

static int _ctsvc_ipc_create(pims_ipc_h *p_ipc)
{
	char sock_file[CTSVC_PATH_MAX_LEN] = {0};
	snprintf(sock_file, sizeof(sock_file), CTSVC_SOCK_PATH"/.%s", getuid(), CTSVC_IPC_SERVICE);
	pims_ipc_h ipc = pims_ipc_create(sock_file);
	if (NULL == ipc) {
		if (errno == EACCES) {
			CTS_ERR("pims_ipc_create() Failed(%d)", CONTACTS_ERROR_PERMISSION_DENIED);
			return CONTACTS_ERROR_PERMISSION_DENIED;
		}
		else {
			CTS_ERR("pims_ipc_create() Failed(%d)", CONTACTS_ERROR_IPC_NOT_AVALIABLE);
			return CONTACTS_ERROR_IPC_NOT_AVALIABLE;
		}
	}

	*p_ipc = ipc;
	return CONTACTS_ERROR_NONE;
}

static void _ctsvc_ipc_data_free(gpointer p)
{
	struct ctsvc_ipc_s *ipc_data = p;
	if (NULL == ipc_data)
		return;

	if (ipc_data->ipc) {
		ctsvc_ipc_unset_disconnected_cb(ipc_data->ipc);
		pims_ipc_destroy(ipc_data->ipc);
	}

	g_list_free(ipc_data->list_handle);

	free(ipc_data);
}

static int _ctsvc_ipc_connect(contacts_h contact, pims_ipc_h ipc)
{
	int ret;
	pims_ipc_data_h outdata = NULL;
	pims_ipc_data_h indata = NULL;

	/* Access control : put cookie to indata */
	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("pims_ipc_data_create() return NULL");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	ret = ctsvc_ipc_marshal_handle(contact, indata);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_ipc_marshal_handle Fail(%d)", ret);
		pims_ipc_data_destroy(indata);
		return ret;
	}

	/* ipc call */
	if (pims_ipc_call(ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_CONNECT, indata, &outdata) != 0) {
		CTS_ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}
	pims_ipc_data_destroy(indata);

	if (outdata) {
		ctsvc_ipc_unmarshal_int(outdata, &ret);
		pims_ipc_data_destroy(outdata);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_server_connect return(%d)", ret);
	}
	return ret;
}

static void _ctsvc_ipc_disconnected_cb(void *user_data)
{
	ctsvc_ipc_set_disconnected(true);
}

int ctsvc_ipc_connect(contacts_h contact, unsigned int handle_id)
{
	int ret = CONTACTS_ERROR_NONE;
	struct ctsvc_ipc_s *ipc_data = NULL;
	char ipc_key[CTSVC_STR_SHORT_LEN] = {0};

	snprintf(ipc_key, sizeof(ipc_key), "%u", handle_id);

	if (NULL == _ctsvc_ipc_table)
		_ctsvc_ipc_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _ctsvc_ipc_data_free);
	else
		ipc_data = g_hash_table_lookup(_ctsvc_ipc_table, ipc_key);

	if (NULL == ipc_data) {
		ipc_data = calloc(1, sizeof(struct ctsvc_ipc_s));
		if (NULL == ipc_data)
		{
			CTS_ERR("calloc() Fail");
			return CONTACTS_ERROR_OUT_OF_MEMORY;
		}
		ret = _ctsvc_ipc_create(&(ipc_data->ipc));
		if (CONTACTS_ERROR_NONE != ret) {
			_ctsvc_ipc_data_free(ipc_data);
			return ret;
		}
		g_hash_table_insert(_ctsvc_ipc_table, strdup(ipc_key), ipc_data);
		ctsvc_ipc_set_disconnected_cb(ipc_data->ipc, _ctsvc_ipc_disconnected_cb, NULL);
	}
	_ctsvc_ipc_connect(contact, ipc_data->ipc);
	ipc_data->list_handle = g_list_append(ipc_data->list_handle, contact);

	return CONTACTS_ERROR_NONE;
}


int ctsvc_ipc_disconnect(contacts_h contact, unsigned int handle_id, int connection_count)
{
	int ret = CONTACTS_ERROR_NONE;
	struct ctsvc_ipc_s *ipc_data = NULL;
	pims_ipc_data_h outdata = NULL;
	pims_ipc_data_h indata = NULL;
	char ipc_key[CTSVC_STR_SHORT_LEN] = {0};

	RETVM_IF(NULL == _ctsvc_ipc_table, CONTACTS_ERROR_IPC, "contacts not connected");
	snprintf(ipc_key, sizeof(ipc_key), "%u", handle_id);

	ipc_data = g_hash_table_lookup(_ctsvc_ipc_table, ipc_key);
	RETVM_IF(ipc_data == NULL, CONTACTS_ERROR_IPC, "contacts not connected");

	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail!");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	ret = ctsvc_ipc_marshal_handle(contact, indata);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_marshal_handle() Fail(%d)", ret);

	if (pims_ipc_call(ipc_data->ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_DISCONNECT, indata, &outdata) != 0) {
		pims_ipc_data_destroy(indata);
		CTS_ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata) {
		ctsvc_ipc_unmarshal_int(outdata, &ret);
		pims_ipc_data_destroy(outdata);

		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("[GLOBAL_IPC_CHANNEL] pims_ipc didn't destroyed!!!(%d)", ret);
			return ret;
		}

		if (1 == connection_count) {
			g_hash_table_remove(_ctsvc_ipc_table, ipc_key);
		}
	}
	else {
		CTS_ERR("pims_ipc_call out data is NULL");
		return CONTACTS_ERROR_IPC;
	}

	return ret;
}

static void __ctsvc_ipc_lock()
{
	if (0 == ctsvc_client_get_thread_connection_count())
		ctsvc_mutex_lock(CTS_MUTEX_PIMS_IPC_CALL);
}

static void __ctsvc_ipc_unlock(void)
{
	if (0 == ctsvc_client_get_thread_connection_count())
		ctsvc_mutex_unlock(CTS_MUTEX_PIMS_IPC_CALL);
}

int ctsvc_ipc_call(char *module, char *function, pims_ipc_h data_in, pims_ipc_data_h *data_out)
{
	pims_ipc_h ipc_handle;

	if (true == ctsvc_ipc_get_disconnected()) {
		ctsvc_ipc_set_disconnected(false);
		ctsvc_ipc_recovery();
		ctsvc_ipc_recover_for_change_subscription();
	}

	ipc_handle = _ctsvc_get_ipc_handle();

	__ctsvc_ipc_lock();
	int ret = pims_ipc_call(ipc_handle, module, function, data_in, data_out);
	__ctsvc_ipc_unlock();

	return ret;
}

void ctsvc_client_ipc_set_change_version(contacts_h contact, int version)
{
	RETM_IF(NULL == contact, "contact is NULL");
	ctsvc_base_s *base = (ctsvc_base_s *)contact;
	base->version = version;
}

int ctsvc_client_ipc_get_change_version(contacts_h contact)
{
	RETVM_IF(NULL == contact, -1, "contact is NULL");
	ctsvc_base_s *base = (ctsvc_base_s *)contact;
	return base->version;
}

int ctsvc_ipc_client_check_permission(int permission, bool *result)
{
	pims_ipc_data_h indata = NULL;
	pims_ipc_data_h outdata = NULL;
	int ret;

	if (result)
		*result = false;

	indata = pims_ipc_data_create(0);
	if (indata == NULL) {
		CTS_ERR("ipc data created fail !");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	ret = ctsvc_ipc_marshal_int(permission, indata);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("marshal fail");
		pims_ipc_data_destroy(indata);
		return ret;
	}

	if (ctsvc_ipc_call(CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_CHECK_PERMISSION, indata, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call Fail");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(outdata, &ret)) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			pims_ipc_data_destroy(outdata);
			return CONTACTS_ERROR_IPC;
		}

		if (CONTACTS_ERROR_NONE == ret && result) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_bool(outdata, result)) {
				CTS_ERR("ctsvc_ipc_unmarshal_bool() Fail");
				pims_ipc_data_destroy(outdata);
				return CONTACTS_ERROR_IPC;
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}

int ctsvc_ipc_set_disconnected_cb(pims_ipc_h ipc, void (*cb)(void *), void *user_data)
{
	return pims_ipc_add_server_disconnected_cb(ipc, cb, user_data);
}

int ctsvc_ipc_unset_disconnected_cb(pims_ipc_h ipc)
{
	return pims_ipc_remove_server_disconnected_cb(ipc);
}

void ctsvc_ipc_set_disconnected(bool is_disconnected)
{
	pthread_mutex_lock(&_ctsvc_mutex_disconnected);
	_ctsvc_ipc_disconnected = is_disconnected;
	pthread_mutex_unlock(&_ctsvc_mutex_disconnected);
}

int ctsvc_ipc_get_disconnected()
{
	pthread_mutex_lock(&_ctsvc_mutex_disconnected);
	CTS_DBG("_ctsvc_ipc_disconnected=%d", _ctsvc_ipc_disconnected);
	pthread_mutex_unlock(&_ctsvc_mutex_disconnected);
	return _ctsvc_ipc_disconnected;
}

static void _ctsvc_ipc_recovery_foreach_cb(gpointer key, gpointer value, gpointer user_data)
{
	GList *c;
	struct ctsvc_ipc_s *ipc_data = value;

	ctsvc_ipc_unset_disconnected_cb(ipc_data->ipc);
	int ret = _ctsvc_ipc_create(&(ipc_data->ipc));
	RETM_IF(CONTACTS_ERROR_NONE != ret, "_ctsvc_ipc_create() Fail(%d)", ret);
	ctsvc_ipc_set_disconnected_cb(ipc_data->ipc, _ctsvc_ipc_disconnected_cb, NULL);

	for (c=ipc_data->list_handle;c;c=c->next) {
		contacts_h contact = c->data;
		ret = _ctsvc_ipc_connect(contact, ipc_data->ipc);
		WARN_IF(CONTACTS_ERROR_NONE != ret, "_ctsvc_ipc_connect() Fail(%d)", ret);
	}
}

void ctsvc_ipc_recovery()
{
	CTS_FN_CALL;
	g_hash_table_foreach(_ctsvc_ipc_table, _ctsvc_ipc_recovery_foreach_cb, NULL);
}



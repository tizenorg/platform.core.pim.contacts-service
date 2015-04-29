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

#include <sys/types.h>
#include <unistd.h>
#include <glib.h>
#include <stdlib.h>
#include <pims-ipc-data.h>

#include "ctsvc_client_ipc.h"

#include "ctsvc_internal.h"
#include "ctsvc_list.h"
#include "ctsvc_record.h"
#include "ctsvc_query.h"
#include "ctsvc_inotify.h"

#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_view.h"
#include "ctsvc_mutex.h"

#define CTS_STR_SHORT_LEN 1024 //short sql string length

static GHashTable *_ctsvc_ipc_table = NULL;
static bool _ctsvc_ipc_disconnected = false;
static int disconnected_cb_count = 0;

static __thread int __contacts_change_version = 0;
static int __contacts_global_change_version = 0;

static inline void _ctsvc_ipc_get_pid_str(char *buf, int buf_size)
{
	pid_t pid = getpid();
	snprintf(buf, buf_size, "%d", (unsigned int)pid);
}

static inline void _ctsvc_ipc_get_tid_str(char *buf, int buf_size)
{
	pthread_t tid = pthread_self();
	snprintf(buf, buf_size, "%d", (unsigned int)tid);
}

static pims_ipc_h _ctsvc_get_ipc_handle()
{
	pims_ipc_h ipc = NULL;
	char ipc_key[CTS_STR_SHORT_LEN] = {0};
	RETVM_IF(NULL == _ctsvc_ipc_table, NULL, "contacts not connected");

	_ctsvc_ipc_get_tid_str(ipc_key, sizeof(ipc_key));
	ipc = g_hash_table_lookup(_ctsvc_ipc_table, ipc_key);
	if (NULL == ipc) {
		_ctsvc_ipc_get_pid_str(ipc_key, sizeof(ipc_key));
		ipc = g_hash_table_lookup(_ctsvc_ipc_table, ipc_key);
	}

	return ipc;
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
	int ret;
	pims_ipc_data_h outdata = NULL;

	char sock_path[CTSVC_PATH_MAX_LEN] = {0};
	snprintf(sock_path, sizeof(sock_path), CTSVC_SOCK_PATH"/.%s", getuid(), CTSVC_IPC_SERVICE);
	pims_ipc_h ipc = pims_ipc_create(sock_path);
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

	// ipc call
	if (pims_ipc_call(ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_CONNECT, NULL, &outdata) != 0) {
		CTS_ERR("pims_ipc_call failed");
		ret = CONTACTS_ERROR_IPC;
		goto DATA_FREE;
	}

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		pims_ipc_data_destroy(outdata);

		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_server_connect return(%d)", ret);
			goto DATA_FREE;
		}
	}
	*p_ipc = ipc;
	return CONTACTS_ERROR_NONE;
DATA_FREE:
	pims_ipc_destroy(ipc);
	return ret;
}

static void _ctsvc_ipc_data_free(gpointer p)
{
	pims_ipc_h ipc = p;
	if (NULL == ipc)
		return;

	if (ipc)
		pims_ipc_destroy(ipc);
}

static int _ctsvc_ipc_connect(pims_ipc_h ipc)
{
	int ret;
	pims_ipc_data_h outdata = NULL;

	// ipc call
	if (pims_ipc_call(ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_CONNECT, NULL, &outdata) != 0) {
		CTS_ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata) {
		ctsvc_ipc_unmarshal_int(outdata, &ret);
		pims_ipc_data_destroy(outdata);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_server_connect return(%d)", ret);
	}
	return ret;
}

int ctsvc_ipc_connect()
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_h ipc = NULL;
	char ipc_key[CTS_STR_SHORT_LEN] = {0};

	RETV_IF(_ctsvc_ipc_disconnected, CONTACTS_ERROR_IPC_NOT_AVALIABLE);

	_ctsvc_ipc_get_tid_str(ipc_key, sizeof(ipc_key));

	if (NULL == _ctsvc_ipc_table)
		_ctsvc_ipc_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, _ctsvc_ipc_data_free);
	else
		ipc = g_hash_table_lookup(_ctsvc_ipc_table, ipc_key);

	if (NULL == ipc) {
		ret = _ctsvc_ipc_create(&ipc);
		if (CONTACTS_ERROR_NONE != ret) {
			_ctsvc_ipc_data_free(ipc);
			return ret;
		}
		g_hash_table_insert(_ctsvc_ipc_table, strdup(ipc_key), ipc);
	}
	_ctsvc_ipc_connect(ipc);

	return CONTACTS_ERROR_NONE;
}


int ctsvc_ipc_disconnect()
{
	int ret = CONTACTS_ERROR_NONE;
	char ipc_key[CTS_STR_SHORT_LEN] = {0};
	pims_ipc_h ipc = NULL;
	pims_ipc_data_h outdata = NULL;

	RETV_IF(_ctsvc_ipc_disconnected, CONTACTS_ERROR_IPC_NOT_AVALIABLE);
	RETVM_IF(NULL == _ctsvc_ipc_table, CONTACTS_ERROR_IPC, "contacts not connected");

	_ctsvc_ipc_get_tid_str(ipc_key, sizeof(ipc_key));

	ipc = g_hash_table_lookup(_ctsvc_ipc_table, ipc_key);
	RETVM_IF(ipc == NULL, CONTACTS_ERROR_IPC, "contacts not connected");

	if (pims_ipc_call(ipc, CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_DISCONNECT, NULL, &outdata) != 0) {
		CTS_ERR("[GLOBAL_IPC_CHANNEL] pims_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata) {
		ctsvc_ipc_unmarshal_int(outdata, &ret);
		pims_ipc_data_destroy(outdata);

		if (ret != CONTACTS_ERROR_NONE)
			CTS_ERR("[GLOBAL_IPC_CHANNEL] pims_ipc didn't destroyed!!!(%d)", ret);

		g_hash_table_remove(_ctsvc_ipc_table, ipc_key);
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
	pims_ipc_h ipc_handle = _ctsvc_get_ipc_handle();

	__ctsvc_ipc_lock();

	int ret = pims_ipc_call(ipc_handle, module, function, data_in, data_out);

	__ctsvc_ipc_unlock();

	return ret;
}

int ctsvc_ipc_call_async(char *module, char *function, pims_ipc_h data_in, pims_ipc_call_async_cb callback, void *userdata)
{
	pims_ipc_h ipc_handle = _ctsvc_get_ipc_handle();

	__ctsvc_ipc_lock();

	int ret = pims_ipc_call_async(ipc_handle, module, function, data_in, callback, userdata);

	__ctsvc_ipc_unlock();

	return ret;
}

void ctsvc_client_ipc_set_change_version(int version)
{
	pims_ipc_h ipc = NULL;
	char ipc_key[CTS_STR_SHORT_LEN] = {0};
	RETVM_IF(NULL == _ctsvc_ipc_table, NULL, "contacts not connected");

	_ctsvc_ipc_get_tid_str(ipc_key, sizeof(ipc_key));
	ipc = g_hash_table_lookup(_ctsvc_ipc_table, ipc_key);

	if (NULL == ipc) {
		__contacts_global_change_version = version;
		CTS_DBG("change_version = %d", version);
		return;
	}
	__contacts_change_version = version;
	CTS_DBG("change_version = %d", version);
}

int ctsvc_client_ipc_get_change_version(void)
{
	pims_ipc_h ipc = NULL;
	char ipc_key[CTS_STR_SHORT_LEN] = {0};
	RETVM_IF(NULL == _ctsvc_ipc_table, NULL, "contacts not connected");

	_ctsvc_ipc_get_tid_str(ipc_key, sizeof(ipc_key));
	ipc = g_hash_table_lookup(_ctsvc_ipc_table, ipc_key);

	if (NULL == ipc)
		return __contacts_global_change_version;

	return __contacts_change_version;
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
		CTS_ERR("ctsvc_ipc_call failed");
		pims_ipc_data_destroy(indata);
		return CONTACTS_ERROR_IPC;
	}

	pims_ipc_data_destroy(indata);

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata,&size);

		if (ret == CONTACTS_ERROR_NONE) {
			if (result)
				*result = *(bool*) pims_ipc_data_get(outdata, &size);
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}



int ctsvc_ipc_set_disconnected_cb(void (*cb)(void *), void *user_data)
{
	if (0 == disconnected_cb_count++)
		return pims_ipc_set_server_disconnected_cb(cb, user_data);
	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_unset_disconnected_cb()
{
	if (1 == disconnected_cb_count--)
		return pims_ipc_unset_server_disconnected_cb();
	return CONTACTS_ERROR_NONE;
}

void ctsvc_ipc_set_disconnected(bool is_disconnected)
{
	_ctsvc_ipc_disconnected = is_disconnected;
}

static void _ctsvc_ipc_recovery_foreach_cb(gpointer key, gpointer value, gpointer user_data)
{
	GList *c;
	pims_ipc_h ipc = value;

	int ret = _ctsvc_ipc_create(&ipc);
	RETM_IF(CONTACTS_ERROR_NONE != ret, "_ctsvc_ipc_create() Fail(%d)", ret);

	ret = _ctsvc_ipc_connect(ipc);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "_ctsvc_ipc_connect() Fail(%d)", ret);
}

void ctsvc_ipc_recovery()
{
	CTS_DBG("ctsvc_ipc_recovery (_ctsvc_ipc_disconnected=%d)", _ctsvc_ipc_disconnected);

	if (false == _ctsvc_ipc_disconnected)
		return;

	g_hash_table_foreach(_ctsvc_ipc_table, _ctsvc_ipc_recovery_foreach_cb, NULL);
}



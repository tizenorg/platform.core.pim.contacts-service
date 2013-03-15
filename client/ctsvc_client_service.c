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
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pims-ipc-data.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_socket.h"
#include "ctsvc_mutex.h"
#include "ctsvc_inotify.h"
#include "ctsvc_setting.h"
#include "ctsvc_client_ipc.h"

static int ctsvc_connection = 0;

static __thread int ctsvc_connection_on_thread = 0;

API int contacts_connect_with_flags(unsigned int flags)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;

	ret = contacts_connect2();
	if (ret == CONTACTS_ERROR_NONE)
		return ret;

	if (flags & CONTACTS_CONNECT_FLAG_RETRY) {
		int i;
		int waiting_time = 500;
		for (i=0;i<7;i++) {
			usleep(waiting_time * 1000);
			DBG("retry cnt=%d, ret=%x, %d",(i+1), ret, waiting_time);
			ret = contacts_connect2();
			if (ret == CONTACTS_ERROR_NONE)
				break;
			waiting_time *= 2;
		}
	}

	return ret;
}

API int contacts_connect2()
{
	CTS_FN_CALL;
	int ret;

	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);
	if (0 == ctsvc_connection) {

		ret = ctsvc_ipc_connect();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_connect() Failed(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}

		ret = ctsvc_socket_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_socket_init() Failed(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}

		ret = ctsvc_inotify_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_inotify_init() Failed(%d)", ret);
			ctsvc_socket_final();
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}

		ctsvc_view_uri_init();
		ctsvc_register_vconf();
		ctsvc_ipc_create_for_change_subscription();
	}
	else
		CTS_DBG("System : Contacts service has been already connected(%d)", ctsvc_connection + 1);

	ctsvc_connection++;
	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}

API int contacts_disconnect2()
{
	int ret;

	CTS_FN_CALL;

	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);
	if (1 == ctsvc_connection) {
		ctsvc_ipc_destroy_for_change_subscription();

		ret = ctsvc_ipc_disconnect();
		if (ret != CONTACTS_ERROR_NONE) {
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			CTS_ERR("ctsvc_ipc_disconnect() Failed(%d)", ret);
			return ret;
		}

		ctsvc_view_uri_deinit();
		ctsvc_inotify_close();
		ctsvc_socket_final();
		ctsvc_deregister_vconf();

	}
	else if (1 < ctsvc_connection)
		CTS_DBG("System : connection count is %d", ctsvc_connection);
	else {
		CTS_DBG("System : please call contacts_connect2(), connection count is (%d)", ctsvc_connection);
		ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	ctsvc_connection--;
	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}

API int contacts_connect_on_thread()
{
	int ret;

	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);

	ret = ctsvc_ipc_connect_on_thread();
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_ipc_connect_on_thread() Failed(%d)", ret);
		ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
		return ret;
	}

	if (0 < ctsvc_connection_on_thread)
		CTS_DBG("System : Contacts service has been already connected");

	ctsvc_connection_on_thread++;

	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}

API int contacts_disconnect_on_thread()
{
	int ret;

	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);

	ret = ctsvc_ipc_disconnect_on_thread();
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_ipc_disconnect_on_thread() Failed(%d)", ret);
		ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
		return ret;
	}

	if (1 == ctsvc_connection_on_thread) {
		CTS_DBG("System : connection_on_thread was destroyed successfully");
	}
	else if (1 < ctsvc_connection_on_thread) {
		CTS_DBG("System : connection count is %d", ctsvc_connection_on_thread);
	}
	else {
		CTS_DBG("System : please call contacts_connect_on_thread(), connection count is (%d)", ctsvc_connection_on_thread);
		ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	ctsvc_connection_on_thread--;

	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}

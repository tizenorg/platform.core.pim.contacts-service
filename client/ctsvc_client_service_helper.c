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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_socket.h"
#include "ctsvc_mutex.h"
#include "ctsvc_inotify.h"
#include "ctsvc_client_ipc.h"
#include "ctsvc_client_utils.h"
#include "ctsvc_client_handle.h"
#include "ctsvc_client_service_helper.h"

static int _ctsvc_connection = 0;
static __thread int _ctsvc_connection_on_thread = 0;

int ctsvc_client_get_thread_connection_count()
{
	return _ctsvc_connection_on_thread;
}

int ctsvc_client_connect_with_flags(contacts_h contact, unsigned int flags)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;

	/* If new flag is defined, errer check should be updated */
	RETVM_IF(flags & 0x11111110, CONTACTS_ERROR_INVALID_PARAMETER, "flags is invalid");

	ret = ctsvc_client_connect(contact);
	if (ret == CONTACTS_ERROR_PERMISSION_DENIED)
		return ret;
	else if (ret == CONTACTS_ERROR_NONE)
		return ret;

	if (flags & CONTACTS_CONNECT_FLAG_RETRY) {
		int i;
		int waiting_time = 500;
		for (i=0;i<9;i++) {
			usleep(waiting_time * 1000);
			CTS_DBG("retry cnt=%d, ret=%x, %d",(i+1), ret, waiting_time);
			ret = ctsvc_client_connect(contact);
			if (ret == CONTACTS_ERROR_NONE)
				break;
			if (6 < i)
				waiting_time += 30000;
			else
				waiting_time *= 2;
		}
	}

	return ret;
}

static void _ctsvc_ipc_disconnected_cb(void *user_data)
{
	ctsvc_ipc_set_disconnected(true);
}

static void _ctsvc_ipc_initialized_cb(void *user_data)
{
	ctsvc_ipc_recovery();
	ctsvc_ipc_recover_for_change_subscription();
	ctsvc_ipc_set_disconnected(false);
}

int ctsvc_client_connect(contacts_h contact)
{
	CTS_FN_CALL;
	int ret;
	ctsvc_base_s *base = (ctsvc_base_s *)contact;

 	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);
	if (0 == base->connection_count) {
		ret = ctsvc_ipc_connect(contact, ctsvc_client_get_pid());
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_connect() Fail(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}
	}
	base->connection_count++;

	if (0 == _ctsvc_connection) {
		ret = ctsvc_socket_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_socket_init() Fail(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}

		ret = ctsvc_inotify_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_inotify_init() Fail(%d)", ret);
			ctsvc_socket_final();
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}
		ctsvc_view_uri_init();
		ctsvc_ipc_create_for_change_subscription();
		ctsvc_ipc_set_disconnected_cb(_ctsvc_ipc_disconnected_cb, NULL);
	}
	else
		CTS_DBG("System : Contacts service has been already connected(%d)", _ctsvc_connection + 1);

	if (1 == base->connection_count)
		ctsvc_inotify_subscribe_ipc_ready(contact, _ctsvc_ipc_initialized_cb, NULL);

	_ctsvc_connection++;
	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_client_disconnect(contacts_h contact)
{
	CTS_FN_CALL;
	int ret;
	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);

	ctsvc_base_s *base = (ctsvc_base_s *)contact;
	if (1 == base->connection_count) {
		ret = ctsvc_ipc_disconnect(contact, ctsvc_client_get_pid(), _ctsvc_connection);
		if (ret != CONTACTS_ERROR_NONE) {
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			CTS_ERR("ctsvc_ipc_disconnect() Fail(%d)", ret);
			return ret;
		}
		ctsvc_inotify_unsubscribe_ipc_ready(contact);
	}
	base->connection_count--;

	if (1 == _ctsvc_connection) {
		ctsvc_ipc_destroy_for_change_subscription();
		ctsvc_view_uri_deinit();
		ctsvc_inotify_close();
		ctsvc_socket_final();
		ctsvc_ipc_unset_disconnected_cb();
	}
	else if (1 < _ctsvc_connection)
		CTS_DBG("System : connection count is %d", _ctsvc_connection);
	else {
		CTS_DBG("System : please call contacts_connect(), connection count is (%d)", _ctsvc_connection);
		ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
		return CONTACTS_ERROR_DB;
	}

	_ctsvc_connection--;
	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_client_connect_on_thread(contacts_h contact)
{
	int ret;
	ctsvc_base_s *base = (ctsvc_base_s *)contact;

	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);

	if (0 == base->connection_count) {
		ret = ctsvc_ipc_connect(contact, ctsvc_client_get_tid());
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_connect() Fail(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}
	}
	base->connection_count++;

	if (0 == _ctsvc_connection_on_thread) {
		ret = ctsvc_socket_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_socket_init() Fail(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}

		ret = ctsvc_inotify_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_inotify_init() Fail(%d)", ret);
			ctsvc_socket_final();
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}
		ctsvc_view_uri_init();
		ctsvc_ipc_create_for_change_subscription();
		ctsvc_ipc_set_disconnected_cb(_ctsvc_ipc_disconnected_cb, NULL);
	}
	else if (0 < _ctsvc_connection_on_thread)
		CTS_DBG("System : Contacts service has been already connected");

	if (1 == base->connection_count)
		ctsvc_inotify_subscribe_ipc_ready(contact, _ctsvc_ipc_initialized_cb, NULL);

	_ctsvc_connection_on_thread++;

	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_client_disconnect_on_thread(contacts_h contact)
{
	int ret;
	ctsvc_base_s *base = (ctsvc_base_s *)contact;

	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);

	if (1 == base->connection_count) {
		ret = ctsvc_ipc_disconnect(contact, ctsvc_client_get_tid(), _ctsvc_connection_on_thread);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_disconnect() Fail(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}
		ctsvc_inotify_unsubscribe_ipc_ready(contact);
	}
	base->connection_count--;

	if (1 == _ctsvc_connection_on_thread) {
		ctsvc_ipc_destroy_for_change_subscription();
		ctsvc_view_uri_deinit();
		ctsvc_inotify_close();
		ctsvc_socket_final();
		ctsvc_ipc_unset_disconnected_cb();
		CTS_DBG("System : connection_on_thread was destroyed successfully");
	}
	else if (1 < _ctsvc_connection_on_thread) {
		CTS_DBG("System : connection count is %d", _ctsvc_connection_on_thread);
	}
	else {
		CTS_DBG("System : please call contacts_connect_on_thread(), connection count is (%d)", _ctsvc_connection_on_thread);
		ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
		return CONTACTS_ERROR_DB;
	}

	_ctsvc_connection_on_thread--;

	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}


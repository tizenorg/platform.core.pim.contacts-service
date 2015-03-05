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

#ifdef _CONTACTS_NATIVE
#include <security-server.h>
#endif

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_socket.h"
#include "ctsvc_mutex.h"
#include "ctsvc_inotify.h"
#include "ctsvc_db_init.h"
#include "ctsvc_setting.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_number_utils.h"

static int ctsvc_connection = 0;
static __thread int thread_connection = 0;

#ifdef _CONTACTS_NATIVE
void __ctsvc_addressbook_deleted_cb(const char* view_uri, void* user_data)
{
	// access control update
	ctsvc_reset_all_client_access_info();
}
#endif

API int contacts_connect()
{
	CTS_FN_CALL;
	int ret;

	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);
	if (0 == ctsvc_connection) {
#ifdef _CONTACTS_NATIVE
		ret = ctsvc_socket_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_socket_init() Failed(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}
#endif
		ret = ctsvc_inotify_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_inotify_init() Failed(%d)", ret);
#ifdef _CONTACTS_NATIVE
			ctsvc_socket_final();
#endif
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}
		ctsvc_db_plugin_init();
		ctsvc_view_uri_init();
		ctsvc_register_vconf();
#ifdef _CONTACTS_NATIVE
		contacts_db_add_changed_cb(_contacts_address_book._uri, __ctsvc_addressbook_deleted_cb, NULL);
#endif
	}
	else
		CTS_DBG("System : Contacts service has been already connected");

	ctsvc_connection++;

	if (0 == thread_connection) {
		ret = ctsvc_db_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_db_init() Failed(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}
#ifdef _CONTACTS_NATIVE
		// Access control : get cookie from security-server
		char *smack_label = NULL;
		size_t cookie_size = security_server_get_cookie_size();

		if (cookie_size <= 0) {
			CTS_ERR("security_server_get_cookie_size : cookie_size is %d", cookie_size);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return CONTACTS_ERROR_SYSTEM;
		}

		char cookie[cookie_size];
		cookie[0] = '\0';
		ret = security_server_request_cookie(cookie, cookie_size);
		if(ret < 0) {
			CTS_ERR("security_server_request_cookie fail (%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return CONTACTS_ERROR_SYSTEM;
		}

		smack_label = security_server_get_smacklabel_cookie(cookie);
		if (NULL == smack_label) {
			CTS_ERR("security_server_get_smacklabel_cookie fail");
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return CONTACTS_ERROR_SYSTEM;
		}

		// In case of server, set SMACK label in ctsvc_ipc_server_connect()
		ctsvc_set_client_access_info(smack_label, cookie);
		free(smack_label);
#endif
	}
	thread_connection++;

	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}

API int contacts_disconnect()
{
	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);

	if (1 == thread_connection) {
		ctsvc_db_deinit();
#ifdef _CONTACTS_NATIVE
		ctsvc_unset_client_access_info();
#endif
	}
	else if (thread_connection <= 0) {
		CTS_DBG("System : please call contacts_connect_on_thread(), thread_connection count is (%d)", thread_connection);
		ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	thread_connection--;

	if (1 == ctsvc_connection) {
#ifdef _CONTACTS_NATIVE
		ctsvc_socket_final();
#endif
		ctsvc_inotify_close();
		ctsvc_deregister_vconf();
		ctsvc_view_uri_deinit();
		ctsvc_db_plugin_deinit();
		ctsvc_deinit_tapi_handle_for_cc();
#ifdef _CONTACTS_NATIVE
		contacts_db_remove_changed_cb(_contacts_address_book._uri, __ctsvc_addressbook_deleted_cb, NULL);
#endif
	}
	else if (1 < ctsvc_connection)
		CTS_DBG("System : connection count is %d", ctsvc_connection);
	else {
		CTS_DBG("System : please call contacts_connect(), connection count is (%d)", ctsvc_connection);
		ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	ctsvc_connection--;
	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_contacts_internal_disconnect()
{
	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);

	if (1 == thread_connection) {
		ctsvc_db_deinit();
		thread_connection--;

		if (1 <= ctsvc_connection) {
			ctsvc_connection--;
		}
	}

	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
	return CONTACTS_ERROR_NONE;
}

#ifdef _CONTACTS_NATIVE
API int contacts_connect_with_flags(unsigned int flags)
{
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;

	ret = contacts_connect();
	if (ret == CONTACTS_ERROR_NONE)
		return ret;

	if (flags & CONTACTS_CONNECT_FLAG_RETRY) {
		int i;
		int waiting_time = 500;
		for (i=0;i<9;i++) {
			usleep(waiting_time * 1000);
			CTS_DBG("retry cnt=%d, ret=%x, %d",(i+1), ret, waiting_time);
			ret = contacts_connect();
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

API int contacts_connect_on_thread()
{
	int ret;

	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);

	if (0 == thread_connection) {
#ifdef _CONTACTS_NATIVE
		ret = ctsvc_socket_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_socket_init() Failed(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}
#endif
		ret = ctsvc_inotify_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_inotify_init() Failed(%d)", ret);
#ifdef _CONTACTS_NATIVE
			ctsvc_socket_final();
#endif
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}
		ctsvc_db_plugin_init();
		ctsvc_view_uri_init();
		ctsvc_register_vconf();
		contacts_db_add_changed_cb(_contacts_address_book._uri, __ctsvc_addressbook_deleted_cb, NULL);
		ret = ctsvc_db_init();
		if (ret != CONTACTS_ERROR_NONE)
		{
			CTS_ERR("ctsvc_db_init() Failed(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}

		// Access control : get cookie from security-server
		char *smack_label = NULL;
		size_t cookie_size = security_server_get_cookie_size();

		if (cookie_size <= 0) {
			CTS_ERR("security_server_get_cookie_size : cookie_size is %d", cookie_size);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return CONTACTS_ERROR_SYSTEM;
		}

		char cookie[cookie_size];
		cookie[0] = '\0';
		ret = security_server_request_cookie(cookie, cookie_size);
		if(ret < 0) {
			CTS_ERR("security_server_request_cookie fail (%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return CONTACTS_ERROR_SYSTEM;
		}

		smack_label = security_server_get_smacklabel_cookie(cookie);
		if (NULL == smack_label) {
			CTS_ERR("security_server_get_smacklabel_cookie fail");
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return CONTACTS_ERROR_SYSTEM;
		}

		// In case of server, set SMACK label in ctsvc_ipc_server_connect()
		ctsvc_set_client_access_info(smack_label, cookie);
		free(smack_label);
	}
	else
	{
		CTS_DBG("System : db connection on thread already exist");
	}
	thread_connection++;

	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}

API int contacts_disconnect_on_thread()
{
	int ret;
	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);

	if (1 == thread_connection) {
		ctsvc_db_deinit();
		ctsvc_unset_client_access_info();
#ifdef _CONTACTS_NATIVE
		ctsvc_socket_final();
#endif
		ctsvc_inotify_close();
		ctsvc_deregister_vconf();
		ctsvc_view_uri_deinit();
		ctsvc_db_plugin_deinit();
		ctsvc_deinit_tapi_handle_for_cc();
		contacts_db_remove_changed_cb(_contacts_address_book._uri, __ctsvc_addressbook_deleted_cb, NULL);
	}
	else if (thread_connection <= 0) {
		CTS_DBG("System : please call contacts_connect_on_thread(), connection count is (%d)", thread_connection);
		ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	else
	{
		CTS_DBG("System : db connection on thread count is (%d)", thread_connection);
	}

	thread_connection--;

	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}
#endif


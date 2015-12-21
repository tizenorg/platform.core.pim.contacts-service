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
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_socket.h"
#include "ctsvc_mutex.h"
#include "ctsvc_inotify.h"
#include "ctsvc_db_init.h"
#include "ctsvc_server_setting.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_number_utils.h"

static int ctsvc_connection = 0;
static __thread int thread_connection = 0;

int ctsvc_connect()
{
	CTS_FN_CALL;
	int ret;

	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);
	if (0 == ctsvc_connection) {
		ret = ctsvc_inotify_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_inotify_init() Fail(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}
		ctsvc_db_plugin_init();
		ctsvc_view_uri_init();
		ctsvc_register_vconf();
	} else {
		CTS_DBG("System : Contacts service has been already connected");
	}

	ctsvc_connection++;

	if (0 == thread_connection) {
		ret = ctsvc_db_init();
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_db_init() Fail(%d)", ret);
			ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}
	}
	thread_connection++;

	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_disconnect()
{
	ctsvc_mutex_lock(CTS_MUTEX_CONNECTION);

	if (1 == thread_connection) {
		ctsvc_db_deinit();
	} else if (thread_connection <= 0) {
		CTS_DBG("System : please call contacts_connect_on_thread(), thread_connection count is (%d)", thread_connection);
		ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	thread_connection--;

	if (1 == ctsvc_connection) {
		ctsvc_inotify_close();
		ctsvc_deregister_vconf();
		ctsvc_view_uri_deinit();
		ctsvc_db_plugin_deinit();
		ctsvc_deinit_tapi_handle_for_cc();
	} else if (1 < ctsvc_connection) {
		CTS_DBG("System : connection count is %d", ctsvc_connection);
	} else {
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

		if (1 <= ctsvc_connection)
			ctsvc_connection--;
	}

	ctsvc_mutex_unlock(CTS_MUTEX_CONNECTION);
	return CONTACTS_ERROR_NONE;
}


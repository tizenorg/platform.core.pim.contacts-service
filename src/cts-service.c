/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjae Shin <yj99.shin@samsung.com>
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
#include <stdarg.h>
#include <dlfcn.h>

#include "cts-internal.h"
#include "cts-schema.h"
#include "cts-sqlite.h"
#include "cts-utils.h"
#include "cts-service.h"
#include "cts-socket.h"
#include "cts-normalize.h"
#include "cts-list.h"
#include "cts-pthread.h"

static int cts_conn_refcnt = 0;

API int contacts_svc_connect(void)
{
	CTS_FN_CALL;
	int ret;

	cts_mutex_lock(CTS_MUTEX_CONNECTION);
	if (0 < cts_conn_refcnt) {
		CTS_DBG("The contacts-service is already connected. refcnt=%d", cts_conn_refcnt);
		cts_conn_refcnt++;
	}
	else
	{
		ret = cts_socket_init();
		if (CTS_SUCCESS != ret) {
			void *handle, *fn;
			handle = dlopen(NULL, RTLD_GLOBAL);
			fn = dlsym(handle, "cts_helper_normalize_name");
			if (NULL == fn) {
				ERR("cts_socket_init() Failed(%d)", ret);
				cts_mutex_unlock(CTS_MUTEX_CONNECTION);
				return ret;
			}
			cts_set_extra_normalize_fn(fn);
		}

		ret = cts_db_open();
		if (ret != CTS_SUCCESS) {
			ERR("cts_db_open() Failed(%d)", ret);
			cts_socket_final();
			cts_mutex_unlock(CTS_MUTEX_CONNECTION);
			return ret;
		}

		cts_register_noti();
		cts_conn_refcnt = 1;
	}
	cts_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CTS_SUCCESS;
}

API int contacts_svc_disconnect(void)
{
	CTS_FN_CALL;
	retvm_if(0 == cts_conn_refcnt, CTS_ERR_ENV_INVALID,
			"Contacts service was not connected");
	CTS_DBG("CTS DB refcnt = %d", cts_conn_refcnt);

	cts_mutex_lock(CTS_MUTEX_CONNECTION);
	if (1 == cts_conn_refcnt)
	{
		cts_socket_final();
		cts_deregister_noti();
		cts_db_close();
		cts_conn_refcnt--;
	}
	else
		cts_conn_refcnt--;
	cts_mutex_unlock(CTS_MUTEX_CONNECTION);

	return CTS_SUCCESS;
}

#if 0
typedef enum {
	CTS_DELETE_ALL_CONTACT_OF_ACCOUNT,
	CTS_DELETE_ALL_GROUP_OF_ACCOUNT,
	CTS_DELETE_ALL_PLOG
};
int contacts_svc_delete_all(int op_code, int index)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};

	switch (op_code)
	{
	case CTS_DELETE_ALL_CONTACT_OF_ACCOUNT:
		snprintf(query, sizeof(query),
				"DELETE FROM %s WHERE account_id=%d"
				CTS_TABLE_CONTACTS, CTS_SCHEMA_NUMBERS, CTS_PLOG_TYPE_MMS_INCOMMING);
		break;
	case CTS_DELETE_ALL_GROUP_OF_ACCOUNT:
	default:
		ERR("Invalid op_code. Your op_code(%d) is not supported.", op_code);
		return CTS_ERR_ARG_INVALID;
	}


}
#endif

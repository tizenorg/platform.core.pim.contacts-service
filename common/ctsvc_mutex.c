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
#include <pthread.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_mutex.h"

typedef struct {
	int (*lock) (pthread_mutex_t *mutex);
	int (*unlock) (pthread_mutex_t *mutex);
} cts_mutex_fns;

static cts_mutex_fns __ctsvc_mutex_funtions = {
	pthread_mutex_lock,
	pthread_mutex_unlock
};

static pthread_mutex_t conn_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t sockfd_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ipc_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ipc_pubsub_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t access_control_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t handle_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t timeout_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t socket_client_info_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t cynara_mutex = PTHREAD_MUTEX_INITIALIZER;

static int __old_type = 0;
static int __defered_ref = 0;

static inline pthread_mutex_t* __ctsvc_mutex_get_mutex(int type)
{
	pthread_mutex_t *ret_val;

	switch (type) {
	case CTS_MUTEX_CONNECTION:
		ret_val = &conn_mutex;
		break;
	case CTS_MUTEX_SOCKET_FD:
		ret_val = &sockfd_mutex;
		break;
	case CTS_MUTEX_PIMS_IPC_CALL:
		ret_val = &ipc_mutex;
		break;
	case CTS_MUTEX_PIMS_IPC_PUBSUB:
		ret_val = &ipc_pubsub_mutex;
		break;
	case CTS_MUTEX_ACCESS_CONTROL:
		ret_val = &access_control_mutex;
		break;
	case CTS_MUTEX_HANDLE:
		ret_val = &handle_mutex;
		break;
	case CTS_MUTEX_TIMEOUT:
		ret_val = &timeout_mutex;
		break;
	case CTS_MUTEX_SOCKET_CLIENT_INFO:
		ret_val = &socket_client_info_mutex;
		break;
	case CTS_MUTEX_CYNARA:
		ret_val = &cynara_mutex;
		break;
	default:
		ERR("unknown type(%d)", type);
		ret_val = NULL;
		break;
	}
	return ret_val;
}

void ctsvc_mutex_lock(int type)
{
	int ret;
	pthread_mutex_t *mutex;

	/*
	 * If user use pthread_cancel, lock can be occured
	 * protect it, call pthread_set_canceltype as PTHREAD_CANCEL_DEFERRED
	 * But, if another module call PTHREAD_CANCEL_ASYNCHRONOUS,
	 * it can be occured again
	 * So, Do not use the pthread_cancel with contacts_service API
	 */
	if (__defered_ref == 0)
		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &__old_type);
	else
		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	__defered_ref++;

	mutex = __ctsvc_mutex_get_mutex(type);

	if (__ctsvc_mutex_funtions.lock) {
		ret = __ctsvc_mutex_funtions.lock(mutex);
		WARN_IF(ret, "mutex_lock Fail(%d)", ret);
	}
}

void ctsvc_mutex_unlock(int type)
{
	int ret;
	pthread_mutex_t *mutex;

	mutex = __ctsvc_mutex_get_mutex(type);

	if (__ctsvc_mutex_funtions.unlock) {
		ret = __ctsvc_mutex_funtions.unlock(mutex);
		WARN_IF(ret, "mutex_unlock Fail(%d)", ret);
	}

	__defered_ref--;
	if (__defered_ref == 0)
		pthread_setcanceltype(__old_type, NULL);
}


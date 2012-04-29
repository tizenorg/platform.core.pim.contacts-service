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
#include <pthread.h>

#include "cts-internal.h"
#include "cts-pthread.h"

typedef struct {
	int (* lock) (pthread_mutex_t *mutex);
	int (* unlock) (pthread_mutex_t *mutex);
}cts_thread_fns;

static cts_thread_fns cts_thread_funtions =
{
	pthread_mutex_lock,
	pthread_mutex_unlock
};

static pthread_mutex_t conn_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t sockfd_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t trans_mutex = PTHREAD_MUTEX_INITIALIZER;


static inline pthread_mutex_t* cts_pthread_get_mutex(int type)
{
	pthread_mutex_t *ret_val;

	switch (type) {
	case CTS_MUTEX_CONNECTION:
		ret_val = &conn_mutex;
		break;
	case CTS_MUTEX_SOCKET_FD:
		ret_val = &sockfd_mutex;
		break;
	case CTS_MUTEX_TRANSACTION:
		ret_val = &trans_mutex;
		break;
	default:
		ERR("unknown type(%d)", type);
		ret_val = NULL;
	}
	return ret_val;
}

void cts_mutex_lock(int type)
{
	int ret;
	pthread_mutex_t *mutex;

	mutex = cts_pthread_get_mutex(type);

	if (cts_thread_funtions.lock) {
		ret = cts_thread_funtions.lock(mutex);
		warn_if(ret, "pthread_mutex_lock Failed(%d)", ret);
	}
}

void cts_mutex_unlock(int type)
{
	int ret;
	pthread_mutex_t *mutex;

	mutex = cts_pthread_get_mutex(type);

	if (cts_thread_funtions.unlock) {
		ret = cts_thread_funtions.unlock(mutex);
		warn_if(ret, "pthread_mutex_unlock Failed(%d)", ret);
	}
}

void contacts_svc_thread_init(void)
{
	cts_thread_funtions.lock = pthread_mutex_lock;
	cts_thread_funtions.unlock = pthread_mutex_unlock;
}

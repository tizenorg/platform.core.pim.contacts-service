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
#ifndef __CTSVC_MUTEX_H__
#define __CTSVC_MUTEX_H__

enum {
	CTS_MUTEX_CONNECTION,
	CTS_MUTEX_UPDTATED_LIST_MEMPOOL,
	CTS_MUTEX_SOCKET_FD,
	CTS_MUTEX_PIMS_IPC_CALL,
	CTS_MUTEX_PIMS_IPC_PUBSUB,
	CTS_MUTEX_ACCESS_CONTROL,
	CTS_MUTEX_HANDLE,
	CTS_MUTEX_TIMEOUT,
	CTS_MUTEX_CYNARA,
	CTS_MUTEX_SOCKET_CLIENT_INFO,
};

void ctsvc_mutex_lock(int type);
void ctsvc_mutex_unlock(int type);


#endif /* __CTSVC_MUTEX_H__ */

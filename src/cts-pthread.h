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
#ifndef __CTS_PTHREAD_H__
#define __CTS_PTHREAD_H__

enum {
	CTS_MUTEX_CONNECTION,
	CTS_MUTEX_UPDTATED_LIST_MEMPOOL,
	CTS_MUTEX_SOCKET_FD,
	CTS_MUTEX_TRANSACTION,
};

void cts_mutex_lock(int type);
void cts_mutex_unlock(int type);


#endif //__CTS_PTHREAD_H__


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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_mutex.h"
#include "ctsvc_socket.h"

API int contacts_sim_import_all_contacts()
{
	int ret;

	ctsvc_mutex_lock(CTS_MUTEX_SOCKET_FD);
	ret = ctsvc_request_sim_import(0);
	ctsvc_mutex_unlock(CTS_MUTEX_SOCKET_FD);

	return ret;
}

API int contacts_sim_get_initialization_status(bool *completed)
{
	int ret;

	RETV_IF(NULL == completed, CONTACTS_ERROR_INVALID_PARAMETER);

	ctsvc_mutex_lock(CTS_MUTEX_SOCKET_FD);
	ret = ctsvc_request_sim_get_initialization_status(0, completed);
	ctsvc_mutex_unlock(CTS_MUTEX_SOCKET_FD);

	return ret;
}

API int contacts_sim_import_all_contacts_by_sim_slot_no(int sim_slot_no)
{
	int ret;

	RETV_IF(sim_slot_no < 0, CONTACTS_ERROR_INVALID_PARAMETER);

	ctsvc_mutex_lock(CTS_MUTEX_SOCKET_FD);
	ret = ctsvc_request_sim_import(sim_slot_no);
	ctsvc_mutex_unlock(CTS_MUTEX_SOCKET_FD);

	return ret;
}

API int contacts_sim_get_initialization_status_by_sim_slot_no(int sim_slot_no, bool *completed)
{
	int ret;

	RETV_IF(NULL == completed, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(sim_slot_no < 0, CONTACTS_ERROR_INVALID_PARAMETER);

	ctsvc_mutex_lock(CTS_MUTEX_SOCKET_FD);
	ret = ctsvc_request_sim_get_initialization_status(sim_slot_no, completed);
	ctsvc_mutex_unlock(CTS_MUTEX_SOCKET_FD);

	return ret;
}


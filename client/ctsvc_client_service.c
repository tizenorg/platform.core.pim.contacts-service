/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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
#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_client_ipc.h"
#include "ctsvc_client_handle.h"
#include "ctsvc_client_service_helper.h"

API int contacts_connect_with_flags(unsigned int flags)
{
	CTS_FN_CALL;
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_current_p(&contact);
	if (CONTACTS_ERROR_NO_DATA == ret) {
		ret = ctsvc_client_handle_create(&contact);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_create() Fail(%d)", ret);
	}
	else if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_client_handle_get_current_p() Fail(%d)", ret);
		return ret;
	}

	ret = ctsvc_client_connect_with_flags(contact, flags);

	return ret;
}

API int contacts_connect(void)
{
	CTS_FN_CALL;
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_current_p(&contact);
	if (CONTACTS_ERROR_NO_DATA == ret) {
		ret = ctsvc_client_handle_create(&contact);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_create() Fail(%d)", ret);
	}
	else if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_client_handle_get_current_p() Fail(%d)", ret);
		return ret;
	}
	ret = ctsvc_client_connect(contact);

	return ret;
}

API int contacts_disconnect(void)
{
	CTS_FN_CALL;
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_current_p(&contact);
	RETV_IF(CONTACTS_ERROR_NO_DATA == ret, CONTACTS_ERROR_NONE);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_current_p() Fail(%d)", ret);

	ret = ctsvc_client_disconnect(contact);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_client_disconnect() Fail(%d)", ret);

	return ret;
}

API int contacts_connect_on_thread(void)
{
	CTS_FN_CALL;
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_current_p(&contact);
	if (CONTACTS_ERROR_NO_DATA == ret) {
		ret = ctsvc_client_handle_create(&contact);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_create() Fail(%d)", ret);
	}
	else if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_client_handle_get_current_p() Fail(%d)", ret);
		return ret;
	}

	ret = ctsvc_client_connect_on_thread(contact);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_client_connect_on_thread() Fail(%d)", ret);

	return ret;
}

API int contacts_disconnect_on_thread(void)
{
	CTS_FN_CALL;
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_current_p(&contact);
	RETV_IF(CONTACTS_ERROR_NO_DATA == ret, CONTACTS_ERROR_NONE);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_current_p() Fail(%d)", ret);

	ret = ctsvc_client_disconnect_on_thread(contact);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_client_disconnect_on_thread() Fail(%d)", ret);

	return ret;
}


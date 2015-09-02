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
#include "ctsvc_inotify.h"
#include "ctsvc_client_handle.h"

API int contacts_db_add_changed_cb(const char* view_uri, contacts_db_changed_cb cb,
		void* user_data)
{
	int ret;
	contacts_h contact = NULL;

	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : view_uri is null");
	RETVM_IF(NULL == cb, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : callback is null");

	ret = ctsvc_client_handle_get_current_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_current_p() Fail(%d)", ret);

	ret = ctsvc_inotify_subscribe(contact, view_uri, cb, user_data);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret,
			"ctsvc_inotify_subscribe(%s) Fail(%d)", view_uri, ret);

	return CONTACTS_ERROR_NONE;
}

API int contacts_db_remove_changed_cb(const char* view_uri, contacts_db_changed_cb cb,
		void* user_data)
{
	int ret;
	contacts_h contact = NULL;

	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : view_uri is null");
	RETVM_IF(NULL == cb, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : callback is null");

	ret = ctsvc_client_handle_get_current_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_current_p() Fail(%d)", ret);

	ret = ctsvc_inotify_unsubscribe(contact, view_uri, cb, user_data);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret,
			"ctsvc_inotify_unsubscribe(%s) Fail(%d)", view_uri, ret);

	return CONTACTS_ERROR_NONE;
}


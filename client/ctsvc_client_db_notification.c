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
#include "ctsvc_client_ipc.h"

static int _ctsvc_db_view_check_read_permission(const char* view_uri)
{
	int ret;
	bool result = false;

	if (STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_ADDRESSBOOK, strlen(CTSVC_VIEW_URI_ADDRESSBOOK))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_PERSON, strlen(CTSVC_VIEW_URI_PERSON))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_CONTACT, strlen(CTSVC_VIEW_URI_CONTACT))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_SIMPLE_CONTACT, strlen(CTSVC_VIEW_URI_SIMPLE_CONTACT))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_GROUP, strlen(CTSVC_VIEW_URI_GROUP))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_MY_PROFILE, strlen(CTSVC_VIEW_URI_MY_PROFILE))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_NAME, strlen(CTSVC_VIEW_URI_NAME))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_NUMBER, strlen(CTSVC_VIEW_URI_NUMBER))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_EMAIL, strlen(CTSVC_VIEW_URI_EMAIL))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_ADDRESS, strlen(CTSVC_VIEW_URI_ADDRESS))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_NOTE, strlen(CTSVC_VIEW_URI_NOTE))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_URL, strlen(CTSVC_VIEW_URI_URL))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_EVENT, strlen(CTSVC_VIEW_URI_EVENT))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_IMAGE, strlen(CTSVC_VIEW_URI_IMAGE))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_COMPANY, strlen(CTSVC_VIEW_URI_COMPANY))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_NICKNAME, strlen(CTSVC_VIEW_URI_NICKNAME))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_MESSENGER, strlen(CTSVC_VIEW_URI_MESSENGER))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_EXTENSION, strlen(CTSVC_VIEW_URI_EXTENSION))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_PROFILE, strlen(CTSVC_VIEW_URI_PROFILE))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_RELATIONSHIP, strlen(CTSVC_VIEW_URI_RELATIONSHIP))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_ACTIVITY, strlen(CTSVC_VIEW_URI_ACTIVITY))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_ACTIVITY_PHOTO, strlen(CTSVC_VIEW_URI_ACTIVITY_PHOTO))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_SPEEDDIAL, strlen(CTSVC_VIEW_URI_SPEEDDIAL))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_SDN, strlen(CTSVC_VIEW_URI_SDN))
			|| STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_GROUP_RELATION, strlen(CTSVC_VIEW_URI_GROUP_RELATION))) {
		ret = ctsvc_ipc_client_check_permission(CTSVC_PERMISSION_CONTACT_READ, &result);
		RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "ctsvc_ipc_client_check_permission() Fail(%d)", ret);
		RETVM_IF(result == false, CONTACTS_ERROR_PERMISSION_DENIED, "Permission denied (contact read)");
	}
	else if (STRING_EQUAL == strncmp(view_uri, CTSVC_VIEW_URI_PHONELOG, strlen(CTSVC_VIEW_URI_PHONELOG))) {
		ret = ctsvc_ipc_client_check_permission(CTSVC_PERMISSION_PHONELOG_READ, &result);
		RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "ctsvc_ipc_client_check_permission() Fail(%d)", ret);
		RETVM_IF(result == false, CONTACTS_ERROR_PERMISSION_DENIED, "Permission denied (phonelog read)");
	}

	return CONTACTS_ERROR_NONE;
}

API int contacts_db_add_changed_cb(const char* view_uri, contacts_db_changed_cb cb,
		void* user_data)
{
	int ret;
	contacts_h contact = NULL;

	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : view_uri is null");
	RETVM_IF(NULL == cb, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : callback is null");

	ret = _ctsvc_db_view_check_read_permission(view_uri);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "_ctsvc_db_view_check_read_permission() Fail(%d)", ret);

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

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

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_inotify_unsubscribe(contact, view_uri, cb, user_data);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret,
			"ctsvc_inotify_unsubscribe(%s) Fail(%d)", view_uri, ret);

	return CONTACTS_ERROR_NONE;
}


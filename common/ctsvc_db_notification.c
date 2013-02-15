/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <glib.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_inotify.h"

API int contacts_db_add_changed_cb( const char* view_uri, contacts_db_changed_cb cb,
		void* user_data )
{
	int ret;

	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invaild parameter : view_uri is null");

	ret = ctsvc_inotify_subscribe(view_uri, cb, user_data);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret,
			"ctsvc_inotify_subscribe(%d) Failed(%d)", view_uri, ret);

	return CONTACTS_ERROR_NONE;
}

API int contacts_db_remove_changed_cb( const char* view_uri, contacts_db_changed_cb cb,
		void* user_data )
{
	int ret;

	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invaild parameter : view_uri is null");

	ret = ctsvc_inotify_unsubscribe(view_uri, cb, user_data);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret,
			"ctsvc_inotify_unsubscribe(%d) Failed(%d)", view_uri, ret);

	return CONTACTS_ERROR_NONE;
}


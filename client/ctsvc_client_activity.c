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
#include "ctsvc_client_handle.h"
#include "ctsvc_client_activity_helper.h"

API int contacts_activity_delete_by_contact_id(int contact_id)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_activity_delete_by_contact_id(contact, contact_id);

	return ret;
}

API int contacts_activity_delete_by_account_id(int account_id)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_activity_delete_by_account_id(contact, account_id);

	return ret;
}


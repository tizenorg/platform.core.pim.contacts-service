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
#include "ctsvc_client_phonelog_helper.h"

API int contacts_phone_log_reset_statistics(void)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		ret = CONTACTS_ERROR_IPC;
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_phone_log_reset_statistics(contact);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		ret = CONTACTS_ERROR_IPC;

	return ret;
}

API int contacts_phone_log_reset_statistics_by_sim(int sim_slot_no)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_phone_log_reset_statistics_by_sim(contact, sim_slot_no);

	return ret;
}


API int contacts_phone_log_delete(contacts_phone_log_delete_e op, ...)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	va_list args;
	va_start(args, op);
	ret = ctsvc_client_phone_log_delete(contact, op, args);
	va_end(args);

	return ret;
}


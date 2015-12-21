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
#include "ctsvc_client_person_helper.h"

API int contacts_person_link_person(int base_person_id, int person_id)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_person_link_person(contact, base_person_id, person_id);

	return ret;
}

API int contacts_person_unlink_contact(int person_id, int contact_id, int *unlinked_person_id)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_person_unlink_contact(contact, person_id, contact_id,
			unlinked_person_id);

	return ret;

}

API int contacts_person_reset_usage(int person_id, contacts_usage_type_e type)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_person_reset_usage(contact, person_id, type);

	return ret;
}

API int contacts_person_set_favorite_order(int person_id, int previous_person_id,
		int next_person_id)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_person_set_favorite_order(contact, person_id, previous_person_id,
			next_person_id);

	return ret;
}

API int contacts_person_set_default_property(contacts_person_property_e property,
		int person_id, int id)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_person_set_default_property(contact, property, person_id, id);

	return ret;
}

API int contacts_person_get_default_property(contacts_person_property_e property,
		int person_id, int *id)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_person_get_default_property(contact, property, person_id, id);

	return ret;
}


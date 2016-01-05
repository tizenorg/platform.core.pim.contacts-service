/*
 * Contacts Service
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#include "ctsvc_internal.h"
#include "ctsvc_handle.h"

int ctsvc_handle_create(contacts_h *contact)
{
	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	ctsvc_base_s *base = calloc(1, sizeof(ctsvc_base_s));
	if (NULL == base) {
		ERR("calloc() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}
	*contact = (contacts_h)base;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_handle_destroy(contacts_h contact)
{
	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	ctsvc_base_s *base = (ctsvc_base_s*)contact;
	free(base);
	base = NULL;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_handle_clone(contacts_h contact, contacts_h *pcontact)
{
	RETV_IF(NULL == contact, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == pcontact, CONTACTS_ERROR_INVALID_PARAMETER);

	ctsvc_base_s *base = (ctsvc_base_s*)contact;
	ctsvc_base_s *clone = calloc(1, sizeof(ctsvc_base_s));
	clone->connection_count = base->connection_count;
	clone->version = base->version;

	*pcontact = (contacts_h)clone;

	return CONTACTS_ERROR_NONE;
}

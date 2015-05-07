/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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
#include "ctsvc_record.h"

static int __ctsvc_addressbook_create(contacts_record_h *out_record);
static int __ctsvc_addressbook_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_addressbook_clone( contacts_record_h record, contacts_record_h* out_record );
static int __ctsvc_addressbook_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_addressbook_get_str(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_addressbook_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_addressbook_set_int(contacts_record_h record, unsigned int property_id, int value);
static int __ctsvc_addressbook_set_str(contacts_record_h record, unsigned int property_id, const char* str);

ctsvc_record_plugin_cb_s addressbook_plugin_cbs = {
	.create = __ctsvc_addressbook_create,
	.destroy = __ctsvc_addressbook_destroy,
	.clone = __ctsvc_addressbook_clone,
	.get_str = __ctsvc_addressbook_get_str,
	.get_str_p =  __ctsvc_addressbook_get_str_p,
	.get_int = __ctsvc_addressbook_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_addressbook_set_str,
	.set_int = __ctsvc_addressbook_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

static int __ctsvc_addressbook_create(contacts_record_h *out_record)
{
	ctsvc_addressbook_s *addressbook;

	addressbook = (ctsvc_addressbook_s*)calloc(1, sizeof(ctsvc_addressbook_s));
	RETVM_IF(NULL == addressbook, CONTACTS_ERROR_OUT_OF_MEMORY,
			"Out of memory : calloc is failed");

	*out_record = (contacts_record_h)addressbook;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_addressbook_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_addressbook_s *addressbook = (ctsvc_addressbook_s*)record;

	addressbook->base.plugin_cbs = NULL;	// help to find double destroy bug (refer to the contacts_record_destroy)
	free(addressbook->base.properties_flags);
	free(addressbook->name);
	free(addressbook);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_addressbook_clone( contacts_record_h record, contacts_record_h* out_record )
{
	ctsvc_addressbook_s *src = (ctsvc_addressbook_s*)record;
	ctsvc_addressbook_s *dest;

	dest = calloc(1, sizeof(ctsvc_addressbook_s));
	RETVM_IF(NULL == dest, CONTACTS_ERROR_OUT_OF_MEMORY,
			"Out of memory : calloc is failed");

	CTSVC_RECORD_COPY_BASE(&(dest->base), &(src->base));

	dest->id = src->id;
	dest->account_id = src->account_id;
	dest->mode = src->mode;
	dest->name = SAFE_STRDUP(src->name);

	*out_record = (contacts_record_h)dest;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_addressbook_get_str_real(contacts_record_h record,
		unsigned int property_id, char** out_str, bool copy )
{
	ctsvc_addressbook_s *addressbook = (ctsvc_addressbook_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_ADDRESSBOOK_NAME:
		*out_str = GET_STR(copy, addressbook->name);
		break;
	default :
		CTS_ERR("This field(%d) is not supported in value(addressbook)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_addressbook_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_addressbook_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_addressbook_get_str(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_addressbook_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_addressbook_set_str(contacts_record_h record,
		unsigned int property_id, const char* str )
{
	ctsvc_addressbook_s *addressbook = (ctsvc_addressbook_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_ADDRESSBOOK_NAME:
		FREEandSTRDUP(addressbook->name, str);
		break;
	default :
		CTS_ERR("This field(%d) is not supported in value(addressbook)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_addressbook_get_int(contacts_record_h record,
		unsigned int property_id, int *out)
{
	ctsvc_addressbook_s *addressbook = (ctsvc_addressbook_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_ADDRESSBOOK_ID:
		*out = addressbook->id;
		break;
	case CTSVC_PROPERTY_ADDRESSBOOK_ACCOUNT_ID:
		*out = addressbook->account_id;
		break;
	case CTSVC_PROPERTY_ADDRESSBOOK_MODE:
		*out = addressbook->mode;
		break;
	default:
		CTS_ERR("Invalid parameter : property_id(%d) is not supported in value(addressbook)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_addressbook_set_int(contacts_record_h record,
		unsigned int property_id, int value)
{
	ctsvc_addressbook_s *addressbook = (ctsvc_addressbook_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_ADDRESSBOOK_ID:
		addressbook->id = value;
		break;
	case CTSVC_PROPERTY_ADDRESSBOOK_MODE:
		RETVM_IF(value != CONTACTS_ADDRESS_BOOK_MODE_NONE
						&& value != CONTACTS_ADDRESS_BOOK_MODE_READONLY,
				CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : address_book mode is %d", value);
		addressbook->mode = value;
		break;
	case CTSVC_PROPERTY_ADDRESSBOOK_ACCOUNT_ID:
		RETVM_IF(addressbook->id > 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : property_id(%d) is a read-only value (addressbook)", property_id);
		addressbook->account_id = value;
		break;
	default:
		CTS_ERR("Invalid parameter : property_id(%d) is not supported in value(addressbook)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}


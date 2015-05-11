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
#include "ctsvc_view.h"

static int __ctsvc_sdn_create(contacts_record_h* out_record);
static int __ctsvc_sdn_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_sdn_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_sdn_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_sdn_get_str(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_sdn_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_sdn_set_int(contacts_record_h record, unsigned int property_id, int value);
static int __ctsvc_sdn_set_str(contacts_record_h record, unsigned int property_id, const char* str);

ctsvc_record_plugin_cb_s sdn_plugin_cbs = {
	.create = __ctsvc_sdn_create,
	.destroy = __ctsvc_sdn_destroy,
	.clone = __ctsvc_sdn_clone,
	.get_str = __ctsvc_sdn_get_str,
	.get_str_p = __ctsvc_sdn_get_str_p,
	.get_int = __ctsvc_sdn_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_sdn_set_str,
	.set_int = __ctsvc_sdn_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

static int __ctsvc_sdn_create(contacts_record_h* out_record)
{
	ctsvc_sdn_s *sdn;
	sdn = (ctsvc_sdn_s*)calloc(1, sizeof(ctsvc_sdn_s));
	RETVM_IF(NULL == sdn, CONTACTS_ERROR_OUT_OF_MEMORY,
			"Out of memory calloc is failed");

	*out_record = (contacts_record_h)sdn;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_sdn_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_sdn_s* sdn = (ctsvc_sdn_s*)record;
	sdn->base.plugin_cbs = NULL;	// help to find double destroy bug (refer to the contacts_record_destroy)
	free(sdn->base.properties_flags);

	free(sdn->name);
	free(sdn->number);
	free(sdn);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_sdn_clone(contacts_record_h record, contacts_record_h *out_record)
{
    ctsvc_sdn_s *out_data = NULL;
    ctsvc_sdn_s *src_data = NULL;

    src_data = (ctsvc_sdn_s*)record;
    out_data = calloc(1, sizeof(ctsvc_sdn_s));
    RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY,
			 "Out of memeory : calloc(ctsvc_sdn_s) Failed(%d)", CONTACTS_ERROR_OUT_OF_MEMORY);

	out_data->id = src_data->id;
	out_data->name = SAFE_STRDUP(src_data->name);
	out_data->number = SAFE_STRDUP(src_data->number);
	out_data->sim_slot_no = src_data->sim_slot_no;

	CTSVC_RECORD_COPY_BASE(&(out_data->base), &(src_data->base));

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_sdn_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_sdn_s* sdn = (ctsvc_sdn_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_SDN_ID:
		*out = sdn->id;
		break;
	case CTSVC_PROPERTY_SDN_SIM_SLOT_NO:
		*out = sdn->sim_slot_no;
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(sdn)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_sdn_get_str_real(contacts_record_h record, unsigned int property_id, char** out_str, bool copy)
{
	ctsvc_sdn_s* sdn = (ctsvc_sdn_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_SDN_NAME:
		*out_str = GET_STR(copy, sdn->name);
		break;
	case CTSVC_PROPERTY_SDN_NUMBER:
		*out_str = GET_STR(copy, sdn->number);
		break;
	default :
		CTS_ERR("This field(%d) is not supported in value(sdn)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_sdn_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_sdn_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_sdn_get_str(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_sdn_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_sdn_set_int(contacts_record_h record, unsigned int property_id, int value)
{
	ctsvc_sdn_s* sdn = (ctsvc_sdn_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_SDN_ID:
		sdn->id = value;
		break;
	case CTSVC_PROPERTY_SDN_SIM_SLOT_NO:
		sdn->sim_slot_no = value;
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(sdn)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_sdn_set_str(contacts_record_h record, unsigned int property_id, const char* str)
{
	ctsvc_sdn_s* sdn = (ctsvc_sdn_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_SDN_NAME:
		FREEandSTRDUP(sdn->name, str);
		break;
	case CTSVC_PROPERTY_SDN_NUMBER:
		FREEandSTRDUP(sdn->number, str);
		break;
	default :
		CTS_ERR("This field(%d) is not supported in value(sdn)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}


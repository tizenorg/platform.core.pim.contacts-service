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

static int __ctsvc_phonelog_create(contacts_record_h *out_record);
static int __ctsvc_phonelog_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_phonelog_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_phonelog_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_phonelog_get_str(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_phonelog_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_phonelog_set_int(contacts_record_h record, unsigned int property_id, int value);
static int __ctsvc_phonelog_set_str(contacts_record_h record, unsigned int property_id, const char* str);

ctsvc_record_plugin_cb_s phonelog_plugin_cbs = {
	.create = __ctsvc_phonelog_create,
	.destroy = __ctsvc_phonelog_destroy,
	.clone = __ctsvc_phonelog_clone,
	.get_str = __ctsvc_phonelog_get_str,
	.get_str_p = __ctsvc_phonelog_get_str_p,
	.get_int = __ctsvc_phonelog_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_phonelog_set_str,
	.set_int = __ctsvc_phonelog_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

static int __ctsvc_phonelog_create(contacts_record_h *out_record)
{
	ctsvc_phonelog_s *phonelog;

	phonelog = (ctsvc_phonelog_s*)calloc(1, sizeof(ctsvc_phonelog_s));
	RETVM_IF(NULL == phonelog, CONTACTS_ERROR_OUT_OF_MEMORY,
			"Out of memory : calloc is Fail");

	*out_record = (contacts_record_h)phonelog;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_phonelog_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_phonelog_s* phonelog = (ctsvc_phonelog_s*)record;
	phonelog->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(phonelog->base.properties_flags);

	free(phonelog->address);
	free(phonelog->extra_data2);
	free(phonelog);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_phonelog_clone(contacts_record_h record, contacts_record_h *out_record)
{
    ctsvc_phonelog_s *out_data = NULL;
    ctsvc_phonelog_s *src_data = NULL;

    src_data = (ctsvc_phonelog_s*)record;
    out_data = calloc(1, sizeof(ctsvc_phonelog_s));
    RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY,
			 "Out of memeory : calloc(ctsvc_phonelog_s) Fail(%d)", CONTACTS_ERROR_OUT_OF_MEMORY);

	out_data->id = src_data->id;
	out_data->address = SAFE_STRDUP(src_data->address);
	out_data->person_id = src_data->person_id;
	out_data->log_time = src_data->log_time;
	out_data->log_type = src_data->log_type;
	out_data->extra_data1 = src_data->extra_data1;
	out_data->extra_data2 = SAFE_STRDUP(src_data->extra_data2);
	out_data->sim_slot_no = src_data->sim_slot_no;

	CTSVC_RECORD_COPY_BASE(&(out_data->base), &(src_data->base));

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_phonelog_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_phonelog_s* phonelog = (ctsvc_phonelog_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_PHONELOG_ID:
		*out = phonelog->id;
		break;
	case CTSVC_PROPERTY_PHONELOG_PERSON_ID:
		*out = phonelog->person_id;
		break;
	case CTSVC_PROPERTY_PHONELOG_LOG_TIME:
		*out = phonelog->log_time;
		break;
	case CTSVC_PROPERTY_PHONELOG_LOG_TYPE:
		*out = phonelog->log_type;
		break;
	case CTSVC_PROPERTY_PHONELOG_EXTRA_DATA1:
		*out = phonelog->extra_data1;
		break;
	case CTSVC_PROPERTY_PHONELOG_SIM_SLOT_NO:
		*out = phonelog->sim_slot_no;
		break;
	default:
		CTS_ERR("Invalid parameter : property_id(%d) is not supported in value(phonelog)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_phonelog_get_str_real(contacts_record_h record, unsigned int property_id, char** out_str, bool copy)
{
	ctsvc_phonelog_s* phonelog = (ctsvc_phonelog_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_PHONELOG_ADDRESS:
		*out_str = GET_STR(copy, phonelog->address);
		break;
	case CTSVC_PROPERTY_PHONELOG_EXTRA_DATA2:
		*out_str = GET_STR(copy, phonelog->extra_data2);
		break;
	default :
		CTS_ERR("Invalid parameter : property_id(%d) is not supported in value(phonelog)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_phonelog_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_phonelog_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_phonelog_get_str(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_phonelog_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_phonelog_set_int(contacts_record_h record, unsigned int property_id, int value)
{
	ctsvc_phonelog_s* phonelog = (ctsvc_phonelog_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_PHONELOG_ID:
		phonelog->id = value;
		break;
	case CTSVC_PROPERTY_PHONELOG_PERSON_ID:
		RETVM_IF(0 < phonelog->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : property_id(%d) is a read-only value (phonelog)", property_id);
		phonelog->person_id = value;
		break;
	case CTSVC_PROPERTY_PHONELOG_LOG_TIME:
		RETVM_IF(0 < phonelog->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : property_id(%d) is a read-only value (phonelog)", property_id);
		phonelog->log_time = value;
		break;
	case CTSVC_PROPERTY_PHONELOG_LOG_TYPE:
		if ((CONTACTS_PLOG_TYPE_NONE <= value
					&& value <= CONTACTS_PLOG_TYPE_VIDEO_BLOCKED)
				|| (CONTACTS_PLOG_TYPE_MMS_INCOMMING <= value
					&& value <= CONTACTS_PLOG_TYPE_MMS_BLOCKED)
				|| (CONTACTS_PLOG_TYPE_EMAIL_RECEIVED <= value
					&& value <= CONTACTS_PLOG_TYPE_EMAIL_SENT)
			)
			phonelog->log_type = value;
		else {
			CTS_ERR("Invalid parameter : log type is in invalid range (%d)", value);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		break;
	case CTSVC_PROPERTY_PHONELOG_EXTRA_DATA1:
		RETVM_IF(0 < phonelog->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : property_id(%d) is a read-only value (phonelog)", property_id);
		phonelog->extra_data1 = value;
		break;
	case CTSVC_PROPERTY_PHONELOG_SIM_SLOT_NO:
		RETVM_IF(0 < phonelog->id, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property_id(%d) is a read-only value (phonelog)", property_id);
		phonelog->sim_slot_no = value;
		break;
	default:
		CTS_ERR("Invalid parameter : property_id(%d) is not supported in value(phonelog)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_phonelog_set_str(contacts_record_h record, unsigned int property_id, const char* str)
{
	ctsvc_phonelog_s* phonelog = (ctsvc_phonelog_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_PHONELOG_ADDRESS:
		RETVM_IF(0 < phonelog->id, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property_id(%d) is a read-only value (phonelog)", property_id);
		FREEandSTRDUP(phonelog->address, str);
		break;
	case CTSVC_PROPERTY_PHONELOG_EXTRA_DATA2:
		FREEandSTRDUP(phonelog->extra_data2, str);
		break;
	default :
		CTS_ERR("Invalid parameter : property_id(%d) is not supported in value(phonelog)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}


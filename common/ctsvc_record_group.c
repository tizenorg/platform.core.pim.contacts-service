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

#include <glib.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_record.h"
#include "ctsvc_view.h"

static int __ctsvc_group_create(contacts_record_h *out_record);
static int __ctsvc_group_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_group_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_group_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_group_get_str(contacts_record_h record, unsigned int property_id, char** out_str );
static int __ctsvc_group_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str );
static int __ctsvc_group_get_bool( contacts_record_h record, unsigned int property_id, bool *value );
static int __ctsvc_group_set_int(contacts_record_h record, unsigned int property_id, int value);
static int __ctsvc_group_set_str(contacts_record_h record, unsigned int property_id, const char* str );
static int __ctsvc_group_set_bool(contacts_record_h record, unsigned int property_id, bool value);


ctsvc_record_plugin_cb_s group_plugin_cbs = {
	.create = __ctsvc_group_create,
	.destroy = __ctsvc_group_destroy,
	.clone = __ctsvc_group_clone,
	.get_str = __ctsvc_group_get_str,
	.get_str_p = __ctsvc_group_get_str_p,
	.get_int = __ctsvc_group_get_int,
	.get_bool = __ctsvc_group_get_bool,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_group_set_str,
	.set_int = __ctsvc_group_set_int,
	.set_bool = __ctsvc_group_set_bool,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

static int __ctsvc_group_create(contacts_record_h *out_record)
{
	ctsvc_group_s *group;

	group = (ctsvc_group_s*)calloc(1, sizeof(ctsvc_group_s));
	RETVM_IF(NULL == group, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc is failed");

	*out_record = (contacts_record_h)group;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_group_s *group = (ctsvc_group_s*)record;
	group->base.plugin_cbs = NULL;	// help to find double destroy bug (refer to the contacts_record_destroy)
	free(group->base.properties_flags);

	free(group->name);
	free(group->ringtone_path);
	free(group->vibration);
	free(group->image_thumbnail_path);
	free(group->extra_data);
	free(group);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_clone(contacts_record_h record, contacts_record_h *out_record)
{
    ctsvc_group_s *out_data = NULL;
    ctsvc_group_s *src_data = NULL;

    src_data = (ctsvc_group_s*)record;
    out_data = calloc(1, sizeof(ctsvc_group_s));
    RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY,
			 "Out of memeory : calloc(ctsvc_group_s) Failed(%d)", CONTACTS_ERROR_OUT_OF_MEMORY);

	out_data->id = src_data->id;
	out_data->addressbook_id = src_data->addressbook_id;
	out_data->is_read_only = src_data->is_read_only;
	out_data->image_thumbnail_changed = src_data->image_thumbnail_changed;
	out_data->name = SAFE_STRDUP(src_data->name);
	out_data->extra_data = SAFE_STRDUP(src_data->extra_data);
	out_data->vibration = SAFE_STRDUP(src_data->vibration);
	out_data->ringtone_path = SAFE_STRDUP(src_data->ringtone_path);
	out_data->image_thumbnail_path = SAFE_STRDUP(src_data->image_thumbnail_path);

	CTSVC_RECORD_COPY_BASE(&(out_data->base), &(src_data->base));

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_group_s *group = (ctsvc_group_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_GROUP_ID:
		*out = group->id;
		break;
	case CTSVC_PROPERTY_GROUP_ADDRESSBOOK_ID:
		*out = group->addressbook_id;
		break;
	default:
		ASSERT_NOT_REACHED("Invalid parameter : property_id(%d) is not supported in value(group)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_get_str_real(contacts_record_h record, unsigned int property_id, char** out_str, bool copy )
{
	ctsvc_group_s *group = (ctsvc_group_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_GROUP_NAME:
		*out_str = GET_STR(copy, group->name);
		break;
	case CTSVC_PROPERTY_GROUP_RINGTONE:
		*out_str = GET_STR(copy, group->ringtone_path);
		break;
	case CTSVC_PROPERTY_GROUP_IMAGE:
		*out_str = GET_STR(copy, group->image_thumbnail_path);
		break;
	case CTSVC_PROPERTY_GROUP_VIBRATION:
		*out_str = GET_STR(copy, group->vibration);
		break;
	case CTSVC_PROPERTY_GROUP_EXTRA_DATA:
		*out_str = GET_STR(copy, group->extra_data);
		break;
	default :
		ASSERT_NOT_REACHED("Invalid parameter : property_id(%d) is not supported in value(group)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_group_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_group_get_str(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_group_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_group_set_int(contacts_record_h record, unsigned int property_id, int value)
{
	ctsvc_group_s *group = (ctsvc_group_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_GROUP_ID:
		group->id = value;
		break;
/*
		ASSERT_NOT_REACHED("Invalid parameter : property_id(%d) is a read-only value (group)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
*/
	case CTSVC_PROPERTY_GROUP_ADDRESSBOOK_ID:
		RETVM_IF(group->id > 0, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : property_id(%d) is a read-only value (group)", property_id);
		group->addressbook_id = value;
		break;
	default:
		ASSERT_NOT_REACHED("Invalid parameter : property_id(%d) is not supported in value(group)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_set_str(contacts_record_h record, unsigned int property_id, const char* str )
{
	ctsvc_group_s *group = (ctsvc_group_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_GROUP_NAME:
		FREEandSTRDUP(group->name, str);
		break;
	case CTSVC_PROPERTY_GROUP_RINGTONE:
		FREEandSTRDUP(group->ringtone_path, str);
		break;
	case CTSVC_PROPERTY_GROUP_IMAGE:
		FREEandSTRDUP(group->image_thumbnail_path, str);
		group->image_thumbnail_changed = true;
		break;
	case CTSVC_PROPERTY_GROUP_VIBRATION:
		FREEandSTRDUP(group->vibration, str);
		break;
	case CTSVC_PROPERTY_GROUP_EXTRA_DATA:
		FREEandSTRDUP(group->extra_data, str);
		break;
	default :
		ASSERT_NOT_REACHED("Invalid parameter : property_id(%d) is not supported in value(group)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_get_bool(contacts_record_h record, unsigned int property_id, bool *value )
{
	ctsvc_group_s *group = (ctsvc_group_s*)record;
	switch (property_id) {
	case CTSVC_PROPERTY_GROUP_IS_READ_ONLY:
		*value = group->is_read_only;
		break;
	default:
		ASSERT_NOT_REACHED("Invalid parameter : property_id(%d) is not supported in value(company)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_set_bool(contacts_record_h record, unsigned int property_id, bool value)
{
	ctsvc_group_s *group = (ctsvc_group_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_GROUP_IS_READ_ONLY:
		group->is_read_only = value;
		break;
	default:
		ASSERT_NOT_REACHED("Invalid parameter : property_id(%d) is not supported in value(group)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}


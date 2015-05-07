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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_record.h"
#include "ctsvc_view.h"

static int __ctsvc_updated_info_create(contacts_record_h* out_record);
static int __ctsvc_updated_info_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_updated_info_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_updated_info_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_updated_info_set_int(contacts_record_h record, unsigned int property_id, int value);
static int __ctsvc_updated_info_get_bool(contacts_record_h record, unsigned int property_id, bool *out);
static int __ctsvc_updated_info_set_bool(contacts_record_h record, unsigned int property_id, bool value);

ctsvc_record_plugin_cb_s updated_info_plugin_cbs = {
	.create = __ctsvc_updated_info_create,
	.destroy = __ctsvc_updated_info_destroy,
	.clone = __ctsvc_updated_info_clone,
	.get_str = NULL,
	.get_str_p = NULL,
	.get_int = __ctsvc_updated_info_get_int,
	.get_bool = __ctsvc_updated_info_get_bool,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = NULL,
	.set_int = __ctsvc_updated_info_set_int,
	.set_bool = __ctsvc_updated_info_set_bool,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

static int __ctsvc_updated_info_create(contacts_record_h* out_record)
{
	ctsvc_updated_info_s *updated_info;
	updated_info = (ctsvc_updated_info_s*)calloc(1, sizeof(ctsvc_updated_info_s));
	RETVM_IF(NULL == updated_info, CONTACTS_ERROR_OUT_OF_MEMORY,
			"Out of memory calloc is failed");

	*out_record = (contacts_record_h)updated_info;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_updated_info_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_updated_info_s* updated_info = (ctsvc_updated_info_s*)record;
	updated_info->base.plugin_cbs = NULL;	// help to find double destroy bug (refer to the contacts_record_destroy)
	free(updated_info->base.properties_flags);
	free(updated_info);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_updated_info_clone(contacts_record_h record, contacts_record_h *out_record)
{
    ctsvc_updated_info_s *out_data = NULL;
    ctsvc_updated_info_s *src_data = NULL;

    src_data = (ctsvc_updated_info_s*)record;
    out_data = calloc(1, sizeof(ctsvc_updated_info_s));
    RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY,
			 "Out of memeory : calloc(ctsvc_updated_info_s) Failed(%d)", CONTACTS_ERROR_OUT_OF_MEMORY);

	out_data->id = src_data->id;
	out_data->changed_type = src_data->changed_type;
	out_data->changed_ver = src_data->changed_ver;
	out_data->addressbook_id = src_data->addressbook_id;

	CTSVC_RECORD_COPY_BASE(&(out_data->base), &(src_data->base));

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_updated_info_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_updated_info_s* updated_info = (ctsvc_updated_info_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_UPDATE_INFO_ID :
		*out = updated_info->id;
		break;
	case CTSVC_PROPERTY_UPDATE_INFO_ADDRESSBOOK_ID:
		*out = updated_info->addressbook_id;
		break;
	case CTSVC_PROPERTY_UPDATE_INFO_TYPE:
		*out = updated_info->changed_type;
		break;
	case CTSVC_PROPERTY_UPDATE_INFO_VERSION:
		*out = updated_info->changed_ver;
		break;
	case CTSVC_PROPERTY_UPDATE_INFO_LAST_CHANGED_TYPE:
		*out = updated_info->last_changed_type;
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(updated_info)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_updated_info_set_int(contacts_record_h record, unsigned int property_id, int value)
{
	ctsvc_updated_info_s* updated_info = (ctsvc_updated_info_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_UPDATE_INFO_ID :
		updated_info->id = value;
		break;
	case CTSVC_PROPERTY_UPDATE_INFO_ADDRESSBOOK_ID:
		updated_info->addressbook_id = value;
		break;
	case CTSVC_PROPERTY_UPDATE_INFO_TYPE:
		RETVM_IF(value < CONTACTS_CHANGE_INSERTED
						|| value > CONTACTS_CHANGE_DELETED,
				CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : update info type is in invalid range (%d)", value);
		updated_info->changed_type = value;
		break;
	case CTSVC_PROPERTY_UPDATE_INFO_VERSION:
		updated_info->changed_ver = value;
		break;
	case CTSVC_PROPERTY_UPDATE_INFO_LAST_CHANGED_TYPE:
		updated_info->last_changed_type = value;
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(updated_info)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_updated_info_get_bool(contacts_record_h record, unsigned int property_id, bool *out)
{
	ctsvc_updated_info_s* updated_info = (ctsvc_updated_info_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_UPDATE_INFO_IMAGE_CHANGED :
		*out = updated_info->image_changed;
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(updated_info)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_updated_info_set_bool(contacts_record_h record, unsigned int property_id, bool value)
{
	ctsvc_updated_info_s* updated_info = (ctsvc_updated_info_s*)record;

	switch(property_id) {
	case CTSVC_PROPERTY_UPDATE_INFO_IMAGE_CHANGED :
		updated_info->image_changed = value;
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(updated_info)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}


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
#include "ctsvc_record.h"
#include "ctsvc_view.h"

static int __ctsvc_speeddial_create(contacts_record_h *out_record);
static int __ctsvc_speeddial_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_speeddial_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_speeddial_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_speeddial_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_speeddial_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_speeddial_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_speeddial_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);


ctsvc_record_plugin_cb_s speeddial_plugin_cbs = {
	.create = __ctsvc_speeddial_create,
	.destroy = __ctsvc_speeddial_destroy,
	.clone = __ctsvc_speeddial_clone,
	.get_str = __ctsvc_speeddial_get_str,
	.get_str_p = __ctsvc_speeddial_get_str_p,
	.get_int = __ctsvc_speeddial_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_speeddial_set_str,
	.set_int = __ctsvc_speeddial_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

static int __ctsvc_speeddial_create(contacts_record_h *out_record)
{
	ctsvc_speeddial_s *speeddial;
	speeddial = calloc(1, sizeof(ctsvc_speeddial_s));
	RETVM_IF(NULL == speeddial, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	*out_record = (contacts_record_h)speeddial;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_speeddial_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_speeddial_s *speeddial = (ctsvc_speeddial_s*)record;
	speeddial->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(speeddial->base.properties_flags);

	free(speeddial->display_name);
	free(speeddial->image_thumbnail_path);
	free(speeddial->label);
	free(speeddial->number);
	free(speeddial);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_speeddial_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_speeddial_s *out_data = NULL;
	ctsvc_speeddial_s *src_data = NULL;

	src_data = (ctsvc_speeddial_s*)record;
	out_data = calloc(1, sizeof(ctsvc_speeddial_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY,
			"Out of memeory : calloc(ctsvc_speeddial_s) Fail(%d)", CONTACTS_ERROR_OUT_OF_MEMORY);

	out_data->id = src_data->id;
	out_data->dial_number = src_data->dial_number;
	out_data->number_id = src_data->number_id;
	out_data->person_id = src_data->person_id;
	out_data->number_type = src_data->number_type;
	out_data->display_name = SAFE_STRDUP(src_data->display_name);
	out_data->image_thumbnail_path = SAFE_STRDUP(src_data->image_thumbnail_path);
	out_data->label = SAFE_STRDUP(src_data->label);
	out_data->number = SAFE_STRDUP(src_data->number);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_speeddial_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_speeddial_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_speeddial_s *speeddial = (ctsvc_speeddial_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_SPEEDDIAL_DIAL_NUMBER:
		*out = speeddial->dial_number;
		break;
	case CTSVC_PROPERTY_SPEEDDIAL_NUMBER_ID:
		*out = speeddial->number_id;
		break;
	case CTSVC_PROPERTY_SPEEDDIAL_NUMBER_TYPE:
		*out = speeddial->number_type;
		break;
	case CTSVC_PROPERTY_SPEEDDIAL_PERSON_ID:
		*out = speeddial->person_id;
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(speeddial)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_speeddial_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_speeddial_s *speeddial = (ctsvc_speeddial_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_SPEEDDIAL_DISPLAY_NAME:
		*out_str = GET_STR(copy, speeddial->display_name);
		break;
	case CTSVC_PROPERTY_SPEEDDIAL_NUMBER:
		*out_str = GET_STR(copy, speeddial->number);
		break;
	case CTSVC_PROPERTY_SPEEDDIAL_IMAGE_THUMBNAIL:
		*out_str = GET_STR(copy, speeddial->image_thumbnail_path);
		break;
	case CTSVC_PROPERTY_SPEEDDIAL_NUMBER_LABEL:
		*out_str = GET_STR(copy, speeddial->label);
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(speeddial)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_speeddial_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_speeddial_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_speeddial_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_speeddial_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_speeddial_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_speeddial_s *speeddial = (ctsvc_speeddial_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_SPEEDDIAL_NUMBER_TYPE:
		CHECK_DIRTY_VAL(speeddial->number_type, value, is_dirty);
		speeddial->number_type = value;
		break;
	case CTSVC_PROPERTY_SPEEDDIAL_PERSON_ID:
		CHECK_DIRTY_VAL(speeddial->person_id, value, is_dirty);
		speeddial->person_id = value;
		break;
	case CTSVC_PROPERTY_SPEEDDIAL_DIAL_NUMBER:
		RETVM_IF(0 < speeddial->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : property_id(%d) is a read-only value (speeddial)", property_id);
		CHECK_DIRTY_VAL(speeddial->dial_number, value, is_dirty);
		speeddial->dial_number = value;
		break;
	case CTSVC_PROPERTY_SPEEDDIAL_NUMBER_ID:
		CHECK_DIRTY_VAL(speeddial->number_id, value, is_dirty);
		speeddial->number_id = value;
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(speeddial)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_speeddial_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_speeddial_s *speeddial = (ctsvc_speeddial_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_SPEEDDIAL_DISPLAY_NAME:
		CHECK_DIRTY_STR(speeddial->display_name, str, is_dirty);
		FREEandSTRDUP(speeddial->display_name, str);
		break;
	case CTSVC_PROPERTY_SPEEDDIAL_NUMBER:
		CHECK_DIRTY_STR(speeddial->number, str, is_dirty);
		FREEandSTRDUP(speeddial->number, str);
		break;
	case CTSVC_PROPERTY_SPEEDDIAL_IMAGE_THUMBNAIL:
		CHECK_DIRTY_STR(speeddial->image_thumbnail_path, str, is_dirty);
		FREEandSTRDUP(speeddial->image_thumbnail_path, str);
		break;
	case CTSVC_PROPERTY_SPEEDDIAL_NUMBER_LABEL:
		CHECK_DIRTY_STR(speeddial->label, str, is_dirty);
		FREEandSTRDUP(speeddial->label, str);
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(speeddial)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

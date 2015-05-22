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

static int __ctsvc_person_create(contacts_record_h* out_record);
static int __ctsvc_person_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_person_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_person_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_person_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_person_get_str(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_person_get_bool(contacts_record_h record, unsigned int property_id, bool *value);
static int __ctsvc_person_set_int(contacts_record_h record, unsigned int property_id, int value);
static int __ctsvc_person_set_str(contacts_record_h record, unsigned int property_id, const char* str);
static int __ctsvc_person_set_bool(contacts_record_h record, unsigned int property_id, bool value);


ctsvc_record_plugin_cb_s person_plugin_cbs = {
	.create = __ctsvc_person_create,
	.destroy = __ctsvc_person_destroy,
	.clone = __ctsvc_person_clone,
	.get_str = __ctsvc_person_get_str,
	.get_str_p = __ctsvc_person_get_str_p,
	.get_int = __ctsvc_person_get_int,
	.get_bool = __ctsvc_person_get_bool,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_person_set_str,
	.set_int = __ctsvc_person_set_int,
	.set_bool = __ctsvc_person_set_bool,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

static int __ctsvc_person_create(contacts_record_h* out_record)
{
	ctsvc_person_s *person;
	person = (ctsvc_person_s*)calloc(1, sizeof(ctsvc_person_s));
	RETVM_IF(NULL == person, CONTACTS_ERROR_OUT_OF_MEMORY,
			"Out of memory : calloc is Fail");

	*out_record = (contacts_record_h)person;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_person_s *person = (ctsvc_person_s*)record;
	person->base.plugin_cbs = NULL; /* help to find double-destroy bug (refer to the contacts_record_destroy) */
	free(person->base.properties_flags);

	free(person->display_name);
	free(person->display_name_index);
	free(person->ringtone_path);
	free(person->vibration);
	free(person->message_alert);
	free(person->image_thumbnail_path);
	free(person->status);
	free(person->addressbook_ids);
	free(person);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_clone(contacts_record_h record, contacts_record_h *out_record)
{
    ctsvc_person_s *out_data = NULL;
    ctsvc_person_s *src_data = NULL;

    src_data = (ctsvc_person_s*)record;
    out_data = calloc(1, sizeof(ctsvc_person_s));
    RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY,
			 "Out of memeory : calloc(ctsvc_person_s) Fail(%d)", CONTACTS_ERROR_OUT_OF_MEMORY);

	out_data->is_favorite = src_data->is_favorite;
	out_data->has_phonenumber = src_data->has_phonenumber;
	out_data->has_email = src_data->has_email;
	out_data->person_id = src_data->person_id;
	out_data->name_contact_id = src_data->name_contact_id;
	out_data->link_count = src_data->link_count;
	out_data->addressbook_ids = SAFE_STRDUP(src_data->addressbook_ids);
	out_data->display_name = SAFE_STRDUP(src_data->display_name);
	out_data->display_name_index = SAFE_STRDUP(src_data->display_name_index);
	out_data->image_thumbnail_path = SAFE_STRDUP(src_data->image_thumbnail_path);
	out_data->ringtone_path = SAFE_STRDUP(src_data->ringtone_path);
	out_data->vibration = SAFE_STRDUP(src_data->vibration);
	out_data->message_alert = SAFE_STRDUP(src_data->message_alert);
	out_data->status = SAFE_STRDUP(src_data->status);

	CTSVC_RECORD_COPY_BASE(&(out_data->base), &(src_data->base));

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_person_s *person = (ctsvc_person_s *)record;
	switch(property_id) {
	case CTSVC_PROPERTY_PERSON_ID:
		*out = person->person_id;
		break;
	case CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID:
		*out = person->name_contact_id;
		break;
	case CTSVC_PROPERTY_PERSON_LINK_COUNT:
		*out = person->link_count;
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(person)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_get_str_real(contacts_record_h record, unsigned int property_id, char** out_str, bool copy)
{
	ctsvc_person_s *person = (ctsvc_person_s *)record;
	switch(property_id) {
	case CTSVC_PROPERTY_PERSON_DISPLAY_NAME:
		*out_str = GET_STR(copy, person->display_name);
		break;
	case CTSVC_PROPERTY_PERSON_RINGTONE:
		*out_str = GET_STR(copy, person->ringtone_path);
		break;
	case CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL:
		*out_str = GET_STR(copy, person->image_thumbnail_path);
		break;
	case CTSVC_PROPERTY_PERSON_VIBRATION:
		*out_str = GET_STR(copy, person->vibration);
		break;
	case CTSVC_PROPERTY_PERSON_MESSAGE_ALERT:
		*out_str = GET_STR(copy, person->message_alert);
		break;
	case CTSVC_PROPERTY_PERSON_STATUS:
		*out_str = GET_STR(copy, person->status);
		break;
	case CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX:
		*out_str = GET_STR(copy, person->display_name_index);
		break;
	case CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS:
		*out_str = GET_STR(copy, person->addressbook_ids);
		break;
	default :
		CTS_ERR("This field(%d) is not supported in value(person)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_person_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_person_get_str(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_person_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_person_get_bool(contacts_record_h record, unsigned int property_id, bool *value)
{
	ctsvc_person_s *person = (ctsvc_person_s *)record;
	switch (property_id) {
	case CTSVC_PROPERTY_PERSON_IS_FAVORITE:
		*value = person->is_favorite;
		break;
	case CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER:
		*value = person->has_phonenumber;
		break;
	case CTSVC_PROPERTY_PERSON_HAS_EMAIL:
		*value = person->has_email;
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(company)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_set_int(contacts_record_h record, unsigned int property_id, int value)
{
	ctsvc_person_s *person = (ctsvc_person_s *)record;

	switch(property_id) {
	case CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID:
		person->name_contact_id = value;
		break;
	case CTSVC_PROPERTY_PERSON_ID:
		person->person_id = value;
		break;
	case CTSVC_PROPERTY_PERSON_LINK_COUNT:
		person->link_count = value;
		break;
	default:
		CTS_ERR("This field(0x%0x) is not supported in value(person)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_set_str(contacts_record_h record, unsigned int property_id, const char* str)
{
	ctsvc_person_s *person = (ctsvc_person_s *)record;

	switch(property_id) {
	case CTSVC_PROPERTY_PERSON_DISPLAY_NAME:
		FREEandSTRDUP(person->display_name, str);
		break;
	case CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX:
		FREEandSTRDUP(person->display_name_index, str);
		break;
	case CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL:
		FREEandSTRDUP(person->image_thumbnail_path, str);
		break;
	case CTSVC_PROPERTY_PERSON_RINGTONE:
		FREEandSTRDUP(person->ringtone_path, str);
		break;
	case CTSVC_PROPERTY_PERSON_VIBRATION:
		FREEandSTRDUP(person->vibration, str);
		break;
	case CTSVC_PROPERTY_PERSON_MESSAGE_ALERT:
		FREEandSTRDUP(person->message_alert, str);
		break;
	default :
		CTS_ERR("This field(%d) is not supported in value(person)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_set_bool(contacts_record_h record, unsigned int property_id, bool value)
{
	ctsvc_person_s *person = (ctsvc_person_s *)record;

	switch(property_id) {
	case CTSVC_PROPERTY_PERSON_IS_FAVORITE:
		if (person->is_favorite != value) {
			person->is_favorite = value;
		}
		break;
	default:
		CTS_ERR("This field(%d) is not supported in value(person)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}



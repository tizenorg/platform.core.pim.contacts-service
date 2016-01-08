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
#include "ctsvc_snippet.h"

static int __ctsvc_person_create(contacts_record_h *out_record);
static int __ctsvc_person_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_person_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_person_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_person_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_person_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_person_get_bool(contacts_record_h record, unsigned int property_id, bool *value);
static int __ctsvc_person_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_person_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);
static int __ctsvc_person_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty);

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

static int __ctsvc_person_create(contacts_record_h *out_record)
{
	ctsvc_person_s *person;
	person = calloc(1, sizeof(ctsvc_person_s));
	RETVM_IF(NULL == person, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	*out_record = (contacts_record_h)person;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_person_s *person = (ctsvc_person_s*)record;

	/* help to find double-destroy bug (refer to the contacts_record_destroy) */
	person->base.plugin_cbs = NULL;

	free(person->base.properties_flags);
	free(person->display_name);
	free(person->display_name_index);
	free(person->ringtone_path);
	free(person->vibration);
	free(person->message_alert);
	free(person->image_thumbnail_path);
	free(person->status);
	free(person->addressbook_ids);
	ctsvc_snippet_free(&person->snippet);
	free(person);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_person_s *out_data = NULL;
	ctsvc_person_s *src_data = NULL;

	src_data = (ctsvc_person_s*)record;
	out_data = calloc(1, sizeof(ctsvc_person_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

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

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_person_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_get_int(contacts_record_h record, unsigned int property_id,
		int *out)
{
	ctsvc_person_s *person = (ctsvc_person_s*)record;
	switch (property_id) {
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
		ERR("This field(%d) is not supported in value(person)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static bool _ctsvc_result_is_snippet(contacts_record_h record, char **out_text,
		char **out_start_match, char **out_end_match)
{
	bool ret = false;
	char *text = NULL;

	int type = ctsvc_view_get_record_type(((ctsvc_record_s *)record)->view_uri);
	switch (type) {
	case CTSVC_RECORD_PERSON:
		if (false == ((ctsvc_person_s *)record)->snippet.is_snippet)
			break;
		text = ((ctsvc_person_s *)record)->snippet.text;
		if (NULL == text || '\0' == *text)
			break;
		*out_text = text;
		if (out_start_match)
			*out_start_match = ((ctsvc_person_s *)record)->snippet.start_match;
		if (out_end_match)
			*out_end_match = ((ctsvc_person_s *)record)->snippet.end_match;
		ret = true;
		break;
	case CTSVC_RECORD_RESULT:
		if (false == ((ctsvc_result_s *)record)->snippet.is_snippet)
			break;
		text = ((ctsvc_result_s *)record)->snippet.text;
		if (NULL == text || '\0' == *text)
			break;
		*out_text = text;
		if (out_start_match)
			*out_start_match = ((ctsvc_result_s *)record)->snippet.start_match;
		if (out_end_match)
			*out_end_match = ((ctsvc_result_s *)record)->snippet.end_match;
		ret = true;
		break;
	default:
		break;
	}
	return ret;
}

static char *_get_snippet_string(char *origin_string, const char *text,
		char *start_match, char *end_match, int len_start_match, int len_end_match)
{
	char *pos_start = NULL;
	pos_start = strstr(origin_string, text);
	if (NULL == pos_start) {
		ERR("strstr() Fail");
		return origin_string;
	}

	int len_value = strlen(origin_string);
	int len_offset = len_value - strlen(pos_start);
	int len_total = len_value + strlen(text) + len_start_match + len_end_match + 1;

	char *new_string = calloc(len_total, sizeof(char));
	snprintf(new_string, len_offset, "%s", origin_string);
	snprintf(new_string + len_offset, len_total - len_offset, "%s%s%s%s",
			start_match, text, end_match, origin_string + len_offset);
	free(origin_string);
	return new_string;
}

static int __ctsvc_person_get_str_real(contacts_record_h record, unsigned int property_id,
		char **out_str, bool copy)
{
	ctsvc_person_s *person = (ctsvc_person_s*)record;

	char *text = NULL;
	char *start_match = NULL;
	char *end_match = NULL;
	bool is_snippet = false;
	int len_start_match = 0;
	int len_end_match = 0;

	switch (property_id) {
	case CTSVC_PROPERTY_PERSON_DISPLAY_NAME:
		is_snippet = ctsvc_snippet_is_snippet(record, &text, &start_match, &end_match);

		if (true == is_snippet) {
			len_start_match = strlen(start_match);
			len_end_match = strlen(end_match);
			person->display_name = ctsvc_snippet_mod_string(person->display_name, text,
					start_match, end_match, len_start_match, len_end_match);
		}

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
	default:
		ERR("This field(%d) is not supported in value(person)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_get_str_p(contacts_record_h record, unsigned int property_id,
		char **out_str)
{
	return __ctsvc_person_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_person_get_str(contacts_record_h record, unsigned int property_id,
		char **out_str)
{
	return __ctsvc_person_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_person_get_bool(contacts_record_h record, unsigned int property_id,
		bool *value)
{
	ctsvc_person_s *person = (ctsvc_person_s*)record;
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
		ERR("This field(%d) is not supported in value(company)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_set_int(contacts_record_h record, unsigned int property_id,
		int value, bool *is_dirty)
{
	ctsvc_person_s *person = (ctsvc_person_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID:
		CHECK_DIRTY_VAL(person->name_contact_id, value, is_dirty);
		person->name_contact_id = value;
		break;
	case CTSVC_PROPERTY_PERSON_ID:
		CHECK_DIRTY_VAL(person->person_id, value, is_dirty);
		person->person_id = value;
		break;
	case CTSVC_PROPERTY_PERSON_LINK_COUNT:
		CHECK_DIRTY_VAL(person->link_count, value, is_dirty);
		person->link_count = value;
		break;
	default:
		ERR("This field(0x%0x) is not supported in value(person)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_set_str(contacts_record_h record, unsigned int property_id,
		const char *str, bool *is_dirty)
{
	ctsvc_person_s *person = (ctsvc_person_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_PERSON_DISPLAY_NAME:
		CHECK_DIRTY_STR(person->display_name, str, is_dirty);
		FREEandSTRDUP(person->display_name, str);
		break;
	case CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX:
		CHECK_DIRTY_STR(person->display_name_index, str, is_dirty);
		FREEandSTRDUP(person->display_name_index, str);
		break;
	case CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL:
		CHECK_DIRTY_STR(person->image_thumbnail_path, str, is_dirty);
		FREEandSTRDUP(person->image_thumbnail_path, str);
		break;
	case CTSVC_PROPERTY_PERSON_RINGTONE:
		CHECK_DIRTY_STR(person->ringtone_path, str, is_dirty);
		FREEandSTRDUP(person->ringtone_path, str);
		break;
	case CTSVC_PROPERTY_PERSON_VIBRATION:
		CHECK_DIRTY_STR(person->vibration, str, is_dirty);
		FREEandSTRDUP(person->vibration, str);
		break;
	case CTSVC_PROPERTY_PERSON_MESSAGE_ALERT:
		CHECK_DIRTY_STR(person->message_alert, str, is_dirty);
		FREEandSTRDUP(person->message_alert, str);
		break;
	default:
		ERR("This field(%d) is not supported in value(person)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_person_set_bool(contacts_record_h record, unsigned int property_id,
		bool value, bool *is_dirty)
{
	ctsvc_person_s *person = (ctsvc_person_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_PERSON_IS_FAVORITE:
		CHECK_DIRTY_VAL(person->is_favorite, value, is_dirty);
		if (person->is_favorite != value)
			person->is_favorite = value;
		break;
	default:
		ERR("This field(%d) is not supported in value(person)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}



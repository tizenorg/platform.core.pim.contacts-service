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
#include "ctsvc_notify.h"

#ifdef _CONTACTS_IPC_SERVER
#ifdef ENABLE_SIM_FEATURE
#include "ctsvc_server_sim.h"
#endif // ENABLE_SIM_FEATURE
#endif

static int __ctsvc_result_create(contacts_record_h* out_record);
static int __ctsvc_result_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_result_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_result_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_result_get_str(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_result_get_int(contacts_record_h record, unsigned int property_id, int* out_value);
static int __ctsvc_result_get_lli(contacts_record_h record, unsigned int property_id, long long int* out_value);
static int __ctsvc_result_get_bool(contacts_record_h record, unsigned int property_id, bool* out_value);
static int __ctsvc_result_get_double(contacts_record_h record, unsigned int property_id, double* out_value);
static int __ctsvc_result_set_int(contacts_record_h record, unsigned int property_id, int value);
static int __ctsvc_result_set_lli(contacts_record_h record, unsigned int property_id, long long int value);
static int __ctsvc_result_set_bool(contacts_record_h record, unsigned int property_id, bool value);
static int __ctsvc_result_set_str(contacts_record_h record, unsigned int property_id, const char *str);
static int __ctsvc_result_set_double(contacts_record_h record, unsigned int property_id, double value);

ctsvc_record_plugin_cb_s result_plugin_cbs = {
	.create = __ctsvc_result_create,
	.destroy = __ctsvc_result_destroy,
	.clone = __ctsvc_result_clone,
	.get_str = __ctsvc_result_get_str,
	.get_str_p = __ctsvc_result_get_str_p,
	.get_int = __ctsvc_result_get_int,
	.get_bool = __ctsvc_result_get_bool,
	.get_lli = __ctsvc_result_get_lli,
	.get_double = __ctsvc_result_get_double,
	.set_str = __ctsvc_result_set_str,
	.set_int = __ctsvc_result_set_int,
	.set_bool = __ctsvc_result_set_bool,
	.set_lli = __ctsvc_result_set_lli,
	.set_double = __ctsvc_result_set_double,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

static int __ctsvc_result_create(contacts_record_h* out_record)
{
	ctsvc_result_s *result;
	result = (ctsvc_result_s*)calloc(1, sizeof(ctsvc_result_s));
	RETVM_IF(NULL == result, CONTACTS_ERROR_OUT_OF_MEMORY,
			"Out of memory : calloc is Fail");

	*out_record = (contacts_record_h)result;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_result_destroy(contacts_record_h record, bool delete_child)
{
	GSList *cursor;
	ctsvc_result_s* result = (ctsvc_result_s*)record;

	for (cursor = result->values;cursor;cursor=cursor->next) {
		ctsvc_result_value_s *data = cursor->data;
		if (data->type == CTSVC_VIEW_DATA_TYPE_STR)
			free(data->value.s);
		free(data);
	}
	g_slist_free(result->values);
	result->base.plugin_cbs = NULL;	// help to find double destroy bug (refer to the contacts_record_destroy)
	free(result->base.properties_flags);

	free(result);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_result_clone(contacts_record_h record, contacts_record_h *out_record)
{
    ctsvc_result_s *out_data = NULL;
    ctsvc_result_s *src_data = NULL;
    GSList *cursor;

    src_data = (ctsvc_result_s*)record;
    out_data = calloc(1, sizeof(ctsvc_result_s));
    RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY,
			 "Out of memeory : calloc(ctsvc_result_s) Fail(%d)", CONTACTS_ERROR_OUT_OF_MEMORY);

	for (cursor=src_data->values;cursor;cursor=cursor->next) {
		ctsvc_result_value_s *src = cursor->data;
		ctsvc_result_value_s *dest = calloc(1, sizeof(ctsvc_result_value_s));
		dest->property_id = src->property_id;
		dest->type = src->type;
		switch(src->type) {
		case CTSVC_VIEW_DATA_TYPE_BOOL:
			dest->value.b = src->value.b;
			break;
		case CTSVC_VIEW_DATA_TYPE_INT:
			dest->value.i = src->value.i;
			break;
		case CTSVC_VIEW_DATA_TYPE_LLI:
			dest->value.l = src->value.l;
			break;
		case CTSVC_VIEW_DATA_TYPE_STR:
			dest->value.s = SAFE_STRDUP(src->value.s);
			break;
		case CTSVC_VIEW_DATA_TYPE_DOUBLE:
			dest->value.d = src->value.d;
			break;
		default:
			break;
		}
		out_data->values = g_slist_append(out_data->values, (void*)dest);
	}

	CTSVC_RECORD_COPY_BASE(&(out_data->base), &(src_data->base));

	*out_record = (contacts_record_h)out_data;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_result_get_str_real(contacts_record_h record, unsigned int property_id,
		char** out_str, bool copy)
{
	ctsvc_result_s* result = (ctsvc_result_s *)record;

	GSList *cursor;

	for (cursor = result->values;cursor;cursor=cursor->next) {
		ctsvc_result_value_s *data = cursor->data;
		if (data->property_id == property_id) {
			if (data->type == CTSVC_VIEW_DATA_TYPE_STR) {
				*out_str = GET_STR(copy, data->value.s);
				return CONTACTS_ERROR_NONE;
			}
			else {
				CTS_ERR("use another get_type API, (type : %d)", data->type);
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}
		}
	}

	return CONTACTS_ERROR_NO_DATA;
}

static int __ctsvc_result_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_result_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_result_get_str(contacts_record_h record, unsigned int property_id, char** out_str)
{
	return __ctsvc_result_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_result_get_int(contacts_record_h record, unsigned int property_id, int* out_value)
{
	ctsvc_result_s* result = (ctsvc_result_s *)record;

	GSList *cursor;
	//TODO: check the value type of property_id is int
	for (cursor = result->values;cursor;cursor=cursor->next) {
		ctsvc_result_value_s *data = cursor->data;
		if (data->property_id == property_id) {
			if (data->type == CTSVC_VIEW_DATA_TYPE_INT) {
				*out_value = data->value.i;
				return CONTACTS_ERROR_NONE;
			}
			else {
				CTS_ERR("use another get_type API, (type : %d)", data->type);
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}
		}
	}

	return CONTACTS_ERROR_NO_DATA;
}

static int __ctsvc_result_set_int(contacts_record_h record, unsigned int property_id, int value)
{
	ctsvc_result_s* result = (ctsvc_result_s *)record;
	GSList *cursor;
	ctsvc_result_value_s *data;

	// TODO: check the value type of property_id is int
	for (cursor = result->values;cursor;cursor=cursor->next) {
		data = cursor->data;
		if (data->property_id == property_id) {
			if (data->type == CTSVC_VIEW_DATA_TYPE_INT) {
#ifdef _CONTACTS_IPC_SERVER
#ifdef ENABLE_SIM_FEATURE

				if (property_id == CTSVC_PROPERTY_PHONELOG_SIM_SLOT_NO)
					data->value.i = ctsvc_server_sim_get_sim_slot_no_by_info_id(value);
				else
#endif // ENABLE_SIM_FEATURE
#endif
				data->value.i = value;
				return CONTACTS_ERROR_NONE;
			}
			else {
				CTS_ERR("use another get_type API, (type : %d)", data->type);
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}
		}
	}

	data = calloc(1, sizeof(ctsvc_result_value_s));
	data->property_id = property_id;
	data->type = CTSVC_VIEW_DATA_TYPE_INT;
#ifdef _CONTACTS_IPC_SERVER
#ifdef ENABLE_SIM_FEATURE
	if (property_id == CTSVC_PROPERTY_PHONELOG_SIM_SLOT_NO)
		data->value.i = ctsvc_server_sim_get_sim_slot_no_by_info_id(value);
	else
#endif // ENABLE_SIM_FEATURE
#endif
	data->value.i = value;
	result->values = g_slist_append(result->values, (void*)data);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_result_set_lli(contacts_record_h record, unsigned int property_id, long long int value)
{
	ctsvc_result_s* result = (ctsvc_result_s *)record;
	GSList *cursor;
	ctsvc_result_value_s *data;

	// TODO: check the value type of property_id is int
	for (cursor = result->values;cursor;cursor=cursor->next) {
		data = cursor->data;
		if (data->property_id == property_id) {
			if (data->type == CTSVC_VIEW_DATA_TYPE_LLI) {
				data->value.l = value;
				return CONTACTS_ERROR_NONE;
			}
			else {
				CTS_ERR("use another get_type API, (type : %d)", data->type);
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}
		}
	}

	data = calloc(1, sizeof(ctsvc_result_value_s));
	data->property_id = property_id;
	data->type = CTSVC_VIEW_DATA_TYPE_LLI;
	data->value.l = value;
	result->values = g_slist_append(result->values, (void*)data);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_result_set_double(contacts_record_h record, unsigned int property_id, double value)
{
	ctsvc_result_s* result = (ctsvc_result_s *)record;
	GSList *cursor;
	ctsvc_result_value_s *data;

	for (cursor = result->values;cursor;cursor=cursor->next) {
		data = cursor->data;
		if (data->property_id == property_id) {
			if (data->type == CTSVC_VIEW_DATA_TYPE_DOUBLE) {
				data->value.d = value;
				return CONTACTS_ERROR_NONE;
			}
			else {
				CTS_ERR("use another get_type API, (type : %d)", data->type);
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}
		}
	}

	data = calloc(1, sizeof(ctsvc_result_value_s));
	data->property_id = property_id;
	data->type = CTSVC_VIEW_DATA_TYPE_DOUBLE;
	data->value.d = value;
	result->values = g_slist_append(result->values, (void*)data);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_result_set_bool(contacts_record_h record, unsigned int property_id, bool value)
{
	ctsvc_result_s* result = (ctsvc_result_s *)record;
	GSList *cursor;
	ctsvc_result_value_s *data;

	// TODO: check the value type of property_id is int
	for (cursor = result->values;cursor;cursor=cursor->next) {
		data = cursor->data;
		if (data->property_id == property_id) {
			if (data->type == CTSVC_VIEW_DATA_TYPE_BOOL) {
				data->value.b = value;
				return CONTACTS_ERROR_NONE;
			}
			else {
				CTS_ERR("use another get_type API, (type : %d)", data->type);
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}
		}
	}

	data = calloc(1, sizeof(ctsvc_result_value_s));
	data->property_id = property_id;
	data->type = CTSVC_VIEW_DATA_TYPE_BOOL;
	data->value.i = value;
	result->values = g_slist_append(result->values, (void*)data);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_result_set_str(contacts_record_h record, unsigned int property_id, const char *str)
{
	ctsvc_result_s* result = (ctsvc_result_s *)record;
	GSList *cursor;
	ctsvc_result_value_s *data;
	char *full_path = NULL;
	int str_len;

	// TODO: check the value type of property_id is int
	for (cursor = result->values;cursor;cursor=cursor->next) {
		data = cursor->data;
		if (data->property_id == property_id) {
			if (data->type == CTSVC_VIEW_DATA_TYPE_STR) {
				switch (property_id) {
				case CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL:
				case CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL:
					if (str) {
						str_len = strlen(CTSVC_CONTACT_IMG_FULL_LOCATION) + strlen(str) + 2;
						full_path = calloc(1, str_len);
						snprintf(full_path, str_len, "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, str);
					}
					free(data->value.s);
					data->value.s = full_path;
					return CONTACTS_ERROR_NONE;
				default:
					FREEandSTRDUP(data->value.s, str);
					return CONTACTS_ERROR_NONE;
				}
			}
			else {
				CTS_ERR("use another get_type API, (type : %d)", data->type);
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}
		}
	}

	data = calloc(1, sizeof(ctsvc_result_value_s));
	data->property_id = property_id;
	data->type = CTSVC_VIEW_DATA_TYPE_STR;
	switch (property_id) {
	case CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL:
	case CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL:
		if (str) {
			str_len = strlen(CTSVC_CONTACT_IMG_FULL_LOCATION) + strlen(str) + 2;
			full_path = calloc(1, str_len);
			snprintf(full_path, str_len, "%s/%s", CTSVC_CONTACT_IMG_FULL_LOCATION, str);
		}
		free(data->value.s);
		data->value.s = full_path;
		break;
	default:
		data->value.s = SAFE_STRDUP(str);
		break;
	}

	result->values = g_slist_append(result->values, (void*)data);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_result_get_bool(contacts_record_h record, unsigned int property_id, bool* out_value)
{
	ctsvc_result_s* result = (ctsvc_result_s *)record;
	GSList *cursor;
	for (cursor = result->values;cursor;cursor=cursor->next) {
		ctsvc_result_value_s *data = cursor->data;
		if (data->property_id == property_id) {
			if (data->type == CTSVC_VIEW_DATA_TYPE_BOOL) {
				*out_value = data->value.b;
				return CONTACTS_ERROR_NONE;
			}
			else {
				CTS_ERR("use another get_type API, (type : %d)", data->type);
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}
		}
	}

	return CONTACTS_ERROR_NO_DATA;
}

static int __ctsvc_result_get_lli(contacts_record_h record, unsigned int property_id, long long int* out_value)
{
	ctsvc_result_s* result = (ctsvc_result_s *)record;
	GSList *cursor;
	for (cursor = result->values;cursor;cursor=cursor->next) {
		ctsvc_result_value_s *data = cursor->data;
		if (data->property_id == property_id) {
			if (data->type == CTSVC_VIEW_DATA_TYPE_LLI) {
				*out_value = data->value.l;
				return CONTACTS_ERROR_NONE;
				}
			else {
				CTS_ERR("use another get_type API, (type : %d)", data->type);
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}
		}
	}

	return CONTACTS_ERROR_NO_DATA;
}

static int __ctsvc_result_get_double(contacts_record_h record, unsigned int property_id, double* out_value)
{
	ctsvc_result_s* result = (ctsvc_result_s *)record;
	GSList *cursor;
	for (cursor = result->values;cursor;cursor=cursor->next) {
		ctsvc_result_value_s *data = cursor->data;
		if (data->property_id == property_id) {
			if (data->type == CTSVC_VIEW_DATA_TYPE_DOUBLE) {
				*out_value = data->value.d;
				return CONTACTS_ERROR_NONE;
				}
			else {
				CTS_ERR("use another get_type API, (type : %d)", data->type);
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}
		}
	}

	return CONTACTS_ERROR_NO_DATA;
}


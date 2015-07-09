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
#include "ctsvc_server_sim.h"
#endif /* _CONTACTS_IPC_SERVER */

static int __ctsvc_result_create(contacts_record_h* out_record);
static int __ctsvc_result_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_result_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_result_get_str_p(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_result_get_str(contacts_record_h record, unsigned int property_id, char** out_str);
static int __ctsvc_result_get_int(contacts_record_h record, unsigned int property_id, int* out_value);

ctsvc_record_plugin_cb_s result_plugin_cbs = {
	.create = __ctsvc_result_create,
	.destroy = __ctsvc_result_destroy,
	.clone = __ctsvc_result_clone,
	.get_str = __ctsvc_result_get_str,
	.get_str_p = __ctsvc_result_get_str_p,
	.get_int = __ctsvc_result_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = NULL,
	.set_int = NULL,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
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
	result->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
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
		if (NULL == dest) {
			CTS_ERR("calloc() Fail");
			__ctsvc_result_destroy((contacts_record_h)out_data, true);
			return CONTACTS_ERROR_OUT_OF_MEMORY;
		}

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

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_result_destroy((contacts_record_h)out_data, true);
		return ret;
	}

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
	/* TODO: check the value type of property_id is int */
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


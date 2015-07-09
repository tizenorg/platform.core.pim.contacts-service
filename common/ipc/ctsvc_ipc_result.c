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

#include "ctsvc_internal.h"
#include "ctsvc_ipc_marshal.h"
#include "contacts_record.h"

static int __ctsvc_ipc_unmarshal_result(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record);
static int __ctsvc_ipc_marshal_result(const contacts_record_h record, pims_ipc_data_h ipc_data);

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_result_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_result,
	.marshal_record = __ctsvc_ipc_marshal_result
};


static int __ctsvc_ipc_unmarshal_search_value(pims_ipc_data_h ipc_data, ctsvc_result_value_s* pvalue)
{
	if (ctsvc_ipc_unmarshal_int(ipc_data,&pvalue->property_id) != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_ipc_unmarshal fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	if (ctsvc_ipc_unmarshal_int(ipc_data,&pvalue->type) != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_ipc_unmarshal fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}


	if (CTSVC_VIEW_CHECK_DATA_TYPE(pvalue->property_id, CTSVC_VIEW_DATA_TYPE_STR) == true) {
		if (ctsvc_ipc_unmarshal_string(ipc_data,&pvalue->value.s) != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_marshal fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CTSVC_VIEW_CHECK_DATA_TYPE(pvalue->property_id, CTSVC_VIEW_DATA_TYPE_BOOL) == true) {
		if (ctsvc_ipc_unmarshal_bool(ipc_data,&pvalue->value.b) != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_marshal fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CTSVC_VIEW_CHECK_DATA_TYPE(pvalue->property_id, CTSVC_VIEW_DATA_TYPE_INT) == true) {
		if (ctsvc_ipc_unmarshal_int(ipc_data,&pvalue->value.i) != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_marshal fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CTSVC_VIEW_CHECK_DATA_TYPE(pvalue->property_id, CTSVC_VIEW_DATA_TYPE_DOUBLE) == true) {
		return CONTACTS_ERROR_NONE;
	}
	else if (CTSVC_VIEW_CHECK_DATA_TYPE(pvalue->property_id, CTSVC_VIEW_DATA_TYPE_LLI) == true) {
		return CONTACTS_ERROR_NONE;
	}
	else {
		ASSERT_NOT_REACHED("invalid parameter (property:%d)",pvalue->property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_ipc_marshal_search_value(const ctsvc_result_value_s* pvalue, pims_ipc_data_h ipc_data)
{
	if (ctsvc_ipc_marshal_int(pvalue->property_id,ipc_data) != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_ipc_marshal fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	if (ctsvc_ipc_marshal_int(pvalue->type,ipc_data) != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_ipc_marshal fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}



	if (CTSVC_VIEW_CHECK_DATA_TYPE(pvalue->property_id, CTSVC_VIEW_DATA_TYPE_STR) == true) {
		if (ctsvc_ipc_marshal_string(pvalue->value.s,ipc_data) != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_marshal fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CTSVC_VIEW_CHECK_DATA_TYPE(pvalue->property_id, CTSVC_VIEW_DATA_TYPE_BOOL) == true) {
		if (ctsvc_ipc_marshal_bool(pvalue->value.b,ipc_data) != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_marshal fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CTSVC_VIEW_CHECK_DATA_TYPE(pvalue->property_id, CTSVC_VIEW_DATA_TYPE_INT) == true) {
		if (ctsvc_ipc_marshal_int(pvalue->value.i,ipc_data) != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_marshal fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}
	else if (CTSVC_VIEW_CHECK_DATA_TYPE(pvalue->property_id, CTSVC_VIEW_DATA_TYPE_DOUBLE) == true) {
		return CONTACTS_ERROR_NONE;
	}
	else if (CTSVC_VIEW_CHECK_DATA_TYPE(pvalue->property_id, CTSVC_VIEW_DATA_TYPE_LLI) == true) {
		return CONTACTS_ERROR_NONE;
	}
	else {
		ASSERT_NOT_REACHED("invalid parameter (property:%d)",pvalue->property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}


static int __ctsvc_ipc_unmarshal_result(pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h record)
{
	RETV_IF(ipc_data==NULL, CONTACTS_ERROR_NO_DATA);
	RETV_IF(record==NULL, CONTACTS_ERROR_NO_DATA);

	ctsvc_result_s* result_p = (ctsvc_result_s*)record;

	unsigned int count = 0;
	if (ctsvc_ipc_unmarshal_unsigned_int(ipc_data, &count) != CONTACTS_ERROR_NONE) {
		CTS_ERR("ctsvc_ipc_unmarshal fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	unsigned int i = 0;
	for (i=0; i<count; i++) {
		ctsvc_result_value_s* value_data = NULL;
		value_data = calloc(1, sizeof(ctsvc_result_value_s));
		if (value_data == NULL) {
			return CONTACTS_ERROR_OUT_OF_MEMORY;
		}

		if (__ctsvc_ipc_unmarshal_search_value(ipc_data, value_data) != CONTACTS_ERROR_NONE) {
			CONTACTS_FREE(value_data);
			CTS_ERR("ctsvc_ipc_unmarshal fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		result_p->values = g_slist_append(result_p->values, value_data);
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_ipc_marshal_result(const contacts_record_h record, pims_ipc_data_h ipc_data)
{
	RETV_IF(ipc_data==NULL,CONTACTS_ERROR_NO_DATA);

	ctsvc_result_s* result_p = (ctsvc_result_s*)record;
	RETV_IF(result_p==NULL,CONTACTS_ERROR_NO_DATA);

	if (result_p->values) {
		unsigned int count = g_slist_length(result_p->values);
		if (ctsvc_ipc_marshal_unsigned_int(count, ipc_data) != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_marshal fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}

		GSList *cursor = result_p->values;
		while (cursor) {
			ctsvc_result_value_s* value_data = (ctsvc_result_value_s *)cursor->data;
			if (value_data == NULL) {
				cursor = g_slist_next(cursor);
				continue;
			}
			if (__ctsvc_ipc_marshal_search_value((const ctsvc_result_value_s*)value_data, ipc_data) != CONTACTS_ERROR_NONE) {
				CTS_ERR("ctsvc_ipc_marshal fail");
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}
			cursor = g_slist_next(cursor);
		}
	}
	else {
		if (ctsvc_ipc_marshal_int(0, ipc_data) != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_marshal fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}

	return CONTACTS_ERROR_NONE;
}


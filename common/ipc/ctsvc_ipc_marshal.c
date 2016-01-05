/*
 * Contacts Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <stdlib.h>
#include <string.h>
#include "ctsvc_ipc_marshal.h"
#include "contacts_record.h"
#include "ctsvc_internal.h"
#include "contacts_query.h"
#include "contacts_filter.h"
#include "contacts_list.h"
#include "ctsvc_list.h"
#include "ctsvc_handle.h"

extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_contact_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_my_profile_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_addressbook_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_group_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_person_plugin_cb;
#ifdef ENABLE_LOG_FEATURE
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_phonelog_plugin_cb;
#endif /* ENABLE_LOG_FEATURE */
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_result_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_sdn_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_speeddial_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_updated_info_plugin_cb;

extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_simple_contact_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_address_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_activity_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_activity_photo_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_company_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_email_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_event_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_extension_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_group_relation_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_messenger_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_name_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_nickname_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_note_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_number_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_profile_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_relationship_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_url_plugin_cb;
extern ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_image_plugin_cb;

static ctsvc_ipc_marshal_record_plugin_cb_s* __ctsvc_ipc_marshal_get_plugin_cb(ctsvc_record_type_e type);


static int __ctsvc_ipc_unmarshal_composite_filter(const pims_ipc_data_h ipc_data, ctsvc_composite_filter_s *filter);
static int __ctsvc_ipc_marshal_composite_filter(const ctsvc_composite_filter_s *filter, pims_ipc_data_h ipc_data);
static int __ctsvc_ipc_unmarshal_attribute_filter(const pims_ipc_data_h ipc_data, const ctsvc_filter_type_e filter_type, ctsvc_attribute_filter_s *filter);
static int __ctsvc_ipc_marshal_attribute_filter(const ctsvc_attribute_filter_s *filter, pims_ipc_data_h ipc_data);


static ctsvc_ipc_marshal_record_plugin_cb_s* __ctsvc_ipc_marshal_get_plugin_cb(ctsvc_record_type_e type)
{
	switch (type) {
	case CTSVC_RECORD_ADDRESSBOOK:
		return (&_ctsvc_ipc_record_addressbook_plugin_cb);
	case CTSVC_RECORD_GROUP:
		return (&_ctsvc_ipc_record_group_plugin_cb);
	case CTSVC_RECORD_PERSON:
		return (&_ctsvc_ipc_record_person_plugin_cb);
	case CTSVC_RECORD_CONTACT:
		return (&_ctsvc_ipc_record_contact_plugin_cb);
	case CTSVC_RECORD_MY_PROFILE:
		return (&_ctsvc_ipc_record_my_profile_plugin_cb);
	case CTSVC_RECORD_UPDATED_INFO:
		return (&_ctsvc_ipc_record_updated_info_plugin_cb);
#ifdef ENABLE_LOG_FEATURE
	case CTSVC_RECORD_PHONELOG:
		return (&_ctsvc_ipc_record_phonelog_plugin_cb);
#endif /* ENABLE_LOG_FEATURE */
	case CTSVC_RECORD_SPEEDDIAL:
		return (&_ctsvc_ipc_record_speeddial_plugin_cb);
	case CTSVC_RECORD_SDN:
		return (&_ctsvc_ipc_record_sdn_plugin_cb);
	case CTSVC_RECORD_RESULT:
		return (&_ctsvc_ipc_record_result_plugin_cb);
	case CTSVC_RECORD_SIMPLE_CONTACT:
		return &_ctsvc_ipc_record_simple_contact_plugin_cb;
	case CTSVC_RECORD_NAME:
		return (&_ctsvc_ipc_record_name_plugin_cb);
	case CTSVC_RECORD_COMPANY:
		return (&_ctsvc_ipc_record_company_plugin_cb);
	case CTSVC_RECORD_NOTE:
		return (&_ctsvc_ipc_record_note_plugin_cb);
	case CTSVC_RECORD_NUMBER:
		return (&_ctsvc_ipc_record_number_plugin_cb);
	case CTSVC_RECORD_EMAIL:
		return (&_ctsvc_ipc_record_email_plugin_cb);
	case CTSVC_RECORD_URL:
		return (&_ctsvc_ipc_record_url_plugin_cb);
	case CTSVC_RECORD_EVENT:
		return (&_ctsvc_ipc_record_event_plugin_cb);
	case CTSVC_RECORD_NICKNAME:
		return (&_ctsvc_ipc_record_nickname_plugin_cb);
	case CTSVC_RECORD_ADDRESS:
		return (&_ctsvc_ipc_record_address_plugin_cb);
	case CTSVC_RECORD_MESSENGER:
		return (&_ctsvc_ipc_record_messenger_plugin_cb);
	case CTSVC_RECORD_GROUP_RELATION:
		return (&_ctsvc_ipc_record_group_relation_plugin_cb);
	case CTSVC_RECORD_ACTIVITY:
		return (&_ctsvc_ipc_record_activity_plugin_cb);
	case CTSVC_RECORD_ACTIVITY_PHOTO:
		return (&_ctsvc_ipc_record_activity_photo_plugin_cb);
	case CTSVC_RECORD_PROFILE:
		return (&_ctsvc_ipc_record_profile_plugin_cb);
	case CTSVC_RECORD_RELATIONSHIP:
		return (&_ctsvc_ipc_record_relationship_plugin_cb);
	case CTSVC_RECORD_IMAGE:
		return (&_ctsvc_ipc_record_image_plugin_cb);
	case CTSVC_RECORD_EXTENSION:
		return (&_ctsvc_ipc_record_extension_plugin_cb);
	default:
		ASSERT_NOT_REACHED("Unimplemented IPC module (%d)", type);
		return NULL;
	}
}

static void __ctsvc_ipc_unmarshal_composite_filter_free(ctsvc_composite_filter_s *filter)
{
	if (filter->filters) {
		GSList *cursor = NULL;
		for (cursor = filter->filters; cursor; cursor = cursor->next) {
			ctsvc_filter_s *src = cursor->data;
			if (src->filter_type == CTSVC_FILTER_COMPOSITE) {
				__ctsvc_ipc_unmarshal_composite_filter_free((ctsvc_composite_filter_s*)src);
			} else {
				ctsvc_attribute_filter_s *attr = (ctsvc_attribute_filter_s*)src;
				if (attr->filter_type == CTSVC_FILTER_STR)
					free(attr->value.s);
			}
			free(src);
		}
		g_slist_free(filter->filters);
	}

	if (filter->filter_ops)
		g_slist_free(filter->filter_ops);

	free(filter->view_uri);
}

static int __ctsvc_ipc_unmarshal_composite_filter(const pims_ipc_data_h ipc_data, ctsvc_composite_filter_s *filter)
{
	unsigned int size = 0;
	char *str = NULL;
	int count = 0, i = 0;
	ctsvc_filter_type_e filter_type = CTSVC_FILTER_COMPOSITE;
	contacts_filter_operator_e op = CONTACTS_FILTER_OPERATOR_AND;

	filter->filter_type = CTSVC_FILTER_COMPOSITE;

	/* view_uri */
	str = (char*)pims_ipc_data_get(ipc_data, &size);
	filter->view_uri = (char*)SAFE_STRDUP(str);

	/* filters */
	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(ipc_data, &count)) {
		ERR("ctsvc_ipc_unmarshal_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	for (i = 0; i < count; i++) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(ipc_data, (int*)&filter_type)) {
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			__ctsvc_ipc_unmarshal_composite_filter_free(filter);
			return CONTACTS_ERROR_IPC;
		}
		if (filter_type == CTSVC_FILTER_COMPOSITE) {
			ctsvc_composite_filter_s *com_filter = NULL;
			com_filter = calloc(1, sizeof(ctsvc_composite_filter_s));
			if (com_filter == NULL) {
				ERR("calloc() Fail");
				__ctsvc_ipc_unmarshal_composite_filter_free(filter);
				return CONTACTS_ERROR_OUT_OF_MEMORY;
			}
			if (CONTACTS_ERROR_NONE != __ctsvc_ipc_unmarshal_composite_filter(ipc_data, com_filter)) {
				ERR("__ctsvc_ipc_unmarshal_composite_filter() Fail");
				free(com_filter);
				__ctsvc_ipc_unmarshal_composite_filter_free(filter);
				return CONTACTS_ERROR_IPC;
			}
			filter->filters = g_slist_append(filter->filters, com_filter);
		} else {
			ctsvc_attribute_filter_s *attr_filter = NULL;
			attr_filter = calloc(1, sizeof(ctsvc_attribute_filter_s));
			if (attr_filter == NULL) {
				ERR("calloc() Fail");
				__ctsvc_ipc_unmarshal_composite_filter_free(filter);
				return CONTACTS_ERROR_OUT_OF_MEMORY;
			}
			if (CONTACTS_ERROR_NONE != __ctsvc_ipc_unmarshal_attribute_filter(ipc_data, filter_type, attr_filter)) {
				free(attr_filter);
				ERR("__ctsvc_ipc_unmarshal_attribute_filter() Fail");
				__ctsvc_ipc_unmarshal_composite_filter_free(filter);
				return CONTACTS_ERROR_IPC;
			}
			filter->filters = g_slist_append(filter->filters, attr_filter);
		}
	}

	/* filters */
	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(ipc_data, &count)) {
		ERR("ctsvc_ipc_unmarshal_int() Fail");
		__ctsvc_ipc_unmarshal_composite_filter_free(filter);
		return CONTACTS_ERROR_IPC;
	}

	for (i = 0; i < count; i++) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(ipc_data, (int *)&op)) {
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			__ctsvc_ipc_unmarshal_composite_filter_free(filter);
			return CONTACTS_ERROR_IPC;
		}
		filter->filter_ops = g_slist_append(filter->filter_ops, (void*)op);
	}

	/* properties  property_count */
	filter->properties = (property_info_s*)ctsvc_view_get_all_property_infos(filter->view_uri, &filter->property_count);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_ipc_marshal_composite_filter(const ctsvc_composite_filter_s *filter, pims_ipc_data_h ipc_data)
{
	if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int((filter->filter_type), ipc_data)) {
		ERR("ctsvc_ipc_marshal_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	/* view_uri */
	int length = strlen(filter->view_uri);
	if (pims_ipc_data_put(ipc_data, (void*)filter->view_uri, length+1) < 0) {
		ERR("pims_ipc_data_put() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	/* filter->filters */
	if (filter->filters) {
		int count = g_slist_length(filter->filters);
		GSList *cursor = filter->filters;
		ctsvc_filter_s *child_filter;

		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(count, ipc_data)) {
			ERR("ctsvc_ipc_marshal_int() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		while (cursor) {
			child_filter = cursor->data;

			if (child_filter->filter_type == CTSVC_FILTER_COMPOSITE) {
				if (CONTACTS_ERROR_NONE != __ctsvc_ipc_marshal_composite_filter((ctsvc_composite_filter_s*)child_filter, ipc_data)) {
					ERR("__ctsvc_ipc_marshal_composite_filter() Fail");
					return CONTACTS_ERROR_INVALID_PARAMETER;
				}
			} else {
				if (CONTACTS_ERROR_NONE != __ctsvc_ipc_marshal_attribute_filter((ctsvc_attribute_filter_s*)child_filter, ipc_data)) {
					ERR("__ctsvc_ipc_marshal_attribute_filter() Fail");
					return CONTACTS_ERROR_INVALID_PARAMETER;
				}
			}
			cursor = g_slist_next(cursor);
		}
	} else {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(0, ipc_data)) {
			ERR("ctsvc_ipc_marshal_int() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}

	if (filter->filter_ops) {
		int count = g_slist_length(filter->filter_ops);
		GSList *cursor = filter->filter_ops;

		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(count, ipc_data)) {
			ERR("ctsvc_ipc_marshal_int() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		while (cursor) {
			contacts_filter_operator_e op = (contacts_filter_operator_e)cursor->data;

			if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(op, ipc_data)) {
				ERR("ctsvc_ipc_marshal_int() Fail");
				return CONTACTS_ERROR_INVALID_PARAMETER;
			}

			cursor = g_slist_next(cursor);
		}
	} else {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(0, ipc_data)) {
			ERR("ctsvc_ipc_marshal_int() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_ipc_unmarshal_attribute_filter(const pims_ipc_data_h ipc_data, const ctsvc_filter_type_e filter_type, ctsvc_attribute_filter_s *filter)
{
	filter->filter_type = filter_type;
	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(ipc_data, &filter->property_id)) {
		ERR("ctsvc_ipc_unmarshal_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(ipc_data, &filter->match)) {
		ERR("ctsvc_ipc_unmarshal_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	switch (filter->filter_type) {
	case CTSVC_FILTER_STR:
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_string(ipc_data, &filter->value.s)) {
			ERR("ctsvc_ipc_unmarshal_string() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		break;
	case CTSVC_FILTER_INT:
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(ipc_data, &filter->value.i)) {
			ERR("ctsvc_ipc_unmarshal_int() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		break;
	case CTSVC_FILTER_BOOL:
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_bool(ipc_data, &filter->value.b)) {
			ERR("ctsvc_ipc_unmarshal_bool() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		break;
	case CTSVC_FILTER_DOUBLE:
	case CTSVC_FILTER_LLI:
	default:
		break;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_ipc_marshal_attribute_filter(const ctsvc_attribute_filter_s *filter, pims_ipc_data_h ipc_data)
{
	if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int((filter->filter_type), ipc_data)) {
		ERR("ctsvc_ipc_marshal_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int((filter->property_id), ipc_data)) {
		ERR("ctsvc_ipc_marshal_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int((filter->match), ipc_data)) {
		ERR("ctsvc_ipc_marshal_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	switch (filter->filter_type) {
	case CTSVC_FILTER_STR:
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_string((filter->value.s), ipc_data)) {
			ERR("ctsvc_ipc_marshal_string() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		break;
	case CTSVC_FILTER_INT:
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int((filter->value.i), ipc_data)) {
			ERR("ctsvc_ipc_marshal_int() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		break;
	case CTSVC_FILTER_BOOL:
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_bool((filter->value.b), ipc_data)) {
			ERR("ctsvc_ipc_marshal_bool() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		break;
	case CTSVC_FILTER_DOUBLE:
	case CTSVC_FILTER_LLI:
	default:
		break;
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_marshal_handle(const contacts_h contact, pims_ipc_data_h ipc_data)
{
	int ret;
	ctsvc_base_s *base = (ctsvc_base_s*)contact;

	ret = ctsvc_ipc_marshal_int(base->version, ipc_data);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_marshal_int() Fail(%d)", ret);

	ret = ctsvc_ipc_marshal_int(base->connection_count, ipc_data);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_marshal_int() Fail(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_unmarshal_handle(const pims_ipc_data_h ipc_data, contacts_h *pcontact)
{
	int ret;
	ctsvc_base_s *base = NULL;

	ret = ctsvc_handle_create(pcontact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_handle_create() Fail(%d)", ret);

	base = (ctsvc_base_s*)*pcontact;

	ret = ctsvc_ipc_unmarshal_int(ipc_data, &(base->version));
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_unmarshal_int() Fail(%d)", ret);

	ret = ctsvc_ipc_unmarshal_int(ipc_data, &(base->connection_count));
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_unmarshal_int() Fail(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_unmarshal_record(const pims_ipc_data_h ipc_data, contacts_record_h *precord)
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_record_s common = {0,};
	ctsvc_record_s *precord_common = NULL;
	ctsvc_ipc_marshal_record_plugin_cb_s *plugin_cb;

	RETV_IF(NULL == precord, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_record_common(ipc_data, &common)) {
		ERR("ctsvc_ipc_unmarshal_record_common() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	plugin_cb = __ctsvc_ipc_marshal_get_plugin_cb(common.r_type);
	if (NULL == plugin_cb || NULL == plugin_cb->unmarshal_record) {
		ERR("Invalid parameter");
		free(common.properties_flags);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	ret = contacts_record_create(common.view_uri, precord);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("contacts_record_create() Fail");
		free(common.properties_flags);
		return ret;
	}

	precord_common = (ctsvc_record_s*)(*precord);
	precord_common->property_max_count = common.property_max_count;
	precord_common->properties_flags = common.properties_flags;
	precord_common->property_flag = common.property_flag;

	ret = plugin_cb->unmarshal_record(ipc_data, common.view_uri, *precord);
	if (CONTACTS_ERROR_NONE != ret) {
		contacts_record_destroy(*precord, true);
		*precord = NULL;
		return CONTACTS_ERROR_IPC;
	}

	return ret;
}

int ctsvc_ipc_marshal_record(const contacts_record_h record, pims_ipc_data_h ipc_data)
{
	ctsvc_record_s *common = (ctsvc_record_s*)(record);

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_record_common(common, ipc_data)) {
		ERR("ctsvc_ipc_marshal_record_common() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	ctsvc_ipc_marshal_record_plugin_cb_s *plugin_cb = __ctsvc_ipc_marshal_get_plugin_cb(common->r_type);
	RETV_IF(NULL == plugin_cb, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin_cb->marshal_record, CONTACTS_ERROR_INVALID_PARAMETER);

	int ret = plugin_cb->marshal_record(record, ipc_data);

	return ret;
}

int ctsvc_ipc_unmarshal_string(const pims_ipc_data_h ipc_data, char **ppbufchar)
{
	int ret = CONTACTS_ERROR_NONE;

	void *tmp = NULL;
	unsigned int size = 0;
	char *str = NULL;

	int length = 0;

	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ppbufchar, CONTACTS_ERROR_INVALID_PARAMETER);

	tmp = pims_ipc_data_get(ipc_data, &size);
	if (tmp == NULL) {
		ERR("pims_ipc_data_get() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	length = *(int*)tmp;

	if (length == -1) {
		DBG("string is null");
		ret = CONTACTS_ERROR_NONE;
		*ppbufchar = NULL;
		return ret;
	}

	str = (char*)pims_ipc_data_get(ipc_data, &size);
	if (str)
		*ppbufchar = SAFE_STRDUP(str);

	return ret;
}

int ctsvc_ipc_unmarshal_int(const pims_ipc_data_h data, int *pout)
{
	unsigned int size = 0;

	RETV_IF(NULL == data, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == pout, CONTACTS_ERROR_INVALID_PARAMETER);

	void *tmp = pims_ipc_data_get(data, &size);
	if (tmp == NULL) {
		ERR("pims_ipc_data_get() Fail");
		return CONTACTS_ERROR_NO_DATA;
	} else {
		*pout = *(int*)tmp;
	}
	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_unmarshal_unsigned_int(const pims_ipc_data_h data, unsigned int *pout)
{
	unsigned int size = 0;

	RETV_IF(NULL == data, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == pout, CONTACTS_ERROR_INVALID_PARAMETER);

	void *tmp = pims_ipc_data_get(data, &size);
	if (tmp == NULL) {
		ERR("pims_ipc_data_get() Fail");
		return CONTACTS_ERROR_NO_DATA;
	} else {
		*pout = *(unsigned int*)tmp;
	}
	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_unmarshal_bool(const pims_ipc_data_h data, bool *pout)
{
	unsigned int size = 0;

	RETV_IF(NULL == data, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == pout, CONTACTS_ERROR_INVALID_PARAMETER);

	void *tmp = pims_ipc_data_get(data, &size);
	if (tmp == NULL) {
		ERR("pims_ipc_data_get() Fail");
		return CONTACTS_ERROR_NO_DATA;
	} else {
		*pout = *(bool*)tmp;
	}
	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_unmarshal_record_common(const pims_ipc_data_h ipc_data, ctsvc_record_s *common)
{
	void *tmp = NULL;
	unsigned int size = 0;
	const char *str = NULL;

	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_NO_DATA);

	tmp = pims_ipc_data_get(ipc_data, &size);
	if (tmp == NULL) {
		ERR("pims_ipc_data_get() Fail");
		return CONTACTS_ERROR_NO_DATA;
	} else {
		common->r_type = *(ctsvc_record_type_e*)tmp;
	}

	str = (char*)pims_ipc_data_get(ipc_data, &size);
	if (NULL == str) {
		ERR("pims_ipc_data_get() Fail");
		return CONTACTS_ERROR_NO_DATA;
	}
	common->view_uri = ctsvc_view_get_uri(str);

	tmp = pims_ipc_data_get(ipc_data, &size);
	if (NULL == tmp) {
		ERR("pims_ipc_data_get() Fail");
		return CONTACTS_ERROR_NO_DATA;
	}
	common->property_max_count = *(unsigned int*)tmp;

	if (0 < common->property_max_count) {
		unsigned char *tmp_properties_flags;
		tmp_properties_flags = (unsigned char*)pims_ipc_data_get(ipc_data, &size);
		if (NULL == tmp_properties_flags) {
			ERR("pims_ipc_data_get() Fail");
			return CONTACTS_ERROR_NO_DATA;
		}

		common->properties_flags  = calloc(common->property_max_count, sizeof(char));
		if (common->properties_flags == NULL) {
			ERR("calloc() Fail");
			return CONTACTS_ERROR_OUT_OF_MEMORY;
		}
		memcpy(common->properties_flags, tmp_properties_flags, sizeof(char)*common->property_max_count);
	}
	tmp = pims_ipc_data_get(ipc_data, &size);
	if (NULL == tmp) {
		ERR("pims_ipc_data_get() Fail");
		return CONTACTS_ERROR_NO_DATA;
	}
	common->property_flag = *(unsigned char*)tmp;

	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_marshal_string(const char *bufchar, pims_ipc_data_h ipc_data)
{
	int ret = CONTACTS_ERROR_NONE;

	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);

	if (bufchar) {
		int length = strlen(bufchar);
		if (pims_ipc_data_put(ipc_data, (void*)&length, sizeof(int)) != 0)
			ret = CONTACTS_ERROR_OUT_OF_MEMORY;

		if (pims_ipc_data_put(ipc_data, (void*)bufchar, length+1) != 0) {
			ret = CONTACTS_ERROR_OUT_OF_MEMORY;
			return ret;
		}
	} else {
		int length = -1;

		if (pims_ipc_data_put(ipc_data, (void*)&length, sizeof(int)) != 0)
			ret = CONTACTS_ERROR_OUT_OF_MEMORY;
	}
	return ret;
}

int ctsvc_ipc_marshal_int(const int in, pims_ipc_data_h ipc_data)
{
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);

	if (pims_ipc_data_put(ipc_data, (void*)&in, sizeof(int)) != 0)
		return CONTACTS_ERROR_OUT_OF_MEMORY;

	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_marshal_unsigned_int(const unsigned int in, pims_ipc_data_h ipc_data)
{
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);

	if (pims_ipc_data_put(ipc_data, (void*)&in, sizeof(unsigned int)) != 0)
		return CONTACTS_ERROR_OUT_OF_MEMORY;

	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_marshal_bool(const bool in, pims_ipc_data_h ipc_data)
{
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);

	if (pims_ipc_data_put(ipc_data, (void*)&in, sizeof(bool)) != 0)
		return CONTACTS_ERROR_OUT_OF_MEMORY;

	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_marshal_record_common(const ctsvc_record_s *common, pims_ipc_data_h ipc_data)
{

	RETV_IF(NULL == common, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);

	if (pims_ipc_data_put(ipc_data, (void*)&common->r_type, sizeof(int)) < 0)
		return CONTACTS_ERROR_NO_DATA;

	int length = strlen(common->view_uri);
	if (pims_ipc_data_put(ipc_data, (void*)common->view_uri, length+1) < 0) {
		ERR("pims_ipc_data_put() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	if (pims_ipc_data_put(ipc_data, (void*)&common->property_max_count, sizeof(unsigned int)) < 0)
		return CONTACTS_ERROR_NO_DATA;

	if (0 < common->property_max_count) {
		if (pims_ipc_data_put(ipc_data, (void*)common->properties_flags, sizeof(unsigned char)*common->property_max_count) < 0) {
			ERR("pims_ipc_data_put() Fail");
			return CONTACTS_ERROR_NO_DATA;
		}
	}

	if (pims_ipc_data_put(ipc_data, (void*)&common->property_flag, sizeof(char)) < 0)
		return CONTACTS_ERROR_NO_DATA;

	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_unmarshal_query(const pims_ipc_data_h ipc_data, contacts_query_h *out_query)
{
	ctsvc_query_s *query = NULL;
	unsigned int size = 0;
	char *str = NULL;
	unsigned int count = 0, i = 0;
	int ret = CONTACTS_ERROR_NONE;

	RETV_IF(NULL == out_query, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);

	/* view_uri */
	str = (char*)pims_ipc_data_get(ipc_data, &size);

	ret = contacts_query_create(str, out_query);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("contacts_query_create() Fail");
		return ret;
	}
	query = (ctsvc_query_s*)*out_query;

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_unsigned_int(ipc_data, &count)) {
		ERR("contacts_query_create() Fail");
		contacts_query_destroy(*out_query);
		*out_query = NULL;
		return CONTACTS_ERROR_IPC;
	}

	if (0 == count) {
		query->filter = NULL;
	} else {
		ctsvc_composite_filter_s *filter = NULL;
		filter = calloc(1, sizeof(ctsvc_composite_filter_s));
		if (NULL == filter) {
			ERR("calloc() Fail");
			contacts_query_destroy(*out_query);
			*out_query = NULL;
			return CONTACTS_ERROR_OUT_OF_MEMORY;
		}
		filter->filter_type = CTSVC_FILTER_COMPOSITE;
		filter->properties = (property_info_s*)ctsvc_view_get_all_property_infos(query->view_uri, &filter->property_count);
		query->filter = filter;

		/* for filter_type */
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_unsigned_int(ipc_data, &count)) {
			ERR("ctsvc_ipc_unmarshal_unsigned_int() Fail");
			contacts_query_destroy(*out_query);
			*out_query = NULL;
			return CONTACTS_ERROR_IPC;
		}

		if (CONTACTS_ERROR_NONE != __ctsvc_ipc_unmarshal_composite_filter(ipc_data, query->filter)) {
			ERR("__ctsvc_ipc_unmarshal_composite_filter() Fail");
			contacts_query_destroy(*out_query);
			*out_query = NULL;
			return CONTACTS_ERROR_IPC;
		}
	}

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_unsigned_int(ipc_data, &(query->projection_count))) {
		ERR("ctsvc_ipc_unmarshal_unsigned_int() Fail");
		contacts_query_destroy(*out_query);
		*out_query = NULL;
		return CONTACTS_ERROR_IPC;
	}

	if (0 < query->projection_count) {
		query->projection = malloc(sizeof(unsigned int)*query->projection_count);
		if (query->projection == NULL) {
			ERR("malloc() Fail");
			contacts_query_destroy(*out_query);
			*out_query = NULL;
			return CONTACTS_ERROR_OUT_OF_MEMORY;
		}
		for (i = 0; i < query->projection_count; i++) {
			if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_unsigned_int(ipc_data, &(query->projection[i]))) {
				ERR("ctsvc_ipc_unmarshal_unsigned_int() Fail");
				contacts_query_destroy(*out_query);
				*out_query = NULL;
				return CONTACTS_ERROR_IPC;
			}
		}
	} else {
		query->projection = NULL;
	}

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_unsigned_int(ipc_data, &(query->sort_property_id))) {
		ERR("ctsvc_ipc_unmarshal_unsigned_int() Fail");
		contacts_query_destroy(*out_query);
		*out_query = NULL;
		return CONTACTS_ERROR_IPC;
	}

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_bool(ipc_data, &(query->sort_asc))) {
		ERR("ctsvc_ipc_unmarshal_bool() Fail");
		contacts_query_destroy(*out_query);
		*out_query = NULL;
		return CONTACTS_ERROR_IPC;
	}

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_bool(ipc_data, &(query->is_distinct))) {
		ERR("ctsvc_ipc_unmarshal_bool() Fail");
		contacts_query_destroy(*out_query);
		*out_query = NULL;
		return CONTACTS_ERROR_IPC;
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_marshal_query(const contacts_query_h query, pims_ipc_data_h ipc_data)
{
	ctsvc_query_s *que = NULL;
	int i = 0;
	int length = 0;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);
	que = (ctsvc_query_s*)query;

	/* view_uri */
	length = strlen(que->view_uri);
	if (pims_ipc_data_put(ipc_data, (void*)que->view_uri, length+1) < 0) {
		ERR("pims_ipc_data_put() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	if (que->filter) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(1, ipc_data)) {
			ERR("ctsvc_ipc_marshal_int() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}

		if (CONTACTS_ERROR_NONE != __ctsvc_ipc_marshal_composite_filter(que->filter, ipc_data)) {
			ERR("__ctsvc_ipc_marshal_composite_filter() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	} else {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(0, ipc_data)) {
			ERR("ctsvc_ipc_marshal_int() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_unsigned_int(que->projection_count, ipc_data)) {
		ERR("ctsvc_ipc_marshal_unsigned_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	for (i = 0; i < que->projection_count; i++) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_unsigned_int(que->projection[i], ipc_data)) {
			ERR("ctsvc_ipc_marshal_unsigned_int() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_unsigned_int(que->sort_property_id, ipc_data)) {
		ERR("ctsvc_ipc_marshal_unsigned_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_bool(que->sort_asc, ipc_data)) {
		ERR("ctsvc_ipc_marshal_bool() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_bool(que->is_distinct, ipc_data)) {
		ERR("ctsvc_ipc_marshal_bool() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_unmarshal_list(const pims_ipc_data_h ipc_data, contacts_list_h *list)
{
	int count = 0;
	unsigned int deleted_count = 0;
	contacts_record_h record;

	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);

	if (CONTACTS_ERROR_NONE != contacts_list_create(list)) {
		ERR("contacts_list_create() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(ipc_data, &(count))) {
		ERR("ctsvc_ipc_unmarshal_int() Fail");
		contacts_list_destroy(*list, true);
		*list = NULL;
		return CONTACTS_ERROR_IPC;
	}

	unsigned int i = 0;
	for (i = 0; i < count; i++) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_record(ipc_data, &record)) {
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			contacts_list_destroy(*list, true);
			*list = NULL;
			return CONTACTS_ERROR_IPC;
		}

		if (CONTACTS_ERROR_NONE != contacts_list_add(*list, record)) {
			ERR("contacts_list_add() Fail");
			contacts_list_destroy(*list, true);
			*list = NULL;
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_unsigned_int(ipc_data, &deleted_count)) {
		ERR("ctsvc_ipc_unmarshal_unsigned_int() Fail");
		contacts_list_destroy(*list, true);
		*list = NULL;
		return CONTACTS_ERROR_IPC;
	}

	i = 0;
	for (i = 0; i < deleted_count; i++) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_record(ipc_data, &record)) {
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			contacts_list_destroy(*list, true);
			*list = NULL;
			return CONTACTS_ERROR_IPC;
		}

		if (CONTACTS_ERROR_NONE != ctsvc_list_append_deleted_record(*list, record)) {
			ERR("ctsvc_list_append_deleted_record() Fail");
			contacts_list_destroy(*list, true);
			*list = NULL;
			return CONTACTS_ERROR_IPC;
		}
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_unmarshal_child_list(const pims_ipc_data_h ipc_data, contacts_list_h *list)
{
	unsigned int i = 0;
	int count = 0;
	unsigned int deleted_count = 0;
	contacts_record_h record;

	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_int(ipc_data, &(count))) {
		ERR("ctsvc_ipc_unmarshal_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	for (i = 0; i < count; i++) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_record(ipc_data, &record)) {
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}

		if (CONTACTS_ERROR_NONE != contacts_list_add(*list, record)) {
			ERR("contacts_list_add() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}

	if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_unsigned_int(ipc_data, &deleted_count)) {
		ERR("ctsvc_ipc_unmarshal_unsigned_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	i = 0;
	for (i = 0; i < deleted_count; i++) {
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_unmarshal_record(ipc_data, &record)) {
			ERR("ctsvc_ipc_unmarshal_record() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}

		if (CONTACTS_ERROR_NONE != ctsvc_list_append_deleted_record(*list, record)) {
			ERR("ctsvc_list_append_deleted_record() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_ipc_marshal_list(const contacts_list_h list, pims_ipc_data_h ipc_data)
{
	int count = 0;
	unsigned int deleted_count = 0;
	contacts_record_h record;

	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_INVALID_PARAMETER);

	/* count */
	if (CONTACTS_ERROR_NONE != contacts_list_get_count(list, &count)) {
		ERR("contacts_list_get_count() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(count, ipc_data)) {
		ERR("ctsvc_ipc_marshal_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	contacts_list_first(list);

	unsigned int i = 0;
	for (i = 0; i < count; i++) {
		if (CONTACTS_ERROR_NONE != contacts_list_get_current_record_p(list, &record)) {
			ERR("contacts_list_get_current_record_p() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_record(record, ipc_data)) {
			ERR("ctsvc_ipc_marshal_record() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}

		if (CONTACTS_ERROR_NO_DATA == contacts_list_next(list))
			break;
	}

	/* count */
	if (ctsvc_list_get_deleted_count(list, &deleted_count)) {
		ERR("ctsvc_list_get_deleted_count() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(deleted_count, ipc_data)) {
		ERR("ctsvc_ipc_marshal_int() Fail");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	i = 0;
	for (i = 0; i < deleted_count; i++) {
		if (CONTACTS_ERROR_NONE != ctsvc_list_get_deleted_nth_record_p(list, i, &record)) {
			ERR("ctsvc_list_get_deleted_nth_record_p() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_record(record, ipc_data)) {
			ERR("ctsvc_ipc_marshal_record() Fail");
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}

	return CONTACTS_ERROR_NONE;
}


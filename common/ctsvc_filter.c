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
#include <glib.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_filter.h"

static inline bool _filter_property_check(const property_info_s *properties,
		int count, unsigned int property_id, int *type)
{
	int i;
	for (i = 0; i < count; i++) {
		property_info_s *p = (property_info_s*)&(properties[i]);
		if (property_id == p->property_id) {
			if (p->property_type == CTSVC_SEARCH_PROPERTY_ALL
					|| p->property_type == CTSVC_SEARCH_PROPERTY_FILTER) {
				*type = (property_id & CTSVC_VIEW_DATA_TYPE_MASK);
				return true;
			} else {
				return false;
			}
		}
	}
	return false;
}

API int contacts_filter_create(const char *view_uri, contacts_filter_h *out_filter)
{
	ctsvc_composite_filter_s *com_filter;

	RETV_IF(NULL == out_filter, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_filter = NULL;

	RETV_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER);

	com_filter = calloc(1, sizeof(ctsvc_composite_filter_s));
	RETV_IF(NULL == com_filter, CONTACTS_ERROR_OUT_OF_MEMORY);

	com_filter->filter_type = CTSVC_FILTER_COMPOSITE;
	com_filter->view_uri = strdup(view_uri);
	com_filter->properties = (property_info_s*)ctsvc_view_get_all_property_infos(view_uri,
			&com_filter->property_count);
	*out_filter = (contacts_filter_h)com_filter;
	return CONTACTS_ERROR_NONE;
}

API int contacts_filter_add_operator(contacts_filter_h filter,
		contacts_filter_operator_e op)
{
	ctsvc_composite_filter_s *com_filter;

	RETV_IF(NULL == filter, CONTACTS_ERROR_INVALID_PARAMETER);

	com_filter = (ctsvc_composite_filter_s*)filter;
	RETVM_IF(g_slist_length(com_filter->filter_ops) != (g_slist_length(com_filter->filters)-1),
			CONTACTS_ERROR_INVALID_PARAMETER, "Please check the operator of filter");

	com_filter->filter_ops = g_slist_append(com_filter->filter_ops, (void*)op);

	return CONTACTS_ERROR_NONE;
}

API int contacts_filter_add_filter(contacts_filter_h filter1, contacts_filter_h filter2)
{
	int ret;
	ctsvc_composite_filter_s *s_filter1;
	ctsvc_composite_filter_s *s_filter2;
	contacts_filter_h new_filter;

	RETV_IF(NULL == filter1 || NULL == filter2, CONTACTS_ERROR_INVALID_PARAMETER);
	s_filter1 = (ctsvc_composite_filter_s*)filter1;
	s_filter2 = (ctsvc_composite_filter_s*)filter2;

	RETVM_IF(g_slist_length(s_filter1->filter_ops) != (g_slist_length(s_filter1->filters)),
			CONTACTS_ERROR_INVALID_PARAMETER, "Please check the operator of filter");
	RETVM_IF(STRING_EQUAL != strcmp(s_filter1->view_uri, s_filter2->view_uri),
			CONTACTS_ERROR_INVALID_PARAMETER,
			"The filter view_uri is different (filter1:%s, filter2:%s)",
			s_filter1->view_uri, s_filter2->view_uri);

	ret = ctsvc_filter_clone(filter2, &new_filter);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_filter_clone is Fail(%d)", ret);
	s_filter1->filters = g_slist_append(s_filter1->filters, new_filter);

	return CONTACTS_ERROR_NONE;
}

static int _filter_create_attribute(ctsvc_composite_filter_s *com_filter,
		unsigned int property_id,
		int match,
		int filter_type,
		ctsvc_attribute_filter_s **out_filter)
{
	ctsvc_attribute_filter_s *filter;
	int type;
	bool find = false;

	RETVM_IF(g_slist_length(com_filter->filter_ops) != g_slist_length(com_filter->filters),
			CONTACTS_ERROR_INVALID_PARAMETER, "Please check the operator of filter");

	find = _filter_property_check(com_filter->properties, com_filter->property_count, property_id, &type);
	RETVM_IF(false == find, CONTACTS_ERROR_INVALID_PARAMETER,
			"property_id(%d) is not supported on view_uri(%s)", property_id, com_filter->view_uri);

	if (type == CTSVC_VIEW_DATA_TYPE_INT && CTSVC_FILTER_INT != filter_type) {
		//LCOV_EXCL_START
	ERR("use contacts_filter_add_int() (%d)", filter_type);
		return CONTACTS_ERROR_INVALID_PARAMETER;
		//LCOV_EXCL_STOP
	}

	if (type == CTSVC_VIEW_DATA_TYPE_STR && CTSVC_FILTER_STR != filter_type) {
		//LCOV_EXCL_START
	ERR("use contacts_filter_add_str() (%d)", filter_type);
		return CONTACTS_ERROR_INVALID_PARAMETER;
		//LCOV_EXCL_STOP
	}

	if (type == CTSVC_VIEW_DATA_TYPE_BOOL && CTSVC_FILTER_BOOL != filter_type) {
		//LCOV_EXCL_START
	ERR("use contacts_filter_add_bool() (%d)", filter_type);
		return CONTACTS_ERROR_INVALID_PARAMETER;
		//LCOV_EXCL_STOP
	}

	if (type == CTSVC_VIEW_DATA_TYPE_LLI && CTSVC_FILTER_LLI != filter_type) {
		//LCOV_EXCL_START
	ERR("use contacts_filter_add_lli() (%d)", filter_type);
		return CONTACTS_ERROR_INVALID_PARAMETER;
		//LCOV_EXCL_STOP
	}

	if (type == CTSVC_VIEW_DATA_TYPE_DOUBLE && CTSVC_FILTER_DOUBLE != filter_type) {
		//LCOV_EXCL_START
	ERR("use contacts_filter_add_double() (%d)", filter_type);
		return CONTACTS_ERROR_INVALID_PARAMETER;
		//LCOV_EXCL_STOP
	}

	filter = calloc(1, sizeof(ctsvc_attribute_filter_s));
	if (NULL == filter) {
		//LCOV_EXCL_START
		ERR("calloc() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		//LCOV_EXCL_STOP
	}
	filter->filter_type = filter_type;
	filter->property_id = property_id;
	filter->match = match;

	com_filter->filters = g_slist_append(com_filter->filters, filter);
	*out_filter = filter;
	return CONTACTS_ERROR_NONE;
}

API int contacts_filter_add_str(contacts_filter_h filter, unsigned int property_id,
		contacts_match_str_flag_e match, const char *match_value)
{
	ctsvc_composite_filter_s *com_filter;
	ctsvc_attribute_filter_s *str_filter;
	int ret;

	RETV_IF(NULL == filter || NULL == match_value, CONTACTS_ERROR_INVALID_PARAMETER);

	com_filter = (ctsvc_composite_filter_s*)filter;
	ret = _filter_create_attribute(com_filter, property_id, match,
			CTSVC_FILTER_STR, &str_filter);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret,
			"The parameter is not proper (view_uri:%s, property_id:0x%x, match:%d)",
			com_filter->view_uri, property_id, match);

	str_filter->value.s = SAFE_STRDUP(match_value);
	return CONTACTS_ERROR_NONE;
}

API int contacts_filter_add_int(contacts_filter_h filter, unsigned int property_id,
		contacts_match_int_flag_e match, int match_value)
{
	ctsvc_composite_filter_s *com_filter;
	ctsvc_attribute_filter_s *int_filter;
	int ret;

	RETV_IF(NULL == filter, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF((CTSVC_PROPERTY_PHONELOG_SIM_SLOT_NO == property_id
					|| CTSVC_PROPERTY_PHONELOG_STAT_SIM_SLOT_NO == property_id)
			&& (CONTACTS_MATCH_GREATER_THAN <= match
					&& match <= CONTACTS_MATCH_LESS_THAN_OR_EQUAL),
			CONTACTS_ERROR_INVALID_PARAMETER, "Not support this condition");

	com_filter = (ctsvc_composite_filter_s*)filter;
	ret = _filter_create_attribute(com_filter, property_id, match,
			CTSVC_FILTER_INT, &int_filter);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret,
			"The parameter is not proper (view_uri:%s, property_id:0x%x, match:%d)",
			com_filter->view_uri, property_id, match);

	int_filter->value.i = match_value;

	return CONTACTS_ERROR_NONE;
}

API int contacts_filter_add_lli(contacts_filter_h filter, unsigned int property_id,
		contacts_match_int_flag_e match, long long int match_value)
{
	ctsvc_composite_filter_s *com_filter;
	ctsvc_attribute_filter_s *lli_filter;
	int ret;

	RETV_IF(NULL == filter, CONTACTS_ERROR_INVALID_PARAMETER);

	com_filter = (ctsvc_composite_filter_s*)filter;
	ret = _filter_create_attribute(com_filter, property_id, match,
			CTSVC_FILTER_LLI, &lli_filter);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret,
			"The parameter is not proper (view_uri:, property_id:0x%x, match:%d)",
			property_id, match);

	lli_filter->value.l = match_value;

	return CONTACTS_ERROR_NONE;
}

API int contacts_filter_add_double(contacts_filter_h filter, unsigned int property_id,
		contacts_match_int_flag_e match, double match_value)
{
	ctsvc_composite_filter_s *com_filter;
	ctsvc_attribute_filter_s *double_filter;
	int ret;

	RETV_IF(NULL == filter, CONTACTS_ERROR_INVALID_PARAMETER);

	com_filter = (ctsvc_composite_filter_s*)filter;
	ret = _filter_create_attribute(com_filter, property_id, match,
			CTSVC_FILTER_DOUBLE, &double_filter);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret,
			"The parameter is not proper (view_uri:, property_id:0x%x, match:%d)",
			property_id, match);

	double_filter->value.d = match_value;

	return CONTACTS_ERROR_NONE;
}

API int contacts_filter_add_bool(contacts_filter_h filter, unsigned int property_id,
		bool match_value)
{
	ctsvc_composite_filter_s *com_filter;
	ctsvc_attribute_filter_s *bool_filter;
	int ret;

	RETV_IF(NULL == filter, CONTACTS_ERROR_INVALID_PARAMETER);

	com_filter = (ctsvc_composite_filter_s*)filter;
	ret = _filter_create_attribute(com_filter, property_id, 0, CTSVC_FILTER_BOOL,
			&bool_filter);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret,
			"The parameter is not proper (view_uri:, property_id:%d)",
			property_id);

	bool_filter->value.b = match_value;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_composite_filter_destroy(ctsvc_composite_filter_s *com_filter)
{
	CTS_FN_CALL;
	GSList *cursor;

	RETV_IF(NULL == com_filter, CONTACTS_ERROR_INVALID_PARAMETER);

	for (cursor = com_filter->filters; cursor; cursor = cursor->next) {
		ctsvc_filter_s *sub_filter = cursor->data;

		if (sub_filter->filter_type == CTSVC_FILTER_COMPOSITE) {
			__ctsvc_composite_filter_destroy((ctsvc_composite_filter_s*)sub_filter);
		} else {
			ctsvc_attribute_filter_s *attr = (ctsvc_attribute_filter_s*)sub_filter;
			if (attr->filter_type == CTSVC_FILTER_STR)
				free(attr->value.s);
			free(attr);
		}
	}
	g_slist_free(com_filter->filters);
	g_slist_free(com_filter->filter_ops);

	free(com_filter->view_uri);

	free(com_filter);
	return CONTACTS_ERROR_NONE;
}


API int contacts_filter_destroy(contacts_filter_h filter)
{
	RETV_IF(NULL == filter, CONTACTS_ERROR_INVALID_PARAMETER);

	return __ctsvc_composite_filter_destroy((ctsvc_composite_filter_s*)filter);
}

static int __ctsvc_attribute_filter_clone(ctsvc_attribute_filter_s *src,
		ctsvc_attribute_filter_s **dest)
{
	ctsvc_attribute_filter_s *out;
	out = calloc(1, sizeof(ctsvc_attribute_filter_s));
	if (NULL == out) {
		//LCOV_EXCL_START
	ERR("calloc() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		//LCOV_EXCL_STOP
	}
	out->filter_type = src->filter_type;
	out->property_id = src->property_id;
	out->match = src->match;
	if (src->filter_type == CTSVC_FILTER_STR)
		out->value.s = SAFE_STRDUP(src->value.s);
	else if (src->filter_type == CTSVC_FILTER_INT)
		out->value.i = src->value.i;
	else if (src->filter_type == CTSVC_FILTER_BOOL)
		out->value.b = src->value.b;
	else if (src->filter_type == CTSVC_FILTER_LLI)
		out->value.l = src->value.l;
	else if (src->filter_type == CTSVC_FILTER_DOUBLE)
		out->value.d = src->value.d;
	else
		ERR("unknown type (%d)", src->filter_type);

	*dest = out;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_composite_filter_clone(ctsvc_composite_filter_s * filter,
		ctsvc_composite_filter_s **out_filter)
{
	GSList *cursor;
	ctsvc_composite_filter_s *out;

	RETV_IF(NULL == filter, CONTACTS_ERROR_INVALID_PARAMETER);
	contacts_filter_create(filter->view_uri, (contacts_filter_h*)&out);

	for (cursor = filter->filters; cursor; cursor = cursor->next) {
		ctsvc_filter_s *src = cursor->data;
		ctsvc_filter_s *dest = NULL;

		if (src->filter_type == CTSVC_FILTER_COMPOSITE) {
			__ctsvc_composite_filter_clone((ctsvc_composite_filter_s*)src,
					(ctsvc_composite_filter_s **)&dest);
		} else {
			__ctsvc_attribute_filter_clone((ctsvc_attribute_filter_s*)src,
					(ctsvc_attribute_filter_s **)&dest);
		}

		out->filters = g_slist_append(out->filters, dest);
	}

	out->filter_ops = g_slist_copy(filter->filter_ops);
	*out_filter = out;

	return CONTACTS_ERROR_NONE;
}

int ctsvc_filter_clone(contacts_filter_h filter, contacts_filter_h *out_filter)
{
	RETV_IF(NULL == filter || NULL == out_filter, CONTACTS_ERROR_INVALID_PARAMETER);

	return __ctsvc_composite_filter_clone((ctsvc_composite_filter_s*)filter,
			(ctsvc_composite_filter_s **)out_filter);
}

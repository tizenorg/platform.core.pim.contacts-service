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
#include "ctsvc_query.h"
#include "ctsvc_filter.h"

typedef enum {
	QUERY_SORTKEY,
	QUERY_PROJECTION,
}query_property_type_e;

static bool __ctsvc_query_property_check(const property_info_s *properties,
		int count, query_property_type_e property_type, unsigned int property_id)
{
	int i;

	for (i=0;i<count;i++) {
		property_info_s *p = (property_info_s*)&(properties[i]);
		if (property_id == p->property_id) {
			if (property_type == QUERY_PROJECTION) {
				if (p->property_type == CTSVC_SEARCH_PROPERTY_ALL || p->property_type == CTSVC_SEARCH_PROPERTY_PROJECTION)
					return true;
				else
					return false;
			}
			else
				return true;
		}
	}
	return false;
}

API int contacts_query_create( const char* view_uri, contacts_query_h* out_query )
{
	ctsvc_query_s *query;

	RETV_IF(NULL == out_query, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_query = NULL;

	RETV_IF(NULL == view_uri || NULL == out_query, CONTACTS_ERROR_INVALID_PARAMETER);

	query = (ctsvc_query_s *)calloc(1, sizeof(ctsvc_query_s));
	RETV_IF(NULL == query, CONTACTS_ERROR_OUT_OF_MEMORY);

	query->view_uri = strdup(view_uri);
	query->properties = (property_info_s *)ctsvc_view_get_all_property_infos(view_uri, &query->property_count);
	*out_query = (contacts_query_h)query;

	return CONTACTS_ERROR_NONE;
}

API int contacts_query_set_projection(contacts_query_h query, unsigned int property_ids[], int count)
{
	ctsvc_query_s *query_s;
	int i;
	bool find;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	query_s = (ctsvc_query_s *)query;

	for (i=0;i<count;i++) {
		find = __ctsvc_query_property_check(query_s->properties, query_s->property_count, QUERY_PROJECTION, property_ids[i]);
		RETVM_IF(false == find, CONTACTS_ERROR_INVALID_PARAMETER,
					"Invalid parameter : property_id(%d) is not supported on view_uri(%s)", property_ids[i], query_s->view_uri);
	}
	if (query_s->projection)
		free(query_s->projection);

	query_s->projection = calloc(count, sizeof(unsigned int));
	memcpy(query_s->projection, property_ids, sizeof(unsigned int) * count);
	query_s->projection_count = count;

	return CONTACTS_ERROR_NONE;
}

API int contacts_query_set_filter(contacts_query_h query, contacts_filter_h filter)
{
	int ret;
	ctsvc_query_s *s_query;
	contacts_filter_h new_filter;

	RETV_IF(NULL == query || NULL == filter, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s *)query;

	ret = ctsvc_filter_clone(filter, &new_filter);
	RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "ctsvc_filter_clone Fail(%d)", ret);
	s_query->filter = (ctsvc_composite_filter_s*)new_filter;

	return CONTACTS_ERROR_NONE;
}

API int contacts_query_set_sort(contacts_query_h query, unsigned int property_id, bool asc)
{
	ctsvc_query_s *query_s;
	bool find = false;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	query_s = (ctsvc_query_s *)query;

	find = __ctsvc_query_property_check(query_s->properties, query_s->property_count, QUERY_SORTKEY, property_id);
	RETVM_IF(false == find, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalid parameter : property_id(%d) is not supported on view_uri(%s)", property_id, query_s->view_uri);
	query_s->sort_property_id = property_id;
	query_s->sort_asc = asc;

	return CONTACTS_ERROR_NONE;
}

API int contacts_query_destroy( contacts_query_h query )
{
	ctsvc_query_s *s_query;
	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s *)query;

	if (s_query->filter)
		contacts_filter_destroy((contacts_filter_h)s_query->filter);

	free(s_query->projection);
	free(s_query->view_uri);
	free(s_query);

	return CONTACTS_ERROR_NONE;
}

API int contacts_query_set_distinct(contacts_query_h query, bool set)
{
	ctsvc_query_s *query_s;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	query_s = (ctsvc_query_s *)query;
	query_s->distinct = set;

	return CONTACTS_ERROR_NONE;
}


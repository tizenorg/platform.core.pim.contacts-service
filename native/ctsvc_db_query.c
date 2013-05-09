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
#include "ctsvc_sqlite.h"
#include "ctsvc_schema.h"
#include "ctsvc_list.h"
#include "ctsvc_record.h"
#include "ctsvc_utils.h"
#include "ctsvc_normalize.h"
#include "ctsvc_db_init.h"
#include "ctsvc_view.h"
#include "ctsvc_inotify.h"
#include "ctsvc_localize.h"
#include "ctsvc_setting.h"

#include "ctsvc_db_plugin_person_helper.h"

typedef enum {
	QUERY_SORTKEY,
	QUERY_FILTER,
	QUERY_PROJECTION,
}db_query_property_type_e;

static const char * __ctsvc_db_get_property_field_name(const property_info_s *properties,
		int count, db_query_property_type_e property_type, unsigned int property_id)
{
	int i;

	for (i=0;i<count;i++) {
		property_info_s *p = (property_info_s*)&(properties[i]);
		if (property_id == p->property_id) {
			if (p->fields) {
				if (property_type == QUERY_PROJECTION) {
					if (p->property_type == CTSVC_SEARCH_PROPERTY_PROJECTION || p->property_type == CTSVC_SEARCH_PROPERTY_ALL)
						return p->fields;
				}
				else if (property_type == QUERY_FILTER) {
					if (p->property_type == CTSVC_SEARCH_PROPERTY_FILTER || p->property_type == CTSVC_SEARCH_PROPERTY_ALL)
						return p->fields;
				}
				else if (property_type == QUERY_SORTKEY) {
					return p->fields;
				}
				return NULL;
			}
			/*
			else if (property_id == CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX) {
				if (property_type != QUERY_PROJECTION)
					return NULL;
				const char* temp = ctsvc_get_display_column();
				// snprintf(temp, sizeof(temp), "_NORMALIZE_INDEX_(%s)", ctsvc_get_display_column());
				return "_NORMALIZE_INDEX_"temp;
			}
			*/
			else
				return ctsvc_get_display_column();
		}
	}
	return NULL;
}

static inline int __ctsvc_db_get_property_type(const property_info_s *properties,
		int count, unsigned int property_id)
{
	int i;
	for (i=0;i<count;i++) {
		property_info_s *p = (property_info_s*)&(properties[i]);
		if (property_id == p->property_id) {
			return (property_id & CTSVC_VIEW_DATA_TYPE_MASK);
		}
	}
	return -1;
}

static inline int __ctsvc_db_create_int_condition(ctsvc_composite_filter_s *com_filter,
		ctsvc_attribute_filter_s *filter, char **condition )
{
	const char *field_name;
	char out_cond[CTS_SQL_MAX_LEN] = {0};

	field_name = __ctsvc_db_get_property_field_name(com_filter->properties,
							com_filter->property_count, QUERY_FILTER, filter->property_id);
	RETVM_IF(NULL == field_name, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property id(%d)", filter->property_id);

	switch(filter->match) {
	case CONTACTS_MATCH_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s = %d", field_name, filter->value.i);
		break;
	case CONTACTS_MATCH_GREATER_THAN:
		snprintf(out_cond, sizeof(out_cond), "%s > %d", field_name, filter->value.i);
		break;
	case CONTACTS_MATCH_GREATER_THAN_OR_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s >= %d", field_name, filter->value.i);
		break;
	case CONTACTS_MATCH_LESS_THAN:
		snprintf(out_cond, sizeof(out_cond), "%s < %d", field_name, filter->value.i);
		break;
	case CONTACTS_MATCH_LESS_THAN_OR_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s <= %d", field_name, filter->value.i);
		break;
	case CONTACTS_MATCH_NOT_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s <> %d", field_name, filter->value.i);
		break;
	case CONTACTS_MATCH_NONE:
		snprintf(out_cond, sizeof(out_cond), "%s IS NULL", field_name);
		break;
	default :
		CTS_ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	*condition = strdup(out_cond);
	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_db_create_double_condition(ctsvc_composite_filter_s *com_filter,
		ctsvc_attribute_filter_s *filter, char **condition )
{
	const char *field_name;
	char out_cond[CTS_SQL_MAX_LEN] = {0};

	field_name = __ctsvc_db_get_property_field_name(com_filter->properties,
							com_filter->property_count, QUERY_FILTER, filter->property_id);
	RETVM_IF(NULL == field_name, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property id(%d)", filter->property_id);

	switch(filter->match) {
	case CONTACTS_MATCH_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s = %lf", field_name, filter->value.d);
		break;
	case CONTACTS_MATCH_GREATER_THAN:
		snprintf(out_cond, sizeof(out_cond), "%s > %lf", field_name, filter->value.d);
		break;
	case CONTACTS_MATCH_GREATER_THAN_OR_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s >= %lf", field_name, filter->value.d);
		break;
	case CONTACTS_MATCH_LESS_THAN:
		snprintf(out_cond, sizeof(out_cond), "%s < %lf", field_name, filter->value.d);
		break;
	case CONTACTS_MATCH_LESS_THAN_OR_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s <= %lf", field_name, filter->value.d);
		break;
	case CONTACTS_MATCH_NOT_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s <> %lf", field_name, filter->value.d);
		break;
	case CONTACTS_MATCH_NONE:
		snprintf(out_cond, sizeof(out_cond), "%s IS NULL", field_name);
		break;
	default :
		CTS_ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	*condition = strdup(out_cond);
	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_db_create_lli_condition(ctsvc_composite_filter_s *com_filter,
		ctsvc_attribute_filter_s *filter, char **condition )
{
	const char *field_name;
	char out_cond[CTS_SQL_MAX_LEN] = {0};

	field_name = __ctsvc_db_get_property_field_name(com_filter->properties,
							com_filter->property_count, QUERY_FILTER, filter->property_id);
	RETVM_IF(NULL == field_name, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property id(%d)", filter->property_id);

	switch(filter->match) {
	case CONTACTS_MATCH_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s = %lld", field_name, filter->value.l);
		break;
	case CONTACTS_MATCH_GREATER_THAN:
		snprintf(out_cond, sizeof(out_cond), "%s > %lld", field_name, filter->value.l);
		break;
	case CONTACTS_MATCH_GREATER_THAN_OR_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s >= %lld", field_name, filter->value.l);
		break;
	case CONTACTS_MATCH_LESS_THAN:
		snprintf(out_cond, sizeof(out_cond), "%s < %lld", field_name, filter->value.l);
		break;
	case CONTACTS_MATCH_LESS_THAN_OR_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s <= %lld", field_name, filter->value.l);
		break;
	case CONTACTS_MATCH_NOT_EQUAL:
		snprintf(out_cond, sizeof(out_cond), "%s <> %lld", field_name, filter->value.l);
		break;
	case CONTACTS_MATCH_NONE:
		snprintf(out_cond, sizeof(out_cond), "%s IS NULL", field_name);
		break;
	default :
		CTS_ERR("Invalid parameter : int match rule(%d) is not supported", filter->match);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	*condition = strdup(out_cond);
	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_db_create_str_condition(ctsvc_composite_filter_s *com_filter,
		ctsvc_attribute_filter_s *filter, char **condition, GSList **bind_text)
{
	int ret;
	const char *field_name;
	char out_cond[CTS_SQL_MAX_LEN] = {0};

	*condition = NULL;

	field_name = __ctsvc_db_get_property_field_name(com_filter->properties,
							com_filter->property_count, QUERY_FILTER, filter->property_id);
	RETVM_IF(NULL == field_name, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property id(%d)", filter->property_id);

	if (filter->property_id == CTSVC_PROPERTY_NUMBER_NUMBER_FILTER)
		filter->match = CONTACTS_MATCH_EXACTLY;

	switch(filter->match) {
	case CONTACTS_MATCH_EXACTLY:
		snprintf(out_cond, sizeof(out_cond), "%s = ?", field_name);
		break;
	case CONTACTS_MATCH_FULLSTRING:
		snprintf(out_cond, sizeof(out_cond), "%s LIKE ?", field_name);
		break;
	case CONTACTS_MATCH_CONTAINS:
		snprintf(out_cond, sizeof(out_cond), "%s LIKE ('%%' || ? || '%%')", field_name);
		break;
	case CONTACTS_MATCH_STARTSWITH:
		snprintf(out_cond, sizeof(out_cond), "%s LIKE ( ? || '%%')", field_name);
		break;
	case CONTACTS_MATCH_ENDSWITH:
		snprintf(out_cond, sizeof(out_cond), "%s LIKE ('%%' || ?)", field_name);
		break;
	case CONTACTS_MATCH_EXISTS:
		snprintf(out_cond, sizeof(out_cond), "%s IS NOT NULL", field_name);
		break;
	default :
		CTS_ERR("Invalid parameter : int match rule (%d) is not supported", filter->match);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	if (filter->value.s) {
		if (filter->property_id == CTSVC_PROPERTY_NUMBER_NUMBER_FILTER) {
			char dest[strlen(filter->value.s)+1];
			ret = ctsvc_normalize_number(filter->value.s, dest, sizeof(dest), ctsvc_get_phonenumber_min_match_digit());
			if (CONTACTS_ERROR_NONE == ret)
				*bind_text = g_slist_append(*bind_text, strdup(dest));
			else
				return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		else
			*bind_text = g_slist_append(*bind_text, strdup(filter->value.s));
	}
	*condition = strdup(out_cond);
	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_db_create_bool_condition(ctsvc_composite_filter_s *com_filter,
		ctsvc_attribute_filter_s *filter, char **condition )
{
	const char *field_name;
	char out_cond[CTS_SQL_MAX_LEN] = {0};

	field_name = __ctsvc_db_get_property_field_name(com_filter->properties,
							com_filter->property_count, QUERY_FILTER, filter->property_id);
	RETVM_IF(NULL == field_name, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property id(%d)", filter->property_id);

	snprintf(out_cond, sizeof(out_cond), "%s = %d", field_name, filter->value.b?1:0);
	*condition = strdup(out_cond);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_create_attribute_condition(ctsvc_composite_filter_s *com_filter,
		ctsvc_attribute_filter_s *filter, char **condition, GSList **bind_text)
{
	int ret;
	char *cond = NULL;

	RETV_IF(NULL == filter, CONTACTS_ERROR_INVALID_PARAMETER);

	switch (filter->filter_type) {
	case CTSVC_FILTER_INT:
		ret = __ctsvc_db_create_int_condition(com_filter, filter, &cond);
		break;
	case CTSVC_FILTER_BOOL:
		ret = __ctsvc_db_create_bool_condition(com_filter, filter, &cond);
		break;
	case CTSVC_FILTER_STR:
		ret = __ctsvc_db_create_str_condition(com_filter, filter, &cond, bind_text);
		break;
	case CTSVC_FILTER_LLI:
		ret = __ctsvc_db_create_lli_condition(com_filter, filter, &cond);
		break;
	case CTSVC_FILTER_DOUBLE:
		ret = __ctsvc_db_create_double_condition(com_filter, filter, &cond);
		break;
	default :
		CTS_ERR("The filter type is not supported (%d)", filter->filter_type);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	if (CONTACTS_ERROR_NONE == ret)
		*condition = cond;

	return ret;
}

static inline int __ctsvc_db_create_composite_condition(ctsvc_composite_filter_s *com_filter,
		char **condition, GSList **bind_text)
{
	RETV_IF(NULL == com_filter, CONTACTS_ERROR_INVALID_PARAMETER);

	int len;
	char *cond, out_cond[CTS_SQL_MAX_LEN] = {0};
	GSList *cursor_filter;
	GSList *cursor_ops;
	ctsvc_filter_s *filter;
	contacts_filter_operator_e op;
	GSList *bind;
	GSList *filters = com_filter->filters;
	GSList *ops = com_filter->filter_ops;

	RETV_IF(NULL == filters, CONTACTS_ERROR_INVALID_PARAMETER);

	cond = NULL;
	bind = NULL;
	filter = (ctsvc_filter_s *)filters->data;
	if (filter->filter_type == CTSVC_FILTER_COMPOSITE)
		__ctsvc_db_create_composite_condition((ctsvc_composite_filter_s *)filter, &cond, bind_text);
	else
		__ctsvc_db_create_attribute_condition(com_filter, (ctsvc_attribute_filter_s*)filter, &cond, bind_text);

	cursor_filter = filters->next;

	len = 0;
	len = snprintf(out_cond, sizeof(out_cond), "(%s)", cond);
	free(cond);

	for(cursor_ops=ops; cursor_ops && cursor_filter; cursor_filter=cursor_filter->next, cursor_ops=cursor_ops->next) {
		cond = NULL;
		bind = NULL;

		filter = (ctsvc_filter_s *)cursor_filter->data;
		if (filter->filter_type == CTSVC_FILTER_COMPOSITE)
			__ctsvc_db_create_composite_condition((ctsvc_composite_filter_s *)filter, &cond, &bind);
		else
			__ctsvc_db_create_attribute_condition(com_filter, (ctsvc_attribute_filter_s*)filter, &cond, &bind);

		if (cond) {
			op = (contacts_filter_operator_e)cursor_ops->data;
			if (op == CONTACTS_FILTER_OPERATOR_AND)
				len += snprintf(out_cond+len, sizeof(out_cond)-len, " AND (%s)", cond);
			else
				len += snprintf(out_cond+len, sizeof(out_cond)-len, " OR (%s)", cond);
			if (bind)
				*bind_text = g_slist_concat(*bind_text, bind);
			free(cond);
		}
	}
	*condition = strdup(out_cond);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_db_update_record_with_set_query(const char *set, GSList *bind_text, const char *table, int id)
{
	int ret = CONTACTS_ERROR_NONE;
	char query[CTS_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	GSList *cursor = NULL;

	snprintf(query, sizeof(query), "UPDATE %s SET %s WHERE id = %d", table, set, id);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		CTS_ERR("DB error : cts_query_prepare() Failed");
		return CONTACTS_ERROR_DB;
	}

	if (bind_text) {
		int i = 0;
		for (cursor=bind_text,i=1;cursor;cursor=cursor->next,i++) {
			const char *text = cursor->data;
			if (text && *text)
				cts_stmt_bind_text(stmt, i, text);
		}
	}
	ret = cts_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return ret;
	}
	cts_stmt_finalize(stmt);
	return ret;
}

int ctsvc_db_create_set_query(contacts_record_h record, char **set, GSList **bind_text)
{
	ctsvc_record_s *s_record;
	int i = 0;
	const property_info_s* property_info = NULL;
	unsigned int property_info_count = 0;
	char out_set[CTS_SQL_MAX_LEN] = {0};
	int len = 0;
	const char *field_name;
	int ret = CONTACTS_ERROR_NONE;

	RETV_IF(record == NULL, CONTACTS_ERROR_INVALID_PARAMETER);

	s_record = (ctsvc_record_s *)record;
	if (0 == s_record->property_max_count || NULL == s_record->properties_flags) {
		CTS_ERR("record don't have properties");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	property_info = ctsvc_view_get_all_property_infos(s_record->view_uri, &property_info_count);

	for(i=0;i<property_info_count;i++) {
		if (ctsvc_record_check_property_flag(s_record, property_info[i].property_id, CTSVC_PROPERTY_FLAG_DIRTY)) {
			field_name = property_info[i].fields;
			if (NULL == field_name)
				continue;

			if (CTSVC_VIEW_CHECK_DATA_TYPE(property_info[i].property_id, CTSVC_VIEW_DATA_TYPE_BOOL)) {
				bool tmp = false;
				ret = contacts_record_get_bool(record,property_info[i].property_id, &tmp);
				if (ret != CONTACTS_ERROR_NONE)
					continue;
				if (len != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d", field_name, tmp);
			}
			else if (CTSVC_VIEW_CHECK_DATA_TYPE(property_info[i].property_id, CTSVC_VIEW_DATA_TYPE_INT)) {
				int tmp = 0;
				ret = contacts_record_get_int(record,property_info[i].property_id, &tmp);
				if (ret != CONTACTS_ERROR_NONE)
					continue;
				if (len != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%d", field_name, tmp);
			}
			else if (CTSVC_VIEW_CHECK_DATA_TYPE(property_info[i].property_id, CTSVC_VIEW_DATA_TYPE_LLI)) {
				long long int tmp = 0;
				ret = contacts_record_get_lli(record, property_info[i].property_id, &tmp);
				if (ret != CONTACTS_ERROR_NONE)
					continue;
				if (len != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%lld", field_name,tmp);
			}
			else if (CTSVC_VIEW_CHECK_DATA_TYPE(property_info[i].property_id, CTSVC_VIEW_DATA_TYPE_STR)) {
				char *tmp = NULL;
				ret = contacts_record_get_str_p(record,property_info[i].property_id, &tmp);
				if (ret != CONTACTS_ERROR_NONE)
					continue;
				if (len != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=?", field_name);
				*bind_text = g_slist_append(*bind_text, strdup(SAFE_STR(tmp)));
			}
			else if (CTSVC_VIEW_CHECK_DATA_TYPE(property_info[i].property_id, CTSVC_VIEW_DATA_TYPE_DOUBLE)) {
				double tmp = 0;
				ret = contacts_record_get_double(record, property_info[i].property_id, &tmp);
				if (ret != CONTACTS_ERROR_NONE)
					continue;
				if (len != 0)
					len += snprintf(out_set+len, sizeof(out_set)-len, ", ");
				len += snprintf(out_set+len, sizeof(out_set)-len, "%s=%lf", field_name, tmp);
			}
		}
	}
	*set = strdup(out_set);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_create_projection(const property_info_s *properties, int ids_count,
		unsigned int *projections, int pro_count, char **projection)
{
	bool first;
	int i;
	int len;
	const char *field_name = NULL;
	char out_projection[CTS_SQL_MAX_LEN] = {0};
	char temp[CTS_SQL_MAX_LEN] = {0};

	len = 0;
	first = true;
	if (0 < pro_count) {
		for (i=0;i<pro_count;i++) {
			if (projections[i] == CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX) {
				snprintf(temp, sizeof(temp), "_NORMALIZE_INDEX_(%s)", ctsvc_get_sort_name_column());
				field_name = temp;
			}
			else
				field_name = __ctsvc_db_get_property_field_name(properties, ids_count, QUERY_PROJECTION, projections[i]);

			if (field_name) {
				if (first) {
					len += snprintf(out_projection+len, sizeof(out_projection)-len, "%s", field_name);
					first = false;
				}
				else
					len += snprintf(out_projection+len, sizeof(out_projection)-len, ", %s", field_name);
			}
		}
	}
	else {
		for (i=0;i<ids_count;i++) {
			if (CTSVC_VIEW_DATA_TYPE_REC == (properties[i].property_id & CTSVC_VIEW_DATA_TYPE_MASK))
				continue;
			if (properties[i].fields)
				field_name = properties[i].fields;
			else if (properties[i].property_id == CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX) {
				snprintf(temp, sizeof(temp), "_NORMALIZE_INDEX_(%s)", ctsvc_get_sort_name_column());
				field_name = temp;
				CTS_DBG("field_name : %s", field_name);
			}
			else
				field_name = ctsvc_get_display_column();

			if (first) {
				len += snprintf(out_projection+len, sizeof(out_projection)-len, "%s", field_name);
				first = false;
			}
			else
				len += snprintf(out_projection+len, sizeof(out_projection)-len, ", %s", field_name);
		}
	}

	*projection = strdup(out_projection);
	return CONTACTS_ERROR_NONE;
}

static inline bool __ctsvc_db_view_has_display_name(const char *view_uri,
		const property_info_s *properties, int count)
{
	int i;
	if (0 == strcmp(view_uri, _contacts_person_phone_log._uri))
		return false;

	for (i=0;i<count;i++) {
		property_info_s *p = (property_info_s*)&(properties[i]);
		switch (p->property_id) {
		case CTSVC_PROPERTY_PERSON_DISPLAY_NAME:
		case CTSVC_PROPERTY_CONTACT_DISPLAY_NAME:
			return true;
		default:
			break;
		}
	}
	return false;
}

int ctsvc_db_make_get_records_query_stmt(ctsvc_query_s *s_query, int offset, int limit, cts_stmt *stmt)
{
	char query[CTS_SQL_MAX_LEN] = {0};
	int len;
	int ret;
	int i;
	const char *table;
	const char *sortkey = NULL;
	char *condition = NULL;
	char *projection = NULL;
	GSList *bind_text = NULL;
	GSList *cursor;

	ret = ctsvc_db_get_table_name(s_query->view_uri, &table);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "Invalid parameter : view uri (%s)", s_query->view_uri);

	ret = __ctsvc_db_create_projection(s_query->properties, s_query->property_count,
								s_query->projection, s_query->projection_count, &projection);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_create_projection is failed(%d)", ret);
		return ret;
	}

	if (s_query->distinct)
		len = snprintf(query, sizeof(query), "SELECT DISTINCT %s FROM %s", projection, table);
	else
		len = snprintf(query, sizeof(query), "SELECT %s FROM %s", projection, table);

	if (s_query->filter) {
		ret = __ctsvc_db_create_composite_condition(s_query->filter, &condition, &bind_text);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("__ctsvc_db_create_composite_condition is failed(%d)", ret);
			free(projection);
			return ret;
		}
		if (condition && *condition)
			len += snprintf(query+len, sizeof(query)-len, " WHERE %s", condition);
	}

	if (__ctsvc_db_view_has_display_name(s_query->view_uri, s_query->properties, s_query->property_count))
		sortkey = ctsvc_get_sort_column();

	if (s_query->sort_property_id) {
		const char *field_name;

		switch(s_query->sort_property_id) {
		case CTSVC_PROPERTY_PERSON_DISPLAY_NAME:
		case CTSVC_PROPERTY_CONTACT_DISPLAY_NAME:
			len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s %s", sortkey, s_query->sort_asc?"":"DESC");
			break;
		default :
			field_name = __ctsvc_db_get_property_field_name(s_query->properties,
								s_query->property_count, QUERY_SORTKEY, s_query->sort_property_id);
			if (field_name) {
				len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s", field_name);
//				if (sortkey)
//					len += snprintf(query+len, sizeof(query)-len, ", %s", sortkey);
				if (false == s_query->sort_asc)
					len += snprintf(query+len, sizeof(query)-len, " DESC");
			}
			else if (sortkey)
				len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s", sortkey);
			break;
		}
	}
	else if (sortkey)
		len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s", sortkey);
	else if (0 == strcmp(s_query->view_uri, CTSVC_VIEW_URI_GROUP))
		len += snprintf(query+len, sizeof(query)-len, " ORDER BY group_prio");

	if (0 < limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	free(condition);
	free(projection);

	CTS_DBG("query : %s", query);
	*stmt = cts_query_prepare(query);
	if(NULL == *stmt) {
		CTS_ERR("DB error : cts_query_prepare() Failed");
		for (cursor=bind_text;cursor;cursor=cursor->next)
			free(cursor->data);
		g_slist_free(bind_text);
		return CONTACTS_ERROR_DB;
	}

	for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
		cts_stmt_bind_copy_text(*stmt, i, cursor->data, strlen(cursor->data));

	for (cursor=bind_text;cursor;cursor=cursor->next)
		free(cursor->data);
	g_slist_free(bind_text);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_records_with_query_exec(ctsvc_query_s *query, int offset,
		int limit, contacts_list_h* out_list )
{
	int ret;
	int i;
	int type;
	cts_stmt stmt = NULL;
	contacts_list_h list = NULL;

	ret = ctsvc_db_make_get_records_query_stmt(query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "DB error : ctsvc_db_make_get_records_query_stmt(%d)", ret);

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		int field_count;
		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		contacts_record_create(query->view_uri, (contacts_record_h*)&record);

		if (0 == query->projection_count)
			field_count = query->property_count;
		else {
			field_count = query->projection_count;

			if( CONTACTS_ERROR_NONE != ctsvc_record_set_projection_flags(record, query->projection, query->projection_count, query->property_count) )
			{
				ASSERT_NOT_REACHED("To set projection is failed.\n");
			}
		}

		for(i=0;i<field_count;i++) {
			int property_id;
			if (0 == query->projection_count)
				property_id = query->properties[i].property_id;
			else {
				property_id = query->projection[i];
			}
			type = __ctsvc_db_get_property_type(query->properties, query->property_count, property_id);
			if (type == CTSVC_VIEW_DATA_TYPE_INT)
				ctsvc_record_set_int(record, property_id, ctsvc_stmt_get_int(stmt, i));
			else if (type == CTSVC_VIEW_DATA_TYPE_STR)
				ctsvc_record_set_str(record, property_id, ctsvc_stmt_get_text(stmt, i));
			else if (type == CTSVC_VIEW_DATA_TYPE_BOOL)
				ctsvc_record_set_bool(record, property_id, (ctsvc_stmt_get_int(stmt, i)?true:false));
			else if (type == CTSVC_VIEW_DATA_TYPE_LLI)
				ctsvc_record_set_lli(record, property_id, ctsvc_stmt_get_int64(stmt, i));
			else if (type == CTSVC_VIEW_DATA_TYPE_DOUBLE)
				ctsvc_record_set_double(record, property_id, ctsvc_stmt_get_dbl(stmt, i));
			else
				CTS_ERR("DB error : unknown type (%d)", type);
		}
		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_all_records_exec(const char *view_uri, const property_info_s* properties, int ids_count,
		const char *projection, int offset,	int limit, contacts_list_h* out_list )
{
	char query[CTS_SQL_MAX_LEN] = {0};
	const char *table;
	int len;
	int ret;
	int i;
	int type;
	cts_stmt stmt = NULL;
	contacts_list_h list = NULL;
	ctsvc_record_type_e r_type;
	const char *sortkey;

	ret = ctsvc_db_get_table_name(view_uri, &table);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "Invalid parameter : view uri (%s)", view_uri);

	len = snprintf(query, sizeof(query), "SELECT %s FROM %s", projection, table);

	if (__ctsvc_db_view_has_display_name(view_uri, properties, ids_count)) {
		sortkey = ctsvc_get_sort_column();
		len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s", sortkey);
	} else if (0 == strcmp(view_uri, CTSVC_VIEW_URI_GROUP))
		len += snprintf(query+len, sizeof(query)-len, " ORDER BY group_prio");

	if (0 < limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB , "DB error : cts_query_prepare() Failed");

	r_type = ctsvc_view_get_record_type(view_uri);

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return CONTACTS_ERROR_NO_DATA;
		}

		contacts_record_create(view_uri, &record);
		for(i=0;i<ids_count;i++) {
			type = (properties[i].property_id & CTSVC_VIEW_DATA_TYPE_MASK);
			if (type == CTSVC_VIEW_DATA_TYPE_INT)
				ctsvc_record_set_int(record, properties[i].property_id, ctsvc_stmt_get_int(stmt, i));
			else if (type == CTSVC_VIEW_DATA_TYPE_STR)
				ctsvc_record_set_str(record, properties[i].property_id, ctsvc_stmt_get_text(stmt, i));
			else if (type == CTSVC_VIEW_DATA_TYPE_BOOL)
				ctsvc_record_set_bool(record, properties[i].property_id, (ctsvc_stmt_get_int(stmt, i)?true:false));
			else if (type == CTSVC_VIEW_DATA_TYPE_LLI)
				ctsvc_record_set_lli(record, properties[i].property_id, ctsvc_stmt_get_int64(stmt, i));
			else if (type == CTSVC_VIEW_DATA_TYPE_DOUBLE)
				ctsvc_record_set_double(record, properties[i].property_id, ctsvc_stmt_get_dbl(stmt, i));
			else
				CTS_ERR("DB error : unknown type (%d)", type);
		}
		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_all_records( const char* view_uri, int offset, int limit, contacts_list_h* out_list )
{
	int ret;
	unsigned int count;
	char *projection;

	const property_info_s *p = ctsvc_view_get_all_property_infos(view_uri, &count);
	ret = __ctsvc_db_create_projection(p, count, NULL, 0, &projection);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_create_projection is failed(%d)", ret);

	ret = __ctsvc_db_get_all_records_exec(view_uri, p, count, projection, offset, limit, out_list);
	free(projection);

	return ret;
}

static inline bool __ctsvc_db_view_can_keyword_search(const char *view_uri)
{
	RETV_IF(NULL == view_uri, false);

	if (0 == strcmp(view_uri, CTSVC_VIEW_URI_PERSON)
		|| 0 == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_CONTACT)
		|| 0 == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP)) {
		return true;
	}
	return false;
}

static int __ctsvc_db_append_search_query(const char *keyword, char *query, int size)
{
	bool emailaddress = false;
	bool phonenumber = false;
	int ret;
	int len = 0, i;
	if (strstr(keyword, "@") != NULL) {
		emailaddress = true;
	}
	else {
		len = strlen(keyword);

		phonenumber = true;
		for(i=0; i<len; i++) {
			if (keyword[i] < '0' || keyword[i] > '9') {
				phonenumber = false;
				break;
			}
		}
	}

	if (emailaddress) {
		ret = snprintf(query, size,
				"(SELECT contact_id FROM %s WHERE %s MATCH 'data:%s*') ",
				CTS_TABLE_SEARCH_INDEX, CTS_TABLE_SEARCH_INDEX,
				keyword );
	}
	else if (phonenumber) {
		char normalized_number[CTSVC_NUMBER_MAX_LEN];

		ctsvc_normalize_number(keyword, normalized_number, CTSVC_NUMBER_MAX_LEN, CTSVC_NUMBER_MAX_LEN -1);

		ret =snprintf(query, size,
				"(SELECT contact_id FROM %s WHERE %s MATCH 'name:%s* OR number:%s* OR  data:%s*' "
					"UNION "
					"SELECT contact_id FROM %s WHERE number LIKE '%%%s%%') ",
				CTS_TABLE_SEARCH_INDEX, CTS_TABLE_SEARCH_INDEX,
				keyword, keyword, keyword, CTS_TABLE_PHONE_LOOKUP, normalized_number );
	}
	else {
		char *normalized_name = NULL;
		ret = ctsvc_normalize_str(keyword, &normalized_name);

		if (CTSVC_LANG_KOREAN == ret) {
			char *chosung = calloc(1, strlen(keyword) * 5);
			char *korean_pattern = calloc(1, strlen(keyword) * 5);

			ctsvc_get_chosung(keyword, chosung, strlen(keyword) * 5 );
			ctsvc_get_korean_search_pattern(keyword, korean_pattern, strlen(keyword) * 5 );
			ret = snprintf(query, size,
					"(SELECT contact_id FROM %s WHERE %s MATCH 'name:%s* OR number:%s* OR  data:%s*' "
						"INTERSECT "
						"SELECT contact_id FROM %s WHERE name GLOB '*%s*') ",
					CTS_TABLE_SEARCH_INDEX, CTS_TABLE_SEARCH_INDEX,
					chosung, keyword, keyword, CTS_TABLE_NAME_LOOKUP, korean_pattern );
			free(chosung);
			free(korean_pattern);
		}
		else if (CTSVC_LANG_JAPANESE == ret){
			char *hiragana = NULL;

			ctsvc_convert_japanese_to_hiragana(keyword, &hiragana);

			ret = snprintf(query, size,
								"(SELECT contact_id FROM %s WHERE %s MATCH 'name:%s* OR number:%s* OR  data:%s*') ",
								CTS_TABLE_SEARCH_INDEX, CTS_TABLE_SEARCH_INDEX,
								hiragana, hiragana, hiragana);
			free(hiragana);
		}
		else if (CONTACTS_ERROR_NONE <= ret) {
			ret = snprintf(query, size,
					"(SELECT contact_id FROM %s WHERE %s MATCH 'name:%s* OR number:%s* OR  data:%s*') ",
					CTS_TABLE_SEARCH_INDEX, CTS_TABLE_SEARCH_INDEX,
					normalized_name, keyword, keyword);
		}
		else {
			ret = snprintf(query, size,
					"(SELECT contact_id FROM %s WHERE %s MATCH 'name:%s* OR number:%s* OR  data:%s*') ",
					CTS_TABLE_SEARCH_INDEX, CTS_TABLE_SEARCH_INDEX,
					keyword, keyword, keyword);
		}
		free(normalized_name);
	}

	return ret;
}

static int __ctsvc_db_search_records_exec(const char *view_uri, const property_info_s* properties,
		int ids_count, const char *projection, const char *keyword, int offset, int limit, contacts_list_h* out_list )
{
	char query[CTS_SQL_MAX_LEN*8] = {0}; // temporarily extend
	const char *table;
	int len;
	int ret;
	int i;
	int type;
	cts_stmt stmt = NULL;
	contacts_list_h list = NULL;
	ctsvc_record_type_e r_type;
	const char *sortkey;

	ret = ctsvc_db_get_table_name(view_uri, &table);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "Invalid parameter : view uri (%s)", view_uri);

	if (0 == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_CONTACT)
			|| 0 == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP)) {
		len = snprintf(query, sizeof(query), "SELECT %s FROM %s "
					"WHERE contact_id IN ",
					projection, table);
		len += __ctsvc_db_append_search_query(keyword, query + len, sizeof(query) - len);
	}
	else {		// CTSVC_VIEW_URI_PERSON
		len = snprintf(query, sizeof(query), "SELECT %s FROM %s, "
					"(SELECT person_id person_id_in_contact "
							"FROM "CTS_TABLE_CONTACTS " "
							"WHERE deleted = 0 AND contact_id IN ",
							projection, table);
		len += __ctsvc_db_append_search_query(keyword, query + len, sizeof(query) - len);
		len += snprintf(query + len, sizeof(query) - len, " GROUP BY person_id_in_contact) temp_contacts "
						"ON %s.person_id = temp_contacts.person_id_in_contact", table);
	}
/*
	len += snprintf(query+len, sizeof(query)-len, "FROM %s, %s "
					"LEFT JOIN (SELECT contact_id, person_id person_id_in_contact FROM %s) temp_contacts "
					"ON %s.person_id = temp_contacts.person_id_in_contact AND "
					"temp_contacts.contact_id = %s.contact_id",
					table, CTS_TABLE_SEARCH_INDEX, CTS_TABLE_CONTACTS, table, CTS_TABLE_SEARCH_INDEX);
*/

	if (__ctsvc_db_view_has_display_name(view_uri, properties, ids_count)) {
		sortkey = ctsvc_get_sort_column();
		len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s", sortkey);
	} else if (0 == strcmp(view_uri, CTSVC_VIEW_URI_GROUP))
		len += snprintf(query+len, sizeof(query)-len, " ORDER BY group_prio");

	if (0 < limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB , "DB error : cts_query_prepare() Failed");

	r_type = ctsvc_view_get_record_type(view_uri);

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return CONTACTS_ERROR_NO_DATA;
		}

		if( r_type == CTSVC_RECORD_PERSON )
		{
			unsigned int *project = malloc(sizeof(unsigned int)*ids_count);
			int i;
			for(i=0;i<ids_count;i++)
			{
				project[i] = properties[i].property_id;
			}

			int ret = ctsvc_db_person_create_record_from_stmt_with_projection(stmt, project, ids_count, &record);

			free(project);

			if( CONTACTS_ERROR_NONE != ret )
			{
				CTS_ERR("DB error : make record Failed(%d)", ret);
			}
		}
		else {
			contacts_record_create(view_uri, &record);

			for(i=0;i<ids_count;i++) {
				type = (properties[i].property_id & CTSVC_VIEW_DATA_TYPE_MASK);
				if (type == CTSVC_VIEW_DATA_TYPE_INT)
					ctsvc_record_set_int(record, properties[i].property_id, ctsvc_stmt_get_int(stmt, i));
				else if (type == CTSVC_VIEW_DATA_TYPE_STR)
					ctsvc_record_set_str(record, properties[i].property_id, ctsvc_stmt_get_text(stmt, i));
				else if (type == CTSVC_VIEW_DATA_TYPE_BOOL)
					ctsvc_record_set_bool(record, properties[i].property_id, (ctsvc_stmt_get_int(stmt, i)?true:false));
				else if (type == CTSVC_VIEW_DATA_TYPE_LLI)
					ctsvc_record_set_lli(record, properties[i].property_id, ctsvc_stmt_get_int64(stmt, i));
				else if (type == CTSVC_VIEW_DATA_TYPE_DOUBLE)
					ctsvc_record_set_double(record, properties[i].property_id, ctsvc_stmt_get_dbl(stmt, i));
				else
					CTS_ERR("DB error : unknown type (%d)", type);
			}
		}

		ctsvc_list_prepend(list, record);
	}

	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);
	*out_list = (contacts_list_h)list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_search_records(const char* view_uri, const char *keyword,
		int offset, int limit, contacts_list_h* out_list)
{
	int ret;
	unsigned int count;
	char *projection;
	const property_info_s *p;
	bool can_keyword_search = false;

	RETVM_IF(NULL == keyword, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : keyword is NULL");

	can_keyword_search = __ctsvc_db_view_can_keyword_search(view_uri);
	RETVM_IF(false == can_keyword_search, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : can not keyword search");

	p = ctsvc_view_get_all_property_infos(view_uri, &count);
	ret = __ctsvc_db_create_projection(p, count, NULL, 0, &projection);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_create_projection is failed(%s)", ret);

	__ctsvc_db_search_records_exec(view_uri, p, count, projection, keyword, offset, limit, out_list);
	free(projection);

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_db_search_records_with_query_exec(ctsvc_query_s *s_query, const char *projection,
	const char *condition, GSList *bind, const char *keyword, int offset, int limit, contacts_list_h * out_list )
{
	char query[CTS_SQL_MAX_LEN*8] = {0}; // temporarily extend
	int len;
	int ret;
	int i;
	int type;
	GSList *cursor;
	cts_stmt stmt = NULL;
	contacts_list_h list = NULL;
	const char *table;
	const char *sortkey = NULL;

	RETV_IF(NULL == projection || '\0' == *projection, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = ctsvc_db_get_table_name(s_query->view_uri, &table);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "Invalid parameter : view uri (%s)", s_query->view_uri);

	if (s_query->distinct)
		len = snprintf(query, sizeof(query), "SELECT DISTINCT %s ", projection);
	else
		len = snprintf(query, sizeof(query), "SELECT %s ", projection);

	if (0 == strcmp(s_query->view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_CONTACT)
			|| 0 == strcmp(s_query->view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP)) {
		len += snprintf(query+len, sizeof(query)-len, "FROM %s temp_contacts ", table);
	}
	else {		// CTSVC_VIEW_URI_PERSON
		len += snprintf(query+len, sizeof(query)-len, "FROM %s, "
						"(SELECT contact_id, person_id person_id_in_contact FROM %s WHERE deleted = 0) temp_contacts "
						"ON %s.person_id = temp_contacts.person_id_in_contact "
						, table, CTS_TABLE_CONTACTS, table);
	}
/*	len += snprintf(query+len, sizeof(query)-len, "FROM %s, "CTS_TABLE_SEARCH_INDEX" "
					"ON %s.contact_id = "CTS_TABLE_SEARCH_INDEX".contact_id", table, table);*/

	len += snprintf(query+len, sizeof(query)-len,
			" WHERE temp_contacts.contact_id IN ");
	len += __ctsvc_db_append_search_query(keyword, query + len, sizeof(query) - len);

	if (condition && *condition)
		len += snprintf(query+len, sizeof(query)-len, " AND (%s)", condition);

	if (__ctsvc_db_view_has_display_name(s_query->view_uri, s_query->properties, s_query->property_count))
		sortkey = ctsvc_get_sort_column();

	if (s_query->sort_property_id) {
		const char *field_name;

		switch(s_query->sort_property_id) {
		case CTSVC_PROPERTY_PERSON_DISPLAY_NAME:
		case CTSVC_PROPERTY_CONTACT_DISPLAY_NAME:
			len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s %s", sortkey, s_query->sort_asc?"":"DESC");
			break;
		default :
			field_name = __ctsvc_db_get_property_field_name(s_query->properties,
								s_query->property_count, QUERY_SORTKEY, s_query->sort_property_id);
			if (field_name) {
				len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s", field_name);
//				if (sortkey)
//					len += snprintf(query+len, sizeof(query)-len, ", %s", sortkey);
				if (false == s_query->sort_asc)
					len += snprintf(query+len, sizeof(query)-len, " DESC");
			}
			else if (sortkey)
				len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s", sortkey);
			break;
		}
	}
	else if (sortkey)
		len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s", sortkey);
	else if (0 == strcmp(s_query->view_uri, CTSVC_VIEW_URI_GROUP))
		len += snprintf(query+len, sizeof(query)-len, " ORDER BY group_prio");

	if (0 < limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	CTS_DBG("%s", query);
	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB , "DB error : cts_query_prepare() Failed");

	i = 1;
	len = g_slist_length(bind);
	for (cursor=bind; cursor;cursor=cursor->next, i++)
		sqlite3_bind_text(stmt, i, cursor->data, strlen(cursor->data), SQLITE_STATIC);

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		if( ctsvc_view_get_record_type(s_query->view_uri) == CTSVC_RECORD_PERSON ) {
			unsigned int ids_count;
			unsigned int *project;
			if (0 == s_query->projection_count)
				ids_count = s_query->property_count;
			else
				ids_count = s_query->projection_count;

			project = malloc(sizeof(unsigned int)*ids_count);

			for(i=0;i<ids_count;i++) {
				if (0 == s_query->projection_count)
					project[i] = s_query->properties[i].property_id;
				else
					project[i] = s_query->projection[i];
			}

			ret = ctsvc_db_person_create_record_from_stmt_with_projection(stmt, project, ids_count, &record);
			free(project);

			if( CONTACTS_ERROR_NONE != ret )
				CTS_ERR("DB error : make record Failed(%d)", ret);
		}
		else {
			contacts_record_create(s_query->view_uri, (contacts_record_h*)&record);
			int field_count;
			if (0 == s_query->projection_count)
				field_count = s_query->property_count;
			else
			{
				field_count = s_query->projection_count;

				if( CONTACTS_ERROR_NONE != ctsvc_record_set_projection_flags(record, s_query->projection, s_query->projection_count, s_query->property_count) )
				{
					ASSERT_NOT_REACHED("To set projection is failed.\n");
				}
			}

			for(i=0;i<field_count;i++) {
				int property_id;

				if (0 == s_query->projection_count)
					property_id = s_query->properties[i].property_id;
				else
					property_id = s_query->projection[i];

				type = __ctsvc_db_get_property_type(s_query->properties, s_query->property_count, s_query->projection[i]);
				if (type == CTSVC_VIEW_DATA_TYPE_INT)
					ctsvc_record_set_int(record,property_id, ctsvc_stmt_get_int(stmt, i));
				else if (type == CTSVC_VIEW_DATA_TYPE_STR)
					ctsvc_record_set_str(record, property_id, ctsvc_stmt_get_text(stmt, i));
				else if (type == CTSVC_VIEW_DATA_TYPE_BOOL)
					ctsvc_record_set_bool(record, property_id, (ctsvc_stmt_get_int(stmt, i)?true:false));
				else if (type == CTSVC_VIEW_DATA_TYPE_LLI)
					ctsvc_record_set_lli(record, property_id, ctsvc_stmt_get_int64(stmt, i));
				else if (type == CTSVC_VIEW_DATA_TYPE_DOUBLE)
					ctsvc_record_set_double(record, property_id, ctsvc_stmt_get_dbl(stmt, i));
				else
					CTS_ERR("DB error : unknown type (%d)", type);
			}
		}
		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_search_records_with_query( contacts_query_h query, const char *keyword,
		int offset, int limit, contacts_list_h* out_list)
{
	int ret;
	char *condition = NULL;
	char *projection;
	ctsvc_query_s *s_query;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	bool can_keyword_search;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(NULL == keyword, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : keyword is NULL");
	s_query = (ctsvc_query_s *)query;

	can_keyword_search = __ctsvc_db_view_can_keyword_search(s_query->view_uri);
	RETVM_IF(false == can_keyword_search, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : can not keyword search");

	if (s_query->filter) {
		ret = __ctsvc_db_create_composite_condition(s_query->filter, &condition, &bind_text);
		RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_create_composite_condition is failed(%s)", ret);
	}

	ret = __ctsvc_db_create_projection(s_query->properties, s_query->property_count,
								s_query->projection, s_query->projection_count, &projection);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_create_projection is failed(%s)", ret);

	ret = __ctsvc_db_search_records_with_query_exec(s_query, projection, condition, bind_text, keyword, offset, limit, out_list);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_search_records_with_query_exec is failed(%s)", ret);

	for (cursor=bind_text;cursor;cursor=cursor->next)
		free(cursor->data);
	g_slist_free(bind_text);

	free(condition);
	free(projection);

	return CONTACTS_ERROR_NONE;
}

typedef struct {
	contacts_list_h list;
	int *ids;
	unsigned int count;
	unsigned int index;
	const char *view_uri;
	void *cb;
	void *user_data;
}ctsvc_bulk_info_s;

static int __ctsvc_db_insert_records(contacts_list_h list, int **ids)
{
	int ret;
	int index;
	int *id = NULL;
	unsigned int count;
	contacts_record_h record = NULL;

	ret = contacts_list_get_count(list, &count);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("contacts_list_get_count() failed(%d)", ret);
		return ret;
	}

	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_db_begin_trans is failed(%d)", ret);

	id = calloc(count, sizeof(int));

	contacts_list_first(list);
	index = 0;
	do {
		ret = contacts_list_get_current_record_p(list, &record);
		if( CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("contacts_list_get_current_record_p is faild(%d)", ret);
			ctsvc_end_trans(false);
			free(id);
			return ret;
		}

		ret = contacts_db_insert_record(record, &id[index++]);
		if( ret != CONTACTS_ERROR_NONE ) {
			CTS_ERR("contacts_db_insert_record is faild(%d)", ret);
			ctsvc_end_trans(false);
			free(id);
			return ret;
		}
	}while (CONTACTS_ERROR_NONE  == contacts_list_next(list));

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
	{
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		free(id);
		return ret;
	}

	if (ids)
		*ids = id;
	else
		free(id);
	return CONTACTS_ERROR_NONE;
}

#ifdef _CONTACTS_NATIVE
static gboolean __ctsvc_db_insert_idler(void *data)
{
	int ret;
	ctsvc_bulk_info_s *info = data;
	contacts_db_insert_result_cb cb;

	ret = __ctsvc_db_insert_records(info->list, &info->ids);

	if (info->cb) {
		cb = info->cb;
		if( CONTACTS_ERROR_NONE != ret) {
			cb(ret, NULL, 0, info->user_data);
		}
		else {
			unsigned int count = 0;
			contacts_list_get_count(info->list, &count);
			cb(ret, info->ids, count, info->user_data);
		}
	}
	contacts_list_destroy(info->list, true);
	free(info->ids);
	free(info);
	return false;
}
#endif

static int __ctsvc_db_delete_records(const char* view_uri, int ids[], int count)
{
	int ret = CONTACTS_ERROR_NONE;
	int index;

	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_db_begin_trans is failed(%d)", ret);

	index = 0;
	do {
		ret = contacts_db_delete_record(view_uri, ids[index++]);
		if (CONTACTS_ERROR_NO_DATA == ret) {
			CTS_DBG("the record is not exist : %d", ret);
			continue;
		}
		else if( ret != CONTACTS_ERROR_NONE ) {
			CTS_ERR("contacts_db_delete_record is faild(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
	} while(index < count);

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
	{
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}

	return CONTACTS_ERROR_NONE;
}

#ifdef _CONTACTS_NATIVE
static gboolean __ctsvc_db_delete_idler(void *data)
{
	int ret;
	ctsvc_bulk_info_s *info = data;
	contacts_db_result_cb cb;

	ret = __ctsvc_db_delete_records(info->view_uri, info->ids, info->count);

	if (info->cb) {
		cb = info->cb;
		cb(ret, info->user_data);
	}
	free(info->ids);
	free(info);
	return false;
}
#endif

static int __ctsvc_db_update_records( contacts_list_h list)
{
	int ret = CONTACTS_ERROR_NONE;
	unsigned int count;
	contacts_record_h record;

	ret = contacts_list_get_count(list, &count);
	RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "contacts_list_get_count is falied(%d)", ret);
	RETVM_IF(count <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : count is 0");

	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_db_begin_trans is failed(%d)", ret);

	contacts_list_first(list);
	do {
		ret = contacts_list_get_current_record_p(list, &record);
		if( CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("contacts_list_get_current_record_p is faild(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}

		ret = contacts_db_update_record(record);
		if( ret != CONTACTS_ERROR_NONE ) {
			CTS_ERR("contacts_db_update_record is faild(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(list));
	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE)
	{
		CTS_ERR("DB error : ctsvc_end_trans() Failed(%d)", ret);
		return ret;
	}


	return CONTACTS_ERROR_NONE;
}

#ifdef _CONTACTS_NATIVE
static gboolean __ctsvc_db_update_idler(void *data)
{
	int ret;
	ctsvc_bulk_info_s *info = data;
	contacts_db_result_cb cb;

	ret = __ctsvc_db_update_records(info->list);

	if (info->cb) {
		cb = info->cb;
		cb(ret, info->user_data);
	}
	contacts_list_destroy(info->list, true);
	free(info);
	return false;
}
#endif

static int __ctsvc_db_get_count_exec(const char *view_uri, const property_info_s* properties, int ids_count,
		const char *projection, int *out_count )
{
	char query[CTS_SQL_MAX_LEN] = {0};
	const char *table;
	int len;
	int ret;

	ret = ctsvc_db_get_table_name(view_uri, &table);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "Invalid parameter : view uri (%s)", view_uri);

	len = snprintf(query, sizeof(query), "SELECT COUNT(*) FROM (SELECT %s FROM %s)", projection, table);

	ret = ctsvc_query_get_first_int_result(query, out_count);

	return ret;
}

static int __ctsvc_db_get_count( const char* view_uri, int *out_count)
{
	int ret;
	unsigned int count;
	char *projection;

	const property_info_s *p = ctsvc_view_get_all_property_infos(view_uri, &count);
	ret = __ctsvc_db_create_projection(p, count, NULL, 0, &projection);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_create_projection is failed(%s)", ret);

	__ctsvc_db_get_count_exec(view_uri, p, count, projection, out_count);
	free(projection);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_count_with_query_exec(ctsvc_query_s *s_query, const char *projection,
	const char *condition, GSList *bind_text, int *out_count )
{
	char query[CTS_SQL_MAX_LEN] = {0};
	int len;
	int ret;
	const char *table;

	RETV_IF(NULL == projection || '\0' == *projection, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = ctsvc_db_get_table_name(s_query->view_uri, &table);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "Invalid parameter : view uri (%s)", s_query->view_uri);

	if (s_query->distinct)
		len = snprintf(query, sizeof(query), "SELECT COUNT(*) FROM (SELECT DISTINCT %s FROM %s", projection, table);
	else
		len = snprintf(query, sizeof(query), "SELECT COUNT(*) FROM (SELECT %s FROM %s", projection, table);

	if (condition && *condition)
		len += snprintf(query+len, sizeof(query)-len, " WHERE %s)", condition);
	else
		len += snprintf(query+len, sizeof(query)-len, ")");

	if (bind_text) {
		cts_stmt stmt;
		GSList *cursor;
		int i;
		stmt = cts_query_prepare(query);
		if(NULL == stmt) {
			CTS_ERR("DB error : cts_query_prepare() Failed");
			return CONTACTS_ERROR_DB;
		}

		for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
			cts_stmt_bind_copy_text(stmt, i, cursor->data, strlen(cursor->data));
		ret = ctsvc_stmt_get_first_int_result(stmt, out_count);
	}
	else
		ret = ctsvc_query_get_first_int_result(query, out_count);
	return ret;
}

static int __ctsvc_db_get_count_with_query( contacts_query_h query, int *out_count)
{
	int ret;
	char *condition = NULL;
	char *projection;
	ctsvc_query_s *query_s;
	GSList *bind_text = NULL;
	GSList *cursor;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	query_s = (ctsvc_query_s *)query;

	if (query_s->filter) {
		ret = __ctsvc_db_create_composite_condition(query_s->filter, &condition, &bind_text);
		RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_create_composite_condition is failed(%s)", ret);
	}

	ret = __ctsvc_db_create_projection(query_s->properties, query_s->property_count,
								query_s->projection, query_s->projection_count, &projection);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_create_projection is failed(%d)", ret);
		for (cursor=bind_text;cursor;cursor=cursor->next)
			free(cursor->data);
		g_slist_free(bind_text);
		return ret;
	}

	ret = __ctsvc_db_get_count_with_query_exec(query_s, projection, condition, bind_text, out_count);
	for (cursor=bind_text;cursor;cursor=cursor->next)
		free(cursor->data);
	g_slist_free(bind_text);

	free(condition);
	free(projection);

	return ret;
}

API int contacts_db_get_records_with_query( contacts_query_h query, int offset, int limit,
	contacts_list_h* out_list )
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_db_plugin_info_s* plugin_info = NULL;
	ctsvc_query_s *s_query;

	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s*)query;

	if (( plugin_info = ctsvc_db_get_plugin_info(ctsvc_view_get_record_type(s_query->view_uri)))){
		if( plugin_info->get_records_with_query ) {
			ret = plugin_info->get_records_with_query( query, offset, limit, out_list );
			return ret;
		}
	}

	return __ctsvc_db_get_records_with_query_exec(s_query, offset, limit, out_list);
}

static int __ctsvc_db_get_contact_changes(const char* view_uri, int addressbook_id,
		int version, contacts_list_h* out_list, int* out_current_version)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;
	cts_stmt stmt;

	if (0 <= addressbook_id) {
		snprintf(query, sizeof(query),
			"SELECT %d, contact_id, changed_ver, created_ver, addressbook_id, image_changed_ver "
					"FROM "CTS_TABLE_CONTACTS" "
					"WHERE changed_ver > %d AND addressbook_id = %d AND deleted = 0 "
			"UNION "
			"SELECT %d, contact_id, deleted_ver, -1, addressbook_id, 0 "
					"FROM "CTS_TABLE_DELETEDS" "
					"WHERE deleted_ver > %d AND created_ver <= %d AND addressbook_id = %d "
			"UNION "
			"SELECT %d, contact_id, changed_ver, -1, addressbook_id, 0 "
					"FROM "CTS_TABLE_CONTACTS" "
					"WHERE changed_ver > %d AND created_ver <= %d AND addressbook_id = %d AND deleted = 1 "
						"AND addressbook_id = (SELECT addressbook_id FROM "CTS_TABLE_ADDRESSBOOKS" WHERE addressbook_id = %d)",
			CONTACTS_CHANGE_UPDATED, version, addressbook_id,
			CONTACTS_CHANGE_DELETED, version, version, addressbook_id,
			CONTACTS_CHANGE_DELETED, version, version, addressbook_id, addressbook_id);
	}
	else {
		snprintf(query, sizeof(query),
			"SELECT %d, contact_id, changed_ver, created_ver, addressbook_id, image_changed_ver "
					"FROM "CTS_TABLE_CONTACTS" "
					"WHERE changed_ver > %d AND deleted = 0 "
			"UNION "
			"SELECT %d, contact_id, deleted_ver, -1, addressbook_id, 0 "
					"FROM "CTS_TABLE_DELETEDS" "
					"WHERE deleted_ver > %d AND created_ver <= %d "
			"UNION "
			"SELECT %d, contact_id, changed_ver, -1, "CTS_TABLE_CONTACTS".addressbook_id, 0 "
					"FROM "CTS_TABLE_CONTACTS",  "CTS_TABLE_ADDRESSBOOKS" "
					"WHERE changed_ver > %d AND created_ver <= %d AND deleted = 1 "
						"AND "CTS_TABLE_CONTACTS".addressbook_id = "CTS_TABLE_ADDRESSBOOKS".addressbook_id",
			CONTACTS_CHANGE_UPDATED, version,
			CONTACTS_CHANGE_DELETED, version, version,
			CONTACTS_CHANGE_DELETED, version ,version);
	}

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB , "DB error : cts_query_prepare() Failed");

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		ctsvc_updated_info_s *update_info;

		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		ret = contacts_record_create(_contacts_contact_updated_info._uri, &record);
		update_info = (ctsvc_updated_info_s *)record;
		update_info->changed_type = ctsvc_stmt_get_int(stmt, 0);
		update_info->id = ctsvc_stmt_get_int(stmt, 1);
		update_info->changed_ver = ctsvc_stmt_get_int(stmt, 2);

		if (ctsvc_stmt_get_int(stmt, 3) == update_info->changed_ver || version < ctsvc_stmt_get_int(stmt, 3))
			update_info->changed_type = CONTACTS_CHANGE_INSERTED;

		update_info->addressbook_id = ctsvc_stmt_get_int(stmt, 4);

		if (version < ctsvc_stmt_get_int(stmt, 5))
			update_info->image_changed = true;

		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	snprintf(query, sizeof(query), "SELECT ver FROM "CTS_TABLE_VERSION);
	ret = ctsvc_query_get_first_int_result(query, out_current_version);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_group_changes(const char* view_uri, int addressbook_id,
		int version, contacts_list_h* out_list, int* out_current_version)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;
	cts_stmt stmt;

	if (0 <= addressbook_id) {
		snprintf(query, sizeof(query),
			"SELECT %d, group_id, changed_ver, created_ver, addressbook_id FROM %s "
			"WHERE changed_ver > %d AND addressbook_id = %d "
			"UNION "
			"SELECT %d, group_id, deleted_ver, -1, addressbook_id FROM %s "
			"WHERE deleted_ver > %d AND created_ver <= %d AND addressbook_id = %d",
			CONTACTS_CHANGE_UPDATED, CTS_TABLE_GROUPS, version, addressbook_id,
			CONTACTS_CHANGE_DELETED, CTS_TABLE_GROUP_DELETEDS, version, version, addressbook_id);
	}
	else {
		snprintf(query, sizeof(query),
			"SELECT %d, group_id, changed_ver, created_ver, addressbook_id FROM %s "
			"WHERE changed_ver > %d "
			"UNION "
			"SELECT %d, group_id, deleted_ver, -1, addressbook_id FROM %s "
			"WHERE deleted_ver > %d AND created_ver <= %d",
			CONTACTS_CHANGE_UPDATED, CTS_TABLE_GROUPS, version,
			CONTACTS_CHANGE_DELETED, CTS_TABLE_GROUP_DELETEDS, version, version);
	}

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB , "DB error : cts_query_prepare() Failed");

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		ctsvc_updated_info_s *update_info;

		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		ret = contacts_record_create(_contacts_group_updated_info._uri, &record);
		update_info = (ctsvc_updated_info_s *)record;
		update_info->changed_type = ctsvc_stmt_get_int(stmt, 0);
		update_info->id = ctsvc_stmt_get_int(stmt, 1);
		update_info->changed_ver = ctsvc_stmt_get_int(stmt, 2);

		if (ctsvc_stmt_get_int(stmt, 3) == update_info->changed_ver || version < ctsvc_stmt_get_int(stmt, 3))
			update_info->changed_type = CONTACTS_CHANGE_INSERTED;

		update_info->addressbook_id = ctsvc_stmt_get_int(stmt, 4);

		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	snprintf(query, sizeof(query), "SELECT ver FROM "CTS_TABLE_VERSION);
	ret = ctsvc_query_get_first_int_result(query, out_current_version);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_group_relations_changes(const char* view_uri, int addressbook_id,
		int version, contacts_list_h* out_list, int* out_current_version)
{
	int len;
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;
	cts_stmt stmt;

	len = snprintf(query, sizeof(query),
			"SELECT %d, group_id, contact_id, addressbook_id, ver "
				"FROM "CTS_TABLE_GROUP_RELATIONS", "CTS_TABLE_GROUPS" USING (group_id) "
				"WHERE ver > %d AND deleted = 0 ", CONTACTS_CHANGE_INSERTED, version);
	if (0 <= addressbook_id) {
		len += snprintf(query + len , sizeof(query) - len ,
				"AND addressbook_id = %d ", addressbook_id);
	}

	len += snprintf(query + len, sizeof(query) - len,
			"UNION SELECT %d, group_id, contact_id, addressbook_id, ver "
				"FROM "CTS_TABLE_GROUP_RELATIONS", "CTS_TABLE_GROUPS" USING (group_id) "
				"WHERE ver > %d AND deleted = 1 ", CONTACTS_CHANGE_DELETED, version);
	if (0 <= addressbook_id) {
		len += snprintf(query + len , sizeof(query) - len ,
				"AND addressbook_id = %d ", addressbook_id);
	}

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB , "DB error : cts_query_prepare() Failed");

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		ret = contacts_record_create(view_uri, &record);
		ctsvc_record_set_int(record, _contacts_grouprel_updated_info.type, ctsvc_stmt_get_int(stmt, 0));
		ctsvc_record_set_int(record, _contacts_grouprel_updated_info.group_id, ctsvc_stmt_get_int(stmt, 1));
		ctsvc_record_set_int(record, _contacts_grouprel_updated_info.contact_id, ctsvc_stmt_get_int(stmt, 2));
		ctsvc_record_set_int(record, _contacts_grouprel_updated_info.address_book_id, ctsvc_stmt_get_int(stmt, 3));
		ctsvc_record_set_int(record, _contacts_grouprel_updated_info.version, ctsvc_stmt_get_int(stmt, 4));

		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	snprintf(query, sizeof(query), "SELECT ver FROM "CTS_TABLE_VERSION);
	ret = ctsvc_query_get_first_int_result(query, out_current_version);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_group_member_changes(const char* view_uri, int addressbook_id,
		int version, contacts_list_h* out_list, int* out_current_version)
{
	int len;
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;
	cts_stmt stmt;

	len = snprintf(query, sizeof(query),
			"SELECT group_id, member_changed_ver, addressbook_id "
				"FROM "CTS_TABLE_GROUPS" WHERE member_changed_ver > %d", version);

	if (0 <= addressbook_id)
		len += snprintf(query+len, sizeof(query)-len, " AND addressbook_id = %d ", addressbook_id);

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB , "DB error : cts_query_prepare() Failed");

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		ret = contacts_record_create(view_uri, &record);
		ctsvc_record_set_int(record, _contacts_group_member_updated_info.group_id, ctsvc_stmt_get_int(stmt, 0));
		ctsvc_record_set_int(record, _contacts_group_member_updated_info.version, ctsvc_stmt_get_int(stmt, 1));
		ctsvc_record_set_int(record, _contacts_group_member_updated_info.address_book_id, ctsvc_stmt_get_int(stmt, 2));

		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	snprintf(query, sizeof(query), "SELECT ver FROM "CTS_TABLE_VERSION);
	ret = ctsvc_query_get_first_int_result(query, out_current_version);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_my_profile_changes(const char* view_uri, int addressbook_id,
		int version, contacts_list_h* out_list, int* out_current_version)
{
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;
	cts_stmt stmt;

	if (0 <= addressbook_id) {
		snprintf(query, sizeof(query),
			"SELECT changed_ver, addressbook_id, %d FROM %s "
			"WHERE changed_ver > %d AND changed_ver == created_ver AND deleted = 0 AND addressbook_id = %d "
			"UNION "
			"SELECT changed_ver, addressbook_id, %d FROM %s "
			"WHERE changed_ver > %d AND changed_ver != created_ver AND deleted = 0 AND addressbook_id = %d "
			"UNION "
			"SELECT changed_ver, addressbook_id, %d FROM %s "
			"WHERE changed_ver > %d AND deleted = 1 AND addressbook_id = %d",
			CONTACTS_CHANGE_INSERTED, CTS_TABLE_MY_PROFILES, version, addressbook_id,
			CONTACTS_CHANGE_UPDATED, CTS_TABLE_MY_PROFILES, version, addressbook_id,
			CONTACTS_CHANGE_DELETED, CTS_TABLE_MY_PROFILES, version, addressbook_id);
	}
	else {
		snprintf(query, sizeof(query),
			"SELECT changed_ver, addressbook_id, %d FROM %s "
			"WHERE changed_ver > %d AND changed_ver == created_ver AND deleted = 0 "
			"UNION "
			"SELECT changed_ver, addressbook_id, %d FROM %s "
			"WHERE changed_ver > %d AND changed_ver != created_ver AND deleted = 0 "
			"UNION "
			"SELECT changed_ver, addressbook_id, %d FROM %s "
			"WHERE changed_ver > %d AND deleted = 1",
			CONTACTS_CHANGE_INSERTED, CTS_TABLE_MY_PROFILES, version,
			CONTACTS_CHANGE_UPDATED, CTS_TABLE_MY_PROFILES, version,
			CONTACTS_CHANGE_DELETED, CTS_TABLE_MY_PROFILES, version);
	}

	stmt = cts_query_prepare(query);
	RETVM_IF(NULL == stmt, CONTACTS_ERROR_DB , "DB error : cts_query_prepare() Failed");

	contacts_list_create(&list);
	while ((ret = cts_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		ret = contacts_record_create(view_uri, &record);
		ctsvc_record_set_int(record, _contacts_my_profile_updated_info.version, ctsvc_stmt_get_int(stmt, 0));
		ctsvc_record_set_int(record, _contacts_my_profile_updated_info.address_book_id, ctsvc_stmt_get_int(stmt, 1));
		ctsvc_record_set_int(record, _contacts_my_profile_updated_info.last_changed_type, ctsvc_stmt_get_int(stmt, 2));
		ctsvc_list_prepend(list, record);
	}
	cts_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	snprintf(query, sizeof(query), "SELECT ver FROM "CTS_TABLE_VERSION);
	ret = ctsvc_query_get_first_int_result(query, out_current_version);

	return CONTACTS_ERROR_NONE;
}

API int contacts_db_get_changes_by_version( const char* view_uri, int addressbook_id,
		int version, contacts_list_h* out_list, int* out_current_version )
{
	int ret;
	RETV_IF(version < 0, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;
	RETV_IF(NULL == out_current_version, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_current_version = 0;

	if (0 == strcmp(view_uri, _contacts_contact_updated_info._uri)) {
		ret = __ctsvc_db_get_contact_changes(view_uri, addressbook_id,
					version, out_list, out_current_version);
		return ret;
	}
	else if (0 == strcmp(view_uri, _contacts_group_updated_info._uri)) {
		ret = __ctsvc_db_get_group_changes(view_uri, addressbook_id,
					version, out_list, out_current_version);
		return ret;
	}
	else if (0 == strcmp(view_uri, _contacts_group_member_updated_info._uri)) {
		ret = __ctsvc_db_get_group_member_changes(view_uri, addressbook_id,
					version, out_list, out_current_version);
		return ret;
	}
	else if (0 == strcmp(view_uri, _contacts_grouprel_updated_info._uri)) {
		ret = __ctsvc_db_get_group_relations_changes(view_uri, addressbook_id,
					version, out_list, out_current_version);
		return ret;
	}
	else if (0 == strcmp(view_uri, _contacts_my_profile_updated_info._uri)) {
		ret = __ctsvc_db_get_my_profile_changes(view_uri, addressbook_id,
					version, out_list, out_current_version);
		return ret;
	}

	CTS_ERR("Invalid parameter : this API does not support uri(%s)", view_uri);
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

API int contacts_db_get_current_version( int* out_current_version )
{
	RETVM_IF(NULL == out_current_version, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	return ctsvc_get_current_version(out_current_version);
}

API int contacts_db_search_records(const char* view_uri, const char *keyword,
		int offset, int limit, contacts_list_h* out_list)
{
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;
	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	return __ctsvc_db_search_records(view_uri, keyword, offset, limit, out_list);
}

API int contacts_db_search_records_with_query( contacts_query_h query, const char *keyword,
		int offset, int limit, contacts_list_h* out_list)
{
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;
	RETVM_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	return __ctsvc_db_search_records_with_query(query, keyword, offset, limit, out_list);
}

API int contacts_db_get_count( const char* view_uri, int *out_count)
{
	int ret;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETV_IF(NULL == out_count, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_count = 0;
	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	if (( plugin_info = ctsvc_db_get_plugin_info(ctsvc_view_get_record_type(view_uri)))){
		if( plugin_info->get_count ) {
			ret = plugin_info->get_count(out_count);
			return ret;
		}
	}

	return __ctsvc_db_get_count( view_uri, out_count );
}

API int contacts_db_get_count_with_query( contacts_query_h query, int *out_count)
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_record_type_e type = CTSVC_RECORD_INVALID;
	ctsvc_db_plugin_info_s* plugin_info = NULL;
	ctsvc_query_s *s_query;

	RETV_IF(NULL == out_count, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_count = 0;

	RETVM_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	s_query = (ctsvc_query_s*)query;

	type = ctsvc_view_get_record_type(s_query->view_uri);
	plugin_info = ctsvc_db_get_plugin_info(type);

	if (plugin_info){
		if(plugin_info->get_count_with_query ) {
			ret = plugin_info->get_count_with_query( query, out_count );
			return ret;
		}
	}

	return __ctsvc_db_get_count_with_query( query, out_count );
}

API int contacts_db_insert_record(contacts_record_h record, int *id )
{
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	if (id)
		*id = 0;

	RETVM_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	plugin_info = ctsvc_db_get_plugin_info(((ctsvc_record_s*)record)->r_type);
	RETVM_IF(NULL == plugin_info, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(NULL == plugin_info->insert_record, CONTACTS_ERROR_INVALID_PARAMETER, "Not permitted");

	return plugin_info->insert_record(record, id);
}

API int contacts_db_update_record(contacts_record_h record)
{
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETVM_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	plugin_info = ctsvc_db_get_plugin_info(((ctsvc_record_s*)record)->r_type);
	RETVM_IF(NULL == plugin_info, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(NULL == plugin_info->update_record, CONTACTS_ERROR_INVALID_PARAMETER, "Not permitted");

	return plugin_info->update_record(record);
}

API int contacts_db_delete_record(const char* view_uri, int id)
{
	ctsvc_record_type_e type = CTSVC_RECORD_INVALID;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	type = ctsvc_view_get_record_type(view_uri);
	plugin_info = ctsvc_db_get_plugin_info(type);
	RETVM_IF(NULL == plugin_info, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(NULL == plugin_info->delete_record, CONTACTS_ERROR_INVALID_PARAMETER, "Not permitted");

	return plugin_info->delete_record(id);
}

API int contacts_db_get_record(const char* view_uri, int id, contacts_record_h* out_record )
{
	ctsvc_record_type_e type = CTSVC_RECORD_INVALID;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;
	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	type = ctsvc_view_get_record_type(view_uri);
	plugin_info = ctsvc_db_get_plugin_info(type);

	RETVM_IF(NULL == plugin_info, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(NULL == plugin_info->get_record, CONTACTS_ERROR_INVALID_PARAMETER, "Not permitted");

	return plugin_info->get_record(id, out_record);
}

API int contacts_db_replace_record( contacts_record_h record, int id )
{
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETVM_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : record is NULL");

	plugin_info = ctsvc_db_get_plugin_info(((ctsvc_record_s*)record)->r_type);
	RETVM_IF(NULL == plugin_info, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(NULL == plugin_info->replace_record, CONTACTS_ERROR_INVALID_PARAMETER, "Not permitted");

	return plugin_info->replace_record(record, id);
}

API int contacts_db_get_all_records(const char* view_uri, int offset, int limit, contacts_list_h* out_list )
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_record_type_e type = CTSVC_RECORD_INVALID;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;
	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	type = ctsvc_view_get_record_type(view_uri);
	plugin_info = ctsvc_db_get_plugin_info(type);

	if (plugin_info){
		if( plugin_info->get_all_records ) {
			ret = plugin_info->get_all_records(offset, limit, out_list);
			return ret;
		}
	}

	return __ctsvc_db_get_all_records( view_uri, offset, limit, out_list );
}

int ctsvc_db_insert_records(contacts_list_h list, int **ids, unsigned int *count)
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETVM_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	if (count)
		contacts_list_get_count(list, count);
	if (( plugin_info = ctsvc_db_get_plugin_info(((ctsvc_list_s*)list)->l_type))){
		if( plugin_info->insert_records ) {
			ret = plugin_info->insert_records( list, ids );
			return ret;
		}
	}

	return __ctsvc_db_insert_records(list, ids);
}

API int contacts_db_insert_records_async( contacts_list_h list,
		contacts_db_insert_result_cb callback, void *user_data)
{
	RETVM_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

#ifdef _CONTACTS_NATIVE
	if (callback) {
		ctsvc_bulk_info_s *info;
		info = (ctsvc_bulk_info_s *)calloc(1, sizeof(ctsvc_bulk_info_s));
		ctsvc_list_clone(list, &info->list);
		info->cb = callback;
		info->user_data = user_data;
		g_idle_add(__ctsvc_db_insert_idler, info);		// should be changed after ipc implementation
		return CONTACTS_ERROR_NONE;
	}
#endif
	return ctsvc_db_insert_records(list, NULL, NULL);
}

int ctsvc_db_update_records(contacts_list_h list)
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETVM_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	if (( plugin_info = ctsvc_db_get_plugin_info(((ctsvc_list_s*)list)->l_type))){
		if( plugin_info->update_records ) {
			ret = plugin_info->update_records( list );
			return ret;
		}
	}

	return __ctsvc_db_update_records(list);
}

API int contacts_db_update_records_async( contacts_list_h list,
		contacts_db_result_cb callback, void *user_data)
{
	RETVM_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

#ifdef _CONTACTS_NATIVE
	if (callback) {
		ctsvc_bulk_info_s *info;
		info = (ctsvc_bulk_info_s *)calloc(1, sizeof(ctsvc_bulk_info_s));
		ctsvc_list_clone(list, &info->list);
		info->cb = callback;
		info->user_data = user_data;
		g_idle_add(__ctsvc_db_update_idler, info);		// should be changed after ipc implementation
		return CONTACTS_ERROR_NONE;
	}
#endif
	return ctsvc_db_update_records(list);
}

int ctsvc_db_delete_records(const char* view_uri, int* ids, int count)
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETV_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER);

	if (( plugin_info = ctsvc_db_get_plugin_info(ctsvc_view_get_record_type(view_uri)))){
		if( plugin_info->delete_records ) {
			ret = plugin_info->delete_records( ids, count );
			return ret;
		}
	}

	return __ctsvc_db_delete_records(view_uri, ids, count);
}

API int contacts_db_delete_records_async( const char* view_uri, int* ids, int count,
		contacts_db_result_cb callback, void *user_data)
{
	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

#ifdef _CONTACTS_NATIVE
	if (callback) {
		ctsvc_bulk_info_s *info;
		info = (ctsvc_bulk_info_s *)calloc(1, sizeof(ctsvc_bulk_info_s));
		info->ids = calloc(count, sizeof(int));
		memcpy(info->ids, ids, sizeof(int)*count);
		info->view_uri = view_uri;
		info->count = count;
		info->cb = callback;
		info->user_data = user_data;
		g_idle_add(__ctsvc_db_delete_idler, info);		// should be changed after ipc implementation
		return CONTACTS_ERROR_NONE;
	}
#endif
	return ctsvc_db_delete_records(view_uri, ids, count);
}

static int __ctsvc_db_replace_records( contacts_list_h list, int ids[], int count)
{
	int ret = CONTACTS_ERROR_NONE;
	unsigned int record_count;
	contacts_record_h record;
	int i;

	ret = contacts_list_get_count(list, &record_count);
	RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "contacts_list_get_count is falied(%d)", ret);
	RETVM_IF(record_count <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : count is 0");
	RETVM_IF(record_count != count, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : list count and ids count are not matched");

	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_db_begin_trans is failed(%d)", ret);

	contacts_list_first(list);
	i = 0;
	do {
		ret = contacts_list_get_current_record_p(list, &record);
		if( CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("contacts_list_get_current_record_p is faild(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}

		ret = contacts_db_replace_record(record, ids[i++]);
		if( ret != CONTACTS_ERROR_NONE ) {
			CTS_ERR("contacts_db_replace_record is faild(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
	}while(CONTACTS_ERROR_NONE == contacts_list_next(list));

	ret = ctsvc_end_trans(true);

	return ret;
}


#ifdef _CONTACTS_NATIVE
static gboolean __ctsvc_db_replace_idler(void *data)
{
	int ret;
	ctsvc_bulk_info_s *info = data;
	contacts_db_result_cb cb;

	ret = __ctsvc_db_replace_records(info->list, info->ids, info->count);

	if (info->cb) {
		cb = info->cb;
		cb(ret, info->user_data);
	}
	contacts_list_destroy(info->list, true);
	free(info->ids);
	free(info);
	return false;
}
#endif

int ctsvc_db_replace_records(contacts_list_h list, int ids[], unsigned int count)
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETVM_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	if (( plugin_info = ctsvc_db_get_plugin_info(((ctsvc_list_s*)list)->l_type))){
		if( plugin_info->replace_records ) {
			ret = plugin_info->replace_records( list, ids, count );
			return ret;
		}
	}

	return __ctsvc_db_replace_records(list, ids, count);
}

API int contacts_db_replace_records_async( contacts_list_h list, int ids[], unsigned int count,
		contacts_db_result_cb callback, void *user_data )
{
	RETVM_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

#ifdef _CONTACTS_NATIVE
	if (callback) {
		ctsvc_bulk_info_s *info;
		info = (ctsvc_bulk_info_s *)calloc(1, sizeof(ctsvc_bulk_info_s));
		ctsvc_list_clone(list, &info->list);
		info->ids = calloc(count, sizeof(int));
		memcpy(info->ids, ids, sizeof(int)*count);
		info->count = count;
		info->cb = callback;
		info->user_data = user_data;
		g_idle_add(__ctsvc_db_replace_idler, info);		// should be changed after ipc implementation
		return CONTACTS_ERROR_NONE;
	}
#endif
	return ctsvc_db_replace_records(list, ids, count);
}

API int contacts_db_insert_records( contacts_list_h record_list, int **ids, unsigned int *count)
{
	return ctsvc_db_insert_records(record_list, ids, count);
}

API int contacts_db_update_records( contacts_list_h record_list)
{
	return ctsvc_db_update_records(record_list);
}

API int contacts_db_delete_records(const char* view_uri, int record_id_array[], int count)
{
	return ctsvc_db_delete_records(view_uri, record_id_array, count);
}

API int contacts_db_replace_records( contacts_list_h list, int record_id_array[], unsigned int count )
{
	return ctsvc_db_replace_records(list, record_id_array, count);
}

API int contacts_db_get_last_change_version(int* last_version)
{
	int ret = CONTACTS_ERROR_NONE;

	RETVM_IF(NULL == last_version, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	*last_version = ctsvc_get_transaction_ver();
	return ret;
}

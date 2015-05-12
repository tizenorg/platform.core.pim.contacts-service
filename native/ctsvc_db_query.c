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
#include "contacts_internal.h"

#include "ctsvc_internal.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_schema.h"
#include "ctsvc_list.h"
#include "ctsvc_record.h"
#include "ctsvc_utils.h"
#include "ctsvc_normalize.h"
#include "ctsvc_number_utils.h"
#include "ctsvc_db_init.h"
#include "ctsvc_view.h"
#include "ctsvc_inotify.h"
#include "ctsvc_localize.h"
#include "ctsvc_localize_utils.h"
#include "ctsvc_setting.h"
#include "ctsvc_notify.h"

#include "ctsvc_db_access_control.h"
#include "ctsvc_db_plugin_person_helper.h"
#include "ctsvc_db_plugin_group_helper.h"
#include "ctsvc_db_plugin_company_helper.h"

#ifdef _CONTACTS_IPC_SERVER
#ifdef ENABLE_SIM_FEATURE
#include "ctsvc_server_sim.h"
#endif // ENABLE_SIM_FEATURE
#include "ctsvc_server_change_subject.h"
#endif

// It is used to sort search results
const char *hangul_syllable[19][3] = {
	{"ㄱ", "가", "깋"},	// AC00, AE3B
	{"ㄲ", "까", "낗"},	// AE3C, B097
	{"ㄴ", "나", "닣"},	// B098, B2E3
	{"ㄷ", "다", "딯"},	// B2E4, B52F
	{"ㄸ", "따", "띻"},	// B530, B77B
	{"ㄹ", "라", "맇"},	// B77C, B9C7
	{"ㅁ", "마", "밓"},	// B9C8, BC13
	{"ㅂ", "바", "빟"},	// BC14, BE5F
	{"ㅃ", "빠", "삫"},	// BE60, C0AB
	{"ㅅ", "사", "싷"},	// C0AC, C2F7
	{"ㅆ", "싸", "앃"},	// C2F8, C543
	{"ㅇ", "아", "잏"},	// C544, C78F
	{"ㅈ", "자", "짛"},	// C790, C9DB
	{"ㅉ", "짜", "찧"},	// C9DC, CC27
	{"ㅊ", "차", "칳"},	// CC28, CE73
	{"ㅋ", "카", "킿"},	// CE74, D0AF
	{"ㅌ", "타", "팋"},	// D0C0, D30B
	{"ㅍ", "파", "핗"},	// D30C, D557
	{"ㅎ", "하", "힣"},	// D558, D7A3
};

typedef enum {
	QUERY_SORTKEY,
	QUERY_FILTER,
	QUERY_PROJECTION,
}db_query_property_type_e;

static contacts_db_status_e __db_status = CONTACTS_DB_STATUS_NORMAL;

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

// return data type of the property
// bool / int / long long int / char string / double / child record
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
		ctsvc_attribute_filter_s *filter, char **condition)
{
	const char *field_name;
	char out_cond[CTS_SQL_MAX_LEN] = {0};

	field_name = __ctsvc_db_get_property_field_name(com_filter->properties,
							com_filter->property_count, QUERY_FILTER, filter->property_id);
	RETVM_IF(NULL == field_name, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property id(%d)", filter->property_id);

#ifdef _CONTACTS_IPC_SERVER
#ifdef ENABLE_SIM_FEATURE
	if (filter->property_id == CTSVC_PROPERTY_PHONELOG_SIM_SLOT_NO) {
		// get real sim info id by SIM slot number 0/1
		int sim_info_id = ctsvc_server_sim_get_info_id_by_sim_slot_no(filter->value.i);
		if (0 < sim_info_id) {
			snprintf(out_cond, sizeof(out_cond), "%s = %d", field_name, sim_info_id);
			*condition = strdup(out_cond);
			return CONTACTS_ERROR_NONE;
		}
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
#endif // ENABLE_SIM_FEATURE
#endif

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
		ctsvc_attribute_filter_s *filter, char **condition)
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
		ctsvc_attribute_filter_s *filter, char **condition)
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

#define CTSVC_DB_ESCAPE_CHAR	'\\'

static char * __ctsvc_db_get_str_with_escape(char *str, int len, bool with_escape)
{
	int i, j = 0;
	char temp_str[len*2+1];

	if (false == with_escape)
		return strdup(str);

	for (i=0;i<len;i++) {
		if (str[i] == '\'' || str[i] == '_' || str[i] == '%' || str[i] == '\\') {
			temp_str[j++] = CTSVC_DB_ESCAPE_CHAR;
		}
		temp_str[j++] = str[i];
	}
	temp_str[j] = '\0';

	return strdup(temp_str);
}

static inline int __ctsvc_db_add_str_matching_rule(const char *field_name, int match, char **condition, bool *with_escape)
{
	int cond_len = 0;
	char out_cond[CTS_SQL_MAX_LEN] = {0};

	*with_escape = true;
	switch(match) {
	case CONTACTS_MATCH_EXACTLY:
		cond_len = snprintf(out_cond, sizeof(out_cond), "%s = ?", field_name);
		*with_escape = false;
		break;
	case CONTACTS_MATCH_FULLSTRING:
		cond_len = snprintf(out_cond, sizeof(out_cond), "%s LIKE ? ESCAPE '%c'", field_name, CTSVC_DB_ESCAPE_CHAR);
		break;
	case CONTACTS_MATCH_CONTAINS:
		cond_len = snprintf(out_cond, sizeof(out_cond), "%s LIKE ('%%' || ? || '%%') ESCAPE '%c'", field_name, CTSVC_DB_ESCAPE_CHAR);
		break;
	case CONTACTS_MATCH_STARTSWITH:
		cond_len = snprintf(out_cond, sizeof(out_cond), "%s LIKE (? || '%%') ESCAPE '%c'", field_name, CTSVC_DB_ESCAPE_CHAR);
		break;
	case CONTACTS_MATCH_ENDSWITH:
		cond_len = snprintf(out_cond, sizeof(out_cond), "%s LIKE ('%%' || ?) ESCAPE '%c'", field_name, CTSVC_DB_ESCAPE_CHAR);
		break;
	case CONTACTS_MATCH_EXISTS:
		cond_len = snprintf(out_cond, sizeof(out_cond), "%s IS NOT NULL", field_name);
		break;
	default :
		CTS_ERR("Invalid parameter : int match rule (%d) is not supported", match);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	if (0 < cond_len)
		*condition = strdup(out_cond);

	return cond_len;
}

static inline int __ctsvc_db_create_str_condition(ctsvc_composite_filter_s *com_filter,
		ctsvc_attribute_filter_s *filter, char **condition, GSList **bind_text)
{
	int ret;
	const char *field_name;
	char out_cond[CTS_SQL_MAX_LEN] = {0};
	char *temp = NULL;
	int cond_len = 0;
	bool with_escape = true;

	*condition = NULL;

	field_name = __ctsvc_db_get_property_field_name(com_filter->properties,
							com_filter->property_count, QUERY_FILTER, filter->property_id);
	RETVM_IF(NULL == field_name, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : property id(%d)", filter->property_id);

	// number_filter condition is only used to find exactly matched number
	// based on internal logic : _NUMBER_COMPARE_
	if (filter->property_id == CTSVC_PROPERTY_NUMBER_NUMBER_FILTER
			|| filter->property_id == CTSVC_PROPERTY_PHONELOG_ADDRESS_FILTER
			|| filter->property_id == CTSVC_PROPERTY_SPEEDDIAL_NUMBER_FILTER)
		filter->match = CONTACTS_MATCH_EXACTLY;

	ret = __ctsvc_db_add_str_matching_rule(field_name, filter->match, &temp, &with_escape);
	RETVM_IF(ret <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
			"__ctsvc_db_add_str_matching_rule Fail");
	cond_len = snprintf(out_cond, sizeof(out_cond), "%s", temp);
	free(temp);
	temp = NULL;

	if (filter->value.s) {
		// number filter
		if (filter->property_id == CTSVC_PROPERTY_NUMBER_NUMBER_FILTER
			|| filter->property_id == CTSVC_PROPERTY_PHONELOG_ADDRESS_FILTER
			|| filter->property_id == CTSVC_PROPERTY_SPEEDDIAL_NUMBER_FILTER) {
			// number filter is for matching number depends on internal rule _NUMBER_COMPARE_
			char clean_num[strlen(filter->value.s)+1+5];	// for cc
			char normal_num[strlen(filter->value.s)+1+5];	// for cc
			bool add_condition = false;
			ret = ctsvc_clean_number(filter->value.s, clean_num, sizeof(clean_num), false);
			if (0 < ret) {
				ret = ctsvc_normalize_number(clean_num, normal_num, sizeof(normal_num), false);
				if (0 < ret) {
					char min_match[strlen(filter->value.s)+1+5];	// for cc
					ret = ctsvc_get_minmatch_number(normal_num, min_match, sizeof(min_match), ctsvc_get_phonenumber_min_match_digit());
					if (CONTACTS_ERROR_NONE == ret) {
						// minmatch filter is to improve performance
						*bind_text = g_slist_append(*bind_text, __ctsvc_db_get_str_with_escape(min_match, strlen(min_match), with_escape));

						// _NUMBER_COMPARE_(noraml_num, normalized keyword)
						if (STRING_EQUAL != strcmp(normal_num, "+")) {
							const char *number_field = NULL;
							if (filter->property_id == CTSVC_PROPERTY_NUMBER_NUMBER_FILTER)
								number_field = __ctsvc_db_get_property_field_name(com_filter->properties,
												com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_NUMBER_NORMALIZED_NUMBER);
							else if (filter->property_id == CTSVC_PROPERTY_PHONELOG_ADDRESS_FILTER)
								number_field = __ctsvc_db_get_property_field_name(com_filter->properties,
												com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_PHONELOG_NORMALIZED_ADDRESS);
							else if (filter->property_id == CTSVC_PROPERTY_SPEEDDIAL_NUMBER_FILTER)
								number_field = __ctsvc_db_get_property_field_name(com_filter->properties,
												com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_SPEEDDIAL_NORMALIZED_NUMBER);

							if (number_field) {
								cond_len += snprintf(out_cond+cond_len, sizeof(out_cond)-cond_len, " AND _NUMBER_COMPARE_(%s, ?, NULL, NULL)", number_field);
								*bind_text = g_slist_append(*bind_text, strdup(normal_num));
								add_condition = true;
							}
						}
					}
				}
			}

			if (add_condition == false) {
				// If fail to get normalized number then compare with original number
				const char *number_field = NULL;
				if (filter->property_id == CTSVC_PROPERTY_NUMBER_NUMBER_FILTER)
					number_field = __ctsvc_db_get_property_field_name(com_filter->properties,
									com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_NUMBER_NUMBER);
				else if (filter->property_id == CTSVC_PROPERTY_PHONELOG_ADDRESS_FILTER)
					number_field = __ctsvc_db_get_property_field_name(com_filter->properties,
									com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_PHONELOG_ADDRESS);
				else if (filter->property_id == CTSVC_PROPERTY_SPEEDDIAL_NUMBER_FILTER)
					number_field = __ctsvc_db_get_property_field_name(com_filter->properties,
									com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_SPEEDDIAL_NUMBER);


				if (number_field) {
					snprintf(out_cond, sizeof(out_cond), "%s = ?", number_field);
					*bind_text = g_slist_append(*bind_text, strdup(filter->value.s));
				}
			}
		}
		// normalized number
		else if (filter->property_id == CTSVC_PROPERTY_NUMBER_NORMALIZED_NUMBER
				|| filter->property_id == CTSVC_PROPERTY_PHONELOG_NORMALIZED_ADDRESS
				|| filter->property_id == CTSVC_PROPERTY_SPEEDDIAL_NORMALIZED_NUMBER) {
			char clean_num[strlen(filter->value.s)+1+5]; // for cc
			ret = ctsvc_clean_number(filter->value.s, clean_num, sizeof(clean_num), false);
			if (0 < ret) {
				bool add_condition = true;
				const char *clean_field = NULL;
				// has clean number or normalized number
				const char *cc = ctsvc_get_network_cc(false);
				if (cc && cc[0] == '7' && clean_num[0] == '8') {		// Russia
					char normal_num[strlen(clean_num)+1+5];	// for cc
					ret = ctsvc_normalize_number(clean_num, normal_num, sizeof(normal_num), false);
					if (0 < ret)
						*bind_text = g_slist_append(*bind_text, __ctsvc_db_get_str_with_escape(normal_num, strlen(normal_num), with_escape));
					else
						*bind_text = g_slist_append(*bind_text, __ctsvc_db_get_str_with_escape(clean_num, strlen(clean_num), with_escape));
				}
				else if (STRING_EQUAL != strcmp(clean_num, "+")) {
					*bind_text = g_slist_append(*bind_text, __ctsvc_db_get_str_with_escape(clean_num, strlen(clean_num), with_escape));
				}
				else
					add_condition = false;

				if (filter->property_id == CTSVC_PROPERTY_NUMBER_NORMALIZED_NUMBER)
					clean_field = __ctsvc_db_get_property_field_name(com_filter->properties,
									com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_NUMBER_CLEANED_NUMBER);
				else if (filter->property_id == CTSVC_PROPERTY_PHONELOG_NORMALIZED_ADDRESS)
					clean_field =  __ctsvc_db_get_property_field_name(com_filter->properties,
										com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_PHONELOG_CLEANED_ADDRESS);
				else if (filter->property_id == CTSVC_PROPERTY_SPEEDDIAL_NORMALIZED_NUMBER)
					clean_field =  __ctsvc_db_get_property_field_name(com_filter->properties,
										com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_SPEEDDIAL_CLEANED_NUMBER);

				if (clean_field) {
					if (add_condition)
						cond_len += snprintf(out_cond+cond_len, sizeof(out_cond)-cond_len, " OR ");
					ret = __ctsvc_db_add_str_matching_rule(clean_field, filter->match, &temp, &with_escape);
					RETVM_IF(ret <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
							"__ctsvc_db_add_str_matching_rule Fail");
					cond_len += snprintf(out_cond+cond_len, sizeof(out_cond)-cond_len, "%s", temp);
					free(temp);
					temp = NULL;
					*bind_text = g_slist_append(*bind_text, __ctsvc_db_get_str_with_escape(clean_num, strlen(clean_num), with_escape));
				}
			}
			else if (ret == 0) {
				// If fail to get cleaned number then compare with original number
				const char *number_field = NULL;
				if (filter->property_id == CTSVC_PROPERTY_NUMBER_NORMALIZED_NUMBER)
					number_field = __ctsvc_db_get_property_field_name(com_filter->properties,
									com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_NUMBER_NUMBER);
				else if (filter->property_id == CTSVC_PROPERTY_PHONELOG_NORMALIZED_ADDRESS)
					number_field =  __ctsvc_db_get_property_field_name(com_filter->properties,
										com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_PHONELOG_ADDRESS);
				else if (filter->property_id == CTSVC_PROPERTY_SPEEDDIAL_NORMALIZED_NUMBER)
					number_field =  __ctsvc_db_get_property_field_name(com_filter->properties,
										com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_SPEEDDIAL_NUMBER);

				if (number_field) {
					ret = __ctsvc_db_add_str_matching_rule(number_field, filter->match, &temp, &with_escape);
					RETVM_IF(ret <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
								"__ctsvc_db_add_str_matching_rule Fail");
					cond_len = snprintf(out_cond, sizeof(out_cond), "%s", temp);
					free(temp);
					temp = NULL;
					*bind_text = g_slist_append(*bind_text, __ctsvc_db_get_str_with_escape(filter->value.s, strlen(filter->value.s), with_escape));
				}
			}
			else
				return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		// cleaned number
		else if (filter->property_id == CTSVC_PROPERTY_NUMBER_CLEANED_NUMBER
				|| filter->property_id == CTSVC_PROPERTY_PHONELOG_CLEANED_ADDRESS
				|| filter->property_id == CTSVC_PROPERTY_SPEEDDIAL_CLEANED_NUMBER) {
			char clean_num[strlen(filter->value.s)+1+5]; // for cc
			ret = ctsvc_clean_number(filter->value.s, clean_num, sizeof(clean_num), false);
			if (0 < ret) {
				*bind_text = g_slist_append(*bind_text, __ctsvc_db_get_str_with_escape(clean_num, strlen(clean_num), with_escape));
			}
			else if (ret == 0) {
				// If fail to get cleaned number then compare with original number
				const char *number_field = NULL;
				if (filter->property_id == CTSVC_PROPERTY_NUMBER_CLEANED_NUMBER)
					number_field = __ctsvc_db_get_property_field_name(com_filter->properties,
									com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_NUMBER_NUMBER);
				else if (filter->property_id == CTSVC_PROPERTY_PHONELOG_CLEANED_ADDRESS)
					number_field =  __ctsvc_db_get_property_field_name(com_filter->properties,
										com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_PHONELOG_ADDRESS);
				else if (filter->property_id == CTSVC_PROPERTY_SPEEDDIAL_CLEANED_NUMBER)
					number_field =  __ctsvc_db_get_property_field_name(com_filter->properties,
										com_filter->property_count, QUERY_FILTER, CTSVC_PROPERTY_SPEEDDIAL_NUMBER);

				if (number_field) {
					ret = __ctsvc_db_add_str_matching_rule(number_field, filter->match, &temp, &with_escape);
					RETVM_IF(ret <= 0, CONTACTS_ERROR_INVALID_PARAMETER,
								"__ctsvc_db_add_str_matching_rule Fail");
					cond_len = snprintf(out_cond, sizeof(out_cond), "%s", temp);
					free(temp);
					temp = NULL;
					*bind_text = g_slist_append(*bind_text, __ctsvc_db_get_str_with_escape(filter->value.s, strlen(filter->value.s), with_escape));
				}
			}
			else
				return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		else if (filter->property_id == CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL
				|| filter->property_id == CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL
				|| filter->property_id == CTSVC_PROPERTY_MY_PROFILE_IMAGE_THUMBNAIL
				|| filter->property_id == CTSVC_PROPERTY_IMAGE_PATH) {
			if (STRING_EQUAL == strncmp(filter->value.s, CTSVC_CONTACT_IMG_FULL_LOCATION, strlen(CTSVC_CONTACT_IMG_FULL_LOCATION))) {
				*bind_text = g_slist_append(*bind_text,
									__ctsvc_db_get_str_with_escape(filter->value.s+strlen(CTSVC_CONTACT_IMG_FULL_LOCATION)+1,
									strlen(filter->value.s)-strlen(CTSVC_CONTACT_IMG_FULL_LOCATION)-1, with_escape));
			}
			else
				*bind_text = g_slist_append(*bind_text, __ctsvc_db_get_str_with_escape(filter->value.s, strlen(filter->value.s), with_escape));
		}
		else if (filter->property_id == CTSVC_PROPERTY_GROUP_IMAGE) {
			if (STRING_EQUAL == strncmp(filter->value.s, CTS_GROUP_IMAGE_LOCATION, strlen(CTS_GROUP_IMAGE_LOCATION))) {
				*bind_text = g_slist_append(*bind_text,
									__ctsvc_db_get_str_with_escape(filter->value.s+strlen(CTS_GROUP_IMAGE_LOCATION)+1,
									strlen(filter->value.s)-strlen(CTS_GROUP_IMAGE_LOCATION)-1, with_escape));
			}
			else
				*bind_text = g_slist_append(*bind_text, __ctsvc_db_get_str_with_escape(filter->value.s, strlen(filter->value.s), with_escape));
		}
		else if (filter->property_id == CTSVC_PROPERTY_COMPANY_LOGO) {
			if (STRING_EQUAL == strncmp(filter->value.s, CTS_LOGO_IMAGE_LOCATION, strlen(CTS_LOGO_IMAGE_LOCATION))) {
				*bind_text = g_slist_append(*bind_text,
									__ctsvc_db_get_str_with_escape(filter->value.s+strlen(CTS_LOGO_IMAGE_LOCATION)+1,
									strlen(filter->value.s)-strlen(CTS_LOGO_IMAGE_LOCATION)-1, with_escape));
			}
			else
				*bind_text = g_slist_append(*bind_text, __ctsvc_db_get_str_with_escape(filter->value.s, strlen(filter->value.s), with_escape));

		}
		else
			*bind_text = g_slist_append(*bind_text, __ctsvc_db_get_str_with_escape(filter->value.s, strlen(filter->value.s), with_escape));
	}

	*condition = strdup(out_cond);

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_db_create_bool_condition(ctsvc_composite_filter_s *com_filter,
		ctsvc_attribute_filter_s *filter, char **condition)
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

#define CTSVC_FILTER_LENGTH	100

// If there are too many condition, sqlite return fail SQLITE_ERROR(1).
// Expression tree is too large (maximum depth 1000).
// It is related to SQLITE_LIMIT_EXPR_DEPTH.
static inline int __ctsvc_db_create_composite_condition(ctsvc_composite_filter_s *com_filter,
		char **condition, GSList **bind_text)
{
	RETV_IF(NULL == com_filter, CONTACTS_ERROR_INVALID_PARAMETER);

	int len;
	int temp_len;
	int buf_size;
	char *cond;
	char *out_cond = NULL;
	GSList *cursor_filter;
	GSList *cursor_ops;
	ctsvc_filter_s *filter;
	contacts_filter_operator_e op;
	GSList *bind;
	GSList *filters = com_filter->filters;
	GSList *ops = com_filter->filter_ops;

	*condition = NULL;
	// the case : did not set filter condition after calling contacts_filter_create()
	RETV_IF(NULL == filters, CONTACTS_ERROR_NONE);

	cond = NULL;
	bind = NULL;

	filter = (ctsvc_filter_s *)filters->data;
	if (filter->filter_type == CTSVC_FILTER_COMPOSITE)
		__ctsvc_db_create_composite_condition((ctsvc_composite_filter_s *)filter, &cond, bind_text);
	else
		__ctsvc_db_create_attribute_condition(com_filter, (ctsvc_attribute_filter_s*)filter, &cond, bind_text);

	buf_size = CTSVC_FILTER_LENGTH;
	out_cond = calloc(1, buf_size);
	len = 0;
	if (cond) {
		temp_len = SAFE_SNPRINTF(&out_cond, &buf_size, len, "(");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&out_cond, &buf_size, len, cond);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&out_cond, &buf_size, len, ")");
		if (0 <= temp_len) len+= temp_len;
		free(cond);
	}

	cursor_filter = filters->next;
	for (cursor_ops=ops; cursor_ops && cursor_filter; cursor_filter=cursor_filter->next, cursor_ops=cursor_ops->next) {
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
				temp_len = SAFE_SNPRINTF(&out_cond, &buf_size, len, " AND (");
			else
				temp_len = SAFE_SNPRINTF(&out_cond, &buf_size, len, " OR (");
			if (0 <= temp_len) len+= temp_len;

			temp_len = SAFE_SNPRINTF(&out_cond, &buf_size, len, cond);
			if (0 <= temp_len) len+= temp_len;
			temp_len = SAFE_SNPRINTF(&out_cond, &buf_size, len, ")");
			if (0 <= temp_len) len+= temp_len;

			if (bind)
				*bind_text = g_slist_concat(*bind_text, bind);
			free(cond);
		}
	}

	*condition = out_cond;

	return CONTACTS_ERROR_NONE;
}

// Make and execute 'UPDATE' sqlite statement to update record
int ctsvc_db_update_record_with_set_query(const char *set, GSList *bind_text, const char *table, int id)
{
	int ret = CONTACTS_ERROR_NONE;
	char query[CTS_SQL_MAX_LEN] = {0};
	sqlite3_stmt *stmt = NULL;
	GSList *cursor = NULL;

	snprintf(query, sizeof(query), "UPDATE %s SET %s WHERE id = %d", table, set, id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	if (bind_text) {
		int i = 0;
		for (cursor=bind_text,i=1;cursor;cursor=cursor->next,i++) {
			const char *text = cursor->data;
			if (text && *text)
				ctsvc_stmt_bind_text(stmt, i, text);
		}
	}
	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		return ret;
	}
	ctsvc_stmt_finalize(stmt);
	return ret;
}

// make 'SET' sqlite statement to update record
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

	for (i=0;i<property_info_count;i++) {
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

static int __ctsvc_db_create_projection(const char *view_uri, const property_info_s *properties, int ids_count,
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
		// add all properties
		for (i=0;i<ids_count;i++) {
			if (CTSVC_VIEW_DATA_TYPE_REC == (properties[i].property_id & CTSVC_VIEW_DATA_TYPE_MASK))
				continue;

			if (properties[i].fields)
				field_name = properties[i].fields;
			else if (properties[i].property_id == CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX) {
				snprintf(temp, sizeof(temp), "_NORMALIZE_INDEX_(%s)", ctsvc_get_sort_name_column());
				field_name = temp;
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
#ifdef ENABLE_LOG_FEATURE
	if (STRING_EQUAL == strcmp(view_uri, _contacts_person_phone_log._uri))
		return false;
#endif // ENABLE_LOG_FEATURE

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

#define SORT_CHECK_LEN	7

int ctsvc_db_make_get_records_query_stmt(ctsvc_query_s *s_query, int offset, int limit, cts_stmt *stmt)
{
	char *query = NULL;
	char temp_str[100] = {0};
	int query_size = 0;
	int temp_len = 0;
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

	ret = __ctsvc_db_create_projection(s_query->view_uri, s_query->properties, s_query->property_count,
								s_query->projection, s_query->projection_count, &projection);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_create_projection Fail(%d)", ret);
		return ret;
	}

	query_size = CTS_SQL_MAX_LEN;
	query = calloc(1, query_size);
	len = 0;
	if (s_query->distinct)
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, "SELECT DISTINCT ");
	else
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, "SELECT ");
	if (0 <= temp_len) len+= temp_len;
	temp_len = SAFE_SNPRINTF(&query, &query_size, len, projection);
	if (0 <= temp_len) len+= temp_len;
	temp_len = SAFE_SNPRINTF(&query, &query_size, len, " FROM ");
	if (0 <= temp_len) len+= temp_len;

	temp_len = SAFE_SNPRINTF(&query, &query_size, len, " (");
	if (0 <= temp_len) len+= temp_len;
	temp_len = SAFE_SNPRINTF(&query, &query_size, len, table);
	if (0 <= temp_len) len+= temp_len;
	temp_len = SAFE_SNPRINTF(&query, &query_size, len, ")");
	if (0 <= temp_len) len+= temp_len;

	if (s_query->filter) {
		ret = __ctsvc_db_create_composite_condition(s_query->filter, &condition, &bind_text);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("__ctsvc_db_create_composite_condition Fail(%d)", ret);
			free(projection);
			return ret;
		}
		if (condition && *condition) {
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, " WHERE (");
			if (0 <= temp_len) len+= temp_len;
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, condition);
			if (0 <= temp_len) len+= temp_len;
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, ")");
			if (0 <= temp_len) len+= temp_len;
		}
	}

	// If the view_uri has display_name, default sortkey is display_name
	if (__ctsvc_db_view_has_display_name(s_query->view_uri, s_query->properties, s_query->property_count))
		sortkey = ctsvc_get_sort_column();

	if (s_query->sort_property_id) {
		const char *field_name;

		switch(s_query->sort_property_id) {
		case CTSVC_PROPERTY_PERSON_DISPLAY_NAME:
		case CTSVC_PROPERTY_CONTACT_DISPLAY_NAME:
			if (sortkey) {
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, " ORDER BY ");
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, sortkey);
				if (0 <= temp_len) len+= temp_len;
				if (false == s_query->sort_asc) {
					temp_len = SAFE_SNPRINTF(&query, &query_size, len, " DESC ");
					if (0 <= temp_len) len+= temp_len;
				}
			}
			break;
		default :
			field_name = __ctsvc_db_get_property_field_name(s_query->properties,
								s_query->property_count, QUERY_SORTKEY, s_query->sort_property_id);
			if (field_name) {
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, " ORDER BY ");
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, field_name);
				if (0 <= temp_len) len+= temp_len;

//				if (sortkey)
//					len += snprintf(query+len, sizeof(query)-len, ", %s", sortkey);
				if (false == s_query->sort_asc) {
					temp_len = SAFE_SNPRINTF(&query, &query_size, len, " DESC ");
					if (0 <= temp_len) len+= temp_len;
				}
			}
			else if (sortkey) {
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, " ORDER BY ");
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, sortkey);
				if (0 <= temp_len) len+= temp_len;
			}
			break;
		}
	}
	else if (sortkey) {
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, " ORDER BY ");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, sortkey);
		if (0 <= temp_len) len+= temp_len;
	}
	else if (STRING_EQUAL == strcmp(s_query->view_uri, CTSVC_VIEW_URI_GROUP)) {
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, " ORDER BY group_prio");
		if (0 <= temp_len) len+= temp_len;
	}

	if (0 != limit) {
		snprintf(temp_str, sizeof(temp_str), " LIMIT %d", limit);
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, temp_str);
		if (0 <= temp_len) len+= temp_len;
		if (0 < offset) {
			snprintf(temp_str, sizeof(temp_str), " OFFSET %d", offset);
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, temp_str);
			if (0 <= temp_len) len+= temp_len;
		}
	}

	free(condition);
	free(projection);

	ret = ctsvc_query_prepare(query, stmt);
	free(query);
	if (NULL == *stmt) {
		CTS_ERR("DB error : ctsvc_query_prepare() Fail(%d)", ret);
		for (cursor=bind_text;cursor;cursor=cursor->next)
			free(cursor->data);
		g_slist_free(bind_text);
		return ret;
	}

	for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
		ctsvc_stmt_bind_copy_text(*stmt, i, cursor->data, strlen(cursor->data));

	for (cursor=bind_text;cursor;cursor=cursor->next)
		free(cursor->data);
	g_slist_free(bind_text);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_records_with_query_exec(ctsvc_query_s *query, int offset,
		int limit, contacts_list_h* out_list)
{
	int ret;
	int i;
	int type;
	cts_stmt stmt = NULL;
	contacts_list_h list = NULL;

	ret = ctsvc_db_make_get_records_query_stmt(query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "DB error : ctsvc_db_make_get_records_query_stmt(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		int field_count;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		contacts_record_create(query->view_uri, (contacts_record_h*)&record);

		if (0 == query->projection_count)
			field_count = query->property_count;
		else {
			field_count = query->projection_count;

			if (CONTACTS_ERROR_NONE != ctsvc_record_set_projection_flags(record, query->projection, query->projection_count, query->property_count)) {
				ASSERT_NOT_REACHED("To set projection is Fail.\n");
			}
		}

		for (i=0;i<field_count;i++) {
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
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_all_records_exec(const char *view_uri, const property_info_s* properties, int ids_count,
		const char *projection, int offset,	int limit, contacts_list_h* out_list)
{
	char query[CTS_SQL_MAX_LEN] = {0};
	const char *table;
	int len;
	int ret;
	int i;
	int type;
	cts_stmt stmt = NULL;
	contacts_list_h list = NULL;
	const char *sortkey;

	ret = ctsvc_db_get_table_name(view_uri, &table);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "Invalid parameter : view uri (%s)", view_uri);

	len = snprintf(query, sizeof(query), "SELECT %s FROM ", projection);

	len += snprintf(query+len, sizeof(query)-len, " (%s)", table);

	if (__ctsvc_db_view_has_display_name(view_uri, properties, ids_count)) {
		sortkey = ctsvc_get_sort_column();
		len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s", sortkey);
	} else if (STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_GROUP))
		len += snprintf(query+len, sizeof(query)-len, " ORDER BY group_prio");

	if (0 != limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		contacts_record_create(view_uri, &record);
		for (i=0;i<ids_count;i++) {
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
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_all_records(const char* view_uri, int offset, int limit, contacts_list_h* out_list)
{
	int ret;
	unsigned int count;
	char *projection;

	const property_info_s *p = ctsvc_view_get_all_property_infos(view_uri, &count);
	ret = __ctsvc_db_create_projection(view_uri, p, count, NULL, 0, &projection);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_create_projection Fail(%d)", ret);

	ret = __ctsvc_db_get_all_records_exec(view_uri, p, count, projection, offset, limit, out_list);
	free(projection);

	return ret;
}

static inline bool __ctsvc_db_view_can_keyword_search(const char *view_uri)
{
	RETV_IF(NULL == view_uri, false);

	if (STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_PERSON)
		|| STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_CONTACT)
		|| STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_NUMBER)
		|| STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_EMAIL)
		|| STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP)
		|| STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_ASSIGNED)
		|| STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_NOT_ASSIGNED)) {
		return true;
	}
	return false;
}

static char* __ctsvc_db_make_search_keyword(const char *keyword)
{
	int size;
	if (keyword == NULL)
		return NULL;

	size = strlen(keyword);
	if (strstr(keyword, " ")) {
		int i = 0;
		int j = 0;
		char search_keyword[size * 2+1];
		for (i=0;i<size;i++) {
			if (j>0 && keyword[i] == ' ') {
				if (search_keyword[j-1] != ' ')
					search_keyword[j++] = '*';
			}
			search_keyword[j++] = keyword[i];
		}
		if (0 < j && search_keyword[j-1] != ' ')
			search_keyword[j++] = '*';
		search_keyword[j] = '\0';
		return strdup(search_keyword);
	}
	else {
		char search_keyword[size+2];
		snprintf(search_keyword, sizeof(search_keyword), "%s*", keyword);
		return strdup(search_keyword);
	}
}

static int __ctsvc_db_append_search_query_range(char **query, int *query_size, int len, int range, char *name, char *number, char *data)
{
	bool first = true;
	int temp_len;

	temp_len = SAFE_SNPRINTF(query, query_size, len, " '");
	if (0 <= temp_len) len += temp_len;
	if (range & CONTACTS_SEARCH_RANGE_NAME) {
		temp_len = SAFE_SNPRINTF(query, query_size, len, "name:");
		if (0 <= temp_len) len += temp_len;
		temp_len = SAFE_SNPRINTF(query, query_size, len, name);
		if (0 <= temp_len) len += temp_len;
		first = false;
	}

	if (range & CONTACTS_SEARCH_RANGE_NUMBER) {
		if (first == false) {
			temp_len = SAFE_SNPRINTF(query, query_size, len, " OR ");
			if (0 <= temp_len) len += temp_len;
		}
		temp_len = SAFE_SNPRINTF(query, query_size, len, "number:");
		if (0 <= temp_len) len += temp_len;
		temp_len = SAFE_SNPRINTF(query, query_size, len, number);
		if (0 <= temp_len) len += temp_len;
		first = false;
	}

	if (range & CONTACTS_SEARCH_RANGE_DATA) {
		if (first == false) {
			temp_len = SAFE_SNPRINTF(query, query_size, len, " OR ");
			if (0 <= temp_len) len += temp_len;
		}
		temp_len = SAFE_SNPRINTF(query, query_size, len, "data:");
		if (0 <= temp_len) len += temp_len;
		temp_len = SAFE_SNPRINTF(query, query_size, len, data);
		if (0 <= temp_len) len += temp_len;
		first = false;
	}

	temp_len = SAFE_SNPRINTF(query, query_size, len, "' ");
	if (0 <= temp_len) len += temp_len;
	return len;
}

static int __ctsvc_db_append_search_query(const char *src, char **query, int *query_size, int len, int range)
{
	bool phonenumber = true;
	int i = 0;
	int keyword_temp_len = 0;
	int temp_len;

	if (ctsvc_is_phonenumber(src) == false || STRING_EQUAL == strcmp(src, "+")) {
		phonenumber = false;
	}

	if (strstr(src, "@")) {
		// If the search key is email address format, DO NOT search it from NAME
		range &= ~CONTACTS_SEARCH_RANGE_NAME;
	}

	if (STRING_EQUAL == strcmp(src, "+")) {
		range &= ~CONTACTS_SEARCH_RANGE_NUMBER;
	}

	char* keyword = NULL;
	int keyword_size = 0;
	bool use_replaced_keyword = true;
	// full width characters -> half width characters (apply to only FW ASCII & some symbols)
	if (ctsvc_get_halfwidth_string(src, &keyword, &keyword_size) != CONTACTS_ERROR_NONE) {
		CTS_ERR("UChar converting error : ctsvc_get_halfwidth_string() Fail");
		keyword = (char*)src;
		use_replaced_keyword = false;
	}

	char *search_keyword = NULL;
	search_keyword = __ctsvc_db_make_search_keyword(keyword);

	if (phonenumber) {
		temp_len = SAFE_SNPRINTF(query, query_size, len, "(SELECT contact_id FROM ");
		if (0 <= temp_len) len += temp_len;
		temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
		if (0 <= temp_len) len += temp_len;
		temp_len = SAFE_SNPRINTF(query, query_size, len, " WHERE ");
		if (0 <= temp_len) len += temp_len;
		temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
		if (0 <= temp_len) len += temp_len;
		temp_len = SAFE_SNPRINTF(query, query_size, len, " MATCH ");
		if (0 <= temp_len) len += temp_len;

		temp_len = __ctsvc_db_append_search_query_range(query, query_size, len,
								range, search_keyword, search_keyword, search_keyword);
		if (0 <= temp_len) len = temp_len;

		if (range & CONTACTS_SEARCH_RANGE_NUMBER) {
			// to find contact which has number including keyword
			// FTS can only startwiths search
			char clean_number[SAFE_STRLEN(keyword)+1];
			int err = ctsvc_clean_number(keyword, clean_number, sizeof(clean_number), false);
			if (0 < err) {
				const char *cc = ctsvc_get_network_cc(false);
				temp_len = SAFE_SNPRINTF(query, query_size, len, " UNION SELECT contact_id FROM ");
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_PHONE_LOOKUP);
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, " WHERE number LIKE '%%");
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, clean_number);
				if (0 <= temp_len) len += temp_len;

				if (cc && cc[0] == '7' && clean_number[0] == '8') {		// Russia
					char normal_num[strlen(clean_number)+1+5];	// for cc
					int ret;
					ret = ctsvc_normalize_number(clean_number, normal_num, sizeof(normal_num), false);
					if (0 < ret) {
						temp_len = SAFE_SNPRINTF(query, query_size, len, "%%' OR number LIKE '%%");
						if (0 <= temp_len) len+= temp_len;
						temp_len = SAFE_SNPRINTF(query, query_size, len, normal_num);
						if (0 <= temp_len) len+= temp_len;
					}
				}

				temp_len = SAFE_SNPRINTF(query, query_size, len, "%%' ");
				if (0 <= temp_len) len += temp_len;
			}
		}
		temp_len = SAFE_SNPRINTF(query, query_size, len, ")");
		if (0 <= temp_len) len += temp_len;
	}
	else {
		char *normalized_name = NULL;
		int lang = CTSVC_LANG_OTHERS;
		bool add_brace = false;
		char *hiragana = NULL;
		char *search_hiragana = NULL;
		bool need_union = false;

		lang = ctsvc_normalize_str(keyword, &normalized_name);
		temp_len = SAFE_SNPRINTF(query, query_size, len, "(");
		if (0 <= temp_len) len += temp_len;

		if (CTSVC_LANG_JAPANESE == lang) {
			ctsvc_convert_japanese_to_hiragana(keyword, &hiragana);
			search_hiragana = __ctsvc_db_make_search_keyword(hiragana);
		}

		if (range & CONTACTS_SEARCH_RANGE_NUMBER) {
			temp_len = SAFE_SNPRINTF(query, query_size, len, "SELECT contact_id FROM ");
			if (0 <= temp_len) len += temp_len;
			temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
			if (0 <= temp_len) len += temp_len;
			temp_len = SAFE_SNPRINTF(query, query_size, len, " WHERE ");
			if (0 <= temp_len) len += temp_len;
			temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
			if (0 <= temp_len) len += temp_len;
			temp_len = SAFE_SNPRINTF(query, query_size, len, " MATCH ");
			if (0 <= temp_len) len += temp_len;

			if (CTSVC_LANG_JAPANESE == lang) {
				temp_len = __ctsvc_db_append_search_query_range(query, query_size, len,
									CONTACTS_SEARCH_RANGE_NUMBER, NULL, search_hiragana, NULL);
				if (0 <= temp_len) len = temp_len;
			}
			else {
				temp_len = __ctsvc_db_append_search_query_range(query, query_size, len,
									CONTACTS_SEARCH_RANGE_NUMBER, NULL, search_keyword, NULL);
				if (0 <= temp_len) len = temp_len;
			}
			need_union = true;
		}

		if (range & CONTACTS_SEARCH_RANGE_DATA) {
			if (need_union) {
				temp_len = SAFE_SNPRINTF(query, query_size, len, " UNION SELECT contact_id FROM (");
				if (0 <= temp_len) len += temp_len;
				add_brace = true;
			}
			temp_len = SAFE_SNPRINTF(query, query_size, len, " SELECT contact_id FROM ");
			if (0 <= temp_len) len += temp_len;
			temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
			if (0 <= temp_len) len += temp_len;
			temp_len = SAFE_SNPRINTF(query, query_size, len, " WHERE ");
			if (0 <= temp_len) len += temp_len;
			temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
			if (0 <= temp_len) len += temp_len;
			temp_len = SAFE_SNPRINTF(query, query_size, len, " MATCH ");
			if (0 <= temp_len) len += temp_len;

			if (CTSVC_LANG_JAPANESE == lang) {
				temp_len = __ctsvc_db_append_search_query_range(query, query_size, len,
									CONTACTS_SEARCH_RANGE_DATA, NULL, NULL, search_hiragana);
				if (0 <= temp_len) len = temp_len;
			}
			else {
				keyword_temp_len = strlen(search_keyword);
				// replace '-' -> '_' because FTS does not support search '-'
				char temp_str[keyword_temp_len+1];
				for (i=0;i<keyword_temp_len;i++) {
					if (search_keyword[i] == '-') {
						temp_str[i] = '_';
					}
					else
						temp_str[i] = search_keyword[i];
				}
				temp_str[i] = '\0';
				temp_len = __ctsvc_db_append_search_query_range(query, query_size, len,
									CONTACTS_SEARCH_RANGE_DATA, NULL, NULL, temp_str);
				if (0 <= temp_len) len = temp_len;
			}
			if (add_brace) {
				temp_len = SAFE_SNPRINTF(query, query_size, len, ") ");
				if (0 <= temp_len) len += temp_len;
				add_brace = false;
			}
			need_union = true;
		}

		if (range & CONTACTS_SEARCH_RANGE_NAME) {
			if (need_union) {
				temp_len = SAFE_SNPRINTF(query, query_size, len, " UNION SELECT contact_id FROM ");
				if (0 <= temp_len) len += temp_len;
			}

			if (CTSVC_LANG_KOREAN == lang) {		// chosung search
				char *chosung = calloc(1, strlen(keyword) * 5);
				char *korean_pattern = calloc(1, strlen(keyword) * 5);
				char *search_chosung = NULL;
				int count = 0;

				// If try to find '홍길동' by 'ㄱ동'
				// search by 'ㄱㄷ' in search_index table
				// intersect
				// search by '*ㄱ*동*' in name_lookup table

				count = ctsvc_get_chosung(keyword, chosung, strlen(keyword) * 5);
				ctsvc_get_korean_search_pattern(keyword, korean_pattern, strlen(keyword) * 5);

				if (0 < count)
					search_chosung = __ctsvc_db_make_search_keyword(chosung);
				else
					search_chosung = __ctsvc_db_make_search_keyword(keyword);

				temp_len = SAFE_SNPRINTF(query, query_size, len, " (SELECT contact_id FROM ");
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, " WHERE ");
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, " MATCH ");
				if (0 <= temp_len) len += temp_len;

				temp_len = __ctsvc_db_append_search_query_range(query, query_size, len,
									CONTACTS_SEARCH_RANGE_NAME, search_chosung, NULL, NULL);
				if (0 <= temp_len) len = temp_len;

				temp_len = SAFE_SNPRINTF(query, query_size, len,  " INTERSECT SELECT contact_id FROM ");
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_NAME_LOOKUP);
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, " WHERE name GLOB '*");
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, korean_pattern);
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, "*' ");
				if (0 <= temp_len) len += temp_len;

				free(chosung);
				free(korean_pattern);
				free(search_chosung);
			}
			else if (CTSVC_LANG_JAPANESE == lang) {	// hiragana search
				temp_len = SAFE_SNPRINTF(query, query_size, len,  " (SELECT contact_id FROM ");
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, " WHERE ");
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, " MATCH ");
				if (0 <= temp_len) len += temp_len;

				temp_len = __ctsvc_db_append_search_query_range(query, query_size, len,
									CONTACTS_SEARCH_RANGE_NAME, search_hiragana, NULL, NULL);
				if (0 <= temp_len) len = temp_len;
			}
			else if (CONTACTS_ERROR_NONE <= lang) {	// normalized string search
				char *search_normal_name = NULL;
				search_normal_name = __ctsvc_db_make_search_keyword(normalized_name);
				temp_len = SAFE_SNPRINTF(query, query_size, len,  " (SELECT contact_id FROM ");
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, " WHERE ");
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, " MATCH ");
				if (0 <= temp_len) len += temp_len;

				temp_len = __ctsvc_db_append_search_query_range(query, query_size, len,
									CONTACTS_SEARCH_RANGE_NAME, search_normal_name, NULL, NULL);
				if (0 <= temp_len) len = temp_len;
				free(search_normal_name);
			}
			else {		// original keyword search
				temp_len = SAFE_SNPRINTF(query, query_size, len,  " (SELECT contact_id FROM ");
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, " WHERE ");
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_SEARCH_INDEX);
				if (0 <= temp_len) len += temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, " MATCH ");
				if (0 <= temp_len) len += temp_len;

				temp_len = __ctsvc_db_append_search_query_range(query, query_size, len,
									CONTACTS_SEARCH_RANGE_NAME, search_keyword, NULL, NULL);
				if (0 <= temp_len) len = temp_len;
			}

			int j = 0;
			keyword_temp_len = strlen(keyword);
			char temp_str[keyword_temp_len*2+1];
			for (i=0;i<keyword_temp_len;i++) {
				if (keyword[i] == '\'' || keyword[i] == '_' || keyword[i] == '%' || keyword[i] == '\\') {
					temp_str[j++] = CTSVC_DB_ESCAPE_CHAR;
				}
				temp_str[j++] = keyword[i];
			}
			temp_str[j] = '\0';
			temp_len = SAFE_SNPRINTF(query, query_size, len, " UNION SELECT contact_id FROM ");
			if (0 <= temp_len) len += temp_len;
			temp_len = SAFE_SNPRINTF(query, query_size, len, CTS_TABLE_NAME_LOOKUP);
			if (0 <= temp_len) len += temp_len;
			temp_len = SAFE_SNPRINTF(query, query_size, len, " WHERE name LIKE '");
			if (0 <= temp_len) len += temp_len;
			temp_len = SAFE_SNPRINTF(query, query_size, len, temp_str);
			if (0 <= temp_len) len += temp_len;
			temp_len = SAFE_SNPRINTF(query, query_size, len, "%%' ESCAPE '\\') ");	//CTSVC_DB_ESCAPE_CHAR
			if (0 <= temp_len) len += temp_len;
		}

		free(normalized_name);
		free(hiragana);
		free(search_hiragana);

		temp_len = SAFE_SNPRINTF(query, query_size, len, ") ");
		if (0 <= temp_len) len += temp_len;
	}

	free(search_keyword);

	if (use_replaced_keyword)
		free(keyword);

	return len;
}

static int __ctsvc_db_search_records_character_count(const char *keyword)
{
	int char_len = 0;
	int str_len = strlen(keyword);
	int i;
	int count = 0;
	bool after_space = true;

	for (i=0;i<str_len; i+=char_len) {
		char_len = ctsvc_check_utf8(keyword[i]);
		if (char_len == 1 && keyword[i] == ' ') {
			after_space = true;
			continue;
		}
		if (after_space == true) {
			count++;
			after_space = false;
		}
	}
	return count;
}

static int __ctsvc_db_search_records_append_sort(const char *view_uri,
		const char *sortkey, const char *keyword, int len, char **query, int *query_size)
{
	int i;
	int temp_len;

	if (ctsvc_get_primary_sort() == CTSVC_SORT_KOREAN) {
		contacts_name_sorting_order_e order;
		const char *field = NULL;
		char *temp_keyword = NULL;
		contacts_setting_get_name_sorting_order(&order);

		if (CONTACTS_NAME_SORTING_ORDER_FIRSTLAST == order)
			field = "display_name";
		else
			field = "reverse_display_name";

		if (ctsvc_has_chosung(keyword)) {
			if (__ctsvc_db_search_records_character_count(keyword) == 1) {
				int j;
				int m;
				int char_len = 0;
				int keyword_len = strlen(keyword);
				bool first = true;
				char temp_str[((250*keyword_len) + 30) * SORT_CHECK_LEN + 50];
				int temp_str_len = 0;

				temp_str_len = snprintf(temp_str+temp_str_len, sizeof(temp_str)-temp_str_len,
								" ORDER BY CASE ");

				for (j=1;j<=SORT_CHECK_LEN;j++) {
					temp_str_len += snprintf(temp_str+temp_str_len, sizeof(temp_str)-temp_str_len,
											" WHEN ");
					for (i=0, m=0;i<keyword_len;i+=char_len, m++) {
						char temp[10] = {0};
						int k = -1;
						char_len = ctsvc_check_utf8(keyword[i]);
						if (char_len <= 0) {
							char_len = 1;
							continue;
						}
						memcpy(temp, &keyword[i], char_len);
						temp[char_len] = '\0';

						if (char_len == 1 && temp[0] == ' ')
							continue;
						if (first == false && i != 0)
							temp_str_len += snprintf(temp_str+temp_str_len, sizeof(temp_str)-temp_str_len, " AND ");
						if (ctsvc_is_chosung(temp)) {
							for (k=0;k<19;k++) {
								if (STRING_EQUAL == strcmp(hangul_syllable[k][0], temp))
									break;
							}
						}

						if (0<=k && k<=18) {
							temp_str_len += snprintf(temp_str+temp_str_len, sizeof(temp_str)-temp_str_len,
											" ((substr(%s, %d, 1) BETWEEN '%s' AND '%s') OR (substr(%s, %d, 1) = '%s'))  ",
											field, j+m, hangul_syllable[k][1], hangul_syllable[k][2],
											field, j+m, temp);
						}
						else
							temp_str_len += snprintf(temp_str+temp_str_len, sizeof(temp_str)-temp_str_len,
											" (substr(%s, %d, 1) = '%s')  ",
											field, j+m, temp);
						if (first)
							first = false;
					}
					temp_str_len += snprintf(temp_str+temp_str_len, sizeof(temp_str)-temp_str_len,
								" THEN %d ", j);
				}
				temp_str_len = snprintf(temp_str+temp_str_len, sizeof(temp_str)-temp_str_len,
								" ELSE %d END ", j);
				temp_len = SAFE_SNPRINTF(query, query_size, len, temp_str);
				if (0 <= temp_len) len+= temp_len;
			}
			else {
				temp_len = SAFE_SNPRINTF(query, query_size, len, " ORDER BY ");
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(query, query_size, len, sortkey);
				if (0 <= temp_len) len+= temp_len;
			}
		}
		else {
			temp_keyword = __ctsvc_db_get_str_with_escape((char*)keyword, strlen((char*)keyword), true);
			char temp_str[CTS_SQL_MIN_LEN + (strlen(field) + strlen(temp_keyword)) * SORT_CHECK_LEN];
			snprintf(temp_str, sizeof(temp_str),
							" ORDER BY "
							"	CASE "
							"		WHEN %s LIKE '%s%%' THEN 1 "
							"		WHEN %s LIKE '_%s%%' THEN 2 "
							"		WHEN %s LIKE '__%s%%' THEN 3 "
							"		WHEN %s LIKE '___%s%%' THEN 4 "
							"		WHEN %s LIKE '____%s%%' THEN 5 "
							"		WHEN %s LIKE '_____%s%%' THEN 6 "
							"		WHEN %s LIKE '______%s%%' THEN 7 "
							"		ELSE 8 "
							"	END ",
							field, temp_keyword, field, temp_keyword,
							field, temp_keyword, field, temp_keyword,
							field, temp_keyword, field, temp_keyword,
								field, temp_keyword);
			temp_len = SAFE_SNPRINTF(query, query_size, len, temp_str);
			if (0 <= temp_len) len+= temp_len;
				free(temp_keyword);
		}
		temp_len = SAFE_SNPRINTF(query, query_size, len, ", ");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(query, query_size, len, sortkey);
		if (0 <= temp_len) len+= temp_len;
	}
	else {
		temp_len = SAFE_SNPRINTF(query, query_size, len, " ORDER BY ");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(query, query_size, len, sortkey);
		if (0 <= temp_len) len+= temp_len;
	}
	return len;
}

static int __ctsvc_db_search_records_exec(const char *view_uri, const property_info_s* properties,
		int ids_count, const char *projection, const char *keyword, int offset, int limit, int range, contacts_list_h* out_list)
{
	char *query = NULL;
	char temp_query[CTS_SQL_MAX_LEN];
	int query_size;
	int temp_len;
	const char *table;
	int len = 0;
	int ret;
	int i;
	int type;
	cts_stmt stmt = NULL;
	contacts_list_h list = NULL;
	ctsvc_record_type_e r_type;
	const char *sortkey = NULL;

	ret = ctsvc_db_get_table_name(view_uri, &table);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "Invalid parameter : view uri (%s)", view_uri);

	query_size = CTS_SQL_MAX_LEN;
	query = calloc(1, query_size);
	len = 0;

	if (STRING_EQUAL == strcmp(keyword, "+")) {
		range &= ~CONTACTS_SEARCH_RANGE_NUMBER;
	}

	if (STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_CONTACT)
			|| STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP)
			|| STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_ASSIGNED)
			|| STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_NOT_ASSIGNED)) {
		if (range & CONTACTS_SEARCH_RANGE_EMAIL) {
			free(query);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}

		temp_len = SAFE_SNPRINTF(&query, &query_size, len, "SELECT ");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, projection);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, " FROM ");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, table);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, " WHERE contact_id IN ");
		if (0 <= temp_len) len+= temp_len;

		temp_len = __ctsvc_db_append_search_query(keyword, &query, &query_size, len, range);
		if (0 <= temp_len) len = temp_len;
	}
	else if (STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_NUMBER)) {
		bool need_union = false;
		if (range & CONTACTS_SEARCH_RANGE_DATA || range & CONTACTS_SEARCH_RANGE_EMAIL) {
			free(query);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, "SELECT ");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, projection);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, " FROM (");
		if (0 <= temp_len) len+= temp_len;

		if (range & CONTACTS_SEARCH_RANGE_NUMBER) {
			char clean_num[strlen(keyword)+1+5]; // for cc
			// Original number can include '-' or special character. So search by cleaned_number
			// If contact has 010 1234 5678 (normalized number is +cc 10 1234 5678),
			//	then the contack should be searched by keyword +cc 10 1234 5678
			ret = ctsvc_clean_number(keyword, clean_num, sizeof(clean_num), false);
			if (0 < ret) {
				const char *cc = ctsvc_get_network_cc(false);

				temp_len = SAFE_SNPRINTF(&query, &query_size, len, " SELECT * FROM ");
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, table);
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, " WHERE normalized_number LIKE '%%");
				if (0 <= temp_len) len+= temp_len;

				if (cc && cc[0] == '7' && clean_num[0] == '8') {		// Russia
					char normal_num[strlen(clean_num)+1+5];	// for cc
					ret = ctsvc_normalize_number(clean_num, normal_num, sizeof(normal_num), false);
					if (0 < ret) {
						temp_len = SAFE_SNPRINTF(&query, &query_size, len, normal_num);
					}
					else
						temp_len = SAFE_SNPRINTF(&query, &query_size, len, clean_num);
				}
				else
					temp_len = SAFE_SNPRINTF(&query, &query_size, len, clean_num);
				if (0 <= temp_len) len+= temp_len;

				temp_len = SAFE_SNPRINTF(&query, &query_size, len, "%%' OR cleaned_number LIKE '%%");
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, clean_num);
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, "%%' ");
				if (0 <= temp_len) len+= temp_len;
			}
			else {
				char *temp_keyword = __ctsvc_db_get_str_with_escape((char*)keyword, strlen((char*)keyword), true);
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, " SELECT * FROM ");
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, table);
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, " WHERE number LIKE '%%");
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, temp_keyword);
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, "%%' ESCAPE '\\'");
				if (0 <= temp_len) len+= temp_len;
				free(temp_keyword);
			}
			need_union = true;
		}

		if (range & CONTACTS_SEARCH_RANGE_NAME) {
			if (need_union) {
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, " UNION ");
				if (0 <= temp_len) len+= temp_len;
			}
			snprintf(temp_query, sizeof(temp_query),
						"SELECT "CTSVC_DB_VIEW_PERSON_CONTACT".*, "
									"temp_data.* "
							"FROM "CTSVC_DB_VIEW_PERSON_CONTACT" "
							"JOIN (SELECT id number_id, "
									"contact_id, "
									"data1 type, "
									"is_primary_default, "
									"data2 label, "
									"data3 number, "
									"data4 minmatch, "
									"data5 normalized_number, "
									"data6 cleaned_number "
						"FROM "CTS_TABLE_DATA" WHERE datatype = %d AND is_my_profile = 0) temp_data "
						"ON temp_data.contact_id = "CTSVC_DB_VIEW_PERSON_CONTACT".contact_id ",
							CTSVC_DATA_NUMBER);
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, temp_query);
			if (0 <= temp_len) len+= temp_len;

			// search contact from search_index table by name and join the results
			// FTS can support to serach with multiple words
			// If contact display_name is 'abc def', then the contact should be searched by 'def'
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, " JOIN ");
			if (0 <= temp_len) len+= temp_len;
			temp_len = __ctsvc_db_append_search_query(keyword, &query, &query_size, len, CONTACTS_SEARCH_RANGE_NAME);
			if (0 <= temp_len) len = temp_len;
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, " temp_search_index ON ");
			if (0 <= temp_len) len+= temp_len;
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, CTSVC_DB_VIEW_PERSON_CONTACT);
			if (0 <= temp_len) len+= temp_len;
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, ".name_contact_id = temp_search_index.contact_id ");
			if (0 <= temp_len) len+= temp_len;
		}

		temp_len = SAFE_SNPRINTF(&query, &query_size, len, ") ");
		if (0 <= temp_len) len+= temp_len;
	}
	else if (STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_EMAIL)) {
		bool need_union = false;
		if (range & CONTACTS_SEARCH_RANGE_NUMBER || range & CONTACTS_SEARCH_RANGE_DATA) {
			free(query);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}

		temp_len = SAFE_SNPRINTF(&query, &query_size, len, "SELECT ");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, projection);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, " FROM (");
		if (0 <= temp_len) len+= temp_len;
		if (range & CONTACTS_SEARCH_RANGE_EMAIL) {
			// search contact which has email address started with keyword
			char *temp_keyword = __ctsvc_db_get_str_with_escape((char*)keyword, strlen((char*)keyword), true);
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, "SELECT * FROM ");
			if (0 <= temp_len) len+= temp_len;
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, table);
			if (0 <= temp_len) len+= temp_len;
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, " WHERE email LIKE '");
			if (0 <= temp_len) len+= temp_len;
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, temp_keyword);
			if (0 <= temp_len) len+= temp_len;
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, "%%' ESCAPE '\\'");
			if (0 <= temp_len) len+= temp_len;
			free(temp_keyword);
			need_union = true;
		}

		if (range & CONTACTS_SEARCH_RANGE_NAME) {
			if (need_union) {
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, " UNION ");
				if (0 <= temp_len) len+= temp_len;
			}

			snprintf(temp_query, sizeof(temp_query),
						"SELECT "CTSVC_DB_VIEW_PERSON_CONTACT".*, "
									"temp_data.* "
							"FROM "CTSVC_DB_VIEW_PERSON_CONTACT" "
						"JOIN (SELECT id email_id, "
									"contact_id, "
									"data1 type, "
									"is_primary_default, "
									"data2 label, "
									"data3 email "
					"FROM "CTS_TABLE_DATA" WHERE datatype = %d AND is_my_profile = 0) temp_data "
			"ON temp_data.contact_id = "CTSVC_DB_VIEW_PERSON_CONTACT".contact_id",
							CTSVC_DATA_EMAIL);
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, temp_query);
			if (0 <= temp_len) len+= temp_len;

			temp_len = SAFE_SNPRINTF(&query, &query_size, len, " JOIN ");
			if (0 <= temp_len) len+= temp_len;
			temp_len = __ctsvc_db_append_search_query(keyword, &query, &query_size, len, CONTACTS_SEARCH_RANGE_NAME);
			if (0 <= temp_len) len = temp_len;
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, " temp_search_index ON ");
			if (0 <= temp_len) len+= temp_len;
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, CTSVC_DB_VIEW_PERSON_CONTACT);
			if (0 <= temp_len) len+= temp_len;
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, ".name_contact_id = temp_search_index.contact_id ");
			if (0 <= temp_len) len+= temp_len;
		}

		temp_len = SAFE_SNPRINTF(&query, &query_size, len, ") ");
		if (0 <= temp_len) len+= temp_len;
	}
	else {		// CTSVC_VIEW_URI_PERSON
		if (range & CONTACTS_SEARCH_RANGE_EMAIL) {
			free(query);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}

		snprintf(temp_query, sizeof(temp_query), "SELECT %s FROM %s, "
					"(SELECT person_id person_id_in_contact, addressbook_id "
							"FROM "CTS_TABLE_CONTACTS " "
							"WHERE deleted = 0 AND contact_id IN ",
							projection, table);
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, temp_query);
		if (0 <= temp_len) len+= temp_len;
		temp_len = __ctsvc_db_append_search_query(keyword, &query, &query_size, len, range);
		if (0 <= temp_len) len = temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, "GROUP BY person_id_in_contact) temp_contacts ON ");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, table);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, ".person_id = temp_contacts.person_id_in_contact");
		if (0 <= temp_len) len+= temp_len;
	}

	if (__ctsvc_db_view_has_display_name(view_uri, properties, ids_count))
		sortkey = ctsvc_get_sort_column();

	if (sortkey) {
		len = __ctsvc_db_search_records_append_sort(view_uri, sortkey, keyword, len, &query, &query_size);
	}
	else if (STRING_EQUAL == strcmp(view_uri, CTSVC_VIEW_URI_GROUP)) {
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, " ORDER BY group_prio");
		if (0 <= temp_len) len+= temp_len;
	}

	if (0 != limit) {
		snprintf(temp_query, sizeof(temp_query), " LIMIT %d", limit);
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, temp_query);
		if (0 <= temp_len) len+= temp_len;

		if (0 < offset) {
			snprintf(temp_query, sizeof(temp_query), " OFFSET %d", offset);
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, temp_query);
			if (0 <= temp_len) len+= temp_len;
		}
	}

	ret = ctsvc_query_prepare(query, &stmt);
	free(query);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	r_type = ctsvc_view_get_record_type(view_uri);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		if (r_type == CTSVC_RECORD_PERSON) {
			unsigned int *project = malloc(sizeof(unsigned int)*ids_count);
			for (i=0;i<ids_count;i++) {
				project[i] = properties[i].property_id;
			}

			int ret = ctsvc_db_person_create_record_from_stmt_with_projection(stmt, project, ids_count, &record);

			free(project);

			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("DB error : make record Fail(%d)", ret);
			}
		}
		else {
			contacts_record_create(view_uri, &record);

			for (i=0;i<ids_count;i++) {
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

	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);
	*out_list = list;
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
	int range = CONTACTS_SEARCH_RANGE_NAME | CONTACTS_SEARCH_RANGE_NUMBER | CONTACTS_SEARCH_RANGE_DATA;

	RETVM_IF(NULL == keyword, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : keyword is NULL");

	can_keyword_search = __ctsvc_db_view_can_keyword_search(view_uri);
	RETVM_IF(false == can_keyword_search, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : can not keyword search");

	p = ctsvc_view_get_all_property_infos(view_uri, &count);
	ret = __ctsvc_db_create_projection(view_uri, p, count, NULL, 0, &projection);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_create_projection Fail(%d)", ret);

	ret = __ctsvc_db_search_records_exec(view_uri, p, count, projection, keyword, offset, limit, range, out_list);
	free(projection);

	return ret;
}

static int __ctsvc_db_search_records_with_range(const char* view_uri, const char *keyword,
		int offset, int limit, int range, contacts_list_h* out_list)
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
	ret = __ctsvc_db_create_projection(view_uri, p, count, NULL, 0, &projection);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_create_projection Fail(%d)", ret);

	ret = __ctsvc_db_search_records_exec(view_uri, p, count, projection, keyword, offset, limit, range, out_list);
	free(projection);

	return ret;
}

static inline int __ctsvc_db_search_records_with_query_exec(ctsvc_query_s *s_query, const char *projection,
	const char *condition, GSList *bind, const char *keyword, int offset, int limit, contacts_list_h * out_list)
{
	char *query = NULL;
	int query_size;
	int temp_len;
	int len;
	int ret;
	int i;
	int type;
	GSList *cursor;
	cts_stmt stmt = NULL;
	contacts_list_h list = NULL;
	const char *table;
	const char *sortkey = NULL;
	int range = CONTACTS_SEARCH_RANGE_NAME | CONTACTS_SEARCH_RANGE_NUMBER | CONTACTS_SEARCH_RANGE_DATA;

	RETV_IF(NULL == projection || '\0' == *projection, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = ctsvc_db_get_table_name(s_query->view_uri, &table);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "Invalid parameter : view uri (%s)", s_query->view_uri);

	query_size = CTS_SQL_MAX_LEN;
	query = calloc(1, query_size);
	len = 0;

	if (s_query->distinct)
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, "SELECT DISTINCT ");
	else
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, "SELECT ");
	if (0 <= temp_len) len+= temp_len;
	temp_len = SAFE_SNPRINTF(&query, &query_size, len, projection);
	if (0 <= temp_len) len+= temp_len;
	temp_len = SAFE_SNPRINTF(&query, &query_size, len, " FROM ");
	if (0 <= temp_len) len+= temp_len;

	if (STRING_EQUAL == strcmp(s_query->view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_CONTACT)
			|| STRING_EQUAL == strcmp(s_query->view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP)
			|| STRING_EQUAL == strcmp(s_query->view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_ASSIGNED)
			|| STRING_EQUAL == strcmp(s_query->view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_NOT_ASSIGNED)) {
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, table);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, " WHERE ");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, table);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, ".contact_id IN ");
		if (0 <= temp_len) len+= temp_len;
	}
	else if (STRING_EQUAL == strcmp(s_query->view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_NUMBER)
			|| STRING_EQUAL == strcmp(s_query->view_uri, CTSVC_VIEW_URI_READ_ONLY_PERSON_EMAIL)) {
		free(query);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	else {		// CTSVC_VIEW_URI_PERSON
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, table);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, ", "
						"(SELECT contact_id, person_id person_id_in_contact, addressbook_id FROM ");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, CTS_TABLE_CONTACTS);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, " WHERE deleted = 0) temp_contacts ON ");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, table);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, ".person_id = temp_contacts.person_id_in_contact WHERE temp_contacts.contact_id IN ");
		if (0 <= temp_len) len+= temp_len;
	}

	temp_len = __ctsvc_db_append_search_query(keyword, &query, &query_size, len, range);
	if (0 <= temp_len) len = temp_len;

	if (condition && *condition) {
		len += snprintf(query+len, sizeof(query)-len, " AND (%s)", condition);
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, " AND (");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, condition);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, ")");
		if (0 <= temp_len) len+= temp_len;
	}

	if (__ctsvc_db_view_has_display_name(s_query->view_uri, s_query->properties, s_query->property_count))
		sortkey = ctsvc_get_sort_column();

	if (s_query->sort_property_id) {
		const char *field_name;

		switch(s_query->sort_property_id) {
		case CTSVC_PROPERTY_PERSON_DISPLAY_NAME:
		case CTSVC_PROPERTY_CONTACT_DISPLAY_NAME:
			if (sortkey) {
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, " ORDER BY ");
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, sortkey);
				if (0 <= temp_len) len+= temp_len;
				if (false == s_query->sort_asc) {
					temp_len = SAFE_SNPRINTF(&query, &query_size, len, " DESC ");
					if (0 <= temp_len) len+= temp_len;
				}
			}
			break;
		default :
			field_name = __ctsvc_db_get_property_field_name(s_query->properties,
								s_query->property_count, QUERY_SORTKEY, s_query->sort_property_id);
			if (field_name) {
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, " ORDER BY ");
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, field_name);
				if (0 <= temp_len) len+= temp_len;
//				if (sortkey)
//					len += snprintf(query+len, sizeof(query)-len, ", %s", sortkey);
				if (false == s_query->sort_asc) {
					temp_len = SAFE_SNPRINTF(&query, &query_size, len, " DESC ");
					if (0 <= temp_len) len+= temp_len;
				}
			}
			else if (sortkey) {
				len += snprintf(query+len, sizeof(query)-len, " ORDER BY %s", sortkey);
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, " ORDER BY ");
				if (0 <= temp_len) len+= temp_len;
				temp_len = SAFE_SNPRINTF(&query, &query_size, len, sortkey);
				if (0 <= temp_len) len+= temp_len;
			}
			break;
		}
	}
	else if (sortkey) {
		len = __ctsvc_db_search_records_append_sort(s_query->view_uri, sortkey, keyword, len, &query, &query_size);
	}
	else if (STRING_EQUAL == strcmp(s_query->view_uri, CTSVC_VIEW_URI_GROUP)) {
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, " ORDER BY group_prio");
		if (0 <= temp_len) len+= temp_len;
	}

	if (0 != limit) {
		char temp_str[20] = {0};
		snprintf(temp_str, sizeof(temp_str), " LIMIT %d", limit);
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, temp_str);
		if (0 <= temp_len) len+= temp_len;

		if (0 < offset) {
			snprintf(temp_str, sizeof(temp_str), " OFFSET %d", offset);
			temp_len = SAFE_SNPRINTF(&query, &query_size, len, temp_str);
			if (0 <= temp_len) len+= temp_len;
		}
	}

	ret = ctsvc_query_prepare(query, &stmt);
	free(query);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	i = 1;
	len = g_slist_length(bind);
	for (cursor=bind; cursor;cursor=cursor->next, i++)
		ctsvc_stmt_bind_text(stmt, i, cursor->data);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		if (ctsvc_view_get_record_type(s_query->view_uri) == CTSVC_RECORD_PERSON) {
			unsigned int ids_count;
			unsigned int *project;
			if (0 == s_query->projection_count)
				ids_count = s_query->property_count;
			else
				ids_count = s_query->projection_count;

			project = malloc(sizeof(unsigned int)*ids_count);

			for (i=0;i<ids_count;i++) {
				if (0 == s_query->projection_count)
					project[i] = s_query->properties[i].property_id;
				else
					project[i] = s_query->projection[i];
			}

			ret = ctsvc_db_person_create_record_from_stmt_with_projection(stmt, project, ids_count, &record);
			free(project);

			if (CONTACTS_ERROR_NONE != ret)
				CTS_ERR("DB error : make record Fail(%d)", ret);
		}
		else {
			contacts_record_create(s_query->view_uri, (contacts_record_h*)&record);
			int field_count;
			if (0 == s_query->projection_count)
				field_count = s_query->property_count;
			else {
				field_count = s_query->projection_count;

				if (CONTACTS_ERROR_NONE != ctsvc_record_set_projection_flags(record, s_query->projection, s_query->projection_count, s_query->property_count)) {
					ASSERT_NOT_REACHED("To set projection is Fail.\n");
				}
			}

			for (i=0;i<field_count;i++) {
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
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = list;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_search_records_with_query(contacts_query_h query, const char *keyword,
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

	ret = __ctsvc_db_create_projection(s_query->view_uri, s_query->properties, s_query->property_count,
								s_query->projection, s_query->projection_count, &projection);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_create_projection Fail(%d)", ret);

	if (s_query->filter) {
		ret = __ctsvc_db_create_composite_condition(s_query->filter, &condition, &bind_text);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("__ctsvc_db_create_composite_condition Fail(%d)", ret);
			free(projection);
			return ret;
		}
	}

	ret = __ctsvc_db_search_records_with_query_exec(s_query, projection, condition, bind_text, keyword, offset, limit, out_list);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_search_records_with_query_exec Fail(%d)", ret);
		for (cursor=bind_text;cursor;cursor=cursor->next)
			free(cursor->data);
		g_slist_free(bind_text);

		free(condition);
		free(projection);
		return ret;
	}

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
	int count;
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
	int count;
	contacts_record_h record = NULL;

	ret = contacts_list_get_count(list, &count);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("contacts_list_get_count() Fail(%d)", ret);
		return ret;
	}

	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_begin_trans Fail(%d)", ret);

	id = calloc(count, sizeof(int));

	contacts_list_first(list);
	index = 0;
	do {
		ret = contacts_list_get_current_record_p(list, &record);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("contacts_list_get_current_record_p is faild(%d)", ret);
			ctsvc_end_trans(false);
			free(id);
			return ret;
		}

		ret = contacts_db_insert_record(record, &id[index++]);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("contacts_db_insert_record is faild(%d)", ret);
			ctsvc_end_trans(false);
			free(id);
			return ret;
		}
	} while (CONTACTS_ERROR_NONE  == contacts_list_next(list));

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_end_trans() Fail(%d)", ret);
		free(id);
		return ret;
	}

	if (ids)
		*ids = id;
	else
		free(id);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_delete_records(const char* view_uri, int ids[], int count)
{
	int ret = CONTACTS_ERROR_NONE;
	int index;

	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_begin_trans Fail(%d)", ret);

	index = 0;
	do {
		ret = contacts_db_delete_record(view_uri, ids[index++]);
		if (CONTACTS_ERROR_NO_DATA == ret) {
			CTS_DBG("the record is not exist : %d", ret);
			continue;
		}
		else if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("contacts_db_delete_record is faild(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
	} while (index < count);

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_update_records(contacts_list_h list)
{
	int ret = CONTACTS_ERROR_NONE;
	int count;
	contacts_record_h record;

	ret = contacts_list_get_count(list, &count);
	RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "contacts_list_get_count is falied(%d)", ret);
	RETVM_IF(count <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : count is 0");

	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_begin_trans Fail(%d)", ret);

	contacts_list_first(list);
	do {
		ret = contacts_list_get_current_record_p(list, &record);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("contacts_list_get_current_record_p is faild(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}

		ret = contacts_db_update_record(record);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("contacts_db_update_record is faild(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
	} while (CONTACTS_ERROR_NONE == contacts_list_next(list));
	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		CTS_ERR("DB error : ctsvc_end_trans() Fail(%d)", ret);
		return ret;
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_count_exec(const char *view_uri, const property_info_s* properties, int ids_count,
		const char *projection, int *out_count)
{
	char query[CTS_SQL_MAX_LEN] = {0};
	const char *table;
	int len;
	int ret;

	ret = ctsvc_db_get_table_name(view_uri, &table);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "Invalid parameter : view uri (%s)", view_uri);

	len = snprintf(query, sizeof(query), "SELECT COUNT(*) FROM (SELECT %s FROM ", projection);

	len += snprintf(query+len, sizeof(query)-len, "  (%s)", table);

	len += snprintf(query+len, sizeof(query)-len, ") ");

	ret = ctsvc_query_get_first_int_result(query, out_count);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "DB error : ctsvc_query_get_first_int_result() Fail(%d)", ret);

	return ret;
}

static int __ctsvc_db_get_count(const char* view_uri, int *out_count)
{
	int ret;
	unsigned int count;
	char *projection;

	const property_info_s *p = ctsvc_view_get_all_property_infos(view_uri, &count);
	ret = __ctsvc_db_create_projection(view_uri, p, count, NULL, 0, &projection);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_create_projection Fail(%d)", ret);

	__ctsvc_db_get_count_exec(view_uri, p, count, projection, out_count);
	free(projection);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_count_with_query_exec(ctsvc_query_s *s_query, const char *projection,
	const char *condition, GSList *bind_text, int *out_count)
{
	char *query = NULL;
	int query_size;
	int temp_len;
	int len;
	int ret;
	const char *table;

	RETV_IF(NULL == projection || '\0' == *projection, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = ctsvc_db_get_table_name(s_query->view_uri, &table);
	RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "Invalid parameter : view uri (%s)", s_query->view_uri);

	query_size = CTS_SQL_MAX_LEN;
	query = calloc(1, query_size);
	len = 0;
	if (s_query->distinct) {
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, "SELECT COUNT(*) FROM (SELECT DISTINCT ");
	}
	else {
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, "SELECT COUNT(*) FROM (SELECT ");
	}
	if (0 <= temp_len) len+= temp_len;
	temp_len = SAFE_SNPRINTF(&query, &query_size, len, projection);
	if (0 <= temp_len) len+= temp_len;
	temp_len = SAFE_SNPRINTF(&query, &query_size, len, " FROM ");
	if (0 <= temp_len) len+= temp_len;

	temp_len = SAFE_SNPRINTF(&query, &query_size, len, " (");
	if (0 <= temp_len) len+= temp_len;
	temp_len = SAFE_SNPRINTF(&query, &query_size, len,  table);
	if (0 <= temp_len) len+= temp_len;

	temp_len = SAFE_SNPRINTF(&query, &query_size, len, ") ");
	if (0 <= temp_len) len+= temp_len;

	if (condition && *condition) {
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, " WHERE (");
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, condition);
		if (0 <= temp_len) len+= temp_len;
		temp_len = SAFE_SNPRINTF(&query, &query_size, len, ") ");
		if (0 <= temp_len) len+= temp_len;
	}

	temp_len = SAFE_SNPRINTF(&query, &query_size, len, ") ");
	if (0 <= temp_len) len+= temp_len;

	if (bind_text) {
		cts_stmt stmt;
		GSList *cursor;
		int i;
		ret = ctsvc_query_prepare(query, &stmt);
		free(query);
		RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

		for (cursor=bind_text, i=1; cursor;cursor=cursor->next, i++)
			ctsvc_stmt_bind_copy_text(stmt, i, cursor->data, strlen(cursor->data));
		ret = ctsvc_stmt_get_first_int_result(stmt, out_count);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "DB error : ctsvc_stmt_get_first_int_result() Fail(%d)", ret);
	}
	else {
		ret = ctsvc_query_get_first_int_result(query, out_count);
		free(query);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "DB error : ctsvc_query_get_first_int_result() Fail(%d)", ret);
	}
	return ret;
}

static int __ctsvc_db_get_count_with_query(contacts_query_h query, int *out_count)
{
	int ret;
	char *condition = NULL;
	char *projection = NULL;
	ctsvc_query_s *s_query;
	GSList *bind_text = NULL;
	GSList *cursor;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s *)query;

	if (s_query->filter) {
		ret = __ctsvc_db_create_composite_condition(s_query->filter, &condition, &bind_text);
		RETVM_IF (CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_db_create_composite_condition Fail(%d)", ret);
	}

	ret = __ctsvc_db_create_projection(s_query->view_uri, s_query->properties, s_query->property_count,
								s_query->projection, s_query->projection_count, &projection);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("__ctsvc_db_create_projection Fail(%d)", ret);
		for (cursor=bind_text;cursor;cursor=cursor->next)
			free(cursor->data);
		g_slist_free(bind_text);
		free(condition);
		return ret;
	}

	ret = __ctsvc_db_get_count_with_query_exec(s_query, projection, condition, bind_text, out_count);
	for (cursor=bind_text;cursor;cursor=cursor->next)
		free(cursor->data);
	g_slist_free(bind_text);

	free(condition);
	free(projection);

	return ret;
}

API int contacts_db_get_records_with_query(contacts_query_h query, int offset, int limit,
	contacts_list_h* out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_db_plugin_info_s* plugin_info = NULL;
	ctsvc_query_s *s_query;

	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s*)query;

	if ((plugin_info = ctsvc_db_get_plugin_info(ctsvc_view_get_record_type(s_query->view_uri)))) {
		if (plugin_info->get_records_with_query) {
			ret = plugin_info->get_records_with_query(query, offset, limit, out_list);
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
			CONTACTS_CHANGE_DELETED, version,version);
	}

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		ctsvc_updated_info_s *update_info;

		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
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
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	snprintf(query, sizeof(query), "SELECT ver FROM "CTS_TABLE_VERSION);
	ret = ctsvc_query_get_first_int_result(query, out_current_version);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "DB error : ctsvc_query_get_first_int_result() Fail(%d)", ret);
	*out_list = list;

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

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		ctsvc_updated_info_s *update_info;

		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
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
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	snprintf(query, sizeof(query), "SELECT ver FROM "CTS_TABLE_VERSION);
	ret = ctsvc_query_get_first_int_result(query, out_current_version);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "DB error : ctsvc_query_get_first_int_result() Fail(%d)", ret);
	*out_list = list;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_get_group_relations_changes(const char* view_uri, int addressbook_id,
		int version, contacts_list_h* out_list, int* out_current_version)
{
	int len;
	int ret;
	char query[CTS_SQL_MAX_LEN] = {0};
	char temp_query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;
	cts_stmt stmt;

	len = snprintf(temp_query, sizeof(temp_query),
			"SELECT %d, group_id, contact_id, addressbook_id, ver "
				"FROM "CTS_TABLE_GROUP_RELATIONS", "CTS_TABLE_GROUPS" USING (group_id) "
				"WHERE ver > %d AND deleted = 0 "
			"UNION SELECT %d, group_id, contact_id, addressbook_id, ver "
				"FROM "CTS_TABLE_GROUP_RELATIONS", "CTS_TABLE_GROUPS" USING (group_id) "
				"WHERE ver > %d AND deleted = 1 ",
				CONTACTS_CHANGE_INSERTED, version,
				CONTACTS_CHANGE_DELETED, version);

	if (0 <= addressbook_id) {
		len += snprintf(query, sizeof(query),
					"SELECT * FROM (%s) WHERE addressbook_id = %d ", temp_query, addressbook_id);
	}

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
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
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	snprintf(query, sizeof(query), "SELECT ver FROM "CTS_TABLE_VERSION);
	ret = ctsvc_query_get_first_int_result(query, out_current_version);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "DB error : ctsvc_query_get_first_int_result() Fail(%d)", ret);
	*out_list = list;

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

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		ret = contacts_record_create(view_uri, &record);
		ctsvc_record_set_int(record, _contacts_group_member_updated_info.group_id, ctsvc_stmt_get_int(stmt, 0));
		ctsvc_record_set_int(record, _contacts_group_member_updated_info.version, ctsvc_stmt_get_int(stmt, 1));
		ctsvc_record_set_int(record, _contacts_group_member_updated_info.address_book_id, ctsvc_stmt_get_int(stmt, 2));

		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	snprintf(query, sizeof(query), "SELECT ver FROM "CTS_TABLE_VERSION);
	ret = ctsvc_query_get_first_int_result(query, out_current_version);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "DB error : ctsvc_query_get_first_int_result() Fail(%d)", ret);
	*out_list = list;

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

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
		}

		ret = contacts_record_create(view_uri, &record);
		ctsvc_record_set_int(record, _contacts_my_profile_updated_info.version, ctsvc_stmt_get_int(stmt, 0));
		ctsvc_record_set_int(record, _contacts_my_profile_updated_info.address_book_id, ctsvc_stmt_get_int(stmt, 1));
		ctsvc_record_set_int(record, _contacts_my_profile_updated_info.last_changed_type, ctsvc_stmt_get_int(stmt, 2));
		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	snprintf(query, sizeof(query), "SELECT ver FROM "CTS_TABLE_VERSION);
	ret = ctsvc_query_get_first_int_result(query, out_current_version);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "DB error : ctsvc_query_get_first_int_result() Fail(%d)", ret);
	*out_list = list;

	return CONTACTS_ERROR_NONE;
}

API int contacts_db_get_changes_by_version(const char* view_uri, int addressbook_id,
		int version, contacts_list_h* out_list, int* out_current_version)
{
	int ret;
	RETV_IF(version < 0, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;
	RETV_IF(NULL == out_current_version, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_current_version = 0;

	if (STRING_EQUAL == strcmp(view_uri, _contacts_contact_updated_info._uri)) {
		ret = __ctsvc_db_get_contact_changes(view_uri, addressbook_id,
					version, out_list, out_current_version);
		return ret;
	}
	else if (STRING_EQUAL == strcmp(view_uri, _contacts_group_updated_info._uri)) {
		ret = __ctsvc_db_get_group_changes(view_uri, addressbook_id,
					version, out_list, out_current_version);
		return ret;
	}
	else if (STRING_EQUAL == strcmp(view_uri, _contacts_group_member_updated_info._uri)) {
		ret = __ctsvc_db_get_group_member_changes(view_uri, addressbook_id,
					version, out_list, out_current_version);
		return ret;
	}
	else if (STRING_EQUAL == strcmp(view_uri, _contacts_grouprel_updated_info._uri)) {
		ret = __ctsvc_db_get_group_relations_changes(view_uri, addressbook_id,
					version, out_list, out_current_version);
		return ret;
	}
	else if (STRING_EQUAL == strcmp(view_uri, _contacts_my_profile_updated_info._uri)) {
		ret = __ctsvc_db_get_my_profile_changes(view_uri, addressbook_id,
					version, out_list, out_current_version);
		return ret;
	}

	CTS_ERR("Invalid parameter : this API does not support uri(%s)", view_uri);
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

API int contacts_db_get_current_version(int* out_current_version)
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

API int contacts_db_search_records_with_range(const char* view_uri, const char *keyword,
		int offset, int limit, int range, contacts_list_h* out_list)
{
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;
	RETVM_IF(range == 0, CONTACTS_ERROR_INVALID_PARAMETER, "range is 0");
	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	return __ctsvc_db_search_records_with_range(view_uri, keyword, offset, limit, range, out_list);
}

API int contacts_db_search_records_with_query(contacts_query_h query, const char *keyword,
		int offset, int limit, contacts_list_h* out_list)
{
	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;
	RETVM_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	return __ctsvc_db_search_records_with_query(query, keyword, offset, limit, out_list);
}

API int contacts_db_get_count(const char* view_uri, int *out_count)
{
	int ret;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETV_IF(NULL == out_count, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_count = 0;
	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	if ((plugin_info = ctsvc_db_get_plugin_info(ctsvc_view_get_record_type(view_uri)))) {
		if (plugin_info->get_count) {
			ret = plugin_info->get_count(out_count);
			return ret;
		}
	}

	return __ctsvc_db_get_count(view_uri, out_count);
}

API int contacts_db_get_count_with_query(contacts_query_h query, int *out_count)
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

	if (plugin_info) {
		if (plugin_info->get_count_with_query) {
			ret = plugin_info->get_count_with_query(query, out_count);
			return ret;
		}
	}

	return __ctsvc_db_get_count_with_query(query, out_count);
}

API int contacts_db_insert_record(contacts_record_h record, int *id)
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

API int contacts_db_get_record(const char* view_uri, int id, contacts_record_h* out_record)
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

API int contacts_db_replace_record(contacts_record_h record, int id)
{
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETVM_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : record is NULL");

	plugin_info = ctsvc_db_get_plugin_info(((ctsvc_record_s*)record)->r_type);
	RETVM_IF(NULL == plugin_info, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(NULL == plugin_info->replace_record, CONTACTS_ERROR_INVALID_PARAMETER, "Not permitted");

	return plugin_info->replace_record(record, id);
}

API int contacts_db_get_all_records(const char* view_uri, int offset, int limit, contacts_list_h* out_list)
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_record_type_e type = CTSVC_RECORD_INVALID;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;
	RETVM_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	type = ctsvc_view_get_record_type(view_uri);
	plugin_info = ctsvc_db_get_plugin_info(type);

	if (plugin_info) {
		if (plugin_info->get_all_records) {
			ret = plugin_info->get_all_records(offset, limit, out_list);
			return ret;
		}
	}

	return __ctsvc_db_get_all_records(view_uri, offset, limit, out_list);
}

int ctsvc_db_insert_records(contacts_list_h list, int **ids, int *count)
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETVM_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	if (count)
		contacts_list_get_count(list, count);
	if ((plugin_info = ctsvc_db_get_plugin_info(((ctsvc_list_s*)list)->l_type))) {
		if (plugin_info->insert_records) {
			ret = plugin_info->insert_records(list, ids);
			return ret;
		}
	}

	return __ctsvc_db_insert_records(list, ids);
}

int ctsvc_db_update_records(contacts_list_h list)
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETVM_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	if ((plugin_info = ctsvc_db_get_plugin_info(((ctsvc_list_s*)list)->l_type))) {
		if (plugin_info->update_records) {
			ret = plugin_info->update_records(list);
			return ret;
		}
	}

	return __ctsvc_db_update_records(list);
}

int ctsvc_db_delete_records(const char* view_uri, int* ids, int count)
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETV_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER);

	if ((plugin_info = ctsvc_db_get_plugin_info(ctsvc_view_get_record_type(view_uri)))) {
		if (plugin_info->delete_records) {
			ret = plugin_info->delete_records(ids, count);
			return ret;
		}
	}

	return __ctsvc_db_delete_records(view_uri, ids, count);
}

static int __ctsvc_db_replace_records(contacts_list_h list, int ids[], int count)
{
	int ret = CONTACTS_ERROR_NONE;
	int record_count;
	contacts_record_h record;
	int i;

	ret = contacts_list_get_count(list, &record_count);
	RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "contacts_list_get_count is falied(%d)", ret);
	RETVM_IF(record_count <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : count is 0");
	RETVM_IF(record_count != count, CONTACTS_ERROR_INVALID_PARAMETER,
			"Invalid parameter : list count and ids count are not matched");

	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_begin_trans Fail(%d)", ret);

	contacts_list_first(list);
	i = 0;
	do {
		ret = contacts_list_get_current_record_p(list, &record);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("contacts_list_get_current_record_p is faild(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}

		ret = contacts_db_replace_record(record, ids[i++]);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("contacts_db_replace_record is faild(%d)", ret);
			ctsvc_end_trans(false);
			return ret;
		}
	} while (CONTACTS_ERROR_NONE == contacts_list_next(list));

	ret = ctsvc_end_trans(true);

	return ret;
}

int ctsvc_db_replace_records(contacts_list_h list, int ids[], unsigned int count)
{
	int ret = CONTACTS_ERROR_NONE;
	ctsvc_db_plugin_info_s* plugin_info = NULL;

	RETVM_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(NULL == ids, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter");

	if ((plugin_info = ctsvc_db_get_plugin_info(((ctsvc_list_s*)list)->l_type))) {
		if (plugin_info->replace_records) {
			ret = plugin_info->replace_records(list, ids, count);
			return ret;
		}
	}

	return __ctsvc_db_replace_records(list, ids, count);
}

API int contacts_db_insert_records(contacts_list_h record_list, int **ids, int *count)
{
	return ctsvc_db_insert_records(record_list, ids, count);
}

API int contacts_db_update_records(contacts_list_h record_list)
{
	return ctsvc_db_update_records(record_list);
}

API int contacts_db_delete_records(const char* view_uri, int record_id_array[], int count)
{
	return ctsvc_db_delete_records(view_uri, record_id_array, count);
}

API int contacts_db_replace_records(contacts_list_h list, int record_id_array[], int count)
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

API int contacts_db_get_status(contacts_db_status_e *status)
{
	*status = __db_status;
	return CONTACTS_ERROR_NONE;
}

void ctsvc_db_set_status(contacts_db_status_e status)
{
	__db_status = status;

#ifdef _CONTACTS_IPC_SERVER
	ctsvc_change_subject_publish_status(status);
#endif
	return;
}


/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjae Shin <yj99.shin@samsung.com>
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
#include "cts-internal.h"
#include "cts-schema.h"
#include "cts-utils.h"
#include "cts-types.h"
#include "cts-normalize.h"
#include "cts-restriction.h"
#include "cts-list-filter.h"


enum {
	CTS_FILTER_TYPE_NONE,
	CTS_FILTER_TYPE_INT,
	CTS_FILTER_TYPE_STR,
};

API int contacts_svc_list_filter_free(CTSfilter *filter)
{
	retv_if(NULL == filter, CTS_ERR_ARG_NULL);

	free(filter->search_val);
	free(filter);
	return CTS_SUCCESS;
}

static inline int cts_filter_parse_args(va_list args, int type, CTSfilter *ret)
{
	while (type) {
		switch (type) {
		case CTS_LIST_FILTER_NONE:
			break;
		case CTS_LIST_FILTER_ADDRESBOOK_ID_INT:
			ret->addrbook_on = true;
			ret->addrbook_id = va_arg(args, int);
			break;
		case CTS_LIST_FILTER_GROUP_ID_INT:
			ret->group_on = true;
			ret->group_id = va_arg(args, int);
			break;
		case CTS_LIST_FILTER_LIMIT_INT:
			ret->limit_on = true;
			ret->limit = va_arg(args, int);
			break;
		case CTS_LIST_FILTER_OFFSET_INT:
			ret->offset_on = true;
			ret->offset = va_arg(args, int);
			break;
		default:
			ERR("Invalid type. Your type(%d) is not supported.", type);
			return CTS_ERR_ARG_INVALID;
		}
		type = va_arg(args, int);
	}

	retvm_if(ret->offset_on && !ret->limit_on, CTS_ERR_ARG_INVALID, "OFFSET is depends on LIMIT");

	return CTS_SUCCESS;
}

API CTSfilter* contacts_svc_list_str_filter_new(cts_str_filter_op list_type,
		const char *search_value, cts_filter_type first_type, ...)
{
	int ret;
	CTSfilter *ret_val;
	va_list args;

	retvm_if(NULL == search_value, NULL, "The parameter(search_value) is NULL");

/*  DISABLED. for OSP - permission of making transparent filter
	retvm_if(CTS_LIST_FILTER_NONE == first_type, NULL,
			"filter constraint is missing(use contacts_svc_get_list_with_str()");
*/

	ret_val = calloc(1, sizeof(CTSfilter));
	ret_val->type = CTS_FILTER_TYPE_STR;
	ret_val->list_type = list_type;
	ret_val->search_val = strdup(search_value);

	va_start(args, first_type);
	ret = cts_filter_parse_args(args, first_type, ret_val);
	va_end(args);

	if (ret) {
		contacts_svc_list_filter_free(ret_val);
		return NULL;
	}

	return (CTSfilter *)ret_val;
}

API CTSfilter* contacts_svc_list_filter_new(cts_filter_op list_type, cts_filter_type first_type, ...)
{
	int ret;
	CTSfilter *ret_val;
	va_list args;

/* DISABLED. for OSP - permission of making transparent filter
	retvm_if(CTS_LIST_FILTER_NONE == first_type, NULL,
			"filter constraint is missing(use contacts_svc_get_list()");
*/
	ret_val = calloc(1, sizeof(CTSfilter));
	ret_val->type = CTS_FILTER_TYPE_NONE;
	ret_val->list_type = list_type;

	va_start(args, first_type);
	ret = cts_filter_parse_args(args, first_type, ret_val);
	va_end(args);

	if (ret) {
		contacts_svc_list_filter_free(ret_val);
		return NULL;
	}

	return (CTSfilter *)ret_val;
}

static int cts_list_str_filter_make_query(CTSfilter *filter, CTSiter *iter)
{
	int ret;
	cts_stmt stmt;
	const char *display, *data;
	char query[CTS_SQL_MAX_LEN] = {0};
	char remake_val[CTS_SQL_MIN_LEN] = {0};

	retvm_if(NULL == filter->search_val,
			CTS_ERR_ARG_INVALID, "The parameter(filter) doesn't have search_val");

	if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
		display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
	else
		display = CTS_SCHEMA_DATA_NAME_LOOKUP;

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	switch (filter->list_type) {
	case CTS_FILTERED_PLOGS_OF_NUMBER:
		iter->i_type = CTS_ITER_PLOGS_OF_NUMBER;
		if (filter->search_val && *filter->search_val) {
			ret = snprintf(query, sizeof(query),
					"SELECT A.id, A.log_type, A.log_time, A.data1, A.data2, MIN(B.contact_id) "
					"FROM %s A LEFT JOIN %s B ON A.normal_num = B.data3 AND B.datatype = %d AND "
					"(A.related_id = B.contact_id OR A.related_id IS NULL OR "
					"NOT EXISTS (SELECT id FROM %s "
					"WHERE datatype = %d AND contact_id = A.related_id AND data3 = ?)) "
					"WHERE A.number = ? "
					"GROUP BY A.id "
					"ORDER BY A.log_time DESC",
					CTS_TABLE_PHONELOGS, data, CTS_DATA_NUMBER, data, CTS_DATA_NUMBER);
		}
		else {
			ret = snprintf(query, sizeof(query),
					"SELECT id, log_type, log_time, data1, data2, NULL "
					"FROM %s WHERE number ISNULL and log_type < %d "
					"ORDER BY id DESC",
					CTS_TABLE_PHONELOGS, CTS_PLOG_TYPE_EMAIL_RECEIVED);
		}
		if (filter->limit_on) {
			ret += snprintf(query+ret, sizeof(query)-ret, " LIMIT %d", filter->limit);
			if (filter->offset_on)
				ret += snprintf(query+ret, sizeof(query)-ret, " OFFSET %d", filter->offset);
		}

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		if (filter->search_val) {
			const char *normal_num;
			ret = cts_clean_number(filter->search_val, remake_val, sizeof(remake_val));
			retvm_if(ret <= 0, CTS_ERR_ARG_INVALID, "Number(%s) is invalid", filter->search_val);

			normal_num = cts_normalize_number(remake_val);
			cts_stmt_bind_copy_text(stmt, 1, normal_num, strlen(normal_num));
			cts_stmt_bind_copy_text(stmt, 2, filter->search_val, strlen(filter->search_val));
		}
		iter->stmt = stmt;
		break;
	case CTS_FILTERED_CONTACTS_WITH_NAME:
		retvm_if(CTS_SQL_MIN_LEN <= strlen(filter->search_val), CTS_ERR_ARG_INVALID,
				"search_value is too long");
		iter->i_type = CTS_ITER_CONTACTS_WITH_NAME;
		memset(remake_val, 0x00, sizeof(remake_val));

		ret = cts_normalize_str(filter->search_val, remake_val, CTS_SQL_MIN_LEN);
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_str() Failed(%d)", ret);

		if (filter->addrbook_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT B.person_id, data1, data2, data3, data5, B.addrbook_id, B.image0, B.contact_id "
					"FROM %s A, %s B ON A.contact_id = B.contact_id "
					"WHERE datatype = %d AND B.addrbook_id = %d AND %s LIKE ('%%' || ? || '%%') "
					"ORDER BY data1, %s",
					data, CTS_TABLE_CONTACTS, CTS_DATA_NAME, filter->addrbook_id,
					display, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else if (filter->group_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT B.person_id, data1, data2, data3, data5, B.addrbook_id, B.image0, B.contact_id "
					"FROM %s A, %s B ON A.contact_id = B.contact_id "
					"WHERE datatype = %d AND %s LIKE ('%%' || ? || '%%') "
					"AND contact_id IN (SELECT contact_id FROM %s WHERE group_id = %d) "
					"ORDER BY data1, %s",
					data, CTS_TABLE_CONTACTS, CTS_DATA_NAME, display,
					CTS_TABLE_GROUPING_INFO, filter->group_id, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else {
			ret = snprintf(query, sizeof(query),
					"SELECT B.person_id, data1, data2, data3, data5, B.addrbook_id, B.image0, B.contact_id "
					"FROM %s A, %s B ON A.contact_id = B.contact_id "
					"WHERE datatype = %d AND %s LIKE ('%%' || ? || '%%') "
					"ORDER BY data1, %s",
					data, CTS_TABLE_CONTACTS, CTS_DATA_NAME, display, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		}

		if (filter->limit_on) {
			ret += snprintf(query+ret, sizeof(query)-ret, " LIMIT %d", filter->limit);
			if (filter->offset_on)
				ret += snprintf(query+ret, sizeof(query)-ret, " OFFSET %d", filter->offset);
		}
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		cts_stmt_bind_copy_text(stmt, 1, remake_val, strlen(remake_val));
		break;
	case CTS_FILTERED_NUMBERINFOS_WITH_NAME:
		retvm_if(CTS_SQL_MIN_LEN <= strlen(filter->search_val), CTS_ERR_ARG_INVALID,
				"search_value is too long");
		iter->i_type = CTS_ITER_NUMBERINFOS;
		memset(remake_val, 0x00, sizeof(remake_val));

		ret = cts_normalize_str(filter->search_val, remake_val, CTS_SQL_MIN_LEN);
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_str() Failed(%d)", ret);

		if (filter->addrbook_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT C.person_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0, C.contact_id "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = C.person_id AND B.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d "
					"AND C.addrbook_id = %d AND A.%s LIKE ('%%' || ? || '%%') "
					"ORDER BY A.data1, A.%s",
					data, data, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER,
					filter->addrbook_id, display, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else if (filter->group_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT C.person_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0, C.contact_id "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = C.person_id AND B.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d AND A.%s LIKE ('%%' || ? || '%%') "
					"AND A.contact_id IN "
					"(SELECT contact_id FROM %s WHERE group_id = %d) "
					"ORDER BY A.data1, A.%s",
					data, data, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER, display,
					CTS_TABLE_GROUPING_INFO, filter->group_id, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else {
			ret = snprintf(query, sizeof(query),
					"SELECT C.person_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0, C.contact_id "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = C.person_id AND B.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d AND A.%s LIKE ('%%' || ? || '%%') "
					"ORDER BY A.data1, A.%s",
					data, data, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER,
					display, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		}
		if (filter->limit_on) {
			ret += snprintf(query+ret, sizeof(query)-ret, " LIMIT %d", filter->limit);
			if (filter->offset_on)
				ret += snprintf(query+ret, sizeof(query)-ret, " OFFSET %d", filter->offset);
		}

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		cts_stmt_bind_copy_text(stmt, 1, remake_val, strlen(remake_val));
		break;
	case CTS_FILTERED_NUMBERINFOS_WITH_NUM:
		iter->i_type = CTS_ITER_NUMBERINFOS;

		if (filter->addrbook_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT C.person_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0, C.contact_id "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = C.person_id AND B.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d "
					"AND C.addrbook_id = %d AND B.data2 LIKE ('%%' || ? || '%%') "
					"ORDER BY A.data1, A.%s",
					data, data, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER,
					filter->addrbook_id, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else if (filter->group_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT C.person_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0, C.contact_id "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = C.person_id AND B.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d AND B.data2 LIKE ('%%' || ? || '%%') "
					"AND A.contact_id IN (SELECT contact_id FROM %s WHERE group_id = %d) "
					"ORDER BY A.data1, A.%s",
					data, data, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER,
					CTS_TABLE_GROUPING_INFO, filter->group_id, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else {
			ret = snprintf(query, sizeof(query),
					"SELECT C.person_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0, C.contact_id "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = C.person_id AND B.contact_id = C.contact_id "
					"WHERE B.data2 LIKE ('%%' || ? || '%%') "
					"AND A.datatype = %d AND B.datatype = %d "
					"ORDER BY A.data1, A.%s",
					data, data, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		}
		if (filter->limit_on) {
			ret += snprintf(query+ret, sizeof(query)-ret, " LIMIT %d", filter->limit);
			if (filter->offset_on)
				ret += snprintf(query+ret, sizeof(query)-ret, " OFFSET %d", filter->offset);
		}

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		cts_stmt_bind_copy_text(stmt, 1, filter->search_val, strlen(filter->search_val));
		break;
	case CTS_FILTERED_EMAILINFOS_WITH_EMAIL:
		iter->i_type = CTS_ITER_EMAILINFOS_WITH_EMAIL;

		if (filter->addrbook_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT C.person_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0, C.contact_id "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = C.person_id AND B.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d AND C.addrbook_id = %d "
					"AND B.data2 LIKE ('%%' || ? || '%%') "
					"ORDER BY A.data1, A.%s",
					data, data, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_EMAIL, filter->addrbook_id,
					CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else if (filter->group_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT C.person_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0, C.contact_id "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = C.person_id AND B.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d AND "
					"B.data2 LIKE ('%%' || ? || '%%') AND A.contact_id IN "
					"(SELECT contact_id FROM %s WHERE group_id = %d) "
					"ORDER BY A.data1, A.%s",
					data, data, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_EMAIL,
					CTS_TABLE_GROUPING_INFO, filter->group_id,
					CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else {
			ret = snprintf(query, sizeof(query),
					"SELECT C.person_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0, C.contact_id "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = C.person_id AND B.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d AND B.data2 LIKE ('%%' || ? || '%%') "
					"ORDER BY A.data1, A.%s",
					data, data, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_EMAIL,
					CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		}
		if (filter->limit_on) {
			ret += snprintf(query+ret, sizeof(query)-ret, " LIMIT %d", filter->limit);
			if (filter->offset_on)
				ret += snprintf(query+ret, sizeof(query)-ret, " OFFSET %d", filter->offset);
		}

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		cts_stmt_bind_copy_text(stmt, 1, filter->search_val, strlen(filter->search_val));
		break;
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", filter->list_type);
		return CTS_ERR_ARG_INVALID;
	}
	iter->stmt = stmt;

	return CTS_SUCCESS;
}

static inline void cts_filter_make_query_ALL_CONTACT(CTSfilter *filter, char *buf, int buf_size)
{
	int ret;
	const char *display, *data;

	if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
		display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
	else
		display = CTS_SCHEMA_DATA_NAME_LOOKUP;

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	if (filter->addrbook_on) {
		ret = snprintf(buf, buf_size,
				"SELECT B.person_id, data1, data2, data3, data5, B.addrbook_id, B.image0, B.contact_id, %s "
				"FROM %s A, %s B ON A.contact_id = B.person_id "
				"WHERE datatype = %d AND B.addrbook_id = %d "
				"GROUP BY B.person_id "
				"ORDER BY data1, %s",
				display, data, CTS_TABLE_CONTACTS,
				CTS_DATA_NAME, filter->addrbook_id, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	} else if (filter->group_on) {
		ret = snprintf(buf, buf_size,
				"SELECT B.person_id, data1, data2, data3, data5, B.addrbook_id, B.image0, B.contact_id, %s "
				"FROM %s A, %s B ON A.contact_id = B.person_id "
				"WHERE datatype = %d AND B.contact_id IN "
				"(SELECT contact_id FROM %s WHERE group_id = %d) "
				"GROUP BY B.person_id "
				"ORDER BY data1, %s",
				display, data, CTS_TABLE_CONTACTS, CTS_DATA_NAME,
				CTS_TABLE_GROUPING_INFO, filter->group_id, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	} else {
		ret = snprintf(buf, buf_size,
				"SELECT B.person_id, data1, data2, data3, data5, B.addrbook_id, B.image0, B.person_id, %s "
				"FROM %s A, %s B ON A.contact_id = B.person_id "
				"WHERE A.datatype = %d AND B.person_id = B.contact_id "
				"ORDER BY A.data1, A.%s",
				display, data, CTS_TABLE_CONTACTS,
				CTS_DATA_NAME, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	}

	if (filter->limit_on) {
		ret += snprintf(buf+ret, buf_size-ret, " LIMIT %d", filter->limit);
		if (filter->offset_on)
			ret += snprintf(buf+ret, buf_size-ret, " OFFSET %d", filter->offset);
	}
}

static inline void cts_filter_make_query_ALL_CONTACT_OSP(CTSfilter *filter, char *buf, int buf_size)
{
	int ret;
	const char *display, *data;

	if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
		display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
	else
		display = CTS_SCHEMA_DATA_NAME_LOOKUP;

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	if (filter->addrbook_on) {
		ret = snprintf(buf, buf_size,
				"SELECT B.person_id, A.data1, A.data2, A.data3, A.data5, B.addrbook_id, B.image0, B.person_id, "
				"C.data1, C.data2, D.data1, D.data2, A.%s "
				"FROM (%s A, %s B ON A.contact_id = B.person_id) LEFT JOIN %s C ON B.default_num = C.id "
				"LEFT JOIN %s D ON B.default_email = D.id "
				"WHERE A.datatype = %d AND B.addrbook_id = %d "
				"GROUP BY B.person_id "
				"ORDER BY A.data1, A.%s",
				display, data, CTS_TABLE_CONTACTS, data, data,
				CTS_DATA_NAME, filter->addrbook_id,
				CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	} else if (filter->group_on) {
		ret = snprintf(buf, buf_size,
				"SELECT B.person_id, A.data1, A.data2, A.data3, A.data5, B.addrbook_id, B.image0, B.person_id, "
				"C.data1, C.data2, D.data1, D.data2, A.%s "
				"FROM (%s A, %s B ON A.contact_id = B.person_id) LEFT JOIN %s C ON B.default_num = C.id "
				"LEFT JOIN %s D ON B.default_email = D.id "
				"WHERE A.datatype = %d AND B.contact_id IN (SELECT contact_id FROM %s WHERE group_id = %d) "
				"GROUP BY B.person_id "
				"ORDER BY A.data1, A.%s",
				display, data, CTS_TABLE_CONTACTS, data, data,
				CTS_DATA_NAME, CTS_TABLE_GROUPING_INFO, filter->group_id,
				CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	} else {
		ret = snprintf(buf, buf_size,
				"SELECT B.person_id, A.data1, A.data2, A.data3, A.data5, B.addrbook_id, B.image0, B.person_id, "
				"C.data1, C.data2, D.data1, D.data2, A.%s "
				"FROM (%s A, %s B ON A.contact_id = B.person_id) LEFT JOIN %s C ON B.default_num = C.id "
				"LEFT JOIN %s D ON B.default_email = D.id "
				"WHERE A.datatype = %d AND B.person_id = B.contact_id "
				"ORDER BY A.data1, A.%s",
				display, data, CTS_TABLE_CONTACTS, data, data,
				CTS_DATA_NAME, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	}

	if (filter->limit_on) {
		ret += snprintf(buf+ret, buf_size-ret, " LIMIT %d", filter->limit);
		if (filter->offset_on)
			ret += snprintf(buf+ret, buf_size-ret, " OFFSET %d", filter->offset);
	}
}

static inline void cts_filter_make_query_ALL_CONTACT_HAD_NUMBER(CTSfilter *filter, char *buf, int buf_size)
{
	int ret;
	const char *display, *data;

	if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
		display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
	else
		display = CTS_SCHEMA_DATA_NAME_LOOKUP;

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	if (filter->addrbook_on) {
		ret = snprintf(buf, buf_size,
				"SELECT B.person_id, A.data1, A.data2, A.data3, A.data5, B.addrbook_id, B.image0, B.contact_id, A.%s "
				"FROM %s A, %s B ON A.contact_id = B.person_id "
				"WHERE A.datatype = %d AND B.default_num > 0 AND B.addrbook_id = %d "
				"GROUP BY B.person_id "
				"ORDER BY A.data1, A.%s",
				display, data, CTS_TABLE_CONTACTS, CTS_DATA_NAME, filter->addrbook_id,
				CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	} else if (filter->group_on) {
		ret = snprintf(buf, buf_size,
				"SELECT B.person_id, A.data1, A.data2, A.data3, A.data5, B.addrbook_id, B.image0, B.contact_id, A.%s "
				"FROM %s A, %s B ON A.contact_id = B.person_id "
				"WHERE A.datatype = %d AND B.default_num > 0 AND B.contact_id IN "
					"(SELECT contact_id FROM %s WHERE group_id = %d) "
				"GROUP BY B.person_id "
				"ORDER BY A.data1, A.%s",
				display, data, CTS_TABLE_CONTACTS, CTS_DATA_NAME,
				CTS_TABLE_GROUPING_INFO, filter->group_id, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	} else {
		snprintf(buf, buf_size,
				"SELECT B.person_id, A.data1, A.data2, A.data3, A.data5, B.addrbook_id, B.image0, B.contact_id, A.%s "
				"FROM %s A, %s B ON A.contact_id = B.person_id "
				"WHERE A.datatype = %d AND B.default_num > 0 "
				"GROUP BY B.person_id "
				"ORDER BY A.data1, A.%s",
				display, data, CTS_TABLE_CONTACTS, CTS_DATA_NAME, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	}

	if (filter->limit_on) {
		ret += snprintf(buf+ret, buf_size-ret, " LIMIT %d", filter->limit);
		if (filter->offset_on)
			ret += snprintf(buf+ret, buf_size-ret, " OFFSET %d", filter->offset);
	}
}


static inline void cts_filter_make_query_ALL_CONTACT_HAD_EMAIL(CTSfilter *filter, char *buf, int buf_size)
{
	int ret;
	const char *display, *data;

	if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
		display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
	else
		display = CTS_SCHEMA_DATA_NAME_LOOKUP;

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	if (filter->addrbook_on) {
		ret = snprintf(buf, buf_size,
				"SELECT B.person_id, A.data1, A.data2, A.data3, A.data5, B.addrbook_id, B.image0, B.contact_id, A.%s "
				"FROM %s A, %s B ON A.contact_id = B.person_id "
				"WHERE A.datatype = %d AND B.default_email > 0 AND B.addrbook_id = %d "
				"GROUP BY B.person_id "
				"ORDER BY A.data1, A.%s",
				display, data, CTS_TABLE_CONTACTS, CTS_DATA_NAME, filter->addrbook_id,
				CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	} else if (filter->group_on) {
		ret = snprintf(buf, buf_size,
				"SELECT B.person_id, A.data1, A.data2, A.data3, A.data5, B.addrbook_id, B.image0, B.contact_id, A.%s "
				"FROM %s A, %s B ON A.contact_id = B.person_id "
				"WHERE A.datatype = %d AND B.default_email > 0 AND B.contact_id IN "
					"(SELECT contact_id FROM %s WHERE group_id = %d) "
				"GROUP BY B.person_id "
				"ORDER BY A.data1, A.%s",
				display, data, CTS_TABLE_CONTACTS, CTS_DATA_NAME,
				CTS_TABLE_GROUPING_INFO, filter->group_id, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	} else {
		snprintf(buf, buf_size,
				"SELECT B.person_id, A.data1, A.data2, A.data3, A.data5, B.addrbook_id, B.image0, B.contact_id, A.%s "
				"FROM %s A, %s B ON A.contact_id = B.person_id "
				"WHERE A.datatype = %d AND B.default_email > 0 "
				"GROUP BY B.person_id "
				"ORDER BY A.data1, A.%s",
				display, data, CTS_TABLE_CONTACTS, CTS_DATA_NAME, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	}

	if (filter->limit_on) {
		ret += snprintf(buf+ret, buf_size-ret, " LIMIT %d", filter->limit);
		if (filter->offset_on)
			ret += snprintf(buf+ret, buf_size-ret, " OFFSET %d", filter->offset);
	}
}

static int cts_list_filter_make_query(CTSfilter *filter, CTSiter *iter)
{
	cts_stmt stmt;
	const char *display, *data;
	char query[CTS_SQL_MAX_LEN] = {0};

	if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
		display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
	else
		display = CTS_SCHEMA_DATA_NAME_LOOKUP;

	if (cts_restriction_get_permit())
		data = CTS_TABLE_DATA;
	else
		data = CTS_TABLE_RESTRICTED_DATA_VIEW;

	switch (filter->list_type) {
	case CTS_FILTERED_ALL_CONTACT:
		iter->i_type = CTS_ITER_CONTACTS;

		cts_filter_make_query_ALL_CONTACT(filter, query, sizeof(query));

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_FILTERED_ALL_CONTACT_OSP:
		iter->i_type = CTS_ITER_OSP;

		cts_filter_make_query_ALL_CONTACT_OSP(filter, query, sizeof(query));

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		break;
	case CTS_FILTERED_ALL_CONTACT_HAD_NUMBER:
		iter->i_type = CTS_ITER_CONTACTS;

		cts_filter_make_query_ALL_CONTACT_HAD_NUMBER(filter, query, sizeof(query));

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		break;
	case CTS_FILTERED_ALL_CONTACT_HAD_EMAIL:
		iter->i_type = CTS_ITER_CONTACTS;

		cts_filter_make_query_ALL_CONTACT_HAD_EMAIL(filter, query, sizeof(query));

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		break;
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", filter->list_type);
		return CTS_ERR_ARG_INVALID;
	}
	iter->stmt = stmt;

	return CTS_SUCCESS;
}

API int contacts_svc_get_list_with_filter(CTSfilter *filter, CTSiter **iter)
{
	int ret;
	CTSiter *result;

	retv_if(NULL == filter, CTS_ERR_ARG_NULL);
	retv_if(NULL == iter, CTS_ERR_ARG_NULL);
	retvm_if(CTS_FILTER_TYPE_NONE != filter->type && CTS_FILTER_TYPE_STR != filter->type,
			CTS_ERR_ARG_INVALID, "Invalid CTSfilter(type = %d)", filter->type);

	result = calloc(1, sizeof(CTSiter));
	retvm_if(NULL == result, CTS_ERR_OUT_OF_MEMORY, "calloc() Failed");

	if (CTS_FILTER_TYPE_NONE == filter->type) {
		ret = cts_list_filter_make_query(filter, result);
		if (ret) {
			ERR("cts_list_filter_make_query() Failed(%d)", ret);
			free(result);
			return ret;
		}
	} else if (CTS_FILTER_TYPE_STR == filter->type) {
		ret = cts_list_str_filter_make_query(filter, result);
		if (ret) {
			ERR("cts_list_str_filter_make_query() Failed(%d)", ret);
			free(result);
			return ret;
		}
	}

	*iter = (CTSiter *)result;
	INFO(",CTSiter,1");
	return CTS_SUCCESS;
}

API int contacts_svc_list_with_filter_foreach(CTSfilter *filter,
		cts_foreach_fn cb, void *user_data)
{
	int ret;
	CTSiter iter = {0};

	if (CTS_FILTER_TYPE_STR == filter->type) {
		ret = cts_list_str_filter_make_query(filter, &iter);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_list_str_filter_make_query() Failed(%d)", ret);
	} else if (CTS_FILTER_TYPE_NONE == filter->type) {
		ret = cts_list_filter_make_query(filter, &iter);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_list_filter_make_query() Failed(%d)", ret);
	} else {
		ERR("Invalid CTSfilter(type = %d)", filter->type);
		return CTS_ERR_ARG_INVALID;
	}

	cts_foreach_run(&iter, cb, user_data);

	return CTS_SUCCESS;
}


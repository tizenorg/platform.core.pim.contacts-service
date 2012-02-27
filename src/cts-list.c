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
#include "cts-favorite.h"
#include "cts-list.h"

#define CTS_MALLOC_DEFAULT_NUM 256 //4Kbytes
#define CTS_OFTEN_USED_NUM 1

static inline updated_contact* cts_updated_contact_add_mempool(void)
{
	int i;
	updated_contact *mempool;

	mempool = calloc(CTS_MALLOC_DEFAULT_NUM, sizeof(updated_contact));
	for (i=0;i<CTS_MALLOC_DEFAULT_NUM-1;i++)
		mempool[i].next = &mempool[i+1];
	return mempool;
}

static inline int cts_updated_contact_free_mempool(updated_contact *mempool)
{
	updated_contact *memseg, *tmp;

	retv_if(NULL == mempool, CTS_ERR_ARG_NULL);

	memseg = mempool;
	while (memseg) {
		tmp = memseg[CTS_MALLOC_DEFAULT_NUM-1].next;
		free(memseg);
		memseg = tmp;
	}

	return CTS_SUCCESS;
}

API int contacts_svc_iter_next(CTSiter *iter)
{
	int ret;

	retv_if(NULL == iter, CTS_ERR_ARG_NULL);
	retvm_if(iter->i_type <= CTS_ITER_NONE || CTS_ITER_MAX <= iter->i_type,
			CTS_ERR_ARG_INVALID, "iter is Invalid(type=%d", iter->i_type);

	if (CTS_ITER_UPDATED_CONTACTS_AFTER_VER == iter->i_type)
	{
		retv_if(NULL == iter->info, CTS_ERR_ARG_INVALID);

		if (NULL == iter->info->cursor)
			iter->info->cursor = iter->info->head;
		else
			iter->info->cursor = iter->info->cursor->next;
		if (NULL == iter->info->cursor || 0 == iter->info->cursor->id) {
			iter->info->cursor = NULL;
			cts_updated_contact_free_mempool(iter->info->head);
			iter->info->head = NULL;
			return CTS_ERR_FINISH_ITER;
		}
	}
	else
	{
		ret = cts_stmt_step(iter->stmt);
		if (CTS_TRUE != ret)
		{
			if (CTS_SUCCESS != ret)
				ERR("cts_stmt_step() Failed(%d)", ret);
			cts_stmt_finalize(iter->stmt);
			iter->stmt = NULL;
			return CTS_ERR_FINISH_ITER;
		}
	}
	return CTS_SUCCESS;
}

API int contacts_svc_iter_remove(CTSiter *iter)
{
	retv_if(NULL == iter, CTS_ERR_ARG_NULL);
	retvm_if(iter->i_type <= CTS_ITER_NONE || CTS_ITER_MAX <= iter->i_type,
			CTS_ERR_ARG_INVALID, "iter is Invalid(type=%d", iter->i_type);

	if (CTS_ITER_UPDATED_CONTACTS_AFTER_VER == iter->i_type) {
		retv_if(NULL == iter->info, CTS_ERR_ARG_INVALID);
		if (iter->info->head)
			cts_updated_contact_free_mempool(iter->info->head);
		free(iter->info);
	}
	else {
		cts_stmt_finalize(iter->stmt);
	}

	free(iter);
	return CTS_SUCCESS;
}

static inline int cts_get_list(cts_get_list_op op_code, CTSiter *iter)
{
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	const char *display;

	retv_if(NULL == iter, CTS_ERR_ARG_NULL);

	iter->i_type = CTS_ITER_NONE;
	iter->stmt = NULL;

	switch (op_code)
	{
	case CTS_LIST_ALL_CONTACT:
		iter->i_type = CTS_ITER_CONTACTS;

		if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
		else
			display = CTS_SCHEMA_DATA_NAME_LOOKUP;

		snprintf(query, sizeof(query),
				"SELECT A.contact_id, data1, data2, data3, data5, B.addrbook_id, B.image0, %s "
				"FROM %s A, %s B ON A.contact_id = B.contact_id "
				"WHERE A.datatype = %d "
				"ORDER BY A.data1, A.%s",
				display, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
				CTS_DATA_NAME, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_ADDRESSBOOK:
		iter->i_type = CTS_ITER_ADDRESSBOOKS;
		snprintf(query, sizeof(query),
				"SELECT addrbook_id, addrbook_name, acc_id, acc_type, mode "
				"FROM %s ORDER BY acc_id, addrbook_id",
				CTS_TABLE_ADDRESSBOOKS);

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_GROUP:
		iter->i_type = CTS_ITER_GROUPS;
		snprintf(query, sizeof(query), "SELECT group_id, addrbook_id, group_name "
				"FROM %s ORDER BY addrbook_id, group_name COLLATE NOCASE",
				CTS_TABLE_GROUPS);

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_CUSTOM_NUM_TYPE:
		iter->i_type = CTS_ITER_ALL_CUSTOM_NUM_TYPE;
		snprintf(query, sizeof(query), "SELECT id, name FROM %s WHERE class = %d",
				CTS_TABLE_CUSTOM_TYPES, CTS_TYPE_CLASS_NUM);

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_GROUPING_PLOG:
		iter->i_type = CTS_ITER_GROUPING_PLOG;
		snprintf(query, sizeof(query),
				"SELECT C.id, F.data1, F.data2, F.data3, F.data5, F.image0, C.number, "
				"C.log_type, C.log_time, C.data1, C.data2, C.contact_id, C.number_type "
				"FROM "
				"(SELECT A.id, A.number, A.log_type, A.log_time, A.data1, A.data2, "
				"MIN(B.contact_id) contact_id, B.data1 number_type "
				"FROM %s A LEFT JOIN %s B ON B.datatype = %d AND A.normal_num = B.data3 AND "
				"(A.related_id = B.contact_id OR A.related_id IS NULL OR "
				"NOT EXISTS (SELECT id FROM %s "
				"WHERE datatype = %d AND contact_id = A.related_id "
				"AND A.normal_num = data3)) "
				"GROUP BY A.id) C "
				"LEFT JOIN (SELECT D.contact_id, data1, data2, data3, data5, image0 "
				"FROM %s D, %s E ON D.datatype = %d AND D.contact_id = E.contact_id) F "
				"ON C.contact_id = F.contact_id "
				"GROUP BY F.data2, F.data3, F.data5, C.number "
				"ORDER BY C.log_time DESC",
				CTS_TABLE_PHONELOGS, CTS_TABLE_DATA, CTS_DATA_NUMBER, CTS_TABLE_DATA,
				CTS_DATA_NUMBER, CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_GROUPING_MSG_PLOG:
		iter->i_type = CTS_ITER_GROUPING_PLOG;
		snprintf(query, sizeof(query),
				"SELECT C.id, F.data1, F.data2, F.data3, F.data5, F.image0, C.number, "
				"C.log_type, C.log_time, C.data1, C.data2, C.contact_id, C.number_type "
				"FROM "
				"(SELECT A.id, A.number, A.log_type, A.log_time, A.data1, A.data2, "
				"MIN(B.contact_id) contact_id, B.data1 number_type "
				"FROM %s A LEFT JOIN %s B ON B.datatype = %d AND A.normal_num = B.data3 AND "
				"(A.related_id = B.contact_id OR A.related_id IS NULL OR "
				"NOT EXISTS (SELECT id FROM %s WHERE datatype = %d AND contact_id = A.related_id "
				"AND A.normal_num = data3)) "
				"WHERE A.log_type >= %d "
				"GROUP BY A.id) C "
				"LEFT JOIN (SELECT D.contact_id, data1, data2, data3, data5, image0 "
				"FROM %s D, %s E ON D.datatype = %d AND D.contact_id = E.contact_id) F "
				"ON C.contact_id = F.contact_id "
				"GROUP BY F.data2, F.data3, F.data5, C.number "
				"ORDER BY C.log_time DESC",
				CTS_TABLE_PHONELOGS, CTS_TABLE_DATA, CTS_DATA_NUMBER, CTS_TABLE_DATA, CTS_DATA_NUMBER,
				CTS_PLOG_TYPE_MMS_INCOMMING, CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_GROUPING_CALL_PLOG:
		iter->i_type = CTS_ITER_GROUPING_PLOG;
		snprintf(query, sizeof(query),
				"SELECT C.id, F.data1, F.data2, F.data3, F.data5, F.image0, C.number, "
				"C.log_type, C.log_time, C.data1, C.data2, C.contact_id, C.number_type "
				"FROM "
				"(SELECT A.id, A.number, A.log_type, A.log_time, A.data1, A.data2, "
				"MIN(B.contact_id) contact_id, B.data1 number_type "
				"FROM %s A LEFT JOIN %s B ON B.datatype = %d AND A.normal_num = B.data3 AND "
				"(A.related_id = B.contact_id OR A.related_id IS NULL OR "
				"NOT EXISTS (SELECT id FROM %s WHERE datatype = %d AND contact_id = A.related_id "
				"AND A.normal_num = data3)) "
				"WHERE A.log_type < %d "
				"GROUP BY A.id) C "
				"LEFT JOIN (SELECT D.contact_id, data1, data2, data3, data5, image0 "
				"FROM %s D, %s E ON D.datatype = %d AND D.contact_id = E.contact_id) F "
				"ON C.contact_id = F.contact_id "
				"GROUP BY F.data2, F.data3, F.data5, C.number "
				"ORDER BY C.log_time DESC",
				CTS_TABLE_PHONELOGS, CTS_TABLE_DATA, CTS_DATA_NUMBER, CTS_TABLE_DATA, CTS_DATA_NUMBER,
				CTS_PLOG_TYPE_MMS_INCOMMING, CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_PLOG:
		iter->i_type = CTS_ITER_GROUPING_PLOG;
		snprintf(query, sizeof(query),
				"SELECT C.id, F.data1, F.data2, F.data3, F.data5, F.image0, C.number, "
				"C.log_type, C.log_time, C.data1, C.data2, C.contact_id, C.number_type "
				"FROM "
				"(SELECT A.id, A.number, A.log_type, A.log_time, A.data1, A.data2, "
				"MIN(B.contact_id) contact_id, B.data1 number_type "
				"FROM %s A LEFT JOIN %s B ON B.datatype = %d AND A.normal_num = B.data3 AND "
				"(A.related_id = B.contact_id OR A.related_id IS NULL OR "
				"NOT EXISTS (SELECT id FROM %s "
				"WHERE datatype = %d AND contact_id = A.related_id "
				"AND A.normal_num = data3)) "
				"GROUP BY A.id) C "
				"LEFT JOIN (SELECT D.contact_id, data1, data2, data3, data5, image0 "
				"FROM %s D, %s E ON D.datatype = %d AND D.contact_id = E.contact_id) F "
				"ON C.contact_id = F.contact_id "
				"ORDER BY C.log_time DESC",
				CTS_TABLE_PHONELOGS, CTS_TABLE_DATA, CTS_DATA_NUMBER, CTS_TABLE_DATA,
				CTS_DATA_NUMBER, CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_MISSED_CALL:
		iter->i_type = CTS_ITER_GROUPING_PLOG;
		snprintf(query, sizeof(query),
				"SELECT C.id, F.data1, F.data2, F.data3, F.data5, F.image0, C.number, "
				"C.log_type, C.log_time, C.data1, C.data2, C.contact_id, C.number_type "
				"FROM "
				"(SELECT A.id, A.number, A.log_type, A.log_time, A.data1, A.data2, "
				"MIN(B.contact_id) contact_id, B.data1 number_type "
				"FROM %s A LEFT JOIN %s B ON B.datatype = %d AND A.normal_num = B.data3 AND "
				"(A.related_id = B.contact_id OR A.related_id IS NULL OR "
				"NOT EXISTS (SELECT id FROM %s "
				"WHERE datatype = %d AND contact_id = A.related_id "
				"AND A.normal_num = data3)) "
				"WHERE (A.log_type = %d OR A.log_type = %d) "
				"GROUP BY A.id) C "
				"LEFT JOIN (SELECT D.contact_id, data1, data2, data3, data5, image0 "
				"FROM %s D, %s E ON D.datatype = %d AND D.contact_id = E.contact_id) F "
				"ON C.contact_id = F.contact_id "
				"ORDER BY C.log_time DESC",
				CTS_TABLE_PHONELOGS, CTS_TABLE_DATA, CTS_DATA_NUMBER, CTS_TABLE_DATA, CTS_DATA_NUMBER,
				CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN, CTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN,
				CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_NUMBER_FAVORITE:
		iter->i_type = CTS_ITER_ALL_NUM_FAVORITE;
		snprintf(query, sizeof(query),
				"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, D.image0, "
				"B.id, B.data1, B.data2 "
				"FROM %s A, %s B, %s C, %s D "
				"ON A.contact_id = B.contact_id AND B.id = C.related_id AND A.contact_id = D.contact_id "
				"WHERE A.datatype = %d AND B.datatype = %d AND C.type = %d "
				"ORDER BY C.favorite_prio",
				CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_FAVORITES, CTS_TABLE_CONTACTS,
				CTS_DATA_NAME, CTS_DATA_NUMBER, CTS_FAVOR_NUMBER);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_CONTACT_FAVORITE:
		iter->i_type = CTS_ITER_ALL_CONTACT_FAVORITE;
		snprintf(query, sizeof(query),
				"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, C.image0, B.id "
				"FROM %s A, %s B, %s C "
				"ON A.contact_id = B.related_id AND A.contact_id = C.contact_id "
				"WHERE A.datatype = %d AND B.type = %d "
				"ORDER BY B.favorite_prio",
				CTS_TABLE_DATA, CTS_TABLE_FAVORITES, CTS_TABLE_CONTACTS,
				CTS_DATA_NAME, CTS_FAVOR_CONTACT);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_SPEEDDIAL:
		iter->i_type = CTS_ITER_ALL_SPEEDDIAL;
		snprintf(query, sizeof(query),
				"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, D.image0, "
				"B.id, B.data1, B.data2, C.speed_num "
				"FROM %s A, %s B, %s C, %s D "
				"WHERE A.datatype = %d AND B.datatype = %d AND B.id = C.number_id "
				"AND A.contact_id = B.contact_id AND A.contact_id = D.contact_id "
				"ORDER BY C.speed_num",
				CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_SPEEDDIALS,
				CTS_TABLE_CONTACTS, CTS_DATA_NAME, CTS_DATA_NUMBER);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_SDN:
		iter->i_type = CTS_ITER_ALL_SDN;
		snprintf(query, sizeof(query),"SELECT name, number FROM %s",
				CTS_TABLE_SIM_SERVICES);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_CONTACT_HAD_NUMBER:
		iter->i_type = CTS_ITER_CONTACTS;

		if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
		else
			display = CTS_SCHEMA_DATA_NAME_LOOKUP;

		snprintf(query, sizeof(query),
				"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.addrbook_id, B.image0, A.%s "
				"FROM %s A, %s B ON A.contact_id = B.contact_id "
				"WHERE A.datatype = %d AND B.default_num > 0 "
				"ORDER BY A.data1, A.%s",
				display, CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_CONTACT_HAD_EMAIL:
		iter->i_type = CTS_ITER_CONTACTS;
		if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
		else
			display = CTS_SCHEMA_DATA_NAME_LOOKUP;

		snprintf(query, sizeof(query),
				"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.addrbook_id, B.image0, A.%s "
				"FROM %s A, %s B ON A.contact_id = B.contact_id "
				"WHERE A.datatype = %d AND B.default_email > 0 "
				"ORDER BY A.data1, A.%s",
				display, CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_EMAIL_NUMBER:
		iter->i_type = CTS_ITER_NUMBERS_EMAILS;

		if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
		else
			display = CTS_SCHEMA_DATA_NAME_LOOKUP;

		snprintf(query, sizeof(query),
				"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0, C.addrbook_id, A.%s "
				"FROM %s A, %s B, %s C "
				"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
				"WHERE A.datatype = %d AND (B.datatype = %d OR B.datatype = %d) "
				"ORDER BY A.data1, A.%s",
				display, CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
				CTS_DATA_NAME, CTS_DATA_NUMBER, CTS_DATA_EMAIL, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_OFTEN_USED_CONTACT:
		iter->i_type = CTS_ITER_CONTACTS;

		if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
		else
			display = CTS_SCHEMA_DATA_NAME_LOOKUP;

		snprintf(query, sizeof(query),
				"SELECT A.contact_id, data1, data2, data3, data5, A.addrbook_id, A.image0, %s "
				"FROM %s A, %s B "
				"WHERE A.outgoing_count > %d AND B.datatype = %d "
				"AND B.contact_id = A.contact_id "
				"ORDER BY A.outgoing_count DESC, data1, %s",
				display, CTS_TABLE_CONTACTS, CTS_TABLE_DATA,
				CTS_OFTEN_USED_NUM, CTS_DATA_NAME, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ALL_NUMBER:
		iter->i_type = CTS_ITER_NUMBERS_EMAILS;

		if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
		else
			display = CTS_SCHEMA_DATA_NAME_LOOKUP;

		snprintf(query, sizeof(query),
				"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0, C.addrbook_id, A.%s "
				"FROM %s A, %s B, %s C "
				"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
				"WHERE A.datatype = %d AND (B.datatype = %d) "
				"ORDER BY A.data1, A.%s",
				display, CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
				CTS_DATA_NAME, CTS_DATA_NUMBER, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	return CTS_SUCCESS;
}

API int contacts_svc_get_list(cts_get_list_op op_code, CTSiter **iter)
{
	int ret;
	CTSiter *result;

	retv_if(NULL == iter, CTS_ERR_ARG_NULL);

	result = calloc(1, sizeof(CTSiter));
	retvm_if(NULL == result, CTS_ERR_OUT_OF_MEMORY, "calloc() Failed");

	ret = cts_get_list(op_code, result);
	if (ret) {
		ERR("cts_get_list() Failed(%d)", ret);
		free(result);
		return ret;
	}

	*iter = (CTSiter *)result;
	return CTS_SUCCESS;
}

static inline int cts_get_list_with_str(cts_get_list_str_op op_code,
		const char *search_value, CTSiter *iter)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	const char *display;
	char remake_val[CTS_SQL_MIN_LEN];

	CTS_START_TIME_CHECK;

	retv_if(NULL == iter, CTS_ERR_ARG_NULL);

	iter->i_type = CTS_ITER_NONE;
	iter->stmt = NULL;

	retvm_if(NULL == search_value && CTS_LIST_PLOGS_OF_NUMBER != op_code,
			CTS_ERR_ARG_NULL, "The search_value is NULL");

	switch ((int)op_code)
	{
	case CTS_LIST_PLOGS_OF_NUMBER:
		iter->i_type = CTS_ITER_PLOGS_OF_NUMBER;
		if (search_value && *search_value) {
			snprintf(query, sizeof(query),
					"SELECT A.id, A.log_type, A.log_time, A.data1, A.data2, MIN(B.contact_id) "
					"FROM %s A LEFT JOIN %s B ON A.normal_num = B.data3 AND B.datatype = %d AND "
					"(A.related_id = B.contact_id OR A.related_id IS NULL OR "
					"NOT EXISTS (SELECT id FROM %s "
					"WHERE datatype = %d AND contact_id = A.related_id AND data3 = ?)) "
					"WHERE A.number = ? "
					"GROUP BY A.id "
					"ORDER BY A.log_time DESC",
					CTS_TABLE_PHONELOGS, CTS_TABLE_DATA, CTS_DATA_NUMBER, CTS_TABLE_DATA, CTS_DATA_NUMBER);
		}
		else {
			snprintf(query, sizeof(query),
					"SELECT id, log_type, log_time, data1, data2, NULL "
					"FROM %s WHERE number ISNULL ORDER BY id DESC", CTS_TABLE_PHONELOGS);
		}
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		if (search_value) {
			const char *normal_num;
			ret = cts_clean_number(search_value, remake_val, sizeof(remake_val));
			retvm_if(ret <= 0, CTS_ERR_ARG_INVALID, "Number(%s) is invalid", search_value);

			normal_num = cts_normalize_number(remake_val);
			cts_stmt_bind_copy_text(stmt, 1, normal_num, strlen(normal_num));
			cts_stmt_bind_copy_text(stmt, 2, remake_val, strlen(remake_val));
		}
		iter->stmt = stmt;
		break;
	case CTS_LIST_CONTACTS_WITH_NAME:
		retvm_if(CTS_SQL_MIN_LEN <= strlen(search_value), CTS_ERR_ARG_INVALID,
				"search_value is too long");
		iter->i_type = CTS_ITER_CONTACTS_WITH_NAME;
		memset(remake_val, 0x00, sizeof(remake_val));

		ret = cts_normalize_str(search_value, remake_val, CTS_SQL_MIN_LEN);
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_str() Failed(%d)", ret);

		if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
		else
			display = CTS_SCHEMA_DATA_NAME_LOOKUP;

		snprintf(query, sizeof(query),
				"SELECT A.contact_id, data1, data2, data3, data5, B.addrbook_id, B.image0 "
				"FROM %s A, %s B ON A.contact_id = B.contact_id "
				"WHERE datatype = %d AND %s LIKE ('%%' || ? || '%%') "
				"ORDER BY data1, %s",
				CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME, display, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		cts_stmt_bind_copy_text(stmt, 1, remake_val, strlen(remake_val));
		iter->stmt = stmt;
		break;
	case CTS_LIST_NUMBERINFOS_WITH_NAME:
		retvm_if(CTS_SQL_MIN_LEN <= strlen(search_value), CTS_ERR_ARG_INVALID,
				"search_value is too long");
		iter->i_type = CTS_ITER_NUMBERINFOS;
		memset(remake_val, 0x00, sizeof(remake_val));

		ret = cts_normalize_str(search_value, remake_val, CTS_SQL_MIN_LEN);
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_str() Failed(%d)", ret);

		if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
		else
			display = CTS_SCHEMA_DATA_NAME_LOOKUP;

		snprintf(query, sizeof(query),
				"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
				"FROM %s A, %s B, %s C "
				"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
				"WHERE A.datatype = %d AND B.datatype = %d AND A.%s LIKE ('%%' || ? || '%%') "
				"ORDER BY A.data1, A.%s",
				CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
				CTS_DATA_NAME, CTS_DATA_NUMBER, display, CTS_SCHEMA_DATA_NAME_SORTING_KEY);

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		cts_stmt_bind_copy_text(stmt, 1, remake_val, strlen(remake_val));
		iter->stmt = stmt;
		break;
	case CTS_LIST_NUMBERINFOS_WITH_NUM:
		iter->i_type = CTS_ITER_NUMBERINFOS;

		snprintf(query, sizeof(query),
				"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
				"FROM %s A, %s B, %s C "
				"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
				"WHERE A.datatype = %d AND B.datatype = %d AND B.data2 LIKE ('%%' || ? || '%%') "
				"ORDER BY A.data1, A.%s",
				CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
				CTS_DATA_NAME, CTS_DATA_NUMBER, CTS_SCHEMA_DATA_NAME_SORTING_KEY);

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		cts_stmt_bind_copy_text(stmt, 1, search_value, strlen(search_value));
		iter->stmt = stmt;
		break;
	case CTS_LIST_EMAILINFOS_WITH_EMAIL:
		iter->i_type = CTS_ITER_EMAILINFOS_WITH_EMAIL;
		snprintf(query, sizeof(query),
				"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
				"FROM %s A, %s B, %s C "
				"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
				"WHERE A.datatype = %d AND B.datatype = %d AND B.data2 LIKE ('%%' || ? || '%%') "
				"ORDER BY A.data1, A.%s",
				CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
				CTS_DATA_NAME, CTS_DATA_EMAIL, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		cts_stmt_bind_copy_text(stmt, 1, search_value, strlen(search_value));
		iter->stmt = stmt;
		break;
	case 10000: /* It is not supported. use only inhouse phone and message application */
		retvm_if(CTS_SQL_MIN_LEN - 50 < strlen(search_value),
				CTS_ERR_ARG_INVALID, "search_value is too long");
		iter->i_type = CTS_ITER_PLOGNUMBERS_WITH_NUM;
		snprintf(query, sizeof(query),
				"SELECT number FROM %s WHERE number LIKE '%%%s%%' GROUP BY number",
				CTS_TABLE_PHONELOGS, search_value);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
		return CTS_ERR_ARG_INVALID;
	}
	CTS_START_TIME_CHECK;

	return CTS_SUCCESS;
}

API int contacts_svc_get_list_with_str(cts_get_list_str_op op_code,
		const char *search_value, CTSiter **iter)
{
	int ret;
	CTSiter *result;

	retv_if(NULL == iter, CTS_ERR_ARG_NULL);

	result = calloc(1, sizeof(CTSiter));
	retvm_if(NULL == result, CTS_ERR_OUT_OF_MEMORY, "calloc() Failed");

	ret = cts_get_list_with_str(op_code, search_value, result);
	if (ret) {
		ERR("cts_get_list_with_str() Failed(%d)", ret);
		free(result);
		return ret;
	}

	*iter = (CTSiter *)result;
	return CTS_SUCCESS;
}

static inline int cts_get_list_with_int(cts_get_list_int_op op_code,
		unsigned int search_value, CTSiter *iter)
{
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	const char *display;

	retv_if(NULL == iter, CTS_ERR_ARG_NULL);
	iter->i_type = CTS_ITER_NONE;
	iter->stmt = NULL;

	if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
		display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
	else
		display = CTS_SCHEMA_DATA_NAME_LOOKUP;

	switch (op_code)
	{
	case CTS_LIST_MEMBERS_OF_GROUP_ID:
		iter->i_type = CTS_ITER_CONTACTS;
		snprintf(query, sizeof(query),
				"SELECT A.contact_id, data1, data2, data3, data5, B.addrbook_id, B.image0, %s "
				"FROM %s A, %s B WHERE datatype = %d AND A.contact_id IN "
				"(SELECT contact_id FROM %s WHERE group_id = %d) "
				"AND A.contact_id = B.contact_id "
				"ORDER BY data1, %s",
				display, CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME,
				CTS_TABLE_GROUPING_INFO, search_value, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_NO_GROUP_MEMBERS_OF_ADDRESSBOOK_ID:
		iter->i_type = CTS_ITER_CONTACTS;
		snprintf(query, sizeof(query),
				"SELECT A.contact_id, data1, data2, data3, data5, A.addrbook_id, A.image0, %s "
				"FROM %s A, %s B ON A.contact_id = B.contact_id "
				"WHERE A.addrbook_id = %d AND NOT EXISTS "
				"(SELECT contact_id FROM %s WHERE contact_id=A.contact_id LIMIT 1) "
				"AND datatype = %d "
				"ORDER BY data1, %s",
				display, CTS_TABLE_CONTACTS, CTS_TABLE_DATA,
				search_value, CTS_TABLE_GROUPING_INFO, CTS_DATA_NAME,
				CTS_SCHEMA_DATA_NAME_SORTING_KEY);

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_MEMBERS_OF_ADDRESSBOOK_ID:
		iter->i_type = CTS_ITER_CONTACTS;
		snprintf(query, sizeof(query),
				"SELECT A.contact_id, data1, data2, data3, data5, B.addrbook_id, B.image0, %s "
				"FROM %s A, %s B ON A.contact_id = B.contact_id "
				"WHERE datatype = %d AND B.addrbook_id = %d "
				"ORDER BY data1, %s",
				display, CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME, search_value,
				CTS_SCHEMA_DATA_NAME_SORTING_KEY);

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_GROUPS_OF_ADDRESSBOOK_ID:
		iter->i_type = CTS_ITER_GROUPS;
		snprintf(query, sizeof(query),
				"SELECT group_id, %d, group_name "
				"FROM %s WHERE addrbook_id = %d "
				"ORDER BY group_name COLLATE NOCASE",
				search_value, CTS_TABLE_GROUPS, search_value);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	case CTS_LIST_ADDRESSBOOKS_OF_ACCOUNT_ID:
		iter->i_type = CTS_ITER_ADDRESSBOOKS;
		snprintf(query, sizeof(query),
				"SELECT addrbook_id, addrbook_name, acc_id, acc_type, mode "
				"FROM %s WHERE acc_id = %d "
				"ORDER BY acc_id, addrbook_id",
				CTS_TABLE_ADDRESSBOOKS, search_value);
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		iter->stmt = stmt;
		break;
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	return CTS_SUCCESS;
}

API int contacts_svc_get_list_with_int(cts_get_list_int_op op_code,
		unsigned int search_value, CTSiter **iter)
{
	int ret;
	CTSiter *result;

	retv_if(NULL == iter, CTS_ERR_ARG_NULL);

	result = calloc(1, sizeof(CTSiter));
	retvm_if(NULL == result, CTS_ERR_OUT_OF_MEMORY, "calloc() Failed");

	ret = cts_get_list_with_int(op_code, search_value, result);
	if (ret) {
		ERR("cts_get_list_with_int() Failed(%d)", ret);
		free(result);
		return ret;
	}

	*iter = (CTSiter *)result;
	return CTS_SUCCESS;
}

static inline int cts_get_updated_contacts(int addressbook_id, int version,
		CTSiter *iter)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	updated_contact *result;

	retv_if(NULL == iter, CTS_ERR_ARG_NULL);
	retv_if(NULL == iter->info, CTS_ERR_ARG_INVALID);

	iter->i_type = CTS_ITER_UPDATED_CONTACTS_AFTER_VER;
	snprintf(query, sizeof(query),
			"SELECT %d, contact_id, changed_ver, created_ver FROM %s "
			"WHERE changed_ver > %d AND addrbook_id = %d "
			"UNION "
			"SELECT %d, contact_id, deleted_ver, -1 FROM %s "
			"WHERE deleted_ver > %d AND addrbook_id = %d",
			CTS_OPERATION_UPDATED, CTS_TABLE_CONTACTS, version, addressbook_id,
			CTS_OPERATION_DELETED, CTS_TABLE_DELETEDS, version, addressbook_id);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		warn_if(CTS_SUCCESS != ret, "cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	iter->info->head = result = cts_updated_contact_add_mempool();
	do {
		result->type = cts_stmt_get_int(stmt, 0);
		result->id = cts_stmt_get_int(stmt, 1);
		result->ver = cts_stmt_get_int(stmt, 2);
		if (cts_stmt_get_int(stmt, 3) == result->ver || version < cts_stmt_get_int(stmt, 3))
			result->type = CTS_OPERATION_INSERTED;
		if (NULL == result->next)
			result->next = cts_updated_contact_add_mempool();
		result = result->next;
	}while(CTS_TRUE == cts_stmt_step(stmt));

	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}

API int contacts_svc_get_updated_contacts(int addressbook_id,
		int version, CTSiter **iter)
{
	int ret;
	CTSiter *result;

	retv_if(NULL == iter, CTS_ERR_ARG_NULL);
	retvm_if(version < 0, CTS_ERR_ARG_INVALID, "The version(%d) is invalid", version);

	result = calloc(1, sizeof(CTSiter));
	retvm_if(NULL == result, CTS_ERR_OUT_OF_MEMORY, "calloc() Failed");

	result->info = calloc(1, sizeof(updated_contact_info));
	if (NULL == result->info) {
		ERR("calloc() Failed");
		free(result);
		return CTS_ERR_OUT_OF_MEMORY;
	}

	ret = cts_get_updated_contacts(addressbook_id, version, result);
	if (ret) {
		ERR("cts_get_updated_contacts() Failed(%d)", ret);
		free(result->info);
		free(result);
		return ret;
	}

	*iter = (CTSiter *)result;
	return CTS_SUCCESS;
}

static inline void cts_foreach_run(CTSiter *iter, cts_foreach_fn cb, void *data)
{
	while (CTS_SUCCESS == contacts_svc_iter_next(iter)) {
		int ret;
		CTSvalue *value;
		value = contacts_svc_iter_get_info(iter);

		ret = cb(value, data);
		if (CTS_SUCCESS != ret) {
			ERR("cts_foreach_fn(%p) Failed(%d)", cb, ret);
			contacts_svc_value_free(value);
			break;
		}

		contacts_svc_value_free(value);
	}
	cts_stmt_finalize(iter->stmt);
}

API int contacts_svc_list_foreach(cts_get_list_op op_code,
		cts_foreach_fn cb, void *user_data)
{
	int ret;
	CTSiter iter = {0};

	ret = cts_get_list(op_code, &iter);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_get_list() Failed(%d)", ret);

	cts_foreach_run(&iter, cb, user_data);

	return CTS_SUCCESS;
}

API int contacts_svc_list_with_int_foreach(cts_get_list_int_op op_code,
		unsigned int search_value, cts_foreach_fn cb, void *user_data)
{
	int ret;
	CTSiter iter = {0};

	ret = cts_get_list_with_int(op_code, search_value, &iter);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_get_list_with_int() Failed(%d)", ret);

	cts_foreach_run(&iter, cb, user_data);

	return CTS_SUCCESS;
}

API int contacts_svc_list_with_str_foreach(cts_get_list_str_op op_code,
		const char *search_value, cts_foreach_fn cb, void *user_data)
{
	int ret;
	CTSiter iter = {0};

	ret = cts_get_list_with_str(op_code, search_value, &iter);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_get_list_with_str() Failed(%d)", ret);

	cts_foreach_run(&iter, cb, user_data);

	return CTS_SUCCESS;
}

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

enum {
	CTS_FILTER_TYPE_NONE,
	CTS_FILTER_TYPE_INT,
	CTS_FILTER_TYPE_STR,
};

API CTSfilter* contacts_svc_list_str_filter_new(cts_str_filter_op list_type,
		const char *search_value, cts_filter_type first_type, ...)
{
	int ret;
	CTSfilter *ret_val;
	va_list args;

	retvm_if(NULL == search_value, NULL, "The parameter(search_value) is NULL");
	retvm_if(CTS_LIST_FILTER_NONE == first_type, NULL,
			"filter constraint is missing(use contacts_svc_get_list_with_str()");

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

	retvm_if(CTS_LIST_FILTER_NONE == first_type, NULL,
			"filter constraint is missing(use contacts_svc_get_list()");

	ret_val = calloc(1, sizeof(CTSfilter));
	ret_val = CTS_FILTER_TYPE_NONE;
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

static int cts_list_with_str_make_query(CTSfilter *filter, CTSiter *iter)
{
	int ret;
	cts_stmt stmt;
	const char *display;
	char query[CTS_SQL_MAX_LEN] = {0};
	char remake_val[CTS_SQL_MIN_LEN] = {0};

	retvm_if(NULL == filter->search_val,
			CTS_ERR_ARG_INVALID, "The parameter(filter) doesn't have search_val");

	if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
		display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
	else
		display = CTS_SCHEMA_DATA_NAME_LOOKUP;

	switch (filter->list_type) {
	case CTS_LIST_PLOGS_OF_NUMBER:
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
					CTS_TABLE_PHONELOGS, CTS_TABLE_DATA, CTS_DATA_NUMBER, CTS_TABLE_DATA, CTS_DATA_NUMBER);
		}
		else {
			ret = snprintf(query, sizeof(query),
					"SELECT id, log_type, log_time, data1, data2, NULL "
					"FROM %s WHERE number ISNULL ORDER BY id DESC", CTS_TABLE_PHONELOGS);
		}
		if (filter->limit_on) {
			ret += snprintf(query+ret, sizeof(query)-ret, " LIMIT %d ", filter->limit);
			if (filter->offset_on)
				ret += snprintf(query+ret, sizeof(query)-ret, ", %d", filter->offset);
		}

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		if (filter->search_val) {
			const char *normal_num;
			ret = cts_clean_number(filter->search_val, remake_val, sizeof(remake_val));
			retvm_if(ret <= 0, CTS_ERR_ARG_INVALID, "Number(%s) is invalid", filter->search_val);

			normal_num = cts_normalize_number(remake_val);
			cts_stmt_bind_copy_text(stmt, 1, normal_num, strlen(normal_num));
			cts_stmt_bind_copy_text(stmt, 2, remake_val, strlen(remake_val));
		}
		iter->stmt = stmt;
		break;
	case CTS_LIST_CONTACTS_WITH_NAME:
		retvm_if(CTS_SQL_MIN_LEN <= strlen(filter->search_val), CTS_ERR_ARG_INVALID,
				"search_value is too long");
		iter->i_type = CTS_ITER_CONTACTS_WITH_NAME;
		memset(remake_val, 0x00, sizeof(remake_val));

		ret = cts_normalize_str(filter->search_val, remake_val, CTS_SQL_MIN_LEN);
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_str() Failed(%d)", ret);

		if (filter->addrbook_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT A.contact_id, data1, data2, data3, data5, B.addrbook_id, B.image0 "
					"FROM %s A, %s B ON A.contact_id = B.contact_id "
					"WHERE datatype = %d AND B.addrbook_id = %d AND %s LIKE ('%%' || ? || '%%') "
					"ORDER BY data1, %s",
					CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME, filter->addrbook_id,
					display, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else if (filter->group_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT A.contact_id, data1, data2, data3, data5, B.addrbook_id, B.image0 "
					"FROM %s A, %s B ON A.contact_id = B.contact_id "
					"WHERE datatype = %d AND %s LIKE ('%%' || ? || '%%') "
					"AND contact_id IN (SELECT contact_id FROM %s WHERE group_id = %d) "
					"ORDER BY data1, %s",
					CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME, display,
					CTS_TABLE_GROUPING_INFO, filter->group_id,
					CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else {
			ret = snprintf(query, sizeof(query),
					"SELECT A.contact_id, data1, data2, data3, data5, B.addrbook_id, B.image0 "
					"FROM %s A, %s B ON A.contact_id = B.contact_id "
					"WHERE datatype = %d AND %s LIKE ('%%' || ? || '%%') "
					"ORDER BY data1, %s",
					CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_DATA_NAME, display,
					CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		}

		if (filter->limit_on) {
			ret += snprintf(query+ret, sizeof(query)-ret, " LIMIT %d ", filter->limit);
			if (filter->offset_on)
				ret += snprintf(query+ret, sizeof(query)-ret, ", %d", filter->offset);
		}
		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		cts_stmt_bind_copy_text(stmt, 1, remake_val, strlen(remake_val));
		break;
	case CTS_LIST_NUMBERINFOS_WITH_NAME:
		retvm_if(CTS_SQL_MIN_LEN <= strlen(filter->search_val), CTS_ERR_ARG_INVALID,
				"search_value is too long");
		iter->i_type = CTS_ITER_NUMBERINFOS;
		memset(remake_val, 0x00, sizeof(remake_val));

		ret = cts_normalize_str(filter->search_val, remake_val, CTS_SQL_MIN_LEN);
		retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_str() Failed(%d)", ret);

		if (filter->addrbook_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND A.datatype = %d "
					"AND C.addrbook_id = %d AND A.%s LIKE ('%%' || ? || '%%') "
					"ORDER BY A.data1, A.%s",
					CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER,
					filter->addrbook_id, display, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else if (filter->group_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d AND A.%s LIKE ('%%' || ? || '%%') "
					"AND A.contact_id IN "
					"(SELECT contact_id FROM %s WHERE group_id = %d) "
					"ORDER BY A.data1, A.%s",
					CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER, display,
					CTS_TABLE_GROUPING_INFO, filter->group_id, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else {
			ret = snprintf(query, sizeof(query),
					"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d AND A.%s LIKE ('%%' || ? || '%%') "
					"ORDER BY A.data1, A.%s",
					CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER,
					display, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		}
		if (filter->limit_on) {
			ret += snprintf(query+ret, sizeof(query)-ret, " LIMIT %d ", filter->limit);
			if (filter->offset_on)
				ret += snprintf(query+ret, sizeof(query)-ret, ", %d", filter->offset);
		}

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		cts_stmt_bind_copy_text(stmt, 1, remake_val, strlen(remake_val));
		break;
	case CTS_LIST_NUMBERINFOS_WITH_NUM:
		iter->i_type = CTS_ITER_NUMBERINFOS;

		if (filter->addrbook_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d "
					"AND C.addrbook_id = %d AND B.data2 LIKE ('%%' || ? || '%%') "
					"ORDER BY A.data1, A.%s",
					CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER,
					filter->addrbook_id, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else if (filter->group_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d AND B.data2 LIKE ('%%' || ? || '%%') "
					"AND A.contact_id IN (SELECT contact_id FROM %s WHERE group_id = %d) "
					"ORDER BY A.data1, A.%s",
					CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER,
					CTS_TABLE_GROUPING_INFO, filter->group_id, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else {
			ret = snprintf(query, sizeof(query),
					"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = B.contact_id AND A.contact_id = C.conatct_id "
					"WHERE B.data2 LIKE ('%%' || ? || '%%') "
					"AND A.datatype = %d AND B.datatype = %d "
					"ORDER BY A.data1, A.%s",
					CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		}
		if (filter->limit_on) {
			ret += snprintf(query+ret, sizeof(query)-ret, " LIMIT %d ", filter->limit);
			if (filter->offset_on)
				ret += snprintf(query+ret, sizeof(query)-ret, ", %d", filter->offset);
		}

		stmt = cts_query_prepare(query);
		retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");
		cts_stmt_bind_copy_text(stmt, 1, filter->search_val, strlen(filter->search_val));
		break;
	case CTS_LIST_EMAILINFOS_WITH_EMAIL:
		iter->i_type = CTS_ITER_EMAILINFOS_WITH_EMAIL;

		if (filter->addrbook_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d AND C.addrbook_id = %d "
					"AND B.data2 LIKE ('%%' || ? || '%%') "
					"ORDER BY A.data1, A.%s",
					CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER, filter->addrbook_id,
					CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else if (filter->group_on) {
			ret = snprintf(query, sizeof(query),
					"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d AND "
					"B.data2 LIKE ('%%' || ? || '%%') AND A.contact_id IN "
					"(SELECT contact_id FROM %s WHERE group_id = %d) "
					"ORDER BY A.data1, A.%s",
					CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_NUMBER,
					CTS_TABLE_GROUPING_INFO, filter->group_id,
					CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		} else {
			ret = snprintf(query, sizeof(query),
					"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
					"FROM %s A, %s B, %s C "
					"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
					"WHERE A.datatype = %d AND B.datatype = %d AND B.data2 LIKE ('%%' || ? || '%%') "
					"ORDER BY A.data1, A.%s",
					CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
					CTS_DATA_NAME, CTS_DATA_EMAIL,
					CTS_SCHEMA_DATA_NAME_SORTING_KEY);
		}
		if (filter->limit_on) {
			ret += snprintf(query+ret, sizeof(query)-ret, " LIMIT %d ", filter->limit);
			if (filter->offset_on)
				ret += snprintf(query+ret, sizeof(query)-ret, ", %d", filter->offset);
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

#ifdef CTS_DEPRECATED
API int contacts_svc_get_list_with_filter(CTSfilter *filter, CTSiter **iter)
{
	int ret;
	CTSiter *result;

	retv_if(NULL == filter, CTS_ERR_ARG_NULL);
	retv_if(NULL == iter, CTS_ERR_ARG_NULL);

	if (CTS_FILTER_TYPE_STR == filter->type) {
		result = calloc(1, sizeof(CTSiter));
		retvm_if(NULL == result, CTS_ERR_OUT_OF_MEMORY, "calloc() Failed");

		ret = cts_list_with_str_make_query(filter, result);
		if (ret) {
			ERR("cts_list_with_str_make_query() Failed(%d)", ret);
			free(result);
			return ret;
		}
	} else {
		ERR("Invalid CTSfilter(type = %d)", filter->type);
		return CTS_ERR_ARG_INVALID;
	}

	return CTS_SUCCESS;
}
#endif

API int contacts_svc_list_with_filter_foreach(CTSfilter *filter,
		cts_foreach_fn cb, void *user_data)
{
	int ret;
	CTSiter iter = {0};

	if (CTS_FILTER_TYPE_STR == filter->type) {
		ret = cts_list_with_str_make_query(filter, &iter);
		retvm_if(CTS_SUCCESS != ret, ret, "contacts_svc_get_list_with_filter() Failed(%d)", ret);
	} else {
		ERR("Invalid CTSfilter(type = %d)", filter->type);
		return CTS_ERR_ARG_INVALID;
	}

	cts_foreach_run(&iter, cb, user_data);

	return CTS_SUCCESS;
}

static inline bool cts_is_number(const char *str)
{
	int i;

	for (i=0;i<strlen(str);i++)
	{
		switch (str[i])
		{
		case '0' ... '9':
		case 'p':
		case 'w':
		case '+':
			break;
		default:
			return false;
		}
	}
	return true;
}

API int contacts_svc_smartsearch_excl(const char *search_str, int limit, int offset,
		cts_foreach_fn cb, void *user_data)
{
	int ret, len;
	CTSiter iter = {0};
	const char *display;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN];
	char remake_name[CTS_SQL_MIN_LEN];

	retv_if(NULL == search_str, CTS_ERR_ARG_NULL);
	retvm_if(CTS_SQL_MIN_LEN <= strlen(search_str), CTS_ERR_ARG_INVALID,
			"search_str is too long");

	iter.i_type = CTS_ITER_NUMBERINFOS;

	if (CTS_ORDER_NAME_LASTFIRST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
		display = CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP;
	else
		display = CTS_SCHEMA_DATA_NAME_LOOKUP;

	if (cts_is_number(search_str)) {
		len = snprintf(query, sizeof(query),
				"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, B.data2, C.image0 "
				"FROM %s A, %s B, %s C "
				"ON A.contact_id = B.contact_id AND A.contact_id = C.contact_id "
				"WHERE A.datatype = %d AND B.datatype = %d "
				"AND (B.data2 LIKE '%%%s%%' OR A.%s LIKE ('%%' || ? || '%%')) "
				"ORDER BY A.data1, A.%s",
				CTS_TABLE_DATA, CTS_TABLE_DATA, CTS_TABLE_CONTACTS,
				CTS_DATA_NAME, CTS_DATA_NUMBER,
				search_str, display, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	}
	else {
		len = snprintf(query, sizeof(query),
				"SELECT A.contact_id, A.data1, A.data2, A.data3, A.data5, D.data2, D.image0 "
				"FROM %s A "
				"LEFT JOIN (%s B, %s C ON B.default_num = C.id AND C.datatype = %d ) D "
				"ON A.contact_id = D.contact_id "
				"WHERE A.datatype = %d AND A.%s LIKE ('%%' || ? || '%%') "
				"ORDER BY A.data1, A.%s",
				CTS_TABLE_DATA, CTS_TABLE_CONTACTS, CTS_TABLE_DATA,
				CTS_DATA_NUMBER, CTS_DATA_NAME, display, CTS_SCHEMA_DATA_NAME_SORTING_KEY);
	}

	if (limit)
		snprintf(query+len, sizeof(query)-len, " LIMIT %d OFFSET %d", limit, offset);

	ret = cts_normalize_str(search_str, remake_name, sizeof(remake_name));
	retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_str() Failed(%d)", ret);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	cts_stmt_bind_copy_text(stmt, 1, remake_name, strlen(remake_name));
	iter.stmt = stmt;

	cts_foreach_run(&iter, cb, user_data);

	return CTS_SUCCESS;
}

static inline int cts_group_get_relation_changes(int addressbook_id, int version,
		CTSiter *iter)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	updated_contact *result;

	retv_if(NULL == iter, CTS_ERR_ARG_NULL);
	retv_if(NULL == iter->info, CTS_ERR_ARG_INVALID);

	iter->i_type = CTS_ITER_UPDATED_CONTACTS_AFTER_VER;
	snprintf(query, sizeof(query),
			"SELECT group_id, type, ver FROM %s, %s USING (group_id) "
			"WHERE ver > %d AND addrbook_id = %d ",
			"group_relations_log", CTS_TABLE_GROUPS,
			version, addressbook_id);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret)
	{
		warn_if(CTS_SUCCESS != ret, "cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	iter->info->head = result = cts_updated_contact_add_mempool();
	do {
		result->id = cts_stmt_get_int(stmt, 0);
		result->type = cts_stmt_get_int(stmt, 1);
		result->ver = cts_stmt_get_int(stmt, 2);

		if (NULL == result->next)
			result->next = cts_updated_contact_add_mempool();
		result = result->next;
	}while(CTS_TRUE == cts_stmt_step(stmt));

	cts_stmt_finalize(stmt);

	return CTS_SUCCESS;
}

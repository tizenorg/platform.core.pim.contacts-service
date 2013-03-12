/*
 * Contact Service
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

#include <glib.h>
#include <stdlib.h>
#include <unistd.h>	//sleep

#include "contacts.h"
#include "internal.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_server_bg.h"
#include "ctsvc_utils.h"

#define CTSVC_SERVER_BG_DELETE_COUNT 50
#define CTSVC_SERVER_BG_DELETE_STEP_TIME 1
#define CTSVC_SERVER_BG_DELETE_THREAD "ctsvc_server_bg_delete"

typedef enum
{
	STEP_1, // get contact_ids
	STEP_2, // delete data
	STEP_3,	// delete activity
	STEP_4,	// delete search_index, contact(image by trigger)
} __ctsvc_delete_step_e;

typedef struct {
	GSList *contact_ids;
	int current_contact_id;
	__ctsvc_delete_step_e step;
} __ctsvc_delete_data_s;

GThread *__ctsvc_server_bg_delete_thread = NULL;
GCond __ctsvc_server_bg_delete_cond;
GMutex __ctsvc_server_bg_delete_mutex;

static int __ctsvc_server_bg_contact_delete_step1(__ctsvc_delete_data_s* data)
{
	char query[CTS_SQL_MIN_LEN] = {0,};
	int ret;
	cts_stmt stmt = NULL;
	int count = 0;
	GSList *cursor;

	if (data->contact_ids == NULL){
		// get event_list
		snprintf(query, sizeof(query), "SELECT contact_id FROM "CTS_TABLE_CONTACTS" WHERE deleted = 1");
		stmt = cts_query_prepare(query);
		if (NULL == stmt) {
			ERR("cts_query_prepare() Failed");
			return CONTACTS_ERROR_DB;
		}

		while(1 == (ret = cts_stmt_step(stmt))) {
			int id = 0;
			id = ctsvc_stmt_get_int(stmt, 0);
			data->contact_ids = g_slist_append(data->contact_ids, GINT_TO_POINTER(id));
		}
		cts_stmt_finalize(stmt);
	}

	count = g_slist_length(data->contact_ids);
	if (count <= 0)
		return CONTACTS_ERROR_DB;

	cursor = g_slist_nth(data->contact_ids, 0);
	if (cursor) {
		data->current_contact_id = GPOINTER_TO_INT(cursor->data);
		data->contact_ids = g_slist_remove(data->contact_ids, GINT_TO_POINTER(data->current_contact_id));

		return CONTACTS_ERROR_NONE;
	}
	else {
		return CONTACTS_ERROR_NO_DATA;
	}
}

// remove data
static int __ctsvc_server_bg_contact_delete_step2(__ctsvc_delete_data_s* data)
{
    SERVER_FN_CALL;
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};
	GSList *list = NULL;
	cts_stmt stmt = NULL;
	int count = 0;
	GSList *cursor;

	// get event_list
	snprintf(query, sizeof(query),
		"SELECT id FROM "CTS_TABLE_DATA" WHERE contact_id = %d LIMIT %d",
		data->current_contact_id, CTSVC_SERVER_BG_DELETE_COUNT);

	stmt = cts_query_prepare(query);
		if (NULL == stmt) {
		ERR("cts_query_prepare() Failed");
		return CONTACTS_ERROR_DB;
	}

	while(1 == (ret = cts_stmt_step(stmt))) {
		int id = 0;
		id = ctsvc_stmt_get_int(stmt, 0);
		list = g_slist_append(list, GINT_TO_POINTER(id));
	}
	cts_stmt_finalize(stmt);

	count = g_slist_length(list);
	if (count <= 0)
		return CONTACTS_ERROR_NO_DATA;

	ret = ctsvc_begin_trans();
	h_retvm_if(CONTACTS_ERROR_NONE != ret, CONTACTS_ERROR_DB, "DB failed" );

	cursor = g_slist_nth(list, 0);
	while(cursor) {
		int id = GPOINTER_TO_INT(cursor->data);
		snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_DATA" WHERE id = %d", id);

		ret = ctsvc_query_exec(query);
		SERVER_DBG("%s",query);
		if (ret != CONTACTS_ERROR_NONE) {
			ERR("DB failed");
			ctsvc_end_trans(false);
			g_slist_free(list);
			return CONTACTS_ERROR_DB;
		}
		cursor = g_slist_next(cursor);
	}
	g_slist_free(list);
	ret = ctsvc_end_trans(true);

	return ret;
}

// remove activities
static int __ctsvc_server_bg_contact_delete_step3(__ctsvc_delete_data_s* data)
{
    SERVER_FN_CALL;

	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};
	GSList *list = NULL;
	cts_stmt stmt = NULL;
	int count = 0;
	GSList *cursor;

	// get event_list
	snprintf(query, sizeof(query),
		"SELECT id FROM "CTS_TABLE_ACTIVITIES" WHERE contact_id = %d LIMIT %d",
		data->current_contact_id, CTSVC_SERVER_BG_DELETE_COUNT);

	stmt = cts_query_prepare(query);
		if (NULL == stmt) {
		ERR("cts_query_prepare() Failed");
		return CONTACTS_ERROR_DB;
	}

	while(1 == (ret = cts_stmt_step(stmt))) {
		int id = 0;
		id = ctsvc_stmt_get_int(stmt, 0);
		list = g_slist_append(list, GINT_TO_POINTER(id));
	}
	cts_stmt_finalize(stmt);

	count = g_slist_length(list);
	if (count <= 0)
		return CONTACTS_ERROR_NO_DATA;

	ret = ctsvc_begin_trans();
	h_retvm_if(CONTACTS_ERROR_NONE != ret, CONTACTS_ERROR_DB, "DB failed" );

	cursor = g_slist_nth(list, 0);
	while(cursor) {
		int id = GPOINTER_TO_INT(cursor->data);
		snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_ACTIVITIES" WHERE id = %d", id);

		ret = ctsvc_query_exec(query);
		SERVER_DBG("%s",query);
		if (ret != CONTACTS_ERROR_NONE) {
			ERR("DB failed");
			ctsvc_end_trans(false);
			g_slist_free(list);
			return CONTACTS_ERROR_DB;
		}
		cursor = g_slist_next(cursor);
	}
	g_slist_free(list);
	ret = ctsvc_end_trans(true);

	return ret;
}

static int __ctsvc_server_bg_contact_delete_step4(__ctsvc_delete_data_s* data)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	ret = ctsvc_begin_trans();
	h_retvm_if(CONTACTS_ERROR_NONE != ret, CONTACTS_ERROR_DB, "DB failed" );

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_SEARCH_INDEX" WHERE contact_id = %d",
					data->current_contact_id);
	ret = ctsvc_query_exec(query);
	SERVER_DBG("%s",query);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("DB failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_CONTACTS" WHERE contact_id = %d",
					data->current_contact_id);
	ret = ctsvc_query_exec(query);
	SERVER_DBG("%s",query);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("DB failed");
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_DB;
	}

	ret = ctsvc_end_trans(true);
	return ret;
}

static bool __ctsvc_server_bg_contact_delete_step(int ret, __ctsvc_delete_data_s* data)
{
	if (ret != CONTACTS_ERROR_NONE && ret != CONTACTS_ERROR_NO_DATA) {
	if(data->contact_ids)
		g_slist_free(data->contact_ids);
		ERR("fail (%d)",ret);
		return false;
	}

	switch (data->step) {
	case STEP_1:
		if (ret == CONTACTS_ERROR_NO_DATA) {
			if(data->contact_ids)
				g_slist_free(data->contact_ids);
			ERR("step_1 no_data");
			return false;
		}
		data->step = STEP_2;
		break;
	case STEP_2:
		if (ret == CONTACTS_ERROR_NO_DATA)
			data->step = STEP_3;
		break;
	case STEP_3:
		if (ret == CONTACTS_ERROR_NO_DATA)
			data->step = STEP_4;
		break;
	case STEP_4:
		data->step = STEP_1;
		break;
	default:
		data->step = STEP_1;
		break;
	}

	return true;
}

static bool  __ctsvc_server_db_delete_run(__ctsvc_delete_data_s* data)
{
	SERVER_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;

	if(data == NULL) {
		ERR("data is NULL");
		return false;
	}

	switch (data->step) {
		case STEP_1:
			ret = __ctsvc_server_bg_contact_delete_step1(data);
			break;
		case STEP_2:
			ret = __ctsvc_server_bg_contact_delete_step2(data);
			break;
		case STEP_3:
			ret = __ctsvc_server_bg_contact_delete_step3(data);
			break;
		case STEP_4:
			ret = __ctsvc_server_bg_contact_delete_step4(data);
			break;
		default:
			ERR("invalid step");
			if(data->contact_ids)
			g_slist_free(data->contact_ids);
			return false;
	}

	return __ctsvc_server_bg_contact_delete_step(ret, data);
}

static gpointer __ctsvc_server_bg_delete(gpointer user_data)
{
	SERVER_FN_CALL;
	int ret;
	__ctsvc_delete_data_s *callback_data = NULL;

	while(1) {
		callback_data = calloc(1,sizeof(__ctsvc_delete_data_s));
		if (callback_data == NULL) {
			ERR("calloc fail");
			continue;
		}
		callback_data->step = STEP_1;

		ret = contacts_connect2();
		if (CONTACTS_ERROR_NONE != ret) {
			SERVER_DBG("%d", ret);
			free(callback_data);
			continue;
		}

		while(1) {
			sleep(CTSVC_SERVER_BG_DELETE_STEP_TIME); // sleep 1 sec.
			if (__ctsvc_server_db_delete_run(callback_data) == false) {
				SERVER_DBG("end");
				free(callback_data);
				break;
			}
		}
		ret = contacts_disconnect2();
		if (CONTACTS_ERROR_NONE != ret)
			SERVER_DBG("%d", ret);

		g_mutex_lock(&__ctsvc_server_bg_delete_mutex);
		SERVER_DBG("wait");
		g_cond_wait(&__ctsvc_server_bg_delete_cond, &__ctsvc_server_bg_delete_mutex);
		g_mutex_unlock(&__ctsvc_server_bg_delete_mutex);
	}

    return NULL;
}

void ctsvc_server_bg_delete_start()
{
	SERVER_FN_CALL;

	if (__ctsvc_server_bg_delete_thread == NULL) {
		g_mutex_init(&__ctsvc_server_bg_delete_mutex);
		g_cond_init(&__ctsvc_server_bg_delete_cond);
		__ctsvc_server_bg_delete_thread = g_thread_new(CTSVC_SERVER_BG_DELETE_THREAD,
				__ctsvc_server_bg_delete, NULL);
	}

	// don't use mutex.
	g_cond_signal(&__ctsvc_server_bg_delete_cond);
}

void __ctsvc_server_addressbook_deleted_cb(const char *view_uri, void *data)
{
	ctsvc_server_bg_delete_start();
}

int ctsvc_server_bg_add_cb()
{
	return contacts_db_add_changed_cb(_contacts_address_book._uri, __ctsvc_server_addressbook_deleted_cb, NULL);
}

int ctsvc_server_bg_remove_cb()
{
	return contacts_db_remove_changed_cb(_contacts_address_book._uri, __ctsvc_server_addressbook_deleted_cb, NULL);
}


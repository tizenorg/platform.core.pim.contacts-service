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
#include <unistd.h>

#include <account.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_inotify.h"
#include "ctsvc_handle.h"
#include "ctsvc_db_schema.h"
#include "ctsvc_db_sqlite.h"
#include "ctsvc_server_service.h"
#include "ctsvc_server_bg.h"
#include "ctsvc_server_utils.h"
#include "ctsvc_db_utils.h"
#include "ctsvc_db_plugin_addressbook_helper.h"
#include "ctsvc_db_access_control.h"

#define CTSVC_SERVER_BG_DELETE_COUNT 50
#define CTSVC_SERVER_BG_DELETE_STEP_TIME 1
#define CTSVC_SERVER_BG_DELETE_THREAD "ctsvc_server_bg_delete"

#define CTSVC_SERVER_BG_BASE_CPU_USAGE 10  /* Delete contacts when cpu usage is under the value */

typedef enum
{
	STEP_1, /* get contact_ids */
	STEP_2, /* delete data */
	STEP_3, /* delete activity */
	STEP_4, /* delete search_index, contact(image by trigger) */
} __ctsvc_delete_step_e;

typedef struct {
	GSList *contact_ids;
	int current_contact_id;
	__ctsvc_delete_step_e step;
} __ctsvc_delete_data_s;

GThread *__ctsvc_server_bg_delete_thread = NULL;
GCond __ctsvc_server_bg_delete_cond;
GMutex __ctsvc_server_bg_delete_mutex;

account_subscribe_h account = NULL;
contacts_h bg_contact;

static int __ctsvc_server_bg_contact_delete_step1(__ctsvc_delete_data_s* data)
{
	char query[CTS_SQL_MIN_LEN] = {0,};
	int ret;
	cts_stmt stmt = NULL;
	int count = 0;
	GSList *cursor;

	if (data->contact_ids == NULL) {
		/* get event_list */
		snprintf(query, sizeof(query), "SELECT contact_id FROM "CTS_TABLE_CONTACTS" WHERE deleted = 1");
		ret = ctsvc_query_prepare(query, &stmt);
		RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

		while (1 == (ret = ctsvc_stmt_step(stmt))) {
			int id = 0;
			id = ctsvc_stmt_get_int(stmt, 0);
			data->contact_ids = g_slist_append(data->contact_ids, GINT_TO_POINTER(id));
		}
		ctsvc_stmt_finalize(stmt);
		if (ret < CONTACTS_ERROR_NONE)
			return ret;
	}

	count = g_slist_length(data->contact_ids);
	if (count <= 0)
		return CONTACTS_ERROR_NO_DATA;

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

/* remove data */
static int __ctsvc_server_bg_contact_delete_step2(__ctsvc_delete_data_s* data)
{
    CTS_FN_CALL;
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};
	GSList *list = NULL;
	cts_stmt stmt = NULL;
	int count = 0;
	GSList *cursor;

	/* get event_list */
	snprintf(query, sizeof(query),
		"SELECT id FROM "CTS_TABLE_DATA" WHERE contact_id = %d AND is_my_profile = 0 LIMIT %d",
		data->current_contact_id, CTSVC_SERVER_BG_DELETE_COUNT);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	while (1 == (ret = ctsvc_stmt_step(stmt))) {
		int id = 0;
		id = ctsvc_stmt_get_int(stmt, 0);
		list = g_slist_append(list, GINT_TO_POINTER(id));
	}
	ctsvc_stmt_finalize(stmt);

	count = g_slist_length(list);
	if (count <= 0)
		return CONTACTS_ERROR_NO_DATA;

	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, CONTACTS_ERROR_DB, "DB Fail");

	cursor = g_slist_nth(list, 0);
	while (cursor) {
		int id = GPOINTER_TO_INT(cursor->data);
		snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_DATA" WHERE id = %d", id);

		ret = ctsvc_query_exec(query);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("DB Fail");
			ctsvc_end_trans(false);
			g_slist_free(list);
			return ret;
		}
		cursor = g_slist_next(cursor);
	}
	g_slist_free(list);
	ret = ctsvc_end_trans(true);

	return ret;
}

/* remove activities */
static int __ctsvc_server_bg_contact_delete_step3(__ctsvc_delete_data_s* data)
{
    CTS_FN_CALL;

	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};
	GSList *list = NULL;
	cts_stmt stmt = NULL;
	int count = 0;
	GSList *cursor;

	/* get event_list */
	snprintf(query, sizeof(query),
		"SELECT id FROM "CTS_TABLE_ACTIVITIES" WHERE contact_id = %d LIMIT %d",
		data->current_contact_id, CTSVC_SERVER_BG_DELETE_COUNT);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "DB error : ctsvc_query_prepare() Fail(%d)", ret);

	while (1 == (ret = ctsvc_stmt_step(stmt))) {
		int id = 0;
		id = ctsvc_stmt_get_int(stmt, 0);
		list = g_slist_append(list, GINT_TO_POINTER(id));
	}
	ctsvc_stmt_finalize(stmt);

	count = g_slist_length(list);
	if (count <= 0)
		return CONTACTS_ERROR_NO_DATA;

	ret = ctsvc_begin_trans();
	RETVM_IF(CONTACTS_ERROR_NONE != ret, CONTACTS_ERROR_DB, "DB Fail");

	cursor = g_slist_nth(list, 0);
	while (cursor) {
		int id = GPOINTER_TO_INT(cursor->data);
		snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_ACTIVITIES" WHERE id = %d", id);

		ret = ctsvc_query_exec(query);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("DB Fail");
			ctsvc_end_trans(false);
			g_slist_free(list);
			return ret;
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
	RETVM_IF(CONTACTS_ERROR_NONE != ret, CONTACTS_ERROR_DB, "DB Fail");

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_SEARCH_INDEX" WHERE contact_id = %d",
					data->current_contact_id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB Fail");
		ctsvc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query), "DELETE FROM "CTS_TABLE_CONTACTS" WHERE contact_id = %d",
					data->current_contact_id);
	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB Fail");
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_end_trans(true);
	return ret;
}

static bool __ctsvc_server_bg_contact_delete_step(int ret, __ctsvc_delete_data_s* data)
{
	if (ret != CONTACTS_ERROR_NONE && ret != CONTACTS_ERROR_NO_DATA) {
		if (data->contact_ids)
			g_slist_free(data->contact_ids);
		CTS_ERR("fail (%d)",ret);
		return false;
	}

	switch (data->step) {
	case STEP_1:
		if (ret == CONTACTS_ERROR_NO_DATA) {
			if (data->contact_ids)
				g_slist_free(data->contact_ids);
			CTS_ERR("step_1 no_data");
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
	CTS_FN_CALL;
	int ret = CONTACTS_ERROR_NONE;

	if (data == NULL) {
		CTS_ERR("data is NULL");
		return false;
	}

	switch (data->step) {
		case STEP_1:
			/* get deleted contact id list */
			ret = __ctsvc_server_bg_contact_delete_step1(data);
			break;
		case STEP_2:
			/* delete data of current contact id (MAX CTSVC_SERVER_BG_DELETE_COUNT) */
			ret = __ctsvc_server_bg_contact_delete_step2(data);
			break;
		case STEP_3:
			/* delete activity of current contact id (MAX CTSVC_SERVER_BG_DELETE_COUNT each time) */
			ret = __ctsvc_server_bg_contact_delete_step3(data);
			break;
		case STEP_4:
			/* delete search index of current contact id */
			ret = __ctsvc_server_bg_contact_delete_step4(data);
			break;
		default:
			CTS_ERR("invalid step");
			if (data->contact_ids)
			g_slist_free(data->contact_ids);
			return false;
	}

	return __ctsvc_server_bg_contact_delete_step(ret, data);
}

typedef struct {
	unsigned long int cpu_work_time;
	unsigned long int cpu_total_time;
}process_stat;

static process_stat* __ctsvc_get_cpu_stat()
{
	int i;
	int ret;
	long unsigned int cpu_time[10];
	process_stat *result = NULL;

	FILE *fstat = fopen("/proc/stat", "r");
	if (fstat == NULL) {
		return NULL;
	}
	memset(cpu_time, 0x0, sizeof(cpu_time));
	ret = fscanf(fstat, "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
			&cpu_time[0], &cpu_time[1], &cpu_time[2], &cpu_time[3],
			&cpu_time[4], &cpu_time[5], &cpu_time[6], &cpu_time[7],
			&cpu_time[8], &cpu_time[9]);
	fclose(fstat);

	if (ret < 0) {
		return NULL;
	}

	result = calloc(1, sizeof(process_stat));
	if (NULL == result) {
		CTS_ERR("calloc() Fail");
		return NULL;
	}
	for (i=0; i < 10;i++) {
		if (i < 3)
			result->cpu_work_time += cpu_time[i];
		result->cpu_total_time += cpu_time[i];
	}

	return result;
}

static bool __ctsvc_cpu_is_busy()
{
	unsigned long int total_time_diff;
	unsigned long int work_time_diff;
	process_stat *result1 = NULL;
	process_stat *result2 = NULL;
	double cpu_usage;

	result1 = __ctsvc_get_cpu_stat();
	sleep(1);
	result2 = __ctsvc_get_cpu_stat();
	if (result1 == NULL || result2 == NULL) {
		free(result1);
		free(result2);
		return false;
	}

	total_time_diff = result2->cpu_total_time - result1->cpu_total_time;
	work_time_diff = result2->cpu_work_time - result1->cpu_work_time;

	free(result1);
	free(result2);

	cpu_usage = ((double)work_time_diff/(double)total_time_diff) * 100;
	CTS_INFO("cpu usage : %.2lf (%ld/%ld)", cpu_usage, work_time_diff, total_time_diff);
	if (CTSVC_SERVER_BG_BASE_CPU_USAGE < cpu_usage)
		return true;
	return false;
}

static gpointer __ctsvc_server_bg_delete(gpointer user_data)
{
	CTS_FN_CALL;
	int ret;
	__ctsvc_delete_data_s *callback_data = NULL;

	while (1) {
		callback_data = calloc(1,sizeof(__ctsvc_delete_data_s));
		if (callback_data == NULL) {
			CTS_ERR("calloc fail");
			continue;
		}
		callback_data->step = STEP_1;

		ret = ctsvc_connect();
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("contacts_connect() fail(%d)", ret);
			free(callback_data);
			continue;
		}
		ctsvc_set_client_access_info(NULL, NULL);

		ctsvc_server_stop_timeout();
		while (1) {
			if (__ctsvc_cpu_is_busy()) { /* sleep 1 sec in function */
				CTS_ERR("Now CPU is busy.. waiting");
				sleep(CTSVC_SERVER_BG_DELETE_STEP_TIME*59); /* sleep 60 sec(1 min) totally */
				continue;
			}
			if (__ctsvc_server_db_delete_run(callback_data) == false) {
				CTS_DBG("end");
				free(callback_data);
				break;
			}
		}

		ctsvc_unset_client_access_info();

		ret = ctsvc_disconnect();

		if (CONTACTS_ERROR_NONE != ret)
			CTS_ERR("contacts_disconnect Fail(%d)", ret);
		ctsvc_server_start_timeout();

		g_mutex_lock(&__ctsvc_server_bg_delete_mutex);
		CTS_DBG("wait");
		g_cond_wait(&__ctsvc_server_bg_delete_cond, &__ctsvc_server_bg_delete_mutex);
		g_mutex_unlock(&__ctsvc_server_bg_delete_mutex);
	}

	ctsvc_server_start_timeout();
	return NULL;
}

void ctsvc_server_bg_delete_start()
{
	CTS_FN_CALL;

	if (__ctsvc_server_bg_delete_thread == NULL) {
		g_mutex_init(&__ctsvc_server_bg_delete_mutex);
		g_cond_init(&__ctsvc_server_bg_delete_cond);
		__ctsvc_server_bg_delete_thread = g_thread_new(CTSVC_SERVER_BG_DELETE_THREAD,
				__ctsvc_server_bg_delete, NULL);
	}

	/* don't use mutex. */
	g_cond_signal(&__ctsvc_server_bg_delete_cond);
}

static void __ctsvc_server_addressbook_deleted_cb(const char *view_uri, void *data)
{
	/* access control update */
	ctsvc_reset_all_client_access_info();

	ctsvc_server_bg_delete_start();
}

static void __ctsvc_server_contact_deleted_cb(const char *view_uri, void *data)
{
	ctsvc_server_bg_delete_start();
}

static bool __ctsvc_server_account_delete_cb(const char* event_type, int account_id, void* user_data)
{
	CTS_FN_CALL;
	CTS_INFO("event_type : %s, account_id : %d", event_type, account_id);

	if (STRING_EQUAL == strcmp(event_type, ACCOUNT_NOTI_NAME_DELETE)) {
		ctsvc_server_stop_timeout();
		ctsvc_addressbook_delete(account_id);
	}
	ctsvc_server_start_timeout();
	return true;
}

void ctsvc_server_bg_add_cb()
{
	int ret;
	ctsvc_handle_create(&bg_contact);
	ret = ctsvc_inotify_subscribe(bg_contact, _contacts_address_book._uri, __ctsvc_server_addressbook_deleted_cb, NULL);
	CTS_DBG("call ctsvc_inotify_subscribe (_contacts_address_book)  : return (%d)", ret);

	ret = ctsvc_inotify_subscribe(bg_contact, _contacts_contact._uri, __ctsvc_server_contact_deleted_cb, NULL);
	CTS_DBG("call ctsvc_inotify_subscribe (_contacts_contact): return (%d)", ret);

	ret = account_subscribe_create(&account);
	if (ACCOUNT_ERROR_NONE == ret) {
		ret = account_subscribe_notification(account, __ctsvc_server_account_delete_cb, NULL);
		if (ACCOUNT_ERROR_NONE != ret) {
			CTS_ERR("account_subscribe_notification Fail (%d)", ret);
		}
	}
	else
		CTS_ERR("account_subscribe_create Fail (%d)", ret);
}

void ctsvc_server_bg_remove_cb()
{
	int ret;

	ret = ctsvc_inotify_unsubscribe(bg_contact, _contacts_address_book._uri, __ctsvc_server_addressbook_deleted_cb, NULL);
	CTS_ERR("call ctsvc_inotify_unsubscribe (_contacts_address_book): return (%d)", ret);
	ret = ctsvc_inotify_unsubscribe(bg_contact, _contacts_contact._uri, __ctsvc_server_contact_deleted_cb, NULL);
	CTS_ERR("call ctsvc_inotify_unsubscribe (_contacts_contact) : return (%d)", ret);

	if (account) {
		account_unsubscribe_notification(account);  /* unsubscirbe & destroy */
		account = NULL;
	}
}


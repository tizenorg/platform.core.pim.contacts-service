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
#include <stdio.h>
#include <glib.h>
#include <contacts-svc.h>

static void favorite_change_callback(void *data)
{
	printf("Favorite data of contacts service is changed\n");
}

static void plog_change_callback(void *data)
{
	printf("Phone log data of contacts service is changed\n");
}

static void contact_change_callback(void *data)
{
	int ret;
	static int latest_ver = 0;
	CTSiter *iter;

	printf("Contact data of contacts service is changed\n");

	ret = contacts_svc_get_updated_contacts(0, latest_ver, &iter);
	if (CTS_SUCCESS != ret) {
		printf("contacts_svc_get_updated_contacts() Failed(%d)", ret);
		return;
	}

	while (CTS_SUCCESS == contacts_svc_iter_next(iter)) {
		int index;
		CTSvalue *row_info = NULL;
		row_info = contacts_svc_iter_get_info(iter);

		index = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_ID_INT);
		printf("(%8d) is ", index);
		int type = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_TYPE_INT);
		int ver = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_VER_INT);
		if (CTS_OPERATION_INSERTED == type) {
			printf("Inserted at %d\n", ver);
			if (latest_ver < ver) latest_ver = ver;
		}
		else if (CTS_OPERATION_UPDATED == type) {
			printf("Updated at %d\n", ver);
			if (latest_ver < ver) latest_ver = ver;
		}
		else if (CTS_OPERATION_DELETED == type) {
			printf("Deleted at %d\n", ver);
			if (latest_ver < ver) latest_ver = ver;
		}
		else {
			printf("unknown type (%d)", type);
		}

		contacts_svc_value_free(row_info);
	}
	contacts_svc_iter_remove(iter);
}

static void missed_call_change_callback(void *data)
{
	printf("Missed Call is changed\n");
}

static void group_change_callback(void *data)
{
	int ret;
	static int latest_ver = 0;
	CTSiter *iter;

	printf("Group data of contacts service is changed\n");

	ret = contacts_svc_get_updated_groups(0, latest_ver, &iter);
	if (CTS_SUCCESS != ret) {
		printf("contacts_svc_get_updated_groups() Failed(%d)", ret);
		return;
	}

	while (CTS_SUCCESS == contacts_svc_iter_next(iter)) {
		int index;
		CTSvalue *row_info = NULL;
		row_info = contacts_svc_iter_get_info(iter);

		index = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_ID_INT);
		printf("(%8d) is ", index);
		int type = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_TYPE_INT);
		int ver = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_VER_INT);
		if (CTS_OPERATION_INSERTED == type) {
			printf("Inserted at %d\n", ver);
			if (latest_ver < ver) latest_ver = ver;
		}
		else if (CTS_OPERATION_UPDATED == type) {
			printf("Updated at %d\n", ver);
			if (latest_ver < ver) latest_ver = ver;
		}
		else if (CTS_OPERATION_DELETED == type) {
			printf("Deleted at %d\n", ver);
			if (latest_ver < ver) latest_ver = ver;
		}
		else {
			printf("unknown type (%d)", type);
		}

		contacts_svc_value_free(row_info);
	}
	contacts_svc_iter_remove(iter);
}

static void group_rel_change_callback(void *data)
{
	int ret;
	static int latest_ver = 0;
	CTSiter *iter;

	printf("Group relation of contacts service is changed\n");

	ret = contacts_svc_group_get_relation_changes(0, latest_ver, &iter);
	if (CTS_SUCCESS != ret) {
		printf("contacts_svc_group_get_relation_changes() Failed(%d)", ret);
		return;
	}

	while (CTS_SUCCESS == contacts_svc_iter_next(iter)) {
		int index;
		CTSvalue *row_info = NULL;
		row_info = contacts_svc_iter_get_info(iter);

		index = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_ID_INT);
		printf("(%8d) is ", index);
		int type = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_TYPE_INT);
		int ver = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_VER_INT);
		if (CTS_OPERATION_INSERTED == type) {
			printf("Inserted at %d\n", ver);
			if (latest_ver < ver) latest_ver = ver;
		}
		else if (CTS_OPERATION_UPDATED == type) {
			printf("Updated at %d\n", ver);
			if (latest_ver < ver) latest_ver = ver;
		}
		else if (CTS_OPERATION_DELETED == type) {
			printf("Deleted at %d\n", ver);
			if (latest_ver < ver) latest_ver = ver;
		}
		else {
			printf("unknown type (%d)", type);
		}

		contacts_svc_value_free(row_info);
	}
	contacts_svc_iter_remove(iter);
}

int main()
{
	GMainLoop *loop;

	contacts_svc_connect();
	contacts_svc_subscribe_change(CTS_SUBSCRIBE_CONTACT_CHANGE, contact_change_callback, NULL);
	contacts_svc_subscribe_change(CTS_SUBSCRIBE_PLOG_CHANGE, plog_change_callback, NULL);
	contacts_svc_subscribe_change(CTS_SUBSCRIBE_FAVORITE_CHANGE, favorite_change_callback, NULL);
	contacts_svc_subscribe_change(CTS_SUBSCRIBE_MISSED_CALL_CHANGE, missed_call_change_callback, NULL);
	contacts_svc_subscribe_change(CTS_SUBSCRIBE_GROUP_CHANGE, group_change_callback, NULL);
	contacts_svc_subscribe_change(CTS_SUBSCRIBE_GROUP_RELATION_CHANGE, group_rel_change_callback, NULL);

	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	contacts_svc_unsubscribe_change(CTS_SUBSCRIBE_CONTACT_CHANGE, contact_change_callback);
	contacts_svc_unsubscribe_change(CTS_SUBSCRIBE_PLOG_CHANGE, plog_change_callback);
	contacts_svc_unsubscribe_change(CTS_SUBSCRIBE_FAVORITE_CHANGE, favorite_change_callback);
	contacts_svc_unsubscribe_change(CTS_SUBSCRIBE_MISSED_CALL_CHANGE, missed_call_change_callback);
	contacts_svc_unsubscribe_change(CTS_SUBSCRIBE_GROUP_CHANGE, group_change_callback);
	contacts_svc_unsubscribe_change(CTS_SUBSCRIBE_GROUP_RELATION_CHANGE, group_rel_change_callback);

	contacts_svc_disconnect();
	g_main_loop_unref(loop);

	return 0;
}


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
	printf("Contact data of contacts service is changed\n");
}

static void missed_call_change_callback(void *data)
{
	printf("Missed Call is changed\n");
}

int main()
{
	GMainLoop *loop;

	contacts_svc_connect();
	contacts_svc_subscribe_change(CTS_SUBSCRIBE_CONTACT_CHANGE, contact_change_callback, NULL);
	contacts_svc_subscribe_change(CTS_SUBSCRIBE_PLOG_CHANGE, plog_change_callback, NULL);
	contacts_svc_subscribe_change(CTS_SUBSCRIBE_FAVORITE_CHANGE, favorite_change_callback, NULL);
	contacts_svc_subscribe_change(CTS_SUBSCRIBE_MISSED_CALL_CHANGE, missed_call_change_callback, NULL);

	loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop);

	contacts_svc_unsubscribe_change(CTS_SUBSCRIBE_CONTACT_CHANGE, contact_change_callback);
	contacts_svc_unsubscribe_change(CTS_SUBSCRIBE_PLOG_CHANGE, plog_change_callback);
	contacts_svc_unsubscribe_change(CTS_SUBSCRIBE_FAVORITE_CHANGE, favorite_change_callback);
	contacts_svc_unsubscribe_change(CTS_SUBSCRIBE_MISSED_CALL_CHANGE, favorite_change_callback);

	contacts_svc_disconnect();
	g_main_loop_unref(loop);

	return 0;
}


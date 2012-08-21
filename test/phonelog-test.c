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
#include <stdlib.h>
#include <glib.h>
#include <unistd.h>
#include <string.h>
#include <contacts-svc.h>

void phonelog_insert_test(void)
{
	CTSvalue *plog;

	plog = contacts_svc_value_new(CTS_VALUE_PHONELOG);
	contacts_svc_value_set_str(plog, CTS_PLOG_VAL_ADDRESS_STR, "0123456789");
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TIME_INT,(int) time(NULL));
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TYPE_INT,
			CTS_PLOG_TYPE_VOICE_INCOMMING);
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_DURATION_INT, 65);
	contacts_svc_insert_phonelog(plog);

	contacts_svc_value_set_str(plog, CTS_PLOG_VAL_ADDRESS_STR, "0987654321");
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TIME_INT,(int) time(NULL));
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TYPE_INT,
			CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN);
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_DURATION_INT, 65);
	contacts_svc_insert_phonelog(plog);

	contacts_svc_value_set_str(plog, CTS_PLOG_VAL_ADDRESS_STR, "0987654321");
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TIME_INT,(int) time(NULL));
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TYPE_INT,
			CTS_PLOG_TYPE_VOICE_INCOMMING);
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_DURATION_INT, 65);
	contacts_svc_insert_phonelog(plog);


	contacts_svc_value_free(plog);
}

void phonelog_insert_email_test()
{
	CTSvalue *plog;

	plog = contacts_svc_value_new(CTS_VALUE_PHONELOG);
	contacts_svc_value_set_str(plog, CTS_PLOG_VAL_NUMBER_STR, "kildong.hong@samsung.com");
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TIME_INT, (int)time(NULL));
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TYPE_INT,
			CTS_PLOG_TYPE_EMAIL_RECEIVED);
	contacts_svc_value_set_str(plog, CTS_PLOG_VAL_SHORTMSG_STR, "Subject : Hello~");
	contacts_svc_value_set_int(plog, CTS_PLOG_VAL_MSGID_INT, 1);
	contacts_svc_insert_phonelog(plog);
	contacts_svc_value_free(plog);
}

void phonelog_modify_test(void)
{
	contacts_svc_phonelog_set_seen(2, CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN);

	contacts_svc_delete_phonelog(CTS_PLOG_DEL_BY_ID, 3);
	contacts_svc_delete_phonelog(CTS_PLOG_DEL_BY_NUMBER, "0123456789");
}

void phonelog_get_list_test(int op)
{
	CTSiter *iter = NULL;
	char display[1024]={0};

	contacts_svc_get_list(op, &iter);

	while (CTS_SUCCESS == contacts_svc_iter_next(iter))
	{
		CTSvalue *plog= NULL;
		plog = contacts_svc_iter_get_info(iter);

		const char *img_path = contacts_svc_value_get_str(plog, CTS_LIST_PLOG_IMG_PATH_STR);
		const char *display_name = contacts_svc_value_get_str(plog, CTS_LIST_PLOG_DISPLAY_NAME_STR);
		const char *number = contacts_svc_value_get_str(plog, CTS_LIST_PLOG_NUMBER_STR);
		if (display_name)
			snprintf(display, sizeof(display), "%s", display_name);
		else
		{
			const char *first = contacts_svc_value_get_str(plog, CTS_LIST_PLOG_FIRST_NAME_STR);
			const char *last = contacts_svc_value_get_str(plog, CTS_LIST_PLOG_LAST_NAME_STR);
			if (first && last) {
				if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
					snprintf(display, sizeof(display), "%s %s", first, last);
				else
					snprintf(display, sizeof(display), "%s %s", last, first);
			}else if (first)
				strcpy(display, first);
			else if (last)
				strcpy(display, last);
			else {
				if (number)
					strcpy(display, number);
			}
		}

		int num_type = contacts_svc_value_get_int(plog, CTS_LIST_PLOG_NUM_TYPE_INT);

		int index = contacts_svc_value_get_int(plog, CTS_LIST_PLOG_ID_INT);
		int type = contacts_svc_value_get_int(plog, CTS_LIST_PLOG_LOG_TYPE_INT);
		int time = contacts_svc_value_get_int(plog, CTS_LIST_PLOG_LOG_TIME_INT);

		if (strlen(display))
			printf("%d:%s(%s:%d):type=%d:time=%d\n",
					index, display, number, num_type, type, time);
		if (img_path)
			printf("%s\n", img_path);
		contacts_svc_value_free(plog);
	}
	contacts_svc_iter_remove(iter);
}

void phonelog_get_detail_list_test(void)
{
	int ret;
	CTSiter *iter = NULL;
	contacts_svc_get_list_with_str(CTS_LIST_PLOGS_OF_NUMBER,"0987654321", &iter);

	ret = contacts_svc_iter_next(iter);
	while (CTS_SUCCESS == ret)
	{
		CTSvalue *plog=NULL;
		plog = contacts_svc_iter_get_info(iter);

		int index = contacts_svc_value_get_int(plog, CTS_LIST_PLOG_ID_INT);
		int type = contacts_svc_value_get_int(plog, CTS_LIST_PLOG_LOG_TYPE_INT);
		int time = contacts_svc_value_get_int(plog, CTS_LIST_PLOG_LOG_TIME_INT);
		int duration = contacts_svc_value_get_int(plog, CTS_LIST_PLOG_DURATION_INT);

		printf("%d::type=%d:time=%d:duration=%d\n", index, type, time, duration);
		contacts_svc_value_free(plog);
		ret = contacts_svc_iter_next(iter);
	}
	contacts_svc_iter_remove(iter);
}

static int plog_get_number_list_cb(const char *number, void *user_data)
{
	printf("number = %s\n", number);
	return CTS_SUCCESS;
}

void phonelog_get_number_list_test(void)
{
	int ret;

	ret = contacts_svc_phonelog_get_all_number(plog_get_number_list_cb, NULL);
	if (CTS_SUCCESS != ret)
		printf("contacts_svc_phonelog_get_all_number() Failed(%d)\n", ret);

}

void phonelog_get_last_call_number_test(void)
{
	char *number = contacts_svc_phonelog_get_last_number(CTS_PLOG_LAST_ALL);

	printf("Last Call Number is %s\n", number);
	free(number);
}

int main()
{
	contacts_svc_connect();
	phonelog_insert_test();
	sleep(2);
	phonelog_insert_test();
	printf("grouping List 1 <<<<<<<<<<<\n");
	phonelog_get_list_test(CTS_LIST_GROUPING_PLOG);
	phonelog_modify_test();
	printf("grouping List 2 <<<<<<<<<<<\n");
	phonelog_get_list_test(CTS_LIST_GROUPING_PLOG);
	printf("email List 2 <<<<<<<<<<<\n");
	phonelog_insert_email_test();
	phonelog_get_list_test(CTS_LIST_ALL_EMAIL_PLOG);
	printf("detail List <<<<<<<<<<<\n");
	phonelog_get_detail_list_test();
	printf("phonelog number List <<<<<<<<<<<\n");
	phonelog_get_number_list_test();

	phonelog_get_last_call_number_test();

	contacts_svc_disconnect();
	return 0;
}

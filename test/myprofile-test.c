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

#include <contacts-svc.h>

static int insert_my_test(void)
{
	CTSstruct *contact;
	CTSvalue *name, *number1, *number2;
	CTSvalue *nick, *event, *company;
	GSList *numbers, *nicknames, *events;
	contact = contacts_svc_struct_new(CTS_STRUCT_CONTACT);

	CTSvalue *base = contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);
	if (base) {
		//contacts_svc_value_set_str(base, CTS_BASE_VAL_IMG_PATH_STR, "/opt/media/Images and videos/Wallpapers/Wallpaper3.jpg");
	}
	contacts_svc_struct_store_value(contact, CTS_CF_BASE_INFO_VALUE, base);
	contacts_svc_value_free(base);

	name = contacts_svc_value_new(CTS_VALUE_NAME);
	if (name) {
		contacts_svc_value_set_str(name, CTS_NAME_VAL_FIRST_STR, "MY");
		contacts_svc_value_set_str(name, CTS_NAME_VAL_LAST_STR, "NAME");
		contacts_svc_value_set_str(name, CTS_NAME_VAL_SUFFIX_STR, "TEST");
	}
	contacts_svc_struct_store_value(contact, CTS_CF_NAME_VALUE, name);
	contacts_svc_value_free(name);

	numbers = NULL;
	number1 = contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number1) {
		contacts_svc_value_set_str(number1, CTS_NUM_VAL_NUMBER_STR, "7777777");
		contacts_svc_value_set_int(number1, CTS_NUM_VAL_TYPE_INT,
				CTS_NUM_TYPE_CELL);
		contacts_svc_value_set_bool(number1, CTS_NUM_VAL_DEFAULT_BOOL, true);
	}
	numbers = g_slist_append(numbers, number1);

	number2 = contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number2) {
		contacts_svc_value_set_str(number2, CTS_NUM_VAL_NUMBER_STR, "3333333");
		contacts_svc_value_set_int(number2, CTS_NUM_VAL_TYPE_INT,
				CTS_NUM_TYPE_WORK|CTS_NUM_TYPE_VOICE);
	}
	numbers = g_slist_append(numbers, number2);

	contacts_svc_struct_store_list(contact, CTS_CF_NUMBER_LIST, numbers);
	contacts_svc_value_free(number1);
	contacts_svc_value_free(number2);
	g_slist_free(numbers);

	nicknames = NULL;
	nick = contacts_svc_value_new(CTS_VALUE_NICKNAME);
	if (nick)
		contacts_svc_value_set_str(nick, CTS_NICKNAME_VAL_NAME_STR, "MYnickname");

	nicknames = g_slist_append(nicknames, nick);
	contacts_svc_struct_store_list(contact, CTS_CF_NICKNAME_LIST, nicknames);
	contacts_svc_value_free(nick);
	g_slist_free(nicknames);

	nicknames = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_NICKNAME_LIST, &nicknames);
	if (nicknames)
		nick = contacts_svc_value_new(CTS_VALUE_NICKNAME);
	if (nick)
		contacts_svc_value_set_str(nick, CTS_NICKNAME_VAL_NAME_STR, "MYnickname2");
	nicknames = g_slist_append(nicknames, nick);
	contacts_svc_struct_store_list(contact, CTS_CF_NICKNAME_LIST, nicknames);
	contacts_svc_value_free(nick);
	//never free nicknames

	events = NULL;
	event = contacts_svc_value_new(CTS_VALUE_EVENT);
	if (event) {
		contacts_svc_value_set_int(event, CTS_EVENT_VAL_DATE_INT, 20110526);
		contacts_svc_value_set_int(event, CTS_EVENT_VAL_TYPE_INT, CTS_EVENT_TYPE_BIRTH);
	}

	events = g_slist_append(events, event);
	contacts_svc_struct_store_list(contact, CTS_CF_EVENT_LIST, events);
	contacts_svc_value_free(event);
	g_slist_free(events);

	company = contacts_svc_value_new(CTS_VALUE_COMPANY);
	if (company) {
		contacts_svc_value_set_str(company, CTS_COMPANY_VAL_NAME_STR, "Company");
		contacts_svc_value_set_str(company, CTS_COMPANY_VAL_DEPARTMENT_STR, "department");
		contacts_svc_value_set_str(company, CTS_COMPANY_VAL_JOB_TITLE_STR, "engineer");
	}
	contacts_svc_struct_store_value(contact, CTS_CF_COMPANY_VALUE, company);

	int ret = contacts_svc_set_myprofile(contact);
	contacts_svc_struct_free(contact);

	return ret;
}

static void get_myprofile(void)
{
	int index=0, ret=-1;
	CTSstruct *contact;
	CTSvalue *value=NULL;
	GSList *get_list, *cursor;

	ret = contacts_svc_get_contact(0, &contact);
	if (ret < 0) {
		printf("No found record\n");
		return;
	}

	contacts_svc_struct_get_value(contact, CTS_CF_NAME_VALUE, &value);
	printf("First Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_FIRST_STR));
	printf("Last Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_LAST_STR));
	printf("Additional Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_ADDITION_STR));
	printf("Display Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_DISPLAY_STR));
	printf("Prefix Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_PREFIX_STR));
	printf("Suffix Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_SUFFIX_STR));

	value = NULL;
	contacts_svc_struct_get_value(contact, CTS_CF_COMPANY_VALUE, &value);
	printf("Company Name : %s\n", contacts_svc_value_get_str(value, CTS_COMPANY_VAL_NAME_STR));
	printf("Company Department : %s\n", contacts_svc_value_get_str(value, CTS_COMPANY_VAL_DEPARTMENT_STR));

	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_NUMBER_LIST, &get_list);

	for (cursor=get_list;cursor;cursor=g_slist_next(cursor))
	{
		int type;
		type = contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_TYPE_INT);
		printf("number Type = %d  ", type);

		if (contacts_svc_value_get_bool(cursor->data, CTS_NUM_VAL_FAVORITE_BOOL))
			printf("(favorite)");
		printf("Number = %s\n",
				contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
	}

	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_EMAIL_LIST, &get_list);

	cursor = get_list;
	for (;cursor;cursor=g_slist_next(cursor))
	{
		printf("email Type = %d",
				contacts_svc_value_get_int(cursor->data, CTS_EMAIL_VAL_TYPE_INT));

		printf("email = %s\n",
				contacts_svc_value_get_str(cursor->data, CTS_EMAIL_VAL_ADDR_STR));
	}

	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_GROUPREL_LIST, &get_list);
	cursor = get_list;
	for (;cursor;cursor=g_slist_next(cursor))
	{
		printf("group = %s:",
				contacts_svc_value_get_str(cursor->data, CTS_GROUPREL_VAL_NAME_STR));

		printf("%d\n",
				contacts_svc_value_get_int(cursor->data, CTS_GROUPREL_VAL_ID_INT));
	}

	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_NICKNAME_LIST, &get_list);
	cursor = get_list;
	for (;cursor;cursor=g_slist_next(cursor))
		printf("nickname = %s\n",
				contacts_svc_value_get_str(cursor->data, CTS_NICKNAME_VAL_NAME_STR));

	if (index)
		contacts_svc_struct_free(contact);
}

int main()
{
	contacts_svc_connect();
	insert_my_test();
	get_myprofile();
	contacts_svc_disconnect();
	return 0;
}

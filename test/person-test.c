/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
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


static int make_preconditon(const char *first, const char *last, const char *num)
{
	int ret;
	CTSstruct *contact;
	CTSvalue *name, *number1, *base;
	GSList *numbers;

	contact = contacts_svc_struct_new(CTS_STRUCT_CONTACT);

	name = contacts_svc_value_new(CTS_VALUE_NAME);
	if (name) {
		contacts_svc_value_set_str(name, CTS_NAME_VAL_FIRST_STR, first);
		contacts_svc_value_set_str(name, CTS_NAME_VAL_LAST_STR, last);
	}
	contacts_svc_struct_store_value(contact, CTS_CF_NAME_VALUE, name);
	contacts_svc_value_free(name);

	numbers = NULL;
	number1 = contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number1) {
		contacts_svc_value_set_str(number1, CTS_NUM_VAL_NUMBER_STR, num);
		contacts_svc_value_set_int(number1, CTS_NUM_VAL_TYPE_INT,
				CTS_NUM_TYPE_CELL);
		contacts_svc_value_set_bool(number1, CTS_NUM_VAL_DEFAULT_BOOL, true);
	}
	numbers = g_slist_append(numbers, number1);

	contacts_svc_struct_store_list(contact, CTS_CF_NUMBER_LIST, numbers);
	contacts_svc_value_free(number1);
	g_slist_free(numbers);

	contacts_svc_insert_contact(0, contact);
	contacts_svc_struct_get_value(contact, CTS_CF_BASE_INFO_VALUE, &base);
	ret = contacts_svc_value_get_int(base, CTS_BASE_VAL_PERSON_ID_INT);
	contacts_svc_struct_free(contact);

	return ret;
}

static void get_person(int id)
{
	int index=0, ret=-1;
	CTSstruct *person;
	CTSvalue *value=NULL;
	GSList *get_list, *cursor;

	ret = contacts_svc_get_person(id, &person);
	if (ret < 0) {
		printf("No found record\n");
		return;
	}
	contacts_svc_struct_get_value(person, CTS_CF_NAME_VALUE, &value);
	printf("First Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_FIRST_STR));
	printf("Last Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_LAST_STR));
	//printf("Additional Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_ADDITION_STR));
	//printf("Display Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_DISPLAY_STR));
	//printf("Prefix Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_PREFIX_STR));
	//printf("Suffix Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_SUFFIX_STR));

	get_list = NULL;
	contacts_svc_struct_get_list(person, CTS_CF_NUMBER_LIST, &get_list);
	for (cursor=get_list;cursor;cursor=g_slist_next(cursor))
	{
		int type;
		type = contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_TYPE_INT);
		printf("number Type = %d  ", type);

		if (contacts_svc_value_get_bool(cursor->data, CTS_NUM_VAL_FAVORITE_BOOL))
			printf("(favorite)");
		printf("Number = %s\n",
				contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
		if (index)
			contacts_svc_set_favorite(CTS_FAVOR_NUMBER,
					contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_ID_INT));
	}

	get_list = NULL;
	contacts_svc_struct_get_list(person, CTS_CF_EMAIL_LIST, &get_list);

	cursor = get_list;
	for (;cursor;cursor=g_slist_next(cursor))
	{
		printf("email Type = %d",
				contacts_svc_value_get_int(cursor->data, CTS_EMAIL_VAL_TYPE_INT));

		printf("email = %s\n",
				contacts_svc_value_get_str(cursor->data, CTS_EMAIL_VAL_ADDR_STR));
	}

	get_list = NULL;
	contacts_svc_struct_get_list(person, CTS_CF_GROUPREL_LIST, &get_list);
	cursor = get_list;
	for (;cursor;cursor=g_slist_next(cursor))
	{
		printf("group = %s:",
				contacts_svc_value_get_str(cursor->data, CTS_GROUPREL_VAL_NAME_STR));

		printf("%d\n",
				contacts_svc_value_get_int(cursor->data, CTS_GROUPREL_VAL_ID_INT));
	}

	contacts_svc_struct_free(person);
}

static void get_person_list(void)
{
	CTSiter *iter = NULL;
	printf("Phone contact NUM = %d\n",
			contacts_svc_count(CTS_GET_ALL_CONTACT));

	contacts_svc_get_list(CTS_LIST_ALL_CONTACT, &iter);

	while (CTS_SUCCESS == contacts_svc_iter_next(iter))
	{
		CTSvalue *contact = NULL;
		const char *first, *last, *display;
		contact = contacts_svc_iter_get_info(iter);

		printf("(%8d)", contacts_svc_value_get_int(contact, CTS_LIST_CONTACT_ID_INT));
		display = contacts_svc_value_get_str(contact, CTS_LIST_CONTACT_DISPLAY_STR);
		if (display)
			printf("%s :", display);
		else
		{
			first = contacts_svc_value_get_str(contact, CTS_LIST_CONTACT_FIRST_STR);
			last = contacts_svc_value_get_str(contact, CTS_LIST_CONTACT_LAST_STR);
			if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
				printf("%s %s :", first, last);
			else
				printf("%s %s :", last, first);
		}
		printf("%s", contacts_svc_value_get_str(contact, CTS_LIST_CONTACT_IMG_PATH_STR));
		printf("\n");
		contacts_svc_value_free(contact);
	}
	contacts_svc_iter_remove(iter);
}

int main(int argc, char **argv)
{
	int person1, person2;

	contacts_svc_connect();

	person1 = make_preconditon("111", "111", "11111111");
	person2 = make_preconditon("222", "222", "22222222");
	get_person_list();

	contacts_svc_link_person(person1, person2);
	get_person(person1);
	get_person_list();

	contacts_svc_unlink_person(person1, person2);
	get_person_list();

	contacts_svc_disconnect();

	return 0;
}


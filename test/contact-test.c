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
#include <fcntl.h>
#include <contacts-svc.h>

static int insert_test(void)
{
	CTSstruct *contact;
	CTSvalue *name, *number1, *number2;
	CTSvalue *nick, *event;
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
		contacts_svc_value_set_str(name, CTS_NAME_VAL_FIRST_STR, "gildong");
		contacts_svc_value_set_str(name, CTS_NAME_VAL_LAST_STR, "Hong");
		contacts_svc_value_set_str(name, CTS_NAME_VAL_SUFFIX_STR, "engineer");
	}
	contacts_svc_struct_store_value(contact, CTS_CF_NAME_VALUE, name);
	contacts_svc_value_free(name);

	numbers = NULL;
	number1 = contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number1) {
		contacts_svc_value_set_str(number1, CTS_NUM_VAL_NUMBER_STR, "0987654321");
		contacts_svc_value_set_int(number1, CTS_NUM_VAL_TYPE_INT,
				CTS_NUM_TYPE_CELL);
		contacts_svc_value_set_bool(number1, CTS_NUM_VAL_DEFAULT_BOOL, true);
	}
	numbers = g_slist_append(numbers, number1);

	number2 = contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number2) {
		contacts_svc_value_set_str(number2, CTS_NUM_VAL_NUMBER_STR, "0123456789");
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
		contacts_svc_value_set_str(nick, CTS_NICKNAME_VAL_NAME_STR, "Samm");

	nicknames = g_slist_append(nicknames, nick);
	contacts_svc_struct_store_list(contact, CTS_CF_NICKNAME_LIST, nicknames);
	contacts_svc_value_free(nick);
	g_slist_free(nicknames);

	nicknames = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_NICKNAME_LIST, &nicknames);
	if (nicknames)
		nick = contacts_svc_value_new(CTS_VALUE_NICKNAME);
	if (nick)
		contacts_svc_value_set_str(nick, CTS_NICKNAME_VAL_NAME_STR, "3star");
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

	int ret = contacts_svc_insert_contact(0, contact);
	contacts_svc_struct_free(contact);

	return ret;
}

void delete_test(void)
{
	//get contact
	//CTSstruct *contact;
	//contacts_svc_struct_get_value(contact, CTS_CF_INDEX_INT, &value);
	//int index = contacts_svc_value_get_int(value, CTS_BASIC_VAL_INT);

	contacts_svc_delete_contact(2);

	//contacts_svc_struct_free(contact);

#if DELETE_CONTACTS
	// TODO: get each index of contacts
	int i, index_list[10] ={1,3,4,65,345,54,5,2,9,10};
	int ret;

	ret = contacts_svc_begin_trans();
	if (CTS_SUCCESS != ret) return;
	for (i=0;i<10;i++) {
		ret = contacts_svc_delete_contact(index_list[i]);
		if (CTS_SUCCESS != ret) {
			contacts_svc_end_trans(false);
			return;
		}
	}
	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS){
		printf("all work were rollbacked");
		return;
	}
#endif

}

void update_test()
{
	int ret;
	GSList *numbers, *cursor, *groupList, *nicknames;
	CTSvalue *number=NULL, *name=NULL, *group = NULL, *company, *nick;
	CTSstruct *contact=NULL;

	ret = contacts_svc_get_contact(1, &contact);
	if (ret < CTS_SUCCESS) return;

	contacts_svc_struct_get_value(contact, CTS_CF_NAME_VALUE, &name);
	contacts_svc_value_set_str(name, CTS_NAME_VAL_FIRST_STR, "Changed first");
	contacts_svc_value_set_str(name, CTS_NAME_VAL_LAST_STR, "Changed last");
	contacts_svc_value_set_str(name, CTS_NAME_VAL_PREFIX_STR, "Changed prefix");
	contacts_svc_value_set_str(name, CTS_NAME_VAL_SUFFIX_STR, NULL);

	numbers = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_NUMBER_LIST, &numbers);
	cursor = numbers;
	if (cursor) {
		//char *temp = contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR);
		contacts_svc_value_set_str(cursor->data, CTS_NUM_VAL_NUMBER_STR, "0987651234");

		cursor = g_slist_next(cursor);
		if (cursor)
			contacts_svc_value_set_bool(cursor->data, CTS_NUM_VAL_DELETE_BOOL, true);

		number = contacts_svc_value_new(CTS_VALUE_NUMBER);
		if (number) {
			contacts_svc_value_set_str(number, CTS_NUM_VAL_NUMBER_STR, "+82125439876");
			contacts_svc_value_set_int(number, CTS_NUM_VAL_TYPE_INT,
					CTS_NUM_TYPE_WORK|CTS_NUM_TYPE_FAX);
			//         contacts_svc_value_set_bool(number, CTS_NUM_VAL_DEFAULT_BOOL, true);
			numbers = g_slist_append(numbers, number);
		}
	}

	contacts_svc_struct_store_list(contact, CTS_CF_NUMBER_LIST, numbers);
	contacts_svc_value_free(number);
	//free("+82125439876");

	groupList = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_GROUPREL_LIST, &groupList);

	cursor = groupList;
	cursor=g_slist_next(groupList);
	if (cursor)
		contacts_svc_value_set_bool(cursor->data, CTS_GROUPREL_VAL_DELETE_BOOL, true);

	group = contacts_svc_value_new(CTS_VALUE_GROUP_RELATION);
	if (group) {
		contacts_svc_value_set_int(group, CTS_GROUPREL_VAL_ID_INT, 2);
		groupList = g_slist_append(groupList, group);
	}
	contacts_svc_struct_store_list(contact, CTS_CF_GROUPREL_LIST, groupList);
	contacts_svc_value_free(group);

	company = contacts_svc_value_new(CTS_VALUE_COMPANY);
	if (company) {
		contacts_svc_value_set_str(company, CTS_COMPANY_VAL_NAME_STR, "Company");
		contacts_svc_value_set_str(company, CTS_COMPANY_VAL_DEPARTMENT_STR, "department");
		contacts_svc_value_set_str(company, CTS_COMPANY_VAL_JOB_TITLE_STR, "engineer");
	}
	contacts_svc_struct_store_value(contact, CTS_CF_COMPANY_VALUE, company);

	nicknames = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_NICKNAME_LIST, &nicknames);
	cursor = nicknames;
	if (cursor) {
		contacts_svc_value_set_bool(cursor->data, CTS_NICKNAME_VAL_DELETE_BOOL, true);

		nick = contacts_svc_value_new(CTS_VALUE_NICKNAME);
		if (nick) {
			contacts_svc_value_set_str(nick, CTS_NICKNAME_VAL_NAME_STR, "good company");
			nicknames = g_slist_append(nicknames, nick);
		}
		contacts_svc_struct_store_list(contact, CTS_CF_NICKNAME_LIST, nicknames);
		contacts_svc_value_free(nick);
	}


	contacts_svc_update_contact(contact);
	contacts_svc_struct_free(contact);
}

static void translation_type(int type, char *dest, int dest_size)
{
	const char *type_str;
	if (type & CTS_NUM_TYPE_CUSTOM)
	{
		char *custom;
		custom = contacts_svc_get_custom_type(CTS_TYPE_CLASS_NUM, type);
		if (NULL == custom)
			type_str = "Other";
		else {
			snprintf(dest, dest_size, custom);
			free(custom);
			return;
		}
	}
	else if (type & CTS_NUM_TYPE_CELL)
		type_str = "Mobile";
	else if (type & CTS_NUM_TYPE_VOICE)
	{
		if (type & CTS_NUM_TYPE_HOME)
			type_str = "Home";
		else if (type & CTS_NUM_TYPE_WORK)
			type_str = "Work";
		else
			type_str = "Telephone";
	}
	else if (type & CTS_NUM_TYPE_FAX)
	{
		if (type & CTS_NUM_TYPE_HOME)
			type_str = "Fax(home)";
		else if (type & CTS_NUM_TYPE_WORK)
			type_str = "Fax(work)";
		else
			type_str = "Fax";
	}
	else if (type & CTS_NUM_TYPE_PAGER)
		type_str = "Pager";
	else if (type & CTS_NUM_TYPE_CAR)
		type_str = "Car Telephone";
	else if (type & CTS_NUM_TYPE_ASSISTANT)
		type_str = "Assistant";
	else
		type_str = "Other";

	snprintf(dest, dest_size, type_str);
}

void get_contact(CTSstruct *contact)
{
	int index=0, ret=-1;
	CTSvalue *value=NULL;
	GSList *get_list, *cursor;

	if (!contact) {
		index = contacts_svc_find_contact_by(CTS_FIND_BY_NUMBER, "125439876");
		if (index > CTS_SUCCESS)
			ret = contacts_svc_get_contact(index, &contact);
		if (ret < 0)
		{
			printf("No found record\n");
			return;
		}
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
		char type_str[100];
		type = contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_TYPE_INT);
		translation_type(type, type_str, sizeof(type_str));
		printf("number Type = %s  ", type_str);

		if (contacts_svc_value_get_bool(cursor->data, CTS_NUM_VAL_FAVORITE_BOOL))
			printf("(favorite)");
		printf("Number = %s\n",
				contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
		if (index)
			contacts_svc_set_favorite(CTS_FAVOR_NUMBER,
					contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_ID_INT));
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

void get_contact_default_num(void)
{
	int index, ret;
	CTSvalue *number=NULL;
	const char *default_num;

	index = contacts_svc_find_contact_by(CTS_FIND_BY_NUMBER, "0125439876");

	ret = contacts_svc_get_contact_value(CTS_GET_DEFAULT_NUMBER_VALUE, index, &number);
	if (CTS_SUCCESS != ret) {
		printf("contacts_svc_get_contact_value() Failed(%d)\n", ret);
		return;
	}

	default_num = contacts_svc_value_get_str(number, CTS_NUM_VAL_NUMBER_STR);
	printf("The default Number is %s\n", default_num);
	contacts_svc_value_free(number);
}
void get_contact_list(void)
{
	CTSiter *iter = NULL;
	printf("Phone contact NUM = %d\n",
			contacts_svc_count_with_int(CTS_GET_COUNT_CONTACTS_IN_ADDRESSBOOK, 0));

	contacts_svc_get_list(CTS_LIST_ALL_CONTACT, &iter);
	//contacts_svc_get_list_with_int(CTS_LIST_MEMBERS_OF_ADDRESSBOOK_ID, 0, &iter);

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

void sync_data(int ver)
{
	int ret, index_num;
	CTSiter *iter;

	contacts_svc_get_updated_contacts(0, ver, &iter);

	while (CTS_SUCCESS == contacts_svc_iter_next(iter))
	{
		CTSstruct *contact= NULL;
		CTSvalue *row_info = NULL;
		row_info = contacts_svc_iter_get_info(iter);

		index_num = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_ID_INT);
		printf("(%8d)\n", index_num);
		int type = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_TYPE_INT);
		int ver = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_VER_INT);

		if (CTS_OPERATION_UPDATED == type || CTS_OPERATION_INSERTED == type) {
			contacts_svc_get_contact(index_num, &contact);
			char *vcard_stream, *new_stream;
			char file[128];
			snprintf(file, sizeof(file), "test%d.vcf", index_num);
			ret = contacts_svc_get_vcard_from_contact(contact, &vcard_stream);
			new_stream = contacts_svc_vcard_put_content(vcard_stream, "X-TEST", "1234");
			printf("%s\n", (char *)new_stream);
			free(new_stream);
			if (CTS_SUCCESS == ret) {
				//int fd = open(file, O_RDWR | O_CREAT);
				//write(fd, (char *)vcard_stream, strlen((char *)vcard_stream));
				//close(fd);
				CTSstruct *new_contact = NULL;
				ret = contacts_svc_get_contact_from_vcard(vcard_stream, &new_contact);
				if (CTS_SUCCESS == ret) {
					get_contact(new_contact);
					contacts_svc_struct_free(new_contact);
				}
				free(vcard_stream);
			}
			if (CTS_OPERATION_INSERTED == type)
				printf("Added : %d \n", ver);
			else
				printf("Updated : %d \n", ver);
			contacts_svc_struct_free(contact);
		}
		else
			printf("Deleted : %d \n", ver);

		contacts_svc_value_free(row_info);
	}
	contacts_svc_iter_remove(iter);
}

void search_contacts_by_name(void)
{
	int ret;
	CTSiter *iter;
	ret = contacts_svc_get_list_with_str(CTS_LIST_CONTACTS_WITH_NAME,
			"ch", &iter);
	if (CTS_SUCCESS != ret) return;

	while (CTS_SUCCESS == contacts_svc_iter_next(iter))
	{
		CTSvalue *row_info = NULL;
		const char *first, *last, *display;
		row_info = contacts_svc_iter_get_info(iter);

		printf("(%8d)", contacts_svc_value_get_int(row_info, CTS_LIST_CONTACT_ID_INT));

		display = contacts_svc_value_get_str(row_info, CTS_LIST_CONTACT_DISPLAY_STR);
		if (display)
			printf("%s :", display);
		else
		{
			first = contacts_svc_value_get_str(row_info, CTS_LIST_CONTACT_FIRST_STR);
			last = contacts_svc_value_get_str(row_info, CTS_LIST_CONTACT_LAST_STR);
			if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
				printf("%s %s :", first, last);
			else
				printf("%s %s :", last, first);
		}
		printf("%s", contacts_svc_value_get_str(row_info, CTS_LIST_CONTACT_IMG_PATH_STR));
		printf("\n");
		contacts_svc_value_free(row_info);
	}
	contacts_svc_iter_remove(iter);
}

void get_favorite_list(void)
{
	CTSiter *iter;
	contacts_svc_get_list(CTS_LIST_ALL_NUMBER_FAVORITE, &iter);

	while (CTS_SUCCESS == contacts_svc_iter_next(iter)) {
		CTSvalue *favorite = NULL;
		const char *first, *last, *display;
		favorite = contacts_svc_iter_get_info(iter);

		printf("(%8d)", contacts_svc_value_get_int(favorite, CTS_LIST_SHORTCUT_ID_INT));
		printf(":%d", contacts_svc_value_get_int(favorite, CTS_LIST_SHORTCUT_CONTACT_ID_INT));
		display = contacts_svc_value_get_str(favorite, CTS_LIST_SHORTCUT_DISPLAY_NAME_STR);
		if (display)
			printf("%s :", display);
		else
		{
			first = contacts_svc_value_get_str(favorite, CTS_LIST_SHORTCUT_FIRST_NAME_STR);
			last = contacts_svc_value_get_str(favorite, CTS_LIST_SHORTCUT_LAST_NAME_STR);
			if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
				printf("%s %s :", first, last);
			else
				printf("%s %s :", last, first);
		}
		printf("%s", contacts_svc_value_get_str(favorite, CTS_LIST_SHORTCUT_NUMBER_STR));
		printf("(%d)", contacts_svc_value_get_int(favorite, CTS_LIST_SHORTCUT_NUMBER_TYPE_INT));
		printf("-%d)", contacts_svc_value_get_int(favorite, CTS_LIST_SHORTCUT_SPEEDDIAL_INT));
		printf("%s", contacts_svc_value_get_str(favorite, CTS_LIST_SHORTCUT_IMG_PATH_STR));
		printf("\n");
		contacts_svc_value_free(favorite);
	}
	contacts_svc_iter_remove(iter);
}

void put_value_test()
{
	CTSvalue *value, *number;

	value = contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);
	if (value) {
		contacts_svc_value_set_str(value, CTS_BASE_VAL_RINGTONE_PATH_STR,
				"/opt/test/test.mp3");
		contacts_svc_put_contact_value(CTS_PUT_VAL_REPLACE_RINGTONE, 1, value);
		contacts_svc_value_free(value);
	}

	number = contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number) {
		contacts_svc_value_set_str(number, CTS_NUM_VAL_NUMBER_STR, "0123337777");
		contacts_svc_value_set_int(number, CTS_NUM_VAL_TYPE_INT, CTS_NUM_TYPE_CELL);
		//      contacts_svc_value_set_bool(number, CTS_NUM_VAL_DEFAULT_BOOL, true);

		contacts_svc_put_contact_value(CTS_PUT_VAL_ADD_NUMBER, 1, number);
		contacts_svc_value_free(number);
	}
}

static void print_extend_contact(CTSstruct *contact)
{
	int ret;
	CTSvalue *value;
	GSList *get_list, *cursor;
	value = NULL;
	contacts_svc_struct_get_value(contact, CTS_CF_NAME_VALUE, &value);
	printf("First Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_FIRST_STR));
	printf("Last Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_LAST_STR));

	value = NULL;
	ret = contacts_svc_find_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "YomiName");
	ret = contacts_svc_struct_get_value(contact, ret, &value);
	if (CTS_SUCCESS == ret) {
		printf("extend1 data2 : %s\n", contacts_svc_value_get_str(value, CTS_EXTEND_VAL_DATA2_STR));
		printf("extend1 data3 : %s\n", contacts_svc_value_get_str(value, CTS_EXTEND_VAL_DATA3_STR));
		printf("extend1 data4 : %s\n", contacts_svc_value_get_str(value, CTS_EXTEND_VAL_DATA4_STR));
	}
	value = NULL;
	ret = contacts_svc_find_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "Family");
	ret = contacts_svc_struct_get_value(contact, ret, &value);
	if (CTS_SUCCESS == ret) {
		printf("extend2 data2 : %s\n", contacts_svc_value_get_str(value, CTS_EXTEND_VAL_DATA2_STR));
		printf("extend2 data3 : %s\n", contacts_svc_value_get_str(value, CTS_EXTEND_VAL_DATA3_STR));
		printf("extend2 data4 : %s\n", contacts_svc_value_get_str(value, CTS_EXTEND_VAL_DATA4_STR));
	}
	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_NUMBER_LIST, &get_list);

	cursor = get_list;
	for (;cursor;cursor=g_slist_next(cursor))
	{
		int type;
		char type_str[100];
		type = contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_TYPE_INT);
		translation_type(type, type_str, sizeof(type_str));
		printf("number Type = %s", type_str);

		if (contacts_svc_value_get_bool(cursor->data, CTS_NUM_VAL_FAVORITE_BOOL))
			printf("(favorite)");
		printf("Number = %s\n",
				contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
	}
}

void extend_data_test(void)
{
	int ret, index;
	CTSstruct *contact;
	CTSvalue *name, *number1, *extend_value;
	GSList *numbers=NULL;

	contact = contacts_svc_struct_new(CTS_STRUCT_CONTACT);

	name = contacts_svc_value_new(CTS_VALUE_NAME);
	if (name) {
		contacts_svc_value_set_str(name, CTS_NAME_VAL_FIRST_STR, "People");
		contacts_svc_value_set_str(name, CTS_NAME_VAL_LAST_STR, "Japan");
	}
	contacts_svc_struct_store_value(contact, CTS_CF_NAME_VALUE, name);
	contacts_svc_value_free(name);

	number1 = contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number1) {
		contacts_svc_value_set_str(number1, CTS_NUM_VAL_NUMBER_STR, "0333333333");
		contacts_svc_value_set_int(number1, CTS_NUM_VAL_TYPE_INT, CTS_NUM_TYPE_CELL);
		contacts_svc_value_set_bool(number1, CTS_NUM_VAL_DEFAULT_BOOL, true);
	}
	numbers = g_slist_append(numbers, number1);

	contacts_svc_struct_store_list(contact, CTS_CF_NUMBER_LIST, numbers);
	contacts_svc_value_free(number1);
	g_slist_free(numbers);

	extend_value = contacts_svc_value_new(CTS_VALUE_EXTEND);
	if (extend_value) {
		contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA2_STR, "YomiFirstName");
		contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA3_STR, "YomiLastName");
		contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA4_STR, "YomiCompanyName");
	}
	ret = contacts_svc_find_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "YomiName");
	if (CTS_ERR_DB_RECORD_NOT_FOUND == ret)
		ret = contacts_svc_insert_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "YomiName");
	contacts_svc_struct_store_value(contact, ret, extend_value);
	contacts_svc_value_free(extend_value);

	extend_value = contacts_svc_value_new(CTS_VALUE_EXTEND);
	if (extend_value) {
		contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA2_STR, "Children1");
		contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA3_STR, "Children2");
		contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA4_STR, "Children3");
	}
	ret = contacts_svc_find_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "Family");
	if (CTS_ERR_DB_RECORD_NOT_FOUND == ret)
		ret = contacts_svc_insert_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "Family");
	contacts_svc_struct_store_value(contact, ret, extend_value);
	contacts_svc_value_free(extend_value);

	index = contacts_svc_insert_contact(0, contact);
	contacts_svc_struct_free(contact);
	contact = NULL;

	ret = contacts_svc_get_contact(index, &contact);
	if (ret < 0)
	{
		printf("No found record\n");
		return;
	}
	print_extend_contact(contact);

	//update test

	extend_value = NULL;
	int type = contacts_svc_find_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "Family");
	ret = contacts_svc_struct_get_value(contact, type, &extend_value);
	if (CTS_SUCCESS == ret)
		contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA2_STR, "Children4");
	contacts_svc_struct_store_value(contact, type, extend_value);

	contacts_svc_update_contact(contact);
	contacts_svc_struct_free(contact);
	contact = NULL;

	ret = contacts_svc_get_contact(index, &contact);
	if (ret < 0)
	{
		printf("No found record\n");
		return;
	}
	print_extend_contact(contact);
	contacts_svc_struct_free(contact);
}

void get_number_list(void)
{
	CTSiter *iter;
	contacts_svc_get_list_with_str(CTS_LIST_NUMBERINFOS_WITH_NUM, "123", &iter);
	//contacts_svc_get_list_with_str(CTS_LIST_NUMBERINFOS_WITH_NAME, "kim", &iter);

	while (CTS_SUCCESS == contacts_svc_iter_next(iter))
	{
		CTSvalue *contact = NULL;
		const char *first, *last, *display;
		contact = contacts_svc_iter_get_info(iter);

		printf("(%8d)", contacts_svc_value_get_int(contact, CTS_LIST_NUM_CONTACT_ID_INT));
		display = contacts_svc_value_get_str(contact, CTS_LIST_NUM_CONTACT_DISPLAY_STR);
		if (display)
			printf("%s :", display);
		else
		{
			first = contacts_svc_value_get_str(contact, CTS_LIST_NUM_CONTACT_FIRST_STR);
			last = contacts_svc_value_get_str(contact, CTS_LIST_NUM_CONTACT_LAST_STR);
			if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
				printf("%s %s :", first, last);
			else
				printf("%s %s :", last, first);
		}
		printf("%s", contacts_svc_value_get_str(contact, CTS_LIST_NUM_CONTACT_IMG_PATH_STR));
		printf("\n");
		printf("%s\n", contacts_svc_value_get_str(contact, CTS_LIST_NUM_NUMBER_STR));
		contacts_svc_value_free(contact);
	}
	contacts_svc_iter_remove(iter);
}

static int get_search_cb(CTSvalue *value, void *user_data)
{
	const char *number, *img_path;
	const char *first, *last, *display;

	printf("(%8d)", contacts_svc_value_get_int(value, CTS_LIST_NUM_CONTACT_ID_INT));
	display = contacts_svc_value_get_str(value, CTS_LIST_NUM_CONTACT_DISPLAY_STR);
	if (display)
		printf("%s :", display);
	else
	{
		first = contacts_svc_value_get_str(value, CTS_LIST_NUM_CONTACT_FIRST_STR);
		last = contacts_svc_value_get_str(value, CTS_LIST_NUM_CONTACT_LAST_STR);
		if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			printf("%s %s :", first, last);
		else
			printf("%s %s :", last, first);
	}

	img_path = contacts_svc_value_get_str(value, CTS_LIST_NUM_CONTACT_IMG_PATH_STR);
	if (img_path)
		printf("%s\n", img_path);
	else
		printf("\n");

	number = contacts_svc_value_get_str(value, CTS_LIST_NUM_NUMBER_STR);
	if (number)
		printf("%s\n", number);

	return CTS_SUCCESS;
}

void get_search(void)
{
	contacts_svc_smartsearch_excl("fir", 0, 0, get_search_cb, NULL);
}

static int get_list_with_filter_cb(CTSvalue *value, void *user_data)
{
	const char *img_path;
	const char *first, *last, *display;

	printf("(%8d)", contacts_svc_value_get_int(value, CTS_LIST_CONTACT_ID_INT));
	display = contacts_svc_value_get_str(value, CTS_LIST_CONTACT_DISPLAY_STR);
	if (display)
		printf("%s :", display);
	else
	{
		first = contacts_svc_value_get_str(value, CTS_LIST_CONTACT_FIRST_STR);
		last = contacts_svc_value_get_str(value, CTS_LIST_CONTACT_LAST_STR);
		if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			printf("%s %s :", first, last);
		else
			printf("%s %s :", last, first);
	}

	img_path = contacts_svc_value_get_str(value, CTS_LIST_CONTACT_IMG_PATH_STR);
	if (img_path)
		printf("%s\n", img_path);
	else
		printf("\n");

	return CTS_SUCCESS;
}

void get_list_with_filter(void)
{
	int ret;
	CTSfilter *filter;

	filter = contacts_svc_list_str_filter_new(CTS_FILTERED_CONTACTS_WITH_NAME,
			"kim", CTS_LIST_FILTER_ADDRESBOOK_ID_INT, 0, CTS_LIST_FILTER_NONE);
	if (NULL == filter) {
		printf("contacts_svc_list_str_filter_new() Failed\n");
		return;
	}

	ret = contacts_svc_list_with_filter_foreach(filter, get_list_with_filter_cb, NULL);
	if (CTS_SUCCESS != ret)
		printf("contacts_svc_list_with_filter_foreach() Failed(%d)\n", ret);

	contacts_svc_list_filter_free(filter);
}

int main()
{
	int ret, ver;
	contacts_svc_connect();
	printf("\n##Insert##\n");

	contacts_svc_begin_trans();
	ret = insert_test();
	if (CTS_SUCCESS < ret)
		ver = contacts_svc_end_trans(true);
	else
		contacts_svc_end_trans(false);
	printf("\n##Insert##\n");
	insert_test();
	sleep(2);
	printf("\n##Update test##\n");
	update_test();
	put_value_test();
	printf("\n##All Contact Information##\n");
	get_contact(NULL);
	printf("\n##Default Number##\n");
	get_contact_default_num();

	printf("\n##Contact List##\n");
	get_contact_list();
	printf("\n##Delete Test##\n");
	delete_test();
	printf("\n##Sync Test##\n");
	sync_data(ver);

	printf("\n##Search Name##\n");
	search_contacts_by_name();
	printf("\n##Favorite List##\n");
	get_favorite_list();
	printf("\n##Favorite 1 Delete##\n");
	contacts_svc_delete_favorite(1);
	get_favorite_list();

	printf("\n##Extend Data insert##\n");
	extend_data_test();

	printf("\n##Number List##\n");
	get_number_list();

	printf("\n##search##\n");
	get_search();

	printf("\n##list_with_filter##\n");
	get_list_with_filter();

	contacts_svc_disconnect();

	return 0;
}

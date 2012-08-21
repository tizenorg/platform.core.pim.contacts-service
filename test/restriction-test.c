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
#include <stdlib.h>
#include <fcntl.h>
#include <contacts-svc.h>

#include "test-log.h"

static int fb_insert(int is_facebook)
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

	if (is_facebook)
		contacts_svc_struct_set_restriction(contact);
	int ret = contacts_svc_insert_contact(0, contact);
	contacts_svc_struct_free(contact);

	return ret;
}

static inline void get_contact(int index)
{
	int ret;
	CTSvalue *value=NULL;
	GSList *get_list, *cursor;
	CTSstruct *contact;

	ret = contacts_svc_get_contact(index, &contact);
	if (ret < CTS_SUCCESS) {
		ERR("No found record");
		return;
	}

	contacts_svc_struct_get_value(contact, CTS_CF_NAME_VALUE, &value);
	INFO("First Name : %s", contacts_svc_value_get_str(value, CTS_NAME_VAL_FIRST_STR));
	INFO("Last Name : %s", contacts_svc_value_get_str(value, CTS_NAME_VAL_LAST_STR));
	INFO("Additional Name : %s", contacts_svc_value_get_str(value, CTS_NAME_VAL_ADDITION_STR));
	INFO("Display Name : %s", contacts_svc_value_get_str(value, CTS_NAME_VAL_DISPLAY_STR));
	INFO("Prefix Name : %s", contacts_svc_value_get_str(value, CTS_NAME_VAL_PREFIX_STR));
	INFO("Suffix Name : %s", contacts_svc_value_get_str(value, CTS_NAME_VAL_SUFFIX_STR));

	value = NULL;
	contacts_svc_struct_get_value(contact, CTS_CF_COMPANY_VALUE, &value);
	INFO("Company Name : %s", contacts_svc_value_get_str(value, CTS_COMPANY_VAL_NAME_STR));
	INFO("Company Department : %s", contacts_svc_value_get_str(value, CTS_COMPANY_VAL_DEPARTMENT_STR));

	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_NUMBER_LIST, &get_list);

	for (cursor=get_list;cursor;cursor=g_slist_next(cursor))
	{
		int type;

		type = contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_TYPE_INT);
		if (contacts_svc_value_get_bool(cursor->data, CTS_NUM_VAL_FAVORITE_BOOL))
			INFO("number Type = %d : %s(favorite)", type,
					contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
		else
			INFO("number Type = %d : %s", type,
					contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
	}

	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_EMAIL_LIST, &get_list);

	cursor = get_list;
	for (;cursor;cursor=g_slist_next(cursor))
	{
		INFO("email Type = %d : %s",
				contacts_svc_value_get_int(cursor->data, CTS_EMAIL_VAL_TYPE_INT),
				contacts_svc_value_get_str(cursor->data, CTS_EMAIL_VAL_ADDR_STR));
	}

	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_GROUPREL_LIST, &get_list);
	cursor = get_list;
	for (;cursor;cursor=g_slist_next(cursor))
	{
		INFO("group = %s:%d",
			contacts_svc_value_get_str(cursor->data, CTS_GROUPREL_VAL_NAME_STR),
			contacts_svc_value_get_int(cursor->data, CTS_GROUPREL_VAL_ID_INT));
	}

	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_NICKNAME_LIST, &get_list);
	cursor = get_list;
	for (;cursor;cursor=g_slist_next(cursor))
		INFO("nickname = %s",
				contacts_svc_value_get_str(cursor->data, CTS_NICKNAME_VAL_NAME_STR));

	contacts_svc_struct_free(contact);
}

static inline void get_contact_list(void)
{
	CTSiter *iter = NULL;
	INFO("Phone contact NUM = %d", contacts_svc_count(CTS_GET_ALL_CONTACT));

	contacts_svc_get_list(CTS_LIST_ALL_CONTACT, &iter);
	//contacts_svc_get_list_with_int(CTS_LIST_MEMBERS_OF_ADDRESSBOOK_ID, 0, &iter);

	while (CTS_SUCCESS == contacts_svc_iter_next(iter))
	{
		int index;
		CTSvalue *contact = NULL;
		const char *first, *last, *display, *img;
		contact = contacts_svc_iter_get_info(iter);

		index = contacts_svc_value_get_int(contact, CTS_LIST_CONTACT_ID_INT);
		img = contacts_svc_value_get_str(contact, CTS_LIST_CONTACT_IMG_PATH_STR);
		display = contacts_svc_value_get_str(contact, CTS_LIST_CONTACT_DISPLAY_STR);
		if (display)
			INFO("(%8d)%s : %s", index, display, img);
		else
		{
			first = contacts_svc_value_get_str(contact, CTS_LIST_CONTACT_FIRST_STR);
			last = contacts_svc_value_get_str(contact, CTS_LIST_CONTACT_LAST_STR);
			if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
				INFO("(%8d)%s %s : %s", index, first, last, img);
			else
				INFO("(%8d)%s %s : %s", index, last, first, img);
		}
		contacts_svc_value_free(contact);
	}
	contacts_svc_iter_remove(iter);
}

int main()
{
	int id;
	contacts_svc_connect();

	id = fb_insert(1);
	fb_insert(0);

	get_contact(1);
	get_contact_list();

	contacts_svc_disconnect();
	return 0;
}


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
#include <unistd.h>
#include <fcntl.h>

#include <contacts-svc.h>

#define MAX_BUF 1048576

static void get_contact(CTSstruct *contact)
{
	CTSvalue *value;
	GSList *get_list, *cursor;
	const char *tmp_str;

	value = NULL;
	contacts_svc_struct_get_value(contact, CTS_CF_BASE_INFO_VALUE, &value);
	printf("ID = %d\n", contacts_svc_value_get_int(value, CTS_BASE_VAL_ID_INT));
	printf("changed time = %d\n", contacts_svc_value_get_int(value, CTS_BASE_VAL_CHANGED_TIME_INT));

	value = NULL;
	contacts_svc_struct_get_value(contact, CTS_CF_NAME_VALUE, &value);
	if ((tmp_str = contacts_svc_value_get_str(value, CTS_NAME_VAL_FIRST_STR)))
		printf("First Name : %s\n", tmp_str);
	if ((tmp_str = contacts_svc_value_get_str(value, CTS_NAME_VAL_LAST_STR)))
		printf("Last Name : %s\n", tmp_str);
	if ((tmp_str = contacts_svc_value_get_str(value, CTS_NAME_VAL_ADDITION_STR)))
		printf("Additional Name : %s\n", tmp_str);
	if ((tmp_str = contacts_svc_value_get_str(value, CTS_NAME_VAL_DISPLAY_STR)))
		printf("Display Name : %s\n", tmp_str);
	if ((tmp_str = contacts_svc_value_get_str(value, CTS_NAME_VAL_PREFIX_STR)))
		printf("Prefix Name : %s\n", tmp_str);
	if ((tmp_str = contacts_svc_value_get_str(value, CTS_NAME_VAL_SUFFIX_STR)))
		printf("Suffix Name : %s\n", tmp_str);

	value = NULL;
	contacts_svc_struct_get_value(contact, CTS_CF_COMPANY_VALUE, &value);
	if ((tmp_str = contacts_svc_value_get_str(value, CTS_COMPANY_VAL_NAME_STR)))
		printf("Company Name : %s\n", tmp_str);
	if ((tmp_str = contacts_svc_value_get_str(value, CTS_COMPANY_VAL_DEPARTMENT_STR)))
		printf("Company Department : %s\n", tmp_str);

	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_NUMBER_LIST, &get_list);

	for (cursor = get_list;cursor;cursor=cursor->next)
	{
		printf("number Type = %d",
				contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_TYPE_INT));
		if (contacts_svc_value_get_bool(cursor->data, CTS_NUM_VAL_FAVORITE_BOOL))
			printf("(favorite)");
		printf("Number = %s\n",
				contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
	}

	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_EMAIL_LIST, &get_list);

	for (cursor = get_list;cursor;cursor=cursor->next)
	{
		printf("email Type = %d",
				contacts_svc_value_get_int(cursor->data, CTS_EMAIL_VAL_TYPE_INT));

		printf("email = %s\n",
				contacts_svc_value_get_str(cursor->data, CTS_EMAIL_VAL_ADDR_STR));
	}

	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_POSTAL_ADDR_LIST, &get_list);
	for (cursor = get_list;cursor;cursor=cursor->next)
	{
		printf(">>> Postal(type = %d) <<<\n",
				contacts_svc_value_get_int(cursor->data, CTS_POSTAL_VAL_TYPE_INT));
		if ((tmp_str = contacts_svc_value_get_str(cursor->data, CTS_POSTAL_VAL_POBOX_STR)))
			printf("Pobox = %s\n", tmp_str);
		if ((tmp_str = contacts_svc_value_get_str(cursor->data, CTS_POSTAL_VAL_POSTALCODE_STR)))
			printf("POSTALCODE = %s\n", tmp_str);
		if ((tmp_str = contacts_svc_value_get_str(cursor->data, CTS_POSTAL_VAL_LOCALITY_STR)))
			printf("LOCALITY = %s\n", tmp_str);
		if ((tmp_str = contacts_svc_value_get_str(cursor->data, CTS_POSTAL_VAL_STREET_STR)))
			printf("STREET = %s\n", tmp_str);
		if ((tmp_str = contacts_svc_value_get_str(cursor->data, CTS_POSTAL_VAL_EXTENDED_STR)))
			printf("EXTENDED = %s\n", tmp_str);
		if ((tmp_str = contacts_svc_value_get_str(cursor->data, CTS_POSTAL_VAL_COUNTRY_STR)))
			printf("COUNTRY = %s\n", tmp_str);
	}
	get_list = NULL;
	contacts_svc_struct_get_list(contact, CTS_CF_EVENT_LIST, &get_list);
	for (cursor = get_list;cursor;cursor=cursor->next)
	{
		printf("type=%d:%d\n",
				contacts_svc_value_get_int(cursor->data, CTS_EVENT_VAL_TYPE_INT),
				contacts_svc_value_get_int(cursor->data, CTS_EVENT_VAL_DATE_INT));
	}
}

static int vcard_handler(const char *vcard, void *data)
{
	int ret;
	CTSstruct *new_contact = NULL;

	ret = contacts_svc_get_contact_from_vcard(vcard, &new_contact);
	if (CTS_SUCCESS == ret) {
		get_contact(new_contact);
		contacts_svc_struct_free(new_contact);
	}

	contacts_svc_insert_vcard(0, vcard);
	contacts_svc_replace_by_vcard(1, vcard);

	return CTS_SUCCESS;
}

int main(int argc, char **argv)
{
	if (argc < 2) return -1;
	contacts_svc_connect();

	printf("vcard file has %d vcards\n", contacts_svc_vcard_count(argv[1]));
	contacts_svc_vcard_foreach(argv[1], vcard_handler, NULL);

	contacts_svc_disconnect();

	return 0;
}

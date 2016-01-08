/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <contacts.h>

#include "test_main.h"
#include "test_debug.h"

int test_base_insert_contact(char *first_name, char *last_name, char *phone_number, int *out_id)
{
	DBG("[%s]", __func__);

	char *fn = first_name ? first_name : "First";
	char *ln = last_name ? last_name : "last";
	char *no = phone_number ? phone_number : "012-3456-7890";

	contacts_record_h name = NULL;
	contacts_record_create(_contacts_name._uri, &name);
	contacts_record_set_str(name, _contacts_name.first, fn);
	contacts_record_set_str(name, _contacts_name.last, ln);

	contacts_record_h number = NULL;
	contacts_record_create(_contacts_number._uri, &number);
	contacts_record_set_str(number, _contacts_number.number, no);

	int contact_id = 0;
	contacts_record_h contact = NULL;
	contacts_record_create(_contacts_contact._uri, &contact);
	contacts_record_add_child_record(contact, _contacts_contact.name, name);
	contacts_record_add_child_record(contact, _contacts_contact.number, number);
	contacts_db_insert_record(contact, &contact_id);
	contacts_record_destroy(contact, true);

	*out_id = contact_id;
	DBG("contact_id(%d)", contact_id);

	return 0;
}

int test_base_delete_contact_with_id(int id)
{
	DBG("[%s]", __func__);

	contacts_db_delete_record(_contacts_contact._uri, id);
	return 0;
}

int test_base_delete_contact(int argc, char **argv)
{
	if (argc < 2)
		return 0;

	contacts_connect();

	test_base_delete_contact_with_id(atoi(argv[2]));

	contacts_disconnect();
	return 0;
}


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

int test_base_insert_contact(int argc, char **argv)
{
	char *first_name = argc > 2 ? argv[0] : "Default first";
	char *last_name = argc > 3 ? argv[0] : "Default last";

	contacts_connect();

	int name_id = 0;
	contacts_record_h name = NULL;
	contacts_record_create(_contacts_name._uri, &name);
	contacts_record_set_str(name, _contacts_name.first, first_name);
	contacts_record_set_str(name, _contacts_name.last, last_name);
	DBG("name_id(%d)", name_id);

	int contact_id = 0;
	contacts_record_h contact = NULL;
	contacts_record_create(_contacts_contact._uri, &contact);
	contacts_record_add_child_record(contact, _contacts_contact.name, name);
	contacts_db_insert_record(contact, &contact_id);
	contacts_record_destroy(contact, true);
	DBG("contact_id(%d)", contact_id);

	contacts_disconnect();

	return 0;


}

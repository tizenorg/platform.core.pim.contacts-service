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
#include "test_query.h"
#include "test_snippet.h"

void _print_errors(void)
{
	DBG("CONTACTS_ERROR_NONE               (%d)", CONTACTS_ERROR_NONE              );
	DBG("CONTACTS_ERROR_OUT_OF_MEMORY      (%d)", CONTACTS_ERROR_OUT_OF_MEMORY     );
	DBG("CONTACTS_ERROR_INVALID_PARAMETER  (%d)", CONTACTS_ERROR_INVALID_PARAMETER );
	DBG("CONTACTS_ERROR_FILE_NO_SPACE      (%d)", CONTACTS_ERROR_FILE_NO_SPACE     );
	DBG("CONTACTS_ERROR_PERMISSION_DENIED  (%d)", CONTACTS_ERROR_PERMISSION_DENIED );
	DBG("CONTACTS_ERROR_NOT_SUPPORTED      (%d)", CONTACTS_ERROR_NOT_SUPPORTED     );
	DBG("CONTACTS_ERROR_NO_DATA            (%d)", CONTACTS_ERROR_NO_DATA	         );
	DBG("CONTACTS_ERROR_DB_LOCKED          (%d)", CONTACTS_ERROR_DB_LOCKED         );
	DBG("CONTACTS_ERROR_DB                 (%d)", CONTACTS_ERROR_DB                );
	DBG("CONTACTS_ERROR_IPC_NOT_AVALIABLE  (%d)", CONTACTS_ERROR_IPC_NOT_AVALIABLE );
	DBG("CONTACTS_ERROR_IPC                (%d)", CONTACTS_ERROR_IPC               );
	DBG("CONTACTS_ERROR_SYSTEM             (%d)", CONTACTS_ERROR_SYSTEM            );
	DBG("CONTACTS_ERROR_INTERNAL           (%d)", CONTACTS_ERROR_INTERNAL			 );
}

int _insert_person(char **argv)
{
	char *first_name = argv[0];
	char *last_name = argv[1];

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

int main(int argc, char **argv)
{
	ENTER();

	if (1 == argc) {
		_print_errors();
		return 0;
	}

	int index = atoi(argv[1]);
	DBG("index(%d)", index);

	switch (index) {
	case 1:
		_insert_person(argv +2);
		break;
	case 2:
		test_query_person_contact(argv +2);
		break;
	case 3:
		test_snippet_with_query_person_contact(argc -2, argv +2);
		break;
	default:
		break;
	}

	return 0;
}

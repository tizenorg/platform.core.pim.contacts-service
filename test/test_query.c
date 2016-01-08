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

int test_query_person_contact(char **argv)
{
	int limit = -1;
	int offset = -1;

	contacts_connect();

	contacts_list_h list = NULL;
	contacts_db_get_all_records(_contacts_person_contact._uri, limit, offset, &list);

	int count = 0;
	contacts_list_get_count(list, &count);
	DBG("count(%d)", count);

	contacts_list_first(list);
	contacts_record_h record = NULL;
	while (CONTACTS_ERROR_NONE == contacts_list_get_current_record_p(list, &record)) {
		char *name = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_contact.display_name, &name);
		char *name_index = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_contact.display_name_index, &name_index);

		DBG("name_index[%s] name[%s]", name_index, name);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}


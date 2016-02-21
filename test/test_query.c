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

int test_query_person_contact(int argc, char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	DBG("keyword[%s]", keyword);

	int limit = -1;
	int offset = -1;

	contacts_connect();

	contacts_list_h list = NULL;
	contacts_db_search_records(_contacts_person_contact._uri, keyword, limit, offset, &list);

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

		DBG("-------------------------------------------");
		DBG("name_index[%s] name[%s]", name_index, name);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}

int test_query_with_query_person_contact(int argc, char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	DBG("keyword[%s]", keyword);

	int limit = -1;
	int offset = -1;

	contacts_connect();

	contacts_query_h query = NULL;
	contacts_query_create(_contacts_person_contact._uri, &query);

	contacts_filter_h filter = NULL;
	contacts_filter_create(_contacts_person_contact._uri, &filter);

	contacts_filter_add_int(filter,
			_contacts_person_contact.address_book_id, CONTACTS_MATCH_EQUAL, 0);
	contacts_query_set_filter(query, filter);

	contacts_list_h list = NULL;
	contacts_db_search_records_with_query(query, keyword, limit, offset, &list);

	contacts_filter_destroy(filter);
	contacts_query_destroy(query);

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

		DBG("-------------------------------------------");
		DBG("name_index[%s] name[%s]", name_index, name);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}

int test_query_person_number(int argc, char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	DBG("keyword[%s]", keyword);

	int limit = -1;
	int offset = -1;

	contacts_connect();

	contacts_list_h list = NULL;
	contacts_db_search_records(_contacts_person_number._uri, keyword, limit, offset, &list);

	int count = 0;
	contacts_list_get_count(list, &count);
	DBG("count(%d)", count);

	contacts_list_first(list);
	contacts_record_h record = NULL;
	while (CONTACTS_ERROR_NONE == contacts_list_get_current_record_p(list, &record)) {
		char *name = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_number.display_name, &name);
		char *name_index = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_number.display_name_index, &name_index);
		char *number = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_number.number, &number);
		char *normalized_number = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_number.normalized_number, &normalized_number);

		DBG("-------------------------------------------");
		DBG("name_index[%s] name[%s] number[%s] normalized_number[%s]",
				name_index, name, number, normalized_number);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}

int test_query_with_query_person_number(int argc, char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	DBG("keyword[%s]", keyword);

	int limit = -1;
	int offset = -1;

	contacts_connect();

	contacts_query_h query = NULL;
	contacts_query_create(_contacts_person_number._uri, &query);

	contacts_filter_h filter = NULL;
	contacts_filter_create(_contacts_person_number._uri, &filter);

	contacts_filter_add_bool(filter,
			_contacts_person_number.has_phonenumber, true);
	contacts_query_set_filter(query, filter);

	contacts_list_h list = NULL;
	contacts_db_search_records_with_query(query, keyword, limit, offset, &list);

	contacts_filter_destroy(filter);
	contacts_query_destroy(query);

	int count = 0;
	contacts_list_get_count(list, &count);
	DBG("count(%d)", count);

	contacts_list_first(list);
	contacts_record_h record = NULL;
	while (CONTACTS_ERROR_NONE == contacts_list_get_current_record_p(list, &record)) {
		char *name = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_number.display_name, &name);
		char *name_index = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_number.display_name_index, &name_index);
		char *number = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_number.number, &number);
		char *normalized_number = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_number.normalized_number, &normalized_number);

		DBG("-------------------------------------------");
		DBG("name_index[%s] name[%s] number[%s] normalized_number[%s]",
				name_index, name, number, normalized_number);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}

int test_query_person_email(int argc, char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	DBG("keyword[%s]", keyword);

	int limit = -1;
	int offset = -1;

	contacts_connect();

	contacts_list_h list = NULL;
	contacts_db_search_records(_contacts_person_email._uri, keyword, limit, offset, &list);

	int count = 0;
	contacts_list_get_count(list, &count);
	DBG("count(%d)", count);

	contacts_list_first(list);
	contacts_record_h record = NULL;
	while (CONTACTS_ERROR_NONE == contacts_list_get_current_record_p(list, &record)) {
		char *name = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_email.display_name, &name);
		char *name_index = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_email.display_name_index, &name_index);
		char *email = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_email.email, &email);

		DBG("-------------------------------------------");
		DBG("name_index[%s] name[%s] email[%s]", name_index, name, email);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}

int test_query_with_query_person_email(int argc, char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	DBG("keyword[%s]", keyword);

	int limit = -1;
	int offset = -1;

	contacts_connect();

	contacts_query_h query = NULL;
	contacts_query_create(_contacts_person_email._uri, &query);

	contacts_filter_h filter = NULL;
	contacts_filter_create(_contacts_person_email._uri, &filter);

	contacts_filter_add_bool(filter,
			_contacts_person_email.is_favorite, false);
	contacts_query_set_filter(query, filter);

	contacts_list_h list = NULL;
	contacts_db_search_records_with_query(query, keyword, limit, offset, &list);

	contacts_filter_destroy(filter);
	contacts_query_destroy(query);

	int count = 0;
	contacts_list_get_count(list, &count);
	DBG("count(%d)", count);

	contacts_list_first(list);
	contacts_record_h record = NULL;
	while (CONTACTS_ERROR_NONE == contacts_list_get_current_record_p(list, &record)) {
		char *name = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_email.display_name, &name);
		char *name_index = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_email.display_name_index, &name_index);
		char *email = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_email.email, &email);

		DBG("-------------------------------------------");
		DBG("name_index[%s] name[%s] email[%s]", name_index, name, email);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}

static const func _func[] = {
	test_query_person_contact,
	test_query_with_query_person_contact,
	test_query_person_number,
	test_query_with_query_person_number,
	test_query_person_email,
	test_query_with_query_person_email,
};

int test_query(int argc, char **argv)
{
	ENTER();

	if (true == test_main_is_selected(argc, argv, 2, _func))
		return 0;

	int i = 0;
	int count = sizeof(_func) / sizeof(func);
	for (i = 0; i < count; i++) {
		if (_func[i](argc, argv) < 0)
			break;
	}
	return 0;
}


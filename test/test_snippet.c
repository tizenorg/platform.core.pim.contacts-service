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

int test_snippet_with_query_person_contact(int argc, char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	char *start_match = argc > 0 ? argv[1] : NULL;
	char *end_match = argc > 1 ? argv[2] : NULL;
	DBG("keyword[%s] start_match[\"%s\"] end_match[\"%s\"]",
			keyword, start_match, end_match);

	int limit = -1;
	int offset = -1;

	contacts_connect();

	contacts_query_h query = NULL;
	contacts_query_create(_contacts_person_contact._uri, &query);
	contacts_query_set_snippet(query, true, start_match, end_match);

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

int test_snippet_with_query_person(int argc, char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	char *start_match = argc > 2 ? argv[1] : NULL;
	char *end_match = argc > 3 ? argv[2] : NULL;
	DBG("keyword[%s] start_match[\"%s\"] end_match[\"%s\"]",
			keyword, start_match, end_match);

	int limit = -1;
	int offset = -1;

	contacts_connect();

	contacts_query_h query = NULL;
	contacts_query_create(_contacts_person._uri, &query);
	contacts_query_set_snippet(query, true, start_match, end_match);

	contacts_filter_h filter = NULL;
	contacts_filter_create(_contacts_person._uri, &filter);

	contacts_filter_add_int(filter,
			_contacts_person.link_count, CONTACTS_MATCH_GREATER_THAN_OR_EQUAL, -1);
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
				_contacts_person.display_name, &name);
		char *name_index = NULL;
		contacts_record_get_str_p(record,
				_contacts_person.display_name_index, &name_index);

		DBG("-------------------------------------------");
		DBG("name_index[%s] name[%s]", name_index, name);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}

int test_snippet_with_query_person_number(char **argv)
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

int test_snippet_with_query_person_email(char **argv)
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


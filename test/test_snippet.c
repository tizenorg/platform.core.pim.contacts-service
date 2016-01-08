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
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <contacts.h>

#include "test_main.h"
#include "test_debug.h"
#include "test_base.h"

typedef struct {
	char *first;
	char *last;
	char *number;
	char *keyword;
	bool result;
	int id;
} test_s;

static bool _is_valid(int argc, char **argv)
{
	bool is_valid = false;
	if (argc > 1) {
		if (-1 < atoi(argv[1])) {
			DBG("Valid");
			is_valid = true;
		} else {
			ERR("Invalid");
			is_valid = false;
		}
	} else if (argc == 1) {
		DBG("Valid and default");
		is_valid = true;

	} else {
		ERR("Invalid paramter");
		is_valid = false;
	}
	return is_valid;
}

static int _get_snippet_query_person(char *keyword, char *start_match, char *end_match,
		contacts_list_h *out_list)
{
	if (NULL == out_list) {
		ERR("Invalid paramter");
		return -1;
	}

	contacts_query_h query = NULL;
	contacts_query_create(_contacts_person._uri, &query);
	contacts_query_set_snippet(query, start_match, end_match);

	contacts_filter_h filter = NULL;
	contacts_filter_create(_contacts_person._uri, &filter);

	contacts_filter_add_int(filter, _contacts_person.link_count,
			CONTACTS_MATCH_GREATER_THAN_OR_EQUAL, -1);
	contacts_query_set_filter(query, filter);

	contacts_list_h list = NULL;
	contacts_db_search_records_with_query(query, keyword, -1, -1, &list);

	contacts_filter_destroy(filter);
	contacts_query_destroy(query);

	*out_list = list;
	return 0;
}

static bool _check_snippet_person(contacts_list_h list, const char *start_match, char *keyword)
{
	if (NULL == list)
		return false;

	int count = 0;
	contacts_list_get_count(list, &count);
	DBG("count(%d)", count);
	if (0 == count) {
		DBG("No list");
		return false;
	}

	const char *match = start_match ? start_match : "[";

	contacts_list_first(list);
	contacts_record_h record = NULL;
	while (CONTACTS_ERROR_NONE == contacts_list_get_current_record_p(list, &record)) {
		int id = 0;
		contacts_record_get_int(record, _contacts_person.id, &id);
		char *name = NULL;
		contacts_record_get_str_p(record, _contacts_person.display_name, &name);
		char *name_index = NULL;
		contacts_record_get_str_p(record, _contacts_person.display_name_index, &name_index);

		/* if start_match exists, this means FOUND */
		if (strstr(name, match)) {
			DBG("[Name matched] id:%d name_index:%s name:%s", id, name_index, name);
		} else {
			DBG("[Data matched] id:%d name_index:%s name:%s", id, name_index, name);
		}
		contacts_list_next(list);
	}
	return true;
}

/*
 * With no parameter, check all keyword_list.
 * With parameters, check onece.
 */
int test_snippet_with_query_person(int argc, char **argv)
{
	ENTER();

	if (false == _is_valid(argc, argv))
		return 0;

	int ret = 0;

	test_s tests[] = {
		{ "Default", "name", NULL, "defau", true},
		{ "Default", "name", NULL, "Defau", true},
		{ "Default", "name", NULL, "efau", false},
		{ "Default", "name", NULL, "nam", true},
		{ "Default", "name", NULL, "nAM", true},
		{ "홍길동", "citizen", NULL, "ㅎㄱ", true},
		{ "홍길동", "아저씨", NULL, "동아", false},
		{ "홍길동", "citizen", NULL, "ㄱㄷ", true},
		{ "홍길동", "citizen", NULL, "ㅎ길ㄷ", false},
		{ "A1", "B1", "010-1234-5678",   "1234", true},
		{ "A2", "B2", "01012345678",     "1234", true},
		{ "A3", "B3", "+8510-1234-5678", "1234", true},
		{ "A4", "B4", "+8501012345678",  "1234", true},
		{ "A5", "B5", "010-1234-5678",   "3456", true},
		{ "A6", "B6", "01012345678",     "3456", true},
		{ "A7", "B7", "+8510-1234-5678", "3456", true},
		{ "A8", "B8", "+8501012345678",  "3456", true},
		{ "A9", "B9", "010-1234-5678",   "34-56", false},
	};

	int test_loop = argc < 3 ? (sizeof(tests) / sizeof(tests[0])) : 1;
	DBG("test loop(%d)", test_loop);

	char *start_match = 3 < argc ? argv[3] : NULL;
	char *end_match = 4 < argc ? argv[4] : NULL;
	DBG("start_match:\"%s\" end_match:\"%s\"", start_match, end_match);

	contacts_connect();

	int i = 0;
	for (i = 0; i < test_loop; i++) {
		test_base_insert_contact(tests[i].first, tests[i].last, NULL, &tests[i].id);
		TEST_START("(%02d) test with keyword[%s]", i, tests[i].keyword);

		contacts_list_h list = NULL;
		_get_snippet_query_person(tests[i].keyword, start_match, end_match, &list);
		if (tests[i].result != _check_snippet_person(list, start_match, tests[i].keyword)) {
			TEST_FAILURE();
			contacts_list_destroy(list, true);
			ret = -1;
			break;
		}
		TEST_SUCCESS();

		contacts_list_destroy(list, true);
		test_base_delete_contact_with_id(tests[i].id);
	}

	contacts_disconnect();

	return ret;
}

static int _get_snippet_query_person_contact(char *keyword, char *start_match,
		char *end_match, contacts_list_h *out_list)
{
	if (NULL == out_list) {
		ERR("Invalid paramter");
		return -1;
	}

	contacts_query_h query = NULL;
	contacts_query_create(_contacts_person_contact._uri, &query);
	contacts_query_set_snippet(query, start_match, end_match);

	contacts_filter_h filter = NULL;
	contacts_filter_create(_contacts_person_contact._uri, &filter);

	contacts_filter_add_int(filter, _contacts_person_contact.address_book_id,
			CONTACTS_MATCH_EQUAL, 0);
	contacts_query_set_filter(query, filter);

	contacts_list_h list = NULL;
	contacts_db_search_records_with_query(query, keyword, -1, -1, &list);

	contacts_filter_destroy(filter);
	contacts_query_destroy(query);

	*out_list = list;
	return 0;
}

static bool _check_snippet_person_contact(contacts_list_h list, const char *start_match, char *keyword)
{
	if (NULL == list)
		return false;

	bool is_found = false;
	const char *match = start_match ? start_match : "[";

	int count = 0;
	contacts_list_get_count(list, &count);
	DBG("count(%d)", count);

	contacts_list_first(list);
	contacts_record_h record = NULL;
	while (CONTACTS_ERROR_NONE == contacts_list_get_current_record_p(list, &record)) {
		char *name = NULL;
		contacts_record_get_str_p(record, _contacts_person_contact.display_name, &name);
		char *name_index = NULL;
		contacts_record_get_str_p(record, _contacts_person_contact.display_name_index, &name_index);

		/* if start_match exists, this means FOUND */
		if (strstr(name, match)) {
			DBG("[Found] name_index:%s name:%s", name_index, name);
			is_found = true;
		} else {
			DBG("[  -  ] name_index:%s name:%s", name_index, name);
		}
		contacts_list_next(list);
	}
	return is_found;
}

int test_snippet_with_query_person_contact(int argc, char **argv)
{
	ENTER();

	if (false == _is_valid(argc, argv))
		return 0;

	int ret = 0;

	test_s tests[] = {
		{ "Default", "name", "01012345678", "1234", true},
		{ "Default", "name", NULL, "Defau", true},
		{ "Default", "name", NULL, "efau", false},
		{ "Default", "name", NULL, "nam", true},
		{ "Default", "name", NULL, "nAM", true},
		{ "홍길동", "citizen", NULL, "ㅎㄱ", true},
		{ "홍길동", "citizen", NULL, "ㄱㄷ", true},
		{ "홍길동", "citizen", NULL, "ㅎ길ㄷ", false},
	};

	int test_loop = argc < 3 ? (sizeof(tests) / sizeof(tests[0])) : 1;
	DBG("test loop(%d)", test_loop);

	char *start_match = 3 < argc ? argv[3] : NULL;
	char *end_match = 4 < argc ? argv[4] : NULL;
	DBG("start_match:\"%s\" end_match:\"%s\"", start_match, end_match);

	contacts_connect();

	int i = 0;
	for (i = 0; i < test_loop; i++) {
		test_base_insert_contact(tests[i].first, tests[i].last, NULL, &tests[i].id);
		TEST_START("(%02d) test with keyword[%s]", i + 1, tests[i].keyword);

		contacts_list_h list = NULL;
		_get_snippet_query_person_contact(tests[i].keyword, start_match, end_match, &list);
		if (tests[i].result != _check_snippet_person_contact(list, start_match, tests[i].keyword)) {
			TEST_FAILURE();
			contacts_list_destroy(list, true);
			test_base_delete_contact_with_id(tests[i].id);
			ret = -1;
			break;
		}
		TEST_SUCCESS();

		contacts_list_destroy(list, true);
		test_base_delete_contact_with_id(tests[i].id);
	}

	contacts_disconnect();

	return ret;

}

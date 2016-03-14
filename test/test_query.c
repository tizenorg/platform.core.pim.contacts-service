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
#include "contacts.h"
#include "test_debug.h"
#include "test_main.h"
#include "test_utils.h"

#define TEST_CONTACT_COUNT 2

enum {
	TEST_URI_PERSON_CONTACT,
	TEST_URI_PERSON_GROUPREL,
	TEST_URI_PERSON_GROUP_ASSIGNED,
	TEST_URI_PERSON_GROUP_NOT_ASSIGNED,
	TEST_URI_PERSON_NUMBER,
	TEST_URI_PERSON_EMAIL,
	TEST_URI_PERSON,
};

static const char *uris[] = {
	//"tizen.contacts_view.person/simple_contact",
	//"tizen.contacts_view.person/simple_contact/number",
	//"tizen.contacts_view.person/simple_contact/email",
	//"tizen.contacts_view.person/simple_contact/group",
	//"tizen.contacts_view.person/simple_contact/group_assigned",
	//"tizen.contacts_view.person/simple_contact/group_not_assigned",
	"tizen.contacts_view.person",
};

static const char *keywords[] = {
	"gildo",
	"123", "ㅎㄱ", "ㄱㄷ", "music", "Seoul",
	"SEOUL", "run", "tiz", "plat", "hong",
	"+", " ",
};



static contacts_record_h _get_name(const char *first, const char *last)
{
	contacts_record_h name = NULL;
	contacts_record_create(_contacts_name._uri, &name);
	contacts_record_set_str(name, _contacts_name.first, first);
	contacts_record_set_str(name, _contacts_name.last, last);
	return name;
}

static contacts_record_h _get_number(const char *phone_number)
{
	contacts_record_h number = NULL;
	contacts_record_create(_contacts_number._uri, &number);
	contacts_record_set_str(number, _contacts_number.number, phone_number);
	return number;
}

static contacts_record_h _get_email(int type, const char *email_address)
{
	contacts_record_h email = NULL;
	contacts_record_create(_contacts_email._uri, &email);
	contacts_record_set_int(email, _contacts_email.type, type);
	contacts_record_set_str(email, _contacts_email.email, email_address);
	return email;
}

static contacts_record_h _get_nickname(const char *name)
{
	contacts_record_h nickname = NULL;
	contacts_record_create(_contacts_nickname._uri, &nickname);
	contacts_record_set_str(nickname, _contacts_nickname.name, name);
	return nickname;
}

static contacts_record_h _get_address(const char *street, const char *locality,
		const char *region, const char *country)
{
	contacts_record_h address = NULL;
	contacts_record_create(_contacts_address._uri, &address);
	contacts_record_set_str(address, _contacts_address.street, street);
	contacts_record_set_str(address, _contacts_address.locality, locality);
	contacts_record_set_str(address, _contacts_address.region, region);
	contacts_record_set_str(address, _contacts_address.country, country);
	return address;
}

static contacts_record_h _get_note(const char *contents)
{
	contacts_record_h note = NULL;
	contacts_record_create(_contacts_note._uri, &note);
	contacts_record_set_str(note, _contacts_note.note, contents);
	return note;
}

static contacts_record_h _get_company(const char *department, const char *name,
		const char *role)
{
	contacts_record_h company = NULL;
	contacts_record_create(_contacts_company._uri, &company);
	contacts_record_set_str(company, _contacts_company.department, department);
	contacts_record_set_str(company, _contacts_company.name, name);
	contacts_record_set_str(company, _contacts_company.role, role);
	return company;
}

int _insert_contact_01(int *id)
{
	int contact_id = 0;
	contacts_record_h contact = NULL;
	contacts_record_create(_contacts_contact._uri, &contact);
	contacts_record_add_child_record(contact, _contacts_contact.name,
			_get_name("홍길동", "korea123"));
	contacts_record_add_child_record(contact, _contacts_contact.number,
			_get_number("010-1234-1234"));
	contacts_record_add_child_record(contact, _contacts_contact.email,
			_get_email(CONTACTS_EMAIL_TYPE_HOME, "gildong.hong@music.com"));
	contacts_record_add_child_record(contact, _contacts_contact.nickname,
			_get_nickname("super"));
	contacts_record_add_child_record(contact, _contacts_contact.address,
			_get_address("hongic Univ", "mapo", "Seoul", "Korea"));
	contacts_record_add_child_record(contact, _contacts_contact.note,
			_get_note("running"));
	contacts_record_add_child_record(contact, _contacts_contact.company,
			_get_company("Platform Lab", "tizen", "PIMS"));
	contacts_db_insert_record(contact, &contact_id);
	contacts_record_destroy(contact, true);
	DBG("contact_id(%d)", contact_id);

	if (id)
		*id = contact_id;

	return 0;
}

int _insert_contact_02(int *id)
{
	int contact_id = 0;
	contacts_record_h contact = NULL;
	contacts_record_create(_contacts_contact._uri, &contact);
	contacts_record_add_child_record(contact, _contacts_contact.name,
			_get_name("AlpaGo", "pro9"));
	contacts_record_add_child_record(contact, _contacts_contact.number,
			_get_number("+851034123412"));
	contacts_record_add_child_record(contact, _contacts_contact.email,
			_get_email(CONTACTS_EMAIL_TYPE_HOME, "alpago@baduk.com"));
	contacts_record_add_child_record(contact, _contacts_contact.nickname,
			_get_nickname("prime"));
	contacts_record_add_child_record(contact, _contacts_contact.address,
			_get_address("broad", "young123", "Gyoung", "Korea"));
	contacts_record_add_child_record(contact, _contacts_contact.note,
			_get_note("day123"));
	contacts_record_add_child_record(contact, _contacts_contact.company,
			_get_company("customer", "new123", "role"));
	contacts_db_insert_record(contact, &contact_id);
	contacts_record_destroy(contact, true);
	DBG("contact_id(%d)", contact_id);

	if (id)
		*id = contact_id;

	return 0;
}


static void _delete_contact(int contact_id)
{
	contacts_db_delete_record(_contacts_contact._uri, contact_id);
}

static void _get_query_on_uri(const char *uri, int limit, int offset, contacts_query_h *out_query)
{
	contacts_query_h query = NULL;
	contacts_query_create(uri, &query);

	contacts_filter_h filter = NULL;
	contacts_filter_create(uri, &filter);


	if (0 == strcmp(uri, _contacts_person_contact._uri)) {
		contacts_filter_add_int(filter, _contacts_person_contact.person_id,
				CONTACTS_MATCH_GREATER_THAN, 0);
	} else if (0 == strcmp(uri, _contacts_person_grouprel._uri)) {
		contacts_filter_add_int(filter, _contacts_person_grouprel.link_count,
				CONTACTS_MATCH_GREATER_THAN, 0);
	} else if (0 == strcmp(uri, _contacts_person_group_assigned._uri)) {
		contacts_filter_add_int(filter, _contacts_person_group_assigned.person_id,
				CONTACTS_MATCH_GREATER_THAN, 0);
	} else if (0 == strcmp(uri, _contacts_person_group_not_assigned._uri)) {
		contacts_filter_add_int(filter, _contacts_person_group_not_assigned.link_count,
				CONTACTS_MATCH_GREATER_THAN, 0);
	} else if (0 == strcmp(uri, _contacts_person_number._uri)) {
		contacts_filter_add_int(filter, _contacts_person_number.person_id,
				CONTACTS_MATCH_GREATER_THAN, 0);
	} else if (0 == strcmp(uri, _contacts_person_email._uri)) {
		contacts_filter_add_int(filter, _contacts_person_email.person_id,
				CONTACTS_MATCH_GREATER_THAN, 0);
	} else if (0 == strcmp(uri, _contacts_person._uri)) {
		contacts_filter_add_int(filter, _contacts_person.link_count,
				CONTACTS_MATCH_GREATER_THAN, 0);
	} else {
		ERR("Invalid uri[%s]", uri);
		return;
	}

	contacts_query_set_filter(query, filter);
	contacts_filter_destroy(filter);

	if (out_query)
		*out_query = query;
}

static int _check_list(const char *uri, contacts_list_h in_list, bool is_snippet)
{
	if (NULL == in_list) {
		ERR("NO list");
		return 0;
	}

	int uri_type = -1;
	if (0 == strcmp(uri, _contacts_person_contact._uri)) {
		uri_type = TEST_URI_PERSON_CONTACT;
	} else if (0 == strcmp(uri, _contacts_person_grouprel._uri)) {
		uri_type = TEST_URI_PERSON_GROUPREL;
	} else if (0 == strcmp(uri, _contacts_person_group_assigned._uri)) {
		uri_type = TEST_URI_PERSON_GROUP_ASSIGNED;
	} else if (0 == strcmp(uri, _contacts_person_group_not_assigned._uri)) {
		uri_type = TEST_URI_PERSON_GROUP_NOT_ASSIGNED;
	} else if (0 == strcmp(uri, _contacts_person_number._uri)) {
		uri_type = TEST_URI_PERSON_NUMBER;
	} else if (0 == strcmp(uri, _contacts_person_email._uri)) {
		uri_type = TEST_URI_PERSON_EMAIL;
	} else if (0 == strcmp(uri, _contacts_person._uri)) {
		uri_type = TEST_URI_PERSON;
	} else {
		ERR("Invalid uri[%s]", uri);
		return 0;
	}

	int count = 0;
	contacts_list_get_count(in_list, &count);
	DBG("count(%d)", count);

	contacts_list_first(in_list);
	contacts_record_h record = NULL;
	while (CONTACTS_ERROR_NONE == contacts_list_get_current_record_p(in_list, &record)) {
		int contact_id = 0;
		char *display_name = NULL;
		char *number = NULL;
		int extra_data1 = NULL;
		char *extra_data2 = NULL;

		switch (uri_type) {
		case TEST_URI_PERSON_CONTACT:
			contacts_record_get_int(record, _contacts_person_contact.contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person_contact.display_name, &display_name);
			if (false == is_snippet)
				break;

			contacts_record_get_int(record, _contacts_person_contact.extra_data1, &extra_data1);
			contacts_record_get_str_p(record, _contacts_person_contact.extra_data2, &extra_data2);
			break;
		case TEST_URI_PERSON_GROUPREL:
			contacts_record_get_int(record, _contacts_person_grouprel.contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person_grouprel.display_name, &display_name);
			if (false == is_snippet)
				break;

			contacts_record_get_int(record, _contacts_person_grouprel.extra_data1, &extra_data1);
			contacts_record_get_str_p(record, _contacts_person_grouprel.extra_data2, &extra_data2);
			break;
		case TEST_URI_PERSON_GROUP_ASSIGNED:
			contacts_record_get_int(record, _contacts_person_group_assigned.contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person_group_assigned.display_name, &display_name);
			if (false == is_snippet)
				break;

			contacts_record_get_int(record, _contacts_person_group_assigned.extra_data1, &extra_data1);
			contacts_record_get_str_p(record, _contacts_person_group_assigned.extra_data2, &extra_data2);
			break;
		case TEST_URI_PERSON_GROUP_NOT_ASSIGNED:
			contacts_record_get_int(record, _contacts_person_group_not_assigned.contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person_group_not_assigned.display_name, &display_name);
			if (false == is_snippet)
				break;

			contacts_record_get_int(record, _contacts_person_group_not_assigned.extra_data1, &extra_data1);
			contacts_record_get_str_p(record, _contacts_person_group_not_assigned.extra_data2, &extra_data2);
			break;
		case TEST_URI_PERSON_NUMBER:
			contacts_record_get_int(record, _contacts_person_number.display_contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person_number.display_name, &display_name);
			contacts_record_get_str_p(record, _contacts_person_number.number, &number);
			if (false == is_snippet)
				break;

			contacts_record_get_int(record, _contacts_person_number.extra_data1, &extra_data1);
			contacts_record_get_str_p(record, _contacts_person_number.extra_data2, &extra_data2);
			break;
		case TEST_URI_PERSON_EMAIL:
			contacts_record_get_int(record, _contacts_person_email.display_contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person_email.display_name, &display_name);
			if (false == is_snippet)
				break;

			contacts_record_get_int(record, _contacts_person_email.extra_data1, &extra_data1);
			contacts_record_get_str_p(record, _contacts_person_email.extra_data2, &extra_data2);
			break;
		case TEST_URI_PERSON:
			contacts_record_get_int(record, _contacts_person.display_contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person.display_name, &display_name);
			if (false == is_snippet)
				break;

			contacts_record_get_int(record, _contacts_person.extra_data1, &extra_data1);
			contacts_record_get_str_p(record, _contacts_person.extra_data2, &extra_data2);
			break;
		}
		DBG("id(%d) name[%s] |%d|%s|", contact_id, display_name, extra_data1, extra_data2);

		contacts_list_next(in_list);
	}

	return 0;
}

static int test_get_records(int argc, char **argv)
{
	DBG("[%s]", __func__);

	int i;
	int i_count = sizeof(uris)/sizeof(uris[0]);

	const char *uri = NULL;
	int offset = 0;
	int limit = 0;

	if (3 < argc) {
		i_count = 1;
	}

	contacts_connect();

	int ids[TEST_CONTACT_COUNT] = {0};
	_insert_contact_01(&ids[0]);
	_insert_contact_02(&ids[1]);

	for (i = 0; i < i_count; i++) {
		uri = 3 < argc ? argv[3] : uris[i];
		DBG("uri[%s]", uri);

		contacts_list_h get_list = NULL;

		DBG("search_records");
		contacts_db_get_all_records(uri, offset, limit, &get_list);
		_check_list(uri, get_list, false);
		contacts_list_destroy(get_list, true);

		DBG("search_records_with_query");
		contacts_query_h query = NULL;
		_get_query_on_uri(uri, limit, offset, &query);
		contacts_db_get_records_with_query(query, offset, limit, &get_list);
		_check_list(uri, get_list, false);
		contacts_list_destroy(get_list, true);
		contacts_query_destroy(query);
	}

	for (i = 0; i < TEST_CONTACT_COUNT; i++)
		_delete_contact(ids[i]);

	contacts_disconnect();
	return 0;
}

static int test_search_records(int argc, char **argv)
{
	DBG("[%s]", __func__);

	int i, j;
	int i_count = sizeof(uris)/sizeof(uris[0]);
	int j_count = sizeof(keywords)/sizeof(keywords[0]);

	const char *uri = NULL;
	const char *keyword = NULL;
	int offset = 0;
	int limit = 0;

	if (3 < argc) {
		i_count = 1;
		j_count = 1;
	}

	contacts_connect();

	int ids[TEST_CONTACT_COUNT] = {0};
	_insert_contact_01(&ids[0]);
	_insert_contact_02(&ids[1]);

	for (i = 0; i < i_count; i++) {
		uri = 3 < argc ? argv[3] : uris[i];
		DBG("uri[%s]", uri);

		for (j = 0; j < j_count; j++) {
			keyword = 4 < argc ? argv[4] : keywords[j];
			DBG("keyword[%s]", keyword);

			contacts_list_h get_list = NULL;

			DBG("search_records");
			contacts_db_search_records(uri, keyword, offset, limit, &get_list);
			_check_list(uri, get_list, false);
			contacts_list_destroy(get_list, true);

			DBG("search_records_with_range:CONTACTS_SEARCH_RANGE_NAME");
			contacts_db_search_records_with_range(uri, keyword, offset, limit,
					CONTACTS_SEARCH_RANGE_NAME, &get_list);
			_check_list(uri, get_list, false);
			contacts_list_destroy(get_list, true);

			DBG("search_records_with_range:CONTACTS_SEARCH_RANGE_NUMBER");
			contacts_db_search_records_with_range(uri, keyword, offset, limit,
					CONTACTS_SEARCH_RANGE_NUMBER, &get_list);
			_check_list(uri, get_list, false);
			contacts_list_destroy(get_list, true);

			DBG("search_records_with_range:CONTACTS_SEARCH_RANGE_DATA");
			contacts_db_search_records_with_range(uri, keyword, offset, limit,
					CONTACTS_SEARCH_RANGE_DATA, &get_list);
			_check_list(uri, get_list, false);
			contacts_list_destroy(get_list, true);

			DBG("search_records_with_query");
			contacts_query_h query = NULL;
			_get_query_on_uri(uri, limit, offset, &query);
			contacts_db_search_records_with_query(query, keyword, offset, limit,
					&get_list);
			_check_list(uri, get_list, false);
			contacts_list_destroy(get_list, true);
			contacts_query_destroy(query);
		}
	}

	for (i = 0; i < TEST_CONTACT_COUNT; i++)
		_delete_contact(ids[i]);

	contacts_disconnect();
	return 0;
}

/*
 * /usr/bin//contacts-service-test 1 2 {uri} {keyword}
 */
static int test_search_records_for_snippet(int argc, char **argv)
{
	DBG("[%s]", __func__);

	int i, j;
	int i_count = sizeof(uris)/sizeof(uris[0]);
	int j_count = sizeof(keywords)/sizeof(keywords[0]);

	const char *uri = NULL;
	const char *keyword = NULL;
	char *start_match = "[";
	char *end_match = "]";
	int offset = 0;
	int limit = 0;

	if (3 < argc) {
		i_count = 1;
		j_count = 1;
	}

	contacts_connect();

	int ids[TEST_CONTACT_COUNT] = {0};
	_insert_contact_01(&ids[0]);
	_insert_contact_02(&ids[1]);

	for (i = 0; i < i_count; i++) {
		uri = 3 < argc ? argv[3] : uris[i];
		DBG(">>>>>   uri[%s]", uri);

		for (j = 0; j < j_count; j++) {
			keyword = 4 < argc ? argv[4] : keywords[j];
			DBG("keyword [%s]", keyword);

			contacts_list_h get_list = NULL;

			DBG("search_records_for_snippet");
			contacts_db_search_records_for_snippet(uri, keyword, offset, limit,
					start_match, end_match, &get_list);
			_check_list(uri, get_list, true);
			contacts_list_destroy(get_list, true);

			DBG("search_records_with_range_for_snippet:CONTACTS_SEARCH_RANGE_NAME");
			contacts_db_search_records_with_range_for_snippet(uri, keyword, offset,
					limit, CONTACTS_SEARCH_RANGE_NAME, start_match, end_match, &get_list);
			_check_list(uri, get_list, true);
			contacts_list_destroy(get_list, true);

			DBG("search_records_with_range_for_snippet:CONTACTS_SEARCH_RANGE_NUMBER");
			contacts_db_search_records_with_range_for_snippet(uri, keyword, offset,
					limit, CONTACTS_SEARCH_RANGE_NUMBER, start_match, end_match, &get_list);
			_check_list(uri, get_list, true);
			contacts_list_destroy(get_list, true);

			DBG("search_records_with_range_for_snippet:CONTACTS_SEARCH_RANGE_DATA");
			contacts_db_search_records_with_range_for_snippet(uri, keyword, offset,
					limit, CONTACTS_SEARCH_RANGE_DATA, start_match, end_match, &get_list);
			_check_list(uri, get_list, true);
			contacts_list_destroy(get_list, true);

			DBG("search_records_with_query_for_snippet");
			contacts_query_h query = NULL;
			_get_query_on_uri(uri, limit, offset, &query);
			contacts_db_search_records_with_query_for_snippet(query, keyword, offset,
					limit, start_match, end_match, &get_list);
			_check_list(uri, get_list, true);
			contacts_list_destroy(get_list, true);
			contacts_query_destroy(query);
		}
	}

	for (i = 0; i < TEST_CONTACT_COUNT; i++)
		_delete_contact(ids[i]);

	contacts_disconnect();
	return 0;
}

static const func _func[] = {
	test_get_records,
	test_search_records,
	test_search_records_for_snippet,
};

int test_query(int argc, char **argv)
{
	ENTER();

	int count = sizeof(_func) / sizeof(func);
	if (true == test_main_is_selected(argc, argv, 2, _func, count))
		return 0;

	int i = 0;
	for (i = 0; i < count; i++) {
		test_utils_cleanup();
		if (_func[i](argc, argv) < 0)
			break;
	}
	return 0;
}


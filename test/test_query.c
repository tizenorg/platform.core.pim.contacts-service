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
#include "test_main.h"
#include "test_debug.h"

enum {
	TEST_URI_PERSON_CONTACT,
	TEST_URI_PERSON_GROUPREL,
	TEST_URI_PERSON_GROUP_ASSIGNED,
	TEST_URI_PERSON_GROUP_NOT_ASSIGNED,
	TEST_URI_PERSON_NUMBER,
	TEST_URI_PERSON_EMAIL,
	TEST_URI_PERSON,
};

int _insert_contact(int *id)
{
	contacts_record_h name = NULL;
	contacts_record_create(_contacts_name._uri, &name);
	contacts_record_set_str(name, _contacts_name.first, "홍길동");
	contacts_record_set_str(name, _contacts_name.last, "korea123");

	contacts_record_h number = NULL;
	contacts_record_create(_contacts_number._uri, &number);
	contacts_record_set_str(number, _contacts_number.number, "010-1234-1234");

	contacts_record_h email = NULL;
	contacts_record_create(_contacts_email._uri, &email);
	contacts_record_set_int(email, _contacts_email.type, CONTACTS_EMAIL_TYPE_HOME);
	contacts_record_set_str(email, _contacts_email.email, "gildong.hong@music.com");

	contacts_record_h nickname = NULL;
	contacts_record_create(_contacts_nickname._uri, &nickname);
	contacts_record_set_str(nickname, _contacts_nickname.name, "super");

	contacts_record_h address = NULL;
	contacts_record_create(_contacts_address._uri, &address);
	contacts_record_set_str(address, _contacts_address.street, "hongic Univ");
	contacts_record_set_str(address, _contacts_address.locality, "mapo");
	contacts_record_set_str(address, _contacts_address.region, "Seo");
	contacts_record_set_str(address, _contacts_address.country, "Korea");

	contacts_record_h note = NULL;
	contacts_record_create(_contacts_note._uri, &note);
	contacts_record_set_str(note, _contacts_note.note, "running");
/*
	contacts_record_h messenger = NULL;
	contacts_record_create(_contacts_messenger._uri, &messenger);
	contacts_record_set_int(messenger, _contacts_messenger.type, CONTACTS_MESSENGER_TYPE_CUSTOM);
	contacts_record_set_str(messenger, _contacts_messenger.label, "bongole");
*/
	contacts_record_h company = NULL;
	contacts_record_create(_contacts_company._uri, &company);
	contacts_record_set_str(company, _contacts_company.department, "Platform Lab");
	contacts_record_set_str(company, _contacts_company.name, "tizen");
	contacts_record_set_str(company, _contacts_company.role, "PIMS");

	int contact_id = 0;
	contacts_record_h contact = NULL;
	contacts_record_create(_contacts_contact._uri, &contact);
	contacts_record_add_child_record(contact, _contacts_contact.name, name);
	contacts_record_add_child_record(contact, _contacts_contact.number, number);
	contacts_record_add_child_record(contact, _contacts_contact.email, email);
	contacts_record_add_child_record(contact, _contacts_contact.nickname, nickname);
	contacts_record_add_child_record(contact, _contacts_contact.address, address);
	contacts_record_add_child_record(contact, _contacts_contact.note, note);
//	contacts_record_add_child_record(contact, _contacts_contact.messenger, messenger);
	contacts_record_add_child_record(contact, _contacts_contact.company, company);
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

static const char *uris[] = {
	"tizen.contacts_view.person/simple_contact"
	//"tizen.contacts_view.simple_contact/number",
	//"tizen.contacts_view.simple_contact/email",
	//"tizen.contacts_view.person/simple_contact/group",
	//"tizen.contacts_view.person/simple_contact/group_assigned",
	//"tizen.contacts_view.person/simple_contact/group_not_assigned",
	//"tizen.contacts_view.person",
};

static const char *keywords[] = {
	"123",
//	"ㅎㄱ", "ㄱㄷ", "gildong", "music", "Seoul",
//	"SEOUL", "run", "tiz", "plat", "hong",
//	"123", "+", " ",
};

static void _get_query_on_uri(const char *uri, const char *keyword, int limit, int offset,
		contacts_query_h *out_query)
{
	contacts_query_h query = NULL;
	contacts_query_create(uri, &query);

	contacts_filter_h filter = NULL;
	contacts_filter_create(uri, &filter);


	if (0 == strcmp(uri, _contacts_person_contact._uri)) {
		contacts_filter_add_int(filter, _contacts_person_contact.person_id,
				CONTACTS_MATCH_GREATER_THAN, 0);
	} else if (0 == strcmp(uri, _contacts_person_grouprel._uri)) {
	} else if (0 == strcmp(uri, _contacts_person_group_assigned._uri)) {
	} else if (0 == strcmp(uri, _contacts_person_group_not_assigned._uri)) {
	} else if (0 == strcmp(uri, _contacts_person_number._uri)) {
	} else if (0 == strcmp(uri, _contacts_person_email._uri)) {
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

//			contacts_record_get_int(record, _contacts_person_contact.extra_data1, &extra_data1);
//			contacts_record_get_str_p(record, _contacts_person_contact.extra_data2, &extra_data2);
//			DBG("extra_data1(%d) extra_data2:%s", extra_data1, extra_data2);
			break;
		case TEST_URI_PERSON_GROUPREL:
			contacts_record_get_int(record, _contacts_person_grouprel.contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person_grouprel.display_name, &display_name);
			DBG("contact id(%d) display_name[%s]", contact_id, display_name);
			if (false == is_snippet)
				break;

			break;
		case TEST_URI_PERSON_GROUP_ASSIGNED:
			contacts_record_get_int(record, _contacts_person_group_assigned.contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person_group_assigned.display_name, &display_name);
			DBG("contact id(%d) display_name[%s]", contact_id, display_name);
			if (false == is_snippet)
				break;

			break;
		case TEST_URI_PERSON_GROUP_NOT_ASSIGNED:
			contacts_record_get_int(record, _contacts_person_group_not_assigned.contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person_group_not_assigned.display_name, &display_name);
			DBG("contact id(%d) display_name[%s]", contact_id, display_name);
			if (false == is_snippet)
				break;

			break;
		case TEST_URI_PERSON_NUMBER:
			contacts_record_get_int(record, _contacts_person_number.display_contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person_number.display_name, &display_name);
			contacts_record_get_str_p(record, _contacts_person_number.number, &number);
			DBG("contact id(%d) display_name[%s]", contact_id, display_name);
			if (false == is_snippet)
				break;

			break;
		case TEST_URI_PERSON_EMAIL:
			contacts_record_get_int(record, _contacts_person_email.display_contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person_email.display_name, &display_name);
			DBG("contact id(%d) display_name[%s]", contact_id, display_name);
			if (false == is_snippet)
				break;

			break;
		case TEST_URI_PERSON:
			contacts_record_get_int(record, _contacts_person.display_contact_id, &contact_id);
			contacts_record_get_str_p(record, _contacts_person.display_name, &display_name);
			DBG("contact id(%d) display_name[%s]", contact_id, display_name);
			if (false == is_snippet)
				break;

			contacts_record_get_int(record, _contacts_person.extra_data1, &extra_data1);
			contacts_record_get_str_p(record, _contacts_person.extra_data2, &extra_data2);
			DBG("extra_data1(%d) extra_data2:%s", extra_data1, extra_data2);
			break;
		}

		contacts_list_next(in_list);
	}

	return 0;
}

static int test_search_records(int argc, char **argv)
{
	contacts_connect();

	int contact_id = 0;
	_insert_contact(&contact_id);

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

	for (i = 0; i < i_count; i++) {
		uri = 3 < argc ? argv[3] : uris[i];
		DBG("uri[%s]", uri);

		for (j = 0; j < j_count; j++) {
			keyword = 4 < argc ? argv[4] : keywords[j];

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
			_get_query_on_uri(uri, keyword, limit, offset, &query);
			contacts_db_search_records_with_query(query, keyword, offset, limit,
					&get_list);
			_check_list(uri, get_list, false);
			contacts_list_destroy(get_list, true);
			contacts_query_destroy(query);
		}
	}

	_delete_contact(contact_id);

	contacts_disconnect();
	return 0;
}

/*
 * /usr/bin//contacts-service-test 1 2 {uri} {keyword}
 */
static int test_search_records_for_snippet(int argc, char **argv)
{
	contacts_connect();

	int contact_id = 0;
	_insert_contact(&contact_id);

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

	for (i = 0; i < i_count; i++) {
		uri = 3 < argc ? argv[3] : uris[i];
		DBG("uri[%s]", uri);

		for (j = 0; j < j_count; j++) {
			keyword = 4 < argc ? argv[4] : keywords[j];

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
			_get_query_on_uri(uri, keyword, limit, offset, &query);
			contacts_db_search_records_with_query_for_snippet(query, keyword, offset,
					limit, start_match, end_match, &get_list);
			_check_list(uri, get_list, true);
			contacts_list_destroy(get_list, true);
			contacts_query_destroy(query);
		}
	}

	_delete_contact(contact_id);

	contacts_disconnect();
	return 0;
}

static const func _func[] = {
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
		if (_func[i](argc, argv) < 0)
			break;
	}
	return 0;
}


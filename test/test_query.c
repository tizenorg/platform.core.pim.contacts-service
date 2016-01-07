
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <contacts.h>

#include "test_main.h"
#include "test_debug.h"

int test_query_person_contact(char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	DEBUG("keyword[%s]", keyword);

	int limit = -1;
	int offset = -1;

	contacts_connect();

	contacts_list_h list = NULL;
	contacts_db_search_records(_contacts_person_contact._uri, keyword, limit, offset, &list);

	int count = 0;
	contacts_list_get_count(list, &count);
	DEBUG("count(%d)", count);

	contacts_list_first(list);
	contacts_record_h record = NULL;
	while (CONTACTS_ERROR_NONE == contacts_list_get_current_record_p(list, &record)) {
		char *name = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_contact.display_name, &name);
		char *name_index = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_contact.display_name_index, &name_index);

		DEBUG("-------------------------------------------");
		DEBUG("name_index[%s] name[%s]", name_index, name);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}

int test_query_with_query_person_contact(char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	DEBUG("keyword[%s]", keyword);

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
	DEBUG("count(%d)", count);

	contacts_list_first(list);
	contacts_record_h record = NULL;
	while (CONTACTS_ERROR_NONE == contacts_list_get_current_record_p(list, &record)) {
		char *name = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_contact.display_name, &name);
		char *name_index = NULL;
		contacts_record_get_str_p(record,
				_contacts_person_contact.display_name_index, &name_index);

		DEBUG("-------------------------------------------");
		DEBUG("name_index[%s] name[%s]", name_index, name);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}

int test_query_person_number(char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	DEBUG("keyword[%s]", keyword);

	int limit = -1;
	int offset = -1;

	contacts_connect();

	contacts_list_h list = NULL;
	contacts_db_search_records(_contacts_person_number._uri, keyword, limit, offset, &list);

	int count = 0;
	contacts_list_get_count(list, &count);
	DEBUG("count(%d)", count);

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

		DEBUG("-------------------------------------------");
		DEBUG("name_index[%s] name[%s] number[%s] normalized_number[%s]",
				name_index, name, number, normalized_number);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}

int test_query_with_query_person_number(char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	DEBUG("keyword[%s]", keyword);

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
	DEBUG("count(%d)", count);

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

		DEBUG("-------------------------------------------");
		DEBUG("name_index[%s] name[%s] number[%s] normalized_number[%s]",
				name_index, name, number, normalized_number);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}

int test_query_person_email(char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	DEBUG("keyword[%s]", keyword);

	int limit = -1;
	int offset = -1;

	contacts_connect();

	contacts_list_h list = NULL;
	contacts_db_search_records(_contacts_person_email._uri, keyword, limit, offset, &list);

	int count = 0;
	contacts_list_get_count(list, &count);
	DEBUG("count(%d)", count);

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

		DEBUG("-------------------------------------------");
		DEBUG("name_index[%s] name[%s] email[%s]", name_index, name, email);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}
int test_query_with_query_person_email(char **argv)
{
	char *keyword = NULL;
	keyword = argv[0];
	DEBUG("keyword[%s]", keyword);

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
	DEBUG("count(%d)", count);

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

		DEBUG("-------------------------------------------");
		DEBUG("name_index[%s] name[%s] email[%s]", name_index, name, email);
		contacts_list_next(list);
	}

	contacts_disconnect();

	return 0;
}

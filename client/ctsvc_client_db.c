/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_client_handle.h"
#include "ctsvc_client_db_helper.h"

API int contacts_db_insert_record(contacts_record_h record, int *id)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_insert_record(contact, record, id);

	return ret;
}

API int contacts_db_get_record(const char *view_uri, int id, contacts_record_h *out_record)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_get_record(contact, view_uri, id, out_record);

	return ret;
}

API int contacts_db_update_record(contacts_record_h record)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_update_record(contact, record);

	return ret;
}

API int contacts_db_delete_record(const char *view_uri, int id)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_delete_record(contact, view_uri, id);

	return ret;
}

API int contacts_db_replace_record(contacts_record_h record, int id)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_replace_record(contact, record, id);

	return ret;

}

API int contacts_db_get_all_records(const char *view_uri, int offset, int limit, contacts_list_h *out_list)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_get_all_records(contact, view_uri, offset, limit, out_list);

	return ret;
}

API int contacts_db_get_records_with_query(contacts_query_h query, int offset, int limit, contacts_list_h *out_list)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_get_records_with_query(contact, query, offset, limit, out_list);

	return ret;
}


API int contacts_db_get_count(const char *view_uri, int *out_count)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_get_count(contact, view_uri, out_count);

	return ret;
}

API int contacts_db_get_count_with_query(contacts_query_h query, int *out_count)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_get_count_with_query(contact, query, out_count);

	return ret;
}

API int contacts_db_insert_records(contacts_list_h list, int **ids, int *count)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_insert_records(contact, list, ids, count);

	return ret;
}

API int contacts_db_update_records(contacts_list_h list)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_update_records(contact, list);

	return ret;
}

API int contacts_db_delete_records(const char *view_uri, int ids[], int count)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_delete_records(contact, view_uri, ids, count);

	return ret;
}

API int contacts_db_replace_records(contacts_list_h list, int ids[], int count)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_replace_records(contact, list, ids, count);

	return ret;
}

API int contacts_db_get_changes_by_version(const char *view_uri, int addressbook_id,
		int contacts_db_version, contacts_list_h *record_list, int *current_contacts_db_version)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_get_changes_by_version(contact, view_uri, addressbook_id, contacts_db_version, record_list, current_contacts_db_version);

	return ret;
}

API int contacts_db_get_current_version(int *contacts_db_version)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_get_current_version(contact, contacts_db_version);

	return ret;

}

API int contacts_db_search_records(const char *view_uri, const char *keyword,
		int offset, int limit, contacts_list_h *out_list)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_search_records(contact, view_uri, keyword, offset, limit, out_list);

	return ret;
}

API int contacts_db_search_records_with_range(const char *view_uri, const char *keyword,
		int offset, int limit, int range, contacts_list_h *out_list)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_search_records_with_range(contact, view_uri, keyword, offset, limit, range, out_list);

	return ret;
}

API int contacts_db_search_records_with_query(contacts_query_h query, const char *keyword,
		int offset, int limit, contacts_list_h *out_list)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_search_records_with_query(contact, query, keyword, offset, limit, out_list);

	return ret;
}

API int contacts_db_search_records_for_snippet(const char *view_uri,
		const char *keyword,
		int offset,
		int limit,
		const char *start_match,
		const char *end_match,
		contacts_list_h *out_list)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_search_records_for_snippet(contact, view_uri, keyword,
			offset, limit, start_match, end_match, out_list);

	return ret;
}

API int contacts_db_search_records_with_range_for_snippet(const char *view_uri,
		const char *keyword,
		int offset,
		int limit,
		int range,
		const char *start_match,
		const char *end_match,
		contacts_list_h *out_list)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_search_records_with_range_for_snippet(contact, view_uri,
			keyword, offset, limit, range, start_match, end_match, out_list);

	return ret;
}

API int contacts_db_search_records_with_query_for_snippet(contacts_query_h query,
		const char *keyword,
		int offset,
		int limit,
		const char *start_match,
		const char *end_match,
		contacts_list_h *out_list)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_search_records_with_query_for_snippet(contact, query, keyword,
			offset, limit, start_match, end_match, out_list);

	return ret;
}

API int contacts_db_get_last_change_version(int *last_version)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_get_last_change_version(contact, last_version);

	return ret;
}

API int contacts_db_get_status(contacts_db_status_e *status)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_get_status(contact, status);

	return ret;
}

API int contacts_db_add_status_changed_cb(
		contacts_db_status_changed_cb cb, void *user_data)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_add_status_changed_cb(contact, cb, user_data);

	return ret;
}

API int contacts_db_remove_status_changed_cb(
		contacts_db_status_changed_cb cb, void *user_data)
{
	int ret;
	contacts_h contact = NULL;

	ret = ctsvc_client_handle_get_p(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_client_handle_get_p() Fail(%d)", ret);

	ret = ctsvc_client_db_remove_status_changed_cb(contact, cb, user_data);

	return ret;
}


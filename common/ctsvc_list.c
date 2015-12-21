/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <glib.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_list.h"
#include "ctsvc_record.h"

API int contacts_list_create(contacts_list_h *out_list)
{
	ctsvc_list_s *list_s;

	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;

	list_s = calloc(1, sizeof(ctsvc_list_s));
	RETVM_IF(NULL == list_s, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	list_s->l_type = -1;
	*out_list = (contacts_list_h)list_s;
	return CONTACTS_ERROR_NONE;
}

API int contacts_list_get_count(contacts_list_h list, int *count)
{
	ctsvc_list_s *list_s;

	RETV_IF(NULL == count, CONTACTS_ERROR_INVALID_PARAMETER);
	*count = 0;

	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);
	list_s = (ctsvc_list_s*)list;

	*count = list_s->count;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_list_add_child(contacts_list_h list, contacts_record_h child_record)
{
	ctsvc_list_s *s_list;
	ctsvc_record_s *s_record;

	RETV_IF(NULL == list || NULL == child_record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_list = (ctsvc_list_s*)list;
	s_record = (ctsvc_record_s*)child_record;

	if (-1 == s_list->l_type)
		s_list->l_type = s_record->r_type;
	else if (s_list->l_type != s_record->r_type)
		return CONTACTS_ERROR_INVALID_PARAMETER;

	s_list->records = g_list_append(s_list->records, child_record);

	if (s_list->count == 0)
		s_list->cursor = s_list->records;

	s_list->count++;
	return CONTACTS_ERROR_NONE;
}

API int contacts_list_add(contacts_list_h list, contacts_record_h child_record)
{
	ctsvc_list_s *s_list;
	ctsvc_record_s *s_record;

	RETV_IF(NULL == list || NULL == child_record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_list = (ctsvc_list_s*)list;
	s_record = (ctsvc_record_s*)child_record;

	if (-1 == s_list->l_type)
		s_list->l_type = s_record->r_type;
	else if (s_list->l_type != s_record->r_type)
		return CONTACTS_ERROR_INVALID_PARAMETER;

	s_list->records = g_list_append(s_list->records, child_record);

	if (s_list->count == 0)
		s_list->cursor = s_list->records;

	s_list->count++;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_list_prepend(contacts_list_h list, contacts_record_h child_record)
{
	ctsvc_list_s *s_list;
	ctsvc_record_s *s_record;

	RETV_IF(NULL == list || NULL == child_record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_list = (ctsvc_list_s*)list;
	s_record = (ctsvc_record_s*)child_record;

	if (-1 == s_list->l_type)
		s_list->l_type = s_record->r_type;
	else if (s_list->l_type != s_record->r_type)
		return CONTACTS_ERROR_INVALID_PARAMETER;

	s_list->records = g_list_prepend(s_list->records, child_record);

	if (s_list->count == 0)
		s_list->cursor = s_list->records;

	s_list->count++;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_list_reverse(contacts_list_h list)
{
	ctsvc_list_s *s_list;

	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);
	s_list = (ctsvc_list_s*)list;

	if (s_list->count != 0)
		s_list->records =  g_list_reverse(s_list->records);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_list_remove_child(contacts_list_h list, contacts_record_h record,
		bool insert_delete_list)
{
	GList *cursor = NULL;
	ctsvc_list_s *s_list;
	ctsvc_record_s *s_record;
	contacts_record_h delete_record;
	contacts_error_e err = CONTACTS_ERROR_NONE;

	RETV_IF(NULL == list || NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_list = (ctsvc_list_s*)list;
	s_record = (ctsvc_record_s*)record;

	if (s_list->l_type != s_record->r_type)
		return CONTACTS_ERROR_INVALID_PARAMETER;

	for (cursor = s_list->records; cursor; cursor = cursor->next) {
		ctsvc_record_s *data = (ctsvc_record_s*)cursor->data;
		if (data == s_record) {
			s_list->count--;
			if (s_list->cursor == cursor)
				s_list->cursor = cursor->next;

			s_list->records = g_list_remove(s_list->records, s_record);
			if (insert_delete_list) {
				err = contacts_record_clone(record, &delete_record);
				if (CONTACTS_ERROR_NONE != err) {
					CTS_ERR("contacts_record_clone() Fail(%d)", err);
					return err;
				}

				s_list->deleted_records = g_list_append(s_list->deleted_records, delete_record);
			}
			return CONTACTS_ERROR_NONE;
		}
	}

	return CONTACTS_ERROR_NO_DATA;
}

API int contacts_list_remove(contacts_list_h list, contacts_record_h record)
{
	GList *cursor = NULL;
	ctsvc_list_s *s_list;
	ctsvc_record_s *s_record;

	RETV_IF(NULL == list || NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_list = (ctsvc_list_s*)list;
	s_record = (ctsvc_record_s*)record;

	if (s_list->l_type != s_record->r_type)
		return CONTACTS_ERROR_INVALID_PARAMETER;

	for (cursor = s_list->records; cursor; cursor = cursor->next) {
		ctsvc_record_s *data = (ctsvc_record_s*)cursor->data;
		if (data == s_record) {
			s_list->count--;
			if (s_list->cursor == cursor)
				s_list->cursor = cursor->next;
			s_list->records = g_list_remove(s_list->records, s_record);
			return CONTACTS_ERROR_NONE;
		}
	}

	return CONTACTS_ERROR_NO_DATA;
}

API int contacts_list_get_current_record_p(contacts_list_h list, contacts_record_h *record)
{
	ctsvc_list_s *list_s;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	*record = NULL;

	RETV_IF(NULL == list || NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	list_s = (ctsvc_list_s*)list;

	if (NULL == list_s->cursor) {
		*record = NULL;
		return CONTACTS_ERROR_NO_DATA;
	}

	*record = list_s->cursor->data;

	return CONTACTS_ERROR_NONE;
}

int ctsvc_list_get_nth_record_p(contacts_list_h list, int index, contacts_record_h *record)
{
	GList *cursor = NULL;
	ctsvc_list_s *list_s;
	int i, j;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	*record = NULL;

	RETV_IF(NULL == list || NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	list_s = (ctsvc_list_s*)list;

	for (i = 0, j = 0, cursor = list_s->records; cursor; cursor = cursor->next, i++) {
		if (j == index) {
			*record = (contacts_record_h)cursor->data;
			return CONTACTS_ERROR_NONE;
		}
		j++;
	}
	*record = NULL;
	return CONTACTS_ERROR_NO_DATA;
}

static int __ctsvc_list_move_cursor(contacts_list_h list, bool next)
{
	ctsvc_list_s *list_s;

	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);
	list_s = (ctsvc_list_s*)list;

	if (NULL == list_s->cursor)
		return CONTACTS_ERROR_NO_DATA;

	list_s->cursor = next ? list_s->cursor->next : list_s->cursor->prev ;

	if (NULL == list_s->cursor)
		return CONTACTS_ERROR_NO_DATA;

	return CONTACTS_ERROR_NONE;
}

API int contacts_list_prev(contacts_list_h list)
{
	return __ctsvc_list_move_cursor(list, false);
}

API int contacts_list_next(contacts_list_h list)
{
	return __ctsvc_list_move_cursor(list, true);
}



API int contacts_list_first(contacts_list_h list)
{
	ctsvc_list_s *list_s;

	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);
	list_s = (ctsvc_list_s*)list;

	list_s->cursor = list_s->records;
	if (NULL == list_s->cursor)
		return CONTACTS_ERROR_NO_DATA;

	return CONTACTS_ERROR_NONE;
}

API int contacts_list_last(contacts_list_h list)
{
	ctsvc_list_s *list_s;

	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);
	list_s = (ctsvc_list_s*)list;

	list_s->cursor = g_list_last(list_s->records);
	if (NULL == list_s->cursor)
		return CONTACTS_ERROR_NO_DATA;

	return CONTACTS_ERROR_NONE;
}

API int contacts_list_destroy(contacts_list_h list, bool delete_child)
{
	ctsvc_list_s *s_list;
	GList *cursor = NULL;

	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);

	s_list = (ctsvc_list_s*)list;

	if (delete_child) {
		for (cursor = s_list->records; cursor; cursor = cursor->next)
			contacts_record_destroy((contacts_record_h)(cursor->data), true);
	}
	g_list_free(s_list->records);

	for (cursor = s_list->deleted_records; cursor; cursor = cursor->next)
		contacts_record_destroy((contacts_record_h)(cursor->data), true);
	g_list_free(s_list->deleted_records);

	free(s_list);
	return CONTACTS_ERROR_NONE;
}

int ctsvc_list_clone(contacts_list_h list, contacts_list_h *out_list)
{
	ctsvc_list_s *list_s;
	contacts_record_h new_record;
	contacts_list_h new_list;
	GList *cursor = NULL;
	contacts_error_e err = CONTACTS_ERROR_NONE;

	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);

	list_s = (ctsvc_list_s*)list;

	contacts_list_create(&new_list);
	for (cursor = list_s->records; cursor; cursor = cursor->next) {
		err = contacts_record_clone((contacts_record_h)cursor->data, &new_record);
		if (CONTACTS_ERROR_NONE != err) {
			CTS_ERR("contacts_record_clone() Fail(%d)", err);
			contacts_list_destroy(new_list, true);
			return err;
		}
		ctsvc_list_prepend(new_list, new_record);
	}
	ctsvc_list_reverse(new_list);

	for (cursor = list_s->deleted_records; cursor; cursor = cursor->next) {
		err = contacts_record_clone((contacts_record_h)cursor->data, &new_record);
		if (CONTACTS_ERROR_NONE != err) {
			CTS_ERR("contacts_record_clone() Fail(%d)", err);
			contacts_list_destroy(new_list, true);
			return err;
		}
		((ctsvc_list_s*)new_list)->deleted_records
			= g_list_prepend(((ctsvc_list_s*)new_list)->deleted_records, new_record);
	}
	if (((ctsvc_list_s*)new_list)->deleted_records) {
		((ctsvc_list_s*)new_list)->deleted_records
			= g_list_reverse(((ctsvc_list_s*)new_list)->deleted_records);
	}

	*out_list = new_list;

	return CONTACTS_ERROR_NONE;
}


int ctsvc_list_get_deleted_count(contacts_list_h list, unsigned int *count)
{
	ctsvc_list_s *list_s;

	RETV_IF(NULL == count, CONTACTS_ERROR_INVALID_PARAMETER);
	*count = 0;

	RETV_IF(NULL == list, CONTACTS_ERROR_INVALID_PARAMETER);
	list_s = (ctsvc_list_s*)list;

	if (NULL == list_s->deleted_records)
		return CONTACTS_ERROR_NONE;

	*count = g_list_length(list_s->deleted_records);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_list_get_deleted_nth_record_p(contacts_list_h list, int index,
		contacts_record_h *record)
{
	ctsvc_list_s *list_s;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	*record = NULL;

	RETV_IF(NULL == list || NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	list_s = (ctsvc_list_s*)list;

	RETV_IF(NULL == list_s->deleted_records, CONTACTS_ERROR_NO_DATA);

	*record = (contacts_record_h)g_list_nth_data(list_s->deleted_records, index);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_list_append_deleted_record(contacts_list_h list, contacts_record_h record)
{
	ctsvc_list_s *list_s;

	RETV_IF(NULL == list || NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);

	list_s = (ctsvc_list_s*)list;

	list_s->deleted_records = g_list_append(list_s->deleted_records, record);

	return CONTACTS_ERROR_NONE;
}

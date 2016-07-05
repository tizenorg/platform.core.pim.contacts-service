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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_record.h"
#include "ctsvc_struct.h"
#include "ctsvc_view.h"

extern ctsvc_record_plugin_cb_s addressbook_plugin_cbs;
extern ctsvc_record_plugin_cb_s group_plugin_cbs;
extern ctsvc_record_plugin_cb_s person_plugin_cbs;
extern ctsvc_record_plugin_cb_s contact_plugin_cbs;
extern ctsvc_record_plugin_cb_s my_profile_plugin_cbs;
extern ctsvc_record_plugin_cb_s simple_contact_plugin_cbs;
extern ctsvc_record_plugin_cb_s updated_info_plugin_cbs;

extern ctsvc_record_plugin_cb_s name_plugin_cbs;
extern ctsvc_record_plugin_cb_s number_plugin_cbs;
extern ctsvc_record_plugin_cb_s address_plugin_cbs;
extern ctsvc_record_plugin_cb_s url_plugin_cbs;
extern ctsvc_record_plugin_cb_s event_plugin_cbs;
extern ctsvc_record_plugin_cb_s messenger_plugin_cbs;
extern ctsvc_record_plugin_cb_s activity_plugin_cbs;
extern ctsvc_record_plugin_cb_s activity_photo_plugin_cbs;
extern ctsvc_record_plugin_cb_s relationship_plugin_cbs;
extern ctsvc_record_plugin_cb_s image_plugin_cbs;
extern ctsvc_record_plugin_cb_s group_relation_plugin_cbs;
extern ctsvc_record_plugin_cb_s note_plugin_cbs;
extern ctsvc_record_plugin_cb_s company_plugin_cbs;
extern ctsvc_record_plugin_cb_s profile_plugin_cbs;
extern ctsvc_record_plugin_cb_s nickname_plugin_cbs;
extern ctsvc_record_plugin_cb_s email_plugin_cbs;
extern ctsvc_record_plugin_cb_s result_plugin_cbs;
extern ctsvc_record_plugin_cb_s sdn_plugin_cbs;
extern ctsvc_record_plugin_cb_s speeddial_plugin_cbs;
extern ctsvc_record_plugin_cb_s extension_plugin_cbs;
#ifdef ENABLE_LOG_FEATURE
extern ctsvc_record_plugin_cb_s phonelog_plugin_cbs;
#endif /* ENABLE_LOG_FEATURE */
extern ctsvc_record_plugin_cb_s sip_plugin_cbs;

static const ctsvc_record_plugin_cb_s *__ctsvc_record_get_plugin_cb(int r_type)
{
	switch ((int)r_type) {
	case CTSVC_RECORD_ADDRESSBOOK:
		return &addressbook_plugin_cbs;
	case CTSVC_RECORD_GROUP:
		return &group_plugin_cbs;
	case CTSVC_RECORD_PERSON:
		return &person_plugin_cbs;
	case CTSVC_RECORD_CONTACT:
		return &contact_plugin_cbs;
	case CTSVC_RECORD_MY_PROFILE:
		return &my_profile_plugin_cbs;
	case CTSVC_RECORD_SIMPLE_CONTACT:
		return &simple_contact_plugin_cbs;
	case CTSVC_RECORD_NAME:
		return &name_plugin_cbs;
	case CTSVC_RECORD_COMPANY:
		return &company_plugin_cbs;
	case CTSVC_RECORD_NOTE:
		return &note_plugin_cbs;
	case CTSVC_RECORD_NUMBER:
		return &number_plugin_cbs;
	case CTSVC_RECORD_EMAIL:
		return &email_plugin_cbs;
	case CTSVC_RECORD_URL:
		return &url_plugin_cbs;
	case CTSVC_RECORD_EVENT:
		return &event_plugin_cbs;
	case CTSVC_RECORD_NICKNAME:
		return &nickname_plugin_cbs;
	case CTSVC_RECORD_ADDRESS:
		return &address_plugin_cbs;
	case CTSVC_RECORD_MESSENGER:
		return &messenger_plugin_cbs;
	case CTSVC_RECORD_GROUP_RELATION:
		return &group_relation_plugin_cbs;
	case CTSVC_RECORD_ACTIVITY:
		return &activity_plugin_cbs;
	case CTSVC_RECORD_ACTIVITY_PHOTO:
		return &activity_photo_plugin_cbs;
	case CTSVC_RECORD_PROFILE:
		return &profile_plugin_cbs;
	case CTSVC_RECORD_RELATIONSHIP:
		return &relationship_plugin_cbs;
	case CTSVC_RECORD_IMAGE:
		return &image_plugin_cbs;
	case CTSVC_RECORD_EXTENSION:
		return &extension_plugin_cbs;
#ifdef ENABLE_LOG_FEATURE
	case CTSVC_RECORD_PHONELOG:
		return &phonelog_plugin_cbs;
#endif /* ENABLE_LOG_FEATURE */
	case CTSVC_RECORD_SPEEDDIAL:
		return &speeddial_plugin_cbs;
	case CTSVC_RECORD_SDN:
		return &sdn_plugin_cbs;
	case CTSVC_RECORD_UPDATED_INFO:
		return &updated_info_plugin_cbs;
	case CTSVC_RECORD_RESULT:
		return &result_plugin_cbs;
	case CTSVC_RECORD_SIP:
		return &sip_plugin_cbs;
	default:
		return NULL;
	}
}

#define __INVALID_PARAMETER_ERROR_HANDLING() \
	ERR("Invalid parameter: Operation restricted."); \
return CONTACTS_ERROR_INVALID_PARAMETER;

/*
 * This function is used for view_uri which is able to CRUD.
 * The view_uri's property should be sequencial value because it is used to find index at the below logic.
 */
bool ctsvc_record_check_property_flag(const ctsvc_record_s *s_record, unsigned int property_id, contacts_property_flag_e flag)
{
	int index = property_id & 0x000000FF;

	if (CTSVC_RECORD_RESULT == s_record->r_type)
		return true;

	/*
	 * Check it when getting value of property
	 * property_flag and properties_flags is set when getting record with query
	 */
	if (CTSVC_PROPERTY_FLAG_PROJECTION == flag) {
		/* all property get. */
		if (NULL == s_record->properties_flags)
			return true;
		/*
		 * Or before inserting record from DB, just get after setting.
		 * properties_flags is not NULL when just setting dirty
		 */
		if (0 == (CTSVC_PROPERTY_FLAG_PROJECTION & s_record->property_flag))
			return true;
	}

	/* Check it when updating record */
	if (CTSVC_PROPERTY_FLAG_DIRTY == flag) {
		/* all property is clean */
		if (NULL == s_record->properties_flags)
			return false;
	}
	return (s_record->properties_flags[index] & flag) ? true : false;
}

int ctsvc_record_set_property_flag(ctsvc_record_s *_record, int property_id, contacts_property_flag_e flag)
{
	int index = property_id & 0x000000FF;

	if (CTSVC_RECORD_RESULT == _record->r_type)
		return CONTACTS_ERROR_NONE;

	if (NULL == _record->properties_flags) {
		unsigned int count = 0;
		ctsvc_view_get_all_property_infos(_record->view_uri, &count);
		RETVM_IF(count <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "ctsvc_view_get_all_property_infos() Fail");

		_record->properties_flags = calloc(count, sizeof(char));
		_record->property_max_count = count;
		RETVM_IF(NULL == _record->properties_flags, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");
	}
	_record->property_flag |= flag;
	_record->properties_flags[index] |= flag;

	return CONTACTS_ERROR_NONE;
}

#define __CHECK_READ_ONLY_PROPERTY() \
	if (CTSVC_READ_ONLY_CHECK(property_id, CTSVC_READ_ONLY_PROPERTY)) { \
		ERR("Don't try to change read-only property.(0x%0x)", property_id); \
		return CONTACTS_ERROR_INVALID_PARAMETER; \
	}

#define __CHECK_PROJECTED_PROPERTY() \
	if (false == ctsvc_record_check_property_flag(s_record, property_id, CTSVC_PROPERTY_FLAG_PROJECTION)) { \
		ERR("Don't try to get un-projected property(0x%0x).", property_id); \
		return CONTACTS_ERROR_INVALID_PARAMETER; \
	}

/* Record constuct/destruct */
API int contacts_record_create(const char *view_uri, contacts_record_h *out_record)
{
	int ret;
	ctsvc_record_type_e r_type;
	const ctsvc_record_plugin_cb_s *plugin_cb;

	RETV_IF(NULL == view_uri, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	r_type = ctsvc_view_get_record_type(view_uri);
	RETVM_IF(CTSVC_RECORD_INVALID == r_type, CONTACTS_ERROR_INVALID_PARAMETER,
			"view_uri(%s)", view_uri);

	plugin_cb = __ctsvc_record_get_plugin_cb(r_type);
	if (plugin_cb && plugin_cb->create) {
		ret = plugin_cb->create(out_record);
		if (CONTACTS_ERROR_NONE == ret)
			CTSVC_RECORD_INIT_BASE((ctsvc_record_s*)*out_record, r_type, plugin_cb, ctsvc_view_get_uri(view_uri));

		return ret;
	}

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	if (s_record && s_record->plugin_cbs && s_record->plugin_cbs->destroy)
		return s_record->plugin_cbs->destroy(record, delete_child);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	if (s_record->plugin_cbs && s_record->plugin_cbs->clone)
		return s_record->plugin_cbs->clone(record, out_record);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_get_uri_p(contacts_record_h record, const char **out_str)
{
	int ret = CONTACTS_ERROR_NONE;

	ctsvc_record_s *temp = (ctsvc_record_s*)(record);

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == out_str, CONTACTS_ERROR_INVALID_PARAMETER);

	*out_str = (temp->view_uri);

	return ret;
}

/* Record get/set int,str, etc.. */
API int contacts_record_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == out_str, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_str = NULL;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	__CHECK_PROJECTED_PROPERTY();

	if (s_record->plugin_cbs && s_record->plugin_cbs->get_str)
		return s_record->plugin_cbs->get_str(record, property_id, out_str);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_get_lli(contacts_record_h record, unsigned int property_id, long long int *value)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == value, CONTACTS_ERROR_INVALID_PARAMETER);
	*value = 0;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	__CHECK_PROJECTED_PROPERTY();

	if (s_record->plugin_cbs && s_record->plugin_cbs->get_lli)
		return s_record->plugin_cbs->get_lli(record, property_id, value);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_get_double(contacts_record_h record, unsigned int property_id, double *value)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == value, CONTACTS_ERROR_INVALID_PARAMETER);
	*value = 0;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	__CHECK_PROJECTED_PROPERTY();

	if (s_record->plugin_cbs && s_record->plugin_cbs->get_double)
		return s_record->plugin_cbs->get_double(record, property_id, value);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == out_str, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_str = NULL;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	__CHECK_PROJECTED_PROPERTY();

	if (s_record->plugin_cbs && s_record->plugin_cbs->get_str_p)
		return s_record->plugin_cbs->get_str_p(record, property_id, out_str);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_get_int(contacts_record_h record, unsigned int property_id, int *out_value)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == out_value, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_value = 0;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	__CHECK_PROJECTED_PROPERTY();

	if (s_record->plugin_cbs && s_record->plugin_cbs->get_int)
		return s_record->plugin_cbs->get_int(record, property_id, out_value);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_set_str(contacts_record_h record, unsigned int property_id, const char *value)
{
	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);

	__CHECK_READ_ONLY_PROPERTY();

	return ctsvc_record_set_str(record, property_id, value);
}

int ctsvc_record_set_str(contacts_record_h record, unsigned int property_id, const char *value)
{
	char *str;
	ctsvc_record_s *s_record;
	int ret;

	s_record = (ctsvc_record_s*)record;
	__CHECK_PROJECTED_PROPERTY();

	if (value && *value)
		str = (char *)value;
	else
		str = NULL;

	if (s_record->plugin_cbs && s_record->plugin_cbs->set_str) {
		bool is_dirty = false;
		ret = s_record->plugin_cbs->set_str(record, property_id, str, &is_dirty);
		if (CONTACTS_ERROR_NONE == ret && is_dirty)
			ctsvc_record_set_property_flag(s_record, property_id, CTSVC_PROPERTY_FLAG_DIRTY);

		return ret;
	}

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_get_bool(contacts_record_h record,
		unsigned int property_id, bool *value)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == value, CONTACTS_ERROR_INVALID_PARAMETER);
	*value = false;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	__CHECK_PROJECTED_PROPERTY();

	if (s_record->plugin_cbs && s_record->plugin_cbs->get_bool)
		return s_record->plugin_cbs->get_bool(record, property_id, value);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_set_bool(contacts_record_h record,
		unsigned int property_id, bool value)
{
	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);

	__CHECK_READ_ONLY_PROPERTY();

	return ctsvc_record_set_bool(record, property_id, value);
}

int ctsvc_record_set_bool(contacts_record_h record, unsigned int property_id, bool value)
{
	int ret;
	ctsvc_record_s *s_record;
	s_record = (ctsvc_record_s*)record;
	__CHECK_PROJECTED_PROPERTY();

	if (s_record->plugin_cbs && s_record->plugin_cbs->set_bool) {
		bool is_dirty = false;
		ret = s_record->plugin_cbs->set_bool(record, property_id, value, &is_dirty);
		if (CONTACTS_ERROR_NONE == ret && is_dirty)
			ctsvc_record_set_property_flag(s_record, property_id, CTSVC_PROPERTY_FLAG_DIRTY);
		return ret;
	}

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_set_int(contacts_record_h record, unsigned int property_id,
		int value)
{
	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);

	__CHECK_READ_ONLY_PROPERTY();

#ifdef _CONTACTS_IPC_CLIENT
	if (CTSVC_RECORD_RESULT == ((ctsvc_record_s*)record)->r_type) {
		ERR("Can not set int to result record");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
#endif
	return ctsvc_record_set_int(record, property_id, value);
}

int ctsvc_record_set_int(contacts_record_h record, unsigned int property_id, int value)
{
	int ret;
	ctsvc_record_s *s_record;
	s_record = (ctsvc_record_s*)record;
	__CHECK_PROJECTED_PROPERTY();

	if (s_record->plugin_cbs && s_record->plugin_cbs->set_int) {
		bool is_dirty = false;
		ret = s_record->plugin_cbs->set_int(record, property_id, value, &is_dirty);
		if (CONTACTS_ERROR_NONE == ret && is_dirty)
			ctsvc_record_set_property_flag(s_record, property_id, CTSVC_PROPERTY_FLAG_DIRTY);
		return ret;
	}
	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_set_lli(contacts_record_h record, unsigned int property_id,
		long long int value)
{
	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);

	__CHECK_READ_ONLY_PROPERTY();

	return ctsvc_record_set_lli(record, property_id, value);
}

int ctsvc_record_set_lli(contacts_record_h record, unsigned int property_id,
		long long int value)
{
	int ret;
	ctsvc_record_s *s_record;
	s_record = (ctsvc_record_s*)record;
	__CHECK_PROJECTED_PROPERTY();

	if (s_record->plugin_cbs && s_record->plugin_cbs->set_lli) {
		bool is_dirty = false;
		ret = s_record->plugin_cbs->set_lli(record, property_id, value, &is_dirty);
		if (CONTACTS_ERROR_NONE == ret && is_dirty)
			ctsvc_record_set_property_flag(s_record, property_id, CTSVC_PROPERTY_FLAG_DIRTY);
		return ret;
	}

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_set_double(contacts_record_h record, unsigned int property_id,
		double value)
{
	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);

	__CHECK_READ_ONLY_PROPERTY();

	return ctsvc_record_set_double(record, property_id, value);
}

int ctsvc_record_set_double(contacts_record_h record, unsigned int property_id,
		double value)
{
	int ret;
	ctsvc_record_s *s_record;

	s_record = (ctsvc_record_s*)record;
	__CHECK_PROJECTED_PROPERTY();

	if (s_record->plugin_cbs && s_record->plugin_cbs->set_double) {
		bool is_dirty = false;
		ret = s_record->plugin_cbs->set_double(record, property_id, value, &is_dirty);
		if (CONTACTS_ERROR_NONE == ret && is_dirty)
			ctsvc_record_set_property_flag(s_record, property_id, CTSVC_PROPERTY_FLAG_DIRTY);
		return ret;
	}

	__INVALID_PARAMETER_ERROR_HANDLING();
}

/* Record get/set child records */
API int contacts_record_add_child_record(contacts_record_h record,
		unsigned int property_id, contacts_record_h child_record)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child_record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	if (s_record->plugin_cbs && s_record->plugin_cbs->add_child_record)
		return s_record->plugin_cbs->add_child_record(record, property_id, child_record);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_remove_child_record(contacts_record_h record,
		unsigned int property_id, contacts_record_h child_record)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child_record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	if (s_record->plugin_cbs && s_record->plugin_cbs->remove_child_record)
		return s_record->plugin_cbs->remove_child_record(record, property_id, child_record);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_get_child_record_count(contacts_record_h record,
		unsigned int property_id, int *count)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == count, CONTACTS_ERROR_INVALID_PARAMETER);
	*count = 0;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	if (s_record->plugin_cbs && s_record->plugin_cbs->get_child_record_count)
		return s_record->plugin_cbs->get_child_record_count(record, property_id, count);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_get_child_record_at_p(contacts_record_h record,
		unsigned int property_id, int index, contacts_record_h *out_record)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	if (s_record->plugin_cbs && s_record->plugin_cbs->get_child_record_at_p)
		return s_record->plugin_cbs->get_child_record_at_p(record, property_id, index, out_record);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

API int contacts_record_clone_child_record_list(contacts_record_h record,
		unsigned int property_id, contacts_list_h *out_list)
{
	ctsvc_record_s *s_record;

	RETV_IF(NULL == out_list, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_list = NULL;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	s_record = (ctsvc_record_s*)record;

	if (s_record->plugin_cbs && s_record->plugin_cbs->clone_child_record_list)
		return s_record->plugin_cbs->clone_child_record_list(record, property_id, out_list);

	__INVALID_PARAMETER_ERROR_HANDLING();
}

int ctsvc_record_set_projection_flags(contacts_record_h record,
		const unsigned int *projection,
		const unsigned int projection_count,
		const unsigned int property_max_count)
{
	int i;

	RETV_IF(record == NULL, CONTACTS_ERROR_INVALID_PARAMETER);

	ctsvc_record_s *_record = (ctsvc_record_s*)record;

	free(_record->properties_flags);
	_record->properties_flags = NULL;

	_record->properties_flags = calloc(property_max_count, sizeof(char));
	if (NULL == _record->properties_flags) {
		//LCOV_EXCL_START
		ERR("calloc fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
		//LCOV_EXCL_STOP
	}

	_record->property_max_count = property_max_count;

	if (CTSVC_RECORD_RESULT == _record->r_type) {
		_record->property_flag |= CTSVC_PROPERTY_FLAG_PROJECTION;
	} else {
		for (i = 0; i < projection_count; i++)
			ctsvc_record_set_property_flag(_record, projection[i], CTSVC_PROPERTY_FLAG_PROJECTION);
	}

	return CONTACTS_ERROR_NONE;
}


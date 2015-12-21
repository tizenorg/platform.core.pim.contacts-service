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
#include <unistd.h>

#include "contacts.h"

#include "ctsvc_internal.h"
#include "ctsvc_list.h"
#include "ctsvc_record.h"
#include "ctsvc_view.h"

static int __ctsvc_activity_create(contacts_record_h *out_record);
static int __ctsvc_activity_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_activity_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_activity_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_activity_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_activity_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_activity_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_activity_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);
static int __ctsvc_activity_add_child_record(contacts_record_h record, unsigned int property_id, contacts_record_h child_record);
static int __ctsvc_activity_remove_child_record(contacts_record_h record, unsigned int property_id, contacts_record_h child_record);
static int __ctsvc_activity_get_child_record_count(contacts_record_h record, unsigned int property_id, int *count);
static int __ctsvc_activity_get_child_record_at_p(contacts_record_h record, unsigned int property_id, int index, contacts_record_h *out_record);
static int __ctsvc_activity_clone_child_record_list(contacts_record_h record, unsigned int property_id, contacts_list_h *out_list);

static int __ctsvc_activity_photo_create(contacts_record_h *out_record);
static int __ctsvc_activity_photo_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_activity_photo_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_activity_photo_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_activity_photo_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_activity_photo_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_activity_photo_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_activity_photo_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);


static int __ctsvc_address_create(contacts_record_h *out_ecord);
static int __ctsvc_address_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_address_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_address_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_address_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_address_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_address_get_bool(contacts_record_h record, unsigned int property_id, bool *value);
static int __ctsvc_address_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_address_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);
static int __ctsvc_address_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty);

static int __ctsvc_company_create(contacts_record_h *out_record);
static int __ctsvc_company_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_company_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_company_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_company_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_company_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_company_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_company_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);

static int __ctsvc_contact_create(contacts_record_h *out_record);
static int __ctsvc_contact_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_contact_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_contact_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_contact_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_contact_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_contact_get_bool(contacts_record_h record, unsigned int property_id, bool *value);
static int __ctsvc_contact_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_contact_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);
static int __ctsvc_contact_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty);
static int __ctsvc_contact_clone_child_record_list(contacts_record_h record, unsigned int property_id, contacts_list_h *out_list);
static int __ctsvc_contact_get_child_record_at_p(contacts_record_h record, unsigned int property_id, int index, contacts_record_h *out_record);
static int __ctsvc_contact_get_child_record_count(contacts_record_h record, unsigned int property_id, int *count);
static int __ctsvc_contact_add_child_record(contacts_record_h record, unsigned int property_id, contacts_record_h child_record);
static int __ctsvc_contact_remove_child_record(contacts_record_h record, unsigned int property_id, contacts_record_h child_record);

static int __ctsvc_email_create(contacts_record_h *out_record);
static int __ctsvc_email_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_email_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_email_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_email_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_email_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_email_get_bool(contacts_record_h record, unsigned int property_id, bool *value);
static int __ctsvc_email_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_email_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);
static int __ctsvc_email_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty);

static int __ctsvc_event_create(contacts_record_h *out_record);
static int __ctsvc_event_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_event_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_event_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_event_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_event_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_event_get_bool(contacts_record_h record, unsigned int property_id, bool *value);
static int __ctsvc_event_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_event_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);
static int __ctsvc_event_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty);

static int __ctsvc_extension_create(contacts_record_h *out_record);
static int __ctsvc_extension_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_extension_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_extension_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_extension_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_extension_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_extension_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_extension_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);

static int __ctsvc_group_relation_create(contacts_record_h *out_record);
static int __ctsvc_group_relation_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_group_relation_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_group_relation_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_group_relation_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_group_relation_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_group_relation_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_group_relation_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);

static int __ctsvc_messenger_create(contacts_record_h *out_record);
static int __ctsvc_messenger_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_messenger_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_messenger_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_messenger_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_messenger_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_messenger_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_messenger_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);

static int __ctsvc_name_create(contacts_record_h *out_record);
static int __ctsvc_name_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_name_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_name_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_name_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_name_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_name_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_name_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);

static int __ctsvc_nickname_create(contacts_record_h *out_record);
static int __ctsvc_nickname_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_nickname_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_nickname_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_nickname_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_nickname_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_nickname_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_nickname_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);

static int __ctsvc_note_create(contacts_record_h *out_record);
static int __ctsvc_note_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_note_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_note_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_note_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_note_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_note_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_note_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);

static int __ctsvc_number_create(contacts_record_h *out_record);
static int __ctsvc_number_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_number_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_number_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_number_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_number_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_number_get_bool(contacts_record_h record, unsigned int property_id, bool *value);
static int __ctsvc_number_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_number_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);
static int __ctsvc_number_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty);

static int __ctsvc_profile_create(contacts_record_h *out_record);
static int __ctsvc_profile_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_profile_clone(contacts_record_h record, contacts_record_h *out_reord);
static int __ctsvc_profile_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_profile_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_profile_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_profile_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_profile_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);

static int __ctsvc_relationship_create(contacts_record_h *out_record);
static int __ctsvc_relationship_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_relationship_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_relationship_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_relationship_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_relationship_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_relationship_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_relationship_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);

static int __ctsvc_image_create(contacts_record_h *out_record);
static int __ctsvc_image_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_image_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_image_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_image_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_image_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_image_get_bool(contacts_record_h record, unsigned int property_id, bool *value);
static int __ctsvc_image_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_image_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);
static int __ctsvc_image_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty);

static int __ctsvc_simple_contact_create(contacts_record_h *out_record);
static int __ctsvc_simple_contact_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_simple_contact_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_simple_contact_get_bool(contacts_record_h record, unsigned int property_id, bool *out);
static int __ctsvc_simple_contact_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_simple_contact_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_simple_contact_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_simple_contact_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_simple_contact_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);

static int __ctsvc_url_create(contacts_record_h *out_record);
static int __ctsvc_url_destroy(contacts_record_h record, bool delete_child);
static int __ctsvc_url_clone(contacts_record_h record, contacts_record_h *out_record);
static int __ctsvc_url_get_int(contacts_record_h record, unsigned int property_id, int *out);
static int __ctsvc_url_get_str(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_url_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str);
static int __ctsvc_url_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty);
static int __ctsvc_url_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty);

static GHashTable *__ctsvc_temp_image_file_hash_table = NULL;

ctsvc_record_plugin_cb_s name_plugin_cbs = {
	.create = __ctsvc_name_create,
	.destroy = __ctsvc_name_destroy,
	.clone = __ctsvc_name_clone,
	.get_str = __ctsvc_name_get_str,
	.get_str_p = __ctsvc_name_get_str_p,
	.get_int = __ctsvc_name_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_name_set_str,
	.set_int = __ctsvc_name_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s number_plugin_cbs = {
	.create = __ctsvc_number_create,
	.destroy = __ctsvc_number_destroy,
	.clone = __ctsvc_number_clone,
	.get_str = __ctsvc_number_get_str,
	.get_str_p = __ctsvc_number_get_str_p,
	.get_int = __ctsvc_number_get_int,
	.get_bool = __ctsvc_number_get_bool,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_number_set_str,
	.set_int = __ctsvc_number_set_int,
	.set_bool = __ctsvc_number_set_bool,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s address_plugin_cbs = {
	.create = __ctsvc_address_create,
	.destroy = __ctsvc_address_destroy,
	.clone = __ctsvc_address_clone,
	.get_str = __ctsvc_address_get_str,
	.get_str_p = __ctsvc_address_get_str_p,
	.get_int = __ctsvc_address_get_int,
	.get_bool = __ctsvc_address_get_bool,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_address_set_str,
	.set_int = __ctsvc_address_set_int,
	.set_bool = __ctsvc_address_set_bool,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s url_plugin_cbs = {
	.create = __ctsvc_url_create,
	.destroy = __ctsvc_url_destroy,
	.clone = __ctsvc_url_clone,
	.get_str = __ctsvc_url_get_str,
	.get_str_p = __ctsvc_url_get_str_p,
	.get_int = __ctsvc_url_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_url_set_str,
	.set_int = __ctsvc_url_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s event_plugin_cbs = {
	.create = __ctsvc_event_create,
	.destroy = __ctsvc_event_destroy,
	.clone = __ctsvc_event_clone,
	.get_str = __ctsvc_event_get_str,
	.get_str_p = __ctsvc_event_get_str_p,
	.get_int = __ctsvc_event_get_int,
	.get_bool = __ctsvc_event_get_bool,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_event_set_str,
	.set_int = __ctsvc_event_set_int,
	.set_bool = __ctsvc_event_set_bool,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s messenger_plugin_cbs = {
	.create = __ctsvc_messenger_create,
	.destroy = __ctsvc_messenger_destroy,
	.clone = __ctsvc_messenger_clone,
	.get_str = __ctsvc_messenger_get_str,
	.get_str_p = __ctsvc_messenger_get_str_p,
	.get_int = __ctsvc_messenger_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_messenger_set_str,
	.set_int = __ctsvc_messenger_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s activity_plugin_cbs = {
	.create = __ctsvc_activity_create,
	.destroy = __ctsvc_activity_destroy,
	.clone = __ctsvc_activity_clone,
	.get_str = __ctsvc_activity_get_str,
	.get_str_p = __ctsvc_activity_get_str_p,
	.get_int = __ctsvc_activity_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_activity_set_str,
	.set_int = __ctsvc_activity_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = __ctsvc_activity_add_child_record,
	.remove_child_record = __ctsvc_activity_remove_child_record,
	.get_child_record_count = __ctsvc_activity_get_child_record_count,
	.get_child_record_at_p = __ctsvc_activity_get_child_record_at_p,
	.clone_child_record_list = __ctsvc_activity_clone_child_record_list,
};

ctsvc_record_plugin_cb_s activity_photo_plugin_cbs = {
	.create = __ctsvc_activity_photo_create,
	.destroy = __ctsvc_activity_photo_destroy,
	.clone = __ctsvc_activity_photo_clone,
	.get_str = __ctsvc_activity_photo_get_str,
	.get_str_p = __ctsvc_activity_photo_get_str_p,
	.get_int = __ctsvc_activity_photo_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_activity_photo_set_str,
	.set_int = __ctsvc_activity_photo_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s relationship_plugin_cbs = {
	.create = __ctsvc_relationship_create,
	.destroy = __ctsvc_relationship_destroy,
	.clone = __ctsvc_relationship_clone,
	.get_str = __ctsvc_relationship_get_str,
	.get_str_p = __ctsvc_relationship_get_str_p,
	.get_int = __ctsvc_relationship_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_relationship_set_str,
	.set_int = __ctsvc_relationship_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s image_plugin_cbs = {
	.create = __ctsvc_image_create,
	.destroy = __ctsvc_image_destroy,
	.clone = __ctsvc_image_clone,
	.get_str = __ctsvc_image_get_str,
	.get_str_p = __ctsvc_image_get_str_p,
	.get_int = __ctsvc_image_get_int,
	.get_bool = __ctsvc_image_get_bool,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_image_set_str,
	.set_int = __ctsvc_image_set_int,
	.set_bool = __ctsvc_image_set_bool,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s group_relation_plugin_cbs = {
	.create = __ctsvc_group_relation_create,
	.destroy = __ctsvc_group_relation_destroy,
	.clone = __ctsvc_group_relation_clone,
	.get_str = __ctsvc_group_relation_get_str,
	.get_str_p = __ctsvc_group_relation_get_str_p,
	.get_int = __ctsvc_group_relation_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_group_relation_set_str,
	.set_int = __ctsvc_group_relation_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s note_plugin_cbs = {
	.create = __ctsvc_note_create,
	.destroy = __ctsvc_note_destroy,
	.clone = __ctsvc_note_clone,
	.get_str = __ctsvc_note_get_str,
	.get_str_p = __ctsvc_note_get_str_p,
	.get_int = __ctsvc_note_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_note_set_str,
	.set_int = __ctsvc_note_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s company_plugin_cbs = {
	.create = __ctsvc_company_create,
	.destroy = __ctsvc_company_destroy,
	.clone = __ctsvc_company_clone,
	.get_str = __ctsvc_company_get_str,
	.get_str_p = __ctsvc_company_get_str_p,
	.get_int = __ctsvc_company_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_company_set_str,
	.set_int = __ctsvc_company_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s profile_plugin_cbs = {
	.create = __ctsvc_profile_create,
	.destroy = __ctsvc_profile_destroy,
	.clone = __ctsvc_profile_clone,
	.get_str = __ctsvc_profile_get_str,
	.get_str_p = __ctsvc_profile_get_str_p,
	.get_int = __ctsvc_profile_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_profile_set_str,
	.set_int = __ctsvc_profile_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s nickname_plugin_cbs = {
	.create = __ctsvc_nickname_create,
	.destroy = __ctsvc_nickname_destroy,
	.clone = __ctsvc_nickname_clone,
	.get_str = __ctsvc_nickname_get_str,
	.get_str_p = __ctsvc_nickname_get_str_p,
	.get_int = __ctsvc_nickname_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_nickname_set_str,
	.set_int = __ctsvc_nickname_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s email_plugin_cbs = {
	.create = __ctsvc_email_create,
	.destroy = __ctsvc_email_destroy,
	.clone = __ctsvc_email_clone,
	.get_str = __ctsvc_email_get_str,
	.get_str_p = __ctsvc_email_get_str_p,
	.get_int = __ctsvc_email_get_int,
	.get_bool = __ctsvc_email_get_bool,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_email_set_str,
	.set_int = __ctsvc_email_set_int,
	.set_bool = __ctsvc_email_set_bool,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s extension_plugin_cbs = {
	.create = __ctsvc_extension_create,
	.destroy = __ctsvc_extension_destroy,
	.clone = __ctsvc_extension_clone,
	.get_str = __ctsvc_extension_get_str,
	.get_str_p = __ctsvc_extension_get_str_p,
	.get_int = __ctsvc_extension_get_int,
	.get_bool = NULL,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_extension_set_str,
	.set_int = __ctsvc_extension_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

ctsvc_record_plugin_cb_s contact_plugin_cbs = {
	.create = __ctsvc_contact_create,
	.destroy = __ctsvc_contact_destroy,
	.clone = __ctsvc_contact_clone,
	.get_str = __ctsvc_contact_get_str,
	.get_str_p = __ctsvc_contact_get_str_p,
	.get_int = __ctsvc_contact_get_int,
	.get_bool = __ctsvc_contact_get_bool,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_contact_set_str,
	.set_int = __ctsvc_contact_set_int,
	.set_bool = __ctsvc_contact_set_bool,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = __ctsvc_contact_add_child_record,
	.remove_child_record = __ctsvc_contact_remove_child_record,
	.get_child_record_count = __ctsvc_contact_get_child_record_count,
	.get_child_record_at_p = __ctsvc_contact_get_child_record_at_p,
	.clone_child_record_list = __ctsvc_contact_clone_child_record_list,
};

ctsvc_record_plugin_cb_s simple_contact_plugin_cbs = {
	.create = __ctsvc_simple_contact_create,
	.destroy = __ctsvc_simple_contact_destroy,
	.clone = __ctsvc_simple_contact_clone,
	.get_str = __ctsvc_simple_contact_get_str,
	.get_str_p = __ctsvc_simple_contact_get_str_p,
	.get_int = __ctsvc_simple_contact_get_int,
	.get_bool = __ctsvc_simple_contact_get_bool,
	.get_lli = NULL,
	.get_double = NULL,
	.set_str = __ctsvc_simple_contact_set_str,
	.set_int = __ctsvc_simple_contact_set_int,
	.set_bool = NULL,
	.set_lli = NULL,
	.set_double = NULL,
	.add_child_record = NULL,
	.remove_child_record = NULL,
	.get_child_record_count = NULL,
	.get_child_record_at_p = NULL,
	.clone_child_record_list = NULL,
};

static int __ctsvc_activity_create(contacts_record_h *out_record)
{
	ctsvc_activity_s *activity;
	activity = calloc(1, sizeof(ctsvc_activity_s));
	RETVM_IF(NULL == activity, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	activity->photos = calloc(1, sizeof(ctsvc_list_s));
	if (NULL == activity->photos) {
		CTS_ERR("calloc() Fail");
		free(activity);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}
	activity->photos->l_type = CTSVC_RECORD_ACTIVITY_PHOTO;

	*out_record = (contacts_record_h)activity;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_photo_create(contacts_record_h *out_record)
{
	ctsvc_activity_photo_s *photo;
	photo = calloc(1, sizeof(ctsvc_activity_photo_s));
	RETVM_IF(NULL == photo, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)photo;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_address_create(contacts_record_h *out_record)
{
	ctsvc_address_s *address;
	address = calloc(1, sizeof(ctsvc_address_s));
	RETVM_IF(NULL == address, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)address;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_company_create(contacts_record_h *out_record)
{
	ctsvc_company_s *company;
	company = calloc(1, sizeof(ctsvc_company_s));
	RETVM_IF(NULL == company, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)company;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_email_create(contacts_record_h *out_record)
{
	ctsvc_email_s *email;
	email = calloc(1, sizeof(ctsvc_email_s));
	RETVM_IF(NULL == email, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)email;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_event_create(contacts_record_h *out_record)
{
	ctsvc_event_s *event;
	event = calloc(1, sizeof(ctsvc_event_s));
	RETVM_IF(NULL == event, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)event;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_extension_create(contacts_record_h *out_record)
{
	ctsvc_extension_s *extension;
	extension = calloc(1, sizeof(ctsvc_extension_s));
	RETVM_IF(NULL == extension, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)extension;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_relation_create(contacts_record_h *out_record)
{
	ctsvc_group_relation_s *group_relation;
	group_relation = calloc(1, sizeof(ctsvc_group_relation_s));
	RETVM_IF(NULL == group_relation, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)group_relation;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_messenger_create(contacts_record_h *out_record)
{
	ctsvc_messenger_s *messenger;
	messenger = calloc(1, sizeof(ctsvc_messenger_s));
	RETVM_IF(NULL == messenger, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)messenger;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_name_create(contacts_record_h *out_record)
{
	ctsvc_name_s *name;
	name = calloc(1, sizeof(ctsvc_name_s));
	RETVM_IF(NULL == name, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)name;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_nickname_create(contacts_record_h *out_record)
{
	ctsvc_nickname_s *nickname;
	nickname = calloc(1, sizeof(ctsvc_nickname_s));
	RETVM_IF(NULL == nickname, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)nickname;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_note_create(contacts_record_h *out_record)
{
	ctsvc_note_s *note;
	note = calloc(1, sizeof(ctsvc_note_s));
	RETVM_IF(NULL == note, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)note;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_number_create(contacts_record_h *out_record)
{
	ctsvc_number_s *number;
	number = calloc(1, sizeof(ctsvc_number_s));
	RETVM_IF(NULL == number, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)number;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_profile_create(contacts_record_h *out_record)
{
	ctsvc_profile_s *profile;
	profile = calloc(1, sizeof(ctsvc_profile_s));
	RETVM_IF(NULL == profile, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)profile;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_relationship_create(contacts_record_h *out_record)
{
	ctsvc_relationship_s *relationship;
	relationship = calloc(1, sizeof(ctsvc_relationship_s));
	RETVM_IF(NULL == relationship, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)relationship;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_image_create(contacts_record_h *out_record)
{
	ctsvc_image_s *image;
	image = calloc(1, sizeof(ctsvc_image_s));
	RETVM_IF(NULL == image, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)image;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_simple_contact_create(contacts_record_h *out_record)
{
	ctsvc_simple_contact_s *simple_contact;
	simple_contact = calloc(1, sizeof(ctsvc_simple_contact_s));
	RETVM_IF(NULL == simple_contact, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)simple_contact;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_url_create(contacts_record_h *out_record)
{
	ctsvc_url_s *url;
	url = calloc(1, sizeof(ctsvc_url_s));
	RETVM_IF(NULL == url, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	*out_record = (contacts_record_h)url;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_name_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_name_s *name = (ctsvc_name_s*)record;
	name->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(name->base.properties_flags);

	free(name->first);
	free(name->last);
	free(name->addition);
	free(name->prefix);
	free(name->suffix);
	free(name->phonetic_first);
	free(name->phonetic_middle);
	free(name->phonetic_last);
	free(name->lookup);
	free(name->reverse_lookup);
	free(name);

	return CONTACTS_ERROR_NONE;
};

static void __ctsvc_temp_image_hash_table_insert(char *filename)
{
	int count = 0;
	if (NULL == __ctsvc_temp_image_file_hash_table)
		__ctsvc_temp_image_file_hash_table = g_hash_table_new(g_str_hash, g_str_equal);

	count = GPOINTER_TO_INT(g_hash_table_lookup(__ctsvc_temp_image_file_hash_table, filename));
	g_hash_table_insert(__ctsvc_temp_image_file_hash_table, filename, GINT_TO_POINTER(count+1));
}

static void __ctsvc_temp_image_hash_table_remove(char *filename)
{
	int count = 0;

	if (NULL == __ctsvc_temp_image_file_hash_table) {
		if (unlink(filename) < 0)
			CTS_WARN("unlink Fail(%d)", errno);
		return;
	}

	count = GPOINTER_TO_INT(g_hash_table_lookup(__ctsvc_temp_image_file_hash_table, filename));
	if (count < 1) {
		if (unlink(filename) < 0)
			CTS_WARN("unlink Fail(%d)", errno);
	} else if (1 == count) {
		g_hash_table_remove(__ctsvc_temp_image_file_hash_table, filename);
		if (0 == g_hash_table_size(__ctsvc_temp_image_file_hash_table)) {
			g_hash_table_destroy(__ctsvc_temp_image_file_hash_table);
			__ctsvc_temp_image_file_hash_table = NULL;
		}
	} else {
		g_hash_table_insert(__ctsvc_temp_image_file_hash_table, filename, GINT_TO_POINTER(count-1));
	}
}

static int __ctsvc_company_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_company_s *company = (ctsvc_company_s*)record;
	company->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(company->base.properties_flags);

	free(company->label);
	free(company->name);
	free(company->department);
	free(company->job_title);
	free(company->role);
	free(company->assistant_name);
	if (company->logo && company->is_vcard)
		__ctsvc_temp_image_hash_table_remove(company->logo);
	free(company->logo);
	free(company->location);
	free(company->description);
	free(company->phonetic_name);
	free(company);

	return CONTACTS_ERROR_NONE;
};

static int __ctsvc_note_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_note_s *note = (ctsvc_note_s*)record;
	note->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(note->base.properties_flags);

	free(note->note);
	free(note);

	return CONTACTS_ERROR_NONE;
};

static int __ctsvc_number_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_number_s *number = (ctsvc_number_s*)record;
	number->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(number->base.properties_flags);

	free(number->label);
	free(number->number);
	free(number->normalized);
	free(number->cleaned);
	free(number->lookup);
	free(number);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_email_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_email_s *email = (ctsvc_email_s*)record;
	email->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(email->base.properties_flags);

	free(email->label);
	free(email->email_addr);
	free(email);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_relation_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_group_relation_s *group = (ctsvc_group_relation_s*)record;
	group->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(group->base.properties_flags);

	free(group->group_name);
	free(group);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_activity_s *activity = (ctsvc_activity_s*)record;
	activity->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(activity->base.properties_flags);

	free(activity->source_name);
	free(activity->status);
	free(activity->service_operation);
	free(activity->uri);
	contacts_list_destroy((contacts_list_h)activity->photos, delete_child);
	free(activity);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_photo_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_activity_photo_s *photo = (ctsvc_activity_photo_s*)record;
	photo->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(photo->base.properties_flags);

	free(photo->photo_url);
	free(photo);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_event_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_event_s *event = (ctsvc_event_s*)record;
	event->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(event->base.properties_flags);

	free(event->label);
	free(event);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_messenger_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_messenger_s *messenger = (ctsvc_messenger_s*)record;
	messenger->base.plugin_cbs = NULL;
	free(messenger->base.properties_flags);

	free(messenger->label);
	free(messenger->im_id);
	free(messenger);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_address_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_address_s *address = (ctsvc_address_s*)record;
	address->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(address->base.properties_flags);

	free(address->label);
	free(address->pobox);
	free(address->postalcode);
	free(address->region);
	free(address->locality);
	free(address->street);
	free(address->extended);
	free(address->country);
	free(address);

	return CONTACTS_ERROR_NONE;
}
static int __ctsvc_url_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_url_s *url = (ctsvc_url_s*)record;
	url->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(url->base.properties_flags);

	free(url->label);
	free(url->url);
	free(url);

	return CONTACTS_ERROR_NONE;
}
static int __ctsvc_nickname_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_nickname_s *nickname = (ctsvc_nickname_s*)record;
	nickname->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(nickname->base.properties_flags);

	free(nickname->label);
	free(nickname->nickname);
	free(nickname);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_profile_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_profile_s *profile = (ctsvc_profile_s*)record;
	profile->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(profile->base.properties_flags);

	free(profile->uid);
	free(profile->text);
	free(profile->service_operation);
	free(profile->mime);
	free(profile->app_id);
	free(profile->uri);
	free(profile->category);
	free(profile->extra_data);
	free(profile);

	return CONTACTS_ERROR_NONE;
}
static int __ctsvc_relationship_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_relationship_s *relationship = (ctsvc_relationship_s*)record;
	relationship->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(relationship->base.properties_flags);

	free(relationship->label);
	free(relationship->name);
	free(relationship);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_image_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_image_s *image = (ctsvc_image_s*)record;
	image->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(image->base.properties_flags);

	free(image->label);
	if (image->path && image->is_vcard)
		__ctsvc_temp_image_hash_table_remove(image->path);
	free(image->path);
	free(image);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_extension_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_extension_s *data = (ctsvc_extension_s*)record;
	data->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(data->base.properties_flags);

	free(data->data2);
	free(data->data3);
	free(data->data4);
	free(data->data5);
	free(data->data6);
	free(data->data7);
	free(data->data8);
	free(data->data9);
	free(data->data10);
	free(data->data11);
	free(data->data12);
	free(data);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_simple_contact_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_simple_contact_s *contact = (ctsvc_simple_contact_s*)record;

	/* help to find double destroy bug (refer to the contacts_record_destroy) */
	contact->base.plugin_cbs = NULL;

	free(contact->base.properties_flags);
	free(contact->display_name);
	free(contact->image_thumbnail_path);
	free(contact->ringtone_path);
	free(contact->vibration);
	free(contact->message_alert);
	free(contact->uid);
	free(contact);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_contact_create(contacts_record_h *out_record)
{
	ctsvc_contact_s *contact;

	contact = calloc(1, sizeof(ctsvc_contact_s));
	RETVM_IF(NULL == contact, CONTACTS_ERROR_OUT_OF_MEMORY,
			"calloc() Fail");

	do {
		contact->name = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->name)
			break;
		contact->name->l_type = CTSVC_RECORD_NAME;

		contact->company = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->company)
			break;
		contact->company->l_type = CTSVC_RECORD_COMPANY;

		contact->note = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->note)
			break;
		contact->note->l_type = CTSVC_RECORD_NOTE;

		contact->numbers = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->numbers)
			break;
		contact->numbers->l_type = CTSVC_RECORD_NUMBER;

		contact->emails = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->emails)
			break;
		contact->emails->l_type = CTSVC_RECORD_EMAIL;

		contact->grouprelations = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->grouprelations)
			break;
		contact->grouprelations->l_type = CTSVC_RECORD_GROUP_RELATION;

		contact->events = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->events)
			break;
		contact->events->l_type = CTSVC_RECORD_EVENT;

		contact->messengers = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->messengers)
			break;
		contact->messengers->l_type = CTSVC_RECORD_MESSENGER;

		contact->postal_addrs = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->postal_addrs)
			break;
		contact->postal_addrs->l_type = CTSVC_RECORD_ADDRESS;

		contact->urls = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->urls)
			break;
		contact->urls->l_type = CTSVC_RECORD_URL;

		contact->nicknames = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->nicknames)
			break;
		contact->nicknames->l_type = CTSVC_RECORD_NICKNAME;

		contact->profiles = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->profiles)
			break;
		contact->profiles->l_type = CTSVC_RECORD_PROFILE;

		contact->relationships = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->relationships)
			break;
		contact->relationships->l_type = CTSVC_RECORD_RELATIONSHIP;

		contact->images = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->images)
			break;
		contact->images->l_type = CTSVC_RECORD_IMAGE;

		contact->extensions = calloc(1, sizeof(ctsvc_list_s));
		if (NULL == contact->extensions)
			break;
		contact->extensions->l_type = CTSVC_RECORD_EXTENSION;

		*out_record = (contacts_record_h)contact;
		return CONTACTS_ERROR_NONE;
	} while (0);

	CTS_ERR("calloc() Fail");
	free(contact->extensions);
	free(contact->images);
	free(contact->relationships);
	free(contact->profiles);
	free(contact->nicknames);
	free(contact->urls);
	free(contact->postal_addrs);
	free(contact->messengers);
	free(contact->events);
	free(contact->grouprelations);
	free(contact->emails);
	free(contact->numbers);
	free(contact->note);
	free(contact->company);
	free(contact->name);
	free(contact);
	return CONTACTS_ERROR_OUT_OF_MEMORY;
}

static int __ctsvc_contact_destroy(contacts_record_h record, bool delete_child)
{
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;
	contact->base.plugin_cbs = NULL; /* help to find double destroy bug (refer to the contacts_record_destroy) */
	free(contact->base.properties_flags);

	free(contact->display_name);
	free(contact->reverse_display_name);
	free(contact->uid);
	free(contact->image_thumbnail_path);
	free(contact->ringtone_path);
	free(contact->vibration);
	free(contact->message_alert);
	free(contact->sort_name);
	free(contact->reverse_sort_name);
	free(contact->sortkey);
	free(contact->reverse_sortkey);

	contacts_list_destroy((contacts_list_h)contact->name, delete_child);

	contacts_list_destroy((contacts_list_h)contact->company, delete_child);

	contacts_list_destroy((contacts_list_h)contact->note, delete_child);

	contacts_list_destroy((contacts_list_h)contact->numbers, delete_child);

	contacts_list_destroy((contacts_list_h)contact->emails, delete_child);

	contacts_list_destroy((contacts_list_h)contact->grouprelations, delete_child);

	contacts_list_destroy((contacts_list_h)contact->events, delete_child);

	contacts_list_destroy((contacts_list_h)contact->messengers, delete_child);

	contacts_list_destroy((contacts_list_h)contact->postal_addrs, delete_child);

	contacts_list_destroy((contacts_list_h)contact->urls, delete_child);

	contacts_list_destroy((contacts_list_h)contact->nicknames, delete_child);

	contacts_list_destroy((contacts_list_h)contact->profiles, delete_child);

	contacts_list_destroy((contacts_list_h)contact->relationships, delete_child);

	contacts_list_destroy((contacts_list_h)contact->images, delete_child);

	contacts_list_destroy((contacts_list_h)contact->extensions, delete_child);
	free(contact);

	return CONTACTS_ERROR_NONE;
}


static int __ctsvc_contact_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_CONTACT_ID:
		*out = contact->id;
		break;
	case CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID:
		*out = contact->display_source_type;
		break;
	case CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID:
		*out = contact->addressbook_id;
		break;
	case CTSVC_PROPERTY_CONTACT_PERSON_ID:
		*out = contact->person_id;
		break;
	case CTSVC_PROPERTY_CONTACT_CHANGED_TIME:
		*out = contact->changed_time;
		break;
	case CTSVC_PROPERTY_CONTACT_LINK_MODE:
		*out = contact->link_mode;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_simple_contact_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_simple_contact_s *contact = (ctsvc_simple_contact_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_CONTACT_ID:
		*out = contact->contact_id;
		break;
	case CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID:
		*out = contact->display_source_type;
		break;
	case CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID:
		*out = contact->addressbook_id;
		break;
	case CTSVC_PROPERTY_CONTACT_PERSON_ID:
		*out = contact->person_id;
		break;
	case CTSVC_PROPERTY_CONTACT_CHANGED_TIME:
		*out = contact->changed_time;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(simple contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_name_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_name_s *name = (ctsvc_name_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NAME_ID:
		*out = name->id;
		break;
	case CTSVC_PROPERTY_NAME_CONTACT_ID:
		*out = name->contact_id;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(name)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_company_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_company_s *company = (ctsvc_company_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_COMPANY_ID:
		*out = company->id;
		break;
	case CTSVC_PROPERTY_COMPANY_CONTACT_ID:
		*out = company->contact_id;
		break;
	case CTSVC_PROPERTY_COMPANY_TYPE:
		*out = company->type;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(company)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_note_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_note_s *note = (ctsvc_note_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NOTE_ID:
		*out = note->id;
		break;
	case CTSVC_PROPERTY_NOTE_CONTACT_ID:
		*out = note->contact_id;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(note)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_number_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_number_s *number = (ctsvc_number_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NUMBER_ID:
		*out = number->id;
		break;
	case CTSVC_PROPERTY_NUMBER_CONTACT_ID:
		*out = number->contact_id;
		break;
	case CTSVC_PROPERTY_NUMBER_TYPE:
		*out = number->type;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(number)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_email_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_email_s *email = (ctsvc_email_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EMAIL_ID:
		*out = email->id;
		break;
	case CTSVC_PROPERTY_EMAIL_CONTACT_ID:
		*out = email->contact_id;
		break;
	case CTSVC_PROPERTY_EMAIL_TYPE:
		*out = email->type;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(email)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_url_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_url_s *url = (ctsvc_url_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_URL_ID:
		*out = url->id;
		break;
	case CTSVC_PROPERTY_URL_CONTACT_ID:
		*out = url->contact_id;
		break;
	case CTSVC_PROPERTY_URL_TYPE:
		*out = url->type;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(url)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_event_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_event_s *event = (ctsvc_event_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EVENT_ID:
		*out = event->id;
		break;
	case CTSVC_PROPERTY_EVENT_CONTACT_ID:
		*out = event->contact_id;
		break;
	case CTSVC_PROPERTY_EVENT_TYPE:
		*out = event->type;
		break;
	case CTSVC_PROPERTY_EVENT_DATE:
		*out = event->date;
		break;
	case CTSVC_PROPERTY_EVENT_CALENDAR_TYPE:
		*out = event->calendar_type;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(event)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_nickname_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_nickname_s *nickname = (ctsvc_nickname_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NICKNAME_ID:
		*out = nickname->id;
		break;
	case CTSVC_PROPERTY_NICKNAME_CONTACT_ID:
		*out = nickname->contact_id;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(nickname)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_address_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_address_s *address = (ctsvc_address_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ADDRESS_ID:
		*out = address->id;
		break;
	case CTSVC_PROPERTY_ADDRESS_CONTACT_ID:
		*out = address->contact_id;
		break;
	case CTSVC_PROPERTY_ADDRESS_TYPE:
		*out = address->type;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(address)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_messenger_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_messenger_s *messenger = (ctsvc_messenger_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_MESSENGER_ID:
		*out = messenger->id;
		break;
	case CTSVC_PROPERTY_MESSENGER_CONTACT_ID:
		*out = messenger->contact_id;
		break;
	case CTSVC_PROPERTY_MESSENGER_TYPE:
		*out = messenger->type;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(messenger)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_relation_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_group_relation_s *group = (ctsvc_group_relation_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_GROUP_RELATION_ID:
		*out = group->id;
		break;
	case CTSVC_PROPERTY_GROUP_RELATION_CONTACT_ID:
		*out = group->contact_id;
		break;
	case CTSVC_PROPERTY_GROUP_RELATION_GROUP_ID:
		*out = group->group_id;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(group)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_activity_s *activity = (ctsvc_activity_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ACTIVITY_ID:
		*out = activity->id;
		break;
	case CTSVC_PROPERTY_ACTIVITY_CONTACT_ID:
		*out = activity->contact_id;
		break;
	case CTSVC_PROPERTY_ACTIVITY_TIMESTAMP:
		*out = activity->timestamp;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(activity)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_add_child_record(contacts_record_h record,
		unsigned int property_id, contacts_record_h child_record)
{
	ctsvc_activity_s *s_activity = (ctsvc_activity_s*)record;
	ctsvc_activity_photo_s *s_activity_photo = (ctsvc_activity_photo_s*)child_record;

	RETVM_IF(property_id != CTSVC_PROPERTY_ACTIVITY_ACTIVITY_PHOTO, CONTACTS_ERROR_INVALID_PARAMETER, "property_id(%d) is not supported", property_id);
	RETVM_IF(s_activity_photo->base.r_type != CTSVC_RECORD_ACTIVITY_PHOTO, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter: r_type(%d) is wrong", s_activity_photo->base.r_type);
	s_activity_photo->id = 0;

	return ctsvc_list_add_child((contacts_list_h)s_activity->photos, child_record);
}

static int __ctsvc_activity_remove_child_record(contacts_record_h record,
		unsigned int property_id, contacts_record_h child_record)
{
	ctsvc_activity_s *s_activity = (ctsvc_activity_s*)record;
	ctsvc_activity_photo_s *s_activity_photo = (ctsvc_activity_photo_s*)child_record;

	RETVM_IF(property_id != CTSVC_PROPERTY_ACTIVITY_ACTIVITY_PHOTO,
			CONTACTS_ERROR_INVALID_PARAMETER,
			"property_id(%d) is not supported",
			property_id);

	ctsvc_list_remove_child((contacts_list_h)s_activity->photos, child_record,
			s_activity_photo->id ? true : false);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_get_child_record_count(contacts_record_h record,
		unsigned int property_id, int *count)
{
	ctsvc_activity_s *s_activity = (ctsvc_activity_s*)record;
	RETVM_IF(property_id != CTSVC_PROPERTY_ACTIVITY_ACTIVITY_PHOTO, CONTACTS_ERROR_INVALID_PARAMETER, "property_id(%d) is not supported", property_id);

	if (s_activity->photos)
		contacts_list_get_count((contacts_list_h)s_activity->photos, count);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_get_child_record_at_p(contacts_record_h record,
		unsigned int property_id, int index, contacts_record_h *out_record)
{
	int count = 0;
	ctsvc_activity_s *s_activity = (ctsvc_activity_s*)record;

	RETVM_IF(property_id != CTSVC_PROPERTY_ACTIVITY_ACTIVITY_PHOTO, CONTACTS_ERROR_INVALID_PARAMETER, "property_id(%d) is not supported", property_id);

	if (s_activity->photos)
		contacts_list_get_count((contacts_list_h)s_activity->photos, &count);

	if (count < index) {
		CTS_ERR("The index(%d) is greather than total length(%d)", index, count);
		*out_record = NULL;
		return CONTACTS_ERROR_NO_DATA;
	}

	return ctsvc_list_get_nth_record_p((contacts_list_h)s_activity->photos, index, out_record);
}


static int __ctsvc_activity_clone_child_record_list(contacts_record_h record,
		unsigned int property_id, contacts_list_h *out_list)
{
	int count;
	ctsvc_activity_s *s_activity = (ctsvc_activity_s*)record;

	RETVM_IF(property_id != CTSVC_PROPERTY_ACTIVITY_ACTIVITY_PHOTO, CONTACTS_ERROR_INVALID_PARAMETER, "property_id(%d) is not supported", property_id);

	contacts_list_get_count((contacts_list_h)s_activity->photos, &count);
	if (count <= 0) {
		*out_list = NULL;
		return CONTACTS_ERROR_NO_DATA;
	}
	return ctsvc_list_clone((contacts_list_h)s_activity->photos, out_list);
}


static int __ctsvc_activity_photo_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_activity_photo_s *photo = (ctsvc_activity_photo_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ACTIVITY_PHOTO_ID:
		*out = photo->id;
		break;
	case CTSVC_PROPERTY_ACTIVITY_PHOTO_ACTIVITY_ID:
		*out = photo->activity_id;
		break;
	case CTSVC_PROPERTY_ACTIVITY_PHOTO_SORT_INDEX:
		*out = photo->sort_index;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(activity)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_profile_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_profile_s *profile = (ctsvc_profile_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_PROFILE_ID:
		*out = profile->id;
		break;
	case CTSVC_PROPERTY_PROFILE_CONTACT_ID:
		*out = profile->contact_id;
		break;
	case CTSVC_PROPERTY_PROFILE_ORDER:
		*out = profile->order;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(profile)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_relationship_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_relationship_s *relationship = (ctsvc_relationship_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_RELATIONSHIP_ID:
		*out = relationship->id;
		break;
	case CTSVC_PROPERTY_RELATIONSHIP_CONTACT_ID:
		*out = relationship->contact_id;
		break;
	case CTSVC_PROPERTY_RELATIONSHIP_TYPE:
		*out = relationship->type;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(relationship)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_image_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_image_s *image = (ctsvc_image_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_IMAGE_ID:
		*out = image->id;
		break;
	case CTSVC_PROPERTY_IMAGE_CONTACT_ID:
		*out = image->contact_id;
		break;
	case CTSVC_PROPERTY_IMAGE_TYPE:
		*out = image->type;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(image)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_extension_get_int(contacts_record_h record, unsigned int property_id, int *out)
{
	ctsvc_extension_s *extension = (ctsvc_extension_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EXTENSION_ID:
		*out = extension->id;
		break;
	case CTSVC_PROPERTY_EXTENSION_CONTACT_ID:
		*out = extension->contact_id;
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA1:
		*out = extension->data1;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(extension)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}


static int __ctsvc_contact_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_CONTACT_ID:
		CHECK_DIRTY_VAL(contact->id, value, is_dirty);
		contact->id = value;
		break;
	case CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID:
		CHECK_DIRTY_VAL(contact->display_source_type, value, is_dirty);
		contact->display_source_type = value;
		break;
	case CTSVC_PROPERTY_CONTACT_PERSON_ID:
		RETVM_IF(0 < contact->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (contact)", property_id);
		CHECK_DIRTY_VAL(contact->person_id, value, is_dirty);
		contact->person_id = value;
		break;
	case CTSVC_PROPERTY_CONTACT_CHANGED_TIME:
		CHECK_DIRTY_VAL(contact->changed_time, value, is_dirty);
		contact->changed_time = value;
		break;
	case CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID:
		RETVM_IF(0 < contact->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (contact)", property_id);
		CHECK_DIRTY_VAL(contact->addressbook_id, value, is_dirty);
		contact->addressbook_id = value;
		break;
	case CTSVC_PROPERTY_CONTACT_LINK_MODE:
		RETVM_IF(value != CONTACTS_CONTACT_LINK_MODE_NONE
				&& value != CONTACTS_CONTACT_LINK_MODE_IGNORE_ONCE,
				CONTACTS_ERROR_INVALID_PARAMETER, "link mode is in invalid range (%d)", value);
		RETVM_IF(0 < contact->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (contact)", property_id);
		CHECK_DIRTY_VAL(contact->link_mode, value, is_dirty);
		contact->link_mode = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value (contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_simple_contact_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_simple_contact_s *contact = (ctsvc_simple_contact_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_CONTACT_ID:
		CHECK_DIRTY_VAL(contact->contact_id, value, is_dirty);
		contact->contact_id = value;
		break;
	case CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID:
		CHECK_DIRTY_VAL(contact->display_source_type, value, is_dirty);
		contact->display_source_type = value;
		break;
	case CTSVC_PROPERTY_CONTACT_PERSON_ID:
		CHECK_DIRTY_VAL(contact->person_id, value, is_dirty);
		contact->person_id = value;
		break;
	case CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID:
		RETVM_IF(0 < contact->contact_id, CONTACTS_ERROR_INVALID_PARAMETER,
				"Invalide parameter : property_id(%d) is a read-only value (contact)", property_id);
		CHECK_DIRTY_VAL(contact->addressbook_id, value, is_dirty);
		contact->addressbook_id = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(simple contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_name_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_name_s *name = (ctsvc_name_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NAME_ID:
		CHECK_DIRTY_VAL(name->id, value, is_dirty);
		name->id = value;
		break;
	case CTSVC_PROPERTY_NAME_CONTACT_ID:
		RETVM_IF(0 < name->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (name)", property_id);
		CHECK_DIRTY_VAL(name->contact_id, value, is_dirty);
		name->contact_id = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(name)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_company_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_company_s *company = (ctsvc_company_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_COMPANY_ID:
		CHECK_DIRTY_VAL(company->id, value, is_dirty);
		company->id = value;
		break;
	case CTSVC_PROPERTY_COMPANY_CONTACT_ID:
		RETVM_IF(0 < company->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (company)", property_id);
		CHECK_DIRTY_VAL(company->contact_id, value, is_dirty);
		company->contact_id = value;
		break;
	case CTSVC_PROPERTY_COMPANY_TYPE:
		RETVM_IF(value < CONTACTS_COMPANY_TYPE_OTHER
				|| CONTACTS_COMPANY_TYPE_WORK < value,
				CONTACTS_ERROR_INVALID_PARAMETER, "company type is in invalid range (%d)", value);

		CHECK_DIRTY_VAL(company->type, value, is_dirty);
		company->type = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(company)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_note_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_note_s *note = (ctsvc_note_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NOTE_ID:
		CHECK_DIRTY_VAL(note->id, value, is_dirty);
		note->id = value;
		break;
	case CTSVC_PROPERTY_NOTE_CONTACT_ID:
		RETVM_IF(0 < note->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (note)", property_id);
		CHECK_DIRTY_VAL(note->contact_id, value, is_dirty);
		note->contact_id = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(note)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_number_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_number_s *number = (ctsvc_number_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NUMBER_ID:
		CHECK_DIRTY_VAL(number->id, value, is_dirty);
		number->id = value;
		break;
	case CTSVC_PROPERTY_NUMBER_CONTACT_ID:
		RETVM_IF(0 < number->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (number)", property_id);
		CHECK_DIRTY_VAL(number->contact_id, value, is_dirty);
		number->contact_id = value;
		break;
	case CTSVC_PROPERTY_NUMBER_TYPE:
		CHECK_DIRTY_VAL(number->type, value, is_dirty);
		number->type = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(number)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_email_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_email_s *email = (ctsvc_email_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EMAIL_ID:
		CHECK_DIRTY_VAL(email->id, value, is_dirty);
		email->id = value;
		break;
	case CTSVC_PROPERTY_EMAIL_CONTACT_ID:
		RETVM_IF(0 < email->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (email)", property_id);
		CHECK_DIRTY_VAL(email->contact_id, value, is_dirty);
		email->contact_id = value;
		break;
	case CTSVC_PROPERTY_EMAIL_TYPE:
		RETVM_IF(value < CONTACTS_EMAIL_TYPE_OTHER
				|| CONTACTS_EMAIL_TYPE_MOBILE < value,
				CONTACTS_ERROR_INVALID_PARAMETER, "email type is in invalid range (%d)", value);

		CHECK_DIRTY_VAL(email->type, value, is_dirty);
		email->type = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(email)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_url_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_url_s *url = (ctsvc_url_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_URL_ID:
		CHECK_DIRTY_VAL(url->id, value, is_dirty);
		url->id = value;
		break;
	case CTSVC_PROPERTY_URL_CONTACT_ID:
		RETVM_IF(0 < url->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (url)", property_id);
		CHECK_DIRTY_VAL(url->contact_id, value, is_dirty);
		url->contact_id = value;
		break;
	case CTSVC_PROPERTY_URL_TYPE:
		RETVM_IF(value < CONTACTS_URL_TYPE_OTHER
				|| CONTACTS_URL_TYPE_WORK < value,
				CONTACTS_ERROR_INVALID_PARAMETER, "url type is in invalid range (%d)", value);

		CHECK_DIRTY_VAL(url->type, value, is_dirty);
		url->type = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(url)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_event_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_event_s *event = (ctsvc_event_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EVENT_ID:
		CHECK_DIRTY_VAL(event->id, value, is_dirty);
		event->id = value;
		break;
	case CTSVC_PROPERTY_EVENT_CONTACT_ID:
		RETVM_IF(0 < event->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (event)", property_id);
		CHECK_DIRTY_VAL(event->contact_id, value, is_dirty);
		event->contact_id = value;
		break;
	case CTSVC_PROPERTY_EVENT_TYPE:
		RETVM_IF(value < CONTACTS_EVENT_TYPE_OTHER
				|| CONTACTS_EVENT_TYPE_ANNIVERSARY < value,
				CONTACTS_ERROR_INVALID_PARAMETER, "event type is in invalid range (%d)", value);
		CHECK_DIRTY_VAL(event->type, value, is_dirty);
		event->type = value;
		break;
	case CTSVC_PROPERTY_EVENT_DATE:
		CHECK_DIRTY_VAL(event->date, value, is_dirty);
		event->date = value;
		break;
	case CTSVC_PROPERTY_EVENT_CALENDAR_TYPE:
		CHECK_DIRTY_VAL(event->calendar_type, value, is_dirty);
		event->calendar_type = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(event)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_nickname_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_nickname_s *nickname = (ctsvc_nickname_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NICKNAME_ID:
		CHECK_DIRTY_VAL(nickname->id, value, is_dirty);
		nickname->id = value;
		break;
	case CTSVC_PROPERTY_NICKNAME_CONTACT_ID:
		RETVM_IF(0 < nickname->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (nickname)", property_id);
		CHECK_DIRTY_VAL(nickname->contact_id, value, is_dirty);
		nickname->contact_id = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(nickname)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_address_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_address_s *address = (ctsvc_address_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ADDRESS_ID:
		CHECK_DIRTY_VAL(address->id, value, is_dirty);
		address->id = value;
		break;
	case CTSVC_PROPERTY_ADDRESS_CONTACT_ID:
		RETVM_IF(0 < address->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (address)", property_id);
		CHECK_DIRTY_VAL(address->contact_id, value, is_dirty);
		address->contact_id = value;
		break;
	case CTSVC_PROPERTY_ADDRESS_TYPE:
		RETVM_IF(value < CONTACTS_ADDRESS_TYPE_OTHER
				|| CONTACTS_ADDRESS_TYPE_PARCEL < value,
				CONTACTS_ERROR_INVALID_PARAMETER, "address type is %d", value);
		CHECK_DIRTY_VAL(address->type, value, is_dirty);
		address->type = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(address)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_messenger_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_messenger_s *messenger = (ctsvc_messenger_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_MESSENGER_ID:
		CHECK_DIRTY_VAL(messenger->id, value, is_dirty);
		messenger->id = value;
		break;
	case CTSVC_PROPERTY_MESSENGER_CONTACT_ID:
		RETVM_IF(0 < messenger->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (messenger)", property_id);
		CHECK_DIRTY_VAL(messenger->contact_id, value, is_dirty);
		messenger->contact_id = value;
		break;
	case CTSVC_PROPERTY_MESSENGER_TYPE:
		RETVM_IF(value < CONTACTS_MESSENGER_TYPE_OTHER
				|| CONTACTS_MESSENGER_TYPE_IRC < value,
				CONTACTS_ERROR_INVALID_PARAMETER, "messenger type is in invalid range (%d)", value);

		CHECK_DIRTY_VAL(messenger->type, value, is_dirty);
		messenger->type = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(messenger)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_relation_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_group_relation_s *group = (ctsvc_group_relation_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_GROUP_RELATION_ID:
		CHECK_DIRTY_VAL(group->id, value, is_dirty);
		group->id = value;
		break;
	case CTSVC_PROPERTY_GROUP_RELATION_CONTACT_ID:
		RETVM_IF(0 < group->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (group)", property_id);
		CHECK_DIRTY_VAL(group->contact_id, value, is_dirty);
		group->contact_id = value;
		break;
	case CTSVC_PROPERTY_GROUP_RELATION_GROUP_ID:
		RETVM_IF(0 < group->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (group)", property_id);
		CHECK_DIRTY_VAL(group->group_id, value, is_dirty);
		group->group_id = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(group relation)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_activity_s *activity = (ctsvc_activity_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ACTIVITY_ID:
		CHECK_DIRTY_VAL(activity->id, value, is_dirty);
		activity->id = value;
		break;
	case CTSVC_PROPERTY_ACTIVITY_CONTACT_ID:
		RETVM_IF(0 < activity->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (activity)", property_id);
		CHECK_DIRTY_VAL(activity->contact_id, value, is_dirty);
		activity->contact_id = value;
		break;
	case CTSVC_PROPERTY_ACTIVITY_TIMESTAMP:
		CHECK_DIRTY_VAL(activity->timestamp, value, is_dirty);
		activity->timestamp = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(activity)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_photo_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_activity_photo_s *photo = (ctsvc_activity_photo_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ACTIVITY_PHOTO_ID:
		CHECK_DIRTY_VAL(photo->id, value, is_dirty);
		photo->id = value;
		break;
	case CTSVC_PROPERTY_ACTIVITY_PHOTO_ACTIVITY_ID:
		CHECK_DIRTY_VAL(photo->activity_id, value, is_dirty);
		photo->activity_id = value;
		break;
	case CTSVC_PROPERTY_ACTIVITY_PHOTO_SORT_INDEX:
		CHECK_DIRTY_VAL(photo->sort_index, value, is_dirty);
		photo->sort_index = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(activity)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_profile_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_profile_s *profile = (ctsvc_profile_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_PROFILE_ID:
		CHECK_DIRTY_VAL(profile->id, value, is_dirty);
		profile->id = value;
		break;
	case CTSVC_PROPERTY_PROFILE_CONTACT_ID:
		RETVM_IF(0 < profile->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (profile)", property_id);
		CHECK_DIRTY_VAL(profile->contact_id, value, is_dirty);
		profile->contact_id = value;
		break;
	case CTSVC_PROPERTY_PROFILE_ORDER:
		CHECK_DIRTY_VAL(profile->order, value, is_dirty);
		profile->order = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(profile)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_relationship_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_relationship_s *relationship = (ctsvc_relationship_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_RELATIONSHIP_ID:
		CHECK_DIRTY_VAL(relationship->id, value, is_dirty);
		relationship->id = value;
		break;
	case CTSVC_PROPERTY_RELATIONSHIP_CONTACT_ID:
		RETVM_IF(0 < relationship->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (relationship)", property_id);
		CHECK_DIRTY_VAL(relationship->contact_id, value, is_dirty);
		relationship->contact_id = value;
		break;
	case CTSVC_PROPERTY_RELATIONSHIP_TYPE:
		RETVM_IF(value < CONTACTS_RELATIONSHIP_TYPE_OTHER
				|| CONTACTS_RELATIONSHIP_TYPE_CUSTOM < value,
				CONTACTS_ERROR_INVALID_PARAMETER, "relationship type is in invalid range (%d)", value);
		CHECK_DIRTY_VAL(relationship->type, value, is_dirty);
		relationship->type = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(relationship)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_image_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_image_s *image = (ctsvc_image_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_IMAGE_ID:
		CHECK_DIRTY_VAL(image->id, value, is_dirty);
		image->id = value;
		break;
	case CTSVC_PROPERTY_IMAGE_CONTACT_ID:
		RETVM_IF(0 < image->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (image)", property_id);
		CHECK_DIRTY_VAL(image->contact_id, value, is_dirty);
		image->contact_id = value;
		break;
	case CTSVC_PROPERTY_IMAGE_TYPE:
		RETVM_IF(value < CONTACTS_IMAGE_TYPE_OTHER || CONTACTS_IMAGE_TYPE_CUSTOM < value,
				CONTACTS_ERROR_INVALID_PARAMETER,
				"image type is in invalid range (%d)", value);
		CHECK_DIRTY_VAL(image->type, value, is_dirty);
		image->type = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(image)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_extension_set_int(contacts_record_h record, unsigned int property_id, int value, bool *is_dirty)
{
	ctsvc_extension_s *extension = (ctsvc_extension_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EXTENSION_ID:
		CHECK_DIRTY_VAL(extension->id, value, is_dirty);
		extension->id = value;
		break;
	case CTSVC_PROPERTY_EXTENSION_CONTACT_ID:
		RETVM_IF(0 < extension->id, CONTACTS_ERROR_INVALID_PARAMETER,
				"property_id(%d) is a read-only value (extension)", property_id);
		CHECK_DIRTY_VAL(extension->contact_id, value, is_dirty);
		extension->contact_id = value;
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA1:
		CHECK_DIRTY_VAL(extension->data1, value, is_dirty);
		extension->data1 = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(extension)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_contact_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;
	switch (property_id) {
	case CTSVC_PROPERTY_CONTACT_DISPLAY_NAME:
		*out_str = GET_STR(copy, contact->display_name);
		break;
	case CTSVC_PROPERTY_CONTACT_RINGTONE:
		*out_str = GET_STR(copy, contact->ringtone_path);
		break;
	case CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL:
		*out_str = GET_STR(copy, contact->image_thumbnail_path);
		break;
	case CTSVC_PROPERTY_CONTACT_UID:
		*out_str = GET_STR(copy, contact->uid);
		break;
	case CTSVC_PROPERTY_CONTACT_VIBRATION:
		*out_str = GET_STR(copy, contact->vibration);
		break;
	case CTSVC_PROPERTY_CONTACT_MESSAGE_ALERT:
		*out_str = GET_STR(copy, contact->message_alert);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_contact_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_contact_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_contact_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_contact_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_contact_get_record_list_p(contacts_record_h record,
		unsigned int property_id, contacts_list_h *list)
{
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_CONTACT_NAME:
		*list = (contacts_list_h)contact->name;
		break;
	case CTSVC_PROPERTY_CONTACT_COMPANY:
		*list = (contacts_list_h)contact->company;
		break;
	case CTSVC_PROPERTY_CONTACT_NOTE:
		*list = (contacts_list_h)contact->note;
		break;
	case CTSVC_PROPERTY_CONTACT_NUMBER:
		*list = (contacts_list_h)contact->numbers;
		break;
	case CTSVC_PROPERTY_CONTACT_EMAIL:
		*list = (contacts_list_h)contact->emails;
		break;
	case CTSVC_PROPERTY_CONTACT_EVENT:
		*list = (contacts_list_h)contact->events;
		break;
	case CTSVC_PROPERTY_CONTACT_MESSENGER:
		*list = (contacts_list_h)contact->messengers;
		break;
	case CTSVC_PROPERTY_CONTACT_ADDRESS:
		*list = (contacts_list_h)contact->postal_addrs;
		break;
	case CTSVC_PROPERTY_CONTACT_URL:
		*list = (contacts_list_h)contact->urls;
		break;
	case CTSVC_PROPERTY_CONTACT_NICKNAME:
		*list = (contacts_list_h)contact->nicknames;
		break;
	case CTSVC_PROPERTY_CONTACT_PROFILE:
		*list = (contacts_list_h)contact->profiles;
		break;
	case CTSVC_PROPERTY_CONTACT_RELATIONSHIP:
		*list = (contacts_list_h)contact->relationships;
		break;
	case CTSVC_PROPERTY_CONTACT_IMAGE:
		*list = (contacts_list_h)contact->images;
		break;
	case CTSVC_PROPERTY_CONTACT_GROUP_RELATION:
		*list = (contacts_list_h)contact->grouprelations;
		break;
	case CTSVC_PROPERTY_CONTACT_EXTENSION:
		*list = (contacts_list_h)contact->extensions;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_contact_get_child_record_count(contacts_record_h record,
		unsigned int property_id, int *count)
{
	int ret;
	contacts_list_h list = NULL;

	*count = 0;
	ret = __ctsvc_contact_get_record_list_p(record, property_id, &list);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		return ret;

	if (list)
		contacts_list_get_count(list, count);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_contact_get_child_record_at_p(contacts_record_h record,
		unsigned int property_id, int index, contacts_record_h *out_record)
{
	int ret;
	int count;
	contacts_list_h list = NULL;

	ret = __ctsvc_contact_get_record_list_p(record, property_id, &list);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		return ret;

	contacts_list_get_count(list, &count);
	if (count < index) {
		CTS_ERR("The index(%d) is greather than total length(%d)", index, count);
		*out_record = NULL;
		return CONTACTS_ERROR_NO_DATA;
	} else {
		return ctsvc_list_get_nth_record_p(list, index, out_record);
	}
}

static int __ctsvc_contact_clone_child_record_list(contacts_record_h record,
		unsigned int property_id, contacts_list_h *out_list)
{
	int ret;
	int count;
	contacts_list_h list = NULL;

	ret = __ctsvc_contact_get_record_list_p(record, property_id, &list);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		return ret;

	contacts_list_get_count(list, &count);
	if (count <= 0) {
		*out_list = NULL;
		return CONTACTS_ERROR_NO_DATA;
	}
	ctsvc_list_clone(list, out_list);

	return CONTACTS_ERROR_NONE;
}


static int __ctsvc_contact_reset_child_record_id(contacts_record_h child_record)
{
	ctsvc_record_s *record = (ctsvc_record_s*)child_record;

	switch (record->r_type) {
	case CTSVC_RECORD_NAME:
		((ctsvc_name_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_COMPANY:
		((ctsvc_company_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_NOTE:
		((ctsvc_note_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_NUMBER:
		((ctsvc_number_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_EMAIL:
		((ctsvc_email_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_URL:
		((ctsvc_url_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_EVENT:
		((ctsvc_event_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_NICKNAME:
		((ctsvc_nickname_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_ADDRESS:
		((ctsvc_address_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_MESSENGER:
		((ctsvc_messenger_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_GROUP_RELATION:
		((ctsvc_group_relation_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_ACTIVITY:
		((ctsvc_activity_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_PROFILE:
		((ctsvc_profile_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_RELATIONSHIP:
		((ctsvc_relationship_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_IMAGE:
		((ctsvc_image_s*)record)->id = 0;
		break;
	case CTSVC_RECORD_EXTENSION:
		((ctsvc_extension_s*)record)->id = 0;
		break;
	default:
		CTS_ERR("record(%d) is not child of contact", record->r_type);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_contact_get_child_record_id(contacts_record_h child_record)
{
	ctsvc_record_s *record = (ctsvc_record_s*)child_record;

	switch (record->r_type) {
	case CTSVC_RECORD_NAME:
		return ((ctsvc_name_s*)record)->id;
	case CTSVC_RECORD_COMPANY:
		return ((ctsvc_company_s*)record)->id;
	case CTSVC_RECORD_NOTE:
		return ((ctsvc_note_s*)record)->id;
	case CTSVC_RECORD_NUMBER:
		return ((ctsvc_number_s*)record)->id;
	case CTSVC_RECORD_EMAIL:
		return ((ctsvc_email_s*)record)->id;
	case CTSVC_RECORD_URL:
		return ((ctsvc_url_s*)record)->id;
	case CTSVC_RECORD_EVENT:
		return ((ctsvc_event_s*)record)->id;
	case CTSVC_RECORD_NICKNAME:
		return ((ctsvc_nickname_s*)record)->id;
	case CTSVC_RECORD_ADDRESS:
		return ((ctsvc_address_s*)record)->id;
	case CTSVC_RECORD_MESSENGER:
		return ((ctsvc_messenger_s*)record)->id;
	case CTSVC_RECORD_GROUP_RELATION:
		return ((ctsvc_group_relation_s*)record)->id;
	case CTSVC_RECORD_ACTIVITY:
		return ((ctsvc_activity_s*)record)->id;
	case CTSVC_RECORD_PROFILE:
		return ((ctsvc_profile_s*)record)->id;
	case CTSVC_RECORD_RELATIONSHIP:
		return ((ctsvc_relationship_s*)record)->id;
	case CTSVC_RECORD_IMAGE:
		return ((ctsvc_image_s*)record)->id;
	case CTSVC_RECORD_EXTENSION:
		return ((ctsvc_extension_s*)record)->id;
	default:
		CTS_ERR("record(%d) is not child of contact", record->r_type);
		return 0;
	}
	return 0;
}

static int __ctsvc_contact_add_child_record(contacts_record_h record,
		unsigned int property_id, contacts_record_h child_record)
{
	int ret;
	contacts_list_h list = NULL;
	ctsvc_record_s *s_record = (ctsvc_record_s*)child_record;

	ret = __ctsvc_contact_get_record_list_p(record, property_id, &list);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		return ret;

	if (CTSVC_RECORD_NAME == s_record->r_type && 1 == ((ctsvc_list_s*)list)->count) {
		CTS_ERR("This type(%d) of child_record can not be added anymore", s_record->r_type);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	ret = __ctsvc_contact_reset_child_record_id(child_record);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		return ret;

	ctsvc_list_add_child(list, child_record);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_contact_remove_child_record(contacts_record_h record,
		unsigned int property_id, contacts_record_h child_record)
{
	int id;
	int ret;
	contacts_list_h list = NULL;

	ret = __ctsvc_contact_get_record_list_p(record, property_id, &list);
	if (CONTACTS_ERROR_INVALID_PARAMETER == ret)
		return ret;

	id = __ctsvc_contact_get_child_record_id(child_record);
	ctsvc_list_remove_child(list, child_record, id ? true : false);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_simple_contact_get_str_real(contacts_record_h record,
		unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_simple_contact_s *contact = (ctsvc_simple_contact_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_CONTACT_DISPLAY_NAME:
		*out_str = GET_STR(copy, contact->display_name);
		break;
	case CTSVC_PROPERTY_CONTACT_RINGTONE:
		*out_str = GET_STR(copy, contact->ringtone_path);
		break;
	case CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL:
		*out_str = GET_STR(copy, contact->image_thumbnail_path);
		break;
	case CTSVC_PROPERTY_CONTACT_UID:
		*out_str = GET_STR(copy, contact->uid);
		break;
	case CTSVC_PROPERTY_CONTACT_VIBRATION:
		*out_str = GET_STR(copy, contact->vibration);
		break;
	case CTSVC_PROPERTY_CONTACT_MESSAGE_ALERT:
		*out_str = GET_STR(copy, contact->message_alert);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(simple_contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_simple_contact_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_simple_contact_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_simple_contact_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_simple_contact_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_name_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_name_s *name = (ctsvc_name_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NAME_FIRST:
		*out_str = GET_STR(copy, name->first);
		break;
	case CTSVC_PROPERTY_NAME_LAST:
		*out_str = GET_STR(copy, name->last);
		break;
	case CTSVC_PROPERTY_NAME_ADDITION:
		*out_str = GET_STR(copy, name->addition);
		break;
	case CTSVC_PROPERTY_NAME_SUFFIX:
		*out_str = GET_STR(copy, name->suffix);
		break;
	case CTSVC_PROPERTY_NAME_PREFIX:
		*out_str = GET_STR(copy, name->prefix);
		break;
	case CTSVC_PROPERTY_NAME_PHONETIC_FIRST:
		*out_str = GET_STR(copy, name->phonetic_first);
		break;
	case CTSVC_PROPERTY_NAME_PHONETIC_MIDDLE:
		*out_str = GET_STR(copy, name->phonetic_middle);
		break;
	case CTSVC_PROPERTY_NAME_PHONETIC_LAST:
		*out_str = GET_STR(copy, name->phonetic_last);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(name)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_name_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_name_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_name_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_name_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_company_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_company_s *company = (ctsvc_company_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_COMPANY_LABEL:
		*out_str = GET_STR(copy, company->label);
		break;
	case CTSVC_PROPERTY_COMPANY_NAME:
		*out_str = GET_STR(copy, company->name);
		break;
	case CTSVC_PROPERTY_COMPANY_DEPARTMENT:
		*out_str = GET_STR(copy, company->department);
		break;
	case CTSVC_PROPERTY_COMPANY_JOB_TITLE:
		*out_str = GET_STR(copy, company->job_title);
		break;
	case CTSVC_PROPERTY_COMPANY_ASSISTANT_NAME:
		*out_str = GET_STR(copy, company->assistant_name);
		break;
	case CTSVC_PROPERTY_COMPANY_ROLE:
		*out_str = GET_STR(copy, company->role);
		break;
	case CTSVC_PROPERTY_COMPANY_LOGO:
		*out_str = GET_STR(copy, company->logo);
		break;
	case CTSVC_PROPERTY_COMPANY_LOCATION:
		*out_str = GET_STR(copy, company->location);
		break;
	case CTSVC_PROPERTY_COMPANY_DESCRIPTION:
		*out_str = GET_STR(copy, company->description);
		break;
	case CTSVC_PROPERTY_COMPANY_PHONETIC_NAME:
		*out_str = GET_STR(copy, company->phonetic_name);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(company)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_company_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_company_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_company_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_company_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_note_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_note_s *note = (ctsvc_note_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NOTE_NOTE:
		*out_str = GET_STR(copy, note->note);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(note)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_note_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_note_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_note_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_note_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_number_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_number_s *number = (ctsvc_number_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NUMBER_LABEL:
		*out_str = GET_STR(copy, number->label);
		break;
	case CTSVC_PROPERTY_NUMBER_NUMBER:
		*out_str = GET_STR(copy, number->number);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(number)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_number_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_number_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_number_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_number_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_email_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_email_s *email = (ctsvc_email_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EMAIL_EMAIL:
		*out_str = GET_STR(copy, email->email_addr);
		break;
	case CTSVC_PROPERTY_EMAIL_LABEL:
		*out_str = GET_STR(copy, email->label);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(email)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_email_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_email_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_email_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_email_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_url_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_url_s *url = (ctsvc_url_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_URL_URL:
		*out_str = GET_STR(copy, url->url);
		break;
	case CTSVC_PROPERTY_URL_LABEL:
		*out_str = GET_STR(copy, url->label);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(url)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_url_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_url_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_url_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_url_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_event_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_event_s *event = (ctsvc_event_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EVENT_LABEL:
		*out_str = GET_STR(copy, event->label);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(event)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_event_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_event_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_event_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_event_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_nickname_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_nickname_s *nickname = (ctsvc_nickname_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NICKNAME_NAME:
		*out_str = GET_STR(copy, nickname->nickname);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(nickname)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_nickname_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_nickname_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_nickname_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_nickname_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_address_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_address_s *address = (ctsvc_address_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ADDRESS_LABEL:
		*out_str = GET_STR(copy, address->label);
		break;
	case CTSVC_PROPERTY_ADDRESS_POSTBOX:
		*out_str = GET_STR(copy, address->pobox);
		break;
	case CTSVC_PROPERTY_ADDRESS_POSTAL_CODE:
		*out_str = GET_STR(copy, address->postalcode);
		break;
	case CTSVC_PROPERTY_ADDRESS_REGION:
		*out_str = GET_STR(copy, address->region);
		break;
	case CTSVC_PROPERTY_ADDRESS_LOCALITY:
		*out_str = GET_STR(copy, address->locality);
		break;
	case CTSVC_PROPERTY_ADDRESS_STREET:
		*out_str = GET_STR(copy, address->street);
		break;
	case CTSVC_PROPERTY_ADDRESS_COUNTRY:
		*out_str = GET_STR(copy, address->country);
		break;
	case CTSVC_PROPERTY_ADDRESS_EXTENDED:
		*out_str = GET_STR(copy, address->extended);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(address)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_address_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_address_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_address_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_address_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_messenger_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_messenger_s *messenger = (ctsvc_messenger_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_MESSENGER_LABEL:
		*out_str = GET_STR(copy, messenger->label);
		break;
	case CTSVC_PROPERTY_MESSENGER_IM_ID:
		*out_str = GET_STR(copy, messenger->im_id);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(messenger)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_messenger_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_messenger_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_messenger_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_messenger_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_group_relation_get_str_real(contacts_record_h record,
		unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_group_relation_s *group_relation = (ctsvc_group_relation_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_GROUP_RELATION_GROUP_NAME:
		*out_str = GET_STR(copy, group_relation->group_name);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(group_relation)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_relation_get_str_p(contacts_record_h record,
		unsigned int property_id, char **out_str)
{
	return __ctsvc_group_relation_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_group_relation_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_group_relation_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_activity_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_activity_s *activity = (ctsvc_activity_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ACTIVITY_SOURCE_NAME:
		*out_str = GET_STR(copy, activity->source_name);
		break;
	case CTSVC_PROPERTY_ACTIVITY_STATUS:
		*out_str = GET_STR(copy, activity->status);
		break;
	case CTSVC_PROPERTY_ACTIVITY_SERVICE_OPERATION:
		*out_str = GET_STR(copy, activity->service_operation);
		break;
	case CTSVC_PROPERTY_ACTIVITY_URI:
		*out_str = GET_STR(copy, activity->uri);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(activity)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_activity_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_activity_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_activity_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_activity_photo_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_activity_photo_s *photo = (ctsvc_activity_photo_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ACTIVITY_PHOTO_URL:
		*out_str = GET_STR(copy, photo->photo_url);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(activity)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_photo_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_activity_photo_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_activity_photo_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_activity_photo_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_profile_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_profile_s *profile = (ctsvc_profile_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_PROFILE_UID:
		*out_str = GET_STR(copy, profile->uid);
		break;
	case CTSVC_PROPERTY_PROFILE_TEXT:
		*out_str = GET_STR(copy, profile->text);
		break;
	case CTSVC_PROPERTY_PROFILE_SERVICE_OPERATION:
		*out_str = GET_STR(copy, profile->service_operation);
		break;
	case CTSVC_PROPERTY_PROFILE_MIME:
		*out_str = GET_STR(copy, profile->mime);
		break;
	case CTSVC_PROPERTY_PROFILE_APP_ID:
		*out_str = GET_STR(copy, profile->app_id);
		break;
	case CTSVC_PROPERTY_PROFILE_URI:
		*out_str = GET_STR(copy, profile->uri);
		break;
	case CTSVC_PROPERTY_PROFILE_CATEGORY:
		*out_str = GET_STR(copy, profile->category);
		break;
	case CTSVC_PROPERTY_PROFILE_EXTRA_DATA:
		*out_str = GET_STR(copy, profile->extra_data);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(profile)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_profile_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_profile_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_profile_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_profile_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_relationship_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_relationship_s *relationship = (ctsvc_relationship_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_RELATIONSHIP_LABEL:
		*out_str = GET_STR(copy, relationship->label);
		break;
	case CTSVC_PROPERTY_RELATIONSHIP_NAME:
		*out_str = GET_STR(copy, relationship->name);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(relationship)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_relationship_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_relationship_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_relationship_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_relationship_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_image_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_image_s *image = (ctsvc_image_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_IMAGE_LABEL:
		*out_str = GET_STR(copy, image->label);
		break;
	case CTSVC_PROPERTY_IMAGE_PATH:
		*out_str = GET_STR(copy, image->path);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(image)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_image_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_image_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_image_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_image_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_extension_get_str_real(contacts_record_h record, unsigned int property_id, char **out_str, bool copy)
{
	ctsvc_extension_s *extension = (ctsvc_extension_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EXTENSION_DATA2:
		*out_str = GET_STR(copy, extension->data2);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA3:
		*out_str = GET_STR(copy, extension->data3);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA4:
		*out_str = GET_STR(copy, extension->data4);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA5:
		*out_str = GET_STR(copy, extension->data5);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA6:
		*out_str = GET_STR(copy, extension->data6);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA7:
		*out_str = GET_STR(copy, extension->data7);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA8:
		*out_str = GET_STR(copy, extension->data8);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA9:
		*out_str = GET_STR(copy, extension->data9);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA10:
		*out_str = GET_STR(copy, extension->data10);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA11:
		*out_str = GET_STR(copy, extension->data11);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA12:
		*out_str = GET_STR(copy, extension->data12);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(extension)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_extension_get_str_p(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_extension_get_str_real(record, property_id, out_str, false);
}

static int __ctsvc_extension_get_str(contacts_record_h record, unsigned int property_id, char **out_str)
{
	return __ctsvc_extension_get_str_real(record, property_id, out_str, true);
}

static int __ctsvc_contact_set_str(contacts_record_h record, unsigned int property_id,
		const char *str, bool *is_dirty)
{
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_CONTACT_DISPLAY_NAME:
		CHECK_DIRTY_STR(contact->display_name, str, is_dirty);
		FREEandSTRDUP(contact->display_name, str);
		break;
		/*
		   CTS_ERR("property_id(%d) is a read-only value (contact)", property_id);
		   return CONTACTS_ERROR_INVALID_PARAMETER;
		   */
	case CTSVC_PROPERTY_CONTACT_RINGTONE:
		CHECK_DIRTY_STR(contact->ringtone_path, str, is_dirty);
		FREEandSTRDUP(contact->ringtone_path, str);
		break;
	case CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL:
		CHECK_DIRTY_STR(contact->image_thumbnail_path, str, is_dirty);
		FREEandSTRDUP(contact->image_thumbnail_path, str);
		break;
	case CTSVC_PROPERTY_CONTACT_UID:
		CHECK_DIRTY_STR(contact->uid, str, is_dirty);
		FREEandSTRDUP(contact->uid, str);
		break;
	case CTSVC_PROPERTY_CONTACT_VIBRATION:
		CHECK_DIRTY_STR(contact->vibration, str, is_dirty);
		FREEandSTRDUP(contact->vibration, str);
		break;
	case CTSVC_PROPERTY_CONTACT_MESSAGE_ALERT:
		CHECK_DIRTY_STR(contact->message_alert, str, is_dirty);
		FREEandSTRDUP(contact->message_alert, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}


static int __ctsvc_simple_contact_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_simple_contact_s *contact = (ctsvc_simple_contact_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_CONTACT_DISPLAY_NAME:
		CHECK_DIRTY_STR(contact->display_name, str, is_dirty);
		FREEandSTRDUP(contact->display_name, str);
		break;
		/*
		   CTS_ERR("property_id(%d) is a read-only value (contact)", property_id);
		   return CONTACTS_ERROR_INVALID_PARAMETER;
		   */
	case CTSVC_PROPERTY_CONTACT_RINGTONE:
		CHECK_DIRTY_STR(contact->ringtone_path, str, is_dirty);
		FREEandSTRDUP(contact->ringtone_path, str);
		break;
	case CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL:
		CHECK_DIRTY_STR(contact->image_thumbnail_path, str, is_dirty);
		FREEandSTRDUP(contact->image_thumbnail_path, str);
		break;
	case CTSVC_PROPERTY_CONTACT_UID:
		CHECK_DIRTY_STR(contact->uid, str, is_dirty);
		FREEandSTRDUP(contact->uid, str);
		break;
	case CTSVC_PROPERTY_CONTACT_VIBRATION:
		CHECK_DIRTY_STR(contact->vibration, str, is_dirty);
		FREEandSTRDUP(contact->vibration, str);
		break;
	case CTSVC_PROPERTY_CONTACT_MESSAGE_ALERT:
		CHECK_DIRTY_STR(contact->message_alert, str, is_dirty);
		FREEandSTRDUP(contact->message_alert, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(simple_contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_name_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_name_s *name = (ctsvc_name_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NAME_FIRST:
		CHECK_DIRTY_STR(name->first, str, is_dirty);
		FREEandSTRDUP(name->first, str);
		break;
	case CTSVC_PROPERTY_NAME_LAST:
		CHECK_DIRTY_STR(name->last, str, is_dirty);
		FREEandSTRDUP(name->last, str);
		break;
	case CTSVC_PROPERTY_NAME_ADDITION:
		CHECK_DIRTY_STR(name->addition, str, is_dirty);
		FREEandSTRDUP(name->addition, str);
		break;
	case CTSVC_PROPERTY_NAME_SUFFIX:
		CHECK_DIRTY_STR(name->suffix, str, is_dirty);
		FREEandSTRDUP(name->suffix, str);
		break;
	case CTSVC_PROPERTY_NAME_PREFIX:
		CHECK_DIRTY_STR(name->prefix, str, is_dirty);
		FREEandSTRDUP(name->prefix, str);
		break;
	case CTSVC_PROPERTY_NAME_PHONETIC_FIRST:
		CHECK_DIRTY_STR(name->phonetic_first, str, is_dirty);
		FREEandSTRDUP(name->phonetic_first, str);
		break;
	case CTSVC_PROPERTY_NAME_PHONETIC_MIDDLE:
		CHECK_DIRTY_STR(name->phonetic_middle, str, is_dirty);
		FREEandSTRDUP(name->phonetic_middle, str);
		break;
	case CTSVC_PROPERTY_NAME_PHONETIC_LAST:
		CHECK_DIRTY_STR(name->phonetic_last, str, is_dirty);
		FREEandSTRDUP(name->phonetic_last, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(name)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_company_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_company_s *company = (ctsvc_company_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_COMPANY_LABEL:
		CHECK_DIRTY_STR(company->label, str, is_dirty);
		FREEandSTRDUP(company->label, str);
		break;
	case CTSVC_PROPERTY_COMPANY_NAME:
		CHECK_DIRTY_STR(company->name, str, is_dirty);
		FREEandSTRDUP(company->name, str);
		break;
	case CTSVC_PROPERTY_COMPANY_DEPARTMENT:
		CHECK_DIRTY_STR(company->department, str, is_dirty);
		FREEandSTRDUP(company->department, str);
		break;
	case CTSVC_PROPERTY_COMPANY_JOB_TITLE:
		CHECK_DIRTY_STR(company->job_title, str, is_dirty);
		FREEandSTRDUP(company->job_title, str);
		break;
	case CTSVC_PROPERTY_COMPANY_ASSISTANT_NAME:
		CHECK_DIRTY_STR(company->assistant_name, str, is_dirty);
		FREEandSTRDUP(company->assistant_name, str);
		break;
	case CTSVC_PROPERTY_COMPANY_ROLE:
		CHECK_DIRTY_STR(company->role, str, is_dirty);
		FREEandSTRDUP(company->role, str);
		break;
	case CTSVC_PROPERTY_COMPANY_LOGO:
		if (company->logo && company->is_vcard && (NULL == str || STRING_EQUAL != strcmp(company->logo, str))) {
			company->is_vcard = false;
			__ctsvc_temp_image_hash_table_remove(company->logo);
		}
		CHECK_DIRTY_STR(company->logo, str, is_dirty);
		FREEandSTRDUP(company->logo, str);
		break;
	case CTSVC_PROPERTY_COMPANY_LOCATION:
		CHECK_DIRTY_STR(company->location, str, is_dirty);
		FREEandSTRDUP(company->location, str);
		break;
	case CTSVC_PROPERTY_COMPANY_DESCRIPTION:
		CHECK_DIRTY_STR(company->description, str, is_dirty);
		FREEandSTRDUP(company->description, str);
		break;
	case CTSVC_PROPERTY_COMPANY_PHONETIC_NAME:
		CHECK_DIRTY_STR(company->phonetic_name, str, is_dirty);
		FREEandSTRDUP(company->phonetic_name, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(company)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_note_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_note_s *note = (ctsvc_note_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NOTE_NOTE:
		CHECK_DIRTY_STR(note->note, str, is_dirty);
		FREEandSTRDUP(note->note, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(note)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_number_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_number_s *number = (ctsvc_number_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NUMBER_LABEL:
		CHECK_DIRTY_STR(number->label, str, is_dirty);
		FREEandSTRDUP(number->label, str);
		break;
	case CTSVC_PROPERTY_NUMBER_NUMBER:
		CHECK_DIRTY_STR(number->number, str, is_dirty);
		FREEandSTRDUP(number->number, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(number)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_email_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_email_s *email = (ctsvc_email_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EMAIL_EMAIL:
		CHECK_DIRTY_STR(email->email_addr, str, is_dirty);
		FREEandSTRDUP(email->email_addr, str);
		break;
	case CTSVC_PROPERTY_EMAIL_LABEL:
		CHECK_DIRTY_STR(email->label, str, is_dirty);
		FREEandSTRDUP(email->label, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(email)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_url_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_url_s *url = (ctsvc_url_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_URL_URL:
		CHECK_DIRTY_STR(url->url, str, is_dirty);
		FREEandSTRDUP(url->url, str);
		break;
	case CTSVC_PROPERTY_URL_LABEL:
		CHECK_DIRTY_STR(url->label, str, is_dirty);
		FREEandSTRDUP(url->label, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(url)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_event_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_event_s *event = (ctsvc_event_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EVENT_LABEL:
		CHECK_DIRTY_STR(event->label, str, is_dirty);
		FREEandSTRDUP(event->label, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(event)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_nickname_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_nickname_s *nickname = (ctsvc_nickname_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NICKNAME_NAME:
		CHECK_DIRTY_STR(nickname->nickname, str, is_dirty);
		FREEandSTRDUP(nickname->nickname, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(nickname)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_address_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_address_s *address = (ctsvc_address_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ADDRESS_LABEL:
		CHECK_DIRTY_STR(address->label, str, is_dirty);
		FREEandSTRDUP(address->label, str);
		break;
	case CTSVC_PROPERTY_ADDRESS_POSTBOX:
		CHECK_DIRTY_STR(address->pobox, str, is_dirty);
		FREEandSTRDUP(address->pobox, str);
		break;
	case CTSVC_PROPERTY_ADDRESS_POSTAL_CODE:
		CHECK_DIRTY_STR(address->postalcode, str, is_dirty);
		FREEandSTRDUP(address->postalcode, str);
		break;
	case CTSVC_PROPERTY_ADDRESS_REGION:
		CHECK_DIRTY_STR(address->region, str, is_dirty);
		FREEandSTRDUP(address->region, str);
		break;
	case CTSVC_PROPERTY_ADDRESS_LOCALITY:
		CHECK_DIRTY_STR(address->locality, str, is_dirty);
		FREEandSTRDUP(address->locality, str);
		break;
	case CTSVC_PROPERTY_ADDRESS_STREET:
		CHECK_DIRTY_STR(address->street, str, is_dirty);
		FREEandSTRDUP(address->street, str);
		break;
	case CTSVC_PROPERTY_ADDRESS_COUNTRY:
		CHECK_DIRTY_STR(address->country, str, is_dirty);
		FREEandSTRDUP(address->country, str);
		break;
	case CTSVC_PROPERTY_ADDRESS_EXTENDED:
		CHECK_DIRTY_STR(address->extended, str, is_dirty);
		FREEandSTRDUP(address->extended, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(address)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_messenger_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_messenger_s *messenger = (ctsvc_messenger_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_MESSENGER_LABEL:
		CHECK_DIRTY_STR(messenger->label, str, is_dirty);
		FREEandSTRDUP(messenger->label, str);
		break;
	case CTSVC_PROPERTY_MESSENGER_IM_ID:
		CHECK_DIRTY_STR(messenger->im_id, str, is_dirty);
		FREEandSTRDUP(messenger->im_id, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(messenger)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_relation_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_group_relation_s *group_relation = (ctsvc_group_relation_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_GROUP_RELATION_GROUP_NAME:
		CHECK_DIRTY_STR(group_relation->group_name, str, is_dirty);
		FREEandSTRDUP(group_relation->group_name, str);
		break;
		/*
		   CTS_ERR("property_id(%d) is a read-only value (group_relation)", property_id);
		   return CONTACTS_ERROR_INVALID_PARAMETER;
		   */
	default:
		CTS_ERR("property_id(%d) is not supported in value(group_relation)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_set_str(contacts_record_h record, unsigned int property_id, const char *str, bool *is_dirty)
{
	ctsvc_activity_s *activity = (ctsvc_activity_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ACTIVITY_SOURCE_NAME:
		CHECK_DIRTY_STR(activity->source_name, str, is_dirty);
		FREEandSTRDUP(activity->source_name, str);
		break;
	case CTSVC_PROPERTY_ACTIVITY_STATUS:
		CHECK_DIRTY_STR(activity->status, str, is_dirty);
		FREEandSTRDUP(activity->status, str);
		break;
	case CTSVC_PROPERTY_ACTIVITY_SERVICE_OPERATION:
		CHECK_DIRTY_STR(activity->service_operation, str, is_dirty);
		FREEandSTRDUP(activity->service_operation, str);
		break;
	case CTSVC_PROPERTY_ACTIVITY_URI:
		CHECK_DIRTY_STR(activity->uri, str, is_dirty);
		FREEandSTRDUP(activity->uri, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(activity)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_photo_set_str(contacts_record_h record, unsigned int property_id,
		const char *str, bool *is_dirty)
{
	ctsvc_activity_photo_s *photo = (ctsvc_activity_photo_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ACTIVITY_PHOTO_URL:
		CHECK_DIRTY_STR(photo->photo_url, str, is_dirty);
		FREEandSTRDUP(photo->photo_url, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(activity)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_profile_set_str(contacts_record_h record, unsigned int property_id,
		const char *str, bool *is_dirty)
{
	ctsvc_profile_s *profile = (ctsvc_profile_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_PROFILE_UID:
		CHECK_DIRTY_STR(profile->uid, str, is_dirty);
		FREEandSTRDUP(profile->uid, str);
		break;
	case CTSVC_PROPERTY_PROFILE_TEXT:
		CHECK_DIRTY_STR(profile->text, str, is_dirty);
		FREEandSTRDUP(profile->text, str);
		break;
	case CTSVC_PROPERTY_PROFILE_SERVICE_OPERATION:
		CHECK_DIRTY_STR(profile->service_operation, str, is_dirty);
		FREEandSTRDUP(profile->service_operation, str);
		break;
	case CTSVC_PROPERTY_PROFILE_MIME:
		CHECK_DIRTY_STR(profile->mime, str, is_dirty);
		FREEandSTRDUP(profile->mime, str);
		break;
	case CTSVC_PROPERTY_PROFILE_APP_ID:
		CHECK_DIRTY_STR(profile->app_id, str, is_dirty);
		FREEandSTRDUP(profile->app_id, str);
		break;
	case CTSVC_PROPERTY_PROFILE_URI:
		CHECK_DIRTY_STR(profile->uri, str, is_dirty);
		FREEandSTRDUP(profile->uri, str);
		break;
	case CTSVC_PROPERTY_PROFILE_CATEGORY:
		CHECK_DIRTY_STR(profile->category, str, is_dirty);
		FREEandSTRDUP(profile->category, str);
		break;
	case CTSVC_PROPERTY_PROFILE_EXTRA_DATA:
		CHECK_DIRTY_STR(profile->extra_data, str, is_dirty);
		FREEandSTRDUP(profile->extra_data, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(profile)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_relationship_set_str(contacts_record_h record, unsigned int property_id,
		const char *str, bool *is_dirty)
{
	ctsvc_relationship_s *relationship = (ctsvc_relationship_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_RELATIONSHIP_LABEL:
		CHECK_DIRTY_STR(relationship->label, str, is_dirty);
		FREEandSTRDUP(relationship->label, str);
		break;
	case CTSVC_PROPERTY_RELATIONSHIP_NAME:
		CHECK_DIRTY_STR(relationship->name, str, is_dirty);
		FREEandSTRDUP(relationship->name, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(relationship)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_image_set_str(contacts_record_h record, unsigned int property_id,
		const char *str, bool *is_dirty)
{
	ctsvc_image_s *image = (ctsvc_image_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_IMAGE_LABEL:
		CHECK_DIRTY_STR(image->label, str, is_dirty);
		FREEandSTRDUP(image->label, str);
		break;
	case CTSVC_PROPERTY_IMAGE_PATH:
		if (image->path && image->is_vcard
				&& (NULL == str || STRING_EQUAL != strcmp(image->path, str))) {
			image->is_vcard = false;
			__ctsvc_temp_image_hash_table_remove(image->path);
		}
		CHECK_DIRTY_STR(image->path, str, is_dirty);
		FREEandSTRDUP(image->path, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(image)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_extension_set_str(contacts_record_h record, unsigned int property_id,
		const char *str, bool *is_dirty)
{
	ctsvc_extension_s *extension = (ctsvc_extension_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EXTENSION_DATA2:
		CHECK_DIRTY_STR(extension->data2, str, is_dirty);
		FREEandSTRDUP(extension->data2, str);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA3:
		CHECK_DIRTY_STR(extension->data3, str, is_dirty);
		FREEandSTRDUP(extension->data3, str);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA4:
		CHECK_DIRTY_STR(extension->data4, str, is_dirty);
		FREEandSTRDUP(extension->data4, str);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA5:
		CHECK_DIRTY_STR(extension->data5, str, is_dirty);
		FREEandSTRDUP(extension->data5, str);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA6:
		CHECK_DIRTY_STR(extension->data6, str, is_dirty);
		FREEandSTRDUP(extension->data6, str);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA7:
		CHECK_DIRTY_STR(extension->data7, str, is_dirty);
		FREEandSTRDUP(extension->data7, str);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA8:
		CHECK_DIRTY_STR(extension->data8, str, is_dirty);
		FREEandSTRDUP(extension->data8, str);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA9:
		CHECK_DIRTY_STR(extension->data9, str, is_dirty);
		FREEandSTRDUP(extension->data9, str);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA10:
		CHECK_DIRTY_STR(extension->data10, str, is_dirty);
		FREEandSTRDUP(extension->data10, str);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA11:
		CHECK_DIRTY_STR(extension->data11, str, is_dirty);
		FREEandSTRDUP(extension->data11, str);
		break;
	case CTSVC_PROPERTY_EXTENSION_DATA12:
		CHECK_DIRTY_STR(extension->data12, str, is_dirty);
		FREEandSTRDUP(extension->data12, str);
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(extension)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_contact_get_bool(contacts_record_h record, unsigned int property_id, bool *value)
{
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;
	switch (property_id) {
	case CTSVC_PROPERTY_CONTACT_IS_FAVORITE:
		*value = contact->is_favorite;
		break;
	case CTSVC_PROPERTY_CONTACT_HAS_PHONENUMBER:
		*value = contact->has_phonenumber;
		break;
	case CTSVC_PROPERTY_CONTACT_HAS_EMAIL:
		*value = contact->has_email;
		break;
	default:
		CTS_ERR("property_id(0x%x) is not supported in value(contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_simple_contact_get_bool(contacts_record_h record, unsigned int property_id, bool *value)
{
	ctsvc_simple_contact_s *contact = (ctsvc_simple_contact_s*)record;
	switch (property_id) {
	case CTSVC_PROPERTY_CONTACT_IS_FAVORITE:
		*value = contact->is_favorite;
		break;
	case CTSVC_PROPERTY_CONTACT_HAS_PHONENUMBER:
		*value = contact->has_phonenumber;
		break;
	case CTSVC_PROPERTY_CONTACT_HAS_EMAIL:
		*value = contact->has_email;
		break;
	default:
		CTS_ERR("property_id(0x%x) is not supported in value(contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}


static int __ctsvc_number_get_bool(contacts_record_h record, unsigned int property_id, bool *value)
{
	ctsvc_number_s *number = (ctsvc_number_s*)record;
	switch (property_id) {
	case CTSVC_PROPERTY_NUMBER_IS_DEFAULT:
		*value = number->is_default;
		break;
	default:
		CTS_ERR("property_id(0x%x) is not supported in value(number)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_email_get_bool(contacts_record_h record, unsigned int property_id, bool *value)
{
	ctsvc_email_s *email = (ctsvc_email_s*)record;
	switch (property_id) {
	case CTSVC_PROPERTY_EMAIL_IS_DEFAULT:
		*value = email->is_default;
		break;
	default:
		CTS_ERR("property_id(0x%x) is not supported in value(email)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_event_get_bool(contacts_record_h record, unsigned int property_id, bool *value)
{
	ctsvc_event_s *event = (ctsvc_event_s*)record;
	switch (property_id) {
	case CTSVC_PROPERTY_EVENT_IS_LEAP_MONTH: /* deprecated */
		*value = event->is_leap_month;
		break;
	default:
		CTS_ERR("property_id(0x%x) is not supported in value(event)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_image_get_bool(contacts_record_h record, unsigned int property_id, bool *value)
{
	ctsvc_image_s *image = (ctsvc_image_s*)record;
	switch (property_id) {
	case CTSVC_PROPERTY_IMAGE_IS_DEFAULT:
		*value = image->is_default;
		break;
	default:
		CTS_ERR("property_id(0x%x) is not supported in value(image)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_address_get_bool(contacts_record_h record, unsigned int property_id, bool *value)
{
	ctsvc_address_s *address = (ctsvc_address_s*)record;
	switch (property_id) {
	case CTSVC_PROPERTY_ADDRESS_IS_DEFAULT:
		*value = address->is_default;
		break;
	default:
		CTS_ERR("property_id(0x%x) is not supported in value(address)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_contact_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty)
{
	ctsvc_contact_s *contact = (ctsvc_contact_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_CONTACT_IS_FAVORITE:
		CHECK_DIRTY_VAL(contact->is_favorite, value, is_dirty);
		contact->is_favorite = value;
		break;
	case CTSVC_PROPERTY_CONTACT_HAS_PHONENUMBER:
		CHECK_DIRTY_VAL(contact->has_phonenumber, value, is_dirty);
		contact->has_phonenumber = value;
		break;
	case CTSVC_PROPERTY_CONTACT_HAS_EMAIL:
		CHECK_DIRTY_VAL(contact->has_email, value, is_dirty);
		contact->has_email = value;
		break;
		/*
		   CTS_ERR("property_id(%d) is a read-only value(contact)", property_id);
		   break;
		   */
	default:
		CTS_ERR("property_id(0x%x) is not supported in value(contact)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_number_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty)
{
	ctsvc_number_s *number = (ctsvc_number_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_NUMBER_IS_DEFAULT:
		CHECK_DIRTY_VAL(number->is_default, value, is_dirty);
		number->is_default = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(number)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_email_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty)
{
	ctsvc_email_s *email = (ctsvc_email_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EMAIL_IS_DEFAULT:
		CHECK_DIRTY_VAL(email->is_default, value, is_dirty);
		email->is_default = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(email)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_event_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty)
{
	ctsvc_event_s *event = (ctsvc_event_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_EVENT_IS_LEAP_MONTH:
		CHECK_DIRTY_VAL(event->is_leap_month, value, is_dirty);
		event->is_leap_month = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(event)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_image_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty)
{
	ctsvc_image_s *image = (ctsvc_image_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_IMAGE_IS_DEFAULT:
		CHECK_DIRTY_VAL(image->is_default, value, is_dirty);
		image->is_default = value;
		break;
	default:
		CTS_ERR("property_id(0x%x) is not supported in value(image)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_address_set_bool(contacts_record_h record, unsigned int property_id, bool value, bool *is_dirty)
{
	ctsvc_address_s *address = (ctsvc_address_s*)record;

	switch (property_id) {
	case CTSVC_PROPERTY_ADDRESS_IS_DEFAULT:
		CHECK_DIRTY_VAL(address->is_default, value, is_dirty);
		address->is_default = value;
		break;
	default:
		CTS_ERR("property_id(%d) is not supported in value(address)", property_id);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_contact_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_contact_s *out_data = NULL;
	ctsvc_contact_s *src_data = NULL;

	src_data = (ctsvc_contact_s*)record;
	out_data = calloc(1, sizeof(ctsvc_contact_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->person_id = src_data->person_id;
	out_data->addressbook_id = src_data->addressbook_id;
	out_data->changed_time = src_data->changed_time;
	out_data->changed_ver = src_data->changed_ver;
	out_data->link_mode = src_data->link_mode;
	out_data->display_source_type = src_data->display_source_type;
	out_data->display_name_language = src_data->display_name_language;
	out_data->reverse_display_name_language = src_data->reverse_display_name_language;
	out_data->has_phonenumber = src_data->has_phonenumber;
	out_data->has_email = src_data->has_email;
	out_data->is_favorite = src_data->is_favorite;

	out_data->display_name = SAFE_STRDUP(src_data->display_name);
	out_data->reverse_display_name = SAFE_STRDUP(src_data->reverse_display_name);
	out_data->uid = SAFE_STRDUP(src_data->uid);
	out_data->ringtone_path = SAFE_STRDUP(src_data->ringtone_path);
	out_data->vibration = SAFE_STRDUP(src_data->vibration);
	out_data->message_alert = SAFE_STRDUP(src_data->message_alert);
	out_data->image_thumbnail_path = SAFE_STRDUP(src_data->image_thumbnail_path);
	out_data->sort_name = SAFE_STRDUP(src_data->sort_name);
	out_data->reverse_sort_name = SAFE_STRDUP(src_data->reverse_sort_name);
	out_data->sortkey = SAFE_STRDUP(src_data->sortkey);
	out_data->reverse_sortkey = SAFE_STRDUP(src_data->reverse_sortkey);

	ctsvc_list_clone((contacts_list_h)src_data->name, (contacts_list_h*)&out_data->name);
	out_data->name->l_type = CTSVC_RECORD_NAME;

	ctsvc_list_clone((contacts_list_h)src_data->company, (contacts_list_h*)&out_data->company);
	out_data->company->l_type = CTSVC_RECORD_COMPANY;

	ctsvc_list_clone((contacts_list_h)src_data->note, (contacts_list_h*)&out_data->note);
	out_data->note->l_type = CTSVC_RECORD_NOTE;

	ctsvc_list_clone((contacts_list_h)src_data->numbers, (contacts_list_h*)&out_data->numbers);
	out_data->numbers->l_type = CTSVC_RECORD_NUMBER;

	ctsvc_list_clone((contacts_list_h)src_data->emails, (contacts_list_h*)&out_data->emails);
	out_data->emails->l_type = CTSVC_RECORD_EMAIL;

	ctsvc_list_clone((contacts_list_h)src_data->grouprelations, (contacts_list_h*)&out_data->grouprelations);
	out_data->grouprelations->l_type = CTSVC_RECORD_GROUP_RELATION;

	ctsvc_list_clone((contacts_list_h)src_data->events, (contacts_list_h*)&out_data->events);
	out_data->events->l_type = CTSVC_RECORD_EVENT;

	ctsvc_list_clone((contacts_list_h)src_data->messengers, (contacts_list_h*)&out_data->messengers);
	out_data->messengers->l_type = CTSVC_RECORD_MESSENGER;

	ctsvc_list_clone((contacts_list_h)src_data->postal_addrs, (contacts_list_h*)&out_data->postal_addrs);
	out_data->postal_addrs->l_type = CTSVC_RECORD_ADDRESS;

	ctsvc_list_clone((contacts_list_h)src_data->urls, (contacts_list_h*)&out_data->urls);
	out_data->urls->l_type = CTSVC_RECORD_URL;

	ctsvc_list_clone((contacts_list_h)src_data->nicknames, (contacts_list_h*)&out_data->nicknames);
	out_data->nicknames->l_type = CTSVC_RECORD_NICKNAME;

	ctsvc_list_clone((contacts_list_h)src_data->profiles, (contacts_list_h*)&out_data->profiles);
	out_data->profiles->l_type = CTSVC_RECORD_PROFILE;

	ctsvc_list_clone((contacts_list_h)src_data->relationships, (contacts_list_h*)&out_data->relationships);
	out_data->relationships->l_type = CTSVC_RECORD_RELATIONSHIP;

	ctsvc_list_clone((contacts_list_h)src_data->images, (contacts_list_h*)&out_data->images);
	out_data->images->l_type = CTSVC_RECORD_IMAGE;

	ctsvc_list_clone((contacts_list_h)src_data->extensions, (contacts_list_h*)&out_data->extensions);
	out_data->extensions->l_type = CTSVC_RECORD_EXTENSION;

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_contact_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;

	return CONTACTS_ERROR_NONE;
}


static int __ctsvc_activity_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_activity_s *out_data = NULL;
	ctsvc_activity_s *src_data = NULL;

	src_data = (ctsvc_activity_s*)record;
	out_data = calloc(1, sizeof(ctsvc_activity_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->timestamp = src_data->timestamp;
	out_data->source_name = SAFE_STRDUP(src_data->source_name);
	out_data->status = SAFE_STRDUP(src_data->status);
	out_data->service_operation = SAFE_STRDUP(src_data->service_operation);
	out_data->uri = SAFE_STRDUP(src_data->uri);

	ctsvc_list_clone((contacts_list_h)src_data->photos, (contacts_list_h*)&out_data->photos);
	out_data->photos->l_type = CTSVC_RECORD_ACTIVITY_PHOTO;

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_activity_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_activity_photo_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_activity_photo_s *out_data = NULL;
	ctsvc_activity_photo_s *src_data = NULL;

	src_data = (ctsvc_activity_photo_s*)record;
	out_data = calloc(1, sizeof(ctsvc_activity_photo_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->activity_id = src_data->activity_id;
	out_data->photo_url = SAFE_STRDUP(src_data->photo_url);
	out_data->sort_index = src_data->sort_index;

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_activity_photo_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_address_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_address_s *out_data = NULL;
	ctsvc_address_s *src_data = NULL;

	src_data = (ctsvc_address_s*)record;
	out_data = calloc(1, sizeof(ctsvc_address_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->type = src_data->type;
	out_data->is_default = src_data->is_default;
	out_data->label = SAFE_STRDUP(src_data->label);
	out_data->pobox = SAFE_STRDUP(src_data->pobox);
	out_data->postalcode = SAFE_STRDUP(src_data->postalcode);
	out_data->region = SAFE_STRDUP(src_data->region);
	out_data->locality = SAFE_STRDUP(src_data->locality);
	out_data->street = SAFE_STRDUP(src_data->street);
	out_data->extended = SAFE_STRDUP(src_data->extended);
	out_data->country = SAFE_STRDUP(src_data->country);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_address_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_company_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_company_s *out_data = NULL;
	ctsvc_company_s *src_data = NULL;

	src_data = (ctsvc_company_s*)record;
	out_data = calloc(1, sizeof(ctsvc_company_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->is_default = src_data->is_default;
	out_data->type = src_data->type;
	out_data->is_vcard = src_data->is_vcard;
	out_data->label = SAFE_STRDUP(src_data->label);
	out_data->name = SAFE_STRDUP(src_data->name);
	out_data->department = SAFE_STRDUP(src_data->department);
	out_data->job_title = SAFE_STRDUP(src_data->job_title);
	out_data->role = SAFE_STRDUP(src_data->role);
	out_data->assistant_name = SAFE_STRDUP(src_data->assistant_name);
	if (src_data->logo && src_data->is_vcard)
		__ctsvc_temp_image_hash_table_insert(src_data->logo);
	out_data->logo = SAFE_STRDUP(src_data->logo);
	out_data->location = SAFE_STRDUP(src_data->location);
	out_data->description = SAFE_STRDUP(src_data->description);
	out_data->phonetic_name = SAFE_STRDUP(src_data->phonetic_name);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_company_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_email_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_email_s *out_data = NULL;
	ctsvc_email_s *src_data = NULL;

	src_data = (ctsvc_email_s*)record;
	out_data = calloc(1, sizeof(ctsvc_email_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->is_default = src_data->is_default;
	out_data->contact_id = src_data->contact_id;
	out_data->type = src_data->type;
	out_data->label = SAFE_STRDUP(src_data->label);
	out_data->email_addr = SAFE_STRDUP(src_data->email_addr);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_email_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_event_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_event_s *out_data = NULL;
	ctsvc_event_s *src_data = NULL;

	src_data = (ctsvc_event_s*)record;
	out_data = calloc(1, sizeof(ctsvc_event_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->type = src_data->type;
	out_data->label = SAFE_STRDUP(src_data->label);
	out_data->date = src_data->date;
	out_data->calendar_type = src_data->calendar_type;
	out_data->is_leap_month = src_data->is_leap_month;

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_event_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_extension_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_extension_s *out_data = NULL;
	ctsvc_extension_s *src_data = NULL;

	src_data = (ctsvc_extension_s*)record;
	out_data = calloc(1, sizeof(ctsvc_extension_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	/* out_data->is_default = src_data->is_default; */
	out_data->data1 = src_data->data1;
	out_data->data2 = SAFE_STRDUP(src_data->data2);
	out_data->data3 = SAFE_STRDUP(src_data->data3);
	out_data->data4 = SAFE_STRDUP(src_data->data4);
	out_data->data5 = SAFE_STRDUP(src_data->data5);
	out_data->data6 = SAFE_STRDUP(src_data->data6);
	out_data->data7 = SAFE_STRDUP(src_data->data7);
	out_data->data8 = SAFE_STRDUP(src_data->data8);
	out_data->data9 = SAFE_STRDUP(src_data->data9);
	out_data->data10 = SAFE_STRDUP(src_data->data10);
	out_data->data11 = SAFE_STRDUP(src_data->data11);
	out_data->data12 = SAFE_STRDUP(src_data->data12);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_extension_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_group_relation_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_group_relation_s *out_data = NULL;
	ctsvc_group_relation_s *src_data = NULL;

	src_data = (ctsvc_group_relation_s*)record;
	out_data = calloc(1, sizeof(ctsvc_group_relation_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->group_id = src_data->group_id;
	out_data->contact_id = src_data->contact_id;
	out_data->group_name = SAFE_STRDUP(src_data->group_name);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_group_relation_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_messenger_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_messenger_s *out_data = NULL;
	ctsvc_messenger_s *src_data = NULL;

	src_data = (ctsvc_messenger_s*)record;
	out_data = calloc(1, sizeof(ctsvc_messenger_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->type = src_data->type;
	out_data->label = SAFE_STRDUP(src_data->label);
	out_data->im_id = SAFE_STRDUP(src_data->im_id);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_messenger_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_name_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_name_s *out_data = NULL;
	ctsvc_name_s *src_data = NULL;

	src_data = (ctsvc_name_s*)record;

	out_data = calloc(1, sizeof(ctsvc_name_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->is_default = src_data->is_default;
	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->language_type = src_data->language_type;
	out_data->first = SAFE_STRDUP(src_data->first);
	out_data->last = SAFE_STRDUP(src_data->last);
	out_data->addition = SAFE_STRDUP(src_data->addition);
	out_data->prefix = SAFE_STRDUP(src_data->prefix);
	out_data->suffix = SAFE_STRDUP(src_data->suffix);
	out_data->phonetic_first = SAFE_STRDUP(src_data->phonetic_first);
	out_data->phonetic_middle = SAFE_STRDUP(src_data->phonetic_middle);
	out_data->phonetic_last = SAFE_STRDUP(src_data->phonetic_last);
	out_data->lookup = SAFE_STRDUP(src_data->lookup);
	out_data->reverse_lookup = SAFE_STRDUP(src_data->reverse_lookup);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_name_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_nickname_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_nickname_s *out_data = NULL;
	ctsvc_nickname_s *src_data = NULL;

	src_data = (ctsvc_nickname_s*)record;
	out_data = calloc(1, sizeof(ctsvc_nickname_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->type = src_data->type;
	out_data->label = SAFE_STRDUP(src_data->label);
	out_data->nickname = SAFE_STRDUP(src_data->nickname);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_nickname_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_note_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_note_s *out_data = NULL;
	ctsvc_note_s *src_data = NULL;

	src_data = (ctsvc_note_s*)record;
	out_data = calloc(1, sizeof(ctsvc_note_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->note = SAFE_STRDUP(src_data->note);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_note_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_number_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_number_s *out_data = NULL;
	ctsvc_number_s *src_data = NULL;

	src_data = (ctsvc_number_s*)record;
	out_data = calloc(1, sizeof(ctsvc_number_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->is_default = src_data->is_default;
	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->type = src_data->type;
	out_data->label = SAFE_STRDUP(src_data->label);
	out_data->number = SAFE_STRDUP(src_data->number);
	out_data->lookup = SAFE_STRDUP(src_data->lookup);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_number_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_profile_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_profile_s *out_data = NULL;
	ctsvc_profile_s *src_data = NULL;

	src_data = (ctsvc_profile_s*)record;
	out_data = calloc(1, sizeof(ctsvc_profile_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->order = src_data->order;
	out_data->uid = SAFE_STRDUP(src_data->uid);
	out_data->text = SAFE_STRDUP(src_data->text);
	out_data->service_operation = SAFE_STRDUP(src_data->service_operation);
	out_data->mime = SAFE_STRDUP(src_data->mime);
	out_data->app_id = SAFE_STRDUP(src_data->app_id);
	out_data->uri = SAFE_STRDUP(src_data->uri);
	out_data->category = SAFE_STRDUP(src_data->category);
	out_data->extra_data = SAFE_STRDUP(src_data->extra_data);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_profile_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_relationship_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_relationship_s *out_data = NULL;
	ctsvc_relationship_s *src_data = NULL;

	src_data = (ctsvc_relationship_s*)record;
	out_data = calloc(1, sizeof(ctsvc_relationship_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->type = src_data->type;
	out_data->label = SAFE_STRDUP(src_data->label);
	out_data->name = SAFE_STRDUP(src_data->name);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_relationship_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_image_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_image_s *out_data = NULL;
	ctsvc_image_s *src_data = NULL;

	src_data = (ctsvc_image_s*)record;
	out_data = calloc(1, sizeof(ctsvc_image_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->is_default = src_data->is_default;
	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->type = src_data->type;
	out_data->is_vcard = src_data->is_vcard;
	out_data->label = SAFE_STRDUP(src_data->label);
	if (src_data->path && src_data->is_vcard)
		__ctsvc_temp_image_hash_table_insert(src_data->path);
	out_data->path = SAFE_STRDUP(src_data->path);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_image_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_simple_contact_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_simple_contact_s *out_data = NULL;
	ctsvc_simple_contact_s *src_data = NULL;

	src_data = (ctsvc_simple_contact_s*)record;
	out_data = calloc(1, sizeof(ctsvc_simple_contact_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->contact_id = src_data->contact_id;
	out_data->person_id = src_data->person_id;
	out_data->addressbook_id = src_data->addressbook_id;
	out_data->changed_time = src_data->changed_time;
	out_data->display_source_type = src_data->display_source_type;
	out_data->has_phonenumber = src_data->has_phonenumber;
	out_data->has_email = src_data->has_email;
	out_data->is_favorite = src_data->is_favorite;

	out_data->display_name = SAFE_STRDUP(src_data->display_name);
	out_data->uid = SAFE_STRDUP(src_data->uid);
	out_data->ringtone_path = SAFE_STRDUP(src_data->ringtone_path);
	out_data->vibration = SAFE_STRDUP(src_data->vibration);
	out_data->message_alert = SAFE_STRDUP(src_data->message_alert);
	out_data->image_thumbnail_path = SAFE_STRDUP(src_data->image_thumbnail_path);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_simple_contact_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_url_clone(contacts_record_h record, contacts_record_h *out_record)
{
	ctsvc_url_s *out_data = NULL;
	ctsvc_url_s *src_data = NULL;

	src_data = (ctsvc_url_s*)record;
	out_data = calloc(1, sizeof(ctsvc_url_s));
	RETVM_IF(NULL == out_data, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	out_data->id = src_data->id;
	out_data->contact_id = src_data->contact_id;
	out_data->type = src_data->type;
	out_data->label = SAFE_STRDUP(src_data->label);
	out_data->url = SAFE_STRDUP(src_data->url);

	int ret = ctsvc_record_copy_base(&(out_data->base), &(src_data->base));
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_record_copy_base() Fail");
		__ctsvc_url_destroy((contacts_record_h)out_data, true);
		return ret;
	}

	*out_record = (contacts_record_h)out_data;
	return CONTACTS_ERROR_NONE;
}


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

#ifndef __CTSVC_DB_PLUGIN_CONTACT_HELPER_H__
#define __CTSVC_DB_PLUGIN_CONTACT_HELPER_H__

#include "ctsvc_struct.h"
#include "ctsvc_db_sqlite.h"

int ctsvc_contact_add_image_file(int parent_id, int img_id,
		char *src_img, char *dest_name, int dest_size);
int ctsvc_contact_update_image_file(int parent_id, int img_id,
		char *src_img, char *dest_name, int dest_size);
int ctsvc_contact_delete_image_file_with_path(const unsigned char *image_path);

void ctsvc_contact_make_display_name(ctsvc_contact_s *contact);
int ctsvc_contact_make_search_name(ctsvc_contact_s *contact, char **search_name);
int ctsvc_contact_get_name_language(const ctsvc_name_s *name);

int ctsvc_db_contact_update_changed_time(int contact_id);
int ctsvc_contact_update_display_name(int contact_id, contacts_display_name_source_type_e changed_record_type);

int ctsvc_db_contact_delete(int contact_id);
int ctsvc_db_contact_get(int id, contacts_record_h *out_record);

int ctsvc_get_data_info_name(cts_stmt stmt, contacts_list_h name_list);
int ctsvc_get_data_info_event(cts_stmt stmt, contacts_list_h list);
int ctsvc_get_data_info_number(cts_stmt stmt, contacts_list_h number_list);
int ctsvc_get_data_info_email(cts_stmt stmt, contacts_list_h list);
int ctsvc_get_data_info_address(cts_stmt stmt, contacts_list_h list);
int ctsvc_get_data_info_messenger(cts_stmt stmt, contacts_list_h list);
int ctsvc_get_data_info_note(cts_stmt stmt, contacts_list_h list);
int ctsvc_get_data_info_company(cts_stmt stmt, contacts_list_h list);
int ctsvc_get_data_info_profile(cts_stmt stmt, contacts_list_h list);
int ctsvc_get_data_info_relationship(cts_stmt stmt, contacts_list_h list);
int ctsvc_get_data_info_image(cts_stmt stmt, contacts_list_h list);
int ctsvc_get_data_info_url(cts_stmt stmt, contacts_list_h list);
int ctsvc_get_data_info_nickname(cts_stmt stmt, contacts_list_h list);
int ctsvc_get_data_info_extension(cts_stmt stmt, contacts_list_h list);

bool ctsvc_contact_check_default_number(contacts_list_h number_list);
bool ctsvc_contact_check_default_email(contacts_list_h email_list);
bool ctsvc_contact_check_default_address(contacts_list_h address_list);
bool ctsvc_contact_check_default_image(contacts_list_h image_list);
bool ctsvc_contact_check_image_location(const char *path);

int ctsvc_contact_update_data_name(contacts_list_h name_list, int contact_id, bool is_my_profile);
int ctsvc_contact_update_data_company(contacts_list_h company_list, int contact_id, bool is_my_profile);
int ctsvc_contact_update_data_note(contacts_list_h note_list, int contact_id, bool is_my_profile);
int ctsvc_contact_update_data_event(contacts_list_h event_list, int contact_id, bool is_my_profile);
int ctsvc_contact_update_data_messenger(contacts_list_h messenger_list, int contact_id, bool is_my_profile);
int ctsvc_contact_update_data_address(contacts_list_h address_list, int contact_id, bool is_my_profile);
int ctsvc_contact_update_data_url(contacts_list_h url_list, int contact_id, bool is_my_profile);
int ctsvc_contact_update_data_profile(contacts_list_h profile_list, int contact_id, bool is_my_profile);
int ctsvc_contact_update_data_relationship(contacts_list_h relationship_list, int contact_id, bool is_my_profile);
int ctsvc_contact_update_data_image(contacts_list_h image_list, int contact_id, bool is_my_profile);
int ctsvc_contact_update_data_nickname(contacts_list_h nickname_list, int contact_id, bool is_my_profile);
int ctsvc_contact_update_data_extension(contacts_list_h extension_list, int contact_id, bool is_my_profile);
int ctsvc_contact_update_data_number(contacts_list_h number_list, int contact_id, bool is_my_profile, bool *had_phonenumber);
int ctsvc_contact_update_data_email(contacts_list_h email_list, int contact_id, bool is_my_profile, bool *had_email);

int ctsvc_contact_insert_data_name(contacts_list_h name_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_number(contacts_list_h number_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_email(contacts_list_h email_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_profile(contacts_list_h profile_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_company(contacts_list_h company_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_note(contacts_list_h note_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_event(contacts_list_h event_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_messenger(contacts_list_h messenger_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_address(contacts_list_h address_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_url(contacts_list_h url_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_nickname(contacts_list_h nickname_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_relationship(contacts_list_h relationship_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_image(contacts_list_h image_list, int contact_id, bool is_my_profile);
int ctsvc_contact_insert_data_extension(contacts_list_h extension_list, int contact_id, bool is_my_profile);

#endif /* __CTSVC_DB_PLUGIN_CONTACT_HELPER_H__ */

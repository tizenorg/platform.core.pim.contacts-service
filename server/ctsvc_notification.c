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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "contacts.h"
#include "contacts_db_status.h"
#include "ctsvc_internal.h"
#include "ctsvc_notify.h"
#include "ctsvc_notification.h"

static TLS bool contact_change = false;
static TLS bool my_profile_change = false;
static TLS bool phonelog_change = false;
static TLS bool speed_change = false;
static TLS bool addressbook_change = false;
static TLS bool group_change = false;
static TLS bool group_rel_change = false;
static TLS bool person_change = false;
static TLS bool activity_change = false;
static TLS bool activity_photo_change = false;
static TLS bool address_change = false;
static TLS bool data_change = false;
static TLS bool event_change = false;
static TLS bool number_change = false;
static TLS bool email_change = false;
static TLS bool messenger_change = false;
static TLS bool name_change = false;
static TLS bool note_change = false;
static TLS bool url_change = false;
static TLS bool nickname_change = false;
static TLS bool sdn_change = false;
static TLS bool relationship_change = false;
static TLS bool image_change = false;
static TLS bool profile_change = false;
static TLS bool company_change = false;

void ctsvc_noti_publish_socket_initialize(void)
{
	int fd = open(CTSVC_NOTI_IPC_READY, O_TRUNC | O_RDWR);

	if (0 <= fd)
		close(fd);
}

static inline void __ctsvc_noti_publish_contact_change(void)
{
	int fd = open(CTSVC_NOTI_CONTACT_CHANGED, O_TRUNC | O_RDWR);

	if (0 <= fd) {
		close(fd);
		contact_change = false;
	}
}

static inline void __ctsvc_noti_publish_my_profile_change(void)
{
	int fd = open(CTSVC_NOTI_MY_PROFILE_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		my_profile_change = false;
	}
}

static inline void __ctsvc_noti_publish_phonelog_change(void)
{
	int fd = open(CTSVC_NOTI_PHONELOG_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		phonelog_change = false;
	}
}

static inline void __ctsvc_noti_publish_speed_change(void)
{
	int fd = open(CTSVC_NOTI_SPEEDDIAL_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		speed_change = false;
	}
}

static inline void __ctsvc_noti_publish_addressbook_change(void)
{
	int fd = open(CTSVC_NOTI_ADDRESSBOOK_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		addressbook_change = false;
	}
}

static inline void __ctsvc_noti_publish_group_change(void)
{
	int fd = open(CTSVC_NOTI_GROUP_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		group_change = false;
	}
}

static inline void __ctsvc_noti_publish_group_rel_change(void)
{
	int fd = open(CTSVC_NOTI_GROUP_RELATION_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		group_rel_change = false;
	}
}

static inline void __ctsvc_noti_publish_person_change(void)
{
	int fd = open(CTSVC_NOTI_PERSON_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		person_change = false;
	}
}

static inline void __ctsvc_noti_publish_name_change(void)
{
	int fd = open(CTSVC_NOTI_NAME_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		name_change = false;
	}
}

static inline void __ctsvc_noti_publish_number_change(void)
{
	int fd = open(CTSVC_NOTI_NUMBER_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		number_change = false;
	}
}

static inline void __ctsvc_noti_publish_email_change(void)
{
	int fd = open(CTSVC_NOTI_EMAIL_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		email_change = false;
	}
}

static inline void __ctsvc_noti_publish_event_change(void)
{
	int fd = open(CTSVC_NOTI_EVENT_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		event_change = false;
	}
}

static inline void __ctsvc_noti_publish_url_change(void)
{
	int fd = open(CTSVC_NOTI_URL_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		url_change = false;
	}
}

static inline void __ctsvc_noti_publish_address_change(void)
{
	int fd = open(CTSVC_NOTI_ADDRESS_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		address_change = false;
	}
}

static inline void __ctsvc_noti_publish_note_change(void)
{
	int fd = open(CTSVC_NOTI_NOTE_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		note_change = false;
	}
}

static inline void __ctsvc_noti_publish_company_change(void)
{
	int fd = open(CTSVC_NOTI_COMPANY_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		company_change = false;
	}
}

static inline void __ctsvc_noti_publish_relationship_change(void)
{
	int fd = open(CTSVC_NOTI_RELATIONSHIP_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		relationship_change = false;
	}
}

static inline void __ctsvc_noti_publish_image_change(void)
{
	int fd = open(CTSVC_NOTI_IMAGE_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		image_change = false;
	}
}

static inline void __ctsvc_noti_publish_nickname_change(void)
{
	int fd = open(CTSVC_NOTI_NICKNAME_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		nickname_change = false;
	}
}

static inline void __ctsvc_noti_publish_messenger_change(void)
{
	int fd = open(CTSVC_NOTI_MESSENGER_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		messenger_change = false;
	}
}

static inline void __ctsvc_noti_publish_data_change(void)
{
	int fd = open(CTSVC_NOTI_DATA_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		data_change = false;
	}
}

static inline void __ctsvc_noti_publish_sdn_change(void)
{
	int fd = open(CTSVC_NOTI_SDN_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		sdn_change = false;
	}
}

static inline void __ctsvc_noti_publish_profile_change(void)
{
	int fd = open(CTSVC_NOTI_PROFILE_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		profile_change = false;
	}
}

static inline void __ctsvc_noti_publish_activity_change(void)
{
	int fd = open(CTSVC_NOTI_ACTIVITY_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		activity_change = false;
	}
}

static inline void __ctsvc_noti_publish_activity_photo_change(void)
{
	int fd = open(CTSVC_NOTI_ACTIVITY_PHOTO_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		activity_photo_change = false;
	}
}

void ctsvc_nofitication_cancel(void)
{
	contact_change = false;
	my_profile_change = false;
	phonelog_change = false;
	speed_change = false;
	addressbook_change = false;
	group_change = false;
	group_rel_change = false;
	person_change = false;
	activity_change = false;
	activity_photo_change = false;
	address_change = false;
	data_change = false;
	event_change = false;
	number_change = false;
	email_change = false;
	messenger_change = false;
	name_change = false;
	note_change = false;
	url_change = false;
	nickname_change = false;
	sdn_change = false;
	relationship_change = false;
	image_change = false;
	profile_change = false;
	company_change = false;
}

void ctsvc_set_contact_noti(void)
{
	contact_change = true;
}

void ctsvc_set_my_profile_noti(void)
{
	my_profile_change = true;
}

void ctsvc_set_phonelog_noti(void)
{
	phonelog_change = true;
}

void ctsvc_set_speed_noti(void)
{
	speed_change = true;
}

void ctsvc_set_addressbook_noti(void)
{
	addressbook_change = true;
}

void ctsvc_set_group_noti(void)
{
	group_change = true;
}

void ctsvc_set_group_rel_noti(void)
{
	group_rel_change = true;
}

void ctsvc_set_person_noti(void)
{
	person_change = true;
}

void ctsvc_set_activity_noti(void)
{
	activity_change = true;
}

void ctsvc_set_activity_photo_noti(void)
{
	activity_photo_change = true;
}

void ctsvc_set_address_noti(void)
{
	address_change = true;
}

void ctsvc_set_data_noti(void)
{
	data_change = true;
}

void ctsvc_set_event_noti(void)
{
	event_change = true;
}

void ctsvc_set_number_noti(void)
{
	number_change = true;
}

void ctsvc_set_email_noti(void)
{
	email_change = true;
}

void ctsvc_set_messenger_noti(void)
{
	messenger_change = true;
}

void ctsvc_set_nickname_noti(void)
{
	nickname_change = true;
}

void ctsvc_set_name_noti(void)
{
	name_change = true;
}

void ctsvc_set_note_noti(void)
{
	note_change = true;
}

void ctsvc_set_url_noti(void)
{
	url_change = true;
}

void ctsvc_set_sdn_noti(void)
{
	sdn_change = true;
}

void ctsvc_set_relationship_noti(void)
{
	relationship_change = true;
}

void ctsvc_set_image_noti(void)
{
	image_change = true;
}

void ctsvc_set_profile_noti(void)
{
	profile_change = true;
}

void ctsvc_set_company_noti(void)
{
	company_change = true;
}

void ctsvc_notification_send()
{
	if (contact_change) __ctsvc_noti_publish_contact_change();
	if (my_profile_change) __ctsvc_noti_publish_my_profile_change();
	if (phonelog_change) __ctsvc_noti_publish_phonelog_change();
	if (speed_change) __ctsvc_noti_publish_speed_change();
	if (addressbook_change) __ctsvc_noti_publish_addressbook_change();
	if (group_change) __ctsvc_noti_publish_group_change();
	if (group_rel_change) __ctsvc_noti_publish_group_rel_change();
	if (person_change) __ctsvc_noti_publish_person_change();
	if (activity_change) __ctsvc_noti_publish_activity_change();
	if (activity_photo_change) __ctsvc_noti_publish_activity_photo_change();
	if (address_change) __ctsvc_noti_publish_address_change();
	if (data_change) __ctsvc_noti_publish_data_change();
	if (company_change) __ctsvc_noti_publish_company_change();
	if (event_change) __ctsvc_noti_publish_event_change();
	if (number_change) __ctsvc_noti_publish_number_change();
	if (email_change) __ctsvc_noti_publish_email_change();
	if (messenger_change) __ctsvc_noti_publish_messenger_change();
	if (name_change) __ctsvc_noti_publish_name_change();
	if (note_change) __ctsvc_noti_publish_note_change();
	if (url_change) __ctsvc_noti_publish_url_change();
	if (nickname_change) __ctsvc_noti_publish_nickname_change();
	if (sdn_change) __ctsvc_noti_publish_sdn_change();
	if (relationship_change) __ctsvc_noti_publish_relationship_change();
	if (image_change) __ctsvc_noti_publish_image_change();
	if (profile_change) __ctsvc_noti_publish_profile_change();
}

/*
 * Whenever deleting data table record, this function will be called
 * in order to set notification
 */
void ctsvc_db_data_delete_callback(sqlite3_context  *context,
		int argc, sqlite3_value **argv)
{
	CTS_FN_CALL;
	int datatype;

	if (2 < argc) {
		sqlite3_result_null(context);
		return;
	}

	datatype = sqlite3_value_int(argv[1]);

	switch (datatype) {
	case CTSVC_DATA_NAME:
		ctsvc_set_name_noti();
		break;
	case CTSVC_DATA_POSTAL:
		ctsvc_set_address_noti();
		break;
	case CTSVC_DATA_MESSENGER:
		ctsvc_set_messenger_noti();
		break;
	case CTSVC_DATA_URL:
		ctsvc_set_url_noti();
		break;
	case CTSVC_DATA_EVENT:
		ctsvc_set_event_noti();
		break;
	case CTSVC_DATA_COMPANY:
		ctsvc_set_company_noti();
		break;
	case CTSVC_DATA_NICKNAME:
		ctsvc_set_nickname_noti();
		break;
	case CTSVC_DATA_NUMBER:
		ctsvc_set_number_noti();
		break;
	case CTSVC_DATA_EMAIL:
		ctsvc_set_email_noti();
		break;
	case CTSVC_DATA_PROFILE:
		ctsvc_set_profile_noti();
		break;
	case CTSVC_DATA_RELATIONSHIP:
		ctsvc_set_relationship_noti();
		break;
	case CTSVC_DATA_NOTE:
		ctsvc_set_note_noti();
		break;
	case CTSVC_DATA_IMAGE:
		ctsvc_set_image_noti();
		break;
	case CTSVC_DATA_EXTENSION:
		ctsvc_set_data_noti();
		break;
	default:
		break;
	}
	sqlite3_result_null(context);
	CTS_FN_END;
}

API int contacts_db_add_status_changed_cb(
		contacts_db_status_changed_cb cb, void *user_data)
{
	CTS_ERR("Please use contacts-service2 instead of contacts-service3");
	return CONTACTS_ERROR_INTERNAL;
}

API int contacts_db_remove_status_changed_cb(
		contacts_db_status_changed_cb cb, void *user_data)
{
	CTS_ERR("Please use contacts-service2 instead of contacts-service3");
	return CONTACTS_ERROR_INTERNAL;
}

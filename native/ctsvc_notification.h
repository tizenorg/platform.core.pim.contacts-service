/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __TIZEN_SOCIAL_CTSVC_NOTIFICATION_H__
#define __TIZEN_SOCIAL_CTSVC_NOTIFICATION_H__

#include "ctsvc_sqlite.h"

void ctsvc_set_contact_noti(void);
void ctsvc_set_my_profile_noti(void);
void ctsvc_set_phonelog_noti(void);
void ctsvc_set_speed_noti(void);
void ctsvc_set_addressbook_noti(void);
void ctsvc_set_group_noti(void);
void ctsvc_set_group_rel_noti(void);
void ctsvc_set_person_noti(void);
void ctsvc_set_data_noti(void);
void ctsvc_set_activity_noti(void);
void ctsvc_set_activity_photo_noti(void);
void ctsvc_set_address_noti(void);
void ctsvc_set_event_noti(void);
void ctsvc_set_messenger_noti(void);
void ctsvc_set_number_noti(void);
void ctsvc_set_email_noti(void);
void ctsvc_set_name_noti(void);
void ctsvc_set_note_noti(void);
void ctsvc_set_url_noti(void);
void ctsvc_set_nickname_noti(void);
void ctsvc_set_relationship_noti(void);
void ctsvc_set_image_noti(void);
void ctsvc_set_profile_noti(void);
void ctsvc_set_sdn_noti(void);
void ctsvc_set_company_noti(void);

void ctsvc_notification_send();
void ctsvc_nofitication_cancel(void);

void ctsvc_db_data_delete_callback(sqlite3_context * context,
		int argc, sqlite3_value ** argv);

#endif /* __TIZEN_SOCIAL_CTSVC_NOTIFICATION_H__ */

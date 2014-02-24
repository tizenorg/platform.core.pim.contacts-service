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
#include <tzplatform_config.h>

#ifndef __TIZEN_SOCIAL_CTSVC_NOTIFY_H__
#define __TIZEN_SOCIAL_CTSVC_NOTIFY_H__

#define CTSVC_NOTI_REPERTORY tzplatform_mkpath(TZ_USER_DATA,"contacts-svc")
#define CTSVC_NOTI_IMG_REPERTORY tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/img/")
#define CTSVC_NOTI_ADDRESSBOOK_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_AB_CHANGED")
#define CTSVC_NOTI_GROUP_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_GROUP_CHANGED")
#define CTSVC_NOTI_PERSON_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_PERSON_CHANGED")
#define CTSVC_NOTI_CONTACT_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_DB_CHANGED")
#define CTSVC_NOTI_MY_PROFILE_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_MY_PROFILE_CHANGED")
#define CTSVC_NOTI_NAME_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_NAME_CHANGED")
#define CTSVC_NOTI_NUMBER_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_NUMBER_CHANGED")
#define CTSVC_NOTI_EMAIL_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_EMAIL_CHANGED")
#define CTSVC_NOTI_EVENT_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_EVENT_CHANGED")
#define CTSVC_NOTI_URL_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_URL_CHANGED")
#define CTSVC_NOTI_GROUP_RELATION_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_GROUP_RELATION_CHANGED")
#define CTSVC_NOTI_ADDRESS_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_ADDRESS_CHANGED")
#define CTSVC_NOTI_NOTE_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_NOTE_CHANGED")
#define CTSVC_NOTI_COMPANY_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_COMPANY_CHANGED")
#define CTSVC_NOTI_RELATIONSHIP_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_RELATIONSHIP_CHANGED")
#define CTSVC_NOTI_IMAGE_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_IMAGE_CHANGED")
#define CTSVC_NOTI_NICKNAME_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_NICKNAME_CHANGED")
#define CTSVC_NOTI_MESSENGER_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_MESSENGER_CHANGED")
#define CTSVC_NOTI_DATA_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_DATA_CHANGED")
#define CTSVC_NOTI_SDN_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_SDN_CHANGED")
#define CTSVC_NOTI_PROFILE_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_PROFILE_CHANGED")
#define CTSVC_NOTI_ACTIVITY_CHANGED tzplatform_mkpath(TZ_USER_DATA, "contacts-svc/.CONTACTS_SVC_ACTIVITY_CHANGED")
#define CTSVC_NOTI_PHONELOG_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_PLOG_CHANGED")
#define CTSVC_NOTI_SPEEDDIAL_CHANGED tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/.CONTACTS_SVC_SPEED_CHANGED")

#define CTSVC_SETTING_DISPLAY_ORDER_CHANGED "contacts.setting.display_order"
#define CTSVC_SETTING_SORTING_ORDER_CHANGED "contacts.setting.sorting_order"

#endif /* __TIZEN_SOCIAL_CTSVC_NOTIFY_H__ */

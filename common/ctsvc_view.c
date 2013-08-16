/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Dohyung Jin <dh.jin@samsung.com>
 *                 Jongwon Lee <gogosing.lee@samsung.com>
 *                 Donghee Ye <donghee.ye@samsung.com>
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
#include "ctsvc_view.h"

API const _contacts_address_book_property_ids _contacts_address_book = {
	._uri		= CTSVC_VIEW_URI_ADDRESSBOOK,
	.id			= CTSVC_PROPERTY_ADDRESSBOOK_ID,
	.account_id	= CTSVC_PROPERTY_ADDRESSBOOK_ACCOUNT_ID,
	.name		= CTSVC_PROPERTY_ADDRESSBOOK_NAME,
	.mode		= CTSVC_PROPERTY_ADDRESSBOOK_MODE
};

API const _contacts_group_property_ids _contacts_group = {
	._uri			= CTSVC_VIEW_URI_GROUP,
	.id				= CTSVC_PROPERTY_GROUP_ID,
	.address_book_id	= CTSVC_PROPERTY_GROUP_ADDRESSBOOK_ID,
	.name			= CTSVC_PROPERTY_GROUP_NAME,
	.ringtone_path	= CTSVC_PROPERTY_GROUP_RINGTONE,
	.image_path		= CTSVC_PROPERTY_GROUP_IMAGE,
	.vibration		= CTSVC_PROPERTY_GROUP_VIBRATION,
	.extra_data		= CTSVC_PROPERTY_GROUP_EXTRA_DATA,
	.is_read_only	= CTSVC_PROPERTY_GROUP_IS_READ_ONLY,
	.message_alert = CTSVC_PROPERTY_GROUP_MESSAGE_ALERT
};

API const _contacts_person_property_ids _contacts_person = {
	._uri					= CTSVC_VIEW_URI_PERSON,
	.id						= CTSVC_PROPERTY_PERSON_ID,
	.display_name			= CTSVC_PROPERTY_PERSON_DISPLAY_NAME,
	.display_name_index		= CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,
	.display_contact_id		= CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,
	.ringtone_path			= CTSVC_PROPERTY_PERSON_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,
	.vibration				= CTSVC_PROPERTY_PERSON_VIBRATION,
	.message_alert		= CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,
	.status					= CTSVC_PROPERTY_PERSON_STATUS,
	.is_favorite			= CTSVC_PROPERTY_PERSON_IS_FAVORITE,
	.favorite_priority		= CTSVC_PROPERTY_PERSON_FAVORITE_PRIORITY,
	.link_count				= CTSVC_PROPERTY_PERSON_LINK_COUNT,
	.addressbook_ids		= CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS,
	.has_phonenumber		= CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,
	.has_email				= CTSVC_PROPERTY_PERSON_HAS_EMAIL,
};

API const _contacts_contact_property_ids _contacts_contact = {
	._uri					= CTSVC_VIEW_URI_CONTACT,
	.id						= CTSVC_PROPERTY_CONTACT_ID,
	.display_name			= CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,
	.display_source_type	= CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,
	.address_book_id		= CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,
	.ringtone_path			= CTSVC_PROPERTY_CONTACT_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,
	.is_favorite			= CTSVC_PROPERTY_CONTACT_IS_FAVORITE,
	.has_phonenumber		= CTSVC_PROPERTY_CONTACT_HAS_PHONENUMBER,
	.has_email				= CTSVC_PROPERTY_CONTACT_HAS_EMAIL,
	.person_id				= CTSVC_PROPERTY_CONTACT_PERSON_ID,
	.uid					= CTSVC_PROPERTY_CONTACT_UID,
	.vibration				= CTSVC_PROPERTY_CONTACT_VIBRATION,
	.message_alert		= CTSVC_PROPERTY_CONTACT_MESSAGE_ALERT,
	.changed_time			= CTSVC_PROPERTY_CONTACT_CHANGED_TIME,
	.link_mode		= CTSVC_PROPERTY_CONTACT_LINK_MODE,
	.name					= CTSVC_PROPERTY_CONTACT_NAME,
	.company				= CTSVC_PROPERTY_CONTACT_COMPANY,
	.note					= CTSVC_PROPERTY_CONTACT_NOTE,
	.number					= CTSVC_PROPERTY_CONTACT_NUMBER,
	.email					= CTSVC_PROPERTY_CONTACT_EMAIL,
	.event					= CTSVC_PROPERTY_CONTACT_EVENT,
	.messenger				= CTSVC_PROPERTY_CONTACT_MESSENGER,
	.address				= CTSVC_PROPERTY_CONTACT_ADDRESS,
	.url					= CTSVC_PROPERTY_CONTACT_URL,
	.nickname				= CTSVC_PROPERTY_CONTACT_NICKNAME,
	.profile				= CTSVC_PROPERTY_CONTACT_PROFILE,
	.relationship			= CTSVC_PROPERTY_CONTACT_RELATIONSHIP,
	.image					= CTSVC_PROPERTY_CONTACT_IMAGE,
	.group_relation			= CTSVC_PROPERTY_CONTACT_GROUP_RELATION,
	.extension				= CTSVC_PROPERTY_CONTACT_EXTENSION,
	.is_unknown				= CTSVC_PROPERTY_CONTACT_IS_UNKNOWN,
};

API const _contacts_unknown_property_ids _contacts_unknown = {
        ._uri                                   = CTSVC_VIEW_URI_CONTACT_INCLUDE_UNKNOWN,
        .id                                             = CTSVC_PROPERTY_CONTACT_ID,
        .display_name                   = CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,
        .display_source_type    = CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,
        .address_book_id                = CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,
        .ringtone_path                  = CTSVC_PROPERTY_CONTACT_RINGTONE,
        .image_thumbnail_path   = CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,
        .is_favorite                    = CTSVC_PROPERTY_CONTACT_IS_FAVORITE,
        .has_phonenumber                = CTSVC_PROPERTY_CONTACT_HAS_PHONENUMBER,
        .has_email                              = CTSVC_PROPERTY_CONTACT_HAS_EMAIL,
        .person_id                              = CTSVC_PROPERTY_CONTACT_PERSON_ID,
        .uid                                    = CTSVC_PROPERTY_CONTACT_UID,
        .vibration                              = CTSVC_PROPERTY_CONTACT_VIBRATION,
	.message_alert		= CTSVC_PROPERTY_CONTACT_MESSAGE_ALERT,
        .changed_time                   = CTSVC_PROPERTY_CONTACT_CHANGED_TIME,
	.link_mode		= CTSVC_PROPERTY_CONTACT_LINK_MODE,
        .name                                   = CTSVC_PROPERTY_CONTACT_NAME,
        .company                                = CTSVC_PROPERTY_CONTACT_COMPANY,
        .note                                   = CTSVC_PROPERTY_CONTACT_NOTE,
        .number                                 = CTSVC_PROPERTY_CONTACT_NUMBER,
        .email                                  = CTSVC_PROPERTY_CONTACT_EMAIL,
        .event                                  = CTSVC_PROPERTY_CONTACT_EVENT,
        .messenger                              = CTSVC_PROPERTY_CONTACT_MESSENGER,
        .address                                = CTSVC_PROPERTY_CONTACT_ADDRESS,
        .url                                    = CTSVC_PROPERTY_CONTACT_URL,
        .nickname                               = CTSVC_PROPERTY_CONTACT_NICKNAME,
        .profile                                = CTSVC_PROPERTY_CONTACT_PROFILE,
        .relationship                   = CTSVC_PROPERTY_CONTACT_RELATIONSHIP,
        .image                                  = CTSVC_PROPERTY_CONTACT_IMAGE,
        .group_relation                 = CTSVC_PROPERTY_CONTACT_GROUP_RELATION,
        .extension                              = CTSVC_PROPERTY_CONTACT_EXTENSION,
        .is_unknown                             = CTSVC_PROPERTY_CONTACT_IS_UNKNOWN,
};

API const _contacts_my_profile_property_ids _contacts_my_profile = {
	._uri					= CTSVC_VIEW_URI_MY_PROFILE,
	.id						= CTSVC_PROPERTY_MY_PROFILE_ID,
	.display_name			= CTSVC_PROPERTY_MY_PROFILE_DISPLAY_NAME,
	.address_book_id		= CTSVC_PROPERTY_MY_PROFILE_ADDRESSBOOK_ID,
	.image_thumbnail_path	= CTSVC_PROPERTY_MY_PROFILE_IMAGE_THUMBNAIL,
	.uid					= CTSVC_PROPERTY_MY_PROFILE_UID,
	.changed_time			= CTSVC_PROPERTY_MY_PROFILE_CHANGED_TIME,
	.name					= CTSVC_PROPERTY_MY_PROFILE_NAME,
	.company				= CTSVC_PROPERTY_MY_PROFILE_COMPANY,
	.note					= CTSVC_PROPERTY_MY_PROFILE_NOTE,
	.number					= CTSVC_PROPERTY_MY_PROFILE_NUMBER,
	.email					= CTSVC_PROPERTY_MY_PROFILE_EMAIL,
	.event					= CTSVC_PROPERTY_MY_PROFILE_EVENT,
	.messenger				= CTSVC_PROPERTY_MY_PROFILE_MESSENGER,
	.address				= CTSVC_PROPERTY_MY_PROFILE_ADDRESS,
	.url					= CTSVC_PROPERTY_MY_PROFILE_URL,
	.nickname				= CTSVC_PROPERTY_MY_PROFILE_NICKNAME,
	.profile				= CTSVC_PROPERTY_MY_PROFILE_PROFILE,
	.relationship			= CTSVC_PROPERTY_MY_PROFILE_RELATIONSHIP,
	.image					= CTSVC_PROPERTY_MY_PROFILE_IMAGE,
	.extension				= CTSVC_PROPERTY_MY_PROFILE_EXTENSION,
};

API const _contacts_simple_contact_property_ids _contacts_simple_contact = {
	._uri					= CTSVC_VIEW_URI_SIMPLE_CONTACT,
	.id						= CTSVC_PROPERTY_CONTACT_ID,
	.display_name			= CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,
	.display_source_type	= CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,
	.address_book_id		= CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,
	.person_id				= CTSVC_PROPERTY_CONTACT_PERSON_ID,
	.ringtone_path			= CTSVC_PROPERTY_CONTACT_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,
	.is_favorite			= CTSVC_PROPERTY_CONTACT_IS_FAVORITE,
	.has_phonenumber		= CTSVC_PROPERTY_CONTACT_HAS_PHONENUMBER,
	.has_email				= CTSVC_PROPERTY_CONTACT_HAS_EMAIL,
	.uid					= CTSVC_PROPERTY_CONTACT_UID,
	.vibration				= CTSVC_PROPERTY_CONTACT_VIBRATION,
	.message_alert		= CTSVC_PROPERTY_CONTACT_MESSAGE_ALERT,
	.changed_time			= CTSVC_PROPERTY_CONTACT_CHANGED_TIME,
};

API const _contacts_name_property_ids _contacts_name = {
	._uri			= CTSVC_VIEW_URI_NAME,
	.id				= CTSVC_PROPERTY_NAME_ID,
	.contact_id		= CTSVC_PROPERTY_NAME_CONTACT_ID,
	.first			= CTSVC_PROPERTY_NAME_FIRST,
	.last			= CTSVC_PROPERTY_NAME_LAST,
	.addition		= CTSVC_PROPERTY_NAME_ADDITION,
	.suffix			= CTSVC_PROPERTY_NAME_SUFFIX,
	.prefix			= CTSVC_PROPERTY_NAME_PREFIX,
	.phonetic_first	= CTSVC_PROPERTY_NAME_PHONETIC_FIRST,
	.phonetic_middle= CTSVC_PROPERTY_NAME_PHONETIC_MIDDLE,
	.phonetic_last	= CTSVC_PROPERTY_NAME_PHONETIC_LAST
};

API const _contacts_number_property_ids _contacts_number = {
	._uri		= CTSVC_VIEW_URI_NUMBER,
	.id			= CTSVC_PROPERTY_NUMBER_ID,
	.contact_id	= CTSVC_PROPERTY_NUMBER_CONTACT_ID,
	.type		= CTSVC_PROPERTY_NUMBER_TYPE,
	.label		= CTSVC_PROPERTY_NUMBER_LABEL,
	.is_default	= CTSVC_PROPERTY_NUMBER_IS_DEFAULT,
	.number		= CTSVC_PROPERTY_NUMBER_NUMBER,
	.normalized_number = CTSVC_PROPERTY_NUMBER_NORMALIZED_NUMBER
};

API const _contacts_email_property_ids _contacts_email = {
	._uri		= CTSVC_VIEW_URI_EMAIL,
	.id			= CTSVC_PROPERTY_EMAIL_ID,
	.contact_id	= CTSVC_PROPERTY_EMAIL_CONTACT_ID,
	.type		= CTSVC_PROPERTY_EMAIL_TYPE,
	.label		= CTSVC_PROPERTY_EMAIL_LABEL,
	.is_default	= CTSVC_PROPERTY_EMAIL_IS_DEFAULT,
	.email		= CTSVC_PROPERTY_EMAIL_EMAIL
};

API const _contacts_address_property_ids _contacts_address = {
	._uri			= CTSVC_VIEW_URI_ADDRESS,
	.id				= CTSVC_PROPERTY_ADDRESS_ID,
	.contact_id		= CTSVC_PROPERTY_ADDRESS_CONTACT_ID,
	.type			= CTSVC_PROPERTY_ADDRESS_TYPE,
	.label			= CTSVC_PROPERTY_ADDRESS_LABEL,
	.postbox		= CTSVC_PROPERTY_ADDRESS_POSTBOX,
	.postal_code	= CTSVC_PROPERTY_ADDRESS_POSTAL_CODE,
	.region			= CTSVC_PROPERTY_ADDRESS_REGION,
	.locality		= CTSVC_PROPERTY_ADDRESS_LOCALITY,
	.street			= CTSVC_PROPERTY_ADDRESS_STREET,
	.country		= CTSVC_PROPERTY_ADDRESS_COUNTRY,
	.extended		= CTSVC_PROPERTY_ADDRESS_EXTENDED,
	.is_default		= CTSVC_PROPERTY_ADDRESS_IS_DEFAULT
};

API const _contacts_url_property_ids _contacts_url = {
	._uri		= CTSVC_VIEW_URI_URL,
	.id			= CTSVC_PROPERTY_URL_ID,
	.contact_id	= CTSVC_PROPERTY_URL_CONTACT_ID,
	.type		= CTSVC_PROPERTY_URL_TYPE,
	.label		= CTSVC_PROPERTY_URL_LABEL,
	.url		= CTSVC_PROPERTY_URL_URL
};

API const _contacts_event_property_ids _contacts_event = {
	._uri		= CTSVC_VIEW_URI_EVENT,
	.id			= CTSVC_PROPERTY_EVENT_ID,
	.contact_id	= CTSVC_PROPERTY_EVENT_CONTACT_ID,
	.type		= CTSVC_PROPERTY_EVENT_TYPE,
	.label		= CTSVC_PROPERTY_EVENT_LABEL,
	.date		= CTSVC_PROPERTY_EVENT_DATE,
	.is_lunar	= CTSVC_PROPERTY_EVENT_IS_LUNAR,
	.lunar_date	= CTSVC_PROPERTY_EVENT_LUNAR_DATE
};

API const _contacts_company_property_ids _contacts_company = {
	._uri			= CTSVC_VIEW_URI_COMPANY,
	.id				= CTSVC_PROPERTY_COMPANY_ID,
	.contact_id		= CTSVC_PROPERTY_COMPANY_CONTACT_ID,
	.type			= CTSVC_PROPERTY_COMPANY_TYPE,
	.label			= CTSVC_PROPERTY_COMPANY_LABEL,
	.name			= CTSVC_PROPERTY_COMPANY_NAME,
	.department		= CTSVC_PROPERTY_COMPANY_DEPARTMENT,
	.job_title		= CTSVC_PROPERTY_COMPANY_JOB_TITLE,
	.assistant_name	= CTSVC_PROPERTY_COMPANY_ASSISTANT_NAME,
	.role			= CTSVC_PROPERTY_COMPANY_ROLE,
	.logo			= CTSVC_PROPERTY_COMPANY_LOGO,
	.location		= CTSVC_PROPERTY_COMPANY_LOCATION,
	.description	= CTSVC_PROPERTY_COMPANY_DESCRIPTION,
	.phonetic_name  = CTSVC_PROPERTY_COMPANY_PHONETIC_NAME,
};

API const _contacts_nickname_property_ids _contacts_nickname = {
	._uri		= CTSVC_VIEW_URI_NICKNAME,
	.id			= CTSVC_PROPERTY_NICKNAME_ID,
	.contact_id	= CTSVC_PROPERTY_NICKNAME_CONTACT_ID,
	.name		= CTSVC_PROPERTY_NICKNAME_NAME,
};

API const _contacts_note_property_ids _contacts_note = {
	._uri		= CTSVC_VIEW_URI_NOTE,
	.id			= CTSVC_PROPERTY_NOTE_ID,
	.contact_id	= CTSVC_PROPERTY_NOTE_CONTACT_ID,
	.note		= CTSVC_PROPERTY_NOTE_NOTE
};

API const _contacts_profile_property_ids _contacts_profile = {
	._uri			= CTSVC_VIEW_URI_PROFILE,
	.id			= CTSVC_PROPERTY_PROFILE_ID,
	.uid			= CTSVC_PROPERTY_PROFILE_UID,
	.text			= CTSVC_PROPERTY_PROFILE_TEXT,
	.order		= CTSVC_PROPERTY_PROFILE_ORDER,
	.service_operation	= CTSVC_PROPERTY_PROFILE_SERVICE_OPERATION,
	.mime		= CTSVC_PROPERTY_PROFILE_MIME,
	.app_id		= CTSVC_PROPERTY_PROFILE_APP_ID,
	.uri			= CTSVC_PROPERTY_PROFILE_URI,
	.category		= CTSVC_PROPERTY_PROFILE_CATEGORY,
	.extra_data	= CTSVC_PROPERTY_PROFILE_EXTRA_DATA,
	.contact_id	= CTSVC_PROPERTY_PROFILE_CONTACT_ID
};

API const _contacts_group_relation_property_ids _contacts_group_relation = {
	._uri		= CTSVC_VIEW_URI_GROUP_RELATION,
	.id			= CTSVC_PROPERTY_GROUP_RELATION_ID,
	.group_id	= CTSVC_PROPERTY_GROUP_RELATION_GROUP_ID,
	.contact_id	= CTSVC_PROPERTY_GROUP_RELATION_CONTACT_ID,
	.name		= CTSVC_PROPERTY_GROUP_RELATION_GROUP_NAME,
};

API const _contacts_relationship_property_ids _contacts_relationship = {
	._uri		= CTSVC_VIEW_URI_RELATIONSHIP,
	.id			= CTSVC_PROPERTY_RELATIONSHIP_ID,
	.contact_id	= CTSVC_PROPERTY_RELATIONSHIP_CONTACT_ID,
	.type		= CTSVC_PROPERTY_RELATIONSHIP_TYPE,
	.label		= CTSVC_PROPERTY_RELATIONSHIP_LABEL,
	.name		= CTSVC_PROPERTY_RELATIONSHIP_NAME,
};

API const _contacts_image_property_ids _contacts_image = {
	._uri		= CTSVC_VIEW_URI_IMAGE,
	.id			= CTSVC_PROPERTY_IMAGE_ID,
	.contact_id	= CTSVC_PROPERTY_IMAGE_CONTACT_ID,
	.type		= CTSVC_PROPERTY_IMAGE_TYPE,
	.label		= CTSVC_PROPERTY_IMAGE_LABEL,
	.path		= CTSVC_PROPERTY_IMAGE_PATH,
	.is_default = CTSVC_PROPERTY_IMAGE_IS_DEFAULT,
};

API const _contacts_messenger_property_ids _contacts_messenger = {
	._uri		= CTSVC_VIEW_URI_MESSENGER,
	.id			= CTSVC_PROPERTY_MESSENGER_ID,
	.contact_id	= CTSVC_PROPERTY_MESSENGER_CONTACT_ID,
	.type		= CTSVC_PROPERTY_MESSENGER_TYPE,
	.label		= CTSVC_PROPERTY_MESSENGER_LABEL,
	.im_id		= CTSVC_PROPERTY_MESSENGER_IM_ID,
};

API const _contacts_sdn_property_ids _contacts_sdn = {
	._uri	= CTSVC_VIEW_URI_SDN,
	.id		= CTSVC_PROPERTY_SDN_ID,
	.name	= CTSVC_PROPERTY_SDN_NAME,
	.number	= CTSVC_PROPERTY_SDN_NUMBER,
};

API const _contacts_speeddial_property_ids _contacts_speeddial = {
	._uri					= CTSVC_VIEW_URI_SPEEDDIAL,
	.speeddial_number		= CTSVC_PROPERTY_SPEEDDIAL_DIAL_NUMBER,
	.number_id				= CTSVC_PROPERTY_SPEEDDIAL_NUMBER_ID,
	.number					= CTSVC_PROPERTY_SPEEDDIAL_NUMBER,
	.number_label			= CTSVC_PROPERTY_SPEEDDIAL_NUMBER_LABEL,
	.number_type			= CTSVC_PROPERTY_SPEEDDIAL_NUMBER_TYPE,
	.person_id				= CTSVC_PROPERTY_SPEEDDIAL_PERSON_ID,
	.display_name			= CTSVC_PROPERTY_SPEEDDIAL_DISPLAY_NAME,
	.image_thumbnail_path	= CTSVC_PROPERTY_SPEEDDIAL_IMAGE_THUMBNAIL,
	.normalized_number	= CTSVC_PROPERTY_SPEEDDIAL_NORMALIZED_NUMBER,
};

API const _contacts_contact_updated_info_property_ids _contacts_contact_updated_info = {
	._uri			= CTSVC_VIEW_URI_CONTACTS_UPDATED_INFO,
	.contact_id		= CTSVC_PROPERTY_UPDATE_INFO_ID,
	.address_book_id	= CTSVC_PROPERTY_UPDATE_INFO_ADDRESSBOOK_ID,
	.type			= CTSVC_PROPERTY_UPDATE_INFO_TYPE,
	.version		= CTSVC_PROPERTY_UPDATE_INFO_VERSION,
	.image_changed	= CTSVC_PROPERTY_UPDATE_INFO_IMAGE_CHANGED,
};

API const _contacts_my_profile_updated_info_property_ids _contacts_my_profile_updated_info = {
	._uri			= CTSVC_VIEW_URI_MY_PROFILE_UPDATED_INFO,
	.address_book_id	= CTSVC_PROPERTY_UPDATE_INFO_ADDRESSBOOK_ID,
	.last_changed_type	= CTSVC_PROPERTY_UPDATE_INFO_LAST_CHANGED_TYPE,
	.version		= CTSVC_PROPERTY_UPDATE_INFO_VERSION,
};

API const _contacts_group_updated_info_property_ids _contacts_group_updated_info = {
	._uri			= CTSVC_VIEW_URI_GROUPS_UPDATED_INFO,
	.group_id		= CTSVC_PROPERTY_UPDATE_INFO_ID,
	.address_book_id	= CTSVC_PROPERTY_UPDATE_INFO_ADDRESSBOOK_ID,
	.type			= CTSVC_PROPERTY_UPDATE_INFO_TYPE,
	.version		= CTSVC_PROPERTY_UPDATE_INFO_VERSION,
};

API const _contacts_group_member_updated_info_property_ids _contacts_group_member_updated_info = {
	._uri			= CTSVC_VIEW_URI_GROUPS_MEMBER_UPDATED_INFO,
	.group_id		= CTSVC_PROPERTY_UPDATE_INFO_ID,
	.address_book_id	= CTSVC_PROPERTY_UPDATE_INFO_ADDRESSBOOK_ID,
	.version		= CTSVC_PROPERTY_UPDATE_INFO_VERSION,
};

API const _contacts_grouprel_updated_info_property_ids _contacts_grouprel_updated_info = {
	._uri			= CTSVC_VIEW_URI_GROUPRELS_UPDATED_INFO,
	.group_id		= CTSVC_PROPERTY_GROUP_ID,
	.contact_id		= CTSVC_PROPERTY_CONTACT_ID,
	.address_book_id	= CTSVC_PROPERTY_ADDRESSBOOK_ID,
	.type			= CTSVC_PROPERTY_UPDATE_INFO_TYPE,
	.version		= CTSVC_PROPERTY_UPDATE_INFO_VERSION,
};

API const _contacts_activity_property_ids _contacts_activity = {
	._uri			= CTSVC_VIEW_URI_ACTIVITY,
	.id				= CTSVC_PROPERTY_ACTIVITY_ID,
	.contact_id		= CTSVC_PROPERTY_ACTIVITY_CONTACT_ID,
	.source_name	= CTSVC_PROPERTY_ACTIVITY_SOURCE_NAME,
	.status			= CTSVC_PROPERTY_ACTIVITY_STATUS,
	.timestamp		= CTSVC_PROPERTY_ACTIVITY_TIMESTAMP,
	.service_operation	= CTSVC_PROPERTY_ACTIVITY_SERVICE_OPERATION,
	.uri				= CTSVC_PROPERTY_ACTIVITY_URI,
	.photo			= CTSVC_PROPERTY_ACTIVITY_ACTIVITY_PHOTO,
};

API const _contacts_activity_photo_property_ids _contacts_activity_photo = {
	._uri			= CTSVC_VIEW_URI_ACTIVITY_PHOTO,
	.id				= CTSVC_PROPERTY_ACTIVITY_PHOTO_ID,
	.activity_id	= CTSVC_PROPERTY_ACTIVITY_PHOTO_ACTIVITY_ID,
	.photo_url		= CTSVC_PROPERTY_ACTIVITY_PHOTO_URL,
	.sort_index		= CTSVC_PROPERTY_ACTIVITY_PHOTO_SORT_INDEX,
};

API const _contacts_phone_log_property_ids _contacts_phone_log = {
	._uri			= CTSVC_VIEW_URI_PHONELOG,
	.id				= CTSVC_PROPERTY_PHONELOG_ID,
	.person_id		= CTSVC_PROPERTY_PHONELOG_PERSON_ID,
	.address		= CTSVC_PROPERTY_PHONELOG_ADDRESS,
	.log_time		= CTSVC_PROPERTY_PHONELOG_LOG_TIME,
	.log_type		= CTSVC_PROPERTY_PHONELOG_LOG_TYPE,
	.extra_data1	= CTSVC_PROPERTY_PHONELOG_EXTRA_DATA1,
	.extra_data2	= CTSVC_PROPERTY_PHONELOG_EXTRA_DATA2,
	.normalized_address = CTSVC_PROPERTY_PHONELOG_NORMALIZED_ADDRESS,
};

API const _contacts_extension_property_ids _contacts_extension = {
	._uri		= CTSVC_VIEW_URI_EXTENSION,
	.id			= CTSVC_PROPERTY_EXTENSION_ID,
	.contact_id	= CTSVC_PROPERTY_EXTENSION_CONTACT_ID,
	.data1		= CTSVC_PROPERTY_EXTENSION_DATA1,
	.data2		= CTSVC_PROPERTY_EXTENSION_DATA2,
	.data3		= CTSVC_PROPERTY_EXTENSION_DATA3,
	.data4		= CTSVC_PROPERTY_EXTENSION_DATA4,
	.data5		= CTSVC_PROPERTY_EXTENSION_DATA5,
	.data6		= CTSVC_PROPERTY_EXTENSION_DATA6,
	.data7		= CTSVC_PROPERTY_EXTENSION_DATA7,
	.data8		= CTSVC_PROPERTY_EXTENSION_DATA8,
	.data9		= CTSVC_PROPERTY_EXTENSION_DATA9,
	.data10		= CTSVC_PROPERTY_EXTENSION_DATA10,
	.data11		= CTSVC_PROPERTY_EXTENSION_DATA11,
	.data12		= CTSVC_PROPERTY_EXTENSION_DATA12,
};

API const _contacts_person_contact_property_ids _contacts_person_contact = {
	._uri					= CTSVC_VIEW_URI_READ_ONLY_PERSON_CONTACT,
	.person_id				= CTSVC_PROPERTY_PERSON_ID,
	.display_name			= CTSVC_PROPERTY_PERSON_DISPLAY_NAME,
	.display_name_index		= CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,
	.display_contact_id		= CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,
	.ringtone_path			= CTSVC_PROPERTY_PERSON_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,
	.vibration				= CTSVC_PROPERTY_PERSON_VIBRATION,
	.message_alert		= CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,
	.status					= CTSVC_PROPERTY_PERSON_STATUS,
	.is_favorite			= CTSVC_PROPERTY_PERSON_IS_FAVORITE,
	.link_count				= CTSVC_PROPERTY_PERSON_LINK_COUNT,
	.addressbook_ids		= CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS,
	.has_phonenumber		= CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,
	.has_email				= CTSVC_PROPERTY_PERSON_HAS_EMAIL,
	.contact_id				= CTSVC_PROPERTY_CONTACT_ID,
	.address_book_id		= CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,
	.address_book_name		= CTSVC_PROPERTY_ADDRESSBOOK_NAME,
	.address_book_mode		= CTSVC_PROPERTY_ADDRESSBOOK_MODE
};

API const _contacts_person_contact_include_unknown_property_ids
_contacts_person_contact_include_unknown = {
	._uri = CTSVC_VIEW_URI_READ_ONLY_PERSON_CONTACT_INCLUDE_UNKNOWN,
	.person_id = CTSVC_PROPERTY_PERSON_ID,
	.display_name = CTSVC_PROPERTY_PERSON_DISPLAY_NAME,
	.display_name_index = CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,
	.display_contact_id = CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,
	.ringtone_path = CTSVC_PROPERTY_PERSON_RINGTONE,
	.image_thumbnail_path = CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,
	.vibration = CTSVC_PROPERTY_PERSON_VIBRATION,
	.message_alert = CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,
	.status	= CTSVC_PROPERTY_PERSON_STATUS,
	.is_favorite = CTSVC_PROPERTY_PERSON_IS_FAVORITE,
	.link_count = CTSVC_PROPERTY_PERSON_LINK_COUNT,
	.addressbook_ids = CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS,
	.has_phonenumber = CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,
	.has_email = CTSVC_PROPERTY_PERSON_HAS_EMAIL,
	.contact_id = CTSVC_PROPERTY_CONTACT_ID,
	.address_book_id = CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,
	.address_book_name = CTSVC_PROPERTY_ADDRESSBOOK_NAME,
	.address_book_mode = CTSVC_PROPERTY_ADDRESSBOOK_MODE
};

API const _contacts_person_number_property_ids _contacts_person_number = {
	._uri					= CTSVC_VIEW_URI_READ_ONLY_PERSON_NUMBER,
	.person_id				= CTSVC_PROPERTY_PERSON_ID,
	.display_name			= CTSVC_PROPERTY_PERSON_DISPLAY_NAME,
	.display_name_index		= CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,
	.display_contact_id		= CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,
	.ringtone_path			= CTSVC_PROPERTY_PERSON_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,
	.vibration				= CTSVC_PROPERTY_PERSON_VIBRATION,
	.message_alert		= CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,
	.is_favorite			= CTSVC_PROPERTY_PERSON_IS_FAVORITE,
	.has_phonenumber		= CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,
	.has_email				= CTSVC_PROPERTY_PERSON_HAS_EMAIL,
	.number_id			= CTSVC_PROPERTY_NUMBER_ID,
	.type				= CTSVC_PROPERTY_NUMBER_TYPE,
	.label				= CTSVC_PROPERTY_NUMBER_LABEL,
	.is_primary_default	= CTSVC_PROPERTY_DATA_IS_PRIMARY_DEFAULT,
	.number				= CTSVC_PROPERTY_NUMBER_NUMBER,
	.number_filter		= CTSVC_PROPERTY_NUMBER_NUMBER_FILTER,
	.normalized_number = CTSVC_PROPERTY_NUMBER_NORMALIZED_NUMBER,
};

API const _contacts_person_email_property_ids _contacts_person_email = {
	._uri					= CTSVC_VIEW_URI_READ_ONLY_PERSON_EMAIL,
	.person_id				= CTSVC_PROPERTY_PERSON_ID,
	.display_name			= CTSVC_PROPERTY_PERSON_DISPLAY_NAME,
	.display_name_index		= CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,
	.display_contact_id		= CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,
	.ringtone_path			= CTSVC_PROPERTY_PERSON_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,
	.vibration				= CTSVC_PROPERTY_PERSON_VIBRATION,
	.message_alert		= CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,
	.is_favorite			= CTSVC_PROPERTY_PERSON_IS_FAVORITE,
	.has_phonenumber		= CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,
	.has_email				= CTSVC_PROPERTY_PERSON_HAS_EMAIL,
	.email_id			= CTSVC_PROPERTY_EMAIL_ID,
	.type				= CTSVC_PROPERTY_EMAIL_TYPE,
	.label				= CTSVC_PROPERTY_EMAIL_LABEL,
	.is_primary_default	= CTSVC_PROPERTY_DATA_IS_PRIMARY_DEFAULT,
	.email				= CTSVC_PROPERTY_EMAIL_EMAIL
};

API const _contacts_person_address_property_ids _contacts_person_address = {
	._uri			= CTSVC_VIEW_URI_READ_ONLY_PERSON_ADDRESS,
	.person_id		= CTSVC_PROPERTY_PERSON_ID,
	.is_default		= CTSVC_PROPERTY_PERSON_ADDRESS_IS_DEFAULT,
	.street			= CTSVC_PROPERTY_PERSON_ADDRESS_STREET,
	.type			= CTSVC_PROPERTY_PERSON_ADDRESS_TYPE,
	.label                  = CTSVC_PROPERTY_PERSON_ADDRESS_LABEL,
};

API const _contacts_person_company_property_ids _contacts_person_company = {
	._uri			= CTSVC_VIEW_URI_READ_ONLY_PERSON_COMPANY,
	.person_id		= CTSVC_PROPERTY_PERSON_ID,
	.is_default		= CTSVC_PROPERTY_PERSON_COMPANY_IS_DEFAULT,
	.name			= CTSVC_PROPERTY_PERSON_COMPANY_NAME,
	.department		= CTSVC_PROPERTY_PERSON_COMPANY_DEPARTMENT,
	.job_title              = CTSVC_PROPERTY_PERSON_COMPANY_JOB_TITLE,
};

API const _contacts_person_url_property_ids _contacts_person_url = {
	._uri			= CTSVC_VIEW_URI_READ_ONLY_PERSON_URL,
	.person_id		= CTSVC_PROPERTY_PERSON_ID,
	.url		        = CTSVC_PROPERTY_PERSON_URL_URL,
	.label			= CTSVC_PROPERTY_PERSON_URL_LABEL,
	.type			= CTSVC_PROPERTY_PERSON_URL_TYPE,
};

API const _contacts_person_nickname_property_ids _contacts_person_nickname = {
	._uri			= CTSVC_VIEW_URI_READ_ONLY_PERSON_NICKNAME,
	.person_id		= CTSVC_PROPERTY_PERSON_ID,
	.name		        = CTSVC_PROPERTY_PERSON_NICKNAME_NAME,
};

API const _contacts_person_messenger_property_ids _contacts_person_messenger = {
	._uri			= CTSVC_VIEW_URI_READ_ONLY_PERSON_MESSENGER,
	.person_id		= CTSVC_PROPERTY_PERSON_ID,
	.id		        = CTSVC_PROPERTY_PERSON_MESSENGER_ID,
	.im_id		        = CTSVC_PROPERTY_PERSON_MESSENGER_IM_ID,
	.label		        = CTSVC_PROPERTY_PERSON_MESSENGER_LABEL,
	.type		        = CTSVC_PROPERTY_PERSON_MESSENGER_TYPE,
};

API const _contacts_person_event_property_ids _contacts_person_event = {
	._uri			= CTSVC_VIEW_URI_READ_ONLY_PERSON_EVENT,
	.person_id		= CTSVC_PROPERTY_PERSON_ID,
	.id				= CTSVC_PROPERTY_PERSON_EVENT_ID,
	.type			= CTSVC_PROPERTY_PERSON_EVENT_TYPE,
	.date			= CTSVC_PROPERTY_PERSON_EVENT_DATE,
	.label		= CTSVC_PROPERTY_PERSON_EVENT_LABEL,
};

API const _contacts_person_unknown_property_ids _contacts_person_unknown = {
	._uri			= CTSVC_VIEW_URI_READ_ONLY_PERSON_UNKNOWN,
	.id		        = CTSVC_PROPERTY_PERSON_ID,
	.display_name	        = CTSVC_PROPERTY_PERSON_UNKNOWN_DISPLAY_NAME,
	.image_thumbnail_path	= CTSVC_PROPERTY_PERSON_UNKNOWN_IMAGE_THUMBNAIL,
	.display_contact_id	= CTSVC_PROPERTY_PERSON_UNKNOWN_DISPLAY_CONTACT_ID,
	.is_favorite = CTSVC_PROPERTY_PERSON_UNKNOWN_IS_FAVORITE,
	.link_count = CTSVC_PROPERTY_PERSON_UNKNOWN_LINK_COUNT,
};

API const _contacts_person_usage_property_ids _contacts_person_usage = {
	._uri					= CTSVC_VIEW_URI_READ_ONLY_PERSON_USAGE,
	.person_id				= CTSVC_PROPERTY_PERSON_ID,
	.display_name			= CTSVC_PROPERTY_PERSON_DISPLAY_NAME,
	.display_name_index		= CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,
	.display_contact_id		= CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,
	.ringtone_path			= CTSVC_PROPERTY_PERSON_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,
	.vibration				= CTSVC_PROPERTY_PERSON_VIBRATION,
	.message_alert		= CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,
	.is_favorite			= CTSVC_PROPERTY_PERSON_IS_FAVORITE,
	.has_phonenumber		= CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,
	.has_email				= CTSVC_PROPERTY_PERSON_HAS_EMAIL,
	.usage_type				= CTSVC_PROPERTY_PERSON_USAGE_TYPE,
	.times_used				= CTSVC_PROPERTY_PERSON_TIMES_USED
};

API const _contacts_person_grouprel_property_ids _contacts_person_grouprel = {
	._uri					= CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP,
	.person_id				= CTSVC_PROPERTY_PERSON_ID,
	.display_name			= CTSVC_PROPERTY_PERSON_DISPLAY_NAME,
	.display_name_index		= CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,
	.display_contact_id		= CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,
	.ringtone_path			= CTSVC_PROPERTY_PERSON_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,
	.vibration				= CTSVC_PROPERTY_PERSON_VIBRATION,
	.message_alert			= CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,
	.status					= CTSVC_PROPERTY_PERSON_STATUS,
	.is_favorite			= CTSVC_PROPERTY_PERSON_IS_FAVORITE,
	.has_phonenumber		= CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,
	.has_email				= CTSVC_PROPERTY_PERSON_HAS_EMAIL,
	.link_count				= CTSVC_PROPERTY_PERSON_LINK_COUNT,
	.addressbook_ids		= CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS,
	.address_book_id		= CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,
	.group_id		= CTSVC_PROPERTY_GROUP_RELATION_GROUP_ID,
	.address_book_name		= CTSVC_PROPERTY_ADDRESSBOOK_NAME,
	.address_book_mode		= CTSVC_PROPERTY_ADDRESSBOOK_MODE,
	.contact_id				= CTSVC_PROPERTY_GROUP_RELATION_CONTACT_ID
};

API const _contacts_person_group_not_assigned_property_ids _contacts_person_group_not_assigned = {
	._uri						= CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_NOT_ASSIGNED,
	.person_id				= CTSVC_PROPERTY_PERSON_ID,
	.display_name			= CTSVC_PROPERTY_PERSON_DISPLAY_NAME,
	.display_name_index		= CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,
	.display_contact_id		= CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,
	.ringtone_path				= CTSVC_PROPERTY_PERSON_RINGTONE,
	.image_thumbnail_path		= CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,
	.vibration					= CTSVC_PROPERTY_PERSON_VIBRATION,
	.message_alert			= CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,
	.status					= CTSVC_PROPERTY_PERSON_STATUS,
	.is_favorite				= CTSVC_PROPERTY_PERSON_IS_FAVORITE,
	.has_phonenumber			= CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,
	.has_email				= CTSVC_PROPERTY_PERSON_HAS_EMAIL,
	.link_count				= CTSVC_PROPERTY_PERSON_LINK_COUNT,
	.linked_address_book_ids	= CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS,
	.contact_id				= CTSVC_PROPERTY_CONTACT_ID,
	.address_book_id			= CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,
	.address_book_mode		= CTSVC_PROPERTY_ADDRESSBOOK_MODE
};

API const _contacts_person_group_assigned_property_ids _contacts_person_group_assigned = {
	._uri					= CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_ASSIGNED,
	.person_id				= CTSVC_PROPERTY_PERSON_ID,
	.display_name			= CTSVC_PROPERTY_PERSON_DISPLAY_NAME,
	.display_name_index		= CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,
	.display_contact_id		= CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,
	.ringtone_path			= CTSVC_PROPERTY_PERSON_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,
	.vibration				= CTSVC_PROPERTY_PERSON_VIBRATION,
	.message_alert			= CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,
	.status					= CTSVC_PROPERTY_PERSON_STATUS,
	.is_favorite			= CTSVC_PROPERTY_PERSON_IS_FAVORITE,
	.has_phonenumber		= CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,
	.has_email				= CTSVC_PROPERTY_PERSON_HAS_EMAIL,
	.link_count				= CTSVC_PROPERTY_PERSON_LINK_COUNT,
	.linked_address_book_ids		= CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS,
	.address_book_id		= CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,
	.group_id				= CTSVC_PROPERTY_GROUP_RELATION_GROUP_ID,
	.address_book_mode		= CTSVC_PROPERTY_ADDRESSBOOK_MODE,
	.contact_id				= CTSVC_PROPERTY_GROUP_RELATION_CONTACT_ID
};

API const _contacts_person_phone_log_property_ids _contacts_person_phone_log = {
	._uri			= CTSVC_VIEW_URI_READ_ONLY_PERSON_PHONELOG,
	.person_id				= CTSVC_PROPERTY_PERSON_ID,
	.display_name			= CTSVC_PROPERTY_PERSON_DISPLAY_NAME,
	.image_thumbnail_path	= CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,
	.log_id	= CTSVC_PROPERTY_PHONELOG_ID,
	.address		= CTSVC_PROPERTY_PHONELOG_ADDRESS,
	.address_type	= CTSVC_PROPERTY_DATA_DATA1,
	.log_time		= CTSVC_PROPERTY_PHONELOG_LOG_TIME,
	.log_type		= CTSVC_PROPERTY_PHONELOG_LOG_TYPE,
	.extra_data1	= CTSVC_PROPERTY_PHONELOG_EXTRA_DATA1,
	.extra_data2	= CTSVC_PROPERTY_PHONELOG_EXTRA_DATA2,
	.normalized_address = CTSVC_PROPERTY_PHONELOG_NORMALIZED_ADDRESS,
};

API const _contacts_contact_number_property_ids _contacts_contact_number = {
	._uri					= CTSVC_VIEW_URI_READ_ONLY_CONTACT_NUMBER,
	.contact_id				= CTSVC_PROPERTY_CONTACT_ID,
	.display_name			= CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,
	.display_source_type	= CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,
	.address_book_id		= CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,
	.person_id				= CTSVC_PROPERTY_CONTACT_PERSON_ID,
	.ringtone_path			= CTSVC_PROPERTY_CONTACT_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,
	.number_id			= CTSVC_PROPERTY_NUMBER_ID,
	.type				= CTSVC_PROPERTY_NUMBER_TYPE,
	.label				= CTSVC_PROPERTY_NUMBER_LABEL,
	.is_default			= CTSVC_PROPERTY_NUMBER_IS_DEFAULT,
	.number				= CTSVC_PROPERTY_NUMBER_NUMBER,
	.number_filter		= CTSVC_PROPERTY_NUMBER_NUMBER_FILTER,
	.normalized_number = CTSVC_PROPERTY_NUMBER_NORMALIZED_NUMBER,
};

API const _contacts_contact_email_property_ids _contacts_contact_email = {
	._uri					= CTSVC_VIEW_URI_READ_ONLY_CONTACT_EMAIL,
	.contact_id				= CTSVC_PROPERTY_CONTACT_ID,
	.display_name			= CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,
	.display_source_type	= CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,
	.address_book_id		= CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,
	.person_id				= CTSVC_PROPERTY_CONTACT_PERSON_ID,
	.ringtone_path			= CTSVC_PROPERTY_CONTACT_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,
	.email_id			= CTSVC_PROPERTY_EMAIL_ID,
	.type				= CTSVC_PROPERTY_EMAIL_TYPE,
	.label				= CTSVC_PROPERTY_EMAIL_LABEL,
	.is_default			= CTSVC_PROPERTY_EMAIL_IS_DEFAULT,
	.email				= CTSVC_PROPERTY_EMAIL_EMAIL
};

API const _contacts_contact_grouprel_property_ids _contacts_contact_grouprel = {
	._uri					= CTSVC_VIEW_URI_READ_ONLY_CONTACT_GROUP,
	.contact_id				= CTSVC_PROPERTY_CONTACT_ID,
	.display_name			= CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,
	.display_source_type	= CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,
	.address_book_id		= CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,
	.person_id				= CTSVC_PROPERTY_CONTACT_PERSON_ID,
	.ringtone_path			= CTSVC_PROPERTY_CONTACT_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,
	.group_id		= CTSVC_PROPERTY_GROUP_RELATION_GROUP_ID,
	.group_name		= CTSVC_PROPERTY_GROUP_RELATION_GROUP_NAME
};

API const _contacts_contact_activity_property_ids _contacts_contact_activity = {
	._uri			= CTSVC_VIEW_URI_READ_ONLY_CONTACT_ACTIVITY,
	.contact_id				= CTSVC_PROPERTY_CONTACT_ID,
	.display_name			= CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,
	.display_source_type	= CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,
	.address_book_id		= CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,
	.person_id				= CTSVC_PROPERTY_CONTACT_PERSON_ID,
	.ringtone_path			= CTSVC_PROPERTY_CONTACT_RINGTONE,
	.image_thumbnail_path	= CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,
	.activity_id	= CTSVC_PROPERTY_ACTIVITY_ID,
	.source_name	= CTSVC_PROPERTY_ACTIVITY_SOURCE_NAME,
	.status			= CTSVC_PROPERTY_ACTIVITY_STATUS,
	.timestamp		= CTSVC_PROPERTY_ACTIVITY_TIMESTAMP,
	.service_operation	= CTSVC_PROPERTY_ACTIVITY_SERVICE_OPERATION,
	.uri				= CTSVC_PROPERTY_ACTIVITY_URI,
	.account_id		= CTSVC_PROPERTY_ADDRESSBOOK_ACCOUNT_ID,
};

API const _contacts_phone_log_number_property_ids _contacts_phone_log_number = {
	._uri	= CTSVC_VIEW_URI_READ_ONLY_PHONELOG_NUMBER,
	.number	= CTSVC_PROPERTY_PHONELOG_ADDRESS,
};

API const _contacts_phone_log_stat_property_ids _contacts_phone_log_stat = {
	._uri		= CTSVC_VIEW_URI_READ_ONLY_PHONELOG_STAT,
	.log_count	= CTSVC_PROPERTY_PHONELOG_STAT_LOG_COUNT,
	.log_type	= CTSVC_PROPERTY_PHONELOG_STAT_LOG_TYPE,
};

const property_info_s __property_addressbook[] = {
	{CTSVC_PROPERTY_ADDRESSBOOK_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	{CTSVC_PROPERTY_ADDRESSBOOK_ACCOUNT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"account_id"},
	{CTSVC_PROPERTY_ADDRESSBOOK_NAME,		CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_name"},
	{CTSVC_PROPERTY_ADDRESSBOOK_MODE,		CTSVC_SEARCH_PROPERTY_ALL,	"mode"},
};

const property_info_s __property_sdn[] = {		// _contacts_sdn
	{CTSVC_PROPERTY_SDN_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_SDN_NAME,		CTSVC_SEARCH_PROPERTY_ALL,	"name"},
	{CTSVC_PROPERTY_SDN_NUMBER,	CTSVC_SEARCH_PROPERTY_ALL,	"number"},
};

const property_info_s __property_group[] = {
	{CTSVC_PROPERTY_GROUP_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"group_id"},
	{CTSVC_PROPERTY_GROUP_ADDRESSBOOK_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	{CTSVC_PROPERTY_GROUP_NAME,			CTSVC_SEARCH_PROPERTY_ALL,	"group_name"},
	{CTSVC_PROPERTY_GROUP_RINGTONE,		CTSVC_SEARCH_PROPERTY_ALL,	"ringtone_path"},
	{CTSVC_PROPERTY_GROUP_IMAGE,			CTSVC_SEARCH_PROPERTY_ALL,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_GROUP_VIBRATION,		CTSVC_SEARCH_PROPERTY_ALL,	"vibration"},
	{CTSVC_PROPERTY_GROUP_EXTRA_DATA,		CTSVC_SEARCH_PROPERTY_ALL,	"extra_data"},
	{CTSVC_PROPERTY_GROUP_IS_READ_ONLY,	CTSVC_SEARCH_PROPERTY_ALL,	"is_read_only"},
	{CTSVC_PROPERTY_GROUP_MESSAGE_ALERT,	CTSVC_SEARCH_PROPERTY_ALL,	"message_alert"},
};

const property_info_s __property_person[] = {
	{CTSVC_PROPERTY_PERSON_ID,					CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME,		CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,	CTSVC_SEARCH_PROPERTY_PROJECTION,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,CTSVC_SEARCH_PROPERTY_ALL,	"name_contact_id"},
	{CTSVC_PROPERTY_PERSON_RINGTONE,			CTSVC_SEARCH_PROPERTY_ALL,	"ringtone_path"},
	{CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL, CTSVC_SEARCH_PROPERTY_ALL,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_PERSON_VIBRATION,		CTSVC_SEARCH_PROPERTY_ALL,	"vibration"},
	{CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,	CTSVC_SEARCH_PROPERTY_ALL,	"message_alert"},
	{CTSVC_PROPERTY_PERSON_STATUS,			CTSVC_SEARCH_PROPERTY_ALL,	"status"},
	{CTSVC_PROPERTY_PERSON_IS_FAVORITE,		CTSVC_SEARCH_PROPERTY_ALL,	"is_favorite"},
	{CTSVC_PROPERTY_PERSON_FAVORITE_PRIORITY,	CTSVC_SEARCH_PROPERTY_FILTER,	"favorite_prio"},
	{CTSVC_PROPERTY_PERSON_LINK_COUNT,		CTSVC_SEARCH_PROPERTY_ALL,	"link_count"},
	{CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS,	CTSVC_SEARCH_PROPERTY_PROJECTION,	"addressbook_ids"},
	{CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,	CTSVC_SEARCH_PROPERTY_ALL,	"has_phonenumber"},
	{CTSVC_PROPERTY_PERSON_HAS_EMAIL,		CTSVC_SEARCH_PROPERTY_ALL,	"has_email"},
};

const property_info_s __property_simple_contact[] = {
	{CTSVC_PROPERTY_CONTACT_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,	CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"display_name_source"},
	{CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	{CTSVC_PROPERTY_CONTACT_RINGTONE,		CTSVC_SEARCH_PROPERTY_ALL,	"ringtone_path"},
	{CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,CTSVC_SEARCH_PROPERTY_ALL,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_CONTACT_IS_FAVORITE,		CTSVC_SEARCH_PROPERTY_ALL,	"is_favorite"},
	{CTSVC_PROPERTY_CONTACT_HAS_PHONENUMBER, CTSVC_SEARCH_PROPERTY_ALL,	"has_phonenumber"},
	{CTSVC_PROPERTY_CONTACT_HAS_EMAIL,		CTSVC_SEARCH_PROPERTY_ALL,	"has_email"},
	{CTSVC_PROPERTY_CONTACT_PERSON_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_CONTACT_UID,				CTSVC_SEARCH_PROPERTY_ALL,	"uid"},
	{CTSVC_PROPERTY_CONTACT_VIBRATION,		CTSVC_SEARCH_PROPERTY_ALL,	"vibration"},
	{CTSVC_PROPERTY_CONTACT_MESSAGE_ALERT,		CTSVC_SEARCH_PROPERTY_ALL,	"message_alert"},
	{CTSVC_PROPERTY_CONTACT_CHANGED_TIME,	CTSVC_SEARCH_PROPERTY_ALL,	"changed_time"},
};

const property_info_s __property_name[] = {
	{CTSVC_PROPERTY_NAME_ID,					CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_NAME_CONTACT_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_NAME_FIRST,				CTSVC_SEARCH_PROPERTY_ALL,	"data2"},
	{CTSVC_PROPERTY_NAME_LAST,					CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
	{CTSVC_PROPERTY_NAME_ADDITION,			CTSVC_SEARCH_PROPERTY_ALL,	"data4"},
	{CTSVC_PROPERTY_NAME_PREFIX,				CTSVC_SEARCH_PROPERTY_ALL,	"data5"},
	{CTSVC_PROPERTY_NAME_SUFFIX,				CTSVC_SEARCH_PROPERTY_ALL,	"data6"},
	{CTSVC_PROPERTY_NAME_PHONETIC_FIRST,	CTSVC_SEARCH_PROPERTY_ALL,	"data7"},
	{CTSVC_PROPERTY_NAME_PHONETIC_MIDDLE,	CTSVC_SEARCH_PROPERTY_ALL,	"data8"},
	{CTSVC_PROPERTY_NAME_PHONETIC_LAST,		CTSVC_SEARCH_PROPERTY_ALL,	"data9"},
};

const property_info_s __property_number[] = {		//_contacts_number
	{CTSVC_PROPERTY_NUMBER_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_NUMBER_CONTACT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_NUMBER_TYPE,			CTSVC_SEARCH_PROPERTY_ALL,	"data1"},
	{CTSVC_PROPERTY_NUMBER_LABEL,			CTSVC_SEARCH_PROPERTY_ALL,	"data2"},
	{CTSVC_PROPERTY_NUMBER_IS_DEFAULT,	CTSVC_SEARCH_PROPERTY_ALL,	"is_default"},
	{CTSVC_PROPERTY_NUMBER_NUMBER,		CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
	{CTSVC_PROPERTY_NUMBER_NORMALIZED_NUMBER,	CTSVC_SEARCH_PROPERTY_FILTER,	"data5"},
};

const property_info_s __property_email[] = {
	{CTSVC_PROPERTY_EMAIL_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_EMAIL_CONTACT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_EMAIL_TYPE,			CTSVC_SEARCH_PROPERTY_ALL,	"data1"},
	{CTSVC_PROPERTY_EMAIL_LABEL,			CTSVC_SEARCH_PROPERTY_ALL,	"data2"},
	{CTSVC_PROPERTY_EMAIL_IS_DEFAULT,	CTSVC_SEARCH_PROPERTY_ALL,	"is_default"},
	{CTSVC_PROPERTY_EMAIL_EMAIL,			CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
};

const property_info_s __property_address[] = {
	{CTSVC_PROPERTY_ADDRESS_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_ADDRESS_CONTACT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_ADDRESS_TYPE,			CTSVC_SEARCH_PROPERTY_ALL,	"data1"},
	{CTSVC_PROPERTY_ADDRESS_LABEL,		CTSVC_SEARCH_PROPERTY_ALL,	"data2"},
	{CTSVC_PROPERTY_ADDRESS_POSTBOX,		CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
	{CTSVC_PROPERTY_ADDRESS_POSTAL_CODE,CTSVC_SEARCH_PROPERTY_ALL,	"data4"},
	{CTSVC_PROPERTY_ADDRESS_REGION,		CTSVC_SEARCH_PROPERTY_ALL,	"data5"},
	{CTSVC_PROPERTY_ADDRESS_LOCALITY,	CTSVC_SEARCH_PROPERTY_ALL,	"data6"},
	{CTSVC_PROPERTY_ADDRESS_STREET,		CTSVC_SEARCH_PROPERTY_ALL,	"data7"},
	{CTSVC_PROPERTY_ADDRESS_COUNTRY,		CTSVC_SEARCH_PROPERTY_ALL,	"data9"},
	{CTSVC_PROPERTY_ADDRESS_EXTENDED,	CTSVC_SEARCH_PROPERTY_ALL,	"data8"},
	{CTSVC_PROPERTY_ADDRESS_IS_DEFAULT,	CTSVC_SEARCH_PROPERTY_ALL,	"is_default"},
};

const property_info_s __property_person_address[] = {
	{CTSVC_PROPERTY_PERSON_ID, CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_PERSON_ADDRESS_IS_DEFAULT, CTSVC_SEARCH_PROPERTY_ALL,	"is_default"},
	{CTSVC_PROPERTY_PERSON_ADDRESS_STREET, CTSVC_SEARCH_PROPERTY_ALL, "street"},
	{CTSVC_PROPERTY_PERSON_ADDRESS_TYPE, CTSVC_SEARCH_PROPERTY_ALL,	"type"},
	{CTSVC_PROPERTY_PERSON_ADDRESS_LABEL, CTSVC_SEARCH_PROPERTY_ALL, "label"},
};

const property_info_s __property_person_company[] = {
	{CTSVC_PROPERTY_PERSON_ID, CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_PERSON_COMPANY_IS_DEFAULT, CTSVC_SEARCH_PROPERTY_ALL,	"is_default"},
	{CTSVC_PROPERTY_PERSON_COMPANY_NAME, CTSVC_SEARCH_PROPERTY_ALL,	"company_name"},
	{CTSVC_PROPERTY_PERSON_COMPANY_DEPARTMENT, CTSVC_SEARCH_PROPERTY_ALL, "department"},
	{CTSVC_PROPERTY_PERSON_COMPANY_JOB_TITLE, CTSVC_SEARCH_PROPERTY_ALL, "job_title"},
};

const property_info_s __property_person_url[] = {
	{CTSVC_PROPERTY_PERSON_ID, CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_PERSON_URL_URL, CTSVC_SEARCH_PROPERTY_ALL, "url"},
	{CTSVC_PROPERTY_PERSON_URL_LABEL, CTSVC_SEARCH_PROPERTY_ALL, "label"},
	{CTSVC_PROPERTY_PERSON_URL_TYPE, CTSVC_SEARCH_PROPERTY_ALL, "type"},
};

const property_info_s __property_person_nickname[] = {
	{CTSVC_PROPERTY_PERSON_ID, CTSVC_SEARCH_PROPERTY_ALL, "person_id"},
	{CTSVC_PROPERTY_PERSON_NICKNAME_NAME, CTSVC_SEARCH_PROPERTY_ALL, "name"},
};

const property_info_s __property_person_messenger[] = {
	{CTSVC_PROPERTY_PERSON_ID, CTSVC_SEARCH_PROPERTY_ALL, "person_id"},
	{CTSVC_PROPERTY_PERSON_MESSENGER_ID, CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_PERSON_MESSENGER_IM_ID, CTSVC_SEARCH_PROPERTY_ALL,"im_id"},
	{CTSVC_PROPERTY_PERSON_MESSENGER_LABEL, CTSVC_SEARCH_PROPERTY_ALL,"label"},
	{CTSVC_PROPERTY_PERSON_MESSENGER_TYPE, CTSVC_SEARCH_PROPERTY_ALL, "type"},
};

const property_info_s __property_person_event[] = {
	{CTSVC_PROPERTY_PERSON_ID, CTSVC_SEARCH_PROPERTY_ALL, "person_id"},
	{CTSVC_PROPERTY_PERSON_EVENT_ID, CTSVC_SEARCH_PROPERTY_ALL, "id"},
	{CTSVC_PROPERTY_PERSON_EVENT_TYPE, CTSVC_SEARCH_PROPERTY_ALL, "type"},
	{CTSVC_PROPERTY_PERSON_EVENT_DATE, CTSVC_SEARCH_PROPERTY_ALL, "date"},
	{CTSVC_PROPERTY_PERSON_EVENT_LABEL, CTSVC_SEARCH_PROPERTY_ALL, "label"},
};

const property_info_s __property_person_unknown[] = {
	{CTSVC_PROPERTY_PERSON_ID, CTSVC_SEARCH_PROPERTY_ALL, "person_id"},
	{CTSVC_PROPERTY_PERSON_UNKNOWN_DISPLAY_NAME, CTSVC_SEARCH_PROPERTY_ALL,	"display_name"},
	{CTSVC_PROPERTY_PERSON_UNKNOWN_IMAGE_THUMBNAIL, CTSVC_SEARCH_PROPERTY_ALL, "image_thumbnail_path"},
	{CTSVC_PROPERTY_PERSON_UNKNOWN_DISPLAY_CONTACT_ID, CTSVC_SEARCH_PROPERTY_ALL, "name_contact_id"},
	{CTSVC_PROPERTY_PERSON_UNKNOWN_IS_FAVORITE, CTSVC_SEARCH_PROPERTY_ALL, "is_favorite"},
	{CTSVC_PROPERTY_PERSON_UNKNOWN_LINK_COUNT, CTSVC_SEARCH_PROPERTY_ALL, "link_count"},
};

const property_info_s __property_url[] = {
	{CTSVC_PROPERTY_URL_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_URL_CONTACT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_URL_TYPE,			CTSVC_SEARCH_PROPERTY_ALL,	"data1"},
	{CTSVC_PROPERTY_URL_LABEL,			CTSVC_SEARCH_PROPERTY_ALL,	"data2"},
	{CTSVC_PROPERTY_URL_URL,			CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
};

const property_info_s __property_event[] = {
	{CTSVC_PROPERTY_EVENT_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_EVENT_CONTACT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_EVENT_TYPE,			CTSVC_SEARCH_PROPERTY_ALL,	"data1"},
	{CTSVC_PROPERTY_EVENT_LABEL,			CTSVC_SEARCH_PROPERTY_ALL,	"data2"},
	{CTSVC_PROPERTY_EVENT_DATE,			CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
	{CTSVC_PROPERTY_EVENT_IS_LUNAR,		CTSVC_SEARCH_PROPERTY_ALL,	"data4"},
	{CTSVC_PROPERTY_EVENT_LUNAR_DATE,	CTSVC_SEARCH_PROPERTY_ALL,	"data5"},
};

const property_info_s __property_group_relation[] = {
	{CTSVC_PROPERTY_GROUP_RELATION_GROUP_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"group_id"},
	{CTSVC_PROPERTY_GROUP_RELATION_CONTACT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_GROUP_RELATION_GROUP_NAME,CTSVC_SEARCH_PROPERTY_ALL,	"group_name"},
};

const property_info_s __property_relationship[] = {
	{CTSVC_PROPERTY_RELATIONSHIP_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_RELATIONSHIP_CONTACT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_RELATIONSHIP_TYPE,			CTSVC_SEARCH_PROPERTY_ALL,	"data1"},
	{CTSVC_PROPERTY_RELATIONSHIP_LABEL,			CTSVC_SEARCH_PROPERTY_ALL,	"data2"},
	{CTSVC_PROPERTY_RELATIONSHIP_NAME,			CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
};

const property_info_s __property_image[] = {
	{CTSVC_PROPERTY_IMAGE_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_IMAGE_CONTACT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_IMAGE_TYPE,			CTSVC_SEARCH_PROPERTY_ALL,	"data1"},
	{CTSVC_PROPERTY_IMAGE_LABEL,			CTSVC_SEARCH_PROPERTY_ALL,	"data2"},
	{CTSVC_PROPERTY_IMAGE_PATH,			CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
	{CTSVC_PROPERTY_IMAGE_IS_DEFAULT,	CTSVC_SEARCH_PROPERTY_ALL,	"is_default"},
};

const property_info_s __property_company[] = {
	{CTSVC_PROPERTY_COMPANY_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_COMPANY_CONTACT_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_COMPANY_TYPE,				CTSVC_SEARCH_PROPERTY_ALL,	"data1"},
	{CTSVC_PROPERTY_COMPANY_LABEL,			CTSVC_SEARCH_PROPERTY_ALL,	"data2"},
	{CTSVC_PROPERTY_COMPANY_NAME,				CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
	{CTSVC_PROPERTY_COMPANY_DEPARTMENT,		CTSVC_SEARCH_PROPERTY_ALL,	"data4"},
	{CTSVC_PROPERTY_COMPANY_JOB_TITLE,		CTSVC_SEARCH_PROPERTY_ALL,	"data5"},
	{CTSVC_PROPERTY_COMPANY_ROLE,			CTSVC_SEARCH_PROPERTY_ALL,	"data6"},
	{CTSVC_PROPERTY_COMPANY_ASSISTANT_NAME,	CTSVC_SEARCH_PROPERTY_ALL,	"data7"},
	{CTSVC_PROPERTY_COMPANY_LOGO,				CTSVC_SEARCH_PROPERTY_ALL,	"data8"},
	{CTSVC_PROPERTY_COMPANY_LOCATION,		CTSVC_SEARCH_PROPERTY_ALL,	"data9"},
	{CTSVC_PROPERTY_COMPANY_DESCRIPTION,	CTSVC_SEARCH_PROPERTY_ALL,	"data10"},
	{CTSVC_PROPERTY_COMPANY_PHONETIC_NAME,	CTSVC_SEARCH_PROPERTY_ALL,	"data11"},
};

const property_info_s __property_nickname[] = {
	{CTSVC_PROPERTY_NICKNAME_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"id",},
	{CTSVC_PROPERTY_NICKNAME_CONTACT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_NICKNAME_NAME,			CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
};

const property_info_s __property_messenger[] = {
	{CTSVC_PROPERTY_MESSENGER_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_MESSENGER_CONTACT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_MESSENGER_TYPE,			CTSVC_SEARCH_PROPERTY_ALL,	"data1"},
	{CTSVC_PROPERTY_MESSENGER_LABEL,			CTSVC_SEARCH_PROPERTY_ALL,	"data2"},
	{CTSVC_PROPERTY_MESSENGER_IM_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
};

const property_info_s __property_note[] = {
	{CTSVC_PROPERTY_NOTE_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_NOTE_CONTACT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_NOTE_NOTE,			CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
};

const property_info_s __property_profile[] = {
	{CTSVC_PROPERTY_PROFILE_ID,					CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_PROFILE_CONTACT_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_PROFILE_UID,					CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
	{CTSVC_PROPERTY_PROFILE_TEXT,					CTSVC_SEARCH_PROPERTY_ALL,	"data4"},
	{CTSVC_PROPERTY_PROFILE_ORDER,				CTSVC_SEARCH_PROPERTY_ALL,	"data5"},
	{CTSVC_PROPERTY_PROFILE_SERVICE_OPERATION,	CTSVC_SEARCH_PROPERTY_ALL,	"data6"},
	{CTSVC_PROPERTY_PROFILE_MIME,				CTSVC_SEARCH_PROPERTY_ALL,	"data7"},
	{CTSVC_PROPERTY_PROFILE_APP_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"data8"},
	{CTSVC_PROPERTY_PROFILE_URI,					CTSVC_SEARCH_PROPERTY_ALL,	"data9"},
	{CTSVC_PROPERTY_PROFILE_CATEGORY,			CTSVC_SEARCH_PROPERTY_ALL,	"data10"},
	{CTSVC_PROPERTY_PROFILE_EXTRA_DATA,		CTSVC_SEARCH_PROPERTY_ALL,	"data11"},
};

const property_info_s __property_activity_photo[] = {
	{CTSVC_PROPERTY_ACTIVITY_PHOTO_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_ACTIVITY_PHOTO_ACTIVITY_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"activity_id"},
	{CTSVC_PROPERTY_ACTIVITY_PHOTO_URL,			CTSVC_SEARCH_PROPERTY_ALL,	"photo_url"},
	{CTSVC_PROPERTY_ACTIVITY_PHOTO_SORT_INDEX,	CTSVC_SEARCH_PROPERTY_ALL,	"sort_index"},
};

const property_info_s __property_activity[] = {
	{CTSVC_PROPERTY_ACTIVITY_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_ACTIVITY_CONTACT_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_ACTIVITY_SOURCE_NAME,	CTSVC_SEARCH_PROPERTY_ALL,	"source_name"},
	{CTSVC_PROPERTY_ACTIVITY_STATUS,			CTSVC_SEARCH_PROPERTY_ALL,	"status"},
	{CTSVC_PROPERTY_ACTIVITY_TIMESTAMP,		CTSVC_SEARCH_PROPERTY_ALL,	"timestamp"},
	{CTSVC_PROPERTY_ACTIVITY_SERVICE_OPERATION, CTSVC_SEARCH_PROPERTY_ALL,	"service_operation"},
	{CTSVC_PROPERTY_ACTIVITY_URI,				CTSVC_SEARCH_PROPERTY_ALL,	"uri"},
	{CTSVC_PROPERTY_ACTIVITY_ACTIVITY_PHOTO,CTSVC_SEARCH_PROPERTY_NONE,	(void*)__property_activity_photo},
};

const property_info_s __property_extension[] = {
	{CTSVC_PROPERTY_EXTENSION_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_EXTENSION_CONTACT_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_EXTENSION_DATA1,		CTSVC_SEARCH_PROPERTY_ALL,	"data1"},
	{CTSVC_PROPERTY_EXTENSION_DATA2,		CTSVC_SEARCH_PROPERTY_ALL,	"data2"},
	{CTSVC_PROPERTY_EXTENSION_DATA3,		CTSVC_SEARCH_PROPERTY_ALL,	"data3"},
	{CTSVC_PROPERTY_EXTENSION_DATA4,		CTSVC_SEARCH_PROPERTY_ALL,	"data4"},
	{CTSVC_PROPERTY_EXTENSION_DATA5,		CTSVC_SEARCH_PROPERTY_ALL,	"data5"},
	{CTSVC_PROPERTY_EXTENSION_DATA6,		CTSVC_SEARCH_PROPERTY_ALL,	"data6"},
	{CTSVC_PROPERTY_EXTENSION_DATA7,		CTSVC_SEARCH_PROPERTY_ALL,	"data7"},
	{CTSVC_PROPERTY_EXTENSION_DATA8,		CTSVC_SEARCH_PROPERTY_ALL,	"data8"},
	{CTSVC_PROPERTY_EXTENSION_DATA9,		CTSVC_SEARCH_PROPERTY_ALL,	"data9"},
	{CTSVC_PROPERTY_EXTENSION_DATA10,		CTSVC_SEARCH_PROPERTY_ALL,	"data10"},
	{CTSVC_PROPERTY_EXTENSION_DATA11,		CTSVC_SEARCH_PROPERTY_ALL,	"data11"},
	{CTSVC_PROPERTY_EXTENSION_DATA12,		CTSVC_SEARCH_PROPERTY_ALL,	"data12"},
};

const property_info_s __property_contact[] = {
	{CTSVC_PROPERTY_CONTACT_ID,					CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,		CTSVC_SEARCH_PROPERTY_ALL,	NULL},	//dispaly_name, reverse_display_name
	{CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,	CTSVC_SEARCH_PROPERTY_ALL,	"display_name_source"},
	{CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	{CTSVC_PROPERTY_CONTACT_RINGTONE,			CTSVC_SEARCH_PROPERTY_ALL,	"ringtone_path"},
	{CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,	CTSVC_SEARCH_PROPERTY_ALL,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_CONTACT_IS_FAVORITE,			CTSVC_SEARCH_PROPERTY_ALL,	"is_favorite"},
	{CTSVC_PROPERTY_CONTACT_HAS_PHONENUMBER,	CTSVC_SEARCH_PROPERTY_ALL,	"has_phonenumber"},
	{CTSVC_PROPERTY_CONTACT_HAS_EMAIL,			CTSVC_SEARCH_PROPERTY_ALL,	"has_email"},
	{CTSVC_PROPERTY_CONTACT_PERSON_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_CONTACT_UID,					CTSVC_SEARCH_PROPERTY_ALL,	"uid"},
	{CTSVC_PROPERTY_CONTACT_VIBRATION,			CTSVC_SEARCH_PROPERTY_ALL,	"vibration"},
	{CTSVC_PROPERTY_CONTACT_MESSAGE_ALERT,		CTSVC_SEARCH_PROPERTY_ALL,	"message_alert"},
	{CTSVC_PROPERTY_CONTACT_CHANGED_TIME,		CTSVC_SEARCH_PROPERTY_ALL,	"changed_time"},
	{CTSVC_PROPERTY_CONTACT_IS_UNKNOWN,		CTSVC_SEARCH_PROPERTY_ALL,      "is_unknown"},
	{CTSVC_PROPERTY_CONTACT_LINK_MODE,			CTSVC_SEARCH_PROPERTY_ALL,	"link_mode"},
	{CTSVC_PROPERTY_CONTACT_NAME,					CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_name},
	{CTSVC_PROPERTY_CONTACT_COMPANY,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_company},
	{CTSVC_PROPERTY_CONTACT_NOTE,					CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_note},
	{CTSVC_PROPERTY_CONTACT_NUMBER,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_number},
	{CTSVC_PROPERTY_CONTACT_EMAIL,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_email},
	{CTSVC_PROPERTY_CONTACT_EVENT,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_event},
	{CTSVC_PROPERTY_CONTACT_MESSENGER,			CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_messenger},
	{CTSVC_PROPERTY_CONTACT_ADDRESS,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_address},
	{CTSVC_PROPERTY_CONTACT_URL,					CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_url},
	{CTSVC_PROPERTY_CONTACT_NICKNAME,			CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_nickname},
	{CTSVC_PROPERTY_CONTACT_PROFILE,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_profile},
	{CTSVC_PROPERTY_CONTACT_RELATIONSHIP,		CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_relationship},
	{CTSVC_PROPERTY_CONTACT_IMAGE,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_image},
	{CTSVC_PROPERTY_CONTACT_GROUP_RELATION,		CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_group_relation},
	{CTSVC_PROPERTY_CONTACT_EXTENSION,			CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_extension},
};

const property_info_s __property_my_profile[] = {
	{CTSVC_PROPERTY_MY_PROFILE_ID,					CTSVC_SEARCH_PROPERTY_ALL,	"my_profile_id"},
	{CTSVC_PROPERTY_MY_PROFILE_DISPLAY_NAME,		CTSVC_SEARCH_PROPERTY_ALL,	NULL},	//dispaly_name, reverse_display_name
	{CTSVC_PROPERTY_MY_PROFILE_ADDRESSBOOK_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	{CTSVC_PROPERTY_MY_PROFILE_IMAGE_THUMBNAIL,	CTSVC_SEARCH_PROPERTY_ALL,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_MY_PROFILE_UID,					CTSVC_SEARCH_PROPERTY_ALL,	"uid"},
	{CTSVC_PROPERTY_MY_PROFILE_CHANGED_TIME,		CTSVC_SEARCH_PROPERTY_ALL,	"changed_time"},
	{CTSVC_PROPERTY_MY_PROFILE_NAME,					CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_name},
	{CTSVC_PROPERTY_MY_PROFILE_COMPANY,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_company},
	{CTSVC_PROPERTY_MY_PROFILE_NOTE,					CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_note},
	{CTSVC_PROPERTY_MY_PROFILE_NUMBER,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_number},
	{CTSVC_PROPERTY_MY_PROFILE_EMAIL,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_email},
	{CTSVC_PROPERTY_MY_PROFILE_EVENT,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_event},
	{CTSVC_PROPERTY_MY_PROFILE_MESSENGER,			CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_messenger},
	{CTSVC_PROPERTY_MY_PROFILE_ADDRESS,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_address},
	{CTSVC_PROPERTY_MY_PROFILE_URL,					CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_url},
	{CTSVC_PROPERTY_MY_PROFILE_NICKNAME,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_nickname},
	{CTSVC_PROPERTY_MY_PROFILE_PROFILE,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_profile},
	{CTSVC_PROPERTY_MY_PROFILE_RELATIONSHIP,		CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_relationship},
	{CTSVC_PROPERTY_MY_PROFILE_IMAGE,				CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_image},
	{CTSVC_PROPERTY_MY_PROFILE_EXTENSION,			CTSVC_SEARCH_PROPERTY_NONE,(void*)__property_extension},
};

const property_info_s __property_speeddial[] = {		// _contacts_speeddial
	{CTSVC_PROPERTY_SPEEDDIAL_DIAL_NUMBER,		CTSVC_SEARCH_PROPERTY_ALL,	"speed_number"},
	{CTSVC_PROPERTY_SPEEDDIAL_NUMBER_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"number_id"},
	{CTSVC_PROPERTY_SPEEDDIAL_NUMBER,			CTSVC_SEARCH_PROPERTY_ALL,	"number"},
	{CTSVC_PROPERTY_SPEEDDIAL_NUMBER_LABEL,	CTSVC_SEARCH_PROPERTY_ALL,	"label"},
	{CTSVC_PROPERTY_SPEEDDIAL_NUMBER_TYPE,		CTSVC_SEARCH_PROPERTY_ALL,	"type"},
	{CTSVC_PROPERTY_SPEEDDIAL_PERSON_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_SPEEDDIAL_DISPLAY_NAME,	CTSVC_SEARCH_PROPERTY_ALL,	NULL},		// display_name or reverse_display_name
	{CTSVC_PROPERTY_SPEEDDIAL_IMAGE_THUMBNAIL,	CTSVC_SEARCH_PROPERTY_ALL,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_SPEEDDIAL_NORMALIZED_NUMBER,	CTSVC_SEARCH_PROPERTY_FILTER,	"normalized_number"},
};

const property_info_s __property_phonelog[] = {		// _contacts_phone_log
	{CTSVC_PROPERTY_PHONELOG_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_PHONELOG_PERSON_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_PHONELOG_ADDRESS,		CTSVC_SEARCH_PROPERTY_ALL,	"number"},
	{CTSVC_PROPERTY_PHONELOG_LOG_TIME,		CTSVC_SEARCH_PROPERTY_ALL,	"log_time"},
	{CTSVC_PROPERTY_PHONELOG_LOG_TYPE,		CTSVC_SEARCH_PROPERTY_ALL,	"log_type"},
	{CTSVC_PROPERTY_PHONELOG_EXTRA_DATA1,	CTSVC_SEARCH_PROPERTY_ALL,	"data1"},		// duration
	{CTSVC_PROPERTY_PHONELOG_EXTRA_DATA2,	CTSVC_SEARCH_PROPERTY_ALL,	"data2"},		// short message, email subject
	{CTSVC_PROPERTY_PHONELOG_NORMALIZED_ADDRESS,	CTSVC_SEARCH_PROPERTY_FILTER,	"normal_num"},
};

#if 0
const property_info_s __property_updated_info[] = {
	{CTSVC_PROPERTY_UPDATE_INFO_ID, 				CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_UPDATE_INFO_ADDRESSBOOK_ID, CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	{CTSVC_PROPERTY_UPDATE_INFO_TYPE,			CTSVC_SEARCH_PROPERTY_ALL,	"type"},
	{CTSVC_PROPERTY_UPDATE_INFO_VERSION,		CTSVC_SEARCH_PROPERTY_ALL,	"version"},
	{CTSVC_PROPERTY_UPDATE_INFO_IMAGE_CHANGED,	CTSVC_SEARCH_PROPERTY_ALL,	"image_changed"},
	{CTSVC_PROPERTY_UPDATE_INFO_LAST_CHANGED_TYPE, 	CTSVC_SEARCH_PROPERTY_ALL,	"is_deleted"},
};

const property_info_s __property_grouprel_updated_info[] = {
	{CTSVC_PROPERTY_GROUP_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"group_id"},
	{CTSVC_PROPERTY_CONTACT_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_ADDRESSBOOK_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	{CTSVC_PROPERTY_UPDATE_INFO_TYPE,	CTSVC_SEARCH_PROPERTY_ALL,	"type"},
	{CTSVC_PROPERTY_UPDATE_INFO_VERSION,	CTSVC_SEARCH_PROPERTY_ALL,	"version"},
};
#endif

// search properties ///////////////////////////////////////////////////////////////////////////////////////////////////
const property_info_s __property_person_contact[] = {		// _contacts_person_contact
	{CTSVC_PROPERTY_PERSON_ID,						CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME,			CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,		CTSVC_SEARCH_PROPERTY_PROJECTION,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"name_contact_id"},
	{CTSVC_PROPERTY_PERSON_RINGTONE,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"ringtone_path"},
	{CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_PERSON_VIBRATION,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"vibration"},
	{CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"message_alert"},
	{CTSVC_PROPERTY_PERSON_STATUS,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"status"},
	{CTSVC_PROPERTY_PERSON_IS_FAVORITE,			CTSVC_SEARCH_PROPERTY_ALL,	"is_favorite"},
	{CTSVC_PROPERTY_PERSON_LINK_COUNT,				CTSVC_SEARCH_PROPERTY_ALL,	"link_count"},
	{CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"addressbook_ids"},
	{CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,			CTSVC_SEARCH_PROPERTY_ALL,	"has_phonenumber"},
	{CTSVC_PROPERTY_PERSON_HAS_EMAIL,				CTSVC_SEARCH_PROPERTY_ALL,	"has_email"},
	// contact
	{CTSVC_PROPERTY_CONTACT_ID,						CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	// addressbook
	{CTSVC_PROPERTY_ADDRESSBOOK_NAME,				CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_name"},
	{CTSVC_PROPERTY_ADDRESSBOOK_MODE,				CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_mode"},
};

const property_info_s __property_person_contact_include_unknown[] = {
	// _contacts_person_contact_include_unknown
	{CTSVC_PROPERTY_PERSON_ID, CTSVC_SEARCH_PROPERTY_ALL,"person_id"},
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME, CTSVC_SEARCH_PROPERTY_ALL, NULL},
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX, CTSVC_SEARCH_PROPERTY_PROJECTION, NULL},
	{CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID, CTSVC_SEARCH_PROPERTY_PROJECTION, "name_contact_id"},
	{CTSVC_PROPERTY_PERSON_RINGTONE, CTSVC_SEARCH_PROPERTY_PROJECTION, "ringtone_path"},
	{CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL, CTSVC_SEARCH_PROPERTY_PROJECTION, "image_thumbnail_path"},
	{CTSVC_PROPERTY_PERSON_VIBRATION, CTSVC_SEARCH_PROPERTY_PROJECTION, "vibration"},
	{CTSVC_PROPERTY_PERSON_MESSAGE_ALERT, CTSVC_SEARCH_PROPERTY_PROJECTION,	"message_alert"},
	{CTSVC_PROPERTY_PERSON_STATUS, CTSVC_SEARCH_PROPERTY_PROJECTION, "status"},
	{CTSVC_PROPERTY_PERSON_IS_FAVORITE, CTSVC_SEARCH_PROPERTY_ALL, "is_favorite"},
	{CTSVC_PROPERTY_PERSON_LINK_COUNT, CTSVC_SEARCH_PROPERTY_ALL, "link_count"},
	{CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS, CTSVC_SEARCH_PROPERTY_PROJECTION, "addressbook_ids"},
	{CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,	CTSVC_SEARCH_PROPERTY_ALL, "has_phonenumber"},
	{CTSVC_PROPERTY_PERSON_HAS_EMAIL, CTSVC_SEARCH_PROPERTY_ALL, "has_email"},
	// contact
	{CTSVC_PROPERTY_CONTACT_ID, CTSVC_SEARCH_PROPERTY_ALL, "contact_id"},
	{CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID, CTSVC_SEARCH_PROPERTY_ALL, "addressbook_id"},
	// addressbook
	{CTSVC_PROPERTY_ADDRESSBOOK_NAME, CTSVC_SEARCH_PROPERTY_ALL, "addressbook_name"},
	{CTSVC_PROPERTY_ADDRESSBOOK_MODE, CTSVC_SEARCH_PROPERTY_ALL,"addressbook_mode"},
};

const property_info_s __property_person_number[] = {		// _contacts_person_number
	{CTSVC_PROPERTY_PERSON_ID,						CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME,			CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,		CTSVC_SEARCH_PROPERTY_PROJECTION,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"name_contact_id"},
	{CTSVC_PROPERTY_PERSON_RINGTONE,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"ringtone_path"},
	{CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_PERSON_VIBRATION,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"vibration"},
	{CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"message_alert"},
	{CTSVC_PROPERTY_PERSON_IS_FAVORITE,			CTSVC_SEARCH_PROPERTY_ALL,	"is_favorite"},
	{CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,			CTSVC_SEARCH_PROPERTY_ALL,	"has_phonenumber"},
	{CTSVC_PROPERTY_PERSON_HAS_EMAIL,				CTSVC_SEARCH_PROPERTY_ALL,	"has_email"},
	// number
	{CTSVC_PROPERTY_NUMBER_ID,						CTSVC_SEARCH_PROPERTY_ALL,	"number_id"},
	{CTSVC_PROPERTY_DATA_IS_PRIMARY_DEFAULT,		CTSVC_SEARCH_PROPERTY_ALL,	"is_primary_default"},
	{CTSVC_PROPERTY_NUMBER_TYPE,					CTSVC_SEARCH_PROPERTY_PROJECTION,	"type"},
	{CTSVC_PROPERTY_NUMBER_LABEL,					CTSVC_SEARCH_PROPERTY_PROJECTION,	"label"},
	{CTSVC_PROPERTY_NUMBER_NUMBER,				CTSVC_SEARCH_PROPERTY_ALL,	"number"},
	{CTSVC_PROPERTY_NUMBER_NUMBER_FILTER,			CTSVC_SEARCH_PROPERTY_FILTER,	"minmatch"},
	{CTSVC_PROPERTY_NUMBER_NORMALIZED_NUMBER,	CTSVC_SEARCH_PROPERTY_FILTER,	"normalized_number"},
};

const property_info_s __property_person_email[] = {	// _contacts_person_email
	{CTSVC_PROPERTY_PERSON_ID,						CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME,			CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,		CTSVC_SEARCH_PROPERTY_PROJECTION,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"name_contact_id"},
	{CTSVC_PROPERTY_PERSON_RINGTONE,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"ringtone_path"},
	{CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_PERSON_VIBRATION,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"vibration"},
	{CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"message_alert"},
	{CTSVC_PROPERTY_PERSON_IS_FAVORITE,			CTSVC_SEARCH_PROPERTY_ALL,	"is_favorite"},
	{CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,			CTSVC_SEARCH_PROPERTY_ALL,	"has_phonenumber"},
	{CTSVC_PROPERTY_PERSON_HAS_EMAIL,				CTSVC_SEARCH_PROPERTY_ALL,	"has_email"},
	// email
	{CTSVC_PROPERTY_EMAIL_ID,						CTSVC_SEARCH_PROPERTY_ALL,	"email_id"},
	{CTSVC_PROPERTY_EMAIL_TYPE,					CTSVC_SEARCH_PROPERTY_PROJECTION,	"type"},
	{CTSVC_PROPERTY_EMAIL_LABEL,					CTSVC_SEARCH_PROPERTY_PROJECTION,	"label"},
	{CTSVC_PROPERTY_DATA_IS_PRIMARY_DEFAULT,		CTSVC_SEARCH_PROPERTY_ALL,	"is_primary_default"},
	{CTSVC_PROPERTY_EMAIL_EMAIL,					CTSVC_SEARCH_PROPERTY_ALL,	"email"},
};

const property_info_s __property_person_grouprel[] = {	// _contacts_person_grouprel
	{CTSVC_PROPERTY_PERSON_ID,						CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME,			CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,		CTSVC_SEARCH_PROPERTY_PROJECTION,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"name_contact_id"},
	{CTSVC_PROPERTY_PERSON_RINGTONE,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"ringtone_path"},
	{CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_PERSON_VIBRATION,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"vibration"},
	{CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"message_alert"},
	{CTSVC_PROPERTY_PERSON_STATUS,					CTSVC_SEARCH_PROPERTY_PROJECTION,	"status"},
	{CTSVC_PROPERTY_PERSON_IS_FAVORITE,			CTSVC_SEARCH_PROPERTY_ALL,	"is_favorite"},
	{CTSVC_PROPERTY_PERSON_LINK_COUNT,				CTSVC_SEARCH_PROPERTY_ALL,	"link_count"},
	{CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"addressbook_ids"},
	{CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,			CTSVC_SEARCH_PROPERTY_ALL,	"has_phonenumber"},
	{CTSVC_PROPERTY_PERSON_HAS_EMAIL,				CTSVC_SEARCH_PROPERTY_ALL,	"has_email"},
	// contacts
	{CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	// group relation
	{CTSVC_PROPERTY_GROUP_RELATION_GROUP_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"group_id"},
	// addressbook
	{CTSVC_PROPERTY_ADDRESSBOOK_NAME,				CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_name"},
	{CTSVC_PROPERTY_ADDRESSBOOK_MODE,				CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_mode"},
	{CTSVC_PROPERTY_GROUP_RELATION_CONTACT_ID,			CTSVC_SEARCH_PROPERTY_PROJECTION,	"contact_id"},
};

const property_info_s __property_person_group_assigned[] = {	// _contacts_person_group_assigned
	{CTSVC_PROPERTY_PERSON_ID,						CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME,			CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,		CTSVC_SEARCH_PROPERTY_PROJECTION,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"name_contact_id"},
	{CTSVC_PROPERTY_PERSON_RINGTONE,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"ringtone_path"},
	{CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_PERSON_VIBRATION,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"vibration"},
	{CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"message_alert"},
	{CTSVC_PROPERTY_PERSON_STATUS,					CTSVC_SEARCH_PROPERTY_PROJECTION,	"status"},
	{CTSVC_PROPERTY_PERSON_IS_FAVORITE,			CTSVC_SEARCH_PROPERTY_ALL,	"is_favorite"},
	{CTSVC_PROPERTY_PERSON_LINK_COUNT,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"link_count"},
	{CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"addressbook_ids"},
	{CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,			CTSVC_SEARCH_PROPERTY_ALL,	"has_phonenumber"},
	{CTSVC_PROPERTY_PERSON_HAS_EMAIL,				CTSVC_SEARCH_PROPERTY_ALL,	"has_email"},
	// contacts
	{CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	// group relation
	{CTSVC_PROPERTY_GROUP_RELATION_GROUP_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"group_id"},
	// addressbook
	{CTSVC_PROPERTY_ADDRESSBOOK_MODE,				CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_mode"},
	{CTSVC_PROPERTY_GROUP_RELATION_CONTACT_ID,			CTSVC_SEARCH_PROPERTY_PROJECTION,	"contact_id"},
};

const property_info_s __property_person_group_not_assigned[] = {	// _contacts_person_group_not_assigned
	{CTSVC_PROPERTY_PERSON_ID,						CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME,			CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,		CTSVC_SEARCH_PROPERTY_PROJECTION,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"name_contact_id"},
	{CTSVC_PROPERTY_PERSON_RINGTONE,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"ringtone_path"},
	{CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_PERSON_VIBRATION,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"vibration"},
	{CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"message_alert"},
	{CTSVC_PROPERTY_PERSON_STATUS,					CTSVC_SEARCH_PROPERTY_PROJECTION,	"status"},
	{CTSVC_PROPERTY_PERSON_IS_FAVORITE,			CTSVC_SEARCH_PROPERTY_ALL,	"is_favorite"},
	{CTSVC_PROPERTY_PERSON_LINK_COUNT,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"link_count"},
	{CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"addressbook_ids"},
	{CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,			CTSVC_SEARCH_PROPERTY_ALL,	"has_phonenumber"},
	{CTSVC_PROPERTY_PERSON_HAS_EMAIL,				CTSVC_SEARCH_PROPERTY_ALL,	"has_email"},
	// contacts
	{CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	{CTSVC_PROPERTY_CONTACT_ID,							CTSVC_SEARCH_PROPERTY_PROJECTION,	"contact_id"},
	// addressbook
	{CTSVC_PROPERTY_ADDRESSBOOK_MODE,				CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_mode"},
};

const property_info_s __property_person_phonelog[] = {	// _contacts_person_phone_log
	{CTSVC_PROPERTY_PERSON_ID,						CTSVC_SEARCH_PROPERTY_ALL,	"id"},
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME,			CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"image_thumbnail_path"},
	// phonelog
	{CTSVC_PROPERTY_PHONELOG_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"phonelog_id"},
	{CTSVC_PROPERTY_PHONELOG_ADDRESS,			CTSVC_SEARCH_PROPERTY_ALL,	"address"},
	{CTSVC_PROPERTY_DATA_DATA1,					CTSVC_SEARCH_PROPERTY_PROJECTION,	"address_type"},
	{CTSVC_PROPERTY_PHONELOG_LOG_TIME,			CTSVC_SEARCH_PROPERTY_ALL,	"log_time"},
	{CTSVC_PROPERTY_PHONELOG_LOG_TYPE,			CTSVC_SEARCH_PROPERTY_ALL,	"log_type"},
	{CTSVC_PROPERTY_PHONELOG_EXTRA_DATA1,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"data1"},		// duration
	{CTSVC_PROPERTY_PHONELOG_EXTRA_DATA2,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"data2"},		// message_id
	{CTSVC_PROPERTY_PHONELOG_NORMALIZED_ADDRESS,	CTSVC_SEARCH_PROPERTY_FILTER,	"normal_num"},
};

const property_info_s __property_person_usage[] = {	// _contacts_person_usage
	{CTSVC_PROPERTY_PERSON_ID,						CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME,			CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX,				CTSVC_SEARCH_PROPERTY_PROJECTION,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"name_contact_id"},
	{CTSVC_PROPERTY_PERSON_RINGTONE,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"ringtone_path"},
	{CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_PERSON_VIBRATION,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"vibration"},
	{CTSVC_PROPERTY_PERSON_MESSAGE_ALERT,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"message_alert"},
	{CTSVC_PROPERTY_PERSON_IS_FAVORITE,			CTSVC_SEARCH_PROPERTY_ALL,	"is_favorite"},
	{CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER,			CTSVC_SEARCH_PROPERTY_ALL,	"has_phonenumber"},
	{CTSVC_PROPERTY_PERSON_HAS_EMAIL,				CTSVC_SEARCH_PROPERTY_ALL,	"has_email"},
	// contact_stat
	{CTSVC_PROPERTY_PERSON_USAGE_TYPE,				CTSVC_SEARCH_PROPERTY_ALL,	"usage_type"},
	{CTSVC_PROPERTY_PERSON_TIMES_USED,				CTSVC_SEARCH_PROPERTY_ALL,	"times_used"},
};

const property_info_s __property_contact_number[] = {		// _contacts_contact_number
	{CTSVC_PROPERTY_CONTACT_ID,					CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,		CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,			CTSVC_SEARCH_PROPERTY_PROJECTION,	"display_name_source"},
	{CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	{CTSVC_PROPERTY_CONTACT_PERSON_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_CONTACT_RINGTONE,			CTSVC_SEARCH_PROPERTY_PROJECTION,	"ringtone_path"},
	{CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,	CTSVC_SEARCH_PROPERTY_PROJECTION,	"image_thumbnail_path"},
	// number
	{CTSVC_PROPERTY_NUMBER_ID,					CTSVC_SEARCH_PROPERTY_ALL,	"number_id"},
	{CTSVC_PROPERTY_NUMBER_TYPE,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"type"},
	{CTSVC_PROPERTY_NUMBER_LABEL,				CTSVC_SEARCH_PROPERTY_PROJECTION,	"label"},
	{CTSVC_PROPERTY_NUMBER_IS_DEFAULT,			CTSVC_SEARCH_PROPERTY_ALL,	"is_default"},
	{CTSVC_PROPERTY_NUMBER_NUMBER,				CTSVC_SEARCH_PROPERTY_ALL,	"number"},
	{CTSVC_PROPERTY_NUMBER_NUMBER_FILTER,		CTSVC_SEARCH_PROPERTY_FILTER,	"minmatch"},
	{CTSVC_PROPERTY_NUMBER_NORMALIZED_NUMBER,	CTSVC_SEARCH_PROPERTY_FILTER,	"normalized_number"},
};

const property_info_s __property_contact_email[] = {		// _contacts_contact_email
	{CTSVC_PROPERTY_CONTACT_ID,					CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,		CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"display_name_source"},
	{CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	{CTSVC_PROPERTY_CONTACT_PERSON_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_CONTACT_RINGTONE,			CTSVC_SEARCH_PROPERTY_PROJECTION,	"ringtone_path"},
	{CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"image_thumbnail_path"},
	// email
	{CTSVC_PROPERTY_EMAIL_ID,						CTSVC_SEARCH_PROPERTY_ALL,	"email_id"},
	{CTSVC_PROPERTY_EMAIL_TYPE,						CTSVC_SEARCH_PROPERTY_PROJECTION,	"type"},
	{CTSVC_PROPERTY_EMAIL_LABEL,					CTSVC_SEARCH_PROPERTY_PROJECTION,	"label"},
	{CTSVC_PROPERTY_EMAIL_IS_DEFAULT,				CTSVC_SEARCH_PROPERTY_ALL,	"is_default"},
	{CTSVC_PROPERTY_EMAIL_EMAIL,					CTSVC_SEARCH_PROPERTY_ALL,	"email"},
};

const property_info_s __property_contact_grouprel[] = {		// _contacts_contact_grouprel
	{CTSVC_PROPERTY_CONTACT_ID,					CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,		CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,			CTSVC_SEARCH_PROPERTY_PROJECTION,	"display_name_source"},
	{CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	{CTSVC_PROPERTY_CONTACT_PERSON_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_CONTACT_RINGTONE,			CTSVC_SEARCH_PROPERTY_PROJECTION,	"ringtone_path"},
	{CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"image_thumbnail_path"},
	// group relation
	{CTSVC_PROPERTY_GROUP_RELATION_GROUP_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"group_id"},
	{CTSVC_PROPERTY_GROUP_RELATION_GROUP_NAME,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"group_name"},
};

const property_info_s __property_contact_activity[] = {		// _contacts_contact_activity
	{CTSVC_PROPERTY_CONTACT_ID,					CTSVC_SEARCH_PROPERTY_ALL,	"contact_id"},
	{CTSVC_PROPERTY_CONTACT_DISPLAY_NAME,		CTSVC_SEARCH_PROPERTY_ALL,	NULL},	// "dispaly_name" or "reverse_dispaly_name"
	{CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID,			CTSVC_SEARCH_PROPERTY_PROJECTION,	"display_name_source"},
	{CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"addressbook_id"},
	{CTSVC_PROPERTY_CONTACT_PERSON_ID,			CTSVC_SEARCH_PROPERTY_ALL,	"person_id"},
	{CTSVC_PROPERTY_CONTACT_RINGTONE,			CTSVC_SEARCH_PROPERTY_PROJECTION,	"ringtone_path"},
	{CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"image_thumbnail_path"},
	{CTSVC_PROPERTY_ACTIVITY_ID,				CTSVC_SEARCH_PROPERTY_ALL,	"activity_id"},
	{CTSVC_PROPERTY_ACTIVITY_SOURCE_NAME,		CTSVC_SEARCH_PROPERTY_ALL,	"source_name"},
	{CTSVC_PROPERTY_ACTIVITY_STATUS,			CTSVC_SEARCH_PROPERTY_PROJECTION,	"status"},
	{CTSVC_PROPERTY_ACTIVITY_TIMESTAMP,		CTSVC_SEARCH_PROPERTY_ALL,	"timestamp"},
	{CTSVC_PROPERTY_ACTIVITY_SERVICE_OPERATION, CTSVC_SEARCH_PROPERTY_ALL,	"service_operation"},
	{CTSVC_PROPERTY_ACTIVITY_URI,					CTSVC_SEARCH_PROPERTY_ALL,	"uri"},
	{CTSVC_PROPERTY_ADDRESSBOOK_ACCOUNT_ID,		CTSVC_SEARCH_PROPERTY_ALL,	"account_id"},
};

const property_info_s __property_phonelog_number[] = {		//_contacts_phone_log_number
	{CTSVC_PROPERTY_PHONELOG_ADDRESS,			CTSVC_SEARCH_PROPERTY_ALL,	"number"},
};

const property_info_s __property_phonelog_stat[] = {		//_contacts_phone_log_stat
	{CTSVC_PROPERTY_PHONELOG_STAT_LOG_COUNT,		CTSVC_SEARCH_PROPERTY_PROJECTION,	"log_count"},
	{CTSVC_PROPERTY_PHONELOG_STAT_LOG_TYPE,			CTSVC_SEARCH_PROPERTY_ALL,	"log_type"},
};

typedef struct {
	char *view_uri;
	ctsvc_record_type_e type;
	property_info_s *properties;
	unsigned int property_count;
}view_uri_info_s;

#define PTR_COUNT(X)	(void*)(X), sizeof(X)/sizeof(property_info_s)

static const view_uri_info_s __tables[] = {
	{CTSVC_VIEW_URI_ADDRESSBOOK,	CTSVC_RECORD_ADDRESSBOOK,		PTR_COUNT(__property_addressbook)},
	{CTSVC_VIEW_URI_GROUP,			CTSVC_RECORD_GROUP,				PTR_COUNT(__property_group)},
	{CTSVC_VIEW_URI_PERSON,			CTSVC_RECORD_PERSON,			PTR_COUNT(__property_person)},
	{CTSVC_VIEW_URI_SIMPLE_CONTACT, CTSVC_RECORD_SIMPLE_CONTACT,	PTR_COUNT(__property_simple_contact)},
	{CTSVC_VIEW_URI_CONTACT,		CTSVC_RECORD_CONTACT,			PTR_COUNT(__property_contact)},
	{CTSVC_VIEW_URI_MY_PROFILE,		CTSVC_RECORD_MY_PROFILE,		PTR_COUNT(__property_my_profile)},
	{CTSVC_VIEW_URI_ACTIVITY,		CTSVC_RECORD_ACTIVITY,			PTR_COUNT(__property_activity)},
	{CTSVC_VIEW_URI_ACTIVITY_PHOTO,	CTSVC_RECORD_ACTIVITY_PHOTO,	PTR_COUNT(__property_activity_photo)},
	{CTSVC_VIEW_URI_PHONELOG,		CTSVC_RECORD_PHONELOG,			PTR_COUNT(__property_phonelog)},
	{CTSVC_VIEW_URI_SPEEDDIAL,		CTSVC_RECORD_SPEEDDIAL,			PTR_COUNT(__property_speeddial)},
	{CTSVC_VIEW_URI_SDN,			CTSVC_RECORD_SDN,				PTR_COUNT(__property_sdn)},

	{CTSVC_VIEW_URI_NAME,			CTSVC_RECORD_NAME,				PTR_COUNT(__property_name)},
	{CTSVC_VIEW_URI_COMPANY,		CTSVC_RECORD_COMPANY,			PTR_COUNT(__property_company)},
	{CTSVC_VIEW_URI_NUMBER,		CTSVC_RECORD_NUMBER,			PTR_COUNT(__property_number)},
	{CTSVC_VIEW_URI_EMAIL,			CTSVC_RECORD_EMAIL,				PTR_COUNT(__property_email)},
	{CTSVC_VIEW_URI_URL,			CTSVC_RECORD_URL,				PTR_COUNT(__property_url)},
	{CTSVC_VIEW_URI_ADDRESS,		CTSVC_RECORD_ADDRESS,			PTR_COUNT(__property_address)},
	{CTSVC_VIEW_URI_PROFILE,		CTSVC_RECORD_PROFILE,			PTR_COUNT(__property_profile)},
	{CTSVC_VIEW_URI_RELATIONSHIP,	CTSVC_RECORD_RELATIONSHIP,		PTR_COUNT(__property_relationship)},
	{CTSVC_VIEW_URI_IMAGE,			CTSVC_RECORD_IMAGE,				PTR_COUNT(__property_image)},
	{CTSVC_VIEW_URI_NOTE,			CTSVC_RECORD_NOTE,				PTR_COUNT(__property_note)},
	{CTSVC_VIEW_URI_NICKNAME,		CTSVC_RECORD_NICKNAME,			PTR_COUNT(__property_nickname)},
	{CTSVC_VIEW_URI_EVENT,			CTSVC_RECORD_EVENT,				PTR_COUNT(__property_event)},
	{CTSVC_VIEW_URI_MESSENGER,		CTSVC_RECORD_MESSENGER,			PTR_COUNT(__property_messenger)},
	{CTSVC_VIEW_URI_GROUP_RELATION, CTSVC_RECORD_GROUP_RELATION,	PTR_COUNT(__property_group_relation)},
	{CTSVC_VIEW_URI_EXTENSION,			CTSVC_RECORD_EXTENSION,			PTR_COUNT(__property_extension)},

	{CTSVC_VIEW_URI_GROUPS_UPDATED_INFO,  CTSVC_RECORD_UPDATED_INFO, NULL, 0},
	{CTSVC_VIEW_URI_GROUPS_MEMBER_UPDATED_INFO,  CTSVC_RECORD_UPDATED_INFO, NULL, 0},
	{CTSVC_VIEW_URI_CONTACTS_UPDATED_INFO, CTSVC_RECORD_UPDATED_INFO, NULL, 0},
	{CTSVC_VIEW_URI_MY_PROFILE_UPDATED_INFO, CTSVC_RECORD_UPDATED_INFO,  NULL, 0},
	{CTSVC_VIEW_URI_GROUPRELS_UPDATED_INFO, CTSVC_RECORD_RESULT, NULL, 0},

	{CTSVC_VIEW_URI_READ_ONLY_PERSON_CONTACT,		CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_contact)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_CONTACT_INCLUDE_UNKNOWN,
	 CTSVC_RECORD_RESULT,
	 PTR_COUNT(__property_person_contact_include_unknown)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_NUMBER,		CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_number)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_EMAIL,		CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_email)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_ADDRESS, 	CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_address)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_COMPANY, 	CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_company)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_URL, 	CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_url)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_NICKNAME, 	CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_nickname)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_MESSENGER, 	CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_messenger)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_EVENT, 	CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_event)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_UNKNOWN, 	CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_unknown)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP,		CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_grouprel)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_ASSIGNED,		CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_group_assigned)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_NOT_ASSIGNED,		CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_group_not_assigned)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_PHONELOG,				CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_phonelog)},
	{CTSVC_VIEW_URI_READ_ONLY_PERSON_USAGE,				CTSVC_RECORD_RESULT, PTR_COUNT(__property_person_usage)},
	{CTSVC_VIEW_URI_READ_ONLY_CONTACT_NUMBER,				CTSVC_RECORD_RESULT, PTR_COUNT(__property_contact_number)},
	{CTSVC_VIEW_URI_READ_ONLY_CONTACT_EMAIL,				CTSVC_RECORD_RESULT, PTR_COUNT(__property_contact_email)},
	{CTSVC_VIEW_URI_READ_ONLY_CONTACT_GROUP,				CTSVC_RECORD_RESULT, PTR_COUNT(__property_contact_grouprel)},
	{CTSVC_VIEW_URI_READ_ONLY_CONTACT_ACTIVITY,			CTSVC_RECORD_RESULT, PTR_COUNT(__property_contact_activity)},
	{CTSVC_VIEW_URI_READ_ONLY_PHONELOG_NUMBER,				CTSVC_RECORD_RESULT, PTR_COUNT(__property_phonelog_number)},
	{CTSVC_VIEW_URI_READ_ONLY_PHONELOG_STAT,				CTSVC_RECORD_RESULT, PTR_COUNT(__property_phonelog_stat)},
	{CTSVC_VIEW_URI_CONTACT_INCLUDE_UNKNOWN,        CTSVC_RECORD_UNKNOWN,                   PTR_COUNT(__property_contact)},
};

static GHashTable *__ctsvc_view_uri_hash = NULL;

void ctsvc_view_uri_init()
{
	int i;
	int count;
	if (__ctsvc_view_uri_hash)
		return;

	__ctsvc_view_uri_hash = g_hash_table_new(g_str_hash, g_str_equal);

	i = 0;
	count = sizeof(__tables)/sizeof(view_uri_info_s);
	for (i=0;i<count;i++)
		g_hash_table_insert(__ctsvc_view_uri_hash, __tables[i].view_uri, GINT_TO_POINTER(&__tables[i]));
}

void ctsvc_view_uri_deinit()
{
#if 0
	if (NULL == __ctsvc_view_uri_hash)
		ASSERT_NOT_REACHED("__ctsvc_view_uri_hash is NULL");

	g_hash_table_destroy(__ctsvc_view_uri_hash);
	__ctsvc_view_uri_hash = NULL;
#endif
}

ctsvc_record_type_e ctsvc_view_get_record_type(const char* view_uri)
{
	view_uri_info_s* view_uri_info = NULL;
	ctsvc_record_type_e type = CTSVC_RECORD_INVALID;

	if (NULL == __ctsvc_view_uri_hash) {
		ASSERT_NOT_REACHED("__ctsvc_view_uri_hash is NULL");
	}

	view_uri_info = g_hash_table_lookup(__ctsvc_view_uri_hash, view_uri);
	if (view_uri_info)
		type = view_uri_info->type;

	return type;
}

const char* ctsvc_view_get_uri( const char* view_uri )
{
	view_uri_info_s* view_uri_info = NULL;

	if (NULL == __ctsvc_view_uri_hash) {
		ASSERT_NOT_REACHED("__ctsvc_view_uri_hash is NULL");
	}

	view_uri_info = g_hash_table_lookup(__ctsvc_view_uri_hash, view_uri);
	if (view_uri_info)
		return view_uri_info->view_uri;

	return NULL;
}

const property_info_s* ctsvc_view_get_all_property_infos(const char *view_uri, unsigned int *count)
{
	view_uri_info_s* view_uri_info = NULL;

	if (NULL == __ctsvc_view_uri_hash) {
		ASSERT_NOT_REACHED("__ctsvc_view_uri_hash is NULL");
	}

	view_uri_info = g_hash_table_lookup(__ctsvc_view_uri_hash, view_uri);
	if (view_uri_info) {
		*count = view_uri_info->property_count;
		return view_uri_info->properties;
	}

	return NULL;
}


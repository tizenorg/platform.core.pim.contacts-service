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

#ifndef __TIZEN_SOCIAL_CTSVC_VIEW_H__
#define __TIZEN_SOCIAL_CTSVC_VIEW_H__

#include "ctsvc_struct.h"

#define CTSVC_VIEW_URI_ADDRESSBOOK		"tizen.contacts_view.addressbook"
#define CTSVC_VIEW_URI_GROUP			"tizen.contacts_view.group"
#define CTSVC_VIEW_URI_PERSON			"tizen.contacts_view.person"
#define CTSVC_VIEW_URI_SIMPLE_CONTACT	"tizen.contacts_view.simple_contact"
#define CTSVC_VIEW_URI_CONTACT			"tizen.contacts_view.contact"
#define CTSVC_VIEW_URI_MY_PROFILE			"tizen.contacts_view.my_profile"
#define CTSVC_VIEW_URI_ACTIVITY			"tizen.contacts_view.activity"
#define CTSVC_VIEW_URI_ACTIVITY_PHOTO	"tizen.contacts_view.activity/photo"
#define CTSVC_VIEW_URI_PHONELOG			"tizen.contacts_view.phonelog"
#define CTSVC_VIEW_URI_SPEEDDIAL		"tizen.contacts_view.speeddial"
#define CTSVC_VIEW_URI_SDN				"tizen.contacts_view.sdn"
#define CTSVC_VIEW_URI_CONTACTS_UPDATED_INFO	"tizen.contacts_view.contacts_updated_info"
#define CTSVC_VIEW_URI_MY_PROFILE_UPDATED_INFO	"tizen.contacts_view.my_profile_updated_info"
#define CTSVC_VIEW_URI_GROUPS_UPDATED_INFO		"tizen.contacts_view.groups_updated_info"
#define CTSVC_VIEW_URI_GROUPS_MEMBER_UPDATED_INFO "tizen.contacts_view.groups_member_updated_info"
#define CTSVC_VIEW_URI_GROUPRELS_UPDATED_INFO		"tizen.contacts_view.group_relations_updated_info"
#define CTSVC_VIEW_URI_NAME				"tizen.contacts_view.name"
#define CTSVC_VIEW_URI_COMPANY			"tizen.contacts_view.company"
#define CTSVC_VIEW_URI_NUMBER			"tizen.contacts_view.number"
#define CTSVC_VIEW_URI_EMAIL			"tizen.contacts_view.email"
#define CTSVC_VIEW_URI_URL				"tizen.contacts_view.url"
#define CTSVC_VIEW_URI_ADDRESS			"tizen.contacts_view.address"
#define CTSVC_VIEW_URI_PROFILE			"tizen.contacts_view.profile"
#define CTSVC_VIEW_URI_RELATIONSHIP		"tizen.contacts_view.relationship"
#define CTSVC_VIEW_URI_IMAGE			"tizen.contacts_view.image"
#define CTSVC_VIEW_URI_NOTE				"tizen.contacts_view.note"
#define CTSVC_VIEW_URI_NICKNAME			"tizen.contacts_view.nickname"
#define CTSVC_VIEW_URI_EVENT			"tizen.contacts_view.event"
#define CTSVC_VIEW_URI_MESSENGER		"tizen.contacts_view.messenger"
#define CTSVC_VIEW_URI_GROUP_RELATION	"tizen.contacts_view.group_relation"
#define CTSVC_VIEW_URI_EXTENSION			"tizen.contacts_view.extension"

#define CTSVC_VIEW_URI_READ_ONLY_PERSON_CONTACT					"tizen.contacts_view.person/simple_contact"
#define CTSVC_VIEW_URI_READ_ONLY_PERSON_NUMBER					"tizen.contacts_view.person/simple_contact/number"
#define CTSVC_VIEW_URI_READ_ONLY_PERSON_EMAIL					"tizen.contacts_view.person/simple_contact/email"

#define CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP					"tizen.contacts_view.person/simple_contact/group"
#define CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_ASSIGNED		"tizen.contacts_view.person/simple_contact/group_assigned"
#define CTSVC_VIEW_URI_READ_ONLY_PERSON_GROUP_NOT_ASSIGNED		"tizen.contacts_view.person/simple_contact/group_not_assigned"

#define CTSVC_VIEW_URI_READ_ONLY_PERSON_PHONELOG				"tizen.contacts_view.person/simple_contact/phonelog"
#define CTSVC_VIEW_URI_READ_ONLY_PERSON_USAGE							"tizen.contacts_view.person/usage"

#define CTSVC_VIEW_URI_READ_ONLY_CONTACT_NUMBER						"tizen.contacts_view.simple_contact/number"
#define CTSVC_VIEW_URI_READ_ONLY_CONTACT_EMAIL						"tizen.contacts_view.simple_contact/email"
#define CTSVC_VIEW_URI_READ_ONLY_CONTACT_GROUP						"tizen.contacts_view.simple_contact/group"
#define CTSVC_VIEW_URI_READ_ONLY_CONTACT_ACTIVITY					"tizen.contacts_view.simple_contact/activity"

#define CTSVC_VIEW_URI_READ_ONLY_PHONELOG_STAT						"tizen.contacts_view.phonelog_stat"


typedef enum
{
	CTSVC_PROPERTY_FLAG_PROJECTION = 0x00000001,
	CTSVC_PROPERTY_FLAG_DIRTY = 0x00000002, // for dirty bit
} contacts_property_flag_e;


// for type check									// data_type mask 0x000FF000
#define CTSVC_VIEW_DATA_TYPE_MASK             0x000F0000
#define CTSVC_VIEW_DATA_TYPE_BOOL	          0x00010000
#define CTSVC_VIEW_DATA_TYPE_INT              0x00020000
#define CTSVC_VIEW_DATA_TYPE_LLI              0x00030000
#define CTSVC_VIEW_DATA_TYPE_STR              0x00040000
#define CTSVC_VIEW_DATA_TYPE_DOUBLE           0x00050000
#define CTSVC_VIEW_DATA_TYPE_REC              0x00060000
#define CTSVC_VIEW_CHECK_DATA_TYPE(property_id,data_type) \
	((property_id&CTSVC_VIEW_DATA_TYPE_MASK) == data_type ? true : false)

#define CTSVC_READ_WRITE_TYPE_MASK            0x0000F000
#define CTSVC_READ_ONLY_PROPERTY	          0x00001000

#define CTSVC_READ_ONLY_CHECK(property_id, data_type) \
	((property_id & CTSVC_READ_WRITE_TYPE_MASK) == data_type ? true : false)


// for property                            //  0x0FF00000
#define CTSVC_PROPERTY_MASK                      0x0FF00000

#define CTSVC_PROPERTY_ADDRESSBOOK              0x00100000
#define CTSVC_PROPERTY_GROUP                    0x00200000
#define CTSVC_PROPERTY_PERSON                   0x00300000
#define CTSVC_PROPERTY_ACTIVITY						0x00500000
#define CTSVC_PROPERTY_DATA							0x00600000
#define CTSVC_PROPERTY_SPEEDDIAL						0x00700000
#define CTSVC_PROPERTY_PHONELOG						0x00800000
#define CTSVC_PROPERTY_UPDATE_INFO					0x00900000
#define CTSVC_PROPERTY_SDN								0x00A00000
#define CTSVC_PROPERTY_PHONELOG_STAT				0x00B00000

#define CTSVC_PROPERTY_CONTACT                  0x01000000
#define CTSVC_PROPERTY_NAME                     0x01100000
#define CTSVC_PROPERTY_NUMBER							0x01200000
#define CTSVC_PROPERTY_EMAIL							0x01300000
#define CTSVC_PROPERTY_ADDRESS						0x01400000
#define CTSVC_PROPERTY_URL                      0x01500000
#define CTSVC_PROPERTY_EVENT							0x01600000
#define CTSVC_PROPERTY_GROUP_RELATION           0x01700000
#define CTSVC_PROPERTY_RELATIONSHIP             0x01800000
#define CTSVC_PROPERTY_COMPANY						0x01900000
#define CTSVC_PROPERTY_NICKNAME						0x01A00000
#define CTSVC_PROPERTY_MESSENGER						0x01B00000
#define CTSVC_PROPERTY_NOTE							0x01C00000
#define CTSVC_PROPERTY_PROFILE						0x01D00000
#define CTSVC_PROPERTY_IMAGE							0x01E00000
#define CTSVC_PROPERTY_EXTENSION						0x01F00000
#define CTSVC_PROPERTY_MY_PROFILE					0x02000000
#define CTSVC_PROPERTY_ACTIVITY_PHOTO				0x02100000

#define CTSVC_PROPERTY_CHECK(property_id,data_type) \
	((property_id & CTSVC_PROPERTY_MASK) == data_type ? true : false)

#define CTSVC_SEARCH_PROPERTY_MASK              0xF0000000
#define CTSVC_SEARCH_PROPERTY_NONE			      0x10000000
#define CTSVC_SEARCH_PROPERTY_FILTER            0x20000000
#define CTSVC_SEARCH_PROPERTY_PROJECTION        0x30000000
#define CTSVC_SEARCH_PROPERTY_ALL               0x40000000
#define CTSVC_SEARCH_PROPERTY_CHECK(property_id,data_type) \
	((property_id & CTSVC_SEARCH_PROPERTY_MASK) == data_type ? true : false)

typedef enum {
	// addressbook
	CTSVC_PROPERTY_ADDRESSBOOK_ID = (CTSVC_PROPERTY_ADDRESSBOOK | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_ADDRESSBOOK_ACCOUNT_ID = (CTSVC_PROPERTY_ADDRESSBOOK | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_ADDRESSBOOK_NAME = (CTSVC_PROPERTY_ADDRESSBOOK | CTSVC_VIEW_DATA_TYPE_STR) +2,
	CTSVC_PROPERTY_ADDRESSBOOK_MODE = (CTSVC_PROPERTY_ADDRESSBOOK | CTSVC_VIEW_DATA_TYPE_INT) +3,

	// group
	CTSVC_PROPERTY_GROUP_ID = (CTSVC_PROPERTY_GROUP | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_GROUP_ADDRESSBOOK_ID = (CTSVC_PROPERTY_GROUP | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_GROUP_NAME = (CTSVC_PROPERTY_GROUP | CTSVC_VIEW_DATA_TYPE_STR) +2,
	CTSVC_PROPERTY_GROUP_RINGTONE = (CTSVC_PROPERTY_GROUP | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_GROUP_IMAGE = (CTSVC_PROPERTY_GROUP | CTSVC_VIEW_DATA_TYPE_STR) +4,
	CTSVC_PROPERTY_GROUP_VIBRATION = (CTSVC_PROPERTY_GROUP | CTSVC_VIEW_DATA_TYPE_STR) +5,
	CTSVC_PROPERTY_GROUP_EXTRA_DATA = (CTSVC_PROPERTY_GROUP | CTSVC_VIEW_DATA_TYPE_STR) +6,
	CTSVC_PROPERTY_GROUP_IS_READ_ONLY = (CTSVC_PROPERTY_GROUP | CTSVC_VIEW_DATA_TYPE_BOOL) +7,
	CTSVC_PROPERTY_GROUP_MESSAGE_ALERT = (CTSVC_PROPERTY_GROUP | CTSVC_VIEW_DATA_TYPE_STR) +8,

	// person
	CTSVC_PROPERTY_PERSON_ID = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_PERSON_DISPLAY_NAME = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +1,
	CTSVC_PROPERTY_PERSON_DISPLAY_CONTACT_ID = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_PERSON_RINGTONE = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_PERSON_IMAGE_THUMBNAIL = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +4,
	CTSVC_PROPERTY_PERSON_VIBRATION = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_STR) +5,
	CTSVC_PROPERTY_PERSON_IS_FAVORITE = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_BOOL) +6,
	CTSVC_PROPERTY_PERSON_FAVORITE_PRIORITY = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_DOUBLE | CTSVC_READ_ONLY_PROPERTY) +7,
	CTSVC_PROPERTY_PERSON_LINK_COUNT = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY) +8,
	CTSVC_PROPERTY_PERSON_ADDRESSBOOK_IDS = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +9,
	CTSVC_PROPERTY_PERSON_HAS_PHONENUMBER = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_BOOL | CTSVC_READ_ONLY_PROPERTY) +10,
	CTSVC_PROPERTY_PERSON_HAS_EMAIL = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_BOOL | CTSVC_READ_ONLY_PROPERTY) +11,
	CTSVC_PROPERTY_PERSON_DISPLAY_NAME_INDEX = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +12,
	CTSVC_PROPERTY_PERSON_STATUS = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +13,
	CTSVC_PROPERTY_PERSON_MESSAGE_ALERT = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_STR) +14,

	// person-stat
	CTSVC_PROPERTY_PERSON_USAGE_TYPE = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_INT) +100,
	CTSVC_PROPERTY_PERSON_TIMES_USED = (CTSVC_PROPERTY_PERSON | CTSVC_VIEW_DATA_TYPE_INT) +101,

	// simple contact : read only
	// contact
	CTSVC_PROPERTY_CONTACT_ID = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_CONTACT_DISPLAY_NAME = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +1,
	CTSVC_PROPERTY_CONTACT_DISPLAY_SOURCE_DATA_ID = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY) +2,
	CTSVC_PROPERTY_CONTACT_ADDRESSBOOK_ID = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_INT) +3,
	CTSVC_PROPERTY_CONTACT_RINGTONE = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_STR) +4,
	CTSVC_PROPERTY_CONTACT_IMAGE = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +5,
	CTSVC_PROPERTY_CONTACT_IMAGE_THUMBNAIL = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +6,
	CTSVC_PROPERTY_CONTACT_IS_FAVORITE = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_BOOL) +7,
	CTSVC_PROPERTY_CONTACT_HAS_PHONENUMBER = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_BOOL | CTSVC_READ_ONLY_PROPERTY) +8,
	CTSVC_PROPERTY_CONTACT_HAS_EMAIL = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_BOOL | CTSVC_READ_ONLY_PROPERTY) +9,
	CTSVC_PROPERTY_CONTACT_PERSON_ID = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_INT) +10,
	CTSVC_PROPERTY_CONTACT_UID = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_STR) +11,
	CTSVC_PROPERTY_CONTACT_VIBRATION = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_STR) +12,
	CTSVC_PROPERTY_CONTACT_CHANGED_TIME = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY) +13,
	CTSVC_PROPERTY_CONTACT_NAME = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +14,
	CTSVC_PROPERTY_CONTACT_COMPANY = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +15,
	CTSVC_PROPERTY_CONTACT_NOTE = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +16,
	CTSVC_PROPERTY_CONTACT_NUMBER = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +17,
	CTSVC_PROPERTY_CONTACT_EMAIL = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +18,
	CTSVC_PROPERTY_CONTACT_EVENT = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +19,
	CTSVC_PROPERTY_CONTACT_MESSENGER = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +20,
	CTSVC_PROPERTY_CONTACT_ADDRESS = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +21,
	CTSVC_PROPERTY_CONTACT_URL = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +22,
	CTSVC_PROPERTY_CONTACT_NICKNAME = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +23,
	CTSVC_PROPERTY_CONTACT_PROFILE = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +24,
	CTSVC_PROPERTY_CONTACT_RELATIONSHIP = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +25,
	CTSVC_PROPERTY_CONTACT_GROUP_RELATION = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +26,
	CTSVC_PROPERTY_CONTACT_EXTENSION = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_REC) +27,
	CTSVC_PROPERTY_CONTACT_LINK_MODE = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_INT) +28,
	CTSVC_PROPERTY_CONTACT_MESSAGE_ALERT = (CTSVC_PROPERTY_CONTACT | CTSVC_VIEW_DATA_TYPE_STR) +29,

	// my_profile
	CTSVC_PROPERTY_MY_PROFILE_ID = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_MY_PROFILE_DISPLAY_NAME = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +1,
	CTSVC_PROPERTY_MY_PROFILE_ADDRESSBOOK_ID = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_MY_PROFILE_IMAGE = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +3,
	CTSVC_PROPERTY_MY_PROFILE_IMAGE_THUMBNAIL = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +4,
	CTSVC_PROPERTY_MY_PROFILE_UID = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_STR) +5,
	CTSVC_PROPERTY_MY_PROFILE_CHANGED_TIME = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_INT) +6,
	CTSVC_PROPERTY_MY_PROFILE_NAME = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +7,
	CTSVC_PROPERTY_MY_PROFILE_COMPANY = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +8,
	CTSVC_PROPERTY_MY_PROFILE_NOTE = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +9,
	CTSVC_PROPERTY_MY_PROFILE_NUMBER = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +10,
	CTSVC_PROPERTY_MY_PROFILE_EMAIL = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +11,
	CTSVC_PROPERTY_MY_PROFILE_EVENT = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +12,
	CTSVC_PROPERTY_MY_PROFILE_MESSENGER = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +13,
	CTSVC_PROPERTY_MY_PROFILE_ADDRESS = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +14,
	CTSVC_PROPERTY_MY_PROFILE_URL = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +15,
	CTSVC_PROPERTY_MY_PROFILE_NICKNAME = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +16,
	CTSVC_PROPERTY_MY_PROFILE_PROFILE = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +17,
	CTSVC_PROPERTY_MY_PROFILE_RELATIONSHIP = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +18,
	CTSVC_PROPERTY_MY_PROFILE_EXTENSION = (CTSVC_PROPERTY_MY_PROFILE | CTSVC_VIEW_DATA_TYPE_REC) +19,

	// contact_name
	CTSVC_PROPERTY_NAME_ID = (CTSVC_PROPERTY_NAME | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_NAME_CONTACT_ID = (CTSVC_PROPERTY_NAME | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_NAME_FIRST = (CTSVC_PROPERTY_NAME | CTSVC_VIEW_DATA_TYPE_STR) +2,
	CTSVC_PROPERTY_NAME_LAST = (CTSVC_PROPERTY_NAME | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_NAME_ADDITION = (CTSVC_PROPERTY_NAME | CTSVC_VIEW_DATA_TYPE_STR) +4,
	CTSVC_PROPERTY_NAME_SUFFIX = (CTSVC_PROPERTY_NAME | CTSVC_VIEW_DATA_TYPE_STR) +5,
	CTSVC_PROPERTY_NAME_PREFIX = (CTSVC_PROPERTY_NAME | CTSVC_VIEW_DATA_TYPE_STR) +6,
	CTSVC_PROPERTY_NAME_PHONETIC_FIRST = (CTSVC_PROPERTY_NAME | CTSVC_VIEW_DATA_TYPE_STR) +7,
	CTSVC_PROPERTY_NAME_PHONETIC_MIDDLE = (CTSVC_PROPERTY_NAME | CTSVC_VIEW_DATA_TYPE_STR) +8,
	CTSVC_PROPERTY_NAME_PHONETIC_LAST = (CTSVC_PROPERTY_NAME | CTSVC_VIEW_DATA_TYPE_STR) +9,

	// contact_number
	CTSVC_PROPERTY_NUMBER_ID = (CTSVC_PROPERTY_NUMBER | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_NUMBER_CONTACT_ID = (CTSVC_PROPERTY_NUMBER | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_NUMBER_TYPE = (CTSVC_PROPERTY_NUMBER | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_NUMBER_LABEL = (CTSVC_PROPERTY_NUMBER | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_NUMBER_IS_DEFAULT = (CTSVC_PROPERTY_NUMBER | CTSVC_VIEW_DATA_TYPE_BOOL) +4,
	CTSVC_PROPERTY_NUMBER_NUMBER = (CTSVC_PROPERTY_NUMBER | CTSVC_VIEW_DATA_TYPE_STR) +5,
	CTSVC_PROPERTY_NUMBER_NUMBER_FILTER = (CTSVC_PROPERTY_NUMBER | CTSVC_VIEW_DATA_TYPE_STR) +6,
	CTSVC_PROPERTY_NUMBER_NORMALIZED_NUMBER = (CTSVC_PROPERTY_NUMBER | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +7,
	CTSVC_PROPERTY_NUMBER_CLEANED_NUMBER = (CTSVC_PROPERTY_NUMBER | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +8,

	// contact_email
	CTSVC_PROPERTY_EMAIL_ID = (CTSVC_PROPERTY_EMAIL | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_EMAIL_CONTACT_ID = (CTSVC_PROPERTY_EMAIL | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_EMAIL_TYPE = (CTSVC_PROPERTY_EMAIL | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_EMAIL_LABEL = (CTSVC_PROPERTY_EMAIL | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_EMAIL_IS_DEFAULT = (CTSVC_PROPERTY_EMAIL | CTSVC_VIEW_DATA_TYPE_BOOL) +4,
	CTSVC_PROPERTY_EMAIL_EMAIL = (CTSVC_PROPERTY_EMAIL | CTSVC_VIEW_DATA_TYPE_STR) +5,

	// contact_address
	CTSVC_PROPERTY_ADDRESS_ID = (CTSVC_PROPERTY_ADDRESS | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_ADDRESS_CONTACT_ID = (CTSVC_PROPERTY_ADDRESS | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_ADDRESS_TYPE = (CTSVC_PROPERTY_ADDRESS | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_ADDRESS_LABEL = (CTSVC_PROPERTY_ADDRESS | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_ADDRESS_POSTBOX = (CTSVC_PROPERTY_ADDRESS | CTSVC_VIEW_DATA_TYPE_STR) +4,
	CTSVC_PROPERTY_ADDRESS_POSTAL_CODE = (CTSVC_PROPERTY_ADDRESS | CTSVC_VIEW_DATA_TYPE_STR) +5,
	CTSVC_PROPERTY_ADDRESS_REGION = (CTSVC_PROPERTY_ADDRESS | CTSVC_VIEW_DATA_TYPE_STR) +6,
	CTSVC_PROPERTY_ADDRESS_LOCALITY = (CTSVC_PROPERTY_ADDRESS | CTSVC_VIEW_DATA_TYPE_STR) +7,
	CTSVC_PROPERTY_ADDRESS_STREET = (CTSVC_PROPERTY_ADDRESS | CTSVC_VIEW_DATA_TYPE_STR) +8,
	CTSVC_PROPERTY_ADDRESS_COUNTRY = (CTSVC_PROPERTY_ADDRESS | CTSVC_VIEW_DATA_TYPE_STR) +9,
	CTSVC_PROPERTY_ADDRESS_EXTENDED = (CTSVC_PROPERTY_ADDRESS | CTSVC_VIEW_DATA_TYPE_STR) +10,
	CTSVC_PROPERTY_ADDRESS_IS_DEFAULT = (CTSVC_PROPERTY_ADDRESS | CTSVC_VIEW_DATA_TYPE_BOOL) +11,

	// contact_url
	CTSVC_PROPERTY_URL_ID = (CTSVC_PROPERTY_URL | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_URL_CONTACT_ID = (CTSVC_PROPERTY_URL | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_URL_TYPE = (CTSVC_PROPERTY_URL | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_URL_LABEL = (CTSVC_PROPERTY_URL | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_URL_URL = (CTSVC_PROPERTY_URL | CTSVC_VIEW_DATA_TYPE_STR) +4,

	// contact_event
	CTSVC_PROPERTY_EVENT_ID = (CTSVC_PROPERTY_EVENT | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_EVENT_CONTACT_ID = (CTSVC_PROPERTY_EVENT | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_EVENT_TYPE = (CTSVC_PROPERTY_EVENT | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_EVENT_LABEL = (CTSVC_PROPERTY_EVENT | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_EVENT_DATE = (CTSVC_PROPERTY_EVENT | CTSVC_VIEW_DATA_TYPE_INT) +4,
	CTSVC_PROPERTY_EVENT_CALENDAR_TYPE = (CTSVC_PROPERTY_EVENT | CTSVC_VIEW_DATA_TYPE_INT) +5,
	CTSVC_PROPERTY_EVENT_IS_LEAP_MONTH = (CTSVC_PROPERTY_EVENT | CTSVC_VIEW_DATA_TYPE_BOOL) +6,

	// contact_grouprelation
	CTSVC_PROPERTY_GROUP_RELATION_ID = (CTSVC_PROPERTY_GROUP_RELATION | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_GROUP_RELATION_GROUP_ID = (CTSVC_PROPERTY_GROUP_RELATION | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_GROUP_RELATION_CONTACT_ID = (CTSVC_PROPERTY_GROUP_RELATION | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_GROUP_RELATION_GROUP_NAME = (CTSVC_PROPERTY_GROUP_RELATION | CTSVC_VIEW_DATA_TYPE_STR) +3,

	// contact_relationship
	CTSVC_PROPERTY_RELATIONSHIP_ID = (CTSVC_PROPERTY_RELATIONSHIP | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_RELATIONSHIP_CONTACT_ID = (CTSVC_PROPERTY_RELATIONSHIP | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_RELATIONSHIP_TYPE = (CTSVC_PROPERTY_RELATIONSHIP | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_RELATIONSHIP_LABEL = (CTSVC_PROPERTY_RELATIONSHIP | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_RELATIONSHIP_NAME = (CTSVC_PROPERTY_RELATIONSHIP | CTSVC_VIEW_DATA_TYPE_STR) +4,

	// contact_image
	CTSVC_PROPERTY_IMAGE_ID = (CTSVC_PROPERTY_IMAGE | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_IMAGE_CONTACT_ID = (CTSVC_PROPERTY_IMAGE | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_IMAGE_TYPE = (CTSVC_PROPERTY_IMAGE | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_IMAGE_LABEL = (CTSVC_PROPERTY_IMAGE | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_IMAGE_PATH = (CTSVC_PROPERTY_IMAGE | CTSVC_VIEW_DATA_TYPE_STR) +4,
	CTSVC_PROPERTY_IMAGE_IS_DEFAULT = (CTSVC_PROPERTY_IMAGE | CTSVC_VIEW_DATA_TYPE_BOOL) + 5,

	// contact_company
	CTSVC_PROPERTY_COMPANY_ID = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_COMPANY_CONTACT_ID = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_COMPANY_TYPE = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_COMPANY_LABEL = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_COMPANY_NAME = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_STR) +4,
	CTSVC_PROPERTY_COMPANY_DEPARTMENT = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_STR) +5,
	CTSVC_PROPERTY_COMPANY_JOB_TITLE = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_STR) +6,
	CTSVC_PROPERTY_COMPANY_ROLE = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_STR) +7,
	CTSVC_PROPERTY_COMPANY_ASSISTANT_NAME = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_STR) +8,
	CTSVC_PROPERTY_COMPANY_LOGO = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_STR) +9,
	CTSVC_PROPERTY_COMPANY_LOCATION = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_STR) +10,
	CTSVC_PROPERTY_COMPANY_DESCRIPTION = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_STR) +11,
	CTSVC_PROPERTY_COMPANY_PHONETIC_NAME = (CTSVC_PROPERTY_COMPANY | CTSVC_VIEW_DATA_TYPE_STR) +12,

	// contact_nickname
	CTSVC_PROPERTY_NICKNAME_ID = (CTSVC_PROPERTY_NICKNAME | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_NICKNAME_CONTACT_ID = (CTSVC_PROPERTY_NICKNAME | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_NICKNAME_NAME = (CTSVC_PROPERTY_NICKNAME | CTSVC_VIEW_DATA_TYPE_STR) +2,

	// contact_messenger
	CTSVC_PROPERTY_MESSENGER_ID = (CTSVC_PROPERTY_MESSENGER | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_MESSENGER_CONTACT_ID = (CTSVC_PROPERTY_MESSENGER | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_MESSENGER_TYPE = (CTSVC_PROPERTY_MESSENGER | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_MESSENGER_LABEL = (CTSVC_PROPERTY_MESSENGER | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_MESSENGER_IM_ID = (CTSVC_PROPERTY_MESSENGER | CTSVC_VIEW_DATA_TYPE_STR) +4,

	// contact_note
	CTSVC_PROPERTY_NOTE_ID = (CTSVC_PROPERTY_NOTE | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_NOTE_CONTACT_ID = (CTSVC_PROPERTY_NOTE | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_NOTE_NOTE = (CTSVC_PROPERTY_NOTE | CTSVC_VIEW_DATA_TYPE_STR) +2,

	// contact_extend
	CTSVC_PROPERTY_EXTENSION_ID = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_EXTENSION_CONTACT_ID = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_EXTENSION_DATA1 = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_EXTENSION_DATA2 = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_EXTENSION_DATA3 = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_STR) +4,
	CTSVC_PROPERTY_EXTENSION_DATA4 = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_STR) +5,
	CTSVC_PROPERTY_EXTENSION_DATA5 = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_STR) +6,
	CTSVC_PROPERTY_EXTENSION_DATA6 = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_STR) +7,
	CTSVC_PROPERTY_EXTENSION_DATA7 = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_STR) +8,
	CTSVC_PROPERTY_EXTENSION_DATA8 = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_STR) +9,
	CTSVC_PROPERTY_EXTENSION_DATA9 = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_STR) +10,
	CTSVC_PROPERTY_EXTENSION_DATA10 = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_STR) +11,
	CTSVC_PROPERTY_EXTENSION_DATA11 = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_STR) +12,
	CTSVC_PROPERTY_EXTENSION_DATA12 = (CTSVC_PROPERTY_EXTENSION | CTSVC_VIEW_DATA_TYPE_STR) +13,

	// contact_profile
	CTSVC_PROPERTY_PROFILE_ID = (CTSVC_PROPERTY_PROFILE | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_PROFILE_CONTACT_ID = (CTSVC_PROPERTY_PROFILE | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_PROFILE_UID = (CTSVC_PROPERTY_PROFILE | CTSVC_VIEW_DATA_TYPE_STR) +2,
	CTSVC_PROPERTY_PROFILE_TEXT = (CTSVC_PROPERTY_PROFILE | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_PROFILE_ORDER = (CTSVC_PROPERTY_PROFILE | CTSVC_VIEW_DATA_TYPE_INT) +4,
	CTSVC_PROPERTY_PROFILE_SERVICE_OPERATION = (CTSVC_PROPERTY_PROFILE | CTSVC_VIEW_DATA_TYPE_STR) +5,
	CTSVC_PROPERTY_PROFILE_MIME = (CTSVC_PROPERTY_PROFILE | CTSVC_VIEW_DATA_TYPE_STR) +6,
	CTSVC_PROPERTY_PROFILE_APP_ID = (CTSVC_PROPERTY_PROFILE | CTSVC_VIEW_DATA_TYPE_STR) +7,
	CTSVC_PROPERTY_PROFILE_URI = (CTSVC_PROPERTY_PROFILE | CTSVC_VIEW_DATA_TYPE_STR) +8,
	CTSVC_PROPERTY_PROFILE_CATEGORY = (CTSVC_PROPERTY_PROFILE | CTSVC_VIEW_DATA_TYPE_STR) +9,
	CTSVC_PROPERTY_PROFILE_EXTRA_DATA = (CTSVC_PROPERTY_PROFILE | CTSVC_VIEW_DATA_TYPE_STR) +10,

	// activity
	CTSVC_PROPERTY_ACTIVITY_ID = (CTSVC_PROPERTY_ACTIVITY | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_ACTIVITY_CONTACT_ID = (CTSVC_PROPERTY_ACTIVITY | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_ACTIVITY_SOURCE_NAME = (CTSVC_PROPERTY_ACTIVITY | CTSVC_VIEW_DATA_TYPE_STR) +2,
	CTSVC_PROPERTY_ACTIVITY_STATUS = (CTSVC_PROPERTY_ACTIVITY | CTSVC_VIEW_DATA_TYPE_STR) +3,
	CTSVC_PROPERTY_ACTIVITY_TIMESTAMP = (CTSVC_PROPERTY_ACTIVITY | CTSVC_VIEW_DATA_TYPE_INT) +4,
	CTSVC_PROPERTY_ACTIVITY_SERVICE_OPERATION = (CTSVC_PROPERTY_ACTIVITY | CTSVC_VIEW_DATA_TYPE_STR) +5,
	CTSVC_PROPERTY_ACTIVITY_URI = (CTSVC_PROPERTY_ACTIVITY | CTSVC_VIEW_DATA_TYPE_STR) +6,
	CTSVC_PROPERTY_ACTIVITY_ACTIVITY_PHOTO = (CTSVC_PROPERTY_ACTIVITY | CTSVC_VIEW_DATA_TYPE_REC) +7,

	// activity photo
	CTSVC_PROPERTY_ACTIVITY_PHOTO_ID = (CTSVC_PROPERTY_ACTIVITY_PHOTO | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_ACTIVITY_PHOTO_ACTIVITY_ID = (CTSVC_PROPERTY_ACTIVITY_PHOTO | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_ACTIVITY_PHOTO_URL = (CTSVC_PROPERTY_ACTIVITY_PHOTO | CTSVC_VIEW_DATA_TYPE_STR) +2,
	CTSVC_PROPERTY_ACTIVITY_PHOTO_SORT_INDEX = (CTSVC_PROPERTY_ACTIVITY_PHOTO | CTSVC_VIEW_DATA_TYPE_INT) +3,

	// data
	CTSVC_PROPERTY_DATA_ID = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_INT),
	CTSVC_PROPERTY_DATA_CONTACT_ID = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_DATA_TYPE = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_DATA_IS_PRIMARY_DEFAULT = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_BOOL) +3,
	CTSVC_PROPERTY_DATA_IS_DEFAULT = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_BOOL) +4,
	CTSVC_PROPERTY_DATA_DATA1 = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_INT) +5,
	CTSVC_PROPERTY_DATA_DATA2 = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_STR) +6,
	CTSVC_PROPERTY_DATA_DATA3 = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_STR) +7,
	CTSVC_PROPERTY_DATA_DATA4 = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_STR) +8,
	CTSVC_PROPERTY_DATA_DATA5 = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_STR) +9,
	CTSVC_PROPERTY_DATA_DATA6 = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_STR) +10,
	CTSVC_PROPERTY_DATA_DATA7 = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_STR) +11,
	CTSVC_PROPERTY_DATA_DATA8 = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_STR) +12,
	CTSVC_PROPERTY_DATA_DATA9 = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_STR) +13,
	CTSVC_PROPERTY_DATA_DATA10 = (CTSVC_PROPERTY_DATA | CTSVC_VIEW_DATA_TYPE_STR) +14,

	// speeddial
	CTSVC_PROPERTY_SPEEDDIAL_DIAL_NUMBER = (CTSVC_PROPERTY_SPEEDDIAL | CTSVC_VIEW_DATA_TYPE_INT),
	CTSVC_PROPERTY_SPEEDDIAL_NUMBER_ID = (CTSVC_PROPERTY_SPEEDDIAL | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_SPEEDDIAL_NUMBER = (CTSVC_PROPERTY_SPEEDDIAL | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +2,
	CTSVC_PROPERTY_SPEEDDIAL_NUMBER_LABEL = (CTSVC_PROPERTY_SPEEDDIAL | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +3,
	CTSVC_PROPERTY_SPEEDDIAL_NUMBER_TYPE = (CTSVC_PROPERTY_SPEEDDIAL | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY) +4,
	CTSVC_PROPERTY_SPEEDDIAL_PERSON_ID = (CTSVC_PROPERTY_SPEEDDIAL | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY) +5,
	CTSVC_PROPERTY_SPEEDDIAL_DISPLAY_NAME = (CTSVC_PROPERTY_SPEEDDIAL | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +6,
	CTSVC_PROPERTY_SPEEDDIAL_IMAGE_THUMBNAIL = (CTSVC_PROPERTY_SPEEDDIAL | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +7,
	CTSVC_PROPERTY_SPEEDDIAL_NORMALIZED_NUMBER = (CTSVC_PROPERTY_SPEEDDIAL | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +8,
	CTSVC_PROPERTY_SPEEDDIAL_CLEANED_NUMBER = (CTSVC_PROPERTY_SPEEDDIAL | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +9,
	CTSVC_PROPERTY_SPEEDDIAL_NUMBER_FILTER = (CTSVC_PROPERTY_SPEEDDIAL | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +10,

	// phonelog
	CTSVC_PROPERTY_PHONELOG_ID = (CTSVC_PROPERTY_PHONELOG | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_PHONELOG_PERSON_ID = (CTSVC_PROPERTY_PHONELOG | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_PHONELOG_ADDRESS = (CTSVC_PROPERTY_PHONELOG | CTSVC_VIEW_DATA_TYPE_STR) +2,
	CTSVC_PROPERTY_PHONELOG_LOG_TIME = (CTSVC_PROPERTY_PHONELOG | CTSVC_VIEW_DATA_TYPE_INT) +3,
	CTSVC_PROPERTY_PHONELOG_LOG_TYPE = (CTSVC_PROPERTY_PHONELOG | CTSVC_VIEW_DATA_TYPE_INT) +4,
	CTSVC_PROPERTY_PHONELOG_EXTRA_DATA1 = (CTSVC_PROPERTY_PHONELOG | CTSVC_VIEW_DATA_TYPE_INT) +5,		// duration, message_id, email_id
	CTSVC_PROPERTY_PHONELOG_EXTRA_DATA2 = (CTSVC_PROPERTY_PHONELOG | CTSVC_VIEW_DATA_TYPE_STR) +6,		// short message, subject
	CTSVC_PROPERTY_PHONELOG_NORMALIZED_ADDRESS = (CTSVC_PROPERTY_PHONELOG | CTSVC_VIEW_DATA_TYPE_STR|CTSVC_READ_ONLY_PROPERTY) +7,		// for search by calllog number
	CTSVC_PROPERTY_PHONELOG_CLEANED_ADDRESS = (CTSVC_PROPERTY_PHONELOG | CTSVC_VIEW_DATA_TYPE_STR|CTSVC_READ_ONLY_PROPERTY) +8,		// for search by calllog number
	CTSVC_PROPERTY_PHONELOG_ADDRESS_FILTER = (CTSVC_PROPERTY_PHONELOG | CTSVC_VIEW_DATA_TYPE_STR|CTSVC_READ_ONLY_PROPERTY) +9,		// for search by calllog number
	CTSVC_PROPERTY_PHONELOG_SIM_SLOT_NO = (CTSVC_PROPERTY_PHONELOG | CTSVC_VIEW_DATA_TYPE_INT) +10,

	// updated_info : read only
	CTSVC_PROPERTY_UPDATE_INFO_ID = (CTSVC_PROPERTY_UPDATE_INFO | CTSVC_VIEW_DATA_TYPE_INT),
	CTSVC_PROPERTY_UPDATE_INFO_ADDRESSBOOK_ID = (CTSVC_PROPERTY_UPDATE_INFO | CTSVC_VIEW_DATA_TYPE_INT) +1,
	CTSVC_PROPERTY_UPDATE_INFO_TYPE = (CTSVC_PROPERTY_UPDATE_INFO | CTSVC_VIEW_DATA_TYPE_INT) +2,
	CTSVC_PROPERTY_UPDATE_INFO_VERSION = (CTSVC_PROPERTY_UPDATE_INFO | CTSVC_VIEW_DATA_TYPE_INT) +3,
	CTSVC_PROPERTY_UPDATE_INFO_IMAGE_CHANGED = (CTSVC_PROPERTY_UPDATE_INFO | CTSVC_VIEW_DATA_TYPE_BOOL) +4,
	CTSVC_PROPERTY_UPDATE_INFO_LAST_CHANGED_TYPE = (CTSVC_PROPERTY_UPDATE_INFO | CTSVC_VIEW_DATA_TYPE_INT)+5,		// now, it is used for _contacts_my_profile_updated_info

	// contact_sdn
	CTSVC_PROPERTY_SDN_ID = (CTSVC_PROPERTY_SDN | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_SDN_NAME = (CTSVC_PROPERTY_SDN | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +1,
	CTSVC_PROPERTY_SDN_NUMBER = (CTSVC_PROPERTY_SDN | CTSVC_VIEW_DATA_TYPE_STR | CTSVC_READ_ONLY_PROPERTY) +2,
	CTSVC_PROPERTY_SDN_SIM_SLOT_NO = (CTSVC_PROPERTY_SDN | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY) +3,

	// phonelog_stat
	CTSVC_PROPERTY_PHONELOG_STAT_LOG_COUNT = (CTSVC_PROPERTY_PHONELOG_STAT | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY),
	CTSVC_PROPERTY_PHONELOG_STAT_LOG_TYPE = (CTSVC_PROPERTY_PHONELOG_STAT | CTSVC_VIEW_DATA_TYPE_INT | CTSVC_READ_ONLY_PROPERTY) +1,

}ctsvc_property_ids_e;

void ctsvc_view_uri_init();
void ctsvc_view_uri_deinit();

const char* ctsvc_view_get_uri(const char* view_uri);
ctsvc_record_type_e ctsvc_view_get_record_type(const char* view_uri);
const property_info_s* ctsvc_view_get_all_property_infos(const char *view_uri, unsigned int *count);

#endif /* __TIZEN_SOCIAL_CTSVC_VIEW_H__ */

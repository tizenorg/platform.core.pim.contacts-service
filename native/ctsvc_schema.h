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

#ifndef __TIZEN_SOCIAL_CTSVC_SCHEMA_H__
#define __TIZEN_SOCIAL_CTSVC_SCHEMA_H__

#define CTSVC_DB_PATH "/opt/usr/dbspace/.contacts-svc.db"
#define CTSVC_DB_JOURNAL_PATH "/opt/usr/dbspace/.contacts-svc.db-journal"

// For Security
#define CTS_SECURITY_FILE_GROUP 6005
#define CTS_SECURITY_DEFAULT_PERMISSION 0660
#define CTS_SECURITY_DIR_DEFAULT_PERMISSION 0770

#define CTS_SCHEMA_TABLE_TOTAL 10

// DB Tables
#define CTS_TABLE_PERSONS "persons"
#define CTS_TABLE_CONTACTS "contacts"
#define CTS_TABLE_GROUPS "groups"
#define CTS_TABLE_ADDRESSBOOKS "addressbooks"
#define CTS_TABLE_DATA "data"     // This is the data table for contact all fields
#define CTS_TABLE_FAVORITES "favorites"
#define CTS_TABLE_PHONELOGS "phonelogs"
#define CTS_TABLE_PHONELOG_ACC "phonelog_accumulation"
#define CTS_TABLE_PHONELOG_STAT "phonelog_stat"
#define CTS_TABLE_GROUP_RELATIONS "group_relations"
#define CTS_TABLE_DELETEDS "contact_deleteds"
#define CTS_TABLE_GROUP_DELETEDS "group_deleteds"
#define CTS_TABLE_CUSTOM_TYPES "custom_types"
#define CTS_TABLE_SDN "sdn"
#define CTS_TABLE_SPEEDDIALS "speeddials"
#define CTS_TABLE_VERSION "cts_version"
#define CTS_TABLE_MY_PROFILES "my_profiles"
#define CTS_TABLE_CONTACT_STAT "contact_stat"
#define CTS_TABLE_NAME_LOOKUP "name_lookup"
#define CTS_TABLE_PHONE_LOOKUP "phone_lookup"
#define CTS_TABLE_SEARCH_INDEX "search_index"
#define CTS_TABLE_ACTIVITIES "activities"
#define CTS_TABLE_ACTIVITY_PHOTOS "activity_photos"

// DB views /////////////////////////////////////////////////////////////////////
#define CTSVC_DB_VIEW_PERSON					"view_person"
#define CTSVC_DB_VIEW_CONTACT					"view_contact"
#define CTSVC_DB_VIEW_MY_PROFILE				"view_my_profile"

#define CTSVC_DB_VIEW_NAME						"view_name"
#define CTSVC_DB_VIEW_NUMBER					"view_number"
#define CTSVC_DB_VIEW_EMAIL						"view_email"
#define CTSVC_DB_VIEW_ADDRESS					"view_address"
#define CTSVC_DB_VIEW_URL						"view_url"
#define CTSVC_DB_VIEW_EVENT						"view_event"
#define CTSVC_DB_VIEW_RELATIONSHIP				"view_relationship"
#define CTSVC_DB_VIEW_IMAGE						"view_image"
#define CTSVC_DB_VIEW_COMPANY					"view_company"
#define CTSVC_DB_VIEW_GROUP_RELATION			"view_group_relation"
#define CTSVC_DB_VIEW_NICKNAME					"view_nickname"
#define CTSVC_DB_VIEW_MESSENGER					"view_messenger"
#define CTSVC_DB_VIEW_NOTE						"view_note"
#define CTSVC_DB_VIEW_PROFILE					"view_profile"
#define CTSVC_DB_VIEW_EXTENSION					"view_extension"

#define CTSVC_DB_VIEW_ACTIVITY					"view_activity"
#define CTSVC_DB_VIEW_SPEEDIDAL					"view_speeddial"

//#define CTSVC_DB_VIEW_GROUPS_UPDATED_INFO		"view_group_changes"
//#define CTSVC_DB_VIEW_GROUPS_MEMBER_UPDATED_INFO	"view_group_member_changes"
//#define CTSVC_DB_VIEW_CONTACTS_UPDATED_INFO		"view_contacts_changes"

#define CTSVC_DB_VIEW_PERSON_CONTACT			"view_person_contact"
#define CTSVC_DB_VIEW_PERSON_NUMBER		"view_person_contact_number"
#define CTSVC_DB_VIEW_PERSON_EMAIL		"view_person_contact_email"
#define CTSVC_DB_VIEW_PERSON_ADDRESS		"view_person_contact_address"
#define CTSVC_DB_VIEW_PERSON_COMPANY		"view_person_contact_company"
#define CTSVC_DB_VIEW_PERSON_GROUP			"view_person_contact_group"
#define CTSVC_DB_VIEW_PERSON_GROUP_ASSIGNED	"view_person_contact_group_assigned"
#define CTSVC_DB_VIEW_PERSON_GROUP_NOT_ASSIGNED	"view_person_contact_group_not_assigned"

#define CTSVC_DB_VIEW_PERSON_PHONELOG	"view_person_contact_phonelog"
#define CTSVC_DB_VIEW_PERSON_USAGE				"view_person_usage"

#define CTSVC_DB_VIEW_CONTACT_NUMBER		"view_contact_number"
#define CTSVC_DB_VIEW_CONTACT_EMAIL			"view_contact_email"
#define CTSVC_DB_VIEW_CONTACT_GROUP			"view_contact_group"

#define CTSVC_DB_VIEW_CONTACT_ACTIVITY			"view_contact_activity"

#define CTSVC_DB_VIEW_PHONELOG_NUMBER			"view_phonelog_number"

/////////////////////////////////////////////////////////////////////////////////

#define CTS_SCHEMA_DATA_NAME_LANG_INFO "data1"
#define CTS_SCHEMA_DATA_NAME_LOOKUP "data7"
#define CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP "data8"

#define CTS_SCHEMA_SQLITE_SEQ "sqlite_sequence"

#define CTS_SCHEMA_DISPLAY_NAME "display_name"
#define CTS_SCHEMA_REVERSE_DISPLAY_NAME "reverse_display_name"
#define CTS_SCHEMA_SORTKEY  "sortkey"
#define CTS_SCHEMA_REVERSE_SORTKEY  "reverse_sortkey"


#endif /* __TIZEN_SOCIAL_CTSVC_SCHEMA_H__ */

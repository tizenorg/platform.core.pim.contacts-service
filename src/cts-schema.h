/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjae Shin <yj99.shin@samsung.com>
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
#ifndef __CTS_SCHEMA_H__
#define __CTS_SCHEMA_H__

#define CTS_DB_PATH "/opt/dbspace/.contacts-svc.db"
#define CTS_DB_JOURNAL_PATH "/opt/dbspace/.contacts-svc.db-journal"

// For Security
#define CTS_SECURITY_FILE_GROUP 6005
#define CTS_SECURITY_DEFAULT_PERMISSION 0660
#define CTS_SECURITY_DIR_DEFAULT_PERMISSION 0770

#define CTS_SCHEMA_TABLE_TOTAL 10

// Tables
#define CTS_TABLE_PERSONS "persons"
#define CTS_TABLE_CONTACTS "contacts"
#define CTS_TABLE_GROUPS "groups"
#define CTS_TABLE_ADDRESSBOOKS "addressbooks"
#define CTS_TABLE_DATA "data"     // This is the data table for contact all fields
#define CTS_TABLE_FAVORITES "favorites"
#define CTS_TABLE_PHONELOGS "phonelogs"
#define CTS_TABLE_PHONELOG_ACC "phonelog_accumulation"
#define CTS_TABLE_GROUPING_INFO "group_relations"
#define CTS_TABLE_DELETEDS "deleteds"
#define CTS_TABLE_GROUP_DELETEDS "group_deleteds"
#define CTS_TABLE_CUSTOM_TYPES "custom_types"
#define CTS_TABLE_SIM_SERVICES "sim_services"
#define CTS_TABLE_SPEEDDIALS "speeddials"
#define CTS_TABLE_VERSION "cts_version"
#define CTS_TABLE_MY_PROFILES "my_profiles"

#define CTS_TABLE_RESTRICTED_DATA_VIEW "restricted_data"

#define CTS_SCHEMA_DATA_NAME_LANG_INFO "data1"
#define CTS_SCHEMA_DATA_NAME_LOOKUP "data8"
#define CTS_SCHEMA_DATA_NAME_REVERSE_LOOKUP "data9"
#define CTS_SCHEMA_DATA_NAME_SORTING_KEY "data10"

#define CTS_SCHEMA_SQLITE_SEQ "sqlite_sequence"


#endif /* __CTS_SCHEMA_H__ */


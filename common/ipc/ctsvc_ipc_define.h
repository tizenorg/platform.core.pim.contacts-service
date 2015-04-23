/*
 * Contacts Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __CTSVC_IPC_DEFINE_H__
#define __CTSVC_IPC_DEFINE_H__

#define CTSVC_SOCK_PATH "/run/user/%d"
#define CTSVC_PATH_MAX_LEN 1024

#define CTSVC_IPC_SERVICE              "contacts_svc_ipc"
#define CTSVC_IPC_SOCKET_PATH          "/tmp/."CTSVC_IPC_SERVICE

#define CTSVC_IPC_SOCKET_PATH_FOR_CHANGE_SUBSCRIPTION    "/tmp/."CTSVC_IPC_SERVICE"_for_subscribe"
#define CTSVC_IPC_SUBSCRIBE_MODULE               "ctsvc_ipc_subscribe_module"

#define CTSVC_IPC_MODULE               "ctsvc_ipc_module"
#define CTSVC_IPC_DB_MODULE               "ctsvc_ipc_db_module"
#define CTSVC_IPC_ACTIVITY_MODULE               "ctsvc_ipc_activity_module"
#define CTSVC_IPC_GROUP_MODULE               "ctsvc_ipc_group_module"
#define CTSVC_IPC_PERSON_MODULE               "ctsvc_ipc_person_module"
#define CTSVC_IPC_PHONELOG_MODULE               "ctsvc_ipc_phonelog_module"
#define CTSVC_IPC_SIM_MODULE               "ctsvc_ipc_sim_module"
#define CTSVC_IPC_SETTING_MODULE               "ctsvc_ipc_setting_module"
#define CTSVC_IPC_UTILS_MODULE               "ctsvc_ipc_utils_module"

#define CTSVC_IPC_SERVER_CONNECT                      "connect"
#define CTSVC_IPC_SERVER_DISCONNECT                   "disconnect"
#define CTSVC_IPC_SERVER_CHECK_PERMISSION				"check_permission"

#define CTSVC_IPC_SERVER_DB_INSERT_RECORD             "insert_record"
#define CTSVC_IPC_SERVER_DB_GET_RECORD                "get_record"
#define CTSVC_IPC_SERVER_DB_UPDATE_RECORD             "update_record"
#define CTSVC_IPC_SERVER_DB_DELETE_RECORD             "delete_record"
#define CTSVC_IPC_SERVER_DB_REPLACE_RECORD			"replace_record"
#define CTSVC_IPC_SERVER_DB_GET_ALL_RECORDS           "get_all_records"
#define CTSVC_IPC_SERVER_DB_GET_RECORDS_WITH_QUERY    "get_records_with_query"
#define CTSVC_IPC_SERVER_DB_GET_COUNT                 "get_count"
#define CTSVC_IPC_SERVER_DB_GET_COUNT_WITH_QUERY      "get_count_with_query"
#define CTSVC_IPC_SERVER_DB_INSERT_RECORDS            "insert_records"
#define CTSVC_IPC_SERVER_DB_UPDATE_RECORDS            "update_records"
#define CTSVC_IPC_SERVER_DB_DELETE_RECORDS            "delete_records"
#define CTSVC_IPC_SERVER_DB_REPLACE_RECORDS			"replace_records"
#define CTSVC_IPC_SERVER_DB_CHANGES_BY_VERSION        "changes_by_version"
#define CTSVC_IPC_SERVER_DB_GET_CURRENT_VERSION       "get_current_version"
#define CTSVC_IPC_SERVER_DB_SEARCH_RECORDS            "search_records"
#define CTSVC_IPC_SERVER_DB_SEARCH_RECORDS_WITH_RANGE		"search_records_with_range"
#define CTSVC_IPC_SERVER_DB_SEARCH_RECORDS_WITH_QUERY    "search_records_with_query"
#define CTSVC_IPC_SERVER_DB_GET_STATUS					"get_db_status"
#define CTSVC_IPC_SERVER_DB_STATUS_CHANGED			"db_status_changed"


#define CTSVC_IPC_SERVER_ACTIVITY_DELETE_BY_CONTACT_ID   "activity_delete_by_contact_id"
#define CTSVC_IPC_SERVER_ACTIVITY_DELETE_BY_ACCOUNT_ID   "activity_delete_by_account_id"

#define CTSVC_IPC_SERVER_GROUP_ADD_CONTACT					 "group_add_contact"
#define CTSVC_IPC_SERVER_GROUP_REMOVE_CONTACT				 "group_remove_contact"
#define CTSVC_IPC_SERVER_GROUP_SET_GROUP_ORDER       "group_set_group_order"

#define CTSVC_IPC_SERVER_PERSON_LINK_PERSON				"person_link_person"
#define CTSVC_IPC_SERVER_PERSON_UNLINK_CONTACT			"person_unlink_contact"
#define CTSVC_IPC_SERVER_PERSON_RESET_USAGE				"person_reset_usgae"
#define CTSVC_IPC_SERVER_PERSON_SET_FAVORITE_ORDER       "person_set_favorite_order"
#define CTSVC_IPC_SERVER_PERSON_SET_DEFAULT_PROPERTY      "person_set_default_property"
#define CTSVC_IPC_SERVER_PERSON_GET_DEFAULT_PROPERTY      "person_get_default_property"

#define CTSVC_IPC_SERVER_PHONELOG_RESET_STATISTICS    "phonelog_reset_statistics"
#define CTSVC_IPC_SERVER_PHONELOG_DELETE    "phonelog_delete"

#define CTSVC_IPC_SERVER_SETTING_GET_NAME_DISPLAY_ORDER "setting_get_name_display_order"
#define CTSVC_IPC_SERVER_SETTING_SET_NAME_DISPLAY_ORDER "setting_set_name_display_order"
#define CTSVC_IPC_SERVER_SETTING_GET_NAME_SORTING_ORDER "setting_get_name_sorting_order"
#define CTSVC_IPC_SERVER_SETTING_SET_NAME_SORTING_ORDER "setting_set_name_sorting_order"

#define CTSVC_IPC_SERVER_SIM_IMPORT_ALL_CONTACTS		"sim_import_all_contacts"

#endif /*__CTSVC_IPC_DEFINE_H__ */

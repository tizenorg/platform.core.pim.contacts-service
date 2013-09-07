/*
 * Contact Service
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pims-ipc.h>
#include <pims-ipc-svc.h>

#include "ctsvc_internal.h"
#include "ctsvc_db_init.h"
#include "ctsvc_schema.h"
#include "ctsvc_schema_recovery.h"
#include "ctsvc_server_socket.h"
#include "ctsvc_server_utils.h"
#include "ctsvc_server_bg.h"

#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_server.h"
#include "ctsvc_ipc_server2.h"
#include "ctsvc_ipc_server_sim.h"

static int __server_main(void)
{
	int ret;
	pims_ipc_svc_init(CTSVC_IPC_SOCKET_PATH, CTS_SECURITY_FILE_GROUP, 0777);

	do {
		if (pims_ipc_svc_register(CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_CONNECT, ctsvc_ipc_server_connect, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_DISCONNECT, ctsvc_ipc_server_disconnect, NULL) != 0) break;

		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_INSERT_RECORD, ctsvc_ipc_server_db_insert_record, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_RECORD, ctsvc_ipc_server_db_get_record, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_UPDATE_RECORD, ctsvc_ipc_server_db_update_record, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_DELETE_RECORD, ctsvc_ipc_server_db_delete_record, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_REPLACE_RECORD, ctsvc_ipc_server_db_replace_record, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_ALL_RECORDS, ctsvc_ipc_server_db_get_all_records, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_RECORDS_WITH_QUERY, ctsvc_ipc_server_db_get_records_with_query, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_COUNT, ctsvc_ipc_server_db_get_count, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_COUNT_WITH_QUERY, ctsvc_ipc_server_db_get_count_with_query, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_INSERT_RECORDS, ctsvc_ipc_server_db_insert_records, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_UPDATE_RECORDS, ctsvc_ipc_server_db_update_records, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_DELETE_RECORDS, ctsvc_ipc_server_db_delete_records, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_REPLACE_RECORDS, ctsvc_ipc_server_db_replace_records, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_CHANGES_BY_VERSION, ctsvc_ipc_server_db_get_changes_by_version, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_CURRENT_VERSION, ctsvc_ipc_server_db_get_current_version, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_SEARCH_RECORDS, ctsvc_ipc_server_db_search_records, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_SEARCH_RECORDS_WITH_RANGE, ctsvc_ipc_server_db_search_records_with_range, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_SEARCH_RECORDS_WITH_QUERY, ctsvc_ipc_server_db_search_records_with_query, NULL) != 0) break;

		if (pims_ipc_svc_register(CTSVC_IPC_ACTIVITY_MODULE, CTSVC_IPC_SERVER_ACTIVITY_DELETE_BY_CONTACT_ID, ctsvc_ipc_activity_delete_by_contact_id, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_ACTIVITY_MODULE, CTSVC_IPC_SERVER_ACTIVITY_DELETE_BY_ACCOUNT_ID, ctsvc_ipc_activity_delete_by_account_id, NULL) != 0) break;

		if (pims_ipc_svc_register(CTSVC_IPC_GROUP_MODULE, CTSVC_IPC_SERVER_GROUP_ADD_CONTACT, ctsvc_ipc_group_add_contact, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_GROUP_MODULE, CTSVC_IPC_SERVER_GROUP_REMOVE_CONTACT, ctsvc_ipc_group_remove_contact, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_GROUP_MODULE, CTSVC_IPC_SERVER_GROUP_SET_GROUP_ORDER, ctsvc_ipc_group_set_group_order, NULL) != 0) break;

		if (pims_ipc_svc_register(CTSVC_IPC_PERSON_MODULE, CTSVC_IPC_SERVER_PERSON_LINK_PERSON, ctsvc_ipc_person_link_person, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_PERSON_MODULE, CTSVC_IPC_SERVER_PERSON_UNLINK_CONTACT, ctsvc_ipc_person_unlink_contact, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_PERSON_MODULE, CTSVC_IPC_SERVER_PERSON_RESET_USAGE, ctsvc_ipc_person_reset_usage, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_PERSON_MODULE, CTSVC_IPC_SERVER_PERSON_SET_FAVORITE_ORDER, ctsvc_ipc_person_set_favorite_order, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_PERSON_MODULE, CTSVC_IPC_SERVER_PERSON_SET_DEFAULT_PROPERTY, ctsvc_ipc_person_set_default_property, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_PERSON_MODULE, CTSVC_IPC_SERVER_PERSON_GET_DEFAULT_PROPERTY, ctsvc_ipc_person_get_default_property, NULL) != 0) break;

		if (pims_ipc_svc_register(CTSVC_IPC_PHONELOG_MODULE, CTSVC_IPC_SERVER_PHONELOG_RESET_STATISTICS, ctsvc_ipc_phone_log_reset_statistics, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_PHONELOG_MODULE, CTSVC_IPC_SERVER_PHONELOG_DELETE, ctsvc_ipc_phone_log_delete, NULL) != 0) break;

		/*
			if (pims_ipc_svc_register(CTSVC_IPC_SIM_MODULE, CTSVC_IPC_SERVER_SIM_IMPORT_ALL_CONTACTS, ctsvc_ipc_sim_import_all_contacts, NULL) != 0) break;
		*/
		if (pims_ipc_svc_register(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_GET_NAME_DISPLAY_ORDER, ctsvc_ipc_setting_get_name_display_order, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_SET_NAME_DISPLAY_ORDER, ctsvc_ipc_setting_set_name_display_order, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_GET_NAME_SORTING_ORDER, ctsvc_ipc_setting_get_name_sorting_order, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_SET_NAME_SORTING_ORDER, ctsvc_ipc_setting_set_name_sorting_order, NULL) != 0) break;

		if (pims_ipc_svc_register(CTSVC_IPC_UTILS_MODULE, CTSVC_IPC_SERVER_UTILS_GET_INDEX_CHARACTERS, ctsvc_ipc_utils_get_index_characters, NULL) != 0) break;

		pims_ipc_svc_init_for_publish(CTSVC_IPC_SOCKET_PATH_FOR_CHANGE_SUBSCRIPTION, CTS_SECURITY_FILE_GROUP, 0660);

		ret = contacts_connect2();
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("contacts_connect2 fail(%d)", ret);
			break;
		}

		ctsvc_server_bg_add_cb();
		ctsvc_server_bg_delete_start();

		ret = ctsvc_server_init_configuration();
		CTS_DBG("%d", ret);

		pims_ipc_svc_run_main_loop(NULL);

		ctsvc_server_final_configuration();

		ctsvc_server_bg_remove_cb();

		ret = contacts_disconnect2();
		if (CONTACTS_ERROR_NONE != ret)
			CTS_DBG("%d", ret);

		pims_ipc_svc_deinit_for_publish();

		pims_ipc_svc_deinit();

		return 0;

	} while(0);

	CTS_ERR("pims_ipc_svc_register error");
	return -1;
}

int main(int argc, char *argv[])
{
	CTS_FN_CALL;
	int ret;
	ctsvc_server_check_schema();
	if (2 <= argc && !strcmp(argv[1], "schema"))
		return CONTACTS_ERROR_NONE;

	ret = ctsvc_server_socket_init();
	CTS_DBG("%d", ret);

	__server_main();

	return 0;
}


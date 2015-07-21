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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pims-ipc.h>
#include <pims-ipc-svc.h>
#include <unistd.h>
#include <grp.h>

#include "ctsvc_internal.h"
#include "ctsvc_db_init.h"
#include "ctsvc_db_schema.h"
#include "ctsvc_schema_recovery.h"
#include "ctsvc_server_socket.h"
#include "ctsvc_server_utils.h"
#include "ctsvc_server_bg.h"
#include "ctsvc_server_update.h"
#include "ctsvc_server_service.h"

#include "ctsvc_db_access_control.h"

#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_server.h"
#include "ctsvc_ipc_server2.h"
#include "ctsvc_notify.h"

#define CTSVC_TIMEOUT_FOR_DEFAULT 0

static int ctsvc_list_count = 0;
static int ctsvc_timeout_sec = CTSVC_TIMEOUT_FOR_DEFAULT;
static GMainLoop *main_loop = NULL;

static int __server_main(void)
{
	int ret;

	char sock_file[CTSVC_PATH_MAX_LEN] = {0};
	snprintf(sock_file, sizeof(sock_file), CTSVC_SOCK_PATH"/.%s", getuid(), CTSVC_IPC_SERVICE);
	pims_ipc_svc_init(sock_file, CTS_SECURITY_FILE_GROUP, 0777);

	do {
		/*
		 * register handle functions
		 * These functions will be called when requesting from client module depends on module name and function name (pims_ipc_call, ctsvc_ipc_call)
		 * pims_ipc_svc_register(MODULE_NAME, FUNCTION_NAME ...);
		 */
		if (pims_ipc_svc_register(CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_CONNECT, ctsvc_ipc_server_connect, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_DISCONNECT, ctsvc_ipc_server_disconnect, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_MODULE, CTSVC_IPC_SERVER_CHECK_PERMISSION, ctsvc_ipc_server_check_permission, NULL) != 0) break;

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
		if (pims_ipc_svc_register(CTSVC_IPC_DB_MODULE, CTSVC_IPC_SERVER_DB_GET_STATUS, ctsvc_ipc_server_db_get_status, NULL) != 0) break;

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

#ifdef ENABLE_LOG_FEATURE
		if (pims_ipc_svc_register(CTSVC_IPC_PHONELOG_MODULE, CTSVC_IPC_SERVER_PHONELOG_RESET_STATISTICS, ctsvc_ipc_phone_log_reset_statistics, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_PHONELOG_MODULE, CTSVC_IPC_SERVER_PHONELOG_DELETE, ctsvc_ipc_phone_log_delete, NULL) != 0) break;
#endif /* ENABLE_LOG_FEATURE */

		if (pims_ipc_svc_register(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_GET_NAME_DISPLAY_ORDER, ctsvc_ipc_setting_get_name_display_order, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_SET_NAME_DISPLAY_ORDER, ctsvc_ipc_setting_set_name_display_order, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_GET_NAME_SORTING_ORDER, ctsvc_ipc_setting_get_name_sorting_order, NULL) != 0) break;
		if (pims_ipc_svc_register(CTSVC_IPC_SETTING_MODULE, CTSVC_IPC_SERVER_SETTING_SET_NAME_SORTING_ORDER, ctsvc_ipc_setting_set_name_sorting_order, NULL) != 0) break;

		snprintf(sock_file, sizeof(sock_file), CTSVC_SOCK_PATH"/.%s_for_subscribe", getuid(), CTSVC_IPC_SERVICE);
		pims_ipc_svc_init_for_publish(sock_file, CTS_SECURITY_FILE_GROUP, 0660);
		ctsvc_noti_publish_socket_initialize();

		ret = ctsvc_connect();
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("contacts_connect fail(%d)", ret);
			break;
		}
		ctsvc_set_client_access_info(NULL, NULL);

		ctsvc_server_bg_add_cb();
		ctsvc_server_bg_delete_start();

		ret = ctsvc_server_init_configuration();
		CTS_DBG("%d", ret);

		main_loop = g_main_loop_new(NULL, FALSE);

		pims_ipc_svc_run_main_loop(main_loop);

		ctsvc_server_final_configuration();

		ctsvc_server_bg_remove_cb();

		ctsvc_unset_client_access_info();

		ret = ctsvc_disconnect();
		if (CONTACTS_ERROR_NONE != ret)
			CTS_DBG("%d", ret);

		pims_ipc_svc_deinit_for_publish();

		pims_ipc_svc_deinit();

		return 0;

	} while (0);

	CTS_ERR("pims_ipc_svc_register error");
	return -1;
}

void ctsvc_server_quit(void)
{
	g_main_loop_quit(main_loop);
	main_loop = NULL;
}

int ctsvc_server_get_timeout_sec(void)
{
	CTS_DBG("ctsvc_timeout_sec:%d", ctsvc_timeout_sec);
	return ctsvc_timeout_sec;
}

#define CTSVC_SECURITY_FILE_GROUP 6005
void ctsvc_create_file_set_permission(const char* file, mode_t mode)
{
	int fd, ret;
	fd = creat(file, mode);
	if (0 <= fd)
	{
		ret = fchown(fd, -1, CTSVC_SECURITY_FILE_GROUP);
		if (-1 == ret)
		{
			printf("Fail to fchown\n");
			return;
		}
		close(fd);
	}

}

void ctsvc_create_rep_set_permission(const char* directory, mode_t mode)
{
	if (-1 == access (directory, F_OK)) {
		mkdir(directory, mode);
	}
}

int main(int argc, char *argv[])
{
	CTS_FN_CALL;
	INFO("Start contacts-service");
	int ret;

	if (getuid() == 0) {   /* root */
		gid_t glist[] = {CTS_SECURITY_FILE_GROUP};
		ret = setgroups(1, glist);   /* client and server should have same Groups */
		WARN_IF(ret <0, "setgroups Fail(%d)", ret);
	}

	if (2 <= argc && STRING_EQUAL == strcmp(argv[1], "timeout"))
		ctsvc_timeout_sec = atoi(argv[2]);

	ctsvc_server_check_schema();

	ctsvc_create_rep_set_permission(DATA_REPERTORY, 0755);
	ctsvc_create_rep_set_permission(CTSVC_NOTI_REPERTORY, 0775);
	ctsvc_create_rep_set_permission(CTSVC_NOTI_IMG_REPERTORY, 0770);
	ctsvc_create_rep_set_permission(CTSVC_VCARD_IMAGE_LOCATION, 0770);
	ctsvc_create_rep_set_permission(CTS_MY_IMAGE_LOCATION, 0770);
	ctsvc_create_rep_set_permission(CTS_GROUP_IMAGE_LOCATION, 0770);
	ctsvc_create_rep_set_permission(CTS_LOGO_IMAGE_LOCATION, 0770);
	ctsvc_create_rep_set_permission(CTSVC_CONTACT_IMG_FULL_LOCATION, 0770);

	ctsvc_create_file_set_permission(CTSVC_NOTI_IPC_READY, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_ADDRESSBOOK_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_GROUP_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_PERSON_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_CONTACT_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_MY_PROFILE_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_NAME_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_NUMBER_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_EMAIL_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_EVENT_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_URL_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_GROUP_RELATION_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_ADDRESS_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_NOTE_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_COMPANY_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_RELATIONSHIP_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_IMAGE_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_NICKNAME_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_MESSENGER_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_DATA_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_SDN_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_PROFILE_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_ACTIVITY_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_ACTIVITY_PHOTO_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_PHONELOG_CHANGED, 0660);
	ctsvc_create_file_set_permission(CTSVC_NOTI_SPEEDDIAL_CHANGED, 0660);

	// update DB for compatability
	ctsvc_server_db_update();

	ctsvc_server_load_feature_list();
	ret = ctsvc_server_socket_init();
	CTS_DBG("%d", ret);

	__server_main();

	ctsvc_server_socket_deinit();

	return 0;
}


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

#include <pims-ipc.h>
#include <pims-ipc-svc.h>
#include <pims-ipc-data.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_server_change_subject.h"

#define CTSVC_SUBSCRIBE_MAX_LEN	1024

#ifdef ENABLE_LOG_FEATURE
static __thread char *__phone_log_chanaged_info = NULL;
static __thread unsigned int __phone_log_buf_size = 0;
#endif /* ENABLE_LOG_FEATURE */

static __thread char *__person_changed_info = NULL;
static __thread unsigned int __person_buf_size = 0;

static gboolean __ctsvc_publish_changes_with_data(const char *view_uri, char *data)
{
	pims_ipc_data_h indata = NULL;
	if (NULL == data)
		return true;

	indata = pims_ipc_data_create(0);
	if (NULL == indata) {
		CTS_ERR("pims_ipc_data_create error\n");
		return false;
	}

	if (pims_ipc_data_put(indata, data, strlen(data) + 1) != 0) {
		CTS_ERR("pims_ipc_data_put error\n");
		pims_ipc_data_destroy(indata);
		return false;
	}

	if (pims_ipc_svc_publish(CTSVC_IPC_SUBSCRIBE_MODULE, (char*)view_uri, indata) != 0) {
		CTS_ERR("pims_ipc_svc_publish error (%s)\n", view_uri);
		return false;
	}

	pims_ipc_data_destroy(indata);

	return true;
}

void ctsvc_change_subject_publish_changed_info()
{
	__ctsvc_publish_changes_with_data(_contacts_person._uri, __person_changed_info);
#ifdef ENABLE_LOG_FEATURE
	__ctsvc_publish_changes_with_data(_contacts_phone_log._uri, __phone_log_chanaged_info);
#endif /* ENABLE_LOG_FEATURE */
	ctsvc_change_subject_clear_changed_info();
}

void ctsvc_change_subject_clear_changed_info()
{
#ifdef ENABLE_LOG_FEATURE
	free(__phone_log_chanaged_info);
	__phone_log_chanaged_info = NULL;
	__phone_log_buf_size = 0;
#endif /* ENABLE_LOG_FEATURE */

	free(__person_changed_info);
	__person_changed_info = NULL;
	__person_buf_size = 0;
}

#ifdef ENABLE_LOG_FEATURE
void ctsvc_change_subject_add_changed_phone_log_id(contacts_changed_e type, int id)
{
	CTS_FN_CALL;
	int len = 0;
	int info_len = 0;
	char changed_info[30] = {0};

	if (NULL == __phone_log_chanaged_info) {
		__phone_log_chanaged_info = (char*)calloc(CTSVC_SUBSCRIBE_MAX_LEN, sizeof(char));
		__phone_log_buf_size = CTSVC_SUBSCRIBE_MAX_LEN;
		__phone_log_chanaged_info[0] = '\0';
	}

	len = snprintf(changed_info, sizeof(changed_info), "%d:%d,", type, id);
	info_len = strlen(__phone_log_chanaged_info);
	CTS_DBG("%s(len : %d), %s (crrent_len : %d), max_len : %d",
		changed_info, len, __phone_log_chanaged_info, info_len, __phone_log_buf_size);
	if (__phone_log_buf_size < info_len + len) {
		__phone_log_chanaged_info = realloc(__phone_log_chanaged_info, __phone_log_buf_size * 2);
		__phone_log_buf_size *= 2;
	}
	snprintf(__phone_log_chanaged_info + info_len, __phone_log_buf_size - info_len, "%s", changed_info);
	CTS_DBG("%s", __phone_log_chanaged_info);
}
#endif /* ENABLE_LOG_FEATURE */

void ctsvc_change_subject_add_changed_person_id(contacts_changed_e type, int id)
{
	CTS_FN_CALL;
	int len = 0;
	int info_len = 0;
	char changed_info[30] = {0};

	if (NULL == __person_changed_info) {
		__person_changed_info = (char*)calloc(CTSVC_SUBSCRIBE_MAX_LEN, sizeof(char));
		__person_buf_size = CTSVC_SUBSCRIBE_MAX_LEN;
		__person_changed_info[0] = '\0';
	}

	len = snprintf(changed_info, sizeof(changed_info), "%d:%d,", type, id);
	info_len = strlen(__person_changed_info);
	if (__person_buf_size < info_len + len) {
		__person_changed_info = realloc(__person_changed_info, __person_buf_size * 2);
		__person_buf_size *= 2;
	}
	snprintf(__person_changed_info + info_len, __person_buf_size - info_len, "%s", changed_info);
}

void ctsvc_change_subject_publish_setting(const char *setting_id, int value)
{
	pims_ipc_data_h indata = NULL;
	indata = pims_ipc_data_create(0);
	RETM_IF(NULL == indata, "pims_ipc_data_create error");

	if (pims_ipc_data_put(indata, (void*)&value, sizeof(int)) != 0) {
		CTS_ERR("pims_ipc_data_put error");
		pims_ipc_data_destroy(indata);
		return;
	}

	if (pims_ipc_svc_publish(CTSVC_IPC_SUBSCRIBE_MODULE, (char*)setting_id, indata) != 0)
		CTS_ERR("pims_ipc_svc_publish error (%s)", setting_id);

	pims_ipc_data_destroy(indata);
}

void ctsvc_change_subject_publish_status(contacts_db_status_e status)
{
	pims_ipc_data_h indata = NULL;
	indata = pims_ipc_data_create(0);
	RETM_IF(NULL == indata, "pims_ipc_data_create error");

	if (pims_ipc_data_put(indata, (void*)&status, sizeof(int)) != 0) {
		CTS_ERR("pims_ipc_data_put error");
		pims_ipc_data_destroy(indata);
		return;
	}

	if (pims_ipc_svc_publish(CTSVC_IPC_SUBSCRIBE_MODULE, CTSVC_IPC_SERVER_DB_STATUS_CHANGED, indata) != 0)
		CTS_ERR("pims_ipc_svc_publish error(service status)");

	pims_ipc_data_destroy(indata);
}


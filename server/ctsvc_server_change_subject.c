/*
 * Contacts Service Helper
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

static __thread char *__phone_log_chanaged_info = NULL;
static __thread unsigned int __phone_log_buf_size = 0;

static __thread char *__person_changed_info = NULL;
static __thread unsigned int __person_buf_size = 0;

static gboolean __ctsvc_publish_changes_with_data(const char *view_uri, char *data)
{
	pims_ipc_data_h indata = NULL;
	if( NULL == data )
		return true;

	indata = pims_ipc_data_create(0);
	if (!indata) {
		ERR("pims_ipc_data_create error\n");
		return false;
	}

	if (pims_ipc_data_put(indata, data, strlen(data) + 1) != 0) {
		ERR("pims_ipc_data_put error\n");
		return false;
	}

	if (pims_ipc_svc_publish(CTSVC_IPC_SUBSCRIBE_MODULE, (char*)view_uri, indata) != 0) {
		ERR("pims_ipc_svc_publish error\n");
		return false;
	}

	pims_ipc_data_destroy(indata);

	return true;
}

void ctsvc_change_subject_publish_changed_info()
{
	__ctsvc_publish_changes_with_data(_contacts_person._uri, __person_changed_info);
	__ctsvc_publish_changes_with_data(_contacts_phone_log._uri, __phone_log_chanaged_info);
	ctsvc_change_subject_clear_changed_info();
}

void ctsvc_change_subject_clear_changed_info()
{
	free(__phone_log_chanaged_info);
	__phone_log_chanaged_info = NULL;
	__phone_log_buf_size = 0;

	free(__person_changed_info);
	__person_changed_info = NULL;
	__person_buf_size = 0;
}

void ctsvc_change_subject_add_changed_phone_log_id(contacts_changed_e type, int id)
{
	CTS_FN_CALL;
	int len = 0;
	int info_len = 0;
	char changed_info[30] = {0};

	if (!__phone_log_chanaged_info) {
		__phone_log_chanaged_info = (char*)calloc(CTSVC_SUBSCRIBE_MAX_LEN, sizeof(char));
		__phone_log_buf_size = CTSVC_SUBSCRIBE_MAX_LEN;
		__phone_log_chanaged_info[0] = '\0';
	}

	len = snprintf(changed_info, sizeof(changed_info), "%d:%d,", type, id);
	info_len = strlen(__phone_log_chanaged_info);
	CTS_DBG("%s(len : %d), %s (crrent_len : %d), max_len : %d",
		changed_info, len, __phone_log_chanaged_info, info_len, __phone_log_buf_size);
	if (info_len + len > __phone_log_buf_size) {
		__phone_log_chanaged_info = realloc(__phone_log_chanaged_info, __phone_log_buf_size * 2);
		__phone_log_buf_size *= 2;
	}
	snprintf(__phone_log_chanaged_info + info_len, __phone_log_buf_size - info_len, "%s", changed_info);
	CTS_DBG("%s", __phone_log_chanaged_info);
}

void ctsvc_change_subject_add_changed_person_id(contacts_changed_e type, int id)
{
	CTS_FN_CALL;
	int len = 0;
	int info_len = 0;
	char changed_info[30] = {0};

	if (!__person_changed_info) {
		__person_changed_info = (char*)calloc(CTSVC_SUBSCRIBE_MAX_LEN, sizeof(char));
		__person_buf_size = CTSVC_SUBSCRIBE_MAX_LEN;
		__person_changed_info[0] = '\0';
	}

	len = snprintf(changed_info, sizeof(changed_info), "%d:%d,", type, id);
	info_len = strlen(__person_changed_info);
	if (info_len + len > __person_buf_size) {
		__person_changed_info = realloc(__person_changed_info, __person_buf_size * 2);
		__person_buf_size *= 2;
	}
	snprintf(__person_changed_info + info_len, __person_buf_size - info_len, "%s", changed_info);
}

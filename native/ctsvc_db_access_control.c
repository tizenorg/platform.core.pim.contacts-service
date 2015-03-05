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
#include <pthread.h>
#include <sys/smack.h>
#include <pims-ipc-svc.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_mutex.h"
#include "ctsvc_service.h"

typedef struct {
	unsigned int thread_id;
	char *smack_label;
	int permission;
	int *write_list;
	int write_list_count;
}ctsvc_permission_info_s;

static GList *__thread_list = NULL;

static int have_smack = -1;

// check SMACK enable or disable
static int __ctsvc_have_smack(void)
{
	if (-1 == have_smack) {
		if (NULL == smack_smackfs_path())
			have_smack = 0;
		else
			have_smack = 1;
	}
	return have_smack;
}

// this function is called in mutex lock
static ctsvc_permission_info_s * __ctsvc_find_access_info(unsigned int thread_id)
{
	GList *cursor;
	CTS_VERBOSE("thread id : %08x", thread_id);
	for (cursor=__thread_list;cursor;cursor = cursor->next) {
		ctsvc_permission_info_s *info = NULL;
		info = cursor->data;
		if (info->thread_id == thread_id)
			return info;
	}
	return NULL;
}

// Check the client has read permission of the file(path)
// success : CONTACTS_ERROR_NONE
// fail : return negative value
int ctsvc_have_file_read_permission(const char *path)
{
	int ret;
	int permission = -1;
	char *file_label = NULL;
	ctsvc_permission_info_s *find = NULL;
	const char *smack_label;
	int have_smack;
	unsigned int thread_id;

	have_smack = __ctsvc_have_smack();
	if (have_smack != 1)		// smack disable
		return CONTACTS_ERROR_NONE;

	// Get SMACK label of the file
	ret = smack_getlabel(path, &file_label, SMACK_LABEL_ACCESS);
	if(ret < 0) {
		CTS_ERR("smack_getlabel Fail (%d)", ret);
		return CONTACTS_ERROR_SYSTEM;
	}

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	thread_id = (unsigned int)pthread_self();
	find = __ctsvc_find_access_info(thread_id);
	if (!find) {
		CTS_ERR("does not have access info of the thread");
		free(file_label);
		ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
		return CONTACTS_ERROR_INTERNAL;
	}

	smack_label = find->smack_label;
	permission = smack_have_access(smack_label, file_label, "r");
	free(file_label);
	 if (permission == 0) {
		CTS_ERR("Thread(0x%x), smack_have_access Fail(%d) : does not have permission", thread_id, permission);
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
	}
	else if (permission != 1) {
		CTS_ERR("Thread(0x%x), smack_have_access Fail(%d)", thread_id, ret);
		ret = CONTACTS_ERROR_SYSTEM;
	}
	else {
		ret= CONTACTS_ERROR_NONE;
	}

	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
	return ret;
}

// this function is called in mutex lock
static void __ctsvc_set_permission_info(unsigned int thread_id, const char *cookie)
{
	int ret;
	int count;
	int write_index;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	ctsvc_permission_info_s *find = NULL;
	const char *smack_label;
	bool smack_enabled = false;

	if (__ctsvc_have_smack() == 1)
		smack_enabled = true;
	else
		INFO("SAMCK disabled");

	find = __ctsvc_find_access_info(thread_id);
	if (!find) {
		CTS_ERR("does not have access info of the thread");
		return;
	}
	smack_label = find->smack_label;

	if (!find->permission) {		// check once
		// contacts-service daemon has all permission
		// Or, if smack is disabled, client has all permission
		if ((smack_label && 0 == strcmp(smack_label, "contacts-service")) || !smack_enabled) {
			find->permission |= CTSVC_PERMISSION_CONTACT_READ;
			find->permission |= CTSVC_PERMISSION_CONTACT_WRITE;
			find->permission |= CTSVC_PERMISSION_PHONELOG_READ;
			find->permission |= CTSVC_PERMISSION_PHONELOG_WRITE;
		}
		else if (cookie && smack_label) {
			if (1 == (ret = smack_have_access(smack_label, "contacts-service::svc", "r")))
				find->permission |= CTSVC_PERMISSION_CONTACT_READ;
			else
				INFO("Thread(0x%x) : does not have contact read permission (%d)", thread_id, ret);
			if (1 == (ret = smack_have_access(smack_label, "contacts-service::svc", "w")))
				find->permission |= CTSVC_PERMISSION_CONTACT_WRITE;
			else
				INFO("Thread(0x%x) : does not have contact write permission (%d)", thread_id, ret);
			if (1 == (ret = smack_have_access(smack_label, "contacts-service::phonelog", "r")))
				find->permission |= CTSVC_PERMISSION_PHONELOG_READ;
			else
				INFO("Thread(0x%x) : does not have phone-log read permission (%d)", thread_id, ret);
			if (1 == (ret = smack_have_access(smack_label, "contacts-service::phonelog", "w")))
				find->permission |= CTSVC_PERMISSION_PHONELOG_WRITE;
			else
				INFO("Thread(0x%x) : does not have phone-log write permission (%d)", thread_id, ret);
		}
	}
	INFO("Thread(0x%x), info(%p), permission:%d", thread_id, find, find->permission);

	// white listing : core module
	free(find->write_list);
	find->write_list = NULL;
	find->write_list_count = 0;

	snprintf(query, sizeof(query),
			"SELECT count(addressbook_id) FROM "CTS_TABLE_ADDRESSBOOKS);
	ret = ctsvc_query_get_first_int_result(query, &count);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("DB error : ctsvc_query_get_first_int_result() Failed(%d)", ret);
		return;
	}

	if (find->permission & CTSVC_PERMISSION_CONTACT_WRITE) {
		find->write_list = calloc(count, sizeof(int));
		find->write_list_count = 0;
	}

	snprintf(query, sizeof(query),
			"SELECT addressbook_id, mode, smack_label FROM "CTS_TABLE_ADDRESSBOOKS);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		CTS_ERR("DB error : ctsvc_query_prepare() Failed(%d)", ret);
		return;
	}

	write_index = 0;
	while ((ret = ctsvc_stmt_step(stmt))) {
		int id;
		int mode;
		char *temp = NULL;

		if (1 != ret) {
			CTS_ERR("DB error : ctsvc_stmt_step() Failed(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			return;
		}

		id = ctsvc_stmt_get_int(stmt, 0);
		mode = ctsvc_stmt_get_int(stmt, 1);
		temp = ctsvc_stmt_get_text(stmt, 2);

		if (smack_label && ((0 == strcmp(temp, smack_label)) || (0 == strcmp(smack_label, "contacts-service")))) {
			if (find->permission & CTSVC_PERMISSION_CONTACT_WRITE)
				find->write_list[write_index++] = id;
			continue;
		}
		else {
			switch(mode) {
			case CONTACTS_ADDRESS_BOOK_MODE_NONE:
				if (find->permission & CTSVC_PERMISSION_CONTACT_WRITE)
					find->write_list[write_index++] = id;
				break;
			default:
				break;
			};
		}
	}

	if (find->permission & CTSVC_PERMISSION_CONTACT_WRITE)
		find->write_list_count = write_index;
	ctsvc_stmt_finalize(stmt);
}

void ctsvc_unset_client_access_info()
{
	ctsvc_permission_info_s *find = NULL;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	find = __ctsvc_find_access_info(pthread_self());
	if (find) {
		free(find->smack_label);
		free(find->write_list);
		__thread_list = g_list_remove(__thread_list, find);
		free(find);
	}
	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
}

// release permission info resource when disconnecting client
// It is set as callback function using pims-ipc API
static void __ctsvc_client_disconnected_cb(pims_ipc_h ipc, void *user_data)
{
	ctsvc_permission_info_s *info;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	info = (ctsvc_permission_info_s*)user_data;
	if (info) {
		INFO("Thread(0x%x), info(%p), permission:%d",
							info->thread_id, info, info->permission);
		free(info->smack_label);
		free(info->write_list);
		__thread_list = g_list_remove(__thread_list, info);
		free(info);
	}

	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);

	// if client did not call disconnect function such as contacts_disconnect, contacts_disconnect_on_thread
	// DB will be closed in ctsvc_contacts_internal_disconnect()
	ctsvc_contacts_internal_disconnect();
}

// Set access permission info per thread(client)
void ctsvc_set_client_access_info(const char *smack_label, const char *cookie)
{
	unsigned int thread_id;
	ctsvc_permission_info_s *info = NULL;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	thread_id = (unsigned int)pthread_self();
	info = __ctsvc_find_access_info(thread_id);
	if (NULL == info) {
		info = calloc(1, sizeof(ctsvc_permission_info_s));
		__thread_list  = g_list_append(__thread_list, info);
	}
	info->thread_id = thread_id;
	FREEandSTRDUP(info->smack_label, smack_label);
	__ctsvc_set_permission_info(thread_id, cookie);

	if (strcmp(smack_label, "contacts-service") != 0)
		pims_ipc_svc_set_client_disconnected_cb(__ctsvc_client_disconnected_cb, info);

	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);

}

// Whenever changing addressbook this function will be called
// to reset read/write permssion info of each addressbook
void ctsvc_reset_all_client_access_info()
{
	GList *cursor;
	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	for (cursor=__thread_list;cursor;cursor=cursor->next) {
		ctsvc_permission_info_s *info = (ctsvc_permission_info_s *)cursor->data;
		if (info == NULL) continue;
		__ctsvc_set_permission_info(info->thread_id, NULL);
	}
	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
}

bool ctsvc_have_permission(int permission)
{
	ctsvc_permission_info_s *find = NULL;
	unsigned int thread_id;

	have_smack = __ctsvc_have_smack();
	if (have_smack != 1)		// smack disable
		return true;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	thread_id = (unsigned int)pthread_self();
	find = __ctsvc_find_access_info(thread_id);
	if (!find) {
		ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
		return false;
	}

	if (CTSVC_PERMISSION_CONTACT_NONE == permission) {
		ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
		return false;
	}

	if ((find->permission & permission) == permission) {
		ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
		return true;
	}

	CTS_ERR("Thread(0x%x), Does not have permission %d, this module has permission %d",
			thread_id, permission, find->permission);
	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);

	return false;
}

bool ctsvc_have_ab_write_permission(int addressbook_id)
{
	int i;
	unsigned int thread_id;
	ctsvc_permission_info_s *find = NULL;

	have_smack = __ctsvc_have_smack();
	if (have_smack != 1)		// smack disable
		return true;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	thread_id = (unsigned int)pthread_self();
	find = __ctsvc_find_access_info(thread_id);
	if (!find) {
		ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
		CTS_ERR("can not found access info");
		return false;
	}

	if (NULL == find->write_list) {
		ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
		CTS_ERR("there is no write access info");
		return false;
	}

	for (i=0;i<find->write_list_count;i++) {
		if (addressbook_id == find->write_list[i]) {
			ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
			return true;
		}
	}

	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
	CTS_ERR("Thread(0x%x), Does not have write permission of addressbook(%d)", thread_id, addressbook_id);
	return false;
}

int ctsvc_get_write_permitted_addressbook_ids(int **addressbook_ids, int *count)
{
	ctsvc_permission_info_s *find = NULL;
	*count = 0;
	*addressbook_ids = NULL;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	find = __ctsvc_find_access_info(pthread_self());
	if (find) {
		if (find->write_list && find->write_list_count > 0) {
			int size = find->write_list_count * sizeof(int);
			int *list = calloc(1, size);
			memcpy(list, find->write_list, size);
			*count = find->write_list_count;
			*addressbook_ids = list;
			ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
			return CONTACTS_ERROR_NONE;
		}
		ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
	return CONTACTS_ERROR_INTERNAL;
}

char* ctsvc_get_client_smack_label()
{
	ctsvc_permission_info_s *find = NULL;
	char *smack = NULL;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	find = __ctsvc_find_access_info(pthread_self());
	if (find)
		smack = strdup(find->smack_label);
	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
	return smack;
}

int ctsvc_is_owner(int addressbook_id)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	char *smack = NULL;
	char *saved_smack = NULL;

	snprintf(query, sizeof(query),
			"SELECT addressbook_name, smack_label FROM "CTS_TABLE_ADDRESSBOOKS" "
				"WHERE addressbook_id = %d", addressbook_id);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		CTS_ERR("DB error : ctsvc_query_prepare() Failed(%d)", ret);
		return ret;
	}

	ret = ctsvc_stmt_step(stmt);
	if (1 != ret) {
		CTS_ERR("ctsvc_stmt_step() Failed(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		return ret;
	}

	ret = CONTACTS_ERROR_PERMISSION_DENIED;

	saved_smack = ctsvc_stmt_get_text(stmt, 1);
	smack = ctsvc_get_client_smack_label();

	if (smack && strcmp(smack, saved_smack) == 0)
		ret = CONTACTS_ERROR_NONE;

	ctsvc_stmt_finalize(stmt);

	free(smack);
	return ret;
}


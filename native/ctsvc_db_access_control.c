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
#include <signal.h>
#include <security-server.h>
#include <sys/smack.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_mutex.h"

typedef struct {
	unsigned int thread_id;
	char *smack_label;
	int permission;
}ctsvc_permission_info_s;

static GList *__thread_list = NULL;

static int have_smack = -1;

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
		CTS_ERR("smack_have_access Fail(%d) : does not have permission", permission);
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
	}
	else if (permission != 1) {
		CTS_ERR("smack_have_access Fail(%d)", ret);
		ret = CONTACTS_ERROR_SYSTEM;
	}
	else {
		ret= CONTACTS_ERROR_NONE;
	}

	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
	return ret;
}

static void __ctsvc_set_permission_info(unsigned int thread_id, const char *cookie)
{
	ctsvc_permission_info_s *find = NULL;
	const char *smack_label;

	find = __ctsvc_find_access_info(thread_id);
	if (!find) {
		CTS_ERR("does not have access info of the thread");
		return;
	}
	smack_label = find->smack_label;

	if (!find->permission) {		// check once
		if (smack_label && 0 == strcmp(smack_label, "contacts-service")) {
			find->permission |= CTSVC_PERMISSION_CONTACT_READ;
			find->permission |= CTSVC_PERMISSION_CONTACT_WRITE;
			find->permission |= CTSVC_PERMISSION_PHONELOG_READ;
			find->permission |= CTSVC_PERMISSION_PHONELOG_WRITE;
		}
		else if (cookie) {
			/*
			if (SECURITY_SERVER_API_SUCCESS == security_server_check_privilege_by_cookie(cookie, "contacts-service::svc", "r"))
				find->permission |= CTSVC_PERMISSION_CONTACT_READ;
			if (SECURITY_SERVER_API_SUCCESS == security_server_check_privilege_by_cookie(cookie, "contacts-service::svc", "w"))
				find->permission |= CTSVC_PERMISSION_CONTACT_WRITE;
			if (SECURITY_SERVER_API_SUCCESS == security_server_check_privilege_by_cookie(cookie, "contacts-service::phonelog", "r"))
				find->permission |= CTSVC_PERMISSION_PHONELOG_READ;
			if (SECURITY_SERVER_API_SUCCESS == security_server_check_privilege_by_cookie(cookie, "contacts-service::phonelog", "w"))
				find->permission |= CTSVC_PERMISSION_PHONELOG_WRITE;
			*/
			find->permission |= CTSVC_PERMISSION_CONTACT_READ;
			find->permission |= CTSVC_PERMISSION_CONTACT_WRITE;
			find->permission |= CTSVC_PERMISSION_PHONELOG_READ;
			find->permission |= CTSVC_PERMISSION_PHONELOG_WRITE;
		}
	}
}

void ctsvc_unset_client_access_info()
{
	ctsvc_permission_info_s *find = NULL;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	find = __ctsvc_find_access_info(pthread_self());
	if (find) {
		free(find->smack_label);
		__thread_list = g_list_remove(__thread_list, find);
		free(find);
	}
	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
}

void ctsvc_set_client_access_info(const char *smack_label, const char *cookie)
{
	unsigned int thread_id = (unsigned int)pthread_self();
	ctsvc_permission_info_s *info = NULL;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	info = __ctsvc_find_access_info(thread_id);
	if (NULL == info) {
		info = calloc(1, sizeof(ctsvc_permission_info_s));
		__thread_list  = g_list_append(__thread_list, info);
	}
	info->thread_id = thread_id;
	FREEandSTRDUP(info->smack_label, smack_label);
	__ctsvc_set_permission_info(thread_id, cookie);
	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
}

bool ctsvc_have_permission(int permission)
{
	ctsvc_permission_info_s *find = NULL;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	find = __ctsvc_find_access_info(pthread_self());
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

	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
	CTS_ERR("Does not have permission %d, this module has permission %d",
			permission, find->permission);
	return false;
}


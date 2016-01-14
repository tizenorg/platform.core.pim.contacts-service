/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <unistd.h>
#include <pthread.h>
#include <sys/smack.h>
#include <pims-ipc-svc.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_db_schema.h"
#include "ctsvc_db_sqlite.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_mutex.h"
#include "ctsvc_server_service.h"

enum {
	CTSVC_PERMISSION_SMACK,
	CTSVC_PERMISSION_BOOK_WRITABLE,
	CTSVC_PERMISSION_BOOK_READONLY,
};

typedef struct {
	int id;
	int type;
} ctsvc_writable_info_s;

typedef struct {
	unsigned int thread_id;
	pims_ipc_h ipc;
	char *smack;
	GList *writable_list; /* ctsvc_writable_info_s */
} ctsvc_permission_info_s;

static GList *__thread_list = NULL;

static int have_smack = -1;

/* check SMACK enable or disable */
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

/* this function is called in mutex lock */
static ctsvc_permission_info_s * __ctsvc_find_access_info(unsigned int thread_id)
{
	GList *cursor;

	DBG("thread id : %08x", thread_id);

	for (cursor = __thread_list; cursor; cursor = cursor->next) {
		ctsvc_permission_info_s *info = NULL;
		info = cursor->data;
		if (info->thread_id == thread_id)
			return info;
	}
	return NULL;
}

/*
 * Check the client has read permission of the file(path)
 * success : CONTACTS_ERROR_NONE
 * Fail: return negative value
 */
int ctsvc_have_file_read_permission(const char *path)
{
	CTS_FN_CALL;
	RETV_IF(NULL == path, CONTACTS_ERROR_INVALID_PARAMETER);

	if (0 != access(path, F_OK|R_OK)) {
		ERR("access(%s) Fail(%d)", path, errno);
		switch (errno) {
		case EACCES:
			return CONTACTS_ERROR_PERMISSION_DENIED;
		default:
			return CONTACTS_ERROR_SYSTEM;
		}
	}

	return CONTACTS_ERROR_NONE;
}

/* this function is called in mutex lock */
static void __ctsvc_set_permission_info(ctsvc_permission_info_s *info)
{
	int ret;
	int count;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	bool smack_enabled = false;

	if (__ctsvc_have_smack() == 1)
		smack_enabled = true;
	else
		INFO("SAMCK disabled");

	/* white listing : core module */
	g_list_free_full(info->writable_list, free);
	info->writable_list = NULL;

	/* don't have write permission */
	if (!ctsvc_have_permission(info->ipc, CTSVC_PERMISSION_CONTACT_WRITE))
		return;

	snprintf(query, sizeof(query),
			"SELECT count(addressbook_id) FROM "CTS_TABLE_ADDRESSBOOKS);
	ret = ctsvc_query_get_first_int_result(query, &count);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR(" ctsvc_query_get_first_int_result() Fail(%d)", ret);
		return;
	}

	snprintf(query, sizeof(query),
			"SELECT addressbook_id, mode, smack_label FROM "CTS_TABLE_ADDRESSBOOKS);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		return;
	}

	int i = 0;
	while ((ret = ctsvc_stmt_step(stmt))) {
		int id;
		int mode;
		char *temp = NULL;

		if (1 != ret) {
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			return;
		}

		id = ctsvc_stmt_get_int(stmt, 0);
		mode = ctsvc_stmt_get_int(stmt, 1);
		temp = ctsvc_stmt_get_text(stmt, 2);

		ctsvc_writable_info_s *wi = calloc(1, sizeof(ctsvc_writable_info_s));
		if (NULL == wi) {
			ERR("calloc() Fail");
			break;
		}

		bool is_appendable = false;
		if (!smack_enabled) { /* smack disabled */
			is_appendable = true;
			wi->id = id;
			wi->type = CTSVC_PERMISSION_SMACK;
		} else if (NULL == info->ipc) { /* contacts-service daemon */
			is_appendable = true;
			wi->id = id;
			wi->type = CTSVC_PERMISSION_SMACK;
		} else if (info->smack && temp && STRING_EQUAL == strcmp(temp, info->smack)) { /* owner */
			is_appendable = true;
			wi->id = id;
			wi->type = CTSVC_PERMISSION_SMACK;
		} else if (CONTACTS_ADDRESS_BOOK_MODE_NONE == mode) {
			is_appendable = true;
			wi->id = id;
			wi->type = CTSVC_PERMISSION_BOOK_WRITABLE;
		} else if (CONTACTS_ADDRESS_BOOK_MODE_READONLY == mode) {
			is_appendable = true;
			wi->id = id;
			wi->type = CTSVC_PERMISSION_BOOK_READONLY;
		} else {
		}

		if (true == is_appendable)
			info->writable_list = g_list_append(info->writable_list, wi);
		else
			free(wi);
	}
	ctsvc_stmt_finalize(stmt);
}

void ctsvc_unset_client_access_info(void)
{
	ctsvc_permission_info_s *find = NULL;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	find = __ctsvc_find_access_info(pthread_self());
	if (find) {
		free(find->smack);
		g_list_free_full(find->writable_list, free);
		__thread_list = g_list_remove(__thread_list, find);
		free(find);
	}
	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
}

/*
 * release permission info resource when disconnecting client
 * It is set as callback function using pims-ipc API
 */
static void __ctsvc_client_disconnected_cb(pims_ipc_h ipc, void *user_data)
{
	ctsvc_permission_info_s *info;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	info = (ctsvc_permission_info_s*)user_data;
	if (info) {
		INFO("Thread(0x%x), info(%p)", info->thread_id, info);
		free(info->smack);
		g_list_free_full(info->writable_list, free);
		__thread_list = g_list_remove(__thread_list, info);
		free(info);
	}

	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);

	/*
	 * if client did not call disconnect function
	 * such as contacts_disconnect, contacts_disconnect_on_thread
	 * DB will be closed in ctsvc_contacts_internal_disconnect()
	 */
	ctsvc_contacts_internal_disconnect();
}

void ctsvc_set_client_access_info(pims_ipc_h ipc, const char *smack)
{
	unsigned int thread_id;
	ctsvc_permission_info_s *info = NULL;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	thread_id = (unsigned int)pthread_self();
	info = __ctsvc_find_access_info(thread_id);
	if (NULL == info) {
		info = calloc(1, sizeof(ctsvc_permission_info_s));
		if (NULL == info) {
			ERR("Thread(0x%x), calloc() Fail", thread_id);
			ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
			return;
		}
		__thread_list  = g_list_append(__thread_list, info);
	}
	info->thread_id = thread_id;
	info->ipc = ipc;

	FREEandSTRDUP(info->smack, smack);
	WARN_IF(NULL == info->smack, "strdup() Fail");
	__ctsvc_set_permission_info(info);

	if (info->ipc)
		pims_ipc_svc_set_client_disconnected_cb(__ctsvc_client_disconnected_cb, info);

	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
}

/*
 * Whenever changing addressbook this function will be called
 * to reset read/write permssion info of each addressbook
 */
void ctsvc_reset_all_client_access_info()
{
	GList *cursor;
	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	for (cursor = __thread_list; cursor; cursor = cursor->next) {
		ctsvc_permission_info_s *info = cursor->data;
		if (info == NULL)
			continue;
		__ctsvc_set_permission_info(info);
	}
	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
}

bool ctsvc_have_permission(pims_ipc_h ipc, int permission)
{
	have_smack = __ctsvc_have_smack();
	if (have_smack != 1)   /* smack disable */
		return true;

	if (NULL == ipc) /* contacts-service daemon */
		return true;

	if ((CTSVC_PERMISSION_CONTACT_READ & permission) &&
			!pims_ipc_svc_check_privilege(ipc, CTSVC_PRIVILEGE_CONTACT_READ))
		return false;

	if ((CTSVC_PERMISSION_CONTACT_WRITE & permission) &&
			!pims_ipc_svc_check_privilege(ipc, CTSVC_PRIVILEGE_CONTACT_WRITE))
		return false;

	if ((CTSVC_PERMISSION_PHONELOG_READ & permission) &&
			!pims_ipc_svc_check_privilege(ipc, CTSVC_PRIVILEGE_CALLHISTORY_READ))
		return false;

	if ((CTSVC_PERMISSION_PHONELOG_WRITE & permission) &&
			!pims_ipc_svc_check_privilege(ipc, CTSVC_PRIVILEGE_CALLHISTORY_WRITE))
		return false;

	return true;
}

bool ctsvc_have_ab_write_permission(int addressbook_id, bool allow_readonly)
{
	unsigned int thread_id;
	ctsvc_permission_info_s *find = NULL;

	have_smack = __ctsvc_have_smack();
	if (have_smack != 1)   /* smack disable */
		return true;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	thread_id = (unsigned int)pthread_self();
	find = __ctsvc_find_access_info(thread_id);
	if (NULL == find) {
		ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
		ERR("can not found access info");
		return false;
	}

	if (NULL == find->writable_list) {
		ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
		ERR("there is no write access info");
		return false;
	}

	GList *cursor = find->writable_list;
	while (cursor) {
		ctsvc_writable_info_s *wi = cursor->data;
		if (NULL == wi) {
			cursor = g_list_next(cursor);
			continue;
		}
		if (addressbook_id == wi->id) {
			if (CTSVC_PERMISSION_BOOK_READONLY == wi->type) {
				if (false == allow_readonly) {
					cursor = g_list_next(cursor);
					continue;
				}
			}
			ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
			return true;
		}
		cursor = g_list_next(cursor);
	}

	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
	ERR("Thread(0x%x), Does not have write permission of addressbook(%d)", thread_id, addressbook_id);
	return false;
}

int ctsvc_get_write_permitted_addressbook_ids(int **addressbook_ids, int *count)
{
	unsigned int thread_id;
	ctsvc_permission_info_s *find = NULL;
	*count = 0;
	*addressbook_ids = NULL;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	thread_id = (unsigned int)pthread_self();
	find = __ctsvc_find_access_info(thread_id);
	if (NULL == find) {
		ERR("__ctsvc_find_access_info() Fail");
		ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	if (NULL == find->writable_list) {
		ERR("No permission info");
		ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
		return CONTACTS_ERROR_PERMISSION_DENIED;
	}

	int book_count = g_list_length(find->writable_list);
	int *book_ids = calloc(book_count, sizeof(int));
	if (NULL == book_ids) {
		ERR("Thread(0x%x), calloc() Fail", thread_id);
		ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}
	int i = 0;
	GList *cursor = find->writable_list;
	while (cursor) {
		ctsvc_writable_info_s *wi = cursor->data;
		if (NULL == wi) {
			cursor = g_list_next(cursor);
			continue;
		}
		if (CTSVC_PERMISSION_BOOK_READONLY == wi->type) {
			cursor = g_list_next(cursor);
			continue;
		}
		book_ids[i] = wi->id;
		i++;

		cursor = g_list_next(cursor);
	}

	*count = i;
	*addressbook_ids = book_ids;

	ctsvc_mutex_unlock(CTS_MUTEX_ACCESS_CONTROL);
	return CONTACTS_ERROR_NONE;
}

char* ctsvc_get_client_smack_label(void)
{
	ctsvc_permission_info_s *find = NULL;
	char *smack = NULL;

	ctsvc_mutex_lock(CTS_MUTEX_ACCESS_CONTROL);
	find = __ctsvc_find_access_info(pthread_self());
	if (find && find->smack) {
		smack = strdup(find->smack);
		WARN_IF(NULL == smack, "strdup() Fail");
	}
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
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		return ret;
	}

	ret = ctsvc_stmt_step(stmt);
	if (1 != ret) {
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		return ret;
	}

	ret = CONTACTS_ERROR_PERMISSION_DENIED;

	saved_smack = ctsvc_stmt_get_text(stmt, 1);
	smack = ctsvc_get_client_smack_label();

	if (smack && STRING_EQUAL == strcmp(smack, saved_smack))
		ret = CONTACTS_ERROR_NONE;

	ctsvc_stmt_finalize(stmt);

	free(smack);
	return ret;
}


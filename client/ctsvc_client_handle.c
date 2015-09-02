/*
 * Contacts Service
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <glib.h>

#include "ctsvc_internal.h"
#include "ctsvc_handle.h"
#include "ctsvc_mutex.h"
#include "ctsvc_client_handle.h"

static GHashTable *_ctsvc_handle_table = NULL;

static int _ctsvc_client_handle_get_key(char *key, int key_len)
{
	int ret;
	int len;

	ret = gethostname(key, key_len);
	RETVM_IF(0 != ret, CONTACTS_ERROR_SYSTEM, "gethostname() Failed(%d)", errno);

	len = strlen(key);
	snprintf(key+len, key_len-len, ":%d", (int)pthread_self());

	return CONTACTS_ERROR_NONE;
}

int ctsvc_client_handle_get_current_p(contacts_h *p_contact)
{
	int ret;
	char key[CTSVC_STR_SHORT_LEN] = {0};
	contacts_h contact = NULL;

	RETVM_IF(NULL == _ctsvc_handle_table, CONTACTS_ERROR_NO_DATA, "_ctsvc_handle_table is NULL");

	ret = _ctsvc_client_handle_get_key(key, sizeof(key));
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "_ctsvc_client_handle_get_key() Fail(%d)", ret);

	ctsvc_mutex_lock(CTS_MUTEX_HANDLE);
	contact = g_hash_table_lookup(_ctsvc_handle_table, key);
	ctsvc_mutex_unlock(CTS_MUTEX_HANDLE);
	RETVM_IF(NULL == contact, CONTACTS_ERROR_NO_DATA, "g_hash_table_lookup() return NULL");

	*p_contact = contact;
	return CONTACTS_ERROR_NONE;
}

static int _ctsvc_client_handle_add(contacts_h contact)
{
	int ret;
	char key[CTSVC_STR_SHORT_LEN] = {0};

	if (NULL == _ctsvc_handle_table)
		_ctsvc_handle_table = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);

	ret = _ctsvc_client_handle_get_key(key, sizeof(key));
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "_ctsvc_client_handle_get_key() Fail(%d)", ret);

	g_hash_table_insert(_ctsvc_handle_table, strdup(key), contact);

	ctsvc_mutex_unlock(CTS_MUTEX_HANDLE);
	return CONTACTS_ERROR_NONE;
}

int ctsvc_client_handle_create(contacts_h *p_contact)
{
	int ret;
	contacts_h contact = NULL;

	RETVM_IF(NULL == p_contact, CONTACTS_ERROR_INVALID_PARAMETER, "p_contact is NULL");
	*p_contact = NULL;

	ret = ctsvc_handle_create(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_handle_create() Fail(%d)", ret);

	ret = _ctsvc_client_handle_add(contact);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("_ctsvc_client_handle_add() Fail(%d)", ret);
		ctsvc_handle_destroy(contact);
		return ret;
	}

	*p_contact = contact;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_client_handle_remove(contacts_h contact)
{
	int ret;
	char key[CTSVC_STR_SHORT_LEN] = {0};
	RETVM_IF(NULL == _ctsvc_handle_table, CONTACTS_ERROR_NONE, "_ctsvc_handle_table is NULL");

	ret = _ctsvc_client_handle_get_key(key, sizeof(key));
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "_ctsvc_client_handle_get_key() Fail(%d)", ret);

	ctsvc_mutex_lock(CTS_MUTEX_HANDLE);
	g_hash_table_remove(_ctsvc_handle_table, key);
	if (0 == g_hash_table_size(_ctsvc_handle_table)) {
		g_hash_table_destroy(_ctsvc_handle_table);
		_ctsvc_handle_table = NULL;
	}
	ctsvc_mutex_unlock(CTS_MUTEX_HANDLE);
	ctsvc_handle_destroy(contact);

	return CONTACTS_ERROR_NONE;
}



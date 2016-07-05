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

#include <glib.h>
#include <unistd.h>

#include "ctsvc_internal.h"
#include "ctsvc_handle.h"
#include "ctsvc_mutex.h"
#include "ctsvc_client_utils.h"
#include "ctsvc_client_handle.h"

static GHashTable *_ctsvc_handle_table = NULL;

static int _client_handle_get_key(unsigned int id, char *key, int key_len)
{
	RETV_IF(NULL == key, CONTACTS_ERROR_INVALID_PARAMETER);

	snprintf(key, key_len, "%d:%u", getuid(), id);
	DBG("key[%s]", key);
	return CONTACTS_ERROR_NONE;
}

int ctsvc_client_handle_get_p_with_id(unsigned int id, contacts_h *p_contact)
{
	int ret;
	char key[CTSVC_STR_SHORT_LEN] = {0};
	contacts_h contact = NULL;

	RETV_IF(NULL == _ctsvc_handle_table, CONTACTS_ERROR_NO_DATA);
	RETV_IF(NULL == p_contact, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = _client_handle_get_key(id, key, sizeof(key));
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "_client_handle_get_key() Fail(%d)", ret);

	ctsvc_mutex_lock(CTS_MUTEX_HANDLE);
	contact = g_hash_table_lookup(_ctsvc_handle_table, key);
	ctsvc_mutex_unlock(CTS_MUTEX_HANDLE);

	if (NULL == contact) {
		/* LCOV_EXCL_START */
		ERR("g_hash_table_lookup() Fail. key[%s]", key);
		return CONTACTS_ERROR_NO_DATA;
		/* LCOV_EXCL_STOP */
	}

	*p_contact = contact;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_client_handle_get_p(contacts_h *p_contact)
{
	int ret;
	char key[CTSVC_STR_SHORT_LEN] = {0};
	contacts_h contact = NULL;

	RETV_IF(NULL == _ctsvc_handle_table, CONTACTS_ERROR_NO_DATA);
	RETV_IF(NULL == p_contact, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = _client_handle_get_key(ctsvc_client_get_tid(), key, sizeof(key));
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "_client_handle_get_key() Fail(%d)", ret);

	ctsvc_mutex_lock(CTS_MUTEX_HANDLE);
	contact = g_hash_table_lookup(_ctsvc_handle_table, key);
	ctsvc_mutex_unlock(CTS_MUTEX_HANDLE);

	if (NULL == contact) {
		ret = _client_handle_get_key(ctsvc_client_get_pid(), key, sizeof(key));
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "_client_handle_get_key() Fail(%d)", ret);

		ctsvc_mutex_lock(CTS_MUTEX_HANDLE);
		contact = g_hash_table_lookup(_ctsvc_handle_table, key);
		ctsvc_mutex_unlock(CTS_MUTEX_HANDLE);
		RETVM_IF(NULL == contact, CONTACTS_ERROR_NO_DATA, "g_hash_table_lookup(%s) Fail", key);
	}
	*p_contact = contact;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_client_handle_create(unsigned int id, contacts_h *p_contact)
{
	int ret;
	char handle_key[CTSVC_STR_SHORT_LEN] = {0};
	contacts_h contact = NULL;

	RETV_IF(NULL == p_contact, CONTACTS_ERROR_INVALID_PARAMETER);
	*p_contact = NULL;

	ret = _client_handle_get_key(id, handle_key, sizeof(handle_key));
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "_client_handle_get_key() Fail(%d)", ret);

	ret = ctsvc_handle_create(&contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_handle_create() Fail(%d)", ret);

	ctsvc_mutex_lock(CTS_MUTEX_HANDLE);
	if (NULL == _ctsvc_handle_table)
		_ctsvc_handle_table = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);

	g_hash_table_insert(_ctsvc_handle_table, strdup(handle_key), contact);
	INFO("handle insert key[%s]", handle_key);
	ctsvc_mutex_unlock(CTS_MUTEX_HANDLE);

	*p_contact = contact;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_client_handle_remove(unsigned int id, contacts_h contact)
{
	int ret;
	char key[CTSVC_STR_SHORT_LEN] = {0};

	RETV_IF(NULL == _ctsvc_handle_table, CONTACTS_ERROR_NONE);

	ret = _client_handle_get_key(id, key, sizeof(key));
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "_client_handle_get_key() Fail(%d)", ret);

	ctsvc_mutex_lock(CTS_MUTEX_HANDLE);
	if (false == g_hash_table_remove(_ctsvc_handle_table, key))
		ERR("g_hash_table_remove() Fail. key[%s]", key);

	if (0 == g_hash_table_size(_ctsvc_handle_table)) {
		g_hash_table_destroy(_ctsvc_handle_table);
		_ctsvc_handle_table = NULL;
	}
	ctsvc_handle_destroy(contact);
	ctsvc_mutex_unlock(CTS_MUTEX_HANDLE);

	return CONTACTS_ERROR_NONE;
}



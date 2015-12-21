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
#include <malloc.h>
#include <vconf.h>
#include <vconf-keys.h>
#include <ITapiPhonebook.h>
#include <TapiUtility.h>
#include <system_info.h>

#include "contacts.h"

#include "ctsvc_internal.h"
#include "ctsvc_server_setting.h"
#include "ctsvc_server_utils.h"
#include "ctsvc_server_sim.h"
#include "ctsvc_server_sqlite.h"
#include "ctsvc_localize.h"
#include "ctsvc_normalize.h"
#include "ctsvc_mutex.h"
#include "ctsvc_server.h"

#define CTSVC_FEATURE_TELEPHONY "http://tizen.org/feature/network.telephony"

static int system_language = -1;
static bool _ctsvc_have_telephony_feature = false;
static guint _ctsvc_timeout = 0;

int ctsvc_server_load_feature_list(void)
{
	system_info_get_platform_bool(CTSVC_FEATURE_TELEPHONY, &_ctsvc_have_telephony_feature);
	return CONTACTS_ERROR_NONE;
}

bool ctsvc_server_have_telephony_feature(void)
{
	return _ctsvc_have_telephony_feature;
}

inline int ctsvc_server_set_default_sort(int sort)
{
	int ret = vconf_set_int(ctsvc_get_default_sort_vconfkey(), sort);
	RETVM_IF(ret < 0, CONTACTS_ERROR_SYSTEM, "vconf_set_int() Fail(%d)", ret);
	ctsvc_set_sort_memory(sort);
	return CONTACTS_ERROR_NONE;
}

static void __ctsvc_server_change_language_cb(keynode_t *key, void *data)
{
	int ret = -1;
	int new_primary_sort, new_secondary_sort;
	int old_primary_sort, old_secondary_sort;
	char *new_langset = NULL;
	char *langset = NULL;

	new_langset = vconf_keynode_get_str(key);
	if (NULL == new_langset) {
		ERR("vconf_keynode_get_str() Fail");
		return;
	}
	langset = ctsvc_get_langset();
	INFO("%s --> %s", langset, new_langset);

	ctsvc_server_stop_timeout();

	if (STRING_EQUAL != strcmp(langset, new_langset)) {
		bool sort_name_update = false;
		old_primary_sort = ctsvc_get_primary_sort();
		if (old_primary_sort < 0)
			RETM_IF(ret < 0, "ctsvc_get_primary_sort() Fail(%d)", ret);

		old_secondary_sort = ctsvc_get_secondary_sort();
		if (old_secondary_sort < 0)
			RETM_IF(ret < 0, "ctsvc_get_secondary_sort() Fail(%d)", ret);

		if (STRING_EQUAL == strncmp(langset, "zh", strlen("zh")) ||
				STRING_EQUAL == strncmp(langset, "ko", strlen("ko")) ||
				STRING_EQUAL == strncmp(langset, "ja", strlen("ja")) ||
				STRING_EQUAL == strncmp(new_langset, "zh", strlen("zh")) ||
				STRING_EQUAL == strncmp(new_langset, "ko", strlen("ko")) ||
				STRING_EQUAL == strncmp(new_langset, "ja", strlen("ja"))) {
			sort_name_update = true;
		}
		ctsvc_set_langset(strdup(new_langset));
		langset = new_langset;

		system_language = ctsvc_get_language_type(langset);
		new_primary_sort = ctsvc_get_sort_type_from_language(system_language);
		if (new_primary_sort == CTSVC_SORT_OTHERS)
			new_primary_sort = CTSVC_SORT_WESTERN;

		new_secondary_sort = CTSVC_SORT_WESTERN;

		if (sort_name_update) {
			ctsvc_server_set_default_sort(new_primary_sort);
			ctsvc_server_update_sort_name();
		} else {
			if (new_primary_sort != old_primary_sort)
				ret = ctsvc_server_update_sort(old_primary_sort, old_secondary_sort, new_primary_sort, new_secondary_sort);

			ctsvc_server_update_collation();
		}
	}

	ctsvc_server_start_timeout();
}

void ctsvc_server_final_configuration(void)
{
	int ret = -1;

	ret = vconf_ignore_key_changed(VCONFKEY_LANGSET, __ctsvc_server_change_language_cb);
	RETM_IF(ret < 0, "vconf_ignore_key_changed(%s) Fail(%d)", VCONFKEY_LANGSET, ret);

	ctsvc_server_sim_final();
}

int ctsvc_server_init_configuration(void)
{
	int ret;
	char *langset = NULL;
	int sort_type;

	langset = vconf_get_str(VCONFKEY_LANGSET);
	WARN_IF(NULL == langset, "language setting is return NULL");
	ctsvc_set_langset(langset);
	system_language = ctsvc_get_language_type(langset);

	ret = vconf_get_int(ctsvc_get_default_sort_vconfkey(), &sort_type);
	if (ret < 0 || sort_type == CTSVC_SORT_OTHERS) {
		ERR("vconf_get_int(%s) Fail(%d)", ctsvc_get_default_sort_vconfkey(), ret);
		sort_type = ctsvc_get_sort_type_from_language(system_language);
		if (sort_type == CTSVC_SORT_OTHERS)
			sort_type = CTSVC_SORT_WESTERN;
	}
	ctsvc_server_set_default_sort(sort_type);

	ret = vconf_notify_key_changed(VCONFKEY_LANGSET,
			__ctsvc_server_change_language_cb, NULL);
	RETVM_IF(ret < 0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Fail(%d)",
			VCONFKEY_LANGSET, ret);

	ret = ctsvc_server_sim_init();
	RETVM_IF(ret != CONTACTS_ERROR_NONE, ret, "ctsvc_server_sim_init Fail(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

void ctsvc_server_trim_memory(void)
{
	malloc_trim(0);
	sqlite3_release_memory(-1);
}

static gboolean _timeout_cb(gpointer user_data)
{
	CTS_FN_CALL;
	ctsvc_server_quit();
	return TRUE;
}

void ctsvc_server_start_timeout(void)
{
	int timeout = ctsvc_server_get_timeout_sec();
	if (timeout < 1)
		return;

	ctsvc_mutex_lock(CTS_MUTEX_TIMEOUT);
	if (_ctsvc_timeout)
		g_source_remove(_ctsvc_timeout);
	_ctsvc_timeout = g_timeout_add_seconds(timeout, _timeout_cb, NULL);
	ctsvc_mutex_unlock(CTS_MUTEX_TIMEOUT);
}

void ctsvc_server_stop_timeout(void)
{
	int timeout = ctsvc_server_get_timeout_sec();
	if (timeout < 1)
		return;

	ctsvc_mutex_lock(CTS_MUTEX_TIMEOUT);
	if (_ctsvc_timeout)
		g_source_remove(_ctsvc_timeout);
	_ctsvc_timeout = 0;
	ctsvc_mutex_unlock(CTS_MUTEX_TIMEOUT);
}

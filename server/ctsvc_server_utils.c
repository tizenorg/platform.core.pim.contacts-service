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
#include <malloc.h>
#include <vconf.h>
#include <vconf-keys.h>
#include <ITapiPhonebook.h>
#include <TapiUtility.h>

#include "contacts.h"

#include "ctsvc_internal.h"
#include "ctsvc_setting.h"
#include "ctsvc_server_utils.h"
#ifdef ENABLE_SIM_FEATURE
#include "ctsvc_server_sim.h"
#endif // ENABLE_SIM_FEATURE
#include "ctsvc_server_sqlite.h"
#include "ctsvc_localize.h"
#include "ctsvc_normalize.h"

#define CTSVC_FEATURE_TELEPHONY "http://tizen.org/feature/network.telephony"

static int system_language = -1;
static bool _ctsvc_have_telephony_feature = false;

int ctsvc_server_load_feature_list(void)
{
#if 0
	system_info_get_platform_bool(CTSVC_FEATURE_TELEPHONY, &_ctsvc_have_telephony_feature);
#else
	_ctsvc_have_telephony_feature = false;
#endif
	return CONTACTS_ERROR_NONE;
}

bool ctsvc_server_have_telephony_feature(void)
{
	return _ctsvc_have_telephony_feature;
}

inline int ctsvc_server_set_default_sort(int sort)
{
	int ret = vconf_set_int(ctsvc_get_default_sort_vconfkey(), sort);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_set_int() Failed(%d)", ret);
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
	langset = ctsvc_get_langset();
	INFO("%s --> %s", langset, new_langset);
	if (strcmp(langset, new_langset) != 0) {
		bool sort_name_update = false;
		old_primary_sort = ctsvc_get_primary_sort();
		if (old_primary_sort < 0) {
			RETM_IF(ret<0, "ctsvc_get_primary_sort() Fail(%d)", ret);
		}
		old_secondary_sort = ctsvc_get_secondary_sort();
		if (old_secondary_sort < 0) {
			RETM_IF(ret<0, "ctsvc_get_secondary_sort() Fail(%d)", ret);
		}

		if (strncmp(langset, "zh", strlen("zh")) == 0 ||
			strncmp(langset, "ko", strlen("ko")) == 0 ||
			strncmp(langset, "ja", strlen("ja")) == 0 ||
			strncmp(new_langset, "zh", strlen("zh")) == 0 ||
			strncmp(new_langset, "ko", strlen("ko")) == 0 ||
			strncmp(new_langset, "ja", strlen("ja")) == 0) {
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
		}
		else {
			if (new_primary_sort != old_primary_sort)
				ret = ctsvc_server_update_sort(old_primary_sort, old_secondary_sort, new_primary_sort, new_secondary_sort);

			ctsvc_server_update_collation();
		}
	}
}

void ctsvc_server_final_configuration(void)
{
	int ret = -1;

	ret = vconf_ignore_key_changed(VCONFKEY_LANGSET, __ctsvc_server_change_language_cb);
	RETM_IF(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)", VCONFKEY_LANGSET, ret);

#ifdef ENABLE_SIM_FEATURE
	ctsvc_server_sim_final();
#endif // ENABLE_SIM_FEATURE
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
		CTS_ERR("vconf_get_int(%s) Failed(%d)", ctsvc_get_default_sort_vconfkey() ,ret);
		sort_type = ctsvc_get_sort_type_from_language(system_language);
		if (sort_type == CTSVC_SORT_OTHERS)
			sort_type = CTSVC_SORT_WESTERN;
	}
	ctsvc_server_set_default_sort(sort_type);

	ret = vconf_notify_key_changed(VCONFKEY_LANGSET,
			__ctsvc_server_change_language_cb, NULL);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
			VCONFKEY_LANGSET, ret);

#ifdef ENABLE_SIM_FEATURE
	ret = ctsvc_server_sim_init();
	RETVM_IF(ret !=CONTACTS_ERROR_NONE, ret, "ctsvc_server_sim_init Failed(%d)", ret);
#endif // ENABLE_SIM_FEATURE

	return CONTACTS_ERROR_NONE;
}

void ctsvc_server_trim_memory(void)
{
	malloc_trim(0);
	sqlite3_release_memory(-1);
}

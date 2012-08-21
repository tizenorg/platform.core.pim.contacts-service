/*
 * Contacts Service Helper
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjae Shin <yj99.shin@samsung.com>
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
#include <contacts-svc.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "cts-utils.h"
#include "internal.h"
#include "sim.h"
#include "sqlite.h"
#include "normalize.h"
#include "localize.h"
#include "utils.h"

static const char *HELPER_VCONF_TAPI_SIM_PB_INIT = VCONFKEY_TELEPHONY_SIM_PB_INIT;
static const char *HELPER_VCONF_SYSTEM_LANGUAGE = VCONFKEY_LANGSET;
static const char *HELPER_VCONF_DISPLAY_ORDER = VCONFKEY_CONTACTS_SVC_NAME_DISPLAY_ORDER;

static int default_language = -1;
static int system_language = -1;

static inline int helper_get_system_language(void)
{
	return system_language;
}

inline int helper_set_default_language(int lang)
{
	int ret = vconf_set_int(CTS_VCONF_DEFAULT_LANGUAGE, lang);
	h_retvm_if(ret<0, CTS_ERR_VCONF_FAILED, "vconf_set_int() Failed(%d)", ret);

	default_language = lang;
	return CTS_SUCCESS;
}

static void helper_change_language_cb(keynode_t *key, void *data)
{
	int ret = -1;
	const char *langset;

	langset = vconf_keynode_get_str(key);
	if (!default_language) {
		ret = vconf_get_int(CTS_VCONF_DEFAULT_LANGUAGE, &default_language);
		h_retm_if(ret<0, "vconf_get_int() Failed(%d)", ret);
	}

	system_language = helper_get_language_type(langset);
	if (system_language != default_language)
		ret = helper_update_default_language(default_language, system_language);
}

static void helper_update_collation_cb(keynode_t *key, void *data)
{
	helper_update_collation();
}

static void helper_tapi_sim_complete_cb(keynode_t *key, void *data)
{
	int ret, init_stat;
	init_stat = vconf_keynode_get_int(key);
	if (VCONFKEY_TELEPHONY_SIM_PB_INIT_COMPLETED == init_stat) {
		ret = helper_sim_read_SDN(NULL);
		h_warn_if(CTS_SUCCESS != ret, "helper_sim_read_SDN() Failed(%d)", ret);

		vconf_ignore_key_changed(HELPER_VCONF_TAPI_SIM_PB_INIT, helper_tapi_sim_complete_cb);
	}
}

void helper_final_configuration(void)
{
	int ret = -1;

	ret = vconf_ignore_key_changed(HELPER_VCONF_SYSTEM_LANGUAGE, helper_change_language_cb);
	h_retm_if(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)",HELPER_VCONF_SYSTEM_LANGUAGE,ret);

	ret = vconf_ignore_key_changed(VCONFKEY_REGIONFORMAT, helper_update_collation_cb);
	h_retm_if(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)",VCONFKEY_REGIONFORMAT,ret);

	ret = vconf_ignore_key_changed(HELPER_VCONF_DISPLAY_ORDER, helper_update_collation_cb);
	h_retm_if(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)",VCONFKEY_REGIONFORMAT,ret);
}

int helper_init_configuration(void)
{
	int ret, sim_stat=-1;
	const char *langset;

	ret = vconf_get_int(CTS_VCONF_DEFAULT_LANGUAGE, &default_language);
	if (ret < 0) {
		ERR("vconf_get_int(%s) Failed(%d)",CTS_VCONF_DEFAULT_LANGUAGE ,ret);
		default_language = 0;
	}

	ret = vconf_notify_key_changed(HELPER_VCONF_SYSTEM_LANGUAGE,
			helper_change_language_cb, NULL);
	h_retvm_if(ret<0, CTS_ERR_VCONF_FAILED, "vconf_notify_key_changed(%s) Failed(%d)",
			HELPER_VCONF_SYSTEM_LANGUAGE, ret);

	ret = vconf_notify_key_changed(VCONFKEY_REGIONFORMAT,
			helper_update_collation_cb, NULL);
	h_retvm_if(ret<0, CTS_ERR_VCONF_FAILED, "vconf_notify_key_changed(%s) Failed(%d)",
			VCONFKEY_REGIONFORMAT, ret);

	ret = vconf_notify_key_changed(HELPER_VCONF_DISPLAY_ORDER,
			helper_update_collation_cb, NULL);
	h_retvm_if(ret<0, CTS_ERR_VCONF_FAILED, "vconf_notify_key_changed(%s) Failed(%d)",
			HELPER_VCONF_DISPLAY_ORDER, ret);

	langset = vconf_get_str(HELPER_VCONF_SYSTEM_LANGUAGE);
	h_warn_if(NULL == langset, "vconf_get_str(%s) return NULL", HELPER_VCONF_SYSTEM_LANGUAGE);

	system_language = helper_get_language_type(langset);
	if (system_language != default_language) {
		ERR("system lang(%s, %d), default lang(%d)", langset, system_language, default_language);
		helper_update_default_language(default_language, system_language);
	}

	ret = vconf_get_int(HELPER_VCONF_TAPI_SIM_PB_INIT, &sim_stat);
	if (VCONFKEY_TELEPHONY_SIM_PB_INIT_COMPLETED == sim_stat) {
		ret = helper_sim_read_SDN(NULL);
		h_warn_if(CTS_SUCCESS != ret, "helper_sim_read_SDN() Failed(%d)", ret);
	} else {
		ret = vconf_notify_key_changed(HELPER_VCONF_TAPI_SIM_PB_INIT,
				helper_tapi_sim_complete_cb, NULL);
		h_retvm_if(ret<0, CTS_ERR_VCONF_FAILED, "vconf_notify_key_changed(%s) Failed(%d)",
				HELPER_VCONF_TAPI_SIM_PB_INIT, ret);
	}

	return CTS_SUCCESS;
}

void helper_trim_memory(void)
{
	malloc_trim(0);
	sqlite3_release_memory(-1);
}

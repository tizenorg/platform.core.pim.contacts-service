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
#include <malloc.h>
#include <contacts.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "internal.h"
#include "ctsvc_setting.h"
#include "ctsvc_server_utils.h"
#include "ctsvc_server_sim.h"
#include "ctsvc_server_sqlite.h"
#include "ctsvc_localize.h"
#include "ctsvc_normalize.h"

static const char *CTSVC_SERVER_VCONF_TAPI_SIM_PB_INIT = VCONFKEY_TELEPHONY_SIM_PB_INIT;
static const char *CTSVC_SERVER_VCONF_SYSTEM_LANGUAGE = VCONFKEY_LANGSET;


static int default_language = -1;
static int system_language = -1;
static int secondary_language = -1;

inline int ctsvc_server_set_default_language(int lang)
{
	int ret = vconf_set_int(ctsvc_get_default_language_vconfkey(), lang);
	h_retvm_if(ret<0, CONTACTS_ERROR_INTERNAL, "vconf_set_int() Failed(%d)", ret);

	default_language = lang;
	return CONTACTS_ERROR_NONE;
}

inline int ctsvc_server_set_secondary_language(int lang)
{
	int ret = vconf_set_int(ctsvc_get_secondary_language_vconfkey(), lang);
	h_retvm_if(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_set_int() Failed(%d)", ret);

	secondary_language = lang;
	return CONTACTS_ERROR_NONE;
}

static void ctsvc_server_change_language_cb(keynode_t *key, void *data)
{
	int ret = -1;
	const char *langset;

	langset = vconf_keynode_get_str(key);
	if (!default_language) {
		ret = vconf_get_int(ctsvc_get_default_language_vconfkey(), &default_language);
		h_retm_if(ret<0, "vconf_get_int() Failed(%d)", ret);
	}


	system_language = ctsvc_get_language_type(langset);
	DBG("system language(%d, %s)", system_language, langset);
	if (system_language != default_language)
		ret = ctsvc_server_update_default_language(default_language, system_language, secondary_language);
}

static void ctsvc_server_change_secondary_language_cb(keynode_t *key, void *data)
{
	int ret = -1;
	const char *langset;
	int lang_type;

	langset = vconf_keynode_get_str(key);
	if (!secondary_language) {
		ret = vconf_get_int(ctsvc_get_secondary_language_vconfkey(), &secondary_language);
		h_retm_if(ret<0, "vconf_get_int() Failed(%d)", ret);
	}

	lang_type = ctsvc_get_language_type(langset);
	if (lang_type != secondary_language)
		ret = ctsvc_server_update_secondary_language(secondary_language, lang_type);
}

static void ctsvc_server_update_collation_cb(keynode_t *key, void *data)
{
	ctsvc_server_update_collation();
}

static void ctsvc_server_tapi_sim_complete_cb(keynode_t *key, void *data)
{
	int ret, init_stat;
	init_stat = vconf_keynode_get_int(key);
	if (VCONFKEY_TELEPHONY_SIM_PB_INIT_COMPLETED == init_stat) {
		ret = ctsvc_server_sim_read_sdn(NULL);
		h_warn_if(CONTACTS_ERROR_NONE != ret, "ctsvc_server_sim_read_sdn() Failed(%d)", ret);

		ret = ctsvc_server_sim_initialize();
		h_warn_if(CONTACTS_ERROR_NONE != ret, "ctsvc_server_sim_initialize() Failed(%d)", ret);

		vconf_ignore_key_changed(CTSVC_SERVER_VCONF_TAPI_SIM_PB_INIT, ctsvc_server_tapi_sim_complete_cb);
	}
}
void ctsvc_server_final_configuration(void)
{
	int ret = -1;

	ret = vconf_ignore_key_changed(CTSVC_SERVER_VCONF_SYSTEM_LANGUAGE, ctsvc_server_change_language_cb);
	h_retm_if(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)",CTSVC_SERVER_VCONF_SYSTEM_LANGUAGE,ret);

	ret = vconf_ignore_key_changed(VCONFKEY_REGIONFORMAT, ctsvc_server_update_collation_cb);
	h_retm_if(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)",VCONFKEY_REGIONFORMAT,ret);

	ctsvc_server_sim_finalize();
}

int ctsvc_server_init_configuration(void)
{
	int ret, sim_stat=-1;
	const char *langset;
	int lang_type;

	ret = vconf_get_int(ctsvc_get_default_language_vconfkey(), &default_language);
	if (ret < 0) {
		ERR("vconf_get_int(%s) Failed(%d)", ctsvc_get_default_language_vconfkey() ,ret);
		default_language = 0;
	}

	ret = vconf_get_int(ctsvc_get_secondary_language_vconfkey(), &secondary_language);
	if (ret < 0) {
		ERR("vconf_get_int(%s) Failed(%d)", ctsvc_get_secondary_language_vconfkey(),ret);
		secondary_language = 0;
	}

	ret = vconf_notify_key_changed(CTSVC_SERVER_VCONF_SYSTEM_LANGUAGE,
			ctsvc_server_change_language_cb, NULL);
	h_retvm_if(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
			CTSVC_SERVER_VCONF_SYSTEM_LANGUAGE, ret);

	ret = vconf_notify_key_changed(VCONFKEY_CONTACTS_SVC_SECONDARY_LANGUAGE,
			ctsvc_server_change_secondary_language_cb, NULL);
	h_retvm_if(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
			VCONFKEY_CONTACTS_SVC_SECONDARY_LANGUAGE, ret);

	ret = vconf_notify_key_changed(VCONFKEY_REGIONFORMAT,
			ctsvc_server_update_collation_cb, NULL);
	h_retvm_if(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
			VCONFKEY_REGIONFORMAT, ret);

	langset = vconf_get_str(CTSVC_SERVER_VCONF_SYSTEM_LANGUAGE);
	h_warn_if(NULL == langset, "vconf_get_str(%s) return NULL", CTSVC_SERVER_VCONF_SYSTEM_LANGUAGE);
	system_language = ctsvc_get_language_type(langset);

	langset = vconf_get_str(VCONFKEY_CONTACTS_SVC_SECONDARY_LANGUAGE);
	h_warn_if(NULL == langset, "vconf_get_str(%s) return NULL", CTSVC_SERVER_VCONF_SYSTEM_LANGUAGE);
	lang_type = ctsvc_get_language_type(langset);
	if (secondary_language != lang_type) {
		ERR("system lang(%s, %d), default lang(%d)", langset, lang_type, secondary_language);
		ctsvc_server_update_secondary_language(secondary_language, lang_type);
	}

	if (system_language != default_language) {
		ERR("system lang(%s, %d), default lang(%d)", langset, system_language, default_language);
		ctsvc_server_update_default_language(default_language, system_language, secondary_language);
	}

	ret = vconf_get_int(CTSVC_SERVER_VCONF_TAPI_SIM_PB_INIT, &sim_stat);
	if (VCONFKEY_TELEPHONY_SIM_PB_INIT_COMPLETED == sim_stat) {
		ret = ctsvc_server_sim_read_sdn(NULL);
		h_warn_if(CONTACTS_ERROR_NONE != ret, "ctsvc_server_sim_read_sdn() Failed(%d)", ret);

		ret = ctsvc_server_sim_initialize();
		h_warn_if(CONTACTS_ERROR_NONE != ret, "ctsvc_server_sim_initialize() Failed(%d)", ret);
	}
	else {
		ret = vconf_notify_key_changed(CTSVC_SERVER_VCONF_TAPI_SIM_PB_INIT,
				ctsvc_server_tapi_sim_complete_cb, NULL);
		h_retvm_if(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
				CTSVC_SERVER_VCONF_TAPI_SIM_PB_INIT, ret);
	}

	return CONTACTS_ERROR_NONE;
}

void ctsvc_server_trim_memory(void)
{
	malloc_trim(0);
	sqlite3_release_memory(-1);
}

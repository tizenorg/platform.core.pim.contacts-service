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


static int primary_sort = -1;
static int system_language = -1;
static int secondary_sort = -1;

inline int ctsvc_server_set_default_language(int lang)
{
	int ret = vconf_set_int(ctsvc_get_default_language_vconfkey(), lang);
	h_retvm_if(ret<0, CONTACTS_ERROR_INTERNAL, "vconf_set_int() Failed(%d)", ret);

	primary_sort = lang;

	{
		if (primary_sort==CTSVC_SORT_KOREAN)
			secondary_sort = CTSVC_SORT_WESTERN;
		else if (primary_sort==CTSVC_SORT_WESTERN)
			secondary_sort = CTSVC_SORT_KOREAN;
		else
			secondary_sort = CTSVC_SORT_WESTERN;
	}
	DBG("primary %d second %d", primary_sort, secondary_sort);
	return CONTACTS_ERROR_NONE;
}

static void ctsvc_server_change_language_cb(keynode_t *key, void *data)
{
	int ret = -1;
	int new_primary_sort, new_secondary_sort;
	const char *langset;

	langset = vconf_keynode_get_str(key);
	system_language = ctsvc_get_language_type(langset);
	switch(system_language)
	{
	case CTSVC_LANG_KOREAN:
		new_primary_sort = CTSVC_SORT_KOREAN;
		break;
	case CTSVC_LANG_JAPANESE:
		new_primary_sort = CTSVC_SORT_JAPANESE;
		break;
	default:
		new_primary_sort = CTSVC_SORT_WESTERN;
			break;
	}

	if (primary_sort ==-1) {
		ret = vconf_get_int(ctsvc_get_default_language_vconfkey(), &primary_sort);
		h_retm_if(ret<0, "vconf_get_int() Failed(%d)", ret);
	}

	{
		if (new_primary_sort==CTSVC_SORT_KOREAN)
			new_secondary_sort = CTSVC_SORT_WESTERN;
		else if (new_primary_sort==CTSVC_SORT_WESTERN)
			new_secondary_sort = CTSVC_SORT_KOREAN;
		else
			new_secondary_sort = CTSVC_SORT_WESTERN;
	}

	if (new_primary_sort != primary_sort)
		ret = ctsvc_server_update_default_language(primary_sort, secondary_sort, new_primary_sort, new_secondary_sort);
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
	int sort_type;

	langset = vconf_get_str(CTSVC_SERVER_VCONF_SYSTEM_LANGUAGE);
	h_warn_if(NULL == langset, "vconf_get_str(%s) return NULL", CTSVC_SERVER_VCONF_SYSTEM_LANGUAGE);
	system_language = ctsvc_get_language_type(langset);

	ret = vconf_get_int(ctsvc_get_default_language_vconfkey(), &sort_type);
	if (ret < 0 || sort_type == CTSVC_SORT_OTHERS) {
		ERR("vconf_get_int(%s) Failed(%d)", ctsvc_get_default_language_vconfkey() ,ret);

		switch(system_language)
		{
		case CTSVC_LANG_KOREAN:
			sort_type = CTSVC_SORT_KOREAN;
			break;
		case CTSVC_LANG_JAPANESE:
			sort_type = CTSVC_SORT_JAPANESE;
			break;
		default:
			sort_type = CTSVC_SORT_WESTERN;
		}
	}

	ctsvc_server_set_default_language(sort_type);

	ret = vconf_notify_key_changed(CTSVC_SERVER_VCONF_SYSTEM_LANGUAGE,
			ctsvc_server_change_language_cb, NULL);
	h_retvm_if(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
			CTSVC_SERVER_VCONF_SYSTEM_LANGUAGE, ret);

	ret = vconf_notify_key_changed(VCONFKEY_REGIONFORMAT,
			ctsvc_server_update_collation_cb, NULL);
	h_retvm_if(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
			VCONFKEY_REGIONFORMAT, ret);

	ret = vconf_get_int(CTSVC_SERVER_VCONF_TAPI_SIM_PB_INIT, &sim_stat);
	if (VCONFKEY_TELEPHONY_SIM_PB_INIT_COMPLETED == sim_stat) {
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

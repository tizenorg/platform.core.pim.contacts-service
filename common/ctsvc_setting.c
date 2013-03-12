/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Dohyung Jin <dh.jin@samsung.com>
 *                 Jongwon Lee <gogosing.lee@samsung.com>
 *                 Donghee Ye <donghee.ye@samsung.com>
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
#include <vconf.h>
#include <vconf-keys.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_setting.h"
#include "ctsvc_normalize.h"

static int name_display_order = -1;
static int default_lang = -1;
static int secondary_lang = -1;

static const char *CTSVC_VCONF_DISPLAY_ORDER = VCONFKEY_CONTACTS_SVC_NAME_DISPLAY_ORDER;

const char* ctsvc_get_default_language_vconfkey(void)
{
	return "file/private/contacts-service/default_lang";
}

const char* ctsvc_get_secondary_language_vconfkey(void)
{
	return "file/private/contacts-service/secondary_lang";
}

API int contacts_setting_get_name_display_order(contacts_name_display_order_e *order)
{
	int ret;
	if (name_display_order < 0)
	{
		ret = vconf_get_int(VCONFKEY_CONTACTS_SVC_NAME_DISPLAY_ORDER, &name_display_order);
		RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "System : vconf_get_int() Failed(%d)", ret);
	}

	*order = name_display_order;

	return CONTACTS_ERROR_NONE;
}

API int contacts_setting_set_name_display_order(contacts_name_display_order_e order)
{
	int ret;
	RETVM_IF(CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST != order && CONTACTS_NAME_DISPLAY_ORDER_LASTFIRST != order,
			CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : The parameter(order:%d) is Invalid", name_display_order);

	ret = vconf_set_int(VCONFKEY_CONTACTS_SVC_NAME_DISPLAY_ORDER, order);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "System : vconf_set_int(%s) Failed(%d)", VCONFKEY_CONTACTS_SVC_NAME_DISPLAY_ORDER, ret);

	name_display_order = order;

	return CONTACTS_ERROR_NONE;
}

static void ctsvc_vconf_diplay_order_cb(keynode_t *key, void *data)
{
	name_display_order = vconf_keynode_get_int(key);
}

static void ctsvc_vconf_language_cb(keynode_t *key, void *data)
{
	default_lang = vconf_keynode_get_int(key);
}

static void ctsvc_vconf_secondary_language_cb(keynode_t *key, void *data)
{
	secondary_lang = vconf_keynode_get_int(key);
}

int ctsvc_register_vconf(void)
{
	int ret;

	ret = vconf_get_int(CTSVC_VCONF_DISPLAY_ORDER, &name_display_order);
	if (ret < 0) {
		CTS_ERR("vconf_get_int() Failed(%d)", ret);
		name_display_order = CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST;
	}

	ret = vconf_get_int(ctsvc_get_default_language_vconfkey(), &default_lang);
	WARN_IF(ret < 0, "vconf_get_int() Failed(%d)", ret);

	ret = vconf_get_int(ctsvc_get_secondary_language_vconfkey(), &secondary_lang);
	WARN_IF(ret < 0, "vconf_get_int() Failed(%d)", ret);

	ret = vconf_notify_key_changed(CTSVC_VCONF_DISPLAY_ORDER,
			ctsvc_vconf_diplay_order_cb, NULL);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
			CTSVC_VCONF_DISPLAY_ORDER, ret);
	ret = vconf_notify_key_changed(ctsvc_get_default_language_vconfkey(),
			ctsvc_vconf_language_cb, NULL);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
			ctsvc_get_default_language_vconfkey(), ret);

	ret = vconf_notify_key_changed(ctsvc_get_secondary_language_vconfkey(),
			ctsvc_vconf_secondary_language_cb, NULL);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
			ctsvc_get_secondary_language_vconfkey(), ret);

	return CONTACTS_ERROR_NONE;
}

void ctsvc_deregister_vconf(void)
{
	int ret;

	ret = vconf_ignore_key_changed(CTSVC_VCONF_DISPLAY_ORDER, ctsvc_vconf_diplay_order_cb);
	RETM_IF(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)",CTSVC_VCONF_DISPLAY_ORDER,ret);
	ret = vconf_ignore_key_changed(ctsvc_get_default_language_vconfkey(), ctsvc_vconf_language_cb);
	RETM_IF(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)", ctsvc_get_default_language_vconfkey(),ret);
	ret = vconf_ignore_key_changed(ctsvc_get_secondary_language_vconfkey(), ctsvc_vconf_secondary_language_cb);
	RETM_IF(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)", ctsvc_get_secondary_language_vconfkey(),ret);
}

int ctsvc_get_default_language(void)
{
	if (default_lang < 0) {
		int ret;
		ret = vconf_get_int(ctsvc_get_default_language_vconfkey(), &default_lang);
		WARN_IF(ret < 0, "vconf_get_int() Failed(%d)", ret);
	}
	return default_lang;
}

int ctsvc_get_secondary_language(void)
{
	if (secondary_lang < 0) {
		int ret;
		ret = vconf_get_int(ctsvc_get_secondary_language_vconfkey(), &secondary_lang);
		WARN_IF(ret < 0, "vconf_get_int() Failed(%d)", ret);
	}
	return secondary_lang;
}


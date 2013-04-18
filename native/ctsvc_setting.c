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
#include "ctsvc_localize.h"

static int primary_sort = -1;
static int secondary_sort = -1;

static int name_display_order = -1;
static int name_sorting_order = -1;
static int phonenumber_min_match_digit = -1;

static const char *CTSVC_VCONF_DISPLAY_ORDER = VCONFKEY_CONTACTS_SVC_NAME_DISPLAY_ORDER;
static const char *CTSVC_VCONF_SORTING_ORDER = VCONFKEY_CONTACTS_SVC_NAME_SORTING_ORDER;
static const char *CTSVC_VCONF_PHONENUMBER_MIN_MATCH_DIGIT = VCONFKEY_CONTACTS_SVC_PHONENUMBER_MIN_MATCH_DIGIT;

API int contacts_setting_get_name_display_order(contacts_name_display_order_e *order)
{
	int ret;
	if (name_display_order < 0){
		ret = vconf_get_int(CTSVC_VCONF_DISPLAY_ORDER, &name_display_order);
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

	ret = vconf_set_int(CTSVC_VCONF_DISPLAY_ORDER, order);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "System : vconf_set_int(%s) Failed(%d)", CTSVC_VCONF_DISPLAY_ORDER, ret);

	name_display_order = order;

	return CONTACTS_ERROR_NONE;
}

API int contacts_setting_get_name_sorting_order(contacts_name_sorting_order_e *order)
{
	int ret;
	if (name_sorting_order < 0)
	{
		ret = vconf_get_int(CTSVC_VCONF_SORTING_ORDER, &name_sorting_order);
		RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "System : vconf_get_int() Failed(%d)", ret);
	}

	*order = name_sorting_order;

	return CONTACTS_ERROR_NONE;
}

API int contacts_setting_set_name_sorting_order(contacts_name_sorting_order_e order)
{
	int ret;
	RETVM_IF(CONTACTS_NAME_SORTING_ORDER_FIRSTLAST != order && CONTACTS_NAME_SORTING_ORDER_LASTFIRST != order,
			CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : The parameter(order:%d) is Invalid", name_sorting_order);

	ret = vconf_set_int(CTSVC_VCONF_SORTING_ORDER, order);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "System : vconf_set_int(%s) Failed(%d)", CTSVC_VCONF_SORTING_ORDER, ret);

	name_sorting_order = order;

	return CONTACTS_ERROR_NONE;
}

static void ctsvc_vconf_display_order_cb(keynode_t *key, void *data)
{
	name_display_order = vconf_keynode_get_int(key);
}

static void ctsvc_vconf_sorting_order_cb(keynode_t *key, void *data)
{
	name_sorting_order = vconf_keynode_get_int(key);
}

static void ctsvc_vconf_language_cb(keynode_t *key, void *data)
{
	primary_sort = vconf_keynode_get_int(key);

	if (primary_sort==CTSVC_SORT_KOREAN)
		secondary_sort = CTSVC_SORT_WESTERN;
	else if (primary_sort==CTSVC_SORT_WESTERN)
		secondary_sort = CTSVC_SORT_KOREAN;
	else
		secondary_sort = CTSVC_SORT_WESTERN;
}

int ctsvc_register_vconf(void)
{
	int ret;

	// display order
	ret = vconf_get_int(CTSVC_VCONF_DISPLAY_ORDER, &name_display_order);
	if (ret < 0) {
		CTS_ERR("vconf_get_int() Failed(%d)", ret);
		name_display_order = CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST;
	}
	ret = vconf_notify_key_changed(CTSVC_VCONF_DISPLAY_ORDER,
			ctsvc_vconf_display_order_cb, NULL);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
			CTSVC_VCONF_DISPLAY_ORDER, ret);

	// sorting order
	ret = vconf_get_int(CTSVC_VCONF_SORTING_ORDER, &name_sorting_order);
	if (ret < 0) {
		CTS_ERR("vconf_get_int() Failed(%d)", ret);
		name_sorting_order = CONTACTS_NAME_SORTING_ORDER_FIRSTLAST;
	}
	ret = vconf_notify_key_changed(CTSVC_VCONF_SORTING_ORDER,
			ctsvc_vconf_sorting_order_cb, NULL);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
			CTSVC_VCONF_SORTING_ORDER, ret);

	// phonenumber min match digit
	ret = vconf_get_int(CTSVC_VCONF_PHONENUMBER_MIN_MATCH_DIGIT, &phonenumber_min_match_digit);
	if (ret < 0) {
		CTS_ERR("vconf_get_int() Failed(%d)", ret);
		phonenumber_min_match_digit = 8;
	}

	ret = vconf_get_int(ctsvc_get_default_language_vconfkey(), &primary_sort);
	WARN_IF(ret < 0, "vconf_get_int() Failed(%d)", ret);

	if (primary_sort==CTSVC_SORT_KOREAN)
		secondary_sort = CTSVC_SORT_WESTERN;
	else if (primary_sort==CTSVC_SORT_WESTERN)
		secondary_sort = CTSVC_SORT_KOREAN;
	else
		secondary_sort = CTSVC_SORT_WESTERN;

	ret = vconf_notify_key_changed(ctsvc_get_default_language_vconfkey(),
			ctsvc_vconf_language_cb, NULL);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(deafult lang) Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

void ctsvc_deregister_vconf(void)
{
	int ret;

	ret = vconf_ignore_key_changed(CTSVC_VCONF_DISPLAY_ORDER, ctsvc_vconf_display_order_cb);
	RETM_IF(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)",CTSVC_VCONF_DISPLAY_ORDER,ret);
	ret = vconf_ignore_key_changed(CTSVC_VCONF_SORTING_ORDER, ctsvc_vconf_sorting_order_cb);
	RETM_IF(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)",CTSVC_VCONF_SORTING_ORDER,ret);
}

int ctsvc_get_phonenumber_min_match_digit(void)
{
	return phonenumber_min_match_digit;
}

const char* ctsvc_get_default_language_vconfkey(void)
{
	return "file/private/contacts-service/default_lang";
}

int ctsvc_get_default_language(void)
{
	if (primary_sort < 0) {
		int ret;
		ret = vconf_get_int(ctsvc_get_default_language_vconfkey(), &primary_sort);
		WARN_IF(ret < 0, "vconf_get_int() Failed(%d)", ret);

		if (primary_sort==CTSVC_SORT_KOREAN)
			secondary_sort = CTSVC_SORT_WESTERN;
		else if (primary_sort==CTSVC_SORT_WESTERN)
			secondary_sort = CTSVC_SORT_KOREAN;
		else
			secondary_sort = CTSVC_SORT_WESTERN;
	}

	return primary_sort;
}

int ctsvc_get_secondary_language(void)
{
	return secondary_sort;
}


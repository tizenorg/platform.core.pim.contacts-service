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
#include "ctsvc_notify.h"
#include "ctsvc_setting.h"
#include "ctsvc_normalize.h"
#include "ctsvc_localize.h"
#include "ctsvc_db_access_control.h"

#ifdef _CONTACTS_IPC_SERVER
#include "ctsvc_server_change_subject.h"
#endif

#ifdef _CONTACTS_NATIVE
static int __ctsvc_vconf_ref_count = 0;
#endif

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
	RETVM_IF(!ctsvc_have_permission(CTSVC_PERMISSION_CONTACT_READ), CONTACTS_ERROR_PERMISSION_DENIED,
				"Permission denied : contact read");

	if (name_display_order < 0) {
		ret = vconf_get_int(CTSVC_VCONF_DISPLAY_ORDER, &name_display_order);
		RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "System : vconf_get_int() Failed(%d)", ret);
	}

	*order = name_display_order;

	return CONTACTS_ERROR_NONE;
}

API int contacts_setting_set_name_display_order(contacts_name_display_order_e order)
{
	int ret;
	RETVM_IF(!ctsvc_have_permission(CTSVC_PERMISSION_CONTACT_WRITE), CONTACTS_ERROR_PERMISSION_DENIED,
				"Permission denied : contact write");
	RETVM_IF(CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST != order && CONTACTS_NAME_DISPLAY_ORDER_LASTFIRST != order,
			CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : The parameter(order:%d) is Invalid", name_display_order);

	if (order == name_display_order)
		return CONTACTS_ERROR_NONE;

	ret = vconf_set_int(CTSVC_VCONF_DISPLAY_ORDER, order);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "System : vconf_set_int(display order) Failed(%d)", ret);

	name_display_order = order;

	return CONTACTS_ERROR_NONE;
}

API int contacts_setting_get_name_sorting_order(contacts_name_sorting_order_e *order)
{
	int ret;
	RETVM_IF(!ctsvc_have_permission(CTSVC_PERMISSION_CONTACT_READ), CONTACTS_ERROR_PERMISSION_DENIED,
				"Permission denied : contact read");
	if (name_sorting_order < 0)
	{
		ret = vconf_get_int(CTSVC_VCONF_SORTING_ORDER, &name_sorting_order);
		RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "System : vconf_get_int(sort order) Failed(%d)", ret);
	}

	*order = name_sorting_order;

	return CONTACTS_ERROR_NONE;
}

API int contacts_setting_set_name_sorting_order(contacts_name_sorting_order_e order)
{
	int ret;
	RETVM_IF(!ctsvc_have_permission(CTSVC_PERMISSION_CONTACT_WRITE), CONTACTS_ERROR_PERMISSION_DENIED,
				"Permission denied : contact write");
	RETVM_IF(CONTACTS_NAME_SORTING_ORDER_FIRSTLAST != order && CONTACTS_NAME_SORTING_ORDER_LASTFIRST != order,
			CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : The parameter(order:%d) is Invalid", name_sorting_order);

	if (order == name_sorting_order)
		return CONTACTS_ERROR_NONE;

	ret = vconf_set_int(CTSVC_VCONF_SORTING_ORDER, order);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "System : vconf_set_int(sort order) Failed(%d)", ret);

	name_sorting_order = order;

	return CONTACTS_ERROR_NONE;
}

static void ctsvc_vconf_display_order_cb(keynode_t *key, void *data)
{
	name_display_order = vconf_keynode_get_int(key);

#ifdef _CONTACTS_IPC_SERVER
	// publish display order changed
	ctsvc_change_subject_publish_setting(CTSVC_SETTING_DISPLAY_ORDER_CHANGED, name_display_order);
#endif
}

static void ctsvc_vconf_sorting_order_cb(keynode_t *key, void *data)
{
	name_sorting_order = vconf_keynode_get_int(key);

#ifdef _CONTACTS_IPC_SERVER
	// publish sort order changed
	ctsvc_change_subject_publish_setting(CTSVC_SETTING_SORTING_ORDER_CHANGED, name_sorting_order);
#endif
}

void ctsvc_set_sort_memory(int sort_type)
{
	primary_sort = sort_type;
	secondary_sort = CTSVC_SORT_WESTERN;
}

static void ctsvc_vconf_sort_change_cb(keynode_t *key, void *data)
{
	int sort = vconf_keynode_get_int(key);
	ctsvc_set_sort_memory(sort);
}

int ctsvc_register_vconf(void)
{
	int ret;

#ifdef _CONTACTS_NATIVE
	__ctsvc_vconf_ref_count++;
	if (__ctsvc_vconf_ref_count != 1)
		return;
#endif

	// display order
	ret = vconf_get_int(CTSVC_VCONF_DISPLAY_ORDER, &name_display_order);
	if (ret < 0) {
		CTS_ERR("vconf_get_int() Failed(%d)", ret);
		name_display_order = CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST;
	}
	ret = vconf_notify_key_changed(CTSVC_VCONF_DISPLAY_ORDER,
			ctsvc_vconf_display_order_cb, NULL);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(display order) Failed(%d)", ret);

	// sorting order
	ret = vconf_get_int(CTSVC_VCONF_SORTING_ORDER, &name_sorting_order);
	if (ret < 0) {
		CTS_ERR("vconf_get_int() Failed(%d)", ret);
		name_sorting_order = CONTACTS_NAME_SORTING_ORDER_FIRSTLAST;
	}
	ret = vconf_notify_key_changed(CTSVC_VCONF_SORTING_ORDER,
			ctsvc_vconf_sorting_order_cb, NULL);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(sort order) Failed(%d)", ret);

	// phonenumber min match digit
	ret = vconf_get_int(CTSVC_VCONF_PHONENUMBER_MIN_MATCH_DIGIT, &phonenumber_min_match_digit);
	if (ret < 0) {
		CTS_ERR("vconf_get_int() Failed(%d)", ret);
		phonenumber_min_match_digit = 8;
	}

	ret = vconf_get_int(ctsvc_get_default_sort_vconfkey(), &primary_sort);
	WARN_IF(ret < 0, "vconf_get_int() Failed(%d)", ret);
	ctsvc_set_sort_memory(primary_sort);

	ret = vconf_notify_key_changed(ctsvc_get_default_sort_vconfkey(),
			ctsvc_vconf_sort_change_cb, NULL);
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(deafult lang) Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

void ctsvc_deregister_vconf(void)
{
	int ret;

#ifdef _CONTACTS_NATIVE
	__ctsvc_vconf_ref_count--;
	if (__ctsvc_vconf_ref_count != 0)
		return;
#endif

	ret = vconf_ignore_key_changed(CTSVC_VCONF_DISPLAY_ORDER, ctsvc_vconf_display_order_cb);
	RETM_IF(ret<0,"vconf_ignore_key_changed(display order) Failed(%d)", ret);
	ret = vconf_ignore_key_changed(CTSVC_VCONF_SORTING_ORDER, ctsvc_vconf_sorting_order_cb);
	RETM_IF(ret<0,"vconf_ignore_key_changed(sort order) Failed(%d)", ret);

	ret = vconf_ignore_key_changed(ctsvc_get_default_sort_vconfkey(), ctsvc_vconf_sort_change_cb);
	RETM_IF(ret<0,"vconf_ignore_key_changed(default_lang) Failed(%d)", ret);
}

int ctsvc_get_phonenumber_min_match_digit(void)
{
	return phonenumber_min_match_digit;
}

const char* ctsvc_get_default_sort_vconfkey(void)
{
	return "file/private/contacts-service/default_lang";
}

int ctsvc_get_primary_sort(void)
{
	if (primary_sort < 0) {
		int ret;
		ret = vconf_get_int(ctsvc_get_default_sort_vconfkey(), &primary_sort);
		WARN_IF(ret < 0, "vconf_get_int() Failed(%d)", ret);
		ctsvc_set_sort_memory(primary_sort);
	}
	return primary_sort;
}

int ctsvc_get_secondary_sort(void)
{
	return secondary_sort;
}

#ifdef _CONTACTS_NATIVE
API int contacts_setting_add_name_display_order_changed_cb(
	contacts_setting_name_display_order_changed_cb cb, void* user_data)
{
	CTS_ERR("Please use contacts-service2 instead of contacts-service3");
	return CONTACTS_ERROR_INTERNAL;
}

API int contacts_setting_remove_name_display_order_changed_cb(
	contacts_setting_name_display_order_changed_cb cb, void* user_data)
{
	CTS_ERR("Please use contacts-service2 instead of contacts-service3");
	return CONTACTS_ERROR_INTERNAL;

}

API int contacts_setting_add_name_sorting_order_changed_cb(
	contacts_setting_name_sorting_order_changed_cb cb, void* user_data)
{
	CTS_ERR("Please use contacts-service2 instead of contacts-service3");
	return CONTACTS_ERROR_INTERNAL;
}


API int contacts_setting_remove_name_sorting_order_changed_cb(
	contacts_setting_name_sorting_order_changed_cb cb, void* user_data)
{
	CTS_ERR("Please use contacts-service2 instead of contacts-service3");
	return CONTACTS_ERROR_INTERNAL;
}

#endif

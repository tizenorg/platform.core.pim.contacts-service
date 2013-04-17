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
#include "ctsvc_common_setting.h"
#include "ctsvc_normalize.h"
#include "ctsvc_localize.h"

static int primary_sort = -1;
static int secondary_sort = -1;

const char* ctsvc_get_default_language_vconfkey(void)
{
	return "file/private/contacts-service/default_lang";
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

int ctsvc_register_common_vconf(void)
{
	int ret;

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
	RETVM_IF(ret<0, CONTACTS_ERROR_SYSTEM, "vconf_notify_key_changed(%s) Failed(%d)",
			ctsvc_get_default_language_vconfkey(), ret);

	return CONTACTS_ERROR_NONE;
}

void ctsvc_deregister_common_vconf(void)
{
	int ret;

	ret = vconf_ignore_key_changed(ctsvc_get_default_language_vconfkey(), ctsvc_vconf_language_cb);
	RETM_IF(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)", ctsvc_get_default_language_vconfkey(),ret);
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


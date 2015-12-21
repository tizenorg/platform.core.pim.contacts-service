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

#include "ctsvc_internal.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_view.h"

static int __ctsvc_ipc_unmarshal_my_profile(pims_ipc_data_h ipc_data, const char *view_uri, contacts_record_h record);
static int __ctsvc_ipc_marshal_my_profile(const contacts_record_h record, pims_ipc_data_h ipc_data);

ctsvc_ipc_marshal_record_plugin_cb_s _ctsvc_ipc_record_my_profile_plugin_cb = {
	.unmarshal_record = __ctsvc_ipc_unmarshal_my_profile,
	.marshal_record = __ctsvc_ipc_marshal_my_profile
};

static int __ctsvc_ipc_unmarshal_my_profile(pims_ipc_data_h ipc_data, const char *view_uri, contacts_record_h record)
{
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_NO_DATA);
	RETV_IF(NULL == record, CONTACTS_ERROR_NO_DATA);

	ctsvc_my_profile_s *pmy_profile = (ctsvc_my_profile_s*) record;

	do {
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pmy_profile->id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pmy_profile->changed_time) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_int(ipc_data, &pmy_profile->addressbook_id) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pmy_profile->display_name) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pmy_profile->reverse_display_name) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pmy_profile->uid) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_string(ipc_data, &pmy_profile->image_thumbnail_path) != CONTACTS_ERROR_NONE) break;

		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->name) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->note) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->company) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->numbers) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->emails) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->events) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->messengers) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->postal_addrs) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->urls) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->nicknames) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->profiles) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->relationships) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->images) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_unmarshal_child_list(ipc_data, (contacts_list_h*)&pmy_profile->extensions) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while (0);

	CTS_ERR("__ctsvc_ipc_unmarshal_my_profile() Fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static int __ctsvc_ipc_marshal_my_profile(const contacts_record_h record, pims_ipc_data_h ipc_data)
{
	ctsvc_my_profile_s *pcontact = (ctsvc_my_profile_s*)record;
	RETV_IF(NULL == ipc_data, CONTACTS_ERROR_NO_DATA);
	RETV_IF(NULL == pcontact, CONTACTS_ERROR_NO_DATA);

	do {
		if (ctsvc_ipc_marshal_int((pcontact->id), ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((pcontact->changed_time), ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_int((pcontact->addressbook_id), ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->display_name), ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->reverse_display_name), ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->uid), ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_string((pcontact->image_thumbnail_path), ipc_data) != CONTACTS_ERROR_NONE) break;

		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->name, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->note, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->company, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->numbers, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->emails, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->events, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->messengers, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->postal_addrs, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->urls, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->nicknames, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->profiles, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->relationships, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->images, ipc_data) != CONTACTS_ERROR_NONE) break;
		if (ctsvc_ipc_marshal_list((contacts_list_h)pcontact->extensions, ipc_data) != CONTACTS_ERROR_NONE) break;

		return CONTACTS_ERROR_NONE;
	} while (0);

	CTS_ERR("_ctsvc_ipc_marshal() Fail");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}


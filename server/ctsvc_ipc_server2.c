/*
 * Contacts Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <stdlib.h>

#include "contacts.h"
#ifdef ENABLE_LOG_FEATURE
#include "contacts_phone_log_internal.h"
#endif /* ENABLE_LOG_FEATURE */

#include "ctsvc_server_service.h"
#include "ctsvc_db_init.h"

#include "ctsvc_server_activity.h"
#include "ctsvc_server_person.h"
#include "ctsvc_server_group.h"
#include "ctsvc_server_phonelog.h"
#include "ctsvc_server_setting.h"

#include "ctsvc_db_access_control.h"

#include "ctsvc_ipc_marshal.h"
#include "ctsvc_internal.h"
#include "ctsvc_ipc_server.h"
#include "ctsvc_db_utils.h"
#include "ctsvc_handle.h"
#include "ctsvc_server_utils.h"

void ctsvc_ipc_activity_delete_by_contact_id(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int contact_id = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &contact_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_server_db_insert_record() Fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}
	ret = ctsvc_activity_delete_by_contact_id(contact_id);


ERROR_RETURN:

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}

		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
			}
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_activity_delete_by_account_id(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int account_id = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &account_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_activity_delete_by_account_id() Fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}
	ret = ctsvc_activity_delete_by_account_id(account_id);

ERROR_RETURN:

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
			}
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_group_add_contact(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int group_id = 0;
	int contact_id = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &group_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &contact_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_group_add_contact() Fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = ctsvc_group_add_contact(group_id, contact_id);

ERROR_RETURN:

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}

		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
			}
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_group_remove_contact(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int group_id = 0;
	int contact_id = 0;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &group_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &contact_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_group_remove_contact() Fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}


	ret = ctsvc_group_remove_contact(group_id, contact_id);

ERROR_RETURN:

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
			}
		}

	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_group_set_group_order(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int group_id = 0;
	int previous_group_id;
	int next_group_id;

	contacts_h contact = NULL;
	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &group_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &previous_group_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &next_group_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_group_link_group() Fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = ctsvc_group_set_group_order(group_id, previous_group_id, next_group_id);

ERROR_RETURN:

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
			}
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_person_link_person(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int base_person_id = 0;
	int person_id = 0;

	contacts_h contact = NULL;
	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &base_person_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &person_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_person_link_person() Fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = ctsvc_person_link_person(base_person_id, person_id);

ERROR_RETURN:

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
			}
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}
void ctsvc_ipc_person_unlink_contact(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int person_id = 0;
	int contact_id = 0;

	contacts_h contact = NULL;
	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &person_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &contact_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_person_link_person() Fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	int unlinked_person_id;
	ret = ctsvc_person_unlink_contact(person_id, contact_id, &unlinked_person_id);

ERROR_RETURN:

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
			}
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(unlinked_person_id, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}

	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}
void ctsvc_ipc_person_reset_usage(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int person_id = 0;
	contacts_usage_type_e type;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &person_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
		int temp = 0;
		ret = ctsvc_ipc_unmarshal_int(indata, &temp);
		type = (int)temp;
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_person_link_person() Fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = ctsvc_person_reset_usage(person_id, type);

ERROR_RETURN:

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
			}
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}
void ctsvc_ipc_person_set_favorite_order(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int person_id = 0;
	int previous_person_id;
	int next_person_id;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &person_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &previous_person_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &next_person_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_person_link_person() Fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = ctsvc_person_set_favorite_order(person_id, previous_person_id, next_person_id);

ERROR_RETURN:

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
			}
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_person_set_default_property(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int person_id = 0;
	int id;
	contacts_person_property_e property;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &person_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_unsigned_int(indata, &property);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_person_set_default_property() Fail");
		goto ERROR_RETURN;
	}

	ret = ctsvc_person_set_default_property(property, person_id, id);

ERROR_RETURN:

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
			}
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_person_get_default_property(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int person_id = 0;
	int id;
	contacts_person_property_e op;
	contacts_h contact = NULL;
	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, &person_id);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_unsigned_int(indata, &op);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_person_get_default_property() Fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_READ)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = ctsvc_person_get_default_property(op, person_id, &id);

ERROR_RETURN:

	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail (return value)");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(id, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail (id)");
			goto DATA_FREE;
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}

#ifdef ENABLE_LOG_FEATURE
void ctsvc_ipc_phone_log_reset_statistics(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_PHONELOG_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = ctsvc_phone_log_reset_statistics();

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
			}
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_phone_log_delete(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret= CONTACTS_ERROR_NONE;
	int extra_data1;
	char *number = NULL;
	contacts_phone_log_delete_e op;
	contacts_h contact = NULL;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_handle(indata, &contact);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_ipc_unmarshal_handle Fail(%d)", ret);
			goto ERROR_RETURN;
		}
		ret = ctsvc_ipc_unmarshal_int(indata, (int*)&op);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_PHONELOG_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

		}

		switch(op) {
		case CONTACTS_PHONE_LOG_DELETE_BY_ADDRESS:
			ret = ctsvc_ipc_unmarshal_string(indata, &number);
			if (ret != CONTACTS_ERROR_NONE) {
				CTS_ERR("ctsvc_ipc_unmarshal_string() Fail");
				goto ERROR_RETURN;
			}
			ret = ctsvc_phone_log_delete(op, number);
			break;
		case CONTACTS_PHONE_LOG_DELETE_BY_MESSAGE_EXTRA_DATA1:
		case CONTACTS_PHONE_LOG_DELETE_BY_EMAIL_EXTRA_DATA1:
			ret = ctsvc_ipc_unmarshal_int(indata, &extra_data1);
			if (ret != CONTACTS_ERROR_NONE) {
				CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
				goto ERROR_RETURN;
			}
			ret = ctsvc_phone_log_delete(op, extra_data1);
			break;
		default:
			CTS_ERR("Invalid parameter : the operation is not proper (op : %d)", op);
			ret = CONTACTS_ERROR_INVALID_PARAMETER;
			break;
		}
	}

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			goto DATA_FREE;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			goto DATA_FREE;
		}
		if (ret == CONTACTS_ERROR_NONE) {
			int transaction_ver = ctsvc_get_transaction_ver();
			if (ctsvc_ipc_marshal_int(transaction_ver, *outdata) != CONTACTS_ERROR_NONE) {
				pims_ipc_data_destroy(*outdata);
				*outdata = NULL;
				CTS_ERR("ctsvc_ipc_marshal_int() Fail");
				goto DATA_FREE;
			}
		}
	}
DATA_FREE:
	ctsvc_handle_destroy(contact);
	free(number);

	ctsvc_server_start_timeout();
	return;
}
#endif /* ENABLE_LOG_FEATURE */

void ctsvc_ipc_setting_get_name_display_order(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_name_display_order_e order;

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_READ)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = ctsvc_setting_get_name_display_order(&order);

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			ctsvc_server_start_timeout();
			return;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail (return value)");
			ctsvc_server_start_timeout();
			return;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(order, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail (id)");
			ctsvc_server_start_timeout();
			return;
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}

	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_setting_get_name_sorting_order(pims_ipc_h ipc, pims_ipc_data_h indata,
		pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	contacts_name_sorting_order_e order;

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_READ)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = ctsvc_setting_get_name_sorting_order(&order);

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			ctsvc_server_start_timeout();
			return;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail (return value)");
			ctsvc_server_start_timeout();
			return;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(order, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail (id)");
			return;
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}

	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_setting_set_name_display_order(pims_ipc_h ipc,
			pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int order;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_int(indata, &order);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_person_set_default_property() Fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = ctsvc_setting_set_name_display_order((contacts_name_display_order_e)order);

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			ctsvc_server_start_timeout();
			return;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			ctsvc_server_start_timeout();
			return;
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}

	ctsvc_server_start_timeout();
	return;
}

void ctsvc_ipc_setting_set_name_sorting_order(pims_ipc_h ipc,
			pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata)
{
	int ret = CONTACTS_ERROR_NONE;
	int order;

	if (indata) {
		ret = ctsvc_ipc_unmarshal_int(indata, &order);
		if (ret != CONTACTS_ERROR_NONE) {
			CTS_ERR("ctsvc_ipc_unmarshal_int() Fail");
			goto ERROR_RETURN;
		}
	}
	else {
		CTS_ERR("ctsvc_ipc_person_set_default_property() Fail");
		goto ERROR_RETURN;
	}

	if (!ctsvc_have_permission(ipc, CTSVC_PERMISSION_CONTACT_WRITE)) {
		ret = CONTACTS_ERROR_PERMISSION_DENIED;
		goto ERROR_RETURN;
	}

	ret = ctsvc_setting_set_name_sorting_order((contacts_name_sorting_order_e)order);

ERROR_RETURN:
	if (outdata) {
		*outdata = pims_ipc_data_create(0);
		if (NULL == *outdata) {
			CTS_ERR("pims_ipc_data_create() Fail");
			ctsvc_server_start_timeout();
			return;
		}
		if (CONTACTS_ERROR_NONE != ctsvc_ipc_marshal_int(ret, *outdata)) {
			pims_ipc_data_destroy(*outdata);
			*outdata = NULL;
			CTS_ERR("ctsvc_ipc_marshal_int() Fail");
			ctsvc_server_start_timeout();
			return;
		}
	}
	else {
		CTS_ERR("outdata is NULL");
	}

	ctsvc_server_start_timeout();
	return;
}


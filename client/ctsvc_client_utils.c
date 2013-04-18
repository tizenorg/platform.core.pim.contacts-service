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

#include <glib.h>
#include <pims-ipc-data.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_ipc_marshal.h"
#include "ctsvc_client_ipc.h"

API int contacts_utils_get_index_characters(char **index_string)
{
	int ret = CONTACTS_ERROR_NONE;
	pims_ipc_data_h outdata = NULL;

	RETVM_IF(index_string == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "The out param is NULL");
	*index_string = NULL;
	RETVM_IF(ctsvc_get_ipc_handle() == NULL,CONTACTS_ERROR_IPC, "contacts not connected");

	if (ctsvc_ipc_call(CTSVC_IPC_UTILS_MODULE, CTSVC_IPC_SERVER_UTILS_GET_INDEX_CHARACTERS, NULL, &outdata) != 0) {
		CTS_ERR("ctsvc_ipc_call failed");
		return CONTACTS_ERROR_IPC;
	}

	if (outdata) {
		unsigned int size = 0;
		ret = *(int*) pims_ipc_data_get(outdata, &size);
		if (CONTACTS_ERROR_NONE == ret) {
			ret = ctsvc_ipc_unmarshal_string(outdata, index_string);
			if (CONTACTS_ERROR_NONE != ret) {
				CTS_ERR("ctsvc_ipc_unmarshal_string Fail9(%d)", ret);
			}
		}
		pims_ipc_data_destroy(outdata);
	}

	return ret;
}


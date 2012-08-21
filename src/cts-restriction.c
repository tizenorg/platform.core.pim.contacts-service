/*
 * Contacts Service
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
#include <fcntl.h>
#include <unistd.h>

#include "cts-internal.h"
#include "cts-sqlite.h"
#include "cts-schema.h"

static const char *CTS_RESTRICTION_CHECK_FILE="/opt/data/contacts-svc/.CONTACTS_SVC_RESTRICTION_CHECK";
static int cts_restriction_permit;

int cts_restriction_init(void)
{
	if (!cts_restriction_permit) {
		int fd = open(CTS_RESTRICTION_CHECK_FILE, O_RDONLY);
		if (0 <= fd) {
			close(fd);
			cts_restriction_permit = TRUE;
		} else {
			ERR("Restriction Mode");
		}
	}
	if (!cts_restriction_permit) {
		int ret;
		const char *query;
		query = "CREATE TEMP VIEW "CTS_TABLE_RESTRICTED_DATA_VIEW" AS SELECT * FROM "CTS_TABLE_DATA" WHERE is_restricted != 1";

		ret = cts_query_exec(query);
		retvm_if(CTS_SUCCESS != ret, ret, "cts_query_exec() Failed(%d)", ret);
	}
	return CTS_SUCCESS;
}

void cts_restriction_final(void)
{
	cts_restriction_permit = FALSE;
}

int cts_restriction_get_permit(void)
{
	return cts_restriction_permit;
}

/**
 * This function make restricted contact.
 * If process does not have permission for restriction, this function will be failed.
 *
 * @param[in] contact The contacts service struct
 * @return	#CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
API int contacts_svc_struct_set_restriction(CTSstruct *contact)
{
	contact_t *record = (contact_t *)contact;

	retv_if(NULL == contact, CTS_ERR_ARG_NULL);
	retv_if(FALSE == cts_restriction_permit, CTS_ERR_ENV_INVALID);

	record->is_restricted = TRUE;

	return CTS_SUCCESS;
}


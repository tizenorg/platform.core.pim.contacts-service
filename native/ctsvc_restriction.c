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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_schema.h"

static const char *CTS_RESTRICTION_CHECK_FILE="/opt/usr/data/contacts-svc/.CONTACTS_SVC_RESTRICTION_CHECK";
static TLS int ctsvc_restriction_permit;

int ctsvc_restriction_init(void)
{
	if (!ctsvc_restriction_permit) {
		int fd = open(CTS_RESTRICTION_CHECK_FILE, O_RDONLY);
		if (0 <= fd) {
			close(fd);
			ctsvc_restriction_permit = TRUE;
		} else {
			CTS_ERR("Restriction Mode");
		}
	}
	if (!ctsvc_restriction_permit) {
		int ret;
		const char *query;
		query = "CREATE TEMP VIEW "CTS_TABLE_RESTRICTED_DATA_VIEW" AS SELECT * FROM "CTS_TABLE_DATA" WHERE is_restricted != 1 AND is_my_profile = 0";

		ret = ctsvc_query_exec(query);
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "cts_query_exec() Failed(%d)", ret);
	}
	return CONTACTS_ERROR_NONE;
}

void ctsvc_restriction_deinit(void)
{
	ctsvc_restriction_permit = FALSE;
}

int ctsvc_restriction_get_permit(void)
{
	return ctsvc_restriction_permit;
}
#if 0
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

	RETV_IF(NULL == contact, CTS_ERR_ARG_NULL);
	RETV_IF(FALSE == ctsvc_restriction_permit, CTS_ERR_ENV_INVALID);

	record->is_restricted = TRUE;

	return CONTACTS_ERROR_NONE;
}
#endif

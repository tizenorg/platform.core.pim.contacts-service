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
#include <ctype.h>
#include <unistd.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_db_schema.h"
#include "ctsvc_db_sqlite.h"
#include "ctsvc_db_utils.h"
#include "ctsvc_db_plugin_group_helper.h"
#include "ctsvc_notify.h"
#include "ctsvc_normalize.h"
#include "ctsvc_server_setting.h"
#include "ctsvc_localize.h"

/*
 * Whenever deleting group, this function will be called
 * in order to deleting group image file
 */
void ctsvc_db_group_delete_callback(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	int ret;
	const unsigned char *path;

	if (1 < argc) {
		sqlite3_result_null(context);
		return;
	}
	path = sqlite3_value_text(argv[0]);

	if (path) {
		char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_GROUP_IMAGE_LOCATION, path);
		ret = unlink(full_path);
		if (ret < 0)
			WARN("unlink() Fail(%d)", errno);
	}

	return;
}

static int _ctsvc_db_group_name_collation_str(const char *str1, const char *str2)
{
	int ret;
	int len1, len2;
	char *str_dest1 = NULL;
	char *str_dest2 = NULL;

	ctsvc_collation_str((char *)str1, &str_dest1);
	ctsvc_collation_str((char *)str2, &str_dest2);

	len1 = strlen(str_dest1);
	len2 = strlen(str_dest2);
	if (len1 < len2)
		ret = strncmp(str_dest1, str_dest2, len1);
	else
		ret = strncmp(str_dest1, str_dest2, len2);

	free(str_dest1);
	free(str_dest2);
	return ret;
}

int ctsvc_db_group_name_sort_callback(void *context, int str1_len, const void *str1, int str2_len, const void *str2)
{
	int ret;
	int str1_sort_type;
	int str2_sort_type;
	char str_src1[CTSVC_STR_SHORT_LEN] = {0};
	char str_src2[CTSVC_STR_SHORT_LEN] = {0};
	int prim_sort  = ctsvc_get_primary_sort();

	strncpy(str_src1, str1, str1_len);
	strncpy(str_src2, str2, str2_len);
	str1_sort_type = ctsvc_get_name_sort_type(str_src1);
	str2_sort_type = ctsvc_get_name_sort_type(str_src2);

	switch (str1_sort_type) {
	case CTSVC_SORT_NUMBER:
		if (CTSVC_SORT_OTHERS == str2_sort_type)
			ret = 1;
		else if (CTSVC_SORT_NUMBER == str2_sort_type)
			ret = _ctsvc_db_group_name_collation_str(str_src1, str_src2);
		else
			ret = -1;
		break;

	case CTSVC_SORT_OTHERS:
		if (CTSVC_SORT_OTHERS == str2_sort_type)
			ret = _ctsvc_db_group_name_collation_str(str_src1, str_src2);
		else
			ret = -1;
		break;

	default:
		if (CTSVC_SORT_NUMBER >= str2_sort_type) {
			ret = 1;
		} else {
			if (str1_sort_type != str2_sort_type) {
				if (str1_sort_type == prim_sort)
					ret = -1;
				else if (str2_sort_type == prim_sort)
					ret = 1;
				else
					ret = _ctsvc_db_group_name_collation_str(str_src1, str_src2);
			} else {
				ret = _ctsvc_db_group_name_collation_str(str_src1, str_src2);
			}
		}
		break;
	}

	return ret;
}

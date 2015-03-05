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

#include <ctype.h>
#include <unistd.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_utils.h"
#include "ctsvc_db_plugin_group_helper.h"

// Whenever deleting group, this function will be called
// in order to deleting group image file
void ctsvc_db_group_delete_callback(sqlite3_context *context, int argc, sqlite3_value ** argv)
{
	int ret;
	const unsigned char* path;

	if (argc > 1) {
		sqlite3_result_null(context);
		return;
	}
	path = sqlite3_value_text(argv[0]);

	if (path) {
		char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_GROUP_IMAGE_LOCATION, path);
		ret = unlink(full_path);
		if (ret < 0) {
			CTS_WARN("unlink Failed(%d)", errno);
		}
	}

	return;
}



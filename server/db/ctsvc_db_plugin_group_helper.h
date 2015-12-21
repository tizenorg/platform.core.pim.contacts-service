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

#ifndef __CTSVC_DB_PLUGIN_GROUP_HELPER_H__
#define __CTSVC_DB_PLUGIN_GROUP_HELPER_H__

#include "contacts.h"
#include "ctsvc_db_sqlite.h"
#include <tzplatform_config.h>

void ctsvc_db_group_delete_callback(sqlite3_context *context, int argc, sqlite3_value **argv);
int ctsvc_db_group_name_sort_callback(void *context, int str1_len, const void *str1, int str2_len, const void *str2);

#endif /* __CTSVC_DB_PLUGIN_GROUP_HELPER_H__ */

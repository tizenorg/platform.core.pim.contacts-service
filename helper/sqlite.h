/*
 * Contacts Service Helper
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
#ifndef __CTS_HELPER_SQLITE_H__
#define __CTS_HELPER_SQLITE_H__

#include <sqlite3.h>

int helper_db_open(sqlite3 **db);
int helper_db_close(void);
int helper_update_default_language(int system_lang, int default_lang);
int helper_insert_SDN_contact(const char *name, const char *number);
int helper_delete_SDN_contact(void);
int helper_update_collation();

#endif // __CTS_HELPER_SQLITE_H__



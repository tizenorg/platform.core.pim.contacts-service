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
#ifndef __CTSVC_SERVER_SQLITE_H__
#define __CTSVC_SERVER_SQLITE_H__

#include <sqlite3.h>

int ctsvc_server_db_open(sqlite3 **db);
int ctsvc_server_db_close(void);
int ctsvc_server_update_sort(int prev_sort_primary, int prev_sort_secondary, int new_sort_primary, int new_sort_secondary);
int ctsvc_server_insert_sdn_contact(const char *name, const char *number, int sim_slot_no);
int ctsvc_server_delete_sdn_contact(int sim_slot_no);
int ctsvc_server_update_collation();
int ctsvc_server_update_sort_name();

int ctsvc_server_get_sim_id(const char *unique_id, int *sim_id);

#endif /* __CTSVC_SERVER_SQLITE_H__ */



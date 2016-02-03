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

#ifndef __CTSVC_SERVER_PHONELOG_H__
#define __CTSVC_SERVER_PHONELOG_H__

#include "ctsvc_db_sqlite.h"
#include "contacts_phone_log_internal.h"

int ctsvc_phone_log_reset_statistics();
int ctsvc_phone_log_reset_statistics_by_sim(int sim_slot_no);
int ctsvc_phone_log_delete(contacts_phone_log_delete_e op, ...);
void ctsvc_db_phone_log_delete_callback(sqlite3_context  *context,
		int argc, sqlite3_value **argv);
int ctsvc_db_phone_log_update_person_id(const char *number, int old_person_id, int candidate_person_id, bool person_link);

#endif /* __CTSVC_SERVER_PHONELOG_H__ */

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

#ifndef __CTSVC_SERVER_PERSON_H__
#define __CTSVC_SERVER_PERSON_H__

#include "ctsvc_db_sqlite.h"

int ctsvc_person_do_garbage_collection(void);
int ctsvc_person_aggregate(int person_id);
void ctsvc_db_person_delete_callback(sqlite3_context  *context, int argc, sqlite3_value **argv);
int ctsvc_person_link_person(int base_person_id, int person_id);
int ctsvc_person_unlink_contact(int person_id, int contact_id, int *out_person_id );
int ctsvc_person_reset_usage(int person_id, contacts_usage_type_e type);
int ctsvc_person_set_favorite_order(int person_id, int front_person_id, int back_person_id);
int ctsvc_person_set_default_property(contacts_person_property_e property, int person_id, int id);
int ctsvc_person_get_default_property(contacts_person_property_e property, int person_id, int *id);
int ctsvc_person_get_aggregation_suggestions(int person_id, contacts_list_h *out_list );
#endif /* __CTSVC_SERVER_PERSON_H__ */

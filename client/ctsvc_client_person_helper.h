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

#ifndef __CTSVC_CLIENT_PERSON_HELPER_H__
#define __CTSVC_CLIENT_PERSON_HELPER_H__

#include "contacts_types.h"

int ctsvc_client_person_link_person(contacts_h contact, int base_person_id, int person_id);
int ctsvc_client_person_unlink_contact(contacts_h contact, int person_id, int contact_id, int *unlinked_person_id);
int ctsvc_client_person_reset_usage(contacts_h contact, int person_id, contacts_usage_type_e type);
int ctsvc_client_person_set_favorite_order(contacts_h contact, int person_id, int previous_person_id, int next_person_id);
int ctsvc_client_person_set_default_property(contacts_h contact, contacts_person_property_e property, int person_id, int id);
int ctsvc_client_person_get_default_property(contacts_h contact, contacts_person_property_e property, int person_id, int *id);
int ctsvc_client_person_search_aggregable(contacts_h contact, int person_id, contacts_list_h *out_list);
#endif /* __CTSVC_CLIENT_PERSON_HELPER_H__ */
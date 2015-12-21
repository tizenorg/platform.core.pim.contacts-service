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

#ifndef __CTSVC_SERVER_GROUP_H__
#define __CTSVC_SERVER_GROUP_H__

int ctsvc_group_add_contact_in_transaction(int group_id, int contact_id);
int ctsvc_group_remove_contact_in_transaction(int group_id, int contact_id);
int ctsvc_group_add_contact(int group_id, int contact_id);
int ctsvc_group_remove_contact(int group_id, int contact_id);
int ctsvc_group_set_group_order(int group_id, int previous_group_id, int next_group_id);


/*
int ctsvc_group_add_contacts_in_person(int group_id, int person_id);
int ctsvc_group_remove_contacts_in_person(int group_id, int person_id);
*/

#endif /* __CTSVC_SERVER_GROUP_H__ */

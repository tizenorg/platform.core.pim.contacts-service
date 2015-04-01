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

#ifndef __TIZEN_SOCIAL_CTSVC_DB_ACCESS_CONTROL_H__
#define __TIZEN_SOCIAL_CTSVC_DB_ACCESS_CONTROL_H__

#define CTSVC_PRIVILEGE_CALLHISTORY_READ "http://tizen.org/privilege/callhistory.read"
#define CTSVC_PRIVILEGE_CALLHISTORY_WRITE "http://tizen.org/privilege/callhistory.write"
#define CTSVC_PRIVILEGE_CONTACT_READ "http://tizen.org/privilege/contact.read"
#define CTSVC_PRIVILEGE_CONTACT_WRITE "http://tizen.org/privilege/contact.write"

int ctsvc_cynara_initialize(void);
void ctsvc_cynara_finalize(void);
bool ctsvc_check_privilege(int fd, const char *privilege);

int ctsvc_have_file_read_permission(const char *path);

void ctsvc_set_client_access_info(int fd, const char *smack);
void ctsvc_unset_client_access_info(void);
void ctsvc_reset_all_client_access_info(void);

bool ctsvc_have_permission(int permission);
char* ctsvc_get_client_smack_label(void);

int ctsvc_get_write_permitted_addressbook_ids(int **addressbook_ids, int *count);
bool ctsvc_have_ab_write_permission(int addressbook_id);
int ctsvc_is_owner(int addressbook_id);


#endif // __TIZEN_SOCIAL_CTSVC_DB_ACCESS_CONTROL_H__

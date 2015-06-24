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

#ifndef __CTSVC_CLIENT_IPC_H__
#define __CTSVC_CLIENT_IPC_H__

#include <pims-ipc.h>
#include "contacts_types.h"

int ctsvc_ipc_connect(contacts_h contact, unsigned int handle_id);
int ctsvc_ipc_disconnect(contacts_h contact, unsigned int handle_id, int connection_count);

int ctsvc_ipc_connect_on_thread(contacts_h contact);
int ctsvc_ipc_disconnect_on_thread(contacts_h contact, int connection_count);


bool ctsvc_ipc_is_busy();

pims_ipc_h ctsvc_ipc_get_handle_for_change_subsciption();

int ctsvc_ipc_create_for_change_subscription();
int ctsvc_ipc_destroy_for_change_subscription();
int ctsvc_ipc_recover_for_change_subscription();

int ctsvc_ipc_call(char *module, char *function, pims_ipc_h data_in, pims_ipc_data_h *data_out);

void ctsvc_client_ipc_set_change_version(contacts_h contact, int version);
int ctsvc_client_ipc_get_change_version(contacts_h contact);

int ctsvc_ipc_client_check_permission(int permission, bool *result);

int ctsvc_ipc_set_disconnected_cb(void (*cb)(void *), void *user_data);
int ctsvc_ipc_unset_disconnected_cb();
void ctsvc_ipc_set_disconnected(bool is_disconnected);
int ctsvc_ipc_get_disconnected();
void ctsvc_ipc_recovery();

#endif /* __CTSVC_CLIENT_IPC_H__ */


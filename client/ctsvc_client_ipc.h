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

#ifndef __TIZEN_SOCIAL_CTSVC_CLIENT_IPC_H__
#define __TIZEN_SOCIAL_CTSVC_CLIENT_IPC_H__

#include <pims-ipc.h>

pims_ipc_h ctsvc_get_ipc_handle();

int ctsvc_ipc_connect(void);
int ctsvc_ipc_disconnect(void);

int ctsvc_ipc_connect_on_thread(void);
int ctsvc_ipc_disconnect_on_thread(void);


bool ctsvc_ipc_is_busy();

pims_ipc_h ctsvc_ipc_get_handle_for_change_subsciption();
int ctsvc_ipc_create_for_change_subscription();
int ctsvc_ipc_destroy_for_change_subscription();

int ctsvc_ipc_call(char *module, char *function, pims_ipc_h data_in, pims_ipc_data_h *data_out);
int ctsvc_ipc_call_async(char *module, char *function, pims_ipc_h data_in, pims_ipc_call_async_cb callback, void *userdata);

void ctsvc_client_ipc_set_change_version(int version);
int ctsvc_client_ipc_get_change_version(void);

int ctsvc_ipc_client_check_permission(int permission, bool *result);

#endif /*  __TIZEN_SOCIAL_CTSVC_CLIENT_IPC_H__ */


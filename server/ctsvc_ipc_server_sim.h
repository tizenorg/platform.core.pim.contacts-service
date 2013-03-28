/*
 * Contacts Service
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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
#ifndef __CTSVC_IPC_SERVER_SIM_H__
#define __CTSVC_IPC_SERVER_SIM_H__

#include <pims-ipc-data.h>

void ctsvc_ipc_sim_insert_contact(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_sim_update_contact(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_sim_delete_contact(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);

#endif /*__CTSVC_IPC_SERVER_SIM_H__*/

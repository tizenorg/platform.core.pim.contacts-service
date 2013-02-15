/*
 * Calendar Service
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
#ifndef __CTSVC_IPC_SERVER_H__
#define __CTSVC_IPC_SERVER_H__

#include <pims-ipc-data.h>

// contacts_service.h
void ctsvc_ipc_server_connect(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_disconnect(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);

// contacts_db_init.h
void ctsvc_ipc_server_db_insert_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_db_get_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_db_update_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_db_delete_record(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_db_replace_record(pims_ipc_h ipc, pims_ipc_data_h indata,pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_db_get_all_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_db_get_records_with_query(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);

void ctsvc_ipc_server_db_get_count(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_db_get_count_with_query(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);

void ctsvc_ipc_server_db_insert_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_db_update_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_db_delete_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_db_replace_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);

void ctsvc_ipc_server_db_get_changes_by_version(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_db_get_current_version(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);

void ctsvc_ipc_server_db_search_records(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);
void ctsvc_ipc_server_db_search_records_with_query(pims_ipc_h ipc, pims_ipc_data_h indata, pims_ipc_data_h *outdata, void *userdata);


#endif /*__CTSVC_IPC_SERVER_H__*/

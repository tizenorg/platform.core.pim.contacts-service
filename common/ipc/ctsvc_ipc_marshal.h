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
#ifndef __ctsvc_ipc_marshal__
#define __ctsvc_ipc_marshal__

#include <pims-ipc-data.h>
#include "ctsvc_struct.h"
#include "contacts_record.h"

/*
 * record
 * pims_ipc_data_h 의 경우 생성된 사항을 넘겨받아야함
 * unmarshal_record의 경우 plugin에서 struct를 alloc하여 return
 * marshal : 각 plugin에서 cal_common_s + other 같이 marshal
 * unmarshal : cal_common_s 는 먼저 marshal 하여, view_uri 만 넘겨준 이후,
 *              각 plug in에서 cal_common_s를 제외 한 사항에 대하여 unmarshal
 */
typedef int (*ctsvc_ipc_unmarshal_record_cb)(const pims_ipc_data_h ipc_data, const char* view_uri, contacts_record_h precord);
typedef int (*ctsvc_ipc_marshal_record_cb)(const contacts_record_h record, pims_ipc_data_h ipc_data);

typedef struct {
    ctsvc_ipc_unmarshal_record_cb unmarshal_record;
    ctsvc_ipc_marshal_record_cb marshal_record;
} ctsvc_ipc_marshal_record_plugin_cb_s;

int ctsvc_ipc_unmarshal_record(const pims_ipc_data_h ipc_data, contacts_record_h* precord);
int ctsvc_ipc_marshal_record(const contacts_record_h record, pims_ipc_data_h ipc_data);

/*
 * string
 * char의 경우 NULL 설정의 이슈로 인하여, [int:string length]+[char*] 로 넘길 수 있도록 설정..
 */
int ctsvc_ipc_unmarshal_string(const pims_ipc_data_h ipc_data, char** ppbufchar);
int ctsvc_ipc_unmarshal_bool(const pims_ipc_data_h data, bool *pout);
int ctsvc_ipc_unmarshal_int(const pims_ipc_data_h data, int *pout);
int ctsvc_ipc_unmarshal_unsigned_int(const pims_ipc_data_h data, unsigned int *pout);
int ctsvc_ipc_unmarshal_record_common(const pims_ipc_data_h ipc_data, ctsvc_record_s* common);

/*
 * NULL 이슈로 ctsvc_ipc_unmarshal_string / ctsvc_ipc_marshal_string 는 pair 를 이루어야함.
 */
int ctsvc_ipc_marshal_string(const char* bufchar, pims_ipc_data_h ipc_data);
int ctsvc_ipc_marshal_bool(const bool in, pims_ipc_data_h ipc_data);
int ctsvc_ipc_marshal_int(const int in, pims_ipc_data_h ipc_data);
int ctsvc_ipc_marshal_unsigned_int(const unsigned int in, pims_ipc_data_h ipc_data);
int ctsvc_ipc_marshal_record_common(const ctsvc_record_s* common, pims_ipc_data_h ipc_data);

/*
 * filter, query
 *
 * marsharl : view_uri + other
 * unmarshal : view_uri를 먼저 get 하고 난 이후 나머지를 ..
 */
int ctsvc_ipc_unmarshal_query(const pims_ipc_data_h ipc_data, contacts_query_h *query);
int ctsvc_ipc_marshal_query(const contacts_query_h query, pims_ipc_data_h ipc_data);
int ctsvc_ipc_unmarshal_list(const pims_ipc_data_h ipc_data, contacts_list_h *list);
int ctsvc_ipc_unmarshal_child_list(const pims_ipc_data_h ipc_data, contacts_list_h* list);
int ctsvc_ipc_marshal_list(const contacts_list_h list, pims_ipc_data_h ipc_data);

#endif /* __ctsvc_ipc_marshal__ */

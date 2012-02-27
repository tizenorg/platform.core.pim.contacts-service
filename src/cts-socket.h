/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjae Shin <yj99.shin@samsung.com>
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
#ifndef __CTS_SOCKET_H__
#define __CTS_SOCKET_H__

#include "cts-struct.h"
#include "cts-sqlite.h"

#define CTS_SOCKET_PATH "/opt/data/contacts-svc/.contacts-svc.sock"
#define CTS_SOCKET_MSG_SIZE 128

//Message TYPE
enum{
	CTS_REQUEST_RETURN_VALUE,
	CTS_REQUEST_IMPORT_SIM,
	CTS_REQUEST_NORMALIZE_STR,
	CTS_REQUEST_NORMALIZE_NAME,
};
//#define CTS_REQUEST_IMPORT_SIM "cts_request_import_sim"
//#define CTS_REQUEST_NORMALIZE_STR "cts_request_normalize_str"
//#define CTS_REQUEST_RETURN_VALUE "cts_request_return_value"
#define CTS_REQUEST_MAX_ATTACH 5

#define CTS_NS_ATTACH_NUM 1 //NS = Normalize String
#define CTS_NN_ATTACH_NUM 3 //NN = Normalize Name

typedef struct{
	int type;
	int val;
	int attach_num;
	int attach_sizes[CTS_REQUEST_MAX_ATTACH];
}cts_socket_msg;

int cts_socket_init(void);
int cts_request_normalize_name(char dest[][CTS_SQL_MAX_LEN]);
int cts_request_normalize_str(const char * src, char * dest, int dest_size);
int cts_request_sim_import(void);
void cts_socket_final(void);

#endif //__CTS_SOCKET_H__


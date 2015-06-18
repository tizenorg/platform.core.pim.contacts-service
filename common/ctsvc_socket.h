/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Dohyung Jin <dh.jin@samsung.com>
 *                 Jongwon Lee <gogosing.lee@samsung.com>
 *                 Donghee Ye <donghee.ye@samsung.com>
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

#ifndef __TIZEN_SOCIAL_CTSVC_SOCKET_H__
#define __TIZEN_SOCIAL_CTSVC_SOCKET_H__

#include "contacts.h"

#define CTSVC_SOCKET_FILE ".contacts-svc.sock"
#define CTSVC_SOCKET_MSG_SIZE 1024

/* for use current contacts-svc-helper daemon */
/* Message type */
enum{
	CTSVC_SOCKET_MSG_TYPE_REQUEST_RETURN_VALUE,
	CTSVC_SOCKET_MSG_TYPE_REQUEST_IMPORT_SIM,
	CTSVC_SOCKET_MSG_TYPE_REQUEST_SIM_INIT_COMPLETE,
};

#define CTSVC_SOCKET_MSG_REQUEST_MAX_ATTACH 5

typedef struct{
	int type;
	int val;
	int attach_num;
	int attach_sizes[CTSVC_SOCKET_MSG_REQUEST_MAX_ATTACH];
}ctsvc_socket_msg_s;

int ctsvc_request_sim_import(int sim_slot_no);
int ctsvc_request_sim_get_initialization_status(int sim_slot_no, bool* completed);
int ctsvc_socket_init(void);
void ctsvc_socket_final(void);

#endif /* __TIZEN_SOCIAL_CTSVC_SOCKET_H__ */


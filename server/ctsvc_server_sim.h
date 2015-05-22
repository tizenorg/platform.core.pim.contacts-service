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
#ifndef __CTSVC_SERVER_SIM_H__
#define __CTSVC_SERVER_SIM_H__

typedef struct {
	int sim_index;
	int contact_id;
	int addressbook_id;
	char *name;
	char *number;
	char *anr1;
	char *anr2;
	char *anr3;
	char *email1;
	char *email2;
	char *email3;
	char *email4;
	char *nickname;
}sim_contact_s;


int ctsvc_server_sim_init(void);
int ctsvc_server_sim_final(void);

int ctsvc_server_sim_import_contact(void* data, int sim_slot_no);
int ctsvc_server_sim_get_info_id_by_sim_slot_no(int sim_slot_no);
int ctsvc_server_sim_get_sim_slot_no_by_info_id(int sim_info_id);
int ctsvc_server_socket_get_sim_init_status(void* data, int sim_slot_no);

#endif /* __CTSVC_SERVER_SIM_H__ */


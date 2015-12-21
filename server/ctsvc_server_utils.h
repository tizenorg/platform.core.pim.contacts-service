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
#ifndef __CTSVC_SERVER_UTILS_H__
#define __CTSVC_SERVER_UTILS_H__

int ctsvc_server_load_feature_list(void);
bool ctsvc_server_have_telephony_feature(void);

int ctsvc_server_init_configuration(void);
void ctsvc_server_final_configuration(void);

int ctsvc_server_set_default_sort(int lang);

void ctsvc_server_trim_memory(void);
void ctsvc_server_start_timeout(void);
void ctsvc_server_stop_timeout(void);

#endif /* __CTSVC_SERVER_UTILS_H__ */


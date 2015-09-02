/*
 * Contacts Service
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __CTSVC_CLIENT_HANDLE_H__
#define __CTSVC_CLIENT_HANDLE_H__
#include "contacts_types.h"

int ctsvc_client_handle_get_current_p(contacts_h *p_contact);
int ctsvc_client_handle_remove(contacts_h contact);
int ctsvc_client_handle_create(contacts_h *p_contact);

#endif /* __CTSVC_CLIENT_HANDLE_H__ */

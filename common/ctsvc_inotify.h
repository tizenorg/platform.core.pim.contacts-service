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
#ifndef __CTSVC_INOTIFY_H__
#define __CTSVC_INOTIFY_H__

#include "contacts_db.h"

int ctsvc_inotify_init(void);
void ctsvc_inotify_close(void);
int ctsvc_inotify_subscribe(contacts_h contact, const char *view_uri, contacts_db_changed_cb cb, void *data);
int ctsvc_inotify_unsubscribe(contacts_h contact, const char *view_uri, contacts_db_changed_cb cb, void *user_data);

int ctsvc_inotify_subscribe_ipc_ready(contacts_h contact, void (*cb)(void *), void *user_data);
int ctsvc_inotify_unsubscribe_ipc_ready(contacts_h contact);

#endif /* __CTSVC_INOTIFY_H__ */

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
#ifndef __TIZEN_SOCIAL_CTSVC_INOTIFY_H__
#define __TIZEN_SOCIAL_CTSVC_INOTIFY_H__

#include "contacts_db.h"

int ctsvc_inotify_init(void);
void ctsvc_inotify_close(void);
int ctsvc_inotify_subscribe(const char *view_uri, contacts_db_changed_cb cb, void *data);
int ctsvc_inotify_unsubscribe(const char *view_uri, contacts_db_changed_cb cb, void *user_data);
void ctsvc_inotify_call_blocked_callback();

#endif //__TIZEN_SOCIAL_CTSVC_INOTIFY_H__

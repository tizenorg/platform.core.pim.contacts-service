/*
 * Contacts Service Helper
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

#ifndef __CTSVC_SERVER_CHANGE_SUBJECT_H__
#define __CTSVC_SERVER_CHANGE_SUBJECT_H__

#include "contacts_types.h"

void ctsvc_change_subject_publish_changed_info();
void ctsvc_change_subject_clear_changed_info();

void ctsvc_change_subject_add_changed_phone_log_id(contacts_changed_e type, int id);
void ctsvc_change_subject_add_changed_person_id(contacts_changed_e type, int id);

#endif // __CTSVC_SERVER_CHANGE_SUBJECT_H__
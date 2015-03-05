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

#ifndef __TIZEN_SOCIAL_CTSVC_LIST_H__
#define __TIZEN_SOCIAL_CTSVC_LIST_H__

#include "contacts_views.h"

int ctsvc_list_clone(contacts_list_h list, contacts_list_h* out_list);
int ctsvc_list_get_nth_record_p( contacts_list_h list, int index, contacts_record_h* record );
int ctsvc_list_add_child( contacts_list_h list, contacts_record_h child_record );
int ctsvc_list_prepend( contacts_list_h list, contacts_record_h child_record );
int ctsvc_list_reverse( contacts_list_h list);
int ctsvc_list_remove_child( contacts_list_h list, contacts_record_h record, bool insert_delete_list );

int ctsvc_list_get_deleted_count(contacts_list_h list, unsigned int *count );
int ctsvc_list_get_deleted_nth_record_p( contacts_list_h list, int index, contacts_record_h* record );
int ctsvc_list_append_deleted_record(contacts_list_h list, contacts_record_h record);

#endif /*  __TIZEN_SOCIAL_CTSVC_LIST_H__ */

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

#ifndef __TIZEN_SOCIAL_CTSVC_DB_ACCESS_CONTROL_H__
#define __TIZEN_SOCIAL_CTSVC_DB_ACCESS_CONTROL_H__


enum {
	CTSVC_PERMISSION_CONTACT_NONE = 0x0,
	CTSVC_PERMISSION_CONTACT_READ = 0x1,
	CTSVC_PERMISSION_CONTACT_WRITE = 0x2,
	CTSVC_PERMISSION_PHONELOG_READ = 0x4,
	CTSVC_PERMISSION_PHONELOG_WRITE = 0x8,
};

int ctsvc_have_file_read_permission(const char *path);

void ctsvc_set_client_access_info(const char *smack_label, const char *cookie);
void ctsvc_unset_client_access_info(void);

bool ctsvc_have_permission(int permission);

#endif // __TIZEN_SOCIAL_CTSVC_DB_ACCESS_CONTROL_H__

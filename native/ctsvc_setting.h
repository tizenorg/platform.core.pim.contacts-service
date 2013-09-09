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

#ifndef __TIZEN_SOCIAL_CTSVC_SETTING_H__
#define __TIZEN_SOCIAL_CTSVC_SETTING_H__

int ctsvc_register_vconf(void);
void ctsvc_deregister_vconf(void);

int ctsvc_get_phonenumber_min_match_digit(void);
int ctsvc_get_primary_sort(void);
int ctsvc_get_secondary_sort(void);
const char *ctsvc_get_default_sort_vconfkey(void);
void ctscts_set_sort_memory(int sort_type);


#endif /*  __TIZEN_SOCIAL_CTSVC_SETTING_H__ */

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

#ifndef __TIZEN_SOCIAL_CTSVC_RECORD_H__
#define __TIZEN_SOCIAL_CTSVC_RECORD_H__

#define CTSVC_RECORD_INIT_BASE(base, type, cb, uri) do {\
    (base)->r_type = (type);\
    (base)->plugin_cbs = (cb);\
    (base)->view_uri = (uri);\
    (base)->property_max_count = 0;\
} while (0)

#define CTSVC_RECORD_COPY_BASE(dest, src) do {\
    (dest)->r_type = (src)->r_type;\
    (dest)->plugin_cbs = (src)->plugin_cbs;\
    (dest)->view_uri = (src)->view_uri;\
    (dest)->property_max_count = (src)->property_max_count;\
    (dest)->property_flag = (src)->property_flag;\
    if ((src)->properties_flags) \
    {\
        (dest)->properties_flags  = calloc((dest)->property_max_count, sizeof(char));\
		memcpy((dest)->properties_flags,(src)->properties_flags,sizeof(char)*(dest)->property_max_count);\
    }\
} while (0)

#define CTSVC_RECORD_RESET_PROPERTY_FLAGS(base)do {\
    if ((base)->properties_flags) \
    {\
        free((base)->properties_flags); \
        (base)->property_max_count = 0;\
        (base)->properties_flags = NULL;\
        (base)->property_flag = 0;\
    }\
} while (0)

int ctsvc_record_set_property_flag(ctsvc_record_s* _record, int property_id, contacts_property_flag_e flag);
int ctsvc_record_set_projection_flags( contacts_record_h record, const unsigned int *projection, const unsigned int projection_count, const unsigned int property_max_count);
int ctsvc_record_set_str( contacts_record_h record, unsigned int property_id, const char* value );
int ctsvc_record_set_bool( contacts_record_h record, unsigned int property_id, bool value );
int ctsvc_record_set_int( contacts_record_h record, unsigned int property_id, int value );
int ctsvc_record_set_lli( contacts_record_h record, unsigned int property_id, long long int value );
int ctsvc_record_set_double( contacts_record_h record, unsigned int property_id, double value );
bool ctsvc_record_check_property_flag(const ctsvc_record_s* s_record, unsigned int property_id, contacts_property_flag_e flag);

#endif /* __TIZEN_SOCIAL_CTSVC_RECORD_H__ */

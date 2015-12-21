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
#ifndef __CTSVC_NUMBER_UTILS_H__
#define __CTSVC_NUMBER_UTILS_H__

#include <sqlite3.h>

char* ctsvc_get_network_cc(bool reload);

int ctsvc_clean_number(const char *src, char *dest, int dest_size, bool replace_alphabet);
int ctsvc_normalize_number(const char *src, char *dest, int dest_size, bool replace_alphabet);
int ctsvc_get_minmatch_number(const char *src, char *dest, int dest_size, int min_match);
bool ctsvc_is_phonenumber(const char *src);
void ctsvc_db_phone_number_equal_callback(sqlite3_context  *context, int argc, sqlite3_value **argv);

void* ctsvc_init_tapi_handle_for_cc(void);
void ctsvc_deinit_tapi_handle_for_cc(void);

#endif /* __CTSVC_NUMBER_UTILS_H__ */

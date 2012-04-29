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
#ifndef __CTS_SQLITE_H__
#define __CTS_SQLITE_H__

#include <sqlite3.h>
#include "cts-struct.h"

#define CTS_SQL_MAX_LEN   2048 //normal string length
#define CTS_SQL_MIN_LEN  1024 //short sql string length

typedef sqlite3_stmt* cts_stmt;

int cts_db_open(void);
int cts_db_close(void);
int cts_db_change();
int cts_db_get_last_insert_id(void);
int cts_db_get_next_id(const char *table);

int cts_query_get_first_int_result(const char *query);
int cts_query_exec(const char *query);
cts_stmt cts_query_prepare(char *query);

int cts_stmt_step(cts_stmt stmt);
void cts_stmt_reset(cts_stmt stmt);
void cts_stmt_finalize(cts_stmt stmt);

int cts_stmt_get_first_int_result(cts_stmt stmt);

static inline int cts_stmt_bind_int(cts_stmt stmt, int pos, int num) {
	return sqlite3_bind_int(stmt, pos, num);
}
static inline int cts_stmt_bind_text(cts_stmt stmt, int pos, const char *str) {
	return sqlite3_bind_text(stmt, pos, str, strlen(str), SQLITE_STATIC);
}
static inline int cts_stmt_bind_copy_text(cts_stmt stmt, int pos,
		const char *str, int strlen){
	return sqlite3_bind_text(stmt, pos, str, strlen, SQLITE_TRANSIENT);
}

int cts_stmt_bind_copy_text(cts_stmt stmt, int pos, const char *str, int strlen);


int cts_stmt_bind_name(cts_stmt stmt, int start_cnt, cts_name *name_struct);
int cts_stmt_bind_postal(cts_stmt stmt, int start_cnt, cts_postal *postal_struct);
int cts_stmt_bind_company(cts_stmt stmt, int start_cnt, cts_company *company_struct);
int cts_stmt_bind_web(cts_stmt stmt, int start_cnt, cts_web *web_struct);
int cts_stmt_bind_messenger(cts_stmt stmt, int start_cnt, cts_messenger *im_struct);
int cts_stmt_bind_event(cts_stmt stmt, int start_cnt, cts_event *event_struct);
int cts_stmt_bind_extend(cts_stmt stmt, int start_cnt, cts_extend *extend_struct);

static inline double cts_stmt_get_dbl(cts_stmt stmt, int pos) {
	return sqlite3_column_double(stmt, pos);
}
static inline int cts_stmt_get_int(cts_stmt stmt, int pos) {
	return sqlite3_column_int(stmt, pos);
}
static inline char* cts_stmt_get_text(cts_stmt stmt, int pos) {
	return (char *)sqlite3_column_text(stmt, pos);
}

int cts_stmt_get_addressbook(cts_stmt stmt, cts_addrbook *ab);
int cts_stmt_get_number(cts_stmt stmt, cts_number *result, int start_cnt);
int cts_stmt_get_email(cts_stmt stmt, cts_email *result, int start_cnt);
int cts_stmt_get_name(cts_stmt stmt, cts_name *result, int start_cnt);


#endif //__CTS_SQLITE_H__


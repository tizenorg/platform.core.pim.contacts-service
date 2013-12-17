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

#ifndef __TIZEN_SOCIAL_CTSVC_SQLITE_H__
#define __TIZEN_SOCIAL_CTSVC_SQLITE_H__

#include <sqlite3.h>

#define CTS_SQL_MAX_LEN   2048 //normal string length
#define CTS_SQL_MIN_LEN  1024 //short sql string length

typedef sqlite3_stmt* cts_stmt;

int ctsvc_db_open(void);
int ctsvc_db_close(void);
int ctsvc_db_change();
int ctsvc_db_get_last_insert_id(void);
int ctsvc_db_get_next_id(const char *table);

int ctsvc_query_get_first_int_result(const char *query, int *result);
int ctsvc_query_exec(const char *query);
int ctsvc_query_prepare(char *query, cts_stmt *stmt);

int ctsvc_stmt_step(cts_stmt stmt);
void ctsvc_stmt_reset(cts_stmt stmt);
void ctsvc_stmt_finalize(cts_stmt stmt);

int ctsvc_stmt_get_first_int_result(cts_stmt stmt, int *result);


static inline double ctsvc_stmt_get_dbl(cts_stmt stmt, int pos) {
	return sqlite3_column_double(stmt, pos);
}
static inline int ctsvc_stmt_get_int(cts_stmt stmt, int pos) {
	return sqlite3_column_int(stmt, pos);
}
static inline char* ctsvc_stmt_get_text(cts_stmt stmt, int pos) {
	return (char *)sqlite3_column_text(stmt, pos);
}
static inline long long int ctsvc_stmt_get_int64(cts_stmt stmt, int pos) {
	return sqlite3_column_int64(stmt, pos);
}

static inline int ctsvc_stmt_bind_int(cts_stmt stmt, int pos, int num) {
	return sqlite3_bind_int(stmt, pos, num);
}
static inline int ctsvc_stmt_bind_text(cts_stmt stmt, int pos, const char *str) {
	return sqlite3_bind_text(stmt, pos, str, strlen(str), SQLITE_STATIC);
}
static inline int ctsvc_stmt_bind_copy_text(cts_stmt stmt, int pos,
		const char *str, int strlen){
	return sqlite3_bind_text(stmt, pos, str, strlen, SQLITE_TRANSIENT);
}

int ctsvc_stmt_bind_copy_text(cts_stmt stmt, int pos, const char *str, int strlen);


#endif //__TIZEN_SOCIAL_CTSVC_SQLITE_H__

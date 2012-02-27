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
#ifndef __CTS_ERRORS_H__
#define __CTS_ERRORS_H__

//<!--
typedef enum
{
	CTS_ERR_DB_LOCK = -204, /**< -204 */
	CTS_ERR_DB_RECORD_NOT_FOUND = -203, /**< -203 */
	CTS_ERR_DB_FAILED = -202, /**< -202 */
	CTS_ERR_DB_NOT_OPENED= -201, /**< -201 */

	CTS_ERR_ICU_FAILED = -106, /**< -106 */
	CTS_ERR_TAPI_FAILED = -105, /**< -105 */
	CTS_ERR_SOCKET_FAILED = -104, /**< -104 */
	CTS_ERR_INOTIFY_FAILED = -103, /**< -103 */
	CTS_ERR_VCONF_FAILED = -102, /**< -102 */
	CTS_ERR_VOBJECT_FAILED = -101, /**< -101 */

	CTS_ERR_NO_SPACE = -13, /**< -13 */
	CTS_ERR_IO_ERR = -12, /**< -12 */
	CTS_ERR_MSG_INVALID = -11, /**< -11 */
	CTS_ERR_ALREADY_RUNNING = -10, /**< -10 */
	CTS_ERR_EXCEEDED_LIMIT = -9, /**< -9 */
	CTS_ERR_OUT_OF_MEMORY = -8, /**< -8 */
	CTS_ERR_ALREADY_EXIST = -7, /**< -7 */
	CTS_ERR_ENV_INVALID = -6, /**< -6 */
	CTS_ERR_ARG_NULL = -5, /**< -5 */
	CTS_ERR_ARG_INVALID = -4, /**< -4 */
	CTS_ERR_NO_DATA = -3, /**< -3 */
	CTS_ERR_FINISH_ITER= -2, /**< -2 */
	CTS_ERR_FAIL= -1, /**< -1 */
	CTS_SUCCESS = 0, /**< 0 */
	CTS_FALSE = 0, /**< 0 */
	CTS_TRUE = 1 /**< 1 */
}cts_error;
//-->

#endif //__CTS_ERRORS_H__


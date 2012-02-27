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
#ifndef __CTS_INTERNAL_H__
#define __CTS_INTERNAL_H__

#include <stdio.h>
#include "cts-errors.h"
#include "cts-struct.h"

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#define SAFE_STR(src) (src)?src:""

#define CTS_DLOG_OUT
//#define CTS_DEBUGGING
//#define CTS_TIMECHECK

#ifdef CTS_DLOG_OUT
#define LOG_TAG "CONTACTS_SVC"
#include <dlog.h>
#define DLOG(prio, fmt, arg...) \
	do { SLOG(prio, LOG_TAG, fmt, ##arg); } while (0)
#define INFO(fmt, arg...) SLOGI(fmt, ##arg)
#define ERR(fmt, arg...) SLOGE("%s(%d): " fmt, __FUNCTION__, __LINE__, ##arg)
#define DBG(fmt, arg...) SLOGD("%s:" fmt, __FUNCTION__, ##arg)
#else
#include <unistd.h>
#define PRT(prio, fmt, arg...) \
	do { fprintf((prio?stderr:stdout),"[Contacts-service]" fmt"\n", ##arg); } while (0)
#define INFO(fmt, arg...) PRT(0, fmt, ##arg)
#define ERR(fmt, arg...) PRT(1,"%s(%d): " fmt, __FUNCTION__, __LINE__, ##arg)
#define DBG(fmt, arg...) \
	do { \
		printf("\x1b[105;37m[%d]\x1b[0m(%s)" fmt "\n", getpid(), __FUNCTION__, ##arg); \
	} while (0)
#endif

#ifdef CTS_DEBUGGING
#define CTS_FN_CALL DBG(">>>>>>>> called")
#define CTS_FN_END DBG("<<<<<<<< ended")
#define CTS_DBG(fmt, arg...) DBG("(%d) " fmt, __LINE__, ##arg)
#else /* CTS_DEBUGGING */
#define CTS_FN_CALL
#define CTS_FN_END
#define CTS_DBG(fmt, arg...)
#endif /* CTS_DEBUGGING */

#define warn_if(expr, fmt, arg...) do { \
	if (expr) { \
		ERR(fmt, ##arg); \
	} \
} while (0)
#define ret_if(expr) do { \
	if (expr) { \
		ERR("(%s)", #expr); \
		return; \
	} \
} while (0)
#define retv_if(expr, val) do { \
	if (expr) { \
		ERR("(%s)", #expr); \
		return (val); \
	} \
} while (0)
#define retm_if(expr, fmt, arg...) do { \
	if (expr) { \
		ERR(fmt, ##arg); \
		return; \
	} \
} while (0)
#define retvm_if(expr, val, fmt, arg...) do { \
	if (expr) { \
		ERR(fmt, ##arg); \
		return (val); \
	} \
} while (0)

/************** TimeCheck ***************/
#ifdef CTS_TIMECHECK

double correction, startT;
double cts_set_start_time(void);
double cts_exec_time(double start);
int cts_init_time(void);
#define CTS_START_TIME_CHECK \
	cts_init_time();\
startT = cts_set_start_time()
#define CTS_END_TIME_CHECK(fmt, arg...) \
	DBG(fmt" time = %f ms\n", ##arg, cts_exec_time(startT))

#else /* CTS_TIMECHECK */

#define CTS_START_TIME_CHECK
#define CTS_END_TIME_CHECK(arg)

#endif /* CTS_TIMECHECK */

#endif /* __CTS_INTERNAL_H__ */


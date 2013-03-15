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
#ifndef __CTSVC_SERVER_INTERNAL_H__
#define __CTSVC_SERVER_INTERNAL_H__

#include <stdio.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

// Additional Error
enum {
	CTSVC_ERR_NO_DB_FILE = -10000,
	CTSVC_ERR_NO_TABLE,
};

#define SERVER_DLOG_OUT
#define SERVER_DEBUGGING

#ifdef SERVER_DLOG_OUT
#define LOG_TAG "CONTACTS_SVC_SERVER"
#include <dlog.h>
#define DLOG(prio, fmt, arg...) \
	do { SLOG(prio, LOG_TAG, fmt, ##arg); } while (0)
#define INFO(fmt, arg...) SLOGI("[SERVER] "fmt, ##arg)
#define ERR(fmt, arg...) SLOGE("[SERVER] "fmt, ##arg)
#define DBG(fmt, arg...) SLOGD("[SERVER] "fmt, ##arg)
#else //SERVER_DLOG_OUT
#define PRT(prio, fmt, arg...) \
	do { fprintf((prio?stderr:stdout),"[SERVER] %s(%d): "fmt"\n", __FUNCTION__, __LINE__, ##arg); } while (0)
#define INFO(fmt, arg...) PRT(0, fmt, ##arg)
#define ERR(fmt, arg...) PRT(1, fmt, ##arg)
#define DBG(fmt, arg...) \
	do { \
		printf("\x1b[105;37m[SERVER]\x1b[0m(%s:%d) "fmt"\n", __FUNCTION__, __LINE__, ##arg); \
	} while (0)
#endif //SERVER_DLOG_OUT

#ifdef SERVER_DEBUGGING
#define SERVER_FN_CALL DBG(">>>>>>>> called")
#define SERVER_FN_END DBG("<<<<<<<< ended")
#define SERVER_DBG(fmt, arg...) DBG(fmt, ##arg)
#else /* SERVER_DEBUGGING */
#define SERVER_FN_CALL
#define SERVER_FN_END
#define SERVER_DBG(fmt, arg...)
#endif /* SERVER_DEBUGGING */

#define h_warn_if(expr, fmt, arg...) do { \
	if (expr) { \
		ERR(fmt, ##arg); \
	} \
} while (0)
#define h_ret_if(expr) do { \
	if (expr) { \
		ERR("(%s)", #expr); \
		return; \
	} \
} while (0)
#define h_retv_if(expr, val) do { \
	if (expr) { \
		ERR("(%s)", #expr); \
		return (val); \
	} \
} while (0)
#define h_retm_if(expr, fmt, arg...) do { \
	if (expr) { \
		ERR(fmt, ##arg); \
		return; \
	} \
} while (0)
#define h_retvm_if(expr, val, fmt, arg...) do { \
	if (expr) { \
		ERR(fmt, ##arg); \
		return (val); \
	} \
} while (0)

#endif // __CTSVC_SERVER_INTERNAL_H__


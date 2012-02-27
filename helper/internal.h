/*
 * Contacts Service Helper
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
#ifndef __CTS_HELPER_INTERNAL_H__
#define __CTS_HELPER_INTERNAL_H__

#include <stdio.h>

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

// Additional Error
enum {
	CTS_ERR_NO_DB_FILE = -10000,
};

// DBUS Information
#define CTS_DBUS_SERVICE "org.tizen.contacts.service"

#define HELPER_DLOG_OUT
//#define HELPER_DEBUGGING

#ifdef HELPER_DLOG_OUT
#define LOG_TAG "CONTACTS_SVC_HELPER"
#include <dlog.h>
#define DLOG(prio, fmt, arg...) \
	do { SLOG(prio, LOG_TAG, fmt, ##arg); } while (0)
#define INFO(fmt, arg...) SLOGI(fmt, ##arg)
#define ERR(fmt, arg...) SLOGE("%s(%d): " fmt, __FUNCTION__, __LINE__, ##arg)
#define DBG(fmt, arg...) SLOGD("%s:" fmt, __FUNCTION__, ##arg)
#else //HELPER_DLOG_OUT
#define PRT(prio, fmt, arg...) \
	do { fprintf((prio?stderr:stdout),"[Contacts-svc-helper]" fmt"\n", ##arg); } while (0)
#define INFO(fmt, arg...) PRT(0, fmt, ##arg)
#define ERR(fmt, arg...) PRT(1,"%s(%d): " fmt, __FUNCTION__, __LINE__, ##arg)
#define DBG(fmt, arg...) \
	do { \
		printf("\x1b[105;37m[Contacts-svc-helper]\x1b[0m(%s)" fmt "\n", __FUNCTION__, ##arg); \
	} while (0)
#endif //HELPER_DLOG_OUT

#ifdef HELPER_DEBUGGING
#define HELPER_FN_CALL DBG(">>>>>>>> called")
#define HELPER_FN_END DBG("<<<<<<<< ended")
#define HELPER_DBG(fmt, arg...) DBG("(%d) " fmt, __LINE__, ##arg)
#else /* HELPER_DEBUGGING */
#define HELPER_FN_CALL
#define HELPER_FN_END
#define HELPER_DBG(fmt, arg...)
#endif /* HELPER_DEBUGGING */

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

#endif // __CTS_HELPER_INTERNAL_H__


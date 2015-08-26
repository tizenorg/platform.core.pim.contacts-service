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

#ifndef __CTSVC_INTERNAL_H__
#define __CTSVC_INTERNAL_H__

#include <stdio.h>
#include <assert.h>

#include "contacts_errors.h"
#include "ctsvc_struct.h"
#include "ctsvc_view.h"

#ifdef API
#undef API
#endif
#define API __attribute__ ((visibility("default")))

/*#define CONTACTS_DEBUGGING */
/*#define CONTACTS_TIMECHECK */

#define LOG_TAG "CONTACTS_SERVICE"
#include <dlog.h>
#define DLOG(prio, fmt, arg...) \
	do { SLOG(prio, LOG_TAG, fmt, ##arg); } while (0)


#if defined(_CONTACTS_IPC_SERVER)
#define IPC_ROLE "[SERVER]"
#elif defined(_CONTACTS_IPC_CLIENT)
#define IPC_ROLE "[CLIENT]"
#else
#define IPC_ROLE "[LIB]"
#endif

#define INFO(fmt, arg...) SLOGI(IPC_ROLE" "fmt, ##arg)
#define ERR(fmt, arg...) SLOGE(IPC_ROLE" "fmt, ##arg)
#define DBG(fmt, arg...) SLOGD(IPC_ROLE" "fmt, ##arg)
#define WARN(fmt, arg...) SLOGD(IPC_ROLE" "fmt, ##arg)
#define VERBOSE(fmt, arg...) SLOGV(IPC_ROLE" "fmt, ##arg)

#ifdef CONTACTS_DEBUGGING

	#define CTS_FN_CALL DBG(">>>>>>>> called")
	#define CTS_FN_END DBG("<<<<<<<< ended")

	#define CTS_DBG(fmt, arg...) DBG(fmt, ##arg)
	#define CTS_WARN(fmt, arg...) WARN(fmt, ##arg)
	#define CTS_ERR(fmt, arg...) ERR(fmt, ##arg)
	#define CTS_INFO(fmt, arg...) INFO(fmt, ##arg)
	#define CTS_VERBOSE(fmt, arg...) VERBOSE(fmt, ##arg)

#else /* CONTACTS_DEBUGGING */
	#define CTS_FN_CALL
	#define CTS_FN_END

	#define CTS_DBG(fmt, arg...)
	#define CTS_WARN(fmt, arg...)
	#define CTS_ERR(fmt, arg...) ERR(fmt, ##arg)
	#define CTS_INFO(fmt, arg...) INFO(fmt, ##arg)
	#define CTS_VERBOSE(fmt, arg...)

	#define G_DISABLE_ASSERT
#endif /* CONTACTS_DEBUGGING */

#define WARN_IF(expr, fmt, arg...) do { \
	if (expr) { \
		CTS_WARN(fmt, ##arg); \
	} \
} while (0)
#define RET_IF(expr) do { \
	if (expr) { \
		CTS_ERR("(%s)", #expr); \
		return; \
	} \
} while (0)
#define RETV_IF(expr, val) do { \
	if (expr) { \
		CTS_ERR("(%s)", #expr); \
		return (val); \
	} \
} while (0)
#define RETM_IF(expr, fmt, arg...) do { \
	if (expr) { \
		CTS_ERR(fmt, ##arg); \
		return; \
	} \
} while (0)
#define RETVM_IF(expr, val, fmt, arg...) do { \
	if (expr) { \
		CTS_ERR(fmt, ##arg); \
		return (val); \
	} \
} while (0)


/* TO DISABLE THIS MACRO, DEFINE "G_DISABLE_ASSERT" */
#define ASSERT_NOT_REACHED(fmt, arg...) do { \
        CTS_ERR(fmt, ##arg); \
        assert(!"DO NOT REACH HERE!"); \
	} while (0)


#define CONTACTS_FREE(ptr) \
	do { \
		free(ptr); \
		ptr = NULL; \
	} while (0)

/* Thread-local storage */
#ifdef _CONTACTS_IPC_SERVER
#define TLS __thread
#elif _CONTACTS_IPC_CLIENT
#define TLS __thread
#else /* _CONTACTS_NATIVE */
#define TLS
#endif

#endif /* __CTSVC_INTERNAL_H__ */


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

#if defined(_CONTACTS_IPC_SERVER)
#define IPC_ROLE "[SERVER]"
#elif defined(_CONTACTS_IPC_CLIENT)
#define IPC_ROLE "[CLIENT]"
#else
#define IPC_ROLE "[LIB]"
#endif

#define CTS_LOG_RED "\033[0;31m"
#define CTS_LOG_GREEN "\033[0;32m"
#define CTS_LOG_BROWN "\033[0;33m"
#define CTS_LOG_BLUE "\033[0;34m"
#define CTS_LOG_END "\033[0;m"

#define _INFO(fmt, arg...) SLOGI(CTS_LOG_GREEN IPC_ROLE CTS_LOG_END fmt, ##arg)
#define _ERR(fmt, arg...) SLOGE(CTS_LOG_GREEN IPC_ROLE CTS_LOG_END fmt, ##arg)
#define _DBG(fmt, arg...) SLOGD(CTS_LOG_GREEN IPC_ROLE CTS_LOG_END fmt, ##arg)
#define _WARN(fmt, arg...) SLOGW(CTS_LOG_GREEN IPC_ROLE CTS_LOG_END fmt, ##arg)

#ifdef CONTACTS_DEBUGGING

	#define CTS_FN_CALL _DBG(">>>>>>>> called")
	#define CTS_FN_END _DBG("<<<<<<<< ended")

	#define DBG(fmt, arg...) _DBG(fmt, ##arg)
	#define WARN(fmt, arg...) _WARN(CTS_LOG_BROWN fmt CTS_LOG_END, ##arg)
	#define ERR(fmt, arg...) _ERR(CTS_LOG_RED fmt CTS_LOG_END, ##arg)
	#define INFO(fmt, arg...) _INFO(CTS_LOG_BLUE fmt CTS_LOG_END, ##arg)

#else /* CONTACTS_DEBUGGING */
	#define CTS_FN_CALL
	#define CTS_FN_END

	#define DBG(fmt, arg...)
	#define WARN(fmt, arg...)
	#define ERR(fmt, arg...) _ERR(fmt, ##arg)
	#define INFO(fmt, arg...) _INFO(fmt, ##arg)

	#define G_DISABLE_ASSERT
#endif /* CONTACTS_DEBUGGING */

#define RET_IF(expr) \
	do { \
		if (expr) { \
			ERR("(%s)", #expr); \
			return; \
		}\
	} while(0)

#define RETV_IF(expr, val) \
	do { \
		if (expr) { \
			ERR("(%s)", #expr); \
			return (val); \
		} \
	} while (0)

#define RETM_IF(expr, fmt, arg...) \
	do { \
		if (expr) { \
			ERR(fmt, ##arg); \
			return; \
		} \
	} while (0)

#define RETVM_IF(expr, val, fmt, arg...) \
	do { \
		if (expr) { \
			ERR(fmt, ##arg); \
			return (val); \
		} \
	} while (0)

#define WARN_IF(expr, fmt, arg...) \
	do { \
		if (expr) { \
			WARN(fmt, ##arg); \
		} \
	} while (0)


/* TO DISABLE THIS MACRO, DEFINE "G_DISABLE_ASSERT" */
#define ASSERT_NOT_REACHED(fmt, arg...) do { \
	ERR(fmt, ##arg); \
	assert(!"DO NOT REACH HERE!"); \
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


/*
 * Contacts Service
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

#ifndef __TIZEN_SOCIAL_CONTACTS_ERROR_H__
#define __TIZEN_SOCIAL_CONTACTS_ERROR_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_DATABASE_MODULE
 * @{
 */

typedef enum
{
	/* GENERAL */
	CONTACTS_ERROR_NONE                 = TIZEN_ERROR_NONE,                   /**< Successful */
	CONTACTS_ERROR_OUT_OF_MEMORY        = TIZEN_ERROR_OUT_OF_MEMORY,          /**< Out of memory */
	CONTACTS_ERROR_INVALID_PARAMETER    = TIZEN_ERROR_INVALID_PARAMETER,      /**< Invalid parameter */
	CONTACTS_ERROR_FILE_NO_SPACE		= TIZEN_ERROR_FILE_NO_SPACE_ON_DEVICE,	  /**< FS Full */
	/* LOGIC & DATA */
	CONTACTS_ERROR_NO_DATA				= TIZEN_ERROR_SOCIAL_CLASS | TIZEN_ERROR_NO_DATA, /**< Requested data does not exist */
	CONTACTS_ERROR_PERMISSION_DENIED	= TIZEN_ERROR_SOCIAL_CLASS | TIZEN_ERROR_PERMISSION_DENIED,	/**< Permission denied */

	/* DB */
//	CONTACTS_ERROR_DB_NOT_OPENED        = TIZEN_ERROR_SOCIAL_CLASS | 0x80,    /**< Database didn't opened not yet */
//	CONTACTS_ERROR_DB_LOCKED 			= TIZEN_ERROR_SOCIAL_CLASS | 0x81,	  /**< Database table locked or file locked */
//	CONTACTS_ERROR_DB_FAILED			= TIZEN_ERROR_SOCIAL_CLASS | 0x82,	  /**< Database operation failure */
//	CONTACTS_ERROR_DB_RECORD_NOT_FOUND  = TIZEN_ERROR_SOCIAL_CLASS | 0x83,	  /**< Empty result set */
//	CONTACTS_ERROR_DB_FULL				= TIZEN_ERROR_SOCIAL_CLASS | 0x84,	  /**< Database Full */
//	CONTACTS_ERROR_DB_IO_ERROR			= TIZEN_ERROR_SOCIAL_CLASS | 0x85,	  /**< Database I/O error */

	CONTACTS_ERROR_DB				    = TIZEN_ERROR_SOCIAL_CLASS | 0x9F,	  /**< Unknown DB error */

	/* IPC */
//	CONTACTS_ERROR_IPC_BUSY 			= TIZEN_ERROR_SOCIAL_CLASS | 0xB0,    /**< IPC bus locked */
	CONTACTS_ERROR_IPC_NOT_AVALIABLE    = TIZEN_ERROR_SOCIAL_CLASS | 0xB1,    /**< IPC server is not available */
	CONTACTS_ERROR_IPC					= TIZEN_ERROR_SOCIAL_CLASS | 0xBF,    /**< Unknown IPC error */

	/* VCARD */
//	CONTACTS_ERROR_VCARD_UNKNOWN_ERROR  = TIZEN_ERROR_SOCIAL_CLASS | 0xCF,    /**< Unknown Vcard error */

	/* ENVIRONMENT & OTEHR MODULE */
	CONTACTS_ERROR_SYSTEM				= TIZEN_ERROR_SOCIAL_CLASS | 0xEF,		/**< . */
/*
	CONTACTS_ERROR_SOCKET_FAILED 		= TIZEN_ERROR_SOCIAL_CLASS | 0xE0,
	CONTACTS_ERROR_INOTIFY_FAILED 		= TIZEN_ERROR_SOCIAL_CLASS | 0xE1,
	CONTACTS_ERROR_VCONF_FAILED 		= TIZEN_ERROR_SOCIAL_CLASS | 0xE2,
	CONTACTS_ERROR_ICU_FAILED 			= TIZEN_ERROR_SOCIAL_CLASS | 0xE3,
	CONTACTS_ERROR_TAPI_FAILED 			= TIZEN_ERROR_SOCIAL_CLASS | 0xE4,
*/

	/* UNHANDLED EXCEPTION */
	CONTACTS_ERROR_INTERNAL				= TIZEN_ERROR_SOCIAL_CLASS | 0xFF,    /**< Implementation Error, Temporary Use */
} contacts_error_e;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /*  __TIZEN_SOCIAL_CONTACTS_ERROR_H__ */


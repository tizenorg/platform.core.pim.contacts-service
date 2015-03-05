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
 * @file contacts_errors.h
 */

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_COMMON_MODULE
 * @{
 */

/**
 * @brief Enumeration for contacts errors.
 *
 * @since_tizen 2.3
 *
 */
typedef enum
{
    /* GENERAL */
    CONTACTS_ERROR_NONE                 = TIZEN_ERROR_NONE,                      /**< Successful */
    CONTACTS_ERROR_OUT_OF_MEMORY        = TIZEN_ERROR_OUT_OF_MEMORY,             /**< Out of memory */
    CONTACTS_ERROR_INVALID_PARAMETER    = TIZEN_ERROR_INVALID_PARAMETER,         /**< Invalid parameter */
    CONTACTS_ERROR_FILE_NO_SPACE        = TIZEN_ERROR_FILE_NO_SPACE_ON_DEVICE,   /**< FS Full */
    CONTACTS_ERROR_PERMISSION_DENIED    = TIZEN_ERROR_PERMISSION_DENIED,         /**< Permission denied */
    CONTACTS_ERROR_NOT_SUPPORTED        = TIZEN_ERROR_NOT_SUPPORTED,             /**< Not supported */

    /* LOGIC & DATA */
    CONTACTS_ERROR_NO_DATA	              = TIZEN_ERROR_NO_DATA,                   /**< Requested data does not exist */

    /* DB */
    CONTACTS_ERROR_DB_LOCKED            = TIZEN_ERROR_CONTACTS | 0x81,           /**< Database table locked or file locked */
    CONTACTS_ERROR_DB                   = TIZEN_ERROR_CONTACTS | 0x9F,           /**< Unknown DB error */

    /* IPC */
    CONTACTS_ERROR_IPC_NOT_AVALIABLE    = TIZEN_ERROR_CONTACTS | 0xB1,           /**< IPC server is not available */
    CONTACTS_ERROR_IPC                  = TIZEN_ERROR_CONTACTS | 0xBF,           /**< Unknown IPC error */

    /* ENVIRONMENT & OTHER MODULE */
    // Socket, inotify, vconf, icu, tapi, security/smack, account and so on
    CONTACTS_ERROR_SYSTEM               = TIZEN_ERROR_CONTACTS | 0xEF,           /**< Internal system module error */

    /* UNHANDLED EXCEPTION */
    CONTACTS_ERROR_INTERNAL				= TIZEN_ERROR_CONTACTS | 0xFF,            /**< Implementation Error, Temporary Use */
} contacts_error_e;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /*  __TIZEN_SOCIAL_CONTACTS_ERROR_H__ */


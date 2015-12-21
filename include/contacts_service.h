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
#ifndef __TIZEN_SOCIAL_CONTACTS_SERVICE_H__
#define __TIZEN_SOCIAL_CONTACTS_SERVICE_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_service.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_COMMON_MODULE Common
 *
 * @brief The contacts common API provides the set of definitions and interfaces to initialize and deinitialize.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_COMMON_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 * @{
 */

/**
 * @brief Connects to the contacts service.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks Connection opening is necessary to access the contacts server such as fetching, inserting, or updating records.\n
 *          The execution of contacts_connect() and contacts_disconnect() could slow down your application. So it is not recommended to call them frequently.
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_SYSTEM              System error
 * @retval  #CONTACTS_ERROR_INTERNAL            Internal error
 *
 * @see contacts_disconnect()
 */
int contacts_connect(void);

/**
 * @brief Disconnects from the contacts service.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks If there is no opened connection, this function returns #CONTACTS_ERROR_DB.
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                  Successful
 * @retval  #CONTACTS_ERROR_IPC                   IPC error
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY         Out of memory
 * @retval  #CONTACTS_ERROR_SYSTEM                System error
 * @retval  #CONTACTS_ERROR_DB                    Database operation failure
 *
 * @see contacts_connect()
 */
int contacts_disconnect(void);

/**
 * @brief Connects to the contacts service with a connection on another thread.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks Opening connection is necessary to access the contact server and to perform operations such as fetching, inserting, or updating records.\n
 *          On multiple thread environment with contacts_connect(), request can be failed in one thread, while another request is working by the connection in the other thread.
 *          To prevent request fail, contacts_connect_on_thread() is recommended. Then new connection is set for the thread.
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_SYSTEM              System error
 * @retval  #CONTACTS_ERROR_INTERNAL            Internal error
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 *
 * @see  contacts_disconnect_on_thread()
 */
int contacts_connect_on_thread(void);

/**
 * @brief Disconnects from the contacts service.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks If there is no opened connection, this function returns #CONTACTS_ERROR_DB.
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                  Successful
 * @retval  #CONTACTS_ERROR_IPC                   IPC error
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY         Out of memory
 * @retval  #CONTACTS_ERROR_SYSTEM                System error
 * @retval  #CONTACTS_ERROR_DB                    Database operation failure
 *
 * @see contacts_connect_on_thread()
 */
int contacts_disconnect_on_thread(void);


/**
 * @brief Definition for contacts_connect_with_flags(). If it is called the API with this flag, then retry to call contacts_connect() for several times.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @see contacts_connect_with_flags()
 */
#define CONTACTS_CONNECT_FLAG_RETRY	0x00000001

/**
 * @brief Definition for default flag of contacts_connect_with_flags().
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @see contacts_connect_with_flags()
 */
#define CONTACTS_CONNECT_FLAG_NONE	0

/**
 * @brief Connects to the contacts service.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   flags	connection flag
 *
 * @remarks Connection opening is necessary to access the contacts server such as fetching, inserting, or updating records.\n
 *          Before contacts-service daemon is ready, if you call contacts_connect(), it will fail.
 *          To prevent it, if you call this API with @ref CONTACTS_CONNECT_FLAG_RETRY flags, it will retry several time.\n
 *          To close the connection, contacts_disconnect() should be called.
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_SYSTEM              System error
 * @retval  #CONTACTS_ERROR_INTERNAL            Internal error
 *
 * @see  contacts_disconnect()
 */
int contacts_connect_with_flags(unsigned int flags);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif /* __TIZEN_SOCIAL_CONTACTS_SERVICE_H__ */

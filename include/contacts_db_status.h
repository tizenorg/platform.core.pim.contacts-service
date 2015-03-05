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
#ifndef __TIZEN_SOCIAL_CONTACTS_DB_STATUS_H__
#define __TIZEN_SOCIAL_CONTACTS_DB_STATUS_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_db_status.h
 */

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_DATABASE_MODULE
 * @{
 */

/**
 * @brief Enumeration for contact DB status.
 *
 * @since_tizen 2.3
 *
 */

typedef enum {
    CONTACTS_DB_STATUS_NORMAL,             /**< Normal */
    CONTACTS_DB_STATUS_CHANGING_COLLATION, /**< DB status is Changing collation */
} contacts_db_status_e;

/**
 * @brief  Gets the current status of server.
 *
 * @since_tizen 2.3
 *
 * @param[in]  status  The current status of server
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @pre     This function requires an open connection to the contacts service by contacts_connect().
 *
 * @see contacts_connect()
 */

int contacts_db_get_status(contacts_db_status_e *status);

/**
 * @brief  Called when contacts-service server status changes.
 *
 * @since_tizen 2.3
 *
 * @param[in]  status       The current status of server
 * @param[in]  user_data    The user data passed from the callback registration function
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect().
 *
 * @see contacts_db_add_status_changed_cb()
 */

typedef void (*contacts_db_status_changed_cb)(contacts_db_status_e status, void* user_data);

/**
 * @brief  Registers a callback function.
 *
 * @since_tizen 2.3
 *
 * @param[in]  callback     The callback function to register
 * @param[in]  user_data    The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @pre		This function requires an open connection to the contacts service by contacts_connect().
 *
 * @see contacts_connect()
 * @see contacts_db_remove_status_changed_cb()
 */

int contacts_db_add_status_changed_cb(contacts_db_status_changed_cb callback, void* user_data);

/**
 * @brief  Unregisters a callback function.
 *
 * @since_tizen 2.3
 *
 * @param[in]  callback   The callback function to register
 * @param[in]  user_data  The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @pre  This function requires an open connection to the contacts service by contacts_connect().
 *
 * @see contacts_connect()
 * @see contacts_db_add_status_changed_cb()
 */

int contacts_db_remove_status_changed_cb(contacts_db_status_changed_cb callback, void* user_data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_DB_STATUS_H__

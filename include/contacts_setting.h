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
#ifndef __TIZEN_SOCIAL_CONTACTS_SETTING_H__
#define __TIZEN_SOCIAL_CONTACTS_SETTING_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_setting.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_SETTING_MODULE Setting
 *
 * @brief The contacts setting API provides the set of definitions and interfaces that enable application developers to set up contacts features.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_SETTING_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 * @{
 */

/**
 * @brief Enumeration for name display order.
 *
 * @since_tizen 2.3
 *
 */
typedef enum
{
    CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST,   /**< First name comes at the first */
    CONTACTS_NAME_DISPLAY_ORDER_LASTFIRST    /**< First name comes at the last */
} contacts_name_display_order_e;

/**
 * @brief Gets the contacts name display order.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @param[out]  name_display_order    The name display order
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_IPC                 Unknown IPC error
 * @retval  #CONTACTS_ERROR_SYSTEM              Internal system module error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 */
int contacts_setting_get_name_display_order(contacts_name_display_order_e *name_display_order);

/**
 * @brief Sets the contacts name display order.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @param[in]  name_display_order    The name display order
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_IPC                 Unknown IPC error
 * @retval  #CONTACTS_ERROR_SYSTEM              Internal system module error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 * @post		contacts_setting_name_display_order_changed_cb() callback will be called upon success.
 *
 * @see contacts_connect()
 */
int contacts_setting_set_name_display_order(contacts_name_display_order_e name_display_order);


/**
 * @brief Enumeration for name sorting order.
 *
 * @since_tizen 2.3
 *
 */
typedef enum
{
    CONTACTS_NAME_SORTING_ORDER_FIRSTLAST,   /**< Contacts are first sorted based on the first name  */
    CONTACTS_NAME_SORTING_ORDER_LASTFIRST    /**< Contacts are first sorted based on the last name  */
} contacts_name_sorting_order_e;


/**
 * @brief Gets the contacts name sorting order in which contacts are returned.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @param[out]  name_sorting_order    The name sorting order
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_IPC                 Unknown IPC error
 * @retval  #CONTACTS_ERROR_SYSTEM              Internal system module error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 */
int contacts_setting_get_name_sorting_order(contacts_name_sorting_order_e *name_sorting_order);

/**
 * @brief Sets the contacts name sorting order in which contacts are returned.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @param[in]  name_sorting_order    The name sorting order
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_IPC                 Unknown IPC error
 * @retval  #CONTACTS_ERROR_SYSTEM              Internal system module error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 * @post		contacts_setting_name_sorting_order_changed_cb() callback will be called upon success.
 *
 * @see contacts_connect()
 */
int contacts_setting_set_name_sorting_order(contacts_name_sorting_order_e name_sorting_order);

/**
 * @brief Called when a designated view changes.
 *
 * @since_tizen 2.3
 *
 * @param[in]   name_display_order  The name display order setting value
 * @param[in]   user_data           The user data passed from the callback registration function
 *
 * @pre The callback must be registered using contacts_setting_add_name_display_order_changed_cb().
 * contacts_setting_set_name_display_order() must be called to invoke this callback.
 *
 * @see contacts_setting_add_name_display_order_changed_cb()
 * @see contacts_setting_remove_name_display_order_changed_cb()
 */
typedef void (*contacts_setting_name_display_order_changed_cb)(contacts_name_display_order_e name_display_order, void* user_data);


/**
 * @brief Registers a callback function.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @param[in]   callback    The callback function to register
 * @param[in]   user_data   The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE               Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval  #CONTACTS_ERROR_IPC                Unknown IPC error
 * @retval  #CONTACTS_ERROR_INTERNAL           Implementation Error, Temporary Use
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED  Permission denied. This application does not have the privilege to call this method.
 *
 * @pre		contacts_connect() should be called to open a connection to the contacts service.
 * @post		contacts_setting_name_display_order_changed_cb() will be called under certain conditions, after calling contacts_setting_set_name_display_order().
 *
 * @see contacts_connect()
 * @see contacts_setting_remove_name_display_order_changed_cb()
 */

int contacts_setting_add_name_display_order_changed_cb(contacts_setting_name_display_order_changed_cb callback, void* user_data);

/**
 * @brief Unregisters a callback function.
 *
 * @since_tizen 2.3
 * @param[in]   callback   The callback function to register
 * @param[in]   user_data  The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_INTERNAL            Implementation Error, Temporary Use
 *
 * @pre contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_setting_add_name_display_order_changed_cb()
 */

int contacts_setting_remove_name_display_order_changed_cb(contacts_setting_name_display_order_changed_cb callback, void* user_data);

/**
 * @brief Called when a designated view changes.
 *
 * @since_tizen 2.3
 * @param[in]   name_sorting_order  The name sorting order setting value
 * @param[in]   user_data           The user data passed from the callback registration function
 *
 * @pre The callback must be registered using contacts_setting_add_name_sorting_order_changed_cb().
 * contacts_setting_set_name_sorting_order() must be called to invoke this callback.
 *
 * @see contacts_setting_add_name_sorting_order_changed_cb()
 * @see contacts_setting_remove_name_sorting_order_changed_cb()
 */
typedef void (*contacts_setting_name_sorting_order_changed_cb)(contacts_name_sorting_order_e name_sorting_order, void* user_data);


/**
 * @brief Registers a callback function.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @param[in]   callback    The callback function to register
 * @param[in]   user_data   The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_IPC                 Unknown IPC error
 * @retval  #CONTACTS_ERROR_INTERNAL            Implementation Error, Temporary Use
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 *
 * @pre		contacts_connect() should be called to open a connection to the contacts service.
 * @post		contacts_setting_name_sorting_order_changed_cb() will be called under certain conditions, after calling contacts_setting_set_name_sorting_order().
 *
 * @see contacts_connect()
 * @see contacts_setting_remove_name_sorting_order_changed_cb()
 */

int contacts_setting_add_name_sorting_order_changed_cb(contacts_setting_name_sorting_order_changed_cb callback, void* user_data);

/**
 * @brief Unregisters a callback function.
 *
 * @since_tizen 2.3
 * @param[in]   callback    The callback function to register
 * @param[in]   user_data   The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_INTERNAL            Implementation Error, Temporary Use
 *
 * @pre contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_setting_add_name_sorting_order_changed_cb()
 */

int contacts_setting_remove_name_sorting_order_changed_cb(contacts_setting_name_sorting_order_changed_cb callback, void* user_data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_SETTING_H__

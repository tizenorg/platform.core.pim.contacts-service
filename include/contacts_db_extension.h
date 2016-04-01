/*
 * Contacts Service
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __TIZEN_SOCIAL_CONTACTS_DB_EXTENSION_H__
#define __TIZEN_SOCIAL_CONTACTS_DB_EXTENSION_H__


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_db_extension.h
 */

/**
 * @brief Called when the designated view changes.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   view_uri	The view URI, only _contacts_person and _contacts_phone_log are now supported
 * @param[in]   changes	It includes changes information ("type:id," string is repeated, you should parse it)
 * @param[in]   user_data	The user data passed from the callback registration function
 *
 * @see contacts_db_add_changed_cb_with_info()
 */

typedef void (*contacts_db_change_cb_with_info)(const char *view_uri, char *changes, void *user_data);

/**
 * @brief Registers a callback function.
 * @details Now, support only _contacts_person and _contacts_phone_log view_uri.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 * @privilege %http://tizen.org/privilege/callhistory.read
 *
 * @remarks %http://tizen.org/privilege/contact.read is needed to get notification whenever record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person is changed, \n
 * %http://tizen.org/privilege/callhistory.read is needed to get notification whenever record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log is changed.
 *
 * @param[in]   view_uri	The view URI of records whose changes are monitored
 * @param[in]   callback	The callback function to register
 * @param[in]	user_data	The user data to be passed to the callback function
 *
 * @return  @c 0 on sucess,
 *          otherwise a negative error value (#contacts_error_e)
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @pre		contacts_connect() should be called to open a connection to the contacts service.
 * @post		contacts_db_change_cb_with_info() callback will be called
 *
 * @see contacts_connect()
 * @see contacts_db_changed_cb_with_info()
 * @see contacts_db_remove_changed_cb_with_info()
 */
int contacts_db_add_changed_cb_with_info(const char *view_uri, contacts_db_change_cb_with_info callback, void *user_data);

/**
 * @brief Unregisters a callback function.
 * @details Now, support only _contacts_person and _contacts_phone_log view_uri.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   view_uri	The view URI of records whose changes are monitored
 * @param[in]   callback	The callback function to register
 * @param[in]	user_data	The user data to be passed to the callback function
 *
 * @return  @c 0 on sucess,
 *          otherwise a negative error value (#contacts_error_e)
 *
 * @retval	#CONTACTS_ERROR_NONE                Successful
 * @retval	#CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @pre		contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_db_changed_cb_with_info()
 * @see contacts_db_add_changed_cb_with_info()
 */
int contacts_db_remove_changed_cb_with_info(const char *view_uri, contacts_db_change_cb_with_info callback, void *user_data);


#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CONTACTS_DB_EXTENSION_H__ */


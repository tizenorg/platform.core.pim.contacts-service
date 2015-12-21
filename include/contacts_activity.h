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
#ifndef __TIZEN_SOCIAL_CONTACTS_ACTIVITY_H__
#define __TIZEN_SOCIAL_CONTACTS_ACTIVITY_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_activity.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_ACTIVITY_MODULE Activity
 *
 * @brief The contacts activity API provides the set of definitions and interfaces that enable application developers to delete activities by @a contact_id and @a account_id. \n
 *        For more details, see @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_ACTIVITY_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 * @{
 */

/**
 * @brief Deletes an activity record from the contacts database by contact ID.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @param[in]  contact_id  The contact ID to delete
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value (#contacts_error_e)
 *
 * @retval  #CONTACTS_ERROR_NONE                  Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY         Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER     Invalid parameter
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE         FS Full
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED     Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_DB                    Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                   IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see  contacts_connect()
 */
int contacts_activity_delete_by_contact_id(int contact_id);

/**
 * @brief Deletes an activity record from the contacts database by account ID.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @param[in]  account_id    The account ID to delete
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE       FS Full
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see  contacts_connect()
 */
int contacts_activity_delete_by_account_id(int account_id);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif /* __TIZEN_SOCIAL_CONTACTS_ACTIVITY_H__ */


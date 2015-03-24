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
#ifndef __TIZEN_SOCIAL_CONTACTS_GROUP_H__
#define __TIZEN_SOCIAL_CONTACTS_GROUP_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_group.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_GROUP_MODULE Group
 *
 * @brief The contacts group API provides the set of definitions and interfaces that enable application developers to add/remove contact as group member.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_GROUP_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 * @{
 */

/**
 * @brief Adds a contact and a group relationship to the contacts database.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @param[in]   group_id       The group ID
 * @param[in]   contact_id     The contact ID
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 *
 * @pre   contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_group_remove_contact()
 */
int contacts_group_add_contact(int group_id, int contact_id);

/**
 * @brief Removes a contact and a group relationship from the contacts database.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @param[in]   group_id       The group ID
 * @param[in]   contact_id     The contact ID
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 *
 * @pre   contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_group_add_contact()
 */
int contacts_group_remove_contact(int group_id, int contact_id);

/**
 * @brief Sets a group between the previous group and the next group.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @param[in]   group_id                The group ID to move
 * @param[in]   previous_group_id       The previous group ID
 * @param[in]   next_group_id           The back group ID
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see  contacts_connect()
 */
int contacts_group_set_group_order(int group_id, int previous_group_id, int next_group_id);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_GROUP_H__

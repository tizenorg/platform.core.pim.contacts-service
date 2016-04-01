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
#ifndef __TIZEN_SOCIAL_CONTACTS_LIST_H__
#define __TIZEN_SOCIAL_CONTACTS_LIST_H__


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_list.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_LIST_MODULE List
 *
 * @brief The contacts record API provides the set of definitions and interfaces that enable application developers to get/set records list data.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_LIST_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 * @{
 */


/**
 * @brief Creates a contacts list.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks You must release @a contacts_list using contacts_list_destroy().
 *
 * @param[out]  contacts_list    The contacts list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_list_destroy()
 */
int contacts_list_create(contacts_list_h *contacts_list);

/**
 * @brief Destroys a contacts list and releases its all resources.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   contacts_list  The contacts list handle
 * @param[in]   delete_child   Set @c true to destroy child records automatically,
 *                             otherwise set @c false to not destroy child records automatically
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                    Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see contacts_list_create()
 */
int contacts_list_destroy(contacts_list_h contacts_list, bool delete_child);

/**
 * @brief Retrieves the number of contact entities from a contacts list.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   contacts_list           The contacts list handle
 * @param[out]  count                   The count of contact entity
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_list_add()
 */
int contacts_list_get_count(contacts_list_h contacts_list, int *count);

/**
 * @brief Adds a record to a contacts list.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks Same kind of record can be added.
 *
 * @param[in]   contacts_list           The contacts list handle
 * @param[in]   record                  The record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_list_remove()
 */
int contacts_list_add(contacts_list_h contacts_list, contacts_record_h record);

/**
 * @brief Removes a record from the contacts list.
 *
 * @details If the record is current record, then current record is changed the next record.\n
 *          If the record is the last record, then current record will be @c NULL.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   contacts_list           The contacts list handle
 * @param[in]   record                  The record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 *
 * @see contacts_list_add()
 */
int contacts_list_remove(contacts_list_h contacts_list, contacts_record_h record);

/**
 * @brief Retrieves a record from the contacts list.
 *
 * @details The default current record is the first record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks You MUST NOT destroy the @a record.
 *          It is destroyed automatically when the @a contacts_list is destroyed.
 *
 * @param[in]   contacts_list           The contacts list handle
 * @param[out]  record                  The record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 */
int contacts_list_get_current_record_p(contacts_list_h contacts_list, contacts_record_h *record);

/**
 * @brief Moves a contacts list to the previous position.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]  contacts_list    The contacts list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 *
 * @see contacts_list_next()
 */
int contacts_list_prev(contacts_list_h contacts_list);

/**
 * @brief Moves a contacts list to the next position.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]  contacts_list      The contacts list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 *
 * @see contacts_list_prev()
 */
int contacts_list_next(contacts_list_h contacts_list);

/**
 * @brief Moves a contacts list to the first position.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]  contacts_list   The contacts list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 *
 * @see contacts_list_last()
 */
int contacts_list_first(contacts_list_h contacts_list);

/**
 * @brief Moves a contacts list to the last position.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]  contacts_list    The contacts list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 *
 * @see contacts_list_first()
 */
int contacts_list_last(contacts_list_h contacts_list);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CONTACTS_LIST_H__ */


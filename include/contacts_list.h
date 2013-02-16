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
#ifndef __TIZEN_SOCIAL_CONTACTS_LIST_H__
#define __TIZEN_SOCIAL_CONTACTS_LIST_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_LIST_MODULE
 * @{
 */

/**
 * @brief Creates a handle to the contacts list.
 *
 * @remarks @a contacts_list must be released with contacts_list_destroy() by you.
 *
 * @param[out]  contacts_list    The contacts list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see contacts_list_destroy()
 */
API int contacts_list_create( contacts_list_h* contacts_list );

/**
 * @brief Destroys a contacts list handle and releases all its resources.
 *
 * @param[in]   contacts_list  The contacts list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                    Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see contacts_list_create()
 */
API int contacts_list_destroy( contacts_list_h contacts_list, bool delete_child );

/**
 * @brief      Retrieves count of contact entity from a contacts list.
 *
 * @param[in]	contacts_list				The contacts list handle
 * @param[out]	count						The count of contact entity
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_list_add()
 */
API int contacts_list_get_count( contacts_list_h contacts_list, unsigned int *count );

/**
 * @brief      Adds a record handle to contacts list handle.
 *
 * @param[in]	contacts_list				The contacts list handle
 * @param[in]	record						The record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_list_remove()
 */
API int contacts_list_add( contacts_list_h contacts_list, contacts_record_h record );

/**
 * @brief      Removes a record handle to contacts list handle.
 * @details    If the record is current record then current record is changed the next record.\n
 * If the record is the last record then current record will be NULL.
 *
 * @param[in]	contacts_list				The contacts list handle
 * @param[in]	record						The record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_list_add()
 */
API int contacts_list_remove( contacts_list_h contacts_list, contacts_record_h record );

/**
 * @brief		Retrieves a record handle from contacts list handle.
 * @details		The default current record is the first record
 * @remarks		The @a record handle MUST NOT destroyed by you.
 * It is destroyed automatically when the @a contacts_list is destroyed.
 *
 * @param[in]	contacts_list				The contacts list handle
 * @param[out]	record						The record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
API int contacts_list_get_current_record_p( contacts_list_h contacts_list, contacts_record_h* record );

/**
 * @brief		Moves a contacts list to previous position.
 *
 * @param[in]	contacts_list				The contacts list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_list_next()
 */
API int contacts_list_prev( contacts_list_h contacts_list );

/**
 * @brief		Moves a contacts list to next position.
 *
 * @param[in]	contacts_list				The contacts list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_list_prev()
 */
API int contacts_list_next( contacts_list_h contacts_list );

/**
 * @brief		Moves a contacts list to the first position.
 *
 * @param[in]	contacts_list				The contacts list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_list_last()
 */
API int contacts_list_first( contacts_list_h contacts_list );

/**
 * @brief		Moves a contacts lis tto the last position.
 *
 * @param[in]	contacts_list				The contacts list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_list_first()
 */
API int contacts_list_last( contacts_list_h contacts_list );

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_LIST_H__

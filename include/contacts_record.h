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
#ifndef __TIZEN_SOCIAL_CONTACTS_RECORD_H__
#define __TIZEN_SOCIAL_CONTACTS_RECORD_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_record.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_RECORD_MODULE Record
 *
 * @brief The contacts record API provides the set of the definitions and interfaces that enable application developers to get/set data from/to contacts record handle.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_RECORD_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 * @{
 */


/**
 * @brief Creates a record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks You must release @a record using contacts_record_destroy().
 *
 * @param[in]   view_uri    The view URI
 * @param[out]  record      The record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @pre     contacts_connect() should be called to initialize.
 *
 * @see contacts_record_destroy()
 */
int contacts_record_create( const char* view_uri, contacts_record_h* record );

/**
 * @brief Destroys a record and releases its all resources.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record         The record handle
 * @param[in]   delete_child   Set @c true to destroy child records automatically,
 *                             otherwise set @c false to not destroy child records automatically
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                    Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see contacts_record_create()
 */
int contacts_record_destroy( contacts_record_h record, bool delete_child );

/**
 * @brief Makes a clone of a record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks You must release @a cloned_record using contacts_record_destroy().
 *
 * @param[in]   record              The record handle
 * @param[out]  cloned_record       The cloned record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                    Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY           Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER       Invalid parameter
 *
 * @see contacts_record_destroy()
 */
int contacts_record_clone( contacts_record_h record, contacts_record_h* cloned_record );

/**
 * @brief Gets a string from the record handle.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks You must release @a value using free().
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[out]  value           The value to be returned
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                 Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_record_get_str_p()
 * @see contacts_record_set_str()
 */
int contacts_record_get_str( contacts_record_h record, unsigned int property_id, char** value );

/**
 * @brief Gets a string pointer from the record handle.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks You MUST NOT release @a value.
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[out]  value           The value to be returned
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                 Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_record_get_str()
 * @see contacts_record_set_str()
 */
int contacts_record_get_str_p( contacts_record_h record, unsigned int property_id, char** value );

/**
 * @brief Sets a string to a record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[in]   value           The value to set
 *
 * @return      @c 0 on success,
 *              otherwise a negative error value
 *
 * @retval      #CONTACTS_ERROR_NONE                    Successful
 * @retval      #CONTACTS_ERROR_INVALID_PARAMETER       Invalid parameter
 * @retval      #CONTACTS_ERROR_NOT_SUPPORTED           Not supported
 *
 * @see contacts_record_get_str()
 * @see contacts_record_get_str_p()
 */
int contacts_record_set_str( contacts_record_h record, unsigned int property_id, const char* value );

/**
 * @brief Gets a record's integer value.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[out]  value           The value to be returned
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_record_set_int()
 */
int contacts_record_get_int( contacts_record_h record, unsigned int property_id, int* value );

/**
 * @brief Sets an integer value to a record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[in]   value           The value to set
 *
 * @return      @c 0 on success,
 *              otherwise a negative error value
 *
 * @retval      #CONTACTS_ERROR_NONE                    Successful
 * @retval      #CONTACTS_ERROR_INVALID_PARAMETER       Invalid parameter
 * @retval      #CONTACTS_ERROR_NOT_SUPPORTED           Not supported
 *
 * @see contacts_record_get_int()
 */
int contacts_record_set_int( contacts_record_h record, unsigned int property_id, int value );

/**
 * @brief Gets a record's long integer value.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[out]  value           The value to be returned
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_record_set_lli()
 */
int contacts_record_get_lli( contacts_record_h record, unsigned int property_id, long long int *value );

/**
 * @brief Sets a long long integer value to a record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[in]   value           The value to set
 *
 * @return      @c 0 on success,
 *              otherwise a negative error value
 *
 * @retval      #CONTACTS_ERROR_NONE                    Successful
 * @retval      #CONTACTS_ERROR_INVALID_PARAMETER       Invalid parameter
 * @retval      #CONTACTS_ERROR_NOT_SUPPORTED           Not supported
 *
 * @see contacts_record_get_lli()
 */
int contacts_record_set_lli( contacts_record_h record, unsigned int property_id, long long int value );

/**
 * @brief Gets a record's boolean value.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[out]  value           The value to be returned
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_record_set_bool()
 */
int contacts_record_get_bool( contacts_record_h record, unsigned int property_id, bool *value );

/**
 * @brief Sets a boolean value to a record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[in]   value           The value to set
 *
 * @return      @c 0 on success,
 *              otherwise a negative error value
 *
 * @retval      #CONTACTS_ERROR_NONE                    Successful
 * @retval      #CONTACTS_ERROR_INVALID_PARAMETER       Invalid parameter
 * @retval      #CONTACTS_ERROR_NOT_SUPPORTED           Not supported
 *
 * @see contacts_record_get_bool()
 */
int contacts_record_set_bool( contacts_record_h record, unsigned int property_id, bool value );

/**
 * @brief Gets a record's double value.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[out]  value           The value to be returned
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_record_set_double()
 */
int contacts_record_get_double( contacts_record_h record, unsigned int property_id, double *value );

/**
 * @brief Sets a double value to a record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[in]   value           The value to set
 *
 * @return      @c 0 on success,
 *              otherwise a negative error value
 *
 * @retval      #CONTACTS_ERROR_NONE                    Successful
 * @retval      #CONTACTS_ERROR_INVALID_PARAMETER       Invalid parameter
 * @retval      #CONTACTS_ERROR_NOT_SUPPORTED           Not supported
 *
 * @see contacts_record_get_double()
 */
int contacts_record_set_double( contacts_record_h record, unsigned int property_id, double value );

/**
 * @brief Adds a child record to the parent record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record          The parent record handle
 * @param[in]   property_id     The property ID
 * @param[in]   child_record    The child record handle to be added to parent record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                    Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER       Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_record_remove_child_record()
 */
int contacts_record_add_child_record( contacts_record_h record, unsigned int property_id, contacts_record_h child_record );

/**
 * @brief Removes a child record from the parent record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record          The parent record handle
 * @param[in]   property_id     The property ID
 * @param[in]   child_record    The child record handle to be removed from parent record handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                    Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER       Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_record_add_child_record()
 */
int contacts_record_remove_child_record( contacts_record_h record, unsigned int property_id, contacts_record_h child_record );

/**
 * @brief Gets the number of child records of a parent record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record          The parent record handle
 * @param[in]   property_id     The property ID
 * @param[out]  count           The child record count
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_record_add_child_record()
 * @see contacts_record_remove_child_record()
 */
int contacts_record_get_child_record_count( contacts_record_h record, unsigned int property_id, int *count );

/**
 * @brief Gets a child record handle pointer from the parent record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks You MUST NOT release @a child_record. It is released when the parent record is destroyed.
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[in]   index           The index of child record
 * @param[out]  child_record    The child record handle pointer
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                 Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval  #CONTACTS_ERROR_NO_DATA
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_record_add_child_record()
 * @see contacts_record_remove_child_record()
 * @see contacts_record_get_child_record_count()
 */
int contacts_record_get_child_record_at_p( contacts_record_h record, unsigned int property_id, int index, contacts_record_h* child_record );

/**
 * @brief Clones a child record list of the given parent record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks You must release @a cloned_list using contacts_list_destroy().
 *
 * @param[in]   record          The record handle
 * @param[in]   property_id     The property ID
 * @param[out]  cloned_list     The cloned list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                 Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_list_destroy()
 */
int contacts_record_clone_child_record_list( contacts_record_h record, unsigned int property_id, contacts_list_h* cloned_list );

/**
 * @brief Gets URI string from a record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record			The record handle
 * @param[out]  view_uri 			The URI of record
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                 Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER    Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 */
int contacts_record_get_uri_p( contacts_record_h record, const char** view_uri );

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_RECORD_H__

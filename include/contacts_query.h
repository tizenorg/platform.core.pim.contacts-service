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
#ifndef __TIZEN_SOCIAL_CONTACTS_QUERY_H__
#define __TIZEN_SOCIAL_CONTACTS_QUERY_H__


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_query.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_QUERY_MODULE Query
 *
 * @brief The contacts Query API provides the set of definitions and interfaces that enable application developers to make query to get list.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_CONTACTS_QUERY_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 * @{
 */

/**
 * @brief Creates a query.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @remarks You must release @a query using contacts_query_destroy().
 *
 * @param[in]   view_uri            The view URI of a query
 * @param[out]  query               The filter handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @pre     contacts_connect() should be called to initialize
 *
 * @see contacts_query_destroy()
 */
int contacts_query_create(const char *view_uri, contacts_query_h *query);

/**
 * @brief Destroys a query.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   query    The query handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_query_create()
 */
int contacts_query_destroy(contacts_query_h query);

/**
 * @brief Adds property IDs for projection.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   query              The query handle
 * @param[in]   property_id_array   The property ID array
 * @param[in]   count               The number of property IDs
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 */
int contacts_query_set_projection(contacts_query_h query, unsigned int property_id_array[], int count);

/**
 * @brief Sets the "distinct" option for projection.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   query           The query handle
 * @param[in]   set             Set @c true to set the distinct option for projection,
 *                              otherwise @c false to unset the distinct option
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
int contacts_query_set_distinct(contacts_query_h query, bool set);

/**
 * @brief Sets a filter for query.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   query           The query handle
 * @param[in]   filter          The filter handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_create()
 */
int contacts_query_set_filter(contacts_query_h query, contacts_filter_h filter);

/**
 * @brief Sets a sort mode for query.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   query           The query handle
 * @param[in]   property_id     The property ID to sort
 * @param[in]   is_ascending    Set @c true for ascending sort mode,
 *                              otherwise @c false for descending sort mode
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 */
int contacts_query_set_sort(contacts_query_h query, unsigned int property_id, bool is_ascending);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif /* __TIZEN_SOCIAL_CONTACTS_QUERY_H__ */
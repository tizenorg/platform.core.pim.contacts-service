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
#ifndef __TIZEN_SOCIAL_CONTACTS_QUERY_H__
#define __TIZEN_SOCIAL_CONTACTS_QUERY_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_QUERY_MODULE
 * @{
 */

/**
 * @brief   Creates a query handle.
 *
 * @remarks		@a query must be released with contacts_query_destroy() by you.
 *
 * @param[in]   view_uri			The view URI of a query
 * @param[out]  query				The filter handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 *
 * @see contacts_query_destroy()
 */
API int contacts_query_create( const char* view_uri, contacts_query_h* query );

/**
 * @brief   Destroys a query handle.
 *
 * @param[in]   query    The query handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_query_create()
 */
API int contacts_query_destroy( contacts_query_h query );

/**
 * @brief		Adds property IDs for projection.
 *
 * @param[in]   filter				The filter handle
 * @param[in]   property_id_array	The property ID array
 * @param[in]   count				The number of property IDs
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
API int contacts_query_set_projection(contacts_query_h query, unsigned int property_id_array[], int count);

/**
 * @brief		Set distinct option  for projection.
 *
 * @param[in]   query				The query handle
 * @param[in]   set				Set or unset
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
API int contacts_query_set_distinct(contacts_query_h query, bool set);

/**
 * @brief		Set a filter handle to query handle.
 *
 * @param[in]   query			The query handle
 * @param[in]   filter			The filter handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_create()
 */
API int contacts_query_set_filter(contacts_query_h query, contacts_filter_h filter);

/**
 * @brief		Sets sort mode.
 *
 * @param[in]   query			The query handle
 * @param[in]   property_id		The property ID to sort
 * @param[in]   is_ascending	Ascending or decending
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
API int contacts_query_set_sort(contacts_query_h query, unsigned int property_id, bool is_ascending);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_QUERY_H__

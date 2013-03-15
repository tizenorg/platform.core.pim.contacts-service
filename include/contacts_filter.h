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
#ifndef __TIZEN_SOCIAL_CONTACTS_FILTER_H__
#define __TIZEN_SOCIAL_CONTACTS_FILTER_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_FILTER_MODULE
 * @{
 */

typedef enum
{
	CONTACTS_MATCH_EXACTLY,		/**< case-sensitive */
	CONTACTS_MATCH_FULLSTRING,		/**< . */
	CONTACTS_MATCH_CONTAINS,		/**< . */
	CONTACTS_MATCH_STARTSWITH,		/**< . */
	CONTACTS_MATCH_ENDSWITH,		/**< . */
	CONTACTS_MATCH_EXISTS			/**< . */
} contacts_match_str_flag_e;

typedef enum
{
	CONTACTS_MATCH_EQUAL,					/**< . */
	CONTACTS_MATCH_GREATER_THAN,			/**< . */
	CONTACTS_MATCH_GREATER_THAN_OR_EQUAL,	/**< . */
	CONTACTS_MATCH_LESS_THAN,				/**< . */
	CONTACTS_MATCH_LESS_THAN_OR_EQUAL,		/**< . */
	CONTACTS_MATCH_NOT_EQUAL,				/**< this flag can yield poor performance */
	CONTACTS_MATCH_NONE,					/**< . */
} contacts_match_int_flag_e;

typedef enum {
	CONTACTS_FILTER_OPERATOR_AND,	/**< . */
	CONTACTS_FILTER_OPERATOR_OR		/**< . */
} contacts_filter_operator_e;


/**
 * @brief   Creates a handle to filter.
 *
 * @remarks		@a filter must be released with contacts_filter_destroy() by you.
 *
 * @param[in]   view_uri			The view URI of a filter
 * @param[out]  filter				The filter handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 *
 * @see contacts_filter_destroy()
 */
API int contacts_filter_create( const char* view_uri, contacts_filter_h* filter );

/**
 * @brief   Destroys a filter handle.
 *
 * @param[in]   filter    The filter handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_create()
 */
API int contacts_filter_destroy( contacts_filter_h filter );

/**
 * @brief		Adds a condition for string type property
 *
 * @param[in]   filter			The filter handle
 * @param[in]   property_id		The property ID to add a condition
 * @param[in]   match			The match flag
 * @param[in]   match_value		The match value
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_add_operator()
 */
API int contacts_filter_add_str( contacts_filter_h filter, unsigned int property_id, contacts_match_str_flag_e match, const char* match_value );

/**
 * @brief		Adds a condition for integer type property
 *
 * @param[in]   filter			The filter handle
 * @param[in]   property_id		The property ID to add a condition
 * @param[in]   match			The match flag
 * @param[in]   match_value		The match value
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_add_operator()
 */
API int contacts_filter_add_int( contacts_filter_h filter, unsigned int property_id, contacts_match_int_flag_e match, int match_value );

/**
 * @brief		Adds a condition for long long int type property
 *
 * @param[in]   filter			The filter handle
 * @param[in]   property_id		The property ID to add a condition
 * @param[in]   match			The match flag
 * @param[in]   match_value		The match value
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_add_operator()
 */
API int contacts_filter_add_lli( contacts_filter_h filter, unsigned int property_id, contacts_match_int_flag_e match, long long int match_value );

/**
 * @brief		Adds a condition for double type property
 *
 * @param[in]   filter			The filter handle
 * @param[in]   property_id		The property ID to add a condition
 * @param[in]   match			The match flag
 * @param[in]   match_value		The match value
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_add_operator()
 */
API int contacts_filter_add_double( contacts_filter_h filter, unsigned int property_id, contacts_match_int_flag_e match, double match_value );

/**
 * @brief		Adds a condition for boolean type property
 *
 * @param[in]   filter			The filter handle
 * @param[in]   property_id		The property ID to add a condition
 * @param[in]   match_value		The match value
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_add_operator()
 */
API int contacts_filter_add_bool( contacts_filter_h filter, unsigned int property_id, bool match_value );

/**
 * @brief		Adds a operator between conditions
 *
 * @param[in]   filter			The filter handle
 * @param[in]   operator_type	The operator type
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_add_str()
 * @see contacts_filter_add_int()
 * @see contacts_filter_add_bool()
 */
API int contacts_filter_add_operator( contacts_filter_h filter, contacts_filter_operator_e operator_type );

/**
 * @brief		Adds a filter handle to filter handle.
 *
 * @param[in]   parent_filter			The parent filter handle
 * @param[in]   child_filter			The child filter handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_add_operator()
 */
API int contacts_filter_add_filter(contacts_filter_h parent_filter, contacts_filter_h child_filter);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_FILTER_H__

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
#ifndef __TIZEN_SOCIAL_CONTACTS_FILTER_H__
#define __TIZEN_SOCIAL_CONTACTS_FILTER_H__


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_filter.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_FILTER_MODULE Filter
 *
 * @brief The contacts Filter API provides the set of definitions and interfaces that enable application developers to make filters to set query.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_CONTACTS_FILTER_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 * @{
 */

/**
 * @brief Enumeration for Contacts match string flags.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 */
typedef enum
{
	CONTACTS_MATCH_EXACTLY,         /**< Full string, case-sensitive */
	CONTACTS_MATCH_FULLSTRING,      /**< Full string, case-insensitive */
	CONTACTS_MATCH_CONTAINS,        /**< Sub string, case-insensitive */
	CONTACTS_MATCH_STARTSWITH,      /**< Start with, case-insensitive */
	CONTACTS_MATCH_ENDSWITH,        /**< End with, case-insensitive */
	CONTACTS_MATCH_EXISTS           /**< IS NOT NULL */
} contacts_match_str_flag_e;

/**
 * @brief Enumeration for Contacts match int flags.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 */
typedef enum
{
	CONTACTS_MATCH_EQUAL,                   /**< '=' */
	CONTACTS_MATCH_GREATER_THAN,            /**< '>' */
	CONTACTS_MATCH_GREATER_THAN_OR_EQUAL,   /**< '>=' */
	CONTACTS_MATCH_LESS_THAN,               /**< '<' */
	CONTACTS_MATCH_LESS_THAN_OR_EQUAL,      /**< '<=' */
	CONTACTS_MATCH_NOT_EQUAL,               /**< '<>', this flag can yield poor performance */
	CONTACTS_MATCH_NONE,                    /**< IS NULL */
} contacts_match_int_flag_e;

/**
 * @brief Enumeration for Contacts filter operators.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 */
typedef enum {
	CONTACTS_FILTER_OPERATOR_AND,   /**< AND */
	CONTACTS_FILTER_OPERATOR_OR     /**< OR */
} contacts_filter_operator_e;


/**
 * @brief Creates a filter.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @remarks You must release @a filter using contacts_filter_destroy().
 *
 * @param[in]   view_uri            The view URI of a filter
 * @param[out]  filter              The filter handle
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
 * @see contacts_filter_destroy()
 */
int contacts_filter_create(const char *view_uri, contacts_filter_h *filter);

/**
 * @brief Destroys a filter.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]   filter    The filter handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_create()
 */
int contacts_filter_destroy(contacts_filter_h filter);

/**
 * @brief Adds a condition for a string type property.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]   filter          The filter handle
 * @param[in]   property_id     The property ID to add a condition
 * @param[in]   match           The match flag
 * @param[in]   match_value     The match value
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_filter_add_operator()
 */
int contacts_filter_add_str(contacts_filter_h filter, unsigned int property_id, contacts_match_str_flag_e match, const char *match_value);

/**
 * @brief Adds a condition for an integer type property.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]   filter          The filter handle
 * @param[in]   property_id     The property ID to add a condition
 * @param[in]   match           The match flag
 * @param[in]   match_value     The match value
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_filter_add_operator()
 */
int contacts_filter_add_int(contacts_filter_h filter, unsigned int property_id, contacts_match_int_flag_e match, int match_value);

/**
 * @brief Adds a condition for a long int type property.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]   filter          The filter handle
 * @param[in]   property_id     The property ID to add a condition
 * @param[in]   match           The match flag
 * @param[in]   match_value     The match value
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_filter_add_operator()
 */
int contacts_filter_add_lli(contacts_filter_h filter, unsigned int property_id, contacts_match_int_flag_e match, long long int match_value);

/**
 * @brief Adds a condition for a double type property.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]   filter          The filter handle
 * @param[in]   property_id     The property ID to add a condition
 * @param[in]   match           The match flag
 * @param[in]   match_value     The match value
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_filter_add_operator()
 */
int contacts_filter_add_double(contacts_filter_h filter, unsigned int property_id, contacts_match_int_flag_e match, double match_value);

/**
 * @brief Adds a condition for a boolean type property.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]   filter          The filter handle
 * @param[in]   property_id     The property ID to add a condition
 * @param[in]   match_value     The match value
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @see contacts_filter_add_operator()
 */
int contacts_filter_add_bool(contacts_filter_h filter, unsigned int property_id, bool match_value);

/**
 * @brief Adds an operator between conditions.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]   filter          The filter handle
 * @param[in]   operator_type   The operator type
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_add_str()
 * @see contacts_filter_add_int()
 * @see contacts_filter_add_bool()
 */
int contacts_filter_add_operator(contacts_filter_h filter, contacts_filter_operator_e operator_type);

/**
 * @brief Adds a filter to a given filter.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in]   parent_filter       The parent filter handle
 * @param[in]   child_filter        The child filter handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @see contacts_filter_add_operator()
 */
int contacts_filter_add_filter(contacts_filter_h parent_filter, contacts_filter_h child_filter);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CONTACTS_FILTER_H__ */


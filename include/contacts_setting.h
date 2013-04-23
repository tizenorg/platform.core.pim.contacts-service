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
#ifndef __TIZEN_SOCIAL_CONTACTS_SETTING_H__
#define __TIZEN_SOCIAL_CONTACTS_SETTING_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_SETTING_MODULE
 * @{
 */

/**
 * @brief  Enumerations of name display order
 */
typedef enum
{
	CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST,	 /**< First name comes at the first */
	CONTACTS_NAME_DISPLAY_ORDER_LASTFIRST	 /**< First name comes at the last */
} contacts_name_display_order_e;

/**
 * @brief	Gets the contacts name display order.
 *
 * @param[out]	name_display_order    The name display order
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE				Successful
 * @retval	#CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre     This function requires an open connection to the contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 */
API int contacts_setting_get_name_display_order(contacts_name_display_order_e *name_display_order);

/**
 * @brief	Sets the contacts name display order.
 *
 * @param[in]	name_display_order    The name display order
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE				Successful
 * @retval	#CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre     This function requires an open connection to the contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 */
API int contacts_setting_set_name_display_order(contacts_name_display_order_e name_display_order);


/**
 * @brief  Enumerations of name display order
 */
typedef enum
{
	CONTACTS_NAME_SORTING_ORDER_FIRSTLAST,	 /**< Contacts are first sorted based on the first name  */
	CONTACTS_NAME_SORTING_ORDER_LASTFIRST	 /**< Contacts are first sorted based on the last name  */
} contacts_name_sorting_order_e;


/**
 * @brief	Gets the contacts name sorting order in which contacts are returned.
 *
 * @param[out]	name_sorting_order    The name sorting order
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE				Successful
 * @retval	#CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre     This function requires an open connection to the contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 */
API int contacts_setting_get_name_sorting_order(contacts_name_sorting_order_e *name_sorting_order);

/**
 * @brief	Sets the contacts name sorting order in which contacts are returned.
 *
 * @param[in]	name_sorting_order    The name sorting order
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE				Successful
 * @retval	#CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre     This function requires an open connection to the contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 */
API int contacts_setting_set_name_sorting_order(contacts_name_sorting_order_e name_sorting_order);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_SETTING_H__

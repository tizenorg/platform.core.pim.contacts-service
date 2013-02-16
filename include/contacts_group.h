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
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_GROUP_MODULE
 * @{
 */

/**
 * @brief       Adds a contact and a group relationship to the contacts database.
 *
 * @param[in]   group_id       The group ID
 * @param[in]   contact_id     The contact ID
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB           		Database operation failure
 *
 * @pre   This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_group_remove_contact()
 */
API int contacts_group_add_contact(int group_id, int contact_id);

/**
 * @brief       Removes a contact and a group relationship from the contacts database.
 *
 * @param[in]   group_id       The group ID
 * @param[in]   contact_id     The contact ID
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB           		Database operation failure
 *
 * @pre   This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_group_add_contact()
 */
API int contacts_group_remove_contact(int group_id, int contact_id);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_GROUP_H__

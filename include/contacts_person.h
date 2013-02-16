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
#ifndef __TIZEN_SOCIAL_CONTACTS_PERSON_H__
#define __TIZEN_SOCIAL_CONTACTS_PERSON_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_PERSON_MODULE
 * @{
 */

/**
 * @brief	Links a person to a person.
 *
 * @param[in]	base_person_id		The base person ID
 * @param[in]	person_id			The person ID to be linked
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE	Successful
 * @retval  #CONTACTS_ERROR_DB	Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see  contacts_connect2()
 */
API int contacts_person_link_person(int base_person_id, int person_id);

/**
 * @brief	Unlinks a contact from a person.
 *
 * @param[in]	person_id				The person ID
 * @param[in]	contact_id				The contact ID to unlink
 * @param[out]	unliked_person_id		The person ID generated
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE	Successful
 * @retval  #CONTACTS_ERROR_DB	Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see  contacts_connect2()
 */
API int contacts_person_unlink_contact(int person_id, int contact_id, int* unlinked_person_id);

/**
 * @brief	Resets a person's usage count.
 * @details The person is no longer in the most frequent contacted person list.
 *
 * @param[in]	person_id			The person ID
 * @param[in]	type				The type to reset
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE	Successful
 * @retval  #CONTACTS_ERROR_DB	Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see  contacts_connect2()
 */
API int contacts_person_reset_usage(int person_id, contacts_usage_type_e type);

/**
 * @brief	Sets a favorite person place between a previous person and a next person.
 *
 * @param[in]	person_id				The person ID to move
 * @param[in]	previous_person_id		The previous person ID
 * @param[in]	next_person_id			The back person ID
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE	Successful
 * @retval  #CONTACTS_ERROR_DB	Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see  contacts_connect2()
 */
API int contacts_person_set_favorite_order(int person_id, int previous_person_id, int next_person_id);

typedef enum {
	CONTACTS_PERSON_PROPERTY_NAME_CONTACT,		/**< . */
	CONTACTS_PERSON_PROPERTY_NUMBER,				/**< . */
	CONTACTS_PERSON_PROPERTY_EMAIL,				/**< . */
	CONTACTS_PERSON_PROPERTY_IMAGE,				/**< . */
} contacts_person_property_e;

/**
 * @brief	Sets a default property for a record.
 *
 * @remarks @a id can be contact_id, number_id, email_id, image_id
 *
 * @param[in]	property				#contacts_person_property_e
 * @param[in]	person_id				The person ID
 * @param[in]	id						The record id
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE	Successful
 * @retval  #CONTACTS_ERROR_DB	Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see  contacts_connect2()
 */
API int contacts_person_set_default_property(contacts_person_property_e property, int person_id,
		int id);

/**
 * @brief	Gets a default property for a record.
 *
 * @remarks @a id can be contact_id, number_id, email_id, image_id
 *
 * @param[in]	property				#contacts_person_property_e
 * @param[in]	person_id				The person ID
 * @param[out]	id						The record id of the property to be set as default

 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE	Successful
 * @retval  #CONTACTS_ERROR_DB	Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see  contacts_connect2()
 */
API int contacts_person_get_default_property(contacts_person_property_e property, int person_id,
		int *id);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_PERSON_H__

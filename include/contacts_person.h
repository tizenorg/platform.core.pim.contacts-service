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
#ifndef __TIZEN_SOCIAL_CONTACTS_PERSON_H__
#define __TIZEN_SOCIAL_CONTACTS_PERSON_H__


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_person.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_PERSON_MODULE Person
 *
 * @brief The contacts person API provides the set of definitions and interfaces that enable application developers to link/unlink person and contact.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_PERSON_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 * @{
 */

/**
 * @brief Links a person to another person.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @param[in]   base_person_id      The base person ID
 * @param[in]   person_id           The person ID to link to
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE       FS Full
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see  contacts_connect()
 */
int contacts_person_link_person(int base_person_id, int person_id);

/**
 * @brief Unlinks a contact from a person.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @param[in]   person_id               The person ID
 * @param[in]   contact_id              The contact ID to unlink
 * @param[out]  unlinked_person_id       The person ID generated
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE       FS Full
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see  contacts_connect()
 */
int contacts_person_unlink_contact(int person_id, int contact_id, int *unlinked_person_id);

/**
 * @brief Resets a person's usage count.
 *
 * @details The person is no longer in the most frequently contacted person list.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @param[in]   person_id           The person ID
 * @param[in]   type                The type to reset
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE       FS Full
 * @retval	#CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error

 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see  contacts_connect()
 */
int contacts_person_reset_usage(int person_id, contacts_usage_type_e type);

/**
 * @brief Sets the order of a (favorite) contact.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @param[in]   person_id               The person ID to move
 * @param[in]   previous_person_id      The previous person ID
 * @param[in]   next_person_id          The back person ID
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE       FS Full
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error

 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see  contacts_connect()
 */
int contacts_person_set_favorite_order(int person_id, int previous_person_id, int next_person_id);

/**
 * @brief Enumeration for Contacts person properties.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_PERSON_PROPERTY_NAME_CONTACT,      /**< Default contacts record */
    CONTACTS_PERSON_PROPERTY_NUMBER,            /**< Default number record */
    CONTACTS_PERSON_PROPERTY_EMAIL,             /**< Default email record */
    CONTACTS_PERSON_PROPERTY_IMAGE,             /**< Default image record */
} contacts_person_property_e;

/**
 * @brief Sets a record's default property.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @remarks @a id can be contact_id, number_id, email_id, image_id.
 *
 * @param[in]   property            #contacts_person_property_e
 * @param[in]   person_id           The person ID
 * @param[in]   id                  The record ID
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE       FS Full
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error

 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see  contacts_connect()
 */
int contacts_person_set_default_property(contacts_person_property_e property, int person_id,
        int id);

/**
 * @brief Gets a default property for a record.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @remarks @a id can be contact_id, number_id, email_id, image_id.
 *
 * @param[in]   property            #contacts_person_property_e
 * @param[in]   person_id           The person ID
 * @param[out]  id                  The record ID of the property to be set as default

 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see  contacts_connect()
 */
int contacts_person_get_default_property(contacts_person_property_e property, int person_id,
        int *id);

/**
 * @brief Gets aggregation suggestions.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @param[in]   person_id               The person ID
 * @param[out]  record_list             The list of person rocords
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE       FS Full
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see  contacts_connect()
 */
int contacts_person_get_aggregation_suggestions(int person_id, contacts_list_h *record_list);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif /* __TIZEN_SOCIAL_CONTACTS_PERSON_H__ */
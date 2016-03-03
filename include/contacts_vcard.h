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
#ifndef __TIZEN_SOCIAL_CONTACTS_VCARD_H__
#define __TIZEN_SOCIAL_CONTACTS_VCARD_H__


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_vcard.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_VCARD_MODULE vCard
 *
 * @brief The contacts record API provides the set of definitions and interfaces that enable application developers to get/set data from/to vCard.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VCARD_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 * @{
 */


/**
 * @brief Called to get a record handle of @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   record     The record handle
 * @param[in]   user_data  The user data passed from the foreach function
 *
 * @return  @c true to continue with the next iteration of the loop,
 *          otherwise @c false to break out of the loop
 *
 * @pre contacts_vcard_parse_to_contact_foreach() will invoke this callback.
 *
 * @see contacts_vcard_parse_to_contact_foreach()
 */
typedef bool (*contacts_vcard_parse_cb)(contacts_record_h record, void *user_data);

/**
 * @brief Retrieves all contacts with a record handle (_contacts_contact) from a vCard file.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   vcard_file_path     The file path of vCard stream file
 * @param[in]   callback            The callback function to invoke
 * @param[in]   user_data           The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 * @retval  #CONTACTS_ERROR_SYSTEM              System error
 *
 * @pre     contacts_connect() should be called to initialize.
 * @post    This function invokes contacts_vcard_parse_cb().
 *
 * @see  contacts_vcard_parse_cb()
 */
int contacts_vcard_parse_to_contact_foreach(const char *vcard_file_path, contacts_vcard_parse_cb callback, void *user_data);

/**
 * @brief Retrieves all contacts with a contacts list from a vCard stream.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   vcard_stream            The vCard stream
 * @param[out]  contacts_list           The contacts list handle
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @pre     contacts_connect() should be called to initialize.
 *
 */
int contacts_vcard_parse_to_contacts(const char *vcard_stream, contacts_list_h *contacts_list);

/**
 * @brief Retrieves the vCard stream from a contact.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   contact                 The contact record handle
 * @param[out]  vcard_stream            The vCard stream
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
int contacts_vcard_make_from_contact(contacts_record_h contact, char **vcard_stream);

/**
 * @brief Retrieves the vCard stream from a contact.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   my_profile              The my_profile record handle
 * @param[out]  vcard_stream            The vCard stream
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
int contacts_vcard_make_from_my_profile(contacts_record_h my_profile, char **vcard_stream);


/**
 * @brief Retrieves the vCard stream from a person.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @param[in]   person                  The person record handle
 * @param[out]  vcard_stream            The vCard stream
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE       FS Full
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_DB                  DB error
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to initialize.
 *
 */
int contacts_vcard_make_from_person(contacts_record_h person, char **vcard_stream);

/**
 * @brief Retrieves the count of contact entities from a vCard file.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   vcard_file_path             The person record handle
 * @param[out]  count                       The count of contact entity
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_SYSTEM              System error
 */
int contacts_vcard_get_entity_count(const char *vcard_file_path, int *count);

/**
 * @brief Gets the limit size of width and hight of photos to append in vCard files.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @param[out]   limit_size             limit size of width and hight of photos to append in vCard files
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
int contacts_vcard_get_limit_size_of_photo(unsigned int *limit_size);

/**
 * @brief Sets the limit size of width and hight of photos to append in vCard files.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @remarks limit_size should be same or bigger than 8 and smaller than 1080.
 *
 * @param[in]   limit_size             limit size of width and hight of photos to append in vCard files
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
int contacts_vcard_set_limit_size_of_photo(unsigned int limit_size);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif /* __TIZEN_SOCIAL_CONTACTS_VCARD_H__ */
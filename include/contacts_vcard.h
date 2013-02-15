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
#ifndef __TIZEN_SOCIAL_CONTACTS_VCARD_H__
#define __TIZEN_SOCIAL_CONTACTS_VCARD_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VCARD_MODULE
 * @{
 */
/**
 * @brief The callback function to get record hadle of \ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact.
 *
 * @param[in]	record	   The record handle
 * @param[in]	user_data  The user data passed from the foreach function
 *
 * @return	@c true to continue with the next iteration of the loop or @c false to break out of the loop.
 *
 * @pre contacts_vcard_parse_to_contact_foreach() will invoke this callback.
 *
 * @see contacts_vcard_parse_to_contact_foreach()
 */
typedef bool (*contacts_vcard_parse_cb)(contacts_record_h record, void *user_data);

/**
 * @brief      Retrieves all contacts with record handle(_contacts_contact) from a vCard file.
 *
 * @param[in]	vcard_file_path			The file path of vCard stream file
 * @param[in]	callback				The callback function to invoke
 * @param[in]	user_data				The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @post This function invokes contacts_vcard_stream_cb().
 *
 * @see  contacts_vcard_parse_cb()
 */
API int contacts_vcard_parse_to_contact_foreach(const char *vcard_file_path, contacts_vcard_parse_cb callback, void *user_data);

/**
 * @brief      Retrieves all contacts with contacts list from vCard stream.
 *
 * @param[in]	vcard_stream			The vCard stream
 * @param[out]	contacts_list			The contacts list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
API int contacts_vcard_parse_to_contacts(const char *vcard_stream, contacts_list_h *contacts_list);

/**
 * @brief      Retrieves vCard stream from a contact.
 *
 * @param[in]	contact					The contact record handle
 * @param[out]	vcard_stream			The vCard stream
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
API int contacts_vcard_make_from_contact(contacts_record_h contact, char **vcard_stream);

/**
 * @brief      Retrieves vCard stream from a person.
 *
 * @param[in]	person					The person record handle
 * @param[out]	vcard_stream			The vCard stream
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
API int contacts_vcard_make_from_person(contacts_record_h person, char **vcard_stream);

/**
 * @brief      Retrieves count of contact entity from a vCard file.
 *
 * @param[in]	vcard_file_path				The person record handle
 * @param[out]	count						The count of contact entity
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 */
API int contacts_vcard_get_entity_count(const char *vcard_file_path, int *count);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_VCARD_H__

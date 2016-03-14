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
#ifndef __TIZEN_SOCIAL_CONTACTS_DB_H__
#define __TIZEN_SOCIAL_CONTACTS_DB_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_db.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_DATABASE_MODULE Database
 *
 * @brief The contacts database API provides the set of definitions and interfaces that enable application developers to handle contacts database.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_DATABASE_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 * @{
 */

/**
 * @brief Enumeration for contact change state.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum
{
	CONTACTS_CHANGE_INSERTED,	/**< Inserted */
	CONTACTS_CHANGE_UPDATED,	/**< Updated */
	CONTACTS_CHANGE_DELETED,	/**< Deleted */
} contacts_changed_e;

/**
 * @brief Called when the designated view changes.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   view_uri    The view URI
 * @param[in]   user_data   The user data passed from the callback registration function
 *
 * @pre		The callback must be registered using contacts_db_add_changed_cb.
 *
 * @see contacts_db_add_changed_cb()
 */
typedef void (*contacts_db_changed_cb)(const char *view_uri, void *user_data);

/**
 * @brief Inserts a record to the contacts database.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 * @privilege %http://tizen.org/privilege/callhistory.write
 *
 * @remarks
 * %http://tizen.org/privilege/contact.write is needed for record which is created with @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact, \n @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile,
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial. \n\n
 * %http://tizen.org/privilege/callhistory.write is needed for record which is created with @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log.
 *
 * @param[in]   record                 The record handle
 * @param[out]  id                     The ID of inserted record
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
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 * @post 	contacts_db_changed_cb() callback wil be called upon success.
 *
 * @see contacts_connect()
 * @see contacts_db_update_record()
 * @see contacts_db_delete_record()
 * @see contacts_db_get_record()
 */
int contacts_db_insert_record(contacts_record_h record, int *id);

/**
 * @brief Gets a record from the contacts database.
 *
 * @details This function creates a new contact handle from the contacts database by the given @a record_id. \n
 *          @a record will be created, which is filled with contact information.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 * @privilege %http://tizen.org/privilege/callhistory.read
 *
 * @remarks %http://tizen.org/privilege/contact.read is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_simple_contact, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_sdn. \n\n
 * %http://tizen.org/privilege/callhistory.read	is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log.
 *
 * @remarks You must release @a record using contacts_record_destroy().
 *
 * @param[in]   view_uri    The view URI of a record
 * @param[in]   record_id   The record ID to get from database
 * @param[out]  record      The record handle associated with the record ID
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
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_record_destroy()
 */
int contacts_db_get_record(const char *view_uri, int record_id, contacts_record_h *record);

/**
 * @brief Updates a record in the contacts database.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 * @privilege %http://tizen.org/privilege/callhistory.write
 *
 * @remarks %http://tizen.org/privilege/contact.write is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact, \n @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile,
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial. \n\n
 * %http://tizen.org/privilege/callhistory.write is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log.
 *
 * @param[in]   record          The record handle
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
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 * @post 	contacts_db_changed_cb() callback wil be called upon success.
 *
 * @see contacts_connect()
 * @see contacts_db_insert_record()
 * @see contacts_db_delete_record()
 * @see contacts_db_get_record()
 */
int contacts_db_update_record(contacts_record_h record);

/**
 * @brief Deletes a record from the contacts database with related child records.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 * @privilege %http://tizen.org/privilege/callhistory.write
 *
 * @remarks %http://tizen.org/privilege/contact.write is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact, \n @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile,
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial. \n\n
 * %http://tizen.org/privilege/callhistory.write is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log.
 *
 * @param[in]   view_uri    The view URI of a record
 * @param[in]   record_id   The record ID to delete
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
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 * @post 	contacts_db_changed_cb() callback wil be called upon success.
 *
 * @see contacts_connect()
 * @see contacts_db_insert_record()
 */
int contacts_db_delete_record(const char *view_uri, int record_id);

/**
 * @brief Replaces an id-identified record with the given record.
 *
 * @details Now, this API supports only _contacts_contact view_uri.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @remarks The write-once value of @a record is not replaced.\n
 * This API works only for @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact
 *
 * @param[in]   record                The new record handle to replace
 * @param[in]   id                    The DB record ID to be replaced
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
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 * @post 	contacts_db_changed_cb() callback wil be called upon success.
 *
 * @see contacts_connect()
 * @see contacts_db_update_record()
 * @see contacts_db_delete_record()
 * @see contacts_db_get_record()
 */
int contacts_db_replace_record(contacts_record_h record, int id);

/**
 * @brief Retrieves all records and returns the results list.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *@privilege  %http://tizen.org/privilege/callhistory.read
 *
 * @remarks You must release @a record_list using contacts_list_destroy(). \n
 * %http://tizen.org/privilege/contact.read is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_simple_contact, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_sdn and all read-only views except views which are related to phone log. \n\n
 * %http://tizen.org/privilege/callhistory.read is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log_stat. \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_phone_log view is needed both privileges.
 *
 * @param[in]   view_uri        The view URI to get records
 * @param[in]   offset          The index from which to get results
 * @param[in]   limit           The number to limit results(value 0 is used for all records)
 * @param[out]  record_list     The record list
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
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre    contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_list_destroy()
 */
int contacts_db_get_all_records(const char *view_uri, int offset, int limit, contacts_list_h *record_list);

/**
 * @brief Uses a query to find records.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 * @privilege  %http://tizen.org/privilege/callhistory.read
 *
 * @remarks You must release @a record_list using contacts_list_destroy(). \n
 * %http://tizen.org/privilege/contact.read	is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_simple_contact, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_sdn and all read-only views except views which are related to phone log. \n\n
 * %http://tizen.org/privilege/callhistory.read	is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log_stat. \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_phone_log view is needed both privileges.
 *
 * @param[in]   query           The query to filter the results
 * @param[in]   offset          The index from which to get results
 * @param[in]   limit           The number to limit results(value 0 is used for get all records)
 * @param[out]  record_list     The record list
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
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre    contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_list_destroy()
 */
int contacts_db_get_records_with_query(contacts_query_h query, int offset, int limit, contacts_list_h *record_list);

/**
 * @brief Inserts multiple records to the contacts database.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 * @privilege %http://tizen.org/privilege/callhistory.write
 *
 * @remarks %http://tizen.org/privilege/contact.write is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact, \n @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile,
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial. \n\n
 * %http://tizen.org/privilege/callhistory.write is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log.
 *
 * @param[in]    record_list         The record list handle
 * @param[out]   ids                 The IDs of inserted records
 * @param[out]   count               The number of IDs
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
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 * @post 	contacts_db_changed_cb() callback wil be called upon success.
 *
 * @see contacts_connect()
 * @see contacts_db_update_records()
 * @see contacts_db_delete_records()
 */
int contacts_db_insert_records(contacts_list_h record_list, int **ids, int *count);

/**
 * @brief Updates multiple records in the contacts database.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 * @privilege %http://tizen.org/privilege/callhistory.write
 *
 * @remarks %http://tizen.org/privilege/contact.write is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact, \n @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile,
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial. \n\n
 * %http://tizen.org/privilege/callhistory.write is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log.
 *
 * @param[in]   record_list       The record list handle
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
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 * @post 	contacts_db_changed_cb() callback wil be called upon success.
 *
 * @see contacts_connect()
 * @see contacts_db_insert_records()
 * @see contacts_db_delete_records()
 */
int contacts_db_update_records(contacts_list_h record_list);

/**
 * @brief Deletes multiple records in the contacts database with related child records.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 * @privilege %http://tizen.org/privilege/callhistory.write
 *
 * @remarks %http://tizen.org/privilege/contact.write is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact, \n @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile,
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial. \n\n
 * %http://tizen.org/privilege/callhistory.write is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log.
 *
 * @param[in]   view_uri            The view URI of records
 * @param[in]   record_id_array     The record IDs to delete
 * @param[in]   count               The size of record ID array
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
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 * @post 	contacts_db_changed_cb() callback wil be called upon success.
 *
 * @see contacts_connect()
 * @see contacts_db_insert_records()
 * @see contacts_db_update_records()
 */
int contacts_db_delete_records(const char *view_uri, int record_id_array[], int count);

/**
 * @brief Replaces database records identified by given ids with a given record list.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.write
 *
 * @remarks The write-once value of record is not replaced.\n
 * This API works only for @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact
 *
 * @param[in]   list          			      The new record list handle to replace
 * @param[in]   record_id_array		The record IDs to replace
 * @param[in]   count				The size of record ID array
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
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 * @post 	contacts_db_changed_cb() callback wil be called upon success.
 *
 * @see contacts_connect()
 * @see contacts_db_update_record()
 * @see contacts_db_delete_record()
 * @see contacts_db_get_record()
 */
int contacts_db_replace_records(contacts_list_h list, int record_id_array[], int count);

/**
 * @brief Gets the current contacts database version.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 * @privilege %http://tizen.org/privilege/callhistory.read
 *
 * @param[out]  contacts_db_version    The contacts database version
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_db_get_changes_by_version()
 */
int contacts_db_get_current_version(int *contacts_db_version);

/**
 * @brief Registers a callback function to be invoked when a record changes.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 * @privilege %http://tizen.org/privilege/callhistory.read
 *
 * @remarks %http://tizen.org/privilege/contact.read is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_simple_contact, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_sdn, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group_relation.\n\n
 * %http://tizen.org/privilege/callhistory.read	is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log.
 * If successive change notification produced on the view_uri are identical,
 * then they are coalesced into a single notification if the older notification has not yet been called
 * because default main loop is doing something.
 * But, it means that a callback function is not called to reliably count of change.
 *
 * @param[in]   view_uri    The view URI of records whose changes are monitored
 * @param[in]   callback    The callback function to register
 * @param[in]   user_data   The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_SYSTEM              System error
 *
 * @pre		contacts_connect() should be called to open a connection to the contacts service.
 * @post		contacts_db_changed_cb() will be invoked when the designated view changes.
 *
 * @see contacts_connect()
 * @see contacts_db_changed_cb()
 * @see contacts_db_remove_changed_cb()
 */
int contacts_db_add_changed_cb(const char *view_uri, contacts_db_changed_cb callback, void *user_data);

/**
 * @brief Unregisters a callback function.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 * @param[in]   view_uri    The view URI of records whose changes are monitored
 * @param[in]   callback    The callback function to register
 * @param[in]   user_data   The user data to be passed to the callback function
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_SYSTEM              System error
 *
 * @pre		contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_db_changed_cb()
 * @see contacts_db_add_changed_cb()
 */
int contacts_db_remove_changed_cb(const char *view_uri, contacts_db_changed_cb callback, void *user_data);

/**
 * @brief Retrieves records changes since the given database version.
 *
 * @details This function will find all changed records since the given @a contacts_db_version. \n
 *          Now, support @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact_updated_info, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group_updated_info \n
 *          @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile_updated_info and @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_grouprel_updated_info.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @remarks You must release @a record_list using contacts_list_destroy().
 *
 * @param[in]   view_uri                    The view URI to get records
 * @param[in]   address_book_id             The address book ID to filter
 * @param[in]   contacts_db_version         The contacts database version
 * @param[out]  change_record_list          The record list
 * @param[out]  current_contacts_db_version The current contacts database version
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
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre    contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_list_destroy()
 */
int contacts_db_get_changes_by_version(const char *view_uri,
		int address_book_id,
		int contacts_db_version,
		contacts_list_h *change_record_list,
		int *current_contacts_db_version);

/**
 * @brief Finds records based on a given keyword.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @remarks You must release @a record_list using contacts_list_destroy(). \n
 * This API works only for @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_contact, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_grouprel, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_assigned \n
 * and @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_not_assigned.
 *
 * @param[in]   view_uri        The view URI to get records
 * @param[in]   keyword         The keyword
 * @param[in]   offset          The index from which to get results
 * @param[in]   limit           The number to limit results(value 0 is used for get all records)
 * @param[out]  record_list     The record list
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre    contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_list_destroy()
 */
int contacts_db_search_records(const char *view_uri, const char *keyword, int offset, int limit, contacts_list_h *record_list);

/**
 * @brief Finds records based on given query and keyword.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @remarks You must release @a record_list using contacts_list_destroy(). \n
 * This API works only for @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_contact, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_grouprel, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_assigned \n
 * and @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_not_assigned
 *
 * @param[in]   query           The query handle to filter
 * @param[in]   keyword         The keyword
 * @param[in]   offset          The index from which to get results
 * @param[in]   limit           The number to limit results(value 0 used for get all records)
 * @param[out]  record_list     The record list
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE       FS Full
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre    contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_list_destroy()
 */
int contacts_db_search_records_with_query(contacts_query_h query, const char *keyword, int offset, int limit, contacts_list_h *record_list);

/**
 * @brief Finds records based on a keyword and range.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @remarks You must release @a record_list using contacts_list_destroy(). \n
 * This API works only for @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_contact, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_grouprel, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_assigned, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_not_assigned. These views can search records with range @ref CONTACTS_SEARCH_RANGE_NAME, @ref CONTACTS_SEARCH_RANGE_NUMBER, @ref CONTACTS_SEARCH_RANGE_DATA. \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_number can search records with @ref CONTACTS_SEARCH_RANGE_NAME and @ref CONTACTS_SEARCH_RANGE_NUMBER.\n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_email can search records with @ref CONTACTS_SEARCH_RANGE_NAME and @ref CONTACTS_SEARCH_RANGE_EMAIL. \n
 *
 * @param[in]   view_uri        The view URI
 * @param[in]   keyword         The keyword
 * @param[in]   offset          The index from which to get results
 * @param[in]   limit           The number to limit results(value 0 is used for get all records)
 * @param[in]   range           The search range
 * @param[out]  record_list     The record list
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre    contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_list_destroy()
 */
int contacts_db_search_records_with_range(const char *view_uri, const char *keyword, int offset, int limit, int range, contacts_list_h *record_list);

/**
 * @brief Finds records based on given query and keyword.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @remarks You must release @a record_list using contacts_list_destroy(). \n
 * This API works only for @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_contact, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_grouprel, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_assigned \n
 * and @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_not_assigned
 *
 * @param[in]   view_uri        The view URI to get records
 * @param[in]   keyword         The keyword
 * @param[in]   offset          The index from which to get results
 * @param[in]   limit           The number to limit results(value 0 used for get all records)
 * @param[in]   start_match     The text which is inserted into the fragment before the keyword(If NULL, default is "[")
 * @param[in]   end_match       The text which is inserted into the fragment after the keyword(If NULL, default is "]")
 * @param[in]   token_number    The one side extra number of tokens near keyword(If negative value, full sentence is printed. e.g. if token number is 3 with 'abc' keyword, "my name is [abc]de and my home")
 * @param[out]  record_list     The record list
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre    contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_list_destroy()
 */
int contacts_db_search_records_for_snippet(const char *view_uri, const char *keyword, int offset, int limit, const char *start_match, const char *end_match, int token_number, contacts_list_h *record_list);

/**
 * @brief Finds records based on a keyword and range.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @remarks You must release @a record_list using contacts_list_destroy(). \n
 * This API works only for @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_contact, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_grouprel, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_assigned, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_not_assigned. These views can search records with range @ref CONTACTS_SEARCH_RANGE_NAME, @ref CONTACTS_SEARCH_RANGE_NUMBER, @ref CONTACTS_SEARCH_RANGE_DATA. \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_number can search records with @ref CONTACTS_SEARCH_RANGE_NAME and @ref CONTACTS_SEARCH_RANGE_NUMBER.\n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_email can search records with @ref CONTACTS_SEARCH_RANGE_NAME and @ref CONTACTS_SEARCH_RANGE_EMAIL. \n
 *
 * @param[in]   view_uri        The view URI
 * @param[in]   keyword         The keyword
 * @param[in]   offset          The index from which to get results
 * @param[in]   limit           The number to limit results(value 0 is used for get all records)
 * @param[in]   range           The search range
 * @param[in]   start_match     The text which is inserted into the fragment before the keyword(If NULL, default is "[")
 * @param[in]   end_match       The text which is inserted into the fragment after the keyword(If NULL, default is "]")
 * @param[in]   token_number    The one side extra number of tokens near keyword(If negative value, full sentence is printed. e.g. if token number is 3 with 'abc' keyword, "my name is [abc]de and my home")
 * @param[out]  record_list     The record list
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre    contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_list_destroy()
 */
int contacts_db_search_records_with_range_for_snippet(const char *view_uri, const char *keyword, int offset, int limit, int range, const char *start_match, const char *end_match, int token_number, contacts_list_h *record_list);

/**
 * @brief Finds records based on given query and keyword.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 *
 * @remarks You must release @a record_list using contacts_list_destroy(). \n
 * This API works only for @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_contact, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_grouprel, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_assigned \n
 * and @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_not_assigned
 *
 * @param[in]   query           The query handle to filter
 * @param[in]   keyword         The keyword
 * @param[in]   offset          The index from which to get results
 * @param[in]   limit           The number to limit results(value 0 used for get all records)
 * @param[in]   start_match     The text which is inserted into the fragment before the keyword(If NULL, default is "[")
 * @param[in]   end_match       The text which is inserted into the fragment after the keyword(If NULL, default is "]")
 * @param[in]   token_number    The one side extra number of tokens near keyword(If negative value, full sentence is printed. e.g. if token number is 3 with 'abc' keyword, "my name is [abc]de and my home")
 * @param[out]  record_list     The record list
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE       FS Full
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre    contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_list_destroy()
 */

int contacts_db_search_records_with_query_for_snippet(contacts_query_h query, const char *keyword, int offset, int limit, const char *start_match, const char *end_match, int token_number, contacts_list_h *record_list);

/**
 * @brief Gets the number of records in a specific view.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 * @privilege %http://tizen.org/privilege/callhistory.read
 *
 * @remarks %http://tizen.org/privilege/contact.read	is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_simple_contact, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_sdn and all read-only views except views which is related to phone log. \n\n
 * %http://tizen.org/privilege/callhistory.read is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log_stat. \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_phone_log view is needed both privilege.
 *
 * @param[in]   view_uri        The view URI
 * @param[out]  count           The count of records
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre    contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 */
int contacts_db_get_count(const char *view_uri, int *count);

/**
 * @brief Gets the number of records matching a query.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 * @privilege %http://tizen.org/privilege/callhistory.read
 *
 * @remarks %http://tizen.org/privilege/contact.read is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_simple_contact, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo, \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_sdn and all read-only views except views which is related to phone log. \n\n
 * %http://tizen.org/privilege/callhistory.read is needed for record which is related to @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log, @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log_stat. \n
 * @ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_phone_log view is needed both privilege.
 *
 * @param[in]   query           The query handle
 * @param[out]  count           The count of records
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_NO_DATA             Requested data does not exist
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 *
 * @pre    contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 */
int contacts_db_get_count_with_query(contacts_query_h query, int *count);

/**
 * @brief Gets the last successful changed contacts database version on the current connection.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/contact.read
 * @privilege %http://tizen.org/privilege/callhistory.read
 *
 * @param[out]  last_change_version    The database version
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see contacts_connect()
 * @see contacts_db_get_current_version()
 */
int contacts_db_get_last_change_version(int *last_change_version);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif /* __TIZEN_SOCIAL_CONTACTS_DB_H__ */

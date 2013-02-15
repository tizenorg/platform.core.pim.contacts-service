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
#ifndef __TIZEN_SOCIAL_CONTACTS_DB_H__
#define __TIZEN_SOCIAL_CONTACTS_DB_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_DATABASE_MODULE
 * @{
 */

typedef enum
{
	CONTACTS_CHANGE_INSERTED,	/**< . */
	CONTACTS_CHANGE_UPDATED,	/**< . */
	CONTACTS_CHANGE_DELETED,	/**< . */
//	CONTACTS_CHANGE_BULK_INSERTED,
//	CONTACTS_CHANGE_BULK_DELETED
} contacts_changed_e;

/**
 * @brief       Called when designated view changes.
 *
 * @param[in]   view_uri	The view uri
 * @param[in]   user_data	The user data passed from the callback registration function
 *
 * @see contacts_db_add_changed_cb()
 */
typedef void (*contacts_db_changed_cb)(const char* view_uri, void* user_data);

/**
 * @brief The callback function to get the result of insert batch operation.
 *
 * @param[in]   error     		Error code for batch operation
 * @param[in]   ids     		IDs of inserted records
 * @param[in]   count     	The number of ids
 * @param[in]   user_data	The user data passed from the batch operation
 *
 * @return  @c true to continue with the next iteration of the loop or @c false to break out of the loop.
 *
 * @pre contacts_db_insert_records() will invoke this callback.
 *
 * @see contacts_db_insert_records()
 */
typedef void (*contacts_db_insert_result_cb)( int error, int *ids, unsigned int count, void *user_data);

/**
 * @brief The callback function to get the result of batch operation.
 *
 * @param[in]   error     		Error code for batch operation
 * @param[in]   user_data		The user data passed from the batch operation
 *
 * @return  @c true to continue with the next iteration of the loop or @c false to break out of the loop.
 *
 * @pre contacts_db_update_records() will invoke this callback.
 *
 * @see contacts_db_update_records()
 */
typedef void (*contacts_db_result_cb)( int error, void *user_data);

/**
 * @brief   Inserts a record to the contacts database.
 *
 * @param[in]   record                 The record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB           		Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_update_record()
 * @see contacts_db_delete_record()
 * @see contacts_db_get_record()
 */
API int contacts_db_insert_record( contacts_record_h record, int *id );

/**
 * @brief   Gets a record from the contacts database.
 *
 * @details This function creates a new contact handle from the contacts database by the given @a record_id. \n
 * @a contact will be created, which is filled with contact information.
 *
 * @remarks  @a record must be released with contacts_record_destroy() by you.
 *
 * @param[in]   view_uri	The view URI of a record
 * @param[in]   record_id	The record ID to get from database
 * @param[out]  record		The record handle associated with the record ID
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_record_destroy()
 */
API int contacts_db_get_record( const char* view_uri, int record_id, contacts_record_h* record );

/**
 * @brief Updates a record to the contacts database.
 *
 * @param[in]   record          The record handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB       		    Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_insert_record()
 * @see contacts_db_delete_record()
 * @see contacts_db_get_record()
 */
API int contacts_db_update_record( contacts_record_h record );

/**
 * @brief Deletes a record from the contacts database.
 *
 * @param[in]   view_uri	The view URI of a record
 * @param[in]   record_id	The record ID to delete
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB		           Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_insert_record()
 */
API int contacts_db_delete_record( const char* view_uri, int record_id );

/**
 * @brief   Replace the record to the contacts database related to id.
 *
 * @remarks     @the write once value of record is not replaced.
 *
 * @param[in]   record                The new record handle to replace
 * @param[in]   id               	  The db record ID to replace
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB           		Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_update_record()
 * @see contacts_db_delete_record()
 * @see contacts_db_get_record()
 */
API int contacts_db_replace_record( contacts_record_h record, int id );

/**
 * @brief       Retrieves all record as a list
 *
 * @remarks     @a record_list must be released with contacts_list_destroy() by you.
 *
 * @param[in]   view_uri		The view URI to get records
 * @param[in]   offset			The index to get results from which index
 * @param[in]   limit			The number to limit results
 * @param[out]  record_list		The record list
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE				Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre    This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_list_destroy()
 */
API int contacts_db_get_all_records( const char* view_uri, int offset, int limit, contacts_list_h* record_list );

/**
 * @brief       Retrieves records with a query handle
 *
 * @remarks     @a record_list must be released with contacts_list_destroy() by you.
 *
 * @param[in]   query			The query handle to filter
 * @param[in]   offset			The index to get results from which index
 * @param[in]   limit			The number to limit results
 * @param[out]  record_list		The record list
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE				Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre    This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_list_destroy()
 */
API int contacts_db_get_records_with_query( contacts_query_h query, int offset, int limit, contacts_list_h* record_list );

/**
 * @brief   Inserts multiple records as batch operation to the contacts database.
 *
 * @param[in]   record_list			The record list handle
 * @param[out]   ids				IDs of inserted records
 * @param[out]   count				The number of ids
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB           		Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_update_records()
 * @see contacts_db_delete_records()
 * @see contacts_db_result_cb()
 */
API int contacts_db_insert_records( contacts_list_h record_list, int **ids, unsigned int *count);

/**
 * @brief   Inserts multiple records as batch operation to the contacts database.
 *
 * @param[in]   record_list			The record list handle
 * @param[in]   callback			The callback function to invoke which lets you know result of batch operation
 * @param[in]   user_data			The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB           		Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_insert_records()
 * @see contacts_db_update_records_async()
 * @see contacts_db_delete_records_async()
 * @see contacts_db_insert_result_cb()
 */
API int contacts_db_insert_records_async( contacts_list_h record_list, contacts_db_insert_result_cb callback, void *user_data);

/**
 * @brief   Updates multiple records as batch operation to the contacts database.
 *
 * @param[in]   record_list			The record list handle
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB           		Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_insert_records()
 * @see contacts_db_delete_records()
 * @see contacts_db_result_cb()
 */
API int contacts_db_update_records( contacts_list_h record_list);

/**
 * @brief   Updates multiple records as batch operation to the contacts database.
 *
 * @param[in]   record_list			The record list handle
 * @param[in]   callback			The callback function to invoke which lets you know result of batch operation
 * @param[in]   user_data			The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB           		Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_update_records()
 * @see contacts_db_insert_records_async()
 * @see contacts_db_delete_records_async()
 * @see contacts_db_result_cb()
 */
API int contacts_db_update_records_async( contacts_list_h record_list, contacts_db_result_cb callback, void *user_data);

/**
 * @brief   Deletes multiple records as batch operation to the contacts database.
 *
 * @param[in]   view_uri			The view URI of records
 * @param[in]   record_id_array		The record IDs to delete
 * @param[in]   count				The number of record ID array
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB           		Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_insert_records()
 * @see contacts_db_update_records()
 * @see contacts_db_result_cb()
 */
API int contacts_db_delete_records(const char* view_uri, int record_id_array[], int count);

/**
 * @brief   Deletes multiple records as batch operation to the contacts database.
 *
 * @param[in]   view_uri			The view URI of records
 * @param[in]   record_id_array		The record IDs to delete
 * @param[in]   count				The number of record ID array
 * @param[in]   callback			The callback function to invoke which lets you know result of batch operation
 * @param[in]   user_data			The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB           		Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_delete_records()
 * @see contacts_db_insert_records_async()
 * @see contacts_db_update_records_async()
 * @see contacts_db_result_cb()
 */
API int contacts_db_delete_records_async(const char* view_uri, int record_id_array[], int count, contacts_db_result_cb callback, void *user_data);

/**
 * @brief   Replace the record to the contacts database related to id.
 *
 * @param[in]   record          			      The new record list handle to replace
 * @param[in]   record_id_array		The record IDs to replace
 * @param[in]   count				The number of record ID array
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB           		Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_update_record()
 * @see contacts_db_delete_record()
 * @see contacts_db_get_record()
 */
API int contacts_db_replace_records( contacts_list_h list, int record_id_array[], unsigned int count );

/**
 * @brief   Replace the record to the contacts database related to id.
 *
 * @param[in]   record          			      The new record list handle to replace
 * @param[in]   record_id_array		The record IDs to replace
 * @param[in]   count				The number of record ID array
 * @param[in]   callback			The callback function to invoke which lets you know result of batch operation
 * @param[in]   user_data			The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_DB           		Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_replace_record()
 * @see contacts_db_update_record_async()
 * @see contacts_db_delete_records_async()
 * @see contacts_db_get_record()
 */
API int contacts_db_replace_records_async( contacts_list_h list, int record_id_array[], unsigned int count, contacts_db_result_cb callback, void *user_data );

/**
 * @brief	Gets the current contacts database version.
 *
 * @param[out]  contacts_db_version    The contacts database version
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE				Successful
 * @retval	#CONTACTS_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre     This function requires an open connection to the contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_get_changes_by_version()
 */
API int contacts_db_get_current_version( int* contacts_db_version );

/**
 * @brief       Registers a callback function to be invoked when the record changes.
 *
 * @param[in]   view_uri	The view URI of record to subscribe to changing notifications
 * @param[in]   callback	The callback function to register
 * @param[in]	user_data	The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval	#CONTACTS_ERROR_NONE                Successful
 * @retval	#CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @pre		This function requires an open connection to the contacts service by contacts_connect2().
 * @post	contacts_db_changed_cb() will be invoked when the designated view changes.
 *
 * @see contacts_connect2()
 * @see contacts_db_changed_cb()
 * @see contacts_db_remove_changed_cb()
 */
API int contacts_db_add_changed_cb( const char* view_uri, contacts_db_changed_cb callback, void* user_data );

/**
 * @brief       Unregisters a callback function.
 *
 * @param[in]   view_uri	The view URI of record to subscribe to changing notifications
 * @param[in]   callback	The callback function to register
 * @param[in]	user_data	The user data to be passed to the callback function
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval	#CONTACTS_ERROR_NONE                Successful
 * @retval	#CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 *
 * @pre		This function requires an open connection to the contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_db_changed_cb()
 * @see contacts_db_add_changed_cb()
 */
API int contacts_db_remove_changed_cb( const char* view_uri, contacts_db_changed_cb callback, void* user_data );

#ifndef _CONTACTS_NATIVE

typedef void (*contacts_db_change_cb_with_info)(const char* view_uri, char *changes, void* user_data);

API int contacts_db_add_changed_cb_with_info(const char* view_uri, contacts_db_change_cb_with_info callback, void* user_data);
API int contacts_db_remove_changed_cb_with_info(const char* view_uri, contacts_db_change_cb_with_info callback, void* user_data);
#endif

/**
 * @brief       Retrieves records with the contacts database version.
 *
 * @details		This function will find all changed records since the given @a contacts_db_version
 *
 * @remarks     @a change_record_list must be released with contacts_list_destroy() by you.
 *
 * @param[in]   view_uri					The view URI to get records
 * @param[in]   address_book_id				The address book ID to filter
 * @param[in]   contacts_db_version			The contacts database version
 * @param[out]  record_list					The record list
 * @param[out]  current_contacts_db_version	The current contacts database version
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE				Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre    This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_list_destroy()
 */
API int contacts_db_get_changes_by_version( const char* view_uri, int address_book_id, int contacts_db_version,
							contacts_list_h* change_record_list, int* current_contacts_db_version );

/**
 * @brief       Retrieves records with a keyword
 *
 * @remarks     @a record_list must be released with contacts_list_destroy() by you. \n
 * This API works only for \ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person and \ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_grouprel.
 *
 * @param[in]   view_uri			The view URI to get records
 * @param[in]   keyword			Thekeyword
 * @param[in]   offset			The index to get results from which index
 * @param[in]   limit			The number to limit results
 * @param[out]  record_list		The record list
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE				Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre    This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_list_destroy()
 */
API int contacts_db_search_records(const char* view_uri, const char *keyword, int offset, int limit, contacts_list_h* record_list);

/**
 * @brief       Retrieves records with a query handle and a keyword
 *
 * @remarks     @a record_list must be released with contacts_list_destroy() by you. \n
 * This API works only for \ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person and \ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_grouprel.
 *
 * @param[in]   query			The query handle to filter
 * @param[in]   keyword			Thekeyword
 * @param[in]   offset			The index to get results from which index
 * @param[in]   limit			The number to limit results
 * @param[out]  record_list		The record list
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE				Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre    This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 * @see contacts_list_destroy()
 */
API int contacts_db_search_records_with_query(contacts_query_h query, const char *keyword, int offset, int limit, contacts_list_h* record_list);

/**
 * @brief       Gets records count of a specific view
 *
 * @param[in]   view_uri		The view URI to get records
 * @param[out]  count			The count of records
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE				Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre    This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 */
API int contacts_db_get_count( const char* view_uri, int *count);

/**
 * @brief       Gets records count with a query handle
 *
 * @param[in]   query			The query handle to filter
 * @param[out]  count			The count of records
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE				Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval  #CONTACTS_ERROR_DB					Database operation failure
 *
 * @pre    This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see contacts_connect2()
 */
API int contacts_db_get_count_with_query( contacts_query_h query, int *count);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_DB_H__

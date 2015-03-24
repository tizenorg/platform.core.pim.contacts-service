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
#ifndef __TIZEN_SOCIAL_CONTACTS_PHONELOG_INTERNAL_H__
#define __TIZEN_SOCIAL_CONTACTS_PHONELOG_INTERNAL_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_phone_log_internal.h
 */

/**
 * @brief Enumeration for Contacts phone log delete flags.
 */
typedef enum {
	CONTACTS_PHONE_LOG_DELETE_BY_ADDRESS,		/**< Delete by address */
	CONTACTS_PHONE_LOG_DELETE_BY_MESSAGE_EXTRA_DATA1,	/**< Delete by message extra_data1 */
	CONTACTS_PHONE_LOG_DELETE_BY_EMAIL_EXTRA_DATA1,	/**< Delete by email extra_data1  */
} contacts_phone_log_delete_e;

/**
 * @brief Deletes a phone log with extra_data1.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 * @privlevel public
 * @privilege %http://tizen.org/privilege/callhistory.write
 *
 * @param[in]	op 				Operation #contacts_phone_log_delete_e
 * @param[in]	address (optional)	Address to delete (number, email,  etc)
 * @param[in]	extra_data1 (optional)	Extra_data1 to delete
 *
 * @return  @c 0 on sucess,
 *          otherwise a negative error value (#contacts_error_e)
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE       FS Full
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY	      Out of memory
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 * @retval  #CONTACTS_ERROR_IPC	               IPC error * @par example
 * @code
 contacts_phone_log_delete(CONTACTS_PHONE_LOG_DELETE_BY_ADDRESS, "0123456789");
 contacts_phone_log_delete(CONTACTS_PHONE_LOG_DELETE_BY_MESSAGE_EXTRA_DATA1,  2);
 contacts_phone_log_delete(CONTACTS_PHONE_LOG_DELETE_BY_EMAIL_EXTRA_DATA1,  1);
 * @endcode
 */
int contacts_phone_log_delete(contacts_phone_log_delete_e op, ...);

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_PHONELOG_INTERNAL_H__

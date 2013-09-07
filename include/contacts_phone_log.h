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
#ifndef __TIZEN_SOCIAL_CONTACTS_PHONELOG_H__
#define __TIZEN_SOCIAL_CONTACTS_PHONELOG_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_PHONELOG_MODULE
 * @{
 */

/**
 * @brief	Resets a phonelog's count.
 * @details The count of all type of phonelog will be 0.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE	Successful
 * @retval  #CONTACTS_ERROR_DB	Database operation failure
 *
 * @pre     This function requires an open connection to contacts service by contacts_connect2().
 *
 * @see  contacts_connect2()
 */
API int contacts_phone_log_reset_statistics(void);

typedef enum {
	CONTACTS_PHONE_LOG_DELETE_BY_ADDRESS,		/**< . */
	CONTACTS_PHONE_LOG_DELETE_BY_MESSAGE_EXTRA_DATA1,	/**< . */
	CONTACTS_PHONE_LOG_DELETE_BY_EMAIL_EXTRA_DATA1,	/**< . */
} contacts_phone_log_delete_e;

/**
 * @brief	Delete phone log with extra_data1
 *
 * @param[in]	op 				operation #contacts_phone_log_delete_e
 * @param[in]	address (optional)	Address to delete (number, email,  etc)
 * @param[in]	extra_data1 (optional)	extra_data1 to delete
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_INVALID_PARAMETER   Invalid parameter
  * @par example
 * @code
 contacts_phone_log_delete(CONTACTS_PHONE_LOG_DELETE_BY_ADDRESS, "0123456789");
 contacts_phone_log_delete(CONTACTS_PHONE_LOG_DELETE_BY_MESSAGE_EXTRA_DATA1,  2);
 contacts_phone_log_delete(CONTACTS_PHONE_LOG_DELETE_BY_EMAIL_EXTRA_DATA1,  1);
 * @endcode
 */
API int contacts_phone_log_delete(contacts_phone_log_delete_e op, ...);

/**
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_PHONELOG_H__

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
#ifndef __TIZEN_SOCIAL_CONTACTS_SERVICE_H__
#define __TIZEN_SOCIAL_CONTACTS_SERVICE_H__

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

/**
 * @brief	Connects to the contacts service.
 *
 * @remarks Connection opening is necessary to access the contacts database such as fetching, inserting, or updating records.\n
 * The execution of contacts_connect2() and contacts_disconnect2() could slow down your application so you are recommended not to call them frequently.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE	Successful
 * @retval  #CONTACTS_ERROR_DB	Database operation failure
 *
 * @see  contacts_disconnect2()
 */
API int contacts_connect2();

/**
 * @brief	Disconnects from the contacts service.
 *
 * @remarks	If there is no opened connection, this function returns #CONTACTS_ERROR_DB.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE	Successful
 * @retval  #CONTACTS_ERROR_DB	Database operation failure
 *
 * @see contacts_connect2()
 */
API int contacts_disconnect2();



/**
 * @brief	Connects to the contacts service with another connection for thread.
 *
 * @remarks
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE	Successful
 * @retval  #CONTACTS_ERROR_DB	Database operation failure
 *
 * @see  contacts_disconnect_on_thread()
 */
API int contacts_connect_on_thread();

/**
 * @brief	Disconnects from the contacts service.
 *
 * @remarks
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE	Successful
 * @retval  #CONTACTS_ERROR_DB	Database operation failure
 *
 * @see contacts_connect_on_thread()
 */
API int contacts_disconnect_on_thread();


#define CONTACTS_CONNECT_FLAG_RETRY	0x00000001
#define CONTACTS_CONNECT_FLAG_NONE	0

/**
 * @brief	Connects to the contacts service. If connection is failed because contacts-service is not running, it will retry for several seconds
 *
 * @remarks Connection opening is necessary to access the contacts database such as fetching, inserting, or updating records.\n
 * The execution of contacts_connect2() and contacts_disconnect2() could slow down your application so you are recommended not to call them frequently.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #CONTACTS_ERROR_NONE	Successful
 * @retval  #CONTACTS_ERROR_DB	Database operation failure
 *
 * @see  contacts_disconnect2()
 */
API int contacts_connect_with_flags(unsigned int flags);



/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_SERVICE_H__

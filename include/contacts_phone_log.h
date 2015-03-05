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
 * @file contacts_phone_log.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_PHONELOG_MODULE Phone log
 *
 * @brief The contacts phone log API provides the set of definitions and interfaces that enable application developers to reset phone log count.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_PHONELOG_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_PHONELOG_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 *  - http://tizen.org/feature/network.telephony\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="../org.tizen.mobile.native.appprogramming/html/ide_sdk_tools/feature_element.htm"><b>Feature Element</b>.</a>
 *
 * <BR>
 * @{
 */


/**
 * @brief   Resets the phone log's count.
 * @details The number of all types in the phone log will be @c 0.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/callhistory.write
 *
 * @return  @c 0 on success,
 *          otherwise a negative error value
 *
 * @retval  #CONTACTS_ERROR_NONE                Successful
 * @retval  #CONTACTS_ERROR_OUT_OF_MEMORY       Out of memory
 * @retval  #CONTACTS_ERROR_FILE_NO_SPACE       FS Full
 * @retval  #CONTACTS_ERROR_DB                  Database operation failure
 * @retval  #CONTACTS_ERROR_IPC                 IPC error
 * @retval  #CONTACTS_ERROR_PERMISSION_DENIED   Permission denied. This application does not have the privilege to call this method.
 * @retval  #CONTACTS_ERROR_NOT_SUPPORTED       Not supported
 *
 * @pre     contacts_connect() should be called to open a connection to the contacts service.
 *
 * @see  contacts_connect()
 */
int contacts_phone_log_reset_statistics(void);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_PHONELOG_H__

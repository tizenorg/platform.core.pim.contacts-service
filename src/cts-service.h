/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjae Shin <yj99.shin@samsung.com>
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
#ifndef __CTS_SERVICE_H__
#define __CTS_SERVICE_H__

//<!--
/**
 * This function connect to contacts service.
 * \n Though the connection already exists, #CTS_SUCCESS is returned.
 * \n It has to disconnect as it connect.
 *
 * for example, if you connect 3 times you have to disconnect 3 times.
 * \n To disconnect early minimizes the runtime resource consumption.
 * On the other hand, a pair of connection and disconnection is expensive.
 * Don't call frequently.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @see contacts_svc_disconnect()
 */
int contacts_svc_connect(void);

/**
 * This function disconnect to contacts service.
 * If connection is called many times,
 * disconnection operation is valid at the last of the times.
 *
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @see contacts_svc_connect()
 */
int contacts_svc_disconnect(void);
//-->

#endif //__CTS_SERVICE_H__

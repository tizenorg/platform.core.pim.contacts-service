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
#ifndef __CTS_IM_H__
#define __CTS_IM_H__
//<!--
/**
 * A kind of status for instant messaging
 * @see contacts_svc_set_im_status()
 */
typedef enum
{
	CTS_IM_STATUS_NONE=0,
	CTS_IM_STATUS_OFFLINE,
	CTS_IM_STATUS_BUSY,
	CTS_IM_STATUS_AWAY,
	CTS_IM_STATUS_ONLINE,
}cts_im_status;

/**
 * This is the signature of a callback function added with contacts_svc_get_im_status().
 * \n The callback function is invoked when the status of IM(Instant Messaging) is got.
 * @param[in] index The index of contact or im information
 * @param[in] stat #cts_im_status
 * @param[in] user_data The data which is set by contacts_svc_get_im_status()
 */
typedef void (*cts_im_callback_fn)(int index, cts_im_status stat, void* user_data);

/**
 * Use for contacts_svc_get_im_status().
 */
typedef enum{
	CTS_IM_STATUS, /**< Status for A Instant Messaging ID */
	CTS_IM_CONTACT_STATUS /**< Status for a contact ID  */
}cts_get_im_op;

/**
 * This function gets status of IM by op_code(CTS_IM_STATUS, CTS_IM_CONTACT_STATUS).
 * #search_id is related to op_code.
 * For #CTS_IM_STATUS, search_id is a id of IM information
 * For #CTS_IM_CONTACT_STATUS, search_id is a id of contact
 * @param[in] op_code #cts_get_im_op
 * @param[in] search_id index for searching.
 * @param[in] cb callback function(#cts_im_callback_fn)
 * @param[in] user_data callback data
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_get_im_status(cts_get_im_op op_code, int search_id,
		cts_im_callback_fn cb, void *user_data);

typedef enum{
	CTS_IM_TYPE_NONE, /**< Others */
	CTS_IM_TYPE_GOOGLE,
	CTS_IM_TYPE_WLM,
	CTS_IM_TYPE_YAHOO,
	CTS_IM_TYPE_FACEBOOK,
	CTS_IM_TYPE_ICQ,
	CTS_IM_TYPE_AIM,
	CTS_IM_TYPE_QQ,
	CTS_IM_TYPE_JABBER,
	CTS_IM_TYPE_SKYPE,
	CTS_IM_TYPE_IRC,
}cts_im_type;

/**
 * This function sets status of IM.
 * @param[in] type The type of IM.(#cts_im_type)
 * @param[in] im_id The user ID of IM
 * @param[in] status status of IM to be set.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_set_im_status(cts_im_type type, const char *im_id, cts_im_status status);
//-->
#endif //__CTS_IM_H__

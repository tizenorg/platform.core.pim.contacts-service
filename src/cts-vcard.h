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
#ifndef __CTS_VCARD_H__
#define __CTS_VCARD_H__

enum{
	CTS_VCARD_IMG_NONE,
	CTS_VCARD_IMG_JPEG,
	CTS_VCARD_IMG_PNG,
	CTS_VCARD_IMG_GIF,
	CTS_VCARD_IMG_TIFF,
};

/**
 * content type
 */
enum VCARDCONTENT {
	CTS_VCARD_CONTENT_NONE = 0,
	CTS_VCARD_CONTENT_BASIC = 1<<0,
	CTS_VCARD_CONTENT_X_SLP_GROUP = 1<<1,
	CTS_VCARD_CONTENT_ALL = 1<<0|1<<1,
};

//<!--
/**
 * This function makes contact record by using vcard file stream.
 *
 * @param[in] vcard_stream start point of the stream of vcard.
 * @param[out] contact Points of the contact record which is returned
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_get_contact_from_vcard(const char *vcard_stream, CTSstruct **contact);

/**
 * This function makes vcard file stream by using contact record.
 *
 * @param[in] contact A contact information
 * @param[out] vcard_stream start point of the stream of vcard.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_get_vcard_from_contact(const CTSstruct *contact, char **vcard_stream);

/**
 * This function inserts a contact made from vcard file stream.
 *
 * @param[in] addressbook_id The index of addressbook. 0 is local(phone internal)
 * @param[in] a_vcard_stream start point of the stream of vcard.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_insert_vcard(int addressbook_id, const char* a_vcard_stream);

/**
 * This function replaces a saved contact made from vcard file stream.
 * \n If index(ex. LUID) exist, it is invalid. Always contact_id is valid and processed.
 * If the contact related with contact_id is not existed, return error.
 *
 * @param[in] contact_id The related index of contact.
 * @param[in] a_vcard_stream start point of the stream of vcard.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_replace_by_vcard(int contact_id, const char* a_vcard_stream);

/**
 * This function calls handling function for each vcard of vcard file.
 *
 * @param[in] vcard_file_name the name of vcard file
 * @param[in] fn function pointer for handling each vcard stream.
 *               If this function doesn't return #CTS_SUCCESS, this function is terminated.
 * @param[in] data data which is passed to callback function
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_vcard_foreach(const char *vcard_file_name,
		int (*fn)(const char *a_vcard_stream, void *data), void *data);

/**
 * This function gets count of vcard in the file.
 *
 * @param[in] vcard_file_name the name of vcard file
 * @return The count number on success, Negative value(#cts_error) on error
 */
int contacts_svc_vcard_count(const char *vcard_file_name);

/**
 * This function puts vcard content.
 * If vcard stream has vcards, this function puts new content into the last vcard.
 * you should free new vcard stream after using.
 * \n This should be used for extended type("X-").
 *
 * @param[in] vcard_stream start point of the stream of vcard.
 * @param[in] content_type The type of vcard content
 * @param[in] content_value The value of vcard content
 * @return new vcard stream on success, NULL on error
 */
char* contacts_svc_vcard_put_content(const char *vcard_stream,
		const char *content_type, const char *content_value);

/**
 * This function gets values of vcard content.
 * The each value will be passed to fn function.
 * \n This should be used for extended type("X-").
 *
 * @param[in] vcard_stream start point of the stream of vcard.
 * @param[in] content_type The type of vcard content
 * @param[in] fn function pointer for handling each content_value.
 *               If this function doesn't return #CTS_SUCCESS, this function is terminated.
 * @param[in] data data which is passed to callback function
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_vcard_get_content(const char *vcard_stream,
		const char *content_type, int (*fn)(const char *content_value, void *data), void *data);

//-->

#endif //__CTS_VCARD_H__


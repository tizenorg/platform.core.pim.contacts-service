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
#ifndef __CTS_ADDRESSBOOK_H__
#define __CTS_ADDRESSBOOK_H__

/**
 * system addressbook id
 */

enum ADDRESSBOOK{
	CTS_ADDRESSBOOK_INTERNAL,
	CTS_ADDRESSBOOK_START,
};

#ifndef __CONTACTS_SVC_H__


//<!--
/**
 * @defgroup   CONTACTS_SVC_ADDRESSBOOK Addressbook Modification
 * @ingroup    CONTACTS_SVC
 * @addtogroup CONTACTS_SVC_ADDRESSBOOK
 * @{
 *
 * This interface provides methods to insert/update/delete the addressbook.
 *
 * - getting all addressbook (0 is logical value for internal addressbook)
 * @code
 void addrbook_list(void)
 {
	 int ret, count;
	 CTSiter *iter;

	 count = contacts_svc_count_with_int(CTS_GET_COUNT_CONTACTS_IN_ADDRESSBOOK, 0);
	 printf("Phone(%d)", count);

	 ret = contacts_svc_get_list(CTS_LIST_ALL_ADDRESSBOOK, &iter);
	 if (CTS_SUCCESS != ret) {
		 printf("contacts_svc_get_list() Failed(%d)\n", ret);
		 return;
	 }

	 while (CTS_SUCCESS == contacts_svc_iter_next(iter)) {
		 int id;
		 const char *name;
		 CTSvalue *info;

		 info = contacts_svc_iter_get_info(iter);
		 id = contacts_svc_value_get_int(info, CTS_LIST_ADDRESSBOOK_ID_INT);
		 name = contacts_svc_value_get_str(info, CTS_LIST_ADDRESSBOOK_NAME_STR);
		 count = contacts_svc_count_with_int(CTS_GET_COUNT_CONTACTS_IN_ADDRESSBOOK, id);

		 printf("%s(%d)", name, count);
	 }
	 contacts_svc_iter_remove(iter);
 }
 * @endcode
 *
 */

/**
 * addressbook permission
 */
enum ADDRESSBOOKPERMISSION {
	CTS_ADDRESSBOOK_MODE_NONE, /**< .*/
	CTS_ADDRESSBOOK_MODE_READONLY, /**< .*/
};

/**
 * This function inserts a addressbook information into database.
 * The implementation assigns an index number of the addressbook automatically.
 * \n The returned index is unique and non-reusable.
 *
 * @param[in] addressbook A addressbook information of #CTSvalue created by contacts_svc_value_new(CTS_VALUE_ADDRESSBOOK).
 * @return the index of contact on success, Negative value(#cts_error) on error
 * @par example
 * @code
 int insert_addrbook(void)
 {
    int ret;
    CTSvalue *ab;
    ab = contacts_svc_value_new(CTS_VALUE_ADDRESSBOOK);

    contacts_svc_value_set_int(ab, CTS_ADDRESSBOOK_VAL_ACC_ID_INT, 1);
    contacts_svc_value_set_int(ab, CTS_ADDRESSBOOK_VAL_ACC_TYPE_INT, CTS_ADDRESSBOOK_TYPE_GOOGLE);
    contacts_svc_value_set_int(ab, CTS_ADDRESSBOOK_VAL_MODE_INT, CTS_ADDRESSBOOK_MODE_NONE);
    contacts_svc_value_set_str(ab, CTS_ADDRESSBOOK_VAL_NAME_STR, "test1");

    ret = contacts_svc_insert_addressbook(ab);
    if(ret < CTS_SUCCESS)
       printf("contacts_svc_insert_addressbook() Failed\n");

    contacts_svc_value_free(ab);
    return ret;
 }
 * @endcode
 */
int contacts_svc_insert_addressbook(CTSvalue *addressbook);

/**
 * This function deletes the addressbook information related to addressbook_id.
 * Also, the related contacts and groups are deleted.
 * @param[in] addressbook_id index of addressbook
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_delete_addressbook(int addressbook_id);

/**
 * This function updates a addressbook information into database.
 *
 * @param[in] addressbook A addressbook information of #CTSvalue created by contacts_svc_get_addressbook().
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void update_addrbook(void)
 {
    int ret;
    CTSvalue *ab = NULL;
    ret = contacts_svc_get_addressbook(2, &ab);
    if(CTS_SUCCESS != ret) {
       printf("contacts_svc_get_addressbook() Failed\n");
       return;
    }

    contacts_svc_value_set_str(ab, CTS_ADDRESSBOOK_VAL_NAME_STR,"Fixed-addressbook");

    ret = contacts_svc_update_addressbook(ab);
    if(ret < CTS_SUCCESS)
       printf("contacts_svc_update_addressbook() Failed\n");

    contacts_svc_value_free(ab);
 }
 * @endcode
 */
int contacts_svc_update_addressbook(CTSvalue *addressbook);

/**
 * This function gets a addressbook record which has the index from the database.
 * Obtained addressbook record should be free using by contacts_svc_value_free().
 * @param[in] addressbook_id The index of addressbook to get
 * @param[out] ret_value Points of the addressbook record which is returned
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void get_addrbook(int addressbook_id)
 {
    int ret;
    const char *name;
    CTSvalue *ab = NULL;

    ret = contacts_svc_get_addressbook(addressbook_id, &ab);
    if(CTS_SUCCESS != ret) {
       printf("contacts_svc_get_addressbook() Failed\n");
       return;
    }

    printf("///////////%d//////////////\n",
          contacts_svc_value_get_int(ab, CTS_ADDRESSBOOK_VAL_ID_INT));
    printf("The related account ID : %d\n",
          contacts_svc_value_get_int(ab, CTS_ADDRESSBOOK_VAL_ACC_ID_INT));
    printf("The related account type : %d\n",
          contacts_svc_value_get_int(ab, CTS_ADDRESSBOOK_VAL_ACC_TYPE_INT));
    printf("permission : %d\n",
          contacts_svc_value_get_int(ab, CTS_ADDRESSBOOK_VAL_MODE_INT));

    name = contacts_svc_value_get_str(ab, CTS_ADDRESSBOOK_VAL_NAME_STR);
    if(name)
       printf("Name : %s\n", name);
    printf("//////////////////////////\n");

   contacts_svc_value_free(ab);
 }
 * @endcode
 */
int contacts_svc_get_addressbook(int addressbook_id, CTSvalue **ret_value);

/**
 * @}
 */
//-->
#endif //__CONTACTS_SVC_H__
#endif //__CTS_ADDRESSBOOK_H__

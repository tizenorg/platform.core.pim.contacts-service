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
#ifndef __CTS_GROUP_H__
#define __CTS_GROUP_H__

int cts_group_set_relation(int group_id, int contact_id, int contact_acc);
int cts_group_unset_relation(int group_id, int contact_id);

/**
 * This function gets index of group found by name.
 * @param[in] addressbook_id The index of addressbook. 0 is local(phone internal)
 * @param[in] name The group name for searching
 * @return index of found group on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void get_group(void)
 {
    int index, ret=-1;
    CTSvalue *group = NULL;

    index = contacts_svc_find_group(0, "Family");
    if(index > CTS_SUCCESS)
       ret = contacts_svc_get_group(index, &group);
    if(ret < CTS_SUCCESS) {
         printf("contacts_svc_get_group() Failed");
         return;
    }
 }
 * @endcode
 */
int contacts_svc_find_group(int addressbook_id, const char *name);


//<!--
/**
 * @defgroup   CONTACTS_SVC_GROUP Group information
 * @ingroup    CONTACTS_SVC
 * @addtogroup CONTACTS_SVC_GROUP
 * @{
 *
 * This interface provides methods for group information.
 *
 */

/**
 * This function sets the relationship between a group and a contact.
 * It is low level api. It is recommanded to use contacts_svc_update_contact()
 * \n The contact and the group must have the same addressbook index.
 * (It is not checked internally for performance.)
 *
 * @param[in] group_id Index of group record
 * @param[in] contact_id Index of contact record to add to group
 * @return the index of group on success, Negative value(#cts_error) on error
 * @par example
 * @code

 * @endcode
 */
int contacts_svc_group_set_relation(int group_id, int contact_id);

/**
 * This function removes relation between a group and a contact.
 * It is low level api. It is recommanded to use contacts_svc_update_contact()
 *
 * @param[in] group_id Index of group record
 * @param[in] contact_id Index of contact record to add to group
 * @return the index of group on success, Negative value(#cts_error) on error
 * @par example
 * @code

 * @endcode
 */
int contacts_svc_group_unset_relation(int group_id, int contact_id);

/**
 * This function inserts a group into database.
 * This api assigns a index of the group automatically.
 * \n The returned index is unique & non-reusable.
 *
 * @param[in] addressbook_id The index of addressbook. 0 is local(phone internal)
 * @param[in] group A group information of CTSvalue() created by contacts_svc_value_new(CTS_VALUE_GROUP).
 * @return the index of group on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void insert_group(const char *group_name)
 {
    int ret;
    CTSvalue *group;
    group = contacts_svc_value_new(CTS_VALUE_GROUP);

    contacts_svc_value_set_str(group, CTS_GROUP_VAL_NAME_STR, group_name);
    contacts_svc_value_set_str(group, CTS_GROUP_VAL_RINGTONE_STR,"/tmp/test.mp3");

    ret = contacts_svc_insert_group(0, group);
    if(ret < CTS_SUCCESS)
       printf("contacts_svc_insert_group() Failed\n");

    contacts_svc_value_free(group);
 }
 * @endcode
 */
int contacts_svc_insert_group(int addressbook_id, CTSvalue *group);

/**
 * This function updates a group in the database.
 *
 * @param[in] group A group information of #CTSvalue.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void update_group(void)
 {
    int ret;
    CTSvalue *group = NULL;
    ret = contacts_svc_get_group(2, &group);
    if(CTS_SUCCESS != ret) {
         printf("contacts_svc_get_group() Failed");
         return;
    }

    contacts_svc_value_set_str(group, CTS_GROUP_VAL_NAME_STR,"Fix-Friends");
    contacts_svc_value_set_str(group, CTS_GROUP_VAL_RINGTONE_STR,"/tmp/change.mp3");

    //free("Fix-Friends");
    //free("/tmp/change.mp3");
    ret = contacts_svc_update_group(group);
    if(ret < CTS_SUCCESS)
       printf("contacts_svc_update_group() Failed\n");

    contacts_svc_value_free(group);
 }
 * @endcode
 */
int contacts_svc_update_group(CTSvalue *group);

/**
 * This function deletes a group in database.
 *
 * @param[in] index The index of group to delete in database.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void delete_group(void)
 {
    int ret;
    ret = contacts_svc_delete_group(3);
    if(CTS_SUCCESS != ret)
       printf("Error : contacts_svc_delete_group() Failed(%d)", ret);
 }
 * @endcode
 */
int contacts_svc_delete_group(int index);

/**
 * This function deletes contacts inclued the group and the group in database.
 * But if the contact has another group, it deletes the group relations only.
 *
 * @param[in] index The index of group to delete in database.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void delete_group_members(void)
 {
    int ret;
    ret = contacts_svc_delete_group_with_members(3);
    if(CTS_SUCCESS != ret)
       printf("Error : contacts_svc_delete_group_with_members() Failed(%d)", ret);
 }
 * @endcode
 */
int contacts_svc_delete_group_with_members(int index);

/**
 * This function gets a group record which has the index from the database.
 * Obtained group record should be free using by contacts_svc_value_free().
 * @param[in] index The index of group to get
 * @param[out] retgroup Points of the group record which is returned
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void get_group(void)
 {
    int ret;
    CTSvalue *group = NULL;
    ret = contacts_svc_get_group(2, &group);
    if(CTS_SUCCESS != ret) {
         printf("contacts_svc_get_list() Failed");
         return;
    }

    printf("Account ID : %d\n",
             contacts_svc_value_get_int(group, CTS_GROUP_VAL_ADDRESSBOOK_ID_INT));
    printf("Name : %s\n",
       contacts_svc_value_get_str(group, CTS_GROUP_VAL_NAME_STR));
    if(contacts_svc_value_get_str(group, CTS_GROUP_VAL_RINGTONE_STR))
       printf("ringtone : %s\n",
          contacts_svc_value_get_str(group, CTS_GROUP_VAL_RINGTONE_STR));
 }
 * @endcode
 */
int contacts_svc_get_group(int index, CTSvalue **retgroup);

/**
 * @}
 */
//-->
#endif //__CTS_GROUP_H__


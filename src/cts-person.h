/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
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
#ifndef __CONTACTS_PERSON_H__
#define __CONTACTS_PERSON_H__

#include "cts-contact.h"

int cts_insert_person(int contact_id, int outgoing_cnt);
int cts_delete_person(int index);
int cts_person_garbagecollection(void);

int cts_person_change_primary_contact(int person_id);

enum {
	CTS_LINKED_NONE,
	CTS_LINKED_PRIMARY,
	CTS_LINKED_SECONDARY,
};
int cts_check_linked_contact(int contact_id);

//<!--
/**
 * @defgroup   CONTACTS_SVC_PERSON Person
 * @ingroup    CONTACTS_SVC
 * @addtogroup CONTACTS_SVC_PERSON
 * @{
 *
 * This interface provides methods for linking.
 *
 * The person means linked contacts.\n
 * If contact is not linked, the related person is same with contact.\n
 * So contacts_svc_get_person() and contacts_svc_get_person_value return same value with
 * contacts_svc_get_contact() and contacts_svc_get_contact_value().
 */

/**
 * This function links a sub_person to base_person.
 *
 * @param[in] base_person_id The index of base person
 * @param[in] sub_person_id The index of sub person
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_link_person(int base_person_id, int sub_person_id);

/**
 * This function unlinks a contact to person.
 *
 * @param[in] person_id The index of person
 * @param[in] contact_id The index of contact to unlink
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_unlink_person(int person_id, int contact_id);

/**
 * This function gets person record which has the index from the database.
 * If person has linked contacts, this function return merged record;
 * Obtained person record should be freed by using contacts_svc_struct_free().
 *
 * @param[in] index The index of person to get
 * @param[out] person Points of the person record which is returned
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void get_person(void)
 {
    int ret=-1;
    CTSstruct *person = NULL;
    CTSvalue *name;
    GSList *get_list, *cursor;

    ret = contacts_svc_get_person(1, &person);
    if(ret < 0)
    {
       printf("No found record\n");
       return;
    }

    contacts_svc_struct_get_value(person, CTS_CF_NAME_VALUE, &name);
    printf("First Name : %s\n", contacts_svc_value_get_str(name, CTS_NAME_VAL_FIRST_STR));
    printf("Last Name : %s\n", contacts_svc_value_get_str(name, CTS_NAME_VAL_LAST_STR));
    printf("Additional Name : %s\n", contacts_svc_value_get_str(name, CTS_NAME_VAL_ADDITION_STR));
    printf("Display Name : %s\n", contacts_svc_value_get_str(name, CTS_NAME_VAL_DISPLAY_STR));
    printf("Prefix Name : %s\n", contacts_svc_value_get_str(name, CTS_NAME_VAL_PREFIX_STR));
    printf("Suffix Name : %s\n", contacts_svc_value_get_str(name, CTS_NAME_VAL_SUFFIX_STR));

    get_list = NULL;
    contacts_svc_struct_get_list(person, CTS_CF_NUMBER_LIST, &get_list);

    cursor = get_list;
    for(;cursor;cursor=g_slist_next(cursor))
    {
       printf("number Type = %d",
          contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_TYPE_INT));

       printf("Number = %s\n",
          contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
    }

    get_list = NULL;
    contacts_svc_struct_get_list(person, CTS_CF_EMAIL_LIST, &get_list);

    cursor = get_list;
    for(;cursor;cursor=g_slist_next(cursor))
    {
       printf("email Type = %d",
          contacts_svc_value_get_int(cursor->data, CTS_EMAIL_VAL_TYPE_INT));

       printf("email = %s\n",
          contacts_svc_value_get_str(cursor->data, CTS_EMAIL_VAL_ADDR_STR));
    }

    contacts_svc_struct_free(person);
 }
 * @endcode
 */
int contacts_svc_get_person(int person_id, CTSstruct **person);

/**
 * This function gets index of person related with user_data.
 * index is found by op_code with user_data related with op_code(#cts_find_op).
 * @param[in] op_code #cts_find_op
 * @param[in] user_data The parameter for searching
 * @return index of found person on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void get_person(void)
 {
    int index, ret=-1;
    CTSstruct *person = NULL;

    index = contacts_svc_find_person_by(CTS_FIND_BY_NUMBER, "0123456789");
    if(index > CTS_SUCCESS) {
       ret = contacts_svc_get_person(index, &person);
       if(ret < 0) {
          printf("No found record\n");
          return;
       }
    }
 }
 * @endcode
 */
int contacts_svc_find_person_by(cts_find_op op_code, const char *user_data);

/**
 * Use for contacts_svc_get_person_value().
 */
typedef enum {
	CTS_GET_PERSON_NAME_VALUE, /**< Use #NAMEVALUE */
	CTS_GET_PERSON_DEFAULT_NUMBER_VALUE,/**< related with person id. Use #NUMBERVALUE */
	CTS_GET_PERSON_DEFAULT_EMAIL_VALUE,/**< related with person id. Use #EMAILVALUE */
}cts_get_person_val_op;
/**
 * This function can get a value data related with person_id and op_code.
 * The value data is decided by op_code(#cts_get_person_val_op)
 * The gotten value is readonly.
 * Obtained record should be freed by using contacts_svc_value_free().
 * @return #CTS_SUCCESS, Negative value(#cts_error) on error
 * @param[in] op_code #cts_get_person_val_op
 * @param[in] person_id The person index
 * @param[out] value Points of the contacts service value(#CTSvalue) which is returned
 * @par example
 * @code
 void get_person_default_num(int person_id)
 {
    int index, ret;
    CTSvalue *number=NULL;
    const char *default_num;

    ret = contacts_svc_get_person_value(CTS_GET_PERSON_DEFAULT_NUMBER_VALUE, person_id, &number);
    if(ret < CTS_SUCCESS) {
       printf("contacts_svc_get_contact_value() Failed(%d)\n", ret);
       return;
    }

    default_num = contacts_svc_value_get_str(number, CTS_NUM_VAL_NUMBER_STR);
    printf("The default Number is %s\n", default_num);
    contacts_svc_value_free(number);
 }
 * @endcode
 */
int contacts_svc_get_person_value(cts_get_person_val_op op_code, int person_id, CTSvalue **value);

/**
 * This function deletes all contacts related with a person.
 * It is not only deletes contact records from contact table,
 * but also clears up all the info of these contacts(group relation info, favorites info and etc.).
 *
 * @param[in] index The index of person to delete in database.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_delete_person(int index);

/**
 * @}
 */
//-->

#endif //__CONTACTS_PERSON_H__


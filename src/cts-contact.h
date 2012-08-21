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
#ifndef __CTS_CONTACT_H__
#define __CTS_CONTACT_H__

enum{
	CTS_GET_DATA_BY_CONTACT_ID,
	CTS_GET_DATA_BY_ID
};

int cts_get_data_info(int op_code, int field, int index, contact_t *contact);

//<!--
/**
 * @defgroup   CONTACTS_SVC_NAME Contact Naming Rule
 * @ingroup    CONTACTS_SVC
 * @addtogroup CONTACTS_SVC_NAME
 * @{
 * - insert
 * @code
 * if (No Name) {
 * 	if (Has "Company Name")
 * 		Display Name = "Company Name";
 * 	else if (Has "Number")
 * 		Display Name = "Number";
 * 	else if (Has "E-mail")
 * 		Display Name = "E-mail";
 * }
 * @endcode
 *
 * - update
 * @code
 * if (No "First Name" & No "Last Name") {
 * 	if (Has "Company Name")
 * 		Display Name = "Company Name";
 * 	else if (Has "Number")
 * 		Display Name = "Number";
 * 	else if (Has "E-mail")
 * 		Display Name = "E-mail";
 * }
 * @endcode
 * @}
 */

/**
 * @defgroup   CONTACTS_SVC_CONTACT Contact information Modification
 * @ingroup    CONTACTS_SVC
 * @addtogroup CONTACTS_SVC_CONTACT
 * @{
 *
 * This interface provides methods to insert/update/delete the Contact information.
 *
 */

/**
 * This function inserts a contact into database.
 * This api assigns a index of the contact automatically.
 * \n The returned index is unique and non-reusable.
 *
 * @param[in] addressbook_id The index of addressbook. 0 is local(phone internal)
 * @param[in] contact A contact information of CTSstruct() created by contacts_svc_struct_new(CTS_STRUCT_CONTACT).
 * @return the index of contact on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void insert_test(void)
 {
    CTSstruct *contact;
    CTSvalue *name, *number1, *number2;
    GSList *numbers=NULL;
    contact = contacts_svc_struct_new(CTS_STRUCT_CONTACT);

    name = contacts_svc_value_new(CTS_VALUE_NAME);
    if(name) {
       contacts_svc_value_set_str(name, CTS_NAME_VAL_FIRST_STR, "gildong");
       contacts_svc_value_set_str(name, CTS_NAME_VAL_LAST_STR, "Hong");
       contacts_svc_value_set_str(name, CTS_NAME_VAL_SUFFIX_STR, "engineer");
    }
    contacts_svc_struct_store_value(contact, CTS_CF_NAME_VALUE, name);
    contacts_svc_value_free(name);

    number1 = contacts_svc_value_new(CTS_VALUE_NUMBER);
    if(number1) {
       contacts_svc_value_set_str(number1, CTS_NUM_VAL_NUMBER_STR, "0987654321");
       contacts_svc_value_set_int(number1, CTS_NUM_VAL_TYPE_INT, CTS_NUM_TYPE_CELL);
       contacts_svc_value_set_bool(number1, CTS_NUM_VAL_DEFAULT_BOOL, true);
    }
    numbers = g_slist_append(numbers, number1);

    number2 = contacts_svc_value_new(CTS_VALUE_NUMBER);
    if(number2) {
       contacts_svc_value_set_str(number2, CTS_NUM_VAL_NUMBER_STR, "0123456789");
       contacts_svc_value_set_int(number2, CTS_NUM_VAL_TYPE_INT,
                                  CTS_NUM_TYPE_BUSINESS|CTS_NUM_TYPE_FAX);
    }
    numbers = g_slist_append(numbers, number2);

    contacts_svc_struct_store_list(contact, CTS_CF_NUMBER_LIST, numbers);
    contacts_svc_value_free(number1);
    contacts_svc_value_free(number2);
    g_slist_free(numbers);

    contacts_svc_insert_contact(0, contact);
    contacts_svc_struct_free(contact);
 }
 * @endcode
 */
int contacts_svc_insert_contact(int addressbook_id, CTSstruct* contact);

/**
 * This function deletes a contact in database.
 * It is not only deletes contact records from contact table,
 * but also clears up all the info of these contacts(group relation info, favorites info and etc.).
 *
 * @param[in] index The index of contact to delete in database.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void delete_test(void)
 {
    //get contact
    //CTSstruct *contact;
    //contacts_svc_struct_get_value(contact, CTS_CF_INDEX_INT, &value);
    //int index = contacts_svc_value_get_int(value, CTS_BASIC_VAL_INT);

    contacts_svc_delete_contact(2);

    //contacts_svc_struct_free(contact);

#if DELETE_CONTACTS
    // TODO: get each index of contacts
    int i, index_list[10] ={1,3,4,65,345,54,5,2,9,10};
    int ret;

    ret = contacts_svc_begin_trans();
    if(CTS_SUCCESS != ret) return;
    for(i=0;i<10;i++) {
       ret = contacts_svc_delete_contact(index_list[i]);
       if(CTS_SUCCESS != ret) {
          contacts_svc_end_trans(false);
          return;
       }
    }
    ret = contacts_svc_end_trans(true);
    if(ret < CTS_SUCCESS){
       printf("all work were rollbacked");
       return;
    }
#endif
 }
 * @endcode
 */
int contacts_svc_delete_contact(int index);

/**
 * This function updates a contact in the database.
 * \n If you want to add to list, store list.(refer to example)
 * \n To remove from list, set the value's VAL_DELETE_BOOL field.
 *
 * @param[in] contact A contact information of #CTSstruct.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code

 void update_test()
 {
    GSList *numbers, *cursor;
    CTSvalue *number=NULL;
    CTSstruct *contact=NULL;

    contacts_svc_get_contact(1, &contact);

    numbers = NULL;
    contacts_svc_struct_get_list(contact, CTS_CF_NUMBER_LIST, &numbers);
    cursor = numbers;
    if(cursor) {
       //char *temp = contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR);
       contacts_svc_value_set_str(cursor->data, CTS_NUM_VAL_NUMBER_STR, "0987651234");

       cursor = g_slist_next(cursor);
       if(cursor)
          contacts_svc_value_set_bool(cursor->data, CTS_NUM_VAL_DELETE_BOOL, true);

       number = contacts_svc_value_new(CTS_VALUE_NUMBER);
       if(number) {
          contacts_svc_value_set_str(number, CTS_NUM_VAL_NUMBER_STR, "0125439876");
          contacts_svc_value_set_int(number, CTS_NUM_VAL_TYPE_INT, CTS_NUM_TYPE_CELL);
          contacts_svc_value_set_bool(number, CTS_NUM_VAL_DEFAULT_BOOL, true);
       }
       numbers = g_slist_append(numbers, number);
    }

    contacts_svc_struct_store_list(contact, CTS_CF_NUMBER_LIST, numbers);
    contacts_svc_value_free(number);
    //free("0125439876");
    contacts_svc_update_contact(contact);

    contacts_svc_struct_free(contact);
 }
 * @endcode
 */
int contacts_svc_update_contact(CTSstruct* contact);

/**
 * Use for contacts_svc_put_contact_value().
 */
typedef enum{
	CTS_PUT_VAL_REPLACE_RINGTONE=0,/**< Use #CTS_VALUE_CONTACT_BASE_INFO */
	CTS_PUT_VAL_REPLACE_NOTE=2,/**< Use #CTS_VALUE_CONTACT_BASE_INFO */
	CTS_PUT_VAL_ADD_NUMBER=3,/**< Use #CTS_VALUE_NUMBER */
	CTS_PUT_VAL_ADD_EMAIL=4,/**< Use #CTS_VALUE_EMAIL */
}cts_put_contact_val_op;

/**
 * This function puts contacts service value(#CTSvalue) with op_code(#cts_put_contact_val_op).
 *
 * @param[in] op_code #cts_put_contact_val_op
 * @param[in] contact_id The index of the contact to put value.
 * @param[in] value The contacts service value which is put.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void put_value_test()
 {
    CTSvalue *value, *number;

    value = contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);
    if(value) {
       contacts_svc_value_set_str(value, CTS_BASE_VAL_RINGTONE_PATH_STR,
                                  "/opt/test/test.mp3");
       contacts_svc_put_contact_value(CTS_PUT_VAL_REPLACE_RINGTONE, 1, value);
       contacts_svc_value_free(value);
       //free("/opt/test/test.mp3")
    }

    number = contacts_svc_value_new(CTS_VALUE_NUMBER);
    if(number) {
       contacts_svc_value_set_str(number, CTS_NUM_VAL_NUMBER_STR, "0123337777");
       contacts_svc_value_set_int(number, CTS_NUM_VAL_TYPE_INT, CTS_NUM_TYPE_CELL);
       contacts_svc_value_set_bool(number, CTS_NUM_VAL_DEFAULT_BOOL, true);

       contacts_svc_put_contact_value(CTS_PUT_VAL_ADD_NUMBER, 1, number);
       contacts_svc_value_free(number);
       //free("0123337777")
    }
 }
 * @endcode
 */
int contacts_svc_put_contact_value(cts_put_contact_val_op op_code,
		int contact_id, CTSvalue* value);


/**
 * Use for contacts_svc_get_contact_value().
 */
typedef enum {
	CTS_GET_NAME_VALUE, /**< Use #NAMEVALUE */
	CTS_GET_DEFAULT_EMAIL_VALUE,/**< related with contact id. Use #EMAILVALUE */
	CTS_GET_DEFAULT_NUMBER_VALUE,/**< related with contact id. Use #NUMBERVALUE */
	CTS_GET_NUMBER_VALUE, /**< related with number id. Use #NUMBERVALUE */
	CTS_GET_EMAIL_VALUE, /**< related with email id. Use #EMAILVALUE */
	CTS_GET_COMPANY_VALUE, /**< related with contact id. Use #COMPANYVALUE */
}cts_get_contact_val_op;
/**
 * This function can get a value data related with id and op_code.
 * The value data is decided by op_code(#cts_get_contact_val_op)
 * The gotten value is readonly.
 * If id is not contact id, it returns the related contact id.
 * Obtained contact record should be freed by using contacts_svc_value_free().
 * @return #CTS_SUCCESS or the related contact id on success, Negative value(#cts_error) on error
 * @param[in] op_code #cts_get_contact_val_op
 * @param[in] id The related index
 * @param[out] value Points of the contacts service value(#CTSvalue) which is returned
 * @par example
 * @code
 void get_contact_default_num(void)
 {
    int index, ret;
    CTSvalue *number=NULL;
    const char *default_num;

    index = contacts_svc_find_contact_by(CTS_FIND_BY_NUMBER, "0125439876");

    ret = contacts_svc_get_contact_value(CTS_GET_DEFAULT_NUMBER_VALUE, index, &number);
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
int contacts_svc_get_contact_value(cts_get_contact_val_op op_code,
		int id, CTSvalue **value);


/**
 * This function gets contact record which has the index from the database.
 * Obtained contact record should be freed by using contacts_svc_struct_free().
 * @param[in] index The index of contact to get
 * @param[out] contact Points of the contact record which is returned
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void get_contact(void)
 {
    int ret=-1;
    CTSstruct *contact = NULL;
    CTSvalue *name;
    GSList *get_list, *cursor;

    ret = contacts_svc_get_contact(1, &contact);
    if(ret < 0)
    {
       printf("No found record\n");
       return;
    }

    contacts_svc_struct_get_value(contact, CTS_CF_NAME_VALUE, &name);
    printf("First Name : %s\n", contacts_svc_value_get_str(name, CTS_NAME_VAL_FIRST_STR));
    printf("Last Name : %s\n", contacts_svc_value_get_str(name, CTS_NAME_VAL_LAST_STR));
    printf("Additional Name : %s\n", contacts_svc_value_get_str(name, CTS_NAME_VAL_ADDITION_STR));
    printf("Display Name : %s\n", contacts_svc_value_get_str(name, CTS_NAME_VAL_DISPLAY_STR));
    printf("Prefix Name : %s\n", contacts_svc_value_get_str(name, CTS_NAME_VAL_PREFIX_STR));
    printf("Suffix Name : %s\n", contacts_svc_value_get_str(name, CTS_NAME_VAL_SUFFIX_STR));

    get_list = NULL;
    contacts_svc_struct_get_list(contact, CTS_CF_NUMBER_LIST, &get_list);

    cursor = get_list;
    for(;cursor;cursor=g_slist_next(cursor))
    {
       printf("number Type = %d",
          contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_TYPE_INT));

       printf("Number = %s\n",
          contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
    }

    get_list = NULL;
    contacts_svc_struct_get_list(contact, CTS_CF_EMAIL_LIST, &get_list);

    cursor = get_list;
    for(;cursor;cursor=g_slist_next(cursor))
    {
       printf("email Type = %d",
          contacts_svc_value_get_int(cursor->data, CTS_EMAIL_VAL_TYPE_INT));

       printf("email = %s\n",
          contacts_svc_value_get_str(cursor->data, CTS_EMAIL_VAL_ADDR_STR));
    }

    contacts_svc_struct_free(contact);
 }
 * @endcode
 */
int contacts_svc_get_contact(int index, CTSstruct **contact);

/**
 * Use for contacts_svc_find_contact_by(), contacts_svc_find_person_by()
 */
typedef enum {
	CTS_FIND_NONE,
	CTS_FIND_BY_NAME,
	CTS_FIND_BY_NUMBER,
	CTS_FIND_BY_EMAIL,
	CTS_FIND_BY_UID,
}cts_find_op;
/**
 * This function gets index of contact related with user_data.
 * index is found by op_code with user_data related with op_code(#cts_find_op).
 * @param[in] op_code #cts_find_op
 * @param[in] user_data The parameter for searching
 * @return index of found contact on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void get_contact(void)
 {
    int index, ret=-1;
    CTSstruct *contact = NULL;

    index = contacts_svc_find_contact_by(CTS_FIND_BY_NUMBER, "0123456789");
    if(index > CTS_SUCCESS)
      ret = contacts_svc_get_contact(index, &contact);
    if(ret < 0)
    {
       printf("No found record\n");
       return;
    }
 }
 * @endcode
 */
int contacts_svc_find_contact_by(cts_find_op op_code, const char *user_data);

/**
 * @}
 */

//-->
#endif //__CTS_CONTACT_H__


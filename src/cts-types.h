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
#ifndef __CTS_TYPES_H__
#define __CTS_TYPES_H__

//<!--
/**
 * @defgroup   CONTACTS_SVC_TYPES Types
 * @ingroup    CONTACTS_SVC
 * @addtogroup CONTACTS_SVC_TYPES
 * @{
 *
 * It is Types of Number, E-mail, Web address, Address, Event, Phone Log, etc.
 * And this interface provides methods to handle custom types.
 *
 */

/**
 * The Number can be made with a set of values by specifying one or more values.
 * \n Example : CTS_NUM_TYPE_HOME|CTS_NUM_TYPE_VOICE
 * \n Exceptionally, CTS_NUM_TYPE_CUSTOM is exclusive.
 * CTS_NUM_TYPE_CUSTOM should be handled earlier.
 */
enum NUMBERTYPE{
	CTS_NUM_TYPE_NONE = 0,
	CTS_NUM_TYPE_HOME = 1<<0,/**< a telephone number associated with a residence */
	CTS_NUM_TYPE_WORK = 1<<1,/**< a telephone number associated with a place of work */
	CTS_NUM_TYPE_VOICE = 1<<2,/**< a voice telephone number */
	CTS_NUM_TYPE_FAX = 1<<3,/**< a facsimile telephone number */
	CTS_NUM_TYPE_MSG = 1<<4,/**< the telephone number has voice messaging support */
	CTS_NUM_TYPE_CELL = 1<<5,/**< a cellular telephone number */
	CTS_NUM_TYPE_PAGER = 1<<6,/**< a paging device telephone number */
	CTS_NUM_TYPE_BBS = 1<<7,/**< a bulletin board system telephone number */
	CTS_NUM_TYPE_MODEM = 1<<8,/**< a MODEM connected telephone number */
	CTS_NUM_TYPE_CAR = 1<<9,/**< a car-phone telephone number */
	CTS_NUM_TYPE_ISDN = 1<<10,/**< an ISDN service telephone number */
	CTS_NUM_TYPE_VIDEO = 1<<11,/**< a video conferencing telephone number */
	CTS_NUM_TYPE_PCS = 1<<12,/**< a personal communication services telephone number */

	CTS_NUM_TYPE_ASSISTANT = 1<<30,/**< a additional type for assistant */
	CTS_NUM_TYPE_CUSTOM = 1<<31,/**< Custom number type */
};

enum EMAILTYPE{
	CTS_EMAIL_TYPE_NONE = 0,/**< Other */
	CTS_EMAIL_TYPE_HOME = 1<<0,/**< . */
	CTS_EMAIL_TYPE_WORK = 1<<1,/**< . */
};

enum ADDRESSTYPE{
	CTS_ADDR_TYPE_NONE = 0,/**< . */
	CTS_ADDR_TYPE_HOME = 1<<0,/**< a delivery address for a residence */
	CTS_ADDR_TYPE_WORK = 1<<1,/**< a delivery address for a place of work */
	CTS_ADDR_TYPE_DOM = 1<<2,/**< a domestic delivery address */
	CTS_ADDR_TYPE_INTL = 1<<3,/**< an international delivery address */
	CTS_ADDR_TYPE_POSTAL = 1<<4,/**< a postal delivery address */
	CTS_ADDR_TYPE_PARCEL = 1<<5,/**< a parcel delivery address */
};

enum WEBTYPE{
	CTS_WEB_TYPE_NONE,/**< Other */
	CTS_WEB_TYPE_HOME,/**< . */
	CTS_WEB_TYPE_WORK,/**< . */
};

enum PLOGTYPE{
	CTS_PLOG_TYPE_NONE,
	CTS_PLOG_TYPE_VOICE_INCOMMING = 1,/**< . */
	CTS_PLOG_TYPE_VOICE_OUTGOING = 2,/**< . */
	CTS_PLOG_TYPE_VIDEO_INCOMMING = 3,/**< . */
	CTS_PLOG_TYPE_VIDEO_OUTGOING = 4,/**< . */
	CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN = 5,/**< Not confirmed missed call */
	CTS_PLOG_TYPE_VOICE_INCOMMING_SEEN = 6,/**< Confirmed missed call */
	CTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN = 7,/**< Not confirmed missed video call */
	CTS_PLOG_TYPE_VIDEO_INCOMMING_SEEN = 8,/**< Confirmed missed video call */
	CTS_PLOG_TYPE_VOICE_REJECT = 9,/**< . */
	CTS_PLOG_TYPE_VIDEO_REJECT = 10,/**< . */
	CTS_PLOG_TYPE_VOICE_BLOCKED = 11,/**< . */
	CTS_PLOG_TYPE_VIDEO_BLOCKED = 12,/**< . */

	CTS_PLOG_TYPE_MMS_INCOMMING = 101,/**< . */
	CTS_PLOG_TYPE_MMS_OUTGOING = 102,/**< . */
	CTS_PLOG_TYPE_SMS_INCOMMING = 103,/**< . */
	CTS_PLOG_TYPE_SMS_OUTGOING = 104,/**< . */
	CTS_PLOG_TYPE_SMS_BLOCKED = 105,/**< . */
	CTS_PLOG_TYPE_MMS_BLOCKED = 106,/**< . */

	CTS_PLOG_TYPE_EMAIL_RECEIVED = 201,/**<.*/
	CTS_PLOG_TYPE_EMAIL_SENT = 202,/**<.*/

	CTS_PLOG_TYPE_MAX
};

enum EVENTTYPE{
	CTS_EVENT_TYPE_BIRTH,/**< . */
	CTS_EVENT_TYPE_ANNIVERSARY/**< . */
};

enum ADDRESSBOOKTYPE{
	CTS_ADDRESSBOOK_TYPE_INTERNAL, /**< . */
	CTS_ADDRESSBOOK_TYPE_EXCHANGE, /**< . */
	CTS_ADDRESSBOOK_TYPE_GOOGLE, /**< . */
	CTS_ADDRESSBOOK_TYPE_YAHOO, /**< . */
	CTS_ADDRESSBOOK_TYPE_FACEBOOK, /**< . */
	CTS_ADDRESSBOOK_TYPE_OTHER, /**< . */
};

/**
 * Use for contacts_svc_insert_custom_type(),
 * contacts_svc_delete_custom_type(), contacts_svc_find_custom_type().
 */
typedef enum {
	CTS_TYPE_CLASS_EXTEND_DATA=0,/**< Extend Data type(@ref CONTACTS_SVC_EXTEND) */
	CTS_TYPE_CLASS_NUM=1,/**< Custom Number type */
}cts_custom_type_class;

/**
 * This function inserts a User defined type into database.
 * This api assigns a index of the group automatically.
 * \n The returned index is unique & non-reusable.
 *
 * @param[in] type_class #cts_custom_type_class
 * @param[in] type_name Name of Custom Type
 * @return the index of the inserted custom type on success, Negative value(#cts_error) on error
 */
int contacts_svc_insert_custom_type(cts_custom_type_class type_class, char *type_name);

/**
 * This function deletes a user defined type in database.
 *
 * @param[in] type_class #cts_custom_type_class
 * @param[in] index The index of User defined type to delete in database.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_delete_custom_type(cts_custom_type_class type_class, int index);


/**
 * This function gets name of custom type.
 * Obtained string should be free using by free().
 * @param[in] type_class #cts_custom_type_class
 * @param[in] index The index of User defined type.
 * @return The gotten information, or NULL if no value is obtained or error
 */
char* contacts_svc_get_custom_type(cts_custom_type_class type_class, int index);

/**
 * This function gets index of user defined type of #name.
 *
 * @param[in] type_class #cts_custom_type_class
 * @param[in] type_name The name of type for searching
 * @return index of found Custom type on success, Negative value(#cts_error) on error
 */
int contacts_svc_find_custom_type(cts_custom_type_class type_class, char *type_name);

/*
 * @}
 */

/**
 * @defgroup   CONTACTS_SVC_EXTEND Using the Extend Data for Contact
 * @ingroup    CONTACTS_SVC
 * @addtogroup CONTACTS_SVC_EXTEND
 * @{
 *
 * This is description of usages of extend data related with a contact.
 *
 * @section extend_sec1 Properties
 * - The extend data is contacts service value(#CTSvalue).
 * - The extend data only exist for contact struct(#CTSstruct).
 * - The type of extend data is defined
 * by contacts_svc_insert_custom_type() with #CTS_TYPE_CLASS_EXTEND_DATA.
 * - The extend data is stored to contact by contacts_svc_struct_store_value().
 * - Extend data can be stored only one at each type in contacts.
 * - The index of custom type is used as the field parameter of contacts_svc_struct_store_value().
 * - The composition of the extend data(#EXTENDVALUE)
 *   -# #CTS_EXTEND_VAL_DATA1_INT
 *   -# #CTS_EXTEND_VAL_DATA2_STR
 *   -# #CTS_EXTEND_VAL_DATA3_STR
 *   -# #CTS_EXTEND_VAL_DATA4_STR
 *   -# #CTS_EXTEND_VAL_DATA5_STR
 *   -# #CTS_EXTEND_VAL_DATA6_STR
 *   -# #CTS_EXTEND_VAL_DATA7_STR
 *   -# #CTS_EXTEND_VAL_DATA8_STR
 *   -# #CTS_EXTEND_VAL_DATA9_STR
 *   -# #CTS_EXTEND_VAL_DATA10_STR
 *
 * @section extend_sec2 Usages
 * - Notice
 * \n The extend data has values of fixed type.
 * \n Therefore if you want to save values of the other types, convert to string.
 * \n This mechanism is a supplementary mechanism. Do not abuse.
 * - example
 * @code
 #include <stdio.h>
 #include <glib.h>
 #include <contacts-svc.h>

 static void print_extend_contact(CTSstruct *contact)
 {
    int ret;
    CTSvalue *value;
    GSList *get_list, *cursor;
    value = NULL;
    contacts_svc_struct_get_value(contact, CTS_CF_NAME_VALUE, &value);
    printf("First Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_FIRST_STR));
    printf("Last Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_LAST_STR));

    value = NULL;
    ret = contacts_svc_find_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "YomiName");
    ret = contacts_svc_struct_get_value(contact, ret, &value);
    if(CTS_SUCCESS == ret) {
       printf("extend1 data2 : %s\n", contacts_svc_value_get_str(value, CTS_EXTEND_VAL_DATA2_STR));
       printf("extend1 data3 : %s\n", contacts_svc_value_get_str(value, CTS_EXTEND_VAL_DATA3_STR));
       printf("extend1 data4 : %s\n", contacts_svc_value_get_str(value, CTS_EXTEND_VAL_DATA4_STR));
    }
    value = NULL;
    ret = contacts_svc_find_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "Family");
    ret = contacts_svc_struct_get_value(contact, ret, &value);
    if(CTS_SUCCESS == ret) {
       printf("extend2 data2 : %s\n", contacts_svc_value_get_str(value, CTS_EXTEND_VAL_DATA2_STR));
       printf("extend2 data3 : %s\n", contacts_svc_value_get_str(value, CTS_EXTEND_VAL_DATA3_STR));
       printf("extend2 data4 : %s\n", contacts_svc_value_get_str(value, CTS_EXTEND_VAL_DATA4_STR));
    }
    get_list = NULL;
    contacts_svc_struct_get_list(contact, CTS_CF_NUMBER_LIST, &get_list);

    cursor = get_list;
    for(;cursor;cursor=g_slist_next(cursor))
    {
       printf("number Type = %d",
          contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_TYPE_INT));
       if(contacts_svc_value_get_bool(cursor->data, CTS_NUM_VAL_FAVORITE_BOOL))
          printf("(favorite)");
       printf("Number = %s\n",
          contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
    }
 }

 void extend_data_test(void)
 {
    int ret, index;
    CTSstruct *contact;
    CTSvalue *name, *number1, *extend_value;
    GSList *numbers=NULL;

    contact = contacts_svc_struct_new(CTS_STRUCT_CONTACT);

    name = contacts_svc_value_new(CTS_VALUE_NAME);
    if(name) {
       contacts_svc_value_set_str(name, CTS_NAME_VAL_FIRST_STR, "People");
       contacts_svc_value_set_str(name, CTS_NAME_VAL_LAST_STR, "Japan");
    }
    contacts_svc_struct_store_value(contact, CTS_CF_NAME_VALUE, name);
    contacts_svc_value_free(name);

    number1 = contacts_svc_value_new(CTS_VALUE_NUMBER);
    if(number1) {
       contacts_svc_value_set_str(number1, CTS_NUM_VAL_NUMBER_STR, "0333333333");
       contacts_svc_value_set_int(number1, CTS_NUM_VAL_TYPE_INT, CTS_NUM_TYPE_MOBILE);
       contacts_svc_value_set_bool(number1, CTS_NUM_VAL_DEFAULT_BOOL, true);
    }
    numbers = g_slist_append(numbers, number1);

    contacts_svc_struct_store_list(contact, CTS_CF_NUMBER_LIST, numbers);
    contacts_svc_value_free(number1);
    g_slist_free(numbers);

    extend_value = contacts_svc_value_new(CTS_VALUE_EXTEND);
    if(extend_value) {
       contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA2_STR, "YomiFirstName");
       contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA3_STR, "YomiLastName");
       contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA4_STR, "YomiCompanyName");
    }
    ret = contacts_svc_find_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "YomiName");
    if(CTS_ERR_DB_RECORD_NOT_FOUND == ret)
       ret = contacts_svc_insert_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "YomiName");
    contacts_svc_struct_store_value(contact, ret, extend_value);
    contacts_svc_value_free(extend_value);

    extend_value = contacts_svc_value_new(CTS_VALUE_EXTEND);
    if(extend_value) {
       contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA2_STR, "Children1");
       contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA3_STR, "Children2");
       contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA4_STR, "Children3");
    }
    ret = contacts_svc_find_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "Family");
    if(CTS_ERR_DB_RECORD_NOT_FOUND == ret)
       ret = contacts_svc_insert_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "Family");
    contacts_svc_struct_store_value(contact, ret, extend_value);
    contacts_svc_value_free(extend_value);

    index = contacts_svc_insert_contact(0, contact);
    contacts_svc_struct_free(contact);
    contact = NULL;

    ret = contacts_svc_get_contact(index, &contact);
    if(ret < 0)
    {
       printf("No found record\n");
       return;
    }
    print_extend_contact(contact);

    //update test

    extend_value = NULL;
    ret = contacts_svc_find_custom_type(CTS_TYPE_CLASS_EXTEND_DATA, "Family");
    ret = contacts_svc_struct_get_value(contact, ret, &extend_value);
    if(CTS_SUCCESS == ret)
       contacts_svc_value_set_str(extend_value, CTS_EXTEND_VAL_DATA2_STR, "Children4");
    contacts_svc_struct_store_value(contact, ret, extend_value);

    contacts_svc_update_contact(contact);
    contacts_svc_struct_free(contact);
    contact = NULL;

    ret = contacts_svc_get_contact(index, &contact);
    if(ret < 0)
    {
       printf("No found record\n");
       return;
    }
    print_extend_contact(contact);
    contacts_svc_struct_free(contact);
 }

 int main()
 {
    contacts_svc_connect();

    extend_data_test();

    contacts_svc_disconnect();

    return 0;
 }

 * @endcode
 *
 * @}
 */

//-->

#endif //__CTS_TYPES_H__


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

/**
 *
 * @ingroup   SLP_PG
 * @defgroup  CONTACTS_SVC_PG Contacts Service


<h1 class="pg">Introduction</h1>
	<h2 class="pg">Purpose of this document</h2>

The purpose of this document is to describe how applications can use contacts-service APIs for handling contact's information. This document gives programming guidelines to application engineers and examples of using contact data.

	<h2 class="pg">Scope</h2>

The scope of this document is limited to Contacts-service API usage.


<h1 class="pg">Contacts Service Architecture</h1>
	<h2 class="pg"> Overview</h2>

Contacts-service is responsible for inserting, deleting, and updating contact data in order to accommodate the needs for application's contact data.
Users can access contacts data without knowing DB schema, SQLite, relations of data

@image html SLP_ContactsService_PG_image001.PNG


	<h2 class="pg">Sub-Components</h2>

Contacts-svc-helper is a process for contacts-servcie. The process wait requests of contacts-service library and respond immediately



<h1 class="pg">Contacts Service Features</h1>
	- Similar to Sqlite3
	- Handle information of Contact
	- Handle information of Group
	- Handle information of Phone log

	<h2 class="pg">Similar to Sqlite3</h2>
Contacts-service API is similar to Sqlite3.

	<h2 class="pg">Handle information of Contact</h2>
Contacts-service supports to insert/update/delete/get/search information of contact.
The Information of contact includes name, numbers, emails, addresses, company, messenger, events, group relation information, web sites and favorite information.

	<h2 class="pg">Handle information of Group</h2>
Contacts-service supports to insert/update/delete/get information of contact.

	<h2 class="pg">Handle information of Group</h2>
Contacts-service supports to insert/update/delete/get information of Phone log.

<h1 class="pg">Contacts Service API Description</h1>

you can refer @ref CONTACTS_SVC



<h1 class="pg">Sample Code</h1>

	<h2 class="pg">Connect to Contact Database</h2>

Before using contact information from contacts service API, caller module should connect to the contact database, and after finishing with the contact information, should disconnect from the contact database

@code
int contacts_svc_connect(void);

int contacts_svc_disconnect(void);
@endcode


	<h2 class="pg">Insert information of contact</h2>

@code

void insert_test(void)
{
   CTSstruct *contact;
   CTSvalue *name, *number1, *number2;
   GSList *numbers=NULL;
   contact = contacts_svc_struct_new(CTS_STRUCT_CONTACT);

   name = contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);
   if(name) {
      contacts_svc_value_set_str(name, CTS_BASE_VAL_IMG_PATH_STR, "test.vcf");
   }
   contacts_svc_struct_store_value(contact, CTS_CF_BASE_INFO_VALUE, name);
   contacts_svc_value_free(name);


   name = contacts_svc_value_new(CTS_VALUE_NAME);
   if(name) {
      contacts_svc_value_set_str(name, CTS_NAME_VAL_FIRST_STR, "Gil-Dong");
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
                                 CTS_NUM_TYPE_WORK);
   }
   numbers = g_slist_append(numbers, number2);

   contacts_svc_struct_store_list(contact, CTS_CF_NUMBER_LIST, numbers);
   contacts_svc_value_free(number1);
   contacts_svc_value_free(number2);
   g_slist_free(numbers);

   contacts_svc_insert_contact(0, contact);
   contacts_svc_struct_free(contact);
}
@endcode


	<h2 class="pg">Get contact</h2>

@code
void get_contact(CTSstruct *contact)
{
   int index=0, ret=-1;
   CTSvalue *value=NULL;
   GSList *get_list, *cursor;

   if(!contact) {
      index = contacts_svc_find_contact_by(CTS_FIND_BY_NUMBER, "0123456789");
      if(index > CTS_SUCCESS)
        ret = contacts_svc_get_contact(index, &contact);
      if(ret < 0)
      {
         printf("No found record\n");
         return;
      }
   }
   contacts_svc_struct_get_value(contact, CTS_CF_NAME_VALUE, &value);
   printf("First Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_FIRST_STR));
   printf("Last Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_LAST_STR));
   printf("Additional Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_ADDITION_STR));
   printf("Display Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_DISPLAY_STR));
   printf("Prefix Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_PREFIX_STR));
   printf("Suffix Name : %s\n", contacts_svc_value_get_str(value, CTS_NAME_VAL_SUFFIX_STR));

   value = NULL;
   contacts_svc_struct_get_value(contact, CTS_CF_COMPANY_VALUE, &value);
   printf("Company Name : %s\n", contacts_svc_value_get_str(value, CTS_COMPANY_VAL_NAME_STR));
   printf("Company Department : %s\n", contacts_svc_value_get_str(value, CTS_COMPANY_VAL_DEPARTMENT_STR));

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
      if(index)
         contacts_svc_insert_favorite(contacts_svc_value_get_int(cursor->data, CTS_NUM_VAL_ID_INT));
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

   get_list = NULL;
   contacts_svc_struct_get_list(contact, CTS_CF_GROUPREL_LIST, &get_list);
   cursor = get_list;
   for(;cursor;cursor=g_slist_next(cursor))
   {
      printf("group = %s:",
         contacts_svc_value_get_str(cursor->data, CTS_GROUPREL_VAL_NAME_STR));

      printf("%d\n",
         contacts_svc_value_get_int(cursor->data, CTS_GROUPREL_VAL_ID_INT));
   }


   if(index)
      contacts_svc_struct_free(contact);

}
@endcode


	<h2 class="pg">Get contact list</h2>

@code
void get_contact_list(void)
{
   CTSiter *iter = NULL;
   contacts_svc_get_list(CTS_LIST_ALL_CONTACT, &iter);

   while(CTS_SUCCESS == contacts_svc_iter_next(iter))
   {
      CTSvalue *contact = NULL;
      const char *first, *last, *display;
      contact = contacts_svc_iter_get_info(iter);

      printf("(%8d)", contacts_svc_value_get_int(contact, CTS_LIST_CONTACT_ID_INT));
      display = contacts_svc_value_get_str(contact, CTS_LIST_CONTACT_DISPLAY_STR);
      if(display)
         printf("%s :", display);
      else
      {
         first = contacts_svc_value_get_str(contact, CTS_LIST_CONTACT_FIRST_STR);
         last = contacts_svc_value_get_str(contact, CTS_LIST_CONTACT_LAST_STR);
         if(CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
            printf("%s %s :", first, last);
         else
            printf("%s %s :", last, first);
      }
      printf("%s", contacts_svc_value_get_str(contact, CTS_LIST_CONTACT_IMG_PATH_STR));
      printf("\n");
      contacts_svc_value_free(contact);
   }
   contacts_svc_iter_remove(iter);
}
@endcode


	<h2 class="pg">Delete contact </h2>

@code

void delete_test(void)
{
   //get contact
   int index=0, ret=-1;
   CTSstruct *contact;

   if(!contact) {
      index = contacts_svc_find_contact_by(CTS_FIND_BY_NUMBER, "0123456789");
      if(index > CTS_SUCCESS)
        ret = contacts_svc_get_contact(index, &contact);
      if(ret < 0)
      {
         printf("No found record\n");
         return;
      }
   }

   contacts_svc_delete_contact(index);

   contacts_svc_struct_free(contact);

}

@endcode

	<h2 class="pg">Search contacts by name </h2>

@code
void search_contacts_by_name(void)
{
   int ret;
   CTSiter *iter = NULL;
   ret = contacts_svc_get_list_with_str(CTS_LIST_CONTACTS_WITH_NAME,
      "Hong", &iter);
   if(CTS_SUCCESS != ret) return;

   while(CTS_SUCCESS == contacts_svc_iter_next(iter))
   {
      CTSvalue *row_info = NULL;
      const char *first, *last, *display;
      row_info = contacts_svc_iter_get_info(iter);

      printf("(%8d)", contacts_svc_value_get_int(row_info, CTS_LIST_CONTACT_ID_INT));

      display = contacts_svc_value_get_str(row_info, CTS_LIST_CONTACT_DISPLAY_STR);
      if(display)
         printf("%s :", display);
      else
      {
         first = contacts_svc_value_get_str(row_info, CTS_LIST_CONTACT_FIRST_STR);
         last = contacts_svc_value_get_str(row_info, CTS_LIST_CONTACT_LAST_STR);
         if(CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
            printf("%s %s :", first, last);
         else
            printf("%s %s :", last, first);
      }
      printf("%s", contacts_svc_value_get_str(row_info, CTS_LIST_CONTACT_IMG_PATH_STR));
      printf("\n");
      contacts_svc_value_free(row_info);
   }
   contacts_svc_iter_remove(iter);
}
@endcode

 * @}
 */


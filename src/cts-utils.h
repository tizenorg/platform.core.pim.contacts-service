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
#ifndef __CTS_UTILS_H__
#define __CTS_UTILS_H__

#include <stdbool.h>

#define CTS_IMG_PATH_SIZE_MAX 1024
#define CTS_IMAGE_LOCATION "/opt/data/contacts-svc/img"
#define CTS_VCARD_IMAGE_LOCATION "/opt/data/contacts-svc/img/vcard"
#define CTS_NOTI_CONTACT_CHANGED_DEF "/opt/data/contacts-svc/.CONTACTS_SVC_DB_CHANGED"
#define CTS_VCONF_DISPLAY_ORDER_DEF "db/service/contacts/name_display_order"

void cts_deregister_noti(void);
void cts_register_noti(void);
int cts_get_default_language(void);
void cts_set_contact_noti(void);
void cts_set_plog_noti(void);
void cts_set_missed_call_noti(void);
void cts_set_favor_noti(void);
void cts_set_speed_noti(void);
void cts_set_addrbook_noti(void);
void cts_set_group_noti(void);
void cts_set_group_rel_noti(void);
int cts_exist_file(char *path);
int cts_convert_nicknames2textlist(GSList *src, char *dest, int dest_size);
GSList* cts_convert_textlist2nicknames(char *text_list);
int cts_increase_outgoing_count(int contact_id);
int cts_get_next_ver(void);
int cts_update_contact_changed_time(int contact_id);
int cts_delete_image_file(int img_type, int index);
int cts_add_image_file(int img_type, int index, char *src_img, char **dest_img);
int cts_update_image_file(int img_type, int index, char *src_img, char **dest_img);

#ifndef __CONTACTS_SVC_H__
//<!--
/**
 * This function starts database transaction
 * If you want to handle a transaction, use it.
 *
 * @par Multiple inserting case
 * case1 has only one DB commit. Therefore it is faster than case 2.
 * And if 5th inserted contact is failed,
 * case 1 insert nothing but case 2 insert 1,2,3 and 4th contacts.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @code
 * //case 1
 * contacts_svc_begin_trans();
 * for(i = 0; i< 20; i++) {
 *    if(CTS_SUCCESS != "insert api") {
 *       contacts_svc_end_trans(false);
 *       return -1;
 *    }
 * }
 * ret = contacts_svc_end_trans(true);
 * if(ret < CTS_SUCCESS){
 *  printf("all work were rollbacked");
 *   return;
 * }
 *
 * //case 2
 * for(i = 0; i< 20; i++) {
 *    if(CTS_SUCCESS != "insert api") {
 *       return -1;
 *    }
 * }
 * @endcode
 */
int contacts_svc_begin_trans(void);

/**
 * This function finishes database transaction of contacts service
 * If you want to handle a transaction, use it.
 * If returned value is error, the transaction was rollbacked.
 * When transaction is success, it returns the last contacts version.
 *
 * @param[in] is_success true : commit, false : rollback
 * @return #CTS_SUCCESS or the last contact version(when success) on success,
 *         Negative value(#cts_error) on error
 */
int contacts_svc_end_trans(bool is_success);

/**
 * A kind of order in contacts service of contacts service
 * @see contacts_svc_get_order()
 */
typedef enum{
	CTS_ORDER_NAME_FIRSTLAST, /**<First Name first */
	CTS_ORDER_NAME_LASTFIRST  /**<Last Name first */
}cts_order_type;

/**
 * Use for contacts_svc_get_order().
 */
typedef enum{
	CTS_ORDER_OF_SORTING, /**< Sorting Order */
	CTS_ORDER_OF_DISPLAY /**< Display Order */
}cts_order_op;

/**
 * This function gets the display or sorting order(Firstname first or LastName first)
 *
 * @param[in] op_code #cts_order_op
 * @return #CTS_ORDER_NAME_FIRSTLAST or #CTS_ORDER_NAME_LASTFIRST on success,
 *           \n Negative value(#cts_error) on error
 */
cts_order_type contacts_svc_get_order(cts_order_op op_code);

/**
 * This function sets the display or sorting order(Firstname first or LastName first)
 *
 * @param[in] op_code #cts_order_op
 * @param[in] order order type(#cts_order_type)
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_set_order(cts_order_op op_code, cts_order_type order);

/**
 * Use for contacts_svc_subscribe_change(), contacts_svc_unsubscribe_change()
 */
typedef enum{
	CTS_SUBSCRIBE_CONTACT_CHANGE,
	CTS_SUBSCRIBE_PLOG_CHANGE,
	CTS_SUBSCRIBE_FAVORITE_CHANGE,
	CTS_SUBSCRIBE_GROUP_CHANGE,
	CTS_SUBSCRIBE_SPEEDDIAL_CHANGE,
	CTS_SUBSCRIBE_ADDRESSBOOK_CHANGE,
	CTS_SUBSCRIBE_MISSED_CALL_CHANGE
}cts_subscribe_type;

/**
 * This function watchs contacts service changes.
 * The notification is sent once per a transaction.
 * This is handled by default context of g_main_loop.
 *
 * @param[in] noti_type A kind of Notification
 * @param[in] cb callback function pointer
 * @param[in] user_data data which is passed to callback function
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
#include <stdio.h>
#include <glib.h>
#include <contacts-svc.h>

void test_callback(void *data)
{
   printf("Contact data of contacts service is changed\n");
}

int main()
{
   GMainLoop *loop;

   contacts_svc_subscribe_change(CTS_SUBSCRIBE_CONTACT_CHANGE, test_callback, NULL);

   loop = g_main_loop_new(NULL, FALSE);
   g_main_loop_run(loop);

   contacts_svc_unsubscribe_change(CTS_SUBSCRIBE_CONTACT_CHANGE, test_callback);
   g_main_loop_unref(loop);

   return 0;
}
 * @endcode
 */
int contacts_svc_subscribe_change(cts_subscribe_type noti_type,
		void (*cb)(void *), void *user_data);

/**
 * This function stops to watch contacts service changes.
 * @param[in] noti_type A kind of Notification(#cts_subscribe_type)
 * @param[in] cb callback function which is added by contacts_svc_subscribe_change()
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_unsubscribe_change(cts_subscribe_type noti_type,
		void (*cb)(void *));

/**
 * This function delete a callback function which is specified with user_data.
 * @param[in] noti_type A kind of Notification(#cts_subscribe_type)
 * @param[in] cb The callback function which is added by contacts_svc_subscribe_change()
 * @param[in] user_data The user_data which is added by contacts_svc_subscribe_change()
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_unsubscribe_change_with_data(cts_subscribe_type noti_type,
		void (*cb)(void *), void *user_data);

/**
 * Use for contacts_svc_count()
 */
typedef enum
{
	CTS_GET_ALL_CONTACT, /**< The count of contacts in the all addressbook */
	CTS_GET_COUNT_SDN, /**< The count of SDN(Service Dialing Number) in SIM */
	CTS_GET_ALL_PHONELOG, /**< The count of all phonelog */
	CTS_GET_UNSEEN_MISSED_CALL, /**< The count of unseen missed call */
}cts_count_op;
/**
 * This function gets count related with op_code.
 *
 * @param[in] op_code #cts_count_op
 * @return The count number on success, Negative value(#cts_error) on error
 */
int contacts_svc_count(cts_count_op op_code);

/**
 * Use for contacts_svc_count_with_int()
 */
typedef enum
{
	CTS_GET_COUNT_CONTACTS_IN_ADDRESSBOOK, /**< The count of contacts in the addressbook related to index(search_value) */
	CTS_GET_COUNT_CONTACTS_IN_GROUP, /**< The count of contacts in the group related to index(search_value) */
	CTS_GET_COUNT_NO_GROUP_CONTACTS_IN_ADDRESSBOOK, /**< The count of not assigned contacts in the addressbook related to index(search_value) */
}cts_count_int_op;
/**
 * This function gets count related with op_code and search_value.
 * \n #search_value is related with op_code. The Word after preposition is a property of search_value.
 *
 * @param[in] op_code #cts_count_int_op
 * @param[in] search_value interger value(almost a related index) for searching
 * @return The count number on success, Negative value(#cts_error) on error
 */
int contacts_svc_count_with_int(cts_count_int_op op_code, int search_value);

/**
 * Use for contacts_svc_save_image()
 */
typedef enum
{
	CTS_IMG_NORMAL, /**< . */
	CTS_IMG_FULL, /**< . */
} cts_img_t;

/**
 * This function saves image to contacts service domain.
 *
 * @param[in] img_type #cts_img_t
 * @param[in] index index of contact
 * @param[in] src_img The image path to copy(Should include extension at path)
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_save_image(cts_img_t img_type, int index, char *src_img);

/**
 * This function gets image from contacts service domain.
 * Usually, You can get the #CTS_IMG_NORMAL in Contacts Struct(#CTSstruct).
 *
 * @param[in] img_type #cts_img_t
 * @param[in] index index of contact
 * @param[in] img_path The pointer of getting image path(should be freed by using free())
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_get_image(cts_img_t img_type, int index, char **img_path);

/**
 * This function imports sim phonebook.
 *
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_import_sim(void);

/**
 * This function sets the outgoing count of the contact to zero.
 *
 * @param[in] contact_id The index of contact
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @see contacts_svc_get_list(), #CTS_LIST_OFTEN_USED_CONTACT
 */
int contacts_svc_reset_outgoing_count(int contact_id);

//-->
#endif //#ifndef __CONTACTS_SVC_H__

#endif //__CTS_UTILS_H__


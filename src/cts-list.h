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
#ifndef __CTS_LIST_H__
#define __CTS_LIST_H__

#include "cts-sqlite.h"

enum
{
	CTS_ITER_NONE,
	CTS_ITER_CONTACTS,
	CTS_ITER_ALL_CUSTOM_NUM_TYPE,
	CTS_ITER_GROUPING_PLOG,
	CTS_ITER_ALL_CONTACT_FAVORITE,
	CTS_ITER_ALL_NUM_FAVORITE,
	CTS_ITER_ALL_SPEEDDIAL,
	CTS_ITER_ALL_SDN,
	CTS_ITER_PLOGS_OF_NUMBER,
	CTS_ITER_PLOGNUMBERS_WITH_NUM,
	CTS_ITER_CONTACTS_WITH_NAME,
	CTS_ITER_NUMBERINFOS,
	CTS_ITER_EMAILINFOS_WITH_EMAIL,
	CTS_ITER_NUMBERS_EMAILS,
	CTS_ITER_GROUPS,
	CTS_ITER_ADDRESSBOOKS,
	CTS_ITER_EMAILS_OF_CONTACT_ID,
	CTS_ITER_NUMBERS_OF_CONTACT_ID,
	CTS_ITER_UPDATED_INFO_AFTER_VER,
	CTS_ITER_PLOGS_OF_PERSON_ID,
	CTS_ITER_OSP,
	CTS_ITER_MAX
};

typedef struct _updated_record {
	int type;
	int id;
	int ver;
	int addressbook_id;
	struct _updated_record *next;
}updated_record;

typedef struct {
	updated_record *head;
	updated_record *cursor;
}updated_info;

struct _cts_iter {
	int i_type;
	cts_stmt stmt;
	updated_info *info;
};

//<!--
/**
 * @defgroup   CONTACTS_SVC_LIST List handling
 * @ingroup    CONTACTS_SVC
 * @addtogroup CONTACTS_SVC_LIST
 * @{
 *
 * This interface provides methods to handle the List.
 *
 * List is handled by iterator. The iterator is same to handle's cursor of Sqlite3.
 * While an iterator is in use, all attempts to write in this or some other process
 * will be blocked. Parallel reads are supported.
 *
 */

/**
 * CTSiter is an opaque type.
 * Iterator can get by contacts_svc_get_list(), contacts_svc_get_list_with_int(),
 * contacts_svc_get_list_with_str(), contacts_svc_get_list_with_filter(), contacts_svc_get_updated_contacts().
 * \n And Iterator can handle by contacts_svc_iter_next(), contacts_svc_iter_remove(), contacts_svc_iter_get_info().
 */
typedef struct _cts_iter CTSiter;

//////////////////// read only value ////////////////////
//////////////////// List row info ////////////////////
/**
 * Phone Log List
 * For #CTS_LIST_PLOGS_OF_NUMBER, it supports CTS_LIST_PLOG_ID_INT, CTS_LIST_PLOG_LOG_TIME_INT,
 * CTS_LIST_PLOG_LOG_TYPE_INT, CTS_LIST_PLOG_DURATION_INT(or CTS_LIST_PLOG_MSGID_INT), CTS_LIST_PLOG_SHORTMSG_STR
 * and CTS_LIST_PLOG_RELATED_ID_INT.
 * For #CTS_LIST_PLOGS_OF_PERSON_ID, it supports CTS_LIST_PLOG_ID_INT, CTS_LIST_PLOG_LOG_TIME_INT,
 * CTS_LIST_PLOG_LOG_TYPE_INT, CTS_LIST_PLOG_DURATION_INT(or CTS_LIST_PLOG_MSGID_INT), CTS_LIST_PLOG_SHORTMSG_STR
 * CTS_LIST_PLOG_RELATED_ID_INT and CTS_LIST_PLOG_NUMBER_STR.
 */
enum PHONELOGLIST{
	CTS_LIST_PLOG_ID_INT,/**< . */
	CTS_LIST_PLOG_NUM_TYPE_INT,/**< you can use #NUMBERTYPE or contacts_svc_find_custom_type(). */
	CTS_LIST_PLOG_FIRST_NAME_STR,/**< . */
	CTS_LIST_PLOG_LAST_NAME_STR,/**< . */
	CTS_LIST_PLOG_DISPLAY_NAME_STR,/**< . */
	CTS_LIST_PLOG_NUMBER_STR,/**< . */
	CTS_LIST_PLOG_IMG_PATH_STR,/**< . */
	CTS_LIST_PLOG_LOG_TIME_INT,/**< The time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds. */
	CTS_LIST_PLOG_LOG_TYPE_INT,/**< #PLOGTYPE  */
	CTS_LIST_PLOG_DURATION_INT,/**< seconds */
	CTS_LIST_PLOG_MSGID_INT,/**< . */
	CTS_LIST_PLOG_SHORTMSG_STR,/**< . */
	CTS_LIST_PLOG_RELATED_ID_INT/**< person id */
};

/**
 * Contact List for OSP(C++)
 * Usually, This is sorted by name related with #CTS_ORDER_OF_SORTING
 */
enum OSPLIST{
	CTS_LIST_OSP_PERSON_ID_INT,/**< . */
	CTS_LIST_OSP_CONTACT_ID_INT,/**< . */
	CTS_LIST_OSP_ADDRESSBOOK_ID_INT,/**< . */
	CTS_LIST_OSP_IMG_PATH_STR,/**< . */
	CTS_LIST_OSP_FIRST_STR,/**< . */
	CTS_LIST_OSP_LAST_STR,/**< . */
	CTS_LIST_OSP_DISPLAY_STR,/**< . */
	CTS_LIST_OSP_DEF_NUM_TYPE_INT,/**< Default number type */
	CTS_LIST_OSP_DEF_NUM_STR,/**< Default number */
	CTS_LIST_OSP_DEF_EMAIL_TYPE_INT,/**< Default email type */
	CTS_LIST_OSP_DEF_EMAIL_STR,/**< Default email */
	CTS_LIST_OSP_NORMALIZED_STR,/**< . */
};

/**
 * Contact List
 * Usually, This is sorted by name related with #CTS_ORDER_OF_SORTING
 */
enum CONTACTLIST{
	CTS_LIST_CONTACT_ID_INT,/**< . */
	CTS_LIST_CONTACT_IMG_PATH_STR,/**< . */
	CTS_LIST_CONTACT_FIRST_STR,/**< . */
	CTS_LIST_CONTACT_LAST_STR,/**< . */
	CTS_LIST_CONTACT_DISPLAY_STR,/**< . */
	CTS_LIST_CONTACT_NUM_OR_EMAIL_STR,/**< optional. related with #CTS_LIST_ALL_EMAIL_NUMBER */
	CTS_LIST_CONTACT_NORMALIZED_STR,/**< optional */
	CTS_LIST_CONTACT_ADDRESSBOOK_ID_INT,/**< . */
	CTS_LIST_CONTACT_PERSON_ID_INT,/**< . */
};

/**
 * Number List
 */
enum NUMBERLIST{
	CTS_LIST_NUM_CONTACT_ID_INT,/**< . */
	CTS_LIST_NUM_CONTACT_IMG_PATH_STR,/**< . */
	CTS_LIST_NUM_CONTACT_FIRST_STR,/**< . */
	CTS_LIST_NUM_CONTACT_LAST_STR,/**< . */
	CTS_LIST_NUM_CONTACT_DISPLAY_STR,/**< . */
	CTS_LIST_NUM_NUMBER_STR, /**< . */
	CTS_LIST_NUM_PERSON_ID_INT,/**< . */
};

/**
 * Email List
 */
enum EMAILLIST{
	CTS_LIST_EMAIL_CONTACT_ID_INT,/**< . */
	CTS_LIST_EMAIL_CONTACT_IMG_PATH_STR,/**< . */
	CTS_LIST_EMAIL_CONTACT_FIRST_STR,/**< . */
	CTS_LIST_EMAIL_CONTACT_LAST_STR,/**< . */
	CTS_LIST_EMAIL_CONTACT_DISPLAY_STR,/**< . */
	CTS_LIST_EMAIL_ADDR_STR,/**< . */
	CTS_LIST_EMAIL_PERSON_ID_INT /**< . */
};


/**
 * Change List
 */
enum CHANGELIST{
	CTS_LIST_CHANGE_ID_INT,/**< . */
	CTS_LIST_CHANGE_TYPE_INT, /**< #CTS_OPERATION_UPDATED, #CTS_OPERATION_DELETED, #CTS_OPERATION_INSERTED */
	CTS_LIST_CHANGE_VER_INT,/**< The version when this contact is changed */
	CTS_LIST_CHANGE_ADDRESSBOOK_ID_INT, /**< The version when this contact is changed */
};

enum {
	CTS_OPERATION_UPDATED, /**< . */
	CTS_OPERATION_DELETED, /**< . */
	CTS_OPERATION_INSERTED /**< . */
};

/**
 * Addressbook List
 * Usually, This is sorted by acc_id and addressbook id
 * Though it is same with ADDRESSBOOKVALUE, Use this for list
 */
enum ADDRESSBOOKLIST{
	CTS_LIST_ADDRESSBOOK_ID_INT, /**< . */
	CTS_LIST_ADDRESSBOOK_NAME_STR, /**< . */
	CTS_LIST_ADDRESSBOOK_ACC_ID_INT, /**< The related account id */
	CTS_LIST_ADDRESSBOOK_ACC_TYPE_INT, /**< #ADDRESSBOOKTYPE */
	CTS_LIST_ADDRESSBOOK_MODE_INT, /**< #ADDRESSBOOKPERMISSION */
};

/**
 * Custom Number Type List
 */
enum CUSTOMNUMTYPELIST{
	CTS_LIST_CUSTOM_NUM_TYPE_ID_INT,/**< . */
	CTS_LIST_CUSTOM_NUM_TYPE_NAME_STR,/**< . */
};


/**
 * Group List
 * Usually, This is sorted by addressbook_id and name.
 */
enum GROUPLIST{
	CTS_LIST_GROUP_ID_INT = CTS_GROUP_VAL_ID_INT,/**< . */
	CTS_LIST_GROUP_ADDRESSBOOK_ID_INT = CTS_GROUP_VAL_ADDRESSBOOK_ID_INT,/**< . */
	CTS_LIST_GROUP_NAME_STR = CTS_GROUP_VAL_NAME_STR,/**< . */
	CTS_LIST_GROUP_RINGTONE_STR = CTS_GROUP_VAL_RINGTONE_STR,/**< . */
	CTS_LIST_GROUP_IMAGE_STR = CTS_GROUP_VAL_IMG_PATH_STR,/**< . */
};

/**
 * Favorite List or Speeddial List
 */
enum SHORTCUTLIST{
	CTS_LIST_SHORTCUT_ID_INT,/**< . */
	CTS_LIST_SHORTCUT_PERSON_ID_INT,/**< . */
	CTS_LIST_SHORTCUT_FIRST_NAME_STR,/**< . */
	CTS_LIST_SHORTCUT_LAST_NAME_STR,/**< . */
	CTS_LIST_SHORTCUT_DISPLAY_NAME_STR,/**< . */
	CTS_LIST_SHORTCUT_IMG_PATH_STR,/**< . */
	CTS_LIST_SHORTCUT_NUMBER_STR,/**< only for #CTS_FAVOR_NUMBER */
	CTS_LIST_SHORTCUT_NUMBER_TYPE_INT,/**< only for #CTS_FAVOR_NUMBER */
	CTS_LIST_SHORTCUT_SPEEDDIAL_INT /**< only for #CTS_LIST_ALL_SPEEDDIAL */
};

/**
 * deprecated
 */
#define CTS_LIST_SHORTCUT_CONTACT_ID_INT CTS_LIST_SHORTCUT_PERSON_ID_INT


/**
 * SDN(Service Dialing Number) List
 */
enum SDNLIST{
	CTS_LIST_SDN_NAME_STR,/**< . */
	CTS_LIST_SDN_NUMBER_STR,/**< . */
};

/**
 * Use for contacts_svc_get_list().
 */
typedef enum{
	CTS_LIST_ALL_CONTACT, /**< #CONTACTLIST */
	CTS_LIST_ALL_GROUP,/**< #GROUPLIST */
	CTS_LIST_ALL_CUSTOM_NUM_TYPE,/**< #CUSTOMNUMTYPELIST */
	CTS_LIST_ALL_CONTACT_FAVORITE,/**< #SHORTCUTLIST */
	CTS_LIST_ALL_SPEEDDIAL,/**< #SHORTCUTLIST */
	CTS_LIST_GROUPING_PLOG,/**< #PHONELOGLIST */
	CTS_LIST_GROUPING_MSG_PLOG,/**< #PHONELOGLIST */
	CTS_LIST_GROUPING_CALL_PLOG,/**< #PHONELOGLIST */
	CTS_LIST_ALL_SDN,/**< #SDNLIST */
	CTS_LIST_ALL_CONTACT_HAD_NUMBER,/**< #CONTACTLIST */
	CTS_LIST_ALL_CONTACT_HAD_EMAIL,/**< #CONTACTLIST */
	CTS_LIST_ALL_EMAIL_NUMBER,/**< #CONTACTLIST */
	CTS_LIST_ALL_NUMBER_FAVORITE,/**< #SHORTCUTLIST */
	CTS_LIST_OFTEN_USED_CONTACT, /**< #CONTACTLIST. sorted by count of using(using means outgoing call/video call)*/
	CTS_LIST_ALL_ADDRESSBOOK, /**< #ADDRESSBOOKLIST */
	CTS_LIST_ALL_PLOG, /**< #PHONELOGLIST */
	CTS_LIST_ALL_MISSED_CALL, /**< #PHONELOGLIST */
	CTS_LIST_ALL_NUMBER, /**< #CONTACTLIST */
	CTS_LIST_ALL_UNSEEN_MISSED_CALL, /**< #PHONELOGLIST */
	CTS_LIST_ALL_CONTACT_FAVORITE_HAD_NUMBER,/**< #SHORTCUTLIST */
	CTS_LIST_ALL_CONTACT_FAVORITE_HAD_EMAIL,/**< #SHORTCUTLIST */
	CTS_LIST_ALL_EMAIL_PLOG, /**< #PHONELOGLIST */
}cts_get_list_op;
/**
 * This function gets iterator of the gotten data by op_code.
 * \n Obtained iterator should be free using by contacts_svc_iter_remove().
 *
 * @param[in] op_code #cts_get_list_op
 * @param[out] iter Point of data iterator
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @see contacts_svc_get_list_with_str(), contacts_svc_get_list_with_int()
 * @par example
 * @code
 void get_contact_list(void)
 {
    CTSiter *iter = NULL;
    contacts_svc_get_list(CTS_LIST_ALL_CONTACT, &iter);

    while(CTS_SUCCESS == contacts_svc_iter_next(iter))
    {
       CTSvalue *contact = NULL;
       char *first, *last, *display;
       contacts_svc_iter_get_info(iter, &contact);

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
 * @endcode
 */
int contacts_svc_get_list(cts_get_list_op op_code, CTSiter **iter);

/**
 * Use for contacts_svc_get_list_with_str().
 */
typedef enum{
	CTS_LIST_PLOGS_OF_NUMBER,/**< #PHONELOGLIST */
	CTS_LIST_CONTACTS_WITH_NAME,/**< #CONTACTLIST */
	CTS_LIST_NUMBERINFOS_WITH_NAME,/**< #NUMBERLIST */
	CTS_LIST_NUMBERINFOS_WITH_NUM,/**< #NUMBERLIST */
	CTS_LIST_EMAILINFOS_WITH_EMAIL,/**< #EMAILLIST */
	//CTS_LIST_NUMBERS_EMAILS_WITH_NAME,/**< #EMAILLIST */
}cts_get_list_str_op;
/**
 * This function gets iterator of the gotten data by op_code with string search value.
 * \n search_value is related with op_code. The Word after preposition is a property of search_value.
 * \n Obtained iterator should be free using by contacts_svc_iter_remove().
 *
 * @param[in] op_code #cts_get_list_str_op
 * @param[in] search_value String search value
 * @param[out] iter Point of data iterator to be got
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @see contacts_svc_get_list(), contacts_svc_get_list_with_int()
 */
int contacts_svc_get_list_with_str(cts_get_list_str_op op_code, const char *search_value, CTSiter **iter);

/**
 * Use for contacts_svc_get_list_with_int().
 */
typedef enum{
	CTS_LIST_MEMBERS_OF_GROUP_ID,/**< #CONTACTLIST */
	CTS_LIST_MEMBERS_OF_ADDRESSBOOK_ID,/**< #CONTACTLIST */
	CTS_LIST_NO_GROUP_MEMBERS_OF_ADDRESSBOOK_ID, /**< #CONTACTLIST */
	CTS_LIST_GROUPS_OF_ADDRESSBOOK_ID, /**< #GROUPLIST */
	CTS_LIST_ADDRESSBOOKS_OF_ACCOUNT_ID, /**< #ADDRESSBOOKLIST */
	CTS_LIST_MEMBERS_OF_PERSON_ID, /**< #CONTACTLIST */
	CTS_LIST_NO_GROUP_MEMBERS_HAD_NUMBER_OF_ADDRESSBOOK_ID, /**< #CONTACTLIST */
	CTS_LIST_NO_GROUP_MEMBERS_HAD_EMAIL_OF_ADDRESSBOOK_ID, /**< #CONTACTLIST */
	CTS_LIST_PLOG_OF_PERSON_ID,/**< #PHONELOGLIST */
	//CTS_LIST_EMAILS_OF_CONTACT_ID,/**< only use #CTS_LIST_EMAIL_CONTACT_ID_INT, #CTS_LIST_EMAIL_ADDR_STR */
	//CTS_LIST_NUMBERS_OF_CONTACT_ID,/**< only use #CTS_LIST_NUM_CONTACT_ID_INT, #CTS_LIST_NUM_NUMBER_STR */
}cts_get_list_int_op;
/**
 * This function gets iterator of the gotten data by op_code with integer search value.
 * \n search_value is related with op_code. The Word after preposition is a property of search_value.
 * \n Obtained iterator should be free using by contacts_svc_iter_remove().
 *
 * @param[in] op_code #cts_get_list_int_op
 * @param[in] search_value Integer search value
 * @param[out] iter Point of data iterator to be got
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @see contacts_svc_get_list(), contacts_svc_get_list_with_str()
 */
int contacts_svc_get_list_with_int(cts_get_list_int_op op_code, unsigned int search_value, CTSiter **iter);

/**
 * This function gets iterator of the gotten data related to updated contacts since the version(not include version).
 * If contact includes both insert and update changes after version, the change type of contact is #CTS_OPERATION_INSERTED.
 * If you want to get the last contacts version, use transaction explicitly.
 * contacts_svc_end_trans() return the last contacts version.
 * Obtained iterator should be free using by contacts_svc_iter_remove().
 *
 * @param[in] addressbook_id The index of addressbook. 0 is local(phone internal)
 * @param[in] version The contact version gotten by contacts_svc_end_trans().
 * @param[out] iter Point of data iterator to be got(#CHANGELIST)
 * @return #CTS_SUCCESS on success, #CTS_ERR_DB_RECORD_NOT_FOUND on No change, Other negative value(#cts_error) on error,
 *
 * @see #CHANGELIST
 * @par example
 * @code
  void sync_data(void)
  {
     int ret, version=0, index_num;
     CTSiter *iter = NULL;
     contacts_svc_get_updated_contacts(0, version, &iter);

     while(CTS_SUCCESS == contacts_svc_iter_next(iter))
     {
        CTSstruct *contact= NULL;
        CTSvalue *row_info = NULL;
        row_info = contacts_svc_iter_get_info(iter);

        index_num = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_ID_INT);
        printf("(%8d)\n", index_num);
        int type = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_TYPE_INT);
        int ver = contacts_svc_value_get_int(row_info, CTS_LIST_CHANGE_TIME_INT);

        if(CTS_OPERATION_UPDATED == type || CTS_OPERATION_INSERTED == type) {
           contacts_svc_get_contact(index_num, &contact);
           void *vcard_stream;
           char file[128];
           snprintf(file, sizeof(file), "test%d.vcf", index_num);
           ret = contacts_svc_get_vcard_from_contact(contact, &vcard_stream);
           if(CTS_SUCCESS == ret) {
              //int fd = open(file, O_RDWR | O_CREAT);
              //write(fd, (char *)vcard_stream, strlen((char *)vcard_stream));
              //close(fd);
              CTSstruct *new_contact = NULL;
              ret = contacts_svc_get_contact_from_vcard(vcard_stream, &new_contact);
              if(CTS_SUCCESS == ret) {
                 get_contact(new_contact);
                 contacts_svc_struct_free(new_contact);
              }
              free(vcard_stream);
           }
           if(CTS_OPERATION_INSERTED == type)
              printf("Added : %d \n", ver);
           else
              printf("Updated : %d \n", ver);
           contacts_svc_struct_free(contact);
        }
        else
           printf("Deleted : %d \n", ver);

        contacts_svc_value_free(row_info);
     }
     contacts_svc_iter_remove(iter);
  }
 * @endcode
 */
int contacts_svc_get_updated_contacts(int addressbook_id,
      int version, CTSiter **iter);

/**
 * This function reads information from the iterator.
 * Obtained information should be free using by contacts_svc_value_free().
 *
 * @param[in] iter The data iterator
 * @return The gotten information, or NULL if no value is obtained or error
 */
CTSvalue* contacts_svc_iter_get_info(CTSiter *iter);

/**
 * This function removes the iterator.
 * \n You should call this function after using iterator.
 *
 * @param[in] iter The data iterator
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_iter_remove(CTSiter *iter);

/**
 * This function moves the iterator to the next record, if any.
 * Must also be called before reading first record, to determine
 * whether there is such a record at all.
 * If there's no next record, returns #CTS_ERR_FINISH_ITER.
 *
 * @param[in] iter The data iterator
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_iter_next(CTSiter *iter);

/**
 * This is the signature of a callback function added with contacts_svc_list_foreach(),
 * contacts_svc_list_with_int_foreach() and contacts_svc_list_with_str_foreach().
 * \n This function is invoked in the above functions.
 * \n If this function doesn't return #CTS_SUCCESS, foreach function is terminated.
 *
 * @param[in] value data of a record.
 * @param[in] user_data The data which is set by contacts_svc_list_foreach(),
 * contacts_svc_list_with_int_foreach() and contacts_svc_list_with_str_foreach().
 * @return #CTS_SUCCESS on success, other value on error
 */
typedef int (*cts_foreach_fn)(CTSvalue *value, void *user_data);


/**
 * This function calls #cts_foreach_fn for each record of list gotten by op_code.
 *
 * @param[in] op_code #cts_get_list_op
 * @param[in] cb callback function pointer(#cts_foreach_fn)
 * @param[in] user_data data which is passed to callback function
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @see contacts_svc_get_list()
 */
int contacts_svc_list_foreach(cts_get_list_op op_code,
   cts_foreach_fn cb, void *user_data);

/**
 * This function calls #cts_foreach_fn for each record of list gotten by op_code.
 *
 * @param[in] op_code #cts_get_list_int_op
 * @param[in] search_value Integer search value
 * @param[in] cb callback function pointer(#cts_foreach_fn)
 * @param[in] user_data data which is passed to callback function
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @see contacts_svc_get_list_with_int()
 */
int contacts_svc_list_with_int_foreach(cts_get_list_int_op op_code,
   unsigned int search_value, cts_foreach_fn cb, void *user_data);

/**
 * This function calls #cts_foreach_fn for each record of list gotten by op_code.
 *
 * @param[in] op_code #cts_get_list_str_op
 * @param[in] search_value String search value
 * @param[in] cb callback function pointer(#cts_foreach_fn)
 * @param[in] data data which is passed to callback function
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @see contacts_svc_get_list_with_str()
 */
int contacts_svc_list_with_str_foreach(cts_get_list_str_op op_code,
   const char *search_value, cts_foreach_fn cb, void *data);

/**
 * It is the smartsearch exclusive function. It is supported for only smartsearch(inhouse application).
 * It can be changed without announcement.
 * This function calls #cts_foreach_fn for each record of list.
 *
 * @param[in] search_str String search value(number or name)
 * @param[in] limit an upper bound on the number of result. If it has a negative value, there is no upper bound.
 * @param[in] offset It omits offset rows for the result.
 * @param[in] cb callback function pointer(#cts_foreach_fn) with #CTSvalue(#NUMBERLIST)
 * @param[in] user_data data which is passed to callback function
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_smartsearch_excl(const char *search_str, int limit, int offset,
   cts_foreach_fn cb, void *user_data);

/**
 * @}
 */
//-->

void cts_foreach_run(CTSiter *iter, cts_foreach_fn cb, void *data);


#endif //__CTS_LIST_H__


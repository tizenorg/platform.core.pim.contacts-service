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
#ifndef __CTS_STRUCT_H__
#define __CTS_STRUCT_H__

#include <stdbool.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>

#define CTS_NUMBER_MAX_LEN 512

#define SMART_STRDUP(src) (src && *src)?strdup(src):NULL
#define SAFE_STRDUP(src) (src)?strdup(src):NULL
#define FREEandSTRDUP(dest, src) \
	do{ \
		free(dest);\
		if (src) dest = strdup(src);\
		else dest = NULL; \
	}while (0)

enum {
	CTS_HANDLE_STR_GET,
	CTS_HANDLE_STR_STEAL,
};

#define HANDLE_STEAL_STRING(op_code, dest, src) \
	do{ \
		dest=src; \
		if (CTS_HANDLE_STR_STEAL == op_code) src=NULL; \
	}while (0)

#define CTS_DATA_FIELD_NAME (1<<0)
#define CTS_DATA_FIELD_POSTAL (1<<1)
#define CTS_DATA_FIELD_MESSENGER (1<<2)
#define CTS_DATA_FIELD_WEB (1<<3)
#define CTS_DATA_FIELD_EVENT (1<<4)
#define CTS_DATA_FIELD_COMPANY (1<<5)
#define CTS_DATA_FIELD_NICKNAME (1<<6)
#define CTS_DATA_FIELD_NUMBER (1<<7)
#define CTS_DATA_FIELD_EMAIL (1<<8)
#define CTS_DATA_FIELD_EXTEND_ALL (1<<9)
#define CTS_DATA_FIELD_ALL ((1<<9)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0))

enum {
	CTS_DATA_NAME = 1,
	CTS_DATA_POSTAL = 2,
	CTS_DATA_MESSENGER = 3,
	CTS_DATA_WEB = 4,
	CTS_DATA_EVENT = 5,
	CTS_DATA_COMPANY = 6,
	CTS_DATA_NICKNAME = 7,
	CTS_DATA_NUMBER = 8,
	CTS_DATA_EMAIL = 9,
	CTS_DATA_EXTEND_START = 100
};

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	int type;
	union {
		int i;
		bool b;
		double d;
		char *s;
	}val;
}cts_basic; //CTS_BASIC_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	bool uid_changed;
	bool img_changed;
	bool full_img_changed;
	bool ringtone_changed;
	bool note_changed;
	bool is_favorite;
	int id;
	int person_id;
	int changed_time;
	int addrbook_id;
	char *uid;
	char *img_path;
	char *full_img_path;
	char *ringtone_path;
	char *note;
	char *vcard_img_path;
}cts_ct_base; //CTS_BASE_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	bool is_changed;
	int id;
	int lang_type;
	char *first;
	char *last;
	char *addition;
	char *display;
	char *prefix;
	char *suffix;
}cts_name; //CTS_NAME_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	bool is_default;
	bool is_favorite;
	int id;
	int type;
	char *number;
	char *added_type;
}cts_number; //CTS_NUM_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	bool is_default;
	int id;
	int type;
	char *email_addr;
}cts_email; //CTS_EMAIL_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	int id;
	int type;
	char *url;
}cts_web; //CTS_WEB_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	bool is_default;
	int id;
	int type;
	char *pobox;
	char *postalcode;
	char *region;
	char *locality;
	char *street;
	char *extended;
	char *country;
}cts_postal; //CTS_POSTAL_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	int id;
	int type;
	int date;
}cts_event;//CTS_EVENT_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	int id;
	int type;
	char *im_id;
	char *svc_name;
	char *svc_op;
}cts_messenger;//CTS_MESSENGER_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	int id;
	char *nick;
}cts_nickname; //CTS_NICKNAME_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	bool img_loaded;
	int id;
	int addrbook_id;
	char *name;
	char *ringtone_path;
	char *vcard_group;
	char *img_path;
}cts_group; //CTS_GROUP_VAL_ or CTS_GROUPREL_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	int id;
	int acc_id;
	int acc_type;
	int mode;
	char *name;
}cts_addrbook; //CTS_ADDRESSBOOK_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted; /* not used */
	int id;
	char *name;
	char *department;
	char *jot_title;
	char *role;
	char *assistant_name;
}cts_company;//CTS_COMPANY_VAL_

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	int id;
	char *number;
	int related_id; /* person id */
	int log_time;
	int log_type;
	int extra_data1; /* duration, message_id */
	char *extra_data2; /*short message*/
}cts_plog;//PHONELOGVALUE

typedef struct {
	int v_type:16;
	bool embedded;
	bool deleted;
	int id;
	int type;
	int data1;
	char *data2;
	char *data3;
	char *data4;
	char *data5;
	char *data6;
	char *data7;
	char *data8;
	char *data9;
	char *data10;
}cts_extend;//EXTENDVALUE

typedef struct {
	int v_type:16;
	bool embedded;
	int id;
	int num_type;
	char *first;
	char *last;
	char *display;
	char *number;
	char *img_path;
	int log_time;
	int log_type;
	int extra_data1; /* duration, message_id */
	char *extra_data2; /*short message*/
	int related_id; /* contact id */
	int person_id;
}plog_list;//CTS_LIST_PLOG_

typedef struct {
	int v_type:16;
	bool embedded;
	int person_id;
	int contact_id;
	int addrbook_id;
	char *img_path;
	char *first;
	char *last;
	char *display;
	char *connect;
	char *normalize;
}contact_list;//CTS_LIST_CONTACT_

typedef struct {
	int v_type:16;
	bool embedded;
	int person_id;
	int contact_id;
	int addrbook_id;
	char *img_path;
	char *first;
	char *last;
	char *display;
	int def_num_type;
	char *def_num;
	int def_email_type;
	char *def_email;
	char *normalize;
}osp_list;//OSPLIST

typedef struct {
	int v_type:16;
	bool embedded;
	char *name;
	char *number;
}sdn_list;//SDNLIST

typedef struct {
	int v_type:16;
	bool embedded;
	int changed_type:8;
	int id;
	int changed_ver;
	int addressbook_id;
}change_list;//CTS_LIST_CHANGE_

typedef struct {
	int v_type:16;
	bool embedded;
	int id;
	int account_type;
	char *name;
}addrbook_list;//CTS_LIST_ADDRESSBOOK_

typedef struct {
	int v_type:16;
	bool embedded;
	int id;
	char *name;
}numtype_list;//CUSTOMNUMTYPELIST

typedef struct {
	int v_type:16;
	bool embedded;
	int id;
	int person_id;
	int contact_id;
	char *first;
	char *last;
	char *display;
	char *number;
	char *img_path;
	int num_type;
	int speeddial;
}shortcut_list;//CTS_LIST_FAVORITE_

typedef struct {
	int s_type;
	int is_restricted;
	cts_ct_base *base;
	cts_name *name;
	GSList *numbers;
	GSList *emails;
	GSList *grouprelations;
	GSList *events;
	GSList *messengers;
	GSList *postal_addrs;
	GSList *web_addrs;
	GSList *nicknames;
	cts_company *company;
	int default_num;
	int default_email;
	GSList *extended_values;
}contact_t; //cts_struct_field

enum{
	CTS_VALUE_BASIC = 100, /**< Deprecated */
	CTS_VALUE_LIST_CONTACT = 101,
	CTS_VALUE_LIST_ADDRBOOK, // ADDRESSBOOKLIST order must be same to ADDRESSBOOKVALUE order.
	CTS_VALUE_LIST_PLOG,
	CTS_VALUE_LIST_CUSTOM_NUM_TYPE,
	CTS_VALUE_LIST_CHANGE,
	CTS_VALUE_LIST_GROUP,
	CTS_VALUE_LIST_NUMBERINFO, // NUMBERLIST order must be same to EMAILLIST order.
	CTS_VALUE_LIST_EMAILINFO, // EMAILLIST order must be same to NUMBERLIST order.
	CTS_VALUE_LIST_NUMS_EMAILS,
	CTS_VALUE_LIST_SDN,
	CTS_VALUE_RDONLY_NAME,
	CTS_VALUE_RDONLY_NUMBER,
	CTS_VALUE_RDONLY_EMAIL,
	CTS_VALUE_RDONLY_COMPANY,
	CTS_VALUE_LIST_SHORTCUT,
	CTS_VALUE_RDONLY_PLOG,
	CTS_VALUE_LIST_OSP,
};

//basic
enum {
	CTS_BASIC_VAL_INT,
	CTS_BASIC_VAL_DBL,
	CTS_BASIC_VAL_BOOL,
	CTS_BASIC_VAL_STR
};

#ifndef __CONTACTS_SVC_H__

struct cts_struct{
	int s_type;
};

struct cts_value{
	int v_type:16;
	bool embedded;
	bool deleted;
};

//<!--

/**
 * CTSstruct is an opaque type, it must be used via accessor functions.
 * Conceptually, each Struct is a set of values and list of values.
 *
 * To remove a value from a list of values, set the VAL_DELETE
 * field in the value and store the list.
 *
 * @see contacts_svc_struct_new(), contacts_svc_struct_free()
 * @see contacts_svc_struct_get_value(), contacts_svc_struct_get_list(),
 * @see contacts_svc_struct_store_value(), contacts_svc_struct_store_list()
 */
typedef struct cts_struct CTSstruct;

/**
 * CTSvalue is an opaque type, it must be
 * used via accessor functions. Conceptually, each value is
 * a tuple of fields with a fixed type per field, with each field
 * accessed inside the value via a fixed index number (for example,
 * #NAMEVALUE). Supported types are int (value range as in C int), string
 * and boolean (C++ bool). The right access methods must be used depending
 * on the field type.
 *
 * @see contacts_svc_value_new(), contacts_svc_value_free()
 * @see contacts_svc_value_set_int(), contacts_svc_value_set_bool(), contacts_svc_value_set_str()
 * @see contacts_svc_value_get_int(), contacts_svc_value_get_bool(), contacts_svc_value_get_str()
 */
typedef struct cts_value CTSvalue;


//////////////////// Value ////////////////////
/**
 * Use for contacts_svc_value_new()
 *
 * Unless noted otherwise, these values are storded in a contact
 * Struct. Some of those values may occur more than once per contact
 * Struct. Those values are stored in lists, see #cts_struct_field.
 */
typedef enum
{
	CTS_VALUE_CONTACT_BASE_INFO,/**< #BASEVALUE */
	CTS_VALUE_NAME,/**< #NAMEVALUE */
	CTS_VALUE_NUMBER,/**< #NUMBERVALUE */
	CTS_VALUE_EMAIL,/**< #EMAILVALUE */
	CTS_VALUE_WEB,/**< #WEBVALUE */
	CTS_VALUE_POSTAL,/**< #POSTALVALUE */
	CTS_VALUE_EVENT,/**< #EVENTVALUE */
	CTS_VALUE_MESSENGER,/**< #MESSENGERVALUE */
	CTS_VALUE_GROUP_RELATION,/**< #GROUPRELATIONVALUE */
	CTS_VALUE_COMPANY,/**< #COMPANYVALUE */
	CTS_VALUE_PHONELOG,/**< #PHONELOGVALUE, not part of a contact, see contacts_svc_insert_phonelog() and friends */
	CTS_VALUE_GROUP,/**< #GROUPVALUE, not part of a contact, see contacts_svc_insert_group() */
	CTS_VALUE_EXTEND,/**< #EXTENDVALUE(@ref CONTACTS_SVC_EXTEND) */
	CTS_VALUE_NICKNAME,/**< #NICKNAMEVALUE */
	CTS_VALUE_ADDRESSBOOK /**< #ADDRESSBOOKVALUE, not part of a contact, see contacts_svc_insert_addressbook() */
}cts_value_type;

/**
 * base information
 */
enum BASEVALUE {
	CTS_BASE_VAL_ID_INT,/**< A contact index number. read only */
	CTS_BASE_VAL_CHANGED_TIME_INT,/**< read only, The time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.
											  It is based on system. If system time is invalid, this value is invalid too.*/
	CTS_BASE_VAL_IMG_PATH_STR, /**< A thumbnail image. Should include extension at path */
	CTS_BASE_VAL_RINGTONE_PATH_STR, /**< .*/
	CTS_BASE_VAL_NOTE_STR, /**< .*/
	CTS_BASE_VAL_UID_STR, /**< A globally (including outside of the device) unique ID. */
	CTS_BASE_VAL_ADDRESSBOOK_ID_INT, /**< read only. Each contact is assigned to a addressbook. */
	CTS_BASE_VAL_FULL_IMG_PATH_STR, /**< For full screen image. Should include extension at path */
	CTS_BASE_VAL_FAVORITE_BOOL, /**< read only. Use contacts_svc_set_favorite(CTS_FAVOR_CONTACT). It can assign for handling struct. But the changes are ignored */
	CTS_BASE_VAL_PERSON_ID_INT /**< read only */
};

/**
 * name
 */
enum NAMEVALUE {
	CTS_NAME_VAL_FIRST_STR,/**< for example, John */
	CTS_NAME_VAL_LAST_STR,/**< for example, Doe */
	CTS_NAME_VAL_ADDITION_STR,/**< also known as "middle name(s)" */
	CTS_NAME_VAL_SUFFIX_STR,/**< for example, Jr. for "Junior" */
	CTS_NAME_VAL_PREFIX_STR,/**< for example, Mr. for "Mister" */
	CTS_NAME_VAL_DISPLAY_STR,/**< see #CONTACTS_SVC_NAME */
};

/**
 * A phone number
 */
enum NUMBERVALUE {
	CTS_NUM_VAL_ID_INT,/**< read only, for use in contacts_svc_set_favorite(CTS_FAVOR_NUMBER) */
	CTS_NUM_VAL_TYPE_INT,/**< you can use #NUMBERTYPE or contacts_svc_find_custom_type().*/
	CTS_NUM_VAL_DEFAULT_BOOL,/**< */
	CTS_NUM_VAL_FAVORITE_BOOL, /**< read only. Set with contacts_svc_set_favorite(CTS_FAVOR_NUMBER). It can assign for handling struct. But the changes are ignored */
	CTS_NUM_VAL_DELETE_BOOL,/**< request to delete in the list of #CTSstruct. */
	CTS_NUM_VAL_NUMBER_STR,/**< .*/
};

/**
 * email
 */
enum EMAILVALUE {
	CTS_EMAIL_VAL_ID_INT,/**< read only */
	CTS_EMAIL_VAL_TYPE_INT,/**< #EMAILTYPE.*/
	CTS_EMAIL_VAL_DEFAULT_BOOL,/**< */
	CTS_EMAIL_VAL_DELETE_BOOL,/**< request to delete in the list of #CTSstruct. */
	CTS_EMAIL_VAL_ADDR_STR,/**< .*/
};

/**
 * group
 */
enum GROUPVALUE {
	CTS_GROUP_VAL_ID_INT = 0, /**< read only */
	CTS_GROUP_VAL_ADDRESSBOOK_ID_INT = 1, /**< . */
	CTS_GROUP_VAL_NAME_STR = 2,/**< . */
	CTS_GROUP_VAL_RINGTONE_STR = 3,/**< . */
	CTS_GROUP_VAL_IMG_PATH_STR = 4,/**< . */
};

/**
 * group relation information for contact
 */
enum GROUPRELATIONVALUE {
	CTS_GROUPREL_VAL_ID_INT = CTS_GROUP_VAL_ID_INT, /**< [write only]group id */
	CTS_GROUPREL_VAL_DELETE_BOOL = 1,/**< request to delete in the list of #CTSstruct. */
	CTS_GROUPREL_VAL_NAME_STR = CTS_GROUP_VAL_NAME_STR,/**< read only, but it can assign for handling struct(Not recommend) */
	CTS_GROUPREL_VAL_RINGTONE_STR = CTS_GROUP_VAL_RINGTONE_STR,/**< read only */
   CTS_GROUPREL_VAL_IMG_PATH_STR = CTS_GROUP_VAL_IMG_PATH_STR,/**< read only */
};

/**
 * event, for example birthday
 */
enum EVENTVALUE {
	CTS_EVENT_VAL_TYPE_INT,/**< #EVENTTYPE*/
	CTS_EVENT_VAL_DELETE_BOOL,/**< request to delete in the list of #CTSstruct. */
	CTS_EVENT_VAL_DATE_INT,/**< The date(YYYYMMDD). ex) 20100107 : year = 2010, month = 01, day = 07 */
};

/**
 * web address
 */
enum WEBVALUE {
	CTS_WEB_VAL_TYPE_INT,/**< #WEBTYPE */
	CTS_WEB_VAL_DELETE_BOOL,/**< request to delete in the list of #CTSstruct. */
	CTS_WEB_VAL_ADDR_STR,/**< .*/
};

/**
 * postal address
 */
enum POSTALVALUE {
	CTS_POSTAL_VAL_TYPE_INT,/**< #ADDRESSTYPE*/
	CTS_POSTAL_VAL_DELETE_BOOL,/**< request to delete in the list of #CTSstruct. */
	CTS_POSTAL_VAL_POBOX_STR,/**< .*/
	CTS_POSTAL_VAL_POSTALCODE_STR,/**< .*/
	CTS_POSTAL_VAL_REGION_STR,/**< e.g., state or province */
	CTS_POSTAL_VAL_LOCALITY_STR,/**< e.g., city */
	CTS_POSTAL_VAL_STREET_STR,/**< .*/
	CTS_POSTAL_VAL_EXTENDED_STR,/**< .*/
	CTS_POSTAL_VAL_COUNTRY_STR,/**< .*/
	CTS_POSTAL_VAL_DEFAULT_BOOL,/**< */
};

/**
 * messenger
 */
enum MESSENGERVALUE {
	CTS_MESSENGER_VAL_TYPE_INT,/**< #cts_im_type */
	CTS_MESSENGER_VAL_DELETE_BOOL,/**< request to delete in the list of #CTSstruct. */
	CTS_MESSENGER_VAL_IM_ID_STR,/**< .*/
	CTS_MESSENGER_VAL_SERVICE_NAME_STR,/**< The name of unknown service. So, this is valid in CTS_IM_TYPE_NONE. */
	CTS_MESSENGER_VAL_SERVICE_OP_STR,/**< The service operation related to launch unknown application. So, this is valid in CTS_IM_TYPE_NONE. */
};

/**
 * company
 */
enum COMPANYVALUE {
	CTS_COMPANY_VAL_NAME_STR,/**< .*/
	CTS_COMPANY_VAL_DEPARTMENT_STR,/**< .*/
	CTS_COMPANY_VAL_JOB_TITLE_STR,/**< .*/
	CTS_COMPANY_VAL_ASSISTANT_NAME_STR,/**< .*/
	CTS_COMPANY_VAL_ROLE_STR,/**< .*/
};

/**
 * nickname
 */
enum NICKNAMEVALUE {
	CTS_NICKNAME_VAL_DELETE_BOOL,/**< request to delete in the list of #CTSstruct. */
	CTS_NICKNAME_VAL_NAME_STR,/**< .*/
};

/**
 * phone log
 */
enum PHONELOGVALUE {
	CTS_PLOG_VAL_ADDRESS_STR = 0,/**< number or email address*/
	CTS_PLOG_VAL_ID_INT,/**< read only */
	CTS_PLOG_VAL_LOG_TIME_INT,/**< The time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.*/
	CTS_PLOG_VAL_LOG_TYPE_INT,/**< #PLOGTYPE */
	CTS_PLOG_VAL_DURATION_INT,/**< seconds. */
	CTS_PLOG_VAL_SHORTMSG_STR,/**<  message short message or email subject */
	CTS_PLOG_VAL_MSGID_INT,/**< message id or email id */
	CTS_PLOG_VAL_RELATED_ID_INT,/**< contact id */
};

/** deprecated */
#define CTS_PLOG_VAL_NUMBER_STR CTS_PLOG_VAL_ADDRESS_STR

/**
 * addressbook
 */
enum ADDRESSBOOKVALUE
{
	CTS_ADDRESSBOOK_VAL_ID_INT, /**< read only */
	CTS_ADDRESSBOOK_VAL_NAME_STR, /**< . */
	CTS_ADDRESSBOOK_VAL_ACC_ID_INT, /**< The related account id */
	CTS_ADDRESSBOOK_VAL_ACC_TYPE_INT, /**< #ADDRESSBOOKTYPE */
	CTS_ADDRESSBOOK_VAL_MODE_INT, /**< #ADDRESSBOOKPERMISSION
											  It is only data. nothing to do in contacts-service */
};

/**
 * extended value
 * @ref CONTACTS_SVC_EXTEND
 *
 * See #CTS_TYPE_CLASS_EXTEND_DATA, contacts_svc_find_custom_type().
 */
enum EXTENDVALUE {
	CTS_EXTEND_VAL_DELETE_BOOL,/**< request to delete in the list of #CTSstruct. */
	CTS_EXTEND_VAL_DATA1_INT,/**< . */
	CTS_EXTEND_VAL_DATA2_STR,/**< . */
	CTS_EXTEND_VAL_DATA3_STR,/**< . */
	CTS_EXTEND_VAL_DATA4_STR,/**< . */
	CTS_EXTEND_VAL_DATA5_STR,/**< . */
	CTS_EXTEND_VAL_DATA6_STR,/**< . */
	CTS_EXTEND_VAL_DATA7_STR,/**< . */
	CTS_EXTEND_VAL_DATA8_STR,/**< . */
	CTS_EXTEND_VAL_DATA9_STR,/**< . */
	CTS_EXTEND_VAL_DATA10_STR,/**< . */
};


//////////////////// Struct ////////////////////
/**
 * Use for contacts_svc_struct_new()
 */
typedef enum {
	CTS_STRUCT_CONTACT = 0, /**< CTS_STRUCT_CONTACT */
}cts_struct_type;

/**
 * Contacts service struct fields
 * CF means Contact Field. Some of these
 * fields may only have one value (_VALUE suffix),
 * others have a list of values (_LIST suffix).
 */
typedef enum
{
	CTS_CF_NONE,/**< CTS_CF_NONE */
	CTS_CF_BASE_INFO_VALUE,/**< #BASEVALUE */
	CTS_CF_NAME_VALUE,/**< #NAMEVALUE */
	CTS_CF_COMPANY_VALUE,/**< #COMPANYVALUE */
	CTS_CF_VALUE_MAX,/**< CTS_CF_VALUE_MAX */
	CTS_CF_NUMBER_LIST,/**< List of #NUMBERVALUE */
	CTS_CF_EMAIL_LIST,/**< List of #EMAILVALUE */
	CTS_CF_GROUPREL_LIST,/**< List of #GROUPVALUE */
	CTS_CF_EVENT_LIST,/**< List of #EVENTVALUE */
	CTS_CF_MESSENGER_LIST,/**< List of #MESSENGERVALUE */
	CTS_CF_POSTAL_ADDR_LIST,/**< List of #POSTALVALUE */
	CTS_CF_WEB_ADDR_LIST,/**< List of #WEBVALUE */
	CTS_CF_NICKNAME_LIST,/**< List of #NICKNAMEVALUE */
	CTS_CF_FIELD_MAX,/**< CTS_CF_FIELD_MAX */
}cts_struct_field;

/**
 * Allocate, initialize and return a new contacts service struct.
 *
 * @param[in] type The type of contacts service struct
 * @return The pointer of New contacts service struct, NULL on error
 * @see contacts_svc_struct_free()
 */
CTSstruct* contacts_svc_struct_new(cts_struct_type type);

/**
 * A destructor for contacts service struct.
 *
 * @param[in] structure A contacts service struct
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @see contacts_svc_struct_new()
 */
int contacts_svc_struct_free(CTSstruct* structure);

/**
 * This function gets the contacts service value of a single-value field(_VALUE suffix) in the contacts service struct.
 * Must not be used with fields which contain a list(_LIST suffix).
 *
 * @param[in] structure A contacts service struct
 * @param[in] field The index of the contacts service value in contacts service struct.
 * @param[out] retval the contacts service value requested with field(should not be freed)
 * @return	#CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_struct_get_value(CTSstruct *structure, cts_struct_field field, CTSvalue** retval);

/**
 * This function sets the contacts service value of a single-value field(_VALUE suffix) in the contacts service struct.
 * \n For efficiency, Ownership of the contacts service value is transferred to the contacts service struct.
 * (Although values be free by contacts_svc_value_free, it is ignored automatically)
 * But string values of contacts service value are copied, and thus ownership of the original string
 * remains with the original creator of it.
 *
 * @param[in] structure A contacts service struct
 * @param[in] field The index of the contacts service value in contacts service struct.
 * @param[in] value the contacts service value to be set
 * @return	#CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_struct_store_value(CTSstruct *structure, cts_struct_field field, CTSvalue* value);

/**
 * This function gets the pointer to a glib singly-linked list holding all values of multi-value field(_LIST suffix) in the contacts service struct.
 * Must not be used with single-value fields.
 *
 * @param[in] structure A contacts service struct
 * @param[in] field The index of the glib singly-linked list in contacts service struct.
 * @param[out] retlist the glib singly-linked list requested with field(should not be freed or removed)
 * @return	#CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_struct_get_list(CTSstruct *structure, cts_struct_field field, GSList** retlist);

/**
 * This function sets the glib singly-linked list to the contacts service struct.
 * \n The list is copied.(list is needed to free)
 * But values(#CTSvalue) of the list are moved to the contacts service struct for efficiency.
 * (Although values be free by contacts_svc_value_free, it is ignored automatically)
 *
 * @param[in] structure A contacts service struct
 * @param[in] field The index of the glib singly-linked list in contacts service struct.
 * @param[in] list the glib singly-linked list to be set
 * @return	#CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_struct_store_list(CTSstruct *structure, cts_struct_field field, GSList* list);

/**
 * Allocate, initialize and return a new contacts service value.
 * @param[in] type The type of contacts service value
 * @return The pointer of New contacts service value, NULL on error
 * @see contacts_svc_value_free()
 */
CTSvalue* contacts_svc_value_new(cts_value_type type);

/**
 * A destructor for contacts service value.
 * If it is in struct, this function will ignore to free and return #CTS_ERR_ARG_INVALID.
 * @param[in] value A contacts service value
 * @return	#CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @see contacts_svc_value_new()
 */
int contacts_svc_value_free(CTSvalue* value);

/**
 * This function sets integer field in the contacts service value.
 * May only be used with fields of type integer (_INT suffix in enum).
 *
 * @param[in] value The contacts service value
 * @param[in] field The index of the integer field in the contacts service value.
 * @param[in] intval The integer value to be set.
 * @return	#CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_value_set_int(CTSvalue* value, int field, int intval);

/**
 * This function sets boolean field in the contacts service value.
 * May only be used with fields of type boolean (_BOOL suffix in enum).
 *
 * @param[in] value The contacts service value
 * @param[in] field The index of the boolean field in the contacts service value.
 * @param[in] boolval The boolean value to be set.
 * @return	#CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_value_set_bool(CTSvalue* value, int field, bool boolval);

/**
 * This function sets string field in the contacts service value.(call by reference)
 * May only be used with fields of type string (_STR suffix in enum).
 * \n If it is in struct, free old string and copy strval to struct.(call by value)
 * \n empty string is handled as NULL and thus will result in NULL being stored
 *
 * @param[in] value The contacts service value
 * @param[in] field The index of the string field in the contacts service value.
 * @param[in] strval The string value to be set.
 * @return	#CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_value_set_str(CTSvalue* value, int field, const char *strval);

/**
 * This function gets the type of the contacts service value.
 * @param[in] value The contacts service value
 * @return Value Type(#cts_value_type) on success, Negative value(#cts_error) on error
 */
int contacts_svc_value_get_type(CTSvalue* value);

/**
 * This function gets the pointer to a string field of the contacts service value.
 * May only be used with fields of type string (_STR suffix in enum).
 *
 * @param[in] value The contacts service value
 * @param[in] field The index of the string field in the contacts service value.
 * @return string value(should not be freed, never empty), or NULL if no value is obtained
 */
const char* contacts_svc_value_get_str(CTSvalue *value, int field);

/**
 * This function gets boolean value of a field in the contacts service value.
 * May only be used with fields of type boolean (_BOOL suffix in enum).
 *
 * @param[in] value The contacts service value
 * @param[in] field The index of the boolean field in the contacts service value.
 * @return boolean value, or false if no value is obtained
 */
bool contacts_svc_value_get_bool(CTSvalue* value, int field);

/**
 * This function gets Integer value of the contacts service value.
 * May only be used with fields of type integer (_INT suffix in enum).
 * @param[in] value The contacts service value
 * @param[in] field The index of the integer field in the contacts service value.
 * @return Integer value, or 0 if no value is obtained
 */
int contacts_svc_value_get_int(CTSvalue* value, int field);

//-->
#endif //__CONTACTS_SVC_H__

#endif //__CTS_STRUCT_H__


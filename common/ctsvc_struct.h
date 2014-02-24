/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Dohyung Jin <dh.jin@samsung.com>
 *                 Jongwon Lee <gogosing.lee@samsung.com>
 *                 Donghee Ye <donghee.ye@samsung.com>
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

#ifndef __TIZEN_SOCIAL_CTSVC_STRUCT_H__
#define __TIZEN_SOCIAL_CTSVC_STRUCT_H__

#include <stdbool.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <tzplatform_config.h>

#include "contacts_views.h"

#define CTSVC_IMG_FULL_PATH_SIZE_MAX 1024
#define CTSVC_NUMBER_MAX_LEN 512
#define CTS_IMG_FULL_LOCATION tzplatform_mkpath(TZ_USER_DATA,"contacts-svc/img")

#define SAFE_STR(src) (src)?src:""
#define SAFE_STRDUP(src) (src)?strdup(src):NULL
#define SAFE_STRLEN(src) ((src)?strlen(src):0)
#define FREEandSTRDUP(dest, src) \
	do{ \
		free(dest);\
		if (src) dest = strdup(src);\
		else dest = NULL; \
	}while (0)
#define GET_STR(copy, str)	(copy)?(SAFE_STRDUP(str)):(str)

enum {
	CTSVC_DATA_NAME = 1,
	CTSVC_DATA_POSTAL = 2,
	CTSVC_DATA_MESSENGER = 3,
	CTSVC_DATA_URL = 4,
	CTSVC_DATA_EVENT = 5,
	CTSVC_DATA_COMPANY = 6,
	CTSVC_DATA_NICKNAME = 7,
	CTSVC_DATA_NUMBER = 8,
	CTSVC_DATA_EMAIL = 9,
	CTSVC_DATA_PROFILE = 10,
	CTSVC_DATA_RELATIONSHIP = 11,
	CTSVC_DATA_NOTE = 12,
	CTSVC_DATA_IMAGE = 13,
	CTSVC_DATA_EXTENSION = 100
};

typedef enum {
	CTSVC_RECORD_INVALID = -1,
	CTSVC_RECORD_ADDRESSBOOK,
	CTSVC_RECORD_GROUP,
	CTSVC_RECORD_PERSON,
	CTSVC_RECORD_CONTACT,
	CTSVC_RECORD_MY_PROFILE,
	CTSVC_RECORD_SIMPLE_CONTACT,
	CTSVC_RECORD_NAME,
	CTSVC_RECORD_COMPANY,
	CTSVC_RECORD_NOTE,
	CTSVC_RECORD_NUMBER,
	CTSVC_RECORD_EMAIL,
	CTSVC_RECORD_URL,
	CTSVC_RECORD_EVENT,
	CTSVC_RECORD_NICKNAME,
	CTSVC_RECORD_ADDRESS,
	CTSVC_RECORD_MESSENGER,
	CTSVC_RECORD_GROUP_RELATION,
	CTSVC_RECORD_ACTIVITY,
	CTSVC_RECORD_ACTIVITY_PHOTO,
	CTSVC_RECORD_PROFILE,
	CTSVC_RECORD_RELATIONSHIP,
	CTSVC_RECORD_IMAGE,
	CTSVC_RECORD_EXTENSION,
	CTSVC_RECORD_UPDATED_INFO,
	CTSVC_RECORD_PHONELOG,
	CTSVC_RECORD_SPEEDDIAL,
	CTSVC_RECORD_SDN,
	CTSVC_RECORD_RESULT,
}ctsvc_record_type_e;

typedef enum {
	CTSVC_FILTER_BOOL,
	CTSVC_FILTER_INT,
	CTSVC_FILTER_LLI,
	CTSVC_FILTER_STR,
	CTSVC_FILTER_DOUBLE,
	CTSVC_FILTER_COMPOSITE,
}ctsvc_filter_type_e;

typedef struct{
	unsigned int property_id;
	int property_type;
	void* fields;
}property_info_s;

typedef int (*__ctsvc_record_create_cb)(contacts_record_h* out_record);
typedef int (*__ctsvc_record_destroy_cb)(contacts_record_h record, bool delete_child );
typedef int (*__ctsvc_record_clone_cb)(contacts_record_h record, contacts_record_h* out_record);

typedef int (*__ctsvc_record_get_str_cb)(contacts_record_h record, unsigned int property_id,char** out_str);
typedef int (*__ctsvc_record_get_str_p_cb)(contacts_record_h record, unsigned int property_id,char** out_str);
typedef int (*__ctsvc_record_get_int_cb)(contacts_record_h record, unsigned int property_id, int* out_value);
typedef int (*__ctsvc_record_get_bool_cb)(contacts_record_h record, unsigned int property_id, bool *value);
typedef int (*__ctsvc_record_get_lli_cb)(contacts_record_h record, unsigned int property_id, long long int *value);
typedef int (*__ctsvc_record_get_double_cb)(contacts_record_h record, unsigned int property_id, double *value);

typedef int (*__ctsvc_record_set_str_cb)(contacts_record_h record, unsigned int property_id, const char* value);
typedef int (*__ctsvc_record_set_int_cb)(contacts_record_h record, unsigned int property_id, int value);
typedef int (*__ctsvc_record_set_bool_cb)(contacts_record_h record, unsigned int property_id, bool value);
typedef int (*__ctsvc_record_set_lli_cb)(contacts_record_h record, unsigned int property_id, long long int value);
typedef int (*__ctsvc_record_set_double_cb)(contacts_record_h record, unsigned int property_id, double value);

typedef int (*__ctsvc_record_add_child_record_cb)(contacts_record_h record, unsigned int property_id, contacts_record_h child_record);
typedef int (*__ctsvc_record_remove_child_record_cb)(contacts_record_h record, unsigned int property_id, contacts_record_h child_record);
typedef int (*__ctsvc_record_get_child_record_count_cb)(contacts_record_h record, unsigned int property_id, unsigned int *count);
typedef int (*__ctsvc_record_get_child_record_at_p_cb)(contacts_record_h record, unsigned int property_id, int index, contacts_record_h* out_record);
typedef int (*__ctsvc_record_clone_child_record_list_cb)(contacts_record_h record, unsigned int property_id, contacts_list_h* out_list);

typedef struct {
	__ctsvc_record_create_cb create;
	__ctsvc_record_destroy_cb destroy;
	__ctsvc_record_clone_cb clone;
	__ctsvc_record_get_str_cb get_str;
	__ctsvc_record_get_str_p_cb get_str_p;
	__ctsvc_record_get_int_cb get_int;
	__ctsvc_record_get_bool_cb get_bool;
	__ctsvc_record_get_lli_cb get_lli;
	__ctsvc_record_get_double_cb get_double;
	__ctsvc_record_set_str_cb set_str;
	__ctsvc_record_set_int_cb set_int;
	__ctsvc_record_set_bool_cb set_bool;
	__ctsvc_record_set_lli_cb set_lli;
	__ctsvc_record_set_double_cb set_double;
	__ctsvc_record_add_child_record_cb add_child_record;
	__ctsvc_record_remove_child_record_cb remove_child_record;
	__ctsvc_record_get_child_record_count_cb get_child_record_count;
	__ctsvc_record_get_child_record_at_p_cb get_child_record_at_p;
	__ctsvc_record_clone_child_record_list_cb clone_child_record_list;
}ctsvc_record_plugin_cb_s;

typedef struct {
	int r_type;
	const ctsvc_record_plugin_cb_s *plugin_cbs;
	const char* view_uri;
	unsigned int property_max_count;
	unsigned char* properties_flags;
	unsigned char property_flag;
}ctsvc_record_s;

typedef struct  {
	int filter_type;
}ctsvc_filter_s;

typedef struct {
	int filter_type;
	char *view_uri;
	GSList *filter_ops;	//ctsvc_filter_operator_e op;
	GSList *filters;	//ctsvc_filter_h l_filter;
	property_info_s *properties;
	unsigned int property_count;
}ctsvc_composite_filter_s;

typedef struct  {
	int filter_type;
	int property_id;
	int match;
	union {
		bool b;
		int i;
		char *s;
		long long int l;
		double d;
	}value;
}ctsvc_attribute_filter_s;

typedef struct  {
	char* view_uri;
	ctsvc_composite_filter_s *filter;
	unsigned int *projection;
	unsigned int projection_count;
	unsigned int sort_property_id;
	bool sort_asc;
	property_info_s *properties;
	unsigned int property_count;
	bool distinct;
}ctsvc_query_s;

typedef struct {
	int l_type;
	int count;
	GList *records;
	GList *deleted_records;
	GList *cursor;
}ctsvc_list_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	char *name;
	int account_id;
	int mode;
}ctsvc_addressbook_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int addressbook_id;
	bool is_read_only;
	char *name;
	char *extra_data;
	char *ringtone_path;
	char *vibration;
	char *image_thumbnail_path;
	char *message_alert;
}ctsvc_group_s;

typedef struct {
	ctsvc_record_s base;
	bool is_favorite;
	bool has_phonenumber;
	bool has_email;
	int person_id;
	int name_contact_id;
	char *display_name;
	char *display_name_index;
	char *image_thumbnail_path;
	char *ringtone_path;
	char *vibration;
	char *message_alert;
	char *status;
	int link_count;
	char *addressbook_ids;
}ctsvc_person_s;

typedef struct {
	ctsvc_record_s base;
	bool is_favorite;
	int changed_time;
	bool has_phonenumber;
	bool has_email;
	int person_id;
	int contact_id;
	int addressbook_id;
	char *image_thumbnail_path;
	char *ringtone_path;
	char *vibration;
	char *message_alert;
	char *display_name;
	char *uid;
	int display_source_type;
}ctsvc_simple_contact_s;

typedef struct {
	ctsvc_record_s base;
	bool is_default;
	int id;
	int contact_id;
	int language_type;
	char *first;
	char *last;
	char *addition;
	char *prefix;
	char *suffix;
	char *phonetic_first;
	char *phonetic_middle;
	char *phonetic_last;
	char *lookup;
	char *reverse_lookup;
}ctsvc_name_s;

typedef struct {
	ctsvc_record_s base;
	bool is_default;
	int id;
	int contact_id;
	int type;
	char *label;
	char *number;
	char *lookup;		// internally used
}ctsvc_number_s;

typedef struct {
	ctsvc_record_s base;
	bool is_default;
	int id;
	int contact_id;
	int type;
	char *label;
	char *email_addr;
}ctsvc_email_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int contact_id;
	int type;
	char *label;
	char *url;
}ctsvc_url_s;

typedef struct {
	ctsvc_record_s base;
	bool is_default;
	int id;
	int contact_id;
	int type;
	char *label;
	char *pobox;
	char *postalcode;
	char *region;
	char *locality;
	char *street;
	char *extended;
	char *country;
}ctsvc_address_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int contact_id;
	int type;
	char *label;
	int date;
	bool is_lunar;
	int lunar_date;
}ctsvc_event_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int contact_id;
	int type;
	char *label;
	char *im_id;
}ctsvc_messenger_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int contact_id;
	int group_id;
	char *group_name;
}ctsvc_group_relation_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int contact_id;
	int type;
	char *label;
	char *nickname;
}ctsvc_nickname_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int contact_id;
	char *uid;
	char *text;
	int  order;
	char *service_operation;
	char *mime;
	char *app_id;
	char *uri;
	char *category;
	char *extra_data;
}ctsvc_profile_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int contact_id;
	int type;
	char *label;
	char *name;
}ctsvc_relationship_s;

typedef struct {
	ctsvc_record_s base;
	bool is_default;
	int id;
	int contact_id;
	int type;
	char *label;
	char *path;
}ctsvc_image_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int contact_id;
	char *note;
}ctsvc_note_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int contact_id;
	char *source_name;
	char *status;
	int timestamp;
	ctsvc_list_s* photos;
	char *service_operation;
	char *uri;
}ctsvc_activity_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int activity_id;
	char *photo_url;
	int	sort_index;
}ctsvc_activity_photo_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int contact_id;
	bool is_default;
	int type;
	char *label;
	char *name;
	char *department;
	char *job_title;
	char *role;
	char *assistant_name;
	char *logo;
	char *location;
	char *description;
	char *phonetic_name;
}ctsvc_company_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	char *address;
	int person_id; /* person id */
	int log_time;
	int log_type;
	int extra_data1; /* duration, message_id */
	char *extra_data2; /*short message*/
}ctsvc_phonelog_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int contact_id;
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
	char *data11;
	char *data12;
}ctsvc_extension_s;

typedef struct {
	ctsvc_record_s base;
	bool is_favorite;
	int id;
	int person_id;
	int changed_time;
	int changed_ver;		// internally used, check when updating contact
	int addressbook_id;
	int link_mode;
	bool has_phonenumber;
	bool has_email;
	char *display_name;
	char *reverse_display_name;
	int display_source_type;
	int display_name_language;
	int reverse_display_name_language;
	char *sort_name;
	char *reverse_sort_name;
	char *sortkey;
	char *reverse_sortkey;
	char *uid;
	char *image_thumbnail_path;
	char *ringtone_path;
	char *vibration;
	char *message_alert;
	ctsvc_list_s* name;
	ctsvc_list_s* note;
	ctsvc_list_s* company;
	ctsvc_list_s* numbers;
	ctsvc_list_s* emails;
	ctsvc_list_s* grouprelations;
	ctsvc_list_s* events;
	ctsvc_list_s* messengers;
	ctsvc_list_s* postal_addrs;
	ctsvc_list_s* urls;
	ctsvc_list_s* nicknames;
	ctsvc_list_s* profiles;
	ctsvc_list_s* relationships;
	ctsvc_list_s* images;
	ctsvc_list_s* extensions;
}ctsvc_contact_s;

typedef struct {
	ctsvc_record_s base;
	int id;
	int addressbook_id;
	int changed_time;
	char *display_name;
	char *reverse_display_name;
	char *uid;
	char *image_thumbnail_path;
	ctsvc_list_s* name;
	ctsvc_list_s* note;
	ctsvc_list_s* company;
	ctsvc_list_s* numbers;
	ctsvc_list_s* emails;
	ctsvc_list_s* events;
	ctsvc_list_s* messengers;
	ctsvc_list_s* postal_addrs;
	ctsvc_list_s* urls;
	ctsvc_list_s* nicknames;
	ctsvc_list_s* profiles;
	ctsvc_list_s* relationships;
	ctsvc_list_s* images;
	ctsvc_list_s* extensions;
}ctsvc_my_profile_s;


typedef struct {
	ctsvc_record_s base;
	int id;
	char *name;
	char *number;
}ctsvc_sdn_s;

typedef struct {
	ctsvc_record_s base;
	int changed_type;
	int id;
	int changed_ver;
	int addressbook_id;
	bool image_changed;
	int last_changed_type;	// it is used for _contacts_my_profile_updated_info
}ctsvc_updated_info_s;

typedef struct {
	ctsvc_record_s base;
	int number_id;
	int person_id;
	char *display_name;
	char *image_thumbnail_path;
	int number_type;
	char *label;
	char *number;
	int dial_number;
}ctsvc_speeddial_s;

typedef struct {
	int property_id;
	int type;
	union {
		bool b;
		int i;
		char *s;
		long long int l;
		double d;
	}value;
}ctsvc_result_value_s;

typedef struct {
	ctsvc_record_s base;
	GSList *values;
}ctsvc_result_s;


#endif /* __TIZEN_SOCIAL_CTSVC_STRUCT_H__ */

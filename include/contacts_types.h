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

#ifndef __TIZEN_SOCIAL_CONTACTS_TYPES_H__
#define __TIZEN_SOCIAL_CONTACTS_TYPES_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define _CONTACTS_BEGIN_VIEW() \
	typedef struct{ \
		const char* const _uri;
#define _CONTACTS_BEGIN_READ_ONLY_VIEW() _CONTACTS_BEGIN_VIEW()
#define _CONTACTS_PROPERTY_INT(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_STR(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_BOOL(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_DOUBLE(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_LLI(property_id_name)	unsigned int property_id_name;

#define _CONTACTS_PROPERTY_CHILD_SINGLE(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_CHILD_MULTIPLE(property_id_name)	unsigned int property_id_name;

#define _CONTACTS_PROPERTY_FILTER_INT(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_FILTER_STR(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_FILTER_BOOL(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_FILTER_DOUBLE(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_FILTER_LLI(property_id_name)	unsigned int property_id_name;

#define _CONTACTS_PROPERTY_PROJECTION_INT(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_PROJECTION_STR(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_PROJECTION_BOOL(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_PROJECTION_DOUBLE(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_PROPERTY_PROJECTION_LLI(property_id_name)	unsigned int property_id_name;
#define _CONTACTS_END_VIEW(name) \
	} name##_property_ids; \
	extern API const name##_property_ids name;
#define _CONTACTS_END_READ_ONLY_VIEW(name) _CONTACTS_END_VIEW(name)

#define _CONTACTS_HANDLE(A) typedef struct __##A{}* A;

_CONTACTS_HANDLE( contacts_record_h )
_CONTACTS_HANDLE( contacts_filter_h )
_CONTACTS_HANDLE( contacts_list_h )
_CONTACTS_HANDLE( contacts_query_h )

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_RECORD_MODULE
 * @{
 */

/**
 * The Number can be made with a set of values by specifying one or more values.
 * \n Example : CTS_NUM_TYPE_HOME|CTS_NUM_TYPE_VOICE
 * \n Exceptionally, CTS_NUM_TYPE_CUSTOM is exclusive.
 */
typedef enum {
	CONTACTS_NUMBER_TYPE_OTHER,			/**< . */
	CONTACTS_NUMBER_TYPE_CUSTOM = 1<<0,		/**< Custom number type */
	CONTACTS_NUMBER_TYPE_HOME = 1<<1,		/**< A telephone number associated with a residence */
	CONTACTS_NUMBER_TYPE_WORK = 1<<2,		/**< A telephone number associated with a place of work */
	CONTACTS_NUMBER_TYPE_VOICE = 1<<3,		/**< A voice telephone number */
	CONTACTS_NUMBER_TYPE_FAX = 1<<4,		/**< A facsimile telephone number */
	CONTACTS_NUMBER_TYPE_MSG = 1<<5,		/**< The telephone number has voice messaging support */
	CONTACTS_NUMBER_TYPE_CELL = 1<<6,		/**< A cellular telephone number */
	CONTACTS_NUMBER_TYPE_PAGER = 1<<7,		/**< A paging device telephone number */
	CONTACTS_NUMBER_TYPE_BBS = 1<<8,		/**< A bulletin board system telephone number */
	CONTACTS_NUMBER_TYPE_MODEM = 1<<9,		/**< A MODEM connected telephone number */
	CONTACTS_NUMBER_TYPE_CAR = 1<<10,		/**< A car-phone telephone number */
	CONTACTS_NUMBER_TYPE_ISDN = 1<<11,		/**< An ISDN service telephone number */
	CONTACTS_NUMBER_TYPE_VIDEO = 1<<12,		/**< A video conferencing telephone number */
	CONTACTS_NUMBER_TYPE_PCS = 1<<13,		/**< A personal communication services telephone number */

	CONTACTS_NUMBER_TYPE_ASSISTANT = 1<<30,		/**< A additional type for assistant */
}contacts_number_type_e;

typedef enum {
	CONTACTS_EMAIL_TYPE_OTHER,		/**< . */
	CONTACTS_EMAIL_TYPE_CUSTOM = 1<<0,	/**< . */
	CONTACTS_EMAIL_TYPE_HOME = 1<<1,	/**< . */
	CONTACTS_EMAIL_TYPE_WORK = 1<<2,	/**< . */
	CONTACTS_EMAIL_TYPE_MOBILE = 1<<3,	/**< . */
}contacts_email_type_e;

typedef enum {
	CONTACTS_COMPANY_TYPE_OTHER,		/**< . */
	CONTACTS_COMPANY_TYPE_CUSTOM = 1<<0,	/**< . */
	CONTACTS_COMPANY_TYPE_WORK = 1<<1,	/**< . */
}contacts_company_type_e;

typedef enum {
	CONTACTS_ADDRESS_TYPE_OTHER,		/**< . */
	CONTACTS_ADDRESS_TYPE_CUSTOM = 1<<0,	/**< a delivery address for a residence */
	CONTACTS_ADDRESS_TYPE_HOME = 1<<1,		/**< a delivery address for a residence */
	CONTACTS_ADDRESS_TYPE_WORK = 1<<2,		/**< a delivery address for a place of work */
	CONTACTS_ADDRESS_TYPE_DOM = 1<<3,		/**< a domestic delivery address */
	CONTACTS_ADDRESS_TYPE_INTL = 1<<4,		/**< an international delivery address */
	CONTACTS_ADDRESS_TYPE_POSTAL = 1<<5,		/**< a postal delivery address */
	CONTACTS_ADDRESS_TYPE_PARCEL = 1<<6,		/**< a parcel delivery address */
}contacts_address_type_e;

typedef enum {
	CONTACTS_URL_TYPE_OTHER,	/**< . */
	CONTACTS_URL_TYPE_CUSTOM,	/**< . */
	CONTACTS_URL_TYPE_HOME,		/**< . */
	CONTACTS_URL_TYPE_WORK,		/**< . */
}contacts_url_type_e;

typedef enum{
	CONTACTS_MESSENGER_TYPE_OTHER,		/**< . */
	CONTACTS_MESSENGER_TYPE_CUSTOM,		/**< . */
	CONTACTS_MESSENGER_TYPE_GOOGLE,		/**< . */
	CONTACTS_MESSENGER_TYPE_WLM,		/**< . */
	CONTACTS_MESSENGER_TYPE_YAHOO,		/**< . */
	CONTACTS_MESSENGER_TYPE_FACEBOOK,	/**< . */
	CONTACTS_MESSENGER_TYPE_ICQ,		/**< . */
	CONTACTS_MESSENGER_TYPE_AIM,		/**< . */
	CONTACTS_MESSENGER_TYPE_QQ,			/**< . */
	CONTACTS_MESSENGER_TYPE_JABBER,		/**< . */
	CONTACTS_MESSENGER_TYPE_SKYPE,		/**< . */
	CONTACTS_MESSENGER_TYPE_IRC,		/**< . */
}contacts_messenger_type_e;

typedef enum {
	CONTACTS_PLOG_TYPE_NONE,
	CONTACTS_PLOG_TYPE_VOICE_INCOMMING = 1,		/**< . */
	CONTACTS_PLOG_TYPE_VOICE_OUTGOING = 2,		/**< . */
	CONTACTS_PLOG_TYPE_VIDEO_INCOMMING = 3,		/**< . */
	CONTACTS_PLOG_TYPE_VIDEO_OUTGOING = 4,		/**< . */
	CONTACTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN = 5,	/**< Not confirmed missed call */
	CONTACTS_PLOG_TYPE_VOICE_INCOMMING_SEEN = 6,	/**< Confirmed missed call */
	CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN = 7,	/**< Not confirmed missed video call */
	CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_SEEN = 8,	/**< Confirmed missed video call */
	CONTACTS_PLOG_TYPE_VOICE_REJECT = 9,		/**< . */
	CONTACTS_PLOG_TYPE_VIDEO_REJECT = 10,		/**< . */
	CONTACTS_PLOG_TYPE_VOICE_BLOCKED = 11,		/**< . */
	CONTACTS_PLOG_TYPE_VIDEO_BLOCKED = 12,		/**< . */

	CONTACTS_PLOG_TYPE_MMS_INCOMMING = 101,		/**< . */
	CONTACTS_PLOG_TYPE_MMS_OUTGOING = 102,		/**< . */
	CONTACTS_PLOG_TYPE_SMS_INCOMMING = 103,		/**< . */
	CONTACTS_PLOG_TYPE_SMS_OUTGOING = 104,		/**< . */
	CONTACTS_PLOG_TYPE_SMS_BLOCKED = 105,		/**< . */
	CONTACTS_PLOG_TYPE_MMS_BLOCKED = 106,		/**< . */

	CONTACTS_PLOG_TYPE_EMAIL_RECEIVED = 201,	/**<.*/
	CONTACTS_PLOG_TYPE_EMAIL_SENT = 202,		/**<.*/

	CONTACTS_PLOG_TYPE_MAX
}contacts_phone_log_type_e;

typedef enum {
	CONTACTS_EVENT_TYPE_OTHER,			/**< . */
	CONTACTS_EVENT_TYPE_CUSTOM,			/**< . */
	CONTACTS_EVENT_TYPE_BIRTH,			/**< . */
	CONTACTS_EVENT_TYPE_ANNIVERSARY		/**< . */
}contacts_event_type_e;

typedef enum {
	CONTACTS_USAGE_STAT_TYPE_NONE,			/**< . */
	CONTACTS_USAGE_STAT_TYPE_OUTGOING_CALL,	/**< . */
	CONTACTS_USAGE_STAT_TYPE_OUTGOING_MSG	/**< . */
}contacts_usage_type_e;

typedef enum {
	CONTACTS_DISPLAY_NAME_SOURCE_TYPE_INVALID,		/**< . */
	CONTACTS_DISPLAY_NAME_SOURCE_TYPE_EMAIL,		/**< . */
	CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NUMBER,		/**< . */
	CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NICKNAME,		/**< . */
	CONTACTS_DISPLAY_NAME_SOURCE_TYPE_COMPANY,		/**< . */
	CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NAME,			/**< . */
}contacts_display_name_source_type_e;

typedef enum {
	CONTACTS_ADDRESS_BOOK_MODE_NONE, /**< All module can read and write contacts of this address_book */
	CONTACTS_ADDRESS_BOOK_MODE_READONLY, /**< All module can only read contacts of this address_book*/
}contacts_address_book_mode_e;

typedef enum{
	CONTACTS_CONTACT_LINK_MODE_NONE,				/**< Auto link immediatly */
	CONTACTS_CONTACT_LINK_MODE_IGNORE_ONCE,		/**< Do not auto link when the contact is inserted */
}contacts_contact_link_mode_e;

typedef enum {
	CONTACTS_RELATIONSHIP_TYPE_OTHER, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_ASSISTANT, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_BROTHER, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_CHILD, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_DOMESTIC_PARTNER, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_FATHER, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_FRIEND, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_MANAGER, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_MOTHER, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_PARENT, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_PARTNER, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_REFERRED_BY, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_RELATIVE, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_SISTER, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_SPOUSE, /**< .*/
	CONTACTS_RELATIONSHIP_TYPE_CUSTOM, /**< .*/
}contacts_relationship_type_e;

typedef enum {
	CONTACTS_SEARCH_RANGE_NAME = 0x00000001,	 /**< .*/
	CONTACTS_SEARCH_RANGE_NUMBER  = 0x00000002,	 /**< .*/
	CONTACTS_SEARCH_RANGE_DATA  = 0x00000004,	 /**< .*/
}contacts_search_range_e;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif	/* __TIZEN_SOCIAL_CONTACTS_TYPES_H__ */


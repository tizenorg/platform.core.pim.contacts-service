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
#define _CONTACTS_PROPERTY_INT(property_id_name)    unsigned int property_id_name;
#define _CONTACTS_PROPERTY_STR(property_id_name)    unsigned int property_id_name;
#define _CONTACTS_PROPERTY_BOOL(property_id_name)   unsigned int property_id_name;
#define _CONTACTS_PROPERTY_DOUBLE(property_id_name) unsigned int property_id_name;
#define _CONTACTS_PROPERTY_LLI(property_id_name)    unsigned int property_id_name;

#define _CONTACTS_PROPERTY_CHILD_SINGLE(property_id_name)   unsigned int property_id_name;
#define _CONTACTS_PROPERTY_CHILD_MULTIPLE(property_id_name) unsigned int property_id_name;

#define _CONTACTS_PROPERTY_FILTER_INT(property_id_name)     unsigned int property_id_name;
#define _CONTACTS_PROPERTY_FILTER_STR(property_id_name)     unsigned int property_id_name;
#define _CONTACTS_PROPERTY_FILTER_BOOL(property_id_name)    unsigned int property_id_name;
#define _CONTACTS_PROPERTY_FILTER_DOUBLE(property_id_name)  unsigned int property_id_name;
#define _CONTACTS_PROPERTY_FILTER_LLI(property_id_name)     unsigned int property_id_name;

#define _CONTACTS_PROPERTY_PROJECTION_INT(property_id_name)     unsigned int property_id_name;
#define _CONTACTS_PROPERTY_PROJECTION_STR(property_id_name)     unsigned int property_id_name;
#define _CONTACTS_PROPERTY_PROJECTION_BOOL(property_id_name)    unsigned int property_id_name;
#define _CONTACTS_PROPERTY_PROJECTION_DOUBLE(property_id_name)  unsigned int property_id_name;
#define _CONTACTS_PROPERTY_PROJECTION_LLI(property_id_name)     unsigned int property_id_name;
#define _CONTACTS_END_VIEW(name) \
    } name##_property_ids; \
    extern API const name##_property_ids name;
#define _CONTACTS_END_READ_ONLY_VIEW(name) _CONTACTS_END_VIEW(name)

#define _CONTACTS_HANDLE(A) typedef struct __##A{}* A;

_CONTACTS_HANDLE(contacts_record_h)
_CONTACTS_HANDLE(contacts_filter_h)
_CONTACTS_HANDLE(contacts_list_h)
_CONTACTS_HANDLE(contacts_query_h)
_CONTACTS_HANDLE(contacts_h)

/**
 * @file contacts_types.h
 */

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_RECORD_MODULE
 * @{
 */

/**
 * @brief Enumeration for contacts number type.
 *
 * @details The number can be made with a set of values by specifying one or more values.
 *          \n Example : CTS_NUM_TYPE_HOME|CTS_NUM_TYPE_VOICE
 *          \n Exceptionally, CTS_NUM_TYPE_CUSTOM is exclusive.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_NUMBER_TYPE_OTHER,                 /**< Other number type */
    CONTACTS_NUMBER_TYPE_CUSTOM = 1<<0,         /**< Custom number type */
    CONTACTS_NUMBER_TYPE_HOME = 1<<1,           /**< A telephone number associated with a residence */
    CONTACTS_NUMBER_TYPE_WORK = 1<<2,           /**< A telephone number associated with a place of work */
    CONTACTS_NUMBER_TYPE_VOICE = 1<<3,          /**< A voice telephone number */
    CONTACTS_NUMBER_TYPE_FAX = 1<<4,            /**< A facsimile telephone number */
    CONTACTS_NUMBER_TYPE_MSG = 1<<5,            /**< The telephone number has voice messaging support */
    CONTACTS_NUMBER_TYPE_CELL = 1<<6,           /**< A cellular telephone number */
    CONTACTS_NUMBER_TYPE_PAGER = 1<<7,          /**< A paging device telephone number */
    CONTACTS_NUMBER_TYPE_BBS = 1<<8,            /**< A bulletin board system telephone number */
    CONTACTS_NUMBER_TYPE_MODEM = 1<<9,          /**< A MODEM connected telephone number */
    CONTACTS_NUMBER_TYPE_CAR = 1<<10,           /**< A car-phone telephone number */
    CONTACTS_NUMBER_TYPE_ISDN = 1<<11,          /**< An ISDN service telephone number */
    CONTACTS_NUMBER_TYPE_VIDEO = 1<<12,         /**< A video conferencing telephone number */
    CONTACTS_NUMBER_TYPE_PCS = 1<<13,           /**< A personal communication services telephone number */
    CONTACTS_NUMBER_TYPE_COMPANY_MAIN = 1<<14,  /**< A company main number */
    CONTACTS_NUMBER_TYPE_RADIO = 1<<15,         /**< A radio phone number */
    CONTACTS_NUMBER_TYPE_MAIN = 1<<29,          /**< An additional type for main */
    CONTACTS_NUMBER_TYPE_ASSISTANT = 1<<30,     /**< An additional type for assistant */
}contacts_number_type_e;

/**
 * @brief Enumeration for Contact email type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_EMAIL_TYPE_OTHER,          /**< Other email type */
    CONTACTS_EMAIL_TYPE_CUSTOM = 1<<0,  /**< Custom email type */
    CONTACTS_EMAIL_TYPE_HOME = 1<<1,    /**< An email address associated with a residence */
    CONTACTS_EMAIL_TYPE_WORK = 1<<2,    /**< An email address associated with a place of work */
    CONTACTS_EMAIL_TYPE_MOBILE = 1<<3,  /**< A mobile email address */
}contacts_email_type_e;

/**
 * @brief Enumeration for Contact company type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_COMPANY_TYPE_OTHER,            /**< Other company type */
    CONTACTS_COMPANY_TYPE_CUSTOM = 1<<0,    /**< Custom company type */
    CONTACTS_COMPANY_TYPE_WORK = 1<<1,      /**< Work company type */
}contacts_company_type_e;

/**
 * @brief Enumeration for Contact address type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_ADDRESS_TYPE_OTHER,            /**< Other address type */
    CONTACTS_ADDRESS_TYPE_CUSTOM = 1<<0,    /**< A delivery address for a residence */
    CONTACTS_ADDRESS_TYPE_HOME = 1<<1,      /**< A delivery address for a residence */
    CONTACTS_ADDRESS_TYPE_WORK = 1<<2,      /**< A delivery address for a place of work */
    CONTACTS_ADDRESS_TYPE_DOM = 1<<3,       /**< A domestic delivery address */
    CONTACTS_ADDRESS_TYPE_INTL = 1<<4,      /**< An international delivery address */
    CONTACTS_ADDRESS_TYPE_POSTAL = 1<<5,    /**< A postal delivery address */
    CONTACTS_ADDRESS_TYPE_PARCEL = 1<<6,    /**< A parcel delivery address */
}contacts_address_type_e;

/**
 * @brief Enumeration for Contact URL type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_URL_TYPE_OTHER,    /**< Other URL type*/
    CONTACTS_URL_TYPE_CUSTOM,   /**< Custom URL type */
    CONTACTS_URL_TYPE_HOME,     /**< Home URL type */
    CONTACTS_URL_TYPE_WORK,     /**< Work URL type */
}contacts_url_type_e;

/**
 * @brief Enumeration for Contact messenger type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum{
    CONTACTS_MESSENGER_TYPE_OTHER,      /**< Other messenger type */
    CONTACTS_MESSENGER_TYPE_CUSTOM,     /**< Custom messenger type */
    CONTACTS_MESSENGER_TYPE_GOOGLE,     /**< Google messenger type */
    CONTACTS_MESSENGER_TYPE_WLM,        /**< Windows live messenger type */
    CONTACTS_MESSENGER_TYPE_YAHOO,      /**< Yahoo messenger type */
    CONTACTS_MESSENGER_TYPE_FACEBOOK,   /**< Facebook messenger type */
    CONTACTS_MESSENGER_TYPE_ICQ,        /**< ICQ type */
    CONTACTS_MESSENGER_TYPE_AIM,        /**< AOL instance messenger type */
    CONTACTS_MESSENGER_TYPE_QQ,         /**< QQ type */
    CONTACTS_MESSENGER_TYPE_JABBER,     /**< Jabber type */
    CONTACTS_MESSENGER_TYPE_SKYPE,      /**< Skype type */
    CONTACTS_MESSENGER_TYPE_IRC,        /**< IRC type */
}contacts_messenger_type_e;

/**
 * @brief Enumeration for Call history type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_PLOG_TYPE_NONE,                        /**< None */
    CONTACTS_PLOG_TYPE_VOICE_INCOMMING = 1,         /**< Incoming call */
    CONTACTS_PLOG_TYPE_VOICE_OUTGOING = 2,          /**< Outgoing call */
    CONTACTS_PLOG_TYPE_VIDEO_INCOMMING = 3,         /**< Incoming video call */
    CONTACTS_PLOG_TYPE_VIDEO_OUTGOING = 4,          /**< Outgoing video call */
    CONTACTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN = 5,  /**< Not confirmed missed call */
    CONTACTS_PLOG_TYPE_VOICE_INCOMMING_SEEN = 6,    /**< Confirmed missed call */
    CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN = 7,  /**< Not confirmed missed video call */
    CONTACTS_PLOG_TYPE_VIDEO_INCOMMING_SEEN = 8,    /**< Confirmed missed video call */
    CONTACTS_PLOG_TYPE_VOICE_REJECT = 9,            /**< Rejected call */
    CONTACTS_PLOG_TYPE_VIDEO_REJECT = 10,           /**< Rejected video call */
    CONTACTS_PLOG_TYPE_VOICE_BLOCKED = 11,          /**< Blocked call */
    CONTACTS_PLOG_TYPE_VIDEO_BLOCKED = 12,          /**< Blocked video call */

    CONTACTS_PLOG_TYPE_MMS_INCOMMING = 101,         /**< Incoming MMS */
    CONTACTS_PLOG_TYPE_MMS_OUTGOING = 102,          /**< Outgoing MMS */
    CONTACTS_PLOG_TYPE_SMS_INCOMMING = 103,         /**< Incoming SMS */
    CONTACTS_PLOG_TYPE_SMS_OUTGOING = 104,          /**< Outgoing SMS */
    CONTACTS_PLOG_TYPE_SMS_BLOCKED = 105,           /**< Blocked SMS */
    CONTACTS_PLOG_TYPE_MMS_BLOCKED = 106,           /**< Blocked MMS */

    CONTACTS_PLOG_TYPE_EMAIL_RECEIVED = 201,        /**< Received email */
    CONTACTS_PLOG_TYPE_EMAIL_SENT = 202,            /**< Sent email */

}contacts_phone_log_type_e;

/**
 * @brief Enumeration for Contact event type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_EVENT_TYPE_OTHER,          /**< Other event type */
    CONTACTS_EVENT_TYPE_CUSTOM,         /**< Custom event type */
    CONTACTS_EVENT_TYPE_BIRTH,          /**< Birthday event type */
    CONTACTS_EVENT_TYPE_ANNIVERSARY     /**< Anniversary event type */
}contacts_event_type_e;

/**
 * @brief Enumeration for Contact event calendar type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
	CONTACTS_EVENT_CALENDAR_TYPE_GREGORIAN, 	/**< Gregorian calendar */
	CONTACTS_EVENT_CALENDAR_TYPE_CHINESE		/**< Chinese calenadr */
}contacts_event_calendar_type_e;

/**
 * @brief Enumeration of Contact image type
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_IMAGE_TYPE_OTHER,       /**< Other type */
    CONTACTS_IMAGE_TYPE_CUSTOM,      /**< Custom type */
}contacts_image_type_e;

/**
 * @brief Enumeration for usage type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_USAGE_STAT_TYPE_NONE,          /**< None */
    CONTACTS_USAGE_STAT_TYPE_OUTGOING_CALL, /**< Outgoing call */
    CONTACTS_USAGE_STAT_TYPE_OUTGOING_MSG   /**< Outgoing message */
}contacts_usage_type_e;

/**
 * @brief Enumeration for Contact display name source type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_DISPLAY_NAME_SOURCE_TYPE_INVALID,      /**< Invalid source of display name */
    CONTACTS_DISPLAY_NAME_SOURCE_TYPE_EMAIL,        /**< Produced display name from email record */
    CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NUMBER,       /**< Produced display name from number record */
    CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NICKNAME,     /**< Produced display name from nickname record */
    CONTACTS_DISPLAY_NAME_SOURCE_TYPE_COMPANY,      /**< Produced display name from company record */
    CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NAME,         /**< Produced display name from name record */
}contacts_display_name_source_type_e;

/**
 * @brief Enumeration for Address book mode.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_ADDRESS_BOOK_MODE_NONE,        /**< All module can read and write contacts of this address_book */
    CONTACTS_ADDRESS_BOOK_MODE_READONLY,    /**< All module can only read contacts of this address_book*/
}contacts_address_book_mode_e;

/**
 * @brief Enumeration for link mode when inserting contact.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum{
    CONTACTS_CONTACT_LINK_MODE_NONE,            /**< Auto link immediately */
    CONTACTS_CONTACT_LINK_MODE_IGNORE_ONCE,     /**< Do not auto link when the contact is inserted */
}contacts_contact_link_mode_e;

/**
 * @brief Enumeration for Contact relationship type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_RELATIONSHIP_TYPE_OTHER,               /**< Other relationship type*/
    CONTACTS_RELATIONSHIP_TYPE_ASSISTANT,           /**< Assistant type */
    CONTACTS_RELATIONSHIP_TYPE_BROTHER,             /**< Brother type */
    CONTACTS_RELATIONSHIP_TYPE_CHILD,               /**< Child type */
    CONTACTS_RELATIONSHIP_TYPE_DOMESTIC_PARTNER,    /**< Domestic Partner type */
    CONTACTS_RELATIONSHIP_TYPE_FATHER,              /**< Father type */
    CONTACTS_RELATIONSHIP_TYPE_FRIEND,              /**< Friend type */
    CONTACTS_RELATIONSHIP_TYPE_MANAGER,             /**< Manager type */
    CONTACTS_RELATIONSHIP_TYPE_MOTHER,              /**< Mother type */
    CONTACTS_RELATIONSHIP_TYPE_PARENT,              /**< Parent type */
    CONTACTS_RELATIONSHIP_TYPE_PARTNER,             /**< Partner type */
    CONTACTS_RELATIONSHIP_TYPE_REFERRED_BY,         /**< Referred by type */
    CONTACTS_RELATIONSHIP_TYPE_RELATIVE,            /**< Relative type */
    CONTACTS_RELATIONSHIP_TYPE_SISTER,              /**< Sister type */
    CONTACTS_RELATIONSHIP_TYPE_SPOUSE,              /**< Spouse type */
    CONTACTS_RELATIONSHIP_TYPE_CUSTOM,              /**< Custom type */
}contacts_relationship_type_e;

/**
 * @brief Enumeration for Contact search range.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.4 @endif
 *
 */
typedef enum {
    CONTACTS_SEARCH_RANGE_NAME = 0x00000001,     /**< Search record from name */
    CONTACTS_SEARCH_RANGE_NUMBER  = 0x00000002,  /**< Search record from name and number */
    CONTACTS_SEARCH_RANGE_DATA  = 0x00000004,    /**< Search record from name,number and data */
    CONTACTS_SEARCH_RANGE_EMAIL  = 0x00000008,   /**< Search record from name,number,data and email. Now, support only _contacts_person_email view_uri*/
}contacts_search_range_e;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif	/* __TIZEN_SOCIAL_CONTACTS_TYPES_H__ */


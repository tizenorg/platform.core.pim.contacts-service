/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __TIZEN_SOCIAL_CONTACTS_VIEWS_H__
#define __TIZEN_SOCIAL_CONTACTS_VIEWS_H__

#include "contacts_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file contacts_views.h
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 *
 * @brief This page provides information about views with properties.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_OVERVIEW Overview
 * In this category, application developers can find tables with view properties.
 *
 * A view is a structure which describes properties of a record.
 *
 * A record can have basic properties of five types: integer, string, boolean, long integer, double. Each property
 * of basic type has functions to operate on it:
 *
 * <table>
 * <tr>
 *    <th>Property type</th>
 *    <th>Setter</th>
 *    <th>Getter</th>
 * </tr>
 * <tr>
 *    <td> string </td>
 *    <td> @ref contacts_record_set_str </td>
 *    <td> @ref contacts_record_get_str </td>
 * </tr>
 * <tr>
 *    <td> integer </td>
 *    <td> @ref contacts_record_set_int </td>
 *    <td> @ref contacts_record_get_int </td>
 * </tr>
 * <tr>
 *    <td> boolean </td>
 *    <td> @ref contacts_record_set_bool </td>
 *    <td> @ref contacts_record_get_bool </td>
 * </tr>
 * <tr>
 *    <td> long integer </td>
 *    <td> @ref contacts_record_set_lli </td>
 *    <td> @ref contacts_record_get_lli </td>
 * </tr>
 * <tr>
 *    <td> double </td>
 *    <td> @ref contacts_record_set_double </td>
 *    <td> @ref contacts_record_get_double </td>
 * </tr>
 * </table>
 *
 * For long integer functions, "lli" stands for long long int, usually used to hold UTC time.
 *
 * Record types which have *_id as their properties, hold identifiers of other records - for example, name, number and email
 * views hold ID of their corresponding contacts in contact_id property
 * (as children of the corresponding contacts record).
 *
 * Properties of type 'record' are other records. For example, the _contacts_contact view
 * has a 'name' property of type 'record'. This means that records of type name (_contacts_name view)
 * can be children of the contact record. If a name record holds the identifier
 * of a contact record in its 'contact_id' property, it is the child record of the corresponding
 * contact record.
 *
 * Records can have many children of a given type.
 *
 * For a more detailed explanation and examples, see the main section of Contacts API.
 *
 * <BR>
 */


/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book _contacts_address_book view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts addressbook view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the addressbook </td></tr>
 * <tr><td>integer</td><td> account_id </td><td>read, write once</td><td> Account ID that the addressbook belongs to </td></tr>
 * <tr><td>string</td><td> name </td><td>read, write</td><td> It cannot be @c NULL. Duplicate names are not allowed. </td></tr>
 * <tr><td>integer</td><td> mode </td><td>read, write</td><td> Addressbook mode, refer to the @ref contacts_address_book_mode_e </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_INT(account_id)            /* read, write once */
	_CONTACTS_PROPERTY_STR(name)                  /* read, write */
	_CONTACTS_PROPERTY_INT(mode)                  /* read, write */
_CONTACTS_END_VIEW(_contacts_address_book)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group _contacts_group view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts group view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the group </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td>read, write once</td><td> Addressbook ID that the group belongs to </td></tr>
 * <tr><td>string</td><td> name </td><td>read, write</td><td> Group name </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td>read, write</td><td> Ringtone path of the group </td></tr>
 * <tr><td>string</td><td> image_path </td><td>read, write</td><td> Image path of the group </td></tr>
 * <tr><td>string</td><td> vibration </td><td>read, write</td><td> Vibration path of the group </td></tr>
 * <tr><td>string</td><td> extra_data </td><td>read, write</td><td> Extra data for default group name </td></tr>
 * <tr><td>boolean</td><td> is_read_only </td><td>read, write once</td><td> The group is read only or not </td></tr>
 * <tr><td>string</td><td> message_alert </td><td>read, write</td><td> Message alert path of the group </td></tr>
 * </table>
 *
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_INT(address_book_id)       /* read, write once */
	_CONTACTS_PROPERTY_STR(name)                  /* read, write */
	_CONTACTS_PROPERTY_STR(ringtone_path)         /* read, write */
	_CONTACTS_PROPERTY_STR(image_path)            /* read, write */
	_CONTACTS_PROPERTY_STR(vibration)             /* read, write */
	_CONTACTS_PROPERTY_STR(extra_data)            /* read, write, string */
	_CONTACTS_PROPERTY_BOOL(is_read_only)         /* read, write once */
	_CONTACTS_PROPERTY_STR(message_alert)         /* read, write */
_CONTACTS_END_VIEW(_contacts_group)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person _contacts_person view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts person view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the person </td></tr>
 * <tr><td>string</td><td> display_name </td><td>read only</td><td> Display name of the person </td></tr>
 * <tr><td>string</td><td> display_name_index </td><td>read only</td><td> The first character of first string for grouping. This is normalized using icu (projection) </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td>read only</td><td> Display contact ID that the person belongs to </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td>read, write</td><td> Ringtone path of the person </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td>read only</td><td> Image thumbnail path of the person </td></tr>
 * <tr><td>string</td><td> vibration </td><td>read, write</td><td> Vibration path of the person </td></tr>
 * <tr><td>string</td><td> message_alert </td><td>read, write</td><td> Message alert path of the person </td></tr>
 * <tr><td>string</td><td> status </td><td>read only</td><td> Status of social account </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td>read, write</td><td> The person is favorite or not </td></tr>
 * <tr><td>double</td><td> favorite_priority </td><td> sort only </td><td> The priority of favorite contacts. You cannot get/set the value but you can use it as sorting key, see the @ref contacts_query_set_sort </td></tr>
 * <tr><td>integer</td><td> link_count </td><td>read only</td><td> Link count of contact records (projection) </td></tr>
 * <tr><td>string</td><td> addressbook_ids </td><td>read only</td><td> Addressbook IDs that the person belongs to (projection) </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td>read only</td><td> The person has phone number or not </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td>read only</td><td> The person has email or not </td></tr>
 * <tr><td>integer</td><td> snippet_type </td><td>read only</td><td> kerword matched data type, refer to they @ref contacts_data_type_e (Since 3.0) </td></tr>
 * <tr><td>string</td><td> snippet_string </td><td>read only</td><td> keyword matched data string (Since 3.0) </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_STR(display_name)          /* read only */
	_CONTACTS_PROPERTY_STR(display_name_index)    /* read only */
	_CONTACTS_PROPERTY_INT(display_contact_id)    /* read, write */
	_CONTACTS_PROPERTY_STR(ringtone_path)         /* read, write */
	_CONTACTS_PROPERTY_STR(image_thumbnail_path)  /* read only */
	_CONTACTS_PROPERTY_STR(vibration)             /* read, write */
	_CONTACTS_PROPERTY_STR(status)                /* read only */
	_CONTACTS_PROPERTY_BOOL(is_favorite)          /* read, write */
	_CONTACTS_PROPERTY_DOUBLE(favorite_priority)  /* sort only */
	_CONTACTS_PROPERTY_INT(link_count)            /* read only */
	_CONTACTS_PROPERTY_STR(addressbook_ids)       /* read only */
	_CONTACTS_PROPERTY_BOOL(has_phonenumber)      /* read only */
	_CONTACTS_PROPERTY_BOOL(has_email)            /* read only */
	_CONTACTS_PROPERTY_STR(message_alert)         /* read, write */
	_CONTACTS_PROPERTY_INT(snippet_type)			/* read only (Since 3.0) */
	_CONTACTS_PROPERTY_STR(snippet_string)			/* read only (Since 3.0) */
_CONTACTS_END_VIEW(_contacts_person)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_simple_contact _contacts_simple_contact view
 * You can only get simple contact using this view.
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this simple contact view </td></tr>
 * <tr><td>integer</td><td>id</td><td> DB record ID of the contact </td></tr>
 * <tr><td>string</td><td>display_name</td><td> Display name of the contact </td></tr>
 * <tr><td>integer</td><td>display_source_id</td><td> The source type of display name, refer to the @ref contacts_display_name_source_type_e </td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td> Addressbook that the contact belongs to </td></tr>
 * <tr><td>string</td><td>ringtone_path</td><td> Ringtone path of the contact </td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td> Image thumbnail path of the contact </td></tr>
 * <tr><td>boolean</td><td>is_favorite</td><td> The contact is favorite or not </td></tr>
 * <tr><td>boolean</td><td>has_phonenumber</td><td> The contact has phone number or not </td></tr>
 * <tr><td>boolean</td><td>has_email</td><td> The contact has email or not </td></tr>
 * <tr><td>integer</td><td>person_id</td><td> Person ID that the contact belongs to </td></tr>
 * <tr><td>string</td><td>uid</td><td> Unique identifier </td></tr>
 * <tr><td>string</td><td>vibration</td><td> Vibration path of the contact </td></tr>
 * <tr><td>string</td><td>message_alert</td><td> Message alert path of the contact </td></tr>
 * <tr><td>integer</td><td>changed_time</td><td> Last changed contact time </td></tr>
 * </table>
 *
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_STR(display_name)          /* read only */
	_CONTACTS_PROPERTY_INT(display_source_type)   /* read only */
	_CONTACTS_PROPERTY_INT(address_book_id)       /* read only */
	_CONTACTS_PROPERTY_STR(ringtone_path)         /* read only */
	_CONTACTS_PROPERTY_STR(image_thumbnail_path)  /* read only */
	_CONTACTS_PROPERTY_BOOL(is_favorite)          /* read only */
	_CONTACTS_PROPERTY_BOOL(has_phonenumber)      /* read only */
	_CONTACTS_PROPERTY_BOOL(has_email)            /* read only */
	_CONTACTS_PROPERTY_INT(person_id)             /* read only */
	_CONTACTS_PROPERTY_STR(uid)                   /* read only */
	_CONTACTS_PROPERTY_STR(vibration)             /* read only */
	_CONTACTS_PROPERTY_INT(changed_time)          /* read only */
	_CONTACTS_PROPERTY_STR(message_alert)         /* read only */
_CONTACTS_END_READ_ONLY_VIEW(_contacts_simple_contact)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact _contacts_contact view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contact view  </td></tr>
 * <tr><td>integer</td><td>id</td><td>read only</td><td> DB record ID of the contact </td></tr>
 * <tr><td>string</td><td>display_name</td><td>read only</td><td> Display name of the contact </td></tr>
 * <tr><td>integer</td><td>display_source_id</td><td>read only</td><td> The source type of display name, refer to the @ref contacts_display_name_source_type_e </td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td>read, write once</td><td> Addressbook ID that the contact belongs to </td></tr>
 * <tr><td>string</td><td>ringtone_path</td><td>read, write</td><td> Ringtone path of the contact </td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td>read only</td><td> Image thumbnail path of the contact </td></tr>
 * <tr><td>boolean</td><td>is_favorite</td><td>read, write</td><td> The contact is favorite or not </td></tr>
 * <tr><td>boolean</td><td>has_phonenumber</td><td>read only</td><td> The contact has phone number or not </td></tr>
 * <tr><td>boolean</td><td>has_email</td><td>read only</td><td> The contact has email or not </td></tr>
 * <tr><td>integer</td><td>person_id</td><td>read, write once</td><td> Person ID that the contact belongs to. If set when inserting, a contact will be linked to person </td></tr>
 * <tr><td>string</td><td>uid</td><td>read, write</td><td> Unique identifier </td></tr>
 * <tr><td>string</td><td>vibration</td><td>read, write</td><td> Vibration path of the contact </td></tr>
 * <tr><td>string</td><td>message_alert</td><td>read, write</td><td> Message alert path of the contact </td></tr>
 * <tr><td>integer</td><td>changed_time</td><td>read only</td><td> Last changed contact time </td></tr>
 * <tr><td>integer</td><td>link_mode</td><td>read, write once</td><td> The link mode, refer to the @ref contacts_contact_link_mode_e. If the person_id was set, this value will be ignored </td></tr>
 * <tr><td>record</td><td>name</td><td>read, write</td><td> _contacts_name child record (single) </td></tr>
 * <tr><td>record</td><td>company</td><td>read, write</td><td> _contacts_company child record (multiple) </td></tr>
 * <tr><td>record</td><td>note</td><td>read, write</td><td> _contacts_note child record (multiple) </td></tr>
 * <tr><td>record</td><td>number</td><td>read, write</td><td> _contacts_number child record (multiple) </td></tr>
 * <tr><td>record</td><td>email</td><td>read, write</td><td> _contacts_email child record (multiple) </td></tr>
 * <tr><td>record</td><td>event</td><td>read, write</td><td> _contacts_event child record (multiple) </td></tr>
 * <tr><td>record</td><td>messenger</td><td>read, write</td><td> _contacts_messenger child record (multiple) </td></tr>
 * <tr><td>record</td><td>address</td><td>read, write</td><td> _contacts_address child record (multiple) </td></tr>
 * <tr><td>record</td><td>url</td><td>read, write</td><td> _contacts_url child record (multiple) </td></tr>
 * <tr><td>record</td><td>nickname</td><td>read, write</td><td> _contacts_nickname child record (multiple) </td></tr>
 * <tr><td>record</td><td>profile</td><td>read, write</td><td>	 _contacts_profile child record (multiple) </td></tr>
 * <tr><td>record</td><td>relationship</td><td>read, write</td><td> _contacts_relationship child record (multiple)</td></tr>
 * <tr><td>record</td><td>image</td><td>read, write</td><td> _contacts_image child record (multiple)</td></tr>
 * <tr><td>record</td><td>group_relation</td><td>read, write</td><td> _contacts_group_relation child record (multiple)</td></tr>
 * <tr><td>record</td><td>sip</td><td>read, write</td><td> _contacts_sip child record (multiple) (Since 3.0)</td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                        /* read only */
	_CONTACTS_PROPERTY_STR(display_name)              /* read only */
	_CONTACTS_PROPERTY_INT(display_source_type)       /* read only */
	_CONTACTS_PROPERTY_INT(address_book_id)           /* read, write once */
	_CONTACTS_PROPERTY_STR(ringtone_path)             /* read, write */
	_CONTACTS_PROPERTY_STR(image_thumbnail_path)      /* read only */
	_CONTACTS_PROPERTY_BOOL(is_favorite)              /* read, write */
	_CONTACTS_PROPERTY_BOOL(has_phonenumber)          /* read only */
	_CONTACTS_PROPERTY_BOOL(has_email)                /* read only */
	_CONTACTS_PROPERTY_INT(person_id)                 /* read, write once */
	_CONTACTS_PROPERTY_STR(uid)                       /* read, write */
	_CONTACTS_PROPERTY_STR(vibration)                 /* read, write */
	_CONTACTS_PROPERTY_INT(changed_time)              /* read only */
	_CONTACTS_PROPERTY_INT(link_mode)                 /* read, write */
	_CONTACTS_PROPERTY_CHILD_SINGLE(name)             /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(image)          /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(company)        /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(note)           /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(number)         /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(email)          /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(event)          /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(messenger)      /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(address)        /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(url)            /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(nickname)       /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(profile)        /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(relationship)   /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(group_relation) /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(extension)      /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(sip)            /* read, write (Since 3.0) */
	_CONTACTS_PROPERTY_STR(message_alert)             /* read, write */
_CONTACTS_END_VIEW(_contacts_contact)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile _contacts_my_profile view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this my profile view </td></tr>
 * <tr><td>integer</td><td>id</td><td>read only</td><td> DB record ID of the my profile </td></tr>
 * <tr><td>string</td><td>display_name</td><td>read only</td><td> Display name of the profile </td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td>read, write once</td><td> Addressbook ID that the profile belongs to </td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td>read only</td><td> Image thumbnail path of the profile </td></tr>
 * <tr><td>string</td><td>uid</td><td>read, write</td><td> Unique identifier </td></tr>
 * <tr><td>integer</td><td>changed_time</td><td>read only</td><td> Last changed profile time </td></tr>
 * <tr><td>record</td><td>name</td><td>read, write</td><td> _contacts_name child record (single) </td></tr>
 * <tr><td>record</td><td>company</td><td>read, write</td><td> _contacts_company child record (multiple) </td></tr>
 * <tr><td>record</td><td>note</td><td>read, write</td><td> _contacts_note child record (multiple) </td></tr>
 * <tr><td>record</td><td>number</td><td>read, write</td><td> _contacts_number child record (multiple) </td></tr>
 * <tr><td>record</td><td>email</td><td>read, write</td><td> _contacts_email child record (multiple) </td></tr>
 * <tr><td>record</td><td>event</td><td>read, write</td><td> _contacts_event child record (multiple) </td></tr>
 * <tr><td>record</td><td>messenger</td><td>read, write</td><td> _contacts_messenger child record (multiple) </td></tr>
 * <tr><td>record</td><td>address</td><td>read, write</td><td> _contacts_address child record (multiple) </td></tr>
 * <tr><td>record</td><td>url</td><td>read, write</td><td> _contacts_url child record (multiple) </td></tr>
 * <tr><td>record</td><td>nickname</td><td>read, write</td><td> _contacts_nickname child record (multiple) </td></tr>
 * <tr><td>record</td><td>profile</td><td>read, write</td><td> _contacts_profile child record (multiple) </td></tr>
 * <tr><td>record</td><td>relationship</td><td>read, write</td><td> _contacts_relationship child record (multiple) </td></tr>
 * <tr><td>record</td><td>image</td><td>read, write</td><td> _contacts_image child record (multiple) </td></tr>
 * <tr><td>record</td><td>sip</td><td>read, write</td><td> _contacts_sip child record (multiple) (Since 3.0) </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                        /* read only */
	_CONTACTS_PROPERTY_STR(display_name)              /* read only */
	_CONTACTS_PROPERTY_INT(address_book_id)           /* read, write once */
	_CONTACTS_PROPERTY_STR(image_thumbnail_path)      /* read only */
	_CONTACTS_PROPERTY_STR(uid)                       /* read, write */
	_CONTACTS_PROPERTY_INT(changed_time)              /* read only */
	_CONTACTS_PROPERTY_CHILD_SINGLE(name)             /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(image)          /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(company)        /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(note)           /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(number)         /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(email)          /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(event)          /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(messenger)      /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(address)        /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(url)            /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(nickname)       /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(profile)        /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(relationship)   /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(extension)      /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(sip)            /* read, write (Since 3.0) */
_CONTACTS_END_VIEW(_contacts_my_profile)


/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name _contacts_name view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 *</tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts name view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the name </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contacts ID that the name record belongs to </td></tr>
 * <tr><td>string</td><td> first </td><td>read, write</td><td> First name </td></tr>
 * <tr><td>string</td><td> last </td><td>read, write</td><td> Last name </td></tr>
 * <tr><td>string</td><td> addition </td><td>read, write</td><td> Middle name </td></tr>
 * <tr><td>string</td><td> suffix </td><td>read, write</td><td> Suffix </td></tr>
 * <tr><td>string</td><td> prefix </td><td>read, write</td><td> Prefix </td></tr>
 * <tr><td>string</td><td> phonetic_first </td><td>read, write</td><td> Pronounce the first name </td></tr>
 * <tr><td>string</td><td> phonetic_middle </td><td>read, write</td><td> Pronounce the middle name  </td></tr>
 * <tr><td>string</td><td> phonetic_last </td><td>read, write</td><td> Pronounce the last name </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
	_CONTACTS_PROPERTY_STR(first)             /* read, write */
	_CONTACTS_PROPERTY_STR(last)              /* read, write */
	_CONTACTS_PROPERTY_STR(addition)          /* read, write */
	_CONTACTS_PROPERTY_STR(suffix)            /* read, write */
	_CONTACTS_PROPERTY_STR(prefix)            /* read, write */
	_CONTACTS_PROPERTY_STR(phonetic_first)    /* read, write */
	_CONTACTS_PROPERTY_STR(phonetic_middle)   /* read, write */
	_CONTACTS_PROPERTY_STR(phonetic_last)     /* read, write */
_CONTACTS_END_VIEW(_contacts_name)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number _contacts_number view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts number view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the number </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the number belongs to</td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> Number type, refer to the @ref contacts_number_type_e </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> Custom number type label, when the number type is #CONTACTS_NUMBER_TYPE_CUSTOM </td></tr>
 * <tr><td>boolean</td><td> is_default </td><td>read, write</td><td> The number is default number or not </td></tr>
 * <tr><td>string</td><td> number </td><td>read, write</td><td> Number </td></tr>
 * <tr><td>string</td><td> normalized_number </td><td> filter only </td><td> You can only use this property for search filter. </td></tr>
 * <tr><td>string</td><td> cleaned_number </td><td> filter only </td><td> You can only use this property for search filter. </td></tr>
 * <tr><td>string</td><td> number_filter </td><td> filter only </td><td> You can only use this property for search filter. </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
	_CONTACTS_PROPERTY_INT(type)              /* read, write */
	_CONTACTS_PROPERTY_STR(label)             /* read, write */
	_CONTACTS_PROPERTY_BOOL(is_default)       /* read, write */
	_CONTACTS_PROPERTY_STR(number)            /* read, write */
	_CONTACTS_PROPERTY_STR(normalized_number) /* filter only */
	_CONTACTS_PROPERTY_STR(cleaned_number)    /* filter only */
	_CONTACTS_PROPERTY_STR(number_filter)     /* filter only */
_CONTACTS_END_VIEW(_contacts_number)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email _contacts_email view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts email view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the email </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the email belongs to </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> Email type, refer to the @ref contacts_email_type_e  </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> Custom mail type label, when the email type is #CONTACTS_EMAIL_TYPE_CUSTOM </td></tr>
 * <tr><td>boolean</td><td> is_default </td><td>read, write</td><td>  The email is default email or not </td></tr>
 * <tr><td>string</td><td> email </td><td>read, write</td><td> Email address</td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
	_CONTACTS_PROPERTY_INT(type)              /* read, write */
	_CONTACTS_PROPERTY_STR(label)             /* read, write */
	_CONTACTS_PROPERTY_BOOL(is_default)       /* read, write */
	_CONTACTS_PROPERTY_STR(email)             /* read, write */
_CONTACTS_END_VIEW(_contacts_email)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address _contacts_address view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts address view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the address </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the address belongs to </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> Address type, refer to the @ref contacts_address_type_e </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> Address type label, when the address type is #CONTACTS_ADDRESS_TYPE_CUSTOM </td></tr>
 * <tr><td>string</td><td> postbox </td><td>read, write</td><td> Post office box </td></tr>
 * <tr><td>string</td><td> postal_code </td><td>read, write</td><td> Postal code </td></tr>
 * <tr><td>string</td><td> region </td><td>read, write</td><td> Region </td></tr>
 * <tr><td>string</td><td> locality </td><td>read, write</td><td> Locality </td></tr>
 * <tr><td>string</td><td> street </td><td>read, write</td><td> Street </td></tr>
 * <tr><td>string</td><td> country </td><td>read, write</td><td> Country </td></tr>
 * <tr><td>string</td><td> extended </td><td>read, write</td><td> Extended address </td></tr>
 * <tr><td>boolean</td><td> is_default </td><td>read, write</td><td> The address is default or not </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
	_CONTACTS_PROPERTY_INT(type)              /* read, write */
	_CONTACTS_PROPERTY_STR(label)             /* read, write */
	_CONTACTS_PROPERTY_STR(postbox)           /* read, write */
	_CONTACTS_PROPERTY_STR(extended)          /* read, write */
	_CONTACTS_PROPERTY_STR(street)            /* read, write */
	_CONTACTS_PROPERTY_STR(locality)          /* read, write */
	_CONTACTS_PROPERTY_STR(region)            /* read, write */
	_CONTACTS_PROPERTY_STR(postal_code)       /* read, write */
	_CONTACTS_PROPERTY_STR(country)           /* read, write */
	_CONTACTS_PROPERTY_BOOL(is_default)       /* read, write */
_CONTACTS_END_VIEW(_contacts_address)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note _contacts_note view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts note view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the note </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the note belongs to </td></tr>
 * <tr><td>string</td><td> note </td><td>read, write</td><td> Note contents </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)            /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)    /* read, write once */
	_CONTACTS_PROPERTY_STR(note)          /* read, write */
_CONTACTS_END_VIEW(_contacts_note)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url _contacts_url view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts URL view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the URL </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the URL belongs to </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> URL type, refer to the @ref contacts_url_type_e </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> Custom URL type label, when the URL type is #CONTACTS_URL_TYPE_CUSTOM </td></tr>
 * <tr><td>string</td><td> url </td><td>read, write</td><td> URL </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
	_CONTACTS_PROPERTY_INT(type)              /* read, write */
	_CONTACTS_PROPERTY_STR(label)             /* read, write */
	_CONTACTS_PROPERTY_STR(url)               /* read, write */
_CONTACTS_END_VIEW(_contacts_url)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event _contacts_event view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts event view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the event </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the event belongs to </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> Event type, refer to the @ref contacts_event_type_e </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> Custom event type label, when the event type is #CONTACTS_EVENT_TYPE_CUSTOM </td></tr>
 * <tr><td>integer</td><td> date </td><td>read, write</td><td> Event date(YYYYMMDD). e.g. 2014/1/1 : 20140101. Even if the calendar_type is set as CONTACTS_EVENT_CALENDAR_TYPE_CHINESE, you SHOULD set Gregorian date </td></tr>
 * <tr><td>integer</td><td> calendar_type </td><td>read, write</td><td> Calendar type, refer to the @ref contacts_event_calendar_type_e </td></tr>
 * <tr><td>bool</td><td> is_leap_month (Deprecated) </td><td>read, write</td><td> The month is leap or not (valid on lunisolar calendar only) </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
	_CONTACTS_PROPERTY_INT(type)              /* read, write */
	_CONTACTS_PROPERTY_STR(label)             /* read, write */
	_CONTACTS_PROPERTY_INT(date)              /* read, write */
	_CONTACTS_PROPERTY_INT(calendar_type)     /* read, write */
	_CONTACTS_PROPERTY_BOOL(is_leap_month)    /* read, write (Deprecated since 2.4) */
_CONTACTS_END_VIEW(_contacts_event)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group_relation _contacts_group_relation view
 * Refer @ref contacts_group_add_contact, @ref contacts_group_remove_contact
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this relationship view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the group (can not be used as filter) </td></tr>
 * <tr><td>integer</td><td> group_id </td><td>read, write once</td><td> DB record ID of the group </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> DB record ID of the contact </td></tr>
 * <tr><td>string</td><td> name </td><td>read only</td><td> Group name </td></tr>
 * <tr><td>integer</td><td> snippet_type </td><td>read only</td><td> kerword matched data type, refer to they @ref contacts_data_type_e (Since 3.0) </td></tr>
 * <tr><td>string</td><td> snippet_string </td><td>read only</td><td> keyword matched data string (Since 3.0) </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                /* read only, can not be used as filter */
	_CONTACTS_PROPERTY_INT(group_id)          /* read, write once */
	_CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
	_CONTACTS_PROPERTY_STR(name)              /* read only */
	_CONTACTS_PROPERTY_INT(snippet_type)		/* read only (Since 3.0) */
	_CONTACTS_PROPERTY_STR(snippet_string)		/* read only (Since 3.0) */
_CONTACTS_END_VIEW(_contacts_group_relation)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship _contacts_relationship view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this relationship view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the relationship </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the relationship belongs to </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> Relationship type, refer to the @ref contacts_relationship_type_e </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> Custom relationship type label, when the relationship type is CONTACTS_RELATIONSHIP_TYPE_CUSTOM </td></tr>
 * <tr><td>string</td><td> name </td><td>read, write</td><td> Selected contact name that the relationship belongs to </td></tr>
 * </table>
 *
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
	_CONTACTS_PROPERTY_INT(type)              /* read, write */
	_CONTACTS_PROPERTY_STR(label)             /* read, write */
	_CONTACTS_PROPERTY_STR(name)              /* read, write */
_CONTACTS_END_VIEW(_contacts_relationship)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image _contacts_image view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts image view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the image </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the image belongs to </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> Image type, refer to the @ref contacts_image_type_e </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> Custom image type label, when the image type is #CONTACTS_IMAGE_TYPE_CUSTOM </td></tr>
 * <tr><td>string</td><td> path </td><td>read, write</td><td> Image thumbnail path </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
	_CONTACTS_PROPERTY_INT(type)              /* read, write */
	_CONTACTS_PROPERTY_STR(label)             /* read, write */
	_CONTACTS_PROPERTY_STR(path)              /* read, write */
	_CONTACTS_PROPERTY_BOOL(is_default)
_CONTACTS_END_VIEW(_contacts_image)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company _contacts_company view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts company view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the company </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the company belongs to </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> Company type, refer to the @ref contacts_company_type_e </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> Custom company type label, when the company type is #CONTACTS_COMPANY_TYPE_CUSTOM </td></tr>
 * <tr><td>string</td><td> name </td><td>read, write</td><td> Company name </td></tr>
 * <tr><td>string</td><td> department </td><td>read, write</td><td> Department </td></tr>
 * <tr><td>string</td><td> job_title </td><td>read, write</td><td> Job title </td></tr>
 * <tr><td>string</td><td> assistant_name </td><td>read, write</td><td> Assistant name </td></tr>
 * <tr><td>string</td><td> role </td><td>read, write</td><td> Role </td></tr>
 * <tr><td>string</td><td> logo </td><td>read, write</td><td> Company logo image file path </td></tr>
 * <tr><td>string</td><td> location </td><td>read, write</td><td> Company location </td></tr>
 * <tr><td>string</td><td> description </td><td>read, write</td><td> Description </td></tr>
 * <tr><td>string</td><td> phonetic_name </td><td>read, write</td><td> Pronounce the company name </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)            /* read, write once */
	_CONTACTS_PROPERTY_INT(type)                  /* read, write */
	_CONTACTS_PROPERTY_STR(label)                 /* read, write */
	_CONTACTS_PROPERTY_STR(name)                  /* read, write */
	_CONTACTS_PROPERTY_STR(department)            /* read, write */
	_CONTACTS_PROPERTY_STR(job_title)             /* read, write */
	_CONTACTS_PROPERTY_STR(assistant_name)        /* read, write */
	_CONTACTS_PROPERTY_STR(role)                  /* read, write */
	_CONTACTS_PROPERTY_STR(logo)                  /* read, write */
	_CONTACTS_PROPERTY_STR(location)              /* read, write */
	_CONTACTS_PROPERTY_STR(description)           /* read, write */
	_CONTACTS_PROPERTY_STR(phonetic_name)         /* read, write */
_CONTACTS_END_VIEW(_contacts_company)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname _contacts_nickname view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts nickname view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the nickname </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the nickname belongs to </td></tr>
 * <tr><td>string</td><td> name </td><td>read, write</td><td> Nickname </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)            /* read, write once */
	_CONTACTS_PROPERTY_STR(name)                  /* read, write */
_CONTACTS_END_VIEW(_contacts_nickname)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger _contacts_messenger view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts messenger view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the messenger </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the messenger belongs to </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> Messenger type, refer to the @ref contacts_messenger_type_e </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> Custom messenger type label, when the messenger type is #CONTACTS_MESSENGER_TYPE_CUSTOM </td></tr>
 * <tr><td>string</td><td> im_id </td><td>read, write</td><td> Messenger ID (email address or email ID...) </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)            /* read, write once */
	_CONTACTS_PROPERTY_INT(type)                  /* read, write */
	_CONTACTS_PROPERTY_STR(label)                 /* read, write */
	_CONTACTS_PROPERTY_STR(im_id)                 /* read, write */
_CONTACTS_END_VIEW(_contacts_messenger)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension _contacts_extension view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts extension view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the contact extension </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the contact extension belongs to </td></tr>
 * <tr><td>integer</td><td> data1 </td><td>read, write</td><td> The extra child record format for non-provided from contacts-service </td></tr>
 * <tr><td>string</td><td> data2 </td><td>read, write</td><td> The extra child record format for non-provided from contacts-service </td></tr>
 * <tr><td>string</td><td> data3 </td><td>read, write</td><td> The extra child record format for non-provided from contacts-service </td></tr>
 * <tr><td>string</td><td> data4 </td><td>read, write</td><td> The extra child record format for non-provided from contacts-service </td></tr>
 * <tr><td>string</td><td> data5 </td><td>read, write</td><td> The extra child record format for non-provided from contacts-service </td></tr>
 * <tr><td>string</td><td> data6 </td><td>read, write</td><td> The extra child record format for non-provided from contacts-service </td></tr>
 * <tr><td>string</td><td> data7 </td><td>read, write</td><td> The extra child record format for non-provided from contacts-service </td></tr>
 * <tr><td>string</td><td> data8 </td><td>read, write</td><td> The extra child record format for non-provided from contacts-service </td></tr>
 * <tr><td>string</td><td> data9 </td><td>read, write</td><td> The extra child record format for non-provided from contacts-service </td></tr>
 * <tr><td>string</td><td> data10 </td><td>read, write</td><td> The extra child record format for non-provided from contacts-service </td></tr>
 * <tr><td>string</td><td> data11 </td><td>read, write</td><td> The extra child record format for non-provided from contacts-service </td></tr>
 * <tr><td>string</td><td> data12 </td><td>read, write</td><td> The extra child record format for non-provided from contacts-service </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)            /* read, write once */
	_CONTACTS_PROPERTY_INT(data1)                 /* read, write */
	_CONTACTS_PROPERTY_STR(data2)                 /* read, write */
	_CONTACTS_PROPERTY_STR(data3)                 /* read, write */
	_CONTACTS_PROPERTY_STR(data4)                 /* read, write */
	_CONTACTS_PROPERTY_STR(data5)                 /* read, write */
	_CONTACTS_PROPERTY_STR(data6)                 /* read, write */
	_CONTACTS_PROPERTY_STR(data7)                 /* read, write */
	_CONTACTS_PROPERTY_STR(data8)                 /* read, write */
	_CONTACTS_PROPERTY_STR(data9)                 /* read, write */
	_CONTACTS_PROPERTY_STR(data10)                /* read, write */
	_CONTACTS_PROPERTY_STR(data11)                /* read, write */
	_CONTACTS_PROPERTY_STR(data12)                /* read, write */
_CONTACTS_END_VIEW(_contacts_extension)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_sdn _contacts_sdn view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts sdn view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the sdn </td></tr>
 * <tr><td>string</td><td> name </td><td>read only</td><td> Provided name of sdn </td></tr>
 * <tr><td>string</td><td> number </td><td>read only</td><td> Provided number of sdn </td></tr>
 * <tr><td>integer</td><td> sim_slot_no </td><td>read only</td><td>It is related to the SIM slot number. sim_slot_no 0 means first SIM card, sim_slot_no 1 means second SIM. It is same with handle index of telephony handle list. Refer to the telephony_init() </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_STR(name)                  /* read only */
	_CONTACTS_PROPERTY_STR(number)                /* read only */
	_CONTACTS_PROPERTY_INT(sim_slot_no)           /* read only */
_CONTACTS_END_VIEW(_contacts_sdn)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile _contacts_profile view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts profile view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of profile </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contacts ID that the profile belongs to </td></tr>
 * <tr><td>string</td><td> uid </td><td>read, write</td><td> Unique identifier </td></tr>
 * <tr><td>string</td><td> text </td><td>read, write</td><td> Profile contents </td></tr>
 * <tr><td>integer</td><td> order </td><td>read, write</td><td> Priority to display the profile </td></tr>
 * <tr><td>string</td><td> service_operation </td><td>read, write</td><td> Data for app_control_set_operation </td></tr>
 * <tr><td>string</td><td> mime </td><td>read, write</td><td> Data for app_control_set_mime </td></tr>
 * <tr><td>string</td><td> app_id </td><td>read, write</td><td> Data for app_control_set_app_id </td></tr>
 * <tr><td>string</td><td> uri </td><td>read, write</td><td> Data for app_control_set_uri </td></tr>
 * <tr><td>string</td><td> catagory </td><td>read, write</td><td> Data for app_control_set_category </td></tr>
 * <tr><td>string</td><td> extra_data </td><td>read, write</td><td> It includes "key:value,key:value," pairs. You should parse it. And you must base64 encode each key and value</td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_STR(uid)                   /* read, write */
	_CONTACTS_PROPERTY_STR(text)                  /* read, write */
	_CONTACTS_PROPERTY_INT(order)                 /* read, write */
	_CONTACTS_PROPERTY_STR(service_operation)     /* read, write */
	_CONTACTS_PROPERTY_STR(mime)                  /* read, write */
	_CONTACTS_PROPERTY_STR(app_id)                /* read, write */
	_CONTACTS_PROPERTY_STR(uri)                   /* read, write */
	_CONTACTS_PROPERTY_STR(category)              /* read, write */
	_CONTACTS_PROPERTY_STR(extra_data)            /* read, write */
	_CONTACTS_PROPERTY_INT(contact_id)            /* read, write once */
_CONTACTS_END_VIEW(_contacts_profile)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo _contacts_activity_photo view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contact activity photo view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of activity photo </td></tr>
 * <tr><td>integer</td><td> activity_id </td><td>read, write once</td><td> Activity ID that the activity photo belongs to </td></tr>
 * <tr><td>string</td><td> photo_url </td><td>read, write</td><td> Photo URL </td></tr>
 * <tr><td>integer</td><td> sort_index </td><td>read, write</td><td> Sorted photo index </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_INT(activity_id)           /* read, write once */
	_CONTACTS_PROPERTY_STR(photo_url)             /* read, write */
	_CONTACTS_PROPERTY_INT(sort_index)            /* read, write */
_CONTACTS_END_VIEW(_contacts_activity_photo)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity _contacts_activity view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this activity view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of activity </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the activity belongs to </td></tr>
 * <tr><td>string</td><td> source_name </td><td>read, write</td><td> Account name that the activity belongs to </td></tr>
 * <tr><td>int</td><td> timestamp </td><td>read, write</td><td> Published time of activity </td></tr>
 * <tr><td>string</td><td> status </td><td>read, write</td><td> Activity status </td></tr>
 * <tr><td>string</td><td> service_operation </td><td>read, write</td><td> Data for app_control_set_operation </td></tr>
 * <tr><td>string</td><td> uri </td><td>read, write</td><td> Data for app_control_set_uri </td></tr>
 * <tr><td>record</td><td> photo </td><td>read, write</td><td> _contacts_activity_photo child record (multiple) </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)            /* read, write once */
	_CONTACTS_PROPERTY_STR(source_name)           /* read, write */
	_CONTACTS_PROPERTY_STR(status)                /* read, write */
	_CONTACTS_PROPERTY_INT(timestamp)             /* read, write */
	_CONTACTS_PROPERTY_STR(service_operation)     /* read, write */
	_CONTACTS_PROPERTY_STR(uri)                   /* read, write */
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(photo)      /* read, write */
_CONTACTS_END_VIEW(_contacts_activity)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial _contacts_speeddial view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contact speed dial view </td></tr>
 * <tr><td>integer</td><td> speeddial_number </td><td>read, write once</td><td> Stored speed dial number </td></tr>
 * <tr><td>integer</td><td> number_id </td><td>read, write</td><td> Number ID that the speed dial belongs to </td></tr>
 * <tr><td>string</td><td> number </td><td>read only</td><td> Contact number of specified speed dial </td></tr>
 * <tr><td>string</td><td> number_label </td><td>read only</td><td> Contact number label of specified speed dial, when the number type is CONTACTS_NUMBER_TYPE_CUSTOM </td></tr>
 * <tr><td>integer</td><td> number_type </td><td>read only</td><td> Contact number type, refer to the @ref contacts_number_type_e </td></tr>
 * <tr><td>integer</td><td> person_id </td><td>read only</td><td> Person ID that the speed dial belongs to </td></tr>
 * <tr><td>string</td><td> display_name </td><td>read only</td><td> Display name that the speed dial belongs to </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td>read only</td><td> Image thumbnail path that the speed dial belongs to </td></tr>
 * <tr><td>string</td><td> normalized_number </td><td>filter only</td><td> You can only use this property for search filter </td></tr>
 * <tr><td>string</td><td> cleaned_number </td><td>filter only</td><td> You can only use this property for search filter </td></tr>
 * <tr><td>string</td><td> number_filter </td><td>filter only</td><td>  If you add filter with this property, the string will be normalized as minmatch length internally and the match rule will be applied CONTACTS_MATCH_EXACTLY </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(speeddial_number)      /* read, write once */
	_CONTACTS_PROPERTY_INT(number_id)             /* read, write */
	_CONTACTS_PROPERTY_STR(number)                /* read only */
	_CONTACTS_PROPERTY_STR(number_label)          /* read only */
	_CONTACTS_PROPERTY_INT(number_type)           /* read only */
	_CONTACTS_PROPERTY_INT(person_id)             /* read only */
	_CONTACTS_PROPERTY_STR(display_name)          /* read only */
	_CONTACTS_PROPERTY_STR(image_thumbnail_path)  /* read only */
	_CONTACTS_PROPERTY_STR(normalized_number)     /* filter only */
	_CONTACTS_PROPERTY_STR(cleaned_number)        /* filter only */
	_CONTACTS_PROPERTY_STR(number_filter)         /* filter only */
_CONTACTS_END_VIEW(_contacts_speeddial)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log _contacts_phone_log view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this phone log view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of phone log </td></tr>
 * <tr><td>integer</td><td> person_id </td><td>read, write once </td><td> Person ID that the phone log belongs to </td></tr>
 * <tr><td>string</td><td> address </td><td>read, write once </td><td> Number or Email that the phone log displays </td></tr>
 * <tr><td>integer</td><td> log_time </td><td>read, write once</td><td> Call end time. The value means number of seconds since 1970-01-01 00:00:00 (UTC) </td></tr>
 * <tr><td>integer</td><td> log_type </td><td>read, write</td><td> Log type, refer to the @ref contacts_phone_log_type_e </td></tr>
 * <tr><td>integer</td><td> extra_data1 </td><td>read, write once</td><td> You can set the related integer data (e.g. message_id, email_id or duration(seconds) of call) </td></tr>
 * <tr><td>string</td><td> extra_data2 </td><td>read, write once</td><td> You can set the related string data (e.g. short message, subject) </td></tr>
 * <tr><td>string</td><td> normalized_address </td><td> filter only</td><td> You can only use this property for search filter</td></tr>
 * <tr><td>string</td><td> cleaned_address </td><td> filter only</td><td> You can only use this property for search filter</td></tr>
 * <tr><td>string</td><td> address_filter </td><td> filter only</td><td> You can only use this property for search filter</td></tr>
 * <tr><td>integer</td><td> sim_slot_no </td><td>read, write once</td><td> You can set the related SIM slot number. sim_slot_no 0 means first SIM card, sim_slot_no 1 means second SIM. It is same with handle index of telephony handle list. Refer to the telephony_init() </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT(id)                    /* read only */
	_CONTACTS_PROPERTY_INT(person_id)             /* read, write once */
	_CONTACTS_PROPERTY_STR(address)               /* read, write once, number or email */
	_CONTACTS_PROPERTY_INT(log_time)              /* read, write once */
	_CONTACTS_PROPERTY_INT(log_type)              /* read, write */
	_CONTACTS_PROPERTY_INT(extra_data1)           /* read, write once : message or email ID, duration(seconds) */
	_CONTACTS_PROPERTY_STR(extra_data2)           /* read, write once : shortmsg, subject */
	_CONTACTS_PROPERTY_STR(normalized_address)    /* filter only */
	_CONTACTS_PROPERTY_STR(cleaned_address)       /* filter only */
	_CONTACTS_PROPERTY_STR(address_filter)        /* filter only */
	_CONTACTS_PROPERTY_INT(sim_slot_no)           /* read, write once */
_CONTACTS_END_VIEW(_contacts_phone_log)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact_updated_info _contacts_contact_updated_info view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this contact updated info view </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td> Updated contact ID </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td> Addressbook ID that the updated contact belongs to </td></tr>
 * <tr><td>integer</td><td> type </td><td> Contact updated type, refer to the @ref contacts_changed_e </td></tr>
 * <tr><td>integer</td><td> version </td><td> Updated version </td></tr>
 * <tr><td>boolean</td><td> image_changed </td><td> Contact image is changed or not </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(contact_id)
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_INT(type)               /* insert/update/delete */
	_CONTACTS_PROPERTY_INT(version)
	_CONTACTS_PROPERTY_BOOL(image_changed)
_CONTACTS_END_READ_ONLY_VIEW(_contacts_contact_updated_info)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile_updated_info _contacts_my_profile_updated_info view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this my profile updated info view </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td> Address book ID that the updated my profile belongs to </td></tr>
 * <tr><td>integer</td><td> last_changed_type </td><td> Changed update type, refer to the @ref contacts_changed_e </td></tr>
 * <tr><td>integer</td><td> version </td><td> Updated version </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_INT(last_changed_type)
	_CONTACTS_PROPERTY_INT(version)
_CONTACTS_END_READ_ONLY_VIEW(_contacts_my_profile_updated_info)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group_updated_info _contacts_group_updated_info view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this group updated info view </td></tr>
 * <tr><td>integer</td><td> group_id </td><td> Updated group ID </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td> Address book ID that the updated group belongs to </td></tr>
 * <tr><td>integer</td><td> type </td><td> Changed update type, refer to the @ref contacts_changed_e </td></tr>
 * <tr><td>integer</td><td> version </td><td> Updated version </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(group_id)
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_INT(type)                /* insert/update/delete */
	_CONTACTS_PROPERTY_INT(version)
_CONTACTS_END_READ_ONLY_VIEW(_contacts_group_updated_info)


/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group_member_updated_info _contacts_group_member_updated_info view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this group member updated info view </td></tr>
 * <tr><td>integer</td><td> group_id </td><td> Updated group ID </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td> Address book ID that the updated group belongs to </td></tr>
 * <tr><td>integer</td><td> version </td><td> Updated version </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(group_id)
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_INT(version)
_CONTACTS_END_READ_ONLY_VIEW(_contacts_group_member_updated_info)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_grouprel_updated_info _contacts_grouprel_updated_info view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this group relation updated info view </td></tr>
 * <tr><td>integer</td><td> group_id </td><td> Group ID of group relation </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td> Contact ID of the updated group relation </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td> Address book ID of contact that the updated group relation </td></tr>
 * <tr><td>integer</td><td> type </td><td> Changed update type, refer to the @ref contacts_changed_e </td></tr>
 * <tr><td>integer</td><td> version </td><td> Updated version </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(group_id)
	_CONTACTS_PROPERTY_INT(contact_id)
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_INT(type)                /* insert/delete */
	_CONTACTS_PROPERTY_INT(version)
_CONTACTS_END_READ_ONLY_VIEW(_contacts_grouprel_updated_info)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_contact _contacts_person_contact view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this person contact view </td></tr>
 * <tr><td>integer</td><td> person_id </td><td> DB record ID of the person </td></tr>
 * <tr><td>string</td><td> display_name </td><td> Display name of the person</td></tr>
 * <tr><td>string</td><td> display_name_index </td><td> The first character of first string for grouping. This is normalized using icu (projection) </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td> Display contact ID that the person belongs to (projection) </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td> Ringtone path of the person (projection) </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td> Image thumbnail path of the person (projection) </td></tr>
 * <tr><td>string</td><td> vibration </td><td> Vibration path of the person (projection) </td></tr>
 * <tr><td>string</td><td> message_alert </td><td> Message alert path of the person  (projection) </td></tr>
 * <tr><td>string</td><td> status </td><td> Status of social account (projection) </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td> The person is favorite or not </td></tr>
 * <tr><td>integer</td><td> link_count </td><td> Link count of contact records (projection) </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td> Contact ID that the person belongs to </td></tr>
 * <tr><td>string</td><td> addressbook_ids </td><td> Addressbook IDs that the person belongs to (projection) </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td> The person has phone number or not </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td> The person has email or not </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td> Addressbook ID that the person belongs to </td></tr>
 * <tr><td>integer</td><td> address_book_mode </td><td> Addressbook mode, refer to the @ref contacts_address_book_mode_e </td></tr>
 * <tr><td>string</td><td> address_book_name </td><td> Addressbook name that the person belongs to </td></tr>
 * <tr><td>integer</td><td> snippet_type </td><td>read only</td><td> kerword matched data type, refer to they @ref contacts_data_type_e (Since 3.0) </td></tr>
 * <tr><td>string</td><td> snippet_string </td><td>read only</td><td> keyword matched data string (Since 3.0) </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(person_id)
	_CONTACTS_PROPERTY_STR(display_name)
	_CONTACTS_PROPERTY_PROJECTION_STR(display_name_index)
	_CONTACTS_PROPERTY_PROJECTION_INT(display_contact_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(ringtone_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(image_thumbnail_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(vibration)
	_CONTACTS_PROPERTY_PROJECTION_STR(status)
	_CONTACTS_PROPERTY_BOOL(is_favorite)
	_CONTACTS_PROPERTY_PROJECTION_INT(link_count)
	_CONTACTS_PROPERTY_PROJECTION_STR(addressbook_ids)
	_CONTACTS_PROPERTY_BOOL(has_phonenumber)
	_CONTACTS_PROPERTY_BOOL(has_email)
	_CONTACTS_PROPERTY_INT(contact_id)
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_STR(address_book_name)
	_CONTACTS_PROPERTY_INT(address_book_mode)
	_CONTACTS_PROPERTY_PROJECTION_STR(message_alert)
	_CONTACTS_PROPERTY_INT(snippet_type)		/* read only (Since 3.0) */
	_CONTACTS_PROPERTY_STR(snippet_string)		/* read only (Since 3.0) */
_CONTACTS_END_READ_ONLY_VIEW(_contacts_person_contact)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_number _contacts_person_number view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this person number view </td></tr>
 * <tr><td>integer</td><td> person_id </td><td> DB record ID of the person </td></tr>
 * <tr><td>string</td><td> display_name </td><td> Display name of the person</td></tr>
 * <tr><td>string</td><td> display_name_index </td><td> The first character of first string for grouping. This is normalized using icu (projection) </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td> Display contact ID that the person belongs to (projection) </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td> Ringtone path of the person (projection) </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td> Image thumbnail path of the person (projection) </td></tr>
 * <tr><td>string</td><td> vibration </td><td> Vibration path of the person (projection) </td></tr>
 * <tr><td>string</td><td> message_alert </td><td> Message alert path of the person  (projection) </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td> The person is favorite or not </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td> The person has phone number or not </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td> The person has email or not </td></tr>
 * <tr><td>integer</td><td> number_id </td><td> Number ID that the person belongs to </td></tr>
 * <tr><td>integer</td><td> type </td><td> Number type, refer to the @ref contacts_number_type_e (projection) </td></tr>
 * <tr><td>string</td><td> label </td><td> Custom number type label, when the number type is #CONTACTS_NUMBER_TYPE_CUSTOM (projection) </td></tr>
 * <tr><td>boolean</td><td> is_primary_default </td><td> The number is default number or not </td></tr>
 * <tr><td>string</td><td> number </td><td> Number </td></tr>
 * <tr><td>string</td><td> number_filter </td><td> If you add filter with this property, the string will be normalized as minmatch length internally and the match rule will be applied CONTACTS_MATCH_EXACTLY </td></tr>
 * <tr><td>string</td><td> normalized_number </td><td> You can only use this property for search filter</td></tr>
 * <tr><td>string</td><td> cleaned_number </td><td>You can only use this property for search filter </td></tr>
 * <tr><td>integer</td><td> snippet_type </td><td>read only</td><td> kerword matched data type, refer to they @ref contacts_data_type_e (Since 3.0) </td></tr>
 * <tr><td>string</td><td> snippet_string </td><td>read only</td><td> keyword matched data string (Since 3.0) </td></tr>
 * </table>

 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(person_id)
	_CONTACTS_PROPERTY_STR(display_name)
	_CONTACTS_PROPERTY_PROJECTION_STR(display_name_index)
	_CONTACTS_PROPERTY_PROJECTION_INT(display_contact_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(ringtone_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(image_thumbnail_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(vibration)
	_CONTACTS_PROPERTY_BOOL(is_favorite)
	_CONTACTS_PROPERTY_BOOL(has_phonenumber)
	_CONTACTS_PROPERTY_BOOL(has_email)
	_CONTACTS_PROPERTY_INT(number_id)
	_CONTACTS_PROPERTY_PROJECTION_INT(type)
	_CONTACTS_PROPERTY_PROJECTION_STR(label)
	_CONTACTS_PROPERTY_BOOL(is_primary_default)
	_CONTACTS_PROPERTY_STR(number)
	_CONTACTS_PROPERTY_FILTER_STR(number_filter)
	_CONTACTS_PROPERTY_FILTER_STR(normalized_number)
	_CONTACTS_PROPERTY_PROJECTION_STR(message_alert)
	_CONTACTS_PROPERTY_FILTER_STR(cleaned_number)
	_CONTACTS_PROPERTY_INT(snippet_type)		/* read only (Since 3.0) */
	_CONTACTS_PROPERTY_STR(snippet_string)		/* read only (Since 3.0) */
_CONTACTS_END_READ_ONLY_VIEW(_contacts_person_number)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_email _contacts_person_email view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this person email view </td></tr>
 * <tr><td>integer</td><td> person_id </td><td> DB record ID of the person </td></tr>
 * <tr><td>string</td><td> display_name </td><td> Display name of the person </td></tr>
 * <tr><td>string</td><td> display_name_index </td><td> The first character of first string for grouping. This is normalized using icu (projection) </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td> Display contact ID that the person belongs to (projection) </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td> Ringtone path of the person (projection) </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td> Image thumbnail path of the person (projection) </td></tr>
 * <tr><td>string</td><td> vibration </td><td> Vibration path of the person (projection) </td></tr>
 * <tr><td>string</td><td> message_alert </td><td> Message alert path of the person  (projection) </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td> The person is favorite or not </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td> The person has phone number or not </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td> The person has email or not </td></tr>
 * <tr><td>integer</td><td> email_id </td><td> Email ID that the person belongs to </td></tr>
 * <tr><td>integer</td><td> type </td><td> Email type, refer to the @ref contacts_email_type_e (projection) </td></tr>
 * <tr><td>string</td><td> label </td><td> Custom mail type label, when the email type is #CONTACTS_EMAIL_TYPE_CUSTOM (projection) </td></tr>
 * <tr><td>boolean</td><td> is_primary_default </td><td> The email is default email or not </td></tr>
 * <tr><td>string</td><td> email </td><td> Email address</td></tr>
 * <tr><td>integer</td><td> snippet_type </td><td>read only</td><td> kerword matched data type, refer to they @ref contacts_data_type_e (Since 3.0) </td></tr>
 * <tr><td>string</td><td> snippet_string </td><td>read only</td><td> keyword matched data string (Since 3.0) </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(person_id)
	_CONTACTS_PROPERTY_STR(display_name)
	_CONTACTS_PROPERTY_PROJECTION_STR(display_name_index)
	_CONTACTS_PROPERTY_PROJECTION_INT(display_contact_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(ringtone_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(image_thumbnail_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(vibration)
	_CONTACTS_PROPERTY_BOOL(is_favorite)
	_CONTACTS_PROPERTY_BOOL(has_phonenumber)
	_CONTACTS_PROPERTY_BOOL(has_email)
	_CONTACTS_PROPERTY_INT(email_id)
	_CONTACTS_PROPERTY_PROJECTION_INT(type)
	_CONTACTS_PROPERTY_PROJECTION_STR(label)
	_CONTACTS_PROPERTY_BOOL(is_primary_default)
	_CONTACTS_PROPERTY_STR(email)
	_CONTACTS_PROPERTY_PROJECTION_STR(message_alert)
	_CONTACTS_PROPERTY_INT(snippet_type)		/* read only (Since 3.0) */
	_CONTACTS_PROPERTY_STR(snippet_string)		/* read only (Since 3.0) */
_CONTACTS_END_READ_ONLY_VIEW(_contacts_person_email)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_grouprel _contacts_person_grouprel view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this person group relation view </td></tr>
 * <tr><td>integer</td><td> person_id </td><td> DB record ID of the person </td></tr>
 * <tr><td>string</td><td> display_name </td><td> Display name of the person </td></tr>
 * <tr><td>string</td><td> display_name_index </td><td> The first character of first string for grouping. This is normalized using icu (projection) </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td> Display contact ID that the person belongs to (projection) </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td> Ringtone path of the person (projection) </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td> Image thumbnail path of the person (projection) </td></tr>
 * <tr><td>string</td><td> vibration </td><td> Vibration path of the person (projection) </td></tr>
 * <tr><td>string</td><td> message_alert </td><td> Message alert path of the person  (projection) </td></tr>
 * <tr><td>string</td><td> status </td><td> Status of social account (projection) </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td> The person is favorite or not </td></tr>
 * <tr><td>integer</td><td> link_count </td><td> Link count of contat records (projection)  </td></tr>
 * <tr><td>string</td><td> addressbook_ids </td><td> Addressbook IDs that the person belongs to (projection) </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td> The person has phone number or not </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td> The person has email or not </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td> Addressbook ID that the person belongs to </td></tr>
 * <tr><td>integer</td><td> address_book_mode </td><td> Addressbook mode, refer to the @ref contacts_address_book_mode_e </td></tr>
 * <tr><td>string</td><td> address_book_name </td><td> Addressbook name that the person belongs to </td></tr>
 * <tr><td>integer</td><td> group_id </td><td> Group ID that the person belongs to </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td> Contact ID that the person belongs to (projection) </td></tr>
 * <tr><td>integer</td><td> snippet_type </td><td>read only</td><td> kerword matched data type, refer to they @ref contacts_data_type_e (Since 3.0) </td></tr>
 * <tr><td>string</td><td> snippet_string </td><td>read only</td><td> keyword matched data string (Since 3.0) </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(person_id)
	_CONTACTS_PROPERTY_STR(display_name)
	_CONTACTS_PROPERTY_PROJECTION_STR(display_name_index)
	_CONTACTS_PROPERTY_PROJECTION_INT(display_contact_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(ringtone_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(image_thumbnail_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(vibration)
	_CONTACTS_PROPERTY_PROJECTION_STR(status)
	_CONTACTS_PROPERTY_BOOL(is_favorite)
	_CONTACTS_PROPERTY_PROJECTION_INT(link_count)
	_CONTACTS_PROPERTY_PROJECTION_STR(addressbook_ids)
	_CONTACTS_PROPERTY_BOOL(has_phonenumber)
	_CONTACTS_PROPERTY_BOOL(has_email)
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_INT(group_id)
	_CONTACTS_PROPERTY_STR(address_book_name)
	_CONTACTS_PROPERTY_INT(address_book_mode)
	_CONTACTS_PROPERTY_PROJECTION_INT(contact_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(message_alert)
	_CONTACTS_PROPERTY_INT(snippet_type)		/* read only (Since 3.0) */
	_CONTACTS_PROPERTY_STR(snippet_string)		/* read only (Since 3.0) */
_CONTACTS_END_READ_ONLY_VIEW(_contacts_person_grouprel)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_assigned _contacts_person_group_assigned view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this person group assigned view </td></tr>
 * <tr><td>integer</td><td> person_id </td><td> DB record ID of the person </td></tr>
 * <tr><td>string</td><td> display_name </td><td> Display name of the person </td></tr>
 * <tr><td>string</td><td> display_name_index </td><td> The first character of first string for grouping. This is normalized using icu (projection) </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td> Display contact ID that the person belongs to (projection) </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td> Ringtone path of the person (projection) </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td> Image thumbnail path of the person (projection) </td></tr>
 * <tr><td>string</td><td> vibration </td><td> Vibration path of the person (projection) </td></tr>
 * <tr><td>string</td><td> message_alert </td><td> Message alert path of the person  (projection) </td></tr>
 * <tr><td>string</td><td> status </td><td> Status of social account (projection) </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td> The person is favorite or not </td></tr>
 * <tr><td>integer</td><td> link_count </td><td> Link count of contact records (projection) </td></tr>
 * <tr><td>string</td><td> linked_address_book_ids </td><td> Addressbook IDs that the linked person belongs to (projection) </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td> The person has phone number or not </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td> The person has email or not </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td> Addressbook ID that the person belongs to </td></tr>
 * <tr><td>integer</td><td> address_book_mode </td><td> Addressbook mode, refer to the @ref contacts_address_book_mode_e </td></tr>
 * <tr><td>integer</td><td> group_id </td><td> Group ID that the person belongs to </td></tr>
 * <tr><td>integer<dtd><td> contact_id </td><td> Contact ID that the person belongs to (projection) </td></tr>
 * <tr><td>integer</td><td> snippet_type </td><td>read only</td><td> kerword matched data type, refer to they @ref contacts_data_type_e (Since 3.0) </td></tr>
 * <tr><td>string</td><td> snippet_string </td><td>read only</td><td> keyword matched data string (Since 3.0) </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(person_id)
	_CONTACTS_PROPERTY_STR(display_name)
	_CONTACTS_PROPERTY_PROJECTION_STR(display_name_index)
	_CONTACTS_PROPERTY_PROJECTION_INT(display_contact_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(ringtone_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(image_thumbnail_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(vibration)
	_CONTACTS_PROPERTY_PROJECTION_STR(status)
	_CONTACTS_PROPERTY_BOOL(is_favorite)
	_CONTACTS_PROPERTY_PROJECTION_INT(link_count)
	_CONTACTS_PROPERTY_PROJECTION_STR(linked_address_book_ids)
	_CONTACTS_PROPERTY_BOOL(has_phonenumber)
	_CONTACTS_PROPERTY_BOOL(has_email)
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_INT(group_id)
	_CONTACTS_PROPERTY_INT(address_book_mode)
	_CONTACTS_PROPERTY_PROJECTION_INT(contact_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(message_alert)
	_CONTACTS_PROPERTY_INT(snippet_type)		/* read only (Since 3.0) */
	_CONTACTS_PROPERTY_STR(snippet_string)		/* read only (Since 3.0) */
_CONTACTS_END_READ_ONLY_VIEW(_contacts_person_group_assigned)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_group_not_assigned _contacts_person_group_not_assigned view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this person group not assigned view </td></tr>
 * <tr><td>integer</td><td> person_id </td><td> DB record ID of the person </td></tr>
 * <tr><td>string</td><td> display_name </td><td> Display name of the person </td></tr>
 * <tr><td>string</td><td> display_name_index </td><td> The first character of first string for grouping. This is normalized using icu (projection) </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td> Display contact ID that the person belongs to (projection) </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td> Ringtone path of the person (projection)  </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td> Image thumbnail path of the person (projection) </td></tr>
 * <tr><td>string</td><td> vibration </td><td> Vibration path of the person (projection) </td></tr>
 * <tr><td>string</td><td> message_alert </td><td> Message alert path of the person  (projection) </td></tr>
 * <tr><td>string</td><td> status </td><td> Status of social account (projection) </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td> The person is favorite or not </td></tr>
 * <tr><td>integer</td><td> link_count </td><td> Link count of contact records (projection) </td></tr>
 * <tr><td>string</td><td> linked_address_book_ids </td><td> Addressbook IDs that the linked person belongs to (projection) </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td> The person has phone number or not </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td> The person has email or not </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td> Addressbook ID that the person belongs to </td></tr>
 * <tr><td>integer</td><td> address_book_mode </td><td> Addressbook mode, refer to the @ref contacts_address_book_mode_e </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td> Contact ID that the person belongs to (projection) </td></tr>
 * <tr><td>integer</td><td> snippet_type </td><td>read only</td><td> kerword matched data type, refer to they @ref contacts_data_type_e (Since 3.0) </td></tr>
 * <tr><td>string</td><td> snippet_string </td><td>read only</td><td> keyword matched data string (Since 3.0) </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(person_id)
	_CONTACTS_PROPERTY_STR(display_name)
	_CONTACTS_PROPERTY_PROJECTION_STR(display_name_index)
	_CONTACTS_PROPERTY_PROJECTION_INT(display_contact_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(ringtone_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(image_thumbnail_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(vibration)
	_CONTACTS_PROPERTY_PROJECTION_STR(status)
	_CONTACTS_PROPERTY_BOOL(is_favorite)
	_CONTACTS_PROPERTY_PROJECTION_INT(link_count)
	_CONTACTS_PROPERTY_PROJECTION_STR(linked_address_book_ids)
	_CONTACTS_PROPERTY_BOOL(has_phonenumber)
	_CONTACTS_PROPERTY_BOOL(has_email)
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_INT(address_book_mode)
	_CONTACTS_PROPERTY_PROJECTION_INT(contact_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(message_alert)
	_CONTACTS_PROPERTY_INT(snippet_type)		/* read only (Since 3.0) */
	_CONTACTS_PROPERTY_STR(snippet_string)		/* read only (Since 3.0) */
_CONTACTS_END_READ_ONLY_VIEW(_contacts_person_group_not_assigned)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_phone_log _contacts_person_phone_log view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this phone log view </td></tr>
 * <tr><td>integer</td><td> person_id </td><td> DB record ID of person </td></tr>
 * <tr><td>string</td><td> display_name </td><td> Display name of the person </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td> Image thumbnail path of the person (projection) </td></tr>
 * <tr><td>integer</td><td> log_id </td><td> DB record ID of phone log </td></tr>
 * <tr><td>string</td><td> address </td><td> Number or Email that the phone log displays </td></tr>
 * <tr><td>integer</td><td> address_type </td><td> Number or Email type (projection)</td></tr>
 * <tr><td>integer</td><td> log_time </td><td> Call end time. The value means number of seconds since 1970-01-01 00:00:00 (UTC) </td></tr>
 * <tr><td>integer</td><td> log_type </td><td> Log type, refer to the @ref contacts_phone_log_type_e </td></tr>
 * <tr><td>integer</td><td> extra_data1 </td><td> You can set the related integer data (e.g. message_id, email_id or duration(seconds) of call) (projection) </td></tr>
 * <tr><td>string</td><td> extra_data2 </td><td> You can set the related string data (e.g. short message, subject) (projection) </td></tr>
 * <tr><td>string</td><td> normalized_address </td><td> You can only use this property for search filter </td></tr>
 * <tr><td>string</td><td> cleaned_address </td><td> You can only use this property for search filter </td></tr>
 * <tr><td>string</td><td> address_filter </td><td> You can only use this property for search filter </td></tr>
 * <tr><td>integer</td><td> sim_slot_no </td><td>It is related to the SIM slot number. sim_slot_no 0 means first SIM card, sim_slot_no 1 means second SIM. It is same with handle index of telephony handle list. Refer to the telephony_init() </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(person_id)
	_CONTACTS_PROPERTY_STR(display_name)
	_CONTACTS_PROPERTY_PROJECTION_STR(image_thumbnail_path)
	_CONTACTS_PROPERTY_INT(log_id)
	_CONTACTS_PROPERTY_STR(address)
	_CONTACTS_PROPERTY_PROJECTION_INT(address_type)
	_CONTACTS_PROPERTY_INT(log_time)
	_CONTACTS_PROPERTY_INT(log_type)
	_CONTACTS_PROPERTY_PROJECTION_INT(extra_data1)
	_CONTACTS_PROPERTY_PROJECTION_STR(extra_data2)
	_CONTACTS_PROPERTY_FILTER_STR(normalized_address)
	_CONTACTS_PROPERTY_FILTER_STR(cleaned_address)
	_CONTACTS_PROPERTY_FILTER_STR(address_filter)
	_CONTACTS_PROPERTY_INT(sim_slot_no)
_CONTACTS_END_READ_ONLY_VIEW(_contacts_person_phone_log)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_usage _contacts_person_usage view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this person usage view </td></tr>
 * <tr><td>integer</td><td> person_id </td><td> DB record ID of the person </td></tr>
 * <tr><td>string</td><td> display_name </td><td> Display name of the person </td></tr>
 * <tr><td>string</td><td> display_name_index </td><td> The first character of first string for grouping. This is normalized using icu (projection) </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td> Display contact ID that the person belongs to (projection)  </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td> Ringtone path of the person (projection) </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td> Image thumbnail path of the person (projection)</td></tr>
 * <tr><td>string</td><td> vibration </td><td> Vibration path of the person (projection) </td></tr>
 * <tr><td>string</td><td> message_alert </td><td> Message alert path of the person (projection) </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td> The person is favorite or not </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td> The person has phone number or not </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td> The person has email or not </td></tr>
 * <tr><td>integer</td><td> usage_type </td><td> Usage type, refer to the @ref contacts_usage_type_e </td></tr>
 * <tr><td>integer</td><td> times_used </td><td> Usage number of person </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(person_id)
	_CONTACTS_PROPERTY_STR(display_name)
	_CONTACTS_PROPERTY_PROJECTION_STR(display_name_index)
	_CONTACTS_PROPERTY_PROJECTION_INT(display_contact_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(ringtone_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(image_thumbnail_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(vibration)
	_CONTACTS_PROPERTY_BOOL(is_favorite)
	_CONTACTS_PROPERTY_BOOL(has_phonenumber)
	_CONTACTS_PROPERTY_BOOL(has_email)
	_CONTACTS_PROPERTY_INT(usage_type)
	_CONTACTS_PROPERTY_INT(times_used)
	_CONTACTS_PROPERTY_PROJECTION_STR(message_alert)
_CONTACTS_END_READ_ONLY_VIEW(_contacts_person_usage)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact_number _contacts_contact_number view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this contacts number view </td></tr>
 * <tr><td>integer</td><td>contact_id</td><td> Contact ID that the number belongs to </td></tr>
 * <tr><td>string</td><td>display_name</td><td> Display name of contact that the number belongs to</td></tr>
 * <tr><td>integer</td><td>display_source_type</td><td> The source type of display name, refer to the @ref contacts_display_name_source_type_e (projection) </td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td> Addressbook ID that the number belongs to </td></tr>
 * <tr><td>integer</td><td>person_id</td><td> Person ID that the number belongs to </td></tr>
 * <tr><td>string</td><td>ringtone_path</td><td> Ringtone path that the number belongs to (projection)  </td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td>  Image thumbnail path that the number belongs to (projection) </td></tr>
 * <tr><td>integer</td><td> number_id </td><td> DB record ID of the number </td></tr>
 * <tr><td>integer</td><td> type </td><td> Number type, refer to the @ref contacts_number_type_e (projection) </td></tr>
 * <tr><td>string</td><td> label </td><td> Custom number type label, when the number type is #CONTACTS_NUMBER_TYPE_CUSTOM (projection) </td></tr>
 * <tr><td>boolean</td><td> is_default </td><td> The number is default number or not </td></tr>
 * <tr><td>string</td><td> number </td><td> Number </td></tr>
 * <tr><td>string</td><td> number_filter </td><td> If you add filter with this property, the string will be normalized as minmatch length internally and the match rule will be applied CONTACTS_MATCH_EXACTLY </td></tr>
 * <tr><td>string</td><td> normalized_number </td><td>You can only use this property for search filter </td></tr>
 * <tr><td>string</td><td> cleaned_number </td><td>You can only use this property for search filter </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(contact_id)
	_CONTACTS_PROPERTY_STR(display_name)
	_CONTACTS_PROPERTY_PROJECTION_INT(display_source_type)
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_INT(person_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(ringtone_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(image_thumbnail_path)
	_CONTACTS_PROPERTY_INT(number_id)
	_CONTACTS_PROPERTY_PROJECTION_INT(type)
	_CONTACTS_PROPERTY_PROJECTION_STR(label)
	_CONTACTS_PROPERTY_BOOL(is_default)
	_CONTACTS_PROPERTY_STR(number)
	_CONTACTS_PROPERTY_FILTER_STR(number_filter)
	_CONTACTS_PROPERTY_FILTER_STR(normalized_number)
	_CONTACTS_PROPERTY_FILTER_STR(cleaned_number)
_CONTACTS_END_READ_ONLY_VIEW(_contacts_contact_number)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact_email _contacts_contact_email view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this contacts email view </td></tr>
 * <tr><td>integer</td><td>contact_id</td><td> Contact ID that the email belongs to </td></tr>
 * <tr><td>string</td><td>display_name</td><td> Display name that the email belongs to </td></tr>
 * <tr><td>integer</td><td>display_source_type</td><td> The source type of display name that the email belongs to (projection) </td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td> Addressbook ID that the email belongs to </td></tr>
 * <tr><td>integer</td><td>person_id</td><td> Person ID that the email belongs to </td></tr>
 * <tr><td>string</td><td>ringtone_path</td><td> Ringtone path that the email belongs to (projection) </td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td> Image thumbnail path that the email belongs to (projection) </td></tr>
 * <tr><td>integer</td><td> email_id </td><td> DB record ID of the email </td></tr>
 * <tr><td>integer</td><td> type </td><td>  Email type, refer to the @ref contacts_email_type_e (projection) </td></tr>
 * <tr><td>string</td><td> label </td><td> Custom mail type label, when the email type is #CONTACTS_EMAIL_TYPE_CUSTOM (projection) </td></tr>
 * <tr><td>boolean</td><td> is_default </td><td> Email is default email or not </td></tr>
 * <tr><td>string</td><td> email </td><td> Email address </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(contact_id)
	_CONTACTS_PROPERTY_STR(display_name)
	_CONTACTS_PROPERTY_PROJECTION_INT(display_source_type)
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_INT(person_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(ringtone_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(image_thumbnail_path)
	_CONTACTS_PROPERTY_INT(email_id)
	_CONTACTS_PROPERTY_PROJECTION_INT(type)
	_CONTACTS_PROPERTY_PROJECTION_STR(label)
	_CONTACTS_PROPERTY_BOOL(is_default)
	_CONTACTS_PROPERTY_STR(email)
_CONTACTS_END_READ_ONLY_VIEW(_contacts_contact_email)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact_grouprel _contacts_contact_grouprel view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this contact grouprel view </td></tr>
 * <tr><td>integer</td><td>contact_id</td><td> Contact ID that the contact group relation belongs to </td></tr>
 * <tr><td>string</td><td>display_name</td><td> Display name of the group relation </td></tr>
 * <tr><td>integer</td><td>display_source_type</td><td> The source type of display name (projection) </td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td> Addressbook ID that the group relation belongs to </td></tr>
 * <tr><td>integer</td><td>person_id</td><td> Person ID that the group relation belongs to </td></tr>
 * <tr><td>string</td><td>ringtone_path</td><td> Ringtone path of the group relation (projection) </td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td> Image thumbnail path of the group relation (projection) </td></tr>
 * <tr><td>integer</td><td> group_id </td><td> DB record ID of the group relation </td></tr>
 * <tr><td>string</td><td> group_name </td><td> Group name (projection) </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(contact_id)
	_CONTACTS_PROPERTY_STR(display_name)
	_CONTACTS_PROPERTY_PROJECTION_INT(display_source_type)
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_INT(person_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(ringtone_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(image_thumbnail_path)
	_CONTACTS_PROPERTY_INT(group_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(group_name)
_CONTACTS_END_READ_ONLY_VIEW(_contacts_contact_grouprel)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact_activity _contacts_contact_activity view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this contact activity view </td></tr>
 * <tr><td>integer</td><td>contact_id</td><td> Contact ID that the activity belongs to</td></tr>
 * <tr><td>string</td><td>display_name</td><td> Display name of the contact that the activity belongs to </td></tr>
 * <tr><td>integer</td><td>display_source_type</td><td> The source type of display name that the activity belongs to </td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td> Addressbook that the activity belongs to </td></tr>
 * <tr><td>integer</td><td>person_id</td><td> Person ID that the activity belongs to </td></tr>
 * <tr><td>string</td><td>ringtone_path</td><td> Ringtone path of the contact that the activity belongs to (projection) </td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td> Image thumbnail path of the contact that the activity belongs to (projection) </td></tr>
 * <tr><td>integer</td><td> activity_id </td><td> DB record ID of the activity </td></tr>
 * <tr><td>string</td><td> source_name </td><td> Account name that the activity belongs to </td></tr>
 * <tr><td>string</td><td> status </td><td> Activity status (projection) </td></tr>
 * <tr><td>integer</td><td> timestamp </td><td> Published time of activity </td></tr>
 * <tr><td>string</td><td> service_operation </td><td> Data for service_set_operation </td></tr>
 * <tr><td>string</td><td> uri </td><td> Data for service_set_uri </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT(contact_id)
	_CONTACTS_PROPERTY_STR(display_name)
	_CONTACTS_PROPERTY_PROJECTION_INT(display_source_type)
	_CONTACTS_PROPERTY_INT(address_book_id)
	_CONTACTS_PROPERTY_INT(account_id)
	_CONTACTS_PROPERTY_INT(person_id)
	_CONTACTS_PROPERTY_PROJECTION_STR(ringtone_path)
	_CONTACTS_PROPERTY_PROJECTION_STR(image_thumbnail_path)
	_CONTACTS_PROPERTY_INT(activity_id)
	_CONTACTS_PROPERTY_STR(source_name)
	_CONTACTS_PROPERTY_PROJECTION_STR(status)
	_CONTACTS_PROPERTY_INT(timestamp)
	_CONTACTS_PROPERTY_STR(service_operation)
	_CONTACTS_PROPERTY_STR(uri)
_CONTACTS_END_READ_ONLY_VIEW(_contacts_contact_activity)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log_stat _contacts_phone_log_stat view (read only)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td> Identifier of this log stat view </td></tr>
 * <tr><td>integer</td><td> log_count </td><td>Log count (projection) </td></tr>
 * <tr><td>integer</td><td> log_type </td><td> Log type, see the @ref contacts_phone_log_type_e </td></tr>
 * <tr><td>integer</td><td> sim_slot_no </td><td>It is related to the SIM slot number. sim_slot_no 0 means first SIM card, sim_slot_no 1 means second SIM. It is same with handle index of telephony handle list. Refer to the telephony_init() (Since 3.0)</td></tr>
 * </table>
 */
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_PROJECTION_INT(log_count)
	_CONTACTS_PROPERTY_INT(log_type)
	_CONTACTS_PROPERTY_INT(sim_slot_no)   /*(Since 3.0)*/
_CONTACTS_END_READ_ONLY_VIEW(_contacts_phone_log_stat)

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_sip _contacts_sip view (Since 3.0)
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> Identifier of this contacts sip view </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> DB record ID of the sip </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> Contact ID that the sip belongs to </td></tr>
 * <tr><td>string</td><td> address </td><td>read, write</td><td> SIP address </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> sip type, refer to the @ref contacts_sip_type_e </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> Custom sip type label, when the sip type is #CONTACTS_SIP_TYPE_CUSTOM </td></tr>
 * </table>
 */
_CONTACTS_BEGIN_VIEW()                        /* (Since 3.0) */
	_CONTACTS_PROPERTY_INT(id)                /* read only */
	_CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
	_CONTACTS_PROPERTY_STR(address)           /* read, write */
	_CONTACTS_PROPERTY_INT(type)              /* read, write */
	_CONTACTS_PROPERTY_STR(label)             /* read, write */
_CONTACTS_END_VIEW(_contacts_sip)

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CONTACTS_VIEWS_H__ */


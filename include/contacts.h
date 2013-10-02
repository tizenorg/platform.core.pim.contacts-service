/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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
#ifndef __TIZEN_SOCIAL_CONTACTS_H__
#define __TIZEN_SOCIAL_CONTACTS_H__

#include <contacts_errors.h>
#include <contacts_service.h>
#include <contacts_views.h>
#include <contacts_types.h>
#include <contacts_record.h>
#include <contacts_list.h>
#include <contacts_filter.h>
#include <contacts_query.h>
#include <contacts_db.h>
#include <contacts_setting.h>
#include <contacts_person.h>
#include <contacts_group.h>
#include <contacts_sim.h>
#include <contacts_vcard.h>
#include <contacts_activity.h>
#include <contacts_utils.h>
#include <contacts_phone_log.h>

#endif //__TIZEN_SOCIAL_CONTACTS_H__

/**
 * @ingroup CAPI_SOCIAL_FRAMEWORK
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_MODULE Contacts(New)
 *
 * @brief The Contacts Service API provides functions for managing phone contacts (a.k.a. phonebook).
 * This API allows you not only to store information about contacts but also to query contact information.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_MODULE_OVERVIEW Overview
 *
 * The basic concept in Contacts APi is a record. It may be helpful to know that a record represents
 * an actual record in the internal database, but in general, you can think of a record as a piece
 * of information, like an address, phone number or group of contacts. A record can be a complex
 * set of data, containing other data, e.g. an address record contains country, region, and street,
 * among others. Also, the contained data can be a reference to another record. For example,
 * a contact record contains the 'address' property, which is a reference to an address record. An address
 * record belongs to a contact record - its 'contact_id' property is set to identifier of the corresponding contact
 * (more on ids later). In this case, the address is the contact's child record and the contact is the parent record.
 *
 * Effectively, a record can be a node in a tree or graph of relations between records.
 *
 * Each record type has a special structure defined for it, called 'view', which contains identifiers of its properties.
 * For example, the _contacts_contact view describes the properties of the contact record.
 * Its properties include name, company, nickname of the contact and many more. The address record is described by
 * the _contacts_address view. A special property in such structures is the URI, which is used to identify
 * the record's type. Every view describing a record has this property. You can think of views
 * as of classes in an object oriented language. Whenever you use a record, you refer to its view to know
 * the record's properties.
 *
 * To operate on a record, you must obtain its handle. The handle is provided during creation of the record.
 * It can also be obtained when referring to a child record of a record the handle of which you already have.
 *
 * When creating a record, you need to specify what type of record you want to create. This is where you should
 * use the URI property. The code below creates a contact record and obtains its handle.
 *
 * @code
 * contacts_record_h hcontact = NULL;
 * contacts_record_create( _contacts_contact._uri, &hcontact );
 * @endcode
 *
 * A record can have basic properties of four types: integer, string, boolean, long integer, double. Each property
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
 *    <td> contacts_record_set_str </td>
 *    <td> contacts_record_get_str </td>
 * </tr>
 * <tr>
 *    <td> integer </td>
 *    <td> contacts_record_set_int </td>
 *    <td> contacts_record_get_int </td>
 * </tr>
 * <tr>
 *    <td> boolean </td>
 *    <td> contacts_record_set_bool </td>
 *    <td> contacts_record_get_bool </td>
 * </tr>
 * <tr>
 *    <td> long integer </td>
 *    <td> contacts_record_set_lli </td>
 *    <td> contacts_record_get_lli </td>
 * </tr>
 * <tr>
 *    <td> long integer </td>
 *    <td> contacts_record_set_double </td>
 *    <td> contacts_record_get_double </td>
 * </tr>
 * </table>
 *
 * For long integer functions, "lli" stands for long long int, ususally used to hold UTC time.
 *
 * These functions also require specifying which property you wish to get/set. The code below sets the "ringtone_path"
 * property of a contact record.
 *
 * @code
 * contacts_record_set_str( hcontact, _contacts_contact.ringtone_path , "My Documents/1.mp3" );
 * @endcode
 *
 * Note on returned values ownership: some functions have the "_p" postfix. It means that the returned
 * value should not be freed by the application, as it is a pointer to data in an existing record. For example:
 *
 * @code
 * API int contacts_record_get_str( contacts_record_h record, unsigned int property_id, char** out_str );
 * API int contacts_record_get_str_p( contacts_record_h record, unsigned int property_id, char** out_str );
 * @endcode
 *
 * In the first case, the returned string should be freed by the application, in the second one it should not.
 *
 *
 * A record can also have properties of type 'record' - other records, called child records.
 * A record can contain many child records of a given type. For example, a contact record
 * can contain many address records, and is called the parent record of the address records.
 * The code below inserts an address record into a contact record.
 * Note that it is not necessary to insert all records - just the contact record needs to be
 * inserted into the database, it is enough for all information to be stored.
 * Both records are then destroyed.
 *
 * @code
 * contacts_record_h haddress = NULL;
 * contacts_record_h himage = NULL;
 * contacts_record_create( _contacts_contact._uri, &hcontact );
 * contacts_record_create( _contacts_image._uri, &himage );
 * contacts_record_create( _contacts_address._uri, &haddress );
 * contacts_record_set_str( himage, _contacts_image.path, "My Documents/1.jpg" );
 * contacts_record_set_str( hcontact, _contacts_address.country, "Korea" );
 * contacts_record_add_child_record( hcontact, _contacts_contact.image, himage );
 * contacts_record_add_child_record( hcontact, _contacts_contact.address, haddress );
 *
 * contacts_db_insert_record( hcontact );
 * contacts_record_destroy( hcontact, true );
 * @endcode
 *
 * Establishing parent/child relation between records is also possible
 * through the use of identifiers, described in following paragraphs.
 *
 * One of record's properties is the identifier (id). If you know the id of a record existing in the database,
 * you can directly access it. The id is available after the record has been inserted into the database
 * and is a read-only property.
 *
 * @code
 * contacts_record_h haddress = NULL;
 * contacts_record_create( _contacts_contact._uri, &hcontact );
 * contacts_db_insert_record( hcontact );
 * int id = contacts_record_get_int( hcontact, _contacts_contact.id );
 * // use hcontact ...
 * // ...
 * contacts_record_destroy( hcontact, true); // hcontact is no longer usable
 *
 * contacts_db_get_record( _contacts_contact._uri, id, &hcontact );
 * // hcontact is now a handle to the same record as before
 * @endcode
 *
 * Identifiers can also be used to establish a relation between two records.
 * The following code sets an address record's 'contact_id' property to the id of the contact.
 * contact_id acts as a foreign key here. After that is done, the address becomes one of the addresses
 * connected to the contact. The address is now the contact's child record, the contact is the
 * the parent record.
 *
 * @code
 * contacts_record_create( _contacts_contact._uri, &hcontact );
 * int id = ... // acquire id of created contact
 *
 * contacts_record_create( _contacts_address._uri, &haddress );
 * contacts_record_set_int( haddress, _contacts_address.contact_id, id );
 * // set other address properties
 * contacts_db_insert_record( haddress );
 * @endcode
 *
 * As mentioned above, a record can have many child records, just as more than one record in a database can have
 * its foreign keys set to the id of another record.
 * Having a record handle, you can access all records of a specific type related to the given record:
 *
 * @code
 * contacts_db_get_record( _contacts_contact._uri, id, &hcontact );
 * int address_num = contacts_record_get_child_record_count( hcontact, _contacts_contact.address );
 * for( int i=0; i < address_num; i++ )
 * {
 *     haddress = contacts_record_get_child_record_at( hcontact, _contacts_contact.address, i );
 *     contacts_record_set_str( haddress, _contacts_address.country, "Korea" );
 * }
 * contacts_db_update_record( hcontact );
 *
 * contacts_record_destroy( hcontact, true );
 * @endcode
 *
 * This code acquires the number of child records of type 'address' and iterates over them. Each address
 * has its 'country' property set to 'Korea'. Only the parent record is saved to the database - all changes
 * made to the child records are saved automatically.
 *
 *
 * Another two important concepts in Contacts API are filters and queries, related to searching.
 * Filters allow returning results that satisfy a given condition, e.g. an integer property
 * value must be greater than a given value, or a string must contain a given substring. Many
 * conditions can be added to a filter, creating a composite filter with the use of AND and OR logical operators.
 *
 * A query acts as a container for filters and as an interface to the stored data. It allows configuring
 * sorting and grouping methods of returned results.
 *
 * Sample code: Create a filter which will accept addresses with their contact's id equal to a given id
 * (integer filter), or their country property equal to "Korea" (string filter). Create a query and
 * add the filter to it. Results are received in a list.
 *
 * @code
 * contacts_filter_h filter = NULL;
 * contacts_list_h list = NULL;
 * contacts_query_h query = NULL;
 *
 * contacts_filter_create( _contacts_address._uri, &filter );
 * contacts_filter_add_int( filter, _contacts_address.contact_id, CONTACTS_MATCH_EQUAL, id );
 * contacts_filter_add_operator( filter, CONTACTS_FILTER_OPERATOR_OR);
 * contacts_filter_add_str( filter, _contacts_address.country, CONTACTS_MATCH_EXACTLY, "Korea" );
 * contacts_query_create( _contacts_address._uri, &query );
 * contacts_query_set_filter(query, filter);
 *
 * contacts_db_get_records_with_query( query, 0, 0, &list );
 *
 * contacts_filter_destroy( filter );
 * contacts_query_destroy( query);
 * // use the list
 * // ...
 * contacts_list_destroy( list, true );
 * @endcode
 *
 * A query can contain more than one fiter. You can add logical operators between filters. Whole filters
 * are operators' arguments, not just the conditions. Extending the example above, the following code creates two filters
 * and connects them with the OR operator:
 *
 * @code
 * contacts_filter_h filter1 = NULL;
 * contacts_filter_h filter2 = NULL;
 * contacts_query_h query = NULL;
 *
 * contacts_filter_create( _contacts_address._uri, &filter1 );
 * contacts_filter_add_int( filter1, _contacts_address.contact_id, CONTACTS_MATCH_EQUAL, id );
 * contacts_filter_add_operator( filter1, CONTACTS_FILTER_OPERATOR_OR);
 * contacts_filter_add_str( filter1, _contacts_address.country, CONTACTS_MATCH_EXACTLY, "Korea" );
 *
 * contacts_filter_create( _contacts_address._uri, &filter2 );
 * contacts_filter_add_str( filter2, _contacts_address.country, CONTACTS_MATCH_EXACTLY, "USA" );
 * contacts_filter_add_operator( filter2, CONTACTS_FILTER_OPERATOR_AND);
 * contacts_filter_add_str( filter2, _contacts_address.region, CONTACTS_MATCH_CONTAINS, "California" );
 *
 * contacts_filter_add_operator(filter1, CONTACTS_FILTER_OPERATOR_OR);
 * contacts_filter_add_filter(filter1, filter2);
 *
 * contacts_query_create( _contacts_address._uri, &query );
 * contacts_query_set_filter(query, filter1);
 *
 * contacts_db_get_records_with_query( query, 0, 0, &list );
 *
 * contacts_filter_destroy( filter1 );
 * contacts_filter_destroy( filter2 );
 * contacts_query_destroy( query );
 * // ...
 * @endcode
 *
 * The first filter accepts addresses with country equal to "Korea" or contact id equal to 'id' variable.
 * The second filter accepts addresses with "USA" as country and region containing the string "California".
 * The query in the code above will return addresses accepted by either of the filters.
 *
 * Furthermore, the order in which filters and operators are added determines the placement of parentheses.
 * For example, If the following are added, in given order:
 *
 * @code
 * Filter F1
 * OR
 * Filter F2
 * AND
 * Filter F3
 * @endcode
 *
 * the result is:
 *
 * @code
 * (F1 OR F2) AND F3
 * @endcode
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_View_properties View properties
 * In \ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE category, you can find tables with view properties. Record types which have *_id
 * as their properties, hold identifiers of other records - e.g. name, number and email
 * views hold id of their corresponding contacts in contact_id property
 * (as children of the corresponding contacts record).
 *
 * Properties of type 'record' are other records. For example, a contact record has 'name',
 * 'number' and 'email' properties, which means that records of those types can be children
 * of contact type records.
 *
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_DATABASE_MODULE Database
 *
 * @brief The contacts database API provides the set of the definitions and interfaces that enable you to handle contacts database.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_DATABASE_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_RECORD_MODULE Record
 *
 * @brief The contacts record API provides the set of the definitions and interfaces that enable you to get/set data from/to contacts record handle.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_RECORD_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_LIST_MODULE List
 *
 * @brief The contacts record API provides the set of the definitions and interfaces that enable you to get/set records list data.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_LIST_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_PERSON_MODULE Person
 *
 * @brief The contacts person API provides the set of the definitions and interfaces that enable you to link/unlink person and contact.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_PERSON_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_GROUP_MODULE Group
 *
 * @brief The contacts group API provides the set of the definitions and interfaces that enable you to add/remove contact as group member.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_GROUP_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */


/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_FILTER_MODULE Filter
 *
 * @brief The contacts Filter API provides the set of the definitions and interfaces that enable you to make filters to set query.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_CONTACTS_FILTER_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_QUERY_MODULE Query
 *
 * @brief The contacts Query API provides the set of the definitions and interfaces that enable you to make query to get list.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_CONTACTS_QUERY_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_VCARD_MODULE vCard
 *
 * @brief The contacts record API provides the set of the definitions and interfaces that enable you to get/set data from/to vCard.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VCARD_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_ACTIVITY_MODULE Activity
 *
 * @brief The contacts activity API provides the set of the definitions and interfaces that enable you to delete activities by contact_id and account_id. \n
 * Please refer to \ref CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity for more details.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_ACTIVITY_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_PHONELOG_MODULE Phone log
 *
 * @brief The contacts phone log API provides the set of the definitions and interfaces that enable you to reset phone log count.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_PHONELOG_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_SETTING_MODULE Setting
 *
 * @brief The contacts setting API provides the set of the definitions and interfaces that enable you to set up contacts features.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_SETTING_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_SIM_MODULE SIM
 *
 * @brief The contacts SIM API provides the set of the definitions and interfaces that enable you to get/set data from/to SIM card.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_CONTACTS_SIM_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_UTILS_MODULE Utils
 *
 * @brief The contacts Utils API provides the set of the definitions and interfaces that enable you to get index characters of language according to phone setting
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_CONTACTS_UTILS_HEADER Required Header
 *  \#include <contacts.h>
 *
 * <BR>
 */

/**
 * @ingroup CAPI_SOCIAL_CONTACTS_SVC_MODULE
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE View/Property
 *
 * @brief This page provides information about views with properties.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_OVERVIEW Overview
 * A view is a structure which describes properties of a record.
 * A record can have basic properties of four types: integer, string, boolean, long integer, double. Each property
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
 *    <td> contacts_record_set_str </td>
 *    <td> contacts_record_get_str </td>
 * </tr>
 * <tr>
 *    <td> integer </td>
 *    <td> contacts_record_set_int </td>
 *    <td> contacts_record_get_int </td>
 * </tr>
 * <tr>
 *    <td> boolean </td>
 *    <td> contacts_record_set_bool </td>
 *    <td> contacts_record_get_bool </td>
 * </tr>
 * <tr>
 *    <td> long integer </td>
 *    <td> contacts_record_set_lli </td>
 *    <td> contacts_record_get_lli </td>
 * </tr>
 * <tr>
 *    <td> long integer </td>
 *    <td> contacts_record_set_double </td>
 *    <td> contacts_record_get_double </td>
 * </tr>
 * </table>
 *
 * For long integer functions, "lli" stands for long long int, ususally used to hold UTC time.
 *
 * Below you can find tables with view properties.
 *
 * Properties of type 'record' are other records. For example, the _contacts_contact view
 * has a 'name' property of type 'record'. This means that records of type name (_contacts_name view)
 * can be children of the contact record. If a name record holds the identifier
 * of a contact record in its 'contact_id' property, it is the child record of the corresponding
 * contact record.
 *
 * Records can have many children of a given type.
 *
 * Please refer to the main section of Contacts API for a more detailed explanation and examples.
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact _contacts_contact view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> View uri for contact </td></tr>
 * <tr><td>integer</td><td>id</td><td>read only</td><td></td></tr>
 * <tr><td>string</td><td>display_name</td><td>read only</td><td></td></tr>
 * <tr><td>integer</td><td>display_source_id</td><td>read only</td><td>The source type of display name contacts_display_name_source_type_e</td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td>read, write once</td><td></td></tr>
 * <tr><td>string</td><td>ringtone_path</td><td>read, write</td><td></td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td>read only</td><td></td></tr>
 * <tr><td>boolean</td><td>is_favorite</td><td>read, write</td><td></td></tr>
 * <tr><td>boolean</td><td>has_phonenumber</td><td>read only</td><td></td></tr>
 * <tr><td>boolean</td><td>has_email</td><td>read only</td><td></td></tr>
 * <tr><td>integer</td><td>person_id</td><td>read only</td><td></td></tr>
 * <tr><td>string</td><td>uid</td><td>read, write</td><td></td></tr>
 * <tr><td>string</td><td>vibration</td><td>read, write</td><td></td></tr>
 * <tr><td>string</td><td>message_alert</td><td>read, write</td><td></td></tr>
 * <tr><td>integer</td><td>changed_time</td><td>read only</td><td></td></tr>
 * <tr><td>integer</td><td>link_mode</td><td>read, write</td><td> contacts_contact_link_mode_e </td></tr>
 * <tr><td>record</td><td>name</td><td>read, write</td><td> single </td></tr>
 * <tr><td>record</td><td>company</td><td>read, write</td><td>multiple </td></tr>
 * <tr><td>record</td><td>note</td><td>read, write</td><td>multiple</td></tr>
 * <tr><td>record</td><td>number</td><td>read, write</td><td>multiple</td></tr>
 * <tr><td>record</td><td>email</td><td>read, write</td><td>multiple</td></tr>
 * <tr><td>record</td><td>event</td><td>read, write</td><td>multiple</td></tr>
 * <tr><td>record</td><td>messenger</td><td>read, write</td><td>multiple</td></tr>
 * <tr><td>record</td><td>address</td><td>read, write</td><td>multiple</td></tr>
 * <tr><td>record</td><td>url</td><td>read, write</td><td>multiple</td></tr>
 * <tr><td>record</td><td>nickname</td><td>read, write</td><td>multiple</td></tr>
 * <tr><td>record</td><td>profile</td><td>read, write</td><td>	multiple</td></tr>
 * <tr><td>record</td><td>relationship</td><td>read, write</td><td>multiple</td></tr>
 * <tr><td>record</td><td>image</td><td>read, write</td><td>multiple</td></tr>
 * <tr><td>record</td><td>group_relation</td><td>read, write</td><td>multiple</td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile _contacts_my_profile view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> View uri for my profile </td></tr>
 * <tr><td>integer</td><td>id</td><td>read only</td><td></td></tr>
 * <tr><td>string</td><td>display_name</td><td>read only</td><td></td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td>read, write once</td><td></td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td>read only</td><td></td></tr>
 * <tr><td>string</td><td>uid</td><td>read, write</td><td></td></tr>
 * <tr><td>integer</td><td>changed_time</td><td>read only</td><td></td></tr>
 * <tr><td>record</td><td>name</td><td>read, write</td><td></td></tr>
 * <tr><td>record</td><td>company</td><td>read, write</td><td></td></tr>
 * <tr><td>record</td><td>note</td><td>read, write</td><td></td></tr>
 * <tr><td>record</td><td>number</td><td>read, write</td><td></td></tr>
 * <tr><td>record</td><td>email</td><td>read, write</td><td></td></tr>
 * <tr><td>record</td><td>event</td><td>read, write</td><td></td></tr>
 * <tr><td>record</td><td>messenger</td><td>read, write</td><td></td></tr>
 * <tr><td>record</td><td>address</td><td>read, write</td><td></td></tr>
 * <tr><td>record</td><td>url</td><td>read, write</td><td></td></tr>
 * <tr><td>record</td><td>nickname</td><td>read, write</td><td></td></tr>
 * <tr><td>record</td><td>profile</td><td>read, write</td><td></td></tr>
 * <tr><td>record</td><td>relationship</td><td>read, write</td><td></td></tr>
 * <tr><td>record</td><td>image</td><td>read, write</td><td></td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_simple_contact _contacts_simple_contact view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td>id</td><td>read only</td><td> </td></tr>
 * <tr><td>string</td><td>display_name</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td>display_source_id</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td>read, write once</td><td> </td></tr>
 * <tr><td>string</td><td>ringtone_path</td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td>read only</td><td> </td></tr>
 * <tr><td>boolean</td><td>is_favorite</td><td>read, write</td><td> </td></tr>
 * <tr><td>boolean</td><td>has_phonenumber</td><td>read only</td><td> </td></tr>
 * <tr><td>boolean</td><td>has_email</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td>person_id</td><td>read only</td><td> </td></tr>
 * <tr><td>string</td><td>uid</td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td>vibration</td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td>message_alert</td><td>read, write</td><td></td></tr>
 * <tr><td>integer</td><td>changed_time</td><td>read only</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person _contacts_person view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>string</td><td> display_name </td><td>read only</td><td> </td></tr>
 * <tr><td>string</td><td> display_name_index </td><td>read only</td><td> The first character of first string for grouping. This is normalized using icu. </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td>read only</td><td> </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td>read only</td><td> </td></tr>
 * <tr><td>string</td><td> vibration </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td>message_alert</td><td>read, write</td><td></td></tr>
 * <tr><td>string</td><td> status </td><td>read only</td><td> </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td>read, write</td><td> </td></tr>
 * <tr><td>double</td><td> favorite_priority </td><td> filter only </td><td> The priority of favorite contacts. You can not set the value but you can use it as sorting key. </td></tr>
 * <tr><td>integer</td><td> link_count </td><td>read, write</td><td> </td></tr>
 * <tr><td>integer</td><td> addressbook_ids </td><td>read, write</td><td> </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td>read only</td><td> </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td>read only</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address_book _contacts_address_book view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> account_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>string</td><td> name </td><td>read, write</td><td> It can not be NULL. Duplicate names are not allowed. </td></tr>
 * <tr><td>integer</td><td> mode </td><td>read, write</td><td> contacts_address_book_mode_e </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group _contacts_group view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>string</td><td> name </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> image_path </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> vibration </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> extra_data </td><td>read, write</td><td> </td></tr>
 * <tr><td>boolean</td><td> is_read_only </td><td>read, write once</td><td> </td></tr>
 * <tr><td>string</td><td> message_alert </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_name _contacts_name view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 *</tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>string</td><td> first </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> last </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> addition </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> suffix </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> prefix </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> phonetic_first </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> phonetic_middle </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> phonetic_last </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_number _contacts_number view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> </td></tr>
 * <tr><td>boolean</td><td> is_default </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> number </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> normalized_number </td><td> filter only </td><td> You can only use this property for search filter. </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_email _contacts_email view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> </td></tr>
 * <tr><td>boolean</td><td> is_default </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> email </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_address _contacts_address view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> postbox </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> postal_code </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> region </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> locality </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> street </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> country </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> extend </td><td>read, write</td><td> </td></tr>
 * <tr><td>boolean</td><td> is_default </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_note _contacts_note view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>string</td><td> note </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_url _contacts_url view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> url </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_event _contacts_event view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> </td></tr>
 * <tr><td>integer</td><td> date </td><td>read, write</td><td> year * 10000 + month * 100 + day </td></tr>
 * <tr><td>integer</td><td> is_lunar </td><td>read, write</td><td> </td></tr>
 * <tr><td>integer</td><td> lunar_date </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_relationship _contacts_relationship view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> name </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_image _contacts_image view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> path </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_company _contacts_company view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> name </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> department </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> job_title </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> assistant_name </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> role </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> logo </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> location </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> description </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> phonetic_name </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_nickname _contacts_nickname view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>string</td><td> nickname </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_messenger _contacts_messenger view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> im_id </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_extension _contacts_extension view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>integer</td><td> data1 </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> data2 </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> data3 </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> data4 </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> data5 </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> data6 </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> data7 </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> data8 </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> data9 </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> data10 </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> data11 </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> data12 </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_sdn _contacts_sdn view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>string</td><td> name </td><td>read, write once</td><td> </td></tr>
 * <tr><td>string</td><td> number </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_profile _contacts_profile view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>string</td><td> uid </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> text </td><td>read, write</td><td> </td></tr>
 * <tr><td>integer</td><td> order </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> service_operation </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> mime </td><td>read, write</td><td>  </td> </tr>
 * <tr><td>string</td><td> app_id </td><td>read, write</td><td> </td>  </tr>
 * <tr><td>string</td><td> uri </td><td>read, write</td><td> </td ></tr>
 * <tr><td>string</td><td> catagory </td><td>read, write</td><td></td></tr>
 * <tr><td>string</td><td> extra_data </td><td>read, write</td><td> It includes "key:value,key:value," pairs. You should parse it. And you must base64 encode each key and value</td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity _contacts_activity view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>string</td><td> source_name </td><td>read, write</td><td> </td></tr>
 * <tr><td>int</td><td> timestamp </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> status </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> service_operation </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> uri </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> photo </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_activity_photo _contacts_activity_photo view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> activity_id </td><td>read, write once</td><td> </td></tr>
 * <tr><td>string</td><td> photo_url </td><td>read, write</td><td> </td></tr>
 * <tr><td>integer</td><td> sort_index </td><td>read, write</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_speeddial _contacts_speeddial view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> speeddial_number </td><td>read, write</td><td> </td></tr>
 * <tr><td>integer</td><td> number_id </td><td>read, write</td><td> </td></tr>
 * <tr><td>string</td><td> number </td><td>read only</td><td> </td></tr>
 * <tr><td>string</td><td> number_label </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> number_type </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> person_id </td><td>read only</td><td> </td></tr>
 * <tr><td>string</td><td> display_name </td><td>read only</td><td> </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td>read only</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log _contacts_phone_log view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Read, Write</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> id </td><td>read only</td><td> </td></tr>
 * <tr><td>integer</td><td> person_id </td><td>read, write once </td><td> </td></tr>
 * <tr><td>string</td><td> address </td><td>read, write once </td><td> </td></tr>
 * <tr><td>integer</td><td> log_time </td><td>read, write once</td><td> </td></tr>
 * <tr><td>integer</td><td> log_type </td><td>read, write</td><td>contacts_phone_log_type_e </td></tr>
 * <tr><td>integer</td><td> extra_data1 </td><td>read, write once</td><td> You can set the related integer data (e.g. message_id, email_id or duration of call).  </td></tr>
 * <tr><td>string</td><td> extra_data2 </td><td>read, write once</td><td> You can set the related string data (e.g. short message, subject). </td></tr>
 * <tr><td>string</td><td> normalized_address </td><td> filter only</td><td> You can only use this property for search filter.</td></tr>
 * </table>
 *
* <br><br>
* Read-only View URIs
* <br>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact_updated_info _contacts_contact_updated_info view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primay Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> contact_id </td><td>*</td><td> </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> version </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> image_changed </td><td></td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_my_profile_updated_info _contacts_my_profile_updated_info view
 * <table>
 * <tr>
 *	  <th>Type</th>
 *	  <th>Property ID</th>
 *	  <th>Primay Key</th>
 *	  <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> last_changed_type </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> version </td><td></td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group_updated_info _contacts_group_updated_info view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primary Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> group_id </td><td>*</td><td> </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> version </td><td></td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_group_member_updated_info _contacts_group_member_updated_info view
 * <table>
 * <tr>
 *	  <th>Type</th>
 *	  <th>Property ID</th>
 *	  <th>Primary Key</th>
 *	  <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> group_id </td><td>*</td><td> </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> version </td><td></td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_number _contacts_person_number view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primary Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> person_id </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> display_name </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> display_name_index </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> vibration </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> message_alert </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> number_id </td><td>*</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> is_primary_default </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> number </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> number_filter </td><td></td><td> If you add filter with this property, the string will be normalized as minmatch length internally and the match rule will be applied CONTACTS_MATCH_EXACTLY </td></tr>
 * <tr><td>string</td><td> normalized_number </td><td></td><td> You can only use this property for search filter</td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_email _contacts_person_email view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primary Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> person_id </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> display_name </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> display_name_index </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> vibration </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> message_alert </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> email_id </td><td>*</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> is_primary_default </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> email </td><td></td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_grouprel _contacts_person_grouprel view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primary Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> person_id </td><td>*</td><td> </td></tr>
 * <tr><td>string</td><td> display_name </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> display_name_index </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> vibration </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> message_alert </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> status </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> link_count </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> addressbook_ids </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> address_book_id </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> group_id </td><td>*</td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_phone_log _contacts_person_phone_log view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primary Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> person_id </td><td> * </td><td> </td></tr>
 * <tr><td>string</td><td> display_name </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> log_id </td><td> * </td><td> </td></tr>
 * <tr><td>string</td><td> address </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> log_time </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> log_type </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> extra_data1 </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> extra_data2 </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> normalized_address </td><td></td><td>You can only use this property for search filter </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_person_usage _contacts_person_usage view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primary Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> person_id </td><td>*</td><td> </td></tr>
 * <tr><td>string</td><td> display_name </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> display_name_index </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> display_contact_id </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> ringtone_path </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> image_thumbnail_path </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> vibration </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> message_alert </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> is_favorite </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> has_phonenumber </td><td></td>><td> </td></tr>
 * <tr><td>boolean</td><td> has_email </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> usage_type </td><td>*</td><td>contacts_usage_type_e </td></tr>
 * <tr><td>integer</td><td> times_used </td><td></td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact_number _contacts_contact_number view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primary Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>contact_id</td><td></td><td> </td></tr>
 * <tr><td>string</td><td>display_name</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>display_source_type</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>person_id</td><td></td><td> </td></tr>
 * <tr><td>string</td><td>ringtone_path</td><td></td><td> </td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> number_id </td><td>*</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> is_ default </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> number </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> number_filter </td><td></td><td> If you add filter with this property, the string will be normalized as minmatch length internally and the match rule will be applied CONTACTS_MATCH_EXACTLY </td></tr>
 * <tr><td>string</td><td> normalized_number </td><td></td><td>You can only use this property for search filter </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact_email _contacts_contact_email view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primary Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>contact_id</td><td></td><td> </td></tr>
 * <tr><td>string</td><td>display_name</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>display_source_type</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>person_id</td><td></td><td> </td></tr>
 * <tr><td>string</td><td>ringtone_path</td><td></td><td> </td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> email_id </td><td>*</td><td> </td></tr>
 * <tr><td>integer</td><td> type </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> label </td><td></td><td> </td></tr>
 * <tr><td>boolean</td><td> is_ default </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> email </td><td></td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact_grouprel _contacts_contact_grouprel view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primary Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>contact_id</td><td>*</td><td> </td></tr>
 * <tr><td>string</td><td>display_name</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>display_source_type</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>person_id</td><td></td><td> </td></tr>
 * <tr><td>string</td><td>ringtone_path</td><td></td><td> </td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> group_id </td><td>*</td><td> </td></tr>
 * <tr><td>string</td><td> group_name </td><td></td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_contact_activity _contacts_contact_activity view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primary Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>contact_id</td><td></td><td> </td></tr>
 * <tr><td>string</td><td>display_name</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>display_source_type</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>address_book_id</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td>person_id</td><td></td><td> </td></tr>
 * <tr><td>string</td><td>ringtone_path</td><td></td><td> </td></tr>
 * <tr><td>string</td><td>image_thumbnail_path</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> activity_id </td><td>*</td><td> </td></tr>
 * <tr><td>string</td><td> source_name </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> status </td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> timestamp </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> service_operation </td><td></td><td> </td></tr>
 * <tr><td>string</td><td> uri </td><td></td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log_number _contacts_phone_log_number view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primary Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>string</td><td> number </td><td> * </td><td> </td></tr>
 * </table>
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_VIEW_MODULE_contacts_phone_log_stat _contacts_phone_log_stat view
 * <table>
 * <tr>
 *    <th>Type</th>
 *    <th>Property ID</th>
 *    <th>Primary Key</th>
 *    <th>Description</th>
 * </tr>
 * <tr><td>string</td><td>_uri</td><td></td><td> </td></tr>
 * <tr><td>integer</td><td> log_count </td><td> </td><td> </td></tr>
 * <tr><td>integer</td><td> log_type </td><td> * </td><td> </td></tr>
 * </table>
 */


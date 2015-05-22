#ifndef __TIZEN_SOCIAL_CONTACTS_DOC_H__
#define __TIZEN_SOCIAL_CONTACTS_DOC_H__


/**
 * @ingroup CAPI_SOCIAL_FRAMEWORK
 * @defgroup CAPI_SOCIAL_CONTACTS_SVC_MODULE Contacts
 *
 * @brief The Contacts Service API provides methods for managing contact information for people.
 * A contact object is always associated with a specific address book.
 * A person object is an aggregation of one or more contacts associated with the same person.
 *
 *
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_MODULE_HEADER Required Header
 *  \#include <contacts.h>
 *
 *
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_MODULE_OVERVIEW Overview
 * The Contacts-service provides functions for managing contact information for people.
 * A contact object is always associated with a specific address book.
 * A person object is an aggregation of one or more contacts associated with the same person.
 *
 * Contacts-service has a relationship between Entities.
 * @image html Contact_structure.png "Figure: Contact structure"
 *
 * There are three same contacts made from different address book.
 * Person1 is an aggregation of Contact1, Contact2 and Contact3.
 *
 * The contacts-service provides an interface to manage the information
 * <table>
 * <tr>
 * <td>
 *	- Managing contacts information stored in database <br>
 *	- Aggregating contacts information from various accounts <br>
 *	- Notifying changes of contacts information <br>
 *	- Searching contacts information <br>
 *	- vCard supports <br>
 * </td>
 * </tr>
 * </table>
 *
 *
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_MODULE_ENTITIES Entities
 * Contacts-Service manages information related to following entities.
 * - Contact
 * 	- Information for individuals, like name, phone number, email, address, job, instant messenger, company, etc.
 * - Account
 * 	- Accounts are handled by account module. Contacts-service should use account ID which is created the module
 * 	- Exceptionally, Local device address book has no account and its related account ID is zero.
 * 	- Each account can create only one address book.
 * - Address book
 * 	- Represents where contacts and groups should belong to
 * 	- Created by one of contacts sources below
 * 		- Local device, which has no account
 * 		- Service providers such as Google or Yahoo, with account
 * 		- Applications like ChatON, Joyn, Facebook, etc.
 * - Group
 * 	- Grouped contacts on a same address book
 * 	- Groups and contacts have many-to-many relationship
 * - Person
 * 	- A virtual contact that keeps merged information of contacts linked together
 * 	- When more than one contact from different sources designate same individual, then instead of showing each contacts separately, by Person concept, they can be shown and managed as one virtual contact.
 * 	- Every contact becomes to be linked to at least one person.
 * - My profile
 * 	- My information which has almost same properties as contact information but it has no properties such as group relation, ringtone and message alert.
 * 	- Single entry can be available on each address book
 * - Activity
 * 	- Social activities are stored.
 * - Speed Dial
 *		- Shortcut dialing number key information
 * - Phone Log
 * 	- Call or message logs are stored
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_ENTITIES_RELATIONSHIP Relationship between entities
 * Contacts-service has a relationship between Entities.
 * @image html Relationship_between_entities.png "Figure: Relationship between entities"
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_RELATIONSHIP_BETWEEN_CONTACT_AND_PERSON Relationship between Contact and Person
 * Person will be created automatically when inserting a contact record. Cannot create person record directly.
 *
 * Sample code: Insert a contact record
 *
 * @code
 * // create contact record
 * contacts_record_h contact = NULL;
 * contacts_record_create(_contacts_contact._uri, &contact);
 *
 * // add name
 * contacts_record_h name = NULL;
 * contacts_record_create(_contacts_name._uri, &name);
 * contacts_record_set_str(name, _contacts_name.first, “test”);
 * contacts_record_add_child_record(contact, _contacts_contact.name, name);
 *
 * // add number
 * contacts_record_h number = NULL;
 * contacts_record_create(_contacts_number._uri, &number);
 * contacts_record_set_str(name, _contacts_number.number, “1234”);
 * contacts_record_add_child_record(contact, _contacts_contact.number, number);
 *
 * // insert to database
 * int contact_id = 0;
 * contacts_db_insert_record(contact, &contact_id);
 *
 * // destroy record
 * contacts_record_destroy(contact, true);
 * @endcode
 *
 * @image html Creating_a_person_record.png "Figure: Creating a person record"
 *
 * Person record can be link to another person. Even though contacts address book is different, link is possible.
 *
 * Sample code: Link contact
 * @code
 * int person_id1 = ... // acquire ID of Person1 record
 * int person_id2 = ... // acquire ID of Person2 record
 *
 * contacts_person_link_person(person_id1, person_id2);
 * @endcode
 *
 * @image html Link_person.png "Figure: Link person"
 *
 * Contact record can be separate from person record. New person will be created when unlinking contact record.
 *
 * Sample code: Unlink contact
 * @code
 * int person_id1 = ... // acquire ID of Person1 record
 * int contact_id3 = ... // acquire ID of Contact3 record
 *
 * contacts_person_unlink_contact(person_id1, contact_id3);
 * @endcode
 *
 * @image html Unlink_contact.png "Figure: Unlink contact"
 *
 *
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_MODULE_VIEWS Views
 * To access and handle entities, views are provided.
 * According to data-view declarations, generic access functions are used (contacts_db_insert_record(), contacts_record_get_int(), …).
 * Data-View is almost same as database “VIEW” which limits access and guarantees the performance by offering various views for the proper purpose.
 * “Record” represents a single row of the data-views. <br>
 * A data-view is a structure, which has property elements.
 * For example, the _contacts_contact view describes the properties of the contact record.
 * Its properties include name, company, nickname of the contact and many more.
 *
 * <table>
 * <caption> Table: Contact editable views </caption>
 * <tr>
 * 	<th>View (Editable)</th>
 * 	<th>Description</th>
 * </tr>
 * <tr>	<td>	_contacts_address_book	</td>	<td>	Describes the properties of the address book		</td>	</tr>
 * <tr>	<td>	_contacts_group	</td>	<td>	Describes the properties of the group		</td>	</tr>
 * <tr>	<td>	_contacts_person	</td>	<td>	Describes the properties of the person		</td>	</tr>
 * <tr>	<td>	_contacts_simple_contact	</td>	<td>	Describes the properties of the contact		</td>	</tr>
 * <tr>	<td>	_contacts_contact	</td>	<td>	Describes the properties of the contact		</td>	</tr>
 * <tr>	<td>	_contacts_my_profile	</td>	<td>	Describes the properties of the my profile		</td>	</tr>
 * <tr>	<td>	_contacts_name	</td>	<td>	Describes the properties of the contacts name		</td>	</tr>
 * <tr>	<td>	_contacts_number	</td>	<td>	Describes the properties of the contacts number		</td>	</tr>
 * <tr>	<td>	_contacts_email	</td>	<td>	Describes the properties of the contacts email		</td>	</tr>
 * <tr>	<td>	_contacts_address	</td>	<td>	Describes the properties of the contacts address		</td>	</tr>
 * <tr>	<td>	_contacts_note	</td>	<td>	Describes the properties of the contacts note		</td>	</tr>
 * <tr>	<td>	_contacts_url	</td>	<td>	Describes the properties of the contacts url		</td>	</tr>
 * <tr>	<td>	_contacts_event	</td>	<td>	Describes the properties of the contacts birthday and anniversary		</td>	</tr>
 * <tr>	<td>	_contacts_group_relation	</td>	<td>	Describes the properties of the contacts group relation		</td>	</tr>
 * <tr>	<td>	_contacts_relationship	</td>	<td>	Describes the properties of the contacts relationship		</td>	</tr>
 * <tr>	<td>	_contacts_image	</td>	<td>	Describes the properties of the contacts image		</td>	</tr>
 * <tr>	<td>	_contacts_company	</td>	<td>	Describes the properties of the contacts company		</td>	</tr>
 * <tr>	<td>	_contacts_nickname	</td>	<td>	Describes the properties of the contacts nickname		</td>	</tr>
 * <tr>	<td>	_contacts_messenger	</td>	<td>	Describes the properties of the contacts messenger		</td>	</tr>
 * <tr>	<td>	_contacts_extension	</td>	<td>	Describes the properties of the extension		</td>	</tr>
 * <tr>	<td>	_contacts_sdn	</td>	<td>	Describes the properties of the service describe number		</td>	</tr>
 * <tr>	<td>	_contacts_profile	</td>	<td>	Describes the properties of the profile		</td>	</tr>
 * <tr>	<td>	_contacts_activity_photo	</td>	<td>	Describes the properties of the photo of contacts activity		</td>	</tr>
 * <tr>	<td>	_contacts_activity	</td>	<td>	Describes the properties of the contacts activity		</td>	</tr>
 * <tr>	<td>	_contacts_speeddial	</td>	<td>	Describes the properties of the speed dial		</td>	</tr>
 * <tr>	<td>	_contacts_phone_log	</td>	<td>	Describes the properties of the log		</td>	</tr>
 * </table>
 *
 * <table>
 * <caption> Table: Contact read only views </caption>
 * <tr>
 * 	<th>View (Read only)</th>
 * 	<th>Description</th>
 *	</tr>
 *	<tr>	<td>	_contacts_contact_updated_info	</td>	<td>	used when identifying contact changes depending on version.	</td>	</tr>
 *	<tr>	<td>	_contacts_my_profile_updated_info	</td>	<td>	used when identifying my profile changes depending on version.	</td>	</tr>
 *	<tr>	<td>	_contacts_group_updated_info	</td>	<td>	used when identifying group changes depending on version.	</td>	</tr>
 *	<tr>	<td>	_contacts_group_member_updated_info	</td>	<td>	used when identifying group member changes depending on version.	</td>	</tr>
 *	<tr>	<td>	_contacts_grouprel_updated_info	</td>	<td>	used when identifying group relation profile changes depending on version.	</td>	</tr>
 *	<tr>	<td>	_contacts_person_contact	</td>	<td>	used when querying to merge information of person and contact	</td>	</tr>
 *	<tr>	<td>	_contacts_person_number	</td>	<td>	used when querying to merge information of person and number	</td>	</tr>
 *	<tr>	<td>	_contacts_person_email	</td>	<td>	used when querying to merge information of person and email	</td>	</tr>
 *	<tr>	<td>	_contacts_person_grouprel	</td>	<td>	used when querying to merge information of person and group relation	</td>	</tr>
 *	<tr>	<td>	_contacts_person_group_assigned	</td>	<td>	used when querying to information of person who assigned group	</td>	</tr>
 *	<tr>	<td>	_contacts_person_group_not_assigned	</td>	<td>	used when querying to information of person who not assigned group	</td>	</tr>
 *	<tr>	<td>	_contacts_person_phone_log	</td>	<td>	used when querying to merge information of person and phone log	</td>	</tr>
 *	<tr>	<td>	_contacts_person_usage	</td>	<td>	used when querying to information of person usage	</td>	</tr>
 *	<tr>	<td>	_contacts_contact_number	</td>	<td>	used when querying to merge information of contact and number	</td>	</tr>
 *	<tr>	<td>	_contacts_contact_email	</td>	<td>	used when querying to merge information of contact and email	</td>	</tr>
 *	<tr>	<td>	_contacts_contact_grouprel	</td>	<td>	used when querying to merge information of contact and group relation	</td>	</tr>
 *	<tr>	<td>	_contacts_contact_activity	</td>	<td>	used when querying to merge information of contact and activity	</td>	</tr>
 *	<tr>	<td>	_contacts_phone_log_stat	</td>	<td>	used when querying to information of phone log status	</td>	</tr>
 * </table>
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_VIEWS_PROPERTIES Properties
 * Property elements have their data types and name. <br>
 * Record types which have *_id as their properties, hold identifiers of other records - e.g. name, number and email views hold ID of their corresponding contacts in contact_id property (as children of the corresponding contacts record). <br>
 * A type of some property is ‘record’.
 * It means that the parent record can have child records.
 * For example, a contact record has 'name', 'number' and 'email' properties, which means that records of those types can be children of contact type records.<br>
 * In contacts_view.h headear file, view macros are found and below figure shows what macro means.
 *
 * @image html Properties.png "Figure: Properties"
 *
 * Sample code: Create a contact record then set caller ID
 * @code
 * contacts_record_h contact = NULL;
 * contacts_record_create(_contacts_contact._uri, &contact);
 *
 * // set image to _contacts_contact view.
 * contacts_record_h image = NULL;
 * contacts_record_create(_contacts_image._uri, &image);
 *
 * char *resource_path = app_get_resource_path();
 * char caller_id_path[1024] = {0};
 * snprintf(caller_id_path, sizeof(caller_id_path), "%s/caller_id.jpg", resource_path);
 * free(resource_path);
 * contacts_record_set_str(image, _contacts_image.path, caller_id_path);
 *
 * // add image record to contact
 * contacts_record_add_child_record(contact, _contacts_contact.image, image);
 * @endcode
 *
 *
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_MODULE_RECORDS Records
 * In contacts-service, one of basic concept is a record.
 * It may be helpful to know that a record represents an actual record in the internal database, but in general,
 * you can think of a record as a piece of information, like an address, phone number or group of contacts.
 * A record can be a complex set of data, containing other data, e.g. an address record contains country, region, and street, among others.<br>
 * Also, the contained data can be a reference to another record.
 * For example, a contact record contains the 'address' property, which is a reference to an address record.
 * An address record belongs to a contact record - its 'contact_id' property is set to identifier of the corresponding contact (more on IDs later).
 * In this case, the address is the contact's child record and the contact is the parent record.<br>
 * Effectively, a record can be a node in a tree or graph of relations between records.
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_RECORDS_URI URI
 * Each record type has a special structure defined for it, called 'view', which contains identifiers of its properties. <br>
 * Every view has a special field - _uri - that uniquely identifies the view.
 * In many cases you need to use the _uri value to indicate what type of record you wish to create or operate on.
 *
 * APIs which needs _uri
 * @code
 * API int contacts_record_create(const char* view_uri, ...)
 * API int contacts_filter_create(const char* view_uri, ...)
 * API int contacts_query_create(const char* view_uri, ...)
 * API int contacts_db_get_record(const char* view_uri, ...)
 * API int contacts_db_delete_record(const char* view_uri, ...)
 * API int contacts_db_get_all_records(const char* view_uri, ...)
 * API int contacts_db_delete_records(const char* view_uri, ...)
 * API int contacts_db_add_changed_cb(const char* view_uri, ...)
 * API int contacts_db_remove_changed_cb(const char* view_uri, ...)
 * API int contacts_db_get_changes_by_version(const char* view_uri, ...)
 * API int contacts_db_search_records(const char* view_uri, ...)
 * API int contacts_db_search_records_with_range(const char* view_uri, ...)
 * API int contacts_db_get_count(const char* view_uri, ...)
 * @endcode
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_RECORDS_HANDLE Record handle
 * To use a record, you must obtain its handle.
 * There are many ways to obtains it, including creating a new record and referring to child records of a record. <br>
 * When creating a record, you need to specify what type of record you want to create.
 * This is where you should use the URI property.
 *
 * Sample code: Creates a contact record
 * @code
 * contacts_record_h contact = NULL;
 * contacts_record_create(_contacts_contact._uri, &contact);
 * @endcode
 *
 * Sample code: Gets a contact record with id
 * @code
 * contacts_record_h contact = NULL;
 * contacts_db_get_record(_contacts_contact._uri, id, &contact);
 * @endcode
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_RECORDS_BASIC_TYPES Basic types
 * A record can have basic properties of five types: integer, string, boolean, long integer, lli (long long int), double.
 * Each property of basic type has functions to operate on it:
 *
 * <table>
 * <caption> Table: Setter and getter functions </caption>
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
 *    <td> long long integer </td>
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
 * Above functions also require specifying which property you wish to get/set.
 * Every getter and setter functions need record and property id.
 * You can make property id by combine data-view name and property name.
 * (.e.g. property id of a contact display_name property : _contacts_contact.display_name)
 *
 * Sample code : Sets the ‘ringtone_path’ property of a contact record.
 * @code
 * char *resource_path = app_get_resource_path();
 * char ringtone_path[1024] = {0};
 * snprintf(ringtone_path, sizeof(ringtone_path), "%s/ringtone.mp3", resource_path);
 * free(resource_path);
 * contacts_record_set_str(contact, _contacts_contact.ringtone_path, ringtone_path);
 * @endcode
 *
 * Note on returned values ownership: string getter function have the "_p" postfix.
 * It means that the returned value should not be freed by the application, as it is a pointer to data in an existing record.
 *
 * Sample code: Two ways of getting string property
 * @code
 * contacts_record_get_str(record, _contacts_person.display_name, &display_name);
 * contacts_record_get_str_p(record, _contacts_person.display_name, &display_name);
 * @endcode
 * In the first case, the returned string should be freed by the application.
 * In second one, display_name value will freed automatically when destroying record handle.
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_RECORDS_CHILD Child
 * A record can have properties of type 'record' called child records.
 * A record can contain several records of a given type. For example, a ‘contact record’(parent) can contain many ‘address records’(children).
 * The code below inserts an address record into a contact record.
 * Note that it is not necessary to insert all records - just the contact record needs to be inserted into the database, it is enough for all information to be stored.
 * Both records are then destroyed.
 *
 * Sample code: Add child record
 * @code
 * contacts_record_h address = NULL;
 * contacts_record_h image = NULL;
 * int contact_id = 0;
 *
 * // image, address record can be child record of contact record
 * contacts_record_create(_contacts_contact._uri, &contact);
 *
 * contacts_record_create(_contacts_image._uri, &image);
 * char *resource_path = app_get_resource_path();
 * char caller_id_path[1024] = {0};
 * snprintf(caller_id_path, sizeof(caller_id_path), "%s/caller_id.jpg", resource_path);
 * free(resource_path);
 * contacts_record_set_str(image, _contacts_image.path, caller_id_path);
 * contacts_record_add_child_record(contact, _contacts_contact.image, image);
 *
 * contacts_record_create(_contacts_address._uri, &address);
 * contacts_record_set_str(address, _contacts_address.country, "Korea");
 * contacts_record_add_child_record(contact, _contacts_contact.address, address);
 *
 * // insert contact to DBs
 * contacts_db_insert_record(contact, &contact_id);
 * contacts_record_destroy(contact, true);
 * @endcode
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_RECORDS_RECORD_ID_PROPERTY Record id property
 * ID is unique number which can identify a record Therefore, if you know the id of a record, you can directly handle the record.
 * The id is read-only property, which is available after the record has been inserted into the database.
 *
 * Sample code: Gets a contact record with id
 * @code
 * contacts_record_h contact = NULL;
 * contacts_record_create(_contacts_contact._uri, &contact);
 *
 * contacts_record_h name = NULL;
 * contacts_record_create(_contacts_name._uri, &name);
 * contacts_record_set_str(name, _contacts_name.first, “first name”);
 * contacts_record_add_child_record(contact, _contacts_contact.name, name);
 *
 * int contact_id = 0;
 * contacts_db_insert_record(contact, &contact_id); // contact_id is unique number of contact record
 * contacts_record_destroy(contact, true); // contact is no longer usable
 *
 * contacts_db_get_record(_contacts_contact._uri, contact_id, &contact); // contact is now a handle to the same record as before
 * char *display_name = NULL;
 * contacts_record_get_str(contact, _contacts_contact.display_name, &display_name);
 * contacts_record_destroy(contact, true); // contact is no longer usable
 * @endcode
 *
 * Identifiers can also be used to establish a relation between two records. The following code sets an address record's 'contact_id' property to the id of the contact. contact_id make relate between the address record and the contact which is identified by the contact_id. After that is done, the address becomes one of the addresses connected to the contact. The address is now the contact's child record, the contact is the parent record.
 *
 * Sample code: Insert a address record with contact_id
 * @code
 * int contact_id = ... // acquire id of created contact
 * int address_id = 0;
 * contacts_record_create(_contacts_address._uri, &address);
 * contacts_record_set_int(address, _contacts_address.contact_id, contact_id);
 * // set other address properties
 * // ...
 * contacts_db_insert_record(address, &address_id);
 * @endcode
 *
 * Having a record handle, you can access all records of a specific type related to the given record.
 *
 * Sample code: Gets a contact record
 * @code
 * int contact_id = ... // acquire id of created contact
 * int address_num = 0;
 * int i = 0;
 * contacts_db_get_record(_contacts_contact._uri, contact_id, &contact);
 * contacts_record_get_child_record_count(contact, _contacts_contact.address, &address_num);
 * for (i=0; i<address_num; i++) {
 * 	contacts_record_h address = NULL;
 * 	contacts_record_get_child_record_at_p(contact, _contacts_contact.address, i, &address);
 * 	contacts_record_set_str(address, _contacts_address.country, "Korea");
 * }
 * contacts_db_update_record(contact);
 * contacts_record_destroy(contact, true);
 * @endcode
 *
 * This example is to change a country of addresses which are child records of a contact. Each address can be traversed by using contacts_record_get_child_record_at_p(). It is possible to apply the changes by updating the contact which is the parent record.
 *
 *
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_MODULE_LISTS Lists
 * The contacts-service provides functions which can handle list of same type records. Lists concept is based on standard doubly linked list.
 *
 * To operate a list, you must obtain its handle. The handle is provided during creation of the list. List handle must destroy after use.
 * @code
 * API int contacts_list_create(contacts_list_h* contacts_list);
 * API int contacts_list_destroy(contacts_list_h contacts_list, bool delete_child);
 * @endcode
 *
 * If ‘delete_child’ parameter is the true, child resources will destroy automatically.
 *
 * Sample code: Create a list handle
 * @code
 * // get list handle with query
 * contacts_list_h list = NULL;
 * contacts_list_create(&list);
 *
 * // use list
 * //  ...
 *
 * contacts_list_destroy(list, true);
 * @endcode
 *
 * Sample code: Gets person list handle from database.
 * @code
 * // get list handle with query
 * contacts_list_h list = NULL;
 * contacts_db_get_all_records(_contacts_person._uri, 0, 0, &list);
 *
 * // use list
 * // ...
 *
 * contacts_list_destroy(list, true);
 * @endcode
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_LISTS_CURSOR Cursor
 * The list can be traversed by using cursor.
 * @code
 * API int contacts_list_first(contacts_list_h contacts_list);
 * API int contacts_list_last(contacts_list_h contacts_list);
 * API int contacts_list_next(contacts_list_h contacts_list);
 * API int contacts_list_prev(contacts_list_h contacts_list);
 * @endcode
 *
 * You can get a record of current cursor.
 * @code
 * API int contacts_list_get_current_record_p(contacts_list_h contacts_list, contacts_record_h* record);
 * @endcode
 *
 * Sample code: Loop list
 * @code
 * contacts_list_h list = NULL;
 * contacts_record_h record = NULL;
 * contacts_db_get_all_records(_contacts_person._uri, 0, 0, &list);
 * do {
 * 	contacts_list_get_current_record_p(list, &record);
 * 	if (NULL == record)
 *			break;
 * 	char *name = NULL;
 * 	contacts_record_get_str_p(record, _contacts_person.display_name, &name);
 * 	printf(“name=%s\n”, name);
 * } while (CONTACTS_ERROR_NONE == contacts_list_next(list));
 * contacts_list_destroy(list, true); // destroy child records automatically
 * @endcode
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_LISTS_ADD_REMOVE Add / Remove
 * The contacts-service provides functions for adding/removing child record on list.
 * @code
 * API int contacts_list_add(contacts_list_h contacts_list, contacts_record_h record);
 * API int contacts_list_remove(contacts_list_h contacts_list, contacts_record_h record);
 * @endcode
 *
 * Sample code: Adds records to the list
 * @code
 * contacts_record_h group1 = NULL;
 * contacts_record_create(_contacts_group._uri, &group1);
 * contacts_record_set_str(group1, _contacts_group.name, “group test1”);
 *
 * contacts_record_h group2 = NULL;
 * contacts_record_create(_contacts_group._uri, &group2);
 * contacts_record_set_str(group2, _contacts_group.name, “group test2”);
 *
 * contacts_list_h list = NULL;
 * contacts_list_create(&list);
 *
 * // Adds records to the list
 * contacts_list_add(list, group1);
 * contacts_list_add(list, group2);
 *
 * contacts_db_insert_records(list, NULL, NULL);
 * contacts_list_destroy(list, true);
 * @endcode
 *
 *
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_MODULE_BULK_APIS Bulk APIs
 * Bulk APIs provide to insert/update/delete multiple records. There is no limit of record count on bulk API, but it cause a process to hang during the api is operated. Bulk APIs guarantee atomicity. That is, the api operations either all, or nothing.
 *
 * @code
 * API int contacts_db_insert_records(contacts_list_h record_list, int **ids, int *count);
 * API int contacts_db_update_records(contacts_list_h record_list);
 * API int contacts_db_delete_records(const char* view_uri, int record_id_array[], int count);
 * API int contacts_db_replace_records(contacts_list_h list, int record_id_array[], int count);
 * @endcode
 *
 * Sample code: Insert two contact records using bulk API.
 * @code
 * contacts_record_h contact1;
 * contacts_record_create(_contacts_contact.uri, &contact1);
 *
 * // fill contact record
 * // ...
 *
 * contacts_record_h contact2;
 * contacts_record_create(_contacts_contact._uri, &contact2);
 *
 * // fill contact record
 * // ...
 *
 * contacts_list_h list = NULL;
 * contacts_list_create(&list);
 *
 * contacts_list_add(list, contact1);
 * contacts_list_add(list, contact2);
 *
 * int **ids = NULL;
 * int count = 0;
 *
 * // Insert conact records using bulk API
 * contacts_db_insert_records(list, &ids, &count);
 *
 * // use ids
 * // ...
 *
 * contacts_list_destroy(list, true);
 * free(ids);
 * @endcode
 *
 *
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_MODULE_FILTER_AND_QUERIES Queries
 * Queries are used to retrieve data which satisfies given criteria, like an integer property being greater than a given value, or a string property containing a given substring. Query needs a filter which can set the condition for search. The contacts-service provides query APIs for sorting, set projection and removing duplicated results.
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_FILTER_AND_QUERIES_FILTER Filter
 * The contacts Filter API provides the set of definitions and interfaces that enable application developers to make filters to set query. <br>
 * When creating a filter, you need to specify what type of filter you want to create using _uri property. Filter handle must destroy after use.
 * @code
 * API int contacts_filter_create(const char* view_uri, contacts_filter_h* filter);
 * API int contacts_filter_destroy(contacts_filter_h filter);
 * @endcode
 *
 * Sample code: Set filter condition to contain a given substring.
 * @code
 * contacts_filter_add_str(filter, _contacts_person.display_name, CONTACTS_MATCH_CONTAINS, “1234”);
 * @endcode
 *
 * Sample code: Set filter condition to property value is true.
 * @code
 * contacts_filter_add_bool(filter, _contacts_person.is_favorite, true);
 * @endcode
 *
 * Conditions can be added to a filter. The conditions SHOULD be joined by using a AND and OR logical operator.
 *
 * Sample code: Creates composite filter with OR operator.
 * @code
 * contacts_filter_add_str(filter1, _contacts_person.display_name, CONTACTS_MATCH_CONTAINS, “1234”);
 * contacts_filter_add_operator(filter1, CONTACTS_FILTER_OPERATOR_OR);
 * contacts_filter_add_str(filter1, _contacts_person.display_name, CONTACTS_MATCH_CONTAINS, “5678”);
 * @endcode
 *
 * Additionally, filters can join each other by using a AND and OR logical operator.
 *
 * Sample code: Creates joined filter with AND operator.
 * @code
 * contacts_filter_add_str(filter1, _contacts_person.display_name, CONTACTS_MATCH_CONTAINS, “1234”);
 * contacts_filter_add_operator(filter1, CONTACTS_FILTER_OPERATOR_OR);
 * contacts_filter_add_str(filter1, _contacts_person.display_name, CONTACTS_MATCH_CONTAINS, “5678”);
 *
 * contacts_filter_add_bool(filter2, _contacts_person.is_fovorite, true);
 *
 * contacts_filter_add_operator(filter1, CONTACTS_FILTER_OPERATOR_AND);
 * contacts_filter_add_filter(filter1, filter2);
 * @endcode
 *
 * Operator precedence in filters is determined by the order in which the conditions and filters are added. For example, if the following sequence is added:
 * <table>
 * <caption> Table: Filter and conditions </caption>
 * <tr>
 *	<th> Filter with conditions </th>
 *	<th> Result </th>
 * </tr>
 * <tr>
 *	<td>
 *		Contidion C1 <br>
 *		OR <br>
 *		Contidion C2 <br>
 *		AND <br>
 *		Condition C3
 *	</td>
 *	<td> (C1 OR C2) AND C3 </td>
 * </tr>
 * <tr>
 *	<td>
 *		Filter F1: <br>
 *			Condition C1 <br>
 *			OR <br>
 *			Condition C2 <br><br>
 *		Filter F2: <br>
 *			Condition C3 <br>
 *			OR <br>
 *			Condition C4 <br><br>
 *		Filter F3: <br>
 *			Condition C5 <br>
 *			AND <br>
 *			F1 <br>
 *			AND <br>
 *			F2
 *	</td>
 *	<td>
 *		(C5 AND F1) AND F2 <br>
 *		Which is: <br>
 *		(C5 AND (C1 OR C2)) AND (C3 OR C4)
 *	</td>
 * </tr>
 * </table>
 *
 * Sample code: Create a filter which will accept addresses with their contact's id equal to a given id (integer filter), or their country property equal to "Korea" (string filter). Create a query and add the filter to it. Results are received in a list.
 * @code
 * contacts_filter_h filter = NULL;
 * contacts_list_h list = NULL;
 * contacts_query_h query = NULL;
 *
 * contacts_filter_create(_contacts_address._uri, &filter);
 * contacts_filter_add_int(filter, _contacts_address.contact_id, CONTACTS_MATCH_EQUAL, id);
 * contacts_filter_add_operator(filter, CONTACTS_FILTER_OPERATOR_OR);
 * contacts_filter_add_str(filter, _contacts_address.country, CONTACTS_MATCH_EXACTLY, "Korea");
 * contacts_query_create(_contacts_address._uri, &query);
 * contacts_query_set_filter(query, filter);
 *
 * contacts_db_get_records_with_query(query, 0, 0, &list);
 *
 * contacts_filter_destroy(filter);
 * contacts_query_destroy(query);
 *
 * // use the list
 * // ...
 *
 * contacts_list_destroy(list, true);
 *
 * @endcode
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_FILTER_AND_QUERIES_SORT Sort
 * Query results list can sort by property id.
 * @code
 * API int contacts_query_set_sort(contacts_query_h query, unsigned int property_id, bool is_ascending);
 * @endcode
 *
 * Sample code: Sort to query result by person id
 * @code
 * contacts_filter_add_str(filter, _contacts_person.display_name, CONTACTS_MATCH_CONTAINS, “Joe”);
 * contacts_query_set_filter(query, filter);
 * contacts_query_set_sort(query, _contacts_person.id, true);
 *
 * contacts_db_get_records_with_query(query, 0, 0, &list);
 *
 * contacts_query_destroy(query);
 * contacts_filter_destroy(filter);
 * contacts_list_destroy(person_list, true);
 * @endcode
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_FILTER_AND_QUERIES_PROJECTION Projection
 * Projection allows you to query the Data for just those specific properties of a record that you actually need, at lower latency and cost than retrieving the entire properties.
 * @code
 * API int contacts_query_set_projection(contacts_query_h query, unsigned int property_ids[], int count)
 * @endcode
 *
 * Sample code: Creates a filter which will get only person id, display name, image thumbnail path from the person record with its vibration path has “test” (string filter). Create a query and add the filter to it. Results are received in a list.
 * @code
 * contacts_filter_add_str (filter, _contacts_person.vibration, CONTACTS_MATCH_CONTAINS, "test");
 * contacts_query_set_filter(query, filter);
 *
 * //set projections to get
 * unsigned int person_projection[] = {
 * 	_contacts_person.id,
 *		_contacts_person.display_name,
 *		_contacts_person.image_thumbnail_path,
 *	};
 *	contacts_query_set_projection(query, person_projection, sizeof(person_projection)/sizeof(int));
 *
 *	contacts_db_get_records_with_query(query, 0, 0, &person_list);
 *
 * // use list
 * // ...
 *
 * contacts_query_destroy(query);
 * contacts_filter_destroy(filter);
 * contacts_list_destroy(person_list, true);
 * @endcode
 *
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_FILTER_AND_QUERIES_DISTINCT Distinct
 * If you query to some read-only view with set projection, result list can contain duplicates. You can remove duplicates using _contacts_query_set_distinct.
 * @code
 * API int contacts_query_set_distinct(contacts_query_h query, bool set)
 * endcode
 *
 * Sample code: Remove duplicates
 * @code
 * unsigned int projection[] = {
 * 	_contacts_person_number.person_id,
 *		_contacts_person_number.display_name,
 *	};
 *	contacts_filter_create(_contacts_person_number._uri, &filter);
 *	contacts_filter_add_bool(filter, _contacts_person_number.has_phonenumber, true);
 *
 * contacts_query_create(_contacts_person_number._uri, &query);
 *	contacts_query_set_projection(query, projection, sizeof(projection)/sizeof(int));
 *	contacts_query_set_filter(query, filter);
 *
 * // set distinct (remove duplicats)
 * contacts_query_set_distinct(query, true);
 *
 * contacts_db_get_records_with_query(query, 0, 0, &list);
 *
 * // use list
 * // ...
 *
 * contacts_list_destroy(list, true);
 * contacts_query_destroy(query);
 * contacts_filter_destroy(filter);
 * @endcode
 *
 *
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_MODULE_DATABASE_CHANGE_NOTIFICATIONS Database Change Notifications
 * Applications add/remove callback function to detect/ignore the contacts DB changes with contacts_db_add_changed_cb() / contacts_db_remove_changed_cb(). <br>
 * Clients wait contacts change notification on client side. <br>
 * If contacts is changed by another module, server publishes notification. The notification will be broadcast to subscribed modules. Finally, user callback function is called with user data.
 *
 * Sample code: Register person changes notification callback
 * @code
 * // callback function
 * static void __person_changed_ cb(const char *view_uri, void *user_data)
 * {
 * 	// jobs for callback
 * }
 * // add changed noti callback
 * contacts_db_add_changed_cb(_contacts_person._uri,  __person_changed_cb,  NULL);
 * @endcode
 *
 *
 *
 * @section CAPI_SOCIAL_CONTACTS_SVC_MODULE_VCARD vCard
 * Contacts-service provides methods for parsing and making vCard . vCard APIs based on vCard v3.0 specification. For more information about vCard, refer to rfc2426 (http://www.ietf.org/rfc/rfc2426.txt)
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_VCARD_PARSING_VCARD Parsing vCard
 * There are two ways for parsing vCard.
 *
 * Sample code: Parsing vCard from stream then insert to database.
 * @code
 * // make contact record list from vcard stream
 * contacts_list_h list = NULL;
 * contacts_vcard_parse_to_contacts(vcard_stream, &list);
 *
 * int *ids = NULL;
 * int count = 0;
 * contacts_db_insert_records(list, &ids, &count);
 *
 * // use ids, count
 * // ...
 *
 * free(ids);
 * contacts_list_destroy(list, true);
 * @endcode
 *
 * Sample code: Parsing vCard from file then insert to database
 * @code
 * // called to get a record handle of _contacts_contact view
 * static bool __vcard_parse_cb(contacts_record_h record, void *user_data)
 * {
 * 	int id = 0;
 * 	contacts_db_insert_record(record, &id);
 *
 * 	// return false to break out of the loop
 * 	// return true to continue with the next iteration of the loop
 * 	return true;
 *	}
 *
 * // parse vCard from file
 * char *resource_path = app_get_resource_path();
 * char vcard_path[1024] = {0};
 * snprintf(vcard_path, sizeof(vcard_path), "%s/vcard.vcf", resource_path);
 * free(resource_path);
 * contacts_vcard_parse_to_contact_foreach(vcard_path, __vcard_parse_cb, NULL);
 * @endcode
 *
 *
 * @subsection CAPI_SOCIAL_CONTACTS_SVC_MODULE_VCARD_MAKING_VCARD_STREAM Making vCard stream
 * You can make vcard stream from contact, person or my profile record.
 *
 * Sample code: Makes vCard stream using contact record.
 * @code
 * contacts_record_h contact;
 * char *vcard_stream = NULL;
 *
 * contacts_db_get_record(_contacts_contact._uri, contact_id, &contact);
 * contacts_vcard_make_from_contact(contact, &vcard_stream);
 *
 * // use the vcard stream
 * // ...
 *
 * free(vcard_stream);
 * contacts_record_destroy(contact, true);
 * @endcode
 *
 */

#endif /* __TIZEN_SOCIAL_CONTACTS_DOC_H__ */


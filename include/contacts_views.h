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

#ifndef __TIZEN_SOCIAL_CONTACTS_VIEWS_H__
#define __TIZEN_SOCIAL_CONTACTS_VIEWS_H__

#include "contacts_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

// address_book
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_INT( account_id )			// read, write-once
	_CONTACTS_PROPERTY_STR( name )					// read, write
	_CONTACTS_PROPERTY_INT( mode )					// read, write
_CONTACTS_END_VIEW( _contacts_address_book )

// group
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_INT( address_book_id )		// read, write-once
	_CONTACTS_PROPERTY_STR( name )					// read, write
	_CONTACTS_PROPERTY_STR( ringtone_path )			// read, write
	_CONTACTS_PROPERTY_STR( image_path )			// read, write
	_CONTACTS_PROPERTY_STR( vibration )				// read, write
	_CONTACTS_PROPERTY_STR( extra_data )				// read, write, string
	_CONTACTS_PROPERTY_BOOL( is_read_only )			// read only
_CONTACTS_END_VIEW( _contacts_group )

// person
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_STR( display_name )			// read only
	_CONTACTS_PROPERTY_STR( display_name_index)		// read only
	_CONTACTS_PROPERTY_INT( display_contact_id )	// read, write
	_CONTACTS_PROPERTY_STR( ringtone_path )			// read, write
	_CONTACTS_PROPERTY_STR( image_thumbnail_path )	// read only
	_CONTACTS_PROPERTY_STR( vibration )				// read, write
	_CONTACTS_PROPERTY_STR( status )				// read only
	_CONTACTS_PROPERTY_BOOL( is_favorite )			// read, write
	_CONTACTS_PROPERTY_DOUBLE( favorite_priority )	// read only
	_CONTACTS_PROPERTY_INT( link_count )			// read only
	_CONTACTS_PROPERTY_STR( addressbook_ids )			// read only
	_CONTACTS_PROPERTY_BOOL( has_phonenumber )		// read only
	_CONTACTS_PROPERTY_BOOL( has_email )			// read only
_CONTACTS_END_VIEW( _contacts_person )

// simple contact
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )			// read only
	_CONTACTS_PROPERTY_STR( display_name )			// read only
	_CONTACTS_PROPERTY_INT( display_source_type)	// read only, internal field ?
	_CONTACTS_PROPERTY_INT( address_book_id )		// read, write-once
	_CONTACTS_PROPERTY_STR( ringtone_path )			// read, write
	_CONTACTS_PROPERTY_STR( image_thumbnail_path )	// read, write
	_CONTACTS_PROPERTY_BOOL( is_favorite )			// read, write
	_CONTACTS_PROPERTY_BOOL( has_phonenumber )		// read only
	_CONTACTS_PROPERTY_BOOL( has_email )			// read only
	_CONTACTS_PROPERTY_INT( person_id )				// read only
	_CONTACTS_PROPERTY_STR( uid )					// read, write
	_CONTACTS_PROPERTY_STR( vibration )				// read, write
	_CONTACTS_PROPERTY_INT( changed_time )			// read only
_CONTACTS_END_VIEW( _contacts_simple_contact )

// contact
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_STR( display_name )			// read only
	_CONTACTS_PROPERTY_INT( display_source_type )	// read only
	_CONTACTS_PROPERTY_INT( address_book_id )		// read, write once
	_CONTACTS_PROPERTY_STR( ringtone_path )			// read, write
	_CONTACTS_PROPERTY_STR( image_thumbnail_path )	// read, write
	_CONTACTS_PROPERTY_BOOL( is_favorite )			// read, write
	_CONTACTS_PROPERTY_BOOL( has_phonenumber )		// read only
	_CONTACTS_PROPERTY_BOOL( has_email )			// read only
	_CONTACTS_PROPERTY_INT( person_id )				// read only
	_CONTACTS_PROPERTY_STR( uid )					// read, write
	_CONTACTS_PROPERTY_STR( vibration )				// read, write
	_CONTACTS_PROPERTY_INT( changed_time )			// read only
	_CONTACTS_PROPERTY_INT( link_mode )			// read, write
	_CONTACTS_PROPERTY_CHILD_SINGLE( name )					// read, write
	_CONTACTS_PROPERTY_CHILD_SINGLE( image )				// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( company )				// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( note )					// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( number )					// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( email )					// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( event )					// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( messenger )				// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( address )				// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( url )					// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( nickname )				// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( profile )				// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( relationship )			// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( group_relation )		// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( extension )		// read, write
_CONTACTS_END_VIEW( _contacts_contact )

// my_profile
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_STR( display_name )			// read only
	_CONTACTS_PROPERTY_INT( address_book_id )		// read, write once
	_CONTACTS_PROPERTY_STR( image_thumbnail_path )	// read only
	_CONTACTS_PROPERTY_STR( uid )					// read, write
	_CONTACTS_PROPERTY_INT( changed_time )			// read only
	_CONTACTS_PROPERTY_CHILD_SINGLE( name )					// read, write
	_CONTACTS_PROPERTY_CHILD_SINGLE( image )				// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( company )				// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( note )					// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( number )					// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( email )					// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( event )					// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( messenger )				// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( address )				// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( url )					// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( nickname )				// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( profile )				// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( relationship )			// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE( extension )		// read, write
_CONTACTS_END_VIEW( _contacts_my_profile )


// contact_name
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )				// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_STR( first )					// read, write
	_CONTACTS_PROPERTY_STR( last )					// read, write
	_CONTACTS_PROPERTY_STR( addition )				// read, write
	_CONTACTS_PROPERTY_STR( suffix )				// read, write
	_CONTACTS_PROPERTY_STR( prefix )				// read, write
	_CONTACTS_PROPERTY_STR( phonetic_first )		// read, write
	_CONTACTS_PROPERTY_STR( phonetic_middle )		// read, write
	_CONTACTS_PROPERTY_STR( phonetic_last )			// read, write
_CONTACTS_END_VIEW( _contacts_name )

// contact_number
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )				// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_INT( type )					// read, write
	_CONTACTS_PROPERTY_STR( label )					// read, write
	_CONTACTS_PROPERTY_BOOL( is_default )			// read, write
	_CONTACTS_PROPERTY_STR( number )				// read, write
_CONTACTS_END_VIEW( _contacts_number )

// contact_email
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )				// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_INT( type )					// read, write
	_CONTACTS_PROPERTY_STR( label )					// read, write
	_CONTACTS_PROPERTY_BOOL( is_default )			// read, write
	_CONTACTS_PROPERTY_STR( email )					// read, write
_CONTACTS_END_VIEW( _contacts_email )

// contact_address
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )				// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_INT( type )					// read, write
	_CONTACTS_PROPERTY_STR( label )					// read, write
	_CONTACTS_PROPERTY_STR( postbox )				// read, write
	_CONTACTS_PROPERTY_STR( extended )				// read, write
	_CONTACTS_PROPERTY_STR( street )				// read, write
	_CONTACTS_PROPERTY_STR( locality )				// read, write
	_CONTACTS_PROPERTY_STR( region )				// read, write
	_CONTACTS_PROPERTY_STR( postal_code )			// read, write
	_CONTACTS_PROPERTY_STR( country )				// read, write
	_CONTACTS_PROPERTY_BOOL( is_default )			// read, write
_CONTACTS_END_VIEW( _contacts_address )

// contact_note
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_STR( note )					// read, write
_CONTACTS_END_VIEW( _contacts_note )

// contact_url
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )				// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_INT( type )					// read, write
	_CONTACTS_PROPERTY_STR( label )					// read, write
	_CONTACTS_PROPERTY_STR( url )					// read, write
_CONTACTS_END_VIEW( _contacts_url )

// contact_event
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )				// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_INT( type )					// read, write
	_CONTACTS_PROPERTY_STR( label )					// read, write
	_CONTACTS_PROPERTY_INT( date )					// read, write
	_CONTACTS_PROPERTY_INT( is_lunar )				// read, write
	_CONTACTS_PROPERTY_INT( lunar_date )			// read, write
_CONTACTS_END_VIEW( _contacts_event )

// contact_grouprelation
// refer to the contacts_group_add_contact, contacts_group_remove_contact
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only, can not used as filter
	_CONTACTS_PROPERTY_INT( group_id )				// read, write
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_STR( name )					// read only
_CONTACTS_END_VIEW( _contacts_group_relation )

// contact_relationship
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_INT( type )					// read, write
	_CONTACTS_PROPERTY_STR( label )					// read, write
	_CONTACTS_PROPERTY_STR( name )					// read, write
_CONTACTS_END_VIEW( _contacts_relationship )

// contact_image
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_INT( type )					// read, write
	_CONTACTS_PROPERTY_STR( label )					// read, write
	_CONTACTS_PROPERTY_STR( path )					// read, write
	_CONTACTS_PROPERTY_BOOL( is_default )
_CONTACTS_END_VIEW( _contacts_image )

// contact_company
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_INT( type )					// read, write
	_CONTACTS_PROPERTY_STR( label )					// read, write
	_CONTACTS_PROPERTY_STR( name )					// read, write
	_CONTACTS_PROPERTY_STR( department )			// read, write
	_CONTACTS_PROPERTY_STR( job_title )				// read, write
	_CONTACTS_PROPERTY_STR( assistant_name )		// read, write
	_CONTACTS_PROPERTY_STR( role )					// read, write
	_CONTACTS_PROPERTY_STR( logo )					// read, write
	_CONTACTS_PROPERTY_STR( location )				// read, write
	_CONTACTS_PROPERTY_STR( description )			// read, write
	_CONTACTS_PROPERTY_STR( phonetic_name )			// read, write
_CONTACTS_END_VIEW( _contacts_company )

// contact_nickname
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_STR( name )					// read, write
_CONTACTS_END_VIEW( _contacts_nickname )

// contact_messenger
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_INT( type )					// read, write
	_CONTACTS_PROPERTY_STR( label )					// read, write
	_CONTACTS_PROPERTY_STR( im_id )					// read, write
_CONTACTS_END_VIEW( _contacts_messenger )

// contact_extend
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_INT( data1 )					// read, write
	_CONTACTS_PROPERTY_STR( data2 )					// read, write
	_CONTACTS_PROPERTY_STR( data3 )					// read, write
	_CONTACTS_PROPERTY_STR( data4 )					// read, write
	_CONTACTS_PROPERTY_STR( data5 )					// read, write
	_CONTACTS_PROPERTY_STR( data6 )					// read, write
	_CONTACTS_PROPERTY_STR( data7 )					// read, write
	_CONTACTS_PROPERTY_STR( data8 )					// read, write
	_CONTACTS_PROPERTY_STR( data9 )					// read, write
	_CONTACTS_PROPERTY_STR( data10 )				// read, write
	_CONTACTS_PROPERTY_STR( data11 )				// read, write
	_CONTACTS_PROPERTY_STR( data12 )				// read, write
_CONTACTS_END_VIEW( _contacts_extension )

// contact_sdn
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_STR( name )					// read, write
	_CONTACTS_PROPERTY_STR( number )				// read, write
_CONTACTS_END_VIEW( _contacts_sdn )

// contact_profile
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_STR( uid )					// read, write
	_CONTACTS_PROPERTY_STR( text )					// read, write
	_CONTACTS_PROPERTY_INT( order )					// read, write
	_CONTACTS_PROPERTY_STR( service_operation )		// read, write
	_CONTACTS_PROPERTY_STR( mime )				// read, write
	_CONTACTS_PROPERTY_STR( app_id )				// read, write
	_CONTACTS_PROPERTY_STR( uri )					// read, write
	_CONTACTS_PROPERTY_STR( category )			// read, write
	_CONTACTS_PROPERTY_STR( extra_data )			// read, write
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
_CONTACTS_END_VIEW( _contacts_profile )

// activity photo
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_INT( activity_id )			// read, write once
	_CONTACTS_PROPERTY_STR( photo_url )				// read, write
	_CONTACTS_PROPERTY_INT( sort_index )			// read, write
_CONTACTS_END_VIEW( _contacts_activity_photo )

// activity
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_INT( contact_id )			// read, write once
	_CONTACTS_PROPERTY_STR( source_name )			// read, write
	_CONTACTS_PROPERTY_STR( status )				// read, write
	_CONTACTS_PROPERTY_INT( timestamp )				// read, write
	_CONTACTS_PROPERTY_STR( service_operation )		// read, write
	_CONTACTS_PROPERTY_STR( uri )					// read, write
	_CONTACTS_PROPERTY_CHILD_MULTIPLE(photo)		// read, write
_CONTACTS_END_VIEW( _contacts_activity )

// speeddial
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( speeddial_number )		// read, write
	_CONTACTS_PROPERTY_INT( number_id )				// read, write
	_CONTACTS_PROPERTY_STR( number )				// read only
	_CONTACTS_PROPERTY_STR( number_label )			// read only
	_CONTACTS_PROPERTY_INT( number_type )			// read only
	_CONTACTS_PROPERTY_INT( person_id )				// read only
	_CONTACTS_PROPERTY_STR( display_name )			// read only
	_CONTACTS_PROPERTY_STR( image_thumbnail_path )	// read only
_CONTACTS_END_VIEW( _contacts_speeddial )

// phone_log
_CONTACTS_BEGIN_VIEW()
	_CONTACTS_PROPERTY_INT( id )					// read only
	_CONTACTS_PROPERTY_INT( person_id )				// read, write once
	_CONTACTS_PROPERTY_STR( address )				// read, write once, number or email
	_CONTACTS_PROPERTY_INT( log_time )				// read, write once
	_CONTACTS_PROPERTY_INT( log_type )				// read, write
	_CONTACTS_PROPERTY_INT( extra_data1 )			// read, write once : message or email id, duration
	_CONTACTS_PROPERTY_STR( extra_data2 )			// read, write once : shortmsg, subject
_CONTACTS_END_VIEW( _contacts_phone_log )

// contact_updated_info : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( contact_id )
	_CONTACTS_PROPERTY_INT( address_book_id )
	_CONTACTS_PROPERTY_INT( type )					// insert/update/delete
	_CONTACTS_PROPERTY_INT( version )
	_CONTACTS_PROPERTY_BOOL( image_changed )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_contact_updated_info )

// my_profile_updated_info : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( address_book_id )
	_CONTACTS_PROPERTY_INT( last_changed_type )
	_CONTACTS_PROPERTY_INT( version )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_my_profile_updated_info )

// group_updated_info : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( group_id )
	_CONTACTS_PROPERTY_INT( address_book_id )
	_CONTACTS_PROPERTY_INT( type )					// insert/update/delete
	_CONTACTS_PROPERTY_INT( version )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_group_updated_info )


// group_updated_info : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( group_id )
	_CONTACTS_PROPERTY_INT( address_book_id )
	_CONTACTS_PROPERTY_INT( version )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_group_member_updated_info )

// grouprel_updated_info : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( group_id )
	_CONTACTS_PROPERTY_INT( contact_id )
	_CONTACTS_PROPERTY_INT( address_book_id )
	_CONTACTS_PROPERTY_INT( type )					// insert/delete
	_CONTACTS_PROPERTY_INT( version )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_grouprel_updated_info )

// only for query (filter/projection)
// person_contact : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( person_id )
	_CONTACTS_PROPERTY_STR( display_name )
	_CONTACTS_PROPERTY_PROJECTION_STR( display_name_index)
	_CONTACTS_PROPERTY_PROJECTION_INT( display_contact_id )
	_CONTACTS_PROPERTY_PROJECTION_STR( ringtone_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( image_thumbnail_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( vibration )
	_CONTACTS_PROPERTY_PROJECTION_STR( status )
	_CONTACTS_PROPERTY_BOOL( is_favorite )
	_CONTACTS_PROPERTY_PROJECTION_INT( link_count )
	_CONTACTS_PROPERTY_PROJECTION_STR( addressbook_ids )
	_CONTACTS_PROPERTY_BOOL( has_phonenumber )
	_CONTACTS_PROPERTY_BOOL( has_email )
	_CONTACTS_PROPERTY_INT( contact_id )
	_CONTACTS_PROPERTY_INT( address_book_id )
	_CONTACTS_PROPERTY_STR( address_book_name )
	_CONTACTS_PROPERTY_INT( address_book_mode )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_person_contact )

// person_number : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( person_id )
	_CONTACTS_PROPERTY_STR( display_name )
	_CONTACTS_PROPERTY_PROJECTION_STR( display_name_index)
	_CONTACTS_PROPERTY_PROJECTION_INT( display_contact_id )
	_CONTACTS_PROPERTY_PROJECTION_STR( ringtone_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( image_thumbnail_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( vibration )
	_CONTACTS_PROPERTY_BOOL( is_favorite )
	_CONTACTS_PROPERTY_BOOL( has_phonenumber )
	_CONTACTS_PROPERTY_BOOL( has_email )
	_CONTACTS_PROPERTY_INT( number_id )
	_CONTACTS_PROPERTY_PROJECTION_INT( type )
	_CONTACTS_PROPERTY_PROJECTION_STR( label )
	_CONTACTS_PROPERTY_BOOL( is_primary_default )
	_CONTACTS_PROPERTY_STR( number )
	_CONTACTS_PROPERTY_FILTER_STR( number_filter )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_person_number )

// person_email : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( person_id )
	_CONTACTS_PROPERTY_STR( display_name )
	_CONTACTS_PROPERTY_PROJECTION_STR( display_name_index)
	_CONTACTS_PROPERTY_PROJECTION_INT( display_contact_id )
	_CONTACTS_PROPERTY_PROJECTION_STR( ringtone_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( image_thumbnail_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( vibration )
	_CONTACTS_PROPERTY_BOOL( is_favorite )
	_CONTACTS_PROPERTY_BOOL( has_phonenumber )
	_CONTACTS_PROPERTY_BOOL( has_email )
	_CONTACTS_PROPERTY_INT( email_id )
	_CONTACTS_PROPERTY_PROJECTION_INT( type )
	_CONTACTS_PROPERTY_PROJECTION_STR( label )
	_CONTACTS_PROPERTY_BOOL( is_primary_default )
	_CONTACTS_PROPERTY_STR( email )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_person_email )

// person_group : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( person_id )
	_CONTACTS_PROPERTY_STR( display_name )
	_CONTACTS_PROPERTY_PROJECTION_STR( display_name_index)
	_CONTACTS_PROPERTY_PROJECTION_INT( display_contact_id )
	_CONTACTS_PROPERTY_PROJECTION_STR( ringtone_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( image_thumbnail_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( vibration )
	_CONTACTS_PROPERTY_PROJECTION_STR( status )
	_CONTACTS_PROPERTY_BOOL( is_favorite )
	_CONTACTS_PROPERTY_PROJECTION_INT( link_count )
	_CONTACTS_PROPERTY_PROJECTION_STR( addressbook_ids )
	_CONTACTS_PROPERTY_BOOL( has_phonenumber )
	_CONTACTS_PROPERTY_BOOL( has_email )
	_CONTACTS_PROPERTY_INT( address_book_id )
	_CONTACTS_PROPERTY_INT( group_id )
	_CONTACTS_PROPERTY_STR( address_book_name )
	_CONTACTS_PROPERTY_INT( address_book_mode )
	_CONTACTS_PROPERTY_PROJECTION_INT( contact_id )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_person_grouprel )

//person phone_log : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( person_id )
	_CONTACTS_PROPERTY_STR( display_name )
	_CONTACTS_PROPERTY_PROJECTION_STR( image_thumbnail_path )
	_CONTACTS_PROPERTY_INT( log_id )
	_CONTACTS_PROPERTY_STR( address )
	_CONTACTS_PROPERTY_PROJECTION_INT( address_type )
	_CONTACTS_PROPERTY_INT( log_time )
	_CONTACTS_PROPERTY_INT( log_type )
	_CONTACTS_PROPERTY_PROJECTION_INT( extra_data1 )
	_CONTACTS_PROPERTY_PROJECTION_STR( extra_data2 )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_person_phone_log )

// person, stat : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( person_id )
	_CONTACTS_PROPERTY_STR( display_name )
	_CONTACTS_PROPERTY_PROJECTION_STR( display_name_index)
	_CONTACTS_PROPERTY_PROJECTION_INT( display_contact_id )
	_CONTACTS_PROPERTY_PROJECTION_STR( ringtone_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( image_thumbnail_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( vibration )
	_CONTACTS_PROPERTY_BOOL( is_favorite )
	_CONTACTS_PROPERTY_BOOL( has_phonenumber )
	_CONTACTS_PROPERTY_BOOL( has_email )
	_CONTACTS_PROPERTY_INT( usage_type )
	_CONTACTS_PROPERTY_INT( times_used )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_person_usage )

// simple contact number : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( contact_id )
	_CONTACTS_PROPERTY_STR( display_name )
	_CONTACTS_PROPERTY_PROJECTION_INT( display_source_type)
	_CONTACTS_PROPERTY_INT( address_book_id )
	_CONTACTS_PROPERTY_INT( person_id )
	_CONTACTS_PROPERTY_PROJECTION_STR( ringtone_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( image_thumbnail_path )
	_CONTACTS_PROPERTY_INT( number_id )
	_CONTACTS_PROPERTY_PROJECTION_INT( type )
	_CONTACTS_PROPERTY_PROJECTION_STR( label )
	_CONTACTS_PROPERTY_BOOL( is_default )
	_CONTACTS_PROPERTY_STR( number )
	_CONTACTS_PROPERTY_FILTER_STR( number_filter )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_contact_number )

// simple contact email : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( contact_id )
	_CONTACTS_PROPERTY_STR( display_name )
	_CONTACTS_PROPERTY_PROJECTION_INT( display_source_type)
	_CONTACTS_PROPERTY_INT( address_book_id )
	_CONTACTS_PROPERTY_INT( person_id )
	_CONTACTS_PROPERTY_PROJECTION_STR( ringtone_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( image_thumbnail_path )
	_CONTACTS_PROPERTY_INT( email_id )
	_CONTACTS_PROPERTY_PROJECTION_INT( type )
	_CONTACTS_PROPERTY_PROJECTION_STR( label )
	_CONTACTS_PROPERTY_BOOL( is_default )
	_CONTACTS_PROPERTY_STR( email )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_contact_email )

// simple contact group : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( contact_id )
	_CONTACTS_PROPERTY_STR( display_name )
	_CONTACTS_PROPERTY_PROJECTION_INT( display_source_type)
	_CONTACTS_PROPERTY_INT( address_book_id )
	_CONTACTS_PROPERTY_INT( person_id )
	_CONTACTS_PROPERTY_PROJECTION_STR( ringtone_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( image_thumbnail_path )
	_CONTACTS_PROPERTY_INT( group_id )
	_CONTACTS_PROPERTY_PROJECTION_STR( group_name )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_contact_grouprel )

// simple contact activity : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_INT( contact_id )
	_CONTACTS_PROPERTY_STR( display_name )
	_CONTACTS_PROPERTY_PROJECTION_INT( display_source_type)
	_CONTACTS_PROPERTY_INT( address_book_id )
	_CONTACTS_PROPERTY_INT( account_id )
	_CONTACTS_PROPERTY_INT( person_id )
	_CONTACTS_PROPERTY_PROJECTION_STR( ringtone_path )
	_CONTACTS_PROPERTY_PROJECTION_STR( image_thumbnail_path )
	_CONTACTS_PROPERTY_INT( activity_id )
	_CONTACTS_PROPERTY_STR( source_name )
	_CONTACTS_PROPERTY_PROJECTION_STR( status )
	_CONTACTS_PROPERTY_INT( timestamp )
	_CONTACTS_PROPERTY_STR( service_operation )
	_CONTACTS_PROPERTY_STR( uri )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_contact_activity )

// phone_log number list : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_STR( number )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_phone_log_number )

// phone_log stat : read only
_CONTACTS_BEGIN_READ_ONLY_VIEW()
	_CONTACTS_PROPERTY_PROJECTION_STR( log_count )
	_CONTACTS_PROPERTY_STR( log_type )
_CONTACTS_END_READ_ONLY_VIEW( _contacts_phone_log_stat )

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_CONTACTS_VIEWS_H__ */


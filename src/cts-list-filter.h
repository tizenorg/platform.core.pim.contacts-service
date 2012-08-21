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
#ifndef __CTS_LIST_FILTER_H__
#define __CTS_LIST_FILTER_H__

#include "cts-list.h"

struct cts_filter {
	int type;
	int list_type;
	char *search_val;
	bool addrbook_on;
	bool group_on;
	bool limit_on;
	bool offset_on;
	int addrbook_id;
	int group_id;
	int limit;
	int offset;
};

//<!--

/**
 * @defgroup   CONTACTS_SVC_LIST_FILTER List handling with filter
 * @ingroup    CONTACTS_SVC_LIST
 * @addtogroup CONTACTS_SVC_LIST_FILTER
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
 * CTSiter is an opaque type, it must be
 * used via accessor functions.
 * @see contacts_svc_list_filter_new(), contacts_svc_list_str_filter_new(), contacts_svc_list_filter_free()
 */
typedef struct cts_filter CTSfilter;

/**
 * Use for contacts_svc_list_str_filter_new().
 */
typedef enum {
	CTS_FILTERED_PLOGS_OF_NUMBER = CTS_LIST_PLOGS_OF_NUMBER,/**< #PHONELOGLIST */
	CTS_FILTERED_CONTACTS_WITH_NAME = CTS_LIST_CONTACTS_WITH_NAME,/**< #CONTACTLIST */
	CTS_FILTERED_NUMBERINFOS_WITH_NAME = CTS_LIST_NUMBERINFOS_WITH_NAME,/**< #NUMBERLIST */
	CTS_FILTERED_NUMBERINFOS_WITH_NUM = CTS_LIST_NUMBERINFOS_WITH_NUM,/**< #NUMBERLIST */
	CTS_FILTERED_EMAILINFOS_WITH_EMAIL= CTS_LIST_EMAILINFOS_WITH_EMAIL,/**< #EMAILLIST */
}cts_str_filter_op;

/**
 * Use for contacts_svc_list_filter_new().
 */
typedef enum {
	CTS_FILTERED_ALL_CONTACT,/**< #CONTACTLIST */
	CTS_FILTERED_ALL_CONTACT_HAD_NUMBER,/**< #CONTACTLIST */
	CTS_FILTERED_ALL_CONTACT_HAD_EMAIL,/**< #CONTACTLIST */
	CTS_FILTERED_ALL_CONTACT_OSP = 1000,/**< #OSPLIST */
}cts_filter_op;

/**
 * Use for contacts_svc_list_filter_new(), contacts_svc_list_str_filter_new().
 */
typedef enum {
	CTS_LIST_FILTER_NONE, /**< . */
	CTS_LIST_FILTER_ADDRESBOOK_ID_INT, /**< exclusive with #CTS_LIST_FILTER_GROUP_ID_INT */
	CTS_LIST_FILTER_GROUP_ID_INT, /**< exclusive with #CTS_LIST_FILTER_ADDRESBOOK_ID_INT */
	CTS_LIST_FILTER_LIMIT_INT, /**< . */
	CTS_LIST_FILTER_OFFSET_INT, /**< Offset depends on Limit(#CTS_LIST_FILTER_LIMIT_INT) */
}cts_filter_type;

/**
 * Allocate, initialize and return a new contacts service list filter with constraints.
 * The constaint is composed with the pair of (type, val).
 * The constaints list should be terminated with #CTS_LIST_FILTER_NONE,
 * therefore the count of parameter is an odd number.
 * This should be used for getting filtered list only,
 * if not, be sure to use contacts_svc_get_list_with_str().
 *
 * @param[in] list_type type of list(#cts_str_filter_op)
 * @param[in] search_value String search value
 * @param[in] first_type type of first constraint
 * @return The pointer of New contacts service list filter, NULL on error
 * @see contacts_svc_list_filter_free()
 */
CTSfilter* contacts_svc_list_str_filter_new(cts_str_filter_op list_type,
   const char *search_value, cts_filter_type first_type, ...);

/**
 * Allocate, initialize and return a new contacts service list filter with constraints.
 * The constaint is composed with the pair of (type, val).
 * The constaints list should be terminated with #CTS_LIST_FILTER_NONE,
 * therefore the count of parameter is an even number.
 * This should be used for getting filtered list only,
 * if not, be sure to use contacts_svc_get_list().
 *
 * @param[in] list_type type of list(#cts_filter_op)
 * @param[in] first_type type of first constraint
 * @return The pointer of New contacts service list filter, NULL on error
 * @see contacts_svc_list_filter_free()
 */
CTSfilter* contacts_svc_list_filter_new(cts_filter_op list_type, cts_filter_type first_type, ...);

/**
 * A destructor for contacts service list filter.
 *
 * @param[in] filter A contacts service struct
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @see contacts_svc_list_filter_new(), contacts_svc_list_str_filter_new()
 */
int contacts_svc_list_filter_free(CTSfilter *filter);

/**
 * This function calls cb(#cts_foreach_fn) for each record of list gotten by filter.
 *
 * @param[in] filter The filter for searching
 * @param[in] cb callback function pointer(#cts_foreach_fn)
 * @param[in] user_data data which is passed to callback function
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_list_with_filter_foreach(CTSfilter *filter,
   cts_foreach_fn cb, void *user_data);

/**
 * This function gets iterator of the gotten data by filter.
 * \n Obtained iterator should be free using by contacts_svc_iter_remove().
 *
 * @param[in] filter The filter for searching
 * @param[out] iter Point of data iterator to be got
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_get_list_with_filter(CTSfilter *filter, CTSiter **iter);

/**
 * @}
 */
//-->

#endif //__CTS_LIST_FILTER_H__


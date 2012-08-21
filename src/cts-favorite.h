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
#ifndef __CTS_FAVORITE_H__
#define __CTS_FAVORITE_H__

//<!--

/**
 * favorite type
 */
typedef enum{
	CTS_FAVOR_PERSON, /**< Favorite for a contact */
	CTS_FAVOR_NUMBER /**< Favorite for a number */
}cts_favor_type;

/** deprecated */
#define CTS_FAVOR_CONTACT CTS_FAVOR_PERSON

/**
 * @defgroup   CONTACTS_SVC_FAVORITE Favorite(speeddial) Modification
 * @ingroup    CONTACTS_SVC
 * @addtogroup CONTACTS_SVC_FAVORITE
 * @{
 *
 * This interface provides methods to insert/update/delete the Favorite(speeddial).
 *
 */

/**
 * This function marks a number or a contact as "favorite".
 * @param[in] op favorite type(#cts_favor_type).
 * @param[in] related_id a contact or number id which is related op.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_set_favorite(cts_favor_type op, int related_id);

/**
 * This function unsets a number or a contact to the favorite.
 * @param[in] op favorite type(#cts_favor_type).
 * @param[in] related_id a contact or number id which is related op.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_unset_favorite(cts_favor_type op, int related_id);

/**
 * This function deletes a favorite from favorite list.
 * @param[in] favorite_id index of favorite.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_delete_favorite(int favorite_id);

/**
 * This function modifies priority of favorite.
 * The priority of favorite will be between a front favorite and a back favorite.
 *
 * @param[in] favorite_id The index of a favorite data
 * @param[in] front_favorite_id Id of the front favorite.
 * @param[in] back_favorite_id Id of the back favorite.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_favorite_order(int favorite_id, int front_favorite_id, int back_favorite_id);

/**
 * This function sets a number to the speeddial.
 * @param[in] speed_num speed dial number.
 * @param[in] number_id the index of number
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_set_speeddial(int speed_num, int number_id);

/**
 * This function unsets speed dial number.
 * @param[in] speed_num speed dial number.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_unset_speeddial(int speed_num);

/**
 * @}
 */

//-->

#endif //__CTS_FAVORITE_H__

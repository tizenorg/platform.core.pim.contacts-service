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
#include "cts-internal.h"
#include "cts-im.h"

API int contacts_svc_get_im_status(cts_get_im_op op_code, int search_id,
		cts_im_callback_fn cb, void *user_data)
{
	// select MAX(status) from connected_im where contact_id = index
	// select status from connected_im where data_id = index
	return CTS_SUCCESS;
}

API int contacts_svc_set_im_status(cts_im_type type,
		const char *im_id, cts_im_status status)
{
	return CTS_SUCCESS;
}


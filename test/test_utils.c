/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include "contacts.h"
#include "test_debug.h"
#include "test_utils.h"

void test_utils_cleanup(void)
{
	DBG("[%s]", __func__);

	int ret = 0;

	contacts_connect();

	contacts_list_h get_list = NULL;
	contacts_db_get_all_records(_contacts_contact._uri, 0, 0, &get_list);

	contacts_list_first(get_list);
	int count = 0;
	contacts_list_get_count(get_list, &count);
	DBG("count(%d)", count);

	int *ids = calloc(count, sizeof(int));
	int index = 0;
	do {
		contacts_record_h record = NULL;
		ret = contacts_list_get_current_record_p(get_list, &record);
		if (CONTACTS_ERROR_NONE != ret)
			break;
		int contact_id = 0;
		contacts_record_get_int(record, _contacts_contact.id, &contact_id);
		ids[index] =  contact_id;
		contacts_list_next(get_list);
	} while (1);

	ret = contacts_db_delete_records(_contacts_contact._uri, ids, count);
	if (CONTACTS_ERROR_NONE != ret)
		ERR("contacts_db_delete_records() Fail(%d)", ret);

	contacts_disconnect();
}

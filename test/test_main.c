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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <contacts.h>

#include "test_main.h"
#include "test_debug.h"
#include "test_base.h"
#include "test_query.h"
#include "test_snippet.h"

typedef int (*func)(int argc, char **argv);

void _print_errors(void)
{
	DEBUG("CONTACTS_ERROR_NONE               (%d)", CONTACTS_ERROR_NONE);
	DEBUG("CONTACTS_ERROR_OUT_OF_MEMORY      (%d)", CONTACTS_ERROR_OUT_OF_MEMORY);
	DEBUG("CONTACTS_ERROR_INVALID_PARAMETER  (%d)", CONTACTS_ERROR_INVALID_PARAMETER);
	DEBUG("CONTACTS_ERROR_FILE_NO_SPACE      (%d)", CONTACTS_ERROR_FILE_NO_SPACE);
	DEBUG("CONTACTS_ERROR_PERMISSION_DENIED  (%d)", CONTACTS_ERROR_PERMISSION_DENIED);
	DEBUG("CONTACTS_ERROR_NOT_SUPPORTED      (%d)", CONTACTS_ERROR_NOT_SUPPORTED);
	DEBUG("CONTACTS_ERROR_NO_DATA            (%d)", CONTACTS_ERROR_NO_DATA);
	DEBUG("CONTACTS_ERROR_DB_LOCKED          (%d)", CONTACTS_ERROR_DB_LOCKED);
	DEBUG("CONTACTS_ERROR_DB                 (%d)", CONTACTS_ERROR_DB);
	DEBUG("CONTACTS_ERROR_IPC_NOT_AVALIABLE  (%d)", CONTACTS_ERROR_IPC_NOT_AVALIABLE);
	DEBUG("CONTACTS_ERROR_IPC                (%d)", CONTACTS_ERROR_IPC);
	DEBUG("CONTACTS_ERROR_SYSTEM             (%d)", CONTACTS_ERROR_SYSTEM);
	DEBUG("CONTACTS_ERROR_INTERNAL           (%d)", CONTACTS_ERROR_INTERNAL);
}

const func _func[] = { _print_errors,
	test_base_delete_contact,
	test_query_person_get_all_person_contact,
	test_snippet_with_query_person,
	test_snippet_with_query_person_contact
	};


/*
 * With no argument, all cases are tested.
 * If argument, specific case could be tested.
 * ex> to test all cases,
 *   # /usr/bin/contacts-service-test
 *   to test 4th case,
 *   # /usr/bin/contacts-service-test 4
 *
 * NOTICE: each case could be tested independently.
 *         create resource -> test -> delete resource.
 */
int main(int argc, char **argv)
{
	ENTER();

	if (1 < argc) {
		int select = atoi(argv[1]);
		DBG("argc(%d) select(%d)", argc, select);
		return _func[select](argc, argv);
	}

	int i = 0;
	int count = sizeof(_func) / sizeof(_func[1]);
	for (i = 0; i < count; i++) {
		DBG(">>>>>>>>>>>>>>>>>>>> test case %04d", i);
		if (_func[i](argc, argv) < 0)
			break;
	}
	return 0;
}

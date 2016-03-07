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

int _print_errors(int argc, char **argv)
{
	DBG("CONTACTS_ERROR_NONE               (%d)", CONTACTS_ERROR_NONE              );
	DBG("CONTACTS_ERROR_OUT_OF_MEMORY      (%d)", CONTACTS_ERROR_OUT_OF_MEMORY     );
	DBG("CONTACTS_ERROR_INVALID_PARAMETER  (%d)", CONTACTS_ERROR_INVALID_PARAMETER );
	DBG("CONTACTS_ERROR_FILE_NO_SPACE      (%d)", CONTACTS_ERROR_FILE_NO_SPACE     );
	DBG("CONTACTS_ERROR_PERMISSION_DENIED  (%d)", CONTACTS_ERROR_PERMISSION_DENIED );
	DBG("CONTACTS_ERROR_NOT_SUPPORTED      (%d)", CONTACTS_ERROR_NOT_SUPPORTED     );
	DBG("CONTACTS_ERROR_NO_DATA            (%d)", CONTACTS_ERROR_NO_DATA	       );
	DBG("CONTACTS_ERROR_DB_LOCKED          (%d)", CONTACTS_ERROR_DB_LOCKED         );
	DBG("CONTACTS_ERROR_DB                 (%d)", CONTACTS_ERROR_DB                );
	DBG("CONTACTS_ERROR_IPC_NOT_AVALIABLE  (%d)", CONTACTS_ERROR_IPC_NOT_AVALIABLE );
	DBG("CONTACTS_ERROR_IPC                (%d)", CONTACTS_ERROR_IPC               );
	DBG("CONTACTS_ERROR_SYSTEM             (%d)", CONTACTS_ERROR_SYSTEM            );
	DBG("CONTACTS_ERROR_INTERNAL           (%d)", CONTACTS_ERROR_INTERNAL		   );

	return 0;
}

static const func _func[] = {
	_print_errors,
};

bool test_main_is_selected(int argc, char **argv, int depth, const func test_func[], int count)
{
	if (argc <= depth) {
		return false;
	}

	int select = atoi(argv[depth]);

	if (select < count)
		test_func[select](argc, argv);
	else
		ERR("no test func (%d)", select);

	return true;
}

/*
 * With no argument, all cases are tested.
 * If argument, specific case could be tested.
 * ex> to test all cases,
 *   # /usr/bin/calendar-service-test
 *   to test 4th case,
 *   # /usr/bin/calendar-service-test 4
 *
 * NOTICE: each case could be tested independently.
 *         create resource -> test -> delete resource.
 */

int main(int argc, char **argv)
{
	ENTER();

	DBG("argc(%d)", argc);

	int count = sizeof(_func) / sizeof(func);
	if (true == test_main_is_selected(argc, argv, 1, _func, count))
		return 0;

	int i = 0;
	for (i = 0; i < count; i++) {
		if (_func[i](argc, argv) < 0)
			break;
	}
	return 0;
}

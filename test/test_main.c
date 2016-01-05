/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include "test_main.h"
#include "test_debug.h"
#include "test_query.h"
#include "test_snippet.h"

int main(int argc, char **argv)
{
	test_query_person_contact(argv +1);
	test_query_person_number(argv +1);
	test_query_person_email(argv +1);
	test_query_with_query_person_contact(argv +1);
	test_query_with_query_person_number(argv +1);
	test_query_with_query_person_email(argv +1);

	test_snippet_with_query_person_contact(argv +1);
	test_snippet_with_query_person_number(argv +1);
	test_snippet_with_query_person_email(argv +1);
	return 0;
}

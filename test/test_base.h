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

#ifndef __TEST_BASE_H__
#define __TEST_BASE_H__

int test_base_insert_contact(char *first_name, char *last_name, char *number, int *out_id);
int test_base_delete_contact_with_id(int id);
int test_base_delete_contact(int argc, char **argv);

#endif /* __TEST_BASE_H__ */

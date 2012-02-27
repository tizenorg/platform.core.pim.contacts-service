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
#include <stdio.h>

#include <contacts-svc.h>

int insert_addrbook(int acc_id, int acc_type, int mode, const char *group_name)
{
	int ret;
	CTSvalue *ab;
	ab = contacts_svc_value_new(CTS_VALUE_ADDRESSBOOK);

	contacts_svc_value_set_int(ab, CTS_ADDRESSBOOK_VAL_ACC_ID_INT, acc_id);
	contacts_svc_value_set_int(ab, CTS_ADDRESSBOOK_VAL_ACC_TYPE_INT, acc_type);
	contacts_svc_value_set_int(ab, CTS_ADDRESSBOOK_VAL_MODE_INT, mode);
	contacts_svc_value_set_str(ab, CTS_ADDRESSBOOK_VAL_NAME_STR, group_name);

	ret = contacts_svc_insert_addressbook(ab);
	if (ret < CTS_SUCCESS)
		printf("contacts_svc_insert_addressbook() Failed\n");

	contacts_svc_value_free(ab);
	return ret;
}

void get_addrbook(int addressbook_id)
{
	int ret;
	const char *name;
	CTSvalue *ab = NULL;

	ret = contacts_svc_get_addressbook(addressbook_id, &ab);
	if (CTS_SUCCESS != ret) {
		printf("contacts_svc_get_addressbook() Failed\n");
		return;
	}

	printf("///////////%d//////////////\n",
			contacts_svc_value_get_int(ab, CTS_ADDRESSBOOK_VAL_ID_INT));
	printf("The related account ID : %d\n",
			contacts_svc_value_get_int(ab, CTS_ADDRESSBOOK_VAL_ACC_ID_INT));
	printf("The related account type : %d\n",
			contacts_svc_value_get_int(ab, CTS_ADDRESSBOOK_VAL_ACC_TYPE_INT));
	printf("permission : %d\n",
			contacts_svc_value_get_int(ab, CTS_ADDRESSBOOK_VAL_MODE_INT));

	name = contacts_svc_value_get_str(ab, CTS_ADDRESSBOOK_VAL_NAME_STR);
	if (name)
		printf("Name : %s\n", name);
	printf("//////////////////////////\n");

	contacts_svc_value_free(ab);
}

void update_addrbook(void)
{
	int ret;
	CTSvalue *ab = NULL;
	ret = contacts_svc_get_addressbook(2, &ab);
	if (CTS_SUCCESS != ret) {
		printf("contacts_svc_get_addressbook() Failed\n");
		return;
	}

	contacts_svc_value_set_str(ab, CTS_ADDRESSBOOK_VAL_NAME_STR,"Fixed-addressbook");

	ret = contacts_svc_update_addressbook(ab);
	if (ret < CTS_SUCCESS)
		printf("contacts_svc_update_addressbook() Failed\n");

	contacts_svc_value_free(ab);
}

void delete_addrbook(int addressbook_id)
{
	int ret;
	ret = contacts_svc_delete_addressbook(addressbook_id);
	if (CTS_SUCCESS != ret)
		printf("Error : contacts_svc_delete_addressbook() Failed(%d)\n", ret);
}

static int list_cb(CTSvalue *ab, void *user_data)
{
	const char *name;

	printf("///////////%d//////////////\n",
			contacts_svc_value_get_int(ab, CTS_LIST_ADDRESSBOOK_ID_INT));
	printf("The related account ID : %d\n",
			contacts_svc_value_get_int(ab, CTS_LIST_ADDRESSBOOK_ACC_ID_INT));
	printf("The related account type : %d\n",
			contacts_svc_value_get_int(ab, CTS_LIST_ADDRESSBOOK_ACC_TYPE_INT));
	printf("permission : %d\n",
			contacts_svc_value_get_int(ab, CTS_LIST_ADDRESSBOOK_MODE_INT));

	name = contacts_svc_value_get_str(ab, CTS_LIST_ADDRESSBOOK_NAME_STR);
	if (name)
		printf("Name : %s\n", name);
	printf("//////////////////////////\n");

	return CTS_SUCCESS;
}

void addrbook_list(void)
{
	int ret;

	printf("///////////0//////////////\n");
	printf("Name : %s\n", "Internal Addressbook(This is logical value)");
	printf("//////////////////////////\n");

	ret = contacts_svc_list_foreach(CTS_LIST_ALL_ADDRESSBOOK, list_cb, NULL);
	if (CTS_SUCCESS != ret) {
		printf("contacts_svc_list_foreach() Failed\n");
		return;
	}
}

void addrbook_list2(void)
{
	int ret, count;
	CTSiter *iter;

	count = contacts_svc_count_with_int(CTS_GET_COUNT_CONTACTS_IN_ADDRESSBOOK, 0);
	printf("Phone(%d)", count);

	ret = contacts_svc_get_list(CTS_LIST_ALL_ADDRESSBOOK, &iter);
	if (CTS_SUCCESS != ret) {
		printf("contacts_svc_get_list() Failed(%d)\n", ret);
		return;
	}

	while (CTS_SUCCESS == contacts_svc_iter_next(iter)) {
		int id;
		const char *name;
		CTSvalue *info;

		info = contacts_svc_iter_get_info(iter);
		id = contacts_svc_value_get_int(info, CTS_LIST_ADDRESSBOOK_ID_INT);
		name = contacts_svc_value_get_str(info, CTS_LIST_ADDRESSBOOK_NAME_STR);
		count = contacts_svc_count_with_int(CTS_GET_COUNT_CONTACTS_IN_ADDRESSBOOK, id);

		printf("%s(%d)", name, count);
	}
	contacts_svc_iter_remove(iter);
}

int main()
{
	int id;
	contacts_svc_connect();
	insert_addrbook(1, CTS_ADDRESSBOOK_TYPE_GOOGLE, CTS_ADDRESSBOOK_MODE_NONE, "test1");
	insert_addrbook(1, CTS_ADDRESSBOOK_TYPE_GOOGLE, CTS_ADDRESSBOOK_MODE_NONE, "test2");
	id = insert_addrbook(2, CTS_ADDRESSBOOK_TYPE_FACEBOOK, CTS_ADDRESSBOOK_MODE_READONLY,
			"facebook-test");
	get_addrbook(id);
	addrbook_list();
	update_addrbook();
	addrbook_list();
	delete_addrbook(id);
	addrbook_list();
	addrbook_list2();

	contacts_svc_disconnect();
	return 0;
}




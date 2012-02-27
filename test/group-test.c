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
#include <glib.h>
#include <contacts-svc.h>

void insert_group(const char *group_name)
{
	int ret;
	CTSvalue *group;
	group = contacts_svc_value_new(CTS_VALUE_GROUP);

	contacts_svc_value_set_str(group, CTS_GROUP_VAL_NAME_STR, group_name);
	contacts_svc_value_set_str(group, CTS_GROUP_VAL_RINGTONE_STR,"/tmp/test.mp3");

	ret = contacts_svc_insert_group(0, group);
	if (ret < CTS_SUCCESS)
		printf("contacts_svc_insert_group() Failed\n");

	contacts_svc_value_free(group);
}
void get_group(void)
{
	int ret;
	CTSvalue *group = NULL;
	ret = contacts_svc_get_group(2, &group);
	if (CTS_SUCCESS != ret) {
		printf("contacts_svc_get_group() Failed\n");
		return;
	}

	printf("Addressbook ID : %d\n",
			contacts_svc_value_get_int(group, CTS_GROUP_VAL_ADDRESSBOOK_ID_INT));
	printf("Name : %s\n",
			contacts_svc_value_get_str(group, CTS_GROUP_VAL_NAME_STR));
	if (contacts_svc_value_get_str(group, CTS_GROUP_VAL_RINGTONE_STR))
		printf("ringtone : %s\n",
				contacts_svc_value_get_str(group, CTS_GROUP_VAL_RINGTONE_STR));
}

void update_group(void)
{
	int ret;
	CTSvalue *group = NULL;
	ret = contacts_svc_get_group(2, &group);
	if (CTS_SUCCESS != ret) {
		printf("contacts_svc_get_group() Failed\n");
		return;
	}

	contacts_svc_value_set_str(group, CTS_GROUP_VAL_NAME_STR,"Fix-Friends");
	contacts_svc_value_set_str(group, CTS_GROUP_VAL_RINGTONE_STR,"/tmp/change.mp3");

	//free("Fix-Friends");
	//free("/tmp/change.mp3");
	ret = contacts_svc_update_group(group);
	if (ret < CTS_SUCCESS)
		printf("contacts_svc_update_group() Failed\n");

	contacts_svc_value_free(group);
}

void delete_group(void)
{
	int ret;
	ret = contacts_svc_delete_group(3);
	if (CTS_SUCCESS != ret)
		printf("Error : contacts_svc_delete_group() Failed(%d)\n", ret);
}

void delete_group_with_members(void)
{
	int ret;
	ret = contacts_svc_delete_group_with_members(1);
	if (CTS_SUCCESS != ret)
		printf("Error : contacts_svc_delete_group_with_members() Failed(%d)\n", ret);
}

void group_list(void)
{
	int ret;
	CTSiter *iter;
	ret = contacts_svc_get_list(CTS_LIST_ALL_GROUP, &iter);
	if (CTS_SUCCESS != ret) {
		printf("contacts_svc_get_list() Failed\n");
		return;
	}

	while (CTS_SUCCESS == contacts_svc_iter_next(iter))
	{
		CTSvalue *group;
		const char *name;
		group = contacts_svc_iter_get_info(iter);

		printf("%8d", contacts_svc_value_get_int(group, CTS_LIST_GROUP_ID_INT));
		printf("(%d)", contacts_svc_value_get_int(group, CTS_LIST_GROUP_ADDRESSBOOK_ID_INT));
		name = contacts_svc_value_get_str(group, CTS_LIST_GROUP_NAME_STR);
		if (name)
			printf("%s :", name);
		printf("\n");
		contacts_svc_value_free(group);
	}
	contacts_svc_iter_remove(iter);
}


int main()
{
	contacts_svc_connect();
	insert_group("Friends");
	insert_group("Home");
	get_group();
	update_group();
	group_list();
	delete_group();
	delete_group_with_members();
	contacts_svc_disconnect();
	return 0;
}


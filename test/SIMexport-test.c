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

int main(int argc, char *argv[])
{
	int pindex;
	int ret;
	if (argc < 2)
		return 0;

	printf("%d, %s, %s\n", argc, argv[0], argv[1]);
	pindex = atoi(argv[1]);
	if (pindex < 0)
		return 0;
	printf("person index : %d\n", pindex);
	contacts_svc_connect();

	ret = contacts_svc_export_sim(pindex);
	printf("contacts_svc_export_sim() return %d\n", ret);

	contacts_svc_disconnect();
	return 0;
}



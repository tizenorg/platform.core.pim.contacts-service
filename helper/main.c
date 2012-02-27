/*
 * Contacts Service Helper
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
#include <glib.h>
#include <string.h>
#include <contacts-svc.h>

#include "internal.h"
#include "schema-recovery.h"
#include "helper-socket.h"
#include "utils.h"
#include "sqlite.h"

int main(int argc, char **argv)
{
	int ret;
	GMainLoop *cts_helper_loop = NULL;

	helper_check_schema();
	if (2 <= argc && !strcmp(argv[1], "schema"))
		return CTS_SUCCESS;

	cts_helper_loop = g_main_loop_new (NULL, FALSE);
	h_retvm_if(NULL == cts_helper_loop, CTS_ERR_FAIL, "g_main_loop_new() Failed");

	ret = contacts_svc_connect();
	h_retvm_if(CTS_SUCCESS != ret, ret, "contacts_svc_connect() Failed(%d)", ret);

	helper_socket_init();
	helper_init_configuration();

	g_main_loop_run(cts_helper_loop);

	helper_final_configuration();
	ret = contacts_svc_disconnect();
	h_retvm_if(CTS_SUCCESS != ret, ret, "contacts_svc_disconnect() Failed(%d)", ret);

	g_main_loop_unref(cts_helper_loop);
	return CTS_SUCCESS;
}

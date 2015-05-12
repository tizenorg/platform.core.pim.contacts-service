/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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


#include "contacts.h"
#include "ctsvc_socket.h"
#include "ctsvc_service.h"
#include "ctsvc_server_bg.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_ipc_define.h"
#include "ctsvc_internal.h"
#include "ctsvc_notification.h"
#include "ctsvc_schema_recovery.h"
#include "ctsvc_server_update.h"
#include "ctsvc_server_zone.h"
#include "ctsvc_server_utils.h"
#include "ctsvc_zone.h"

vsm_context_h ctsvc_vsm_ctx = NULL;
static bool _ctsvc_zone_enabled = false;

void ctsvc_server_zone_finalize_zone(const char *zone_name)
{
	ctsvc_server_bg_remove_cb(zone_name);
	ctsvc_unset_client_access_info(zone_name);
	ctsvc_disconnect(zone_name);
}

char* ctsvc_server_zone_get_default_zone()
{
	if (_ctsvc_zone_enabled)
		return CTSVC_ZONE_NAME_PERSONAL;
	else
		return CTSVC_ZONE_NAME_HOST;
}

void ctsvc_server_zone_initialize_zone(const char *zone_name)
{
	CTS_FN_CALL;

	if (zone_name && *zone_name)
		_ctsvc_zone_enabled = true;

	int ret;

	ctsvc_noti_publish_socket_initialize(zone_name);

	ret = ctsvc_server_check_schema(zone_name);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_server_check_schema() Fail(%d)", ret);

	ret = ctsvc_server_db_update(zone_name);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_server_db_update() Fail(%d)", ret);

	ret = ctsvc_connect(zone_name);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_connect() Fail(%d)", ret);

	ctsvc_set_client_access_info(zone_name, "contacts-service", NULL);
	ctsvc_server_bg_add_cb(zone_name);
	ctsvc_server_bg_delete_start(zone_name);

	if ('\0' == *zone_name || STRING_EQUAL == g_strcmp0(zone_name, CTSVC_ZONE_NAME_PERSONAL)) {
		ret = ctsvc_server_init_configuration();
		WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_server_init_configuration() Fail(%d)", ret);
	}
}

static int _ctsvc_vsm_status_cb(vsm_zone_h zone, vsm_zone_state_t state, void *user_data)
{
	CTS_FN_CALL;

	const char *zone_name = vsm_get_zone_name(zone);
	CTS_DBG("state(%d) [%s]", state, zone_name);

	switch (state) {
	case VSM_ZONE_STATE_STARTING:
		CTS_DBG("STARTING");
		ctsvc_server_zone_initialize_zone(zone_name);
		break;
	case VSM_ZONE_STATE_STOPPED:
		CTS_DBG("STOPPED");
		break;
	case VSM_ZONE_STATE_RUNNING:
		CTS_DBG("RUNNING");
		break;
	case VSM_ZONE_STATE_STOPPING:
		CTS_DBG("STOPPING");
		ctsvc_server_zone_finalize_zone(zone_name);
		break;
	case VSM_ZONE_STATE_ABORTING:
		CTS_DBG("ABORTING");
		break;
	case VSM_ZONE_STATE_FREEZING:
		CTS_DBG("FREEZING");
		break;
	case VSM_ZONE_STATE_FROZEN:
		CTS_DBG("FROZEN");
		break;
	case VSM_ZONE_STATE_THAWED:
		CTS_DBG("THAWED");
		break;
	default:
		CTS_DBG("###### invalid status");
		break;
	}
CTS_FN_END;
	return 0;
}

static gboolean _ctsvc_server_zone_mainloop_cb(GIOChannel *channel, GIOCondition condition, void *data)
{
	CTS_FN_CALL;
	vsm_context_h ctx = data;
	vsm_enter_eventloop(ctx, 0, 0);
	return TRUE;
}

int ctsvc_server_zone_initialize(void)
{
	CTS_FN_CALL;
	int ret = 0;

	if (ctsvc_vsm_ctx) {
		CTS_DBG("already existed");
		ctsvc_server_zone_finalize();
	}

	vsm_context_h ctx = vsm_create_context();
	RETVM_IF(NULL == ctx, CONTACTS_ERROR_DB, "vsm_create_context() return NULL");

	ret = vsm_add_state_changed_callback(ctx, _ctsvc_vsm_status_cb, NULL);
	WARN_IF(ret < 0, "vsm_add_state_callback() Fail(%d)", ret);

	GIOChannel *channel = NULL;
	int fd = vsm_get_poll_fd(ctx);
	channel = g_io_channel_unix_new(fd);
	g_io_add_watch(channel, G_IO_IN, _ctsvc_server_zone_mainloop_cb, ctx);
	g_io_channel_unref(channel);

	ctsvc_vsm_ctx = ctx;

	CTS_FN_END;

	return CONTACTS_ERROR_NONE;
}

int ctsvc_server_zone_declare_link(void)
{
	CTS_FN_CALL;
	int ret = 0;

	if (NULL == ctsvc_vsm_ctx) {
		ret = ctsvc_server_zone_initialize();
		RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_zone_initialize() Fail(%d)", ret);
	}
	vsm_context_h ctx = ctsvc_vsm_ctx;

	ret = vsm_declare_link(ctx, CTSVC_SOCKET_PATH, CTSVC_SOCKET_PATH);
	RETVM_IF(ret < 0, ret, "vsm_declare_link() Fail(%d)", ret);

	ret = vsm_declare_link(ctx, CTSVC_IPC_SOCKET_PATH, CTSVC_IPC_SOCKET_PATH);
	RETVM_IF(ret < 0, ret, "vsm_declare_link() Fail(%d)", ret);

	ret = vsm_declare_link(ctx, CTSVC_IPC_SOCKET_PATH_FOR_CHANGE_SUBSCRIPTION, CTSVC_IPC_SOCKET_PATH_FOR_CHANGE_SUBSCRIPTION);
	RETVM_IF(ret < 0, ret, "vsm_declare_link() Fail(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

void ctsvc_server_zone_finalize(void)
{
	RET_IF(NULL == ctsvc_vsm_ctx);
	vsm_cleanup_context(ctsvc_vsm_ctx);
}

int ctsvc_server_zone_get_zone_name_by_pid(int pid, char **p_zone_name)
{
	if (NULL == ctsvc_vsm_ctx)
		ctsvc_server_zone_initialize();

	RETV_IF(pid < 0, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == p_zone_name, CONTACTS_ERROR_INVALID_PARAMETER);

	vsm_zone_h zone = NULL;
	zone = vsm_lookup_zone_by_pid(ctsvc_vsm_ctx, pid);
	RETVM_IF(NULL == zone, CONTACTS_ERROR_INVALID_PARAMETER, "vsm_lookup_zone_by_pid() return NULL");

	const char *zone_name = vsm_get_zone_name(zone);
	*p_zone_name = g_strdup(zone_name);

	return CONTACTS_ERROR_NONE;
}

static void _ctsvc_server_zone_get_activated_zone_iter_cb(vsm_zone_h zone, void *user_data)
{
	GList **list = user_data;

	// try to connect zone with before launched service.
	RET_IF(NULL == zone);
	const char *zone_name = vsm_get_zone_name(zone);
	RET_IF(NULL == zone_name);

	*list = g_list_append(*list, strdup(zone_name));
}

int ctsvc_server_zone_get_activated_zone_name_list(char ***p_zone_name_list, int *p_list_count)
{
	CTS_FN_CALL;
	int ret;
	GList *list = NULL;

	RETV_IF(NULL == p_zone_name_list, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == p_list_count, CONTACTS_ERROR_INVALID_PARAMETER);
	*p_zone_name_list = NULL;
	*p_list_count = 0;

	RETVM_IF(NULL == ctsvc_vsm_ctx, CONTACTS_ERROR_SYSTEM, "ctsvc_vsm_ctx is NULL");
	ret = vsm_iterate_zone(ctsvc_vsm_ctx, _ctsvc_server_zone_get_activated_zone_iter_cb, &list); // return value is handle
	WARN_IF(0 != ret, "vsm_iterate_zone() Fail(%d)", ret);

	GList *c;
	char **zone_name_list = NULL;
	int list_count = g_list_length(list);
	zone_name_list = calloc(list_count, sizeof(char *));

	int i=0;
	for (c=list;c;c=c->next) {
		char *zone_name = c->data;
		zone_name_list[i++] = zone_name;
	}
	g_list_free(list);

	*p_zone_name_list = zone_name_list;
	*p_list_count = list_count;

	return CONTACTS_ERROR_NONE;
}

vsm_zone_h ctsvc_server_zone_lookup_by_zone_name(const char *zone_name)
{
	RETVM_IF(NULL == ctsvc_vsm_ctx, NULL, "ctsvc_vsm_ctx is NULL");
	vsm_zone_h zone = vsm_lookup_zone_by_name(ctsvc_vsm_ctx, zone_name);
	return zone;
}

vsm_zone_h ctsvc_server_zone_join(vsm_zone_h zone)
{
	CTS_FN_CALL;
	RETVM_IF(NULL == zone, NULL, "zone is NULL");
	RETVM_IF(NULL == ctsvc_vsm_ctx, NULL, "ctsvc_vsm_ctx is NULL");
	vsm_zone_h zone_old = vsm_join_zone(zone);
	return zone_old;
}


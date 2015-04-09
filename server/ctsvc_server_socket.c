/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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

#include <unistd.h>
#include <stdlib.h>
#include <glib.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <pims-ipc-svc.h>

#ifdef ENABLE_SIM_FEATURE
#include <cynara-client.h>
#include <cynara-error.h>
#include <cynara-session.h>
#include <cynara-creds-socket.h>
#endif

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_struct.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_socket.h"
#include "ctsvc_server_socket.h"
#ifdef ENABLE_SIM_FEATURE
#include "ctsvc_mutex.h"
#include "ctsvc_server_sim.h"
#include "ctsvc_db_access_control.h"
#endif




static int sockfd = -1;

#ifdef ENABLE_SIM_FEATURE
struct client_info {
	char *smack;
	char *uid;
	char *client_session;
};
static GHashTable *_client_info_table = NULL; // key : socket_fd, value : struct client_info*
#endif

static inline int __ctsvc_server_socket_safe_write(int fd, char *buf, int buf_size)
{
	int ret, writed = 0;
	while (buf_size) {
		ret = write(fd, buf+writed, buf_size);
		if (-1 == ret) {
			if (EINTR == errno)
				continue;
			else
				return ret;
		}
		writed += ret;
		buf_size -= ret;
	}
	return writed;
}


static inline int __ctsvc_server_socket_safe_read(int fd, char *buf, int buf_size)
{
	int ret, read_size = 0;
	while (buf_size) {
		ret = read(fd, buf+read_size, buf_size);
		if (-1 == ret) {
			if (EINTR == errno)
				continue;
			else
				return ret;
		}
		read_size += ret;
		buf_size -= ret;
	}
	return read_size;
}

int ctsvc_server_socket_return(GIOChannel *src, int value, int attach_num, int *attach_size)
{
	CTS_FN_CALL;
	int ret;
	ctsvc_socket_msg_s msg = {0};

	//	RETVM_IF(CONTACTS_ERROR_SYSTEM == value, value, "Socket has problems");
	RETVM_IF(CTSVC_SOCKET_MSG_REQUEST_MAX_ATTACH < attach_num, CONTACTS_ERROR_INTERNAL,
			"Invalid msg(attach_num = %d)", attach_num);

	msg.type = CTSVC_SOCKET_MSG_TYPE_REQUEST_RETURN_VALUE;
	msg.val = value;
	msg.attach_num = attach_num;

	memcpy(msg.attach_sizes, attach_size, attach_num * sizeof(int));

	CTS_DBG("fd = %d, MSG_TYPE=%d, MSG_VAL=%d, MSG_ATTACH_NUM=%d,"
			"MSG_ATTACH1=%d, MSG_ATTACH2=%d, MSG_ATTACH3=%d, MSG_ATTACH4=%d",
			g_io_channel_unix_get_fd(src), msg.type, msg.val, msg.attach_num,
			msg.attach_sizes[0], msg.attach_sizes[1], msg.attach_sizes[2],
			msg.attach_sizes[3]);

	ret = __ctsvc_server_socket_safe_write(g_io_channel_unix_get_fd(src), (char *)&msg, sizeof(msg));
	RETVM_IF(-1 == ret, CONTACTS_ERROR_SYSTEM,
			"__ctsvc_server_socket_safe_write() Failed(errno = %d)", errno);

	return CONTACTS_ERROR_NONE;
}

#ifdef ENABLE_SIM_FEATURE
static void __ctsvc_server_socket_import_sim(GIOChannel *src, int size)
{
	CTS_FN_CALL;
	int ret;
	gsize len = 0;
	GError *gerr = NULL;
	char receiver[CTS_SQL_MAX_LEN] = {0};

	if (size > 0) {
		g_io_channel_read_chars(src, receiver, size, &len, &gerr);
		if (gerr) {
			CTS_ERR("g_io_channel_read_chars() Failed(%s)", gerr->message);
			g_error_free(gerr);
			return;
		}
		CTS_DBG("Receiver = %s(%d), read_size = %d", receiver, len, size);
	}

	if (len) {
		receiver[len] = '\0';
		CTS_DBG("sim_id %d", atoi(receiver));
		ret = ctsvc_server_sim_import_contact(src, atoi(receiver));
	}
	else {
		ret = ctsvc_server_sim_import_contact(src, 0);
	}

	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_server_sim_import_contact() Failed(%d)", ret);
		ctsvc_server_socket_return(src, ret, 0, NULL);
	}
}

static void __ctsvc_server_socket_get_sim_init_status(GIOChannel *src, int size)
{
	CTS_FN_CALL;

	int ret;
	gsize len = 0;
	GError *gerr = NULL;
	char receiver[CTS_SQL_MAX_LEN] = {0};

	if (size > 0) {
		g_io_channel_read_chars(src, receiver, size, &len, &gerr);
		if (gerr) {
			CTS_ERR("g_io_channel_read_chars() Failed(%s)", gerr->message);
			g_error_free(gerr);
			return;
		}
		CTS_DBG("Receiver = %s(%d), read_size = %d", receiver, len, size);
	}

	if (len) {
		receiver[len] = '\0';
		CTS_DBG("sim_id : %d", atoi(receiver));
		ret = ctsvc_server_socket_get_sim_init_status(src, atoi(receiver));
	}
	else {
		ret = ctsvc_server_socket_get_sim_init_status(src, 0);
	}

	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_server_socket_get_sim_init_status() Failed(%d)", ret);
		ctsvc_server_socket_return(src, ret, 0, NULL);
	}
}

int ctsvc_server_socket_return_sim_int(GIOChannel *src, int value)
{
	CTS_FN_CALL;
	int ret;
	int str_len;
	char count_str[CTS_SQL_MAX_LEN] = {0};

	str_len = snprintf(count_str, sizeof(count_str), "%d", value);
	ret = ctsvc_server_socket_return(src, CONTACTS_ERROR_NONE, 1, &str_len);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_server_socket_return() Failed(%d)", ret);
	CTS_DBG("count_str : %s", count_str);
	ret = __ctsvc_server_socket_safe_write(g_io_channel_unix_get_fd(src), count_str, str_len);
	RETVM_IF(-1 == ret, CONTACTS_ERROR_SYSTEM, "__ctsvc_server_socket_safe_write() Failed(errno = %d)", errno);

	return CONTACTS_ERROR_NONE;
}

static void __ctsvc_server_socket_read_flush(GIOChannel *src, int size)
{
	gsize len;
	GError *gerr = NULL;
	char receiver[CTS_SQL_MAX_LEN] = {0};

	if (size <= 0)
		return;

	g_io_channel_read_chars(src, receiver, size, &len, &gerr);
	if (gerr) {
		CTS_ERR("g_io_channel_read_chars() Failed(%s)", gerr->message);
		g_error_free(gerr);
	}
}
#endif // ENABLIE_SIM_FEATURE


#ifdef ENABLE_SIM_FEATURE
static cynara *_cynara = NULL;
static int _ctsvc_server_initialize_cynara()
{
	int ret;
	ctsvc_mutex_lock(CTS_MUTEX_CYNARA);
	ret = cynara_initialize(&_cynara, NULL);
	ctsvc_mutex_unlock(CTS_MUTEX_CYNARA);
	if (CYNARA_API_SUCCESS != ret) {
		char errmsg[1024] = {0};
		cynara_strerror(ret, errmsg, sizeof(errmsg));
		CTS_ERR("cynara_initialize() Fail(%d,%s)", ret, errmsg);
		return CONTACTS_ERROR_SYSTEM;
	}
	return CONTACTS_ERROR_NONE;
}
#endif

#ifdef ENABLE_SIM_FEATURE
static void _ctsvc_server_finalize_cynara()
{
	int ret;

	ctsvc_mutex_lock(CTS_MUTEX_CYNARA);
	ret = cynara_finish(_cynara);
	_cynara = NULL;
	ctsvc_mutex_unlock(CTS_MUTEX_CYNARA);
	if (CYNARA_API_SUCCESS != ret) {
		char errmsg[1024] = {0};
		cynara_strerror(ret, errmsg, sizeof(errmsg));
		CTS_ERR("cynara_finish() Fail(%d,%s)", ret, errmsg);
	}
}
#endif

#ifdef ENABLE_SIM_FEATURE
static bool _ctsvc_server_check_privilege(struct client_info *info, const char *privilege)
{
	RETVM_IF(NULL == info, false, "info is NULL");

	ctsvc_mutex_lock(CTS_MUTEX_CYNARA);
	int ret = cynara_check(_cynara, info->smack, info->client_session, info->uid, privilege);
	ctsvc_mutex_unlock(CTS_MUTEX_CYNARA);
	if (CYNARA_API_ACCESS_ALLOWED == ret)
		return true;

	return false;
}
#endif

static gboolean __ctsvc_server_socket_request_handler(GIOChannel *src, GIOCondition condition,
		gpointer data)
{
	int ret;
	int fd;
#ifdef ENABLE_SIM_FEATURE
	bool have_read_permission = false;
	bool have_write_permission = false;
#endif // ENABLE_SIM_FEATURE

	ctsvc_socket_msg_s msg = {0};
	CTS_FN_CALL;
	fd = g_io_channel_unix_get_fd(src);

	if (G_IO_HUP & condition) {
#ifdef ENABLE_SIM_FEATURE
		ctsvc_mutex_lock(CTS_MUTEX_SOCKET_CLIENT_INFO);
		if (_client_info_table)
			g_hash_table_remove(_client_info_table, GINT_TO_POINTER(fd));
		ctsvc_mutex_unlock(CTS_MUTEX_SOCKET_CLIENT_INFO);
#endif
		close(fd);
		return FALSE;
	}

	ret = __ctsvc_server_socket_safe_read(fd, (char *)&msg, sizeof(msg));
	RETVM_IF(-1 == ret, TRUE, "__ctsvc_server_socket_safe_read() Failed(errno = %d)", errno);

	CTS_DBG("attach number = %d", msg.attach_num);

#ifdef ENABLE_SIM_FEATURE
	ctsvc_mutex_lock(CTS_MUTEX_SOCKET_CLIENT_INFO);
	struct client_info *info = g_hash_table_lookup(_client_info_table, GINT_TO_POINTER(fd));
	have_read_permission = _ctsvc_server_check_privilege(info, CTSVC_PRIVILEGE_CONTACT_READ);
	if (!have_read_permission)
		INFO("fd(%d) : does not have contact read permission", fd);

	have_write_permission = _ctsvc_server_check_privilege(info, CTSVC_PRIVILEGE_CONTACT_WRITE);
	if (!have_write_permission)
		INFO("fd(%d) : does not have contact write permission", fd);
	ctsvc_mutex_unlock(CTS_MUTEX_SOCKET_CLIENT_INFO);
#endif // ENABLE_SIM_FEATURE

	switch (msg.type) {
#ifdef ENABLE_SIM_FEATURE
	case CTSVC_SOCKET_MSG_TYPE_REQUEST_IMPORT_SIM:
		if (!have_write_permission) {
			CTS_ERR("write permission denied");
			__ctsvc_server_socket_read_flush(src, msg.attach_sizes[0]);		// sim_id
			ctsvc_server_socket_return(src, CONTACTS_ERROR_PERMISSION_DENIED, 0, NULL);
			return TRUE;
		}
		__ctsvc_server_socket_import_sim(src, msg.attach_sizes[0]);
		break;
	case CTSVC_SOCKET_MSG_TYPE_REQUEST_SIM_INIT_COMPLETE:
		if (!have_read_permission) {
			CTS_ERR("read permission denied");
			__ctsvc_server_socket_read_flush(src, msg.attach_sizes[0]);		// sim_id
			ctsvc_server_socket_return(src, CONTACTS_ERROR_PERMISSION_DENIED, 0, NULL);
			return TRUE;
		}
		__ctsvc_server_socket_get_sim_init_status(src, msg.attach_sizes[0]);
		break;
#endif // ENABLE_SIM_FEATURE
	default:
		CTS_ERR("Unknown request type(%d)", msg.type);
		break;
	}
	return TRUE;
}

#ifdef ENABLE_SIM_FEATURE
static void _ctsvc_server_destroy_client_info(gpointer p)
{
	struct client_info *info = p;

	if (NULL == info)
		return;
	free(info->smack);
	free(info->uid);
	free(info->client_session);
	free(info);
}
#endif

#ifdef ENABLE_SIM_FEATURE
static int _ctsvc_server_create_client_info(int fd, struct client_info **p_info)
{
	int ret;
	pid_t pid;
	char errmsg[1024] = {0};

	struct client_info *info = calloc(1, sizeof(struct client_info));
	if (NULL == info) {
		CTS_ERR("calloc() return NULL");
		return CONTACTS_ERROR_SYSTEM;
	}

	ret = cynara_creds_socket_get_client(fd, CLIENT_METHOD_SMACK, &(info->smack));
	if (CYNARA_API_SUCCESS != ret) {
		cynara_strerror(ret, errmsg, sizeof(errmsg));
		CTS_ERR("cynara_creds_socket_get_client() Fail(%d,%s)", ret, errmsg);
		_ctsvc_server_destroy_client_info(info);
		return CONTACTS_ERROR_SYSTEM;
	}

	ret = cynara_creds_socket_get_user(fd, USER_METHOD_UID, &(info->uid));
	if (CYNARA_API_SUCCESS != ret) {
		cynara_strerror(ret, errmsg, sizeof(errmsg));
		CTS_ERR("cynara_creds_socket_get_user() Fail(%d,%s)", ret, errmsg);
		_ctsvc_server_destroy_client_info(info);
		return CONTACTS_ERROR_SYSTEM;
	}

	ret = cynara_creds_socket_get_pid(fd, &pid);
	if (CYNARA_API_SUCCESS != ret) {
		cynara_strerror(ret, errmsg, sizeof(errmsg));
		CTS_ERR("cynara_creds_socket_get_pid() Fail(%d,%s)", ret, errmsg);
		_ctsvc_server_destroy_client_info(info);
		return CONTACTS_ERROR_SYSTEM;
	}

	info->client_session = cynara_session_from_pid(pid);
	if (NULL == info->client_session) {
		CTS_ERR("cynara_session_from_pid() return NULL");
		_ctsvc_server_destroy_client_info(info);
		return CONTACTS_ERROR_SYSTEM;
	}
	*p_info = info;

	return CONTACTS_ERROR_NONE;
}
#endif


static gboolean __ctsvc_server_socket_handler(GIOChannel *src,
		GIOCondition condition, gpointer data)
{
	CTS_FN_CALL;
	int ret;

	GIOChannel *channel;
	int client_sockfd, sockfd = (int)data;
	struct sockaddr_un clientaddr;
	socklen_t client_len = sizeof(clientaddr);

	client_sockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &client_len);
	RETVM_IF(-1 == client_sockfd, TRUE, "accept() Failed(errno = %d)", errno);

#ifdef ENABLE_SIM_FEATURE
	if (NULL == _client_info_table)
		_client_info_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, _ctsvc_server_destroy_client_info);
	struct client_info *info = NULL;
	ret = _ctsvc_server_create_client_info(client_sockfd, &info);
	if (CONTACTS_ERROR_NONE != ret)
		CTS_ERR("_create_client_info() Fail(%d)", ret);
	else
		g_hash_table_insert(_client_info_table, GINT_TO_POINTER(client_sockfd), info);
#endif

	channel = g_io_channel_unix_new(client_sockfd);
	g_io_add_watch(channel, G_IO_IN|G_IO_HUP, __ctsvc_server_socket_request_handler, NULL);
	g_io_channel_unref(channel);

	return TRUE;
}

int ctsvc_server_socket_init(void)
{
	CTS_FN_CALL;

	int ret;
	struct sockaddr_un addr;
	GIOChannel *gio;

#ifdef ENABLE_SIM_FEATURE
	_ctsvc_server_initialize_cynara();
#endif

	unlink(CTSVC_SOCKET_PATH);

	bzero(&addr, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", CTSVC_SOCKET_PATH);

	sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	RETVM_IF(-1 == sockfd, CONTACTS_ERROR_SYSTEM, "socket() Failed(errno = %d)", errno);

	ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	RETVM_IF(-1 == ret, CONTACTS_ERROR_SYSTEM, "bind() Failed(errno = %d)", errno);

	ret = chown(CTSVC_SOCKET_PATH, getuid(), CTS_SECURITY_FILE_GROUP);
	if (0 != ret)
		CTS_ERR("chown(%s) Failed(%d)", CTSVC_SOCKET_PATH, ret);

	ret = chmod(CTSVC_SOCKET_PATH, CTS_SECURITY_DEFAULT_PERMISSION);
	if (0 != ret)
		CTS_ERR("chmod(%s) Failed(%d)", CTSVC_SOCKET_PATH, ret);

	ret = listen(sockfd, 30);
	if (-1 == ret) {
		close(sockfd);
		CTS_ERR("listen() Failed(errno = %d)", errno);
		return CONTACTS_ERROR_SYSTEM;
	}

	gio = g_io_channel_unix_new(sockfd);
	g_io_add_watch(gio, G_IO_IN, __ctsvc_server_socket_handler, (gpointer)sockfd);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_server_socket_deinit(void)
{
#ifdef ENABLE_SIM_FEATURE
	_ctsvc_server_finalize_cynara();
#endif
	if (sockfd != -1)
		close(sockfd);
	return CONTACTS_ERROR_NONE;
}

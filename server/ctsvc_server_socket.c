/*
 * Contacts Service Helper
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
#include <security-server.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_struct.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_socket.h"
#include "ctsvc_server_socket.h"
#include "ctsvc_server_sim.h"

static inline int __ctsvc_server_socket_safe_write(int fd, char *buf, int buf_size)
{
	int ret, writed=0;
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
	int ret, read_size=0;
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

static void __ctsvc_server_socket_import_sim(GIOChannel *src)
{
	CTS_FN_CALL;
	int ret;

	ret = ctsvc_server_sim_import(src);
	if (CONTACTS_ERROR_NONE != ret) {
		CTS_ERR("ctsvc_server_sim_import() Failed(%d)", ret);
		ctsvc_server_socket_return(src, ret, 0, NULL);
	}
}

static void __ctsvc_server_socket_get_sim_init_status(GIOChannel *src)
{
	CTS_FN_CALL;

	int ret;
	ret = ctsvc_server_socket_return_sim_int(src, ctsvc_server_sim_get_init_completed());
	RETM_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_server_socket_return_sim_int() Failed(%d)", ret);
}

int ctsvc_server_socket_return_sim_int(GIOChannel *src, int value)
{
	CTS_FN_CALL;
	int ret;
	int str_len;
	char count_str[CTS_SQL_MAX_LEN] = {0};

	str_len = snprintf(count_str, sizeof(count_str), "%d", value);
	ret = ctsvc_server_socket_return(src, CONTACTS_ERROR_NONE, 1, &str_len);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, CONTACTS_ERROR_SYSTEM, "ctsvc_server_socket_return() Failed(%d)", ret);
	CTS_DBG("count_str : %s", count_str);
	ret = __ctsvc_server_socket_safe_write(g_io_channel_unix_get_fd(src), count_str, str_len);
	RETVM_IF(-1 == ret, CONTACTS_ERROR_SYSTEM, "__ctsvc_server_socket_safe_write() Failed(errno = %d)", errno);

	return CONTACTS_ERROR_NONE;
}

static gboolean __ctsvc_server_socket_request_handler(GIOChannel *src, GIOCondition condition,
		gpointer data)
{
	int ret;
	int fd;
	bool have_read_permission = false;
	bool have_write_permission = false;

	ctsvc_socket_msg_s msg = {0};
	CTS_FN_CALL;

	if (G_IO_HUP & condition) {
		close(g_io_channel_unix_get_fd(src));
		return FALSE;
	}

	fd = g_io_channel_unix_get_fd(src);
	ret = __ctsvc_server_socket_safe_read(fd, (char *)&msg, sizeof(msg));
	RETVM_IF(-1 == ret, TRUE, "__ctsvc_server_socket_safe_read() Failed(errno = %d)", errno);

	CTS_DBG("attach number = %d", msg.attach_num);

	if (SECURITY_SERVER_API_SUCCESS == security_server_check_privilege_by_sockfd(fd, "contacts-service::svc", "r"))
		have_read_permission = true;
	if (SECURITY_SERVER_API_SUCCESS == security_server_check_privilege_by_sockfd(fd, "contacts-service::svc", "w"))
		have_write_permission = true;

	switch (msg.type) {
	case CTSVC_SOCKET_MSG_TYPE_REQUEST_IMPORT_SIM:
		if (!have_write_permission) {
			CTS_ERR("write permission denied");
			ctsvc_server_socket_return(src, CONTACTS_ERROR_PERMISSION_DENIED, 0, NULL);
			return TRUE;
		}
		__ctsvc_server_socket_import_sim(src);
		break;
	case CTSVC_SOCKET_MSG_TYPE_REQUEST_SIM_INIT_COMPLETE:
		if (!have_read_permission) {
			CTS_ERR("read permission denied");
			ctsvc_server_socket_return(src, CONTACTS_ERROR_PERMISSION_DENIED, 0, NULL);
			return TRUE;
		}
		__ctsvc_server_socket_get_sim_init_status(src);
		break;
	default:
		CTS_ERR("Unknown request type(%d)", msg.type);
		break;
	}
	return TRUE;
}

static gboolean __ctsvc_server_socket_handler(GIOChannel *src,
		GIOCondition condition, gpointer data)
{
	CTS_FN_CALL;

	GIOChannel *channel;
	int client_sockfd, sockfd = (int)data;
	struct sockaddr_un clientaddr;
	socklen_t client_len = sizeof(clientaddr);

	client_sockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &client_len);
	RETVM_IF(-1 == client_sockfd, TRUE, "accept() Failed(errno = %d)", errno);

	channel = g_io_channel_unix_new(client_sockfd);
	g_io_add_watch(channel, G_IO_IN|G_IO_HUP, __ctsvc_server_socket_request_handler, NULL);
	g_io_channel_unref(channel);

	return TRUE;
}

int ctsvc_server_socket_init(void)
{
	CTS_FN_CALL;

	int sockfd, ret;
	struct sockaddr_un addr;
	GIOChannel *gio;

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
	RETVM_IF(-1 == ret, CONTACTS_ERROR_SYSTEM, "listen() Failed(errno = %d)", errno);

	gio = g_io_channel_unix_new(sockfd);
	g_io_add_watch(gio, G_IO_IN, __ctsvc_server_socket_handler, (gpointer)sockfd);

	return CONTACTS_ERROR_NONE;
}


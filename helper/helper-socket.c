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
#include <unistd.h>
#include <stdlib.h>
#include <glib.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <contacts-svc.h>
#include <sd-daemon.h>

#include "internal.h"
#include "cts-schema.h"
#include "cts-sqlite.h"
#include "cts-socket.h"
#include "helper-socket.h"
#include "sim.h"
#include "normalize.h"
#include "utils.h"


static inline int helper_safe_write(int fd, char *buf, int buf_size)
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


static inline int helper_safe_read(int fd, char *buf, int buf_size)
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


static void helper_discard_msg(GIOChannel *src, int size)
{
	gsize len;
	GError *gerr = NULL;
	char dummy[CTS_SQL_MAX_LEN];

	while (size) {
		if (sizeof(dummy) < size) {
			g_io_channel_read_chars(src, dummy, sizeof(dummy), &len, &gerr);
			if (gerr) {
				ERR("g_io_channel_read_chars() Failed(%s)", gerr->message);
				g_error_free(gerr);
				return;
			}

			size -= len;
		}
		else {
			g_io_channel_read_chars(src, dummy, size, &len, &gerr);
			if (gerr) {
				ERR("g_io_channel_read_chars() Failed(%s)", gerr->message);
				g_error_free(gerr);
				return;
			}

			size -= len;
		}
	}
}

int helper_socket_return(GIOChannel *src, int value, int attach_num, int *attach_size)
{
	int ret;
	cts_socket_msg msg={0};

	h_retvm_if(CTS_ERR_SOCKET_FAILED == value, value, "Socket has problems");
	h_retvm_if(CTS_REQUEST_MAX_ATTACH < attach_num, CTS_ERR_ARG_INVALID,
			"Invalid msg(attach_num = %d)", attach_num);

	msg.type = CTS_REQUEST_RETURN_VALUE;
	msg.val = value;
	msg.attach_num = attach_num;

	memcpy(msg.attach_sizes, attach_size, attach_num * sizeof(int));

	HELPER_DBG("fd = %d, MSG_TYPE=%d, MSG_VAL=%d, MSG_ATTACH_NUM=%d,"
			"MSG_ATTACH1=%d, MSG_ATTACH2=%d, MSG_ATTACH3=%d, MSG_ATTACH4=%d",
			g_io_channel_unix_get_fd(src), msg.type, msg.val, msg.attach_num,
			msg.attach_sizes[0], msg.attach_sizes[1], msg.attach_sizes[2],
			msg.attach_sizes[3]);

	ret = helper_safe_write(g_io_channel_unix_get_fd(src), (char *)&msg, sizeof(msg));
	h_retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED,
			"helper_safe_write() Failed(errno = %d)", errno);

	return CTS_SUCCESS;
}

static void helper_handle_import_sim(GIOChannel *src)
{
	int ret;

	ret = helper_sim_read_pb_record(src);
	if (CTS_SUCCESS != ret) {
		ERR("helper_sim_read_pb_record() Failed(%d)", ret);
		helper_socket_return(src, ret, 0, NULL);
	}
}

static void helper_handle_export_sim(GIOChannel *src, int size)
{
	int ret;
	gsize len;
	GError *gerr = NULL;
	char receiver[CTS_SQL_MAX_LEN];

	g_io_channel_read_chars(src, receiver, size, &len, &gerr);
	if (gerr) {
		ERR("g_io_channel_read_chars() Failed(%s)", gerr->message);
		g_error_free(gerr);
		return;
	}
	HELPER_DBG("Receiver = %s(%d), read_size = %d", receiver, len, size);

	if (len) {
		receiver[len] = '\0';
		HELPER_DBG("export contact %d", atoi(receiver));
		ret = helper_sim_write_pb_record(src, atoi(receiver));
		if (CTS_SUCCESS != ret) {
			ERR("helper_sim_write_pb_record() Failed(%d)", ret);
			helper_socket_return(src, ret, 0, NULL);
		}
	}
}

static int helper_normalize(GIOChannel *src, int read_size,
		char *dest, int dest_size)
{
	int ret=CTS_SUCCESS;
	gsize len;
	GError *gerr = NULL;
	char receiver[CTS_SQL_MAX_LEN];

	g_io_channel_read_chars(src, receiver, read_size, &len, &gerr);
	if (gerr) {
		ERR("g_io_channel_read_chars() Failed(%s)", gerr->message);
		g_error_free(gerr);
		return CTS_ERR_SOCKET_FAILED;
	}
	HELPER_DBG("Receiver = %s(%d), read_size = %d", receiver, len, read_size);

	if (len) {
		receiver[len] = '\0';
		ret = helper_normalize_str(receiver, dest, dest_size);
		h_retvm_if(ret < CTS_SUCCESS, ret, "helper_normalize_str() Failed(%d)", ret);
		HELPER_DBG("Normalized text of %s = %s(%d)", receiver, dest, strlen(dest));
	}
	return ret;
}

static int helper_collation(GIOChannel *src, int read_size,
		char *dest, int dest_size)
{
	int ret = CTS_SUCCESS;
	gsize len;
	GError *gerr = NULL;
	char receiver[CTS_SQL_MAX_LEN];

	g_io_channel_read_chars(src, receiver, read_size, &len, &gerr);
	if (gerr) {
		ERR("g_io_channel_read_chars() Failed(%s)", gerr->message);
		g_error_free(gerr);
		return CTS_ERR_SOCKET_FAILED;
	}
	HELPER_DBG("Receiver = %s(%d), read_size = %d", receiver, len, read_size);

	if (len) {
		receiver[len] = '\0';
		ret = helper_collation_str(receiver, dest, dest_size);
		h_retvm_if(ret < CTS_SUCCESS, ret, "helper_collation_str() Failed(%d)", ret);
		HELPER_DBG("Sortkey of %s : %s, %d", receiver, dest, strlen(dest));
	}
	return ret;
}

static void helper_handle_normalize_str(GIOChannel *src, int size)
{
	HELPER_FN_CALL;
	int str_len, ret;
	char normalized_str[CTS_SQL_MAX_LEN];

	ret = helper_normalize(src, size, normalized_str, sizeof(normalized_str));
	if (ret < CTS_SUCCESS) {
		ERR("helper_normalize() Failed(%d)", ret);
		helper_socket_return(src, ret, 0, NULL);
		return;
	}
	str_len = strlen(normalized_str);

	ret = helper_socket_return(src, CTS_SUCCESS, 1, &str_len);
	h_retm_if(CTS_SUCCESS != ret, "helper_socket_return() Failed(%d)", ret);
	ret = helper_safe_write(g_io_channel_unix_get_fd(src), normalized_str, str_len);
	h_retm_if(-1 == ret, "helper_safe_write() Failed(errno = %d)", errno);
}

static void helper_handle_normalize_name(GIOChannel *src, int* sizes)
{
	HELPER_FN_CALL;
	int fd, ret;
	int lang_type = 0;
	int msg_size_buf[CTS_REQUEST_MAX_ATTACH]={0};
	char normalized_first[CTS_SQL_MAX_LEN], normalized_last[CTS_SQL_MAX_LEN];
	char sortkey[CTS_SQL_MAX_LEN];

	if (sizes[CTS_NN_FIRST])
	{
		ret = helper_normalize(src, sizes[CTS_NN_FIRST], normalized_first,
				sizeof(normalized_first));
		if (ret < CTS_SUCCESS) {
			ERR("helper_normalize() Failed(%d)", ret);
			helper_discard_msg(src,	sizes[CTS_NN_LAST] + sizes[CTS_NN_SORTKEY]);
			helper_socket_return(src, ret, 0, NULL);
			return;
		}
		lang_type = ret;

		msg_size_buf[CTS_NN_FIRST] = strlen(normalized_first);
	}

	if (sizes[CTS_NN_LAST])
	{
		ret = helper_normalize(src, sizes[CTS_NN_LAST], normalized_last,
				sizeof(normalized_last));
		if (ret < CTS_SUCCESS) {
			ERR("helper_normalize() Failed(%d)", ret);
			helper_discard_msg(src, sizes[CTS_NN_SORTKEY]);
			helper_socket_return(src, ret, 0, NULL);
			return;
		}
		if (lang_type < ret) lang_type = ret;

		msg_size_buf[CTS_NN_LAST] = strlen(normalized_last);
	}

	if (sizes[CTS_NN_SORTKEY])
	{
		ret = helper_collation(src, sizes[CTS_NN_SORTKEY], sortkey, sizeof(sortkey));
		if (ret < CTS_SUCCESS) {
			ERR("helper_collation() Failed(%d)", ret);
			helper_socket_return(src, ret, 0, NULL);
		}
		msg_size_buf[CTS_NN_SORTKEY] = strlen(sortkey);
	}

	ret = helper_socket_return(src, lang_type, 3, msg_size_buf);
	h_retm_if(CTS_SUCCESS != ret, "helper_socket_return() Failed(%d)", ret);

	fd = g_io_channel_unix_get_fd(src);
	ret = helper_safe_write(fd, normalized_first, msg_size_buf[0]);
	h_retm_if(-1 == ret, "helper_safe_write() Failed(errno = %d)", errno);
	ret = helper_safe_write(fd, normalized_last, msg_size_buf[1]);
	h_retm_if(-1 == ret, "helper_safe_write() Failed(errno = %d)", errno);
	ret = helper_safe_write(fd, sortkey, msg_size_buf[2]);
	h_retm_if(-1 == ret, "helper_safe_write() Failed(errno = %d)", errno);
}

static gboolean request_handler(GIOChannel *src, GIOCondition condition,
		gpointer data)
{
	int ret;
	cts_socket_msg msg={0};
	HELPER_FN_CALL;

	if (G_IO_HUP & condition) {
		close(g_io_channel_unix_get_fd(src));
		return FALSE;
	}

	ret = helper_safe_read(g_io_channel_unix_get_fd(src), (char *)&msg, sizeof(msg));
	h_retvm_if(-1 == ret, TRUE, "helper_safe_read() Failed(errno = %d)", errno);

	HELPER_DBG("attach number = %d, attach1 = %d, attach2 = %d, attach3 = %d",
			msg.attach_num, msg.attach_sizes[CTS_NN_FIRST],
			msg.attach_sizes[CTS_NN_LAST], msg.attach_sizes[CTS_NN_SORTKEY]);

	switch (msg.type)
	{
	case CTS_REQUEST_IMPORT_SIM:
		helper_handle_import_sim(src);
		break;
	case CTS_REQUEST_EXPORT_SIM:
		helper_handle_export_sim(src, msg.attach_sizes[0]);
		break;
	case CTS_REQUEST_NORMALIZE_STR:
		if (CTS_NS_ATTACH_NUM != msg.attach_num) {
			ERR("Invalid CTS_NS_ATTACH_NUM = %d", msg.attach_num);
			helper_socket_return(src, CTS_ERR_MSG_INVALID, 0, NULL);
		}
		else
			helper_handle_normalize_str(src, msg.attach_sizes[0]);
		break;
	case CTS_REQUEST_NORMALIZE_NAME:
		if (CTS_NN_ATTACH_NUM != msg.attach_num) {
			ERR("Invalid CTS_NN_ATTACH_NUM = %d", msg.attach_num);
			helper_socket_return(src, CTS_ERR_MSG_INVALID, 0, NULL);
		}else if (!msg.attach_sizes[CTS_NN_FIRST] && !msg.attach_sizes[CTS_NN_LAST]) {
			ERR("No attach names(CTS_NN_FIRST size = %d, CTS_NN_LAST size = %d ",
					msg.attach_sizes[CTS_NN_FIRST], msg.attach_sizes[CTS_NN_LAST]);
			helper_socket_return(src, CTS_ERR_MSG_INVALID, 0, NULL);
		}
		else
			helper_handle_normalize_name(src, msg.attach_sizes);
		break;
	default:
		ERR("Unknown request type(%d)", msg.type);
		break;
	}
	return TRUE;
}

static gboolean socket_handler(GIOChannel *src,
		GIOCondition condition, gpointer data)
{
	GIOChannel *channel;
	int client_sockfd, sockfd = (int)data;
	struct sockaddr_un clientaddr;
	socklen_t client_len = sizeof(clientaddr);

	HELPER_FN_CALL;

	client_sockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &client_len);
	h_retvm_if(-1 == client_sockfd, TRUE, "accept() Failed(errno = %d)", errno);

	channel = g_io_channel_unix_new(client_sockfd);
	g_io_add_watch(channel, G_IO_IN|G_IO_HUP, request_handler, NULL);
	g_io_channel_unref(channel);

	return TRUE;
}

int helper_socket_init(void)
{
	int sockfd, ret;
	struct sockaddr_un addr;
	GIOChannel *gio;

	if (sd_listen_fds(1) == 1 && sd_is_socket_unix(SD_LISTEN_FDS_START, SOCK_STREAM, -1, CTS_SOCKET_PATH, 0) > 0) {
		sockfd = SD_LISTEN_FDS_START;
	} else {
		unlink(CTS_SOCKET_PATH);

		bzero(&addr, sizeof(addr));
		addr.sun_family = AF_UNIX;
		snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", CTS_SOCKET_PATH);

		sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
		h_retvm_if(-1 == sockfd, CTS_ERR_SOCKET_FAILED, "socket() Failed(errno = %d)", errno);

		ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
		h_retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "bind() Failed(errno = %d)", errno);

		chown(CTS_SOCKET_PATH, getuid(), CTS_SECURITY_FILE_GROUP);
		chmod(CTS_SOCKET_PATH, CTS_SECURITY_DEFAULT_PERMISSION);

		ret = listen(sockfd, 30);
		h_retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "listen() Failed(errno = %d)", errno);
	}

	gio = g_io_channel_unix_new(sockfd);
	g_io_add_watch(gio, G_IO_IN, socket_handler, (gpointer)sockfd);

	return CTS_SUCCESS;
}

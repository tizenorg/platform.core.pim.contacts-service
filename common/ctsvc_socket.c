/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
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
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_socket.h"
#include "ctsvc_mutex.h"
#include "ctsvc_inotify.h"

static int ctsvc_conn_refcnt = 0;
static int cts_sockfd = -1;

int ctsvc_socket_init(void)
{
	int ret;
	struct sockaddr_un caddr;

	if (0 < ctsvc_conn_refcnt) {
		ctsvc_conn_refcnt++;
		return  CONTACTS_ERROR_NONE;
	}

	bzero(&caddr, sizeof(caddr));
	caddr.sun_family = AF_UNIX;
	snprintf(caddr.sun_path, sizeof(caddr.sun_path), "%s", CTSVC_SOCKET_PATH);

	cts_sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	RETVM_IF(-1 == cts_sockfd, CONTACTS_ERROR_IPC,
			"socket() Failed(errno = %d)", errno);

	ret = connect(cts_sockfd, (struct sockaddr *)&caddr, sizeof(caddr));
	if (-1 == ret) {
		CTS_ERR("connect() Failed(errno = %d)", errno);
		close(cts_sockfd);
		cts_sockfd = -1;
		return CONTACTS_ERROR_IPC;
	}

	ctsvc_conn_refcnt++;
	return CONTACTS_ERROR_NONE;
}

void ctsvc_socket_final(void)
{
	if (1 < ctsvc_conn_refcnt) {
		CTS_DBG("socket ref count : %d", ctsvc_conn_refcnt);
		ctsvc_conn_refcnt--;
		return;
	}
	else if (ctsvc_conn_refcnt < 1) {
		CTS_DBG("Please call connection API. socket ref count : %d", ctsvc_conn_refcnt);
		return;
	}
	ctsvc_conn_refcnt--;

	close(cts_sockfd);
	cts_sockfd = -1;
}

static inline int __ctsvc_safe_write(int fd, const char *buf, int buf_size)
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

static inline int __ctsvc_safe_read(int fd, char *buf, int buf_size)
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

static int __ctsvc_socket_handle_return(int fd, ctsvc_socket_msg_s *msg)
{
	CTS_FN_CALL;
	int ret;

	ret = __ctsvc_safe_read(fd, (char *)msg, sizeof(ctsvc_socket_msg_s));
	RETVM_IF(-1 == ret, CONTACTS_ERROR_IPC, "__ctsvc_safe_read() Failed(errno = %d)", errno);

	WARN_IF(CTSVC_SOCKET_MSG_TYPE_REQUEST_RETURN_VALUE != msg->type,
			"Unknown Type(%d), ret=%d, attach_num= %d,"
			"attach1 = %d, attach2 = %d, attach3 = %d, attach4 = %d",
			msg->type, msg->val, msg->attach_num,
			msg->attach_sizes[0],msg->attach_sizes[1],msg->attach_sizes[2],
			msg->attach_sizes[3]);

	RETVM_IF(CTSVC_SOCKET_MSG_REQUEST_MAX_ATTACH < msg->attach_num, CONTACTS_ERROR_IPC,
			"Invalid msg(attach_num = %d)", msg->attach_num);

	return CONTACTS_ERROR_NONE;
}

static void __ctsvc_remove_invalid_msg(int fd, int size)
{
	int ret;
	char dummy[CTSVC_SOCKET_MSG_SIZE] = {0};

	while (size) {
		if (sizeof(dummy) < size) {
			ret = read(fd, dummy, sizeof(dummy));
			if (-1 == ret) {
				if (EINTR == errno)
					continue;
				else
					return;
			}
			size -= ret;
		}
		else {
			ret = read(fd, dummy, size);
			if (-1 == ret) {
				if (EINTR == errno)
					continue;
				else
					return;
			}
			size -= ret;
		}
	}
}

int ctsvc_request_sim_import(void)
{
	int i, ret;
	ctsvc_socket_msg_s msg = {0};

	RETVM_IF(-1 == cts_sockfd, CONTACTS_ERROR_IPC, "socket is not connected");

	msg.type = CTSVC_SOCKET_MSG_TYPE_REQUEST_IMPORT_SIM;
	ret = __ctsvc_safe_write(cts_sockfd, (char *)&msg, sizeof(msg));
	RETVM_IF(-1 == ret, CONTACTS_ERROR_IPC, "__ctsvc_safe_write() Failed(errno = %d)", errno);

	ret = __ctsvc_socket_handle_return(cts_sockfd, &msg);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_socket_handle_return() Failed(%d)", ret);
	CTS_DBG("attach_num = %d", msg.attach_num);

	for (i=0;i<msg.attach_num;i++)
		__ctsvc_remove_invalid_msg(cts_sockfd, msg.attach_sizes[i]);

	return msg.val;
}

int ctsvc_request_sim_get_initialization_status(bool *completed)
{
	int ret =0;
	ctsvc_socket_msg_s msg = {0};
	char dest[CTSVC_SOCKET_MSG_SIZE] = {0};

	RETVM_IF(-1 == cts_sockfd, CONTACTS_ERROR_IPC, "socket is not connected");

	msg.type = CTSVC_SOCKET_MSG_TYPE_REQUEST_SIM_INIT_COMPLETE;
	ret = __ctsvc_safe_write(cts_sockfd, (char *)&msg, sizeof(msg));
	RETVM_IF(-1 == ret, CONTACTS_ERROR_IPC, "__ctsvc_safe_write() Failed(errno = %d)", errno);

	ret = __ctsvc_socket_handle_return(cts_sockfd, &msg);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_socket_handle_return() Failed(%d)", ret);
	CTS_DBG("attach_num = %d", msg.attach_num);

	ret = __ctsvc_safe_read(cts_sockfd, dest, msg.attach_sizes[0]);
	RETVM_IF(-1 == ret, CONTACTS_ERROR_IPC, "__ctsvc_safe_read() Failed(errno = %d)", errno);

	if(atoi(dest) ==0)
		*completed = false;
	else
		*completed = true;

	CTS_INFO("sim init complete : %d", *completed);

	return msg.val;
}

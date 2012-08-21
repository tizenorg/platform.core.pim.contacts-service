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
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "cts-internal.h"
#include "cts-normalize.h"
#include "cts-socket.h"

static int cts_csockfd = -1;

static inline int cts_safe_write(int fd, const char *buf, int buf_size)
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

static inline int cts_safe_read(int fd, char *buf, int buf_size)
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

static int cts_socket_handle_return(int fd, cts_socket_msg *msg)
{
	CTS_FN_CALL;
	int ret;

	ret = cts_safe_read(fd, (char *)msg, sizeof(cts_socket_msg));
	retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_read() Failed(errno = %d)", errno);

	warn_if(CTS_REQUEST_RETURN_VALUE != msg->type,
			"Unknown Type(%d), ret=%d, attach_num= %d,"
			"attach1 = %d, attach2 = %d, attach3 = %d, attach4 = %d",
			msg->type, msg->val, msg->attach_num,
			msg->attach_sizes[0],msg->attach_sizes[1],msg->attach_sizes[2],
			msg->attach_sizes[3]);

	retvm_if(CTS_REQUEST_MAX_ATTACH < msg->attach_num, CTS_ERR_SOCKET_FAILED,
			"Invalid msg(attach_num = %d)", msg->attach_num);

	return CTS_SUCCESS;
}

static void cts_remove_invalid_msg(int fd, int size)
{
	int ret;
	char dummy[CTS_SOCKET_MSG_SIZE];

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

int cts_request_sim_import(void)
{
	int i, ret;
	cts_socket_msg msg={0};

	retvm_if(-1 == cts_csockfd, CTS_ERR_ENV_INVALID, "socket is not connected");

	msg.type = CTS_REQUEST_IMPORT_SIM;
	ret = cts_safe_write(cts_csockfd, (char *)&msg, sizeof(msg));
	retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_write() Failed(errno = %d)", errno);

	ret = cts_socket_handle_return(cts_csockfd, &msg);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_socket_handle_return() Failed(%d)", ret);
	CTS_DBG("attach_num = %d", msg.attach_num);

	for (i=0;i<msg.attach_num;i++)
		cts_remove_invalid_msg(cts_csockfd, msg.attach_sizes[i]);

	return msg.val;
}

int cts_request_sim_export(int index)
{
	int i, ret;
	cts_socket_msg msg={0};
	char src[64] = {0};

	retvm_if(-1 == cts_csockfd, CTS_ERR_ENV_INVALID, "socket is not connected");

	snprintf(src, sizeof(src), "%d", index);
	msg.type = CTS_REQUEST_EXPORT_SIM;
	msg.attach_num = 1;
	msg.attach_sizes[0] = strlen(src);

	ret = cts_safe_write(cts_csockfd, (char *)&msg, sizeof(msg));
	retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_write() Failed(errno = %d)", errno);

	ret = cts_safe_write(cts_csockfd, src, msg.attach_sizes[0]);
	retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_write() Failed(errno = %d)", errno);

	ret = cts_socket_handle_return(cts_csockfd, &msg);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_socket_handle_return() Failed(%d)", ret);
	CTS_DBG("attach_num = %d", msg.attach_num);

	for (i=0;i<msg.attach_num;i++)
		cts_remove_invalid_msg(cts_csockfd, msg.attach_sizes[i]);

	return msg.val;
}

int cts_request_normalize_str(const char *src, char *dest, int dest_size)
{
	int i, ret;
	cts_socket_msg msg={0};

	retvm_if(-1 == cts_csockfd, CTS_ERR_ENV_INVALID, "socket is not connected");

	msg.type = CTS_REQUEST_NORMALIZE_STR;
	msg.attach_num = CTS_NS_ATTACH_NUM;
	msg.attach_sizes[0] = strlen(src);
	if (0 == msg.attach_sizes[0]) {
		ERR("The parameter(src) is empty string");
		dest[0] = '\0';
		return CTS_SUCCESS;
	}

	ret = cts_safe_write(cts_csockfd, (char *)&msg, sizeof(msg));
	retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_write() Failed(errno = %d)", errno);
	ret = cts_safe_write(cts_csockfd, src, msg.attach_sizes[0]);
	retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_write() Failed(errno = %d)", errno);
	CTS_DBG("Send message : %s(%d)", src, msg.attach_sizes[0]);

	ret = cts_socket_handle_return(cts_csockfd, &msg);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_socket_handle_return() Failed(%d)", ret);

	warn_if(CTS_NS_ATTACH_NUM != msg.attach_num,
			"Invalid attachments(attach_num = %d)", msg.attach_num);

	if (dest_size <= msg.attach_sizes[0]) {
		ret = cts_safe_read(cts_csockfd, dest, dest_size);
		retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_read() Failed(errno = %d)", errno);

		msg.attach_sizes[0] -= ret;
		dest[dest_size-1] = '\0';
		cts_remove_invalid_msg(cts_csockfd, msg.attach_sizes[0]);
	}
	else {
		ret = cts_safe_read(cts_csockfd, dest, msg.attach_sizes[0]);
		retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_read() Failed(errno = %d)", errno);

		dest[msg.attach_sizes[0]] = '\0';
	}

	for (i=CTS_NS_ATTACH_NUM;i<msg.attach_num;i++)
		cts_remove_invalid_msg(cts_csockfd, msg.attach_sizes[i]);

	return msg.val;
}

int cts_request_normalize_name(char dest[][CTS_SQL_MAX_LEN])
{
	int i, ret;
	cts_socket_msg msg={0};

	retvm_if(-1 == cts_csockfd, CTS_ERR_ENV_INVALID, "socket is not connected");

	msg.type = CTS_REQUEST_NORMALIZE_NAME;
	msg.attach_num = CTS_NN_ATTACH_NUM;
	msg.attach_sizes[CTS_NN_FIRST] = strlen(dest[CTS_NN_FIRST]);
	msg.attach_sizes[CTS_NN_LAST] = strlen(dest[CTS_NN_LAST]);
	msg.attach_sizes[CTS_NN_SORTKEY] = strlen(dest[CTS_NN_SORTKEY]);

	if (!msg.attach_sizes[CTS_NN_FIRST] && !msg.attach_sizes[CTS_NN_LAST]){
		return CTS_SUCCESS;
	}
	ret = cts_safe_write(cts_csockfd, (char *)&msg, sizeof(msg));
	retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_write() Failed(errno = %d)", errno);
	ret = cts_safe_write(cts_csockfd, dest[CTS_NN_FIRST], msg.attach_sizes[CTS_NN_FIRST]);
	retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_write() Failed(errno = %d)", errno);
	ret = cts_safe_write(cts_csockfd, dest[CTS_NN_LAST], msg.attach_sizes[CTS_NN_LAST]);
	retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_write() Failed(errno = %d)", errno);
	ret = cts_safe_write(cts_csockfd, dest[CTS_NN_SORTKEY], msg.attach_sizes[CTS_NN_SORTKEY]);
	retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_write() Failed(errno = %d)", errno);
	CTS_DBG("request_first = %s(%d), request_last = %s(%d), request_sortkey = %s(%d)",
			dest[CTS_NN_FIRST], msg.attach_sizes[CTS_NN_FIRST],
			dest[CTS_NN_LAST], msg.attach_sizes[CTS_NN_LAST],
			dest[CTS_NN_SORTKEY], msg.attach_sizes[CTS_NN_SORTKEY]);

	ret = cts_socket_handle_return(cts_csockfd, &msg);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_socket_handle_return() Failed(%d)", ret);

	if (CTS_NN_MAX < msg.attach_num) {
		ERR("Invalid attachments(attach_num = %d)", msg.attach_num);

		for (i=0;i<msg.attach_num;i++)
			cts_remove_invalid_msg(cts_csockfd, msg.attach_sizes[i]);
		return CTS_ERR_MSG_INVALID;
	}

	for (i=0;i<msg.attach_num;i++)
	{
		CTS_DBG("msg_size = %d", msg.attach_sizes[i]);
		if (CTS_SQL_MAX_LEN <= msg.attach_sizes[i])
		{
			ret = cts_safe_read(cts_csockfd, dest[i], sizeof(dest[i]));
			retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_read() Failed(errno = %d)", errno);

			msg.attach_sizes[i] -= ret;
			dest[i][CTS_SQL_MAX_LEN-1] = '\0';
			cts_remove_invalid_msg(cts_csockfd, msg.attach_sizes[i]);
		}
		else {
			ret = cts_safe_read(cts_csockfd, dest[i], msg.attach_sizes[i]);
			retvm_if(-1 == ret, CTS_ERR_SOCKET_FAILED, "cts_safe_read() Failed(errno = %d)", errno);

			dest[i][msg.attach_sizes[i]] = '\0';
		}
	}

	return msg.val;
}

int cts_socket_init(void)
{
	int ret;
	struct sockaddr_un caddr;

	bzero(&caddr, sizeof(caddr));
	caddr.sun_family = AF_UNIX;
	snprintf(caddr.sun_path, sizeof(caddr.sun_path), "%s", CTS_SOCKET_PATH);

	cts_csockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	retvm_if(-1 == cts_csockfd, CTS_ERR_SOCKET_FAILED,
			"socket() Failed(errno = %d)", errno);

	ret = connect(cts_csockfd, (struct sockaddr *)&caddr, sizeof(caddr));
	if (-1 == ret) {
		ERR("connect() Failed(errno = %d)", errno);
		close(cts_csockfd);
		cts_csockfd = -1;
		return CTS_ERR_SOCKET_FAILED;
	}

	return CTS_SUCCESS;
}

void cts_socket_final(void)
{
	close(cts_csockfd);
	cts_csockfd = -1;
}

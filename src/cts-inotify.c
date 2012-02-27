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
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>

#include "cts-internal.h"

typedef struct
{
	int wd;
	void (*cb)(void *);
	void *cb_data;
}noti_info;

static int cts_inoti_fd = -1;
static guint cts_inoti_handler;
static GSList *cts_noti_list;

static inline void handle_callback(GSList *noti_list, int wd, uint32_t mask)
{
	noti_info *noti;
	GSList *it = NULL;

	for (it = noti_list;it;it=it->next)
	{
		noti = (noti_info *)it->data;
		if (noti->wd == wd) {
			if ((mask & IN_CLOSE_WRITE) && noti->cb)
				noti->cb(noti->cb_data);
		}
	}
}

static gboolean cts_inotify_gio_cb(GIOChannel *src, GIOCondition cond, gpointer data)
{
	int fd, ret;
	struct inotify_event ie;
	char name[FILENAME_MAX];

	fd = g_io_channel_unix_get_fd(src);

	while (0 < (ret = read(fd, &ie, sizeof(ie)))) {
		if (sizeof(ie) == ret) {
			if (cts_noti_list)
				handle_callback(cts_noti_list, ie.wd, ie.mask);

			while (0 < ie.len) {
				ret = read(fd, name, (ie.len<sizeof(name))?ie.len:sizeof(name));
				if (-1 == ret) {
					if (EINTR == errno)
						continue;
					else
						break;
				}
				ie.len -= ret;
			}
		}else {
			while (ret < sizeof(ie)) {
				int read_size;
				read_size = read(fd, name, sizeof(ie)-ret);
				if (-1 == read_size) {
					if (EINTR == errno)
						continue;
					else
						break;
				}
				ret += read_size;
			}
		}
	}

	return TRUE;
}

static inline int cts_inotify_attach_handler(int fd)
{
	guint ret;
	GIOChannel *channel;

	retvm_if(fd < 0, CTS_ERR_ARG_INVALID, "fd is invalid");

	channel = g_io_channel_unix_new(fd);
	retvm_if(NULL == channel, CTS_ERR_INOTIFY_FAILED, "g_io_channel_unix_new() Failed");

	g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);

	ret = g_io_add_watch(channel, G_IO_IN, cts_inotify_gio_cb, NULL);
	g_io_channel_unref(channel);

	return ret;
}

int cts_inotify_init(void)
{
	cts_inoti_fd = inotify_init();
	retvm_if(-1 == cts_inoti_fd, CTS_ERR_INOTIFY_FAILED,
			"inotify_init() Failed(%d)", errno);

	fcntl(cts_inoti_fd, F_SETFD, FD_CLOEXEC);
	fcntl(cts_inoti_fd, F_SETFL, O_NONBLOCK);

	cts_inoti_handler = cts_inotify_attach_handler(cts_inoti_fd);
	if (cts_inoti_handler <= 0) {
		ERR("cts_inotify_attach_handler() Failed");
		close(cts_inoti_fd);
		cts_inoti_fd = -1;
		cts_inoti_handler = 0;
		return CTS_ERR_INOTIFY_FAILED;
	}

	return CTS_SUCCESS;
}

static inline int cts_inotify_get_wd(int fd, const char *notipath)
{
	return inotify_add_watch(fd, notipath, IN_ACCESS);
}

static inline int cts_inotify_watch(int fd, const char *notipath)
{
	int ret;

	ret = inotify_add_watch(fd, notipath, IN_CLOSE_WRITE);
	retvm_if(-1 == ret, CTS_ERR_INOTIFY_FAILED,
			"inotify_add_watch() Failed(%d)", errno);

	return CTS_SUCCESS;
}

int cts_inotify_subscribe(const char *path, void (*cb)(void *), void *data)
{
	int ret, wd;
	noti_info *noti, *same_noti = NULL;
	GSList *it;

	retv_if(NULL==path, CTS_ERR_ARG_NULL);
	retv_if(NULL==cb, CTS_ERR_ARG_NULL);
	retvm_if(cts_inoti_fd < 0, CTS_ERR_ENV_INVALID,
			"cts_inoti_fd(%d) is invalid", cts_inoti_fd);

	wd = cts_inotify_get_wd(cts_inoti_fd, path);
	retvm_if(-1 == wd, CTS_ERR_INOTIFY_FAILED,
			"cts_inotify_get_wd() Failed(%d)", errno);

	for (it=cts_noti_list;it;it=it->next)
	{
		if (it->data)
		{
			same_noti = it->data;
			if (same_noti->wd == wd && same_noti->cb == cb && same_noti->cb_data == data) {
				break;
			}
			else {
				same_noti = NULL;
			}
		}
	}

	if (same_noti) {
		cts_inotify_watch(cts_inoti_fd, path);
		ERR("The same callback(%s) is already exist", path);
		return CTS_ERR_ALREADY_EXIST;
	}

	ret = cts_inotify_watch(cts_inoti_fd, path);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_inotify_watch() Failed");

	noti = calloc(1, sizeof(noti_info));
	retvm_if(NULL == noti, CTS_ERR_OUT_OF_MEMORY, "calloc() Failed");

	noti->wd = wd;
	noti->cb_data = data;
	noti->cb = cb;
	cts_noti_list = g_slist_append(cts_noti_list, noti);

	return CTS_SUCCESS;
}

static inline int del_noti_with_data(GSList **noti_list, int wd,
		void (*cb)(void *), void *user_data)
{
	int del_cnt, remain_cnt;
	GSList *it, *result;

	del_cnt = 0;
	remain_cnt = 0;

	it = result = *noti_list;
	while (it)
	{
		noti_info *noti = it->data;
		if (noti && wd == noti->wd)
		{
			if (cb == noti->cb && user_data == noti->cb_data) {
				it = it->next;
				result = g_slist_remove(result , noti);
				free(noti);
				del_cnt++;
				continue;
			}
			else {
				remain_cnt++;
			}
		}
		it = it->next;
	}
	retvm_if(del_cnt == 0, CTS_ERR_NO_DATA, "nothing deleted");

	*noti_list = result;

	return remain_cnt;
}

static inline int del_noti(GSList **noti_list, int wd, void (*cb)(void *))
{
	int del_cnt, remain_cnt;
	GSList *it, *result;

	del_cnt = 0;
	remain_cnt = 0;

	it = result = *noti_list;
	while (it)
	{
		noti_info *noti = it->data;
		if (noti && wd == noti->wd)
		{
			if (NULL == cb || noti->cb == cb) {
				it = it->next;
				result = g_slist_remove(result, noti);
				free(noti);
				del_cnt++;
				continue;
			}
			else {
				remain_cnt++;
			}
		}
		it = it->next;
	}
	retvm_if(del_cnt == 0, CTS_ERR_NO_DATA, "nothing deleted");

	*noti_list = result;

	return remain_cnt;
}

int cts_inotify_unsubscribe(const char *path, void (*cb)(void *))
{
	int ret, wd;

	retv_if(NULL == path, CTS_ERR_ARG_NULL);
	retvm_if(cts_inoti_fd < 0, CTS_ERR_ENV_INVALID,
			"cts_inoti_fd(%d) is invalid", cts_inoti_fd);

	wd = cts_inotify_get_wd(cts_inoti_fd, path);
	retvm_if(-1 == wd, CTS_ERR_INOTIFY_FAILED,
			"cts_inotify_get_wd() Failed(%d)", errno);

	ret = del_noti(&cts_noti_list, wd, cb);
	warn_if(ret < CTS_SUCCESS, "del_noti() Failed(%d)", ret);

	if (0 == ret)
		return inotify_rm_watch(cts_inoti_fd, wd);

	return cts_inotify_watch(cts_inoti_fd, path);
}

int cts_inotify_unsubscribe_with_data(const char *path,
		void (*cb)(void *), void *user_data)
{
	int ret, wd;

	retv_if(NULL==path, CTS_ERR_ARG_NULL);
	retv_if(NULL==cb, CTS_ERR_ARG_NULL);
	retvm_if(cts_inoti_fd < 0, CTS_ERR_ENV_INVALID,
			"cts_inoti_fd(%d) is invalid", cts_inoti_fd);

	wd = cts_inotify_get_wd(cts_inoti_fd, path);
	retvm_if(-1 == wd, CTS_ERR_INOTIFY_FAILED,
			"cts_inotify_get_wd() Failed(%d)", errno);

	ret = del_noti_with_data(&cts_noti_list, wd, cb, user_data);
	warn_if(ret < CTS_SUCCESS, "del_noti_with_data() Failed(%d)", ret);

	if (0 == ret)
		return inotify_rm_watch(cts_inoti_fd, wd);

	return cts_inotify_watch(cts_inoti_fd, path);
}

static void clear_nslot_list(gpointer data, gpointer user_data)
{
	free(data);
}

static inline gboolean cts_inotify_detach_handler(guint id)
{
	return g_source_remove(id);
}

void cts_inotify_close(void)
{
	if (cts_inoti_handler) {
		cts_inotify_detach_handler(cts_inoti_handler);
		cts_inoti_handler = 0;
	}

	if (cts_noti_list) {
		g_slist_foreach(cts_noti_list, clear_nslot_list, NULL);
		g_slist_free(cts_noti_list);
		cts_noti_list = NULL;
	}

	if (0 <= cts_inoti_fd) {
		close(cts_inoti_fd);
		cts_inoti_fd = -1;
	}
}

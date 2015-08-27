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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_notify.h"
#include "ctsvc_view.h"
#include "ctsvc_handle.h"

#include <stdbool.h>

#ifdef _CONTACTS_IPC_CLIENT
#include "ctsvc_client_ipc.h"
#endif

typedef struct
{
	contacts_h contact;
	int wd;
	char *view_uri;
	contacts_db_changed_cb cb;
	void *cb_data;
	bool blocked;
}noti_info;

struct socket_init_noti_info {
	int wd;
	int subscribe_count;
	void (*cb)(void *);
	void *cb_data;
};

static GHashTable *_ctsvc_socket_init_noti_table = NULL;
static int __ctsvc_inoti_ref = 0;
static int __inoti_fd = -1;
static guint __inoti_handler = 0;
static GSList *__noti_list = NULL;

static inline void __ctsvc_inotify_handle_callback(GSList *noti_list, int wd, uint32_t mask)
{
	noti_info *noti;
	GSList *it = NULL;

	for (it = noti_list;it;it=it->next) {
		noti = (noti_info *)it->data;

		if (noti->wd == wd) {

#ifdef _CONTACTS_IPC_CLIENT
			if (ctsvc_ipc_is_busy()) {
				/* hold the line */
				noti->blocked = true;
				continue;
			}
#endif
			if ((mask & IN_CLOSE_WRITE) && noti->cb) {
				CTS_DBG("%s", noti->view_uri);
				noti->cb(noti->view_uri, noti->cb_data);
			}
		}
	}
}

static void _ctsvc_inotify_socket_init_noti_table_foreach_cb(gpointer key, gpointer value, gpointer user_data)
{
	GList *c;
	struct socket_init_noti_info *noti_info = value;

	int wd = GPOINTER_TO_INT(user_data);
	if (noti_info->wd == wd)
		noti_info->cb(noti_info->cb_data);
}

static gboolean __ctsvc_inotify_gio_cb(GIOChannel *src, GIOCondition cond, gpointer data)
{
	int fd, ret;
	struct inotify_event ie;
	char name[FILENAME_MAX] = {0};

	fd = g_io_channel_unix_get_fd(src);

	while (0 < (ret = read(fd, &ie, sizeof(ie)))) {
		if (sizeof(ie) == ret) {
			if (_ctsvc_socket_init_noti_table)
				g_hash_table_foreach(_ctsvc_socket_init_noti_table, _ctsvc_inotify_socket_init_noti_table_foreach_cb, GINT_TO_POINTER(ie.wd));

			if (__noti_list)
				__ctsvc_inotify_handle_callback(__noti_list, ie.wd, ie.mask);

			while (0 != ie.len) {
				ret = read(fd, name, (ie.len<sizeof(name))?ie.len:sizeof(name));
				if (-1 == ret) {
					if (EINTR == errno)
						continue;
					else
						return TRUE;
				}
				if (ie.len < ret)
					ie.len = 0;
				else
					ie.len -= ret;
			}
		}
		else {
			while (ret < sizeof(ie)) {
				int read_size;
				read_size = read(fd, name, sizeof(ie)-ret);
				if (-1 == read_size) {
					if (EINTR == errno)
						continue;
					else
						return TRUE;
				}
				ret += read_size;
			}
		}
	}

	return TRUE;
}

static inline int __ctsvc_inotify_attach_handler(int fd)
{
	guint ret;
	GIOChannel *channel;

	RETVM_IF(fd < 0, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter: fd is invalid");

	channel = g_io_channel_unix_new(fd);
	RETVM_IF(NULL == channel, CONTACTS_ERROR_SYSTEM, "System: g_io_channel_unix_new() Fail");

	g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);

	ret = g_io_add_watch(channel, G_IO_IN, __ctsvc_inotify_gio_cb, NULL);
	g_io_channel_unref(channel);

	return ret;
}

int ctsvc_inotify_init(void)
{
	int ret;

	if (0 < __ctsvc_inoti_ref) {
		__ctsvc_inoti_ref++;
		return CONTACTS_ERROR_NONE;
	}
	__inoti_fd = inotify_init();
	RETVM_IF(-1 == __inoti_fd, CONTACTS_ERROR_SYSTEM,
			"System: inotify_init() Fail(%d)", errno);

	ret = fcntl(__inoti_fd, F_SETFD, FD_CLOEXEC);
	WARN_IF(ret < 0, "fcntl Fail(%d)", ret);
	ret = fcntl(__inoti_fd, F_SETFL, O_NONBLOCK);
	WARN_IF(ret < 0, "fcntl Fail(%d)", ret);

	__inoti_handler = __ctsvc_inotify_attach_handler(__inoti_fd);
	if (__inoti_handler <= 0) {
		CTS_ERR("__ctsvc_inotify_attach_handler() Fail");
		close(__inoti_fd);
		__inoti_fd = -1;
		__inoti_handler = 0;
		return CONTACTS_ERROR_SYSTEM;
	}

	__ctsvc_inoti_ref++;
	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_inotify_get_wd(int fd, const char *notipath)
{
	return inotify_add_watch(fd, notipath, IN_ACCESS);
}

static inline int __ctsvc_inotify_watch(int fd, const char *notipath)
{
	int ret;

	ret = inotify_add_watch(fd, notipath, IN_CLOSE_WRITE);
	RETVM_IF(-1 == ret, CONTACTS_ERROR_SYSTEM,
			"System: inotify_add_watch() Fail(%d)", errno);

	return CONTACTS_ERROR_NONE;
}

static inline const char* __ctsvc_noti_get_file_path(const char *view_uri)
{
	ctsvc_record_type_e match = ctsvc_view_get_record_type(view_uri);
	switch((int)match) {
	case CTSVC_RECORD_ADDRESSBOOK:
		return CTSVC_NOTI_ADDRESSBOOK_CHANGED;
	case CTSVC_RECORD_GROUP:
		return CTSVC_NOTI_GROUP_CHANGED;
	case CTSVC_RECORD_PERSON:
		return CTSVC_NOTI_PERSON_CHANGED;
	case CTSVC_RECORD_CONTACT:
	case CTSVC_RECORD_SIMPLE_CONTACT:
		return CTSVC_NOTI_CONTACT_CHANGED;
	case CTSVC_RECORD_MY_PROFILE:
		return CTSVC_NOTI_MY_PROFILE_CHANGED;
	case CTSVC_RECORD_NAME:
		return CTSVC_NOTI_NAME_CHANGED;
	case CTSVC_RECORD_COMPANY:
		return CTSVC_NOTI_COMPANY_CHANGED;
	case CTSVC_RECORD_NOTE:
		return CTSVC_NOTI_NOTE_CHANGED;
	case CTSVC_RECORD_NUMBER:
		return CTSVC_NOTI_NUMBER_CHANGED;
	case CTSVC_RECORD_EMAIL:
		return CTSVC_NOTI_EMAIL_CHANGED;
	case CTSVC_RECORD_URL:
		return CTSVC_NOTI_URL_CHANGED;
	case CTSVC_RECORD_EVENT:
		return CTSVC_NOTI_EVENT_CHANGED;
	case CTSVC_RECORD_NICKNAME:
		return CTSVC_NOTI_NICKNAME_CHANGED;
	case CTSVC_RECORD_ADDRESS:
		return CTSVC_NOTI_ADDRESS_CHANGED;
	case CTSVC_RECORD_MESSENGER:
		return CTSVC_NOTI_MESSENGER_CHANGED;
	case CTSVC_RECORD_GROUP_RELATION:
		return CTSVC_NOTI_GROUP_RELATION_CHANGED;
	case CTSVC_RECORD_ACTIVITY:
		return CTSVC_NOTI_ACTIVITY_CHANGED;
	case CTSVC_RECORD_ACTIVITY_PHOTO:
		return CTSVC_NOTI_ACTIVITY_PHOTO_CHANGED;
	case CTSVC_RECORD_PROFILE:
		return CTSVC_NOTI_PROFILE_CHANGED;
	case CTSVC_RECORD_RELATIONSHIP:
		return CTSVC_NOTI_RELATIONSHIP_CHANGED;
	case CTSVC_RECORD_IMAGE:
		return CTSVC_NOTI_IMAGE_CHANGED;
	case CTSVC_RECORD_EXTENSION:
		return CTSVC_NOTI_DATA_CHANGED;
	case CTSVC_RECORD_PHONELOG:
		return CTSVC_NOTI_PHONELOG_CHANGED;
	case CTSVC_RECORD_SPEEDDIAL:
		return CTSVC_NOTI_SPEEDDIAL_CHANGED;
	case CTSVC_RECORD_SDN:
		return CTSVC_NOTI_SDN_CHANGED;
	case CTSVC_RECORD_RESULT:
	default:
		CTS_ERR("Invalid parameter : The type(%s) is not supported", view_uri);
		return NULL;
	}
	return NULL;
}

int ctsvc_inotify_subscribe_ipc_ready(contacts_h contact, void (*cb)(void *), void *user_data)
{
	const char *noti_path = CTSVC_NOTI_IPC_READY;
	struct socket_init_noti_info *noti_info = NULL;

	if (NULL == _ctsvc_socket_init_noti_table)
		_ctsvc_socket_init_noti_table = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	else
		noti_info = g_hash_table_lookup(_ctsvc_socket_init_noti_table, noti_path);

	if (NULL == noti_info) {
		int wd = __ctsvc_inotify_get_wd(__inoti_fd, noti_path);
		if (-1 == wd) {
			CTS_ERR("__ctsvc_inotify_get_wd() Failed(noti_path=%s, errno : %d)", noti_path, errno);
			if (errno == EACCES)
				return CONTACTS_ERROR_PERMISSION_DENIED;
			return CONTACTS_ERROR_SYSTEM;
		}

		int ret = __ctsvc_inotify_watch(__inoti_fd, noti_path);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("__ctsvc_inotify_watch() Failed");
			return ret;
		}

		noti_info = calloc(1, sizeof(struct socket_init_noti_info));
		if (NULL == noti_info) {
			CTS_ERR("calloc() return NULL");
			return ret;
		}

		noti_info->wd = wd;
		noti_info->cb = cb;
		noti_info->cb_data = user_data;
		g_hash_table_insert(_ctsvc_socket_init_noti_table, strdup(noti_path), noti_info);
	}
	noti_info->subscribe_count++;
	return CONTACTS_ERROR_NONE;
}

int ctsvc_inotify_unsubscribe_ipc_ready(contacts_h contact)
{
	RETVM_IF(NULL == _ctsvc_socket_init_noti_table, CONTACTS_ERROR_INVALID_PARAMETER, "_ctsvc_socket_init_noti_table is NULL");

	const char *noti_path = CTSVC_NOTI_IPC_READY;
	struct socket_init_noti_info *noti_info = NULL;

	noti_info = g_hash_table_lookup(_ctsvc_socket_init_noti_table, noti_path);
	RETVM_IF(NULL == noti_info, CONTACTS_ERROR_INVALID_PARAMETER, "g_hash_table_lookup() return NULL");

	if (1 == noti_info->subscribe_count) {
		int wd = noti_info->wd;
		inotify_rm_watch(__inoti_fd, wd);
		g_hash_table_remove(_ctsvc_socket_init_noti_table, noti_path); // free noti_info automatically
	}
	else {
		noti_info->subscribe_count--;
	}

	return CONTACTS_ERROR_NONE;
}


int ctsvc_inotify_subscribe(contacts_h contact, const char *view_uri,
			void *cb, void *data)
{
	int ret, wd;
	noti_info *noti, *same_noti = NULL;
	GSList *it;
	const char *path;

	RETV_IF(NULL==cb, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(__inoti_fd < 0, CONTACTS_ERROR_SYSTEM,
			"__inoti_fd(%d) is invalid", __inoti_fd);

	path = __ctsvc_noti_get_file_path(view_uri);
	RETVM_IF(NULL == path, CONTACTS_ERROR_INVALID_PARAMETER,
			"__ctsvc_noti_get_file_path(%s) Fail", view_uri);

	wd = __ctsvc_inotify_get_wd(__inoti_fd, path);
	if (-1 == wd) {
		CTS_ERR("__ctsvc_inotify_get_wd() Fail(errno : %d)", errno);
		if (errno == EACCES)
			return CONTACTS_ERROR_PERMISSION_DENIED;
		return CONTACTS_ERROR_SYSTEM;
	}

	for (it=__noti_list;it;it=it->next) {
		if (it->data) {
			same_noti = it->data;
			if (same_noti->wd == wd && same_noti->cb == cb &&
					STRING_EQUAL == strcmp(same_noti->view_uri, view_uri) && same_noti->cb_data == data &&
					0 == ctsvc_handle_compare(contact, same_noti->contact))
				break;
			else
				same_noti = NULL;
		}
	}

	if (same_noti) {
		__ctsvc_inotify_watch(__inoti_fd, path);
		CTS_ERR("The same callback(%s) is already exist", view_uri);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	ret = __ctsvc_inotify_watch(__inoti_fd, path);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "__ctsvc_inotify_watch() Fail");

	noti = calloc(1, sizeof(noti_info));
	RETVM_IF(NULL == noti, CONTACTS_ERROR_OUT_OF_MEMORY, "calloc() Fail");

	noti->wd = wd;
	noti->view_uri = strdup(view_uri);
	noti->cb_data = data;
	noti->cb = cb;
	ctsvc_handle_clone(contact, &(noti->contact));
	noti->blocked = false;

	__noti_list = g_slist_append(__noti_list, noti);

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_del_noti(GSList **noti_list, contacts_h contact, int wd,
		const char *view_uri, void *cb, void *user_data)
{
	int del_cnt, remain_cnt;
	GSList *it, *result;

	del_cnt = 0;
	remain_cnt = 0;

	it = result = *noti_list;
	while (it) {
		noti_info *noti = it->data;
		if (noti && wd == noti->wd) {
			if (cb == noti->cb && user_data == noti->cb_data
				&& STRING_EQUAL == strcmp(noti->view_uri, view_uri)
				&& 0 == ctsvc_handle_compare(contact, noti->contact)) {
				it = it->next;
				result = g_slist_remove(result, noti);
				ctsvc_handle_destroy(noti->contact);
				free(noti->view_uri);
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
	RETVM_IF(del_cnt == 0, CONTACTS_ERROR_NO_DATA, "No Data: nothing deleted");

	*noti_list = result;

	return remain_cnt;
}

int ctsvc_inotify_unsubscribe(contacts_h contact, const char *view_uri, void *cb, void *user_data)
{
	int ret, wd;
	const char *path;

	RETV_IF(NULL==cb, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(__inoti_fd < 0, CONTACTS_ERROR_SYSTEM,
			"System : __inoti_fd(%d) is invalid", __inoti_fd);

	path = __ctsvc_noti_get_file_path(view_uri);
	RETVM_IF(NULL == path, CONTACTS_ERROR_INVALID_PARAMETER,
			"__ctsvc_noti_get_file_path(%s) Fail", view_uri);

	wd = __ctsvc_inotify_get_wd(__inoti_fd, path);
	if (-1 == wd) {
		CTS_ERR("__ctsvc_inotify_get_wd() Fail(errno : %d)", errno);
		if (errno == EACCES)
			return CONTACTS_ERROR_PERMISSION_DENIED;
		return CONTACTS_ERROR_SYSTEM;
	}

	ret = __ctsvc_del_noti(&__noti_list, contact, wd, view_uri, cb, user_data);
	WARN_IF(ret < CONTACTS_ERROR_NONE, "__ctsvc_del_noti() Fail(%d)", ret);

	if (0 == ret)
		return inotify_rm_watch(__inoti_fd, wd);

	return __ctsvc_inotify_watch(__inoti_fd, path);
}

static void __clear_nslot_list(gpointer data, gpointer user_data)
{
	noti_info *noti = (noti_info *)data;

	ctsvc_handle_destroy(noti->contact);
	free(noti->view_uri);
	free(noti);
}

static inline gboolean __ctsvc_inotify_detach_handler(guint id)
{
	return g_source_remove(id);
}

void ctsvc_inotify_close(void)
{
	if (1 < __ctsvc_inoti_ref) {
		CTS_DBG("inotify ref count : %d", __ctsvc_inoti_ref);
		__ctsvc_inoti_ref--;
		return;
	}
	else if (__ctsvc_inoti_ref < 1) {
		CTS_DBG("Please call connection API. inotify ref count : %d", __ctsvc_inoti_ref);
		return;
	}

	__ctsvc_inoti_ref--;

	if (__inoti_handler) {
		__ctsvc_inotify_detach_handler(__inoti_handler);
		__inoti_handler = 0;
	}

	if (__noti_list) {
		g_slist_foreach(__noti_list, __clear_nslot_list, NULL);
		g_slist_free(__noti_list);
		__noti_list = NULL;
	}

	if (0 <= __inoti_fd) {
		close(__inoti_fd);
		__inoti_fd = -1;
	}
}


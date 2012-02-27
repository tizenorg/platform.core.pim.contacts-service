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
#include <sys/time.h>
#include <vconf.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#include "cts-internal.h"
#include "cts-utils.h"
#include "cts-schema.h"
#include "cts-sqlite.h"
#include "cts-socket.h"
#include "cts-normalize.h"
#include "cts-inotify.h"
#include "cts-vcard.h"
#include "cts-pthread.h"
#include "cts-types.h"

static const char *CTS_NOTI_CONTACT_CHANGED=CTS_NOTI_CONTACT_CHANGED_DEF;
static const char *CTS_NOTI_PLOG_CHANGED="/opt/data/contacts-svc/.CONTACTS_SVC_PLOG_CHANGED";
static const char *CTS_NOTI_FAVORITE_CHANGED="/opt/data/contacts-svc/.CONTACTS_SVC_FAVOR_CHANGED";
static const char *CTS_NOTI_SPEEDDIAL_CHANGED="/opt/data/contacts-svc/.CONTACTS_SVC_SPEED_CHANGED";
static const char *CTS_NOTI_ADDRBOOK_CHANGED="/opt/data/contacts-svc/.CONTACTS_SVC_AB_CHANGED";
static const char *CTS_NOTI_GROUP_CHANGED="/opt/data/contacts-svc/.CONTACTS_SVC_GROUP_CHANGED";
static const char *CTS_NOTI_GROUP_RELATION_CHANGED="/opt/data/contacts-svc/.CONTACTS_SVC_GROUP_REL_CHANGED";
static const char *CTS_NOTI_MISSED_CALL_CHANGED="/opt/data/contacts-svc/.CONTACTS_SVC_MISSED_CHANGED";

static const char *CTS_VCONF_SORTING_ORDER="db/service/contacts/name_sorting_order";
static const char *CTS_VCONF_DISPLAY_ORDER=CTS_VCONF_DISPLAY_ORDER_DEF;

static int transaction_count = 0;
static int transaction_ver = 0;
static bool version_up=false;

static bool contact_change=false;
static bool plog_change=false;
static bool missed_change=false;
static bool favor_change=false;
static bool speed_change=false;
static bool addrbook_change=false;
static bool group_change=false;
static bool group_rel_change=false;

static int name_sorting_order = -1;
static int name_display_order = -1;
static int default_lang = -1;

static void cts_vconf_callback(keynode_t *key, void *data)
{
	//if(!strcmp(vconf_keynode_get_name(key), CTS_VCONF_SORTING_ORDER));
	if (CTS_ORDER_OF_SORTING == (int)data)
		name_sorting_order = vconf_keynode_get_int(key);
	else if (CTS_ORDER_OF_DISPLAY == (int)data)
		name_display_order = vconf_keynode_get_int(key);
	else if (CTS_ORDER_OF_DISPLAY + 1 == (int)data)
		default_lang = vconf_keynode_get_int(key);
}

int cts_get_default_language(void)
{
	if (default_lang < 0) {
		int ret;
		ret = vconf_get_int(CTS_VCONF_DEFAULT_LANGUAGE, &default_lang);
		warn_if(ret < 0, "vconf_get_int() Failed(%d)", ret);
	}
	return default_lang;
}

void cts_set_contact_noti(void)
{
	contact_change = true;
}
void cts_set_plog_noti(void)
{
	plog_change = true;
}
void cts_set_missed_call_noti(void)
{
	missed_change = true;
}
void cts_set_favor_noti(void)
{
	favor_change = true;
}
void cts_set_speed_noti(void)
{
	speed_change = true;
}
void cts_set_addrbook_noti(void)
{
	addrbook_change = true;
}
void cts_set_group_noti(void)
{
	group_change = true;
}
void cts_set_group_rel_noti(void)
{
	group_rel_change = true;
}

static inline void cts_noti_publish_contact_change(void)
{
	int fd = open(CTS_NOTI_CONTACT_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		contact_change = false;
	}
}

static inline void cts_noti_publish_plog_change(void)
{
	int fd = open(CTS_NOTI_PLOG_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		plog_change = false;
	}
}

static inline void cts_noti_publish_missed_call_change(void)
{
	int fd = open(CTS_NOTI_MISSED_CALL_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		missed_change = false;
	}
}

static inline void cts_noti_publish_favor_change(void)
{
	int fd = open(CTS_NOTI_FAVORITE_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		favor_change = false;
	}
}

static inline void cts_noti_publish_speed_change(void)
{
	int fd = open(CTS_NOTI_SPEEDDIAL_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		speed_change = false;
	}
}

static inline void cts_noti_publish_addrbook_change(void)
{
	int fd = open(CTS_NOTI_ADDRBOOK_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		addrbook_change = false;
	}
}

static inline void cts_noti_publish_group_change(void)
{
	int fd = open(CTS_NOTI_GROUP_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		group_change = false;
	}
}

static inline void cts_noti_publish_group_rel_change(void)
{
	int fd = open(CTS_NOTI_GROUP_RELATION_CHANGED, O_TRUNC | O_RDWR);
	if (0 <= fd) {
		close(fd);
		group_rel_change = false;
	}
}

#define CTS_COMMIT_TRY_MAX 500000 // For 3second
API int contacts_svc_begin_trans(void)
{
	int ret = -1, progress;

	if (transaction_count <= 0)
	{
		ret = cts_query_exec("BEGIN IMMEDIATE TRANSACTION"); //taken 600ms
		progress = 100000;
		while (CTS_ERR_DB_LOCK == ret && progress<CTS_COMMIT_TRY_MAX) {
			usleep(progress);
			ret = cts_query_exec("BEGIN IMMEDIATE TRANSACTION");
			progress *= 2;
		}
		retvm_if(CTS_SUCCESS != ret, ret, "cts_query_exec() Failed(%d)", ret);

		transaction_count = 0;

		const char *query = "SELECT ver FROM "CTS_TABLE_VERSION;
		transaction_ver = cts_query_get_first_int_result(query);
		version_up = false;
	}
	transaction_count++;
	CTS_DBG("transaction_count : %d.", transaction_count);

	return CTS_SUCCESS;
}

static inline void cts_cancel_changes(void)
{
	contact_change = false;
	plog_change = false;
	missed_change = false;
	favor_change = false;
	speed_change = false;
	addrbook_change = false;
	group_change = false;
	group_rel_change = false;
}

API int contacts_svc_end_trans(bool is_success)
{
	int ret = -1, progress;
	char query[CTS_SQL_MIN_LEN];

	transaction_count--;

	if (0 != transaction_count) {
		CTS_DBG("contact transaction_count : %d.", transaction_count);
		return CTS_SUCCESS;
	}

	if (false == is_success) {
		cts_cancel_changes();
		ret = cts_query_exec("ROLLBACK TRANSACTION");
		return CTS_SUCCESS;
	}

	if (version_up) {
		transaction_ver++;
		snprintf(query, sizeof(query), "UPDATE %s SET ver = %d",
				CTS_TABLE_VERSION, transaction_ver);
		ret = cts_query_exec(query);
		warn_if(CTS_SUCCESS != ret, "cts_query_exec(version up) Failed(%d)", ret);
	}

	progress = 400000;
	ret = cts_query_exec("COMMIT TRANSACTION");
	while (CTS_ERR_DB_LOCK == ret && progress<CTS_COMMIT_TRY_MAX) {
		usleep(progress);
		ret = cts_query_exec("COMMIT TRANSACTION");
		progress *= 2;
	}
	if (CTS_SUCCESS != ret) {
		int tmp_ret;
		ERR("cts_query_exec() Failed(%d)", ret);
		cts_cancel_changes();
		tmp_ret = cts_query_exec("ROLLBACK TRANSACTION");
		warn_if(CTS_SUCCESS != tmp_ret, "cts_query_exec(ROLLBACK) Failed(%d)", tmp_ret);
		return ret;
	}
	if (contact_change) cts_noti_publish_contact_change();
	if (plog_change) cts_noti_publish_plog_change();
	if (missed_change) cts_noti_publish_missed_call_change();
	if (favor_change) cts_noti_publish_favor_change();
	if (speed_change) cts_noti_publish_speed_change();
	if (addrbook_change) cts_noti_publish_addrbook_change();
	if (group_change) cts_noti_publish_group_change();
	if (group_rel_change) cts_noti_publish_group_rel_change();

	return transaction_ver;
}

int cts_get_next_ver(void)
{
	const char *query;

	if (0 < transaction_count) {
		version_up = true;
		return transaction_ver + 1;
	}

	query = "SELECT ver FROM "CTS_TABLE_VERSION;
	return (1+cts_query_get_first_int_result(query));
}

void cts_deregister_noti(void)
{
	int ret;

	cts_inotify_close();

	ret = vconf_ignore_key_changed(CTS_VCONF_SORTING_ORDER, cts_vconf_callback);
	retm_if(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)",CTS_VCONF_SORTING_ORDER,ret);
	ret = vconf_ignore_key_changed(CTS_VCONF_DISPLAY_ORDER, cts_vconf_callback);
	retm_if(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)",CTS_VCONF_DISPLAY_ORDER,ret);
	ret = vconf_ignore_key_changed(CTS_VCONF_DEFAULT_LANGUAGE, cts_vconf_callback);
	retm_if(ret<0,"vconf_ignore_key_changed(%s) Failed(%d)",CTS_VCONF_DEFAULT_LANGUAGE,ret);
}

static inline int cts_conf_init_name_info(void)
{
	int ret;

	ret = vconf_get_int(CTS_VCONF_SORTING_ORDER, &name_sorting_order);
	if (ret < 0) {
		ERR("vconf_get_int() Failed(%d)", ret);
		name_sorting_order = CTS_ORDER_NAME_FIRSTLAST;
	}

	ret = vconf_get_int(CTS_VCONF_DISPLAY_ORDER, &name_display_order);
	if (ret < 0) {
		ERR("vconf_get_int() Failed(%d)", ret);
		name_display_order = CTS_ORDER_NAME_FIRSTLAST;
	}

	ret = vconf_get_int(CTS_VCONF_DEFAULT_LANGUAGE, &default_lang);
	warn_if(ret < 0, "vconf_get_int() Failed(%d)", ret);

	ret = vconf_notify_key_changed(CTS_VCONF_SORTING_ORDER,
			cts_vconf_callback, (void *)CTS_ORDER_OF_SORTING);
	retvm_if(ret<0, CTS_ERR_VCONF_FAILED, "vconf_notify_key_changed() Failed(%d)", ret);
	ret = vconf_notify_key_changed(CTS_VCONF_DISPLAY_ORDER,
			cts_vconf_callback, (void *)CTS_ORDER_OF_DISPLAY);
	retvm_if(ret<0, CTS_ERR_VCONF_FAILED, "vconf_notify_key_changed() Failed(%d)", ret);
	ret = vconf_notify_key_changed(CTS_VCONF_DEFAULT_LANGUAGE,
			cts_vconf_callback, (void *)CTS_ORDER_OF_DISPLAY+1);
	retvm_if(ret<0, CTS_ERR_VCONF_FAILED, "vconf_notify_key_changed() Failed(%d)", ret);

	return CTS_SUCCESS;
}

void cts_register_noti(void)
{
	int ret;

	ret = cts_inotify_init();
	retm_if(CTS_SUCCESS != ret, "cts_inotify_init() Failed(%d)", ret);

	ret = cts_conf_init_name_info();
	retm_if(CTS_SUCCESS != ret, "cts_conf_init_name_info() Failed(%d)", ret);
}

API int contacts_svc_set_order(cts_order_op op_code, cts_order_type order)
{
	int ret;
	const char *op;

	retvm_if(CTS_ORDER_NAME_FIRSTLAST != order && CTS_ORDER_NAME_LASTFIRST != order,
			CTS_ERR_ARG_INVALID, "The parameter(order:%d) is Invalid", order);

	if (CTS_ORDER_OF_SORTING == op_code)
		op = CTS_VCONF_SORTING_ORDER;
	else
		op = CTS_VCONF_DISPLAY_ORDER;

	ret = vconf_set_int(op, order);
	retvm_if(ret<0, CTS_ERR_VCONF_FAILED, "vconf_set_int(%s) Failed(%d)", op, ret);

	if (CTS_ORDER_OF_SORTING == op_code)
		name_sorting_order = order;
	else
		name_display_order = order;

	return CTS_SUCCESS;
}

API cts_order_type contacts_svc_get_order(cts_order_op op_code)
{
	int ret;

	if (CTS_ORDER_OF_SORTING == op_code) {
		if (name_sorting_order < 0) {
			ret = vconf_get_int(CTS_VCONF_SORTING_ORDER, &name_sorting_order);
			retvm_if(ret<0, CTS_ERR_VCONF_FAILED, "vconf_get_int() Failed(%d)", ret);
		}

		return name_sorting_order;
	}
	else{
		if (name_display_order < 0) {
			ret = vconf_get_int(CTS_VCONF_DISPLAY_ORDER, &name_display_order);
			retvm_if(ret<0, CTS_ERR_VCONF_FAILED, "vconf_get_int() Failed(%d)", ret);
		}

		return name_display_order;
	}
}

static inline const char* cts_noti_get_file_path(int type)
{
	const char *noti;
	switch (type)
	{
	case CTS_SUBSCRIBE_CONTACT_CHANGE:
		noti = CTS_NOTI_CONTACT_CHANGED;
		break;
	case CTS_SUBSCRIBE_PLOG_CHANGE:
		noti = CTS_NOTI_PLOG_CHANGED;
		break;
	case CTS_SUBSCRIBE_MISSED_CALL_CHANGE:
		noti = CTS_NOTI_MISSED_CALL_CHANGED;
		break;
	case CTS_SUBSCRIBE_FAVORITE_CHANGE:
		noti = CTS_NOTI_FAVORITE_CHANGED;
		break;
	case CTS_SUBSCRIBE_GROUP_CHANGE:
		noti = CTS_NOTI_GROUP_CHANGED;
		break;
	case CTS_SUBSCRIBE_SPEEDDIAL_CHANGE:
		noti = CTS_NOTI_SPEEDDIAL_CHANGED;
		break;
	case CTS_SUBSCRIBE_ADDRESSBOOK_CHANGE:
		noti = CTS_NOTI_ADDRBOOK_CHANGED;
		break;
	default:
		ERR("Invalid parameter : The type(%d) is not supported", type);
		return NULL;
	}

	return noti;
}

API int contacts_svc_subscribe_change(cts_subscribe_type noti_type,
		void (*cb)(void *), void *user_data)
{
	int ret;
	const char *noti;

	noti = cts_noti_get_file_path(noti_type);
	retvm_if(NULL == noti, CTS_ERR_ARG_INVALID,
			"cts_noti_get_file_path(%d) Failed", noti_type);

	ret = cts_inotify_subscribe(noti, cb, user_data);
	retvm_if(CTS_SUCCESS != ret, ret,
			"cts_inotify_subscribe(%d) Failed(%d)", noti_type, ret);

	return CTS_SUCCESS;
}

API int contacts_svc_unsubscribe_change(cts_subscribe_type noti_type,
		void (*cb)(void *))
{
	int ret;
	const char *noti;

	noti = cts_noti_get_file_path(noti_type);
	retvm_if(NULL == noti, CTS_ERR_ARG_INVALID,
			"cts_noti_get_file_path(%d) Failed", noti_type);

	ret = cts_inotify_unsubscribe(noti, cb);
	retvm_if(CTS_SUCCESS != ret, ret,
			"cts_inotify_unsubscribe(%d) Failed(%d)", noti_type, ret);

	return CTS_SUCCESS;
}

API int contacts_svc_unsubscribe_change_with_data(cts_subscribe_type noti_type,
		void (*cb)(void *), void *user_data)
{
	int ret;
	const char *noti;

	noti = cts_noti_get_file_path(noti_type);
	retvm_if(NULL == noti, CTS_ERR_ARG_INVALID,
			"cts_noti_get_file_path(%d) Failed", noti_type);

	ret = cts_inotify_unsubscribe_with_data(noti, cb, user_data);
	retvm_if(CTS_SUCCESS != ret, ret,
			"cts_inotify_unsubscribe_with_data(%d) Failed(%d)", noti_type, ret);

	return CTS_SUCCESS;
}

int cts_exist_file(char *path)
{
	int fd = open(path, O_RDONLY);
	// errno == ENOENT
	retvm_if(fd < 0, CTS_ERR_FAIL, "Open(%s) Failed(%d)", path, errno);
	close(fd);
	return CTS_SUCCESS;
}

#define CTS_COPY_SIZE_MAX 4096
API int contacts_svc_get_image(cts_img_t img_type, int index, char **img_path)
{
	int ret;
	cts_stmt stmt;
	char *tmp_path;
	char query[CTS_SQL_MIN_LEN] = {0};

	retvm_if(CTS_IMG_FULL != img_type && CTS_IMG_NORMAL != img_type,
			CTS_ERR_ARG_INVALID,
			"You have to use CTS_IMG_FULL or CTS_IMG_NORMAL for img_type");
	retvm_if(NULL == img_path, CTS_ERR_ARG_INVALID, "img_path is NULL");

	snprintf(query, sizeof(query),
				"SELECT image%d FROM %s WHERE contact_id = %d",
				img_type, CTS_TABLE_CONTACTS, index);

	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	*img_path = NULL;
	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret) {
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	tmp_path = cts_stmt_get_text(stmt, 0);
	if (tmp_path) {
		ret = cts_exist_file(tmp_path);
		retvm_if(ret, ret, "cts_exist_file() Failed(%d)", ret);
		*img_path = strdup(tmp_path);
		if (NULL == *img_path) {
			ERR("strdup() Failed");
			cts_stmt_finalize(stmt);
			return CTS_ERR_OUT_OF_MEMORY;
		}
	}
	cts_stmt_finalize(stmt);
	return CTS_SUCCESS;
}

int cts_delete_image_file(int img_type, int index)
{
	int ret;
	cts_stmt stmt;
	char *tmp_path;
	char query[CTS_SQL_MIN_LEN];
	snprintf(query, sizeof(query),
			"SELECT image%d FROM %s WHERE contact_id = %d",
			img_type, CTS_TABLE_CONTACTS, index);

	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		ERR("cts_query_prepare() Failed");
		return CTS_ERR_DB_FAILED;
	}

	ret = cts_stmt_step(stmt);
	if (CTS_TRUE != ret) {
		ERR("cts_stmt_step() Failed(%d)", ret);
		cts_stmt_finalize(stmt);
		return CTS_ERR_DB_RECORD_NOT_FOUND;
	}

	tmp_path = cts_stmt_get_text(stmt, 0);
	if (tmp_path) {
		ret = unlink(tmp_path);
		warn_if (ret < 0, "unlink(%s) Failed(%d)", tmp_path, errno);
	}
	cts_stmt_finalize(stmt);
	return CTS_SUCCESS;
}

int cts_add_image_file(int img_type, int index, char *src_img, char **dest_img)
{
	int src_fd;
	int dest_fd;
	int size;
	int ret;
	char *ext;
	char buf[CTS_COPY_SIZE_MAX];
	char dest[CTS_IMG_PATH_SIZE_MAX];

	*dest_img = NULL;
	retvm_if(NULL == src_img, CTS_ERR_ARG_INVALID, "img_path is NULL");

	ext = strrchr(src_img, '.');
	if (NULL == ext) ext = "";

	size = snprintf(dest, sizeof(dest), "%s/%d-%d%s",
			CTS_IMAGE_LOCATION, index, img_type, ext);
	retvm_if(size<=0, CTS_ERR_FAIL, "Destination file name was not created");

	src_fd = open(src_img, O_RDONLY);
	retvm_if(src_fd < 0, CTS_ERR_IO_ERR, "Open(%s) Failed(%d)", src_img, errno);
	dest_fd = open(dest, O_WRONLY|O_CREAT|O_TRUNC, 0660);
	if (dest_fd < 0) {
		ERR("Open(%s) Failed(%d)", dest, errno);
		close(src_fd);
		return CTS_ERR_FAIL;
	}

	while ((size = read(src_fd, buf, CTS_COPY_SIZE_MAX)) > 0) {
		ret = write(dest_fd, buf, size);
		if (ret <= 0) {
			if (EINTR == errno)
				continue;
			else {
				ERR("write() Failed(%d)", errno);
				if (ENOSPC == errno)
					ret = CTS_ERR_NO_SPACE;
				else
					ret = CTS_ERR_IO_ERR;
				close(src_fd);
				close(dest_fd);
				unlink(dest);
				return ret;
			}
		}
	}

	fchown(dest_fd, getuid(), CTS_SECURITY_FILE_GROUP);
	fchmod(dest_fd, CTS_SECURITY_DEFAULT_PERMISSION);
	close(src_fd);
	close(dest_fd);
	*dest_img = strdup(dest);
	return CTS_SUCCESS;
}

int cts_update_image_file(int img_type, int index, char *src_img, char **dest_img)
{
	int ret;

	*dest_img = NULL;
	ret = cts_delete_image_file(img_type, index);
	retvm_if(CTS_SUCCESS != ret && CTS_ERR_DB_RECORD_NOT_FOUND != ret,
		ret, "cts_delete_image_file() Failed(%d)", ret);

	if (src_img) {
		ret = cts_add_image_file(img_type, index, src_img, &(*dest_img));
		retvm_if(CTS_SUCCESS != ret, ret, "cts_add_image_file() Failed(%d)", ret);
	}

	return ret;
}

int cts_update_contact_changed_time(int contact_id)
{
	int ret;
	char query[CTS_SQL_MIN_LEN];

	snprintf(query, sizeof(query),
			"UPDATE %s SET changed_ver=%d, changed_time=%d WHERE contact_id=%d",
			CTS_TABLE_CONTACTS, cts_get_next_ver(), (int)time(NULL), contact_id);

	ret = cts_query_exec(query);
	retvm_if(CTS_SUCCESS != ret, ret, "cts_query_exec() Failed(%d)", ret);

	cts_set_contact_noti();

	return CTS_SUCCESS;
}

API int contacts_svc_save_image(cts_img_t img_type, int index, char *src_img)
{
	int ret;
	char *dest_img;
	char query[CTS_SQL_MIN_LEN];
	cts_stmt stmt;

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	ret = cts_update_image_file(img_type, index, src_img, &dest_img);
	if (CTS_SUCCESS != ret) {
		ERR("cts_update_image_file() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	snprintf(query, sizeof(query),
			"UPDATE %s SET changed_ver=%d, changed_time=%d, image%d=? WHERE contact_id=%d",
			CTS_TABLE_CONTACTS, cts_get_next_ver(), (int)time(NULL), img_type, index);
	stmt = cts_query_prepare(query);
	if (NULL == stmt) {
		ERR("cts_query_prepare() Failed");
		contacts_svc_end_trans(false);
		return CTS_ERR_DB_FAILED;
	}

	if(dest_img)
		cts_stmt_bind_text(stmt, 1, dest_img);
	ret = cts_stmt_step(stmt);
	warn_if(CTS_SUCCESS != ret, "cts_stmt_step() Failed(%d)", ret);
	cts_stmt_finalize(stmt);

	cts_set_contact_noti();

	ret = contacts_svc_end_trans(true);
	retvm_if(ret < CTS_SUCCESS, ret, "contacts_svc_end_trans() Failed(%d)", ret);

	return CTS_SUCCESS;
}

API int contacts_svc_count_with_int(cts_count_int_op op_code, int search_value)
{
	int ret;
	cts_stmt stmt;
	char query[CTS_SQL_MIN_LEN] = {0};

	switch (op_code)
	{
	case CTS_GET_COUNT_CONTACTS_IN_ADDRESSBOOK:
		snprintf(query, sizeof(query),
				"SELECT COUNT(contact_id) FROM %s "
				"WHERE addrbook_id = ?", CTS_TABLE_CONTACTS);
		break;
	case CTS_GET_COUNT_CONTACTS_IN_GROUP:
		snprintf(query, sizeof(query),
				"SELECT COUNT(contact_id) FROM %s WHERE group_id = ?",
				CTS_TABLE_GROUPING_INFO);
		break;
	case CTS_GET_COUNT_NO_GROUP_CONTACTS_IN_ADDRESSBOOK:
		snprintf(query, sizeof(query),
				"SELECT COUNT(contact_id) FROM %s A "
				"WHERE addrbook_id = ? AND NOT EXISTS "
				"(SELECT contact_id FROM %s WHERE contact_id=A.contact_id LIMIT 1)",
				CTS_TABLE_CONTACTS, CTS_TABLE_GROUPING_INFO);
		break;
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
		return CTS_ERR_ARG_INVALID;
	}
	stmt = cts_query_prepare(query);
	retvm_if(NULL == stmt, CTS_ERR_DB_FAILED, "cts_query_prepare() Failed");

	cts_stmt_bind_int(stmt, 1, search_value);
	ret = cts_stmt_get_first_int_result(stmt);
	if (CTS_ERR_DB_RECORD_NOT_FOUND == ret) return 0;
	else return ret;
}

API int contacts_svc_count(cts_count_op op_code)
{
	int ret;
	char query[CTS_SQL_MIN_LEN] = {0};

	switch ((int)op_code)
	{
	case CTS_GET_ALL_CONTACT:
		snprintf(query, sizeof(query),"SELECT COUNT(*) FROM %s",
				CTS_TABLE_CONTACTS);
		break;
	case CTS_GET_COUNT_SDN:
		snprintf(query, sizeof(query),"SELECT COUNT(*) FROM %s",
				CTS_TABLE_SIM_SERVICES);
		break;
	case CTS_GET_ALL_PHONELOG:
		snprintf(query, sizeof(query), "SELECT COUNT(*) FROM %s",
				CTS_TABLE_PHONELOGS);
		break;
	case CTS_GET_UNSEEN_MISSED_CALL:
		snprintf(query, sizeof(query),
				"SELECT COUNT(*) FROM %s "
				"WHERE log_type = %d OR log_type = %d",
				CTS_TABLE_PHONELOGS,
				CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN, CTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN);
		break;
	default:
		ERR("Invalid parameter : The op_code(%d) is not supported", op_code);
		return CTS_ERR_ARG_INVALID;
	}

	ret = cts_query_get_first_int_result(query);
	if (CTS_ERR_DB_RECORD_NOT_FOUND == ret) return 0;
	else return ret;
}

int cts_convert_nicknames2textlist(GSList *src, char *dest, int dest_size)
{
	int len;
	GSList *nick_repeat = src;
	cts_nickname *nick_data;

	retvm_if(dest_size <= 0 || NULL == dest, CTS_ERR_ARG_INVALID,
			"The parameter is Invalid. dest = %p, dest_size = %d", dest, dest_size);

	len = 0;
	dest[0] = '\0';
	while (nick_repeat) {
		if (NULL != (nick_data = (cts_nickname *)nick_repeat->data) && nick_data->nick) {
			if (!nick_data->deleted)
				len += snprintf(dest+len, dest_size-len, "%s,", nick_data->nick);
		}
		nick_repeat = nick_repeat->next;
	}

	len = strlen(dest);
	if (len)
		dest[len - 1] = '\0';
	else
		return CTS_ERR_NO_DATA;
	return CTS_SUCCESS;
}

GSList* cts_convert_textlist2nicknames(char *text_list)
{
	char temp[CTS_SQL_MAX_LEN], *text_start, *text_end;
	GSList *ret_list = NULL;
	cts_nickname *result;

	snprintf(temp, sizeof(temp), "%s", text_list);

	text_start = temp;
	while (text_start)
	{
		text_end = strchr(text_start, ',');
		if (text_end)
			*text_end = '\0';

		result = (cts_nickname *)contacts_svc_value_new(CTS_VALUE_NICKNAME);
		if (result)
		{
			result->embedded = true;
			result->nick = strdup(text_start);
		}

		ret_list = g_slist_append(ret_list, result);

		if (text_end) {
			*text_end = ',';
			text_start = text_end+1;
		}
		else
			text_start = NULL;
	}
	return ret_list;
}

API int contacts_svc_import_sim(void)
{
	int ret;

	cts_mutex_lock(CTS_MUTEX_SOCKET_FD);
	ret = cts_request_sim_import();
	cts_mutex_unlock(CTS_MUTEX_SOCKET_FD);

	return ret;
}

int cts_increase_outgoing_count(int contact_id)
{
	int ret;
	char query[CTS_SQL_MIN_LEN];

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	snprintf(query, sizeof(query),
			"UPDATE %s SET outgoing_count = outgoing_count + 1 WHERE contact_id = %d",
			CTS_TABLE_CONTACTS, contact_id);

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

API int contacts_svc_reset_outgoing_count(int contact_id)
{
	int ret ;
	char query[CTS_SQL_MAX_LEN];

	snprintf(query, sizeof(query),
			"UPDATE %s SET outgoing_count = 0 WHERE contact_id = %d",
			CTS_TABLE_CONTACTS, contact_id);

	ret = contacts_svc_begin_trans();
	retvm_if(ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	ret = cts_query_exec(query);
	if (CTS_SUCCESS != ret)
	{
		ERR("cts_query_exec() Failed(%d)", ret);
		contacts_svc_end_trans(false);
		return ret;
	}

	ret = contacts_svc_end_trans(true);
	if (ret < CTS_SUCCESS)
		return ret;
	else
		return CTS_SUCCESS;
}

#ifdef CTS_TIMECHECK
double cts_set_start_time(void)
{
	struct timeval tv;
	double curtime;

	gettimeofday(&tv, NULL);
	curtime = tv.tv_sec * 1000 + (double)tv.tv_usec/1000;
	return curtime;
}

double cts_exec_time(double start)
{
	double end = cts_set_start_time();
	return (end - start - correction);
}

int cts_init_time(void)
{
	double temp_t;
	temp_t = cts_set_start_time();
	correction = cts_exec_time(temp_t);

	return 0;
}
#endif


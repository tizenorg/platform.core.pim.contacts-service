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
#include <unistd.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_db_schema.h"
#include "ctsvc_db_sqlite.h"
#include "ctsvc_db_utils.h"
#include "ctsvc_list.h"
#include "ctsvc_db_query.h"
#include "ctsvc_db_init.h"
#include "ctsvc_record.h"
#include "ctsvc_notification.h"
#include "ctsvc_db_access_control.h"
#include "ctsvc_db_plugin_group_helper.h"
#include "ctsvc_notify.h"


static double __ctsvc_db_group_get_next_group_prio(void)
{
	int ret;
	double prio = 0.0;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};

	snprintf(query, sizeof(query), "SELECT MAX(group_prio) FROM "CTS_TABLE_GROUPS" ");

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/  == ret)
		prio = ctsvc_stmt_get_dbl(stmt, 0);
	ctsvc_stmt_finalize(stmt);

	return prio + 1.0;
}

static int __ctsvc_db_group_insert_record(contacts_record_h record, int *id)
{
	int ret;
	int ver;
	cts_stmt stmt;
	double group_prio = 0.0;
	ctsvc_group_s *group = (ctsvc_group_s*)record;
	char query[CTS_SQL_MAX_LEN] = {0};

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(CTSVC_RECORD_GROUP != group->base.r_type, CONTACTS_ERROR_INVALID_PARAMETER,
			"record is invalid type(%d)", group->base.r_type);
	RETVM_IF(NULL == group->name, CONTACTS_ERROR_INVALID_PARAMETER,
			"The name of record is empty.");

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret,  "ctsvc_begin_trans() Fail(%d)", ret);

	if (false == ctsvc_have_ab_write_permission(group->addressbook_id, true)) {
		/* LCOV_EXCL_START */
		ERR("No permission in this addresbook_id(%d)", group->addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
		/* LCOV_EXCL_STOP */
	}

	group_prio = __ctsvc_db_group_get_next_group_prio();
	group->id = ctsvc_db_get_next_id(CTS_TABLE_GROUPS);
	if (id)
		*id = group->id;

	snprintf(query, sizeof(query),
			"INSERT INTO "CTS_TABLE_GROUPS"(group_id, addressbook_id, group_name, created_ver, changed_ver, ringtone_path, "
			"vibration, message_alert, image_thumbnail_path, extra_data, is_read_only, group_prio) "
			"VALUES(%d, %d, ?, ?, ?, ?, ?, ?, ?, ?, %d, %lf)",
			group->id, group->addressbook_id, group->is_read_only, group_prio);

	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ctsvc_stmt_bind_text(stmt, 1, group->name);

	ver = ctsvc_get_next_ver();

	ctsvc_stmt_bind_int(stmt, 2, ver);
	ctsvc_stmt_bind_int(stmt, 3, ver);

	if (group->ringtone_path)
		ctsvc_stmt_bind_text(stmt, 4, group->ringtone_path);
	if (group->vibration)
		ctsvc_stmt_bind_text(stmt, 5, group->vibration);
	if (group->message_alert)
		ctsvc_stmt_bind_text(stmt, 6, group->message_alert);

	if (group->image_thumbnail_path) {
		char image[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
		ret = ctsvc_have_file_read_permission(group->image_thumbnail_path);
		if (ret != CONTACTS_ERROR_NONE) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_have_file_read_permission Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			ctsvc_end_trans(false);
			return ret;
			/* LCOV_EXCL_STOP */
		}

		ctsvc_utils_make_image_file_name(0, group->id, group->image_thumbnail_path, image, sizeof(image));
		ret = ctsvc_utils_copy_image(CTS_GROUP_IMAGE_LOCATION, group->image_thumbnail_path, image);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_utils_copy_image() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			ctsvc_end_trans(false);
			return ret;
			/* LCOV_EXCL_STOP */
		}

		free(group->image_thumbnail_path);
		group->image_thumbnail_path = strdup(image);
		if (group->image_thumbnail_path)
			ctsvc_stmt_bind_text(stmt, 7, group->image_thumbnail_path);
	}

	if (group->extra_data)
		ctsvc_stmt_bind_text(stmt, 8, group->extra_data);

	ret = ctsvc_stmt_step(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ctsvc_set_group_noti();

	ctsvc_stmt_finalize(stmt);

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_end_trans() Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_group_update_record(contacts_record_h record)
{
	int addressbook_id = 0;
	int ret = CONTACTS_ERROR_NONE;
	char *set = NULL;
	GSList *bind_text = NULL;
	GSList *cursor = NULL;
	ctsvc_group_s *group = (ctsvc_group_s*)record;
	char query[CTS_SQL_MAX_LEN] = {0};
	cts_stmt stmt = NULL;
	bool is_read_only = false;
	char *image = NULL;
	char *temp = NULL;

	RETV_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(CTSVC_RECORD_GROUP != group->base.r_type, CONTACTS_ERROR_INVALID_PARAMETER,
			"group is invalid type(%d)", group->base.r_type);
	RETVM_IF(CTSVC_PROPERTY_FLAG_DIRTY != (group->base.property_flag & CTSVC_PROPERTY_FLAG_DIRTY), CONTACTS_ERROR_NONE, "No update");
	RETVM_IF(NULL == group->name, CONTACTS_ERROR_INVALID_PARAMETER,
			"The name of group is empty.");

	ret = ctsvc_begin_trans();
	RETVM_IF(ret, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT addressbook_id, is_read_only, image_thumbnail_path FROM %s WHERE group_id = %d",
			CTS_TABLE_GROUPS, group->id);
	ret = ctsvc_query_prepare(query, &stmt);
	if (NULL == stmt) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_prepare() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_stmt_step(stmt);
	if (1 != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		ctsvc_end_trans(false);
		if (CONTACTS_ERROR_NONE == ret) {
			ERR("The group record(%d) is Invalid(%d)", group->id, ret);
			return CONTACTS_ERROR_NO_DATA;
		} else {
			return ret;
		}
		/* LCOV_EXCL_STOP */
	}

	addressbook_id = ctsvc_stmt_get_int(stmt, 0);
	is_read_only = ctsvc_stmt_get_int(stmt, 1);
	temp = ctsvc_stmt_get_text(stmt, 2);
	image = SAFE_STRDUP(temp);
	ctsvc_stmt_finalize(stmt);

	if (is_read_only && ctsvc_record_check_property_flag((ctsvc_record_s*)record, _contacts_group.name, CTSVC_PROPERTY_FLAG_DIRTY)) {
		/* LCOV_EXCL_START */
		ERR("Can not change the group name. It is a read-only group (group_id : %d)", group->id);
		ctsvc_end_trans(false);
		free(image);
		return CONTACTS_ERROR_INVALID_PARAMETER;
		/* LCOV_EXCL_STOP */
	}

	if (false == ctsvc_have_ab_write_permission(addressbook_id, true)) {
		/* LCOV_EXCL_START */
		ERR("No permission in this addresbook_id(%d)", addressbook_id);
		ctsvc_end_trans(false);
		free(image);
		return CONTACTS_ERROR_PERMISSION_DENIED;
		/* LCOV_EXCL_STOP */
	}

	if (ctsvc_record_check_property_flag((ctsvc_record_s*)group, _contacts_group.image_path, CTSVC_PROPERTY_FLAG_DIRTY)) {
		bool same = false;
		bool check_permission = 0;
		/* delete current image */
		if (image) {
			char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
			snprintf(full_path, sizeof(full_path), "%s/%s", CTS_GROUP_IMAGE_LOCATION, image);

			if (group->image_thumbnail_path && STRING_EQUAL == strcmp(group->image_thumbnail_path, full_path)) {
				int index = _contacts_group.image_path & 0x000000FF;
				((ctsvc_record_s*)record)->properties_flags[index] = 0;
				same = true;
			} else {
				if (group->image_thumbnail_path) {
					ret = ctsvc_have_file_read_permission(group->image_thumbnail_path);
					if (ret != CONTACTS_ERROR_NONE) {
						/* LCOV_EXCL_START */
						ERR("Your module does not have read permission of the image file()");
						ctsvc_end_trans(false);
						free(image);
						return ret;
						/* LCOV_EXCL_STOP */
					}
					check_permission = true;
				}
				ret = unlink(full_path);
				if (ret < 0)
					WARN("unlink() Fail(%d)", errno);
			}
		}

		/* add new image file */
		if (false == same && group->image_thumbnail_path) {
			char dest[CTS_SQL_MAX_LEN] = {0};
			if (false == check_permission) {
				ret = ctsvc_have_file_read_permission(group->image_thumbnail_path);
				if (ret != CONTACTS_ERROR_NONE) {
					/* LCOV_EXCL_START */
					ERR("ctsvc_have_file_read_permission Fail(%d)", ret);
					ctsvc_end_trans(false);
					free(image);
					return ret;
					/* LCOV_EXCL_STOP */
				}
			}
			ctsvc_utils_make_image_file_name(0, group->id, group->image_thumbnail_path, dest, sizeof(dest));
			ret = ctsvc_utils_copy_image(CTS_GROUP_IMAGE_LOCATION, group->image_thumbnail_path, dest);
			if (CONTACTS_ERROR_NONE != ret) {
				/* LCOV_EXCL_START */
				ERR("cts_copy_file() Fail(%d)", ret);
				ctsvc_end_trans(false);
				free(image);
				return ret;
				/* LCOV_EXCL_STOP */
			}

			free(group->image_thumbnail_path);
			group->image_thumbnail_path = strdup(dest);
		}
	}
	free(image);

	do {
		char query[CTS_SQL_MAX_LEN] = {0};
		char query_set[CTS_SQL_MAX_LEN] = {0};
		cts_stmt stmt = NULL;

		if (CONTACTS_ERROR_NONE != (ret = ctsvc_db_create_set_query(record, &set, &bind_text))) break;
		if (NULL == set || '\0' == *set)
			break;
		snprintf(query_set, sizeof(query_set), "%s, changed_ver=%d ", set, ctsvc_get_next_ver());

		snprintf(query, sizeof(query), "UPDATE %s SET %s WHERE group_id = %d", CTS_TABLE_GROUPS, query_set, group->id);
		ret = ctsvc_query_prepare(query, &stmt);
		if (NULL == stmt) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_query_prepare() Fail(%d)", ret);
			break;
			/* LCOV_EXCL_STOP */
		}
		if (bind_text) {
			int i = 0;
			for (cursor = bind_text, i = 1; cursor; cursor = cursor->next, i++) {
				const char *text = cursor->data;
				if (text && *text)
					ctsvc_stmt_bind_text(stmt, i, text);
			}
		}
		ret = ctsvc_stmt_step(stmt);
		if (CONTACTS_ERROR_NONE != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			break;
			/* LCOV_EXCL_STOP */
		}
		ctsvc_stmt_finalize(stmt);

		ctsvc_set_group_noti();
	} while (0);

	CTSVC_RECORD_RESET_PROPERTY_FLAGS((ctsvc_record_s*)record);
	free(set);

	if (bind_text) {
		for (cursor = bind_text; cursor; cursor = cursor->next) {
			free(cursor->data);
			cursor->data = NULL;
		}
		g_slist_free(bind_text);
	}

	if (CONTACTS_ERROR_NONE != ret) {
		ctsvc_end_trans(false);
		return ret;
	}

	ret = ctsvc_end_trans(true);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_end_trans() Fail(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_group_delete_record(int id)
{
	int ret;
	int count = 0;
	int addressbook_id;
	char query[CTS_SQL_MAX_LEN] = {0};

	ret = ctsvc_begin_trans();
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_begin_trans() Fail(%d)", ret);

	snprintf(query, sizeof(query),
			"SELECT addressbook_id FROM %s WHERE group_id = %d",
			CTS_TABLE_GROUPS, id);

	ret = ctsvc_query_get_first_int_result(query, &addressbook_id);
	if (ret < CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("The id(%d) is Invalid(%d)", id, addressbook_id);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	if (false == ctsvc_have_ab_write_permission(addressbook_id, true)) {
		/* LCOV_EXCL_START */
		ERR("No permission in this addresbook_id(%d)", addressbook_id);
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_PERMISSION_DENIED;
		/* LCOV_EXCL_STOP */
	}

	snprintf(query, sizeof(query),
			"SELECT COUNT(contact_id) FROM "CTS_TABLE_GROUP_RELATIONS" WHERE group_id = %d", id);
	ret = ctsvc_query_get_first_int_result(query, &count);
	if (ret < CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_get_first_int_result() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	snprintf(query, sizeof(query), "DELETE FROM %s WHERE group_id=%d AND is_read_only=0",
			CTS_TABLE_GROUPS, id);

	ret = ctsvc_query_exec(query);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_query_exec() Fail(%d)", ret);
		ctsvc_end_trans(false);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = ctsvc_db_change();
	if (ret <= 0) {
		ctsvc_end_trans(false);
		return CONTACTS_ERROR_NO_DATA;
	}

	ctsvc_get_next_ver();

	ctsvc_set_group_noti();
	if (0 < count) {
		ctsvc_set_group_rel_noti();
		ctsvc_set_contact_noti();
		ctsvc_set_person_noti();
	}

	ret = ctsvc_end_trans(true);
	if (ret < CONTACTS_ERROR_NONE) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_end_trans() Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_group_value_set(cts_stmt stmt, contacts_record_h *record)
{
	int i;
	int ret;
	char *temp;
	ctsvc_group_s *group;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	ret = contacts_record_create(_contacts_group._uri, record);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create Fail(%d)", ret);
	group = (ctsvc_group_s*)*record;

	i = 0;
	group->id = ctsvc_stmt_get_int(stmt, i++);
	group->addressbook_id = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	group->name = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	group->extra_data = SAFE_STRDUP(temp);
	group->is_read_only = ctsvc_stmt_get_int(stmt, i++);
	temp = ctsvc_stmt_get_text(stmt, i++);
	group->ringtone_path = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	group->vibration = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	group->message_alert = SAFE_STRDUP(temp);
	temp = ctsvc_stmt_get_text(stmt, i++);
	if (temp) {
		snprintf(full_path, sizeof(full_path), "%s/%s", CTS_GROUP_IMAGE_LOCATION, temp);
		group->image_thumbnail_path = strdup(full_path);
	}

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_group_get_record(int id, contacts_record_h *out_record)
{
	int ret;
	cts_stmt stmt = NULL;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_record_h record;

	RETV_IF(NULL == out_record, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_record = NULL;

	snprintf(query, sizeof(query),
			"SELECT group_id, addressbook_id, group_name, extra_data, is_read_only, "
			"ringtone_path, vibration, message_alert, image_thumbnail_path "
			"FROM "CTS_TABLE_GROUPS" WHERE group_id = %d", id);

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	ret = ctsvc_stmt_step(stmt);
	if (1 /*CTS_TRUE*/ != ret) {
		/* LCOV_EXCL_START */
		ERR("ctsvc_stmt_step() Fail(%d)", ret);
		ctsvc_stmt_finalize(stmt);
		if (CONTACTS_ERROR_NONE == ret)
			return CONTACTS_ERROR_NO_DATA;
		else
			return ret;
		/* LCOV_EXCL_STOP */
	}

	ret = __ctsvc_db_group_value_set(stmt, &record);

	ctsvc_stmt_finalize(stmt);
	if (CONTACTS_ERROR_NONE != ret) {
		/* LCOV_EXCL_START */
		ERR("__ctsvc_db_group_value_set(ALL) Fail(%d)", ret);
		return ret;
		/* LCOV_EXCL_STOP */
	}
	*out_record = record;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_group_get_all_records(int offset, int limit, contacts_list_h *out_list)
{
	int ret;
	int len;
	cts_stmt stmt;
	char query[CTS_SQL_MAX_LEN] = {0};
	contacts_list_h list;

	len = snprintf(query, sizeof(query),
			"SELECT group_id, addressbook_id, group_name, extra_data, is_read_only, "
			"ringtone_path, vibration, message_alert, image_thumbnail_path "
			"FROM "CTS_TABLE_GROUPS);


	len += snprintf(query+len, sizeof(query)-len, " ORDER BY addressbook_id, group_prio");

	if (0 != limit) {
		len += snprintf(query+len, sizeof(query)-len, " LIMIT %d", limit);
		if (0 < offset)
			len += snprintf(query+len, sizeof(query)-len, " OFFSET %d", offset);
	}

	ret = ctsvc_query_prepare(query, &stmt);
	RETVM_IF(NULL == stmt, ret, "ctsvc_query_prepare() Fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
			/* LCOV_EXCL_STOP */
		}
		__ctsvc_db_group_value_set(stmt, &record);

		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_db_group_get_records_with_query(contacts_query_h query,
		int offset, int limit, contacts_list_h *out_list)
{
	int ret;
	int i;
	int field_count;
	ctsvc_query_s *s_query;
	cts_stmt stmt;
	contacts_list_h list;
	ctsvc_group_s *group;
	char full_path[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};

	RETV_IF(NULL == query, CONTACTS_ERROR_INVALID_PARAMETER);
	s_query = (ctsvc_query_s*)query;

	ret = ctsvc_db_make_get_records_query_stmt(s_query, offset, limit, &stmt);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_db_make_get_records_query_stmt fail(%d)", ret);

	contacts_list_create(&list);
	while ((ret = ctsvc_stmt_step(stmt))) {
		contacts_record_h record;
		if (1 != ret) {
			/* LCOV_EXCL_START */
			ERR("ctsvc_stmt_step() Fail(%d)", ret);
			ctsvc_stmt_finalize(stmt);
			contacts_list_destroy(list, true);
			return ret;
			/* LCOV_EXCL_STOP */
		}

		contacts_record_create(_contacts_group._uri, &record);
		group = (ctsvc_group_s*)record;
		if (0 == s_query->projection_count) {
			field_count = s_query->property_count;
		} else {
			field_count = s_query->projection_count;

			int err = ctsvc_record_set_projection_flags(record, s_query->projection,
					s_query->projection_count, s_query->property_count);
			if (CONTACTS_ERROR_NONE != err)
				ASSERT_NOT_REACHED("To set projection is Fail.\n");
		}

		for (i = 0; i < field_count; i++) {
			char *temp;
			int property_id;
			if (0 == s_query->projection_count)
				property_id = s_query->properties[i].property_id;
			else
				property_id = s_query->projection[i];

			switch (property_id) {
			case CTSVC_PROPERTY_GROUP_ID:
				group->id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_GROUP_ADDRESSBOOK_ID:
				group->addressbook_id = ctsvc_stmt_get_int(stmt, i);
				break;
			case CTSVC_PROPERTY_GROUP_NAME:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(group->name);
				group->name = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_GROUP_RINGTONE:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(group->ringtone_path);
				group->ringtone_path = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_GROUP_IMAGE:
				temp = ctsvc_stmt_get_text(stmt, i);
				if (temp) {
					snprintf(full_path, sizeof(full_path), "%s/%s", CTS_GROUP_IMAGE_LOCATION, temp);
					free(group->image_thumbnail_path);
					group->image_thumbnail_path = strdup(full_path);
				}
				break;
			case CTSVC_PROPERTY_GROUP_VIBRATION:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(group->vibration);
				group->vibration = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_GROUP_MESSAGE_ALERT:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(group->message_alert);
				group->message_alert = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_GROUP_EXTRA_DATA:
				temp = ctsvc_stmt_get_text(stmt, i);
				free(group->extra_data);
				group->extra_data = SAFE_STRDUP(temp);
				break;
			case CTSVC_PROPERTY_GROUP_IS_READ_ONLY:
				group->is_read_only = ctsvc_stmt_get_int(stmt, i);
				break;
			default:
				break;
			}
		}
		ctsvc_list_prepend(list, record);
	}
	ctsvc_stmt_finalize(stmt);
	ctsvc_list_reverse(list);

	*out_list = (contacts_list_h)list;

	return CONTACTS_ERROR_NONE;
}

ctsvc_db_plugin_info_s ctsvc_db_plugin_group = {
	.is_query_only = false,
	.insert_record = __ctsvc_db_group_insert_record,
	.get_record = __ctsvc_db_group_get_record,
	.update_record = __ctsvc_db_group_update_record,
	.delete_record = __ctsvc_db_group_delete_record,
	.get_all_records = __ctsvc_db_group_get_all_records,
	.get_records_with_query = __ctsvc_db_group_get_records_with_query,
	.insert_records = NULL,
	.update_records = NULL,
	.delete_records = NULL,
	.get_count = NULL,
	.get_count_with_query = NULL,
	.replace_record = NULL,
	.replace_records = NULL,
};


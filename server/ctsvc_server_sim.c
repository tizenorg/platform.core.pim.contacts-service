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

#include <glib-object.h>
#include <string.h>
#include <tapi_common.h>
#include <ITapiSim.h>
#include <ITapiPhonebook.h>
#include <TapiUtility.h>
#include <vconf.h>
#include <vconf-internal-telephony-keys.h>

#include "contacts.h"

#include "ctsvc_internal.h"
#include "ctsvc_struct.h"
#include "ctsvc_schema.h"
#include "ctsvc_sqlite.h"
#include "ctsvc_server_socket.h"
#include "ctsvc_server_sqlite.h"
#include "ctsvc_server_utils.h"
#include "ctsvc_server_sim.h"
#include "ctsvc_utils.h"
#include "ctsvc_list.h"
#include "ctsvc_db_access_control.h"

//#define CTSVC_SIM_FIELD_FULL_SUPPORT// support ANR,EMAIL2,3,NICK NAME
#define DEFAULT_ADDRESS_BOOK_ID 0

#define CTSVC_TAPI_SIM_PB_MAX 0xFFFF

#define TAPI_PB_MAX_FILE_CNT TAPI_PB_3G_PBC+1

#define CTSVC_2GSIM_NAME TAPI_PB_3G_NAME
#define CTSVC_2GSIM_NUMBER TAPI_PB_3G_NUMBER

typedef struct {
	bool support;
	unsigned int index_max;
	unsigned int text_max;
	unsigned int used_count;
}sim_file_s;

typedef struct {
	// SIM slot number
	int sim_slot_no;

	// SIM info table id
	// it is used when inserting/seaching phonelog
	int sim_info_id;

	// SIM slot id
	char *cp_name;

	// Tapi handle to control each SIM slot
	TapiHandle *handle;

	// SIM type
	TelSimPbType_t sim_type;

	// Each sim file info (max index, max text length, used count)
	sim_file_s file_record[TAPI_PB_MAX_FILE_CNT];

	//To bulk insert SIM contact, Free after insert them
	GSList *import_contacts;

	// Set true after read SIM meta info
	// in case of private : set true after reading all SIM contact and save to DB
	bool initialized;

	// unique info of SIM : iccid
	// It should be save to phone log table
	// in order to find which SIM is used to the call/message log
	char* sim_unique_id;

} ctsvc_sim_info_s;


static GSList *__ctsvc_sim_info = NULL;
static void* greturn_data = NULL;
static bool __ctsvc_tapi_cb = false;
static bool __ctsvc_sim_cb = false;

static void __ctsvc_server_sim_get_meta_info_cb(TapiHandle *handle, int result, void *data, void *user_data);
static int __ctsvc_server_sim_init_info(ctsvc_sim_info_s *info);

static TapiHandle* __ctsvc_server_sim_get_tapi_handle(ctsvc_sim_info_s *info)
{
	if (info->handle == NULL) {
		int bReady = 0;
		// TODO: it should be changed API
		vconf_get_bool(VCONFKEY_TELEPHONY_READY, &bReady);

		if (0 == bReady) {
			CTS_ERR("telephony is not ready ");
			return NULL;
		}
		else {
			info->handle = tel_init(info->cp_name);
			RETVM_IF(NULL == info->handle, NULL, "tel_init() Fail");
		}
	}

	return info->handle;
}

static inline void __ctsvc_server_sim_set_return_data(void* data)
{
	greturn_data = data;
}

static inline void* __ctsvc_server_sim_get_return_data(void)
{
	RETVM_IF(greturn_data == NULL, NULL, "greturn_data is NULL");
	return greturn_data;
}

void ctsvc_server_sim_record_destroy(sim_contact_s *record)
{
	RETM_IF(record == NULL, "record is NULL");

	free(record->name);
	free(record->number);
	free(record->anr1);
	free(record->anr2);
	free(record->anr3);
	free(record->email1);
	free(record->email2);
	free(record->email3);
	free(record->email4);
	free(record);

	return;
}

static sim_contact_s * __ctsvc_server_sim_record_clone(TelSimPbRecord_t *sim_record)
{
	sim_contact_s *record = calloc(1,sizeof(sim_contact_s));

	record->sim_index = sim_record->index;
	record->name = SAFE_STRDUP((char*)sim_record->name);
	record->nickname = SAFE_STRDUP((char*)sim_record->sne);
	record->number = SAFE_STRDUP((char*)sim_record->number);
	record->anr1 = SAFE_STRDUP((char*)sim_record->anr1);
	record->anr2 = SAFE_STRDUP((char*)sim_record->anr2);
	record->anr3 = SAFE_STRDUP((char*)sim_record->anr3);
	record->email1 = SAFE_STRDUP((char*)sim_record->email1);
	record->email2 = SAFE_STRDUP((char*)sim_record->email2);
	record->email3 = SAFE_STRDUP((char*)sim_record->email3);
	record->email4 = SAFE_STRDUP((char*)sim_record->email4);

	return record;
}

static inline int __ctsvc_server_sim_record_add_num(contacts_record_h *record, char *number)
{
	int ret;

	RETVM_IF(number == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "invalid number");

	ret = contacts_record_create(_contacts_number._uri, record);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);
	ret = contacts_record_set_str(*record, _contacts_number.number, number);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_set_str() Fail(%d)", ret);
	ret = contacts_record_set_int(*record, _contacts_number.type, CONTACTS_NUMBER_TYPE_OTHER);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_set_int() Fail(%d)", ret);

	return ret;
}

static inline int __ctsvc_server_sim_record_add_email(contacts_record_h *record, char *email)
{
	int ret;

	RETVM_IF(email == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "invalid email");

	ret = contacts_record_create(_contacts_email._uri, record);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);
	ret = contacts_record_set_str(*record, _contacts_email.email, email);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_set_str() Fail(%d)", ret);
	ret = contacts_record_set_int(*record, _contacts_email.type,CONTACTS_EMAIL_TYPE_OTHER);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_set_int() Fail(%d)", ret);

	return ret;
}

static int __ctsvc_server_sim_ctsvc_record_clone(sim_contact_s *record,
	int addressbook_id, contacts_record_h *contact)
{
	CTS_FN_CALL;
	int ret;
	char sim_id[20] = {0};
	contacts_record_h name = NULL;
	contacts_record_h number = NULL;
	contacts_record_h email = NULL;

	RETVM_IF(contact == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : contact is NULL");
	*contact = NULL;
	RETVM_IF(record == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : record is NULL");
	RETVM_IF(record->sim_index <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "The index(%d) is invalid", record->sim_index);

	CTS_DBG("insert record->sim_index %d", record->sim_index);

	ret = contacts_record_create(_contacts_contact._uri, contact);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create Fail(%d)", ret);
	snprintf(sim_id, sizeof(sim_id), "%d", record->sim_index);
	contacts_record_set_str(*contact, _contacts_contact.uid, sim_id);

	if (record->name) {
		contacts_record_create(_contacts_name._uri, &name);
		if (name) {
			contacts_record_set_str(name, _contacts_name.first, (char *)record->name);
			contacts_record_add_child_record(*contact, _contacts_contact.name, name);
		}
	}

#ifdef CTSVC_SIM_FIELD_FULL_SUPPORT
	contacts_record_h nick = NULL;
	if (record->nickname) {
		contacts_record_create(_contacts_nickname._uri, &nick);
		if (nick) {
			contacts_record_set_str(nick, _contacts_nickname.name, (char *)record->nickname);
			contacts_record_add_child_record(*contact, _contacts_contact.nickname, nick);
		}
	}
#endif //CTSVC_SIM_FIELD_FULL_SUPPORT

	ret = __ctsvc_server_sim_record_add_num(&number, (char *)record->number);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(*contact, _contacts_contact.number, number);

#ifdef CTSVC_SIM_FIELD_FULL_SUPPORT
	ret = __ctsvc_server_sim_record_add_num(&number, (char *)record->anr1);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(*contact, _contacts_contact.number, number);

	ret = __ctsvc_server_sim_record_add_num(&number, (char *)record->anr2);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(*contact, _contacts_contact.number, number);

	ret = __ctsvc_server_sim_record_add_num(&number, (char *)record->anr3);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(*contact, _contacts_contact.number, number);
#endif //CTSVC_SIM_FIELD_FULL_SUPPORT

	ret = __ctsvc_server_sim_record_add_email(&email, (char *)record->email1);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(*contact, _contacts_contact.email, email);

#ifdef CTSVC_SIM_FIELD_FULL_SUPPORT
	ret = __ctsvc_server_sim_record_add_email(&email, (char *)record->email2);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(*contact, _contacts_contact.email, email);

	ret = __ctsvc_server_sim_record_add_email(&email, (char *)record->email3);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(*contact, _contacts_contact.email, email);

	ret = __ctsvc_server_sim_record_add_email(&email, (char *)record->email4);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(*contact, _contacts_contact.email, email);
#endif //CTSVC_SIM_FIELD_FULL_SUPPORT

	contacts_record_set_int(*contact, _contacts_contact.address_book_id, addressbook_id);

	return ret;
}

static ctsvc_sim_info_s* __ctsvc_server_sim_get_handle_by_tapi_handle(TapiHandle *handle)
{
	GSList *cursor = NULL;
	for (cursor=__ctsvc_sim_info;cursor;cursor=cursor->next) {
		ctsvc_sim_info_s *info = cursor->data;
		if (info->handle == handle)
			return info;
	}
	return NULL;
}

static ctsvc_sim_info_s* __ctsvc_server_sim_get_handle_by_sim_slot_no(int slot_no)
{
	GSList *cursor = NULL;

	if (slot_no < 0)
		return NULL;

	for (cursor=__ctsvc_sim_info;cursor;cursor=cursor->next) {
		ctsvc_sim_info_s *info = cursor->data;
		if (info->sim_slot_no == slot_no) {
			if (NULL == __ctsvc_server_sim_get_tapi_handle(info))
				return NULL;
			return info;
		}
	}
	return NULL;
}

int ctsvc_server_sim_get_info_id_by_sim_slot_no(int slot_no)
{
	GSList *cursor = NULL;

	if (slot_no < 0)
		return -1;

	for (cursor=__ctsvc_sim_info;cursor;cursor=cursor->next) {
		ctsvc_sim_info_s *info = cursor->data;
		if (info->sim_slot_no == slot_no) {
			if (NULL == __ctsvc_server_sim_get_tapi_handle(info))
				return -1;
			return info->sim_info_id;
		}
	}
	return -1;
}

int ctsvc_server_sim_get_sim_slot_no_by_info_id(int sim_info_id)
{
	GSList *cursor = NULL;
	for (cursor=__ctsvc_sim_info;cursor;cursor=cursor->next) {
		ctsvc_sim_info_s *info = cursor->data;
		if (info->sim_info_id == sim_info_id) {
			if (NULL == __ctsvc_server_sim_get_tapi_handle(info))
				return -1;
			return info->sim_slot_no;
		}
	}
	return -1;
}

static int __ctsvc_server_sim_insert_records_to_db(ctsvc_sim_info_s *info)
{
	CTS_FN_CALL;
	int i;
	int ret = 0;
	sim_contact_s *record = NULL;
	contacts_record_h contact = NULL;
	contacts_list_h list = NULL;
	GSList *cursor = NULL;

	// insert contacts to DB
	for (cursor = info->import_contacts, i=0;cursor;i++) {
		if (list == NULL)
			contacts_list_create(&list);
		ret = __ctsvc_server_sim_ctsvc_record_clone(cursor->data, DEFAULT_ADDRESS_BOOK_ID, &contact);
		contacts_list_add(list, contact);
		if (i == 50) {
			ret = contacts_db_insert_records(list, NULL, NULL);
			WARN_IF(ret < CONTACTS_ERROR_NONE, "contacts_db_insert_records() Fail(%d)", ret);
			contacts_list_destroy(list, true);
			list = NULL;
			i = 0;
		}
		cursor = cursor->next;
		info->import_contacts = g_slist_remove(info->import_contacts, record);
		ctsvc_server_sim_record_destroy(record);
	}
	g_slist_free(info->import_contacts);
	info->import_contacts = NULL;

	if (list) {
		ret = contacts_db_insert_records(list, NULL, NULL);
		WARN_IF(ret < CONTACTS_ERROR_NONE, "contacts_db_insert_records() Fail(%d)", ret);
		contacts_list_destroy(list, true);
	}

	return CONTACTS_ERROR_NONE;
}

static void __ctsvc_server_sim_import_contact_cb(TapiHandle *handle, int result, void *data, void* user_data)
{
	CTS_FN_CALL;
	int ret = 0;
	TelSimPbAccessResult_t access_rt = result;
	TelSimPbRecord_t *sim_info = data;
	sim_contact_s *record = NULL;
	ctsvc_sim_info_s *info;

	if (NULL == sim_info) {
		CTS_ERR("sim_info is NULL, result = %d", access_rt);
		ret = CONTACTS_ERROR_SYSTEM;
		goto ERROR_RETURN;
	}

	info = __ctsvc_server_sim_get_handle_by_tapi_handle(handle);
	if (info == NULL) {
		CTS_ERR("__ctsvc_server_sim_get_handle_by_tapi_handle fail");
		ret = CONTACTS_ERROR_SYSTEM;
		goto ERROR_RETURN;
	}

	if (access_rt == TAPI_SIM_PB_INVALID_INDEX) {
		int start_index = 0;
		if (user_data)
			start_index = (int)user_data;
		CTS_DBG("TAPI_SIM_PB_INVALID_INDEX : start_index = %d",start_index);
		start_index++;
		if (info->file_record[TAPI_PB_3G_NAME].index_max < start_index) {
			CTS_ERR("start_index is invalid start_index = %d, total = %d", start_index,
						info->file_record[TAPI_PB_3G_NAME].index_max);
			ret = CONTACTS_ERROR_SYSTEM;
			goto ERROR_RETURN;
		}
		ret = tel_read_sim_pb_record(handle, info->sim_type, start_index,
					__ctsvc_server_sim_import_contact_cb, (void*)start_index);
		if (ret != TAPI_API_SUCCESS) {
			CTS_ERR("SIM phonebook access Fail(%d) start_indext(%d)", access_rt,start_index);
			ret = CONTACTS_ERROR_SYSTEM;
			goto ERROR_RETURN;
		}
	}

	if (TAPI_SIM_PB_SUCCESS != access_rt) {
		CTS_ERR("SIM phonebook access Fail(%d)", access_rt);
		ret = CONTACTS_ERROR_SYSTEM;
		goto ERROR_RETURN;
	}

	switch (sim_info->phonebook_type) {
	case TAPI_SIM_PB_ADN:
	case TAPI_SIM_PB_3GSIM:
		record = __ctsvc_server_sim_record_clone(sim_info);
		info->import_contacts = g_slist_append(info->import_contacts, (void*)record);
		break;
	case TAPI_SIM_PB_FDN:
	case TAPI_SIM_PB_SDN:
	default:
		CTS_ERR("Unknown storage type(%d)", sim_info->phonebook_type);
		ret = CONTACTS_ERROR_SYSTEM;
		goto ERROR_RETURN;
	}

	if (sim_info->next_index && CTSVC_TAPI_SIM_PB_MAX != sim_info->next_index) {
		CTS_DBG("NextIndex = %d", sim_info->next_index);
		ret = tel_read_sim_pb_record(__ctsvc_server_sim_get_tapi_handle(info), sim_info->phonebook_type,
				sim_info->next_index, __ctsvc_server_sim_import_contact_cb, NULL);
		if (TAPI_API_SUCCESS != ret) {
			CTS_ERR("tel_read_sim_pb_record() Fail(%d)", ret);
			ret = CONTACTS_ERROR_SYSTEM;
			goto ERROR_RETURN;
		}
	}
	else {
		// insert imported contact to DB
		__ctsvc_server_sim_insert_records_to_db(info);
		if (__ctsvc_server_sim_get_return_data()) {
			ret = ctsvc_server_socket_return(__ctsvc_server_sim_get_return_data(), CONTACTS_ERROR_NONE, 0, NULL);
			WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_server_socket_return() Fail(%d)", ret);
			__ctsvc_server_sim_set_return_data(NULL);
		}
	}
	return;

ERROR_RETURN:
	if (__ctsvc_server_sim_get_return_data()) {
		ret = ctsvc_server_socket_return(__ctsvc_server_sim_get_return_data(), ret, 0, NULL);
		WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_server_socket_return() Fail(%d)", ret);
		__ctsvc_server_sim_set_return_data(NULL);
	}
	return;
}

int ctsvc_server_sim_import_contact(void* data, int sim_slot_no)
{
	CTS_FN_CALL;
	int ret;
	ctsvc_sim_info_s *info;

	info = __ctsvc_server_sim_get_handle_by_sim_slot_no(sim_slot_no);
	RETVM_IF(NULL == info, CONTACTS_ERROR_SYSTEM, "sim init is not completed");
	RETVM_IF(false == info->initialized, CONTACTS_ERROR_SYSTEM, "sim init is not completed");
	RETVM_IF(__ctsvc_server_sim_get_return_data(), CONTACTS_ERROR_INTERNAL,
			"Server is already processing with sim");

	__ctsvc_server_sim_set_return_data(data);

	if (info->file_record[TAPI_PB_3G_NAME].used_count == 0) {
		ret = ctsvc_server_socket_return(__ctsvc_server_sim_get_return_data(), CONTACTS_ERROR_NO_DATA, 0, NULL);
		WARN_IF(CONTACTS_ERROR_SYSTEM != ret, "helper_socket_return() Fail(%d)", ret);
		__ctsvc_server_sim_set_return_data(NULL);
		return ret;
	}

	ret = tel_read_sim_pb_record(__ctsvc_server_sim_get_tapi_handle(info), info->sim_type, 1,
			__ctsvc_server_sim_import_contact_cb, NULL);
	if (ret != TAPI_API_SUCCESS) {
		CTS_ERR("tel_read_sim_pb_record = %d",ret);
		__ctsvc_server_sim_set_return_data(NULL);
		return CONTACTS_ERROR_SYSTEM;
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_server_socket_get_sim_init_status(void* data, int sim_slot_no)
{
	CTS_FN_CALL;
	ctsvc_sim_info_s *info;

	info = __ctsvc_server_sim_get_handle_by_sim_slot_no(sim_slot_no);
	RETVM_IF(NULL == info, CONTACTS_ERROR_SYSTEM, "sim init is not completed");
	return ctsvc_server_socket_return_sim_int(data, (int)(info->initialized));
}

static void __ctsvc_server_sim_sdn_read_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
	CTS_FN_CALL;
	int ret=0;
	TelSimPbAccessResult_t sec_rt = result;
	TelSimPbRecord_t *sim_info = data;
	ctsvc_sim_info_s *info = (ctsvc_sim_info_s*)user_data;

	RETM_IF(sim_info == NULL,  "sim_info is NULL");

	if (TAPI_SIM_PB_SUCCESS != sec_rt) {
		if (TAPI_SIM_PB_SDN == sim_info->phonebook_type &&
				TAPI_SIM_PB_INVALID_INDEX == sec_rt) {
			CTS_DBG("Index = %d", sim_info->index);
			ret = tel_read_sim_pb_record(handle, sim_info->phonebook_type, sim_info->index+1,
					__ctsvc_server_sim_sdn_read_cb, info);
			RETM_IF(ret != TAPI_API_SUCCESS,  "tel_read_sim_pb_record() Fail(%d)", ret);
		}
		CTS_ERR("SIM phonebook access Fail(%d)", sec_rt);
		goto ERROR_RETURN;
	}

	switch (sim_info->phonebook_type) {
	case TAPI_SIM_PB_SDN:
		ret = ctsvc_server_insert_sdn_contact((char *)sim_info->name, (char *)sim_info->number, info->sim_slot_no);
		WARN_IF(ret != CONTACTS_ERROR_NONE, "ctsvc_server_insert_sdn_contact() Fail(%d)", ret);
		break;
	case TAPI_SIM_PB_ADN:
	case TAPI_SIM_PB_3GSIM:
	case TAPI_SIM_PB_FDN:
	default:
		CTS_ERR("Not SDN type(%d)", sim_info->phonebook_type);
		goto ERROR_RETURN;
	}

	if (sim_info->next_index && CTSVC_TAPI_SIM_PB_MAX != sim_info->next_index) {
		CTS_DBG("NextIndex = %d", sim_info->next_index);
		ret = tel_read_sim_pb_record(handle, sim_info->phonebook_type,sim_info->next_index,
				__ctsvc_server_sim_sdn_read_cb, info);
		if (TAPI_API_SUCCESS != ret) {
			CTS_ERR("tel_read_sim_pb_record() Fail(%d)", ret);
			goto ERROR_RETURN;
		}
	}
	else {
		ctsvc_server_trim_memory();
	}
	return;
ERROR_RETURN:
	ctsvc_server_trim_memory();
}

static void __ctsvc_server_sim_sdn_meta_info_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
	CTS_FN_CALL;
	int ret = 0;
	TelSimPbAccessResult_t sec_rt = result;
	TelSimPbEntryInfo_t *pe = data;
	ctsvc_sim_info_s *info = (ctsvc_sim_info_s*)user_data;

	RETM_IF(TAPI_SIM_PB_SUCCESS != sec_rt, "TelSimPbAccessResult_t fail(%d)",sec_rt);
	RETM_IF(pe == NULL, "pe is NULL result =%d",sec_rt);

	if (0 < pe->PbUsedCount) {
		ret = tel_read_sim_pb_record(info->handle, TAPI_SIM_PB_SDN, 1, __ctsvc_server_sim_sdn_read_cb, info);
		if (TAPI_API_SUCCESS != ret) {
			CTS_ERR("tel_read_sim_pb_record() Fail(%d)", ret);
			goto ERROR_RETURN;
		}
	}
	else
		CTS_ERR("pe->PbUsedCount : 0 no sdn!!!!", ret);

ERROR_RETURN:
	ctsvc_server_trim_memory();
}

static int __ctsvc_server_sim_sdn_read(ctsvc_sim_info_s* info)
{
	CTS_FN_CALL;
	int ret;
	int card_changed = 0;
	TelSimCardStatus_t sim_status;

	RETVM_IF(__ctsvc_server_sim_get_return_data(), CONTACTS_ERROR_INTERNAL,
			"Server is already processing with sim");

	ret = tel_get_sim_init_info(info->handle, &sim_status, &card_changed);
	if (TAPI_API_SUCCESS != ret) {
		CTS_ERR("tel_get_sim_init_info() Fail(%d)", ret);
		CTS_DBG("sim_status = %d, card_changed = %d", sim_status, card_changed);
		goto ERROR_RETURN;
	}

	if (TAPI_SIM_STATUS_CARD_NOT_PRESENT == sim_status ||
			TAPI_SIM_STATUS_CARD_REMOVED == sim_status) {
		ret = ctsvc_server_delete_sdn_contact(info->sim_slot_no);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_server_delete_sdn_contact() Fail(%d)", ret);
			goto ERROR_RETURN;
		}
	}
	else if (TAPI_SIM_STATUS_SIM_INIT_COMPLETED == sim_status) {
		ret = ctsvc_server_delete_sdn_contact(info->sim_slot_no);
		if (CONTACTS_ERROR_NONE != ret) {
			CTS_ERR("ctsvc_server_delete_sdn_contact() Fail(%d)", ret);
			goto ERROR_RETURN;
		}

		ret = tel_get_sim_pb_meta_info(info->handle, TAPI_SIM_PB_SDN, __ctsvc_server_sim_sdn_meta_info_cb, info);
		if (TAPI_API_SUCCESS != ret) {
			CTS_ERR("tel_get_sim_pb_meta_info() Fail(%d)", ret);
			goto ERROR_RETURN;
		}
	}
	else
		CTS_ERR("sim_status Fail(%d)", sim_status);
	return CONTACTS_ERROR_NONE;

ERROR_RETURN:
	ctsvc_server_trim_memory();
	return ret;
}

static void __ctsvc_server_sim_get_meta_info_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
	CTS_FN_CALL;
	int ret=0;
	int i=0;
	int type = TAPI_PB_3G_NAME;
	TelSimPbAccessResult_t access_rt = result;
	ctsvc_sim_info_s *info = (ctsvc_sim_info_s*)user_data;

	CTS_DBG("sim slot id :%d, sim_type = %d", info->sim_slot_no, info->sim_type);
	if (TAPI_SIM_PB_3GSIM == info->sim_type) {
		TelSimPbCapabilityInfo_t *capa = data;
		RETM_IF(capa == NULL, "capa is NULL result =%d",access_rt);

		for (i=0; i < capa->FileTypeCount; i++) {
			type = capa->FileTypeInfo[i].field_type;

			info->file_record[type].support = true;
			info->file_record[type].index_max = capa->FileTypeInfo[i].index_max;
			info->file_record[type].text_max = capa->FileTypeInfo[i].text_max;
			info->file_record[type].used_count = capa->FileTypeInfo[i].used_count;

			INFO(" field_type[%d] : index_max(%d), text_max(%d), used_count(%d)", type,
					info->file_record[type].index_max,
					info->file_record[type].text_max,
					info->file_record[type].used_count);
		}
	}
	else if (TAPI_SIM_PB_ADN == info->sim_type) {
		TelSimPbEntryInfo_t *pe = data;
		RETM_IF(pe == NULL, "pe is NULL result =%d",access_rt);

		info->file_record[CTSVC_2GSIM_NAME].support = true;
		info->file_record[CTSVC_2GSIM_NAME].index_max = pe->PbIndexMax;
		info->file_record[CTSVC_2GSIM_NAME].text_max = pe->PbTextLenMax;
		info->file_record[CTSVC_2GSIM_NAME].used_count = pe->PbUsedCount;
		CTS_DBG(" CTSVC_2GSIM_NAME : index_max(%d), text_max(%d), used_count(%d)",
				pe->PbIndexMax,pe->PbTextLenMax,pe->PbUsedCount);

		info->file_record[CTSVC_2GSIM_NUMBER].support = true;
		info->file_record[CTSVC_2GSIM_NUMBER].index_max = pe->PbIndexMax;
		info->file_record[CTSVC_2GSIM_NUMBER].text_max = pe->PbNumLenMax;
		info->file_record[CTSVC_2GSIM_NUMBER].used_count = pe->PbUsedCount;
		CTS_DBG(" CTSVC_2GSIM_NUMBER : index_max(%d), text_max(%d), used_count(%d)",
				pe->PbIndexMax,pe->PbNumLenMax,pe->PbUsedCount);

		INFO(" field_type[%d] : index_max(%d), text_max(%d), used_count(%d)", CTSVC_2GSIM_NAME,
				info->file_record[CTSVC_2GSIM_NAME].index_max,
				info->file_record[CTSVC_2GSIM_NAME].text_max,
				info->file_record[CTSVC_2GSIM_NAME].used_count);
	}
	else {
		CTS_ERR("sim_type [%d]error ", info->sim_type);
		return;
	}

	if (false == info->initialized) {
		info->initialized = true;
		ret = __ctsvc_server_sim_sdn_read(info);
		WARN_IF(CONTACTS_ERROR_NONE != ret, "__ctsvc_server_sim_sdn_read() Fail(%d)", ret);
	}
}

static void __ctsvc_server_sim_get_iccid_cb(TapiHandle *handle, int result, void *data,void *user_data)
{
	TelSimAccessResult_t access_rt = result;
	TelSimIccIdInfo_t *iccid = data;
	ctsvc_sim_info_s *info = user_data;
	int ret;
	int id;

	RETM_IF(access_rt != TAPI_SIM_ACCESS_SUCCESS, "tel_get_sim_iccid fail(%d)", access_rt);

	CTS_DBG("%d, %s", iccid->icc_length, iccid->icc_num);

	info->sim_unique_id = strdup(iccid->icc_num);

	ret = ctsvc_server_get_sim_id(info->sim_unique_id, &id);
	if (CONTACTS_ERROR_NONE == ret)
		info->sim_info_id = id;
}

static int __ctsvc_server_sim_init_meta_info(ctsvc_sim_info_s *info)
{
	int ret = TAPI_API_SUCCESS;
	int err = CONTACTS_ERROR_NONE;

	if (info->sim_type == TAPI_SIM_PB_UNKNOWNN) {
		err = __ctsvc_server_sim_init_info(info);
		WARN_IF(CONTACTS_ERROR_NONE != err, "__ctsvc_server_sim_init_info() Fail(%d)", err);
	}

	if (info->sim_type == TAPI_SIM_PB_3GSIM) {
		ret = tel_get_sim_pb_usim_meta_info(info->handle, __ctsvc_server_sim_get_meta_info_cb, info);
	}
	else if (info->sim_type == TAPI_SIM_PB_ADN) {
		ret = tel_get_sim_pb_meta_info(info->handle, info->sim_type, __ctsvc_server_sim_get_meta_info_cb, info);
	}
	else {
		CTS_ERR("info->sim_type is invalid(%d) stop sim init !!!", info->sim_type);
		return CONTACTS_ERROR_SYSTEM;
	}

	RETVM_IF(ret != TAPI_API_SUCCESS, CONTACTS_ERROR_SYSTEM,
				"tel_get_sim_(usim)_meta_info(type:%d) fail (%d)", info->sim_type, ret);

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_server_sim_init_info(ctsvc_sim_info_s *info)
{
	CTS_FN_CALL;
	int ret;
	TelSimCardType_t cardtype = TAPI_SIM_PB_UNKNOWNN;

	// get sim_type
	ret = tel_get_sim_type(info->handle, &cardtype);
	RETVM_IF(ret != TAPI_API_SUCCESS, CONTACTS_ERROR_SYSTEM, "tel_get_sim_type Fail(%d)slot no(%d)", ret, info->sim_slot_no);
	if (cardtype == TAPI_SIM_CARD_TYPE_USIM) {
		info->sim_type = TAPI_SIM_PB_3GSIM;
	}
	else if (cardtype == TAPI_SIM_CARD_TYPE_GSM) {
		info->sim_type = TAPI_SIM_PB_ADN;
	}
	else {
		CTS_ERR("cardtype(%d)is invalid!!!", cardtype);
		return CONTACTS_ERROR_SYSTEM;
	}

	// set iccid : unique info of SIM
	ret = tel_get_sim_iccid (info->handle, __ctsvc_server_sim_get_iccid_cb, info);
	RETVM_IF(ret != TAPI_API_SUCCESS, CONTACTS_ERROR_SYSTEM, "tel_get_sim_iccid Fail(%d)", ret);

	return CONTACTS_ERROR_NONE;
}

static void __ctsvc_server_sim_data_remove(ctsvc_sim_info_s *info)
{
	GSList *cursor;
	sim_contact_s *record = NULL;

	for (cursor=info->import_contacts;cursor;cursor=cursor->next) {
		record = cursor->data;
		info->import_contacts = g_slist_remove(info->import_contacts, record);
		ctsvc_server_sim_record_destroy(record);
	}
	g_slist_free(info->import_contacts);
	info->import_contacts = NULL;
}

static void __ctsvc_server_sim_noti_pb_status(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	CTS_FN_CALL;
	int ret = 0;
	ctsvc_sim_info_s *info = (ctsvc_sim_info_s*)user_data;
	TelSimPbStatus_t *pb_status = (TelSimPbStatus_t *)data;
	RETM_IF(pb_status == NULL, "pb_status is null");
	INFO("received pb status noti : b_3g[%d], b_adn[%d], b_fdn[%d]", pb_status->pb_list.b_3g, pb_status->pb_list.b_adn, pb_status->pb_list.b_fdn);

	if (pb_status->pb_list.b_3g == 1 || pb_status->pb_list.b_adn == 1) {
		if (info->initialized)
			return;
		ret = __ctsvc_server_sim_init_meta_info(info);
		WARN_IF(CONTACTS_ERROR_NONE != ret, "__ctsvc_server_sim_init_meta_info() Fail(%d)", ret);
	}
	// FDN on : can not import sim contacts
	else if (pb_status->pb_list.b_fdn ==1 && pb_status->pb_list.b_adn == 0 && info->sim_type == TAPI_SIM_PB_ADN) {
		CTS_INFO("This is sim card is 2G and FDN on status. sim phonebook will be block");
		info->initialized = false;
		__ctsvc_server_sim_data_remove(info);
	}
	else {
		CTS_ERR("This noti did not control !!!");
	}
}

static void __ctsvc_server_sim_noti_sim_refreshed(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	CTS_FN_CALL;
	ctsvc_sim_info_s *info = (ctsvc_sim_info_s*)user_data;

	INFO("Recieved SIM Refresh event");
	info->initialized = false;
	__ctsvc_server_sim_data_remove(info);
}

static int __ctsvc_server_sim_info_init()
{
	int ret;
	int sim_stat = -1;
	TelSimPbList_t pb_list = {0,};
	char **cp_name = NULL;
	int i;
	unsigned int cp_index = 0;

#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init();
#endif
	cp_name = tel_get_cp_name_list();
	RETVM_IF(cp_name == NULL, CONTACTS_ERROR_SYSTEM, "tel_get_cp_name_list fail (cp_name is NULL)");

	ret = ctsvc_server_delete_sdn_contact(-1);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "ctsvc_server_delete_sdn_contact() Fail(%d)", ret);

	while (cp_name[cp_index]) {
		cp_index++;
	}

	cp_index = 0;
	while (cp_name[cp_index]) {
		TapiHandle *handle;
		ctsvc_sim_info_s *info = calloc(1, sizeof(ctsvc_sim_info_s));
		info->cp_name = strdup(cp_name[cp_index]);
		INFO("SIM cp_name[%d] : %s", cp_index, info->cp_name);
		info->sim_slot_no = cp_index;
		info->sim_info_id = -1;
		info->sim_type = TAPI_SIM_PB_UNKNOWNN;
		info->initialized = false;
		info->import_contacts = NULL;
		info->sim_unique_id = NULL;

		// initialize file_record meta info
		for (i = 0 ;i <TAPI_PB_MAX_FILE_CNT; i++) {
			info->file_record[i].support = false;
			info->file_record[i].index_max = 0;
			info->file_record[i].text_max = 0;
			info->file_record[i].used_count = 0;
		}

		handle = __ctsvc_server_sim_get_tapi_handle(info);
		if (handle) {
			ret = __ctsvc_server_sim_init_info(info);
			WARN_IF(CONTACTS_ERROR_NONE != ret, "__ctsvc_server_sim_init_info() Fail(%d)", ret);
			ret = tel_get_sim_pb_init_info(handle, &sim_stat, &pb_list);
			if (TAPI_API_SUCCESS == ret && (pb_list.b_3g == 1 || pb_list.b_adn == 1)) {
				ret = __ctsvc_server_sim_init_meta_info(info);
				WARN_IF(CONTACTS_ERROR_NONE != ret, "__ctsvc_server_sim_init_meta_info() Fail(%d)", ret);
			}

			ret = tel_register_noti_event(handle, TAPI_NOTI_PB_STATUS, __ctsvc_server_sim_noti_pb_status, info);
			WARN_IF(TAPI_API_SUCCESS != ret, "tel_register_noti_event() Fail(%d)", ret);

			ret = tel_register_noti_event(handle, TAPI_NOTI_SIM_REFRESHED, __ctsvc_server_sim_noti_sim_refreshed, info);
			WARN_IF(TAPI_API_SUCCESS != ret, "tel_register_noti_event() Fail(%d)", ret);
		}
		else {
			CTS_ERR("tel_init fail");
		}

		__ctsvc_sim_info = g_slist_append(__ctsvc_sim_info, (void*)info);
		cp_index++;
	}

	g_strfreev(cp_name);

	return CONTACTS_ERROR_NONE;
}

static void __ctsvc_server_sim_ready_cb(keynode_t *key, void *data)
{
	int status = 0;
	vconf_get_int(VCONFKEY_TELEPHONY_SIM_STATUS, &status);

	if (VCONFKEY_TELEPHONY_SIM_STATUS_INIT_COMPLETED != status) {
		CTS_ERR("sim is not ready (%d)", status);
		return;
	}
	INFO("sim is Ready");
	__ctsvc_sim_cb = false;
	vconf_ignore_key_changed(VCONFKEY_TELEPHONY_SIM_STATUS, __ctsvc_server_sim_ready_cb);

	__ctsvc_server_sim_info_init();
}

static void __ctsvc_server_telephony_ready_cb(keynode_t *key, void *data)
{
	int bReady = 0;
	// TODO: it should be changed API
	vconf_get_bool(VCONFKEY_TELEPHONY_TAPI_STATE, &bReady);

	if (0 == bReady) {
		CTS_ERR("telephony is not ready ");
		return;
	}
	INFO("telephony is Ready");

	vconf_ignore_key_changed(VCONFKEY_TELEPHONY_TAPI_STATE, __ctsvc_server_telephony_ready_cb);
	__ctsvc_tapi_cb = false;

	int status = 0;
	vconf_get_int(VCONFKEY_TELEPHONY_SIM_STATUS, &status);
	if (VCONFKEY_TELEPHONY_SIM_STATUS_INIT_COMPLETED != status) {
		CTS_ERR("sim is not ready (%d)", status);
		vconf_notify_key_changed(VCONFKEY_TELEPHONY_SIM_STATUS, __ctsvc_server_sim_ready_cb, NULL);
		__ctsvc_sim_cb = true;
		return;
	}

	__ctsvc_server_sim_info_init();
}


int ctsvc_server_sim_init()
{
	int bReady = 0;
	// TODO: it should be changed API
	vconf_get_bool(VCONFKEY_TELEPHONY_TAPI_STATE, &bReady);

	if (0 == bReady) {
		CTS_ERR("telephony is not ready ");
		vconf_notify_key_changed(VCONFKEY_TELEPHONY_TAPI_STATE, __ctsvc_server_telephony_ready_cb, NULL);
		__ctsvc_tapi_cb = true;
		return CONTACTS_ERROR_NONE;
	}

	int status = 0;
	vconf_get_int(VCONFKEY_TELEPHONY_SIM_STATUS, &status);
	if (VCONFKEY_TELEPHONY_SIM_STATUS_INIT_COMPLETED != status) {
		CTS_ERR("sim is not ready (%d)", status);
		vconf_notify_key_changed(VCONFKEY_TELEPHONY_SIM_STATUS, __ctsvc_server_sim_ready_cb, NULL);
		__ctsvc_sim_cb = true;
		return CONTACTS_ERROR_NONE;
	}

	return __ctsvc_server_sim_info_init();
}

int ctsvc_server_sim_final(void)
{
	CTS_FN_CALL;
	GSList *info_cursor = NULL;
	sim_contact_s *record = NULL;
	int ret = 0;

	if (__ctsvc_tapi_cb)
		vconf_ignore_key_changed(VCONFKEY_TELEPHONY_TAPI_STATE, __ctsvc_server_telephony_ready_cb);
	if (__ctsvc_sim_cb)
		vconf_ignore_key_changed(VCONFKEY_TELEPHONY_SIM_STATUS, __ctsvc_server_sim_ready_cb);

	for (info_cursor=__ctsvc_sim_info;info_cursor;info_cursor=info_cursor->next) {
		ctsvc_sim_info_s *info = info_cursor->data;
		GSList *cursor;
		free(info->cp_name);
		free(info->sim_unique_id);

		if (info->handle) {
			ret = tel_deinit(info->handle);
			WARN_IF(TAPI_API_SUCCESS != ret, "tel_deinit() Fail(%d)", ret);
		}

		for (cursor=info->import_contacts;cursor;cursor=cursor->next) {
			record = cursor->data;
			info->import_contacts = g_slist_remove(info->import_contacts, record);
			ctsvc_server_sim_record_destroy(record);
		}
		g_slist_free(info->import_contacts);

		free(info);
	}

	return CONTACTS_ERROR_NONE;
}


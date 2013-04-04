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
#include <string.h>
#include <tapi_common.h>
#include <ITapiSim.h>
#include <ITapiPhonebook.h>
#include <TapiUtility.h>
#include <vconf.h>
#include <vconf-internal-telephony-keys.h>

#include <contacts.h>

#include "internal.h"
#include "ctsvc_struct.h"
#include "ctsvc_server_socket.h"
#include "ctsvc_server_sqlite.h"
#include "ctsvc_server_utils.h"
#include "ctsvc_server_sim.h"
#include "ctsvc_utils.h"

#define PHONE_ADDRESSBOOK_ID 0

#define CTSVC_TAPI_SIM_PB_MAX 0xFFFF
#define CTSVC_SIM_DATA_MAX_LENGTH (255+1)

#define TAPI_PB_MAX_FILE_CNT TAPI_PB_3G_PBC+1
#define TAPI_PB_NAME_INDEX TAPI_PB_3G_NAME
#define TAPI_PB_NUMBER_INDEX TAPI_PB_3G_NUMBER

#define CTSVC_2GSIM_NAME TAPI_PB_3G_NAME
#define CTSVC_2GSIM_NUMBER TAPI_PB_3G_NUMBER

static TapiHandle *ghandle = NULL;
static TelSimPbType_t gsim_type = TAPI_SIM_PB_UNKNOWNN;
static void* greturn_data = NULL;
static int gsim_addressbook_id = -1;

typedef struct {
	unsigned int index_max;
	unsigned int text_max;
	unsigned int used_count;

	unsigned int (*get_index_max)(void *);
	unsigned int (*get_text_max)(void *);
	unsigned int (*get_unused_count)(void *);
	void (*set)(void *,int ,int ,int );
}sim_file_s;

static sim_file_s gfile_record[TAPI_PB_MAX_FILE_CNT];
static GSList *gcontact_records = NULL;
static bool ginit_completed = false;

static inline unsigned int get_index_max(void *this)
{
	return ((sim_file_s*)this)->index_max;
}

static inline unsigned int get_unused_count(void *this)
{
	return ((sim_file_s*)this)->index_max - ((sim_file_s*)this)->used_count;
}

static inline unsigned int get_text_max(void *this)
{
	return ((sim_file_s*)this)->text_max;
}

static inline void set(void *this,int index,int text,int used_count)
{
	((sim_file_s*)this)->index_max = index;
	((sim_file_s*)this)->text_max = text;
	((sim_file_s*)this)->used_count = used_count;
}
static inline TelSimPbType_t  __ctsvc_server_get_sim_type(void)
{
	return gsim_type;
}
static inline void __ctsvc_server_set_contact_records(GSList * list)
{
	gcontact_records = list;
}

static inline GSList* __ctsvc_server_get_contact_records(void)
{
	h_retvm_if(gcontact_records == NULL, NULL, "gcontact_records is NULL");
	return gcontact_records;
}

static inline void __ctsvc_server_sim_set_init_completed(bool init)
{
	SERVER_DBG("set ginit_completed = %d ", init);
	ginit_completed = init;
}

bool ctsvc_server_sim_get_init_completed(void)
{
	SERVER_DBG("get complete_init_flag = %d ", ginit_completed);
	return ginit_completed;
}
static inline void __ctsvc_server_set_sim_addressbook_id(int id)
{
	gsim_addressbook_id = id;
}
static void __ctsvc_server_init_sim_file(void)
{
	int i= 0;
	for(i = 0 ;i <TAPI_PB_MAX_FILE_CNT; i++ ){
		gfile_record[i].get_index_max = get_index_max;
		gfile_record[i].get_unused_count = get_unused_count;
		gfile_record[i].get_text_max = get_text_max;
		gfile_record[i].set = set;
		gfile_record[i].index_max = 0;
		gfile_record[i].text_max = 0;
		gfile_record[i].used_count = 0;
	}
}
static TapiHandle* __ctsvc_server_get_tapi_handle(void)
{
	if(ghandle == NULL){
		int bReady = 0;
		vconf_get_bool(VCONFKEY_TELEPHONY_READY, &bReady);

		if(!bReady) {
			SERVER_DBG("telephony is not ready ");
			return NULL;
		}
		else {
			ghandle = tel_init(NULL);
			h_retvm_if(NULL == ghandle, NULL, "tel_init() Failed");
		}
	}
	return ghandle;
}

void ctsvc_server_sim_destroy_contact_record(sim_contact_s *record)
{
	h_retm_if(record == NULL, "record is NULL ");
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
static sim_file_s * __ctsvc_server_sim_get_file_record(TelSimPb3GFileType_t file_type)
{
	return &gfile_record[file_type];
}
int ctsvc_server_sim_finalize(void)
{
	SERVER_FN_CALL;
	GSList *cursor = NULL;
	sim_contact_s *record = NULL;

	int ret = 0;
	if (ghandle) {
		ret = tel_deinit(ghandle);
		h_retvm_if(TAPI_API_SUCCESS != ret, CONTACTS_ERROR_SYSTEM,
				"tel_deinit() Failed(%d)", ret);
	}

	for (cursor=__ctsvc_server_get_contact_records();cursor;cursor=g_slist_next(cursor)){
		record = cursor->data;
		__ctsvc_server_set_contact_records(g_slist_remove(__ctsvc_server_get_contact_records(),cursor));
		ctsvc_server_sim_destroy_contact_record(record);
	}

	contacts_disconnect2();

	return CONTACTS_ERROR_NONE;
}

static void  __ctsvc_server_set_sim_type(void)
{
	int err = 0;
	TelSimCardType_t cardtype = TAPI_SIM_PB_UNKNOWNN;
	err = tel_get_sim_type(__ctsvc_server_get_tapi_handle(), &cardtype);
	h_retm_if(err != TAPI_API_SUCCESS,"tel_get_sim_type failed");

	if(cardtype == TAPI_SIM_CARD_TYPE_USIM)
		gsim_type = TAPI_SIM_PB_3GSIM;
	else
		gsim_type = TAPI_SIM_PB_ADN;
}

static inline void __ctsvc_server_set_return_data(void* data)
{
	greturn_data = data;
}

static inline void* __ctsvc_server_get_return_data(void)
{
	h_retvm_if(greturn_data == NULL, NULL, "ghelper_data is NULL");
	return greturn_data;
}

static sim_contact_s * __ctsvc_server_sim_create_contact_record(TelSimPbRecord_t *sim_record)
{
	char temp[CTSVC_SIM_DATA_MAX_LENGTH] = {0,};

	sim_contact_s *record = calloc(1,sizeof(sim_contact_s));
	record->sim_index = sim_record->index;
	record->next_index = sim_record->next_index;
	record->name = SAFE_STRDUP((char*)sim_record->name);
	record->nickname = SAFE_STRDUP((char*)sim_record->sne);
	if(sim_record->ton == TAPI_SIM_TON_INTERNATIONAL){
		snprintf(temp,sizeof(temp),"+%s",sim_record->number);
		record->number = strdup(temp);
	}
	else
		record->number = SAFE_STRDUP((char*)sim_record->number);

	memset(temp,0,sizeof(temp));
	if(sim_record->anr1_ton == TAPI_SIM_TON_INTERNATIONAL){
		snprintf(temp,sizeof(temp),"+%s",sim_record->anr1);
		record->anr1 = strdup(temp);
	}
	else
		record->anr1 = SAFE_STRDUP((char*)sim_record->anr1);

	memset(temp,0,sizeof(temp));
	if(sim_record->anr2_ton == TAPI_SIM_TON_INTERNATIONAL){
		snprintf(temp,sizeof(temp),"+%s",sim_record->anr2);
		record->anr2 = strdup(temp);
	}
	else
		record->anr2 = SAFE_STRDUP((char*)sim_record->anr2);

	memset(temp,0,sizeof(temp));
	if(sim_record->anr3_ton == TAPI_SIM_TON_INTERNATIONAL){
		snprintf(temp,sizeof(temp),"+%s",sim_record->anr3);
		record->anr3 = strdup(temp);
	}
	else
		record->anr3 = SAFE_STRDUP((char*)sim_record->anr3);

	record->email1 = SAFE_STRDUP((char*)sim_record->email1);
	record->email2 = SAFE_STRDUP((char*)sim_record->email2);
	record->email3 = SAFE_STRDUP((char*)sim_record->email3);
	record->email4 = SAFE_STRDUP((char*)sim_record->email4);

	return record;
}

static inline int __ctsvc_server_insert_db_num(contacts_record_h *record, char *number)
{

	int ret;
	h_retvm_if(number == NULL,CONTACTS_ERROR_INVALID_PARAMETER, "invalid number");

	ret = contacts_record_create(_contacts_number._uri, record);
	h_retvm_if(ret < CONTACTS_ERROR_NONE,ret, "contacts_record_destroy() Failed(%d)", ret);
	ret = contacts_record_set_str(*record, _contacts_number.number, number);
	h_retvm_if(ret < CONTACTS_ERROR_NONE,ret, "contacts_record_set_str() Failed(%d)", ret);
	ret = contacts_record_set_int(*record, _contacts_number.type, CONTACTS_NUMBER_TYPE_CELL);
	h_retvm_if(ret < CONTACTS_ERROR_NONE,ret, "contacts_record_set_int() Failed(%d)", ret);
	return ret;
}

static inline int __ctsvc_server_insert_db_email(contacts_record_h *record, char *email)
{
	int ret;

	h_retvm_if(email == NULL,CONTACTS_ERROR_INVALID_PARAMETER, "invalid email");

	ret = contacts_record_create(_contacts_email._uri, record);
	h_retvm_if(ret < CONTACTS_ERROR_NONE,ret, "contacts_record_destroy() Failed(%d)", ret);
	ret = contacts_record_set_str(*record, _contacts_email.email, email);
	h_retvm_if(ret < CONTACTS_ERROR_NONE,ret, "contacts_record_set_str() Failed(%d)", ret);
	ret = contacts_record_set_int(*record, _contacts_email.type,CONTACTS_EMAIL_TYPE_HOME);
	h_retvm_if(ret < CONTACTS_ERROR_NONE,ret, "contacts_record_set_int() Failed(%d)", ret);
	return ret;
}

static int __ctsvc_server_insert_contact_to_db(sim_contact_s *record,int* contact_id, int addressbook_id)
{
	SERVER_FN_CALL;
	int ret;
	char sim_id[20] = {0};
	contacts_record_h contact = NULL;
	contacts_record_h name = NULL;
	contacts_record_h number = NULL;
	contacts_record_h email = NULL;
//	contacts_record_h nick = NULL;

	h_retvm_if(record == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : record is NULL");
	h_retvm_if(contact_id == NULL, CONTACTS_ERROR_INVALID_PARAMETER, "Invalid parameter : contact_id is NULL");
	h_retvm_if(record->sim_index <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "The index(%d) is invalid", record->sim_index);

	SERVER_DBG("insert record -> sim_index %d",record->sim_index);

	ret = contacts_record_create(_contacts_contact._uri, &contact);
	h_retvm_if(CONTACTS_ERROR_NONE != ret, ret, "contacts_record_create failed(%d)", ret);
	snprintf(sim_id, sizeof(sim_id), "%d", record->sim_index);
	contacts_record_set_str(contact, _contacts_contact.uid, sim_id);

	if (record->name) {
		contacts_record_create(_contacts_name._uri, &name);
		if (name) {
			contacts_record_set_str(name, _contacts_name.first, (char *)record->name);
			contacts_record_add_child_record(contact, _contacts_contact.name, name);
		}
	}
	SERVER_DBG("insert record -> name %s", record->name);

/*	if (record->nickname) {
		contacts_record_create(_contacts_nickname._uri, &nick);
		if (nick) {
			contacts_record_set_str(nick, _contacts_nickname.name, (char *)record->nickname);
			contacts_record_add_child_record(contact, _contacts_contact.nickname, nick);
		}
	}
	SERVER_DBG("insert record -> nick name %s", record->nickname);
*/
	if (record->number) {
		contacts_record_create(_contacts_number._uri, &number);
		if (number) {
			contacts_record_set_str(number, _contacts_number.number, (char *)record->number);
			contacts_record_set_int(number, _contacts_number.type, CONTACTS_NUMBER_TYPE_CELL);
			contacts_record_add_child_record(contact, _contacts_contact.number, number);
		}
	}
	SERVER_DBG("insert record ->number %s", record->number);

/*	ret = __ctsvc_server_insert_db_num(&number, (char *)record->anr1);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(contact, _contacts_contact.number, number);
	SERVER_DBG("insert record ->anr1 %s", record->anr1);

	ret = __ctsvc_server_insert_db_num(&number, (char *)record->anr2);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(contact, _contacts_contact.number, number);
	SERVER_DBG("insert record ->anr2 %s", record->anr2);

	ret = __ctsvc_server_insert_db_num(&number, (char *)record->anr3);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(contact, _contacts_contact.number, number);
	SERVER_DBG("insert record ->anr3 %s", record->anr3);
*/
	ret = __ctsvc_server_insert_db_email(&email, (char *)record->email1);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(contact, _contacts_contact.email, email);
	SERVER_DBG("insert record ->email1 %s", record->email1);
/*
	ret = __ctsvc_server_insert_db_email(&email, (char *)record->email2);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(contact, _contacts_contact.email, email);
	SERVER_DBG("insert record ->email2 %s", record->email2);

	ret = __ctsvc_server_insert_db_email(&email, (char *)record->email3);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(contact, _contacts_contact.email, email);
	SERVER_DBG("insert record ->email3 %s", record->email3);

	ret = __ctsvc_server_insert_db_email(&email, (char *)record->email4);
	if (CONTACTS_ERROR_NONE == ret)
		contacts_record_add_child_record(contact, _contacts_contact.email, email);
	SERVER_DBG("insert record ->email4 %s", record->email4);
*/
	contacts_record_set_int(contact, _contacts_contact.address_book_id, addressbook_id);
	ret = contacts_db_insert_record(contact, contact_id);
	h_warn_if(ret < CONTACTS_ERROR_NONE, "contacts_db_insert_record() Failed(%d)", ret);
	record->contact_id = *contact_id;
	SERVER_DBG("inserted contact id = %d",record->contact_id);
	contacts_record_destroy(contact, true);
	h_warn_if(ret < CONTACTS_ERROR_NONE, "contacts_record_destroy() Failed(%d)", ret);

	return ret;
}

static void __ctsvc_server_sim_import_read_cb(TapiHandle *handle, int result, void *data, void* user_data)
{
	SERVER_FN_CALL;
	int ret = 0;
	TelSimPbAccessResult_t access_rt = result;
	TelSimPbRecord_t *sim_info = data;
	sim_contact_s *record = NULL;
	int start_index = 0;
	int contact_id =0;

	if(user_data != NULL)
		start_index = (int)user_data;


	if (NULL == sim_info) {
		ERR("sim_info is NULL, result = %d", access_rt);
		ret = CONTACTS_ERROR_SYSTEM;
		goto ERROR_RETURN;
	}

	if(access_rt == TAPI_SIM_PB_INVALID_INDEX) {
		SERVER_DBG("TAPI_SIM_PB_INVALID_INDEX : start_index = %d",start_index);
		start_index++; //search for start index
		sim_file_s *file =  __ctsvc_server_sim_get_file_record(TAPI_PB_3G_NAME);
		if(start_index > file->get_index_max((void*)file)){
			ERR("start_index is invalid start_index = %d, total = %d", start_index,file->get_index_max((void*)file));
			ret = CONTACTS_ERROR_SYSTEM;
			goto ERROR_RETURN;
		}
		ret = tel_read_sim_pb_record(handle, __ctsvc_server_get_sim_type(), start_index, __ctsvc_server_sim_import_read_cb, (void*)start_index);
		if(ret != TAPI_API_SUCCESS){
			ERR("SIM phonebook access Failed(%d) start_indext(%d)", access_rt,start_index);
			ret = CONTACTS_ERROR_SYSTEM;
			goto ERROR_RETURN;
		}
	}

	if (TAPI_SIM_PB_SUCCESS != access_rt) {
		ERR("SIM phonebook access Failed(%d)", access_rt);
		ret = CONTACTS_ERROR_SYSTEM;
		goto ERROR_RETURN;
	}
	switch (sim_info->phonebook_type) {
	case TAPI_SIM_PB_ADN:
	case TAPI_SIM_PB_3GSIM:
		record = __ctsvc_server_sim_create_contact_record(sim_info);
		ret = __ctsvc_server_insert_contact_to_db(record, &contact_id,PHONE_ADDRESSBOOK_ID);
		ctsvc_server_sim_destroy_contact_record(record);
		if(ret < CONTACTS_ERROR_NONE || contact_id <= 0) {
			ERR("__ctsvc_server_insert_or_update_contact_to_db is fail(%d)",ret);
			goto ERROR_RETURN;
		}
		break;
	case TAPI_SIM_PB_FDN:
	case TAPI_SIM_PB_SDN:
	default:
		ERR("Unknown storage type(%d)", sim_info->phonebook_type);
		ret = CONTACTS_ERROR_SYSTEM;
		goto ERROR_RETURN;
	}
	if (sim_info->next_index && CTSVC_TAPI_SIM_PB_MAX != sim_info->next_index) {
		SERVER_DBG("NextIndex = %d", sim_info->next_index);
		ret = tel_read_sim_pb_record(__ctsvc_server_get_tapi_handle(), sim_info->phonebook_type,
				sim_info->next_index, __ctsvc_server_sim_import_read_cb, NULL);
		if (TAPI_API_SUCCESS != ret) {
			ERR("tel_read_sim_pb_record() Failed(%d)", ret);
			ret = CONTACTS_ERROR_SYSTEM;
			goto ERROR_RETURN;
		}
	}
	else {
		if (__ctsvc_server_get_return_data()) {
			ret = ctsvc_server_socket_return(__ctsvc_server_get_return_data(), CONTACTS_ERROR_NONE, 0, NULL);
			h_warn_if(CONTACTS_ERROR_NONE != ret, "ctsvc_server_socket_return() Failed(%d)", ret);
			__ctsvc_server_set_return_data(NULL);
		}
	}
	return;

ERROR_RETURN:
	if (__ctsvc_server_get_return_data()) {
		ret = ctsvc_server_socket_return(__ctsvc_server_get_return_data(), ret, 0, NULL);
		h_warn_if(CONTACTS_ERROR_NONE != ret, "ctsvc_server_socket_return() Failed(%d)", ret);
		__ctsvc_server_set_return_data(NULL);
	}
	return;
}

int ctsvc_server_sim_import(void* data)
{
	SERVER_FN_CALL;
	int ret;

	h_retvm_if(false == ctsvc_server_sim_get_init_completed(), CONTACTS_ERROR_SYSTEM,"sim init is not completed");
	h_retvm_if(NULL != __ctsvc_server_get_return_data(), CONTACTS_ERROR_INTERNAL,"Helper is already processing with sim");

	__ctsvc_server_set_return_data(data);

	sim_file_s *file =  __ctsvc_server_sim_get_file_record(TAPI_PB_3G_NAME);
	if(file->get_unused_count((void*)file) == file->get_index_max((void*)file)){
		ret = ctsvc_server_socket_return(__ctsvc_server_get_return_data(), CONTACTS_ERROR_NO_DATA, 0, NULL);
		h_warn_if(CONTACTS_ERROR_SYSTEM != ret, "helper_socket_return() Failed(%d)", ret);
		__ctsvc_server_set_return_data(NULL);
		return ret;
	}

	ret = tel_read_sim_pb_record(__ctsvc_server_get_tapi_handle(), __ctsvc_server_get_sim_type(), 1,
			__ctsvc_server_sim_import_read_cb, NULL);
	if (ret != TAPI_API_SUCCESS) {
		ERR("tel_read_sim_pb_record = %d",ret);
		__ctsvc_server_set_return_data(NULL);
		return CONTACTS_ERROR_SYSTEM;
	}

	return CONTACTS_ERROR_NONE;
}

static void __ctsvc_server_sim_sdn_read_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
	SERVER_FN_CALL;
	int ret=0;
	TelSimPbAccessResult_t sec_rt = result;
	TelSimPbRecord_t *sim_info = data;

	h_retm_if(sim_info == NULL,  "sim_info is NULL");

	if (TAPI_SIM_PB_SUCCESS != sec_rt) {
		if (TAPI_SIM_PB_SDN == sim_info->phonebook_type &&
				TAPI_SIM_PB_INVALID_INDEX == sec_rt) {
			SERVER_DBG("Index = %d", sim_info->index);
			ret = tel_read_sim_pb_record(handle, sim_info->phonebook_type,sim_info->index+1,
					__ctsvc_server_sim_sdn_read_cb, NULL);
			h_retm_if(ret != TAPI_API_SUCCESS,  "tel_read_sim_pb_record() Failed(%d)", ret);
		}
		ERR("SIM phonebook access Failed(%d)", sec_rt);
		goto ERROR_RETURN;
	}

	switch (sim_info->phonebook_type) {
	case TAPI_SIM_PB_SDN:
		ret = ctsvc_server_insert_sdn_contact((char *)sim_info->name, (char *)sim_info->number);
		h_warn_if(ret != CONTACTS_ERROR_NONE, "ctsvc_server_insert_sdn_contact() Failed(%d)", ret);
		break;
	case TAPI_SIM_PB_ADN:
	case TAPI_SIM_PB_3GSIM:
	case TAPI_SIM_PB_FDN:
	default:
		ERR("Unknown storage type(%d)", sim_info->phonebook_type);
		goto ERROR_RETURN;
	}

	if (sim_info->next_index && CTSVC_TAPI_SIM_PB_MAX != sim_info->next_index) {
		SERVER_DBG("NextIndex = %d", sim_info->next_index);
		ret = tel_read_sim_pb_record(handle, sim_info->phonebook_type,sim_info->next_index,
				__ctsvc_server_sim_sdn_read_cb, NULL);
		if (TAPI_API_SUCCESS != ret) {
			ERR("tel_read_sim_pb_record() Failed(%d)", ret);
			goto ERROR_RETURN;
		}
	}
	else {
		ctsvc_server_trim_memory();
	}
	return;

ERROR_RETURN:
	ctsvc_server_trim_memory();
	return;
}
static void __ctsvc_server_sim_sdn_meta_info_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
	SERVER_FN_CALL;
	TelSimPbAccessResult_t sec_rt = result;
	int ret=0;

	if (TAPI_SIM_PB_SUCCESS != sec_rt) {
		ERR("__ctsvc_server_sim_sdn_meta_info_cb() Failed(%d)", sec_rt);
		goto ERROR_RETURN;
	}
	ret = tel_read_sim_pb_record(__ctsvc_server_get_tapi_handle(), TAPI_SIM_PB_SDN,1, __ctsvc_server_sim_sdn_read_cb, NULL);
	if(TAPI_API_SUCCESS != ret) {
		ERR("tel_read_sim_pb_record() Failed(%d)", ret);
		goto ERROR_RETURN;
	}

ERROR_RETURN:
	ctsvc_server_trim_memory();
	return;
}
int ctsvc_server_sim_read_sdn(void* data)
{
	SERVER_FN_CALL;
	int ret, card_changed = 0;
	TelSimCardStatus_t sim_status;

	h_retvm_if(NULL != __ctsvc_server_get_return_data(), CONTACTS_ERROR_INVALID_PARAMETER,
			"Helper is already processing with sim");
	ret = tel_get_sim_init_info(__ctsvc_server_get_tapi_handle(), &sim_status, &card_changed);
	if(TAPI_API_SUCCESS != ret) {
		ERR("tel_get_sim_init_info() Failed(%d)", ret);
		SERVER_DBG("sim_status = %d, card_changed = %d", sim_status, card_changed);
		goto ERROR_RETURN;
	}

	if (TAPI_SIM_STATUS_CARD_NOT_PRESENT == sim_status ||
			TAPI_SIM_STATUS_CARD_REMOVED == sim_status) {
		ret = ctsvc_server_delete_sdn_contact();
		if(CONTACTS_ERROR_NONE != ret) {
			ERR("__ctsvc_server_delete_SDN_contact() Failed(%d)", ret);
			goto ERROR_RETURN;
		}
	}
	else if (TAPI_SIM_STATUS_SIM_INIT_COMPLETED == sim_status) {
		ret = ctsvc_server_delete_sdn_contact();
		if(CONTACTS_ERROR_NONE != ret) {
			ERR("__ctsvc_server_delete_SDN_contact() Failed(%d)", ret);
			goto ERROR_RETURN;
		}

		ret = tel_get_sim_pb_meta_info(__ctsvc_server_get_tapi_handle(), TAPI_SIM_PB_SDN, __ctsvc_server_sim_sdn_meta_info_cb, NULL);
		h_retvm_if(ret != TAPI_API_SUCCESS, CONTACTS_ERROR_SYSTEM, "tel_get_sim_(usim)_meta_info =%d",ret);
		if(TAPI_API_SUCCESS != ret) {
			ERR("tel_get_sim_pb_meta_info() Failed(%d)", ret);
			goto ERROR_RETURN;
		}

	else
		ERR("sim_status Failed(%d)", sim_status);
	}

	return CONTACTS_ERROR_NONE;

ERROR_RETURN:
	ctsvc_server_trim_memory();
	return ret;
}

static void __ctsvc_server_sim_initialize_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
	SERVER_FN_CALL;
	int i=0;
	TelSimPbAccessResult_t access_rt = result;
	TelSimPbType_t sim_type = __ctsvc_server_get_sim_type();

	SERVER_DBG("sim_type = %d",sim_type);
	if (TAPI_SIM_PB_3GSIM == sim_type) {
		TelSimPbCapabilityInfo_t *capa = data;
		h_retm_if(capa == NULL, "capa is NULL result =%d",access_rt);

		for (i=0; i < capa->FileTypeCount; i++) {
			sim_file_s *file_record = NULL;
			int type = capa->FileTypeInfo[i].field_type;
			SERVER_DBG("TYPE = %d",type);
			file_record = __ctsvc_server_sim_get_file_record(type);
			file_record->set(file_record,capa->FileTypeInfo[i].index_max, capa->FileTypeInfo[i].text_max,
					capa->FileTypeInfo[i].used_count);
			SERVER_DBG(" field_type[%d] : i_max(%d), t_max(%d), un_count(%d)", type,
					gfile_record[type].get_index_max(&gfile_record[type]), gfile_record[type].get_text_max(&gfile_record[type]), gfile_record[type].get_unused_count(&gfile_record[type]));
		}
	}
	else if(TAPI_SIM_PB_ADN == sim_type){
		TelSimPbEntryInfo_t *pe = data;
		h_retm_if(pe == NULL, "pe is NULL result =%d",access_rt);
		sim_file_s *file_record = NULL;
		file_record =__ctsvc_server_sim_get_file_record(CTSVC_2GSIM_NAME);
		file_record->set(file_record,pe->PbIndexMax,pe->PbTextLenMax,pe->PbIndexMin);
		SERVER_DBG(" CTSVC_2GSIM_NAME : i_max(%d), t_max(%d), i_min(%d)", pe->PbIndexMax,pe->PbTextLenMax,pe->PbIndexMin);

		file_record =__ctsvc_server_sim_get_file_record(CTSVC_2GSIM_NUMBER);
		file_record->set(file_record,pe->PbIndexMax,pe->PbNumLenMax,pe->PbIndexMin);
		SERVER_DBG(" CTSVC_2GSIM_NUMBER : i_max(%d), t_max(%d), i_min(%d)", pe->PbIndexMax,pe->PbNumLenMax,pe->PbIndexMin);
	}
	else{
		ERR("sim_type [%d]error ", sim_type);
		return;
	}
	__ctsvc_server_sim_set_init_completed(true);
	return;
}

int ctsvc_server_sim_initialize(void)
{
	SERVER_FN_CALL;
	__ctsvc_server_get_tapi_handle();
	__ctsvc_server_set_sim_type();
	contacts_connect2();

	int ret = TAPI_API_SUCCESS;
	TelSimPbType_t sim_type;
	TapiHandle *sim_handle;

	__ctsvc_server_init_sim_file();

	sim_type = __ctsvc_server_get_sim_type();
	sim_handle = __ctsvc_server_get_tapi_handle();

	if(sim_type == TAPI_SIM_PB_3GSIM)
		ret = tel_get_sim_pb_usim_meta_info(sim_handle, __ctsvc_server_sim_initialize_cb, NULL);
	else if(sim_type == TAPI_SIM_PB_ADN)
		ret = tel_get_sim_pb_meta_info(sim_handle, sim_type, __ctsvc_server_sim_initialize_cb, NULL);
	h_retvm_if(ret != TAPI_API_SUCCESS, CONTACTS_ERROR_SYSTEM, "tel_get_sim_(usim)_meta_info =%d",ret);

	return CONTACTS_ERROR_NONE;
}



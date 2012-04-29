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
#include <string.h>
#include <TapiCommon.h>
#include <ITapiSim.h>
#include <contacts-svc.h>

#include "cts-addressbook.h"
#include "helper-socket.h"
#include "internal.h"
#include "normalize.h"
#include "sqlite.h"
#include "utils.h"
#include "sim.h"

#define CTS_SIM_EVENT_NUM 2
#define CTS_TAPI_SIM_PB_MAX 0xFFFF
#define CTS_MIN(a, b) (a>b)?b:a

static int helper_sim_read_record_cb(const TelTapiEvent_t *pdata, void *data);
static int helper_sim_pb_count_cb(const TelTapiEvent_t *pdata, void *data);

static TelSimImsiInfo_t TAPI_imsi;
static void *helper_import_sim_data = NULL;

static unsigned int TAPI_SIM_EVENT_ID[CTS_SIM_EVENT_NUM];
static const int TAPI_SIM_EVENT[CTS_SIM_EVENT_NUM] =
{
	TAPI_EVENT_SIM_PB_ACCESS_READ_CNF,
	TAPI_EVENT_SIM_PB_STORAGE_COUNT_CNF
};
static const TelAppCallback TAPI_SIM_EVENT_CB[CTS_SIM_EVENT_NUM] =
{
	helper_sim_read_record_cb,
	helper_sim_pb_count_cb
};

static int helper_register_tapi_cnt = 0;
static int helper_register_tapi_sim_event(void)
{
	int i, ret;

	if (0 == helper_register_tapi_cnt)
	{
		ret = tel_init();
		h_retvm_if(TAPI_API_SUCCESS != ret, CTS_ERR_TAPI_FAILED,
				"tel_init() is Failed(%d)", ret);

		ret = tel_register_app_name(CTS_DBUS_SERVICE);
		h_retvm_if(TAPI_API_SUCCESS != ret, CTS_ERR_TAPI_FAILED,
				"tel_register_app_name(%s) is Failed(%d)", CTS_DBUS_SERVICE, ret);

		for (i=0;i<CTS_SIM_EVENT_NUM;i++)
		{
			ret = tel_register_event(TAPI_SIM_EVENT[i], &TAPI_SIM_EVENT_ID[i],
					TAPI_SIM_EVENT_CB[i], NULL);
			h_retvm_if(TAPI_API_SUCCESS != ret, CTS_ERR_TAPI_FAILED,
					"tel_register_event(Event[%d]) is Failed(%d)", i, ret);
		}
	}
	helper_register_tapi_cnt++;

	return CTS_SUCCESS;
}

static int helper_deregister_tapi_sim_event(void)
{
	int ret, i;

	if (1 == helper_register_tapi_cnt)
	{
		for (i=0;i<CTS_SIM_EVENT_NUM;i++)
		{
			ret = tel_deregister_event(TAPI_SIM_EVENT_ID[i]);
			h_warn_if(TAPI_API_SUCCESS != ret,
					"tel_register_event(Event[%d]) is Failed(%d)", i, ret);
		}

		ret = tel_deinit();
		h_retvm_if(TAPI_API_SUCCESS != ret, CTS_ERR_TAPI_FAILED,
				"tel_deinti() Failed(%d)", ret);
	}
	helper_register_tapi_cnt--;

	return CTS_SUCCESS;
}

#define HELPER_SIM_DATA_MAX_LENGTH 1024

static int helper_insert_SDN(TelSimPbRecord_t *pb2g_data)
{
	int ret;

	ret = helper_insert_SDN_contact((char *)pb2g_data->name, (char *)pb2g_data->number);
	h_retvm_if(ret != CTS_SUCCESS, ret, "helper_insert_SDN_contact() Failed(%d)", ret);

	return ret;
}

static int helper_insert_2g_contact(TelSimPbRecord_t *pb2g_data)
{
	int ret, found_id;
	char uid[32];
	CTSstruct *contact;
	GSList *numbers=NULL;
	CTSvalue *name_val, *number_val, *base;

	h_retvm_if(pb2g_data->index <= 0, CTS_ERR_ARG_INVALID, "The index(%d) is invalid", pb2g_data->index);

	snprintf(uid, sizeof(uid), "SIM:%s-%s-%s-%d",
			TAPI_imsi.szMcc, TAPI_imsi.szMnc, TAPI_imsi.szMsin, pb2g_data->index);
	HELPER_DBG("UID = %s", uid);

	found_id = contacts_svc_find_contact_by(CTS_FIND_BY_UID, uid);

	contact = contacts_svc_struct_new(CTS_STRUCT_CONTACT);

	base = contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);
	if (base) {
		contacts_svc_value_set_str(base, CTS_BASE_VAL_UID_STR, uid);
		contacts_svc_struct_store_value(contact, CTS_CF_BASE_INFO_VALUE, base);
		contacts_svc_value_free(base);
	}

	name_val = contacts_svc_value_new(CTS_VALUE_NAME);
	if (name_val) {
		contacts_svc_value_set_str(name_val, CTS_NAME_VAL_DISPLAY_STR, (char *)pb2g_data->name);
		contacts_svc_struct_store_value(contact, CTS_CF_NAME_VALUE, name_val);
		contacts_svc_value_free(name_val);
	}

	number_val = contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number_val) {
		contacts_svc_value_set_str(number_val, CTS_NUM_VAL_NUMBER_STR,
				(char *)pb2g_data->number);
		contacts_svc_value_set_int(number_val, CTS_NUM_VAL_TYPE_INT, CTS_NUM_TYPE_CELL);
		contacts_svc_value_set_bool(number_val, CTS_NUM_VAL_DEFAULT_BOOL, true);
	}
	numbers = g_slist_append(numbers, number_val);

	contacts_svc_struct_store_list(contact, CTS_CF_NUMBER_LIST, numbers);
	contacts_svc_value_free(number_val);
	g_slist_free(numbers);

	if (0 < found_id) {
		CTSstruct *temp;
		ret = contacts_svc_get_contact(found_id, &temp);
		if (CTS_SUCCESS == ret) {
			contacts_svc_struct_merge(temp, contact);
			contacts_svc_struct_free(contact);
			contact = temp;
			ret = contacts_svc_update_contact(contact);
			h_warn_if(ret < CTS_SUCCESS, "contacts_svc_update_contact() Failed(%d)", ret);
		} else {
			ERR("contacts_svc_get_contact() Failed(%d)", ret);
		}
	} else {
		ret = contacts_svc_insert_contact(CTS_ADDRESSBOOK_INTERNAL, contact);
		h_warn_if(ret < CTS_SUCCESS, "contacts_svc_insert_contact() Failed(%d)", ret);
	}
	contacts_svc_struct_free(contact);

	return ret;
}

static inline GSList* helper_insert_3g_contact_num(GSList *numbers, char *number)
{
	CTSvalue *value;

	value = contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (value) {
		contacts_svc_value_set_str(value, CTS_NUM_VAL_NUMBER_STR, number);
		//contacts_svc_value_set_int(value, CTS_NUM_VAL_TYPE_INT, CTS_NUM_TYPE_CELL);
	}
	return g_slist_append(numbers, value);
}

static inline GSList* helper_insert_3g_contact_email(GSList *emails, char *email)
{
	CTSvalue *value;

	value = contacts_svc_value_new(CTS_VALUE_EMAIL);
	if (value)
		contacts_svc_value_set_str(value, CTS_EMAIL_VAL_ADDR_STR, email);
	return g_slist_append(emails, value);
}

static int helper_insert_3g_contact(TelSimPbRecord_t *pb3g_data)
{
	int ret, found_id;
	char uid[32];
	CTSstruct *contact;
	CTSvalue *name_val=NULL, *number_val, *base;
	GSList *numbers=NULL, *emails=NULL;

	h_retvm_if(pb3g_data->index <= 0, CTS_ERR_ARG_INVALID, "The index(%d) is invalid", pb3g_data->index);

	snprintf(uid, sizeof(uid), "SIM:%s-%s-%s-%d",
			TAPI_imsi.szMcc, TAPI_imsi.szMnc, TAPI_imsi.szMsin, pb3g_data->index);
	HELPER_DBG("UID = %s", uid);
	found_id = contacts_svc_find_contact_by(CTS_FIND_BY_UID, uid);

	contact = contacts_svc_struct_new(CTS_STRUCT_CONTACT);

	base = contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);
	if (base) {
		contacts_svc_value_set_str(base, CTS_BASE_VAL_UID_STR, uid);
		contacts_svc_struct_store_value(contact, CTS_CF_BASE_INFO_VALUE, base);
		contacts_svc_value_free(base);
	}

	if (*pb3g_data->name) {
		name_val = contacts_svc_value_new(CTS_VALUE_NAME);
		if (name_val)
			contacts_svc_value_set_str(name_val, CTS_NAME_VAL_FIRST_STR, (char *)pb3g_data->name);

		contacts_svc_struct_store_value(contact, CTS_CF_NAME_VALUE, name_val);
	}

	if (*pb3g_data->number) {
		number_val = contacts_svc_value_new(CTS_VALUE_NUMBER);
		if (number_val) {
			contacts_svc_value_set_str(number_val, CTS_NUM_VAL_NUMBER_STR, (char *)pb3g_data->number);
			//contacts_svc_value_set_int(number_val, CTS_NUM_VAL_TYPE_INT, CTS_NUM_TYPE_CELL);
			contacts_svc_value_set_bool(number_val, CTS_NUM_VAL_DEFAULT_BOOL, true);
		}
		numbers = g_slist_append(numbers, number_val);
	}

	numbers = helper_insert_3g_contact_num(numbers, (char *)pb3g_data->anr1);
	numbers = helper_insert_3g_contact_num(numbers, (char *)pb3g_data->anr2);
	numbers = helper_insert_3g_contact_num(numbers, (char *)pb3g_data->anr3);
	contacts_svc_struct_store_list(contact, CTS_CF_NUMBER_LIST, numbers);

	emails = helper_insert_3g_contact_email(emails, (char *)pb3g_data->email1);
	emails = helper_insert_3g_contact_email(emails, (char *)pb3g_data->email2);
	emails = helper_insert_3g_contact_email(emails, (char *)pb3g_data->email3);
	emails = helper_insert_3g_contact_email(emails, (char *)pb3g_data->email4);
	contacts_svc_struct_store_list(contact, CTS_CF_EMAIL_LIST, emails);

	if (0 < found_id) {
		CTSstruct *temp;
		ret = contacts_svc_get_contact(found_id, &temp);
		if (CTS_SUCCESS == ret) {
			contacts_svc_struct_merge(temp, contact);
			contacts_svc_struct_free(contact);
			contact = temp;
			ret = contacts_svc_update_contact(contact);
			h_warn_if(ret < CTS_SUCCESS, "contacts_svc_update_contact() Failed(%d)", ret);
		} else {
			ERR("contacts_svc_get_contact() Failed(%d)", ret);
		}
	} else {
		ret = contacts_svc_insert_contact(CTS_ADDRESSBOOK_INTERNAL, contact);
		h_warn_if(ret < CTS_SUCCESS, "contacts_svc_insert_contact() Failed(%d)", ret);
	}

	contacts_svc_struct_free(contact);
	return ret;
}

static int helper_sim_read_record_cb(const TelTapiEvent_t *sim_event, void *data)
{
	int ret, saved_pb_num, i=0, req_id;
	TelSimPbRecord_t *sim_info;

	HELPER_FN_CALL;

	h_retvm_if(TAPI_EVENT_CLASS_SIM != sim_event->EventClass ||
			TAPI_EVENT_SIM_PB_ACCESS_READ_CNF != sim_event->EventType,
			CTS_ERR_TAPI_FAILED,
			"Unknown Event(EventClass = 0x%X, EventType = 0x%X",
			sim_event->EventClass, sim_event->EventType);

	sim_info = (TelSimPbRecord_t*)sim_event->pData;
	if (NULL == sim_info) {
		ERR("sim_info is NULL, Status = %d", sim_event->Status);
		goto ERROR_RETURN;
	}

	if (TAPI_SIM_PB_SUCCESS != sim_event->Status) {
		if (TAPI_SIM_PB_SDN == sim_info->phonebook_type &&
				TAPI_SIM_PB_INVALID_INDEX == sim_event->Status)
		{
			HELPER_DBG("Index = %d", sim_info->index);
			ret = tel_read_sim_pb_record(sim_info->phonebook_type,
					sim_info->index+1, &req_id);
			if (TAPI_API_SUCCESS != ret) {
				ERR("tel_read_sim_pb_record() Failed(%d)", ret);
				goto ERROR_RETURN;
			}
			return CTS_SUCCESS;
		}
		ERR("SIM phonebook access Failed(%d)", sim_event->Status);
		goto ERROR_RETURN;
	}

	switch (sim_info->phonebook_type)
	{
	case TAPI_SIM_PB_SDN:
		saved_pb_num = sim_event->pDataLen / sizeof(TelSimPbRecordData_t);
		if (saved_pb_num <= 0 || TAPI_SIM_3G_PB_MAX_RECORD_COUNT < saved_pb_num) {
			ERR("received saved_pb_num is invalid(%d)", saved_pb_num);
			goto ERROR_RETURN;
		}
		while (true) {
			ret = helper_insert_SDN(sim_info);
			h_warn_if(ret < CTS_SUCCESS, "helper_insert_SDN() is Failed(%d)", ret);
			if (saved_pb_num == ++i) break;
			sim_info++;
		}
		//sim_info->NextIndex = sim_info->Index+1;
		break;
	case TAPI_SIM_PB_ADN:
		saved_pb_num = sim_event->pDataLen / sizeof(TelSimPbRecordData_t);
		if (saved_pb_num <= 0 || TAPI_SIM_3G_PB_MAX_RECORD_COUNT < saved_pb_num) {
			ERR("received saved_pb_num is invalid(%d)", saved_pb_num);
			goto ERROR_RETURN;
		}
		while (true) {
			ret = helper_insert_2g_contact(sim_info);
			h_warn_if(ret < CTS_SUCCESS, "helper_insert_2g_contact() is Failed(%d)", ret);
			if (saved_pb_num == ++i) break;
			sim_info++;
		}
		break;
	case TAPI_SIM_PB_3GSIM:
		saved_pb_num = sim_event->pDataLen / sizeof(TelSimPbRecordData_t);
		HELPER_DBG("saved_pb_num = %d", saved_pb_num);
		if (saved_pb_num <= 0 || TAPI_SIM_3G_PB_MAX_RECORD_COUNT < saved_pb_num) {
			ERR("received saved_pb_num is invalid(%d)", saved_pb_num);
			goto ERROR_RETURN;
		}
		while (true) {
			ret = helper_insert_3g_contact(sim_info);
			h_warn_if(ret < CTS_SUCCESS, "helper_insert_3g_contact() is Failed(%d)", ret);
			if (saved_pb_num == ++i) break;
			sim_info++;
		}
		break;
	case TAPI_SIM_PB_FDN:
	case TAPI_SIM_PB_MSISDN:
	default:
		ERR("Unknown storage type(%d)", sim_info->phonebook_type);
		goto ERROR_RETURN;
	}
	if (sim_info->next_index && CTS_TAPI_SIM_PB_MAX != sim_info->next_index) {
		HELPER_DBG("NextIndex = %d", sim_info->next_index);
		ret = tel_read_sim_pb_record(sim_info->phonebook_type,
				sim_info->next_index, &req_id);
		if (TAPI_API_SUCCESS != ret) {
			ERR("tel_read_sim_pb_record() Failed(%d)", ret);
			goto ERROR_RETURN;
		}
	}
	else {
		contacts_svc_end_trans(true);
		if (helper_import_sim_data) {
			ret = helper_socket_return(helper_import_sim_data, CTS_SUCCESS, 0, NULL);
			h_warn_if(CTS_SUCCESS != ret, "helper_socket_return() Failed(%d)", ret);
			helper_import_sim_data = NULL;
			memset(&TAPI_imsi, 0x00, sizeof(TelSimImsiInfo_t));
		}

		helper_deregister_tapi_sim_event();
		helper_trim_memory();
	}
	return CTS_SUCCESS;
ERROR_RETURN:
	contacts_svc_end_trans(false);
	if (helper_import_sim_data) {
		ret = helper_socket_return(helper_import_sim_data, CTS_SUCCESS, 0, NULL);
		h_warn_if(CTS_SUCCESS != ret, "helper_socket_return() Failed(%d)", ret);
		helper_import_sim_data = NULL;
		memset(&TAPI_imsi, 0x00, sizeof(TelSimImsiInfo_t));
	}
	helper_deregister_tapi_sim_event();
	helper_trim_memory();
	return CTS_ERR_TAPI_FAILED;
}


static int helper_sim_pb_count_cb(const TelTapiEvent_t *sim_event, void *data)
{
	int ret, req_id;
	TelSimPbStorageInfo_t *sim_info;

	HELPER_FN_CALL;

	h_retvm_if(TAPI_EVENT_CLASS_SIM != sim_event->EventClass ||
			TAPI_EVENT_SIM_PB_STORAGE_COUNT_CNF != sim_event->EventType,
			CTS_ERR_TAPI_FAILED,
			"Unknown Event(EventClass = 0x%X, EventType = 0x%X",
			sim_event->EventClass, sim_event->EventType);

	sim_info = (TelSimPbStorageInfo_t *)sim_event->pData;
	if (NULL == sim_info) {
		ERR("sim_info is NULL, Status = %d", sim_event->Status);
		ret = CTS_ERR_TAPI_FAILED;
		goto ERROR_RETURN;
	}

	if (TAPI_SIM_PB_SUCCESS != sim_event->Status) {
		ERR("SIM phonebook access Failed(%d)", sim_event->Status);
		ret = CTS_ERR_TAPI_FAILED;
		goto ERROR_RETURN;
	}

	switch (sim_info->StorageFileType)
	{
	case TAPI_SIM_PB_SDN:
		if (sim_info->UsedRecordCount) {
			HELPER_DBG("SDN count = %d", sim_info->UsedRecordCount);
			ret = tel_read_sim_pb_record(sim_info->StorageFileType, 1, &req_id);
			if (TAPI_API_SUCCESS != ret) {
				ERR("tel_read_sim_pb_record() Failed(%d)", ret);
				ret = CTS_ERR_TAPI_FAILED;
				goto ERROR_RETURN;
			}
		}
		break;
	case TAPI_SIM_PB_ADN:
	case TAPI_SIM_PB_3GSIM:
		if (sim_info->UsedRecordCount) {
			HELPER_DBG("ADN count = %d", sim_info->UsedRecordCount);
			ret = tel_read_sim_pb_record(sim_info->StorageFileType, 1, &req_id);
			if (TAPI_API_SUCCESS != ret) {
				ERR("tel_read_sim_pb_record() Failed(%d)", ret);
				ret = CTS_ERR_TAPI_FAILED;
				goto ERROR_RETURN;
			}
			ret = contacts_svc_begin_trans();
			if (CTS_SUCCESS != ret) {
				ERR("contacts_svc_begin_trans() Failed(%d)", ret);
				goto ERROR_RETURN;
			}
		} else {
			helper_socket_return(helper_import_sim_data, CTS_ERR_NO_DATA, 0, NULL);
			ret = CTS_SUCCESS;
			goto ERROR_RETURN;
		}
		break;
	case TAPI_SIM_PB_FDN:
	case TAPI_SIM_PB_MSISDN:
	default:
		ERR("Unknown storage type(%d)", sim_info->StorageFileType);
		return CTS_ERR_TAPI_FAILED;
	}

	return CTS_SUCCESS;

ERROR_RETURN:
	helper_deregister_tapi_sim_event();
	return ret;
}


int helper_sim_read_pb_record(void *data)
{
	int ret, req_id;
	TelSimPbFileType_t storage;
	TelSimCardType_t cardInfo;

	h_retvm_if(NULL != helper_import_sim_data, CTS_ERR_ENV_INVALID,
			"Helper is already processing with sim");

	ret = helper_register_tapi_sim_event();
	h_retvm_if(TAPI_API_SUCCESS != ret, ret,
			"helper_register_tapi_sim_event() Failed(%d)", ret);

	ret = tel_get_sim_type(&cardInfo);
	h_retvm_if(TAPI_API_SUCCESS != ret, CTS_ERR_TAPI_FAILED,
			"tel_get_sim_type() Failed(%d)", ret);

	if (TAPI_SIM_CARD_TYPE_USIM == cardInfo)
		storage = TAPI_SIM_PB_3GSIM;
	else
		storage = TAPI_SIM_PB_ADN;

	int first_id, sim_pb_inited;
	TelSimPbList_t pb_list = {0};

	ret = tel_get_sim_pb_init_info(&sim_pb_inited, &pb_list, &first_id);
	h_retvm_if(TAPI_API_SUCCESS != ret, CTS_ERR_TAPI_FAILED,
			"tel_get_sim_pb_init_info() Failed(%d)", ret);
	HELPER_DBG("sim_pb_inited(%d), first_id(%d)", sim_pb_inited, first_id);

	tel_get_sim_imsi(&TAPI_imsi);
	h_retvm_if(CTS_SUCCESS != ret, ret, "tel_get_sim_imsi() Failed(%d)", ret);

	if (sim_pb_inited) {
		ret = tel_get_sim_pb_count(storage, &req_id);
		h_retvm_if(TAPI_API_SUCCESS != ret, CTS_ERR_TAPI_FAILED,
				"tel_get_sim_pb_count() Failed(%d)", ret);
	}

	helper_import_sim_data = data;

	return CTS_SUCCESS;
}


int helper_sim_read_SDN(void* data)
{
	int ret, req_id, card_changed=0;
	TelSimCardStatus_t sim_status;

	HELPER_FN_CALL;

	h_retvm_if(NULL != helper_import_sim_data, CTS_ERR_ENV_INVALID,
			"Helper is already processing with sim");

	ret = helper_register_tapi_sim_event();
	h_retvm_if(TAPI_API_SUCCESS != ret, ret,
			"helper_register_tapi_sim_event() Failed(%d)", ret);

	ret = tel_get_sim_init_info(&sim_status, &card_changed);
	h_retvm_if(TAPI_API_SUCCESS != ret, CTS_ERR_TAPI_FAILED,
			"tel_get_sim_init_info() Failed(%d)", ret);
	HELPER_DBG("sim_status = %d, card_changed = %d", sim_status, card_changed);

	if (TAPI_SIM_STATUS_CARD_NOT_PRESENT == sim_status ||
			TAPI_SIM_STATUS_CARD_REMOVED == sim_status)
	{
		ret = helper_delete_SDN_contact();
		h_retvm_if(CTS_SUCCESS != ret, ret, "helper_delete_SDN_contact() Failed(%d)", ret);
	}
	else if (TAPI_SIM_STATUS_SIM_INIT_COMPLETED == sim_status)
	{
		ret = helper_delete_SDN_contact();
		h_retvm_if(CTS_SUCCESS != ret, ret, "helper_delete_SDN_contact() Failed(%d)", ret);

		ret = tel_get_sim_pb_count(TAPI_SIM_PB_SDN, &req_id);
		h_retvm_if(TAPI_API_SUCCESS != ret, CTS_ERR_TAPI_FAILED,
				"tel_get_sim_pb_count() Failed(%d)", ret);

		//ret = contacts_svc_begin_trans();
		//h_retvm_if(CTS_SUCCESS != ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

		helper_import_sim_data = data;
	}

	return CTS_SUCCESS;
}

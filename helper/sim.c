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

unsigned short HELPER_GSM7BitTable[128] = {
	0x0040, 0x00A3, 0x0024, 0x00A5, 0x00E8, 0x00E9, 0x00F9, 0x00EC,
	0x00F2, 0x00E7, 0x000A, 0x00D8, 0x00F8, 0x000D, 0x00C5, 0x00E5,
	0x0394, 0x005F, 0x03A6, 0x0393, 0x039B, 0x03A9, 0x03A0, 0x03A8,
	0x03A3, 0x0398, 0x039E, 0x00A0, 0x00C6, 0x00E6, 0x00DF, 0x00C9,
	0x0020, 0x0021, 0x0022, 0x0023, 0x00A4, 0x0025, 0x0026, 0x0027,
	0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
	0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
	0x00A1, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
	0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
	0x0058, 0x0059, 0x005A, 0x00C4, 0x00D6, 0x00D1, 0x00DC, 0x00A7,
	0x00BF, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
	0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
	0x0078, 0x0079, 0x007A, 0x00E4, 0x00F6, 0x00F1, 0x00FC, 0x00E0
};

static int helper_sim_data_to_utf8(TelSimTextEncrypt_t type,
		char *src, int src_len, char *dest, int dest_size)
{
	h_retvm_if(0 == src_len || NULL == src, CTS_ERR_ARG_INVALID,
			"src(%p, len=%d) is invalid", src, src_len);
	h_retvm_if(0 == dest_size || NULL == dest, CTS_ERR_ARG_INVALID,
			"dest(%p, len=%d) is invalid", dest, dest_size);

	switch (type)
	{
	case TAPI_SIM_TEXT_ENC_GSM7BIT:
	case TAPI_SIM_TEXT_ENC_ASCII:
		memcpy(dest, src, CTS_MIN(dest_size, src_len));
		dest[CTS_MIN(dest_size-1, src_len)] = '\0';
		break;
	case TAPI_SIM_TEXT_ENC_UCS2:
	case TAPI_SIM_TEXT_ENC_HEX:
		return helper_unicode_to_utf8(src, src_len, dest, dest_size);
	default:
		ERR("Unknown Encryption Type(%d)", type);
		return CTS_ERR_ARG_INVALID;
	}

	return CTS_SUCCESS;
}

#define HELPER_SIM_DATA_MAX_LENGTH 1024

static int helper_insert_SDN(TelSimPb2GData_t *pb2g_data)
{
	int ret;
	char utf_data[HELPER_SIM_DATA_MAX_LENGTH];

	ret = helper_sim_data_to_utf8(pb2g_data->NameEncryptType, (char *)pb2g_data->Name,
			CTS_MIN(sizeof(pb2g_data->Name), pb2g_data->NameLen),
			utf_data, sizeof(utf_data));
	h_retvm_if(ret != CTS_SUCCESS, ret, "helper_sim_data_to_utf8 is Failed(%d)", ret);

	ret = helper_insert_SDN_contact(utf_data, (char *)pb2g_data->Number);
	h_retvm_if(ret != CTS_SUCCESS, ret, "helper_insert_SDN_contact() Failed(%d)", ret);

	return ret;
}

static int helper_insert_2g_contact(int index, TelSimPb2GData_t *pb2g_data)
{
	int ret, found_id;
	char uid[32];
	char utf_data[HELPER_SIM_DATA_MAX_LENGTH];
	CTSstruct *contact;
	GSList *numbers=NULL;
	CTSvalue *name_val, *number_val, *base;

	h_retvm_if(index <= 0, CTS_ERR_ARG_INVALID, "The index(%d) is invalid", index);

	snprintf(uid, sizeof(uid), "SIM:%s-%s-%s-%d",
			TAPI_imsi.szMcc, TAPI_imsi.szMnc, TAPI_imsi.szMsin, index);
	HELPER_DBG("UID = %s", uid);

	found_id = contacts_svc_find_contact_by(CTS_FIND_BY_UID, uid);

	ret = helper_sim_data_to_utf8(pb2g_data->NameEncryptType, (char *)pb2g_data->Name,
			CTS_MIN(sizeof(pb2g_data->Name), pb2g_data->NameLen),
			utf_data, sizeof(utf_data));
	h_retvm_if(ret != CTS_SUCCESS, ret, "helper_sim_data_to_utf8 is Failed(%d)", ret);

	contact = contacts_svc_struct_new(CTS_STRUCT_CONTACT);

	base = contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);
	if (base) {
		contacts_svc_value_set_str(base, CTS_BASE_VAL_UID_STR, uid);
		contacts_svc_struct_store_value(contact, CTS_CF_BASE_INFO_VALUE, base);
		contacts_svc_value_free(base);
	}

	name_val = contacts_svc_value_new(CTS_VALUE_NAME);
	if (name_val) {
		contacts_svc_value_set_str(name_val, CTS_NAME_VAL_DISPLAY_STR, utf_data);
		contacts_svc_struct_store_value(contact, CTS_CF_NAME_VALUE, name_val);
		contacts_svc_value_free(name_val);
	}

	number_val = contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number_val) {
		contacts_svc_value_set_str(number_val, CTS_NUM_VAL_NUMBER_STR,
				(char *)pb2g_data->Number);
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

static int helper_insert_3g_contact(int index, TelSimPb3GData_t *pb3g_data)
{
	int i, ret, found_id;
	char uid[32];
	char utf_data[HELPER_SIM_DATA_MAX_LENGTH];
	CTSstruct *contact;
	CTSvalue *name_val=NULL, *number_val, *email_val, *base;
	TelSimPb3GFileDataInfo_t temp;
	GSList *numbers=NULL, *emails=NULL;

	h_retvm_if(index <= 0, CTS_ERR_ARG_INVALID, "The index(%d) is invalid", index);

	snprintf(uid, sizeof(uid), "SIM:%s-%s-%s-%d",
			TAPI_imsi.szMcc, TAPI_imsi.szMnc, TAPI_imsi.szMsin, index);
	HELPER_DBG("UID = %s", uid);
	found_id = contacts_svc_find_contact_by(CTS_FIND_BY_UID, uid);

	contact = contacts_svc_struct_new(CTS_STRUCT_CONTACT);

	base = contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);
	if (base) {
		contacts_svc_value_set_str(base, CTS_BASE_VAL_UID_STR, uid);
		contacts_svc_struct_store_value(contact, CTS_CF_BASE_INFO_VALUE, base);
		contacts_svc_value_free(base);
	}

	for (i=0;i<pb3g_data->FileTypeCount;i++)
	{
		temp = pb3g_data->PbFileDataInfo[i];
		switch (temp.FileType)
		{
		case TAPI_PB_3G_NAME:
		case TAPI_PB_3G_SNE:
			ret = helper_sim_data_to_utf8(temp.FileDataType.EncryptionType,
					(char *)temp.FileData,
					CTS_MIN(sizeof(temp.FileData), temp.FileDataLength),
					utf_data, sizeof(utf_data));
			if (ret != CTS_SUCCESS) {
				ERR("helper_sim_data_to_utf8() Failed(%d)", ret);
				goto CONVERT_3GPB_FAIL;
			}

			if (!name_val)
				name_val = contacts_svc_value_new(CTS_VALUE_NAME);
			if (name_val) {
				if (TAPI_PB_3G_NAME == temp.FileType)
					contacts_svc_value_set_str(name_val, CTS_NAME_VAL_FIRST_STR, utf_data);
				else
					contacts_svc_value_set_str(name_val, CTS_NAME_VAL_LAST_STR, utf_data);
			}

			contacts_svc_struct_store_value(contact, CTS_CF_NAME_VALUE, name_val);
			break;
		case TAPI_PB_3G_NUMBER:
		case TAPI_PB_3G_ANR:
		case TAPI_PB_3G_ANRA:
		case TAPI_PB_3G_ANRB:
			number_val = contacts_svc_value_new(CTS_VALUE_NUMBER);
			if (number_val) {
				contacts_svc_value_set_str(number_val, CTS_NUM_VAL_NUMBER_STR,
						(char *)temp.FileData);
				//contacts_svc_value_set_int(number_val, CTS_NUM_VAL_TYPE_INT, CTS_NUM_TYPE_CELL);
				if (TAPI_PB_3G_NUMBER == temp.FileType)
					contacts_svc_value_set_bool(number_val, CTS_NUM_VAL_DEFAULT_BOOL, true);
			}
			numbers = g_slist_append(numbers, number_val);
			contacts_svc_struct_store_list(contact, CTS_CF_NUMBER_LIST, numbers);
			contacts_svc_value_free(number_val);
			break;
		case TAPI_PB_3G_EMAIL:
			ret = helper_sim_data_to_utf8(temp.FileDataType.EncryptionType,
					(char *)temp.FileData,
					CTS_MIN(sizeof(temp.FileData), temp.FileDataLength),
					utf_data, sizeof(utf_data));
			if (ret != CTS_SUCCESS) {
				ERR("helper_sim_data_to_utf8() Failed(%d)", ret);
				goto CONVERT_3GPB_FAIL;
			}

			email_val = contacts_svc_value_new(CTS_VALUE_EMAIL);
			if (email_val) {
				contacts_svc_value_set_str(email_val, CTS_EMAIL_VAL_ADDR_STR,
						(char *)temp.FileData);
				contacts_svc_value_set_bool(email_val, CTS_NUM_VAL_DEFAULT_BOOL, true);
			}
			emails = g_slist_append(emails, email_val);
			contacts_svc_struct_store_list(contact, CTS_CF_EMAIL_LIST, emails);
			contacts_svc_value_free(email_val);
			break;
		default:
			ERR("Unknown file type=%d", temp.FileType);
			break;
		}
	}

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

CONVERT_3GPB_FAIL:
	contacts_svc_struct_free(contact);
	return ret;
}

static int helper_sim_read_record_cb(const TelTapiEvent_t *sim_event, void *data)
{
	int ret, saved_pb_num, i=0, req_id;
	TelSimPbRecordData_t *sim_info;

	HELPER_FN_CALL;

	h_retvm_if(TAPI_EVENT_CLASS_SIM != sim_event->EventClass ||
			TAPI_EVENT_SIM_PB_ACCESS_READ_CNF != sim_event->EventType,
			CTS_ERR_TAPI_FAILED,
			"Unknown Event(EventClass = 0x%X, EventType = 0x%X",
			sim_event->EventClass, sim_event->EventType);

	sim_info = (TelSimPbRecordData_t*)sim_event->pData;
	if (NULL == sim_info) {
		ERR("sim_info is NULL, Status = %d", sim_event->Status);
		goto ERROR_RETURN;
	}

	if (TAPI_SIM_PB_SUCCESS != sim_event->Status) {
		if (TAPI_SIM_PB_SDN == sim_info->StorageFileType &&
				TAPI_SIM_PB_INVALID_INDEX == sim_event->Status)
		{
			HELPER_DBG("Index = %d", sim_info->Index);
			ret = tel_read_sim_pb_record(sim_info->StorageFileType,
					sim_info->Index+1, &req_id);
			if (TAPI_API_SUCCESS != ret) {
				ERR("tel_read_sim_pb_record() Failed(%d)", ret);
				goto ERROR_RETURN;
			}
			return CTS_SUCCESS;
		}
		ERR("SIM phonebook access Failed(%d)", sim_event->Status);
		goto ERROR_RETURN;
	}

	switch (sim_info->StorageFileType)
	{
	case TAPI_SIM_PB_SDN:
		saved_pb_num = sim_event->pDataLen / sizeof(TelSimPbRecordData_t);
		if (saved_pb_num <= 0 || TAPI_SIM_3G_PB_MAX_RECORD_COUNT < saved_pb_num) {
			ERR("received saved_pb_num is invalid(%d)", saved_pb_num);
			goto ERROR_RETURN;
		}
		while (true) {
			ret = helper_insert_SDN(&sim_info->ContactInfo.Pb2GData);
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
			ret = helper_insert_2g_contact(sim_info->Index, &sim_info->ContactInfo.Pb2GData);
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
			ret = helper_insert_3g_contact(sim_info->Index, &sim_info->ContactInfo.Pb3GData);
			h_warn_if(ret < CTS_SUCCESS, "helper_insert_3g_contact() is Failed(%d)", ret);
			if (saved_pb_num == ++i) break;
			sim_info++;
		}
		break;
	case TAPI_SIM_PB_FDN:
	case TAPI_SIM_PB_MSISDN:
	default:
		ERR("Unknown storage type(%d)", sim_info->StorageFileType);
		goto ERROR_RETURN;
	}
	if (sim_info->NextIndex && CTS_TAPI_SIM_PB_MAX != sim_info->NextIndex) {
		HELPER_DBG("NextIndex = %d", sim_info->NextIndex);
		ret = tel_read_sim_pb_record(sim_info->StorageFileType,
				sim_info->NextIndex, &req_id);
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
		if (CTS_TAPI_SIM_PB_MAX == first_id) return CTS_ERR_NO_DATA;
		ret = tel_read_sim_pb_record(storage, first_id, &req_id);
		h_retvm_if(TAPI_API_SUCCESS != ret, CTS_ERR_TAPI_FAILED,
				"tel_read_sim_pb_record() Failed(%d)", ret);
	}

	ret = contacts_svc_begin_trans();
	h_retvm_if(CTS_SUCCESS != ret, ret, "contacts_svc_begin_trans() Failed(%d)", ret);

	helper_import_sim_data = data;

	return CTS_SUCCESS;
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
		goto ERROR_RETURN;
	}

	if (TAPI_SIM_PB_SUCCESS != sim_event->Status) {
		ERR("SIM phonebook access Failed(%d)", sim_event->Status);
		goto ERROR_RETURN;
	}

	switch (sim_info->StorageFileType)
	{
	case TAPI_SIM_PB_SDN:
		if (sim_info->UsedRecordCount) {
			ret = tel_read_sim_pb_record(TAPI_SIM_PB_SDN, 1, &req_id);
			h_retvm_if(TAPI_API_SUCCESS != ret, CTS_ERR_TAPI_FAILED,
					"tel_read_sim_pb_record() Failed(%d)", ret);
		}
		break;
	case TAPI_SIM_PB_ADN:
	case TAPI_SIM_PB_3GSIM:
	case TAPI_SIM_PB_FDN:
	case TAPI_SIM_PB_MSISDN:
	default:
		ERR("Unknown storage type(%d)", sim_info->StorageFileType);
		goto ERROR_RETURN;
	}

	return CTS_SUCCESS;

ERROR_RETURN:
	helper_deregister_tapi_sim_event();
	return CTS_ERR_TAPI_FAILED;
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

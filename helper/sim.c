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
#include <tapi_common.h>
#include <ITapiSim.h>
#include <ITapiPhonebook.h>
#include <TapiUtility.h>
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

#define TAPI_PB_MAX_FILE_CNT TAPI_PB_3G_PBC+1
#define TAPI_PB_NAME_INDEX TAPI_PB_3G_NAME
#define TAPI_PB_NUMBER_INDEX TAPI_PB_3G_NUMBER

static TelSimImsiInfo_t TAPI_imsi;
static void *helper_sim_data = NULL;
static TapiHandle *handle;
static int helper_register_tapi_cnt = 0;
static TelSimPbType_t sim_type = TAPI_SIM_PB_UNKNOWNN;
static int text_max_len[TAPI_PB_MAX_FILE_CNT];
static int used_count[TAPI_PB_MAX_FILE_CNT];
static int max_count[TAPI_PB_MAX_FILE_CNT];

static int helper_register_tapi_init(void)
{
	if (0 == helper_register_tapi_cnt) {
		handle = tel_init(NULL);
		h_retvm_if(NULL == handle, CTS_ERR_TAPI_FAILED,
				"tel_init() is Failed()");
	}
	helper_register_tapi_cnt++;

	return CTS_SUCCESS;
}

static int helper_deregister_tapi_deinit(void)
{
	int ret;
	if (1 == helper_register_tapi_cnt) {
		ret = tel_deinit(handle);
		h_retvm_if(TAPI_API_SUCCESS != ret, CTS_ERR_TAPI_FAILED,
				"tel_deinit() Failed(%d)", ret);
	}
	helper_register_tapi_cnt--;

	return CTS_SUCCESS;
}

#ifdef NO_USE_TAPI_DECODER
static int helper_sim_data_to_utf8(TelSimTextEncrypt_t type,
		char *src, int src_len, char *dest, int dest_size)
{
	h_retvm_if(0 == src_len || NULL == src, CTS_ERR_ARG_INVALID,
			"src(%p, len=%d) is invalid", src, src_len);
	h_retvm_if(0 == dest_size || NULL == dest, CTS_ERR_ARG_INVALID,
			"dest(%p, len=%d) is invalid", dest, dest_size);

	switch (type) {
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
#endif

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

static void helper_sim_read_record_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
	HELPER_FN_CALL;
	int ret;
	TelSimPbAccessResult_t sec_rt = result;
	TelSimPbRecord_t *sim_info = data;

	if (NULL == sim_info) {
		ERR("sim_info is NULL, result = %d", sec_rt);
		goto ERROR_RETURN;
	}

	if (TAPI_SIM_PB_SUCCESS != sec_rt) {
		if (TAPI_SIM_PB_SDN == sim_info->phonebook_type &&
				TAPI_SIM_PB_INVALID_INDEX == sec_rt) {
			HELPER_DBG("Index = %d", sim_info->index);
			ret = tel_read_sim_pb_record(handle, sim_info->phonebook_type,
					sim_info->index+1, helper_sim_read_record_cb, NULL);
			if (TAPI_API_SUCCESS != ret) {
				ERR("tel_read_sim_pb_record() Failed(%d)", ret);
				goto ERROR_RETURN;
			}
			return;
		}
		ERR("SIM phonebook access Failed(%d)", sec_rt);
		goto ERROR_RETURN;
	}

	switch (sim_info->phonebook_type) {
	case TAPI_SIM_PB_SDN:
		ret = helper_insert_SDN(sim_info);
		h_warn_if(ret < CTS_SUCCESS, "helper_insert_SDN() is Failed(%d)", ret);
		break;
	case TAPI_SIM_PB_ADN:
		ret = helper_insert_2g_contact(sim_info);
		h_warn_if(ret < CTS_SUCCESS, "helper_insert_2g_contact() is Failed(%d)", ret);
		break;
	case TAPI_SIM_PB_3GSIM:
		ret = helper_insert_3g_contact(sim_info);
		h_warn_if(ret < CTS_SUCCESS, "helper_insert_3g_contact() is Failed(%d)", ret);
		break;
	case TAPI_SIM_PB_FDN:
	default:
		ERR("Unknown storage type(%d)", sim_info->phonebook_type);
		goto ERROR_RETURN;
	}

	if (sim_info->next_index && CTS_TAPI_SIM_PB_MAX != sim_info->next_index) {
		HELPER_DBG("NextIndex = %d", sim_info->next_index);
		ret = tel_read_sim_pb_record(handle, sim_info->phonebook_type,
				sim_info->next_index, helper_sim_read_record_cb, NULL);
		if (TAPI_API_SUCCESS != ret) {
			ERR("tel_read_sim_pb_record() Failed(%d)", ret);
			goto ERROR_RETURN;
		}
	}
	else {
		if (TAPI_SIM_PB_ADN == sim_info->phonebook_type ||
			TAPI_SIM_PB_3GSIM == sim_info->phonebook_type)
			contacts_svc_end_trans(true);
		if (helper_sim_data) {
			ret = helper_socket_return(helper_sim_data, CTS_SUCCESS, 0, NULL);
			h_warn_if(CTS_SUCCESS != ret, "helper_socket_return() Failed(%d)", ret);
			helper_sim_data = NULL;
			memset(&TAPI_imsi, 0x00, sizeof(TelSimImsiInfo_t));
		}

		helper_deregister_tapi_deinit();
		helper_trim_memory();
	}
	return;

ERROR_RETURN:
	if (TAPI_SIM_PB_ADN == sim_info->phonebook_type ||
		TAPI_SIM_PB_3GSIM == sim_info->phonebook_type)
		contacts_svc_end_trans(false);
	if (helper_sim_data) {
		ret = helper_socket_return(helper_sim_data, CTS_SUCCESS, 0, NULL);
		h_warn_if(CTS_SUCCESS != ret, "helper_socket_return() Failed(%d)", ret);
		helper_sim_data = NULL;
		memset(&TAPI_imsi, 0x00, sizeof(TelSimImsiInfo_t));
	}
	helper_deregister_tapi_deinit();
	helper_trim_memory();
	return;
}

static void helper_sim_aync_response_pb_count(TapiHandle *handle, int result, void *data, void *user_data)
{
	HELPER_FN_CALL;
	int ret = CTS_SUCCESS;
	TelSimPbAccessResult_t access_rt = result;
	TelSimPbStorageInfo_t *sim_info = data;

	if (NULL == sim_info) {
		ERR("sim_info is NULL");
		ret = CTS_ERR_TAPI_FAILED;
		goto ERROR_RETURN;
	}

	if (TAPI_SIM_PB_SUCCESS != access_rt) {
		ERR("SIM phonebook access Failed(%d)", access_rt);
		ret = CTS_ERR_TAPI_FAILED;
		goto ERROR_RETURN;
	}

	switch (sim_info->StorageFileType) {
	case TAPI_SIM_PB_SDN:
		if (sim_info->UsedRecordCount) {
			HELPER_DBG("SDN count = %d", sim_info->UsedRecordCount);
			ret = tel_read_sim_pb_record(handle, sim_info->StorageFileType, 1, helper_sim_read_record_cb, NULL);
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
			ret = tel_read_sim_pb_record(handle, sim_info->StorageFileType, 1, helper_sim_read_record_cb, NULL);
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
			ret = CTS_ERR_NO_DATA;
			goto ERROR_RETURN;
		}
		break;
	case TAPI_SIM_PB_FDN:
	default:
		ERR("Unknown storage type(%d)", sim_info->StorageFileType);
		ret = CTS_ERR_FAIL;
		goto ERROR_RETURN;
	}

	return;

ERROR_RETURN:
	if (helper_sim_data) {
		ret = helper_socket_return(helper_sim_data, ret, 0, NULL);
		h_warn_if(CTS_SUCCESS != ret, "helper_socket_return() Failed(%d)", ret);
		helper_sim_data = NULL;
	}
	helper_deregister_tapi_deinit();
}

int helper_sim_read_pb_record(void *data)
{
	int ret;
	int sim_pb_inited;
	TelSimPbList_t pb_list = {0};
	TelSimCardType_t cardInfo;

	h_retvm_if(NULL != helper_sim_data, CTS_ERR_ENV_INVALID,
			"Helper is already processing with sim");

	ret = helper_register_tapi_init();
	h_retvm_if(CTS_SUCCESS != ret, ret,
			"helper_register_tapi_init() Failed(%d)", ret);

	if (sim_type == TAPI_SIM_PB_UNKNOWNN) {
		ret = tel_get_sim_type(handle, &cardInfo);
		if(TAPI_API_SUCCESS != ret) {
			ERR("tel_get_sim_type() Failed(%d)", ret);
			goto ERROR_RETURN;
		}

		if (TAPI_SIM_CARD_TYPE_USIM == cardInfo)
			sim_type = TAPI_SIM_PB_3GSIM;
		else
			sim_type = TAPI_SIM_PB_ADN;
	}

	ret = tel_get_sim_pb_init_info(handle, &sim_pb_inited, &pb_list);
	if(TAPI_API_SUCCESS != ret) {
		ERR("tel_get_sim_pb_init_info() Failed(%d)", ret);
		HELPER_DBG("sim_pb_inited(%d)", sim_pb_inited);
		goto ERROR_RETURN;
	}

	ret = tel_get_sim_imsi(handle, &TAPI_imsi);
	if(TAPI_API_SUCCESS != ret) {
		ERR("tel_get_sim_imsi() Failed(%d)", ret);
		goto ERROR_RETURN;
	}

	if (sim_pb_inited) {
		ret = tel_get_sim_pb_count(handle, sim_type, helper_sim_aync_response_pb_count, NULL);
		if(TAPI_API_SUCCESS != ret) {
			ERR("tel_get_sim_pb_count() Failed(%d)", ret);
			goto ERROR_RETURN;
		}
	}

	helper_sim_data = data;

	return CTS_SUCCESS;

ERROR_RETURN:
	helper_deregister_tapi_deinit();
	return CTS_ERR_TAPI_FAILED;
}

static void helper_sim_aync_response_pb_update(TapiHandle *handle, int result, void *data, void *user_data)
{
	HELPER_FN_CALL;
	int ret;
	TelSimPbAccessResult_t access_rt = result;

	if (TAPI_SIM_PB_SUCCESS != access_rt) {
		ERR("SIM phonebook access Failed(%d)", access_rt);
		ret = CTS_ERR_TAPI_FAILED;
	}
	else {
		HELPER_DBG("Success");
		ret = CTS_SUCCESS;
	}

	if (helper_sim_data) {
		ret = helper_socket_return(helper_sim_data, ret, 0, NULL);
		h_warn_if(CTS_SUCCESS != ret, "helper_socket_return() Failed(%d)", ret);
		helper_sim_data = NULL;
	}
	helper_deregister_tapi_deinit();
}

static inline int helper_sim_get_display_name(CTSvalue *name, char *dest, int dest_size)
{
	int len = 0;
	const char *first, *last;

	first = contacts_svc_value_get_str(name, CTS_NAME_VAL_FIRST_STR);
	last = contacts_svc_value_get_str(name, CTS_NAME_VAL_LAST_STR);
	if (!first && !last)
		return 0;
	else if (!last)
		len = snprintf(dest, dest_size, "%s", first);
	else if (!first)
		len = snprintf(dest, dest_size, "%s", last);
	else if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
		len = snprintf(dest, dest_size, "%s %s", first, last);
	else
		len = snprintf(dest, dest_size, "%s, %s", last, first);

	return len;
}

static void helper_sim_write_contact(int index, TelSimPbType_t type, void *user_data)
{
	HELPER_FN_CALL;
	int ret;
	int text_len;
	int pindex = (int)user_data;
	TelSimPbRecord_t pb;
	CTSstruct *person = NULL;
	CTSvalue *value = NULL;
	GSList *list = NULL;
	GSList *cursor = NULL;
	memset(&pb, 0, sizeof(TelSimPbRecord_t));

	HELPER_DBG("person index : %d", pindex);

	ret = contacts_svc_get_person(pindex, &person);
	if(CTS_SUCCESS != ret) {
		ERR("contacts_svc_get_person is failed(%d)", ret);
		goto ERROR_RETURN;
	}

	pb.phonebook_type = type;
	pb.index = index;

	HELPER_DBG("phonebook_type[%d] 0:fdn, 1:adn, 2:sdn, 3:3gsim, 4:aas, 5:gas, index[%d]", pb.phonebook_type, pb.index);

	ret = contacts_svc_struct_get_value(person, CTS_CF_NAME_VALUE, &value);
	if (CTS_SUCCESS == ret) {
		char name[HELPER_SIM_DATA_MAX_LENGTH] = {0};
		text_len = MIN(sizeof(pb.number), text_max_len[TAPI_PB_NAME_INDEX]);
		ret = helper_sim_get_display_name(value, name, text_len);
		if (ret)
			snprintf((char*)pb.name, text_len, "%s", name);
		HELPER_DBG("name : %s, pb_name : %s", name, (char *)pb.name);
	}

	ret = contacts_svc_struct_get_list(person, CTS_CF_NUMBER_LIST, &list);
	if (CTS_SUCCESS == ret) {
		text_len = MIN(sizeof(pb.number), text_max_len[TAPI_PB_NUMBER_INDEX]);
		cursor = list;
		snprintf((char*)pb.number, text_len, "%s", contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));

		if (TAPI_SIM_PB_3GSIM == type) {
			if (0 < max_count[TAPI_PB_3G_ANR1] - used_count[TAPI_PB_3G_ANR1]) {
				cursor = cursor->next;
				if (cursor) {
					text_len = MIN(sizeof(pb.anr1), text_max_len[TAPI_PB_3G_ANR1]);
					snprintf((char*)pb.anr1, text_len, "%s", contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
					cursor = cursor->next;
				}
			}
			if (0 < max_count[TAPI_PB_3G_ANR2] - used_count[TAPI_PB_3G_ANR2]) {
				if (cursor) {
					text_len = MIN(sizeof(pb.anr2), text_max_len[TAPI_PB_3G_ANR2]);
					snprintf((char*)pb.anr2, text_len, "%s", contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
					cursor = cursor->next;
				}
			}
			if (0 < max_count[TAPI_PB_3G_ANR3] - used_count[TAPI_PB_3G_ANR3]) {
				if (cursor) {
					text_len = MIN(sizeof(pb.anr3), text_max_len[TAPI_PB_3G_ANR3]);
					snprintf((char*)pb.anr3, text_len, "%s", contacts_svc_value_get_str(cursor->data, CTS_NUM_VAL_NUMBER_STR));
				}
			}
		}
		INFO("pb.nubmer : %s, pb.anr1 : %s, pb.anr2 : %s, pb.arn3 : %s", pb.number, pb.anr1, pb.anr2, pb.anr3);
	}

	if (TAPI_SIM_PB_3GSIM == type) {
		ret = contacts_svc_struct_get_list(person, CTS_CF_EMAIL_LIST, &list);
		if (CTS_SUCCESS == ret) {
			cursor = list;
			if (0 < max_count[TAPI_PB_3G_EMAIL1] - used_count[TAPI_PB_3G_EMAIL1]) {
				if (cursor) {
					text_len = MIN(sizeof(pb.email1), text_max_len[TAPI_PB_3G_EMAIL1]);
					snprintf((char*)pb.email1, text_len, "%s", contacts_svc_value_get_str(cursor->data, CTS_EMAIL_VAL_ADDR_STR));
					cursor = cursor->next;
				}
			}
			if (0 < max_count[TAPI_PB_3G_EMAIL2] - used_count[TAPI_PB_3G_EMAIL2]) {
				if (cursor) {
					text_len = MIN(sizeof(pb.email2), text_max_len[TAPI_PB_3G_EMAIL2]);
					snprintf((char*)pb.email2, text_len, "%s", contacts_svc_value_get_str(cursor->data, CTS_EMAIL_VAL_ADDR_STR));
					cursor = cursor->next;
				}
			}
			if (0 < max_count[TAPI_PB_3G_EMAIL3] - used_count[TAPI_PB_3G_EMAIL3]) {
				if (cursor) {
					text_len = MIN(sizeof(pb.email3), text_max_len[TAPI_PB_3G_EMAIL3]);
					snprintf((char*)pb.email3, text_len, "%s", contacts_svc_value_get_str(cursor->data, CTS_EMAIL_VAL_ADDR_STR));
					cursor = cursor->next;
				}
			}
			if (0 < max_count[TAPI_PB_3G_EMAIL4] - used_count[TAPI_PB_3G_EMAIL4]) {
				if (cursor) {
					text_len = MIN(sizeof(pb.email4), text_max_len[TAPI_PB_3G_EMAIL4]);
					snprintf((char*)pb.email4, text_len, "%s", contacts_svc_value_get_str(cursor->data, CTS_EMAIL_VAL_ADDR_STR));
				}
			}
			INFO("pb.email1 : %s, pb.email2 : %s, pb.email3 : %s, pb.email4 : %s", pb.email1, pb.email2, pb.email3, pb.email4);
		}
	}

	contacts_svc_struct_free(person);
	ret = tel_update_sim_pb_record(handle, &pb, helper_sim_aync_response_pb_update, NULL);
	if(TAPI_API_SUCCESS != ret) {
		ERR("tel_update_sim_pb_record() Failed(%d)", ret);
		goto ERROR_RETURN;
	}

	return;

ERROR_RETURN:
	if (helper_sim_data) {
		ret = helper_socket_return(helper_sim_data, CTS_SUCCESS, 0, NULL);
		h_warn_if(CTS_SUCCESS != ret, "helper_socket_return() Failed(%d)", ret);
		helper_sim_data = NULL;
	}
	helper_deregister_tapi_deinit();
	helper_trim_memory();
	return;
}

static void helper_sim_find_empty_slot_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
	HELPER_FN_CALL;
	int ret;
	TelSimPbAccessResult_t sec_rt = result;
	TelSimPbRecord_t *sim_info = data;

	if (NULL == sim_info) {
		ERR("sim_info is NULL, result = %d", sec_rt);
		goto ERROR_RETURN;
	}

	if (TAPI_SIM_PB_SUCCESS != sec_rt) {
		if (TAPI_SIM_PB_INVALID_INDEX == sec_rt) {
			HELPER_DBG("Index = %d", sim_info->index);
			helper_sim_write_contact(sim_info->index, sim_info->phonebook_type, user_data);
			return;
		}
		ERR("SIM phonebook access Failed(%d)", sec_rt);
		goto ERROR_RETURN;
	}

	if (sim_info->next_index && CTS_TAPI_SIM_PB_MAX != sim_info->next_index) {
		HELPER_DBG("NextIndex = %d", sim_info->next_index);
		int diff = sim_info->next_index-sim_info->index;
		if (1 == diff) {
			ret = tel_read_sim_pb_record(handle, sim_info->phonebook_type,
					sim_info->next_index, helper_sim_find_empty_slot_cb, user_data);
			if (TAPI_API_SUCCESS != ret) {
				ERR("tel_read_sim_pb_record() Failed(%d)", ret);
				goto ERROR_RETURN;
			}
		}
		else if(1 < diff) {
			helper_sim_write_contact(sim_info->index+1, sim_info->phonebook_type, user_data);
		}
		else {
			ERR("There is no empty record");
			goto ERROR_RETURN;
		}
	}
	else if (sim_info->index+1 && CTS_TAPI_SIM_PB_MAX != sim_info->index+1){
		helper_sim_write_contact(sim_info->index+1, sim_info->phonebook_type, user_data);
	}
	else {
		ERR("There is no empty record");
		goto ERROR_RETURN;
	}
	return;

ERROR_RETURN:
	if (helper_sim_data) {
		ret = helper_socket_return(helper_sim_data, CTS_SUCCESS, 0, NULL);
		h_warn_if(CTS_SUCCESS != ret, "helper_socket_return() Failed(%d)", ret);
		helper_sim_data = NULL;
	}
	helper_deregister_tapi_deinit();
	return;
}

static void helper_sim_check_write_available(TapiHandle *handle, int result, void *data, void *user_data)
{
	HELPER_FN_CALL;
	int ret;
	TelSimPbAccessResult_t access_rt = result;
	TelSimPbStorageInfo_t *ps = data;

	if (NULL == ps) {
		ERR("PbStorageInfo is NULL, result = %d", access_rt);
		goto ERROR_RETURN;
	}

	INFO("StorageFileType[%d] 0:fdn, 1:adn, 2:sdn, 3:3gsim, 4:aas, 5:gas", ps->StorageFileType);
	INFO("TotalRecordCount[%d]", ps->TotalRecordCount);
	INFO("UsedRecordCount[%d]", ps->UsedRecordCount);

	if (ps->UsedRecordCount < ps->TotalRecordCount) {
		ret = tel_read_sim_pb_record(handle, ps->StorageFileType,
						1, helper_sim_find_empty_slot_cb, user_data);
		if(TAPI_API_SUCCESS != ret) {
			ERR("tel_read_sim_pb_record() Failed(%d)", ret);
			goto ERROR_RETURN;
		}
	}
	else {
		ERR("SIM phonebook(Type:%d) is full", ps->StorageFileType);
		goto ERROR_RETURN;
	}

	return;

ERROR_RETURN:
	if (helper_sim_data) {
		ret = helper_socket_return(helper_sim_data, CTS_SUCCESS, 0, NULL);
		h_warn_if(CTS_SUCCESS != ret, "helper_socket_return() Failed(%d)", ret);
		helper_sim_data = NULL;
	}
	helper_deregister_tapi_deinit();
	return;
}

static bool helper_get_usim_meta_info(TelSimPbCapabilityInfo_t *capa)
{
	HELPER_FN_CALL;
	int i;

	for (i=0; i < TAPI_PB_MAX_FILE_CNT;i++) {
		text_max_len[i] = 0;
		used_count[i] = 0;
		max_count[i] = 0;
	}

	for (i=0; i < capa->FileTypeCount; i++) {
		INFO("======================================");
		INFO(" capa->FileTypeInfo[%d].field_type[%d]",i, capa->FileTypeInfo[i].field_type);
		INFO(" capa->FileTypeInfo[%d].index_max[%d]",i, capa->FileTypeInfo[i].index_max);
		INFO(" capa->FileTypeInfo[%d].text_max[%d]",i, capa->FileTypeInfo[i].text_max);
		INFO(" capa->FileTypeInfo[%d].used_count[%d]",i, capa->FileTypeInfo[i].used_count);
		switch (capa->FileTypeInfo[i].field_type){
		case TAPI_PB_3G_NAME:
		case TAPI_PB_3G_NUMBER:
		case TAPI_PB_3G_ANR1:
		case TAPI_PB_3G_ANR2:
		case TAPI_PB_3G_ANR3:
		case TAPI_PB_3G_EMAIL1:
		case TAPI_PB_3G_EMAIL2:
		case TAPI_PB_3G_EMAIL3:
		case TAPI_PB_3G_EMAIL4:
		case TAPI_PB_3G_SNE :
		case TAPI_PB_3G_GRP:
		case TAPI_PB_3G_PBC:
			text_max_len[capa->FileTypeInfo[i].field_type] = capa->FileTypeInfo[i].text_max;
			used_count[capa->FileTypeInfo[i].field_type] += capa->FileTypeInfo[i].used_count;
			max_count[capa->FileTypeInfo[i].field_type] += capa->FileTypeInfo[i].index_max;
			break;
		default:
			break;
		}
	}

	for (i=0; i < TAPI_PB_MAX_FILE_CNT; i++) {
		INFO(" field_type[%d] : index_max(%d), text_max(%d), used_count(%d)", i,
				max_count[i], text_max_len[i], used_count[i]);
	}
	return true;
}

static void helper_get_sim_pb_meta_info(TapiHandle *handle, int result, void *data, void *user_data)
{
	HELPER_FN_CALL;
	int ret;
	TelSimPbAccessResult_t access_rt = result;

	if (TAPI_SIM_PB_3GSIM == sim_type) {
		TelSimPbCapabilityInfo_t *capa = data;
		if (NULL == capa) {
			ERR("PbCapabilityInfo_t is NULL, result = %d", access_rt);
			goto ERROR_RETURN;
		}

		if (!helper_get_usim_meta_info(capa))
			goto ERROR_RETURN;

		if (0 < max_count[TAPI_PB_3G_NAME] - used_count[TAPI_PB_3G_NAME]){
			ret = tel_read_sim_pb_record(handle, sim_type,
							1, helper_sim_find_empty_slot_cb, user_data);
			if(TAPI_API_SUCCESS != ret) {
				ERR("tel_read_sim_pb_record() Failed(%d)", ret);
				goto ERROR_RETURN;
			}
		}
		else {
			ERR("SIM phonebook(Type:%d) is full", sim_type);
			goto ERROR_RETURN;
		}
	}
	else {
		TelSimPbEntryInfo_t *pe = data;
		if (NULL == pe) {
			ERR("PbStorageInfo is NULL, result = %d", access_rt);
			goto ERROR_RETURN;
		}

		INFO("PbNumLenMax[%d]",pe->PbNumLenMax);
		INFO("PbTextLenMax[%d]",pe->PbTextLenMax);
		text_max_len[TAPI_PB_NAME_INDEX] = pe->PbTextLenMax;
		text_max_len[TAPI_PB_NUMBER_INDEX] = pe->PbNumLenMax;

		ret = tel_get_sim_pb_count(handle, sim_type, helper_sim_check_write_available, (void*)user_data);
		if (TAPI_API_SUCCESS != ret) {
			ret = CTS_ERR_TAPI_FAILED;
			ERR("tel_get_sim_pb_count() Failed(%d)", ret);
			goto ERROR_RETURN;
		}
	}

	return;

ERROR_RETURN:
	if (helper_sim_data) {
		ret = helper_socket_return(helper_sim_data, CTS_SUCCESS, 0, NULL);
		h_warn_if(CTS_SUCCESS != ret, "helper_socket_return() Failed(%d)", ret);
		helper_sim_data = NULL;
	}
	helper_deregister_tapi_deinit();
	return;
}

int helper_sim_write_pb_record(void *data, int index)
{
	int ret;
	int sim_pb_inited;
	TelSimPbList_t pb_list = {0};
	TelSimCardType_t cardInfo;

	h_retvm_if(NULL != helper_sim_data, CTS_ERR_ENV_INVALID,
			"Helper is already processing with sim");

	ret = helper_register_tapi_init();
	h_retvm_if(CTS_SUCCESS != ret, ret,
			"helper_register_tapi_init() Failed(%d)", ret);

	if (sim_type == TAPI_SIM_PB_UNKNOWNN) {
		ret = tel_get_sim_type(handle, &cardInfo);
		if(TAPI_API_SUCCESS != ret) {
			ret = CTS_ERR_TAPI_FAILED;
			ERR("tel_get_sim_type() Failed(%d)", ret);
			goto ERROR_RETURN;
		}

		if (TAPI_SIM_CARD_TYPE_USIM == cardInfo)
			sim_type = TAPI_SIM_PB_3GSIM;
		else
			sim_type = TAPI_SIM_PB_ADN;
	}

	ret = tel_get_sim_pb_init_info(handle, &sim_pb_inited, &pb_list);
	if (TAPI_API_SUCCESS != ret) {
		ret = CTS_ERR_TAPI_FAILED;
		ERR("tel_get_sim_pb_init_info() Failed(%d)", ret);
		goto ERROR_RETURN;
	}

	if (sim_pb_inited) {
		if (TAPI_SIM_PB_3GSIM == sim_type)
			ret = tel_get_sim_pb_usim_meta_info(handle, helper_get_sim_pb_meta_info, (void*)index);
		else
			ret = tel_get_sim_pb_meta_info(handle, sim_type, helper_get_sim_pb_meta_info, (void*)index);

		if (TAPI_API_SUCCESS != ret) {
			ret = CTS_ERR_TAPI_FAILED;
			ERR("tel_get_sim_pb_meta_info() Failed(%d)", ret);
			goto ERROR_RETURN;
		}

		helper_sim_data = data;
	}
	else {
		ret = CTS_ERR_TAPI_FAILED;
		ERR("SIM status is not enabled(%d)", sim_pb_inited);
		goto ERROR_RETURN;
	}

	return CTS_SUCCESS;

ERROR_RETURN:
	helper_deregister_tapi_deinit();
	return ret;
}

int helper_sim_read_SDN(void* data)
{
	HELPER_FN_CALL;
	int ret, card_changed = 0;
	TelSimCardStatus_t sim_status;

	h_retvm_if(NULL != helper_sim_data, CTS_ERR_ENV_INVALID,
			"Helper is already processing with sim");
	sim_type = TAPI_SIM_PB_UNKNOWNN;

	ret = helper_register_tapi_init();
	h_retvm_if(TAPI_API_SUCCESS != ret, ret,
			"helper_register_tapi_init() Failed(%d)", ret);

	ret = tel_get_sim_init_info(handle, &sim_status, &card_changed);
	if(TAPI_API_SUCCESS != ret) {
		ERR("tel_get_sim_init_info() Failed(%d)", ret);
		HELPER_DBG("sim_status = %d, card_changed = %d", sim_status, card_changed);
		goto ERROR_RETURN;
	}

	if (TAPI_SIM_STATUS_CARD_NOT_PRESENT == sim_status ||
			TAPI_SIM_STATUS_CARD_REMOVED == sim_status) {
		ret = helper_delete_SDN_contact();
		if(CTS_SUCCESS != ret) {
			ERR("helper_delete_SDN_contact() Failed(%d)", ret);
			goto ERROR_RETURN;
		}
		helper_deregister_tapi_deinit();
	}
	else if (TAPI_SIM_STATUS_SIM_INIT_COMPLETED == sim_status) {
		ret = helper_delete_SDN_contact();
		if(CTS_SUCCESS != ret) {
			ERR("helper_delete_SDN_contact() Failed(%d)", ret);
			goto ERROR_RETURN;
		}

		ret = tel_read_sim_pb_record(handle, TAPI_SIM_PB_SDN,
						1, helper_sim_read_record_cb, NULL);
		if(TAPI_API_SUCCESS != ret) {
			ERR("tel_read_sim_pb_record() Failed(%d)", ret);
			ret = CTS_ERR_TAPI_FAILED;
			goto ERROR_RETURN;
		}
	}

	return CTS_SUCCESS;

ERROR_RETURN:
	helper_deregister_tapi_deinit();
	helper_trim_memory();
	return ret;
}

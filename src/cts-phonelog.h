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
#ifndef __CTS_PHONELOG_H__
#define __CTS_PHONELOG_H__

//<!--
/**
 * @defgroup   CONTACTS_SVC_PLOG Phone Logs Modification
 * @ingroup    CONTACTS_SVC
 * @addtogroup CONTACTS_SVC_PLOG
 * @{
 *
 * This interface provides methods to insert/update/delete the Phone Logs.
 *
 */

/**
 * This function inserts a phone log to database.
 *
 * @param[in] phone_log A phone log information of CTSvalue() created by contacts_svc_value_new(CTS_VALUE_PHONELOG).
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void phonelog_insert_test(void)
 {
    CTSvalue *plog;

    plog = contacts_svc_value_new(CTS_VALUE_PHONELOG);
    contacts_svc_value_set_str(plog, CTS_PLOG_VAL_NUMBER_STR, "0123456789");
    contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TIME_INT,(int) time(NULL));
    contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TYPE_INT,
                                     CTS_PLOG_TYPE_VOICE_INCOMMING);
    contacts_svc_value_set_int(plog, CTS_PLOG_VAL_DURATION_INT, 65);
    contacts_svc_insert_phonelog(plog);

    contacts_svc_value_set_str(plog, CTS_PLOG_VAL_NUMBER_STR, "0987654321");
    contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TIME_INT,(int) time(NULL));
    contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TYPE_INT,
                                     CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN);
    contacts_svc_value_set_int(plog, CTS_PLOG_VAL_DURATION_INT, 65);
    contacts_svc_insert_phonelog(plog);

    contacts_svc_value_set_str(plog, CTS_PLOG_VAL_NUMBER_STR, "0987654321");
    contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TIME_INT,(int) time(NULL));
    contacts_svc_value_set_int(plog, CTS_PLOG_VAL_LOG_TYPE_INT,
                                     CTS_PLOG_TYPE_VOICE_INCOMMING);
    contacts_svc_value_set_int(plog, CTS_PLOG_VAL_DURATION_INT, 65);
    contacts_svc_insert_phonelog(plog);


    contacts_svc_value_free(plog);
 }
 * @endcode
 */
int contacts_svc_insert_phonelog(CTSvalue* phone_log);

/**
 * Use for contacts_svc_delete_phonelog().
 */
typedef enum{
	CTS_PLOG_DEL_BY_ID, /**< .*/
	CTS_PLOG_DEL_BY_NUMBER, /**< .*/
	CTS_PLOG_DEL_NO_NUMBER, /**< .*/
	CTS_PLOG_DEL_BY_MSGID, /**< .*/
}cts_del_plog_op;
/**
 * This function deletes a phone log with op_code(#CTS_PLOG_DEL_BY_ID, #CTS_PLOG_DEL_BY_NUMBER).
 * @par int contacts_svc_delete_phonelog(CTS_PLOG_DEL_BY_ID, int index)
 * @par int contacts_svc_delete_phonelog(CTS_PLOG_DEL_BY_NUMBER, char *number)
 * Delete all phone logs related with number.
 *
 * @param[in] op_code #cts_del_plog_op
 * @param[in] index (optional) Index of the phone log
 * @param[in] number (optional) Number to be deleted
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 contacts_svc_delete_phonelog(CTS_PLOG_DEL_BY_ID, 3);
 contacts_svc_delete_phonelog(CTS_PLOG_DEL_BY_NUMBER, "0123456789");
 * @endcode
 */
int contacts_svc_delete_phonelog(cts_del_plog_op op_code, ...);

/**
 * This function modifies a phone log from unseen to seen.
 * \n Type should be #CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN or #CTS_PLOG_TYPE_VIDEO_INCOMMING_UNSEEN
 *
 * @param[in] index Index of the phone log
 * @param[in] type The current type of phone log
 * @par example
 * @code
 contacts_svc_phonelog_set_seen(2, CTS_PLOG_TYPE_VOICE_INCOMMING_UNSEEN);
 * @endcode
 */
int contacts_svc_phonelog_set_seen(int index, int type);

/**
 * Use for contacts_svc_phonelog_get_last_number().
 */
typedef enum{
	CTS_PLOG_LAST_ALL, /**< .*/
	CTS_PLOG_LAST_CALL_ONLY, /**< .*/
	CTS_PLOG_LAST_VIDEO_CALL_ONLY, /**< .*/
}cts_plog_get_last_op;

/**
 * This function gets the string of the most recent outgoing call number.
 * \n It specifys by op(#cts_plog_get_last_op).
 * \n It checks voice and video call(Not SMS/MMS).
 * \n It doesn't include the rejected and blocked numbers.
 * \n The obtained string should be free using by free().
 * @return string of the last number, or NULL if no value is obtained or on error
 */
char* contacts_svc_phonelog_get_last_number(cts_plog_get_last_op op);

/**
 * This function gets phonelog record which has the index from the database.
 * Obtained phonelog record should be freed by using contacts_svc_value_free().
 * @param[in] plog_id The index of phonelog to get
 * @param[out] phonelog Points of the phonelog record which is returned
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
 void get_phonelog(void)
 {
    int ret;
    CTSvalue *plog;

    ret = contacts_svc_get_phonelog(1, &plog);
    if(ret < 0)
    {
       printf("No found record\n");
       return;
    }

    printf("Number : %s\n", contacts_svc_value_get_str(plog, CTS_PLOG_VAL_NUMBER_STR));
    printf("Related ID : %d\n", contacts_svc_value_get_int(plog, CTS_PLOG_VAL_ID_INT));
    printf("Time : %d\n", contacts_svc_value_get_int(plog, CTS_PLOG_VAL_LOG_TIME_INT));
    printf("Type : %d\n", contacts_svc_value_get_int(plog, CTS_PLOG_VAL_LOG_TYPE_INT));
    printf("Duration : %d\n", contacts_svc_value_get_int(plog, CTS_PLOG_VAL_DURATION_INT));
    printf("Related ID : %d\n", contacts_svc_value_get_int(plog, CTS_PLOG_VAL_RELATED_ID_INT));

    contacts_svc_value_free(plog);
 }
 * @endcode
 */
int contacts_svc_get_phonelog(int plog_id, CTSvalue **phonelog);

/**
 * @}
 */

//-->

#endif //__CTS_PHONELOG_H__


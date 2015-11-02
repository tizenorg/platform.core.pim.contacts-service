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

#include <ctype.h>
#include <unicode/ustring.h>

#include <TapiUtility.h>
#include <ITapiNetwork.h>
#include <sqlite3.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_server_setting.h"
#include "ctsvc_normalize.h"
#include "ctsvc_localize_utils.h"
#include "ctsvc_number_utils.h"

typedef struct {
	int mcc;
	char *cc;
}ctsvc_mcc_cc_map;

const static ctsvc_mcc_cc_map __mcc_cc_list[] = {
	{0, "1"},
	{202, "30"},
	{204, "31"},
	{206, "32"},
	{208, "33"},
	{212, "377"},
	{213, "376"},
	{214, "34"},
	{216, "36"},
	{218, "387"},
	{219, "385"},
	{220, "381"},
	{222, "39"},
	{225, "39"},
	{226, "40"},
	{228, "41"},
	{230, "420"},
	{231, "421"},
	{232, "43"},
	{234, "44"},
	{235, "44"},
	{238, "45"},
	{240, "46"},
	{242, "47"},
	{244, "358"},
	{246, "370"},
	{247, "371"},
	{248, "372"},
	{250, "7"},
	{255, "380"},
	{257, "375"},
	{259, "373"},
	{260, "48"},
	{262, "49"},
	{266, "350"},
	{268, "351"},
	{270, "352"},
	{272, "353"},
	{274, "354"},
	{276, "355"},
	{278, "356"},
	{280, "357"},
	{282, "995"},
	{283, "374"},
	{284, "359"},
	{286, "90"},
	{288, "298"},
	{290, "299"},
	{292, "378"},
	{293, "386"},
	{294, "389"},
	{295, "423"},
	{297, "382"},
	{302, "1"},
	{308, "508"},
	{310, "1"},
	{311, "1"},
	{312, "1"},
	{313, "1"},
	{314, "1"},
	{315, "1"},
	{316, "1"},
	{330, "1"},
	{332, "1"},
	{334, "52"},
	{338, "1"},
	{340, "590"},
	{340, "596"},
	{342, "1"},
	{344, "1"},
	{346, "1"},
	{348, "1"},
	{350, "1"},
	{352, "1"},
	{354, "1"},
	{356, "1"},
	{358, "1"},
	{360, "1"},
	{362, "599"},
	{363, "297"},
	{364, "1"},
	{365, "1"},
	{366, "1"},
	{368, "53"},
	{370, "1"},
	{372, "509"},
	{374, "1"},
	{376, "1"},
	{400, "994"},
	{401, "7"},
	{402, "975"},
	{404, "91"},
	{405, "91"},
	{406, "91"},
	{410, "92"},
	{412, "93"},
	{413, "94"},
	{414, "95"},
	{415, "961"},
	{416, "962"},
	{417, "963"},
	{418, "964"},
	{419, "965"},
	{420, "966"},
	{421, "967"},
	{422, "968"},
	{424, "971"},
	{425, "972"},
	{426, "973"},
	{427, "974"},
	{428, "976"},
	{429, "977"},
	{430, "971"},
	{431, "971"},
	{432, "98"},
	{434, "998"},
	{436, "992"},
	{437, "996"},
	{438, "993"},
	{440, "81"},
	{441, "81"},
	{450, "82"},
	{452, "84"},
	{454, "852"},
	{455, "853"},
	{456, "855"},
	{457, "856"},
	{460, "86"},
	{461, "86"},
	{466, "886"},
	{467, "850"},
	{470, "880"},
	{472, "960"},
	{502, "60"},
	{505, "61"},
	{510, "62"},
	{514, "670"},
	{515, "63"},
	{520, "66"},
	{525, "65"},
	{528, "673"},
	{530, "64"},
	{536, "674"},
	{537, "675"},
	{539, "676"},
	{540, "677"},
	{541, "678"},
	{542, "679"},
	{543, "681"},
	{544, "1"},
	{545, "686"},
	{546, "687"},
	{547, "689"},
	{548, "682"},
	{549, "685"},
	{550, "691"},
	{551, "692"},
	{552, "680"},
	{602, "20"},
	{603, "213"},
	{604, "212"},
	{605, "216"},
	{606, "218"},
	{607, "220"},
	{608, "221"},
	{609, "222"},
	{610, "223"},
	{611, "224"},
	{612, "225"},
	{613, "226"},
	{614, "227"},
	{615, "228"},
	{616, "229"},
	{617, "230"},
	{618, "231"},
	{619, "232"},
	{620, "233"},
	{621, "234"},
	{622, "235"},
	{623, "236"},
	{624, "237"},
	{625, "238"},
	{626, "239"},
	{627, "240"},
	{628, "241"},
	{629, "242"},
	{630, "243"},
	{631, "244"},
	{632, "245"},
	{633, "248"},
	{634, "249"},
	{635, "250"},
	{636, "251"},
	{637, "252"},
	{638, "253"},
	{639, "254"},
	{640, "255"},
	{641, "256"},
	{642, "257"},
	{643, "258"},
	{645, "260"},
	{646, "261"},
	{647, "262"},
	{648, "263"},
	{649, "264"},
	{650, "265"},
	{651, "266"},
	{652, "267"},
	{653, "268"},
	{654, "269"},
	{655, "27"},
	{657, "291"},
	{702, "501"},
	{704, "502"},
	{706, "503"},
	{708, "504"},
	{710, "505"},
	{712, "506"},
	{714, "507"},
	{716, "51"},
	{722, "54"},
	{724, "55"},
	{730, "56"},
	{732, "57"},
	{734, "58"},
	{736, "591"},
	{738, "592"},
	{740, "593"},
	{742, "594"},
	{744, "595"},
	{746, "597"},
	{748, "598"},
	{750, "500"},
};

static char *cc = NULL;
static TapiHandle *handle_for_cc = NULL;

char* ctsvc_get_network_cc(bool reload)
{
	int i;
	int state;
	int ret;
	int mcc = 0;
	char *temp = NULL;
	TapiHandle *handle = NULL;
	static bool cc_loaded = false;

	if (cc_loaded && false == reload)
		return cc;

	cc_loaded = true;
	cc = NULL;
	handle = (TapiHandle *)ctsvc_init_tapi_handle_for_cc();
	RETVM_IF(NULL == handle, NULL, "tel_init() Fail");

	ret = tel_get_property_int(handle, TAPI_PROP_NETWORK_SERVICE_TYPE, &state);
	if (ret != TAPI_API_SUCCESS) {
		CTS_ERR("tel_get_property_int Fail(%d)", ret);
		tel_deinit(handle);
		return NULL;
	}

	if (state == TAPI_NETWORK_SERVICE_TYPE_UNKNOWN
			|| state == TAPI_NETWORK_SERVICE_TYPE_NO_SERVICE
			|| state == TAPI_NETWORK_SERVICE_TYPE_EMERGENCY
			|| state == TAPI_NETWORK_SERVICE_TYPE_SEARCH) {
		CTS_INFO("network service is not working : state(%d)", state);
		return NULL;
	}

	ret = tel_get_property_string(handle, TAPI_PROP_NETWORK_PLMN,  &temp);
	if (ret != TAPI_API_SUCCESS) {
		CTS_ERR("tel_get_property_string Fail(%d)", ret);
		return NULL;
	}

	if (temp && 3 < strlen(temp))
		temp[3] = '\0';
	mcc = atoi(temp);
	for (i=0;i<sizeof(__mcc_cc_list)/sizeof(ctsvc_mcc_cc_map);i++) {
		if (__mcc_cc_list[i].mcc == mcc) {
			cc = __mcc_cc_list[i].cc;
			break;
		}
	}

	return cc;
}

static void __ctsvc_network_cc_changed(TapiHandle *handle, const char *noti_id, void *data, void *user_data)
{
	ctsvc_get_network_cc(true);
}

void* ctsvc_init_tapi_handle_for_cc()
{
	if (handle_for_cc)
		return handle_for_cc;

	handle_for_cc = tel_init(NULL);
	if (handle_for_cc) {
		int ret = tel_register_noti_event(handle_for_cc,
				TAPI_PROP_NETWORK_PLMN, __ctsvc_network_cc_changed, NULL);
		WARN_IF(ret != TAPI_API_SUCCESS, "tel_register_noti_event Fail(%d)", ret);
	}
	else
		CTS_ERR("tel_init fail");

	return handle_for_cc;
}

void ctsvc_deinit_tapi_handle_for_cc()
{
	if (handle_for_cc) {
		int ret = tel_deregister_noti_event(handle_for_cc,  TAPI_PROP_NETWORK_PLMN);
		WARN_IF(ret != TAPI_API_SUCCESS, "tel_register_noti_event Fail(%d)", ret);
		tel_deinit(handle_for_cc);
	}
	handle_for_cc = NULL;
}

static inline int __ctsvc_phone_number_has_country_code(const char *src, int len)
{
	int ret = 0;

	if (len <= 0)
		return 0;

	switch (src[ret++]-'0') {
	case 1:
	case 7:
		break;
	case 2:
		if (len <= ret)	return 0;
		switch (src[ret++]-'0') {
		case 0:
		case 7:
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 8:
		case 9:
			ret += 1;
			break;
		default:
			CTS_ERR("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 3:
		if (len <= ret)	return 0;
		switch (src[ret++]-'0') {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 6:
		case 9:
			break;
		case 5:
		case 7:
		case 8:
			ret += 1;
			break;
		default:
			CTS_ERR("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 4:
		if (len <= ret)	return 0;
		switch (src[ret++]-'0') {
		case 0:
		case 1:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			break;
		case 2:
			ret += 1;
			break;
		default:
			CTS_ERR("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 5:
		if (len <= ret)	return 0;
		switch (src[ret++]-'0') {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			break;
		case 0:
		case 9:
			ret += 1;
			break;
		default:
			CTS_ERR("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 6:
		if (len <= ret)	return 0;
		switch (src[ret++]-'0') {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			break;
		case 7:
		case 8:
		case 9:
			ret += 1;
			break;
		default:
			CTS_ERR("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 8:
		if (len <= ret)	return 0;
		switch (src[ret++]-'0') {
		case 1:
		case 2:
		case 4:
		case 6:
			break;
		case 0:
		case 3:
		case 5:
		case 7:
		case 8:
		case 9:
			ret += 1;
			break;
		default:
			CTS_ERR("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 9:
		if (len <= ret)	return 0;
		switch (src[ret++]-'0') {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 8:
			break;
		case 6:
		case 7:
		case 9:
			ret += 1;
			break;
		default:
			CTS_ERR("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 0:
	default:
		CTS_ERR("The parameter(src:%s) has invalid character set", src);
		return 0;
	}

	return ret;
}

/*
 * Number Matching Rule
 * refer to the http://www.itu.int/dms_pub/itu-t/opb/sp/T-SP-E.164C-2011-PDF-E.pdf
 */
enum {
	CTSVC_PLUS_ONLY,     /* + */
	CTSVC_PLUS_IP_ONLY,  /* +IP (International prefix) */
	CTSVC_PLUS_CC_ONLY,  /* +CC (Country code) */
	CTSVC_PLUS_IP_CC,    /* +IP CC */
	CTSVC_IP_ONLY,       /* IP */
	CTSVC_CC_ONLY,       /* CC */
	CTSVC_IP_CC,         /* IP CC */
	CTSVC_NONE,
};

static int __ctsvc_number_has_ip_and_cc(const char*number, int len, int *index)
{
	bool have_cc = false;
	bool have_plus = false;
	int ret = CTSVC_NONE;
	int start_index;
	int match_len;
	*index = 0;

	/* Check IP */
	start_index = 0;
	match_len = 0;

	switch(number[start_index]) {
	case '+':
		start_index++;
		have_plus = true;
		if (len <= start_index) {
			*index = start_index;
			return CTSVC_PLUS_ONLY;   /* '+' */
		}
	default:
		{
			/*
			 * IP can be
			 *  0 (Turks and Caicos Islands, Samoa)
			 *  00, 011, 0011, 010, 000
			 *  001/007 (Cambodia), 001/008 (Indonesia, Singapore)
			 *  001/002 (Korea), 002(Taiwan)
			 *  810 (Belarus, Kazakhstan, Russian, Tajikistan, Turkmenistan)
			 *  009/007/005(Colombia), 009(Nigeria)
			 *  119 (Cuba)
			 *  00/012/013/014 (Israel)
			 */
			switch(number[start_index]) {
			case '0':   /* '+0' */
				{
					start_index++;
					if (len <= start_index) {
						*index = start_index;
						return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY); /* '+0' */
					}

					switch(number[start_index]) {
					case '0':   /* '+00' */
						{
							start_index++;
							if (len <= start_index) {
								*index = start_index;
								return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);      /* '+00' */
							}

							switch(number[start_index]) {
							case '0':   /* '+000' */
							case '2':   /* '+002' */
							case '5':   /* '+005' */
							case '7':   /* '+007' */
							case '8':   /* '+008' */
							case '9':   /* '+009' */
									    /* or '+00 CC' */
								start_index++;
								if (len <= start_index) {
									*index = start_index;
									return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);  /* '+00Y' */
								}

								have_cc = __ctsvc_phone_number_has_country_code(&number[start_index], len-start_index);
								if (0 < have_cc) {
									*index = start_index;
									return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);      /* '+00Y CC' */
								}
								else {
									have_cc = __ctsvc_phone_number_has_country_code(&number[start_index-1], len-start_index+1);
									if (0 < have_cc) {
										*index = start_index-1;
										return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);  /* '+00 CC' */
									}
								}
								*index = start_index;
								return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);      /* '+00Y XXX' */
							case '1':   /* '+001' */
								start_index++;
								if (len <= start_index) {
									*index = start_index;
									return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);  /* '+001' */
								}

								if (number[start_index] == '1') {
									start_index++;
									if (len <= start_index) {
										*index = start_index;
										return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);   /* '+0011' */
									}

									have_cc = __ctsvc_phone_number_has_country_code(&number[start_index], len-start_index);
									if (0 < have_cc) {
										*index = start_index;
										return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);  /*  '+0011 CC' */
									}
									start_index--;
								}

								have_cc = __ctsvc_phone_number_has_country_code(&number[start_index], len-start_index);
								*index = start_index;
								if (0 < have_cc)
									return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);      /* '+001 CC' */
								else
									return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);  /* '+001 XXX' */
							default:   /* '+00 3', '+00 4', '+00 6' */
								*index = start_index;
								have_cc = __ctsvc_phone_number_has_country_code(&number[start_index], len-start_index);
								if (0 < have_cc)
									return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);      /* '+00 CC' */
								else
									return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);  /* '+00 XXX' */
							}   /* end of fourth switch */
						}
						break;
					case '1':   /* '+01' */
						{
							start_index++;
							if (len <= start_index) {
								*index = start_index-1;   /* '+0 1' */
								return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_NONE);
							}

							switch(number[start_index]) {
							case '0':   /* '+010' */
							case '1':   /* '+011' */
							case '2':   /* '+012' */
							case '3':   /* '+013' */
							case '4':   /* '+014' */
								{
									start_index++;
									if (len <= start_index) {
										*index = start_index;
										return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);   /* '+01Y' */
									}

									have_cc = __ctsvc_phone_number_has_country_code(&number[start_index], len-start_index);
									*index = start_index;
									if (0 < have_cc)
										return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);   /* '+01Y CC' */
									else
										return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);   /* '+01Y XXX' */
								}
								break;
							default:
								*index = start_index-1;   /* '+0 1' */
								return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_NONE);
							}
						}
						break;
					default:   /* '+0 CC' */
						{
							have_cc = __ctsvc_phone_number_has_country_code(&number[start_index], len-start_index);
							*index = start_index;
							if (0 < have_cc)
								return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);          /* '+0 CC' */
							else
								return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);      /* '+0 XXX' */
						}
						break;
					}   /* end of third switch */
				}
				break;   /* end of '+0' */
			case '1':   /* '+1' */
				start_index++;
				if (start_index+2 <= len && STRING_EQUAL == strncmp(&number[start_index], "19", 2)) {   /* '+119' */
					match_len = start_index + 2;
					ret = (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);
				}
				else {
					match_len = start_index-1;
					ret = (have_plus?CTSVC_PLUS_ONLY:CTSVC_NONE);   /* '+ CC' */
				}
				break;
			case '8':   /* '+8' */
				start_index++;
				if (start_index+2 <= len && STRING_EQUAL == strncmp(&number[start_index], "10", 2)) {   /* '+810' */
					match_len = start_index + 2;
					ret = (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);
				}
				else {
					match_len = start_index-1;
					ret = (have_plus?CTSVC_PLUS_ONLY:CTSVC_NONE);   /* '+ CC' */
				}
				break;
			default:
				match_len = start_index;
				ret = (have_plus?CTSVC_PLUS_ONLY:CTSVC_NONE);   /* '+ CC' */
				break;
			}   /* end of second switch */
		}
		break;   /* '+' default */
	}   /* end of first switch */
	*index = match_len;

	/* Check CC */
	if (match_len < len) {
		have_cc = __ctsvc_phone_number_has_country_code(&number[match_len], len-match_len);
		if (0 < have_cc) {
			switch (ret) {
			case CTSVC_NONE:
				return CTSVC_CC_ONLY;
			case CTSVC_PLUS_ONLY:
				return CTSVC_PLUS_CC_ONLY;
			case CTSVC_PLUS_IP_ONLY:
				return CTSVC_PLUS_IP_CC;
			case CTSVC_IP_ONLY:
				return CTSVC_IP_CC;
			}
		}
	}
	return ret;
}

int ctsvc_normalize_number(const char *src, char *dest, int dest_size, bool replace_alphabet)
{
	int index;
	int n;
	char *cc = NULL;
	int d_pos = 0;
	int first_zero = 0;

	if (NULL == src) {
		CTS_ERR("The parameter(src) is NULL");
		return 0;
	}

	d_pos = strlen(src);
	if (d_pos <= 0)
		return d_pos;

	cc = ctsvc_get_network_cc(false);
	n = __ctsvc_number_has_ip_and_cc(src, d_pos-ctsvc_get_phonenumber_min_match_digit(), &index);

	if (src[0] == '0'   /* remove first '0' */
			|| (cc && cc[0] == '7' && src[0] == '8'))   /* Russian */
		first_zero = 1;

	/*
	 * 001 82 10 1234 5678 -> +82 10 1234 5678
	 * +001 82 10 1234 5678 -> +82 10 1234 5678
	 * 82 10 1234 5678 -> +82 10 1234 5678
	 * add '+'
	 * do not append + if the number without cc is too short
	 * cc 010-1234-5678 ==> +cc 010-1234-5678, cc3456 => cc3456
	 */
	if (CTSVC_IP_CC == n || CTSVC_CC_ONLY == n) {
		if (d_pos + 1 < dest_size) {
			dest[0] = '+';
			memcpy(dest+1, src, d_pos+1);
			d_pos++;
			dest[d_pos] = 0;
			return d_pos;
		}
	}
	else if (CTSVC_PLUS_ONLY == n || CTSVC_PLUS_CC_ONLY == n
		 || CTSVC_PLUS_IP_ONLY == n || CTSVC_PLUS_IP_CC == n) {
		if (d_pos < dest_size) {
			/* Just copy */
			memcpy(dest, src, d_pos+1);
			dest[d_pos] = 0;
			return d_pos;
		}
	}
	/* append country code */
	else {	/* CTSVC_NONE,        invalid case : CTSVC_IP_ONLY */
		if (cc && ctsvc_get_phonenumber_min_match_digit() <= d_pos) {
			/*
			 * add '+cc'
			 * do not append cc if the number is too short
			 * 010-1234-5678 ==> +cc 10-1234-5678, 1234 ==> 1234
			 * 8 XXX-XXX-XX-XX ===> +7 XXX-XXX-XX-XX
			 */
			if (d_pos + strlen(cc) + 1 < dest_size) {
				dest[0] = '+';
				memcpy(dest+1, cc, strlen(cc));
				memcpy(dest+1+strlen(cc), src+first_zero, d_pos+1-first_zero);
				d_pos += (1+strlen(cc));
				dest[d_pos] = 0;
				return d_pos;
			}
		}
	}

	memcpy(dest, src, d_pos+1);
	dest[d_pos] = 0;

	return d_pos;
}

/*
 * vaild character : digit, +, *, #,, ;, alphabet(depends on replace_alphabet parameter)
 * Remove invalid string from number
 */
int ctsvc_clean_number(const char *src, char *dest, int dest_size, bool replace_alphabet)
{
	int s_pos;
	int d_pos;
	int pos;
	char temp[dest_size];

	if (NULL == src) {
		CTS_ERR("The parameter(src) is NULL");
		return 0;
	}

	s_pos = 0;
	pos = 0;
	while (src[s_pos] != 0) {
		int char_len;
		if (dest_size-2 < pos) break;

		char_len = ctsvc_check_utf8(src[s_pos]);
		if (char_len <= 0) {
			break;
		}

		if (char_len == 3) {
			/* fullwidth -> halfwidth */
			if (src[s_pos] == 0xef) {
				if (src[s_pos+1] == 0xbc) {
					if (0x90 <= src[s_pos+2] && src[s_pos+2] <= 0x99)        /* ef bc 90 : '0' ~ ef bc 99 : '9' */
						temp[pos++] = src[s_pos+2] - 0x60;
					else if (0xa1 <= src[s_pos+2] && src[s_pos+2] <= 0xba)   /* ef bc a1 : 'A' ~ ef bc ba : 'Z' */
						temp[pos++] = src[s_pos+2] - 0x60;
					else if (0x8b == src[s_pos+2])   /* ef bc 8b : '+' */
						temp[pos++] = '+';
					else if (0x8a == src[s_pos+2])   /* ef bc 8a : '*' */
						temp[pos++] = '*';
					else if (0x83 == src[s_pos+2])   /* ef bc 83 : '#' */
						temp[pos++] = '#';
					else if (0x8c == src[s_pos+2])   /* ef bc 8c : ',' */
						temp[pos++] = ',';
					else if (0x9b == src[s_pos+2])   /* ef bc 9b : ';' */
						temp[pos++] = ';';
				}
				else if (src[s_pos+1] == 0xbd
						&& (0x81 <= src[s_pos+2] && src[s_pos+2] <= 0x9a))   /* ef bd 81 : 'a' ~ ef bd 9a : 'z' */
					temp[pos++] = src[s_pos+2] - 0x40;
			}
			else {
				s_pos += char_len;
				continue;
			}
		}
		else if (char_len == 1) {
			if (0x41 <= src[s_pos] && src[s_pos] <= 0x5a)        /* 'A' ~ 'Z' */
				temp[pos++] = src[s_pos];
			else if (0x61 <= src[s_pos] && src[s_pos] <= 0x7a)   /* 'a' ~ 'z' */
				temp[pos++] = src[s_pos] - 0x20;
			else
				temp[pos++] = src[s_pos];
		}
		s_pos += char_len;
	}
	temp[pos] = 0x0;

	pos = 0;
	d_pos = 0;
	while (temp[pos] != 0) {
		if ('0' <= temp[pos] && temp[pos] <= '9')
			dest[d_pos++] = temp[pos];
		else if (temp[pos] == '+' || temp[pos] == '#'
				|| temp[pos] == '*' || temp[pos] == ';' || temp[pos] == ',')
			dest[d_pos++] = temp[pos];
		pos++;
	}
	dest[d_pos] = 0;

	return d_pos;
}

static int __ctsvc_minmatch_number(const char *src, char *dest, int dest_size, int min_match)
{
	int i;
	int len = 0;
	int d_pos = 0;
	const char *temp_number;
	const char *cc = ctsvc_get_network_cc(false);

	if ('+' == src[0]) {
		len = __ctsvc_phone_number_has_country_code(&src[1], strlen(src)-1);
		temp_number = src + len +1;
	}
	else if ('0' == src[0])
		temp_number = src+1;
	else if (cc && cc[0] == '7' && src[0] == '8')
		temp_number = src+1;
	else
		temp_number = src;

	len = strlen(temp_number);

	if (0 < len) {
		while (0 <= (len-d_pos-1) && temp_number[len-d_pos-1]
				&& d_pos < min_match) {
			if (dest_size-d_pos == 0) {
				CTS_ERR("Destination string buffer is not enough(%s)", src);
				return CONTACTS_ERROR_INTERNAL;
			}

			dest[d_pos] = temp_number[len-d_pos-1];
			d_pos++;
		}
		dest[d_pos] = 0;

		len = strlen(dest);
		for (i=0; i<len/2;i++) {
			char c;
			c = dest[i];
			dest[i] = dest[len-i-1];
			dest[len-i-1] = c;
		}
	}
	else {
		memcpy(dest, src, strlen(src));
		dest[strlen(src)] = 0;
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_get_minmatch_number(const char *src, char *dest, int dest_size, int min_match)
{
	int ret;

	RETV_IF(NULL == src, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dest, CONTACTS_ERROR_INVALID_PARAMETER);

	ret = __ctsvc_minmatch_number(src, dest, dest_size, min_match);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("__ctsvc_minmatch_number() Fail(%d)", ret);
		return ret;
	}

	return CONTACTS_ERROR_NONE;
}

static bool __ctsvc_is_phonenumber_halfwidth(const char* keyword)
{
	int i;
	int len = strlen(keyword);

	/* TODO: we should add predicate including '+' */
	/* TODO: finally, we try to check the number with regular expression. */
	for (i=0; i<len; i++) {
		if ((keyword[i] < '0' || '9' < keyword[i]) && keyword[i] != '+') {
			CTS_ERR("keyword[%d]: %c is not number)", i, keyword[i]);
			return false;
		}
	}
	return true;
}

#define UTF8_FULLWIDTH_LENGTH 3
static bool __ctsvc_is_phonenumber_fullwidth(const char* keyword)
{
	int char_len = 1;
	int str_len;
	int i;
	if (keyword == NULL || *keyword == '\0')
		return false;

	str_len = strlen(keyword);
	for (i=0;i<str_len;i += char_len) {
		char_len = ctsvc_check_utf8(keyword[i]);
		if (char_len != UTF8_FULLWIDTH_LENGTH || str_len-i < UTF8_FULLWIDTH_LENGTH)
			return false;

		if (keyword[i] == 0xef) {
			if (keyword[i+1] == 0xbc) {
				if (0x90 <= keyword[i+2] && keyword[i+2] <= 0x99)            /* ef bc 90 : '0' ~ ef bc 99 : '9' */
					continue;
				else if (0x8b == keyword[i+2])   /* ef bc 8b : '+' */
					continue;
				else
					return false;
			}
			else
				return false;
		}
		else
			return false;
	}
	return true;
}

bool ctsvc_is_phonenumber(const char* src)
{
	return (__ctsvc_is_phonenumber_halfwidth(src) || __ctsvc_is_phonenumber_fullwidth(src));
}

/* numbers are cleaned number or normalized number */
static bool __ctsvc_number_compare(const char *number1, const char *number2)
{
	int len1;
	int len2;
	int matched;
	int minmatch_len = ctsvc_get_phonenumber_min_match_digit();
	const char *cc = ctsvc_get_network_cc(false);

	if (NULL == number1 || NULL == number2 || '\0' == *number1 || '\0' == *number2)
		return false;

	len1 = strlen(number1);
	len2 = strlen(number2);

	/* compare  number in reverse order */
	for (matched = 0; 0 < len1 && 0 < len2;) {
		if (number1[len1-1] != number2[len2-1])
			break;
		len1--;
		len2--;
		matched++;
	}

	/* full match */
	if (len1 == 0 && len2 == 0)
		return true;

	/* one is substring of the other string */
	if (minmatch_len <= matched&& (len1 == 0 || len2 == 0))
		return true;

	/* one is +IPCC or +CC, the other is start wth NTP (National trunk prefix) */
	if (minmatch_len <= matched) {
		int index1 = 0;
		int index2 = 0;
		int cc_index = 0;

		/*
		 * International Prefix (IP) is related to current location where to call
		 * Country Code (CC) is related to the SIM card who to call
		 * If you try to call in United State using Korea SIM card,
		 * the number will be 011 82 X XXXXXXXX.
		 * So, when comparing number, just check IP validation and CC and natinal number matching.
		 */

		int n1 = __ctsvc_number_has_ip_and_cc(number1, len1, &index1);
		int n2 = __ctsvc_number_has_ip_and_cc(number2, len2, &index2);

		/*
		 * + (IP) CC XXXXXXXX, 0XXXXXXXX
		 * + (810) 7 XXX XXX XX XX, 8XXX XXX XX XX (Russian)
		 */
		if ((CTSVC_PLUS_IP_CC == n1 || CTSVC_PLUS_CC_ONLY == n1 ||
				CTSVC_IP_CC == n1 || CTSVC_CC_ONLY == n1)
				&& (number2[0] == '0' || (cc && cc[0] == '7' && number2[0] == '8')))
			return true;
		else if ((CTSVC_PLUS_IP_CC == n2 || CTSVC_PLUS_CC_ONLY == n2 ||
					CTSVC_IP_CC == n2 || CTSVC_CC_ONLY == n2)
				&& (number1[0] == '0' || (cc && cc[0] == '7' && number1[0] == '8')))
			return true;
		/*
		 * + IP CC XXXXXXXX, + CC XXXXXXXX  (ex. +001 82  11 1234 5678, +82 10 1234 5678)
		 */
		else if ((CTSVC_PLUS_IP_CC == n1 || CTSVC_IP_CC == n1)
				&& (CTSVC_PLUS_CC_ONLY == n2 || CTSVC_CC_ONLY == n2)) {
			int p = (CTSVC_PLUS_CC_ONLY == n2)?1:0;
			cc_index = __ctsvc_phone_number_has_country_code(&number2[p], len2-p);
			if (0 < cc_index && STRING_EQUAL == strncmp(&number1[index1], &number2[p], cc_index))
				return true;
		}
		else if ((CTSVC_PLUS_IP_CC == n2 || CTSVC_IP_CC == n2)
				&& (CTSVC_PLUS_CC_ONLY == n1 || CTSVC_CC_ONLY == n1)) {
			int p = (CTSVC_PLUS_CC_ONLY == n1)?1:0;
			cc_index = __ctsvc_phone_number_has_country_code(&number1[p], len1-p);
			if (0 < cc_index && STRING_EQUAL == strncmp(&number2[index2], &number1[p], cc_index))
				return true;
		}
		/*
		 * + CC XXXXXXXX, + IP CC XXXXXXXX  (ex. +001 82  10 1234 5678, +82 10 1234 5678)
		 */
		else if ((CTSVC_PLUS_IP_ONLY == n1 || CTSVC_IP_ONLY == n1)
				&& CTSVC_PLUS_ONLY == n2) {
			return true;
		}
		else if ((CTSVC_PLUS_IP_ONLY == n2 || CTSVC_IP_ONLY == n2)
				&& CTSVC_PLUS_ONLY == n1) {
			return true;
		}
	}

	return false;
}

/* When querying _NUMBER_COMPARE_, this function will be called. */
void ctsvc_db_phone_number_equal_callback(sqlite3_context * context,
		int argc, sqlite3_value ** argv)
{
#ifdef _CONTACTS_IPC_SERVER
	char *number1;
	char *number2;

	if (argc < 4) {
		sqlite3_result_int(context, 0);
		CTS_ERR("argc invalid");
		return;
	}

	number1 = (char*)sqlite3_value_text(argv[0]);
	number2 = (char*)sqlite3_value_text(argv[1]);

	sqlite3_result_int(context, __ctsvc_number_compare(number1, number2));
	return;
#endif
}


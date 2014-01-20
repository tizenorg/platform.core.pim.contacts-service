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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_setting.h"
#include "ctsvc_normalize.h"
#include "ctsvc_localize.h"
#include "ctsvc_number_utils.h"

static inline int __ctsvc_phone_number_has_country_code(const char *src)
{
	int ret = 0;
	switch (src[ret++]-'0')
	{
	case 1:
	case 7:
		break;
	case 2:
		switch (src[ret++]-'0')
		{
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
		switch (src[ret++]-'0')
		{
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
		switch (src[ret++]-'0')
		{
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
		switch (src[ret++]-'0')
		{
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
		switch (src[ret++]-'0')
		{
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
		switch (src[ret++]-'0')
		{
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
		switch (src[ret++]-'0')
		{
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

// Number Matching Rule
// refer to the http://www.itu.int/dms_pub/itu-t/opb/sp/T-SP-E.164C-2011-PDF-E.pdf
enum {
	CTSVC_PLUS_ONLY,		// +
	CTSVC_PLUS_IP_ONLY,	// + IP (International prefix)
	CTSVC_PLUS_CC_ONLY,	// + CC
	CTSVC_PLUS_IP_CC,	// + IP CC
	CTSVC_IP_ONLY,	// IP (International prefix)
	CTSVC_CC_ONLY,	// CC
	CTSVC_IP_CC,		// IP CC
	CTSVC_NONE,
};

static int __ctsvc_number_has_ip_and_cc(const char*number, int len, int *index)
{
	bool have_cc = false;
	bool have_plus = false;
	*index = 0;

	switch(number[*index]) {
	case '+':
		(*index)++;
		have_plus = true;
		if (--len <= 0) return CTSVC_PLUS_ONLY;		//'+'
	default:		// not start '+'
		switch(number[*index]) {
		case '0':
			(*index)++;
			if (--len <= 0)
				return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);		// '+0'
			if (len >= 3 && strncmp(&number[*index], "685", 3) == 0)
				return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +0 685
			switch(number[*index]) {
			case '0':
				(*index)++;
				if (--len <= 0)
					return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);		// '+00'
				switch(number[*index]) {
				case '0':
					(*index)++;
					if (--len <= 0)
						return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);		// '+000'
					if (len >= 3 && strncmp(&number[*index], "256", 3) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +000 256
					return CTSVC_NONE;
				case '1':
					(*index)++;
					if (--len <= 0)
						return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);		// '+001'
					if (len >= 2 && strncmp(&number[*index], "62", 2) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +001 62
					if (len >= 2 && strncmp(&number[*index], "65", 2) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +001 65
					if (len >= 2 && strncmp(&number[*index], "66", 2) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +001 66
					if (len >= 2 && strncmp(&number[*index], "82", 2) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +001 82
					if (len >= 3 && strncmp(&number[*index], "592", 3) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +001 592
					if (len >= 3 && strncmp(&number[*index], "852", 3) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +001 852
					if (len >= 3 && strncmp(&number[*index], "855", 3) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +001 855
					if (number[*index] == '1') {
						(*index)++;
						if (--len <= 0)
							return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);		// '+0011'
						if (len >= 2 && strncmp(&number[*index], "66", 2) == 0)
							return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +0011 66
						return CTSVC_NONE;
					}
					return CTSVC_NONE;
				default:
					have_cc = __ctsvc_phone_number_has_country_code(&number[*index]);		// +00 CC
					if (have_cc > 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);
					return CTSVC_NONE;
				}	// end of fourth switch
				return CTSVC_NONE;
			case '1':
				(*index)++;
				if (--len <= 0) return CTSVC_NONE;		// '+01'
				if (number[*index] == '0') {
					(*index)++;
					if (--len <= 0)
						return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);		// '+010'
					if (len >= 2 && strncmp(&number[*index], "81", 2) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +010 81
					return CTSVC_NONE;
				}
				else if (number[*index] == '1') {
					(*index)++;
					if (--len <= 0)
						return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);		// '+011'
					if (strncmp(&number[*index], "1", 1) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +011 1
					if (len >= 3 && strncmp(&number[*index], "680", 3) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +011 680
					if (len >= 3 && strncmp(&number[*index], "691", 3) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +011 691
					if (len >= 3 && strncmp(&number[*index], "692", 3) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +011 692
					return CTSVC_NONE;
				}
				else if (number[*index] == '2') {
					(*index)++;
					if (--len <= 0)
						return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);		// '+012'
					if (len >= 3 && strncmp(&number[*index], "972", 3) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +012 972
					return CTSVC_NONE;
				}
				else if (number[*index] == '3') {
					(*index)++;
					if (--len <= 0)
						return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);		// '+013'
					if (len >= 3 && strncmp(&number[*index], "972", 3) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +013 972
					return CTSVC_NONE;
				}
				else if (number[*index] == '4') {
					(*index)++;
					if (--len <= 0)
						return (have_plus?CTSVC_PLUS_IP_ONLY:CTSVC_IP_ONLY);	// '+014'
					if (len >= 3 && strncmp(&number[*index], "972", 3) == 0)
						return (have_plus?CTSVC_PLUS_IP_CC:CTSVC_IP_CC);	// +014 972
					return CTSVC_NONE;
				}
				return CTSVC_NONE;
			default:
				return CTSVC_NONE;
			}		// end of third switch
			break;		// end of '+0'
		default:
			have_cc = __ctsvc_phone_number_has_country_code(&number[*index]);
			if (have_cc > 0)
				return (have_plus?CTSVC_PLUS_CC_ONLY:CTSVC_CC_ONLY);		// +CC
			return CTSVC_NONE;
		}		// end of second switch
		break;
	}	// end of first switch
	return CTSVC_NONE;
}

// remove first '0' and append network cc
int ctsvc_normalize_number(const char *src, char *dest, int dest_size)
{
	int index;
	int n;
	int d_pos = 0;

	if (NULL == src) {
		CTS_ERR("The parameter(src) is NULL");
		return 0;
	}

	d_pos = strlen(src);
	if (d_pos <= 0)
		return d_pos;

	n = __ctsvc_number_has_ip_and_cc(src, d_pos, &index);

	// 001 82 10 1234 5678 -> +001 82 10 1234 5678
	if (CTSVC_IP_CC == n) {
		dest[0] = '+';
		memcpy(dest+1, src, d_pos+1);
		d_pos++;
		return d_pos;
	}
	else if (CTSVC_PLUS_IP_CC == n) {
		memcpy(dest, src, d_pos+1);
		return d_pos;
	}
	// TODO : append country code

	memcpy(dest, src, d_pos+1);
	return d_pos;
}


// ctsvc_clean_number
// Change fullwidth number, alphabet and '+' to halfwidth number, alphabet and '+'
// Remove character except number and '+'
int ctsvc_clean_number(const char *src, char *dest, int dest_size)
{
	int s_pos = 0;
	int d_pos = 0;
	char temp[dest_size];

	if (NULL == src) {
		CTS_ERR("The parameter(src) is NULL");
		return 0;
	}

	while (src[s_pos] != 0) {
		int char_len;
		char ch = 0x0;
		if (d_pos > dest_size-2) break;

		char_len = ctsvc_check_utf8(src[s_pos]);
		if (char_len == 3) {
			// fullwidth -> halfwidth
			if (src[s_pos] == 0xef) {
				if (src[s_pos+1] == 0xbc) {
					if (0x90 <= src[s_pos+2] && src[s_pos+2] <= 0x99)				// ef bc 90 : '0' ~ ef bc 99 : '9'
						ch = src[s_pos+2] - 0x60;
					else if (0xa1 <= src[s_pos+2] && src[s_pos+2] <= 0xba)			// ef bc a1 : 'A' ~ ef bc ba : 'Z'
						ch = src[s_pos+2] - 0x60;
					else if (0x8b == src[s_pos+2])								// ef bc 8b : '+'
						ch = '+';
					else if (0x8a == src[s_pos+2])								// ef bc 8a : '*'
						ch = '*';
					else if (0x83 == src[s_pos+2])								// ef bc 83 : '#'
						ch = '#';
					else if (0x8c == src[s_pos+2])								// ef bc 8c : ','
						ch = ',';
					else if (0x9b == src[s_pos+2])								// ef bc 9b : ';'
						ch = ';';
				}
				else if (src[s_pos+1] == 0xbd
						&& (0x81 <= src[s_pos+2] && src[s_pos+2] <= 0x9a))		// ef bd 81 : 'a' ~ ef bd 9a : 'z'
					ch = src[s_pos+2] - 0x40;
			}
			else {
				s_pos += char_len;
				continue;
			}
		}
		else if (char_len == 1) {
			if (0x41 <= src[s_pos] && src[s_pos] <= 0x5a)			// 'A' ~ 'Z'
				ch = src[s_pos];
			else if (0x61 <= src[s_pos] && src[s_pos] <= 0x7a)			// 'a' ~ 'z'
				ch = src[s_pos] - 0x20;
			else
				ch = src[s_pos];
		}

		if ('0' <= ch && ch <= '9')
			temp[d_pos++] = ch;
		else if (src[s_pos] == '+' || src[s_pos] == '#'
				|| src[s_pos] == '*' || src[s_pos] == ';' || src[s_pos] == ',')
			temp[d_pos++] = ch;
		s_pos += char_len;
	}
	temp[d_pos] = 0x0;
	memcpy(dest, temp, d_pos+1);

	return d_pos;
}

static int __ctsvc_minmatch_number(const char *src, char *dest, int dest_size, int min_match)
{
	int i;
	int len;
	int d_pos = 0;
	const char *temp_number;

	if ('+' == src[0]) {
		len = __ctsvc_phone_number_has_country_code(&src[1]);
		temp_number = src + len +1;
	}
	else if ('0' == src[0])
		temp_number = src+1;
	else
		temp_number = src;

	len = strlen(temp_number);

	if (0 < len) {
		while(0 <= (len-d_pos-1) && temp_number[len-d_pos-1]
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
		for(i=0; i<len/2;i++) {
			char c;
			c = dest[i];
			dest[i] = dest[len-i-1];
			dest[len-i-1] = c;
		}
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
		CTS_ERR("__ctsvc_minmatch_number(src) failed(%d)", src, ret);
		return ret;
	}
	return CONTACTS_ERROR_NONE;
}

static bool __ctsvc_is_phonenumber_halfwidth(const char* keyword)
{
	int i;
	int len = strlen(keyword);

	// TODO: we should add predicate including '+'
	// TODO: finally, we try to check the number with regular expression.
	for(i=0; i<len; i++) {
		if ((keyword[i] < '0' || keyword[i] > '9') && keyword[i] != '+') {
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
				if (0x90 <= keyword[i+2] && keyword[i+2] <= 0x99)			// ef bc 90 : '0' ~ ef bc 99 : '9'
					continue;
				else if (0x8b == keyword[i+2])								// ef bc 8b : '+'
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
	return ( __ctsvc_is_phonenumber_halfwidth(src) || __ctsvc_is_phonenumber_fullwidth(src) );
}

// numbers are cleaned number or normalized number
static bool __ctsvc_number_compare(const char *number1, const char *number2)
{
	int len1;
	int len2;
	int matched;
	int minmatch_len = ctsvc_get_phonenumber_min_match_digit();

	if (NULL == number1 || NULL == number2 || '\0' == *number1 || '\0' == *number2)
		return false;

	len1 = strlen(number1);
	len2 = strlen(number2);

	for (matched = 0; len1 > 0 && len2 > 0;) {
		if (number1[len1-1] != number2[len2-1])
			break;
		len1--;
		len2--;
		matched++;
	}

	// full match
	if (len1 == 0 && len2 == 0)
		return true;

	// one is substring of the other string
	if (matched >= minmatch_len && (len1 == 0 || len2 == 0))
		return true;

	// one is +IPCC or +CC, the other is start wth NTP
	if (matched >= minmatch_len) {
		int index1;
		int index2;
		int cc_index;
		int n1 = __ctsvc_number_has_ip_and_cc(number1, len1, &index1);
		int n2 = __ctsvc_number_has_ip_and_cc(number2, len2, &index2);

		///////////////////////////////////////////////////
		// + (IP) CC XXXXXXXX, 0XXXXXXXX
		if ((CTSVC_PLUS_IP_CC == n1 || CTSVC_PLUS_CC_ONLY == n1 ||
				CTSVC_IP_CC == n1 || CTSVC_CC_ONLY == n1)
				&& number2[0] == '0')
			return true;
		else if ((CTSVC_PLUS_IP_CC == n2 || CTSVC_PLUS_CC_ONLY == n2 ||
					CTSVC_IP_CC == n2 || CTSVC_CC_ONLY == n2)
				&& number1[0] == '0')
			return true;
		//////////////////////////////////////////////////
		// + IP CC XXXXXXXX, + CC XXXXXXXX  (ex. +001 82  11 1234 5678 , +82 10 1234 5678)
		else if ((CTSVC_PLUS_IP_CC == n1 ||CTSVC_IP_CC == n1)
				&& (CTSVC_PLUS_CC_ONLY == n2 || CTSVC_CC_ONLY == n2)) {
			int p = (CTSVC_PLUS_CC_ONLY == n2)?1:0;
			cc_index = __ctsvc_phone_number_has_country_code(&number2[p]);
			if (cc_index > 0 && strncmp(&number1[index1], &number2[p], cc_index) == 0)
				return true;
		}
		else if ((CTSVC_PLUS_IP_CC == n2 ||CTSVC_IP_CC == n2)
				&& (CTSVC_PLUS_CC_ONLY == n1 || CTSVC_CC_ONLY == n1)) {
			int p = (CTSVC_PLUS_CC_ONLY == n1)?1:0;
			cc_index = __ctsvc_phone_number_has_country_code(&number1[p]);
			if (cc_index > 0 && strncmp(&number2[index2], &number1[p], cc_index) == 0)
				return true;
		}
		///////////////////////////////////////////////////
		// + CC XXXXXXXX, + IP CC XXXXXXXX  (ex. +001 82  10 1234 5678 , +82 10 1234 5678)
		// TODO : Fix (+00 82 10 1234 5678, +82 10 1234 5678 can be matched)
		else if ((CTSVC_PLUS_IP_ONLY == n1 || CTSVC_IP_ONLY == n1)
				&& CTSVC_PLUS_ONLY == n2) {
			cc_index = __ctsvc_phone_number_has_country_code(&number2[1]);
			if (cc_index > 0 && strncmp(&number1[index1], &number2[1], cc_index) == 0)
				return true;
		}
		else if ((CTSVC_PLUS_IP_ONLY == n2 || CTSVC_IP_ONLY == n2)
				&& CTSVC_PLUS_ONLY == n1) {
			cc_index = __ctsvc_phone_number_has_country_code(&number1[1]);
			if (cc_index > 0 && strncmp(&number2[index2], &number1[1], cc_index) == 0)
				return true;
		}
	}

	return false;
}

void ctsvc_db_phone_number_equal_callback(sqlite3_context * context,
		int argc, sqlite3_value ** argv)
{
#ifdef _CONTACTS_IPC_SERVER
	char *number1;
	char *number2;

	if (argc < 4) {
		sqlite3_result_int(context, 0);
		return;
	}

	number1 = (char*)sqlite3_value_text(argv[0]);
	number2 = (char*)sqlite3_value_text(argv[1]);

	sqlite3_result_int(context, __ctsvc_number_compare(number1, number2));
	return;
#endif
}


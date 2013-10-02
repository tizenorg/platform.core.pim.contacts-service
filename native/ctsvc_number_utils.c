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
		else if (s_pos == 0 && src[s_pos] == '+')
			temp[d_pos++] = ch;
		else if (src[s_pos] == '#' || src[s_pos] == '*')
			temp[d_pos++] = ch;
		s_pos += char_len;
	}
	temp[d_pos] = 0x0;
	memcpy(dest, temp, d_pos+1);

	return d_pos;
}

// remove first '0' and append network cc
int ctsvc_normalize_number(const char *src, char *dest, int dest_size)
{
	int d_pos = 0;
	char temp[dest_size];

	if (NULL == src) {
		CTS_ERR("The parameter(src) is NULL");
		return 0;
	}

	d_pos = ctsvc_clean_number(src, temp, dest_size);
	if (d_pos <= 0)
		return d_pos;

	// TODO : append country code
	memcpy(dest, temp, d_pos+1);
	return d_pos;
}

static inline const char* __ctsvc_clean_country_code(const char *src)
{
	int ret = 1;
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
		return src;
	}

	return &src[ret];
}

static int __ctsvc_minmatch_number(const char *src, char *dest, int dest_size, int min_match)
{
	int i;
	int len;
	int d_pos = 0;
	const char *temp_number;

	if ('+' == src[0])
		temp_number = __ctsvc_clean_country_code(src);
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

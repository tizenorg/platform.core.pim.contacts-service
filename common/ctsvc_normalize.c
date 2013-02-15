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

#include <unicode/ulocdata.h>
#include <unicode/ustring.h>
#include <unicode/unorm.h>
#include <unicode/ucol.h>
#include <unicode/uset.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_normalize.h"
#include "ctsvc_localize.h"

#define CTSVC_NORMALIZED_NUMBER_SIZE_MAX 7
#define CTSVC_NORMALIZED_MAX_LEN 1024



static inline bool __ctsvc_check_dirty_number(char digit)
{
	switch (digit)
	{
		case '0' ... '9':
		case 'p':
		case 'w':
		case 'P':
		case 'W':
		case '#':
		case '*':
		case '(':
		case '/':
		case ')':
		case 'N':
		case ',':
		case '.':
		case ';':
			return false;
		case '+': //only first position
		default:
			return true;
	}
}

int ctsvc_clean_number(const char *src, char *dest, int dest_size)
{
	int s_pos=0, d_pos=0, char_type;

	if (NULL == src)
		CTS_ERR("The parameter(src) is NULL");
	else {
		if ('+' == src[s_pos])
			dest[d_pos++] = src[s_pos++];

		while (src[s_pos] != 0)
		{
			if (d_pos >= dest_size-2) break;
			char_type = ctsvc_check_utf8(src[s_pos]);
			if (char_type <= 1) {
				if (__ctsvc_check_dirty_number(src[s_pos])) {
					s_pos++;
					continue;
				}
				dest[d_pos++] = src[s_pos++];
			}
			else
				s_pos += char_type;
		}
	}

	dest[d_pos] = 0;
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

int ctsvc_normalize_number(const char *src, char *dest, int dest_size)
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
				&& d_pos < CTSVC_NORMALIZED_NUMBER_SIZE_MAX) {
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

static int __ctsvc_remove_special_char(const char *src, char *dest, int dest_size)
{
	int s_pos=0, d_pos=0, char_type, src_size;

	if (NULL == src) {
		CTS_ERR("The parameter(src) is NULL");
		dest[d_pos] = '\0';
		return 0;
	}
	src_size = strlen(src);

	while (src[s_pos] != 0) {
		char_type = ctsvc_check_utf8(src[s_pos]);

		if (0 < char_type && char_type < dest_size - d_pos && char_type <= src_size - s_pos) {
			memcpy(dest+d_pos, src+s_pos, char_type);
			d_pos += char_type;
			s_pos += char_type;
		}
		else {
			CTS_ERR("The parameter(src:%s) has invalid character set", src);
			dest[d_pos] = '\0';
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}

	dest[d_pos] = '\0';
	return d_pos;
}

#define array_sizeof(a) (sizeof(a) / sizeof(a[0]))

static inline int __ctsvc_collation_str(const char *src, char **dest)
{
	int32_t size = 0;
	UErrorCode status = U_ZERO_ERROR;
	UChar *tmp_result = NULL;
	UCollator *collator;
	const char *region;

	region = vconf_get_str(VCONFKEY_REGIONFORMAT);
	collator = ucol_open(region, &status);
	RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
			"ucol_open() Failed(%s)", u_errorName(status));

	if (U_FAILURE(status)){
		CTS_ERR("ucol_setAttribute Failed(%s)", u_errorName(status));
		ucol_close(collator);
		return CONTACTS_ERROR_SYSTEM;
	}

	u_strFromUTF8(NULL, 0, &size, src, strlen(src), &status);
	if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR) {
		CTS_ERR("u_strFromUTF8 to get the dest length Failed(%s)", u_errorName(status));
		ucol_close(collator);
		return CONTACTS_ERROR_SYSTEM;
	}
	status = U_ZERO_ERROR;
	tmp_result = calloc(1, sizeof(UChar) * (size + 1));
	u_strFromUTF8(tmp_result, size + 1, NULL, src, -1, &status);
	if (U_FAILURE(status)){
		CTS_ERR("u_strFromUTF8 Failed(%s)", u_errorName(status));
		ucol_close(collator);
		return CONTACTS_ERROR_SYSTEM;
	}

	size = ucol_getSortKey(collator, tmp_result, -1, NULL, 0);
	*dest = calloc(1, sizeof(uint8_t) * (size + 1));
	size = ucol_getSortKey(collator, tmp_result, -1, (uint8_t *)*dest, size + 1);

	ucol_close(collator);

	return CONTACTS_ERROR_NONE;
}

int ctsvc_collation_str(char *src, char **dest)
{
	int ret;
	char temp[strlen(src) + 1];

	ret = __ctsvc_remove_special_char(src, temp, sizeof(temp));
	WARN_IF(ret < CONTACTS_ERROR_NONE, "__ctsvc_remove_special_char() Failed(%d)", ret);

	return __ctsvc_collation_str(temp, dest);
}

static int __ctsvc_normalize_str(const char *src, char *dest, int dest_size)
{
	int type = CTSVC_LANG_OTHERS;
	int32_t size;
	UErrorCode status = 0;
	UChar tmp_result[dest_size*2];
	UChar result[dest_size*2];
	int i = 0;
	int j = 0;
	int str_len = strlen(src);
	int char_len = 0;

	for (i=0;i<str_len;i+=char_len) {
		char char_src[10];
		char_len = ctsvc_check_utf8(src[i]);
		if( char_len < 0 )
		{
			return char_len;
		}

		memcpy(char_src, &src[i], char_len);
		char_src[char_len] = '\0';

		u_strFromUTF8(tmp_result, array_sizeof(tmp_result), NULL, char_src, -1, &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"u_strFromUTF8() Failed(%s)", u_errorName(status));

		u_strToUpper(tmp_result, array_sizeof(tmp_result), tmp_result, -1, NULL, &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"u_strToLower() Failed(%s)", u_errorName(status));

		size = unorm_normalize(tmp_result, -1, UNORM_NFD, 0,
				(UChar *)result, array_sizeof(result), &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"unorm_normalize(%s) Failed(%s)", src, u_errorName(status));

		if (i == 0)
			type = ctsvc_check_language(result);
		ctsvc_extra_normalize(result, size);

		u_strToUTF8(&dest[j], dest_size-j, &size, result, -1, &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"u_strToUTF8() Failed(%s)", u_errorName(status));
		j += size;
		//dest[j++] = 0x7E;
	}
	dest[j]='\0';

	return type;
}

int ctsvc_normalize_str(const char *src, char *dest, int dest_size)
{
	int ret = CONTACTS_ERROR_NONE;
	char temp[dest_size];

	dest[0] = '\0';
	ret = __ctsvc_remove_special_char(src, temp, dest_size);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "__ctsvc_remove_special_char() Failed(%d)", ret);

	ret = __ctsvc_normalize_str(temp, dest, dest_size);
	return ret;
}

int ctsvc_normalize_index(const char *src, char *dest, int dest_size)
{
	int ret = CONTACTS_ERROR_NONE;
	char first_str[10] = {0};
	int length = 0;

	dest[0] = '\0';

	length = ctsvc_check_utf8(src[0]);
	RETVM_IF(length <= 0, CONTACTS_ERROR_INTERNAL, "check_utf8 is failed");

	strncpy(first_str, src, length);
	if (length != strlen(first_str))
		return CONTACTS_ERROR_INVALID_PARAMETER;

	ret = __ctsvc_normalize_str(first_str, dest, dest_size);
	if (dest[0] != '\0') {
		length = ctsvc_check_utf8(dest[0]);
		dest[length] = '\0';
	}
	return ret;
}

static int __ctsvc_get_language_index(const char *locale, char ***index, int *count)
{
	ULocaleData* uld;
	USet *indexChars;
	UErrorCode error = U_ZERO_ERROR;
	int32_t itemCount;
	int32_t j;
	char **temp;

	uld = ulocdata_open(locale, &error);
	indexChars = uset_openEmpty();

	ulocdata_getExemplarSet(uld, indexChars, 0, ULOCDATA_ES_INDEX, &error);
	ulocdata_close(uld);
	CTS_VERBOSE("locale : %s\n", locale);

	if (U_FAILURE(error))
		return 0;

	if (error == U_USING_DEFAULT_WARNING)
		uset_clear(indexChars);

	itemCount = uset_size(indexChars);
	CTS_VERBOSE("Size of USet : %d\n", itemCount);

	temp = (char **)calloc(itemCount, sizeof(char*));
	for (j = 0; j < itemCount; j++){
		UChar ch[2] = {0};
		char dest[10];
		int size;
		ch[0] = uset_charAt(indexChars, j);
		u_strToUTF8(dest, sizeof(dest)-1, &size, ch, -1, &error);
		CTS_VERBOSE("[%d] len : %d, %s\n", j+1, size, dest);
		temp[j] = strdup(dest);
	}
	*count = (int)itemCount;
	*index = (char **)temp;
	uset_clear(indexChars);
	return 0;
}

API int contacts_utils_get_index_characters(char **index_string)
{
	const char *first;
	const char *second;
	int lang_first;
	int lang_second;
	char **first_list = NULL;
	char **second_list = NULL;
	char list[1024] = {0,};
	int i;
	int first_len = 0;
	int second_len = 0;

	RETV_IF(NULL == index_string, CONTACTS_ERROR_INVALID_PARAMETER);
	char temp[5];

	i = 0;
	sprintf(list, "%d", i);
	for (i=1;i<10;i++) {
		sprintf(temp, ";%d", i);
		strcat(list, temp);
	}

	strcat(list, ":");
	first = vconf_get_str(VCONFKEY_LANGSET);
	lang_first = ctsvc_get_language_type(first);
	__ctsvc_get_language_index(first, &first_list, &first_len);
	for (i=0;i<first_len;i++) {
		strcat(list, first_list[i]);
		if (i != (first_len-1))
			strcat(list, ";");
		free(first_list[i]);
	}
	free(first_list);

	second = vconf_get_str(VCONFKEY_CONTACTS_SVC_SECONDARY_LANGUAGE);
	lang_second = ctsvc_get_language_type(second);
	if (lang_first != lang_second)
		__ctsvc_get_language_index(second, &second_list, &second_len);

	if (0 < second_len) {
		strcat(list, ":");
		for (i=0;i<second_len;i++) {
			strcat(list, second_list[i]);
			if (i != (second_len-1))
				strcat(list, ";");
			free(second_list[i]);
		}
	}
	free(second_list);

	strcat(list, ":");
	strcat(list, "#");

	*index_string = strdup(list);
	return CONTACTS_ERROR_NONE;
}

////// contacts_normalized_strstr API should be separated from contacts-service /////////////////////////////////////////////////////////////
#define CTSVC_COMPARE_BETWEEN(left_range, value, right_range) (((left_range) <= (value)) && ((value) <= (right_range)))

static inline bool __ctsvc_is_choseong(const char *src)
{
	unsigned short tmp;

	tmp = (src[1] << 8) | src[2];
	if (((char)0xE1 == src[0] && CTSVC_COMPARE_BETWEEN(0x8480, tmp, 0x859F)) /* korean -Hangul Jamo*/
			|| ((char)0xEA == src[0] && CTSVC_COMPARE_BETWEEN(0xA5A0, tmp, 0xA5BC))) /* korean -Hangul Jamo extended A*/
	{
		return true;
	}
	return false;
}

static inline bool __ctsvc_is_diacritical(const char *src)
{
	unsigned short tmp;

	if (!src || !*src || !*(src+1))
		return false;

	tmp = (src[0] << 8) | src[1];
	if (CTSVC_COMPARE_BETWEEN(0xCC80, tmp, 0xCCBF)
			|| CTSVC_COMPARE_BETWEEN(0xCD80, tmp, 0xCDAF))
	{
		return true;
	}
	return false;
}

static inline bool __ctsvc_compare_unicode(const char *str1, const char *str2, int str2_len)
{
	int k;
	for (k=0; k<str2_len;k++)
		if (!str1[k] || !str2[k] || str1[k] != str2[k])
			return false;
	return true;
}

/**
 * This function compares compares two strings which must have been normalized already.
 * If search_str is included in str, this function return #sCONTACTS_ERROR_NONE. \n
 * The behavior of this function cannot fix because of localization.
 * So, The behavior can be different from each other.
 *
 * @param[in] haystack Base string.
 * @param[in] needle searching string
 * @param[out] len substring length
 * @return a position of the beginning of the substring, Negative value(#cts_error) on error or difference.
 * @par example
 * @code
	ret = contacts_normalized_str(str1, str2, &len);
	if(CONTACTS_ERROR_NONE == ret) {
		snprintf(first, ret+1, "%s", item_data->display);
		snprintf(middle, len+1, "%s", item_data->display + ret);
		printf("%s -> %s, %s, %s", item_data->display, first, middle, item_data->display + ret + len);
	} else
		printf("str1 doesn't has str2");
 * @endcode
 */
API int contacts_normalized_strstr(const char *haystack,
		const char *needle, int *len)
{
	int i, j, wind, h_len, n_len;
	int first_needle_len;
	int equal_index;
	int equal_length;
	int equal_wind = 0;
	bool counted = false;
	RETVM_IF(NULL == haystack, -1, "The parameter(haystack) is NULL");
	RETVM_IF(NULL == needle, -1, "The parameter(needle) is NULL");
	CTS_VERBOSE("haystack = %s, needle = %s", haystack, needle);

	h_len = 1;
	n_len = 1;
	equal_index = 0;
	first_needle_len = ctsvc_check_utf8(needle[0]);
	for (i=0, j=0;i<strlen(haystack);i = wind?wind:(i+h_len)) {
		if (equal_wind) {
			equal_index = equal_wind;
			counted = false;
		}
		wind = 0;
		equal_length = 0;
		equal_wind = 0;
		for (j=0;j<strlen(needle);) {
			bool equal;
			h_len = ctsvc_check_utf8(haystack[i]);

			if (h_len == 1 && haystack[i] == 0x7E) {		//skip seperator
				counted = false;
				i+=h_len;
				continue;
			}

			n_len = ctsvc_check_utf8(needle[j]);
			if (n_len == 1 && needle[j] == 0x7E) {		//skip seperator
				j++;
				continue;
			}

			if (wind == 0 && j && 0 < i) {
				if (h_len == first_needle_len && __ctsvc_compare_unicode(&haystack[i], needle, first_needle_len)
						&& !__ctsvc_is_diacritical(&haystack[i])) {
					unsigned short tmp;

					tmp = (haystack[i+1] << 8) | haystack[i+2];
					if (!counted) {
						wind = i;
						equal_wind = equal_index + equal_length;
					}
				}
			}

			if ((2 == h_len && __ctsvc_is_diacritical(&haystack[i]))
					&& (2 != n_len || !__ctsvc_is_diacritical(&needle[j]))) {
				if (j == 0) {
					if (counted)
						equal_index++;
					else {
						equal_index += h_len;
						counted = true;
					}
				}
				else if (!counted) {
					equal_length += h_len;
					counted = true;
				}
				else if (counted)
					equal_length++;
				i+=h_len;
				continue;
			}

			if (h_len != n_len) {
				if (!counted) {
					equal_index += (equal_length + h_len);
					counted = true;
				}
				break;
			}

			if (3 == n_len && __ctsvc_is_choseong(&needle[j]) && !(__ctsvc_is_choseong(&haystack[i]))) {
				if (j < (n_len+1) || !__ctsvc_is_choseong(&needle[j-n_len-1])) {		// skip 강나 search by 가나
					if (!counted) {
						equal_index += (equal_length + h_len);
						counted = true;
					}
					break;
				}
				else {
					if (j == 0) {
						if (!counted) {
							equal_index += h_len;
							counted = true;
						}
					}
					else if (!counted) {
						equal_length += h_len;
						counted = true;
					}
					i+=h_len;
					continue;
				}
			}

			equal = __ctsvc_compare_unicode(&haystack[i], &needle[j], n_len);

			if (equal) {
				if (!counted) {
					equal_length += h_len;
					counted = true;
				}
				else if (2 == n_len && __ctsvc_is_diacritical(&needle[j]))
					equal_length ++;
				j += n_len;
				i+=h_len;
				continue;
			}
			else {
				if (!counted) {
					equal_index += (equal_length + h_len);
					counted = true;
				}
				else {
					if (2 == n_len && __ctsvc_is_diacritical(&needle[j]))
						equal_index += (equal_length + 1);
					else
						equal_index += equal_length;
				}
				break;
			}
		}

		if ('\0' == needle[j]) {
			if ('\0' != haystack[i]) {
				h_len = ctsvc_check_utf8(haystack[i]);
				if(h_len == 2 && __ctsvc_is_diacritical(&haystack[i]))
					equal_length++;
			}
			*len = equal_length;
			return equal_index;
		}
	}

	CTS_VERBOSE("NOT match");
	return -1;
}

/**
 * This function make searchable string.
 * The string can use at contacts_svc_normalized_strstr().
 *
 * @param[in] src the string to convert
 * @param[out] dest The pointer to get normalized string.
 * @param[out] dest_len the size of dest.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 * @par example
 * @code
	char normalized_str[512];
	const char *name = "Test"

	ret = contacts_normalize_str(name, normalized_str, sizeof(normalized_str));

	if(CONTACTS_ERROR_NONE != ret)
		printf("Error : contacts_svc_normalize_str() Failed(%d)", ret);
	else
		printf("original string is %s, normalized string is %s", name, normalized_str);
 * @endcode
 */
API int contacts_normalize_str(const char *src, char *dest, const int dest_len)
{
	int ret;
	RETV_IF(NULL == dest, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(dest_len <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "dest_len(%d) is Invalid", dest_len);

	ret = ctsvc_normalize_str(src, dest, dest_len);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_normalize_str() Failed(%d)", ret);

	return CONTACTS_ERROR_NONE;
}


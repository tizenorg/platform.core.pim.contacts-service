/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjae Shin <yj99.shin@samsung.com>
 *          Donghee Ye <donghee.ye@samsung.com>
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

#include "cts-internal.h"
#include "cts-normalize.h"
#include "cts-socket.h"
#include "cts-pthread.h"
#include "cts-utils.h"

static int (*extra_normalize_fn)(char dest[][CTS_SQL_MAX_LEN]);

static inline int check_utf8(char c)
{
	if (c < 128)
		return 1;
	else if ((c & (char)0xe0) == (char)0xc0)
		return 2;
	else if ((c & (char)0xf0) == (char)0xe0)
		return 3;
	else if ((c & (char)0xf8) == (char)0xf0)
		return 4;
	else if ((c & (char)0xfc) == (char)0xf8)
		return 5;
	else if ((c & (char)0xfe) == (char)0xfc)
		return 6;
	else
		return CTS_ERR_FAIL;
}

static inline bool check_dirty_number(char digit)
{
	switch (digit)
	{
	case '0' ... '9':
	case 'p':
	case 'w':
	case 'P':
	case 'W':
		return false;
	case '+': //only first position
	default:
		return true;
	}
}

int cts_clean_number(const char *src, char *dest, int dest_size)
{
	int s_pos=0, d_pos=0, char_type;

	if (NULL == src)
		ERR("The parameter(src) is NULL");
	else
	{
		if ('+' == src[s_pos])
			dest[d_pos++] = src[s_pos++];

		while (src[s_pos] != 0)
		{
			if (d_pos >= dest_size-2) break;
			char_type = check_utf8(src[s_pos]);
			if (char_type <= 1) {
				if (check_dirty_number(src[s_pos])) {
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

static int cts_remove_special_char(const char *src, char *dest, int dest_size)
{
	int s_pos=0, d_pos=0, char_type, src_size;

	if (NULL == src) {
		ERR("The parameter(src) is NULL");
		dest[d_pos] = '\0';
		return 0;
	}
	src_size = strlen(src);

	while (src[s_pos] != 0)
	{
		char_type = check_utf8(src[s_pos]);

		if (0 < char_type && char_type < dest_size - d_pos && char_type <= src_size - s_pos) {
			memcpy(dest+d_pos, src+s_pos, char_type);
			d_pos += char_type;
			s_pos += char_type;
		}
		else {
			ERR("The parameter(src:%s) has invalid character set", src);
			dest[d_pos] = '\0';
			return CTS_ERR_ARG_INVALID;
		}
	}

	dest[d_pos] = '\0';
	return d_pos;
}

int cts_normalize_str(const char *src, char *dest, int dest_size)
{
	int ret;
	ret = cts_remove_special_char(src, dest, dest_size);
	retvm_if(ret < CTS_SUCCESS, ret, "cts_remove_special_char() Failed(%d)", ret);
	ret = CTS_SUCCESS;

	cts_mutex_lock(CTS_MUTEX_SOCKET_FD);
	ret = cts_request_normalize_str(dest, dest, dest_size);
	cts_mutex_unlock(CTS_MUTEX_SOCKET_FD);

	return ret;
}

void cts_set_extra_normalize_fn(int (*fn)(char dest[][CTS_SQL_MAX_LEN]))
{
	extra_normalize_fn = fn;
}

int cts_normalize_name(cts_name *src,
		char dest[][CTS_SQL_MAX_LEN], bool is_display)
{
	int ret;
	if (is_display) {
		ret = cts_remove_special_char(src->display, dest[CTS_NN_FIRST],
				sizeof(dest[CTS_NN_FIRST]));
		warn_if(ret < CTS_SUCCESS, "cts_remove_special_char() Failed(%d)", ret);
		snprintf(dest[CTS_NN_SORTKEY], sizeof(dest[CTS_NN_SORTKEY]), "%s", dest[CTS_NN_FIRST]);
		ret = CTS_SUCCESS;
	}
	else {
		ret = cts_remove_special_char(src->first, dest[CTS_NN_FIRST],
				sizeof(dest[CTS_NN_FIRST]));
		warn_if(ret < CTS_SUCCESS, "cts_remove_special_char() Failed(%d)", ret);
		ret = CTS_SUCCESS;

		ret = cts_remove_special_char(src->last, dest[CTS_NN_LAST],
				sizeof(dest[CTS_NN_LAST]));
		warn_if(ret < CTS_SUCCESS, "cts_remove_special_char() Failed(%d)", ret);
		ret = CTS_SUCCESS;

		if (!*dest[CTS_NN_LAST])
			snprintf(dest[CTS_NN_SORTKEY], sizeof(dest[CTS_NN_SORTKEY]), "%s", dest[CTS_NN_FIRST]);
		else if (!*dest[CTS_NN_FIRST])
			snprintf(dest[CTS_NN_SORTKEY], sizeof(dest[CTS_NN_SORTKEY]), "%s", dest[CTS_NN_LAST]);
		else if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_DISPLAY))
			snprintf(dest[CTS_NN_SORTKEY], sizeof(dest[CTS_NN_SORTKEY]), "%s %s", dest[CTS_NN_FIRST], dest[CTS_NN_LAST]);
		else
			snprintf(dest[CTS_NN_SORTKEY], sizeof(dest[CTS_NN_SORTKEY]), "%s, %s", dest[CTS_NN_LAST], dest[CTS_NN_FIRST]);
	}

	if (extra_normalize_fn)
		ret = extra_normalize_fn(dest);
	else {
		cts_mutex_lock(CTS_MUTEX_SOCKET_FD);
		ret = cts_request_normalize_name(dest);
		cts_mutex_unlock(CTS_MUTEX_SOCKET_FD);
	}

	return ret;
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

 	ret = contacts_svc_normalize_str(name, normalized_str, sizeof(normalized_str));

 	if(CTS_SUCCESS != ret)
 		printf("Error : contacts_svc_normalize_str() Failed(%d)", ret);
 	else
 		printf("original string is %s, normalized string is %s", name, normalized_str);
 * @endcode
 */
API int contacts_svc_normalize_str(const char *src, char *dest, const int dest_len)
{
	int ret;
	retv_if(NULL == dest, CTS_ERR_ARG_NULL);
	retvm_if(dest_len <= 0, CTS_ERR_ARG_INVALID, "dest_len(%d) is Invalid", dest_len);

	ret = cts_normalize_str(src, dest, dest_len);
	retvm_if(ret < CTS_SUCCESS, ret, "cts_normalize_str() Failed(%d)", ret);

	return CTS_SUCCESS;
}

static inline bool is_choseong(const char *src)
{
	unsigned short tmp;

	tmp = (src[1] << 8) | src[2];
	if (((char)0xE1 == src[0] && CTS_COMPARE_BETWEEN(0x8480, tmp, 0x859F)) /* korean -Hangul Jamo*/
			|| ((char)0xEA == src[0] && CTS_COMPARE_BETWEEN(0xA5A0, tmp, 0xA5BC))) /* korean -Hangul Jamo extended A*/
	{
		return true;
	}
	return false;
}

static inline bool is_jungseong(const char *src)
{
	unsigned short tmp;

	tmp = (src[1] << 8) | src[2];
	if (((char)0xE1 == src[0] && CTS_COMPARE_BETWEEN(0x85A0, tmp, 0x86A7))/* korean -Hangul Jamo*/
			|| ((char)0xED == src[0] && CTS_COMPARE_BETWEEN(0x9EB0, tmp, 0x9F86)))/* korean -Hangul Jamo extended B */
	{
		return true;
	}
	return false;
}

static inline bool is_diacritical(const char *src)
{
	unsigned short tmp;

	if (!src || !*src || !*(src+1))
		return false;

	tmp = (src[0] << 8) | src[1];
	if (CTS_COMPARE_BETWEEN(0xCC80, tmp, 0xCCBF)
			|| CTS_COMPARE_BETWEEN(0xCD80, tmp, 0xCDAF))
	{
		return true;
	}
	return false;
}

static inline bool compare_unicode(const char *str1, const char *str2, int str2_len)
{
	int k;
	for (k=0; k<str2_len;k++)
		if (!str1[k] || !str2[k] || str1[k] != str2[k])
			return false;
	return true;
}

/**
 * This function compares compares two strings which must have been normalized already.
 * If search_str is included in str, this function return #CTS_SUCCESS. \n
 * The behavior of this function cannot fix because of localization.
 * So, The behavior can be different from each other.
 *
 * @param[in] haystack Base string.
 * @param[in] needle searching string
 * @param[out] len substring length
 * @return a position of the beginning of the substring, Negative value(#cts_error) on error or difference.
 * @par example
 * @code
 	ret = contacts_svc_compare_normalized_str(str1, str2, &len);
 	if(CTS_SUCCESS == ret) {
		snprintf(first, ret+1, "%s", item_data->display);
		snprintf(middle, len+1, "%s", item_data->display + ret);
		printf("%s -> %s, %s, %s", item_data->display, first, middle, item_data->display + ret + len);
 	} else
 		printf("str1 doesn't has str2");
 * @endcode
 */
API int contacts_svc_normalized_strstr(const char *haystack,
		const char *needle, int *len)
{
	int i, j, wind, h_len, n_len;
	int first_needle_len;
	int equal_index;
	int equal_length;
	int equal_wind = 0;
	bool counted = false;
	retvm_if(NULL == haystack, -1, "The parameter(haystack) is NULL");
	retvm_if(NULL == needle, -1, "The parameter(needle) is NULL");
	CTS_DBG("haystack = %s, needle = %s", haystack, needle);

	h_len = 1;
	n_len = 1;
	equal_index = 0;
	first_needle_len = check_utf8(needle[0]);
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
			h_len = check_utf8(haystack[i]);

			if (h_len == 1 && haystack[i] == 0x1) {		//skip seperator
				counted = false;
				i+=h_len;
				continue;
			}

			n_len = check_utf8(needle[j]);
			if (n_len == 1 && needle[j] == 0x1) {		//skip seperator
				j++;
				continue;
			}

			if (wind == 0 && j && 0 < i) {
				if (h_len == first_needle_len && compare_unicode(&haystack[i], needle, first_needle_len)
						&& !is_diacritical(&haystack[i])) {
					unsigned short tmp;

					tmp = (haystack[i+1] << 8) | haystack[i+2];
					if (!counted) {
						wind = i;
						equal_wind = equal_index + equal_length;
					}
				}
			}

			if ((2 == h_len && is_diacritical(&haystack[i]))
					&& (2 != n_len || !is_diacritical(&needle[j]))) {
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

			if (3 == n_len && is_choseong(&needle[j]) && !(is_choseong(&haystack[i]))) {
				if (j < (n_len+1) || !is_choseong(&needle[j-n_len-1])) {		// skip 강나 search by 가나
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

			equal = compare_unicode(&haystack[i], &needle[j], n_len);

			if (equal) {
				if (!counted) {
					equal_length += h_len;
					counted = true;
				}
				else if (2 == n_len && is_diacritical(&needle[j]))
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
					if (2 == n_len && is_diacritical(&needle[j]))
						equal_index += (equal_length + 1);
					else
						equal_index += equal_length;
				}
				break;
			}
		}

		if ('\0' == needle[j]) {
			if ('\0' != haystack[i]) {
				h_len = check_utf8(haystack[i]);
				if(h_len == 2 && is_diacritical(&haystack[i]))
					equal_length++;
			}
			*len = equal_length;
			return equal_index;
		}
	}

	CTS_DBG("NOT match");
	return -1;
}

static inline const char* cts_clean_country_code(const char *src)
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
			ERR("The parameter(src:%s) has invalid character set", src);
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
			ERR("The parameter(src:%s) has invalid character set", src);
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
			ERR("The parameter(src:%s) has invalid character set", src);
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
			ERR("The parameter(src:%s) has invalid character set", src);
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
			ERR("The parameter(src:%s) has invalid character set", src);
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
			ERR("The parameter(src:%s) has invalid character set", src);
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
			ERR("The parameter(src:%s) has invalid character set", src);
		}
		break;
	case 0:
	default:
		ERR("The parameter(src:%s) has invalid character set", src);
		return src;
	}

	return &src[ret];
}

const char* cts_normalize_number(const char *src)
{
	const char *normalized_number;

	if ('+' == src[0])
		normalized_number = cts_clean_country_code(src);
	else if ('0' == src[0])
		normalized_number = src+1;
	else
		normalized_number = src;

	CTS_DBG("src = %s, normalized = %s", src, normalized_number);

	return normalized_number;
}

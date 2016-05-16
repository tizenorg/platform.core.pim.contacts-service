/*
 * Contacts Service
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved.
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

#define _GNU_SOURCE
#include <string.h>
#include <ctype.h>
#include <unicode/ulocdata.h>
#include <unicode/ustring.h>
#include <unicode/unorm.h>
#include <unicode/ucol.h>
#include <unicode/uset.h>

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_struct.h"
#include "ctsvc_record.h"
#include "ctsvc_view.h"
#include "ctsvc_normalize.h"
#include "ctsvc_localize.h"
#include "ctsvc_localize_utils.h"
#include "ctsvc_localize_ch.h"
#include "ctsvc_localize_jp.h"

#define CTSVC_COMBINING_DIACRITICAL_MARKS_START 0x0300
#define CTSVC_COMBINING_DIACRITICAL_MARKS_END	0x036f
#define CTSVC_SNIPPET_DEFAULT_START_MATCH "["
#define CTSVC_SNIPPET_DEFAULT_END_MATCH "]"

static inline bool __ctsvc_is_choseong(const char *src)
{
	unsigned short tmp;

	tmp = (src[1] << 8) | src[2];
	if (((char)0xE1 == src[0] && CTSVC_COMPARE_BETWEEN(0x8480, tmp, 0x859F)) /* korean -Hangul Jamo*/
			|| ((char)0xEA == src[0] && CTSVC_COMPARE_BETWEEN(0xA5A0, tmp, 0xA5BC))) /* korean -Hangul Jamo extended A*/
		return true;

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
		return true;

	return false;
}

static inline bool __ctsvc_compare_utf8(const char *str1, const char *str2, int str2_len)
{
	int k;
	for (k = 0; k < str2_len; k++)
		if (!str1[k] || !str2[k] || str1[k] != str2[k])
			return false;
	return true;
}

static int __ctsvc_copy_and_remove_special_char(const char *src, char *dest, int dest_size)
{
	int s_pos = 0, d_pos = 0, char_type, src_size;

	if (NULL == src) {
		ERR("The parameter(src) is NULL");
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
		} else {
			ERR("The parameter(src:%s) has invalid character set", src);
			dest[d_pos] = '\0';
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}

	dest[d_pos] = '\0';
	return d_pos;
}

#define array_sizeof(a) (sizeof(a) / sizeof(a[0]))

static int __ctsvc_normalize_str_to_unicode(const char *src, int src_size, UChar *dest, int dest_size)
{
	int ret;
	int32_t size = dest_size;
	UErrorCode status = 0;
	UChar tmp_result[dest_size];

	u_strFromUTF8(tmp_result, dest_size, &size, src, src_size, &status);

	if (U_FAILURE(status)) {
		ERR("u_strFromUTF8() Failed(%s)", u_errorName(status));
		return CONTACTS_ERROR_SYSTEM;
	}

	u_strToUpper(tmp_result, dest_size, tmp_result, -1, NULL, &status);
	if (U_FAILURE(status)) {
		ERR("u_strToUpper() Failed(%s)", u_errorName(status));
		return CONTACTS_ERROR_SYSTEM;
	}

	size = unorm_normalize(tmp_result, -1, UNORM_NFD, 0,
			(UChar *)dest, dest_size, &status);
	if (U_FAILURE(status)) {
		ERR("unorm_normalize() Failed(%s)", u_errorName(status));
		return CONTACTS_ERROR_SYSTEM;
	}

	ret = ctsvc_check_language(dest);
	ctsvc_extra_normalize(dest, size);

	dest[size] = 0x00;

	return ret;
}

#define SMALL_BUFFER_SIZE 10

static bool __ctsvc_compare_pinyin_letter(const char *haystack, int haystack_lang, const char *needle, int needle_lang, int *h_len, int *n_len)
{
	pinyin_name_s *pinyinname = NULL;
	int size, ret = false;
	int len, i, j, k;
	char temp_needle[strlen(needle) + 1];
	char temp[SMALL_BUFFER_SIZE];
	bool match = false, initial_match = false;

	if (haystack_lang != CTSVC_LANG_CHINESE || needle_lang != CTSVC_LANG_ENGLISH)
		return false;

	for (i = 0, k = 0; i < sizeof(temp_needle); i++) {
		if (isupper(needle[i]))
			temp_needle[i] = tolower(needle[i]);
		else
			temp_needle[i] = needle[i];
	}

	for (i = 0, j = 0; i < strlen(haystack) && j < strlen(temp_needle) ; i += len) {
		len = ctsvc_check_utf8(haystack[i]);
		if (len < 0)
			return false;
		memcpy(temp, haystack + i, len);
		temp[len] = '\0';

		ret = ctsvc_convert_chinese_to_pinyin(temp, &pinyinname, &size);
		if (ret != CONTACTS_ERROR_NONE) {
			ERR("ctsvc_convert_chinese_to_pinyin Fail(%d)", ret);
			return false;
		}

		for (k = 0; k < size; k++) {
			if (!initial_match &&
					strlen(pinyinname[k].pinyin_name) <= strlen(temp_needle + j) &&
					strncmp(pinyinname[k].pinyin_name, temp_needle + j, strlen(pinyinname[k].pinyin_name)) == 0) {
				match = true;
				j += strlen(pinyinname[k].pinyin_name);
				break;

			} else if (!initial_match &&
					strlen(pinyinname[k].pinyin_name) > strlen(temp_needle + j) &&
					strncmp(pinyinname[k].pinyin_name, temp_needle + j, strlen(temp_needle + j)) == 0) {
				match = true;
				j += strlen(temp_needle + j);
				break;

			} else if (pinyinname[k].pinyin_initial[0] ==  temp_needle[j]) {
				initial_match = true;
				match = true;
				j++;
				break;
			} else {
				match = false;
			}
		}
		ctsvc_pinyin_free(pinyinname, size);

		if (match == false)
			break;

	}

	if (match) {
		*h_len = i;
		*n_len = j;
	}

	return match;
}

static bool __ctsvc_compare_unicode_letter(const UChar* haystack, int haystack_lang, const UChar *needle, int needle_lang)
{
	int i, j;
	bool ret = false;

	switch (haystack_lang) {
	case CTSVC_LANG_NUMBER:
	case CTSVC_LANG_OTHERS:
		{
			if (haystack_lang == needle_lang) {
				for (i = 0, j = 0; i < u_strlen(haystack) && j < u_strlen(needle);) {
					if (haystack[i] == needle[j]) {
						ret = true;
					} else {
						ret = false;
						break;
					}
					i++;
					j++;
				}
			}
			return ret;
		}
	case CTSVC_LANG_ENGLISH:
		{
			switch (needle_lang) {
			case CTSVC_LANG_ENGLISH:
				for (i = 0, j = 0; i < u_strlen(haystack) && j < u_strlen(needle);) {
					if (CTSVC_COMPARE_BETWEEN(CTSVC_COMBINING_DIACRITICAL_MARKS_START,
							haystack[i], CTSVC_COMBINING_DIACRITICAL_MARKS_END)) {
						i++;
						continue;
					}
					if (CTSVC_COMPARE_BETWEEN(CTSVC_COMBINING_DIACRITICAL_MARKS_START,
							needle[j], CTSVC_COMBINING_DIACRITICAL_MARKS_END)) {
						j++;
						continue;
					}

					if (haystack[i] == needle[j]) {
						ret = true;
					} else {
						ret = false;
						break;
					}

					i++;
					j++;
				}
				return ret;
			default:
				return false;
			}
		}
		break;
	case CTSVC_LANG_KOREAN:
		{
			if (needle_lang != CTSVC_LANG_KOREAN)
				break;

			if (u_strlen(needle) == 1
					&& CTSVC_COMPARE_BETWEEN(0x3130, needle[0], 0x314e)
					&& haystack[0] == needle[0])
				return true;

			for (i = 0, j = 0; i < u_strlen(haystack) && j < u_strlen(needle);) {
				if (haystack[i] == needle[j]) {
					ret = true;
				} else {
					ret = false;
					break;
				}
				i++;
				j++;
			}
			return ret;
		}
		break;
	case CTSVC_LANG_JAPANESE:
		{
			if (needle_lang != CTSVC_LANG_JAPANESE)
				break;

			if (haystack[0] == needle[0]) {
				ret = true;
			} else {
				UChar temp_haystack[2] = {0x00,};
				UChar temp_needle[2] = {0x00,};
				UChar hiragana_haystack[2] = {0x00,};
				UChar hiragana_needle[2] = {0x00,};
				temp_haystack[0] = haystack[0];
				temp_needle[0] = needle[0];

				ctsvc_convert_japanese_to_hiragana_unicode(temp_haystack, hiragana_haystack, 2);

				ctsvc_convert_japanese_to_hiragana_unicode(temp_needle, hiragana_needle,  2);

				if (hiragana_haystack[0] == hiragana_needle[0])
					ret = true;
			}
			return ret;
		}
	case CTSVC_LANG_CHINESE:
		{
			if (needle_lang == haystack_lang && haystack[0] == needle[0])
				ret = true;
		}
		return ret;
	}

	return false;
}

int ctsvc_utils_string_strstr(const char *haystack, const char *needle, int *len)
{
	int h_len = 0, h_len_temp, n_len, i, j;
	UChar haystack_letter[SMALL_BUFFER_SIZE];
	UChar needle_letter[SMALL_BUFFER_SIZE];
	UChar first_needle_letter[SMALL_BUFFER_SIZE];
	int haystack_letter_lang;
	int needle_letter_lang;
	int first_needle_letter_lang;

	bool matching = false;
	int match_len = 0;
	int match_start = -1;

	RETVM_IF(NULL == haystack, -1, "The parameter(haystack) is NULL");
	RETVM_IF(NULL == needle, -1, "The parameter(needle) is NULL");
	SECURE_SLOGD("haystack = %s, needle = %s", haystack, needle);

	char temp_haystack[strlen(haystack) + 1];
	char temp_needle[strlen(needle) + 1];

	*len = 0;

	__ctsvc_copy_and_remove_special_char(haystack, temp_haystack, strlen(haystack) + 1);
	__ctsvc_copy_and_remove_special_char(needle, temp_needle, strlen(needle) + 1);

	n_len = ctsvc_check_utf8(temp_needle[0]);

	first_needle_letter_lang = __ctsvc_normalize_str_to_unicode(temp_needle, n_len, first_needle_letter, SMALL_BUFFER_SIZE);
	RETVM_IF(first_needle_letter_lang < CONTACTS_ERROR_NONE , -1, "The __ctsvc_normalize_str_to_unicode failed(%d)", first_needle_letter_lang);

	for (i = 0, j = 0; i < strlen(temp_haystack) && j < strlen(temp_needle); i += h_len) {
		h_len_temp = ctsvc_check_utf8(temp_haystack[i]);

		haystack_letter_lang = __ctsvc_normalize_str_to_unicode(temp_haystack + i, h_len_temp, haystack_letter, SMALL_BUFFER_SIZE);
		RETVM_IF(haystack_letter_lang < CONTACTS_ERROR_NONE , -1, "The __ctsvc_normalize_str_to_unicode failed(%d)", haystack_letter_lang);

		if (matching == false) {
			if (__ctsvc_compare_unicode_letter(haystack_letter, haystack_letter_lang, first_needle_letter, first_needle_letter_lang)
					|| __ctsvc_compare_pinyin_letter(temp_haystack + i, haystack_letter_lang, temp_needle + j, first_needle_letter_lang, &h_len_temp, &n_len)) {
				matching = true;
				j += n_len;
				match_start = i;
				match_len = h_len_temp;

				if (temp_needle[j] == '\0') {
					*len = match_len;
					return match_start;
				}
			}
			h_len = h_len_temp;
			continue;
		} else {
			n_len = ctsvc_check_utf8(temp_needle[j]);

			needle_letter_lang = __ctsvc_normalize_str_to_unicode(temp_needle + j, n_len, needle_letter, SMALL_BUFFER_SIZE);
			RETVM_IF(needle_letter_lang < CONTACTS_ERROR_NONE , -1, "The __ctsvc_normalize_str_to_unicode failed(%d)", needle_letter_lang);

			if (__ctsvc_compare_unicode_letter(haystack_letter, haystack_letter_lang, needle_letter, needle_letter_lang)) {
				j += n_len;
				match_len += h_len_temp;

				if (temp_needle[j] == '\0') {
					*len = match_len;
					return match_start;
				}
				h_len = h_len_temp;
				continue;
			} else {
				j = 0;
				matching = false;
				match_start = -1;
				match_len = 0;
				i -= h_len;
			}
		}

	}

	DBG("NOT match");
	return -1;
}

static bool _get_modified_number(char *temp, char *keyword, int *out_len_keyword,
		int *out_len_offset)
{
	RETV_IF(NULL == temp, false);
	RETV_IF(NULL == keyword, false);
	RETV_IF(NULL == out_len_keyword, false);
	RETV_IF(NULL == out_len_offset, false);

	char *start_needle = NULL;
	char *cursor_number = temp;
	char *cursor_keyword = keyword;
	bool is_start = false;
	int len_keyword = 0;

	while (1) {
		if ('\0' == *cursor_keyword)
			break;
		if ('\0' == *cursor_number) {
			if (true == is_start) /* ended even if keyword is remained */
				is_start = false;
			break;
		}
		if (*cursor_number < '0' || '9' < *cursor_number) {
			if (true == is_start)
				len_keyword++;
			cursor_number++;
			continue;
		}
		if (*cursor_keyword < '0' || '9' < *cursor_keyword) {
			cursor_keyword++;
			continue;
		}
		if (*cursor_keyword != *cursor_number) {
			if (true == is_start) {
				is_start = false;
				cursor_keyword = keyword;
				len_keyword = 0;
			} else {
				cursor_number++;
			}
			continue;
		}
		if (false == is_start)
			start_needle = cursor_number;
		is_start = true;
		len_keyword++;
		cursor_number++;
		cursor_keyword++;
	}
	if (false == is_start) { /* false to search */
		return false;
	}
	*out_len_keyword = len_keyword;
	*out_len_offset = strlen(temp) - strlen(start_needle);

	return true;
}

static char *_strchr_with_nth(char *s, int c, int nth)
{
	char *cursor = s;
	while (*cursor) {
		if (*cursor == c)
			nth--;

		if (nth < 0)
			break;

		cursor++;
	}
	return cursor;
}

static char *_strrchr_with_nth(char *ori, char *s, int c, int nth)
{
	char *cursor = s;
	while (*cursor) {
		if (cursor <= ori) {
			ERR("over");
			return ori;
		}

		if (*cursor == c)
			nth--;

		if (nth < 0)
			break;

		cursor--;
	}
	return cursor + 1;
}

char *ctsvc_utils_get_modified_str(char *temp, bool is_snippet, const char *keyword,
		const char *start_match, const char *end_match, int token_number)
{
	char *mod_temp = NULL;

	RETV_IF(NULL == temp, NULL);
	RETV_IF('\0' == *temp, NULL);
	RETV_IF(false == is_snippet, NULL);
	RETV_IF(NULL == keyword, NULL);
	RETV_IF('\0' == *keyword, NULL);

	bool is_modified = true;
	int len_keyword = 0;
	int len_offset = 0;
	int len_full = 0;
	int len_print = 0;
	char *pos_start = NULL;

	char *pos_at = strstr(temp, "@");
	if (NULL == pos_at) {
		ERR("Invalid value[%s]", temp);
		return NULL;
	}
	DBG("[%s]", temp);
	*pos_at = '\0';
	char datatype_str[12] = {0};
	snprintf(datatype_str, sizeof(datatype_str), "%s", temp);
	temp = pos_at + 1;
	DBG("[%s]", temp);
	switch (atoi(datatype_str)) {
	case CONTACTS_DATA_TYPE_NAME:
		DBG("NAME");
		len_offset = ctsvc_utils_string_strstr(temp, keyword, &len_keyword);
		if (len_offset < 0) {
			is_modified = false;
			break;
		}
		break;

	case CONTACTS_DATA_TYPE_NUMBER:
		DBG("NUMBER");
		is_modified = _get_modified_number(temp, (char *)keyword, &len_keyword, &len_offset);
		break;

	default:
		DBG("DATA");
		pos_start = strcasestr(temp, keyword);
		if (NULL == pos_start)
			break;

		len_keyword = strlen(keyword);
		len_offset = strlen(temp) - strlen(pos_start);;
		break;
	}

	if (false == is_modified)
		return NULL;

	len_full = strlen(temp) + strlen(start_match) + strlen(end_match) + 1;

	mod_temp = calloc(len_full, sizeof(char));
	if (NULL == mod_temp) {
		ERR("calloc() Fail");
		return NULL;
	}

	char *keyword_start = NULL;
	snprintf(mod_temp, len_offset + 1, "%s", temp);
	len_print = len_offset;
	len_print += snprintf(mod_temp + len_print, len_full - len_print,
			"%s", start_match);
	keyword_start = mod_temp + len_print;
	snprintf(mod_temp + len_print, len_keyword + 1, "%s", temp + len_offset);
	len_print += len_keyword;
	len_print += snprintf(mod_temp + len_print, len_full - len_print,
			"%s%s", end_match, temp + len_offset + len_keyword);

	if (token_number < 1)
		return mod_temp;

	char *cursor = NULL;
	cursor = _strchr_with_nth(keyword_start, ' ', token_number);
	*cursor = '\0';
	cursor = _strrchr_with_nth(mod_temp, keyword_start, ' ', token_number);
	memcpy(mod_temp, cursor, len_full);

	return mod_temp;
}



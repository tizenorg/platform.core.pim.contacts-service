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
#include "ctsvc_localize_ch.h"

#define CTSVC_NORMALIZED_MAX_LEN 1024

#define CTSVC_COMBINING_DIACRITICAL_MARKS_START 0x0300
#define CTSVC_COMBINING_DIACRITICAL_MARKS_END	0x036f

typedef struct {
	UChar letter;
	char start;
	char end;
}hiragana_group_letter;

static hiragana_group_letter hiragana_group[13] = {
	{0x3042, 0x41, 0x4a}, // ぁ	あ	ぃ	い	ぅ	う	ぇ	え	ぉ	お
	{0x3042, 0x94, 0x94}, // ゔ
	{0x304b, 0x4b, 0x54}, // か	が	き	ぎ	く	ぐ	け	げ	こ	ご
	{0x304b, 0x95, 0x96}, // ゕ	ゖ
	{0x3055, 0x55, 0x5e}, // さ	ざ	し	じ	す	ず	せ	ぜ	そ	ぞ
	{0x305f, 0x5f, 0x69}, // た	だ	ち	ぢ	っ	つ	づ	て	で	と	ど
	{0x306a, 0x6a, 0x6e}, // な	に	ぬ	ね	の
	{0x306f, 0x6f, 0x7d}, // は	ば	ぱ	ひ	び	ぴ	ふ	ぶ	ぷ	へ	べ	ぺ	ほ	ぼ	ぽ
	{0x307e, 0x7e, 0x82}, // ま	み	む	め	も
	{0x3084, 0x83, 0x88}, // ゃ	や	ゅ	ゆ	ょ	よ
	{0x3089, 0x89, 0x8d}, // ら	り	る	れ	ろ
	{0x308f, 0x8e, 0x92}, // ゎ	わ
	{0x3093, 0x93, 0x93}, // ゐ	ゑ	を
};

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
	char *region = NULL;

	region = vconf_get_str(VCONFKEY_REGIONFORMAT);
	collator = ucol_open(region, &status);
	free(region);
	RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
			"ucol_open() Failed(%s)", u_errorName(status));

	if (U_FAILURE(status)) {
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
	if (U_FAILURE(status)) {
		CTS_ERR("u_strFromUTF8 Failed(%s)", u_errorName(status));
		free(tmp_result);
		ucol_close(collator);
		return CONTACTS_ERROR_SYSTEM;
	}

	size = ucol_getSortKey(collator, tmp_result, -1, NULL, 0);
	*dest = calloc(1, sizeof(uint8_t) * (size + 1));
	size = ucol_getSortKey(collator, tmp_result, -1, (uint8_t *)*dest, size + 1);

	ucol_close(collator);
	free(tmp_result);
	return CONTACTS_ERROR_NONE;
}

int ctsvc_collation_str(char *src, char **dest)
{
	int ret;
	char temp[SAFE_STRLEN(src) + 1];

	ret = __ctsvc_remove_special_char(src, temp, sizeof(temp));
	WARN_IF(ret < CONTACTS_ERROR_NONE, "__ctsvc_remove_special_char() Failed(%d)", ret);

	return __ctsvc_collation_str(temp, dest);
}

static int __ctsvc_normalize_str(const char *src, char **dest)
{
	int ret;
	int32_t tmp_size = 100;
	int32_t upper_size;
	int32_t size = 100;
	UErrorCode status = 0;
	UChar *tmp_result = NULL;
	UChar *tmp_upper = NULL;
	UChar *result = NULL;

	tmp_result = calloc(1, sizeof(UChar)*(tmp_size+1));
	u_strFromUTF8(tmp_result, tmp_size + 1, &tmp_size, src, -1, &status);

	if (status == U_BUFFER_OVERFLOW_ERROR) {
		status = U_ZERO_ERROR;
		free(tmp_result);
		tmp_result = calloc(1, sizeof(UChar) * (tmp_size + 1));
		u_strFromUTF8(tmp_result, tmp_size + 1, NULL, src, -1, &status);
		if (U_FAILURE(status)) {
			CTS_ERR("u_strFromUTF8()Failed(%s)", u_errorName(status));
			ret = CONTACTS_ERROR_SYSTEM;
			goto DATA_FREE;
		}
	}
	else if (U_FAILURE(status)) {
		CTS_ERR("u_strFromUTF8() Failed(%s)", u_errorName(status));
		ret = CONTACTS_ERROR_SYSTEM;
		goto DATA_FREE;
	}

	tmp_upper = calloc(1, sizeof(UChar)*(tmp_size+1));
	upper_size = u_strToUpper(tmp_upper, tmp_size+1, tmp_result, -1, NULL, &status);
	if (status == U_BUFFER_OVERFLOW_ERROR) {
		status = U_ZERO_ERROR;
		free(tmp_upper);
		tmp_upper = calloc(1, sizeof(UChar) * (upper_size + 1));
		u_strFromUTF8(tmp_upper, upper_size + 1, NULL, src, -1, &status);
		if (U_FAILURE(status)) {
			CTS_ERR("u_strFromUTF8()Failed(%s)", u_errorName(status));
			ret = CONTACTS_ERROR_SYSTEM;
			goto DATA_FREE;
		}
	}
	else if (U_FAILURE(status)) {
		CTS_ERR("u_strToUpper() Failed(%s)", u_errorName(status));
		ret = CONTACTS_ERROR_SYSTEM;
		goto DATA_FREE;
	}

	result = calloc(1, sizeof(UChar)*(size+1));
	size = unorm_normalize(tmp_upper, -1, UNORM_NFD, 0,
			result, size+1, &status);
	if (status == U_BUFFER_OVERFLOW_ERROR) {
		status = U_ZERO_ERROR;
		free(result);
		result = calloc(1, sizeof(UChar)*(size + 1));
		unorm_normalize(tmp_upper, -1, UNORM_NFD, 0, result, size+1, &status);
		if (U_FAILURE(status)) {
			CTS_ERR("unorm_normalize() Failed(%s)", u_errorName(status));
			ret = CONTACTS_ERROR_SYSTEM;
			goto DATA_FREE;
		}
	}
	else if (U_FAILURE(status)) {
		CTS_ERR("unorm_normalize() Failed(%s)", u_errorName(status));
		ret = CONTACTS_ERROR_SYSTEM;
		goto DATA_FREE;
	}

	ret = ctsvc_check_language(result);
	ctsvc_extra_normalize(result, size);

	// remove diacritical : U+3000 ~ U+034F
	int i, j;
	UChar *temp_result = NULL;
	temp_result = calloc(1, sizeof(UChar)*(size+1));
	bool replaced = false;
	for(i=0,j=0; i<size;i++) {
		if (CTSVC_COMPARE_BETWEEN((UChar)CTSVC_COMBINING_DIACRITICAL_MARKS_START,
			result[i], (UChar)CTSVC_COMBINING_DIACRITICAL_MARKS_END)) {
			replaced = true;
		}
		else
			temp_result[j++] = result[i];
	}

	if (replaced) {
		temp_result[j] = 0x0;
		free(result);
		result = temp_result;
	}
	else
		free(temp_result);

	u_strToUTF8(NULL, 0, &size, result, -1, &status);
	status = U_ZERO_ERROR;
	*dest = calloc(1, sizeof(char) * (size+1));

	u_strToUTF8(*dest, size+1, NULL, result, -1, &status);
	if (U_FAILURE(status)) {
		CTS_ERR("u_strToUTF8() Failed(%s)", u_errorName(status));
		ret = CONTACTS_ERROR_SYSTEM;
		free(*dest);
		*dest = NULL;
		goto DATA_FREE;
	}

DATA_FREE:
	free(tmp_result);
	free(tmp_upper);
	free(result);
	return ret;
}

static int __ctsvc_convert_halfwidth_ascii_and_symbol(const char *src, UChar *dest, int dest_size, int* str_size)
{
	int i;
	int32_t size = dest_size;
	UErrorCode status = 0;

	u_strFromUTF8(dest, dest_size, &size, src, strlen(src), &status);
	if (U_FAILURE(status)) {
		CTS_ERR("u_strFromUTF8() Failed(%s)", u_errorName(status));
		return CONTACTS_ERROR_SYSTEM;
	}

	*str_size = size;

	// full width -> half width
	for (i=0; i < size; i++) {
		// FF00 ~ FF60: Fullwidth ASCII variants
		if (CTSVC_COMPARE_BETWEEN((UChar)0xFF00, dest[i], (UChar)0xFF60)) {
			int unicode_value1 = 0;
			int unicode_value2 = 0;
			unicode_value1 = 0x0;
			unicode_value2 = (0xFF & dest[i]) + 0x20;
			dest[i] = unicode_value1 << 8 | unicode_value2;
		}
		// FFE0~FFE6: Fullwidth symbol variants
		else if (CTSVC_COMPARE_BETWEEN((UChar)0xFFE0, dest[i], (UChar)0xFFE6)) {
			if (dest[i] == (UChar)0xFFE0)
			{
				dest[i] = (UChar)0x00A2;
			}
			else if (dest[i] == (UChar)0xFFE1)
			{
				dest[i] = (UChar)0x00A3;
			}
			else if (dest[i] == (UChar)0xFFE2)
			{
				dest[i] = (UChar)0x00AC;
			}
			else if (dest[i] == (UChar)0xFFE3)
			{
				dest[i] = (UChar)0x00AF;
			}
			else if (dest[i] == (UChar)0xFFE4)
			{
				dest[i] = (UChar)0x00A6;
			}
			else if (dest[i] == (UChar)0xFFE5)
			{
				dest[i] = (UChar)0x00A5;
			}
			else if (dest[i] == (UChar)0xFFE6)
			{
				dest[i] = (UChar)0x20A9;
			}
			else
			{

			}
		}
		else {

		}

	}

	dest[size] = 0x00;
	return CONTACTS_ERROR_NONE;
}

#define LARGE_BUFFER_SIZE 100

int ctsvc_get_halfwidth_string(const char *src, char** dest, int* dest_size)
{
	UChar unicodes[LARGE_BUFFER_SIZE];
	int ustr_size = 0;

	if (__ctsvc_convert_halfwidth_ascii_and_symbol(src, unicodes, LARGE_BUFFER_SIZE, &ustr_size) != CONTACTS_ERROR_NONE)
	{
		CTS_ERR("convert to halfwidth failed! %s ", src);

		return CONTACTS_ERROR_SYSTEM;
	}

	UErrorCode status = 0;

	// pre-flighting
	int size = 0;
	u_strToUTF8(NULL, 0, &size, unicodes, -1, &status);
	status = U_ZERO_ERROR;
	*dest = calloc(1, sizeof(char) * (size+1));

	u_strToUTF8(*dest, size+1, dest_size, unicodes, ustr_size, &status);
	if (U_FAILURE(status)) {
		CTS_ERR("u_strToUTF8() Failed(%s)", u_errorName(status));

		free(*dest);
		*dest = NULL;
		*dest_size = 0;

		return CONTACTS_ERROR_SYSTEM;
	}

	return CONTACTS_ERROR_NONE;
}

////// contacts_normalized_strstr API should be separated from contacts-service /////////////////////////////////////////////////////////////
static int __ctsvc_normalize_str_to_unicode(const char *src, int src_size, UChar *dest, int dest_size)
{
	int ret;
	int32_t size = dest_size;
	UErrorCode status = 0;
	UChar tmp_result[dest_size];

	u_strFromUTF8(tmp_result, dest_size, &size, src, src_size, &status);

	if (U_FAILURE(status)) {
		CTS_ERR("u_strFromUTF8() Failed(%s)", u_errorName(status));
		return CONTACTS_ERROR_SYSTEM;
	}

	u_strToUpper(tmp_result, dest_size, tmp_result, -1, NULL, &status);
	if (U_FAILURE(status)) {
		CTS_ERR("u_strToUpper() Failed(%s)", u_errorName(status));
		return CONTACTS_ERROR_SYSTEM;
	}

	size = unorm_normalize(tmp_result, -1, UNORM_NFD, 0,
			(UChar *)dest, dest_size, &status);
	if (U_FAILURE(status)) {
		CTS_ERR("unorm_normalize() Failed(%s)", u_errorName(status));
		return CONTACTS_ERROR_SYSTEM;
	}

	ret = ctsvc_check_language(dest);
	ctsvc_extra_normalize(dest, size);

	dest[size] = 0x00;

	return ret;
}

int ctsvc_normalize_str(const char *src, char **dest)
{
	int ret = CONTACTS_ERROR_NONE;
	char temp[strlen(src) + 1];

	ret = __ctsvc_remove_special_char(src, temp, strlen(src) + 1);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "__ctsvc_remove_special_char() Failed(%d)", ret);

	ret = __ctsvc_normalize_str(temp, dest);
	return ret;
}

static void __ctsvc_convert_japanese_group_letter(char *dest)
{
	int i, size, dest_len;
	UErrorCode status = 0;
	UChar tmp_result[2];
	UChar result[2] = {0x00};
	int unicode_value1, unicode_value2;

	dest_len = strlen(dest) + 1;
	u_strFromUTF8(tmp_result, array_sizeof(tmp_result), NULL, dest, -1, &status);
	RETM_IF(U_FAILURE(status), "u_strFromUTF8() Failed(%s)", u_errorName(status));

	unicode_value1 = (0xFF00 & (tmp_result[0])) >> 8;
	unicode_value2 = (0xFF & (tmp_result[0]));

	for(i=0; i < 13; i++) {
		if (hiragana_group[i].start <= unicode_value2
				&& unicode_value2 <= hiragana_group[i].end)
			result[0] = hiragana_group[i].letter;
	}

	u_strToUTF8(dest, dest_len, &size, result, -1, &status);
	RETM_IF(U_FAILURE(status), "u_strToUTF8() Failed(%s)", u_errorName(status));

}

int ctsvc_normalize_index(const char *src, char **dest)
{
	int ret = CONTACTS_ERROR_NONE;
	char first_str[10] = {0};
	int length = 0;

	length = ctsvc_check_utf8(src[0]);
	RETVM_IF(length <= 0, CONTACTS_ERROR_INTERNAL, "check_utf8 is failed");

	strncpy(first_str, src, length);
	if (length != strlen(first_str))
		return CONTACTS_ERROR_INVALID_PARAMETER;

	ret = __ctsvc_normalize_str(first_str, dest);
	if ((*dest)[0] != '\0') {
		length = ctsvc_check_utf8((*dest)[0]);
		(*dest)[length] = '\0';
	}

	if (ret == CTSVC_LANG_JAPANESE) {
		__ctsvc_convert_japanese_group_letter(*dest);
	}
	return ret;
}

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

static inline bool __ctsvc_compare_utf8(const char *str1, const char *str2, int str2_len)
{
	int k;
	for (k=0; k<str2_len;k++)
		if (!str1[k] || !str2[k] || str1[k] != str2[k])
			return false;
	return true;
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

	for(i=0, k=0; i < sizeof(temp_needle); i++)
	{
		if (isupper(needle[i]))
			temp_needle[i] = tolower(needle[i]);
		else
			temp_needle[i] = needle[i];
	}

	for(i=0, j=0; i < strlen(haystack) && j < strlen(temp_needle) ; i+=len)
	{
		len = ctsvc_check_utf8(haystack[i]);
		if (len < 0)
			return false;
		memcpy(temp, haystack + i, len);
		temp[len] = '\0';

		ret = ctsvc_convert_chinese_to_pinyin(temp, &pinyinname, &size);
		if (ret != CONTACTS_ERROR_NONE) {
			return false;
		}

		for(k=0; k<size; k++) {
			if (!initial_match &&
					strlen(pinyinname[k].pinyin_name) <= strlen(temp_needle + j) &&
					strncmp(pinyinname[k].pinyin_name, temp_needle + j, strlen(pinyinname[k].pinyin_name)) == 0) {
				match = true;
				j+=strlen(pinyinname[k].pinyin_name);
				break;

			}
			else if (!initial_match &&
					strlen(pinyinname[k].pinyin_name) > strlen(temp_needle + j) &&
					strncmp(pinyinname[k].pinyin_name, temp_needle + j, strlen(temp_needle + j)) == 0) {
				match = true;
				j+=strlen(temp_needle + j);
				break;

			}
			else if (pinyinname[k].pinyin_initial[0] ==  temp_needle[j]) {
				initial_match = true;
				match = true;
				j++;
				break;
			}
			else
				match = false;
		}
		free(pinyinname);

		if (match==false) {
			break;
		}

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

	switch (haystack_lang)
	{
	case CTSVC_LANG_NUMBER:
	case CTSVC_LANG_OTHERS:
		{
			if (haystack_lang == needle_lang)
			{
				for(i=0, j=0; i<u_strlen(haystack) && j<u_strlen(needle);) {
					if (haystack[i] == needle[j])
						ret = true;
					else {
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
			switch(needle_lang)
			{
			case CTSVC_LANG_ENGLISH:
				for(i=0, j=0; i<u_strlen(haystack) && j<u_strlen(needle);) {
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

					if (haystack[i] == needle[j])
						ret = true;
					else {
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
					&& haystack[0] == needle[0]) {
				return true;
			}

			for(i=0, j=0; i<u_strlen(haystack) && j<u_strlen(needle);) {
				if (haystack[i] == needle[j])
					ret = true;
				else {
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

			if (haystack[0] == needle[0])
				ret = true;
			else {
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
			if (needle_lang == haystack_lang
					&& haystack[0] == needle[0])
				ret = true;
		}
		return ret;
	}

	return false;
}

/**
 * This function compares compares two strings which is not normalized.
 * If search_str is included in str, this function return #sCONTACTS_ERROR_NONE. \n
 * The behavior of this function cannot fix because of localization.
 * So, The behavior can be different from each other.
 *
 * @param[in] haystack Base string.
 * @param[in] needle searching string
 * @param[out] len substring length
 * @return a position of the beginning of the substring, Negative value(#cts_error) on error or difference.
 */
API int contacts_utils_strstr(const char *haystack,
		const char *needle, int *len)
{
	int ret, h_len, n_len, i, j;
	UChar haystack_letter[SMALL_BUFFER_SIZE];
	UChar needle_letter[SMALL_BUFFER_SIZE];
	UChar first_needle_letter[SMALL_BUFFER_SIZE];
	int haystack_letter_lang;
	int needle_letter_lang;
	int first_needle_letter_lang;

	bool matching=false;
	int match_len = 0;
	int match_start = -1;

	char temp_haystack[strlen(haystack) + 1];
	char temp_needle[strlen(needle) + 1];

	RETVM_IF(NULL == haystack, -1, "The parameter(haystack) is NULL");
	RETVM_IF(NULL == needle, -1, "The parameter(needle) is NULL");
	CTS_VERBOSE("haystack = %s, needle = %s", haystack, needle);

	*len = 0;

	ret = __ctsvc_remove_special_char(haystack, temp_haystack, strlen(haystack) + 1);
	ret = __ctsvc_remove_special_char(needle, temp_needle, strlen(needle) + 1);

	n_len = ctsvc_check_utf8(temp_needle[0]);

	first_needle_letter_lang = __ctsvc_normalize_str_to_unicode(temp_needle, n_len, first_needle_letter, SMALL_BUFFER_SIZE);
	RETVM_IF(first_needle_letter_lang < CONTACTS_ERROR_NONE , -1, "The __ctsvc_normalize_str_to_unicode failed(%d)", first_needle_letter_lang);

	for (i=0, j=0;i<strlen(temp_haystack) && j<strlen(temp_needle);i+=h_len) {
		h_len = ctsvc_check_utf8(temp_haystack[i]);

		haystack_letter_lang = __ctsvc_normalize_str_to_unicode(temp_haystack + i, h_len, haystack_letter, SMALL_BUFFER_SIZE);
		RETVM_IF(haystack_letter_lang < CONTACTS_ERROR_NONE , -1, "The __ctsvc_normalize_str_to_unicode failed(%d)", haystack_letter_lang);

		if (matching == false) {
			if (__ctsvc_compare_unicode_letter(haystack_letter, haystack_letter_lang, first_needle_letter, first_needle_letter_lang)
					|| __ctsvc_compare_pinyin_letter(temp_haystack + i, haystack_letter_lang, temp_needle + j, first_needle_letter_lang, &h_len, &n_len)) {
				matching = true;
				j+=n_len;
				match_start = i;
				match_len = h_len;

				if (temp_needle[j] == '\0') {
					*len = match_len;
					return match_start;
				}
			}
			continue;
		}
		else if (matching == true) {
			n_len = ctsvc_check_utf8(temp_needle[j]);

			needle_letter_lang = __ctsvc_normalize_str_to_unicode(temp_needle + j, n_len, needle_letter, SMALL_BUFFER_SIZE);
			RETVM_IF(needle_letter_lang < CONTACTS_ERROR_NONE , -1, "The __ctsvc_normalize_str_to_unicode failed(%d)", needle_letter_lang);

			if (__ctsvc_compare_unicode_letter(haystack_letter, haystack_letter_lang, needle_letter, needle_letter_lang)) {
				j+=n_len;
				match_len += h_len;

				if (temp_needle[j] == '\0') {
					*len = match_len;
					return match_start;
				}
				continue;
			}
			else {
				j = 0;
				matching = false;
				match_start = -1;
				match_len = 0;
			}
		}
	}

	CTS_VERBOSE("NOT match");
	return -1;
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
				if (h_len == first_needle_len && __ctsvc_compare_utf8(&haystack[i], needle, first_needle_len)
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

			equal = __ctsvc_compare_utf8(&haystack[i], &needle[j], n_len);

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
				if (h_len == 2 && __ctsvc_is_diacritical(&haystack[i]))
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
 */
API int contacts_normalize_str(const char *src, char *dest, const int dest_len)
{
	int ret;
	char *temp = NULL;
	RETV_IF(NULL == dest, CONTACTS_ERROR_INVALID_PARAMETER);
	RETVM_IF(dest_len <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "dest_len(%d) is Invalid", dest_len);
	dest[0] = '\0';

	ret = ctsvc_normalize_str(src, &temp);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "ctsvc_normalize_str() Failed(%d)", ret);

	snprintf(dest, dest_len, "%s", temp);
	free(temp);

	return CONTACTS_ERROR_NONE;
}


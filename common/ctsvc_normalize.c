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
#include "ctsvc_setting.h"



#define CTSVC_NORMALIZED_MAX_LEN 1024

#define CTSVC_COMBINING_DIACRITICAL_MARKS_START 0x0300
#define CTSVC_COMBINING_DIACRITICAL_MARKS_END 	0x036f

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

static int __ctsvc_normalize_number(const char *src, char *dest, int dest_size, int min_match)
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


int ctsvc_normalize_number(const char *src, char *dest, int dest_size, int min_match)
{
	int ret;

	ret = __ctsvc_normalize_number(src, dest, dest_size, min_match);
	if (ret != CONTACTS_ERROR_NONE) {
		CTS_ERR("__ctsvc_normalize_number(src) failed(%d)", src, ret);
		return ret;
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

static int __ctsvc_normalize_str(const char *src, char **dest)
{
	int ret;
	int32_t tmp_size = 100;
	int32_t size = 100;
	UErrorCode status = 0;
	UChar *tmp_result = NULL;
	UChar *result = NULL;

	tmp_result = calloc(1, sizeof(UChar)*(tmp_size+1));
	result = calloc(1, sizeof(UChar)*(size+1));
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

	u_strToUpper(tmp_result, tmp_size + 1, tmp_result, -1, NULL, &status);
	if (U_FAILURE(status)) {
		CTS_ERR("u_strToUpper() Failed(%s)", u_errorName(status));
		ret = CONTACTS_ERROR_SYSTEM;
		goto DATA_FREE;
	}

	size = unorm_normalize(tmp_result, -1, UNORM_NFD, 0,
			(UChar *)result, size + 1, &status);

	if (status == U_BUFFER_OVERFLOW_ERROR) {
		status = U_ZERO_ERROR;
		free(result);
		result = calloc(1, sizeof(UChar) * (size + 1));
		size = unorm_normalize(tmp_result, -1, UNORM_NFD, 0, (UChar *)result, size + 1, &status);
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

	u_strToUTF8(NULL, 0, &size, result, -1, &status);
	status = U_ZERO_ERROR;
	*dest = calloc(1, sizeof(char) * (size+1));

	u_strToUTF8(*dest, size+1, NULL, result, -1, &status);
	if (U_FAILURE(status)) {
		CTS_ERR("u_strToUTF8() Failed(%s)", u_errorName(status));
		ret = CONTACTS_ERROR_SYSTEM;
		free(*dest);
		goto DATA_FREE;
	}

DATA_FREE:
	free(tmp_result);
	free(result);
	return ret;

}

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


	for(i=0; i < 13; i++)
	{
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
	int lang_first = CTSVC_LANG_ENGLISH;
	int lang_second = CTSVC_LANG_KOREAN;
	int sort_first, sort_second;
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


	sort_first = ctsvc_get_default_language();
	switch(sort_first)
	{
	case CTSVC_SORT_WESTERN:
		lang_first = CTSVC_LANG_ENGLISH;
		break;
	case CTSVC_SORT_KOREAN:
		lang_first = CTSVC_LANG_KOREAN;
		break;
	case CTSVC_SORT_JAPANESE:
		lang_first = CTSVC_LANG_JAPANESE;
		break;
	default:
		CTS_ERR("The default language is not valid");
	}

	first = ctsvc_get_language(lang_first);
	__ctsvc_get_language_index(first, &first_list, &first_len);
	for (i=0;i<first_len;i++) {
		strcat(list, first_list[i]);
		if (i != (first_len-1))
			strcat(list, ";");
		free(first_list[i]);
	}
	free(first_list);

	sort_second = ctsvc_get_secondary_language();
	switch(sort_second)
	{
	case CTSVC_SORT_WESTERN:
		lang_second = CTSVC_LANG_ENGLISH;
		break;
	case CTSVC_SORT_KOREAN:
		lang_second = CTSVC_LANG_KOREAN;
		break;
	case CTSVC_SORT_JAPANESE:
		lang_second = CTSVC_LANG_JAPANESE;
		break;
	default:
		CTS_ERR("The default language is not valid");
	}
	second = ctsvc_get_language(lang_second);
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

	for(i=0, k=0; i < strlen(temp_needle); i++)
	{
		if (isupper(needle[i]))
			temp_needle[i] = tolower(needle[i]);
		else
			temp_needle[i] = needle[i];
	}

	for(i=0, j=0; i < strlen(haystack) && j < strlen(temp_needle) ; i+=len)
	{
		len = ctsvc_check_utf8(haystack[i]);
		memcpy(temp, haystack + i, len );
		temp[len] = '\0';

		ret = ctsvc_convert_chinese_to_pinyin(temp, &pinyinname, &size);
		if (ret != CONTACTS_ERROR_NONE) {
			return false;
		}

		for(k=0; k<size; k++) {
			if (!initial_match &&
					strlen(pinyinname[k].pinyin_name) <= strlen(temp_needle + j) &&
					strncmp(pinyinname[k].pinyin_name, temp_needle + j, strlen(pinyinname[k].pinyin_name)) == 0) {
				DBG("A name matched");
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

					if(haystack[i] == needle[j])
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
			if(needle_lang != CTSVC_LANG_KOREAN)
				break;

			if (u_strlen(needle) == 1
					&& CTSVC_COMPARE_BETWEEN(0x3130, needle[0], 0x314e)
					&& haystack[0] == needle[0]) {
				return true;
			}

			for(i=0, j=0; i<u_strlen(haystack) && j<u_strlen(needle);) {
				if(haystack[i] == needle[j])
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
	case CTSVC_LANG_CHINESE:
		{
			if(needle_lang == haystack_lang
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
 * @par example
 * @code
	ret = contacts_strstr(str1, str2, &len);
	if(CONTACTS_ERROR_NONE == ret) {
		snprintf(first, ret+1, "%s", item_data->display);
		snprintf(middle, len+1, "%s", item_data->display + ret);
		printf("%s -> %s, %s, %s", item_data->display, first, middle, item_data->display + ret + len);
	} else
		printf("str1 doesn't has str2");
 * @endcode
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

		if (matching == false)
		{
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

			if (__ctsvc_compare_unicode_letter(haystack_letter, haystack_letter_lang, needle_letter, needle_letter_lang )){
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


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

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_normalize.h"
#include "ctsvc_localize.h"
#include "ctsvc_localize_utils.h"

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

static inline int __ctsvc_collation_str(const char *src, char **dest)
{
	int32_t size = 0;
	UErrorCode status = U_ZERO_ERROR;
	UChar *tmp_result = NULL;
	UCollator *collator;

	char *region = strdup(ctsvc_get_langset());

	char *dot = strchr(region, '.');
	if (dot)
		*dot = '\0';

	collator = ucol_open(region, &status);
	if (U_FAILURE(status)) {
		CTS_ERR("ucol_open Fail(%s)", u_errorName(status));
		free(region);
		return CONTACTS_ERROR_SYSTEM;
	}

	// TODO: ucol_setAttribute is not called
	if (U_FAILURE(status)) {
		CTS_ERR("ucol_setAttribute Fail(%s)", u_errorName(status));
		free(region);
		ucol_close(collator);
		return CONTACTS_ERROR_SYSTEM;
	}

	u_strFromUTF8(NULL, 0, &size, src, strlen(src), &status);
	if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR) {
		CTS_ERR("u_strFromUTF8 to get the dest length Fail(%s)", u_errorName(status));
		free(region);
		ucol_close(collator);
		return CONTACTS_ERROR_SYSTEM;
	}
	status = U_ZERO_ERROR;
	tmp_result = calloc(1, sizeof(UChar) * (size + 1));
	u_strFromUTF8(tmp_result, size + 1, NULL, src, -1, &status);
	if (U_FAILURE(status)) {
		CTS_ERR("u_strFromUTF8 Fail(%s)", u_errorName(status));
		free(region);
		free(tmp_result);
		ucol_close(collator);
		return CONTACTS_ERROR_SYSTEM;
	}

	size = ucol_getSortKey(collator, tmp_result, -1, NULL, 0);
	*dest = calloc(1, sizeof(uint8_t) * (size + 1));
	size = ucol_getSortKey(collator, tmp_result, -1, (uint8_t *)*dest, size + 1);

	ucol_close(collator);
	free(tmp_result);
	free(region);
	return CONTACTS_ERROR_NONE;
}

int ctsvc_collation_str(char *src, char **dest)
{
	int ret;
	char temp[SAFE_STRLEN(src) + 1];

	ret = __ctsvc_remove_special_char(src, temp, sizeof(temp));
	WARN_IF(ret < CONTACTS_ERROR_NONE, "__ctsvc_remove_special_char() Fail(%d)", ret);

	return __ctsvc_collation_str(temp, dest);
}

static int __ctsvc_normalize_str(const char *src, char **dest)
{
	int32_t tmp_size = 100;
	int32_t upper_size;
	int32_t size = 100;
	UErrorCode status = 0;
	UChar *tmp_result = NULL;
	UChar *tmp_upper = NULL;
	UChar *result = NULL;

	tmp_result = calloc(1, sizeof(UChar)*(tmp_size+1));
	if (NULL == tmp_result) {
		CTS_ERR("calloc() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	u_strFromUTF8(tmp_result, tmp_size + 1, &tmp_size, src, -1, &status);
	if (status == U_BUFFER_OVERFLOW_ERROR) {
		status = U_ZERO_ERROR;
		free(tmp_result);
		tmp_result = calloc(1, sizeof(UChar) * (tmp_size + 1));
		if (NULL == tmp_result) {
			CTS_ERR("calloc() Fail");
			return CONTACTS_ERROR_OUT_OF_MEMORY;
		}

		u_strFromUTF8(tmp_result, tmp_size + 1, NULL, src, -1, &status);
		if (U_FAILURE(status)) {
			CTS_ERR("u_strFromUTF8()Fail(%s)", u_errorName(status));
			free(tmp_result);
			return CONTACTS_ERROR_SYSTEM;
		}
	}
	else if (U_FAILURE(status)) {
		CTS_ERR("u_strFromUTF8() Fail(%s)", u_errorName(status));
		free(tmp_result);
		return CONTACTS_ERROR_SYSTEM;
	}

	tmp_upper = calloc(1, sizeof(UChar)*(tmp_size+1));
	if (NULL == tmp_upper) {
		CTS_ERR("calloc() Fail");
		free(tmp_result);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	upper_size = u_strToUpper(tmp_upper, tmp_size+1, tmp_result, -1, NULL, &status);
	if (status == U_BUFFER_OVERFLOW_ERROR) {
		status = U_ZERO_ERROR;
		free(tmp_upper);
		tmp_upper = calloc(1, sizeof(UChar) * (upper_size + 1));
		if (NULL == tmp_upper) {
			CTS_ERR("calloc() Fail");
			free(tmp_result);
			return CONTACTS_ERROR_OUT_OF_MEMORY;
		}

		u_strFromUTF8(tmp_upper, upper_size + 1, NULL, src, -1, &status);
		if (U_FAILURE(status)) {
			CTS_ERR("u_strFromUTF8()Fail(%s)", u_errorName(status));
			free(tmp_result);
			free(tmp_upper);
			return CONTACTS_ERROR_SYSTEM;
		}
	}
	else if (U_FAILURE(status)) {
		CTS_ERR("u_strToUpper() Fail(%s)", u_errorName(status));
		free(tmp_result);
		free(tmp_upper);
		return CONTACTS_ERROR_SYSTEM;
	}

	result = calloc(1, sizeof(UChar)*(size+1));
	if (NULL == result) {
		CTS_ERR("calloc() Fail");
		free(tmp_result);
		free(tmp_upper);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	size = unorm_normalize(tmp_upper, -1, UNORM_NFD, 0, result, size+1, &status);
	if (status == U_BUFFER_OVERFLOW_ERROR) {
		status = U_ZERO_ERROR;
		free(result);
		result = calloc(1, sizeof(UChar)*(size + 1));
		if (NULL == result) {
			CTS_ERR("calloc() Fail");
			free(tmp_result);
			free(tmp_upper);
			return CONTACTS_ERROR_OUT_OF_MEMORY;
		}

		unorm_normalize(tmp_upper, -1, UNORM_NFD, 0, result, size+1, &status);
		if (U_FAILURE(status)) {
			CTS_ERR("unorm_normalize() Fail(%s)", u_errorName(status));
			free(tmp_result);
			free(tmp_upper);
			free(result);
			return CONTACTS_ERROR_SYSTEM;
		}
	}
	else if (U_FAILURE(status)) {
		CTS_ERR("unorm_normalize() Fail(%s)", u_errorName(status));
		free(tmp_result);
		free(tmp_upper);
		free(result);
		return CONTACTS_ERROR_SYSTEM;
	}

	ctsvc_check_language(result);
	ctsvc_extra_normalize(result, size);

	// remove diacritical : U+3000 ~ U+034F
	int i, j;
	UChar *temp_result = NULL;
	temp_result = calloc(1, sizeof(UChar)*(size+1));
	if (NULL == temp_result) {
		CTS_ERR("calloc() Fail");
		free(tmp_result);
		free(tmp_upper);
		free(result);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	bool replaced = false;
	for (i=0,j=0; i<size;i++) {
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
	if (NULL == *dest) {
		CTS_ERR("calloc() Fail");
		free(tmp_result);
		free(tmp_upper);
		free(result);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	u_strToUTF8(*dest, size+1, NULL, result, -1, &status);
	if (U_FAILURE(status)) {
		CTS_ERR("u_strToUTF8() Fail(%s)", u_errorName(status));
		free(*dest);
		*dest = NULL;
		free(tmp_result);
		free(tmp_upper);
		free(result);
		return CONTACTS_ERROR_SYSTEM;
	}
	free(tmp_result);
	free(tmp_upper);
	free(result);
	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_convert_halfwidth_ascii_and_symbol(const char *src, UChar *dest, int dest_size, int* str_size)
{
	int i;
	int32_t size = dest_size;
	UErrorCode status = 0;

	u_strFromUTF8(dest, dest_size, &size, src, strlen(src), &status);
	if (U_FAILURE(status)) {
		CTS_ERR("u_strFromUTF8() Fail(%s)", u_errorName(status));
		return CONTACTS_ERROR_SYSTEM;
	}

	*str_size = size;

	// full width -> half width
	for (i=0;i<size;i++) {
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
			if (dest[i] == (UChar)0xFFE0) {
				dest[i] = (UChar)0x00A2;
			}
			else if (dest[i] == (UChar)0xFFE1) {
				dest[i] = (UChar)0x00A3;
			}
			else if (dest[i] == (UChar)0xFFE2) {
				dest[i] = (UChar)0x00AC;
			}
			else if (dest[i] == (UChar)0xFFE3) {
				dest[i] = (UChar)0x00AF;
			}
			else if (dest[i] == (UChar)0xFFE4) {
				dest[i] = (UChar)0x00A6;
			}
			else if (dest[i] == (UChar)0xFFE5) {
				dest[i] = (UChar)0x00A5;
			}
			else if (dest[i] == (UChar)0xFFE6) {
				dest[i] = (UChar)0x20A9;
			}
			else {

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
	UChar unicodes[LARGE_BUFFER_SIZE+1];
	int ustr_size = 0;

	if (CONTACTS_ERROR_NONE != __ctsvc_convert_halfwidth_ascii_and_symbol(src, unicodes, LARGE_BUFFER_SIZE, &ustr_size)) {
		CTS_ERR("convert to halfwidth Fail! %s ", src);

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
		CTS_ERR("u_strToUTF8() Fail(%s)", u_errorName(status));

		free(*dest);
		*dest = NULL;
		*dest_size = 0;

		return CONTACTS_ERROR_SYSTEM;
	}

	return CONTACTS_ERROR_NONE;
}

int ctsvc_normalize_str(const char *src, char **dest)
{
	int ret = CONTACTS_ERROR_NONE;
	char temp[strlen(src) + 1];

	ret = __ctsvc_remove_special_char(src, temp, strlen(src) + 1);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "__ctsvc_remove_special_char() Fail(%d)", ret);

	ret = __ctsvc_normalize_str(temp, dest);
	return ret;
}

static void __ctsvc_convert_japanese_group_letter(char *dest)
{
	int i, size, dest_len;
	UErrorCode status = 0;
	UChar tmp_result[2];
	UChar result[2] = {0x00};
	int unicode_value;

	dest_len = strlen(dest) + 1;
	u_strFromUTF8(tmp_result, array_sizeof(tmp_result), NULL, dest, -1, &status);
	RETM_IF(U_FAILURE(status), "u_strFromUTF8() Fail(%s)", u_errorName(status));

	unicode_value = (0xFF & (tmp_result[0]));

	for (i=0; i < 13; i++) {
		if (hiragana_group[i].start <= unicode_value
				&& unicode_value <= hiragana_group[i].end)
			result[0] = hiragana_group[i].letter;
	}

	u_strToUTF8(dest, dest_len, &size, result, -1, &status);
	RETM_IF(U_FAILURE(status), "u_strToUTF8() Fail(%s)", u_errorName(status));

}

static bool __ctsvc_check_range_out_index(const char src[])
{
	if (src[0] == 0xe2 && src[1] == 0x80 && src[2] == 0xa6) {
		return true;
	}
	return false;
}

int ctsvc_normalize_index(const char *src, char **dest)
{
	int ret = CONTACTS_ERROR_NONE;
	char first_str[10] = {0};
	int length = 0;

	if (first_str[0] == '\0' || __ctsvc_check_range_out_index(first_str)) {
		length = ctsvc_check_utf8(src[0]);

		RETVM_IF(length <= 0, CONTACTS_ERROR_INTERNAL, "check_utf8() Fail");
		memset(first_str,0x00, sizeof(first_str));
		strncpy(first_str, src, length);
		if (length != strlen(first_str)) {
			CTS_ERR("length : %d, first_str : %s, strlne : %d", length, first_str, strlen(first_str));
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
	}
	ret = __ctsvc_normalize_str(first_str, dest);

	RETVM_IF(dest == NULL, ret, "__ctsvc_normalize_str() Fail");

	if ((*dest)[0] != '\0') {
		length = ctsvc_check_utf8((*dest)[0]);
		(*dest)[length] = '\0';
	}

	if (ret == CTSVC_LANG_JAPANESE) {
		__ctsvc_convert_japanese_group_letter(*dest);
	}

	return ret;
}



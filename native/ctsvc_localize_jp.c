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

#include <unicode/ustring.h>
#include <unicode/unorm.h>
#include <unicode/ucol.h>
#include <unicode/uset.h>

#include "ctsvc_internal.h"
#include "ctsvc_normalize.h"
#include "ctsvc_localize.h"
#include "ctsvc_localize_utils.h"

#include "ctsvc_localize_jp.h"

/* japanese - katakana */
#define CTSVC_JAPANESE_KATAKANA_START	0x30A0
#define CTSVC_JAPANESE_KATAKANA_END	0x30FF

/* japanese - katakana phonetic extensions */
#define CTSVC_JAPANESE_KATAKANA_PHONETIC_EXTENSIONS_START 0x31F0
#define CTSVC_JAPANESE_KATAKANA_PHONETIC_EXTENSIONS_END 0x31FF

/* japanese - halfwidth and fullwidth forms */
#define CTSVC_JAPANESE_HALFWIDTH_AND_FULLWIDTH_FORMS_START 0xFF00
#define CTSVC_JAPANESE_HALFWIDTH_AND_FULLWIDTH_FORMS_END 0xFFEF

/* japanese - halfwidth and fullwidth forms */
#define CTSVC_ASCII_HALFWIDTH_AND_FULLWIDTH_FORMS_START 0xFF01
#define CTSVC_ASCII_HALFWIDTH_AND_FULLWIDTH_FORMS_END 0xFF5E

/* japanese - hiragana */
#define CTSVC_JAPANESE_HIRAGANA_START 0x3040
#define CTSVC_JAPANESE_HIRAGANA_END 0x309F


static const unsigned char japanese_halfwidth_katakana_to_hiragana[] = { // 0xff66 - 0xff9d
	0x92, 0x41, 0x43, 0x45, 0x47, 0x49, 0x83, 0x85, 0x87, 0x63,
	0x00, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4B, 0x4D, 0x4F, 0x51,
	0x53, 0x55, 0x57, 0x59, 0x5B, 0x5D, 0x5F, 0x61, 0x64, 0x66,
	0x68, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x72, 0x75, 0x78,
	0x7B, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x84, 0x86, 0x88, 0x89,
	0x8A, 0x8B, 0x8C, 0x8D, 0x8F, 0x93};

static const unsigned char japanese_halfwidth_katakana_sonant_to_hiragana[] = { // 0xff76 - 0xff89
	0x4C, 0x4E, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E,
	0x60, 0x62, 0x65, 0x67, 0x69, 0x70, 0x73, 0x76, 0x79, 0x7C};

static const unsigned char japanese_halfwidth_katakana_half_dullness_to_hiragana[] = { // 0xff8a - 0xff8e
	0x71, 0x74, 0x77, 0x7A, 0x7D};


static inline bool is_japanese(UChar src)
{
	if (CTSVC_COMPARE_BETWEEN(CTSVC_JAPANESE_KATAKANA_START, src, CTSVC_JAPANESE_KATAKANA_END)
			|| CTSVC_COMPARE_BETWEEN(CTSVC_JAPANESE_KATAKANA_PHONETIC_EXTENSIONS_START, src, CTSVC_JAPANESE_KATAKANA_PHONETIC_EXTENSIONS_END)
			|| CTSVC_COMPARE_BETWEEN(CTSVC_JAPANESE_HALFWIDTH_AND_FULLWIDTH_FORMS_START, src, CTSVC_JAPANESE_HALFWIDTH_AND_FULLWIDTH_FORMS_END)
			|| CTSVC_COMPARE_BETWEEN(CTSVC_JAPANESE_HIRAGANA_START, src, CTSVC_JAPANESE_HIRAGANA_END))
		return true;
	else
		return false;
}


int ctsvc_convert_japanese_to_hiragana_unicode(UChar *src, UChar *dest, int dest_size)
{
	int i, j = 0, len = 0;

	len = u_strlen(src);

	for (i = 0; i < len; i++) {
		int unicode_value1 = 0;
		int unicode_value2 = 0;

		unicode_value2 = (0xFF & (src[i]));

		if (CTSVC_COMPARE_BETWEEN(CTSVC_JAPANESE_KATAKANA_START, src[i], CTSVC_JAPANESE_KATAKANA_END)) {
			unicode_value1 = 0x30;
			if ((0xa1 <= unicode_value2 && unicode_value2 <= 0xef)
					|| (unicode_value2 == 0xF2 || unicode_value2 == 0xF3)) {
				unicode_value2 -= 0x60;
				dest[j] = unicode_value1 << 8 | unicode_value2;
			}
			else {
				dest[j] = src[i];
			}
		}
		else if (CTSVC_COMPARE_BETWEEN(CTSVC_JAPANESE_HALFWIDTH_AND_FULLWIDTH_FORMS_START,
							src[i], CTSVC_JAPANESE_HALFWIDTH_AND_FULLWIDTH_FORMS_END)) {
			unicode_value1 = 0x30;
			if (i+1 < len && (0xFF & (src[i+1])) == 0x9E
					&& 0x76 <= unicode_value2 && unicode_value2 <= 0x89) {
				unicode_value2 = japanese_halfwidth_katakana_sonant_to_hiragana[unicode_value2 - 0x76];
				dest[j] = unicode_value1 << 8 | unicode_value2;
				i++;
			}
			else if (i+1 < len && (0xFF & (src[i])) == 0x9F
					&& 0x8a <= unicode_value2 && unicode_value2 <= 0x8e) {
				unicode_value2 = japanese_halfwidth_katakana_half_dullness_to_hiragana[unicode_value2 - 0x8a];
				dest[j] = unicode_value1 << 8 | unicode_value2;
				i++;
			}
			else if (0x66 <= unicode_value2 && unicode_value2 <= 0x9d) {
				unicode_value2 = japanese_halfwidth_katakana_to_hiragana[unicode_value2 - 0x66];
				dest[j] = unicode_value1 << 8 | unicode_value2;
			}
			else {
				dest[j] = src[i];
			}
		}
		else if (CTSVC_COMPARE_BETWEEN(CTSVC_ASCII_HALFWIDTH_AND_FULLWIDTH_FORMS_START,
							src[i], CTSVC_ASCII_HALFWIDTH_AND_FULLWIDTH_FORMS_END)) {
			unicode_value1 = 0x00;
			unicode_value2 = unicode_value2 - 0x20;
			dest[j] = unicode_value1 << 8 | unicode_value2;
		} else {
			dest[j] = src[i];
		}
		j++;
	}

	dest[j] = 0x0;

	return j;
}

int ctsvc_convert_japanese_to_hiragana(const char *src, char **dest)
{
	UChar *tmp_result = NULL;
	UChar *result = NULL;
	UErrorCode status = 0;
	int32_t size;

	u_strFromUTF8(NULL, 0, &size, src, strlen(src), &status);
	if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR) {
		CTS_ERR("u_strFromUTF8 to get the dest length Failed(%s)", u_errorName(status));
		return CONTACTS_ERROR_SYSTEM;
	}
	status = U_ZERO_ERROR;
	tmp_result = calloc(1, sizeof(UChar) * (size + 1));
	if (NULL == tmp_result) {
		CTS_ERR("calloc Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	u_strFromUTF8(tmp_result, size + 1, NULL, src, -1, &status);
	if (U_FAILURE(status)) {
		CTS_ERR("u_strFromUTF8 Failed(%s)", u_errorName(status));
		free(tmp_result);
		return CONTACTS_ERROR_SYSTEM;
	}

	result = calloc(1, sizeof(UChar) * (size + 1));
	if (NULL == result) {
		CTS_ERR("calloc() Fail");
		free(tmp_result);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	ctsvc_convert_japanese_to_hiragana_unicode(tmp_result, result, size + 1);
	u_strToUTF8(NULL, 0, &size, result, -1, &status);
	if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR) {
		CTS_ERR("u_strToUTF8 to get the dest length Failed(%s)", u_errorName(status));
		free(tmp_result);
		free(result);
		return CONTACTS_ERROR_SYSTEM;
	}

	status = U_ZERO_ERROR;
	*dest = calloc(1, sizeof(char)*(size+1));
	if (NULL == *dest) {
		CTS_ERR("calloc() Fail");
		free(tmp_result);
		free(result);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	u_strToUTF8(*dest, size + 1, &size, result, -1, &status);
	if (U_FAILURE(status)) {
		CTS_ERR("u_strToUTF8 Failed(%s)", u_errorName(status));
		free(tmp_result);
		free(result);
		free(*dest);
		*dest = NULL;
		return CONTACTS_ERROR_SYSTEM;
	}

	free(tmp_result);
	free(result);
	return CONTACTS_ERROR_NONE;
}


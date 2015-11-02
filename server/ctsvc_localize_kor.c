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

#include "ctsvc_localize_kor.h"


/* korean -Hangul Jamo */
#define CTSVC_HAN_J_START (UChar)0x1100
#define CTSVC_HAN_J_END (UChar)0x11FF

/* korean -Hangul Jamo extended A*/
#define CTSVC_JAMO_A_START (UChar)0xA960
#define CTSVC_JAMO_A_END (UChar)0xA97F

/* korean -Hangul Jamo extended B*/
#define CTSVC_JAMO_B_START (UChar)0xD7B0
#define CTSVC_JAMO_B_END (UChar)0xD7FF

/* korean -Hangul Compatability */
#define CTSVC_HAN_C_START (UChar)0x3130
#define CTSVC_HAN_C_END (UChar)0x318F

/* korean -Hangul halfwidth */
#define CTSVC_HAN_HALF_START (UChar)0xFFA0
#define CTSVC_HAN_HALF_END (UChar)0xFFDC

/* korean -Hangul Syllables */
#define CTSVC_HAN_SYLLABLES_START (UChar)0xAC00
#define CTSVC_HAN_SYLLABLES_END (UChar)0xD7A3


static const char hangul_compatibility_choseong[] = {
	0x32, 0x34, 0x37, 0x38, 0x39, 0x40, 0x41,
	0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x65, 0x66, 0x6E,
	0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80,
	0x81, 0x84, 0x85, 0x86, 0x31, 0x00};

static const char hangul_jamo_choseong[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x1A, 0x06, 0x07,   /* to choseong 0x1100~0x115F */
	0x08, 0x21, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x14, 0x15, 0x1C, 0x1D, 0x1E, 0x20,
	0x22, 0x23, 0x27, 0x29, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x32, 0x36, 0x40, 0x47, 0x4C, 0x57, 0x58, 0x59, 0x00, 0x00};

static const char hangul_compatibility_jungseong[] = {
	0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
	0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E,
	0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x87, 0x88,
	0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x00};

static const char hangul_jamo_jungseong[] = {
	0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,   /* to jungseong 0x1160~0x11A7 */
	0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
	0x73, 0x74, 0x75, 0x60, 0x84, 0x85, 0x88, 0x91, 0x92,
	0x94, 0x9E, 0xA1, 0x00};

static const char hangul_compatibility_jongseong[] = {
	0x33, 0x35, 0x36, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E,
	0x3F, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D,
	0x6F, 0x70, 0x82, 0x83, 0x00};

static const char hangul_jamo_jongseong[] = {
	0xAA, 0xAC, 0xAD, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5,   /* to jongseong 0x11A8~0x11FF */
	0xC7, 0xC8, 0xCC, 0xCE, 0xD3, 0xD7, 0xD9, 0xDD, 0xDF, 0xF1, 0xF2, 0x00};

static inline bool is_chosung(UChar src)
{
	int unicode_value1 = 0;
	int unicode_value2 = 0;

	unicode_value1 = (0xFF00 & (src)) >> 8;
	unicode_value2 = (0xFF & (src));

	if (unicode_value1 == 0x31
			&& (0x30 <= unicode_value2 && unicode_value2 <= 0x4e))   /* compatiblility jame */
		return true;

	if (unicode_value1 == 0xA9
			&& (0x60 <= unicode_value2  && unicode_value2 <= 0x7C)) /* jamo Extended-A */
		return true;

	if (unicode_value1 == 0x11
			&& (0x00 <= unicode_value2  && unicode_value2 <= 0x5E))  /* jamo */
		return true;

	return false;
}

bool ctsvc_is_hangul(UChar src)
{
	if ((0x1100 == (src & 0xFF00))       /* korean -Hangul Jamo*/
			|| CTSVC_COMPARE_BETWEEN(CTSVC_JAMO_A_START, src, CTSVC_JAMO_A_END)
			|| CTSVC_COMPARE_BETWEEN(CTSVC_JAMO_B_START, src, CTSVC_JAMO_B_END)
			|| CTSVC_COMPARE_BETWEEN(CTSVC_HAN_C_START, src, CTSVC_HAN_C_END)
			|| CTSVC_COMPARE_BETWEEN(CTSVC_HAN_HALF_START, src, CTSVC_HAN_HALF_END)
			|| CTSVC_COMPARE_BETWEEN(CTSVC_HAN_SYLLABLES_START, src, CTSVC_HAN_SYLLABLES_END))
		return true;
	else
		return FALSE;
}


void ctsvc_hangul_compatibility2jamo(UChar *src)
{
	int unicode_value1 = 0;
	int unicode_value2 = 0;

	unicode_value1 = (0xFF00 & (*src)) >> 8;
	unicode_value2 = (0xFF & (*src));

	/* korean -Hangul Jamo halfwidth*/
	if (CTSVC_COMPARE_BETWEEN(CTSVC_HAN_HALF_START, *src, CTSVC_HAN_HALF_END)) {
		unicode_value1 = 0x31;

		if (unicode_value2 < 0xBF)
			unicode_value2 -= 0x70;
		else if (unicode_value2 < 0xC8)
			unicode_value2 -= 0x73;
		else if (unicode_value2 < 0xD0)
			unicode_value2 -= 0x75;
		else if (unicode_value2 < 0xD8)
			unicode_value2 -= 0x77;
		else
			unicode_value2 -= 0x79;

		(*src) = unicode_value1 << 8 | unicode_value2;
	}

	if (CTSVC_COMPARE_BETWEEN(CTSVC_HAN_C_START, *src, CTSVC_HAN_C_END)) {
		char *pos;
		if ((pos = strchr(hangul_compatibility_choseong, unicode_value2))) {
			unicode_value1 = 0x11;
			unicode_value2 = hangul_jamo_choseong[pos - hangul_compatibility_choseong];
			(*src) = unicode_value1 << 8 | unicode_value2;
		}
		else if ((pos = strchr(hangul_compatibility_jungseong, unicode_value2))) {
			unicode_value1 = 0x11;
			unicode_value2 = hangul_jamo_jungseong[pos - hangul_compatibility_jungseong];
			(*src) = unicode_value1 << 8 | unicode_value2;
		}
		else if ((pos = strchr(hangul_compatibility_jongseong, unicode_value2))) {
			unicode_value1 = 0x11;
			unicode_value2 = hangul_jamo_jongseong[pos - hangul_compatibility_jongseong];
			(*src) = unicode_value1 << 8 | unicode_value2;
		}
	}
}

void ctsvc_hangul_jamo2compatibility(UChar *src)
{
	int unicode_value1 = 0;
	int unicode_value2 = 0;

	unicode_value1 = (0xFF00 & (*src)) >> 8;
	unicode_value2 = (0xFF & (*src));

	/* korean -Hangul Jamo halfwidth*/
	if (CTSVC_COMPARE_BETWEEN(CTSVC_HAN_HALF_START, *src, CTSVC_HAN_HALF_END)) {
		unicode_value1 = 0x31;

		if (unicode_value2 < 0xBF)
			unicode_value2 -= 0x70;
		else if (unicode_value2 < 0xC8)
			unicode_value2 -= 0x73;
		else if (unicode_value2 < 0xD0)
			unicode_value2 -= 0x75;
		else if (unicode_value2 < 0xD8)
			unicode_value2 -= 0x77;
		else
			unicode_value2 -= 0x79;

		(*src) = unicode_value1 << 8 | unicode_value2;
	}

	if (CTSVC_COMPARE_BETWEEN(CTSVC_HAN_J_START, *src, CTSVC_HAN_J_END)) {
		char *pos;
		if ((pos = strchr(hangul_jamo_choseong, unicode_value2))) {
			unicode_value1 = 0x31;
			unicode_value2 = hangul_compatibility_choseong[pos - hangul_jamo_choseong];
			(*src) = unicode_value1 << 8 | unicode_value2;
		}
		else if ((pos = strchr(hangul_jamo_jungseong, unicode_value2))) {
			unicode_value1 = 0x31;
			unicode_value2 = hangul_compatibility_jungseong[pos - hangul_jamo_jungseong];
			(*src) = unicode_value1 << 8 | unicode_value2;
		}
		else if ((pos = strchr(hangul_jamo_jongseong, unicode_value2))) {
			unicode_value1 = 0x31;
			unicode_value2 = hangul_compatibility_jongseong[pos - hangul_jamo_jongseong];
			(*src) = unicode_value1 << 8 | unicode_value2;
		}
	}
}

int ctsvc_get_chosung(const char *src, char *dest, int dest_size)
{
	int32_t size;
	UErrorCode status = 0;
	UChar tmp_result[10];
	UChar result[10];
	int chosung_len=0, count = 0, i=0, j=0;
	int char_len = 0;
	int str_len = strlen(src);
	char temp[dest_size];

	for (i=0;i<str_len;i+=char_len) {
		char char_src[10];
		char_len = ctsvc_check_utf8(src[i]);
		RETVM_IF(char_len <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "check_utf8 Fail");

		memcpy(char_src, &src[i], char_len);
		char_src[char_len] = '\0';

		u_strFromUTF8(tmp_result, array_sizeof(tmp_result), NULL, char_src, -1, &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"u_strFromUTF8() Fail(%s)", u_errorName(status));

		u_strToUpper(tmp_result, array_sizeof(tmp_result), tmp_result, -1, NULL, &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"u_strToLower() Fail(%s)", u_errorName(status));

		size = unorm_normalize(tmp_result, -1, UNORM_NFD, 0,
				(UChar *)result, array_sizeof(result), &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"unorm_normalize(%s) Fail(%s)", src, u_errorName(status));
		ctsvc_extra_normalize(result, size);
		u_strToUTF8(temp, dest_size, &size, result, -1, &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"u_strToUTF8() Fail(%s)", u_errorName(status));
		chosung_len = ctsvc_check_utf8(temp[0]);
		RETVM_IF(chosung_len <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "check_utf8 Fail");
		memcpy(&dest[j], temp, chosung_len);
		j += chosung_len;
		count++;
	}

	dest[j] = '\0';

	return count;
}

int ctsvc_get_korean_search_pattern(const char *src, char *dest, int dest_size)
{
	int32_t size;
	UErrorCode status = 0;
	UChar tmp_result[10];
	UChar result[10];
	int i=0, j=0, count=0;
	int char_len = 0;
	int str_len = strlen(src);

	for (i=0;i<str_len;i+=char_len) {
		char char_src[10];
		char_len = ctsvc_check_utf8(src[i]);
		RETVM_IF(char_len <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "check_utf8 Fail");
		if (char_len == 1 && src[i] == ' ')
			continue;

		memcpy(char_src, &src[i], char_len);
		char_src[char_len] = '\0';

		u_strFromUTF8(tmp_result, array_sizeof(tmp_result), NULL, char_src, -1, &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"u_strFromUTF8() Fail(%s)", u_errorName(status));

		if (is_chosung(tmp_result[0])) {
			ctsvc_hangul_compatibility2jamo(tmp_result);

			u_strToUTF8(&dest[j], dest_size - j, &size, tmp_result, -1, &status);
			RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
					"u_strToUTF8() Fail(%s)", u_errorName(status));
			j += size;
			dest[j] = '*';
			j++;
		}
		else {
			u_strToUpper(tmp_result, array_sizeof(tmp_result), tmp_result, -1, NULL, &status);
			RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
					"u_strToUpper() Fail(%s)", u_errorName(status));
			size = unorm_normalize(tmp_result, -1, UNORM_NFD, 0,
					(UChar *)result, array_sizeof(result), &status);
			RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
					"unorm_normalize(%s) Fail(%s)", src, u_errorName(status));
			ctsvc_extra_normalize(result, size);
			u_strToUTF8(&dest[j], dest_size - j, &size, result, -1, &status);
			RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
					"u_strToUTF8() Fail(%s)", u_errorName(status));
			j += size;
		}
		count++;
	}

	dest[j] = '\0';
	return count;
}

bool ctsvc_is_chosung(const char *src)
{
	int char_len = 0;

	char_len = ctsvc_check_utf8(src[0]);
	if (char_len < 0) return false;   /* invalid value */

	if (char_len == 3) {
		unsigned short tmp;

		tmp = (src[1] << 8) | src[2];
		if (((char)0xE1 == src[0] && CTSVC_COMPARE_BETWEEN(0x8480, tmp, 0x859F)) /* korean -Hangul Jamo*/
				|| ((char)0xE3 == src[0] && CTSVC_COMPARE_BETWEEN(0x84B1, tmp, 0x858E)) /* korean -Hangul Compatibility Jamo */
				|| ((char)0xEA == src[0] && CTSVC_COMPARE_BETWEEN(0xA5A0, tmp, 0xA5BC))) /* korean -Hangul Jamo extended A*/
			return true;
	}
	return false;
}

bool ctsvc_has_chosung(const char *src)
{
	int  i=0;
	int char_len = 0;
	int str_len = strlen(src);

	for (i=0;i<str_len;i+=char_len) {
		char_len = ctsvc_check_utf8(src[i]);
		if (ctsvc_is_chosung(&(src[i])))
			return true;
	}
	return false;
}

static bool __ctsvc_is_hangul(const char *src)
{
	int char_len = 0;

	char_len = ctsvc_check_utf8(src[0]);
	if (char_len <= 0) return false;   /* invalid value */

	if (char_len == 3) {
		switch(src[0]) {
		/*
		 * Hangul Jamo : 0x1100 ~ 0x11FF
		 *  e1 84 80 ~ e1 87 bf
		 */
		case 0xE1:
			switch(src[1]) {
			case 0x84 ... 0x87:
				if (0x80 <= src[2] && src[2] <= 0xBF)
					return true;
				else return false;
			default :
				return false;
			}
			break;

		/*
		 * Hangul Compatibility Jamo : 0x3130 ~ 0x318F
		 *  e3 84 b0 ~ e3 84 bf
		 *  e3 85 80 ~ e3 85 bf
		 *  e3 86 80 ~ e3 86 8f
		 */
		case 0xE3:
			switch(src[1]) {
			case 0x84:
				if (0xB0 <= src[2] && src[2] <= 0xBF)
					return true;
				else return false;
			case 0x85:
				if (0x80 <= src[2] && src[2] <= 0xBF)
					return true;
				else return false;
			case 0x86:
				if (0x80 <= src[2] && src[2] <= 0x8F)
					return true;
				else return false;
			default :
				return false;
			}
			break;

		/*
		 * Hangul Jamo Extended A : 0xA960 ~ 0xA97F
		 *  ea a5 a0  ~ ea a5 bf
		 */
		/*
		 * Hangul syllables : 0xAC00 ~ 0xD7AF
		 *  ea b0 80 ~ ea bf bf
		 */
		case 0xEA:
			switch(src[1]) {
			case 0xA5:
				if (0xA0 <= src[2] && src[2] <= 0xBF)
					return true;
				else return false;
			case 0xB0 ... 0xBF:
				if (0x80 <= src[2] && src[2] <= 0xBF)
					return true;
				else return false;
			default :
				return false;
			}
			break;

		/*
		 * Hangul syllables : 0xAC00 ~ 0xD7AF
		 *  eb 80 80 ~ eb bf bf
		 *  ec 80 80 ~ ec bf bf
		 */
		case 0xEB ... 0xEC:
			switch(src[1]) {
			case 0x80 ... 0xBF:
				if (0x80 <= src[2] && src[2] <= 0xBF)
					return true;
				else return false;
				break;
			default :
				return false;
			}
			break;

		/*
		 * Hangul syllables : 0xAC00 ~ 0xD7AF
		 *  ed 80 80 ~ ed 9e af
		 */
		/*
		 * Hangul Jamo Extended B : 0xD7B0 ~ 0xD7FF
		 *  ed 9e b0 ~ ed 9f bf
		 */
		case 0xED:
			switch(src[1]) {
			case 0x80 ... 0x9F:
				if (0x80 <= src[2] && src[2] <= 0xBF)
					return true;
				else return false;
			default :
				return false;
			}
			break;

		/*
		 * Hangul halfwidth : 0xFFA0 ~ 0xFFDC
		 *  ef be a0 ~ ef bf 9c
		 */
		case 0xEF:
			switch(src[1]) {
			case 0xBE:
				if (0xA0 <= src[2] && src[2] <= 0xBF)
					return true;
				else return false;
			case 0xbf:
				if (0x80 <= src[2] && src[2] <= 0x9C)
					return true;
				else return false;
			default :
				return false;
			}
			break;
		default:
			return false;
		}
	}
	return false;
}

bool ctsvc_has_korean(const char *src)
{
	int  i=0;
	int char_len = 0;
	int str_len = strlen(src);

	for (i=0;i<str_len;i+=char_len) {
		char_len = ctsvc_check_utf8(src[i]);
		RETV_IF(CONTACTS_ERROR_INVALID_PARAMETER == char_len, false);
		if (__ctsvc_is_hangul(&(src[i])))
			return true;
	}
	return false;
}


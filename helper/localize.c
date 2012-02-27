/*
 * Contacts Service Helper
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
#include "internal.h"
#include "cts-normalize.h"
#include "localize.h"

/* korean -Hangul Jamo extended A*/
#define CTS_JAMO_A_START (UChar)0xA960
#define CTS_JAMO_A_END (UChar)0xA97F

/* korean -Hangul Jamo extended B*/
#define CTS_JAMO_B_START (UChar)0xD7B0
#define CTS_JAMO_B_END (UChar)0xD7FF

/* korean -Hangul Compatability */
#define CTS_HAN_C_START (UChar)0x3130
#define CTS_HAN_C_END (UChar)0x318F

/* korean -Hangul halfwidth */
#define CTS_HAN_HALF_START (UChar)0xFFA0
#define CTS_HAN_HALF_END (UChar)0xFFDC

static const char hangul_compatibility_choseong[] = {
	0x31, 0x32, 0x34, 0x37, 0x38, 0x39, 0x40, 0x41,
	0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x65, 0x66, 0x6E,
	0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80,
	0x81, 0x84, 0x85, 0x86, 0x00};
static const unsigned char hangul_jamo_choseong[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x1A, 0x06, 0x07,		// to choseong 0x1100~0x115F
	0x08, 0x21, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x14, 0x15, 0x1C, 0x1D, 0x1E, 0x20,
	0x22, 0x23, 0x27, 0x29, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x32, 0x36, 0x40, 0x47, 0x4C, 0x57, 0x58, 0x59, 0x00};

static const char hangul_compatibility_jungseong[] = {
	0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
	0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E,
	0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x87, 0x88,
	0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x00};
static const unsigned char hangul_jamo_jungseong[] = {
	0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,	// to jungseong 0x1160~0x11A7
	0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
	0x73, 0x74, 0x75, 0x60, 0x84, 0x85, 0x88, 0x91, 0x92,
	0x94, 0x9E, 0xA1, 0x00};

static const char hangul_compatibility_jongseong[] = {
	0x33, 0x35, 0x36, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E,
	0x3F, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D,
	0x6F, 0x70, 0x82, 0x83, 0x00};
static const unsigned char hangul_jamo_jongseong[] = {
	0xAA, 0xAC, 0xAD, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5,	// to jongseong 0x11A8~0x11FF
	0xC7, 0xC8, 0xCC, 0xCE, 0xD3, 0xD7, 0xD9, 0xDF, 0xF1, 0xF2, 0x00};

static inline bool is_hangul(UChar src)
{
	if ((0x1100 == (src & 0xFF00))       /* korean -Hangul Jamo*/
			|| CTS_COMPARE_BETWEEN(CTS_JAMO_A_START, src, CTS_JAMO_A_END)
			|| CTS_COMPARE_BETWEEN(CTS_JAMO_B_START, src, CTS_JAMO_B_END)
			|| CTS_COMPARE_BETWEEN(CTS_HAN_C_START, src, CTS_HAN_C_END)
			|| CTS_COMPARE_BETWEEN(CTS_HAN_HALF_START, src, CTS_HAN_HALF_END))
		return true;
	else
		return FALSE;
}

static inline void hangul_compatibility2jamo(UChar *src)
{
	int unicode_value1 = 0;
	int unicode_value2 = 0;

	unicode_value1 = (0xFF00 & (*src)) >> 8;
	unicode_value2 = (0xFF & (*src));

	/* korean -Hangul Jamo halfwidth*/
	if (CTS_COMPARE_BETWEEN(CTS_HAN_HALF_START, *src, CTS_HAN_HALF_END)) {
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

	if (CTS_COMPARE_BETWEEN(CTS_HAN_C_START, *src, CTS_HAN_C_END))
	{
		char *pos;
		if (NULL != (pos = strchr(hangul_compatibility_choseong, unicode_value2)))
		{
			unicode_value1 = 0x11;
			unicode_value2 = hangul_jamo_choseong[pos - hangul_compatibility_choseong];
			(*src) = unicode_value1 << 8 | unicode_value2;
		}
		else if (NULL != (pos = strchr(hangul_compatibility_jungseong, unicode_value2)))
		{
			unicode_value1 = 0x11;
			unicode_value2 = hangul_jamo_jungseong[pos - hangul_compatibility_jungseong];
			(*src) = unicode_value1 << 8 | unicode_value2;
		}
		else if (NULL != (pos = strchr(hangul_compatibility_jongseong, unicode_value2)))
		{
			unicode_value1 = 0x11;
			unicode_value2 = hangul_jamo_jongseong[pos - hangul_compatibility_jongseong];
			(*src) = unicode_value1 << 8 | unicode_value2;
		}
	}
}

int helper_check_language(UChar *word)
{
	int type;

	if (CTS_COMPARE_BETWEEN('0', word[0], '9')) {
		type = CTS_LANG_NUMBER;
	}
	else if (CTS_COMPARE_BETWEEN(0x41, word[0], 0x7A)) {  /* english */
		type = CTS_LANG_ENGLISH;
	}
	else if (is_hangul(word[0])){
		type = CTS_LANG_KOREAN;
	}
	else
		type = CTS_LANG_OTHERS;

	return type;
}

void helper_extra_normalize(UChar *word, int32_t word_size)
{
	int i;
	for (i=0;i<word_size;i++) {
		if (is_hangul(word[i])) {
			hangul_compatibility2jamo(&word[i]);
		}
	}
}

int helper_get_language_type(const char *system_lang)
{
	int type;

	h_retv_if(NULL == system_lang, CTS_LANG_OTHERS);

	if (!strncmp(system_lang, "ko", sizeof("ko") - 1))
		type = CTS_LANG_KOREAN;
	else if (!strncmp(system_lang, "en", sizeof("en") - 1))
		type = CTS_LANG_ENGLISH;
	else
		type = CTS_LANG_OTHERS;

	return type;
}


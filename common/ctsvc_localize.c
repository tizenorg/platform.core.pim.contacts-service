/*
 * Contacts Service Helper
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

#define CTSVC_COMPARE_BETWEEN(left_range, value, right_range) (((left_range) <= (value)) && ((value) <= (right_range)))

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


/* japanese - katakana */
#define CTSVC_JAPANESE_KATAKANA_START 	0x30A0
#define CTSVC_JAPANESE_KATAKANA_END 	0x30FF

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

int ctsvc_check_utf8(char c)
{
	if ((c & 0xff) < (128 & 0xff))
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
		return CONTACTS_ERROR_INVALID_PARAMETER;
}

static inline bool is_chosung(UChar src)
{
	int unicode_value1 = 0;
	int unicode_value2 = 0;

	unicode_value1 = (0xFF00 & (src)) >> 8;
	unicode_value2 = (0xFF & (src));

	if (unicode_value1 == 0x31
			&& (unicode_value2 >= 0x30 && unicode_value2 <= 0x4e))
		return true;
	return false;
}

static inline bool is_hangul(UChar src)
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

static inline bool is_japanese(UChar src)
{
	if (CTSVC_COMPARE_BETWEEN(CTSVC_JAPANESE_KATAKANA_START, src, CTSVC_JAPANESE_KATAKANA_END )
			|| CTSVC_COMPARE_BETWEEN(CTSVC_JAPANESE_KATAKANA_PHONETIC_EXTENSIONS_START, src, CTSVC_JAPANESE_KATAKANA_PHONETIC_EXTENSIONS_END )
			|| CTSVC_COMPARE_BETWEEN(CTSVC_JAPANESE_HALFWIDTH_AND_FULLWIDTH_FORMS_START, src, CTSVC_JAPANESE_HALFWIDTH_AND_FULLWIDTH_FORMS_END )
			|| CTSVC_COMPARE_BETWEEN(CTSVC_JAPANESE_HIRAGANA_START, src, CTSVC_JAPANESE_HIRAGANA_END ))
		return true;
	else
		return false;
}

static inline void hangul_compatibility2jamo(UChar *src)
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
		if (NULL != (pos = strchr(hangul_compatibility_choseong, unicode_value2))) {
			unicode_value1 = 0x11;
			unicode_value2 = hangul_jamo_choseong[pos - hangul_compatibility_choseong];
			(*src) = unicode_value1 << 8 | unicode_value2;
		}
		else if (NULL != (pos = strchr(hangul_compatibility_jungseong, unicode_value2))) {
			unicode_value1 = 0x11;
			unicode_value2 = hangul_jamo_jungseong[pos - hangul_compatibility_jungseong];
			(*src) = unicode_value1 << 8 | unicode_value2;
		}
		else if (NULL != (pos = strchr(hangul_compatibility_jongseong, unicode_value2))) {
			unicode_value1 = 0x11;
			unicode_value2 = hangul_jamo_jongseong[pos - hangul_compatibility_jongseong];
			(*src) = unicode_value1 << 8 | unicode_value2;
		}
	}
}

int ctsvc_convert_japanese_to_hiragana_unicode(UChar *src, UChar *dest, int dest_size)
{
	int i, j = 0, len = 0;

	len = u_strlen(src);

	for(i = 0; i < len; i++) {
		int unicode_value1 = 0;
		int unicode_value2 = 0;

		unicode_value2 = (0xFF & (src[i]));

		if (CTSVC_COMPARE_BETWEEN(CTSVC_JAPANESE_KATAKANA_START, src[i], CTSVC_JAPANESE_KATAKANA_END)) {
			unicode_value1 = 0x30;
			if ((unicode_value2 >= 0xa1 && unicode_value2 <= 0xef )
					|| (unicode_value2 == 0xF2 || unicode_value2 == 0xF3) ) {
				unicode_value2 -= 0x60;
				dest[j] = unicode_value1 << 8 | unicode_value2;
			}
			else {
				dest[j] = src[i];
			}
		}
		else if (CTSVC_COMPARE_BETWEEN(CTSVC_JAPANESE_HALFWIDTH_AND_FULLWIDTH_FORMS_START, src[i], CTSVC_JAPANESE_HALFWIDTH_AND_FULLWIDTH_FORMS_END)) {
			unicode_value1 = 0x30;
			if (i+1 < len && (0xFF & (src[i+1])) == 0x9E
					&& unicode_value2 >= 0x76 && unicode_value2 <= 0x89) {
				unicode_value2 = japanese_halfwidth_katakana_sonant_to_hiragana[unicode_value2 - 0x76];
				dest[j] = unicode_value1 << 8 | unicode_value2;
				i++;
			}
			else if (i+1 < len && (0xFF & (src[i])) == 0x9F
					&& unicode_value2 >= 0x8a && unicode_value2 <= 0x8e) {
				unicode_value2 = japanese_halfwidth_katakana_half_dullness_to_hiragana[unicode_value2 - 0x8a];
				dest[j] = unicode_value1 << 8 | unicode_value2;
				i++;
			}
			else if (unicode_value2 >= 0x66 && unicode_value2 <= 0x9d) {
				unicode_value2 = japanese_halfwidth_katakana_to_hiragana[unicode_value2 - 0x66];
				dest[j] = unicode_value1 << 8 | unicode_value2;
			}
			else {
				dest[j] = src[i];
			}
		}
		else if (CTSVC_COMPARE_BETWEEN(CTSVC_ASCII_HALFWIDTH_AND_FULLWIDTH_FORMS_START, src[i], CTSVC_ASCII_HALFWIDTH_AND_FULLWIDTH_FORMS_END)) {
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
	int ret = CONTACTS_ERROR_NONE;
	UChar *tmp_result = NULL;
	UChar *result = NULL;
	UErrorCode status = 0;
	int32_t size;

	u_strFromUTF8(NULL, 0, &size, src, strlen(src), &status);
	if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR) {
		CTS_ERR("u_strFromUTF8 to get the dest length Failed(%s)", u_errorName(status));
		ret = CONTACTS_ERROR_SYSTEM;
		goto DATA_FREE;
	}
	status = U_ZERO_ERROR;
	tmp_result = calloc(1, sizeof(UChar) * (size + 1));
	u_strFromUTF8(tmp_result, size + 1, NULL, src, -1, &status);
	if (U_FAILURE(status)){
		CTS_ERR("u_strFromUTF8 Failed(%s)", u_errorName(status));
		ret = CONTACTS_ERROR_SYSTEM;
		goto DATA_FREE;
	}

	result = calloc(1, sizeof(UChar) * (size + 1));

	ctsvc_convert_japanese_to_hiragana_unicode(tmp_result, result, size + 1 );

	u_strToUTF8(NULL, 0, &size, result, -1, &status);
	if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR) {
		CTS_ERR("u_strToUTF8 to get the dest length Failed(%s)", u_errorName(status));
		ret = CONTACTS_ERROR_SYSTEM;
		goto DATA_FREE;
	}

	status = U_ZERO_ERROR;
	*dest = calloc(1, sizeof(char)*(size+1));

	u_strToUTF8(*dest, size + 1, &size, result, -1, &status);

	if (U_FAILURE(status) ) {
		CTS_ERR("u_strToUTF8 Failed(%s)", u_errorName(status));
		ret =  CONTACTS_ERROR_SYSTEM;
		goto DATA_FREE;
	}

DATA_FREE:
	free(tmp_result);
	free(result);

	return ret;
}

int ctsvc_check_language(UChar *word)
{
	int type;

	int unicode_value1 = 0;
	int unicode_value2 = 0;

	unicode_value1 = (0xFF00 & (*word)) >> 8;
	unicode_value2 = (0xFF & (*word));

	CTS_VERBOSE("0x%x%x", unicode_value1, unicode_value2);

	if (u_isdigit(word[0])) {
		type = CTSVC_LANG_NUMBER;
	}
	else if (u_isalpha(word[0])) {
		// refer to the uchar.h
		// #define U_GC_L_MASK  (U_GC_LU_MASK|U_GC_LL_MASK|U_GC_LT_MASK|U_GC_LM_MASK|U_GC_LO_MASK)
		// U_GC_LU_MASK : U_UPPERCASE_LETTER
		// U_GC_LL_MASK : U_LOWERCASE_LETTER
		// U_GC_LT_MASK : U_TITLECASE_LETTER
		// U_GC_LM_MASK : U_MODIFIER_LETTER
		// U_GC_LO_MASK : U_OTHER_LETTER

		UBlockCode code = ublock_getCode(word[0]);
		CTS_VERBOSE("Character unicode block is %d", code);

		switch (code){
		//english
		case UBLOCK_BASIC_LATIN:							// = 1, /*[0000]*/
		case UBLOCK_LATIN_1_SUPPLEMENT:					// =2, /*[0080]*/
		case UBLOCK_LATIN_EXTENDED_A:						// =3, /*[0100]*/
		case UBLOCK_LATIN_EXTENDED_B:						// =4, /*[0180]*/
		case UBLOCK_LATIN_EXTENDED_ADDITIONAL:			// =38, /*[1E00]*/
			type = CTSVC_LANG_ENGLISH;
			// type = CTSVC_LANG_CATALAN;	// ca, Spain - Catalan
			// type = CTSVC_LANG_GERMAN:	 // de, Germany - German
			// type = CTSVC_LANG_BASQUE:	// eu, Spain - Basque
			// type = CTSVC_LANG_DUTCH;		// nl_Nl, Netherlands Dutch
			// type = CTSVC_LANG_FRENCH; 	// fr_CA, fr_FR
			// type = CTSVC_LANG_ITALIAN: 	// it_IT, Italy - Italian
			// type = CTSVC_LANG_PORTUGUESE:	 // pt_BR, pt_PT, Portugal
			// type = CTSVC_LANG_SPANISH: // es_ES, es_US, El Salvador - Spanish
			// type =  CTSVC_LANG_NORWAY: // nb, Norway
			// type = CTSVC_LANG_DANISH: // da, Denmark - Danish
			// type = CTSVC_LANG_AZERBAIJAN: // az, Azerbaijan
			// type = CTSVC_LANG_ROMANIA: // ro, Romania
			// type = CTSVC_LANG_CZECH: // cs, Czech Republic - Czech
			// type = CTSVC_LANG_ESTONIAN: // et, Estonia - Estonian
			// type = CTSVC_LANG_FINNISH: // fi, Finland - Finnish
			// type = CTSVC_LANG_IRISH: // ga, Ireland - Irish
			// type = CTSVC_LANG_GALICIAN: // gl, Spain - Galician
			// type = CTSVC_LANG_HUNGARIAN: // hu, Hungary - Hungarian
			// type = CTSVC_LANG_SWEDISH: // sv, Finland - Swedish
			// type = CTSVC_LANG_SLOVENIAN: // sl, Slovenia - Slovenian
			// type = CTSVC_LANG_SLOVAK: // sk, Slovakia - Slovak
			// type = CTSVC_LANG_LITHUANIAN: // lt, Lithuania - Lithuanian
			// type = CTSVC_LANG_POLISH: // pl, Polish
			// type = CTSVC_LANG_LATVIAN: // lv, Latvia - Latvian
			// type = CTSVC_LANG_CROATIAN: // hr, Bosnia and Herzegovina - Croatian
			// type = CTSVC_LANG_ICELANDIC: // is, Iceland - Icelandic
			break;

		//korean
		case UBLOCK_HANGUL_JAMO:						// =30, /*[1100]*/
		case UBLOCK_HANGUL_COMPATIBILITY_JAMO:		// =65, /*[3130]*/
		case UBLOCK_HANGUL_SYLLABLES:				// =74, /*[AC00]*/
		case UBLOCK_HANGUL_JAMO_EXTENDED_A:		// = 180, /*[A960]*/
		case UBLOCK_HANGUL_JAMO_EXTENDED_B:		// = 185, /*[D7B0]*/
			type = CTSVC_LANG_KOREAN;
			break;

		// chainese
		case UBLOCK_CJK_RADICALS_SUPPLEMENT:			 //=58, /*[2E80]*/
		case UBLOCK_CJK_SYMBOLS_AND_PUNCTUATION:		 //=61, /*[3000]*/
		case UBLOCK_ENCLOSED_CJK_LETTERS_AND_MONTHS:  //=68, /*[3200]*/
		case UBLOCK_CJK_STROKES:							 // =130, /*[31C0]*/
		case UBLOCK_CJK_COMPATIBILITY:						 // =69, /*[3300]*/
		case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A: //=70, /*[3400]*/
		case UBLOCK_CJK_UNIFIED_IDEOGRAPHS:				 //=71, /*[4E00]*/
		case UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS:		 //=79, /*[F900]*/
		case UBLOCK_CJK_COMPATIBILITY_FORMS:				 //=83, /*[FE30]*/
		case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B :       // =94, /*[20000]*/
		case UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT:         // =95, /*[2F800]*/
		case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C:         // =197, /*[2A700]*/
		case UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D:         // =209, /*[2B740]*/
			type = CTSVC_LANG_CHINESE;
			break;

		// japanese
		case UBLOCK_HIRAGANA:								// =62, /*[3040]*/
		case UBLOCK_KATAKANA:								// =63, /*[30A0]*/
		case UBLOCK_KATAKANA_PHONETIC_EXTENSIONS:		// =107, /*[31F0]*/
		case UBLOCK_JAVANESE:								// =181, /*[A980]*/
			type = CTSVC_LANG_JAPANESE;
			break;

		case UBLOCK_GREEK:						// =8, /*[0370]*/
		case UBLOCK_GREEK_EXTENDED:				// =39, /*[1F00]*/
			type = CTSVC_LANG_GREEK;
			break;

		case UBLOCK_CYRILLIC:								// =9, /*[0400]*/
		case UBLOCK_CYRILLIC_EXTENDED_A:					// = 158, /*[2DE0]*/
		case UBLOCK_CYRILLIC_EXTENDED_B:					// = 160, /*[A640]*/
		case UBLOCK_CYRILLIC_SUPPLEMENTARY:				// = 97, UBLOCK_CYRILLIC_SUPPLEMENT = UBLOCK_CYRILLIC_SUPPLEMENTARY, /*[0500]*/
			type = CTSVC_LANG_RUSSIAN;
			// type = CTSVC_LANG_BULGARIAN: // bg, Bulgaria - Bulgarian
			// type = CTSVC_LANG_MACEDONIA: // mk, Macedonia
			// type = CTSVC_LANG_KAZAKHSTAN: // kk, Kazakhstan
			// type = CTSVC_LANG_SERBIAN: // sr, Serbia - Serbian
			// type = CTSVC_LANG_UKRAINE: // uk, Ukraine
			break;

		case UBLOCK_ARMENIAN:						//=10, /*[0530]*/
			type = CTSVC_LANG_ARMENIAN;
			break;
		case UBLOCK_ARABIC:						//=12, /*[0600]*/
			type = CTSVC_LANG_ARABIC;
			break;
		case UBLOCK_DEVANAGARI:					// =15, /*[0900]*/
		case UBLOCK_DEVANAGARI_EXTENDED:		// = 179, /*[A8E0]*/:
			type = CTSVC_LANG_HINDI;
			break;
		case UBLOCK_GEORGIAN:						//=29, /*[10A0]*/
		case UBLOCK_GEORGIAN_SUPPLEMENT:		// = 135, /*[2D00]*/
			type = CTSVC_LANG_GEORGIAN;
			break;
		case UBLOCK_OLD_TURKIC:					// = 191, /*[10C00]*/
			type = CTSVC_LANG_TURKISH;
			break;
		case UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS:		// =87, /*[FF00]*/	// hangul : FFA0 ~ FFDC
		{
			if (CTSVC_COMPARE_BETWEEN((UChar)0xFF21, word[0], (UChar)0xFF3A)
				|| CTSVC_COMPARE_BETWEEN((UChar)0xFF41, word[0], (UChar)0xFF5A))
				type = CTSVC_LANG_ENGLISH;
			else if (CTSVC_COMPARE_BETWEEN((UChar)0xFF10, word[0], (UChar)0xFF19))
				type = CTSVC_LANG_NUMBER;
			else if (CTSVC_COMPARE_BETWEEN((UChar)0xFF65, word[0], (UChar)0xFF9F))
				type = CTSVC_LANG_JAPANESE;
			else if (CTSVC_COMPARE_BETWEEN((UChar)0xFFA0, word[0], (UChar)0xFFDC))
				type = CTSVC_LANG_KOREAN;
			else
				type = CTSVC_LANG_OTHERS;
			break;
		}
		default:
			type = CTSVC_LANG_OTHERS;
			break;
		}
	}
	else
		type = CTSVC_LANG_OTHERS;

	CTS_VERBOSE("language type = %d", type);
	return type;
}

#define array_sizeof(a) (sizeof(a) / sizeof(a[0]))

int ctsvc_check_language_type(const char *src)
{
	int length = 0;
	char temp[10] = {0};
	UChar tmp_result[2];
	UChar result[10];
	UErrorCode status = 0;
	int32_t size;

	if (src && src[0]) {
		length = ctsvc_check_utf8(src[0]);
		RETVM_IF(length <= 0, CONTACTS_ERROR_INTERNAL, "check_utf8 failed");

		strncpy(temp, src, length);

		CTS_VERBOSE("temp(%s) src(%s) length(%d)", temp, src, length);

		u_strFromUTF8(tmp_result, array_sizeof(tmp_result), NULL, temp, -1, &status);
			RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
					"u_strFromUTF8() Failed(%s)", u_errorName(status));

		u_strToUpper(tmp_result, array_sizeof(tmp_result), tmp_result, -1, NULL, &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"u_strToLower() Failed(%s)", u_errorName(status));

		size = unorm_normalize(tmp_result, -1, UNORM_NFD, 0,
				(UChar *)result, array_sizeof(result), &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"unorm_normalize(%s) Failed(%s)", src, u_errorName(status));

		CTS_VERBOSE("0x%x%x", (0xFF00 & (tmp_result[0])) >> 8,  (0xFF & (tmp_result[0])));

		return ctsvc_check_language(result);
	}

	return CONTACTS_ERROR_INVALID_PARAMETER;
}

int ctsvc_get_sort_type_from_language(int language)
{
	switch(language) {
	case CTSVC_LANG_CHINESE:
		return CTSVC_SORT_CJK;
	case CTSVC_LANG_JAPANESE:
		return CTSVC_SORT_JAPANESE;
	case CTSVC_LANG_KOREAN:
		return CTSVC_SORT_KOREAN;
	case CTSVC_LANG_ENGLISH:
		return CTSVC_SORT_WESTERN;
	case CTSVC_LANG_NUMBER:
		return CTSVC_SORT_NUMBER;
	case CTSVC_LANG_RUSSIAN:
	case CTSVC_LANG_BULGARIAN:
	case CTSVC_LANG_MACEDONIA:
	case CTSVC_LANG_KAZAKHSTAN:
	case CTSVC_LANG_SERBIAN:
	case CTSVC_LANG_UKRAINE:
		return CTSVC_SORT_CYRILLIC;
	case CTSVC_LANG_ARMENIAN:
		return CTSVC_SORT_ARMENIAN;
	case CTSVC_LANG_GREEK:
		return CTSVC_SORT_GREEK;
	case CTSVC_LANG_ARABIC:
		return CTSVC_SORT_ARABIC;
	case CTSVC_LANG_HINDI:
		return CTSVC_SORT_DEVANAGARI;
	case CTSVC_LANG_GEORGIAN:
		return CTSVC_SORT_GEORGIAN;
	case CTSVC_LANG_TURKISH:
		return CTSVC_SORT_TURKISH;
	case CTSVC_LANG_OTHERS:
		return CTSVC_SORT_OTHERS;
	default:
		return CTSVC_SORT_WESTERN;
	}
}

int ctsvc_get_name_sort_type(const char *src)
{
	UErrorCode status = 0;
	UChar tmp_result[10];
	int ret = CTSVC_SORT_OTHERS;
	int char_len = 0;
	int language_type;
	char char_src[10];

	char_len = ctsvc_check_utf8(src[0]);
	RETVM_IF(char_len <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "check_utf8 failed");

	memcpy(char_src, &src[0], char_len);
	char_src[char_len] = '\0';

	u_strFromUTF8(tmp_result, array_sizeof(tmp_result), NULL, char_src, -1, &status);
	RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
			"u_strFromUTF8() Failed(%s)", u_errorName(status));

	language_type = ctsvc_check_language(tmp_result);
	ret = ctsvc_get_sort_type_from_language(language_type);

	return ret;
}

void ctsvc_extra_normalize(UChar *word, int32_t word_size)
{
	int i;
	for (i=0;i<word_size;i++) {
		// FF00 ~ FF60, FFE0~FFE6 : fullwidth -> halfwidth
		if (CTSVC_COMPARE_BETWEEN((UChar)0xFF00, word[i], (UChar)0xFF60)) {
			int unicode_value1 = 0;
			int unicode_value2 = 0;
			unicode_value1 = 0x0;
			unicode_value2 = (0xFF & word[i]) + 0x20;
			word[i] = unicode_value1 << 8 | unicode_value2;
		}
		else if (is_hangul(word[i])) {
			hangul_compatibility2jamo(&word[i]);
		}
	}
}

const char *ctsvc_get_language_locale(int lang)
{
	switch(lang)
	{
	case CTSVC_LANG_AZERBAIJAN: // az, Azerbaijan
		return "az";
	case CTSVC_LANG_ARABIC: // ar, Bahrain - Arabic
		return "ar";
	case CTSVC_LANG_BULGARIAN: // bg, Bulgaria - Bulgarian
		return "bg";
	case CTSVC_LANG_CATALAN: // ca, Spain - Catalan
		return "ca";
	case CTSVC_LANG_CZECH: // cs, Czech Republic - Czech
		return "cs";
	case CTSVC_LANG_DANISH: // da, Denmark - Danish
		return "da";
	case CTSVC_LANG_GERMAN: // de, Germany - German
		return "de";
	case CTSVC_LANG_GREEK: // el, Greece - Greek
		return "el";
	case CTSVC_LANG_ENGLISH: // en, en_PH, en_US
		return "en";
	case CTSVC_LANG_SPANISH: // es_ES, es_US, El Salvador - Spanish
		return "es";
	case CTSVC_LANG_ESTONIAN: // et, Estonia - Estonian
		return "et";
	case CTSVC_LANG_BASQUE: // eu, Spain - Basque
		return "eu";
	case CTSVC_LANG_FINNISH: // fi, Finland - Finnish
		return "fi";
	case CTSVC_LANG_FRENCH: // fr_CA, fr_FR
		return "fr";
	case CTSVC_LANG_IRISH: // ga, Ireland - Irish
		return "ga";
	case CTSVC_LANG_GALICIAN: // gl, Spain - Galician
		return "gl";
	case CTSVC_LANG_HINDI: // hi, India - Hindi
		return "hi";
	case CTSVC_LANG_CROATIAN: // hr, Bosnia and Herzegovina - Croatian
		return "hr";
	case CTSVC_LANG_HUNGARIAN: // hu, Hungary - Hungarian
		return "hu";
	case CTSVC_LANG_ARMENIAN: // hy, Armenia - Armenian
		return "hy";
	case CTSVC_LANG_ICELANDIC: // is, Iceland - Icelandic
		return "is";
	case CTSVC_LANG_ITALIAN: // it_IT, Italy - Italian
		return "it";
	case CTSVC_LANG_JAPANESE: // ja_JP, japan
		return "ja";
	case CTSVC_LANG_GEORGIAN: // ka, Georgia - Georgian
		return "ka";
	case CTSVC_LANG_KAZAKHSTAN: // kk, Kazakhstan
		return "kk";
	case CTSVC_LANG_KOREAN: // ko, ko_KR
		return "ko";
	case CTSVC_LANG_LITHUANIAN: // lt, Lithuania - Lithuanian
		return "lt";
	case CTSVC_LANG_LATVIAN: // lv, Latvia - Latvian
		return "lv";
	case CTSVC_LANG_MACEDONIA: // mk, Macedonia
		return "mk";
	case CTSVC_LANG_NORWAY: // nb, Norway
		return "nb";
	case CTSVC_LANG_DUTCH: // nl_Nl, Netherlands Dutch
		return "nl";
	case CTSVC_LANG_POLISH: // pl, Polish
		return "pl";
	case CTSVC_LANG_PORTUGUESE: // pt_BR, pt_PT, Portugal
		return "pt";
	case CTSVC_LANG_ROMANIA: // ro, Romania
		return "ro";
	case CTSVC_LANG_RUSSIAN: // ru_RU, Russia
		return "ru";
	case CTSVC_LANG_SLOVAK: // sk, Slovakia - Slovak
		return "sk";
	case CTSVC_LANG_SLOVENIAN: // sl, Slovenia - Slovenian
		return "sl";
	case CTSVC_LANG_SERBIAN: // sr, Serbia - Serbian
		return "sr";
	case CTSVC_LANG_SWEDISH: // sv, Finland - Swedish
		return "sv";
	case CTSVC_LANG_TURKISH: // tr_TR, Turkey - Turkish
		return "tr";
	case CTSVC_LANG_UKRAINE: // uk, Ukraine
		return "uk";
	case CTSVC_LANG_CHINESE: // zh_CN, zh_HK, zh_SG, zh_TW
		return "zh";
	}

	return "";
}

int ctsvc_get_language_type(const char *system_lang)
{
	// refer to the VCONFKEY_LANGSET
	int type;

	RETV_IF(NULL == system_lang, CTSVC_LANG_OTHERS);

	// az, Azerbaijan
	if (!strncmp(system_lang, "az", strlen("az")))
		type = CTSVC_LANG_AZERBAIJAN;
	// ar, Bahrain - Arabic
	else if (!strncmp(system_lang, "ar", strlen("ar")))
		type = CTSVC_LANG_ARABIC;
	// bg, Bulgaria - Bulgarian
	else if (!strncmp(system_lang, "bg", strlen("bg")))
		type = CTSVC_LANG_BULGARIAN;
	// ca, Spain - Catalan
	else if (!strncmp(system_lang, "ca", strlen("ca")))
		type = CTSVC_LANG_CATALAN;
	// cs, Czech Republic - Czech
	else if (!strncmp(system_lang, "cs", strlen("cs")))
		type = CTSVC_LANG_CZECH;
	// da, Denmark - Danish
	else if (!strncmp(system_lang, "da", strlen("da")))
		type = CTSVC_LANG_DANISH;
	// de, Germany - German
	else if (!strncmp(system_lang, "de", strlen("de")))
		type = CTSVC_LANG_GERMAN;
	// el, Greece - Greek
	else if (!strncmp(system_lang, "el", strlen("el")))
		type = CTSVC_LANG_GREEK;
	// en, en_PH, en_US
	else if (!strncmp(system_lang, "en", strlen("en")))
		type = CTSVC_LANG_ENGLISH;
	// es_ES, es_US, El Salvador - Spanish
	else if (!strncmp(system_lang, "es", strlen("es")))
		type = CTSVC_LANG_SPANISH;
	// et, Estonia - Estonian
	else if (!strncmp(system_lang, "et", strlen("et")))
		type = CTSVC_LANG_ESTONIAN;
	// eu, Spain - Basque
	else if (!strncmp(system_lang, "eu", strlen("eu")))
		type = CTSVC_LANG_BASQUE;
	// fi, Finland - Finnish
	else if (!strncmp(system_lang, "fi", strlen("fi")))
		type = CTSVC_LANG_FINNISH;
	// fr_CA, fr_FR
	else if (!strncmp(system_lang, "fr", strlen("fr")))
		type = CTSVC_LANG_FRENCH;
	// ga, Ireland - Irish
	else if (!strncmp(system_lang, "ga", strlen("ga")))
		type = CTSVC_LANG_IRISH;
	// gl, Spain - Galician
	else if (!strncmp(system_lang, "gl", strlen("gl")))
		type = CTSVC_LANG_GALICIAN;
	// hi, India - Hindi
	else if (!strncmp(system_lang, "hi", strlen("hi")))
		type = CTSVC_LANG_HINDI;
	// hr, Bosnia and Herzegovina - Croatian
	else if (!strncmp(system_lang, "hr", strlen("hr")))
		type = CTSVC_LANG_CROATIAN;
	// hu, Hungary - Hungarian
	else if (!strncmp(system_lang, "hu", strlen("hu")))
		type = CTSVC_LANG_HUNGARIAN;
	// hy, Armenia - Armenian
	else if (!strncmp(system_lang, "hy", strlen("hy")))
		type = CTSVC_LANG_ARMENIAN;
	// is, Iceland - Icelandic
	else if (!strncmp(system_lang, "is", strlen("is")))
		type = CTSVC_LANG_ICELANDIC;
	// it_IT, Italy - Italian
	else if (!strncmp(system_lang, "it", strlen("it")))
		type = CTSVC_LANG_ITALIAN;
	// ja_JP, japan
	else if (!strncmp(system_lang, "ja", strlen("ja")))
		type = CTSVC_LANG_JAPANESE;
	// ka, Georgia - Georgian
	else if (!strncmp(system_lang, "ka", strlen("ka")))
		type = CTSVC_LANG_GEORGIAN;
	// kk, Kazakhstan
	else if (!strncmp(system_lang, "kk", strlen("kk")))
		type = CTSVC_LANG_KAZAKHSTAN;
	// ko, ko_KR
	else if (!strncmp(system_lang, "ko", strlen("ko")))
		type = CTSVC_LANG_KOREAN;
	// lt, Lithuania - Lithuanian
	else if (!strncmp(system_lang, "lt", strlen("lt")))
		type = CTSVC_LANG_LITHUANIAN;
	// lv, Latvia - Latvian
	else if (!strncmp(system_lang, "lv", strlen("lv")))
		type = CTSVC_LANG_LATVIAN;
	// mk, Macedonia
	else if (!strncmp(system_lang, "mk", strlen("mk")))
		type = CTSVC_LANG_MACEDONIA;
	// nb, Norway
	else if (!strncmp(system_lang, "nb", strlen("nb")))
		type = CTSVC_LANG_NORWAY;
	// nl_Nl, Netherlands Dutch
	else if (!strncmp(system_lang, "nl", strlen("nl")))
		type = CTSVC_LANG_DUTCH;
	// pl, Polish
	else if (!strncmp(system_lang, "pl", strlen("pl")))
		type = CTSVC_LANG_POLISH;
	// pt_BR, pt_PT, Portugal
	else if (!strncmp(system_lang, "pt", strlen("pt")))
		type = CTSVC_LANG_PORTUGUESE;
	// ro, Romania
	else if (!strncmp(system_lang, "ro", strlen("ro")))
		type = CTSVC_LANG_ROMANIA;
	// ru_RU, Russia
	else if (!strncmp(system_lang, "ru", strlen("ru")))
		type = CTSVC_LANG_RUSSIAN;
	// sk, Slovakia - Slovak
	else if (!strncmp(system_lang, "sk", strlen("sk")))
		type = CTSVC_LANG_SLOVAK;
	// sl, Slovenia - Slovenian
	else if (!strncmp(system_lang, "sl", strlen("sl")))
		type = CTSVC_LANG_SLOVENIAN;
	// sr, Serbia - Serbian
	else if (!strncmp(system_lang, "sr", strlen("sr")))
		type = CTSVC_LANG_SERBIAN;
	// sv, Finland - Swedish
	else if (!strncmp(system_lang, "sv", strlen("sv")))
		type = CTSVC_LANG_SWEDISH;
	// tr_TR, Turkey - Turkish
	else if (!strncmp(system_lang, "tr", strlen("tr")))
		type = CTSVC_LANG_TURKISH;
	// uk, Ukraine
	else if (!strncmp(system_lang, "uk", strlen("uk")))
		type = CTSVC_LANG_UKRAINE;
	// zh_CN, zh_HK, zh_SG, zh_TW
	else if (!strncmp(system_lang, "zh", strlen("zh")))
		type = CTSVC_LANG_CHINESE;
	else
		type = CTSVC_LANG_OTHERS;

	return type;
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
		RETVM_IF(char_len <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "check_utf8 failed");

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
		ctsvc_extra_normalize(result, size);
		u_strToUTF8(temp, dest_size, &size, result, -1, &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"u_strToUTF8() Failed(%s)", u_errorName(status));
		chosung_len = ctsvc_check_utf8(temp[0]);
		RETVM_IF(chosung_len <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "check_utf8 failed");
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
		RETVM_IF(char_len <= 0, CONTACTS_ERROR_INVALID_PARAMETER, "check_utf8 failed");

		memcpy(char_src, &src[i], char_len);
		char_src[char_len] = '\0';

		u_strFromUTF8(tmp_result, array_sizeof(tmp_result), NULL, char_src, -1, &status);
		RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
				"u_strFromUTF8() Failed(%s)", u_errorName(status));

		if (is_chosung(tmp_result[0]))
		{
			hangul_compatibility2jamo(tmp_result);

			u_strToUTF8(&dest[j], dest_size - j, &size, tmp_result, -1, &status);
			RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
					"u_strToUTF8() Failed(%s)", u_errorName(status));
			j += size;
			dest[j] = '*';
			j++;
		}
		else {
			u_strToUpper(tmp_result, array_sizeof(tmp_result), tmp_result, -1, NULL, &status);
			RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
					"u_strToUpper() Failed(%s)", u_errorName(status));
			size = unorm_normalize(tmp_result, -1, UNORM_NFD, 0,
					(UChar *)result, array_sizeof(result), &status);
			RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
					"unorm_normalize(%s) Failed(%s)", src, u_errorName(status));
			ctsvc_extra_normalize(result, size);
			u_strToUTF8(&dest[j], dest_size - j, &size, result, -1, &status);
			RETVM_IF(U_FAILURE(status), CONTACTS_ERROR_SYSTEM,
					"u_strToUTF8() Failed(%s)", u_errorName(status));
			j += size;

		}
		count++;
	}

	dest[j] = '\0';

	return count;
}
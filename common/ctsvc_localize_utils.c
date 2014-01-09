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
#include "ctsvc_localize_utils.h"

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

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
#include "ctsvc_localize_jp.h"

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
	case CTSVC_LANG_PERSIAN:
	case CTSVC_LANG_URDU:
		return CTSVC_SORT_ARABIC;
	case CTSVC_LANG_HINDI:
		return CTSVC_SORT_DEVANAGARI;
	case CTSVC_LANG_GEORGIAN:
		return CTSVC_SORT_GEORGIAN;
	case CTSVC_LANG_TURKISH:
		return CTSVC_SORT_TURKISH;
	case CTSVC_LANG_THAI:
		return CTSVC_SORT_THAI;
	case CTSVC_LANG_BENGALI:
		return CTSVC_SORT_BENGALI;
	case CTSVC_LANG_PUNJABI:
		return CTSVC_SORT_PUNJABI;
	case CTSVC_LANG_MALAYALAM:
		return CTSVC_SORT_MALAYALAM;
	case CTSVC_LANG_TELUGU:
		return CTSVC_SORT_TELUGU;
	case CTSVC_LANG_TAMIL:
		return CTSVC_SORT_TAMIL;
	case CTSVC_LANG_ORIYA:
		return CTSVC_SORT_ORIYA;
	case CTSVC_LANG_SINHALA:
		return CTSVC_SORT_SINHALA;
	case CTSVC_LANG_GUJARATI:
		return CTSVC_SORT_GUJARATI;
	case CTSVC_LANG_KANNADA:
		return CTSVC_SORT_KANNADA;
	case CTSVC_LANG_LAO:
		return CTSVC_SORT_LAO;
	case CTSVC_LANG_HEBREW:
		return CTSVC_SORT_HEBREW;
	case CTSVC_LANG_BURMESE:
		return CTSVC_SORT_BURMESE;
	case CTSVC_LANG_KHMER:
		return CTSVC_SORT_KHMER;
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
		else if (ctsvc_is_hangul(word[i])) {
			ctsvc_hangul_compatibility2jamo(&word[i]);
		}
	}
}

const char *ctsvc_get_language_locale(int lang)
{
	char *langset = ctsvc_get_langset();

	switch(lang) {
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
	case CTSVC_LANG_HINDI: // hi, India - Hindi, Marathi, Nepali
		if (!strncmp(langset, "hi", strlen("hi"))) {
			return "hi";
		}
		else if (!strncmp(langset, "mr", strlen("mr"))) {
			return "mr";
		}
		else if (!strncmp(langset, "ne", strlen("ne"))) {
			return "ne";
		}
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
	case CTSVC_LANG_THAI: // th_TH, Thai
		return "th";
	case CTSVC_LANG_BENGALI: // as, bn
		if (!strncmp(langset, "as", strlen("as"))) {
			return "as";
		}
		return "bn";
	case CTSVC_LANG_PUNJABI: // pa
			return "pa";
	case CTSVC_LANG_MALAYALAM:
			return "ml";
	case CTSVC_LANG_TELUGU:
			return "te";
	case CTSVC_LANG_TAMIL:
			return "ta";
	case CTSVC_LANG_ORIYA:
			return "or";
	case CTSVC_LANG_SINHALA:
			return "si";
	case CTSVC_LANG_GUJARATI:
			return "gu";
	case CTSVC_LANG_KANNADA:
			return "kn";
	case CTSVC_LANG_LAO:
			return "lo";
	case CTSVC_LANG_HEBREW:
			return "he";
	case CTSVC_LANG_VIETNAMESE://latin
			return "vi";
	case CTSVC_LANG_PERSIAN:
			return "fa";
	case CTSVC_LANG_UZBEK:
			return "uz";
	case CTSVC_LANG_URDU: //arabic
			return "ur";
	case CTSVC_LANG_ALBANIAN:
			return "sq";
	case CTSVC_LANG_BURMESE:
			return "my";
	case CTSVC_LANG_MALAY:
			return "ms";
	case CTSVC_LANG_KHMER:
			return "km";
	case CTSVC_LANG_INDONESIAN:
			return "id";
	case CTSVC_LANG_TAGALOG:
			return "tl";
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
	// mr, India - marathi
	else if (!strncmp(system_lang, "mr", strlen("mr")))
		type = CTSVC_LANG_HINDI;
	// ne, India - nepal
	else if (!strncmp(system_lang, "ne", strlen("ne")))
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
	// th_TH
	else if (!strncmp(system_lang, "th", strlen("th")))
		type = CTSVC_LANG_THAI;
	else if (!strncmp(system_lang, "as", strlen("as")))
		type = CTSVC_LANG_BENGALI;
	else if (!strncmp(system_lang, "bn", strlen("bn")))
		type = CTSVC_LANG_BENGALI;
	else if (!strncmp(system_lang, "pa", strlen("pa")))
		type = CTSVC_LANG_PUNJABI;
	else if (!strncmp(system_lang, "ml", strlen("ml")))
		type = CTSVC_LANG_MALAYALAM;
	else if (!strncmp(system_lang, "te", strlen("te")))
		type = CTSVC_LANG_TELUGU;
	else if (!strncmp(system_lang, "ta", strlen("ta")))
		type = CTSVC_LANG_TAMIL;
	else if (!strncmp(system_lang, "or", strlen("or")))
		type = CTSVC_LANG_ORIYA;
	else if (!strncmp(system_lang, "si", strlen("si")))
		type = CTSVC_LANG_SINHALA;
	else if (!strncmp(system_lang, "gu", strlen("gu")))
		type = CTSVC_LANG_GUJARATI;
	else if (!strncmp(system_lang, "kn", strlen("kn")))
		type = CTSVC_LANG_KANNADA;
	else if (!strncmp(system_lang, "lo", strlen("lo")))
		type = CTSVC_LANG_LAO;
	else if (!strncmp(system_lang, "he", strlen("he")))
		type = CTSVC_LANG_HEBREW;
	else if (!strncmp(system_lang, "vi", strlen("vi")))
		type = CTSVC_LANG_VIETNAMESE;
	else if (!strncmp(system_lang, "fa", strlen("fa")))
		type = CTSVC_LANG_PERSIAN;
	else if (!strncmp(system_lang, "uz", strlen("uz")))
		type = CTSVC_LANG_UZBEK;
	else if (!strncmp(system_lang, "ur", strlen("ur")))
		type = CTSVC_LANG_URDU;
	else if (!strncmp(system_lang, "sq", strlen("sq")))
		type = CTSVC_LANG_ALBANIAN;
	else if (!strncmp(system_lang, "my", strlen("my")))
		type = CTSVC_LANG_BURMESE;
	else if (!strncmp(system_lang, "ms", strlen("ms")))
		type = CTSVC_LANG_MALAY;
	else if (!strncmp(system_lang, "km", strlen("km")))
		type = CTSVC_LANG_KHMER;
	else if (!strncmp(system_lang, "id", strlen("id")))
		type = CTSVC_LANG_INDONESIAN;
	else if (!strncmp(system_lang, "tl", strlen("tl")))
		type = CTSVC_LANG_TAGALOG;
	else
		type = CTSVC_LANG_OTHERS;

	return type;
}

static char *langset = NULL;

char* ctsvc_get_langset()
{
	return langset;
}

void ctsvc_set_langset(char *new_langset)
{
	free(langset);
	langset = new_langset;
}


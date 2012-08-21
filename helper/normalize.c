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
#include <string.h>
#include <unicode/ustring.h>
#include <unicode/unorm.h>
#include <unicode/ucol.h>
#include <contacts-svc.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "internal.h"
#include "cts-sqlite.h"
#include "localize.h"
#include "normalize.h"
#include "utils.h"

#define array_sizeof(a) (sizeof(a) / sizeof(a[0]))

int helper_unicode_to_utf8(char *src, int src_len, char *dest, int dest_size)
{
	int32_t size = 0;
	UErrorCode status = 0;
	UChar *unicode_src = (UChar *)src;

	u_strToUTF8(dest, dest_size, &size, unicode_src, -1, &status);
	h_retvm_if(U_FAILURE(status), CTS_ERR_ICU_FAILED,
			"u_strToUTF8() Failed(%s)", u_errorName(status));

	dest[size]='\0';
	return CTS_SUCCESS;
}

static inline int check_utf8(char c)
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
		return CTS_ERR_FAIL;
}

int helper_normalize_str(const char *src, char *dest, int dest_size)
{
	int type = CTS_LANG_OTHERS;
	int32_t size;
	UErrorCode status = 0;
	UChar tmp_result[CTS_SQL_MAX_LEN*2];
	UChar result[CTS_SQL_MAX_LEN*2];
	int i = 0;
	int j = 0;
	int str_len = strlen(src);
	int char_len = 0;

	for (i=0;i<str_len;i+=char_len) {
		char char_src[10];
		char_len = check_utf8(src[i]);
		memcpy(char_src, &src[i], char_len);
		char_src[char_len] = '\0';

		u_strFromUTF8(tmp_result, array_sizeof(tmp_result), NULL, char_src, -1, &status);
		h_retvm_if(U_FAILURE(status), CTS_ERR_ICU_FAILED,
				"u_strFromUTF8() Failed(%s)", u_errorName(status));

		u_strToLower(tmp_result, array_sizeof(tmp_result), tmp_result, -1, NULL, &status);
		h_retvm_if(U_FAILURE(status), CTS_ERR_ICU_FAILED,
				"u_strToLower() Failed(%s)", u_errorName(status));

		size = unorm_normalize(tmp_result, -1, UNORM_NFD, 0,
				(UChar *)result, array_sizeof(result), &status);
		h_retvm_if(U_FAILURE(status), CTS_ERR_ICU_FAILED,
				"unorm_normalize(%s) Failed(%s)", char_src, u_errorName(status));

		if (0 == i)
			type = helper_check_language(result);
		helper_extra_normalize(result, size);

		u_strToUTF8(&dest[j], dest_size-j, &size, result, -1, &status);
		h_retvm_if(U_FAILURE(status), CTS_ERR_ICU_FAILED,
				"u_strToUTF8() Failed(%s)", u_errorName(status));
		j += size;
		dest[j++] = 0x01;
	}
	dest[j]='\0';
	HELPER_DBG("src(%s) is transformed(%s)", src, dest);
	return type;
}

int helper_collation_str(const char *src, char *dest, int dest_size)
{
	HELPER_FN_CALL;
	int32_t size = 0;
	UErrorCode status = 0;
	UChar tmp_result[CTS_SQL_MAX_LEN];
	UCollator *collator;
	const char *region;

	region = vconf_get_str(VCONFKEY_REGIONFORMAT);
	HELPER_DBG("region %s", region);
	collator = ucol_open(region, &status);
	h_retvm_if(U_FAILURE(status), CTS_ERR_ICU_FAILED,
			"ucol_open() Failed(%s)", u_errorName(status));

	if (U_FAILURE(status)){
		ERR("ucol_setAttribute Failed(%s)", u_errorName(status));
		ucol_close(collator);
		return CTS_ERR_ICU_FAILED;
	}

	u_strFromUTF8(tmp_result, array_sizeof(tmp_result), NULL, src, -1, &status);
	if (U_FAILURE(status)){
		ERR("u_strFromUTF8 Failed(%s)", u_errorName(status));
		ucol_close(collator);
		return CTS_ERR_ICU_FAILED;
	}
	size = ucol_getSortKey(collator, tmp_result, -1, (uint8_t *)dest, dest_size);
	ucol_close(collator);
	dest[size]='\0';

	return CTS_SUCCESS;
}

API int cts_helper_normalize_name(char dest[][CTS_SQL_MAX_LEN])
{
	int lang_type=0;
	int ret=CTS_ERR_NO_DATA;
	int sizes[CTS_NN_MAX]={0};
	char normalized_first[CTS_SQL_MAX_LEN];
	char normalized_last[CTS_SQL_MAX_LEN];

	sizes[CTS_NN_FIRST] = strlen(dest[CTS_NN_FIRST]);
	sizes[CTS_NN_LAST] = strlen(dest[CTS_NN_LAST]);

	if (sizes[CTS_NN_FIRST]) {
		ret = helper_normalize_str(dest[CTS_NN_FIRST], normalized_first,
				sizeof(normalized_first));
		h_retvm_if(ret < CTS_SUCCESS, ret, "helper_normalize_str() Failed(%d)", ret);
		lang_type = ret;

		sizes[CTS_NN_FIRST] = strlen(normalized_first);
	}

	if (sizes[CTS_NN_LAST]) {
		ret = helper_normalize_str(dest[CTS_NN_LAST], normalized_last,
				sizeof(normalized_first));
		h_retvm_if(ret < CTS_SUCCESS, ret, "helper_normalize_str() Failed(%d)", ret);
		if (lang_type < ret) lang_type = ret;

		sizes[CTS_NN_LAST] = strlen(normalized_last);
	}

	if (sizes[CTS_NN_FIRST])
		snprintf(dest[CTS_NN_FIRST], sizeof(dest[CTS_NN_FIRST]), "%s", normalized_first);
	if (sizes[CTS_NN_LAST])
		snprintf(dest[CTS_NN_LAST], sizeof(dest[CTS_NN_LAST]), "%s", normalized_last);

	return lang_type;
}


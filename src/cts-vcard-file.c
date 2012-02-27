/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngjae Shin <yj99.shin@samsung.com>
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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <iconv.h>

#include "cts-internal.h"
#include "cts-types.h"
#include "cts-utils.h"
#include "cts-normalize.h"
#include "cts-vcard.h"
#include "cts-vcard-file.h"

static int cts_tmp_photo_id;

enum {
	CTS_VCARD_VER_NONE,
	CTS_VCARD_VER_2_1,
	CTS_VCARD_VER_3_0,
	CTS_VCARD_VER_4_0,
};

enum {
	CTS_VCARD_VALUE_NONE,
	CTS_VCARD_VALUE_FN,
	CTS_VCARD_VALUE_N,
	CTS_VCARD_VALUE_NICKNAME,
	CTS_VCARD_VALUE_PHOTO,
	CTS_VCARD_VALUE_BDAY,
	CTS_VCARD_VALUE_ADR,
	CTS_VCARD_VALUE_LABEL,
	CTS_VCARD_VALUE_TEL,
	CTS_VCARD_VALUE_EMAIL,
	CTS_VCARD_VALUE_TITLE,
	CTS_VCARD_VALUE_ROLE,
	CTS_VCARD_VALUE_ORG,
	CTS_VCARD_VALUE_NOTE,
	CTS_VCARD_VALUE_REV,
	CTS_VCARD_VALUE_UID,
	CTS_VCARD_VALUE_URL,
	CTS_VCARD_VALUE_X_ANNIVERSARY,
	CTS_VCARD_VALUE_X_IRMC_LUID,
	CTS_VCARD_VALUE_X_SLP_GROUP,
	CTS_VCARD_VALUE_END,
	CTS_VCARD_VALUE_MAX
};

static const char *content_name[CTS_VCARD_VALUE_MAX];

static inline char* cts_vcard_remove_empty_line(char *src)
{
	while (*src) {
		if ('\n' != *src && '\r' != *src)
			break;
		src++;
	}
	return src;
}

static char* cts_vcard_check_word(char *src, const char *word)
{
	bool start = false;

	retvm_if(NULL == src, NULL, "The src is NULL.");

	src = cts_vcard_remove_empty_line(src);

	while (*src) {
		switch (*src) {
		case ' ':
		case ':':
		case ';':
			src++;
			break;
		default:
			start = true;
			break;
		}
		if (start) break;
	}

	while (*src == *word) {
		src++;
		word++;

		if ('\0' == *src || '\0' == *word)
			break;
	}

	if ('\0' == *word)
		return src;
	else
		return NULL;
}

static int cts_vcard_check_content_type(char **vcard)
{
	int i;
	char *new_start;

	for (i=CTS_VCARD_VALUE_NONE+1;i<CTS_VCARD_VALUE_MAX;i++) {
		new_start = cts_vcard_check_word(*vcard, content_name[i]);
		if (new_start && (':' == *new_start || ';' == *new_start))
			break;
	}

	if (CTS_VCARD_VALUE_MAX == i)
		return CTS_VCARD_VALUE_NONE;
	else {
		*vcard = new_start;
		return i;
	}
}

static inline int cts_vcard_check_quoted(char *src)
{
	int ret;

	while (*src) {
		if ('Q' == *src) {
			ret = strncmp(src, "QUOTED-PRINTABLE", sizeof("QUOTED-PRINTABLE") - 1);
			if (!ret)
				return CTS_TRUE;
		}else if (':' == *src) {
			break;
		}
		src++;
	}
	return CTS_FALSE;
}

static inline int cts_vcard_hex_to_dec(char hex)
{
	switch (hex) {
	case '0' ... '9':
		return hex - '0';
	case 'a' ... 'f':
		return hex - 'a' + 10;
	case 'A' ... 'F':
		return hex - 'A' + 10;
	default:
		return 0;
	}
}
static inline int cts_vcard_decode_quoted_val(char *val)
{
	char *src, *dest;

	src = strchr(val, ':');
	if (NULL == src)
		src = val;

	dest = src;
	while (*src) {
		if ('=' == *src) {
			*dest = (char)((cts_vcard_hex_to_dec(*(src+1)) << 4) + cts_vcard_hex_to_dec(*(src+2)));
			if (*(src+1) && *(src+2))
				src += 2;
		}else
			*dest = *src;
		dest++;
		src++;
	}

	*dest = '\0';
	return dest - val;
}

static inline char* cts_vcard_translate_charset(char *src, int len)
{
	int ret;
	char *val = src;

	while (*val) {
		if ('C' == *val) {
			ret = strncmp(val, "CHARSET", sizeof("CHARSET") - 1);
			if (!ret) {
				val += sizeof("CHARSET");
				break;
			}
		}else if (':' == *val) {
			return NULL;
		}
		val++;
	}

	if (*val) {
		int src_len, dest_len, i = 0;
		iconv_t ic;
		char enc[32], *dest;

		while (';' != *val && ':' != *val) {
			enc[i++] = *val++;
		}
		enc[i] = '\0';
		if (0 == strcasecmp("UTF-8", enc))
			return NULL;

		while (':' != *val)
			val++;

		ic = iconv_open("UTF-8", enc);
		retvm_if(ic == (iconv_t)-1, NULL, "iconv_open(%s) Failed", enc);

		src_len = len - (val - src);
		dest_len = 2048;
		dest = malloc(dest_len);

		while (true) {
			char *in = val;
			char *out = dest;
			size_t in_byte = src_len;
			size_t out_byte = dest_len;

			ret = iconv(ic, &in, &in_byte, &out, &out_byte);

			if (-1 == ret) {
				if (E2BIG == errno) {
					dest_len *= 2;
					dest = realloc(dest, dest_len);
					continue;
				} else {
					if (dest) {
						free(dest);
						dest = NULL;
					}
					ERR("iconv is Failed(errno = %d)", errno);
					break;
				}
			}
			dest[dest_len-out_byte] = '\0';
			break;
		}
		iconv_close(ic);
		return dest;
	}

	return NULL;
}

static char* cts_vcard_get_val(int ver, char *src, char **dest)
{
	int len;
	bool start = false;
	char *cursor;

	retvm_if(NULL == src, NULL, "The src is NULL.");
	retvm_if(NULL == dest, NULL, "The dest is NULL.");

	while (*src) {
		switch (*src) {
		case '\n':
			return NULL;
		case '\r':
		case ' ':
		case ':':
			src++;
			break;
		default:
			start = true;
			break;
		}
		if (start) break;
	}

	cursor = src;
	len = 0;
	while (*cursor)
	{
		if ('\r' == *cursor) cursor++;
		if ('\n' == *cursor) {
			if (' ' != *(cursor+1))
				break;
		}

		cursor++;
	}

	if (src == cursor) {
		*dest = NULL;
		return NULL;
	}
	else {
		int len = 0;
		char temp = *cursor;
		char *new_dest;

		*cursor = '\0';
		*dest = strdup(src);
		//if(CTS_VCARD_VER_2_1 == ver)
		if (cts_vcard_check_quoted(*dest))
			len = cts_vcard_decode_quoted_val(*dest);
		if (0 == len)
			len = strlen(*dest);
		new_dest = cts_vcard_translate_charset(*dest, len);
		if (new_dest) {
			free(*dest);
			*dest = new_dest;
		}
		*cursor = temp;
		return (cursor + 1);
	}
}

static inline int cts_vcard_check_version(const char *src)
{
	bool start = false;
	const char *ver3 = "3.0";

	while (*src) {
		switch (*src) {
		case '\n':
		case '\r':
			return CTS_VCARD_VER_2_1;
		case ' ':
			src++;
			break;
		default:
			start = true;
			break;
		}
		if (start) break;
	}

	if (0 == strcmp(src, ver3))
		return CTS_VCARD_VER_3_0;
	else
		return CTS_VCARD_VER_2_1;
}

static inline int cts_vcard_remove_folding(char *folded_src)
{
	char *result = folded_src;

	retv_if(NULL == folded_src, CTS_ERR_ARG_NULL);

	while (*folded_src) {
		if ('\r' == *folded_src)
			folded_src++;
		if ('\n' == *folded_src && ' ' == *(folded_src+1))
			folded_src += 2;

		if ('\0' == *folded_src)
			break;

		*result = *folded_src;
		result++;
		folded_src++;
	}
	*result = '\0';
	return CTS_SUCCESS;
}


static inline char* cts_get_content_value(char *val)
{
	char *temp;

	temp = strchr(val, ':');
	if (temp)
		temp++;
	else
		temp = val;

	retvm_if('\0' == *(temp) || '\r' == *(temp) || '\n' == *(temp),
		NULL, "Invalid vcard content(%s)", val);

	return temp;
}


static char* cts_strtok(char *val, char c)
{
	while(*val) {
		if(*val == c) {
			*val = '\0';
			return (val+1);
		}
		val++;
	}
	return val;
}


static inline int cts_vcard_get_display_name(cts_name *name, char *val)
{
	char *temp;

	temp = cts_get_content_value(val);
	name->display = SAFE_STRDUP(temp);

	return CTS_SUCCESS;
}


static inline int cts_vcard_get_name(cts_name *name, char *val)
{
	char *temp, *start;
	const char separator = ';';

	start = cts_get_content_value(val);
	retv_if(NULL == start, CTS_ERR_NO_DATA);

	temp = cts_strtok(start, separator);
	name->last = SMART_STRDUP(start);
	start = temp;
	temp = cts_strtok(start, separator);
	name->first = SMART_STRDUP(start);
	start = temp;
	temp = cts_strtok(start, separator);
	name->addition = SMART_STRDUP(start);
	start = temp;
	temp = cts_strtok(start, separator);
	name->prefix = SMART_STRDUP(start);
	start = temp;
	temp = cts_strtok(start, separator);
	name->suffix = SMART_STRDUP(start);

	return CTS_SUCCESS;
}

static inline GSList* cts_vcard_get_nickname(GSList *nicks, char *val)
{
	char *temp;
	cts_nickname *result;
	const char *separator = ",";

	temp = strtok(val, separator);
	while (temp) {
		if ('\0' == *temp) continue;

		result = (cts_nickname *)contacts_svc_value_new(CTS_VALUE_NICKNAME);
		if (result) {
			result->embedded = true;
			result->nick = strdup(temp);
			nicks = g_slist_append(nicks, result);
		}

		temp = strtok(NULL, separator);
	}

	return nicks;
}

static inline GSList* cts_vcard_get_event(GSList *events, int type, char *val)
{
	cts_event *event;

	event = (cts_event *)contacts_svc_value_new(CTS_VALUE_EVENT);
	if (event) {
		char *dest, *src;

		event->embedded = true;
		event->type = type;

		dest = src = val;
		while (*src) {
			if ('0' <= *src && *src <= '9') {
				*dest = *src;
				dest++;
			}
			src++;
			if (8 <= dest - val)
				break;
		}
		*dest = '\0';

		event->date = atoi(val);

		events = g_slist_append(events, event);
	}

	return events;
}

static inline int cts_vcard_get_company(cts_company *org, char *val)
{
	char *temp, *start;

	start = cts_get_content_value(val);
	retv_if(NULL == start, CTS_ERR_NO_DATA);

	temp = strchr(start, ';');
	if (temp) {
		*temp = '\0';
		org->name = SMART_STRDUP(start);
		org->department = SMART_STRDUP(temp+1);
	}
	else
		org->name = strdup(start);

	return CTS_SUCCESS;
}


static inline char* cts_vcard_get_note(char *val)
{
	char *temp;

	temp = cts_get_content_value(val);

	if (temp)
		return g_strcompress(temp);
	else
		return NULL;
}


static inline int cts_vcard_get_time(char *val)
{
	int i;
	char tmp[10];
	struct tm ts = {0};

	i = 0;
	while (*val && (*val < '0' || '9' < *val)) val++;
	while (*val) {
		tmp[i++] = *val;
		val++;
		if (4<=i || *val < '0' || '9' < *val) break;
	}
	tmp[i] = 0;
	ts.tm_year = atoi(tmp)-1900;

	i = 0;
	while (*val && (*val < '0' || '9' < *val)) val++;
	while (*val) {
		tmp[i++] = *val;
		val++;
		if (2<=i || *val < '0' || '9' < *val) break;
	}
	tmp[i] = 0;
	ts.tm_mon = atoi(tmp)-1;

	i = 0;
	while (*val && (*val < '0' || '9' < *val)) val++;
	while (*val) {
		tmp[i++] = *val;
		val++;
		if (2<=i || *val < '0' || '9' < *val) break;
	}
	tmp[i] = 0;
	ts.tm_mday = atoi(tmp);

	i = 0;
	while (*val && (*val < '0' || '9' < *val)) val++;
	while (*val) {
		tmp[i++] = *val;
		val++;
		if (2<=i || *val < '0' || '9' < *val) break;
	}
	tmp[i] = 0;
	ts.tm_hour = atoi(tmp);

	i = 0;
	while (*val && (*val < '0' || '9' < *val)) val++;
	while (*val) {
		tmp[i++] = *val;
		val++;
		if (2<=i || *val < '0' || '9' < *val) break;
	}
	tmp[i] = 0;
	ts.tm_min = atoi(tmp);

	i = 0;
	while (*val && (*val < '0' || '9' < *val)) val++;
	while (*val) {
		tmp[i++] = *val;
		val++;
		if (2<=i || *val < '0' || '9' < *val) break;
	}
	tmp[i] = 0;
	ts.tm_sec = atoi(tmp);

	return (int)mktime(&ts);
}


static inline GSList* cts_vcard_get_web(GSList *webs, char *val)
{
	cts_web *web;
	char *temp;

	temp = cts_get_content_value(val);
	retvm_if(NULL == temp, webs, "Invalid vcard(%s)", val);

	web = (cts_web *)contacts_svc_value_new(CTS_VALUE_WEB);
	if (web) {
		web->embedded = true;
		web->url = strdup(temp);;
		if (val != temp) {
			*(temp-1) = '\0';
			temp = val;
			while (*temp)
			{
				*temp = tolower(*temp);
				temp++;
			}

			temp = strchr(val, ';');
			if (temp) {
				if (strstr(val, "home"))
					web->type = CTS_WEB_TYPE_HOME;
				else if (strstr(val, "work"))
					web->type = CTS_WEB_TYPE_WORK;
			}
		}
		webs = g_slist_append(webs, web);
	} else {
		ERR("contacts_svc_value_new() Failed");
	}

	return webs;
}


static inline bool cts_vcard_get_number_type(cts_number *number, char *val)
{
	char *temp, *result;
	int type = CTS_NUM_TYPE_NONE;
	bool pref = false;

	temp = val;
	while (*temp)
	{
		*temp = tolower(*temp);
		temp++;
	}

	temp = strchr(val , ';');
	if (temp) {
		result = strstr(val, "home");
		if (result) type |= CTS_NUM_TYPE_HOME;
		result = strstr(val, "msg");
		if (result) type |= CTS_NUM_TYPE_MSG;
		result = strstr(val, "work");
		if (result) type |= CTS_NUM_TYPE_WORK;
		result = strstr(val, "pref");
		if (result) pref = true;
		result = strstr(val, "voice");
		if (result) type |= CTS_NUM_TYPE_VOICE;
		result = strstr(val, "fax");
		if (result) type |= CTS_NUM_TYPE_FAX;
		result = strstr(val, "cell");
		if (result) type |= CTS_NUM_TYPE_CELL;
		result = strstr(val, "video");
		if (result) type |= CTS_NUM_TYPE_VIDEO;
		result = strstr(val, "pager");
		if (result) type |= CTS_NUM_TYPE_PAGER;
		result = strstr(val, "bbs");
		if (result) type |= CTS_NUM_TYPE_BBS;
		result = strstr(val, "modem");
		if (result) type |= CTS_NUM_TYPE_MODEM;
		result = strstr(val, "car");
		if (result) type |= CTS_NUM_TYPE_CAR;
		result = strstr(val, "isdn");
		if (result) type |= CTS_NUM_TYPE_ISDN;
		result = strstr(val, "pcs");
		if (result) type |= CTS_NUM_TYPE_PCS;
	}
	number->type = type;

	return pref;
}


static inline GSList* cts_vcard_get_number(GSList *numbers, char *val)
{
	cts_number *number;
	char *temp;

	temp = cts_get_content_value(val);
	retvm_if(NULL == temp, numbers, "Invalid vcard(%s)", val);

	number = (cts_number *)contacts_svc_value_new(CTS_VALUE_NUMBER);
	if (number) {
		number->embedded = true;
		number->number = strdup(temp);
		if (val != temp) {
			*(temp-1) = '\0';
			number->is_default = cts_vcard_get_number_type(number, val);
		}
		numbers = g_slist_append(numbers, number);
	} else {
		ERR("contacts_svc_value_new() Failed");
	}

	return numbers;
}

static inline bool cts_vcard_get_email_type(cts_email *email, char *val)
{
	char *temp;
	int type = CTS_EMAIL_TYPE_NONE;
	bool pref = false;

	temp = val;
	while (*temp)
	{
		*temp = tolower(*temp);
		temp++;
	}

	temp = strchr(val , ';');
	if (temp) {
		if (strstr(val, "home"))
			type = CTS_EMAIL_TYPE_HOME;
		else if (strstr(val, "work"))
			type = CTS_EMAIL_TYPE_WORK;
		if (strstr(val, "pref"))
			pref = true;
	}
	email->type = type;

	return pref;
}

static inline GSList* cts_vcard_get_email(GSList *emails, char *val)
{
	cts_email *email;
	char *temp;

	temp = cts_get_content_value(val);
	retvm_if(NULL == temp, emails, "Invalid vcard(%s)", val);

	email = (cts_email *)contacts_svc_value_new(CTS_VALUE_EMAIL);
	if (email) {
		email->embedded = true;
		email->email_addr = strdup(temp);
		if (val != temp) {
			*(temp-1) = '\0';
			email->is_default = cts_vcard_get_email_type(email, val);
		}
		emails = g_slist_append(emails, email);
	} else {
		ERR("contacts_svc_value_new() Failed");
	}

	return emails;
}

static inline bool cts_vcard_get_postal_type(cts_postal *postal, char *val)
{
	char *temp, *result;
	int type = CTS_ADDR_TYPE_NONE;
	bool pref = false;

	temp = val;
	while (*temp)
	{
		*temp = tolower(*temp);
		temp++;
	}

	temp = strchr(val , ';');
	if (temp) {
		result = strstr(val, "dom");
		if (result) type |= CTS_ADDR_TYPE_DOM;
		result = strstr(val, "intl");
		if (result) type |= CTS_ADDR_TYPE_INTL;
		result = strstr(val, "postal");
		if (result) type |= CTS_ADDR_TYPE_POSTAL;
		result = strstr(val, "parcel");
		if (result) type |= CTS_ADDR_TYPE_PARCEL;
		result = strstr(val, "home");
		if (result) type |= CTS_ADDR_TYPE_HOME;
		result = strstr(val, "work");
		if (result) type |= CTS_ADDR_TYPE_WORK;
		result = strstr(val, "pref");
		if (result) pref = true;
	}
	postal->type = type;

	return pref;
}

#define CTS_GET_ADDR_COMPONENT(dest, src, tmp_char) \
	tmp_char = strchr(src , ';'); \
if (tmp_char) { \
	*tmp_char = '\0'; \
	dest = SMART_STRDUP(src); \
	src = tmp_char+1; \
} \
else { \
	dest = SMART_STRDUP(src); \
	break; \
} \

static inline GSList* cts_vcard_get_postal(GSList *postals, char *val)
{
	char *text;
	cts_postal *postal;

	postal = (cts_postal *)contacts_svc_value_new(CTS_VALUE_POSTAL);
	if (postal) {
		postal->embedded = true;

		text = strrchr(val , ':');
		if (text)
			text++;
		else
			text = val;

		while (true) {
			char *temp;

			CTS_GET_ADDR_COMPONENT(postal->pobox, text, temp);
			CTS_GET_ADDR_COMPONENT(postal->extended, text, temp);
			CTS_GET_ADDR_COMPONENT(postal->street, text, temp);
			CTS_GET_ADDR_COMPONENT(postal->locality, text, temp);
			CTS_GET_ADDR_COMPONENT(postal->region, text, temp);
			CTS_GET_ADDR_COMPONENT(postal->postalcode, text, temp);
			CTS_GET_ADDR_COMPONENT(postal->country, text, temp);

			ERR("invalid ADR type(%s)", temp);
			break;
		}
		if (postal->pobox || postal->extended || postal->street || postal->locality
				|| postal->region || postal->postalcode || postal->country) {
			postal->is_default = cts_vcard_get_postal_type(postal, val);
		} else {
			ERR("Invalid vcard(%s)", val);
			contacts_svc_value_free((CTSvalue *)postal);
			return postals;
		}

		postals = g_slist_append(postals, postal);
	}

	return postals;
}

static inline int cts_vcard_get_photo_type(char *val)
{
	char *temp, *result;

	temp = val;
	while (*temp)
	{
		*temp = tolower(*temp);
		temp++;
	}

	result = strstr(val, "jpeg");
	if (result) return CTS_VCARD_IMG_JPEG;
	result = strstr(val, "jpg");
	if (result) return CTS_VCARD_IMG_JPEG;

	result = strstr(val, "png");
	if (result) return CTS_VCARD_IMG_PNG;

	result = strstr(val, "gif");
	if (result) return CTS_VCARD_IMG_GIF;

	result = strstr(val, "tiff");
	if (result) return CTS_VCARD_IMG_TIFF;

	return CTS_VCARD_IMG_NONE;
}

static inline const char* cts_get_img_suffix(int type)
{
	switch (type)
	{
	case CTS_VCARD_IMG_TIFF:
		return "tiff";
	case CTS_VCARD_IMG_GIF:
		return "gif";
	case CTS_VCARD_IMG_PNG:
		return "png";
	case CTS_VCARD_IMG_JPEG:
	case CTS_VCARD_IMG_NONE:
	default:
		return "jpeg";
	}
}

static inline int cts_vcard_get_photo(cts_ct_base *base, char *val)
{
	int ret, type, fd;
	gsize size;
	guchar *buf;
	char *temp;
	char dest[CTS_IMG_PATH_SIZE_MAX];

	temp = strchr(val , ':');
	retvm_if(NULL == temp, CTS_ERR_ARG_INVALID, "val is invalid(%s)", val);

	*temp = '\0';
	type = cts_vcard_get_photo_type(val);

	ret = snprintf(dest, sizeof(dest), "%s/%d-%d.%s", CTS_VCARD_IMAGE_LOCATION,
			getpid(), cts_tmp_photo_id++, cts_get_img_suffix(type));
	retvm_if(ret<=0, CTS_ERR_FAIL, "Destination file name was not created");

	fd = open(dest, O_WRONLY|O_CREAT|O_TRUNC, 0660);
	retvm_if(fd < 0, CTS_ERR_FAIL, "open(%s) Failed(%d)", dest, errno);

	buf = g_base64_decode(temp+1, &size);

	while (0 < size) {
		ret = write(fd, buf, size);
		if (ret <= 0) {
			if (EINTR == errno)
				continue;
			else {
				ERR("write() Failed(%d)", errno);
				close(fd);
				if (ENOSPC == errno)
					return CTS_ERR_NO_SPACE;
				else
					return CTS_ERR_IO_ERR;
			}
		}
		size -= ret;
	}

	close(fd);
	g_free(buf);

	base->vcard_img_path = strdup(dest);

	return CTS_SUCCESS;
}

static inline GSList* cts_vcard_get_group(GSList *groups, char *val)
{
	char *temp;
	cts_group *result;
	const char *separator = ",";

	temp = strtok(val, separator);
	while (temp) {
		if ('\0' == *temp) continue;

		result = (cts_group *)contacts_svc_value_new(CTS_VALUE_GROUP_RELATION);
		if (result) {
			result->embedded = true;
			result->vcard_group = strdup(temp);
			groups = g_slist_append(groups, result);
		}

		temp = strtok(NULL, separator);
	}

	return groups;
}

static inline char* cts_vcard_pass_unsupported(char *vcard)
{
	while (*vcard) {
		if ('\n' == *vcard)
			return (vcard + 1);
		vcard++;
	}

	return NULL;
}

static inline int cts_vcard_get_contact(int ver, int flags,
		char *vcard, contact_t *contact)
{
	int type;
	char *cursor, *new_start, *val;

	cursor = vcard;
	while (cursor)
	{
		type = cts_vcard_check_content_type(&cursor);
		if (CTS_VCARD_VALUE_NONE == type) {
			new_start = cts_vcard_pass_unsupported(cursor);
			if (new_start) {
				cursor = new_start;
				continue;
			}
			else
				break;
		}

		new_start = cts_vcard_get_val(ver, cursor, &val);
		if (NULL == new_start)
			continue;

		if (NULL == val) {
			cursor = new_start;
			continue;
		}
		cts_vcard_remove_folding(val);

		switch (type) {
		case CTS_VCARD_VALUE_FN:
			cts_vcard_get_display_name(contact->name, val);
			free(val);
			break;
		case CTS_VCARD_VALUE_N:
			cts_vcard_get_name(contact->name, val);
			free(val);
			break;
		case CTS_VCARD_VALUE_NICKNAME:
			contact->nicknames = cts_vcard_get_nickname(contact->nicknames, val);
			free(val);
			break;
		case CTS_VCARD_VALUE_PHOTO:
			cts_vcard_get_photo(contact->base, val);
			free(val);
			break;
		case CTS_VCARD_VALUE_BDAY:
			contact->events = cts_vcard_get_event(contact->events,
					CTS_EVENT_TYPE_BIRTH, val);
			free(val);
			break;
		case CTS_VCARD_VALUE_ADR:
		case CTS_VCARD_VALUE_LABEL:
			contact->postal_addrs = cts_vcard_get_postal(contact->postal_addrs, val);
			free(val);
			break;
		case CTS_VCARD_VALUE_TEL:
			contact->numbers = cts_vcard_get_number(contact->numbers, val);
			free(val);
			break;
		case CTS_VCARD_VALUE_EMAIL:
			contact->emails = cts_vcard_get_email(contact->emails, val);
			free(val);
			break;
		case CTS_VCARD_VALUE_TITLE:
			if (NULL == contact->company) {
				contact->company = (cts_company*)contacts_svc_value_new(CTS_VALUE_COMPANY);
				if (NULL == contact->company) {
					free(val);
					ERR("contacts_svc_value_new(CTS_VALUE_COMPANY) Failed");
					return CTS_ERR_OUT_OF_MEMORY;
				}
				contact->company->embedded = true;
			}
			contact->company->jot_title = val;
			break;
		case CTS_VCARD_VALUE_ROLE:
			if (NULL == contact->company) {
				contact->company = (cts_company*)contacts_svc_value_new(CTS_VALUE_COMPANY);
				if (NULL == contact->company) {
					free(val);
					ERR("contacts_svc_value_new(CTS_VALUE_COMPANY) Failed");
					return CTS_ERR_OUT_OF_MEMORY;
				}
				contact->company->embedded = true;
			}
			contact->company->role = val;
			break;
		case CTS_VCARD_VALUE_ORG:
			if (NULL == contact->company) {
				contact->company = (cts_company*)contacts_svc_value_new(CTS_VALUE_COMPANY);
				if (NULL == contact->company) {
					free(val);
					ERR("contacts_svc_value_new(CTS_VALUE_COMPANY) Failed");
					return CTS_ERR_OUT_OF_MEMORY;
				}
				contact->company->embedded = true;
			}
			cts_vcard_get_company(contact->company, val);
			free(val);
			break;
		case CTS_VCARD_VALUE_NOTE:
			contact->base->note = cts_vcard_get_note(val);
			free(val);
			break;
		case CTS_VCARD_VALUE_REV:
			if (*val)
				contact->base->changed_time = cts_vcard_get_time(val);;
			free(val);
			break;
		case CTS_VCARD_VALUE_UID:
			contact->base->uid = val;
			break;
		case CTS_VCARD_VALUE_URL:
			contact->web_addrs = cts_vcard_get_web(contact->web_addrs, val);
			free(val);
			break;
		case CTS_VCARD_VALUE_X_ANNIVERSARY:
			contact->events = cts_vcard_get_event(contact->events,
					CTS_EVENT_TYPE_ANNIVERSARY, val);
			free(val);
			break;
		case CTS_VCARD_VALUE_X_IRMC_LUID:
			contact->base->id = atoi(val);
			free(val);
			break;
		case CTS_VCARD_VALUE_X_SLP_GROUP:
			if (flags & CTS_VCARD_CONTENT_X_SLP_GROUP)
				contact->grouprelations = cts_vcard_get_group(contact->grouprelations, val);
			free(val);
			break;
		case CTS_VCARD_VALUE_END:
			free(val);
			return CTS_SUCCESS;
		default:
			ERR("cts_vcard_check_content_type() Failed(%d)", type);
			return CTS_ERR_VOBJECT_FAILED;
		}
		cursor = new_start;
	}

	ERR("Invalid vcard(%s)", vcard);
	return CTS_ERR_ARG_INVALID;
}

static void cts_vcard_initial(void)
{
	if (NULL == *content_name) {
		//content_name[CTS_VCARD_VALUE_NAME] = "NAME"; /* not supported */
		//content_name[CTS_VCARD_VALUE_PROFILE] = "PROFILE"; /* not supported */
		//content_name[CTS_VCARD_VALUE_SOURCE] = "SOURCE"; /* not supported */
		content_name[CTS_VCARD_VALUE_FN] = "FN";
		content_name[CTS_VCARD_VALUE_N] = "N";
		content_name[CTS_VCARD_VALUE_NICKNAME] = "NICKNAME";
		content_name[CTS_VCARD_VALUE_PHOTO] = "PHOTO";
		content_name[CTS_VCARD_VALUE_BDAY] = "BDAY";
		content_name[CTS_VCARD_VALUE_ADR] = "ADR";
		content_name[CTS_VCARD_VALUE_LABEL] = "LABEL"; /* not supported */
		content_name[CTS_VCARD_VALUE_TEL] = "TEL";
		content_name[CTS_VCARD_VALUE_EMAIL] = "EMAIL";
		//content_name[CTS_VCARD_VALUE_MAILER] = "MAILER"; /* not supported */
		//content_name[CTS_VCARD_VALUE_TZ] = "TZ"; /* not supported */
		//content_name[CTS_VCARD_VALUE_GEO] = "GEO"; /* not supported */
		content_name[CTS_VCARD_VALUE_TITLE] = "TITLE";
		content_name[CTS_VCARD_VALUE_ROLE] = "ROLE";
		//content_name[CTS_VCARD_VALUE_LOGO] = "LOGO"; /* not supported */
		//content_name[CTS_VCARD_VALUE_AGENT] = "AGENT"; /* not supported */
		content_name[CTS_VCARD_VALUE_ORG] = "ORG";
		//content_name[CTS_VCARD_VALUE_CATEGORIES] = "CATEGORIES"; /* not supported */
		content_name[CTS_VCARD_VALUE_NOTE] = "NOTE";
		//content_name[CTS_VCARD_VALUE_PRODID] = "PRODID"; /* not supported */
		content_name[CTS_VCARD_VALUE_REV] = "REV";
		//content_name[CTS_VCARD_VALUE_SORT-STRING] = "SORT-STRING"; /* not supported */
		//content_name[CTS_VCARD_VALUE_SOUND] = "SOUND"; /* not supported */
		content_name[CTS_VCARD_VALUE_UID] = "UID";
		content_name[CTS_VCARD_VALUE_URL] = "URL";
		//content_name[CTS_VCARD_VALUE_VERSION] = "VERSION"; /* not supported */
		//content_name[CTS_VCARD_VALUE_CLASS] = "CLASS";         /* not supported */
		//content_name[CTS_VCARD_VALUE_KEY] = "KEY"; /* not supported */
		content_name[CTS_VCARD_VALUE_X_ANNIVERSARY] = "X-ANNIVERSARY";
		//content_name[CTS_VCARD_VALUE_X_CHILDREN] = "X-CHILDREN";
		content_name[CTS_VCARD_VALUE_X_IRMC_LUID] = "X-IRMC-LUID";
		content_name[CTS_VCARD_VALUE_X_SLP_GROUP] = "X-SLP-GROUP";
		content_name[CTS_VCARD_VALUE_END] = "END";
	}
};

int cts_vcard_parse(const void *vcard_stream, CTSstruct **contact, int flags)
{
	int ret, ver;
	contact_t *result;
	char *val_begin, *new_start, *val;
	char *vcard = (char *)vcard_stream;

	retv_if(NULL == vcard_stream, CTS_ERR_ARG_NULL);

	cts_vcard_initial();

	vcard = cts_vcard_check_word(vcard, "BEGIN:VCARD");
	retvm_if(NULL == vcard, CTS_ERR_ARG_INVALID, "The vcard is invalid.");

	val_begin = cts_vcard_check_word(vcard, "VERSION:");
	new_start = cts_vcard_get_val(CTS_VCARD_VER_NONE, val_begin, &val);
	if (NULL == new_start || NULL == val)
		ver = CTS_VCARD_VER_2_1;
	else {
		ver = cts_vcard_check_version(val);
		free(val);
		vcard = new_start;
	}

	result = (contact_t *)contacts_svc_struct_new(CTS_STRUCT_CONTACT);
	retvm_if(NULL == result, CTS_ERR_OUT_OF_MEMORY, "contacts_svc_struct_new() Failed");

	result->name = (cts_name*)contacts_svc_value_new(CTS_VALUE_NAME);
	if (NULL == result->name) {
		ERR("contacts_svc_value_new(CTS_VALUE_NAME) Failed");
		contacts_svc_struct_free((CTSstruct *)result);
		return CTS_ERR_OUT_OF_MEMORY;
	}
	result->name->embedded = true;

	result->base = (cts_ct_base*)contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO);
	if (NULL == result->base) {
		ERR("contacts_svc_value_new(CTS_VALUE_CONTACT_BASE_INFO) Failed");
		contacts_svc_struct_free((CTSstruct *)result);
		return CTS_ERR_OUT_OF_MEMORY;
	}
	result->base->embedded = true;

	ret = cts_vcard_get_contact(ver, flags, vcard, result);

	if (CTS_SUCCESS != ret) {
		contacts_svc_struct_free((CTSstruct *)result);
		if (CTS_ERR_ARG_INVALID == ret)
			ERR("cts_vcard_get_contact() Failed(%d)\n %s \n", ret, vcard);
		else
			ERR("cts_vcard_get_contact() Failed(%d)", ret);

		return ret;
	}

	*contact = (CTSstruct *)result;
	return CTS_SUCCESS;
}

/**************************
 *
 * Contact To VCard
 *
 **************************/

const char *CTS_CRLF = "\r\n";

static inline int cts_vcard_append_name(cts_name *name,
		char *dest, int dest_size)
{
	int ret_len;
	ret_len = snprintf(dest, dest_size, "%s", content_name[CTS_VCARD_VALUE_N]);
	ret_len += snprintf(dest+ret_len, dest_size-ret_len, ":%s",
			SAFE_STR(name->last));
	ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s",
			SAFE_STR(name->first));
	ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s",
			SAFE_STR(name->addition));
	ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s",
			SAFE_STR(name->prefix));
	ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s%s",
			SAFE_STR(name->suffix), CTS_CRLF);

	if (name->display)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:%s%s",
				content_name[CTS_VCARD_VALUE_FN],
				name->display, CTS_CRLF);
	else {
		char display[1024];

		if (name->first && name->last) {
			if (CTS_ORDER_NAME_FIRSTLAST == contacts_svc_get_order(CTS_ORDER_OF_SORTING)) {
				snprintf(display, sizeof(display), "%s %s", name->first, name->last);
			} else {
				int lang;
				if (CTS_LANG_DEFAULT == name->lang_type)
					lang = cts_get_default_language();
				else
					lang = name->lang_type;

				if (CTS_LANG_ENGLISH == lang)
					snprintf(display, sizeof(display), "%s, %s", name->last, name->first);
				else
					snprintf(display, sizeof(display), "%s %s", name->last, name->first);
			}
		}
		else
			snprintf(display, sizeof(display), "%s%s", SAFE_STR(name->first), SAFE_STR(name->last));

		ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:%s%s",
				content_name[CTS_VCARD_VALUE_FN],
				display, CTS_CRLF);
	}

	return ret_len;
}

static inline int cts_vcard_put_number_type(int type, char *dest, int dest_size)
{
	int ret_len = 0;
	if (type & CTS_NUM_TYPE_HOME)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "HOME");
	if (type & CTS_NUM_TYPE_MSG)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "MSG");
	if (type & CTS_NUM_TYPE_WORK)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "WORK");
	if (type & CTS_NUM_TYPE_VOICE)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "VOICE");
	if (type & CTS_NUM_TYPE_FAX)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "FAX");
	if (type & CTS_NUM_TYPE_VOICE)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "VOICE");
	if (type & CTS_NUM_TYPE_CELL)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "CELL");
	if (type & CTS_NUM_TYPE_VIDEO)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "VIDEO");
	if (type & CTS_NUM_TYPE_PAGER)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "PAGER");
	if (type & CTS_NUM_TYPE_BBS)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "BBS");
	if (type & CTS_NUM_TYPE_MODEM)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "MODEM");
	if (type & CTS_NUM_TYPE_CAR)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "CAR");
	if (type & CTS_NUM_TYPE_ISDN)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "ISDN");
	if (type & CTS_NUM_TYPE_PCS)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "PCS");

	return ret_len;
}

static inline int cts_vcard_append_numbers(GSList *numbers,
		char *dest, int dest_size)
{
	int ret_len = 0;
	GSList *cursor;
	cts_number *data;

	for (cursor=numbers;cursor;cursor=cursor->next) {
		data = cursor->data;
		if (data->number) {
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s",
					content_name[CTS_VCARD_VALUE_TEL]);
			ret_len += cts_vcard_put_number_type(data->type, dest+ret_len,
					dest_size-ret_len);
			if (data->is_default)
				ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "PREF");
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, ":%s%s",
					data->number, CTS_CRLF);
		}
	}

	return ret_len;
}

static inline int cts_vcard_append_emails(GSList *emails,
		char *dest, int dest_size)
{
	int ret_len = 0;
	GSList *cursor;
	cts_email *data;

	for (cursor=emails;cursor;cursor=cursor->next) {
		data = cursor->data;
		if (data->email_addr) {
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s",
					content_name[CTS_VCARD_VALUE_EMAIL]);
			if (CTS_EMAIL_TYPE_HOME & data->type)
				ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "HOME");
			if (CTS_EMAIL_TYPE_WORK & data->type)
				ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "WORK");

			if (data->is_default)
				ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "PREF");
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, ":%s%s",
					data->email_addr, CTS_CRLF);
		}
	}

	return ret_len;
}

static inline int cts_vcard_put_postal_type(int type, char *dest, int dest_size)
{
	int ret_len = 0;
	if (type & CTS_ADDR_TYPE_DOM)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "DOM");
	if (type & CTS_ADDR_TYPE_INTL)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "INTL");
	if (type & CTS_ADDR_TYPE_HOME)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "HOME");
	if (type & CTS_ADDR_TYPE_WORK)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "WORK");
	if (type & CTS_ADDR_TYPE_POSTAL)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "POSTAL");
	if (type & CTS_ADDR_TYPE_PARCEL)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "PARCEL");

	return ret_len;
}

static inline int cts_vcard_append_postals(GSList *numbers,
		char *dest, int dest_size)
{
	int ret_len = 0;
	GSList *cursor;
	cts_postal *data;

	for (cursor=numbers;cursor;cursor=cursor->next) {
		data = cursor->data;
		if (data) {
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s",
					content_name[CTS_VCARD_VALUE_ADR]);
			ret_len += cts_vcard_put_postal_type(data->type, dest+ret_len,
					dest_size-ret_len);
			if (data->is_default)
				ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "PREF");
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, ":%s",
					SAFE_STR(data->pobox));
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s",
					SAFE_STR(data->extended));
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s",
					SAFE_STR(data->street));
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s",
					SAFE_STR(data->locality));
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s",
					SAFE_STR(data->region));
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s",
					SAFE_STR(data->postalcode));
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s%s",
					SAFE_STR(data->country), CTS_CRLF);
		}
	}

	return ret_len;
}

static inline int cts_vcard_append_company(cts_company *org,
		char *dest, int dest_size)
{
	int ret_len = 0;

	ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:%s",
			content_name[CTS_VCARD_VALUE_ORG],
			SAFE_STR(org->name));
	if (org->department)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s",
				org->department);

	ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s", CTS_CRLF);

	if (org->jot_title)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:%s%s",
				content_name[CTS_VCARD_VALUE_TITLE],
				org->jot_title, CTS_CRLF);
	if (org->role)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:%s%s",
				content_name[CTS_VCARD_VALUE_ROLE],
				org->role, CTS_CRLF);

	return ret_len;
}

static inline int cts_vcard_append_nicks(GSList *nicks,
		char *dest, int dest_size)
{
	bool first;
	int ret_len = 0;
	GSList *cursor;
	cts_nickname *data;

	ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:",
			content_name[CTS_VCARD_VALUE_NICKNAME]);

	first = true;
	for (cursor=nicks;cursor;cursor=cursor->next) {
		data = cursor->data;
		if (data->nick && *data->nick) {
			if (first) {
				ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s", data->nick);
				first = false;
			}
			else {
				ret_len += snprintf(dest+ret_len, dest_size-ret_len, ",%s", data->nick);
			}
		}
	}

	ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s", CTS_CRLF);

	return ret_len;
}

static inline int cts_vcard_append_webs(GSList *webs,
		char *dest, int dest_size)
{
	int ret_len = 0;
	GSList *cursor;
	cts_web *data;

	for (cursor=webs;cursor;cursor=cursor->next) {
		data = cursor->data;
		if (data->url && *data->url) {
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:%s%s",
					content_name[CTS_VCARD_VALUE_URL],
					data->url, CTS_CRLF);
		}
	}

	return ret_len;
}

static inline int cts_vcard_append_events(GSList *webs,
		char *dest, int dest_size)
{
	int ret_len = 0;
	GSList *cursor;
	cts_event *data;

	for (cursor=webs;cursor;cursor=cursor->next) {
		data = cursor->data;
		if (!data->date) continue;

		if (CTS_EVENT_TYPE_BIRTH == data->type) {
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:%d-%02d-%d%s",
					content_name[CTS_VCARD_VALUE_BDAY],
					data->date/10000, (data->date%10000)/100, data->date%100,
					CTS_CRLF);
		}
		else if (CTS_EVENT_TYPE_ANNIVERSARY == data->type) {
			ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:%d-%02d-%d%s",
					content_name[CTS_VCARD_VALUE_X_ANNIVERSARY],
					data->date/10000, (data->date%10000)/100, data->date%100,
					CTS_CRLF);
		}
	}

	return ret_len;
}

static inline const char* cts_get_photo_type_str(int type)
{
	switch (type)
	{
	case CTS_VCARD_IMG_TIFF:
		return "TIFF";
	case CTS_VCARD_IMG_GIF:
		return "GIF";
	case CTS_VCARD_IMG_PNG:
		return "PNG";
	case CTS_VCARD_IMG_JPEG:
	default:
		return "JPEG";
	}
}

static inline int cts_vcard_put_photo(const char *path, char *dest, int dest_size)
{
	int ret, fd, type;
	gsize read_len;
	char *suffix;
	gchar *buf;
	guchar image[CTS_VCARD_PHOTO_MAX_SIZE];

	suffix = strrchr(path, '.');
	retvm_if(NULL == suffix, 0, "Image Type(%s) is invalid", path);

	type = cts_vcard_get_photo_type(suffix);
	retvm_if(CTS_VCARD_IMG_NONE == type, 0, "Image Type(%s) is invalid", path);

	fd = open(path, O_RDONLY);
	retvm_if(fd < 0, 0, "Open(%s) Failed(%d)", path, errno);

	read_len = 0;
	while ((ret=read(fd, image+read_len, sizeof(image)-read_len))) {
		if (-1 == ret) {
			if (EINTR == errno)
				continue;
			else
				break;
		}
		read_len += ret;
	}
	close(fd);
	retvm_if(ret < 0, 0, "read() Failed(%d)", errno);

	ret = 0;
	buf = g_base64_encode(image, read_len);
	if (buf) {
		ret = snprintf(dest, dest_size, "%s;ENCODING=BASE64;TYPE=%s:%s%s%s",
				content_name[CTS_VCARD_VALUE_PHOTO],
				cts_get_photo_type_str(type), buf, CTS_CRLF, CTS_CRLF);
		g_free(buf);
	}

	return ret;
}
static inline int cts_vcard_append_base(cts_ct_base *base,
		char *dest, int dest_size)
{
	int ret_len = 0;

	if (base->img_path)
		ret_len += cts_vcard_put_photo(base->img_path,
				dest+ret_len, dest_size-ret_len);
	if (base->uid)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:%s%s",
				content_name[CTS_VCARD_VALUE_UID],
				base->uid, CTS_CRLF);
	if (base->note) {
		gchar *escaped_note;
		escaped_note = g_strescape(base->note, NULL);
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:%s%s",
				content_name[CTS_VCARD_VALUE_NOTE],
				escaped_note, CTS_CRLF);
		g_free(escaped_note);
	}

	if (base->changed_time) {
		struct tm ts;
		gmtime_r((time_t *)&base->changed_time, &ts);
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:%04d-%02d-%02dT%02d:%02d:%02dZ%s",
				content_name[CTS_VCARD_VALUE_REV],
				1900+ts.tm_year, 1+ts.tm_mon, ts.tm_mday,
				ts.tm_hour, ts.tm_min, ts.tm_sec,
				CTS_CRLF);
	}

	return ret_len;
}

static inline int cts_vcard_append_grouprelations(GSList *grouprelations,
		char *dest, int dest_size)
{
	bool first;
	int ret_len = 0;
	GSList *cursor;
	cts_group *data;

	ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s:",
			content_name[CTS_VCARD_VALUE_X_SLP_GROUP]);

	first = true;
	for (cursor=grouprelations;cursor;cursor=cursor->next) {
		data = cursor->data;
		if (data->name && *data->name) {
			if (first) {
				ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s", data->name);
				first = false;
			}
			else {
				ret_len += snprintf(dest+ret_len, dest_size-ret_len, ",%s", data->name);
			}
		}
	}

	ret_len += snprintf(dest+ret_len, dest_size-ret_len, "%s", CTS_CRLF);

	return ret_len;
}

static inline int cts_vcard_append_contact(int flags, contact_t *contact,
		char *dest, int dest_size)
{
	int ret_len = 0;

	if (contact->name)
		ret_len += cts_vcard_append_name(contact->name,
				dest+ret_len, dest_size-ret_len);
	if (contact->company)
		ret_len += cts_vcard_append_company(contact->company,
				dest+ret_len, dest_size-ret_len);
	if (contact->postal_addrs)
		ret_len += cts_vcard_append_postals(contact->postal_addrs,
				dest+ret_len, dest_size-ret_len);
	if (contact->numbers)
		ret_len += cts_vcard_append_numbers(contact->numbers,
				dest+ret_len, dest_size-ret_len);
	if (contact->emails)
		ret_len += cts_vcard_append_emails(contact->emails,
				dest+ret_len, dest_size-ret_len);
	if (contact->nicknames)
		ret_len += cts_vcard_append_nicks(contact->nicknames,
				dest+ret_len, dest_size-ret_len);
	if (contact->web_addrs)
		ret_len += cts_vcard_append_webs(contact->web_addrs,
				dest+ret_len, dest_size-ret_len);
	if (contact->events)
		ret_len += cts_vcard_append_events(contact->events,
				dest+ret_len, dest_size-ret_len);
	if (contact->base)
		ret_len += cts_vcard_append_base(contact->base,
				dest+ret_len, dest_size-ret_len);
	if (contact->grouprelations && (flags & CTS_VCARD_CONTENT_X_SLP_GROUP))
		ret_len += cts_vcard_append_grouprelations(contact->grouprelations,
				dest+ret_len, dest_size-ret_len);

	return ret_len;
}

#define CTS_VCARD_FOLDING_LIMIT 75

static inline int cts_vcard_add_folding(char *src)
{
	int len, result_len;
	char result[CTS_VCARD_FILE_MAX_SIZE];
	char *r;
	const char *s;

	s = src;
	r = result;
	len = result_len = 0;
	while (*s) {
		if ('\r' == *s)
			len--;
		else if ('\n' == *s)
			len = -1;

		if (CTS_VCARD_FOLDING_LIMIT == len) {
			*r = '\r';
			r++;
			*r = '\n';
			r++;
			*r = ' ';
			r++;
			len = 1;
			result_len += 3;
		}

		*r = *s;
		r++;
		s++;
		len++;
		result_len++;
		retvm_if(sizeof(result) - 5 < result_len, CTS_ERR_ARG_INVALID,
				"src is too long\n(%s)", src);
	}
	*r = '\0';

	memcpy(src, result, result_len+1);
	return CTS_SUCCESS;
}

int cts_vcard_make(const CTSstruct *contact, char **vcard_stream, int flags)
{
	int ret_len, ret;
	char result[CTS_VCARD_FILE_MAX_SIZE];

	retv_if(NULL == contact, CTS_ERR_ARG_NULL);
	retv_if(NULL == vcard_stream, CTS_ERR_ARG_NULL);
	retvm_if(CTS_STRUCT_CONTACT != contact->s_type, CTS_ERR_ARG_INVALID,
			"The record(%d) must be type of CTS_STRUCT_CONTACT.", contact->s_type);

	cts_vcard_initial();

	ret_len = snprintf(result, sizeof(result), "%s%s", "BEGIN:VCARD", CTS_CRLF);
	ret_len += snprintf(result+ret_len, sizeof(result)-ret_len,
			"%s%s%s", "VERSION:", "3.0", CTS_CRLF);

	ret_len += cts_vcard_append_contact(flags, (contact_t *)contact,
			result+ret_len, sizeof(result)-ret_len);

	retvm_if(sizeof(result)-ret_len <= 0, CTS_ERR_EXCEEDED_LIMIT,
			"This contact has too many information");

	ret_len += snprintf(result+ret_len, sizeof(result)-ret_len,
			"%s%s", "END:VCARD", CTS_CRLF);

	ret = cts_vcard_add_folding(result);
	if (CTS_SUCCESS != ret)
		return ret;
	*vcard_stream = strdup(result);

	return CTS_SUCCESS;
}


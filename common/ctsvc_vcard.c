/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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
#include <glib.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <tzplatform_config.h>

#ifdef _CONTACTS_IPC_CLIENT
#include "ctsvc_client_ipc.h"
#endif

#include "contacts.h"
#include "ctsvc_internal.h"
#include "ctsvc_record.h"
#include "ctsvc_list.h"
#include "ctsvc_localize_utils.h"
#include "ctsvc_notify.h"
#include "ctsvc_image_util.h"

#define DEFAULT_ADDRESS_BOOK_ID 0

#define SMART_STRDUP(src) (src && *src) ? strdup(src) : NULL
#define CTSVC_VCARD_PHOTO_MAX_SIZE 1024*1024

#define CTSVC_VCARD_APPEND_STR(buf, buf_size, len, str) do { \
	if ((len = __ctsvc_vcard_append_str(buf, buf_size, len, str, false)) < 0) { \
		ERR("__ctsvc_vcard_append_str() Fail"); \
		return CONTACTS_ERROR_OUT_OF_MEMORY; \
	} \
} while (0)

#define CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, content) do { \
	if ((len = __ctsvc_vcard_append_str(buf, buf_size, len, content, true)) < 0) { \
		ERR("__ctsvc_vcard_append_str() Fail"); \
		return CONTACTS_ERROR_OUT_OF_MEMORY; \
	} \
} while (0)


#define CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, content) do { \
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";CHARSET=UTF-8"); \
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ":"); \
	CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, content); \
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, CTSVC_CRLF); \
} while (0)

enum {
	CTSVC_VCARD_VER_NONE,
	CTSVC_VCARD_VER_2_1,
	CTSVC_VCARD_VER_3_0,
	CTSVC_VCARD_VER_4_0,
};

enum {
	CTSVC_VCARD_VALUE_NONE,
	CTSVC_VCARD_VALUE_FN,
	CTSVC_VCARD_VALUE_N,
	CTSVC_VCARD_VALUE_PHONETIC_FIRST_NAME,
	CTSVC_VCARD_VALUE_PHONETIC_MIDDLE_NAME,
	CTSVC_VCARD_VALUE_PHONETIC_LAST_NAME,
	CTSVC_VCARD_VALUE_NICKNAME,
	CTSVC_VCARD_VALUE_PHOTO,
	CTSVC_VCARD_VALUE_BDAY,
	CTSVC_VCARD_VALUE_X_ANNIVERSARY,
	CTSVC_VCARD_VALUE_X_TIZEN_EVENT,
	CTSVC_VCARD_VALUE_ADR,
	CTSVC_VCARD_VALUE_TEL,
	CTSVC_VCARD_VALUE_EMAIL,
	CTSVC_VCARD_VALUE_TITLE,
	CTSVC_VCARD_VALUE_ROLE,
	CTSVC_VCARD_VALUE_LOGO,
	CTSVC_VCARD_VALUE_ORG,
	CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_LOCATION,
	CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_DESCRIPTION,
	CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_PHONETIC_NAME,
	CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_ASSISTANT_NAME,
	CTSVC_VCARD_VALUE_NOTE,
	CTSVC_VCARD_VALUE_REV,
	CTSVC_VCARD_VALUE_UID,
	CTSVC_VCARD_VALUE_URL,
	CTSVC_VCARD_VALUE_X_MSN,
	CTSVC_VCARD_VALUE_X_YAHOO,
	CTSVC_VCARD_VALUE_X_ICQ,
	CTSVC_VCARD_VALUE_X_AIM,
	CTSVC_VCARD_VALUE_X_JABBER,
	CTSVC_VCARD_VALUE_X_SKYPE_USERNAME,
	CTSVC_VCARD_VALUE_X_SKYPE,
	CTSVC_VCARD_VALUE_X_QQ,
	CTSVC_VCARD_VALUE_X_GOOGLE_TALK,
	CTSVC_VCARD_VALUE_X_TIZEN_MESSENGER,
	CTSVC_VCARD_VALUE_X_TIZEN_RELATIONSHIP,
	CTSVC_VCARD_VALUE_END,
	CTSVC_VCARD_VALUE_MAX
};

enum {
	CTSVC_VCARD_IMG_NONE,
	CTSVC_VCARD_IMG_PNG,    /* Portable Network Graphics */
	/* vcard  2.1 spec */
	CTSVC_VCARD_IMG_JPEG,   /* ISO JPEG format */
	CTSVC_VCARD_IMG_GIF,    /* Graphics Interchange Format */
	CTSVC_VCARD_IMG_TIFF,   /* Tagged Image File Format */
	CTSVC_VCARD_IMG_CGM,    /* ISO Computer Graphics Metafile */
	CTSVC_VCARD_IMG_WMF,    /* MS Windows Metafile */
	CTSVC_VCARD_IMG_BMP,    /* MS Windows Bitmap */
	CTSVC_VCARD_IMG_MET,    /* IBM PM Metafile */
	CTSVC_VCARD_IMG_PMB,    /* IBM PM Bitmap */
	CTSVC_VCARD_IMG_DIB,    /* MS Windows DIB */
	CTSVC_VCARD_IMG_PICT,   /* Apple Picture format */
	CTSVC_VCARD_IMG_PDF,    /* Adobe Page Description Format */
	CTSVC_VCARD_IMG_PS,     /* Adobe PostScript format */
	CTSVC_VCARD_IMG_QTIME,  /* Apple QuickTime format */
	CTSVC_VCARD_IMG_MPEG,   /* ISO MPEG format */
	CTSVC_VCARD_IMG_MPEG2,  /* ISO MPEG version 2 format */
	CTSVC_VCARD_IMG_AVI,    /* Intel AVI format */
};

static const char *content_name[CTSVC_VCARD_VALUE_MAX] = {0};
const char *CTSVC_CRLF = "\r\n";

static int limit_size_of_photo = CTSVC_IMAGE_MAX_SIZE;

static void __ctsvc_vcard_initial(void)
{
	if (NULL == *content_name) {
		/* content_name[CTSVC_VCARD_VALUE_NAME] = "NAME"; // not supported */
		/* content_name[CTSVC_VCARD_VALUE_PROFILE] = "PROFILE"; // not supported */
		/* content_name[CTSVC_VCARD_VALUE_SOURCE] = "SOURCE"; // not supported */
		content_name[CTSVC_VCARD_VALUE_FN] = "FN";
		content_name[CTSVC_VCARD_VALUE_N] = "N";
		content_name[CTSVC_VCARD_VALUE_PHONETIC_FIRST_NAME] = "X-PHONETIC-FIRST-NAME";
		content_name[CTSVC_VCARD_VALUE_PHONETIC_MIDDLE_NAME] = "X-PHONETIC-MIDDLE-NAME";
		content_name[CTSVC_VCARD_VALUE_PHONETIC_LAST_NAME] = "X-PHONETIC-LAST-NAME";
		content_name[CTSVC_VCARD_VALUE_NICKNAME] = "NICKNAME";
		content_name[CTSVC_VCARD_VALUE_PHOTO] = "PHOTO";
		content_name[CTSVC_VCARD_VALUE_BDAY] = "BDAY";
		content_name[CTSVC_VCARD_VALUE_X_ANNIVERSARY] = "ANNIVERSARY";
		content_name[CTSVC_VCARD_VALUE_X_TIZEN_EVENT] = "X-TIZEN-EVENT";
		content_name[CTSVC_VCARD_VALUE_ADR] = "ADR";
		/* content_name[CTSVC_VCARD_VALUE_LABEL] = "LABEL"; // not supported */
		content_name[CTSVC_VCARD_VALUE_TEL] = "TEL";
		content_name[CTSVC_VCARD_VALUE_EMAIL] = "EMAIL";
		/* content_name[CTSVC_VCARD_VALUE_MAILER] = "MAILER"; // not supported */
		/* content_name[CTSVC_VCARD_VALUE_TZ] = "TZ"; // not supported */
		/* content_name[CTSVC_VCARD_VALUE_GEO] = "GEO"; // not supported */
		content_name[CTSVC_VCARD_VALUE_TITLE] = "TITLE";
		content_name[CTSVC_VCARD_VALUE_ROLE] = "ROLE";
		content_name[CTSVC_VCARD_VALUE_LOGO] = "LOGO";
		/* content_name[CTSVC_VCARD_VALUE_AGENT] = "AGENT"; // not supported */
		content_name[CTSVC_VCARD_VALUE_ORG] = "ORG";
		content_name[CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_LOCATION] = "X-TIZEN-COMPANY-LOCATION";
		content_name[CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_DESCRIPTION] = "X-TIZEN-COMPANY-DESCRIPTION";
		content_name[CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_PHONETIC_NAME] = "X-TIZEN-COMPANY-PHONETIC-NAME";
		content_name[CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_ASSISTANT_NAME] = "X-TIZEN-COMPANY-ASSISTANT-NAME";
		/* content_name[CTSVC_VCARD_VALUE_CATEGORIES] = "CATEGORIES"; // not supported */
		content_name[CTSVC_VCARD_VALUE_NOTE] = "NOTE";
		/* content_name[CTSVC_VCARD_VALUE_PRODID] = "PRODID"; // not supported */
		content_name[CTSVC_VCARD_VALUE_REV] = "REV";
		/* content_name[CTSVC_VCARD_VALUE_SORT-STRING] = "SORT-STRING"; // not supported */
		content_name[CTSVC_VCARD_VALUE_UID] = "UID";
		content_name[CTSVC_VCARD_VALUE_URL] = "URL";
		/* content_name[CTSVC_VCARD_VALUE_VERSION] = "VERSION"; // not supported */
		/* content_name[CTSVC_VCARD_VALUE_CLASS] = "CLASS"; // not supported */
		/* content_name[CTSVC_VCARD_VALUE_KEY] = "KEY"; // not supported */
		content_name[CTSVC_VCARD_VALUE_X_MSN] = "X-MSN";
		content_name[CTSVC_VCARD_VALUE_X_YAHOO] = "X-YAHOO";
		content_name[CTSVC_VCARD_VALUE_X_ICQ] = "X-ICQ";
		content_name[CTSVC_VCARD_VALUE_X_AIM] = "X-AIM";
		content_name[CTSVC_VCARD_VALUE_X_JABBER] = "X-JABBER";
		content_name[CTSVC_VCARD_VALUE_X_SKYPE_USERNAME] = "X-SKYPE-USERNAME";
		content_name[CTSVC_VCARD_VALUE_X_SKYPE] = "X-SKYPE";
		content_name[CTSVC_VCARD_VALUE_X_QQ] = "X-QQ";
		content_name[CTSVC_VCARD_VALUE_X_GOOGLE_TALK] = "X-GOOGLE-TALK";
		content_name[CTSVC_VCARD_VALUE_X_TIZEN_MESSENGER] = "X-TIZEN-MESSENGER";
		content_name[CTSVC_VCARD_VALUE_X_TIZEN_RELATIONSHIP] = "X-TIZEN-RELATIONSHIP";
		/* content_name[CTSVC_VCARD_VALUE_X_CHILDREN] = "X-CHILDREN"; */
		content_name[CTSVC_VCARD_VALUE_END] = "END";
	}
};

static int __ctsvc_vcard_append_str(char **buf, int *buf_size, int len, const char *str, bool need_conversion)
{
	int len_temp = 0;
	char *tmp = NULL;
	const char *safe_str = SAFE_STR(str);
	int str_len = 0;
	bool need_realloc = false;

	str_len = strlen(safe_str);
	while ((*buf_size-len) < (str_len+1)) {
		*buf_size = *buf_size * 2;
		need_realloc = true;
	}

	if (need_realloc) {
		if (NULL == (tmp = realloc(*buf, *buf_size)))
			return -1;
		else
			*buf = tmp;
	}

	if (need_conversion) {
		const char *s = safe_str;
		char *r = (char *)(*buf+len);

		while (*s) {
			switch (*s) {
			case '\r':
				if (*(s+1) && '\n' == *(s+1)) {
					s++;
					*r = '\\';
					r++;
					*r = 'n';
				} else {
					*r = *s;
				}
				break;
			case '\n':
				*r = '\\';
				r++;
				str_len++;
				if (*buf_size < str_len+len+1) {
					*buf_size = *buf_size * 2;
					if (NULL == (tmp = realloc(*buf, *buf_size))) {
						return -1;
					} else {
						*buf = tmp;
						r = (char *)(*buf+len+str_len);
					}
				}
				*r = 'n';
				break;
			case ';':
			case ':':
			case ',':
			case '<':
			case '>':
			case '\\':
				*r = '\\';
				r++;
				str_len++;
				if (*buf_size < str_len+len+1) {
					*buf_size = *buf_size * 2;
					if (NULL == (tmp = realloc(*buf, *buf_size))) {
						return -1;
					} else {
						*buf = tmp;
						r = (char *)(*buf+len+str_len);
					}
				}
				*r = *s;
				break;
			case 0xA1:
				if (*(s+1) && 0xAC == *(s+1)) { /* en/em backslash */
					*r = '\\';
					r++;
					str_len++;
					if (*buf_size < str_len+len+1) {
						*buf_size = *buf_size * 2;
						if (NULL == (tmp = realloc(*buf, *buf_size))) {
							return -1;
						} else {
							*buf = tmp;
							r = (char *)(*buf+len+str_len);
						}
					}

					*r = *s;
					r++;
					s++;
					if (*buf_size < str_len+len+1) {
						*buf_size = *buf_size * 2;
						if (NULL == (tmp = realloc(*buf, *buf_size))) {
							return -1;
						} else {
							*buf = tmp;
							r = (char *)(*buf+len+str_len);
						}
					}
					*r = *s;
				} else {
					*r = *s;
				}
				break;
			case 0x81:
				if (*(s+1) && 0x5F == *(s+1)) { /* en/em backslash */
					*r = '\\';
					r++;
					str_len++;
					if (*buf_size < str_len+len+1) {
						*buf_size = *buf_size * 2;
						if (NULL == (tmp = realloc(*buf, *buf_size))) {
							return -1;
						} else {
							*buf = tmp;
							r = (char *)(*buf+len+str_len);
						}
					}

					*r = *s;
					r++;
					s++;
					if (*buf_size < str_len+len+1) {
						*buf_size = *buf_size * 2;
						if (NULL == (tmp = realloc(*buf, *buf_size))) {
							return -1;
						} else {
							*buf = tmp;
							r = (char *)(*buf+len+str_len);
						}
					}
					*r = *s;
				} else {
					*r = *s;
				}
				break;
			default:
				*r = *s;
				break;
			}
			r++;
			s++;
		}
		len_temp = str_len;
	} else {
		len_temp = snprintf(*buf+len, *buf_size-len+1, "%s", safe_str);
	}
	len += len_temp;
	return len;
}

#define CTS_VCARD_FOLDING_LIMIT 75

static inline int __ctsvc_vcard_add_folding(char **buf, int *buf_size, int buf_len)
{
	int char_len = 0;
	char *buf_copy = NULL;
	int len, result_len = 0;
	char *r;
	const char *s;
	bool content_start = false;
	bool encode_64 = false;

	buf_copy = calloc(1, *buf_size);
	if (NULL == buf_copy) {
		ERR("calloc() Fail");
		return 0;
	}

	s = *buf;
	r = buf_copy;
	len = result_len;

	while (*s) {
		if (*buf_size < result_len + 5) {
			char *tmp = NULL;
			*buf_size = *buf_size + 1000;
			if (NULL == (tmp = realloc(buf_copy, *buf_size))) {
				free(buf_copy);
				return -1;
			} else {
				buf_copy = tmp;
				r = (buf_copy + result_len);
			}
		}

		if (false == content_start) {
			if (':' == *s)
				content_start = true;
			else if (STRING_EQUAL == strncmp(s, "ENCODING=BASE64", strlen("ENCODING=BASE64")))
				encode_64 = true;
		}

		if ('\r' == *s) {
			len--;
		} else if ('\n' == *s) {
			len = -1;
			char_len = 0;
			content_start = false;
			encode_64 = false;
		}

		if (0 == char_len) {
			if (false == encode_64)
				char_len = ctsvc_check_utf8(*s);

			if (CTS_VCARD_FOLDING_LIMIT <= len + char_len) {
				*r = '\r';
				r++;
				*r = '\n';
				r++;
				*r = ' ';
				r++;
				len = 1;
				result_len += 3;
			}
		}

		if (char_len)
			char_len--;

		*r = *s;
		r++;
		s++;
		len++;
		result_len++;
	}
	*r = '\0';
	free(*buf);
	*buf = buf_copy;
	return result_len;
}

static inline int __ctsvc_vcard_append_name(ctsvc_list_s *names, char **buf, int *buf_size, int len)
{
	char display[1024] = {0};
	GList *cursor = names->records;
	ctsvc_name_s *name;

	RETV_IF(NULL == cursor, len);

	name = cursor->data;

	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_N]);

	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";CHARSET=UTF-8");
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ":");
	CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, name->last);
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";");
	CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, name->first);
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";");
	CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, name->addition);
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";");
	CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, name->prefix);
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";");
	CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, name->suffix);

	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, CTSVC_CRLF);

	if (name->first && name->last) {
		contacts_name_display_order_e order = CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST;
#ifdef _CONTACTS_IPC_CLIENT
		contacts_setting_get_name_display_order(&order);
#endif
		if (CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST == order) {
			snprintf(display, sizeof(display), "%s %s", name->first, name->last);
		} else {
			/* CONTACTS_NAME_DISPLAY_ORDER_LASTFIRST */
			snprintf(display, sizeof(display), "%s, %s", name->last, name->first);
		}
	} else {
		snprintf(display, sizeof(display), "%s%s", SAFE_STR(name->first), SAFE_STR(name->last));
	}

	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_FN]);
	CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, display);

	if (name->phonetic_first) {
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_PHONETIC_FIRST_NAME]);
		CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, name->phonetic_first);
	}

	if (name->phonetic_middle) {
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_PHONETIC_MIDDLE_NAME]);
		CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, name->phonetic_middle);
	}


	if (name->phonetic_last) {
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_PHONETIC_LAST_NAME]);
		CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, name->phonetic_last);
	}

	return len;
}

static inline const char* __ctsvc_get_img_suffix(int type)
{
	switch (type) {
	case CTSVC_VCARD_IMG_TIFF:
		return "tiff";
	case CTSVC_VCARD_IMG_GIF:
		return "gif";
	case CTSVC_VCARD_IMG_PNG:
		return "png";
	case CTSVC_VCARD_IMG_CGM:
		return "cgm";
	case CTSVC_VCARD_IMG_WMF:
		return "wmf";
	case CTSVC_VCARD_IMG_BMP:
		return "bmp";
	case CTSVC_VCARD_IMG_MET:
		return "met";
	case CTSVC_VCARD_IMG_PMB:
		return "pmb";
	case CTSVC_VCARD_IMG_DIB:
		return "dib";
	case CTSVC_VCARD_IMG_PICT:
		return "pict";
	case CTSVC_VCARD_IMG_PDF:
		return "pdf";
	case CTSVC_VCARD_IMG_PS:
		return "ps";
	case CTSVC_VCARD_IMG_QTIME:
		return "qtime";
	case CTSVC_VCARD_IMG_MPEG:
		return "mpeg";
	case CTSVC_VCARD_IMG_MPEG2:
		return "mpeg2";
	case CTSVC_VCARD_IMG_AVI:
		return "avi";
	case CTSVC_VCARD_IMG_JPEG:
	case CTSVC_VCARD_IMG_NONE:
	default:
		return "jpeg";
	}
}

static inline int __ctsvc_vcard_get_image_type(char *val)
{
	char *temp, *result;
	RETV_IF(NULL == val, CTSVC_VCARD_IMG_NONE);

	temp = val;
	while (*temp) {
		*temp = tolower(*temp);
		temp++;
	}

	result = strstr(val, "jpeg");
	if (result) return CTSVC_VCARD_IMG_JPEG;

	result = strstr(val, "jpg");
	if (result) return CTSVC_VCARD_IMG_JPEG;

	result = strstr(val, "png");
	if (result) return CTSVC_VCARD_IMG_PNG;

	result = strstr(val, "gif");
	if (result) return CTSVC_VCARD_IMG_GIF;

	result = strstr(val, "tiff");
	if (result) return CTSVC_VCARD_IMG_TIFF;

	result = strstr(val, "cgm");
	if (result) return CTSVC_VCARD_IMG_CGM;

	result = strstr(val, "wmf");
	if (result) return CTSVC_VCARD_IMG_WMF;

	result = strstr(val, "bmp");
	if (result) return CTSVC_VCARD_IMG_BMP;

	result = strstr(val, "met");
	if (result) return CTSVC_VCARD_IMG_MET;

	result = strstr(val, "pmb");
	if (result) return CTSVC_VCARD_IMG_PMB;

	result = strstr(val, "dib");
	if (result) return CTSVC_VCARD_IMG_DIB;

	result = strstr(val, "pict");
	if (result) return CTSVC_VCARD_IMG_PICT;

	result = strstr(val, "pdf");
	if (result) return CTSVC_VCARD_IMG_PDF;

	result = strstr(val, "ps");
	if (result) return CTSVC_VCARD_IMG_PS;

	result = strstr(val, "qtime");
	if (result) return CTSVC_VCARD_IMG_QTIME;

	result = strstr(val, "mpeg");
	if (result) return CTSVC_VCARD_IMG_MPEG;

	result = strstr(val, "mpeg2");
	if (result) return CTSVC_VCARD_IMG_MPEG2;

	result = strstr(val, "avi");
	if (result) return CTSVC_VCARD_IMG_AVI;

	return CTSVC_VCARD_IMG_NONE;
}

static inline const char* __ctsvc_get_image_type_str(int type)
{
	switch (type) {
	case CTSVC_VCARD_IMG_TIFF:
		return "TIFF";
	case CTSVC_VCARD_IMG_GIF:
		return "GIF";
	case CTSVC_VCARD_IMG_PNG:
		return "PNG";
	case CTSVC_VCARD_IMG_CGM:
		return "CGM";
	case CTSVC_VCARD_IMG_WMF:
		return "WMF";
	case CTSVC_VCARD_IMG_BMP:
		return "BMP";
	case CTSVC_VCARD_IMG_MET:
		return "MET";
	case CTSVC_VCARD_IMG_PMB:
		return "PMB";
	case CTSVC_VCARD_IMG_DIB:
		return "DIB";
	case CTSVC_VCARD_IMG_PICT:
		return "PICT";
	case CTSVC_VCARD_IMG_PDF:
		return "PDF";
	case CTSVC_VCARD_IMG_PS:
		return "PS";
	case CTSVC_VCARD_IMG_QTIME:
		return "QTIME";
	case CTSVC_VCARD_IMG_MPEG:
		return "MPEG";
	case CTSVC_VCARD_IMG_MPEG2:
		return "MPEG2";
	case CTSVC_VCARD_IMG_AVI:
		return "AVI";
	case CTSVC_VCARD_IMG_JPEG:
	default:
		return "JPEG";
	}
}

static inline int __ctsvc_vcard_put_company_logo(const char *path, char **buf, int *buf_size, int len)
{
	int ret, fd, type;
	gsize read_len;
	char *suffix;
	gchar *buf_image;
	guchar image[CTSVC_VCARD_PHOTO_MAX_SIZE] = {0};

	suffix = strrchr(path, '.');
	type = __ctsvc_vcard_get_image_type(suffix);

	fd = open(path, O_RDONLY);
	RETVM_IF(fd < 0, CONTACTS_ERROR_SYSTEM, "System : Open Fail(%d)", errno);

	read_len = 0;
	while ((ret = read(fd, image+read_len, sizeof(image)-read_len))) {
		if (-1 == ret) {
			if (EINTR == errno)
				continue;
			else
				break;
		}
		read_len += ret;
	}
	close(fd);
	RETVM_IF(ret < 0, CONTACTS_ERROR_SYSTEM, "System : read() Fail(%d)", errno);

	buf_image = g_base64_encode(image, read_len);
	if (buf_image) {
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_LOGO]);
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";ENCODING=BASE64;TYPE=");
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, __ctsvc_get_image_type_str(type));
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ":");
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, buf_image);
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, CTSVC_CRLF);
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, CTSVC_CRLF);
		g_free(buf_image);
	}
	return len;
}

static bool __ctsvc_vcard_is_valid_custom_label(char *label)
{
	char *src = label;
	RETV_IF(NULL == label || '\0' == *label, false);

	while (*src) {
		char c = src[0];
		RETV_IF(1 != ctsvc_check_utf8(c), false);
		if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
				('0' <= c && c <= '9') || c == '-') {
			src++;
			continue;
		}
		return false;
	}
	return true;
}

static inline int __ctsvc_vcard_put_company_type(int type, char *label, char **buf, int *buf_size, int len)
{
	if (type == CONTACTS_COMPANY_TYPE_WORK) {
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=WORK");
	} else if (type == CONTACTS_COMPANY_TYPE_CUSTOM) {
		if (__ctsvc_vcard_is_valid_custom_label(label)) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=X-");
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, label);
		}
	}
	return len;
}

static inline int __ctsvc_vcard_append_company(ctsvc_list_s *company_list, char **buf, int *buf_size, int len)
{
	GList *cursor;
	ctsvc_company_s *company;

	for (cursor = company_list->records; cursor; cursor = cursor->next) {

		company = cursor->data;

		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_ORG]);

		len = __ctsvc_vcard_put_company_type(company->type, SAFE_STR(company->label), buf, buf_size, len);
		RETV_IF(len < 0, CONTACTS_ERROR_OUT_OF_MEMORY);

		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";CHARSET=UTF-8");
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ":");
		CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, company->name);
		if (company->department) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";");
			CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, company->department);
		}

		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, CTSVC_CRLF);

		if (company->job_title) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_TITLE]);
			CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, company->job_title);
		}

		if (company->role) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_ROLE]);
			CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, company->role);
		}

		if (company->location) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_LOCATION]);
			CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, company->location);
		}

		if (company->description) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_DESCRIPTION]);
			CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, company->description);
		}

		if (company->phonetic_name) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_PHONETIC_NAME]);
			CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, company->phonetic_name);
		}

		if (company->assistant_name) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_ASSISTANT_NAME]);
			CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, company->assistant_name);
		}

		if (company->logo) {
			len = __ctsvc_vcard_put_company_logo(company->logo, buf, buf_size, len);
			RETV_IF(len < 0, CONTACTS_ERROR_OUT_OF_MEMORY);
		}
	}

	return len;
}

static inline int __ctsvc_vcard_append_note(ctsvc_list_s *note_list, char **buf, int *buf_size, int len)
{
	GList *cursor;
	ctsvc_note_s *note;

	for (cursor = note_list->records; cursor; cursor = cursor->next) {
		note = cursor->data;
		if (note->note) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_NOTE]);
			CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, note->note);
		}
	}

	return len;
}

static inline int __ctsvc_vcard_2_put_postal_type(int type, char *dest, int dest_size)
{
	int ret_len = 0;

	if (type & CONTACTS_ADDRESS_TYPE_DOM)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "DOM");
	if (type & CONTACTS_ADDRESS_TYPE_INTL)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "INTL");
	if (type & CONTACTS_ADDRESS_TYPE_HOME)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "HOME");
	if (type & CONTACTS_ADDRESS_TYPE_WORK)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "WORK");
	if (type & CONTACTS_ADDRESS_TYPE_POSTAL)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "POSTAL");
	if (type & CONTACTS_ADDRESS_TYPE_PARCEL)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "PARCEL");

	return ret_len;
}

static inline int __ctsvc_vcard_put_postal_type(int type, char *label, char **buf, int *buf_size, int len)
{
	char *type_str = NULL;
	if (type == CONTACTS_ADDRESS_TYPE_DOM)
		type_str = "DOM";
	else if (type == CONTACTS_ADDRESS_TYPE_INTL)
		type_str = "INTL";
	else if (type == CONTACTS_ADDRESS_TYPE_HOME)
		type_str = "HOME";
	else if (type == CONTACTS_ADDRESS_TYPE_WORK)
		type_str = "WORK";
	else if (type == CONTACTS_ADDRESS_TYPE_POSTAL)
		type_str = "POSTAL";
	else if (type == CONTACTS_ADDRESS_TYPE_PARCEL)
		type_str = "PARCEL";

	if (type == CONTACTS_ADDRESS_TYPE_CUSTOM) {
		if (__ctsvc_vcard_is_valid_custom_label(label)) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=X-");
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, label);
		}
		return len;
	}

	if (type_str) {
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=");
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, type_str);
	}
	return len;
}

static inline int __ctsvc_vcard_append_postals(ctsvc_list_s *address_list, char **buf, int *buf_size, int len)
{
	GList *cursor;
	ctsvc_address_s *address;

	for (cursor = address_list->records; cursor; cursor = cursor->next) {
		address = cursor->data;
		if (address) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_ADR]);

			len = __ctsvc_vcard_put_postal_type(address->type, SAFE_STR(address->label), buf, buf_size, len);
			RETV_IF(len < 0, CONTACTS_ERROR_OUT_OF_MEMORY);

			if (address->is_default)
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";PREF");

			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";CHARSET=UTF-8");
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ":");
			CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, address->pobox);
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";");
			CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, address->extended);
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";");
			CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, address->street);
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";");
			CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, address->locality);
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";");
			CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, address->region);
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";");
			CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, address->postalcode);
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";");
			CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, address->country);

			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, CTSVC_CRLF);
		}
	}

	return len;
}

static inline int __ctsvc_vcard_append_nicknames(ctsvc_list_s *nickname_list, char **buf, int *buf_size, int len)
{
	bool first;
	GList *cursor;
	ctsvc_nickname_s *nickname;

	first = true;
	for (cursor = nickname_list->records; cursor; cursor = cursor->next) {
		nickname = cursor->data;
		if (nickname->nickname && *nickname->nickname) {
			if (first) {
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_NICKNAME]);
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";CHARSET=UTF-8");
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ":");
				CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, nickname->nickname);
				first = false;
			} else {
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ",");
				CTSVC_VCARD_APPEND_CONTENT_STR(buf, buf_size, len, nickname->nickname);
			}
		}
	}
	if (false == first)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, CTSVC_CRLF);

	return len;
}

static inline int __ctsvc_vcard_2_put_number_type(int type, char *dest, int dest_size)
{
	int ret_len = 0;
	if (type & CONTACTS_NUMBER_TYPE_HOME)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "HOME");
	if (type & CONTACTS_NUMBER_TYPE_MSG)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "MSG");
	if (type & CONTACTS_NUMBER_TYPE_WORK)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "WORK");
	if (type & CONTACTS_NUMBER_TYPE_VOICE)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "VOICE");
	if (type & CONTACTS_NUMBER_TYPE_FAX)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "FAX");
	if (type & CONTACTS_NUMBER_TYPE_VOICE)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "VOICE");
	if (type & CONTACTS_NUMBER_TYPE_CELL)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "CELL");
	if (type & CONTACTS_NUMBER_TYPE_VIDEO)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "VIDEO");
	if (type & CONTACTS_NUMBER_TYPE_PAGER)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "PAGER");
	if (type & CONTACTS_NUMBER_TYPE_BBS)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "BBS");
	if (type & CONTACTS_NUMBER_TYPE_MODEM)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "MODEM");
	if (type & CONTACTS_NUMBER_TYPE_CAR)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "CAR");
	if (type & CONTACTS_NUMBER_TYPE_ISDN)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "ISDN");
	if (type & CONTACTS_NUMBER_TYPE_PCS)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "PCS");

	return ret_len;
}

static inline int __ctsvc_vcard_put_number_type(int type, char *label, char **buf, int *buf_size, int len)
{
	if (type & CONTACTS_NUMBER_TYPE_HOME)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=HOME");
	if (type & CONTACTS_NUMBER_TYPE_MSG)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=MSG");
	if (type & CONTACTS_NUMBER_TYPE_WORK)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=WORK");
	if (type & CONTACTS_NUMBER_TYPE_VOICE)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=VOICE");
	if (type & CONTACTS_NUMBER_TYPE_FAX)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=FAX");
	if (type & CONTACTS_NUMBER_TYPE_CELL)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=CELL");
	if (type & CONTACTS_NUMBER_TYPE_VIDEO)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=VIDEO");
	if (type & CONTACTS_NUMBER_TYPE_PAGER)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=PAGER");
	if (type & CONTACTS_NUMBER_TYPE_BBS)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=BBS");
	if (type & CONTACTS_NUMBER_TYPE_MODEM)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=MODEM");
	if (type & CONTACTS_NUMBER_TYPE_CAR)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=CAR");
	if (type & CONTACTS_NUMBER_TYPE_ISDN)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=ISDN");
	if (type & CONTACTS_NUMBER_TYPE_PCS)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=PCS");
	if (type & CONTACTS_NUMBER_TYPE_ASSISTANT)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=X-ASSISTANT");
	if (type & CONTACTS_NUMBER_TYPE_RADIO)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=X-RADIO");
	if (type & CONTACTS_NUMBER_TYPE_COMPANY_MAIN)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=X-COMPANY-MAIN");
	if (type & CONTACTS_NUMBER_TYPE_MAIN)
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=X-MAIN");
	if (type == CONTACTS_NUMBER_TYPE_CUSTOM) {
		if (__ctsvc_vcard_is_valid_custom_label(label)) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=X-");
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, label);
		}
		return len;
	}
	return len;
}

static int __ctsvc_vcard_check_utf8(char c)
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

static void __ctsvc_vcard_get_clean_number_for_export(char *str, char *dest)
{
	int char_len = 0;
	char *s = SAFE_STR(str);
	char *r = NULL;

	r = dest;

	while (*s) {
		char_len = __ctsvc_vcard_check_utf8(*s);
		if (3 == char_len) {
			if (*s == 0xef) {
				if (*(s+1) == 0xbc) {
					if (0x90 <= *(s+2) && *(s+2) <= 0x99) {
						/* ef bc 90 : '0' ~ ef bc 99 : '9' */
						*r = '0' + (*(s+2) - 0x90);
						r++;
						s += 3;
					} else if (0x8b == *(s+2)) {
						/* ef bc 8b : '+' */
						*r = '+';
						r++;
						s += 3;
					} else if (0x8a == *(s+2)) {
						/* ef bc 8a : '*' */
						*r = '*';
						r++;
						s += 3;
					} else if (0x83 == *(s+2)) {
						/* ef bc 83 : '#' */
						*r = '#';
						r++;
						s += 3;
					} else if (0x8c == *(s+2)) {
						/* ef bc 8c : ',' */
						*r = 'p';
						r++;
						s += 3;
					} else if (0x9b == *(s+2)) {
						/* ef bc 9b : ';' */
						*r = 'w';
						r++;
						s += 3;
					} else {
						s += char_len;
					}
				} else {
					s += char_len;
				}
			} else {
				s += char_len;
			}
		} else if (1 == char_len) {
			switch (*s) {
			case '/':
			case '.':
			case '0' ... '9':
			case '#':
			case '*':
			case '(':
			case ')':
			case '+':
				*r = *s;
				r++;
				s++;
				break;
			case ',':
			case 'p':
			case 'P':
				*r = 'p';
				r++;
				s++;
				break;
			case ';':
			case 'w':
			case 'W':
				*r = 'w';
				r++;
				s++;
				break;
			case 'a' ... 'o':
			case 'q' ... 'v':
			case 'x' ... 'z':
				*r = *s - 0x20;
				r++;
				s++;
				break;
			case 'A' ... 'O':
			case 'Q' ... 'V':
			case 'X' ... 'Z':
				*r = *s;
				r++;
				s++;
				break;
			default:
				s++;
				break;
			}
		} else {
			s += char_len;
		}
	}
	*r = '\0';
	return;
}

static inline int __ctsvc_vcard_append_numbers(ctsvc_list_s *number_list, char **buf, int *buf_size, int len)
{
	GList *cursor;
	ctsvc_number_s *number;

	for (cursor = number_list->records; cursor; cursor = cursor->next) {
		number = cursor->data;
		if (number->number) {
			char clean_number[strlen(number->number)+1];
			clean_number[0] = '\0';
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_TEL]);

			len = __ctsvc_vcard_put_number_type(number->type, SAFE_STR(number->label), buf, buf_size, len);
			RETV_IF(len < 0, CONTACTS_ERROR_OUT_OF_MEMORY);

			if (number->is_default)
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";PREF");

			__ctsvc_vcard_get_clean_number_for_export(number->number, clean_number);
			if (*clean_number)
				CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, clean_number);
		}
	}
	return len;
}

static inline int __ctsvc_vcard_2_put_email_type(int type, char *dest, int dest_size)
{
	int ret_len = 0;

	if (CONTACTS_EMAIL_TYPE_HOME & type)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "HOME");
	if (CONTACTS_EMAIL_TYPE_WORK & type)
		ret_len += snprintf(dest+ret_len, dest_size-ret_len, ";%s", "WORK");

	return ret_len;
}

static inline int __ctsvc_vcard_put_email_type(int type, char *label, char **buf, int *buf_size, int len)
{
	char *type_str = NULL;
	if (CONTACTS_EMAIL_TYPE_HOME == type) {
		type_str = "HOME";
	} else if (CONTACTS_EMAIL_TYPE_WORK == type) {
		type_str = "WORK";
	} else if (CONTACTS_EMAIL_TYPE_MOBILE == type) {
		type_str = "CELL";
	} else if (CONTACTS_EMAIL_TYPE_CUSTOM == type) {
		if (__ctsvc_vcard_is_valid_custom_label(label)) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=X-");
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, label);
		}
		return len;
	}

	if (type_str) {
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=");
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, type_str);
	}
	return len;
}

static inline int __ctsvc_vcard_append_emails(ctsvc_list_s *email_list, char **buf, int *buf_size, int len)
{
	GList *cursor;
	ctsvc_email_s *email;

	for (cursor = email_list->records; cursor; cursor = cursor->next) {
		email = cursor->data;
		if (email->email_addr) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_EMAIL]);

			len = __ctsvc_vcard_put_email_type(email->type, SAFE_STR(email->label), buf, buf_size, len);
			RETV_IF(len < 0, CONTACTS_ERROR_OUT_OF_MEMORY);

			if (email->is_default)
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";PREF");

			CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, email->email_addr);
		}
	}
	return len;
}

static inline int __ctsvc_vcard_put_url_type(int type, char *label, char **buf, int *buf_size, int len)
{
	char *type_str = NULL;

	if (CONTACTS_URL_TYPE_HOME == type) {
		type_str = "HOME";
	} else if (CONTACTS_URL_TYPE_WORK == type) {
		type_str = "WORK";
	} else if (CONTACTS_URL_TYPE_CUSTOM == type) {
		if (__ctsvc_vcard_is_valid_custom_label(label)) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=X-");
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, label);
		}
		return len;
	}
	if (type_str) {
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=");
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, type_str);
	}
	return len;
}

static inline int __ctsvc_vcard_append_webs(ctsvc_list_s *url_list, char **buf, int *buf_size, int len)
{
	GList *cursor;
	ctsvc_url_s *url;

	for (cursor = url_list->records; cursor; cursor = cursor->next) {
		url = cursor->data;

		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_URL]);

		len = __ctsvc_vcard_put_url_type(url->type, SAFE_STR(url->label), buf, buf_size, len);
		RETV_IF(len < 0, CONTACTS_ERROR_OUT_OF_MEMORY);

		CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, url->url);
	}

	return len;
}

#define VCARD_INIT_LENGTH 1024
#define VCARD_ITEM_LENGTH 1024

static inline int __ctsvc_vcard_append_events(ctsvc_list_s *event_list, char **buf, int *buf_size, int len)
{
	GList *cursor;
	ctsvc_event_s *data;
	char event[VCARD_ITEM_LENGTH] = {0};

	for (cursor = event_list->records; cursor; cursor = cursor->next) {
		data = cursor->data;
		if (0 == data->date) continue;

		event[0] = '\0';
		if (CONTACTS_EVENT_TYPE_BIRTH == data->type) {
			snprintf(event, sizeof(event), "%s:%d-%02d-%02d%s",
					content_name[CTSVC_VCARD_VALUE_BDAY],
					data->date/10000, (data->date%10000)/100, data->date%100,
					CTSVC_CRLF);
		} else if (CONTACTS_EVENT_TYPE_ANNIVERSARY == data->type) {
			snprintf(event, sizeof(event), "%s;TYPE=ANNIVERSARY:%d-%02d-%02d%s",
					content_name[CTSVC_VCARD_VALUE_X_TIZEN_EVENT],
					data->date/10000, (data->date%10000)/100, data->date%100,
					CTSVC_CRLF);
		} else if (CONTACTS_EVENT_TYPE_CUSTOM == data->type) {
			if (__ctsvc_vcard_is_valid_custom_label(data->label)) {
				snprintf(event, sizeof(event), "%s;TYPE=X-%s:%d-%02d-%02d%s",
						content_name[CTSVC_VCARD_VALUE_X_TIZEN_EVENT],
						SAFE_STR(data->label),
						data->date/10000, (data->date%10000)/100, data->date%100,
						CTSVC_CRLF);
			} else {
				snprintf(event, sizeof(event), "%s:%d-%02d-%02d%s",
						content_name[CTSVC_VCARD_VALUE_X_TIZEN_EVENT],
						data->date/10000, (data->date%10000)/100, data->date%100,
						CTSVC_CRLF);
			}
		} else {
			snprintf(event, sizeof(event), "%s:%d-%02d-%02d%s",
					content_name[CTSVC_VCARD_VALUE_X_TIZEN_EVENT],
					data->date/10000, (data->date%10000)/100, data->date%100,
					CTSVC_CRLF);
		}
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, event);
	}

	return len;
}

static inline int __ctsvc_vcard_append_messengers(ctsvc_list_s *messenger_list, char **buf, int *buf_size, int len)
{
	GList *cursor;
	ctsvc_messenger_s *messenger;
	const char *content_name_messenger = NULL;
	const char *content_name_x_type = NULL;

	for (cursor = messenger_list->records; cursor; cursor = cursor->next) {
		messenger = cursor->data;

		content_name_messenger = NULL;
		content_name_x_type = NULL;

		if (messenger->im_id && *messenger->im_id) {
			switch (messenger->type) {
			case CONTACTS_MESSENGER_TYPE_WLM:
				content_name_messenger = content_name[CTSVC_VCARD_VALUE_X_MSN];
				break;
			case CONTACTS_MESSENGER_TYPE_YAHOO:
				content_name_messenger = content_name[CTSVC_VCARD_VALUE_X_YAHOO];
				break;
			case CONTACTS_MESSENGER_TYPE_ICQ:
				content_name_messenger = content_name[CTSVC_VCARD_VALUE_X_ICQ];
				break;
			case CONTACTS_MESSENGER_TYPE_AIM:
				content_name_messenger = content_name[CTSVC_VCARD_VALUE_X_AIM];
				break;
			case CONTACTS_MESSENGER_TYPE_JABBER:
				content_name_messenger = content_name[CTSVC_VCARD_VALUE_X_JABBER];
				break;
			case CONTACTS_MESSENGER_TYPE_SKYPE:
				content_name_messenger = content_name[CTSVC_VCARD_VALUE_X_SKYPE_USERNAME];
				break;
			case CONTACTS_MESSENGER_TYPE_QQ:
				content_name_messenger = content_name[CTSVC_VCARD_VALUE_X_QQ];
				break;
			case CONTACTS_MESSENGER_TYPE_GOOGLE:
				content_name_messenger = content_name[CTSVC_VCARD_VALUE_X_GOOGLE_TALK];
				break;
			case CONTACTS_MESSENGER_TYPE_FACEBOOK:
				content_name_x_type = "FACEBOOK";
				break;
			case CONTACTS_MESSENGER_TYPE_IRC:
				content_name_x_type = "IRC";
				break;
			case CONTACTS_MESSENGER_TYPE_CUSTOM:
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_X_TIZEN_MESSENGER]);
				if (__ctsvc_vcard_is_valid_custom_label(messenger->label)) {
					CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=X-");
					CTSVC_VCARD_APPEND_STR(buf, buf_size, len, messenger->label);
				}
				CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, messenger->im_id);
				break;
			default:
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_X_TIZEN_MESSENGER]);
				CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, messenger->im_id);
				break;
			}

			if (content_name_messenger) {
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name_messenger);
				CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, messenger->im_id);
			} else if (content_name_x_type) {
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_X_TIZEN_MESSENGER]);
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=");
				CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name_x_type);
				CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, messenger->im_id);
			}
		}
	}
	return len;
}

static inline int __ctsvc_vcard_put_relationship_type(int type, char *label, char **buf, int *buf_size, int len)
{
	const char *type_str = NULL;

	switch (type) {
	case CONTACTS_RELATIONSHIP_TYPE_ASSISTANT:
		type_str = "ASSISTANT";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_BROTHER:
		type_str = "BROTHER";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_CHILD:
		type_str = "CHILD";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_DOMESTIC_PARTNER:
		type_str = "DOMESTIC_PARTNER";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_FATHER:
		type_str = "FATHER";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_FRIEND:
		type_str = "FRIEND";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_MANAGER:
		type_str = "MANAGER";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_MOTHER:
		type_str = "MOTHER";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_PARENT:
		type_str = "PARENT";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_PARTNER:
		type_str = "PARTNER";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_REFERRED_BY:
		type_str = "REFERRED_BY";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_RELATIVE:
		type_str = "RELATIVE";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_SISTER:
		type_str = "SISTER";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_SPOUSE:
		type_str = "SPOUSE";
		break;
	case CONTACTS_RELATIONSHIP_TYPE_CUSTOM:
		if (__ctsvc_vcard_is_valid_custom_label(label)) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=X-");
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, label);
		}
		return len;
	}

	if (type_str) {
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";TYPE=");
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, type_str);
	}

	return len;
}

static inline int __ctsvc_vcard_append_relationships(ctsvc_list_s *relationship_list, char **buf, int *buf_size, int len)
{
	GList *cursor;
	ctsvc_relationship_s *relationship;

	for (cursor = relationship_list->records; cursor; cursor = cursor->next) {
		relationship = cursor->data;

		if (relationship->name) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, "X-TIZEN-RELATIONSHIP");

			len = __ctsvc_vcard_put_relationship_type(relationship->type, SAFE_STR(relationship->label), buf, buf_size, len);
			RETV_IF(len < 0, CONTACTS_ERROR_OUT_OF_MEMORY);
			CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, relationship->name);
		}
	}

	return len;
}

typedef struct {
	const char *src;
	unsigned char **image;
	unsigned int *image_size;
	int ret;
} vcard_image_info;

static bool _ctsvc_vcard_image_util_supported_jpeg_colorspace_cb(
		image_util_colorspace_e colorspace, void *user_data)
{
	int width = 0;
	int height = 0;
	int mimetype = 0;
	uint64_t size = 0;
	void *buffer = NULL;
	void *buffer_temp = NULL;
	int ret;
	vcard_image_info *info = user_data;

	ret = ctsvc_image_util_get_mimetype(colorspace, &mimetype);
	if (CONTACTS_ERROR_NONE != ret) {
		info->ret = CONTACTS_ERROR_SYSTEM;
		return true;
	}

	ret = image_util_decode_jpeg(info->src, colorspace, (unsigned char **)&buffer,
			&width, &height, (unsigned int *)&size);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		info->ret = CONTACTS_ERROR_SYSTEM;
		return true;
	}

	if (limit_size_of_photo < width || limit_size_of_photo < height) { /* need resize */
		int resized_width;
		int resized_height;
		media_format_h fmt;
		media_packet_h packet;

		/* set resize */
		if (width > height) {
			resized_width = limit_size_of_photo;
			resized_height = height * limit_size_of_photo / width;
		} else {
			resized_height = limit_size_of_photo;
			resized_width = width * limit_size_of_photo / height;
		}

		if (resized_height % 8)
			resized_height -= (resized_height % 8);

		if (resized_width % 8)
			resized_width -= (resized_width % 8);

		fmt = ctsvc_image_util_create_media_format(mimetype, width, height);
		if (NULL == fmt) {
			ERR("_ctsvc_image_create_media_format() Fail");
			info->ret = CONTACTS_ERROR_SYSTEM;
			free(buffer);
			return false;
		}

		packet = ctsvc_image_util_create_media_packet(fmt, buffer, (unsigned int)size);
		if (NULL == packet) {
			ERR("_ctsvc_image_create_media_packet() Fail");
			media_format_unref(fmt);
			info->ret = CONTACTS_ERROR_SYSTEM;
			free(buffer);
			return false;
		}

		ret = ctsvc_image_util_resize(packet, resized_width, resized_height, &buffer_temp,
				&size);

		media_packet_destroy(packet);
		media_format_unref(fmt);

		if (CONTACTS_ERROR_NONE != ret) {
			free(buffer);
			info->ret = CONTACTS_ERROR_SYSTEM;
			return false;
		}
		free(buffer);
		buffer = buffer_temp;

		width = resized_width;
		height = resized_height;
	}

	ret = image_util_encode_jpeg_to_memory(buffer, width, height, colorspace,
			CTSVC_IMAGE_ENCODE_QUALITY, info->image, info->image_size);
	free(buffer);
	if (IMAGE_UTIL_ERROR_NONE != ret) {
		ERR("image_util_encode_jpeg_to_memory", ret);
		info->ret = CONTACTS_ERROR_SYSTEM;
		return false;
	}

	info->ret = CONTACTS_ERROR_NONE;
	return false;
}

static inline int __ctsvc_vcard_encode_photo(const char *src,
		unsigned char **image, unsigned int *image_size)
{
	int ret;
	vcard_image_info info = {.src = src, .image = image, .image_size = image_size, ret = CONTACTS_ERROR_SYSTEM};

	ret = image_util_foreach_supported_jpeg_colorspace(
			_ctsvc_vcard_image_util_supported_jpeg_colorspace_cb, &info);

	if (IMAGE_UTIL_ERROR_NONE != ret)
		return CONTACTS_ERROR_SYSTEM;

	return info.ret;
}

static inline int __ctsvc_vcard_put_photo(ctsvc_list_s *image_list, char **buf, int *buf_size, int len)
{
	int ret = CONTACTS_ERROR_NONE, fd, type;
	unsigned int read_len;
	char *suffix;
	gchar *buf_image;
	unsigned char *image = NULL;
	unsigned int img_buf_size = 0;
	GList *cursor;
	ctsvc_image_s *data;

	for (cursor = image_list->records; cursor; cursor = cursor->next) {
		data = cursor->data;
		if (NULL == data->path) continue;

		ret = __ctsvc_vcard_encode_photo(data->path, &image, &read_len);

		if (CONTACTS_ERROR_NONE != ret) {
			INFO("__ctsvc_vcard_encode_photo() Fail(%d)", ret);

			img_buf_size = CTSVC_VCARD_PHOTO_MAX_SIZE * sizeof(unsigned char);
			image = calloc(1, img_buf_size);
			if (NULL == image) {
				ERR("calloc() Fail");
				return CONTACTS_ERROR_OUT_OF_MEMORY;
			}

			fd = open(data->path, O_RDONLY);
			if (fd < 0) {
				ERR("System : Open Fail(%d)", errno);
				return CONTACTS_ERROR_SYSTEM;
			}

			read_len = 0;
			while ((ret = read(fd, image+read_len, img_buf_size-read_len))) {
				if (-1 == ret) {
					if (EINTR == errno)
						continue;
					else
						break;
				}
				read_len += ret;
			}
			close(fd);
			if (ret < 0) {
				ERR("System : read() Fail(%d)", errno);
				free(image);
				return CONTACTS_ERROR_SYSTEM;
			}
		}

		suffix = strrchr(data->path, '.');
		type = __ctsvc_vcard_get_image_type(suffix);

		buf_image = g_base64_encode(image, read_len);
		free(image);

		if (buf_image) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_PHOTO]);
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ";ENCODING=BASE64;TYPE=");
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, __ctsvc_get_image_type_str(type));
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, ":");
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, buf_image);
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, CTSVC_CRLF);
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, CTSVC_CRLF);
			g_free(buf_image);
		}
	}

	return len;
}

static inline int __ctsvc_vcard_append_contact(ctsvc_contact_s *contact, char **buf, int *buf_size, int len)
{
	if (contact->name) {
		len = __ctsvc_vcard_append_name(contact->name, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (contact->company) {
		len = __ctsvc_vcard_append_company(contact->company, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (contact->note) {
		len = __ctsvc_vcard_append_note(contact->note, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (contact->postal_addrs) {
		len = __ctsvc_vcard_append_postals(contact->postal_addrs, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (contact->numbers) {
		len = __ctsvc_vcard_append_numbers(contact->numbers, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (contact->emails) {
		len = __ctsvc_vcard_append_emails(contact->emails, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (contact->nicknames) {
		len = __ctsvc_vcard_append_nicknames(contact->nicknames, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (contact->urls) {
		len = __ctsvc_vcard_append_webs(contact->urls, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (contact->events) {
		len = __ctsvc_vcard_append_events(contact->events, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (contact->images) {
		len = __ctsvc_vcard_put_photo(contact->images, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (contact->messengers) {
		len = __ctsvc_vcard_append_messengers(contact->messengers, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (contact->relationships) {
		len = __ctsvc_vcard_append_relationships(contact->relationships, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}

	if (contact->uid && DEFAULT_ADDRESS_BOOK_ID == contact->addressbook_id) {
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_UID]);
		CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, contact->uid);
	}

	if (contact->changed_time) {
		struct tm ts;
		gmtime_r((time_t *)&contact->changed_time, &ts);
		char temp[VCARD_ITEM_LENGTH] = {0};
		snprintf(temp, sizeof(temp), "%s:%04d-%02d-%02dT%02d:%02d:%02dZ%s",
				content_name[CTSVC_VCARD_VALUE_REV],
				1900+ts.tm_year, 1+ts.tm_mon, ts.tm_mday,
				ts.tm_hour, ts.tm_min, ts.tm_sec,
				CTSVC_CRLF);

		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, temp);
	}
#if 0
	ctsvc_list_s *profile;
#endif
	return len;
}

static inline int __ctsvc_vcard_append_my_profile(ctsvc_my_profile_s *my_profile, char **buf, int *buf_size, int len)
{
	if (my_profile->name) {
		len = __ctsvc_vcard_append_name(my_profile->name, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (my_profile->company) {
		len = __ctsvc_vcard_append_company(my_profile->company, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (my_profile->note) {
		len = __ctsvc_vcard_append_note(my_profile->note, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (my_profile->postal_addrs) {
		len = __ctsvc_vcard_append_postals(my_profile->postal_addrs, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (my_profile->numbers) {
		len = __ctsvc_vcard_append_numbers(my_profile->numbers, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (my_profile->emails) {
		len = __ctsvc_vcard_append_emails(my_profile->emails, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (my_profile->nicknames) {
		len = __ctsvc_vcard_append_nicknames(my_profile->nicknames, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (my_profile->urls) {
		len = __ctsvc_vcard_append_webs(my_profile->urls, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (my_profile->events) {
		len = __ctsvc_vcard_append_events(my_profile->events, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (my_profile->images) {
		len = __ctsvc_vcard_put_photo(my_profile->images, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (my_profile->messengers) {
		len = __ctsvc_vcard_append_messengers(my_profile->messengers, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}
	if (my_profile->relationships) {
		len = __ctsvc_vcard_append_relationships(my_profile->relationships, buf, buf_size, len);
		RETV_IF(len < 0, len);
	}

	if (my_profile->uid && DEFAULT_ADDRESS_BOOK_ID == my_profile->addressbook_id) {
		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_UID]);
		CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, my_profile->uid);
	}

	if (my_profile->changed_time) {
		struct tm ts;
		gmtime_r((time_t *)&my_profile->changed_time, &ts);
		char temp[VCARD_ITEM_LENGTH] = {0};
		snprintf(temp, sizeof(temp), "%s:%04d-%02d-%02dT%02d:%02d:%02dZ%s",
				content_name[CTSVC_VCARD_VALUE_REV],
				1900+ts.tm_year, 1+ts.tm_mon, ts.tm_mday,
				ts.tm_hour, ts.tm_min, ts.tm_sec,
				CTSVC_CRLF);

		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, temp);
	}

#if 0
	ctsvc_list_s *profile;
#endif
	return len;
}

static inline int __ctsvc_vcard_append_start_vcard_3_0(char **buf, int *buf_size, int len)
{
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, "BEGIN:VCARD");
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, CTSVC_CRLF);
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, "VERSION:3.0");
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, CTSVC_CRLF);
	return len;
}

static inline int __ctsvc_vcard_append_end_vcard(char **buf, int *buf_size, int len)
{
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, "END:VCARD");
	CTSVC_VCARD_APPEND_STR(buf, buf_size, len, CTSVC_CRLF);
	return len;
}

static int __ctsvc_vcard_make(ctsvc_contact_s *contact, char **vcard_stream)
{
	char *buf = NULL;
	int buf_size = VCARD_INIT_LENGTH;
	int len = 0;

	__ctsvc_vcard_initial();

	buf = calloc(1, buf_size);
	if (NULL == buf) {
		ERR("calloc() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	len = __ctsvc_vcard_append_start_vcard_3_0(&buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	len = __ctsvc_vcard_append_contact(contact, &buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	len = __ctsvc_vcard_append_end_vcard(&buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	len = __ctsvc_vcard_add_folding(&buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}
	*vcard_stream = buf;

	return CONTACTS_ERROR_NONE;
}

static int __ctsvc_vcard_make_from_my_profile(ctsvc_my_profile_s *my_profile, char **vcard_stream)
{
	char *buf = NULL;
	int buf_size = VCARD_INIT_LENGTH;
	int len = 0;

	__ctsvc_vcard_initial();

	buf = calloc(1, buf_size);
	if (NULL == buf) {
		ERR("calloc() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	len = __ctsvc_vcard_append_start_vcard_3_0(&buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	len = __ctsvc_vcard_append_my_profile(my_profile, &buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	len = __ctsvc_vcard_append_end_vcard(&buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	len = __ctsvc_vcard_add_folding(&buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	*vcard_stream = buf;

	return CONTACTS_ERROR_NONE;
}

API int contacts_vcard_make_from_contact(contacts_record_h record, char **vcard_stream)
{
	ctsvc_contact_s *contact;
	RETV_IF(NULL == vcard_stream, CONTACTS_ERROR_INVALID_PARAMETER);
	*vcard_stream = NULL;

	RETVM_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER,
			"contact(%p), vcard_stream(%p)", record, vcard_stream);

	contact = (ctsvc_contact_s*)record;
	RETVM_IF(CTSVC_RECORD_CONTACT != contact->base.r_type, CONTACTS_ERROR_INVALID_PARAMETER,
			"The record is not conatct record (type : %d)", contact->base.r_type);

	return __ctsvc_vcard_make(contact, vcard_stream);
}

API int contacts_vcard_make_from_my_profile(contacts_record_h record, char **vcard_stream)
{
	ctsvc_my_profile_s *my_profile;
	RETV_IF(NULL == vcard_stream, CONTACTS_ERROR_INVALID_PARAMETER);
	*vcard_stream = NULL;

	RETVM_IF(NULL == record, CONTACTS_ERROR_INVALID_PARAMETER,
			"my_profile(%p), vcard_stream(%p)", record, vcard_stream);

	my_profile = (ctsvc_my_profile_s*)record;
	RETVM_IF(CTSVC_RECORD_MY_PROFILE != my_profile->base.r_type, CONTACTS_ERROR_INVALID_PARAMETER,
			"The record is not conatct record (type : %d)", my_profile->base.r_type);

	return __ctsvc_vcard_make_from_my_profile(my_profile, vcard_stream);
}

#ifdef _CONTACTS_IPC_CLIENT
static int __ctsvc_vcard_append_person(ctsvc_person_s *person, ctsvc_list_s *list_contacts, char **buf, int *buf_size, int len)
{
	int changed_time = 0;
	ctsvc_contact_s *contact;
	GList *cursor = NULL;

	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->id == person->name_contact_id && contact->name) {
			len = __ctsvc_vcard_append_name(contact->name, buf, buf_size, len);
			RETV_IF(len < 0, len);
		}
	}

	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->company && contact->company->cursor) {
			len = __ctsvc_vcard_append_company(contact->company, buf, buf_size, len);
			RETV_IF(len < 0, len);
		}
	}

	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->note && contact->note->cursor) {
			len = __ctsvc_vcard_append_note(contact->note, buf, buf_size, len);
			RETV_IF(len < 0, len);
		}
	}
	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->postal_addrs && contact->postal_addrs->cursor) {
			len = __ctsvc_vcard_append_postals(contact->postal_addrs, buf, buf_size, len);
			RETV_IF(len < 0, len);
		}
	}
	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->numbers && contact->numbers->cursor) {
			len = __ctsvc_vcard_append_numbers(contact->numbers, buf, buf_size, len);
			RETV_IF(len < 0, len);
		}
	}

	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->emails && contact->emails->cursor) {
			len = __ctsvc_vcard_append_emails(contact->emails, buf, buf_size, len);
			RETV_IF(len < 0, len);
		}
	}

	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->nicknames && contact->nicknames->cursor) {
			len = __ctsvc_vcard_append_nicknames(contact->nicknames, buf, buf_size, len);
			RETV_IF(len < 0, len);
		}
	}
	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->urls && contact->urls->cursor) {
			len = __ctsvc_vcard_append_webs(contact->urls, buf, buf_size, len);
			RETV_IF(len < 0, len);
		}
	}

	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->events && contact->events->cursor) {
			len = __ctsvc_vcard_append_events(contact->events, buf, buf_size, len);
			RETV_IF(len < 0, len);
		}
	}
	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->images && contact->images->cursor) {
			len = __ctsvc_vcard_put_photo(contact->images, buf, buf_size, len);
			RETV_IF(len < 0, len);
		}
	}
	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->messengers && contact->messengers->cursor) {
			len = __ctsvc_vcard_append_messengers(contact->messengers, buf, buf_size, len);
			RETV_IF(len < 0, len);
		}
	}

	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->relationships && contact->relationships->cursor) {
			len = __ctsvc_vcard_append_relationships(contact->relationships, buf, buf_size, len);
			RETV_IF(len < 0, len);
		}
	}

	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && contact->uid && DEFAULT_ADDRESS_BOOK_ID == contact->addressbook_id) {
			CTSVC_VCARD_APPEND_STR(buf, buf_size, len, content_name[CTSVC_VCARD_VALUE_UID]);
			CTSVC_VCARD_APPEND_CONTENT(buf, buf_size, len, contact->uid);
		}
	}
	for (cursor = list_contacts->records; cursor; cursor = cursor->next) {
		contact = cursor->data;
		if (contact && changed_time < contact->changed_time)
			changed_time = contact->changed_time;
	}

	if (changed_time) {
		struct tm ts;
		gmtime_r((time_t*)&changed_time, &ts);
		char temp[VCARD_ITEM_LENGTH] = {0};
		snprintf(temp, sizeof(temp), "%s:%04d-%02d-%02dT%02d:%02d:%02dZ%s",
				content_name[CTSVC_VCARD_VALUE_REV],
				1900+ts.tm_year, 1+ts.tm_mon, ts.tm_mday,
				ts.tm_hour, ts.tm_min, ts.tm_sec,
				CTSVC_CRLF);

		CTSVC_VCARD_APPEND_STR(buf, buf_size, len, temp);
	}

#if 0
	ctsvc_list_s *profile;
#endif
	return len;
}
#endif /* _CONTACTS_IPC_CLIENT */

#ifdef _CONTACTS_IPC_CLIENT
static int __ctsvc_vcard_make_from_person(ctsvc_person_s *person, ctsvc_list_s *list_contacts,
		char **vcard_stream)
{
	char *buf = NULL;
	int buf_size = VCARD_INIT_LENGTH;
	int len = 0;

	RETV_IF(NULL == vcard_stream, CONTACTS_ERROR_INVALID_PARAMETER);
	*vcard_stream = NULL;

	__ctsvc_vcard_initial();

	buf = calloc(1, buf_size);
	if (NULL == buf) {
		ERR("calloc() Fail");
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	len = __ctsvc_vcard_append_start_vcard_3_0(&buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	len = __ctsvc_vcard_append_person(person, list_contacts, &buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}
	len = __ctsvc_vcard_append_end_vcard(&buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	len = __ctsvc_vcard_add_folding(&buf, &buf_size, len);
	if (len < 0) {
		free(buf);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	*vcard_stream = buf;

	return CONTACTS_ERROR_NONE;
}
#endif /* _CONTACTS_IPC_CLIENT */

#ifdef _CONTACTS_IPC_CLIENT
API int contacts_vcard_make_from_person(contacts_record_h record, char **vcard_stream)
{
	int ret;
	ctsvc_person_s *person;
	contacts_query_h query = NULL;
	contacts_filter_h filter = NULL;
	contacts_list_h list = NULL;

	RETVM_IF(NULL == record || NULL == vcard_stream, CONTACTS_ERROR_INVALID_PARAMETER,
			"person(%p), vcard_stream(%p)", record, vcard_stream);
	*vcard_stream = NULL;

	person = (ctsvc_person_s*)record;

	RETVM_IF(CTSVC_RECORD_PERSON != person->base.r_type, CONTACTS_ERROR_INVALID_PARAMETER,
			"The record is not conatct record (type : %d)", person->base.r_type);

	do {
		if (CONTACTS_ERROR_NONE != (ret = contacts_filter_create(_contacts_contact._uri, &filter))) break;
		if (CONTACTS_ERROR_NONE != (ret = contacts_filter_add_int(filter, _contacts_contact.person_id, CONTACTS_MATCH_EQUAL, person->person_id))) break;
		if (CONTACTS_ERROR_NONE != (ret = contacts_query_create(_contacts_contact._uri, &query))) break;
		if (CONTACTS_ERROR_NONE != (ret = contacts_query_set_filter(query, filter))) break;
		if (CONTACTS_ERROR_NONE != (ret = contacts_db_get_records_with_query(query, 0, 0, &list))) break;
		if (CONTACTS_ERROR_NONE != (ret = __ctsvc_vcard_make_from_person(person, (ctsvc_list_s*)list, vcard_stream))) break;
	} while (0);
	WARN_IF(CONTACTS_ERROR_NONE != ret, "__ctsvc_vcard_make_from_person() Fail(%d)", ret);
	contacts_query_destroy(query);
	contacts_filter_destroy(filter);
	contacts_list_destroy(list, true);
	return ret;
}
#endif

static inline char* __ctsvc_vcard_remove_empty_line(char *src)
{
	while (*src) {
		if ('\n' != *src && '\r' != *src)
			break;
		src++;
	}
	return src;
}

static char* __ctsvc_vcard_check_word(char *src, const char *word)
{
	bool start = false;

	RETV_IF(NULL == src, NULL);

	src = __ctsvc_vcard_remove_empty_line(src);

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

static int __ctsvc_vcard_check_content_type(char **vcard)
{
	int i;
	char *new_start;

	for (i = CTSVC_VCARD_VALUE_NONE+1; i < CTSVC_VCARD_VALUE_MAX; i++) {
		new_start = __ctsvc_vcard_check_word(*vcard, content_name[i]);
		if (new_start && (':' == *new_start || ';' == *new_start))
			break;
	}

	if (CTSVC_VCARD_VALUE_MAX == i) {
		return CTSVC_VCARD_VALUE_NONE;
	} else {
		*vcard = new_start;
		return i;
	}
}

static inline char* __ctsvc_vcard_pass_unsupported(char *vcard)
{
	while (*vcard) {
		if ('\n' == *vcard)
			return (vcard + 1);
		vcard++;
	}

	return NULL;
}

static char* __ctsvc_strtok(char *val, char c)
{
	char *before = NULL;
	while (*val) {
		if (*val == c && (NULL == before || *before != '\\')) {
			*val = '\0';
			return (val+1);
		}
		before = val;
		val++;
	}
	return val;
}

static inline bool __ctsvc_vcard_check_base64_encoded(char *src)
{
	int ret;
	char *tmp = src;

	while (*tmp) {
		if ('B' == *tmp) {
			ret = strncmp(tmp, "BASE64", sizeof("BASE64") - 1);
			if (STRING_EQUAL == ret)
				return true;
		} else if (':' == *tmp || '\r' == *tmp) {
			break;
		}
		tmp++;
	}
	return false;
}

static inline int __ctsvc_vcard_check_quoted(char *src, int max, int *quoted)
{
	int ret;
	if (TRUE == *quoted)
		return TRUE;

	while (*src && max) {
		if ('Q' == *src) {
			ret = strncmp(src, "QUOTED-PRINTABLE", sizeof("QUOTED-PRINTABLE") - 1);
			if (STRING_EQUAL == ret) {
				*quoted = TRUE;
				return TRUE;
			}
		} else if (':' == *src) {
			break;
		}
		src++;
		max--;
	}
	return FALSE;
}

static inline int __ctsvc_vcard_remove_folding(char *folded_src)
{
	char *result = folded_src;

	RETV_IF(NULL == folded_src, CONTACTS_ERROR_INVALID_PARAMETER);

	while (*folded_src) {
		if ('\r' == *folded_src && '\n' == *(folded_src+1) && ' ' == *(folded_src+2))
			folded_src += 3;
		else if ('\n' == *folded_src && ' ' == *(folded_src+1))
			folded_src += 2;

		if ('\0' == *folded_src)
			break;

		*result = *folded_src;
		result++;
		folded_src++;
	}
	*result = '\0';
	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_vcard_hex_to_dec(char hex)
{
	switch (hex) {
	case '0' ... '9':
		return hex - '0';
	case 'a' ... 'f':
		return hex - 'a' + 10;
	case 'A' ... 'F':
		return hex - 'A' + 10;
	default:
		return -1;
	}
}
static inline int __ctsvc_vcard_decode_quoted_val(char *val)
{
	char *src, *dest;
	int pre;

	src = strchr(val, ':');
	if (NULL == src)
		src = val;

	dest = src;
	while (*src) {
		if ('=' == *src) {
			pre = __ctsvc_vcard_hex_to_dec(*(src+1));
			if (0 <= pre) {
				*dest = (char)((pre << 4) + __ctsvc_vcard_hex_to_dec(*(src+2)));
				dest++;
				src += 2;
			} else {
				if ('\r' == *(src+1) && '\n' == *(src+2))
					src += 2;
			}
		} else {
			*dest = *src;
			dest++;
		}
		src++;
	}

	*dest = '\0';
	return dest - val;
}

static inline char* __ctsvc_vcard_translate_charset(char *src, int len)
{
	int ret;
	char *val = src;

	while (*val) {
		if ('C' == *val) {
			ret = strncmp(val, "CHARSET", sizeof("CHARSET") - 1);
			if (STRING_EQUAL == ret) {
				val += sizeof("CHARSET");
				break;
			}
		} else if (':' == *val) {
			return NULL;
		}
		val++;
	}

	if (*val) {
		UChar *temp;
		UConverter *conv;
		UErrorCode err = U_ZERO_ERROR;
		int dest_size = 0;
		int temp_size = 0;
		int src_len, i = 0;
		char enc[32] = {0}, *dest;

		while (';' != *val && ':' != *val)
			enc[i++] = *val++;

		enc[i] = '\0';
		if (0 == strcasecmp("UTF-8", enc))
			return NULL;

		while (':' != *val)
			val++;

		src_len = len - (val - src);

		temp_size = (src_len+1) * sizeof(UChar);
		temp = malloc(temp_size);
		if (NULL == temp) {
			ERR("malloc() Fail");
			return NULL;
		}
		conv = ucnv_open(enc, &err);
		WARN_IF(U_FAILURE(err), "ucnv_open() Fail(%d), enc=%s", err, enc);
		ucnv_toUChars(conv, temp, temp_size, val, src_len, &err);
		WARN_IF(U_FAILURE(err), "ucnv_toUChars() Fail(%d), enc=%s", err, enc);
		ucnv_close(conv);

		dest_size = temp_size*2;
		dest = malloc(dest_size);
		if (NULL == dest) {
			ERR("malloc() Fail");
			free(temp);
			return NULL;
		}
		conv = ucnv_open("UTF-8", &err);
		WARN_IF(U_FAILURE(err), "ucnv_open() Fail(%d), enc=%s", err, enc);
		ucnv_fromUChars(conv, dest, dest_size, temp, u_strlen(temp), &err);
		WARN_IF(U_FAILURE(err), "ucnv_fromUChars() Fail(%d), enc=%s", err, enc);
		ucnv_close(conv);
		free(temp);

		return dest;
	}
	return NULL;
}

static void __ctsvc_vcard_get_prefix(char **prefix, char *src)
{
	char *temp = strchr(src, ':');
	if (temp) {
		int len = (int)temp - (int)src;
		*prefix = calloc(len+1, sizeof(char));
		if (*prefix)
			snprintf(*prefix, len+1, "%s", src);
	} else {
		*prefix = NULL;
	}
}

static char* __ctsvc_vcard_get_val(int ver, char *src, char **prefix, char **dest)
{
	int quoted;
	bool start = false;
	char *cursor;

	RETV_IF(NULL == src, NULL);
	RETV_IF(NULL == dest, NULL);

	while (*src) {
		switch (*src) {
		case '\n':
			return NULL;
		case '\r':
		case ' ':
			src++;
			break;
		default:
			start = true;
			break;
		}
		if (start) break;
	}

	quoted = FALSE;
	cursor = src;
	if (CTSVC_VCARD_VER_2_1 == ver) {
		while (*cursor) {
			if ('=' == *cursor && __ctsvc_vcard_check_quoted(src, cursor - src, &quoted)) {
				if ('\r' == *(cursor+1) && '\n' == *(cursor+2))
					cursor += 2;
			} else {
				if ('\r' == *cursor && '\n' == *(cursor+1) && ' ' != *(cursor+2))
					break;
				if ('\n' == *cursor && ' ' != *(cursor+1))
					break;
			}

			cursor++;
		}
	} else {
		while (*cursor) {
			if ('\r' == *cursor && '\n' == *(cursor+1) && ' ' != *(cursor+2))
				break;

			if ('\n' == *cursor && ' ' != *(cursor+1))
				break;

			cursor++;
		}
	}

	if (src == cursor) {
		*dest = NULL;
		return NULL;
	} else {
		int len = 0;
		char temp = *cursor;
		char *new_dest;

		if (prefix)
			__ctsvc_vcard_get_prefix(prefix, src);

		*cursor = '\0';
		*dest = strdup(src);
		if (NULL == *dest) {
			ERR("strdup() Fail");
			return NULL;
		}
		if (CTSVC_VCARD_VER_2_1 != ver)
			__ctsvc_vcard_remove_folding(*dest);

		if (__ctsvc_vcard_check_quoted(*dest, -1, &quoted))
			len = __ctsvc_vcard_decode_quoted_val(*dest);
		if (0 == len)
			len = strlen(*dest);
		new_dest = __ctsvc_vcard_translate_charset(*dest, len);
		if (new_dest) {
			free(*dest);
			*dest = new_dest;
		}
		*cursor = temp;
		return (cursor + 1);
	}
}

static inline char* __ctsvc_get_content_value(char *val)
{
	char *temp;

	temp = strchr(val, ':');
	if (temp)
		temp++;
	else
		temp = val;

	RETVM_IF('\0' == *(temp) || '\r' == *(temp) || '\n' == *(temp),
			NULL, "Invalid vcard content");

	return temp;
}

static char* __ctsvc_vcard_remove_escape_char(char *str)
{
	char *s = SAFE_STR(str);
	char *r = s;
	while (*s) {
		if (*s == '\\' && *(s+1)) {
			char *n = (char*)(s+1);
			switch (*n) {
			case 'n':
			case 'N':
				*r = '\n';
				s++;
				break;
			case ';':
			case ':':
			case ',':
			case '<':
			case '>':
			case '\\':
				*r = *n;
				s++;
				break;
			case 0xA1:  /* en/em backslash */
				if (*(n+1) && 0xAC == *(n+1)) {
					*r = *n;
					r++;
					*r = *(n+1);
					s += 2;
				}
				break;
			case 0x81:  /* en/em backslash */
				if (*(n+1) && 0x5F == *(n+1)) {
					*r = *n;
					r++;
					*r = *(n+1);
					s += 2;
				}
				break;
			default:
				*r = *s;
				break;
			}
			r++;
			s++;
		} else {
			*r = *s;
			r++;
			s++;
		}
	}
	*r = '\0';
	return str;
}

static inline int __ctsvc_vcard_get_display_name(ctsvc_list_s *name_list, char *val)
{
	int ret;
	int count;
	char *temp;
	char *first_name = NULL;
	char *last_name = NULL;
	contacts_record_h name;

	temp = __ctsvc_get_content_value(val);
	RETVM_IF(NULL == temp, CONTACTS_ERROR_INVALID_PARAMETER, "vcard");

	contacts_list_get_count((contacts_list_h)name_list, &count);
	if (count <= 0) {
		ret = contacts_record_create(_contacts_name._uri, &name);
		RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create is Fail(%d)", ret);
		contacts_list_add((contacts_list_h)name_list, name);
	} else {
		contacts_list_get_current_record_p((contacts_list_h)name_list, &name);
	}

	ret = contacts_record_get_str_p(name, _contacts_name.first, &first_name);
	WARN_IF(ret != CONTACTS_ERROR_NONE, "contacts_record_get_str_p is Fail(%d)", ret);
	ret = contacts_record_get_str_p(name, _contacts_name.last, &last_name);
	WARN_IF(ret != CONTACTS_ERROR_NONE, "contacts_record_get_str_p is Fail(%d)", ret);

	if ((NULL == first_name || '\0' == *first_name) && (NULL == last_name || '\0' == *last_name))
		contacts_record_set_str(name, _contacts_name.first, __ctsvc_vcard_remove_escape_char(temp));

	return CONTACTS_ERROR_NONE;
}

#define CTS_GET_MULTIPLE_COMPONENT(dest, src, src_temp, separator) \
	src_temp = src; \
separator = false; \
while (src_temp && *src_temp) { \
	if (*src_temp == ';') { \
		separator = true; \
		*src_temp = '\0'; \
		src = __ctsvc_vcard_remove_escape_char(src); \
		dest = SMART_STRDUP(src); \
		src = src_temp+1; \
		break; \
	} \
	else if (*src_temp == '\\') {\
		src_temp += 2; \
		continue; \
	} \
	src_temp++; \
} \
if (false == separator && src && *src) { \
	src = __ctsvc_vcard_remove_escape_char(src); \
	dest = SMART_STRDUP(src); \
	break; \
}

static inline int __ctsvc_vcard_get_name(ctsvc_list_s *name_list, char *val)
{
	int ret;
	int count;
	char *start;
	contacts_record_h name;

	start = __ctsvc_get_content_value(val);
	RETV_IF(NULL == start, CONTACTS_ERROR_NO_DATA);

	contacts_list_get_count((contacts_list_h)name_list, &count);
	if (count <= 0) {
		ret = contacts_record_create(_contacts_name._uri, &name);
		RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);
		contacts_list_add((contacts_list_h)name_list, name);
	} else {
		contacts_list_get_current_record_p((contacts_list_h)name_list, &name);
	}

	contacts_record_set_str(name, _contacts_name.first, NULL);  /* remove FN */

	while (true) {
		bool separator = false;
		char *start_temp;

		CTS_GET_MULTIPLE_COMPONENT(((ctsvc_name_s*)name)->last, start, start_temp, separator);
		CTS_GET_MULTIPLE_COMPONENT(((ctsvc_name_s*)name)->first, start, start_temp, separator);
		CTS_GET_MULTIPLE_COMPONENT(((ctsvc_name_s*)name)->addition, start, start_temp, separator);
		CTS_GET_MULTIPLE_COMPONENT(((ctsvc_name_s*)name)->prefix, start, start_temp, separator);
		CTS_GET_MULTIPLE_COMPONENT(((ctsvc_name_s*)name)->suffix, start, start_temp, separator);

		ERR("invalid name type");
		break;
	}

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_vcard_get_phonetic_name(ctsvc_list_s *name_list, int type, char *val)
{
	int ret;
	int count;
	char *start;
	const char separator = ';';
	contacts_record_h name;

	start = __ctsvc_get_content_value(val);
	RETV_IF(NULL == start, CONTACTS_ERROR_NO_DATA);

	contacts_list_get_count((contacts_list_h)name_list, &count);
	if (count <= 0) {
		ret = contacts_record_create(_contacts_name._uri, &name);
		RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);
		contacts_list_add((contacts_list_h)name_list, name);
	} else {
		contacts_list_get_current_record_p((contacts_list_h)name_list, &name);
	}

	__ctsvc_strtok(start, separator);
	if (CTSVC_VCARD_VALUE_PHONETIC_FIRST_NAME == type)
		contacts_record_set_str(name, _contacts_name.phonetic_first, __ctsvc_vcard_remove_escape_char(start));
	else if (CTSVC_VCARD_VALUE_PHONETIC_MIDDLE_NAME == type)
		contacts_record_set_str(name, _contacts_name.phonetic_middle, __ctsvc_vcard_remove_escape_char(start));
	else if (CTSVC_VCARD_VALUE_PHONETIC_LAST_NAME == type)
		contacts_record_set_str(name, _contacts_name.phonetic_last, __ctsvc_vcard_remove_escape_char(start));

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_vcard_get_nickname(ctsvc_list_s *nickname_list, char *val)
{
	int ret = CONTACTS_ERROR_NONE;
	char *temp;
	char *start;
	char *last;
	const char *separator = ",";

	start = __ctsvc_get_content_value(val);
	RETV_IF(NULL == start, CONTACTS_ERROR_NO_DATA);

	temp = strtok_r(start, separator, &last);
	while (temp) {
		if ('\0' == *temp) continue;

		contacts_record_h nickname = NULL;
		ret = contacts_record_create(_contacts_nickname._uri, &nickname);
		if (ret < CONTACTS_ERROR_NONE) {
			GList *cursor = NULL;
			ERR("contacts_record_create() Fail(%d)", ret);
			for (cursor = nickname_list->records; cursor; cursor = cursor->next)
				contacts_record_destroy((contacts_record_h)(cursor->data), true);
			g_list_free(nickname_list->records);
			nickname_list->records = NULL;
			nickname_list->cursor = NULL;
			nickname_list->count = 0;
			return ret;
		}
		contacts_record_set_str(nickname, _contacts_nickname.name, __ctsvc_vcard_remove_escape_char(start));
		contacts_list_add((contacts_list_h)nickname_list, nickname);

		temp = strtok_r(NULL, separator, &last);
	}

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_vcard_get_photo(contacts_record_h contact, ctsvc_list_s *image_list, char *prefix, char *val)
{
	int ret, type, fd;
	gsize size;
	guchar *buf;
	char *temp;
	char dest[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
	contacts_record_h image;
	struct timeval tv;

	temp = strchr(val, ':');
	RETVM_IF(NULL == temp, CONTACTS_ERROR_INVALID_PARAMETER, "val is invalid");

	*temp = '\0';

	type = __ctsvc_vcard_get_image_type(prefix);

	buf = g_base64_decode(temp+1, &size);
	if ((0 == size) || (NULL == buf)) {
		g_free(buf);
		return CONTACTS_ERROR_NONE;
	}

	gettimeofday(&tv, NULL);
	ret = snprintf(dest, sizeof(dest), "%s/vcard-image-%ld%ld.%s",
			CTSVC_VCARD_IMAGE_LOCATION, tv.tv_sec, tv.tv_usec, __ctsvc_get_img_suffix(type));

	fd = open(dest, O_WRONLY|O_CREAT|O_TRUNC, 0660);
	RETVM_IF(fd < 0, CONTACTS_ERROR_SYSTEM, "System : open Fail(%d)", errno);

	while (0 < size) {
		ret = write(fd, buf, size);
		if (ret <= 0) {
			if (EINTR == errno) {
				continue;
			} else {
				ERR("write() Fail(%d)", errno);
				close(fd);
				if (ENOSPC == errno)
					return CONTACTS_ERROR_FILE_NO_SPACE;   /* No space */
				else
					return CONTACTS_ERROR_SYSTEM;   /* IO error */
			}
		}
		size -= ret;
	}

	close(fd);
	g_free(buf);

	ret = contacts_record_create(_contacts_image._uri, &image);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);

	contacts_record_set_str(image, _contacts_image.path, dest);
	((ctsvc_image_s*)image)->is_vcard = true;

	contacts_list_add((contacts_list_h)image_list, image);

	/* _contacts_contact.image_thumbnail_path is a read-only property */
	((ctsvc_contact_s*)contact)->image_thumbnail_path = strdup(dest);

	return CONTACTS_ERROR_NONE;

}

static inline void __ctsvc_vcard_get_event_type(contacts_record_h event, char *val)
{
	int type = CONTACTS_EVENT_TYPE_OTHER;
	char *temp, *result, *last = NULL;
	char *lower, *lower_temp;

	temp = strtok_r(val, ";", &last);
	while (temp) {
		lower = strdup(temp);
		if (NULL == lower) {
			ERR("strdup() Fail");
			break;
		}
		lower_temp = lower;
		while (*lower_temp) {
			*lower_temp = tolower(*lower_temp);
			lower_temp++;
		}
		if (strstr(lower, "anniversary")) {
			type = CONTACTS_EVENT_TYPE_ANNIVERSARY;
		} else if ((result = strstr(lower, "x-"))) {
			type = CONTACTS_EVENT_TYPE_CUSTOM;
			contacts_record_set_str(event, _contacts_event.label, temp+(result-lower)+2);
		}

		free(lower);
		temp = strtok_r(NULL, ";", &last);
	}
	contacts_record_set_int(event, _contacts_event.type, type);
}


static inline int __ctsvc_vcard_get_event(ctsvc_list_s *event_list, int type, char *prefix, char *val)
{
	int ret;
	contacts_record_h event;
	char *dest, *src, *date;

	date = __ctsvc_get_content_value(val);
	if (NULL == date) {
		ERR("vcard");
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	dest = src = date;
	while (*src) {
		if ('0' <= *src && *src <= '9') {
			*dest = *src;
			dest++;
		}
		src++;
		if (8 <= dest - date)
			break;
	}
	*dest = '\0';
	if ('\0' == *date) {
		ERR("date(%d)", date);
		return CONTACTS_ERROR_INVALID_PARAMETER;
	}

	ret = contacts_record_create(_contacts_event._uri, &event);
	if (ret < CONTACTS_ERROR_NONE) {
		ERR("contacts_record_create() Fail(%d)", ret);
		return ret;
	}

	contacts_record_set_int(event, _contacts_event.date, atoi(date));

	if (CTSVC_VCARD_VALUE_BDAY == type)
		contacts_record_set_int(event, _contacts_event.type, CONTACTS_EVENT_TYPE_BIRTH);
	else if (CTSVC_VCARD_VALUE_X_ANNIVERSARY == type)
		contacts_record_set_int(event, _contacts_event.type, CONTACTS_EVENT_TYPE_ANNIVERSARY);
	else if (CTSVC_VCARD_VALUE_X_TIZEN_EVENT == type)
		__ctsvc_vcard_get_event_type(event, prefix);

	contacts_list_add((contacts_list_h)event_list, event);
	return CONTACTS_ERROR_NONE;
}


static inline void __ctsvc_vcard_get_company_type(contacts_record_h company, char *val)
{
	char *temp, *result, *last = NULL;
	char *lower, *lower_temp;
	int type = CONTACTS_COMPANY_TYPE_OTHER;

	temp = strtok_r(val, ";", &last);
	while (temp) {
		lower = strdup(temp);
		if (NULL == lower) {
			ERR("strdup() Fail");
			break;
		}
		lower_temp = lower;
		while (*lower_temp) {
			*lower_temp = tolower(*lower_temp);
			lower_temp++;
		}

		result = strstr(lower, "work");
		if (result)
			type = CONTACTS_COMPANY_TYPE_WORK;

		result = strstr(lower, "x-");
		if (result) {
			type = CONTACTS_COMPANY_TYPE_CUSTOM;
			contacts_record_set_str(company, _contacts_company.label, temp+(result-lower)+2);
		}

		free(lower);
		temp = strtok_r(NULL, ";", &last);
	}
	contacts_record_set_int(company, _contacts_company.type, type);
}

static contacts_record_h __ctsvc_vcard_get_company_empty_record(ctsvc_list_s *company_list, int property_id)
{
	contacts_record_h record_temp = NULL;
	contacts_record_h record = NULL;
	contacts_list_h list = (contacts_list_h)company_list;

	contacts_list_last(list);
	while (CONTACTS_ERROR_NONE == contacts_list_get_current_record_p(list, &record_temp)) {
		char *value = NULL;
		contacts_record_get_str_p(record_temp, property_id, &value);
		if (NULL == value) {
			record = record_temp;
			break;
		}
		contacts_list_prev(list);
	}

	return record;
}

static inline int __ctsvc_vcard_get_company_value(ctsvc_list_s *company_list, int property_id, char *val)
{
	char *value;
	contacts_record_h company;

	company = __ctsvc_vcard_get_company_empty_record(company_list, property_id);
	if (NULL == company) {
		int ret = contacts_record_create(_contacts_company._uri, &company);
		RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);
		contacts_list_add((contacts_list_h)company_list, company);
	}

	value = __ctsvc_get_content_value(val);
	RETV_IF(NULL == value, CONTACTS_ERROR_NO_DATA);

	contacts_record_set_str(company, property_id, __ctsvc_vcard_remove_escape_char(value));

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_vcard_get_company(ctsvc_list_s *company_list, char *prefix, char *val)
{
	char *start, *depart;
	const char separator = ';';
	contacts_record_h company;

	company = __ctsvc_vcard_get_company_empty_record(company_list, _contacts_company.name);
	if (NULL == company) {
		int ret = contacts_record_create(_contacts_company._uri, &company);
		RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);
		contacts_list_add((contacts_list_h)company_list, company);
	}

	start = __ctsvc_get_content_value(val);
	RETV_IF(NULL == start, CONTACTS_ERROR_NO_DATA);

	depart = __ctsvc_strtok(start, separator);
	contacts_record_set_str(company, _contacts_company.name, __ctsvc_vcard_remove_escape_char(start));

	if (depart) {
		__ctsvc_strtok(depart, separator);
		contacts_record_set_str(company, _contacts_company.department, __ctsvc_vcard_remove_escape_char(depart));
	}

	__ctsvc_vcard_get_company_type(company, prefix);

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_vcard_get_company_logo(ctsvc_list_s *company_list, char *prefix, char *val)
{
	int ret, type, fd;
	gsize size;
	guchar *buf;
	char dest[CTSVC_IMG_FULL_PATH_SIZE_MAX] = {0};
	char *temp;
	contacts_record_h company;
	struct timeval tv;

	company = __ctsvc_vcard_get_company_empty_record(company_list, _contacts_company.logo);
	if (NULL == company) {
		ret = contacts_record_create(_contacts_company._uri, &company);
		RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);
		contacts_list_add((contacts_list_h)company_list, company);
	}

	temp = strchr(val, ':');
	RETVM_IF(NULL == temp, CONTACTS_ERROR_INVALID_PARAMETER, "val is invalid");

	*temp = '\0';
	type = __ctsvc_vcard_get_image_type(prefix);

	buf = g_base64_decode(temp+1, &size);
	if ((0 == size) || (NULL == buf)) {
		g_free(buf);
		return CONTACTS_ERROR_NONE;
	}

	gettimeofday(&tv, NULL);
	ret = snprintf(dest, sizeof(dest), "%s/%d-%ld%ld-logo.%s", CTSVC_VCARD_IMAGE_LOCATION,
			getpid(), tv.tv_sec, tv.tv_usec, __ctsvc_get_img_suffix(type));

	fd = open(dest, O_WRONLY|O_CREAT|O_TRUNC, 0660);
	RETVM_IF(fd < 0, CONTACTS_ERROR_SYSTEM, "System : open Fail(%d)", errno);

	while (0 < size) {
		ret = write(fd, buf, size);
		if (ret <= 0) {
			if (EINTR == errno) {
				continue;
			} else {
				ERR("write() Fail(%d)", errno);
				close(fd);
				if (ENOSPC == errno)
					return CONTACTS_ERROR_FILE_NO_SPACE;   /* No space */
				else
					return CONTACTS_ERROR_SYSTEM;   /* IO error */
			}
		}
		size -= ret;
	}

	close(fd);
	g_free(buf);

	((ctsvc_company_s*)company)->is_vcard = true;
	contacts_record_set_str(company, _contacts_company.logo, dest);

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_vcard_get_note(ctsvc_list_s *note_list, char *val)
{
	int ret;
	char *temp;
	contacts_record_h note;

	ret = contacts_record_create(_contacts_note._uri, &note);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);
	contacts_list_add((contacts_list_h)note_list, note);

	temp = __ctsvc_get_content_value(val);
	RETV_IF(NULL == temp, CONTACTS_ERROR_NO_DATA);

	contacts_record_set_str(note, _contacts_note.note, __ctsvc_vcard_remove_escape_char(temp));

	return CONTACTS_ERROR_NONE;
}

static inline int __ctsvc_vcard_get_time(char *val)
{
	int i;
	char tmp[10] = {0};
	struct tm ts = {0};

	i = 0;
	while (*val && (*val < '0' || '9' < *val)) val++;
	while (*val) {
		tmp[i++] = *val;
		val++;
		if (4 <= i || *val < '0' || '9' < *val) break;
	}
	tmp[i] = 0;
	ts.tm_year = atoi(tmp)-1900;

	i = 0;
	while (*val && (*val < '0' || '9' < *val)) val++;
	while (*val) {
		tmp[i++] = *val;
		val++;
		if (2 <= i || *val < '0' || '9' < *val) break;
	}
	tmp[i] = 0;
	ts.tm_mon = atoi(tmp)-1;

	i = 0;
	while (*val && (*val < '0' || '9' < *val)) val++;
	while (*val) {
		tmp[i++] = *val;
		val++;
		if (2 <= i || *val < '0' || '9' < *val) break;
	}
	tmp[i] = 0;
	ts.tm_mday = atoi(tmp);

	i = 0;
	while (*val && (*val < '0' || '9' < *val)) val++;
	while (*val) {
		tmp[i++] = *val;
		val++;
		if (2 <= i || *val < '0' || '9' < *val) break;
	}
	tmp[i] = 0;
	ts.tm_hour = atoi(tmp);

	i = 0;
	while (*val && (*val < '0' || '9' < *val)) val++;
	while (*val) {
		tmp[i++] = *val;
		val++;
		if (2 <= i || *val < '0' || '9' < *val) break;
	}
	tmp[i] = 0;
	ts.tm_min = atoi(tmp);

	i = 0;
	while (*val && (*val < '0' || '9' < *val)) val++;
	while (*val) {
		tmp[i++] = *val;
		val++;
		if (2 <= i || *val < '0' || '9' < *val) break;
	}
	tmp[i] = 0;
	ts.tm_sec = atoi(tmp);

	return (int)mktime(&ts);
}

static inline void __ctsvc_vcard_get_url_type(contacts_record_h url, char *val)
{
	char *temp, *result, *last = NULL;
	char *lower, *lower_temp;
	int type = CONTACTS_URL_TYPE_OTHER;

	temp = strtok_r(val, ";", &last);
	while (temp) {
		lower = strdup(temp);
		if (NULL == lower) {
			ERR("strdup() Fail");
			break;
		}
		lower_temp = lower;
		while (*lower_temp) {
			*lower_temp = tolower(*lower_temp);
			lower_temp++;
		}
		result = strstr(lower, "home");
		if (result) type = CONTACTS_URL_TYPE_HOME;
		result = strstr(lower, "work");
		if (result) type = CONTACTS_URL_TYPE_WORK;
		result = strstr(lower, "x-");
		if (result) {
			type = CONTACTS_URL_TYPE_CUSTOM;
			contacts_record_set_str(url, _contacts_url.label, temp+(result-lower)+2);
		}

		free(lower);
		temp = strtok_r(NULL, ";", &last);
	}
	contacts_record_set_int(url, _contacts_url.type, type);
}

static inline int __ctsvc_vcard_get_url(ctsvc_list_s *url_list, char *prefix, char *val)
{
	int ret;
	contacts_record_h url;
	char *temp;

	temp = __ctsvc_get_content_value(val);
	RETVM_IF(NULL == temp, CONTACTS_ERROR_INVALID_PARAMETER, "vcard");

	ret = contacts_record_create(_contacts_url._uri, &url);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);

	contacts_record_set_str(url, _contacts_url.url, __ctsvc_vcard_remove_escape_char(temp));
	__ctsvc_vcard_get_url_type(url, prefix);
	contacts_list_add((contacts_list_h)url_list, url);

	return CONTACTS_ERROR_NONE;
}

static inline bool __ctsvc_vcard_get_number_type(contacts_record_h number, char *val)
{
	char *temp, *result, *last = NULL;
	char *lower, *lower_temp;
	int type = CONTACTS_NUMBER_TYPE_OTHER;
	bool pref = false;

	temp = strtok_r(val, ";", &last);
	while (temp) {
		lower = strdup(temp);
		if (NULL == lower) {
			ERR("strdup() Fail");
			break;
		}
		lower_temp = lower;
		while (*lower_temp) {
			*lower_temp = tolower(*lower_temp);
			lower_temp++;
		}
		result = strstr(lower, "home");
		if (result) type |= CONTACTS_NUMBER_TYPE_HOME;
		result = strstr(lower, "msg");
		if (result) type |= CONTACTS_NUMBER_TYPE_MSG;
		result = strstr(lower, "work");
		if (result) type |= CONTACTS_NUMBER_TYPE_WORK;
		result = strstr(lower, "pref");
		if (result) pref = true;
		result = strstr(lower, "voice");
		if (result) type |= CONTACTS_NUMBER_TYPE_VOICE;
		result = strstr(lower, "fax");
		if (result) type |= CONTACTS_NUMBER_TYPE_FAX;
		result = strstr(lower, "cell");
		if (result) type |= CONTACTS_NUMBER_TYPE_CELL;
		result = strstr(lower, "video");
		if (result) type |= CONTACTS_NUMBER_TYPE_VIDEO;
		result = strstr(lower, "pager");
		if (result) type |= CONTACTS_NUMBER_TYPE_PAGER;
		result = strstr(lower, "bbs");
		if (result) type |= CONTACTS_NUMBER_TYPE_BBS;
		result = strstr(lower, "modem");
		if (result) type |= CONTACTS_NUMBER_TYPE_MODEM;
		result = strstr(lower, "car");
		if (result) type |= CONTACTS_NUMBER_TYPE_CAR;
		result = strstr(lower, "isdn");
		if (result) type |= CONTACTS_NUMBER_TYPE_ISDN;
		result = strstr(lower, "pcs");
		if (result) type |= CONTACTS_NUMBER_TYPE_PCS;
		result = strstr(lower, "x-");
		if (result) {
			if (strstr(lower, "x-assistant")) {
				type |= CONTACTS_NUMBER_TYPE_ASSISTANT;
			} else if (strstr(lower, "x-radio")) {
				type |= CONTACTS_NUMBER_TYPE_RADIO;
			} else if (strstr(lower, "x-company-main")) {
				type |= CONTACTS_NUMBER_TYPE_COMPANY_MAIN;
			} else if (strstr(lower, "x-main")) {
				type |= CONTACTS_NUMBER_TYPE_MAIN;
			} else {
				type = CONTACTS_NUMBER_TYPE_CUSTOM;
				contacts_record_set_str(number, _contacts_number.label, temp+(result-lower)+2);
			}
		}

		free(lower);
		temp = strtok_r(NULL, ";", &last);
	}
	contacts_record_set_int(number, _contacts_number.type, type);

	return pref;
}

static char* __ctsvc_vcard_get_clean_number_for_import(char *str)
{
	int char_len = 0;
	char *s = SAFE_STR(str);
	char *r = s;
	while (*s) {
		char_len = __ctsvc_vcard_check_utf8(*s);
		if (3 == char_len) {
			if (*s == 0xef) {
				if (*(s+1) == 0xbc) {
					if (0x90 <= *(s+2) && *(s+2) <= 0x99) {   /* ef bc 90 : '0' ~ ef bc 99 : '9' */
						*r = '0' + (*(s+2) - 0x90);
						r++;
						s += 3;
					} else if (0x8b == *(s+2)) {   /* ef bc 8b : '+' */
						*r = '+';
						r++;
						s += 3;
					} else if (0x8a == *(s+2)) {   /* ef bc 8a : '*' */
						*r = '*';
						r++;
						s += 3;
					} else if (0x83 == *(s+2)) {   /* ef bc 83 : '#' */
						*r = '#';
						r++;
						s += 3;
					} else if (0xb0 == *(s+2) || 0x8c == *(s+2)) {   /* ef bc b0 : 'P', ef bc 8c : ',' */
						*r = ',';
						r++;
						s += 3;
					} else if (0xb7 == *(s+2) || 0x9b == *(s+2)) {   /* ef bc b7 : 'W', ef bc 9b : ';' */
						*r = ';';
						r++;
						s += 3;
					} else {
						s += char_len;
					}
				} else if (*(s+1) == 0xbd) {
					if (0x90 == *(s+2)) {
						*r = ',';
						r++;
						s += 3;
					} else if (0x97 == *(s+2)) {
						*r = ';';
						r++;
						s += 3;
					}
				} else {
					s += char_len;
				}
			} else {
				s += char_len;
			}
		} else if (1 == char_len) {
			switch (*s) {
			case '/':
			case '.':
			case '0' ... '9':
			case '#':
			case '*':
			case '(':
			case ')':
			case ',':
			case ';':
			case '+':
				*r = *s;
				r++;
				s++;
				break;
			case 'p':
			case 'P':
				*r = ',';
				r++;
				s++;
				break;
			case 'w':
			case 'W':
				*r = ';';
				r++;
				s++;
				break;
			case 'a' ... 'o':
			case 'q' ... 'v':
			case 'x' ... 'z':
				*r = *s - 0x20;
				r++;
				s++;
				break;
			case 'A' ... 'O':
			case 'Q' ... 'V':
			case 'X' ... 'Z':
				*r = *s;
				r++;
				s++;
				break;
			default:
				s++;
				break;
			}
		} else {
			s += char_len;
		}
	}
	*r = '\0';
	return str;
}

static inline int __ctsvc_vcard_get_number(ctsvc_list_s *numbers, char *prefix, char *val)
{
	bool is_default;
	int ret;
	char *temp;
	contacts_record_h number;

	temp = __ctsvc_get_content_value(val);
	RETVM_IF(NULL == temp, CONTACTS_ERROR_INVALID_PARAMETER, "vcard");

	ret = contacts_record_create(_contacts_number._uri, &number);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);

	temp = __ctsvc_vcard_remove_escape_char(temp);
	contacts_record_set_str(number, _contacts_number.number, __ctsvc_vcard_get_clean_number_for_import(temp));

	is_default = __ctsvc_vcard_get_number_type(number, prefix);
	contacts_record_set_bool(number, _contacts_number.is_default, is_default);
	contacts_list_add((contacts_list_h)numbers, number);

	return CONTACTS_ERROR_NONE;
}

static inline bool __ctsvc_vcard_get_email_type(contacts_record_h email, char *val)
{
	char *temp, *result, *last = NULL;
	char *lower, *lower_temp;
	int type = CONTACTS_EMAIL_TYPE_OTHER;
	bool pref = false;

	temp = strtok_r(val, ";", &last);
	while (temp) {
		lower = strdup(temp);
		if (NULL == lower) {
			ERR("strdup() Fail");
			break;
		}
		lower_temp = lower;
		while (*lower_temp) {
			*lower_temp = tolower(*lower_temp);
			lower_temp++;
		}
		if (strstr(lower, "pref"))
			pref = true;

		if (strstr(lower, "home")) {
			type = CONTACTS_EMAIL_TYPE_HOME;
		} else if (strstr(lower, "work")) {
			type = CONTACTS_EMAIL_TYPE_WORK;
		} else if (strstr(lower, "cell")) {
			type = CONTACTS_EMAIL_TYPE_MOBILE;
		} else if ((result = strstr(lower, "x-"))) {
			type = CONTACTS_EMAIL_TYPE_CUSTOM;
			contacts_record_set_str(email, _contacts_email.label, temp+(result-lower)+2);
		}

		free(lower);
		temp = strtok_r(NULL, ";", &last);
	}
	contacts_record_set_int(email, _contacts_email.type, type);

	return pref;
}

static inline int __ctsvc_vcard_get_email(ctsvc_list_s *emails, char *prefix, char *val)
{
	bool is_default;
	int ret;
	char *temp;
	contacts_record_h email;

	temp = __ctsvc_get_content_value(val);
	RETVM_IF(NULL == temp, CONTACTS_ERROR_INVALID_PARAMETER, "vcard");

	ret = contacts_record_create(_contacts_email._uri, &email);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);

	contacts_record_set_str(email, _contacts_email.email, __ctsvc_vcard_remove_escape_char(temp));
	is_default = __ctsvc_vcard_get_email_type(email, prefix);
	contacts_record_set_bool(email, _contacts_email.is_default, is_default);
	contacts_list_add((contacts_list_h)emails, email);

	return CONTACTS_ERROR_NONE;
}

static inline bool __ctsvc_vcard_get_postal_type(contacts_record_h address, char *val)
{
	char *temp, *result, *last = NULL;
	char *lower, *lower_temp;
	int type = CONTACTS_ADDRESS_TYPE_OTHER;
	bool pref = false;

	temp = strtok_r(val, ";", &last);
	while (temp) {
		lower = strdup(temp);
		if (NULL == lower) {
			ERR("strdup() Fail");
			break;
		}
		lower_temp = lower;
		while (*lower_temp) {
			*lower_temp = tolower(*lower_temp);
			lower_temp++;
		}
		result = strstr(lower, "dom");
		if (result) type |= CONTACTS_ADDRESS_TYPE_DOM;
		result = strstr(lower, "intl");
		if (result) type |= CONTACTS_ADDRESS_TYPE_INTL;
		result = strstr(lower, "address");
		if (result) type |= CONTACTS_ADDRESS_TYPE_POSTAL;
		result = strstr(lower, "parcel");
		if (result) type |= CONTACTS_ADDRESS_TYPE_PARCEL;
		result = strstr(lower, "home");
		if (result) type |= CONTACTS_ADDRESS_TYPE_HOME;
		result = strstr(lower, "work");
		if (result) type |= CONTACTS_ADDRESS_TYPE_WORK;
		result = strstr(lower, "x-");
		if (result) {
			type = CONTACTS_ADDRESS_TYPE_CUSTOM;
			contacts_record_set_str(address, _contacts_address.label, temp+(result-lower)+2);
		}
		result = strstr(val, "pref");
		if (result) pref = true;

		free(lower);
		temp = strtok_r(NULL, ";", &last);
	}

	contacts_record_set_int(address, _contacts_address.type, type);

	return pref;
}

static inline int __ctsvc_vcard_get_address(ctsvc_list_s *address_list, char *prefix, char *val)
{
	char *text;
	char *text_temp;
	contacts_record_h address;

	contacts_record_create(_contacts_address._uri, &address);
	if (address) {

		text = strchr(val, ':');
		if (text) {
			text++;
			*(text-1) = '\0';
		} else {
			text = val;
		}

		while (true) {
			bool separator = false;

			CTS_GET_MULTIPLE_COMPONENT(((ctsvc_address_s*)address)->pobox, text, text_temp, separator);
			CTS_GET_MULTIPLE_COMPONENT(((ctsvc_address_s*)address)->extended, text, text_temp, separator);
			CTS_GET_MULTIPLE_COMPONENT(((ctsvc_address_s*)address)->street, text, text_temp, separator);
			CTS_GET_MULTIPLE_COMPONENT(((ctsvc_address_s*)address)->locality, text, text_temp, separator);
			CTS_GET_MULTIPLE_COMPONENT(((ctsvc_address_s*)address)->region, text, text_temp, separator);
			CTS_GET_MULTIPLE_COMPONENT(((ctsvc_address_s*)address)->postalcode, text, text_temp, separator);
			CTS_GET_MULTIPLE_COMPONENT(((ctsvc_address_s*)address)->country, text, text_temp, separator);

			ERR("invalid ADR type");
			break;
		}

		if (((ctsvc_address_s*)address)->pobox || ((ctsvc_address_s*)address)->extended
				|| ((ctsvc_address_s*)address)->street || ((ctsvc_address_s*)address)->locality
				|| ((ctsvc_address_s*)address)->region || ((ctsvc_address_s*)address)->postalcode
				|| ((ctsvc_address_s*)address)->country) {
			contacts_record_set_bool(address, _contacts_address.is_default, __ctsvc_vcard_get_postal_type(address, prefix));
		} else {
			ERR("Invalid vcard");
			contacts_record_destroy(address, true);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		contacts_list_add((contacts_list_h)address_list, address);
	}

	return CONTACTS_ERROR_NONE;
}

static inline void __ctsvc_vcard_get_messenger_type(contacts_record_h messenger, char *val)
{
	char *temp, *result, *last = NULL;
	char *lower, *lower_temp;
	int type = CONTACTS_MESSENGER_TYPE_OTHER;

	temp = strtok_r(val, ";", &last);
	while (temp) {
		lower = strdup(temp);
		if (NULL == lower) {
			ERR("strdup() Fail");
			break;
		}
		lower_temp = lower;
		while (*lower_temp) {
			*lower_temp = tolower(*lower_temp);
			lower_temp++;
		}
		result = strstr(lower, "facebook");
		if (result) type = CONTACTS_MESSENGER_TYPE_FACEBOOK;
		result = strstr(lower, "irc");
		if (result) type = CONTACTS_MESSENGER_TYPE_IRC;
		result = strstr(lower, "x-");
		if (result) {
			type = CONTACTS_MESSENGER_TYPE_CUSTOM;
			contacts_record_set_str(messenger, _contacts_messenger.label, temp+(result-lower)+2);
		}
		free(lower);
		temp = strtok_r(NULL, ";", &last);
	}
	contacts_record_set_int(messenger, _contacts_messenger.type, type);
}

static inline int __ctsvc_vcard_get_messenger(ctsvc_list_s *messenger_list, int type, char *prefix, char *val)
{
	int ret;
	contacts_record_h messenger;
	char *temp;

	temp = __ctsvc_get_content_value(val);
	RETVM_IF(NULL == temp, CONTACTS_ERROR_INVALID_PARAMETER, "vcard");

	ret = contacts_record_create(_contacts_messenger._uri, &messenger);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);

	contacts_record_set_str(messenger, _contacts_messenger.im_id, __ctsvc_vcard_remove_escape_char(temp));

	switch (type) {
	case CTSVC_VCARD_VALUE_X_MSN:
		contacts_record_set_int(messenger, _contacts_messenger.type, CONTACTS_MESSENGER_TYPE_WLM);
		break;
	case CTSVC_VCARD_VALUE_X_YAHOO:
		contacts_record_set_int(messenger, _contacts_messenger.type, CONTACTS_MESSENGER_TYPE_YAHOO);
		break;
	case CTSVC_VCARD_VALUE_X_ICQ:
		contacts_record_set_int(messenger, _contacts_messenger.type, CONTACTS_MESSENGER_TYPE_ICQ);
		break;
	case CTSVC_VCARD_VALUE_X_AIM:
		contacts_record_set_int(messenger, _contacts_messenger.type, CONTACTS_MESSENGER_TYPE_AIM);
		break;
	case CTSVC_VCARD_VALUE_X_JABBER:
		contacts_record_set_int(messenger, _contacts_messenger.type, CONTACTS_MESSENGER_TYPE_JABBER);
		break;
	case CTSVC_VCARD_VALUE_X_SKYPE_USERNAME:
	case CTSVC_VCARD_VALUE_X_SKYPE:
		contacts_record_set_int(messenger, _contacts_messenger.type, CONTACTS_MESSENGER_TYPE_SKYPE);
		break;
	case CTSVC_VCARD_VALUE_X_QQ:
		contacts_record_set_int(messenger, _contacts_messenger.type, CONTACTS_MESSENGER_TYPE_QQ);
		break;
	case CTSVC_VCARD_VALUE_X_GOOGLE_TALK:
		contacts_record_set_int(messenger, _contacts_messenger.type, CONTACTS_MESSENGER_TYPE_GOOGLE);
		break;
	case CTSVC_VCARD_VALUE_X_TIZEN_MESSENGER:
		__ctsvc_vcard_get_messenger_type(messenger, prefix);
		break;
	}
	contacts_list_add((contacts_list_h)messenger_list, messenger);

	return CONTACTS_ERROR_NONE;
}

static inline void __ctsvc_vcard_get_relationship_type(contacts_record_h relationship, char *val)
{
	char *temp, *result, *last = NULL;
	char *lower, *lower_temp;
	int type = CONTACTS_RELATIONSHIP_TYPE_OTHER;

	temp = strtok_r(val, ";", &last);
	while (temp) {
		lower = strdup(temp);
		if (NULL == lower) {
			ERR("strdup() Fail");
			break;
		}
		lower_temp = lower;
		while (*lower_temp) {
			*lower_temp = tolower(*lower_temp);
			lower_temp++;
		}

		if (strstr(lower, "assistant")) {
			type = CONTACTS_RELATIONSHIP_TYPE_ASSISTANT;
		} else if (strstr(lower, "brother")) {
			type = CONTACTS_RELATIONSHIP_TYPE_BROTHER;
		} else if (strstr(lower, "child")) {
			type = CONTACTS_RELATIONSHIP_TYPE_CHILD;
		} else if (strstr(lower, "domestic_partner")) {
			type = CONTACTS_RELATIONSHIP_TYPE_DOMESTIC_PARTNER;
		} else if (strstr(lower, "father")) {
			type = CONTACTS_RELATIONSHIP_TYPE_FATHER;
		} else if (strstr(lower, "friend")) {
			type = CONTACTS_RELATIONSHIP_TYPE_FRIEND;
		} else if (strstr(lower, "manager")) {
			type = CONTACTS_RELATIONSHIP_TYPE_MANAGER;
		} else if (strstr(lower, "mother")) {
			type = CONTACTS_RELATIONSHIP_TYPE_MOTHER;
		} else if (strstr(lower, "parent")) {
			type = CONTACTS_RELATIONSHIP_TYPE_PARENT;
		} else if (strstr(lower, "partner")) {
			type = CONTACTS_RELATIONSHIP_TYPE_PARTNER;
		} else if (strstr(lower, "referred_by")) {
			type = CONTACTS_RELATIONSHIP_TYPE_REFERRED_BY;
		} else if (strstr(lower, "relative")) {
			type = CONTACTS_RELATIONSHIP_TYPE_RELATIVE;
		} else if (strstr(lower, "sister")) {
			type = CONTACTS_RELATIONSHIP_TYPE_SISTER;
		} else if (strstr(lower, "spouse")) {
			type = CONTACTS_RELATIONSHIP_TYPE_SPOUSE;
		} else if ((result = strstr(lower, "x-"))) {
			type = CONTACTS_RELATIONSHIP_TYPE_CUSTOM;
			contacts_record_set_str(relationship, _contacts_relationship.label, temp+(result-lower)+2);
		}
		free(lower);
		temp = strtok_r(NULL, ";", &last);
	}
	contacts_record_set_int(relationship, _contacts_relationship.type, type);
}


static inline int __ctsvc_vcard_get_relationship(ctsvc_list_s *relationship_list, int type, char *prefix, char *val)
{
	int ret;
	char *temp;
	contacts_record_h relationship;

	temp = __ctsvc_get_content_value(val);
	RETVM_IF(NULL == temp, CONTACTS_ERROR_INVALID_PARAMETER, "vcard");

	ret = contacts_record_create(_contacts_relationship._uri, &relationship);
	RETVM_IF(ret < CONTACTS_ERROR_NONE, ret, "contacts_record_create() Fail(%d)", ret);

	contacts_record_set_str(relationship, _contacts_relationship.name, __ctsvc_vcard_remove_escape_char(temp));
	__ctsvc_vcard_get_relationship_type(relationship, prefix);
	contacts_list_add((contacts_list_h)relationship_list, relationship);

	return CONTACTS_ERROR_NONE;

}

static char* __ctsvc_vcard_decode_base64_val(char *val)
{
	gsize size = 0;
	guchar *decoded_str;
	char *src;
	char *dest = NULL;

	src = strchr(val, ':');
	if (NULL == src)
		src = val;
	else
		src++;

	decoded_str = g_base64_decode(src, &size);

	dest = calloc((src-val)+size+1, sizeof(char));
	if (NULL == dest) {
		ERR("calloc() Fail");
		return NULL;
	}

	snprintf(dest, (src-val)+1, "%s", val);
	snprintf(dest+(src-val), size+1, "%s", decoded_str);
	g_free(decoded_str);

	return dest;
}

static inline int __ctsvc_vcard_get_contact(int ver, char *vcard, contacts_record_h *record)
{
	int type;
	char *cursor, *new_start, *val, *prefix;
	ctsvc_contact_s *contact = (ctsvc_contact_s*)*record;

	cursor = vcard;
	while (cursor) {
		val = NULL;
		prefix = NULL;

		bool base64_encoded = false;
		type = __ctsvc_vcard_check_content_type(&cursor);
		if (CTSVC_VCARD_VALUE_NONE == type) {
			new_start = __ctsvc_vcard_pass_unsupported(cursor);
			if (new_start) {
				cursor = new_start;
				continue;
			} else {
				break;
			}
		}

		if (CTSVC_VCARD_VALUE_PHOTO != type && CTSVC_VCARD_VALUE_LOGO != type)
			base64_encoded = __ctsvc_vcard_check_base64_encoded(cursor);

		new_start = __ctsvc_vcard_get_val(ver, cursor, &prefix, &val);
		if (NULL == new_start) {
			free(prefix);
			free(val);
			continue;
		}

		if (NULL == val) {
			cursor = new_start;
			free(prefix);
			free(val);
			continue;
		}

		if (base64_encoded) {
			char *temp = __ctsvc_vcard_decode_base64_val(val);
			if (NULL == temp) {
				ERR("__ctsvc_vcard_decode_base64_val() Fail");
				free(prefix);
				free(val);
				return CONTACTS_ERROR_OUT_OF_MEMORY;
			}
			free(val);
			val = temp;
		}

		switch (type) {
		case CTSVC_VCARD_VALUE_FN:
			__ctsvc_vcard_get_display_name(contact->name, val);
			break;
		case CTSVC_VCARD_VALUE_N:
			__ctsvc_vcard_get_name(contact->name, val);
			break;
		case CTSVC_VCARD_VALUE_PHONETIC_FIRST_NAME:
		case CTSVC_VCARD_VALUE_PHONETIC_MIDDLE_NAME:
		case CTSVC_VCARD_VALUE_PHONETIC_LAST_NAME:
			__ctsvc_vcard_get_phonetic_name(contact->name, type, val);
			break;
		case CTSVC_VCARD_VALUE_NICKNAME:
			__ctsvc_vcard_get_nickname(contact->nicknames, val);
			break;
		case CTSVC_VCARD_VALUE_PHOTO:
			__ctsvc_vcard_get_photo(*record, contact->images, prefix, val);
			break;
		case CTSVC_VCARD_VALUE_BDAY:
		case CTSVC_VCARD_VALUE_X_ANNIVERSARY:
		case CTSVC_VCARD_VALUE_X_TIZEN_EVENT:
			__ctsvc_vcard_get_event(contact->events, type, prefix, val);
			break;
		case CTSVC_VCARD_VALUE_ADR:
			__ctsvc_vcard_get_address(contact->postal_addrs, prefix, val);
			break;
		case CTSVC_VCARD_VALUE_TEL:
			__ctsvc_vcard_get_number(contact->numbers, prefix, val);
			break;
		case CTSVC_VCARD_VALUE_EMAIL:
			__ctsvc_vcard_get_email(contact->emails, prefix, val);
			break;
		case CTSVC_VCARD_VALUE_TITLE:
			__ctsvc_vcard_get_company_value(contact->company, _contacts_company.job_title, val);
			break;
		case CTSVC_VCARD_VALUE_ROLE:
			__ctsvc_vcard_get_company_value(contact->company, _contacts_company.role, val);
			break;
		case CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_LOCATION:
			__ctsvc_vcard_get_company_value(contact->company, _contacts_company.location, val);
			break;
		case CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_DESCRIPTION:
			__ctsvc_vcard_get_company_value(contact->company, _contacts_company.description, val);
			break;
		case CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_PHONETIC_NAME:
			__ctsvc_vcard_get_company_value(contact->company, _contacts_company.phonetic_name, val);
			break;
		case CTSVC_VCARD_VALUE_X_TIZEN_COMPANY_ASSISTANT_NAME:
			__ctsvc_vcard_get_company_value(contact->company, _contacts_company.assistant_name, val);
			break;
		case CTSVC_VCARD_VALUE_LOGO:
			__ctsvc_vcard_get_company_logo(contact->company, prefix, val);
			break;
		case CTSVC_VCARD_VALUE_ORG:
			__ctsvc_vcard_get_company(contact->company, prefix, val);
			break;
		case CTSVC_VCARD_VALUE_NOTE:
			__ctsvc_vcard_get_note(contact->note, val);
			break;
		case CTSVC_VCARD_VALUE_REV:
			if (*val)
				contact->changed_time = __ctsvc_vcard_get_time(val);
			break;
		case CTSVC_VCARD_VALUE_UID:
			contacts_record_set_str((contacts_record_h)contact, _contacts_contact.uid, __ctsvc_vcard_remove_escape_char(val));
			break;
		case CTSVC_VCARD_VALUE_URL:
			__ctsvc_vcard_get_url(contact->urls, prefix, val);
			break;
		case CTSVC_VCARD_VALUE_X_MSN:
		case CTSVC_VCARD_VALUE_X_YAHOO:
		case CTSVC_VCARD_VALUE_X_ICQ:
		case CTSVC_VCARD_VALUE_X_AIM:
		case CTSVC_VCARD_VALUE_X_JABBER:
		case CTSVC_VCARD_VALUE_X_SKYPE_USERNAME:
		case CTSVC_VCARD_VALUE_X_SKYPE:
		case CTSVC_VCARD_VALUE_X_QQ:
		case CTSVC_VCARD_VALUE_X_GOOGLE_TALK:
		case CTSVC_VCARD_VALUE_X_TIZEN_MESSENGER:
			__ctsvc_vcard_get_messenger(contact->messengers, type, prefix, val);
			break;

		case CTSVC_VCARD_VALUE_X_TIZEN_RELATIONSHIP:
			__ctsvc_vcard_get_relationship(contact->relationships, type, prefix, val);
			break;
		case CTSVC_VCARD_VALUE_END:
			free(val);
			free(prefix);
			return CONTACTS_ERROR_NONE;
		default:
			ERR("__ctsvc_vcard_check_content_type() Fail(%d)", type);
			free(val);
			free(prefix);
			return CONTACTS_ERROR_INVALID_PARAMETER;
		}
		free(val);
		free(prefix);
		cursor = new_start;
	}

	ERR("Invalid vcard");
	return CONTACTS_ERROR_INVALID_PARAMETER;
}

static inline int __ctsvc_vcard_check_version(const char *src)
{
	bool start = false;
	const char *ver3 = "3.0";

	while (*src) {
		switch (*src) {
		case '\n':
		case '\r':
			return CTSVC_VCARD_VER_2_1;
		case ' ':
			src++;
			break;
		default:
			start = true;
			break;
		}
		if (start) break;
	}

	if (STRING_EQUAL == strcmp(src, ver3))
		return CTSVC_VCARD_VER_3_0;
	else
		return CTSVC_VCARD_VER_2_1;
}

static inline void __ctsvc_vcard_make_contact_display_name(ctsvc_contact_s *contact)
{
	ctsvc_name_s *name = NULL;

	free(contact->display_name);
	contact->display_name = NULL;

	free(contact->reverse_display_name);
	contact->reverse_display_name = NULL;

	if (0 < contact->name->count && contact->name->records
			&& contact->name->records->data) {
		name = (ctsvc_name_s*)contact->name->records->data;
	}

	if (name && (name->first || name->last || name->prefix || name->addition
				|| name->suffix)) {
		int reverse_lang_type = -1;
		char *display = NULL;
		char *reverse_display = NULL;
		int len, display_len;
		int temp_display_len;
		char *temp_display = NULL;
		contacts_name_display_order_e name_display_order = CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST;

		/*
		 * Make reverse display name (Last name first)
		 * Default         : Prefix Last, First Middle(addition), Suffix
		 * Korean, Chinese : Prefix LastFirstMiddleSuffix
		 * Japanese        : Prefix Last Middle First Suffix
		 * reverse sort name does not include prefix
		 *    But, if there is only prefix, reverse sort_name is prefix
		 */
		temp_display_len = SAFE_STRLEN(name->first)
			+ SAFE_STRLEN(name->addition)
			+ SAFE_STRLEN(name->last)
			+ SAFE_STRLEN(name->suffix);
		if (0 < temp_display_len) {
			temp_display_len += 7;
			temp_display = calloc(1, temp_display_len);
			if (NULL == temp_display) {
				ERR("calloc() Fail");
				return;
			}

			len = 0;

			if (name->last) {
				len += snprintf(temp_display + len, temp_display_len - len, "%s", name->last);

				if (reverse_lang_type < 0)
					reverse_lang_type = ctsvc_check_language_type(temp_display);

				if (reverse_lang_type != CTSVC_LANG_KOREAN &&
						reverse_lang_type != CTSVC_LANG_CHINESE &&
						reverse_lang_type != CTSVC_LANG_JAPANESE) {
					if (name->first || name->addition)
						len += snprintf(temp_display + len, temp_display_len - len, ",");
				}
			}

			if (reverse_lang_type < 0) {
				if (*temp_display)
					reverse_lang_type = ctsvc_check_language_type(temp_display);
				else if (name->first)
					reverse_lang_type = ctsvc_check_language_type(name->first);
				else if (name->addition)
					reverse_lang_type = ctsvc_check_language_type(name->addition);
			}

			if (reverse_lang_type == CTSVC_LANG_JAPANESE) {
				/* make temp_display name Prefix - Last - Middle - First - Suffix */
				if (name->addition) {
					if (*temp_display)
						len += snprintf(temp_display + len, temp_display_len - len, " ");
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->addition);
				}

				if (name->first) {
					if (*temp_display)
						len += snprintf(temp_display + len, temp_display_len - len, " ");
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->first);
				}
			} else {
				/* make temp_display name Prefix - Last - First -Middle - Suffix */
				if (name->first) {
					if (*temp_display) {
						if (reverse_lang_type < 0)
							reverse_lang_type = ctsvc_check_language_type(temp_display);

						if (reverse_lang_type != CTSVC_LANG_KOREAN &&
								reverse_lang_type != CTSVC_LANG_CHINESE)
							len += snprintf(temp_display + len, temp_display_len - len, " ");
					}
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->first);
				}

				if (name->addition) {
					if (*temp_display) {
						if (reverse_lang_type < 0)
							reverse_lang_type = ctsvc_check_language_type(temp_display);

						if (reverse_lang_type != CTSVC_LANG_KOREAN &&
								reverse_lang_type != CTSVC_LANG_CHINESE)
							len += snprintf(temp_display + len, temp_display_len - len, " ");
					}
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->addition);
				}
			}

			if (name->suffix) {
				if (*temp_display) {
					if (reverse_lang_type < 0)
						reverse_lang_type = ctsvc_check_language_type(temp_display);

					if (reverse_lang_type == CTSVC_LANG_JAPANESE) {
						len += snprintf(temp_display + len, temp_display_len - len, " ");
					} else if (reverse_lang_type != CTSVC_LANG_KOREAN &&
							reverse_lang_type != CTSVC_LANG_CHINESE)  {
						len += snprintf(temp_display + len, temp_display_len - len, ", ");
					}
				}
				len += snprintf(temp_display + len, temp_display_len - len, "%s", name->suffix);
			}
		}

		if (name->prefix && temp_display) {
			display_len = SAFE_STRLEN(name->prefix) + temp_display_len + 2;
			reverse_display = calloc(1, display_len);
			if (NULL == reverse_display) {
				ERR("calloc() Fail");
				free(temp_display);
				return;
			}
			snprintf(reverse_display, display_len, "%s %s", name->prefix, temp_display);
			free(temp_display);
		} else if (temp_display) {
			reverse_display = temp_display;
		} else if (name->prefix) {
			reverse_display = strdup(name->prefix);
		}

		/*
		 * Make display name (First name first)
		 * Default         : Prefix First Middle Last, Suffix
		 * Korean, Chinese : Prefix LastFirstMiddleSuffix (Same as reverse display name)
		 * Japanese        : Prefix First Middle Last Suffix
		 * sort name does not include prefix
		 *    But, if there is only prefix, sort_name is prefix
		 */

		if (reverse_lang_type == CTSVC_LANG_KOREAN ||
				reverse_lang_type == CTSVC_LANG_CHINESE) {
			display = strdup(reverse_display);
		} else {
			int lang_type = -1;
			temp_display = NULL;
			temp_display_len = SAFE_STRLEN(name->first)
				+ SAFE_STRLEN(name->addition)
				+ SAFE_STRLEN(name->last)
				+ SAFE_STRLEN(name->suffix);
			if (0 < temp_display_len) {
				temp_display_len += 6;
				/* make reverse_temp_display_name */
				temp_display = calloc(1, temp_display_len);
				if (NULL == temp_display) {
					ERR("calloc() Fail");
					free(reverse_display);
					return;
				}

				len = 0;

				if (name->first) {
					if (*temp_display)
						len += snprintf(temp_display + len, temp_display_len - len, " ");
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->first);
				}

				if (name->addition) {
					if (*temp_display)
						len += snprintf(temp_display + len, temp_display_len - len, " ");
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->addition);
				}

				if (name->last) {
					if (*temp_display)
						len += snprintf(temp_display + len, temp_display_len - len, " ");
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->last);
				}

				if (name->suffix) {
					if (*temp_display) {
						lang_type = ctsvc_check_language_type(temp_display);
						if (lang_type == CTSVC_LANG_JAPANESE)
							len += snprintf(temp_display + len, temp_display_len - len, " ");
						else
							len += snprintf(temp_display + len, temp_display_len - len, ", ");
					}
					len += snprintf(temp_display + len, temp_display_len - len, "%s", name->suffix);
				}
			}

			if (name->prefix && temp_display) {
				display_len = SAFE_STRLEN(name->prefix) + temp_display_len + 2;
				display = calloc(1, display_len);
				if (NULL == display) {
					ERR("calloc() Fail");
					free(temp_display);
					free(reverse_display);
					return;
				}
				snprintf(display, display_len, "%s %s", name->prefix, temp_display);
				free(temp_display);
			} else if (temp_display) {
				display = temp_display;
			} else if (name->prefix) {
				display = strdup(name->prefix);
			}
		}

#ifdef _CONTACTS_IPC_CLIENT
		contacts_setting_get_name_display_order(&name_display_order);
#endif
		if (CONTACTS_NAME_DISPLAY_ORDER_FIRSTLAST == name_display_order) {
			contact->display_name = display;
			free(reverse_display);
#ifdef _CONTACTS_IPC_CLIENT
		} else {
			contact->display_name = reverse_display;
			free(display);
#endif
		}
		contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NAME;
	} else {
		GList *cur;
		bool set_display_name = false;
		if (contact->company && contact->company->records) {
			for (cur = contact->company->records; cur; cur = cur->next) {
				ctsvc_company_s *company = cur->data;
				if (company && company->name) {
					set_display_name = true;
					contact->display_name = SAFE_STRDUP(company->name);
					contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_COMPANY;
					break;
				}
			}
		}

		if (false == set_display_name &&
				contact->nicknames && contact->nicknames->records) {
			for (cur = contact->nicknames->records; cur; cur = cur->next) {
				ctsvc_nickname_s *nickname = cur->data;
				if (nickname && nickname->nickname) {
					set_display_name = true;
					contact->display_name = SAFE_STRDUP(nickname->nickname);
					contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NICKNAME;
					break;
				}
			}
		}

		if (false == set_display_name &&
				contact->numbers && contact->numbers->records) {
			for (cur = contact->numbers->records; cur; cur = cur->next) {
				ctsvc_number_s *number = cur->data;
				if (number && number->number) {
					set_display_name = true;
					contact->display_name = SAFE_STRDUP(number->number);
					contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_NUMBER;
					break;
				}
			}
		}

		if (false == set_display_name &&
				contact->emails && contact->emails->records) {
			for (cur = contact->emails->records; cur; cur = cur->next) {
				ctsvc_email_s *email = cur->data;
				if (email && email->email_addr) {
					set_display_name = true;
					contact->display_name = SAFE_STRDUP(email->email_addr);
					contact->display_source_type = CONTACTS_DISPLAY_NAME_SOURCE_TYPE_EMAIL;
					break;
				}
			}
		}
	}
	return;
}

static void __ctsvc_vcard_update_contact_has_properties(ctsvc_contact_s *contact)
{
	if (contact->numbers && 0 < contact->numbers->count)
		contact->has_phonenumber = true;

	if (contact->emails && 0 < contact->emails->count)
		contact->has_email = true;
}

static int __ctsvc_vcard_parse(const void *vcard_stream, contacts_record_h *record)
{
	int ret, ver;
	ctsvc_contact_s *contact;
	char *val_begin, *new_start, *val;
	char *vcard = (char *)vcard_stream;

	RETV_IF(NULL == vcard_stream, CONTACTS_ERROR_INVALID_PARAMETER);

	__ctsvc_vcard_initial();

	vcard = __ctsvc_vcard_check_word(vcard, "BEGIN:VCARD");
	RETVM_IF(NULL == vcard, CONTACTS_ERROR_INVALID_PARAMETER, "The vcard is invalid.");

	val_begin = __ctsvc_vcard_check_word(vcard, "VERSION:");
	new_start = __ctsvc_vcard_get_val(CTSVC_VCARD_VER_NONE, val_begin, NULL, &val);
	if (NULL == new_start || NULL == val) {
		ver = CTSVC_VCARD_VER_2_1;
	} else {
		ver = __ctsvc_vcard_check_version(val);
		free(val);
		vcard = new_start;
	}

	contacts_record_create(_contacts_contact._uri, (contacts_record_h*)&contact);
	RETVM_IF(NULL == contact, CONTACTS_ERROR_OUT_OF_MEMORY, "Out of memory : contacts_record_create() Fail");

	ret = __ctsvc_vcard_get_contact(ver, vcard, (contacts_record_h*)&contact);
	if (CONTACTS_ERROR_NONE != ret) {
		ERR("cts_vcard_get_contact() Fail(%d)", ret);
		contacts_record_destroy((contacts_record_h)contact, true);
		return ret;
	}
	__ctsvc_vcard_make_contact_display_name(contact);
	__ctsvc_vcard_update_contact_has_properties(contact);
	*record = (contacts_record_h)contact;
	return CONTACTS_ERROR_NONE;
}

#define CTSVC_VCARD_MAX_SIZE 1024*1024

static const char* __contacts_vcard_remove_line_break(const char *c)
{
	while (c) {
		if ('\r' == *c && '\n' == *(c+1))
			c += 2;
		else if ('\n' == *c)
			c++;
		else
			break;
	}
	return c;
}

typedef struct {
	const char *pos_start;
	const char *pos_end;
} sub_vcard_info_s;

static void __contacts_vcard_free_sub_vcard_info_list(GList *list)
{
	if (NULL == list)
		return;

	GList *cursor;
	for (cursor = list; cursor; cursor = cursor->next) {
		sub_vcard_info_s *vcard_info = cursor->data;
		free(vcard_info);
	}
	g_list_free(list);
}

static void __contacts_vcard_free_vcard_object_list(GList *list)
{
	if (NULL == list)
		return;

	GList *cursor;
	for (cursor = list; cursor; cursor = cursor->next) {
		char *vcard_object = cursor->data;
		free(vcard_object);
	}
	g_list_free(list);

}

static const char* __contacts_vcard_parse_get_vcard_object(const char *cursor, GList **plist_vcard_object)
{
	char *vcard_object = NULL;
	bool new_line = false;
	const char *begin = "BEGIN:VCARD";
	const char *end = "END:VCARD";
	const char *vcard_start_cursor = cursor;
	const char *vcard_cursor = NULL;
	GList *sub_vcard_list = NULL;

	RETV_IF(NULL == plist_vcard_object, cursor);

	*plist_vcard_object = NULL;

	vcard_start_cursor = __contacts_vcard_remove_line_break(vcard_start_cursor);

	if (STRING_EQUAL != strncmp(vcard_start_cursor, begin, strlen(begin)))
		return vcard_start_cursor;

	vcard_cursor = vcard_start_cursor;

	vcard_cursor += strlen(begin);
	vcard_cursor = __contacts_vcard_remove_line_break(vcard_cursor);

	while (*vcard_cursor) {
		if (new_line) {
			if (STRING_EQUAL == strncmp(vcard_cursor, end, strlen(end))) {
				GList *sub_vcard_cursor = NULL;
				int vcard_len = 0;
				const char *pos_start = NULL;

				vcard_cursor += strlen(end);
				vcard_cursor = __contacts_vcard_remove_line_break(vcard_cursor);

				pos_start = vcard_start_cursor;
				for (sub_vcard_cursor = sub_vcard_list; sub_vcard_cursor; sub_vcard_cursor = sub_vcard_cursor->next) {
					sub_vcard_info_s *sub_vcard_info = sub_vcard_cursor->data;
					const char *pos_end = sub_vcard_info->pos_start;
					vcard_len += (pos_end - pos_start);
					pos_start = sub_vcard_info->pos_end;
				}
				vcard_len += (vcard_cursor - pos_start);
				vcard_object = calloc(vcard_len + 1, sizeof(char));
				if (NULL == vcard_object) {
					ERR("calloc() Fail");
					__contacts_vcard_free_sub_vcard_info_list(sub_vcard_list);
					return NULL;
				}

				vcard_len = 0;
				pos_start = vcard_start_cursor;
				for (sub_vcard_cursor = sub_vcard_list; sub_vcard_cursor; sub_vcard_cursor = sub_vcard_cursor->next) {
					sub_vcard_info_s *sub_vcard_info = sub_vcard_cursor->data;
					const char *pos_end = sub_vcard_info->pos_start;
					memcpy(vcard_object+vcard_len, pos_start, pos_end - pos_start);
					vcard_len += (pos_end - pos_start);
					pos_start = sub_vcard_info->pos_end;
				}
				__contacts_vcard_free_sub_vcard_info_list(sub_vcard_list);
				memcpy(vcard_object+vcard_len, pos_start, vcard_cursor - pos_start);
				*plist_vcard_object = g_list_append(*plist_vcard_object, vcard_object);

				return vcard_cursor;
			} else if (STRING_EQUAL == strncmp(vcard_cursor, begin, strlen(begin))) {
				/* sub vcard */
				sub_vcard_info_s *sub_vcard_info = calloc(1, sizeof(sub_vcard_info_s));
				if (NULL == sub_vcard_info) {
					ERR("calloc() Fail");
					__contacts_vcard_free_sub_vcard_info_list(sub_vcard_list);
					return NULL;
				}
				sub_vcard_info->pos_start = vcard_cursor;

				vcard_cursor = __contacts_vcard_parse_get_vcard_object(vcard_cursor, plist_vcard_object);
				sub_vcard_info->pos_end = vcard_cursor;

				sub_vcard_list = g_list_append(sub_vcard_list, sub_vcard_info);
				continue;
			}
			new_line = false;
		}
		vcard_cursor++;
		if (('\n' == *vcard_cursor) || ('\r' == *vcard_cursor && '\n' == *(vcard_cursor+1))) {
			new_line = true;
			vcard_cursor = __contacts_vcard_remove_line_break(vcard_cursor);
		}
	}

	__contacts_vcard_free_sub_vcard_info_list(sub_vcard_list);

	return vcard_cursor;
}

API int contacts_vcard_parse_to_contacts(const char *vcard_stream, contacts_list_h *out_contacts)
{
	int ret;
	contacts_record_h record;
	contacts_list_h list = NULL;
	const char *cursor = NULL;
	char *vcard_object = NULL;
	GList *list_vcard_object = NULL;

	RETV_IF(NULL == out_contacts, CONTACTS_ERROR_INVALID_PARAMETER);
	*out_contacts = NULL;

	RETV_IF(NULL == vcard_stream, CONTACTS_ERROR_INVALID_PARAMETER);

	cursor = vcard_stream;
	while ((cursor = __contacts_vcard_parse_get_vcard_object(cursor, &list_vcard_object))) {
		GList *vcard_cursor = NULL;
		if (NULL == list_vcard_object)
			break;

		for (vcard_cursor = list_vcard_object; vcard_cursor; vcard_cursor = vcard_cursor->next) {
			vcard_object = vcard_cursor->data;
			if (NULL == vcard_object)
				continue;

			ret = __ctsvc_vcard_parse(vcard_object, &record);
			if (ret < CONTACTS_ERROR_NONE) {
				ERR("__ctsvc_vcard_parse() Fail(%d)", ret);
				__contacts_vcard_free_vcard_object_list(list_vcard_object);
				contacts_list_destroy(list, true);
				return ret;
			}

			if (NULL == list)
				contacts_list_create(&list);
			contacts_list_add(list, record);
			vcard_object = NULL;
		}
		__contacts_vcard_free_vcard_object_list(list_vcard_object);
	}
	*out_contacts = list;
	return CONTACTS_ERROR_NONE;
}

API int contacts_vcard_parse_to_contact_foreach(const char *vcard_file_name,
		contacts_vcard_parse_cb cb, void *data)
{
	contacts_record_h record;
	FILE *file;
	int buf_size, len;
	int ret;
	int vcard_depth = 0;
	char *stream;
	char line[1024] = {0};

	RETV_IF(NULL == vcard_file_name, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, CONTACTS_ERROR_INVALID_PARAMETER);

	file = fopen(vcard_file_name, "r");
	RETVM_IF(NULL == file, CONTACTS_ERROR_SYSTEM, "fopen() Fail(%d)", errno);

	len = 0;
	buf_size = CTSVC_VCARD_MAX_SIZE;
	stream = malloc(CTSVC_VCARD_MAX_SIZE);
	if (NULL == stream) {
		ERR("Out of memory : malloc() Fail");
		fclose(file);
		return CONTACTS_ERROR_OUT_OF_MEMORY;
	}

	while (fgets(line, sizeof(line), file)) {
		if (0 == len)
			if (STRING_EQUAL != strncmp(line, "BEGIN:VCARD", strlen("BEGIN:VCARD")))
				continue;

		if (len + sizeof(line) < buf_size) {
			len += snprintf(stream + len, buf_size - len, "%s", line);
		} else {
			char *new_stream;
			buf_size += sizeof(line) * 2;
			new_stream = realloc(stream, buf_size);
			if (new_stream) {
				stream = new_stream;
			} else {
				free(stream);
				fclose(file);
				return CONTACTS_ERROR_OUT_OF_MEMORY;
			}

			len += snprintf(stream + len, buf_size - len, "%s", line);
		}

		if (STRING_EQUAL == strncmp(line, "END:VCARD", 9)) {
			vcard_depth--;

			if (0 == vcard_depth) {
				const char *cursor = stream;
				GList *list_vcard_object = NULL;

				len = 0;

				while ((cursor = __contacts_vcard_parse_get_vcard_object(cursor, &list_vcard_object))) {
					GList *vcard_cursor = NULL;
					if (NULL == list_vcard_object)
						break;

					for (vcard_cursor = list_vcard_object; vcard_cursor; vcard_cursor = vcard_cursor->next) {
						char *vcard_object = vcard_cursor->data;

						if (NULL == vcard_object)
							continue;

						ret = __ctsvc_vcard_parse(vcard_object, &record);
						if (ret < CONTACTS_ERROR_NONE) {
							ERR("vcard stream parsing error");
							free(stream);
							fclose(file);
							__contacts_vcard_free_vcard_object_list(list_vcard_object);
							return ret;
						}

						if (false == cb(record, data)) {
							free(stream);
							fclose(file);
							__contacts_vcard_free_vcard_object_list(list_vcard_object);
							contacts_record_destroy(record, true);
							return CONTACTS_ERROR_NONE;
						}
						contacts_record_destroy(record, true);
					}
					__contacts_vcard_free_vcard_object_list(list_vcard_object);
					list_vcard_object = NULL;
				}
			}
		} else if (STRING_EQUAL == strncmp(line, "BEGIN:VCARD", 11)) { /* sub vcard object */
			vcard_depth++;
		}
	}

	free(stream);
	fclose(file);
	return CONTACTS_ERROR_NONE;
}

API int contacts_vcard_get_entity_count(const char *vcard_file_name, int *count)
{
	FILE *file;
	int cnt;
	char line[1024] = {0};
	RETV_IF(NULL == count, CONTACTS_ERROR_INVALID_PARAMETER);
	*count = 0;

	RETV_IF(NULL == vcard_file_name, CONTACTS_ERROR_INVALID_PARAMETER);

	file = fopen(vcard_file_name, "r");
	RETVM_IF(NULL == file, CONTACTS_ERROR_SYSTEM, "System : fopen() Fail(%d)", errno);

	cnt = 0;
	while (fgets(line, sizeof(line), file)) {
		if (STRING_EQUAL == strncmp(line, "END:VCARD", 9))
			cnt++;
	}
	fclose(file);

	*count = cnt;

	return CONTACTS_ERROR_NONE;
}

API int contacts_vcard_get_limit_size_of_photo(unsigned int *limit_size)
{
#ifdef _CONTACTS_IPC_CLIENT
	int ret;
	bool result = false;
#endif

	RETV_IF(NULL == limit_size, CONTACTS_ERROR_INVALID_PARAMETER);

#ifdef _CONTACTS_IPC_CLIENT
	ret = ctsvc_ipc_client_check_permission(CTSVC_PERMISSION_CONTACT_READ, &result);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission() Fail(%d)", ret);
	RETVM_IF(result == false, CONTACTS_ERROR_PERMISSION_DENIED, "Permission denied (contact read)");
#endif

	*limit_size = limit_size_of_photo;
	return CONTACTS_ERROR_NONE;
}

API int contacts_vcard_set_limit_size_of_photo(unsigned int limit_size)
{
#ifdef _CONTACTS_IPC_CLIENT
	int ret;
	bool result = false;
#endif

	RETV_IF(CTSVC_IMAGE_MAX_SIZE <= limit_size, CONTACTS_ERROR_INVALID_PARAMETER);
	RETV_IF(limit_size < 8, CONTACTS_ERROR_INVALID_PARAMETER);

#ifdef _CONTACTS_IPC_CLIENT
	ret = ctsvc_ipc_client_check_permission(CTSVC_PERMISSION_CONTACT_WRITE, &result);
	RETVM_IF(CONTACTS_ERROR_NONE != ret, ret, "ctsvc_ipc_client_check_permission() Fail(%d)", ret);
	RETVM_IF(result == false, CONTACTS_ERROR_PERMISSION_DENIED, "Permission denied (contact read)");
#endif

	limit_size_of_photo = limit_size;
	return CONTACTS_ERROR_NONE;
}


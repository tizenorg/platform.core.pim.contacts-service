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
#ifndef __TIZEN_SOCIAL_CTSVC_LOCALIZE_CH_H__
#define __TIZEN_SOCIAL_CTSVC_LOCALIZE_CH_H__

#define CHINESE_PINYIN_SPELL_MAX_LEN	15
#define	CHINESE_PINYIN_MAX_LEN			3

typedef struct {
	char pinyin_initial[CHINESE_PINYIN_MAX_LEN+1];
	char pinyin_name[CHINESE_PINYIN_SPELL_MAX_LEN*(CHINESE_PINYIN_MAX_LEN+1)];
} pinyin_name_s;

int ctsvc_convert_chinese_to_pinyin(const char *src, pinyin_name_s **name, int *size);

bool ctsvc_has_chinese(const char *src);

#endif // __TIZEN_SOCIAL_CTSVC_LOCALIZE_CH_H__

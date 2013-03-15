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
#ifndef __TIZEN_SOCIAL_CONTACTS_UTILS_H__
#define __TIZEN_SOCIAL_CONTACTS_UTILS_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @addtogroup CAPI_SOCIAL_CONTACTS_SVC_UTILS_MODULE
 * @{
 */

/**
 * Gets normalized string.
 *
 * @param[out] index_string		The pointer of language index (number, first language, second language(if differ from first), #).
 *
 * @return  0 on success, otherwise a negative error value.
 *
 * @retval  #CONTACTS_ERROR_NONE	Successful
*/
API int contacts_utils_get_index_characters(char **index_string);

/**
 * This function compares compares two strings which is not normalized.
 * If search_str is included in str, this function return #sCONTACTS_ERROR_NONE. \n
 * The behavior of this function cannot fix because of localization.
 * So, The behavior can be different from each other.
 *
 * @param[in] haystack Base string.
 * @param[in] needle searching string
 * @param[out] len substring length
 * @return a position of the beginning of the substring, Negative value(#cts_error) on error or difference.
 * @par example
 * @code
	ret = contacts_strstr(str1, str2, &len);
	if(CONTACTS_ERROR_NONE == ret) {
		snprintf(first, ret+1, "%s", item_data->display);
		snprintf(middle, len+1, "%s", item_data->display + ret);
		printf("%s -> %s, %s, %s", item_data->display, first, middle, item_data->display + ret + len);
	} else
		printf("str1 doesn't has str2");
 * @endcode
 */
API int contacts_utils_strstr(const char *haystack, const char *needle, int *len);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif //__TIZEN_SOCIAL_CONTACTS_UTILS_H__


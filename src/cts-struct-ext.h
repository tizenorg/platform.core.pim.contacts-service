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
#ifndef __CTS_STRUCT_EXT_H__
#define __CTS_STRUCT_EXT_H__

//<!--

/**
 * This function merges two contact. #s2 merges into #s1.
 * Free #s2 immediately, regardless of success.
 * #s2 must not be used after calling this function.
 * If single-value field(_VALUE suffix) is conflict, s2 value is ignored.
 * The case of multi-value field(_LIST suffix), s2 list will be append to s1 list.
 *
 *
 * @param[in] s1 The base contact
 * @param[in] s2 The contact which is added to #s1.
 * @return #CTS_SUCCESS on success, Negative value(#cts_error) on error
 */
int contacts_svc_struct_merge(CTSstruct *s1, CTSstruct *s2);

/**
 * duplicate a contact struct.
 *
 * @param[in] contact a contact struct(#CTSstruct)
 * @return a pointer to a new duplicated contact struct on success, NULL on error
 */
CTSstruct* contacts_svc_struct_duplicate(const CTSstruct *contact);

//-->

#endif //__CTS_STRUCT_EXT_H__


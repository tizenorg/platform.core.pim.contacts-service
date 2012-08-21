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
#ifndef __TEST_LOG_H__
#define __TEST_LOG_H__

#include <stdio.h>
#include <unistd.h>

#define PRT(prio, fmt, arg...) \
	do { fprintf((prio?stderr:stdout),fmt"\n", ##arg); } while (0)
#define INFO(fmt, arg...) PRT(0, fmt, ##arg)
#define ERR(fmt, arg...) PRT(1,"\x1b[101;38m[ERROR]\x1b[0m%s :" fmt, __FUNCTION__, ##arg)
#define DBG(fmt, arg...) \
	do { \
		printf("\x1b[105;37m[%s]\x1b[0m" fmt"\n", __FUNCTION__, ##arg); \
	} while (0)

#define TEST_FN_START DBG("[FUNC_START]")

#endif /* __TEST_LOG_H__ */

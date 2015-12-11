/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __TEST_DEBUG_H__
#define __TEST_DEBUG_H__

#include <dlog.h>
#include <string.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "TEST_CONTACTS"

#define COLOR_RED         "\033[0;31m"
#define COLOR_GREEN       "\033[0;32m"
#define COLOR_BROWN       "\033[0;33m"
#define COLOR_LIGHTYELLOW "\033[1;33m"
#define COLOR_BLUE        "\033[0;34m"
#define COLOR_PURPLE      "\033[0;35m"
#define COLOR_CYAN        "\033[0;36m"
#define COLOR_LIGHTBLUE   "\033[0;37m"
#define COLOR_END		  "\033[0;m"

#define DEBUG(fmt, args...)   LOGD(COLOR_CYAN fmt COLOR_END, ##args)
#define VERBOSE(fmt, args...) LOGD(COLOR_BROWN fmt COLOR_END, ##args)
#define WARNING(fmt, args...) LOGD(COLOR_PURPLE fmt COLOR_END, ##args)
#define ERROR(fmt, args...)   LOGE(COLOR_RED" * Critical * " fmt COLOR_END, ##args)
#define ENTER()               LOGD(COLOR_GREEN" BEGIN >>>> "COLOR_END)
#define LEAVE()               LOGD(COLOR_GREEN" END <<<< "COLOR_END)

#endif /* __TEST_DEBUG_H__ */

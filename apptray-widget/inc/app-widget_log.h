/*
 * Samsung API
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#ifndef __APP_WIDGET_LOG_H__
#define __APP_WIDGET_LOG_H__

#include <unistd.h>
#include <dlog.h>
#include <string.h>
#include <assert.h>

#define WCOLOR_RED "\033[0;31m"
#define WCOLOR_GREEN "\033[0;32m"
#define WCOLOR_BROWN "\033[0;33m"
#define WCOLOR_BLUE "\033[0;34m"
#define WCOLOR_PURPLE "\033[0;35m"
#define WCOLOR_CYAN "\033[0;36m"
#define WCOLOR_LIGHTBLUE "\033[0;37m"
#define WCOLOR_END		"\033[0;m"
#define MODULE_INFO (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "app-widget"


#if !defined(PACKAGE)
#define PACKAGE "org.example.app-widget"
#endif

#if !defined(LOGE)

#define LOGE(fmt, arg...) dlog_print(DLOG_DEBUG, \
        LOG_TAG, "%s: %s(%d) > " WCOLOR_GREEN fmt WCOLOR_END, MODULE_INFO, \
        __func__, __LINE__, ##arg)
#endif
#if !defined(_D)

#define _D(fmt, arg...)  dlog_print(DLOG_DEBUG, \
        LOG_TAG, "%s: %s(%d) > " WCOLOR_GREEN fmt WCOLOR_END, MODULE_INFO, \
        __func__, __LINE__, ##arg)

#endif

#if !defined(_W)
#define _W(fmt, arg...) dlog_print(DLOG_WARN, \
        LOG_TAG, "%s: %s(%d) > " fmt, MODULE_INFO, \
        __func__, __LINE__, ##arg)
#endif

#if !defined(_E)
#define _E(fmt, arg...)  dlog_print(DLOG_ERROR, \
        LOG_TAG, "%s: %s(%d) > " WCOLOR_RED fmt WCOLOR_END, MODULE_INFO, \
        __func__, __LINE__, ##arg)
#endif

#define _ENTER \
	_D("%s - Entry", __func__)
#define _EXIT \
	_D("%s - Exit", __func__)

#if !defined(_SD)
#define _SD(fmt, arg...)  dlog_print(DLOG_DEBUG, \
        LOG_TAG, "%s: %s(%d) > " WCOLOR_GREEN fmt WCOLOR_END, MODULE_INFO, \
        __func__, __LINE__, ##arg)
#endif

#if !defined(_SW)
#define _SW(fmt, arg...) dlog_print(DLOG_WARN, \
        LOG_TAG, "%s: %s(%d) > " fmt, MODULE_INFO, \
        __func__, __LINE__, ##arg)
#endif

#if !defined(_SE)
#define _SE(fmt, arg...)  dlog_print(DLOG_ERROR, \
        LOG_TAG, "%s: %s(%d) > " WCOLOR_RED fmt WCOLOR_END, MODULE_INFO, \
        __func__, __LINE__, ##arg)
#endif

#define retvm_if_timer(timer, expr, val, fmt, arg...) do { \
	if (expr) { \
		_E(fmt, ##arg); \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		timer = NULL; \
		return (val); \
	} \
} while (0)

#define retvm_if(expr, val, fmt, arg...) do { \
	if(expr) { \
		_E(fmt, ##arg); \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return val; \
	} \
} while (0)

#define retv_if(expr, val) do { \
	if(expr) { \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return (val); \
	} \
} while (0)

#define retm_if(expr, fmt, arg...) do { \
	if(expr) { \
		_E(fmt, ##arg); \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define ret_if(expr) do { \
	if(expr) { \
		_E("(%s) -> %s() return", #expr, __FUNCTION__); \
		return; \
	} \
} while (0)

#define goto_if(expr, val) do { \
	if(expr) { \
		_E("(%s) -> goto", #expr); \
		goto val; \
	} \
} while (0)

#define break_if(expr) { \
	if(expr) { \
		_E("(%s) -> break", #expr); \
		break; \
	} \
}

#define continue_if(expr) { \
	if(expr) { \
		_E("(%s) -> continue", #expr); \
		continue; \
	} \
}

#endif

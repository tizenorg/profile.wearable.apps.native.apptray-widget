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
#ifndef __APPS_ITEM_INFO_H__
#define __APPS_ITEM_INFO_H__

#include <Elementary.h>
#include <Evas.h>
#include <util.h>
#define APP_TYPE_WGT "wgt"
#define APP_TYPE_RECENT_APPS "recent"
#define APP_TYPE_MORE_APPS "more"

typedef struct {
	/* innate features */
	char *pkgid;
	char *appid;
	char *name;
	char *icon;
	char *type;

	int ordering;
	int open_app;
	int tts;
	int removable;
	int uneditable;

	/* acquired features */
	Evas_Object *win;
	Evas_Object *layout;

	Evas_Object *scroller;
	Evas_Object *page;

	Evas_Object *item;
	Evas_Object *item_inner;
	Evas_Object *center;
} item_info_s;

typedef enum {
	ITEM_INFO_LIST_TYPE_INVALID = 0,
	ITEM_INFO_LIST_TYPE_ALL,
	ITEM_INFO_LIST_TYPE_FACTORY_BINARY,
	ITEM_INFO_LIST_TYPE_XML,
	ITEM_INFO_LIST_TYPE_MAX,
} item_info_list_type_e;

HAPI void apps_item_info_destroy(item_info_s *item_info);
HAPI item_info_s *apps_item_info_create(const char *appid);

HAPI item_info_s *apps_recent_apps_item_info_create();
HAPI item_info_s *apps_more_apps_item_info_create();
HAPI item_info_s *apps_item_info_get(Evas_Object *win, const char *appid);

HAPI Eina_List *apps_item_info_list_create(item_info_list_type_e list_type);
HAPI void apps_item_info_list_destroy(Eina_List *item_info_list);

HAPI Evas_Object *apps_item_info_list_get_item(Eina_List *item_info_list, const char *appid);
HAPI int apps_item_info_list_get_ordering(Eina_List *item_info_list, const char *appid);

HAPI void apps_item_info_list_change_language(Eina_List *item_info_list);

HAPI int apps_item_info_is_support_tts(const char *appid);

HAPI item_info_s *apps_item_info_dup(item_info_s *item_info);

#endif // __APPS_ITEM_INFO_H__

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

#ifndef __HOME_CUSTOM_SCROLLER_H__
#define __HOME_CUSTOM_SCROLLER_H__

#include <Evas.h>
#include "common/home_custom_scroller_info.h"

#if !defined(SCROLLAPI)
#define HCSAPI __attribute__((visibility("hidden")))
#endif

typedef enum {
	HOME_CUSTOM_SC_DIRECTION_NONE,
	HOME_CUSTOM_SC_DIRECTION_LEFT,
	HOME_CUSTOM_SC_DIRECTION_RIGHT,
	HOME_CUSTOM_SC_DIRECTION_UP,
	HOME_CUSTOM_SC_DIRECTION_DOWN,
} Home_Custom_Sc_Direction;

HCSAPI scroller_info_s *home_custom_scroller_info_create(void);

HCSAPI Evas_Object *home_custom_scroller_add(Evas_Object *parent, scroller_info_s *scroller_info);
HCSAPI void home_custom_scroller_del(Evas_Object *scroller);

HCSAPI void home_custom_scroller_pause(Evas_Object *scroller);
HCSAPI void home_custom_scroller_resume(Evas_Object *scroller);

HCSAPI void home_custom_scroller_page_append(Evas_Object *scroller, Evas_Object *page);
HCSAPI void home_custom_scroller_page_prepend(Evas_Object *scroller, Evas_Object *page);
HCSAPI void home_custom_scroller_page_remove(Evas_Object *scroller, Evas_Object *page);
HCSAPI int home_custom_scroller_get_page_count(Evas_Object *scroller);

HCSAPI void home_custom_scroller_list_append(Evas_Object *scroller, Eina_List *page_list);
HCSAPI void home_custom_scroller_list_remove(Evas_Object *scroller, Eina_List *page_list);
HCSAPI Eina_List *home_custom_scroller_get_page_list(Evas_Object *scroller);

HCSAPI void home_custom_scroller_edit(Evas_Object *scroller);
HCSAPI void home_custom_scroller_unedit(Evas_Object *scroller);

HCSAPI void home_custom_scroller_freeze(Evas_Object *scroller, Eina_Bool status);
HCSAPI Eina_Bool home_custom_scroller_is_freeze(Evas_Object *scroller);

HCSAPI void home_custom_scroller_page_freeze(Evas_Object *scroller, Evas_Object *page, Eina_Bool status);
HCSAPI Eina_Bool home_custom_scroller_is_page_freeze(Evas_Object *scroller, Evas_Object *page);

HCSAPI void home_custom_scroller_bring_in(Evas_Object *scroller, int page_no);
HCSAPI void home_custom_scroller_bring_in_page(Evas_Object *scroller, Evas_Object *page);
HCSAPI void home_custom_scroller_bring_in_direction(Evas_Object *scroller, Home_Custom_Sc_Direction direction);
HCSAPI Eina_Bool home_custom_scroller_is_scrolling(Evas_Object *scroller);

HCSAPI void home_custom_scroller_region_show(Evas_Object *scroller, int x, int y);

HCSAPI Evas_Object *home_custom_scroller_get_first_page(Evas_Object *scroller);
HCSAPI Evas_Object *home_custom_scroller_get_last_page(Evas_Object *scroller);
HCSAPI Evas_Object *home_custom_scroller_get_current_page(Evas_Object *scroller);
HCSAPI Evas_Object *home_custom_scroller_get_page(Evas_Object *scroller, int index);
HCSAPI Eina_Bool home_custom_scroller_is_edge(Evas_Object *page);

//index
HCSAPI Evas_Object *home_custom_scroller_index_add(Evas_Object *parent, Evas_Object *scroller);

#endif //__HOME_CUSTOM_SCROLLER_H__

// End of a file

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

#ifndef __HOME_CUSTOM_SCROLLER_INFO_H__
#define __HOME_CUSTOM_SCROLLER_INFO_H__

#include <Evas.h>

typedef struct {
	Evas_Object *win;

	Evas_Object *parent;
	Evas_Object *box_layout;
	Evas_Object *box;
	Evas_Object *layout_focus;
	Evas_Object *top_focus;
	Evas_Object *bottom_focus;

	Eina_List *list;
	int list_index;

	int page_width;
	int page_height;

	int edge_width;
	int edge_height;
} scroller_info_s;

#endif //__HOME_CUSTOM_SCROLLER_INFO_H__

// End of a file

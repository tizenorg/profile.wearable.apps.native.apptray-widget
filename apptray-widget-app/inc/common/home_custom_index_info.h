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

#ifndef __W_HOME_CUSTOM_INDEX_INFO_H__
#define __W_HOME_CUSTOM_INDEX_INFO_H__

typedef struct {
	/* innate features; */
	int count;
	int current_no;

	/* acquired features */
	Evas_Object *layout;
	Evas_Object *scroller;
	Eina_List *page_index_list;
} hc_index_info_s;

#endif // __W_HOME_CUSTOM_INDEX_INFO_H__

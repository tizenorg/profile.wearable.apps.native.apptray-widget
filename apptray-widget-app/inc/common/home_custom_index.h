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

#ifndef __W_HOME_CUSTOM_INDEX_H__
#define __W_HOME_CUSTOM_INDEX_H__

typedef enum {
	INDEX_BRING_IN_NONE = 0,
	INDEX_BRING_IN_AFTER,
} hc_index_bring_in_e;

extern void home_custom_index_bring_in_page(Evas_Object *index, Evas_Object *page);

extern Evas_Object *home_custom_index_create(Evas_Object *layout, Evas_Object *scroller);
extern void home_custom_index_destroy(Evas_Object *index);
extern void home_custom_index_update(Evas_Object *index, Evas_Object *scroller, hc_index_bring_in_e after);
extern void home_custom_index_show(Evas_Object *index, Eina_Bool bAutoHide);

#endif /* __W_HOME_CUSTOM_INDEX_H__ */

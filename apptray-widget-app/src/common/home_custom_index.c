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
#include <Elementary.h>

#include <bundle.h>
#include <util.h>


#include "common/home_custom_index_info.h"
#include "common/home_custom_index.h"
#include "common/home_custom_scroller.h"
#include "app-widget-app_log.h"

#define PRIVATE_DATA_KEY_INDEX_TIMER "p_in_timer"

typedef struct {
	Evas_Object *page;
	int index;
} page_index_s;


#define INDEX_EVEN_ITEM_NUM 20
#define INDEX_ODD_ITEM_NUM 19
static const char *_index_style_get(int index, int even)
{
	static const char g_it_style_odd[INDEX_ODD_ITEM_NUM][20] = {
		"item/odd_1",
		"item/odd_2",
		"item/odd_3",
		"item/odd_4",
		"item/odd_5",
		"item/odd_6",
		"item/odd_7",
		"item/odd_8",
		"item/odd_9",
		"item/odd_10",
		"item/odd_11",
		"item/odd_12",
		"item/odd_13",
		"item/odd_14",
		"item/odd_15",
		"item/odd_16",
		"item/odd_17",
		"item/odd_18",
		"item/odd_19",
	};
	static const char g_it_style_even[INDEX_EVEN_ITEM_NUM][20] = {
		"item/even_1",
		"item/even_2",
		"item/even_3",
		"item/even_4",
		"item/even_5",
		"item/even_6",
		"item/even_7",
		"item/even_8",
		"item/even_9",
		"item/even_10",
		"item/even_11",
		"item/even_12",
		"item/even_13",
		"item/even_14",
		"item/even_15",
		"item/even_16",
		"item/even_17",
		"item/even_18",
		"item/even_19",
		"item/even_20",
	};

	index = (index < 0) ? 0 : index;
	if (even) {
		index = (index >= INDEX_EVEN_ITEM_NUM) ? INDEX_EVEN_ITEM_NUM - 1 : index;

		return g_it_style_even[index];
	} else {
		index = (index >= INDEX_ODD_ITEM_NUM) ? INDEX_ODD_ITEM_NUM - 1 : index;

		return g_it_style_odd[index];
	}
}


HAPI void home_custom_index_bring_in_page(Evas_Object *index, Evas_Object *page)
{
	_ENTER;
	Elm_Object_Item *idx_it = NULL;
	const Eina_List *l = NULL;
	hc_index_info_s *index_info = NULL;
	page_index_s *page_index = NULL;
	int idx = 0;
	int found = 0;

	ret_if(!index);
	ret_if(!page);

	index_info = evas_object_data_get(index, DATA_KEY_INDEX_INFO);
	ret_if(!index_info);
	ret_if(!index_info->page_index_list);

	EINA_LIST_FOREACH(index_info->page_index_list, l, page_index) {
		if (page_index->page == page) {
			idx = page_index->index;
			found = 1;
			break;
		}
	}

	if (!found) {
		_E("Cannot find a page(%p)", page);
		return;
	}

	idx_it = elm_index_item_find(index, (void *) idx);
	if (idx_it) {
		elm_index_item_selected_set(idx_it, EINA_TRUE);
	} else {
		_E("Critical, the index(%p) cannot find the page(%p:%d)", index, page, idx);
	}
}



HAPI Evas_Object *home_custom_index_create(Evas_Object *parent, Evas_Object *scroller)
{
	_ENTER;
	Evas_Object *index = NULL;
	hc_index_info_s *index_info = NULL;

	retv_if(!parent, NULL);
	retv_if(!scroller, NULL);

	index = elm_index_add(parent);
	retv_if(!index, NULL);
	elm_object_style_set(index, "circle");

	index_info = calloc(1, sizeof(hc_index_info_s));
	if (!index_info) {
		_E("Cannot calloc for index_info");
		evas_object_del(index);
		return NULL;
	}
	evas_object_data_set(index, DATA_KEY_INDEX_INFO, index_info);

	evas_object_size_hint_weight_set(index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(index, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_index_horizontal_set(index, EINA_TRUE);
	elm_index_autohide_disabled_set(index, EINA_TRUE);
	elm_index_level_go(index, 0);
	evas_object_show(index);

	index_info->layout = parent;
	index_info->scroller = scroller;

	return index;
}



HAPI void home_custom_index_destroy(Evas_Object *index)
{
	_ENTER;
	hc_index_info_s *index_info = NULL;
	page_index_s *page_index = NULL;

	ret_if(!index);

	index_info = evas_object_data_del(index, DATA_KEY_INDEX_INFO);
	ret_if(!index_info);

	if (index_info->page_index_list) {
		EINA_LIST_FREE(index_info->page_index_list, page_index) {
			free(page_index);
		}
		index_info->page_index_list = NULL;
	}

	free(index_info);
	elm_index_item_clear(index);
	evas_object_del(index);
}



#define MAX_INDEX_NUMBER 20
static void _update_index(Evas_Object *scroller, Evas_Object *index, const Eina_List *list)
{
	_ENTER;
	Evas_Object *page = NULL;
	Elm_Object_Item *idx_it = NULL;
	const Eina_List *l = NULL;

	hc_index_info_s *index_info = NULL;
	page_index_s *page_index = NULL;

	int extra_idx = 0, page_index_inserting = 0;
	int cur_inserted = 0;
	int index_number = 0;
	int cur_start_idx = 0, cur_mid_idx = 0;
	int total_count = 0, total_inserted = 0;

	ret_if(!index);
	ret_if(!list);

	index_info = evas_object_data_get(index, DATA_KEY_INDEX_INFO);
	ret_if(!index_info);

	/* 0. Remove an old page_index_list */
	if (index_info->page_index_list) {
		EINA_LIST_FREE(index_info->page_index_list, page_index) {
			free(page_index);
		}
		index_info->page_index_list = NULL;
	}

	/* 1. Make indexes (cur/center/other) */
	total_count = home_custom_scroller_get_page_count(scroller);

	total_inserted = total_count;
	if (total_inserted > MAX_INDEX_NUMBER) {
		total_inserted = MAX_INDEX_NUMBER;
	}

	int style_even = 0;
	int style_base = 0;
	if (!(total_inserted % 2)) {
		style_even = 1;
		style_base = (MAX_INDEX_NUMBER/2) - (total_inserted / 2);
	} else {
		style_base = (MAX_INDEX_NUMBER/2) - (total_inserted / 2) - 1;
	}
	_D("style even:%d,style base:%d",style_even,style_base);

	EINA_LIST_FOREACH(list, l, page) {
		if (index_number < total_inserted) {
			if (!cur_inserted) {
				cur_start_idx = index_number;
			}
			cur_inserted++;
			idx_it = elm_index_item_append(index, NULL, NULL, (void *) index_number);
			elm_object_item_style_set(idx_it, _index_style_get(style_base + index_number, style_even));
			index_number++;
		}
	}

	cur_mid_idx = (cur_start_idx + cur_inserted - 1) / 2;
	extra_idx = total_count - cur_inserted;

	/* 2. Make a new page_index_list */
	page_index_inserting = -1;
	EINA_LIST_FOREACH(list, l, page) {
		page_index = calloc(1, sizeof(page_index_s));
		continue_if(!page_index);

		if (page_index_inserting == cur_mid_idx && extra_idx > 0) {
			extra_idx--;
		} else {
			page_index_inserting++;
		}

		page_index->page = page;
		page_index->index = page_index_inserting;

		index_info->page_index_list = eina_list_append(index_info->page_index_list, page_index);
	}
}



HAPI void home_custom_index_update(Evas_Object *index, Evas_Object *scroller, hc_index_bring_in_e after)
{
	_ENTER;
	Evas_Object *page_current = NULL;
	Eina_List *page_list = NULL;

	hc_index_info_s *index_info = NULL;

	ret_if(!index);
	ret_if(!scroller);

	_D("Index(%p) is clear", index);
	elm_index_item_clear(index);

	index_info = evas_object_data_get(index, DATA_KEY_INDEX_INFO);
	ret_if(!index_info);

	page_list = home_custom_scroller_get_page_list(scroller);
	ret_if(!page_list);

	elm_object_style_set(index, "circle");

	_update_index(scroller, index, page_list);

	eina_list_free(page_list);

	elm_index_level_go(index, 0);

	if (INDEX_BRING_IN_AFTER == after) {
		page_current = home_custom_scroller_get_current_page(scroller);
		ret_if(!page_current);
		home_custom_index_bring_in_page(index, page_current);
	}
}


static Eina_Bool _index_hide_timer_cb(void *data)
{
	_ENTER;
	Evas_Object *index = (Evas_Object*)data;
	retv_if(!index, ECORE_CALLBACK_CANCEL);

	Elm_Transit *transit = elm_transit_add();
	retv_if(!transit, ECORE_CALLBACK_CANCEL);

	elm_transit_effect_color_add(transit, 255, 255, 255, 255, 0, 0, 0, 0);
	elm_transit_object_add(transit, index);
	elm_transit_duration_set(transit, 0.5);
	elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
	elm_transit_objects_final_state_keep_set(transit, EINA_TRUE);
	elm_transit_go(transit);

	evas_object_data_del(index, PRIVATE_DATA_KEY_INDEX_TIMER);

	return ECORE_CALLBACK_CANCEL;
}


HAPI void home_custom_index_show(Evas_Object *index, Eina_Bool bAutoHide)
{
	_ENTER;
	Ecore_Timer *timer = NULL;

	ret_if(!index);

	evas_object_color_set(index, 255, 255, 255, 255);
	evas_object_show(index);

	if(bAutoHide) {
		timer = evas_object_data_get(index, PRIVATE_DATA_KEY_INDEX_TIMER);
		if(timer) {
			ecore_timer_del(timer);
		}

		timer = ecore_timer_add(2.0, _index_hide_timer_cb, index);
		ret_if(!timer);

		evas_object_data_set(index, PRIVATE_DATA_KEY_INDEX_TIMER, timer);
	}
}


 // End of file


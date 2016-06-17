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
#include <app_control.h>
//#include <vconf.h>
#include <efl_extension.h>
//#include <uxt_scroller.h>


#include "common/home_custom_scroller.h"
#include "common/home_custom_scroller_info.h"
#include "common/home_custom_index.h"
#include "app-widget-app.h"
#include "util.h"


#define SCROLLER_LAYOUT_EDJE "edje/home_custom_layout.edj"
#define SCROLLER_LAYOUT_GROUP_NAME "layout"
#define SCROLLER_BOX_PART_NAME "box"

#define PRIVATE_DATA_KEY_SCROLLER_MOUSE_DOWN "p_sc_m_dn"
#define PRIVATE_DATA_KEY_SCROLLER_DRAG "p_sc_dr"
#define PRIVATE_DATA_KEY_IS_SCROLLING "p_is_scr"
#define PRIVATE_DATA_KEY_SCROLLER_BRING_IN_TIMER "p_sc_tmer"
#define PRIVATE_DATA_KEY_SCROLLER_BRING_IN_ITH "p_sc_ith"
#define PRIVATE_DATA_KEY_SCROLLER_INFO "p_sc_if"

#define PRIVATE_DATA_KEY_SCROLLER_PAGE_MOUSE_DOWN "p_sc_pg_m_dn"

#define PRIVATE_DATA_KEY_INDEX "p_id"
#define PRIVATE_DATA_KEY_START_EDGE_RECT "p_s_er"
#define PRIVATE_DATA_KEY_END_EDGE_RECT "p_e_er"
#define PRIVATE_DATA_KEY_EDGE_RECT "p_e_r"

Eina_Bool scroll_transit_go = EINA_FALSE;
#define PRIVATE_DATA_KEY_SCROLLER_ANIM_START_FN "p_sc_anim_start_fn"
#define PRIVATE_DATA_KEY_SCROLLER_ANIM_STOP_FN "p_sc_anim_stop_fn"
#define PRIVATE_DATA_KEY_SCROLLER_DRAG_START_FN "p_sc_drag_start_fn"
#define PRIVATE_DATA_KEY_SCROLLER_DRAG_STOP_FN "p_sc_drag_start_fn"
#define PRIVATE_DATA_KEY_SCROLLER_SCROLL_FN "p_sc_scroll_fn"

int home_custom_scroller_get_current_page_no(Evas_Object *scroller);

static void _scroller_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_ENTER;
	Evas_Event_Mouse_Down *ei = event_info;
	ret_if(!ei);

	_D("Mouse is down [%d,%d]", ei->output.x, ei->output.y);
	evas_object_data_set(obj, PRIVATE_DATA_KEY_SCROLLER_MOUSE_DOWN, (void *) 1);
}



static void _scroller_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_ENTER;
	Evas_Event_Mouse_Up *ei = event_info;
	ret_if(!ei);

	_D("Mouse is up [%d,%d]", ei->output.x, ei->output.y);
	evas_object_data_del(obj, PRIVATE_DATA_KEY_SCROLLER_MOUSE_DOWN);

	if (evas_object_data_get(obj, PRIVATE_DATA_KEY_SCROLLER_DRAG)) return;
	evas_object_data_del(obj, PRIVATE_DATA_KEY_IS_SCROLLING);
}



static void _default_anim_start_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_ENTER;
	_D("start the scroller animation");
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_IS_SCROLLING, (void *)1);

	void *(*_anim_start_cb)(void *, Evas_Object *, void *);
	_anim_start_cb = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_ANIM_START_FN);
	if(_anim_start_cb) _anim_start_cb(data, scroller, event_info);

	Evas_Object *page = home_custom_scroller_get_current_page(scroller);
	ret_if(!page);
}



static void _default_anim_stop_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_ENTER;
	_D("stop the scroller animation");

	void *(*_anim_stop_cb)(void *, Evas_Object *, void *);
	_anim_stop_cb = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_ANIM_STOP_FN);
	if(_anim_stop_cb) _anim_stop_cb(data, scroller, event_info);

	if (evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_MOUSE_DOWN)) return;
	evas_object_data_del(scroller, PRIVATE_DATA_KEY_IS_SCROLLING);

	scroll_transit_go = EINA_FALSE;

	Evas_Object *page = home_custom_scroller_get_current_page(scroller);
	ret_if(!page);

//	elm_object_signal_emit(page, "focus_out_effect", "item");

	Evas_Object *index = evas_object_data_get(scroller, PRIVATE_DATA_KEY_INDEX);
	if(index) home_custom_index_bring_in_page(index, page);
}



static void _default_drag_start_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_ENTER;
	ret_if(!scroller);

	_D("start to drag the scroller animation");

	evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_DRAG, (void *) 1);

	void *(*_drag_start_cb)(void *, Evas_Object *, void *);
	_drag_start_cb = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_DRAG_START_FN);
	if(_drag_start_cb) _drag_start_cb(data, scroller, event_info);

	/* _drag_start_cb is called even if the scroller is not moved. */
	//evas_object_data_set(scroller, PRIVATE_DATA_KEY_IS_SCROLLING, (void *) 1);

	Evas_Object *page = home_custom_scroller_get_current_page(scroller);
	ret_if(!page);

//	elm_object_signal_emit(page, "focus_in_effect", "item");
	scroll_transit_go = EINA_TRUE;
}



static void _default_drag_stop_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_ENTER;
	ret_if(!scroller);

	_D("stop to drag the scroller animation");

	void *(*_drag_stop_cb)(void *, Evas_Object *, void *);
	_drag_stop_cb = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_DRAG_STOP_FN);
	if(_drag_stop_cb) _drag_stop_cb(data, scroller, event_info);

	evas_object_data_del(scroller, PRIVATE_DATA_KEY_SCROLLER_DRAG);
}



static void _default_scroll_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_ENTER;
	ret_if(!scroller);

	evas_object_data_set(scroller, PRIVATE_DATA_KEY_IS_SCROLLING, (void *)1);

	void *(*_scroll_cb)(void *, Evas_Object *, void *);
	_scroll_cb = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_SCROLL_FN);
	if(_scroll_cb) _scroll_cb(data, scroller, event_info);

	Evas_Object *page = home_custom_scroller_get_current_page(scroller);
	ret_if(!page);

	Evas_Object *index = evas_object_data_get(scroller, PRIVATE_DATA_KEY_INDEX);
	if(index) home_custom_index_bring_in_page(index, page);
}


static void _init_rotary(Evas_Object *scroller)
{
	_ENTER;
	_D("Initialize the rotary event");
//	eext_rotary_event_callback_set(scroller, _rotary_cb, NULL);
}



static void _destroy_rotary(Evas_Object *scroller)
{
	_ENTER;
	_D("Finish the rotary event");
//	eext_rotary_event_callback_set(scroller, NULL, NULL);
}

scroller_info_s *home_custom_scroller_info_create(void)
{
	_ENTER;
	scroller_info_s *scroller_info = NULL;

	scroller_info = calloc(1, sizeof(scroller_info_s));
	retv_if(!scroller_info, NULL);

	return scroller_info;
}


void home_custom_scroller_register_cb(Evas_Object *scroller,
									void _anim_start_cb(void *, Evas_Object *, void *),
									void _anim_stop_cb(void *, Evas_Object *, void *),
									void _drag_start_cb(void *, Evas_Object *, void *),
									void _drag_stop_cb(void *, Evas_Object *, void *),
									void _scroll_cb(void *, Evas_Object *, void *))
{
	_ENTER;
	ret_if(!scroller);

	evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_ANIM_START_FN, _anim_start_cb);
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_ANIM_STOP_FN, _anim_stop_cb);
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_DRAG_START_FN, _drag_start_cb);
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_DRAG_START_FN, _drag_start_cb);
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_SCROLL_FN, _scroll_cb);
}


void home_custom_scroller_deregister_cb(Evas_Object *scroller)
{
	_ENTER;
	ret_if(!scroller);

	evas_object_data_del(scroller, PRIVATE_DATA_KEY_SCROLLER_ANIM_START_FN);
	evas_object_data_del(scroller, PRIVATE_DATA_KEY_SCROLLER_ANIM_STOP_FN);
	evas_object_data_del(scroller, PRIVATE_DATA_KEY_SCROLLER_DRAG_START_FN);
	evas_object_data_del(scroller, PRIVATE_DATA_KEY_SCROLLER_DRAG_START_FN);
	evas_object_data_del(scroller, PRIVATE_DATA_KEY_SCROLLER_SCROLL_FN);
}

Evas_Object *home_custom_scroller_add(Evas_Object *parent, scroller_info_s *scroller_info)
{
	_ENTER;
//	Evas_Object *win = NULL;
	Evas_Object *scroller = NULL;
	Evas_Object *box_layout = NULL;
	Evas_Object *box = NULL;

//	int height;

	scroller = elm_scroller_add(parent);
	retv_if(!scroller, NULL);
	_D("scroller is added");
	elm_scroller_loop_set(scroller, EINA_FALSE, EINA_FALSE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_page_size_set(scroller, scroller_info->page_width, scroller_info->page_height);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_page_scroll_limit_set(scroller, 1, 0);
	elm_object_scroll_lock_y_set(scroller, EINA_TRUE);
//	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_FALSE);
//	uxt_scroller_set_page_flick_enabled(scroller, EINA_TRUE);

#if 0 /* Scroll to move pages at a time */
	elm_scroller_page_size_set(scroller, instance_info->root_w, ITEM_HEIGHT * apps_main_get_info()->scale);
	elm_scroller_page_scroll_limit_set(scroller, 0, 3);
#endif
//	elm_scroller_content_min_limit(scroller, EINA_FALSE, EINA_TRUE);
//	elm_scroller_single_direction_set(scroller, ELM_SCROLLER_SINGLE_DIRECTION_HARD);

	Evas_Object *circle_scroller = eext_circle_object_scroller_add(scroller, NULL);
	eext_circle_object_scroller_policy_set(circle_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	eext_rotary_object_event_activated_set(circle_scroller, EINA_TRUE);

	elm_object_style_set(scroller, "effect");

	evas_object_event_callback_add(scroller, EVAS_CALLBACK_MOUSE_DOWN, _scroller_mouse_down_cb, NULL);
	evas_object_event_callback_add(scroller, EVAS_CALLBACK_MOUSE_UP, _scroller_mouse_up_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll,anim,start", _default_anim_start_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll,anim,stop", _default_anim_stop_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll,drag,start", _default_drag_start_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll,drag,stop", _default_drag_stop_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll", _default_scroll_cb, NULL);

	/* Use the layout between box and scroller because of alignment of a page in the box. */
	box_layout = elm_layout_add(scroller);
	goto_if(!box_layout, ERROR);
	_D("box layout is created for scroller");
	char full_path[PATH_MAX] = { 0, };
	_get_resource(SCROLLER_LAYOUT_EDJE, full_path, sizeof(full_path));
	_D("full_path:%s",full_path);
	elm_layout_file_set(box_layout, full_path, SCROLLER_LAYOUT_GROUP_NAME);
	evas_object_show(box_layout);
	elm_object_content_set(scroller, box_layout);
	_D("scroller content is set as box layout");
	/* Create Box */
	box = elm_box_add(box_layout);
	goto_if(!box, ERROR);
	_D("box is added to boxlayout");
	elm_box_horizontal_set(box, EINA_TRUE);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(box);
	elm_object_part_content_set(box_layout, SCROLLER_BOX_PART_NAME, box);

	Evas_Object *start_rect = evas_object_rectangle_add(evas_object_evas_get(box));
	evas_object_size_hint_min_set(start_rect, (scroller_info->edge_width), (scroller_info->edge_height));
	evas_object_data_set(start_rect, PRIVATE_DATA_KEY_EDGE_RECT, (void*)1);
	elm_box_pack_end(box, start_rect);

	Evas_Object *end_rect = evas_object_rectangle_add(evas_object_evas_get(box));
	evas_object_size_hint_min_set(end_rect, (scroller_info->edge_width), (scroller_info->edge_height));
	evas_object_data_set(end_rect, PRIVATE_DATA_KEY_EDGE_RECT, (void*)1);
	elm_box_pack_end(box, end_rect);

	evas_object_data_set(scroller, PRIVATE_DATA_KEY_START_EDGE_RECT, start_rect);
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_END_EDGE_RECT, end_rect);

	scroller_info->parent = parent;
	scroller_info->box_layout = box_layout;
	scroller_info->box = box;

	evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO, scroller_info);

//	uxt_scroller_set_rotary_event_enabled(scroller, EINA_TRUE);

	_init_rotary(scroller);
	_D("scroller created success");
	return scroller;

ERROR:
	home_custom_scroller_del(scroller);
	return NULL;
}


void home_custom_scroller_del(Evas_Object *scroller)
{
	_ENTER;
	Evas_Object *page = NULL;
	Eina_List *box_list = NULL;
	scroller_info_s *scroller_info = NULL;

	ret_if(!scroller);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	home_custom_scroller_deregister_cb(scroller);
	_destroy_rotary(scroller);

	box_list = elm_box_children_get(scroller_info->box);

	/* FIXME : We don't need to remove items? */
	EINA_LIST_FREE(box_list, page) {
		if (!page) break;
		evas_object_del(page);
	}

	evas_object_del(scroller_info->box);
	evas_object_del(scroller_info->box_layout);

	evas_object_event_callback_del(scroller, EVAS_CALLBACK_MOUSE_DOWN, _scroller_mouse_down_cb);
	evas_object_event_callback_del(scroller, EVAS_CALLBACK_MOUSE_UP, _scroller_mouse_up_cb);
	evas_object_smart_callback_del(scroller, "scroll,anim,start", _default_anim_start_cb);
	evas_object_smart_callback_del(scroller, "scroll,anim,stop", _default_anim_stop_cb);
	evas_object_smart_callback_del(scroller, "scroll,drag,start", _default_drag_start_cb);
	evas_object_smart_callback_del(scroller, "scroll,drag,stop", _default_drag_stop_cb);
	evas_object_smart_callback_del(scroller, "scroll", _default_scroll_cb);

	free(scroller_info);
	evas_object_del(scroller);
}


void home_custom_scroller_pause(Evas_Object *scroller)
{
	_ENTER;
	_D("");
}


void home_custom_scroller_resume(Evas_Object *scroller)
{
	_ENTER;
	eext_rotary_object_event_activated_set(scroller, EINA_TRUE);

	Evas_Object *index = (Evas_Object*)evas_object_data_get(scroller, PRIVATE_DATA_KEY_INDEX);
	if(index)
		home_custom_index_show(index, EINA_FALSE);
}


static void _page_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_ENTER;
	Evas_Event_Mouse_Down *ei = event_info;
	ret_if(!ei);

	_D("Mouse is down [%d,%d]", ei->output.x, ei->output.y);
	evas_object_data_set(obj, PRIVATE_DATA_KEY_SCROLLER_PAGE_MOUSE_DOWN, (void *) 1);
}


static void _page_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_ENTER;
	Evas_Event_Mouse_Up *ei = event_info;
	ret_if(!ei);

	_D("Mouse is up [%d,%d]", ei->output.x, ei->output.y);
	evas_object_data_del(obj, PRIVATE_DATA_KEY_SCROLLER_PAGE_MOUSE_DOWN);
}


void home_custom_scroller_page_append(Evas_Object *scroller, Evas_Object *page)
{
	_ENTER;
	scroller_info_s *scroller_info = NULL;

	ret_if(!scroller);
	ret_if(!page);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);
	_D("before adding mouse events");
	evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_DOWN, _page_mouse_down_cb, NULL);
	evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_UP, _page_mouse_up_cb, NULL);

	Evas_Object *end_rect = evas_object_data_get(scroller, PRIVATE_DATA_KEY_END_EDGE_RECT);
	if(end_rect) {
		elm_box_pack_before(scroller_info->box, page, end_rect);
	}
	else {
		elm_box_pack_end(scroller_info->box, page);
	}
}


void home_custom_scroller_page_prepend(Evas_Object *scroller, Evas_Object *page)
{
	_ENTER;
	scroller_info_s *scroller_info = NULL;

	ret_if(!scroller);
	ret_if(!page);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_DOWN, _page_mouse_down_cb, NULL);
	evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_UP, _page_mouse_up_cb, NULL);

	Evas_Object *start_rect = evas_object_data_get(scroller, PRIVATE_DATA_KEY_END_EDGE_RECT);
	if(start_rect) {
		elm_box_pack_after(scroller_info->box, page, start_rect);
	}
	else {
		elm_box_pack_start(scroller_info->box, page);
	}
}


void home_custom_scroller_page_remove(Evas_Object *scroller, Evas_Object *page)
{
	_ENTER;
	scroller_info_s *scroller_info = NULL;

	ret_if(!scroller);
	ret_if(!page);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	evas_object_event_callback_del(page, EVAS_CALLBACK_MOUSE_DOWN, _page_mouse_down_cb);
	evas_object_event_callback_del(page, EVAS_CALLBACK_MOUSE_UP, _page_mouse_up_cb);

	elm_box_unpack(scroller_info->box, page);

	int nTotalCount = home_custom_scroller_get_page_count(scroller);
	int nPageNo = home_custom_scroller_get_current_page_no(scroller);

	if(nPageNo >= nTotalCount) {
		home_custom_scroller_region_show(scroller, nTotalCount-1, 0);
	}
}


int home_custom_scroller_get_page_count(Evas_Object *scroller)
{
	_ENTER;
	int count = 0;
	Eina_List *page_list = NULL;
	scroller_info_s *scroller_info = NULL;

	retv_if(!scroller, 0);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	retv_if(!scroller_info, 0);

	page_list = elm_box_children_get(scroller_info->box);
	retv_if(!page_list, 0);

	count = eina_list_count(page_list);
	count = count - 2;

	eina_list_free(page_list);

	return count;
}


void home_custom_scroller_list_append(Evas_Object *scroller, Eina_List *page_list)
{
	_ENTER;
	scroller_info_s *scroller_info = NULL;
	Eina_List *l = NULL;
	Eina_List *n = NULL;
	Evas_Object *page = NULL;

	ret_if(!scroller);
	ret_if(!page_list);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	EINA_LIST_FOREACH_SAFE(page_list, l, n, page) {

		evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_DOWN, _page_mouse_down_cb, NULL);
		evas_object_event_callback_add(page, EVAS_CALLBACK_MOUSE_UP, _page_mouse_up_cb, NULL);

		Evas_Object *end_rect = evas_object_data_get(scroller, PRIVATE_DATA_KEY_END_EDGE_RECT);
		if(end_rect) {
			elm_box_pack_before(scroller_info->box, page, end_rect);
		}
		else {
			elm_box_pack_end(scroller_info->box, page);
		}
	}
}


void home_custom_scroller_list_remove(Evas_Object *scroller, Eina_List *page_list)
{
	_ENTER;
	scroller_info_s *scroller_info = NULL;
	Eina_List *l = NULL;
	Eina_List *n = NULL;
	Evas_Object *page = NULL;

	ret_if(!scroller);
	ret_if(!page_list);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	EINA_LIST_FOREACH_SAFE(page_list, l, n, page) {

		evas_object_event_callback_del(page, EVAS_CALLBACK_MOUSE_DOWN, _page_mouse_down_cb);
		evas_object_event_callback_del(page, EVAS_CALLBACK_MOUSE_UP, _page_mouse_up_cb);

		elm_box_unpack(scroller_info->box, page);
	}
}


Eina_List *home_custom_scroller_get_page_list(Evas_Object *scroller)
{
	_ENTER;
	scroller_info_s *scroller_info = NULL;
	Eina_List *page_list = NULL;

	retv_if(!scroller, NULL);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	retv_if(!scroller_info, NULL);

	page_list = elm_box_children_get(scroller_info->box);
	retv_if(!page_list, NULL);

	Evas_Object *start_rect = evas_object_data_get(scroller, PRIVATE_DATA_KEY_START_EDGE_RECT);
	if(start_rect) {
		page_list = eina_list_remove(page_list, start_rect);
	}

	Evas_Object *end_rect = evas_object_data_get(scroller, PRIVATE_DATA_KEY_END_EDGE_RECT);
	if(end_rect) {
		page_list = eina_list_remove(page_list, end_rect);
	}

	return eina_list_clone(page_list);
}


void home_custom_scroller_edit(Evas_Object *scroller)
{
#if 0
	scroller_info_s *scroller_info = NULL;

	_D("home_custom_scroller_edit");

	ret_if(!scroller);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	elm_object_signal_emit(scroller_info->box_layout, "show,zoom", "scroller");
#endif
}


void home_custom_scroller_unedit(Evas_Object *scroller)
{
#if 0
	scroller_info_s *scroller_info = NULL;

	_D("home_custom_scroller_unedit");

	ret_if(!scroller);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	elm_object_signal_emit(scroller_info->box_layout, "reset,zoom", "scroller");
#endif
	Evas_Object *page = home_custom_scroller_get_current_page(scroller);
	ret_if(!page);

	Evas_Object *index = evas_object_data_get(scroller, PRIVATE_DATA_KEY_INDEX);
	if(index) home_custom_index_bring_in_page(index, page);
}


void home_custom_scroller_freeze(Evas_Object *scroller, Eina_Bool status)
{
	_ENTER;
	scroller_info_s *scroller_info = NULL;

	ret_if(!scroller);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	if(status) {
		elm_object_scroll_freeze_push(scroller_info->box);
	}
	else {
		while (elm_object_scroll_freeze_get(scroller_info->box))
			elm_object_scroll_freeze_pop(scroller_info->box);
	}
}


Eina_Bool home_custom_scroller_is_freeze(Evas_Object *scroller)
{
	_ENTER;
	scroller_info_s *scroller_info = NULL;

	retv_if(!scroller, EINA_FALSE);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	retv_if(!scroller_info, EINA_FALSE);

	return elm_object_scroll_freeze_get(scroller_info->box);
}


void home_custom_scroller_page_freeze(Evas_Object *scroller, Evas_Object *page, Eina_Bool status)
{
	_ENTER;
}


Eina_Bool home_custom_scroller_is_page_freeze(Evas_Object *scroller, Evas_Object *page)
{
	_ENTER;
	return EINA_FALSE;
}


static Eina_Bool _bring_in_timer_cb(void *data)
{
	_ENTER;
	int i = 0;
	Evas_Object *scroller = data;

	i = (int) evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_BRING_IN_ITH);
	_D("bring in to index:[%d]", i);

	Eina_Bool bStatus = home_custom_scroller_is_freeze(scroller);
	if(bStatus) {
		home_custom_scroller_freeze(scroller, EINA_FALSE);
	}

	elm_scroller_page_bring_in(scroller, i, 0);

	home_custom_scroller_freeze(scroller, bStatus);

	evas_object_data_del(scroller, PRIVATE_DATA_KEY_SCROLLER_BRING_IN_TIMER);

	Evas_Object *page = home_custom_scroller_get_current_page(scroller);
	retv_if(!page, ECORE_CALLBACK_CANCEL);

	Evas_Object *index = evas_object_data_get(scroller, PRIVATE_DATA_KEY_INDEX);
	if(index) home_custom_index_bring_in_page(index, page);

	return ECORE_CALLBACK_CANCEL;
}


void home_custom_scroller_bring_in(Evas_Object *scroller, int page_no)
{
	_ENTER;
	Ecore_Timer *timer = NULL;

	ret_if(!scroller);

	/* 1. Remove the old timer */
	timer = evas_object_data_del(scroller, PRIVATE_DATA_KEY_SCROLLER_BRING_IN_TIMER);
	if (timer) ecore_timer_del(timer);

	/* 2. Append the new timer */
	timer = ecore_timer_add(0.01f, _bring_in_timer_cb, scroller);
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_BRING_IN_TIMER, timer);
	evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_BRING_IN_ITH, (void *)page_no);
}


void home_custom_scroller_bring_in_page(Evas_Object *scroller, Evas_Object *page)
{
	_ENTER;
	Evas_Object *tmp = NULL;
	Eina_List *page_list = NULL;
	Ecore_Timer *timer = NULL;
	scroller_info_s *scroller_info = NULL;
	const Eina_List *l;
	const Eina_List *n;
	int i = -1;

	ret_if(!scroller);
	ret_if(!page);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	ret_if(!scroller_info);

	page_list = elm_box_children_get(scroller_info->box);
	ret_if(!page_list);

	EINA_LIST_FOREACH_SAFE(page_list, l, n, tmp) {
		i++;
		break_if(!tmp);
		if (tmp != page) continue;

		/* 1. Remove the old timer */
		timer = evas_object_data_del(scroller, PRIVATE_DATA_KEY_SCROLLER_BRING_IN_TIMER);
		if (timer) ecore_timer_del(timer);

		/* 2. Append the new timer */
		timer = ecore_timer_add(0.01f, _bring_in_timer_cb, scroller);
		evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_BRING_IN_TIMER, timer);
		evas_object_data_set(scroller, PRIVATE_DATA_KEY_SCROLLER_BRING_IN_ITH, (void *) i);

		break;
	}
	eina_list_free(page_list);
}


void home_custom_scroller_bring_in_direction(Evas_Object *scroller, Home_Custom_Sc_Direction direction)
{
	_ENTER;
	int hPageNo = 0;
	int vPageNo = 0;
	int nPageCount = 0;

	elm_scroller_current_page_get(scroller, &hPageNo, &vPageNo);

	nPageCount = home_custom_scroller_get_page_count(scroller);

	switch(direction)
	{
		case HOME_CUSTOM_SC_DIRECTION_LEFT:
			if(hPageNo > 0) {
				home_custom_scroller_bring_in(scroller, hPageNo - 1);
			}
			else {
				_W("pageNo:[%d,%d]", hPageNo, vPageNo);
			}
			break;
		case HOME_CUSTOM_SC_DIRECTION_RIGHT:
			if(hPageNo < nPageCount) {
				home_custom_scroller_bring_in(scroller, hPageNo + 1);
			}
			else {
				_W("Total Page Count:[%d], pageNo:[%d,%d]", nPageCount, hPageNo, vPageNo);
			}
			break;
		default:
			_E("Wrong direction: [%d]", direction);
			break;
	}
}


Eina_Bool home_custom_scroller_is_scrolling(Evas_Object *scroller)
{
	_ENTER;
	retv_if(!scroller, EINA_FALSE);

	return evas_object_data_get(scroller, PRIVATE_DATA_KEY_IS_SCROLLING) ? EINA_TRUE:EINA_FALSE;
}


void home_custom_scroller_region_show(Evas_Object *scroller, int hPageNo, int vPageNo)
{
	_ENTER;
	elm_scroller_page_show(scroller, hPageNo, vPageNo);

	evas_object_data_del(scroller, PRIVATE_DATA_KEY_IS_SCROLLING);

	Evas_Object *page = home_custom_scroller_get_current_page(scroller);
	ret_if(!page);

	Evas_Object *index = evas_object_data_get(scroller, PRIVATE_DATA_KEY_INDEX);
	if(index) home_custom_index_bring_in_page(index, page);
}


Evas_Object *home_custom_scroller_get_first_page(Evas_Object *scroller)
{
	_ENTER;
	Eina_List *page_list = NULL;
	scroller_info_s *scroller_info = NULL;
	Evas_Object *page = NULL;

	retv_if(!scroller, NULL);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	retv_if(!scroller_info, NULL);

	page_list = elm_box_children_get(scroller_info->box);
	retv_if(!page_list, NULL);

	Evas_Object *start_rect = evas_object_data_get(scroller, PRIVATE_DATA_KEY_START_EDGE_RECT);
	if(start_rect) {
		page_list = eina_list_remove(page_list, start_rect);
	}

	Evas_Object *end_rect = evas_object_data_get(scroller, PRIVATE_DATA_KEY_END_EDGE_RECT);
	if(end_rect) {
		page_list = eina_list_remove(page_list, end_rect);
	}

	page = eina_list_nth(page_list, 0);

	eina_list_free(page_list);

	return page;
}


Evas_Object *home_custom_scroller_get_last_page(Evas_Object *scroller)
{
	_ENTER;
	int count = 0;
	Eina_List *page_list = NULL;
	scroller_info_s *scroller_info = NULL;
	Evas_Object *page = NULL;

	retv_if(!scroller, NULL);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	retv_if(!scroller_info, NULL);

	page_list = elm_box_children_get(scroller_info->box);
	retv_if(!page_list, NULL);

	Evas_Object *start_rect = evas_object_data_get(scroller, PRIVATE_DATA_KEY_START_EDGE_RECT);
	if(start_rect) {
		page_list = eina_list_remove(page_list, start_rect);
	}

	Evas_Object *end_rect = evas_object_data_get(scroller, PRIVATE_DATA_KEY_END_EDGE_RECT);
	if(end_rect) {
		page_list = eina_list_remove(page_list, end_rect);
	}

	count = eina_list_count(page_list);

	page = eina_list_nth(page_list, count-1);

	eina_list_free(page_list);

	return page;
}


Evas_Object *home_custom_scroller_get_current_page(Evas_Object *scroller)
{
	_ENTER;
	int hPageNo = 0;
	int vPageNo = 0;

	elm_scroller_current_page_get(scroller, &hPageNo, &vPageNo);
	_D("hpageno:%d, vpageno:%d",hPageNo,vPageNo);

	return home_custom_scroller_get_page(scroller, hPageNo);
}


int home_custom_scroller_get_current_page_no(Evas_Object *scroller)
{
	_ENTER;
	int hPageNo = 0;
	int vPageNo = 0;

	elm_scroller_current_page_get(scroller, &hPageNo, &vPageNo);

	return hPageNo;
}



Evas_Object *home_custom_scroller_get_page(Evas_Object *scroller, int index)
{
	_ENTER;
	Eina_List *page_list = NULL;
	scroller_info_s *scroller_info = NULL;
	Evas_Object *page = NULL;

	retv_if(!scroller, NULL);

	scroller_info = evas_object_data_get(scroller, PRIVATE_DATA_KEY_SCROLLER_INFO);
	retv_if(!scroller_info, NULL);

	page_list = elm_box_children_get(scroller_info->box);
	retv_if(!page_list, NULL);

	Evas_Object *start_rect = evas_object_data_get(scroller, PRIVATE_DATA_KEY_START_EDGE_RECT);
	if(start_rect) {
		page_list = eina_list_remove(page_list, start_rect);
	}

	Evas_Object *end_rect = evas_object_data_get(scroller, PRIVATE_DATA_KEY_END_EDGE_RECT);
	if(end_rect) {
		page_list = eina_list_remove(page_list, end_rect);
	}

	page = eina_list_nth(page_list, index);

	eina_list_free(page_list);

	return page;
}


Eina_Bool home_custom_scroller_is_edge(Evas_Object *page)
{
	_ENTER;
	retv_if(!page, EINA_FALSE);

	return evas_object_data_get(page, PRIVATE_DATA_KEY_EDGE_RECT) ? EINA_TRUE:EINA_FALSE;
}

/* scroller index */
Evas_Object *home_custom_scroller_index_add(Evas_Object *parent, Evas_Object *scroller)
{
	_ENTER;
	retv_if(!parent, NULL);
	retv_if(!scroller, NULL);

	Evas_Object *index = home_custom_index_create(parent, scroller);
	retv_if(!index, NULL);

	evas_object_data_set(scroller, PRIVATE_DATA_KEY_INDEX, index);

	home_custom_index_update(index, scroller, INDEX_BRING_IN_AFTER);

	return index;
}

void home_custom_scroller_index_update(Evas_Object *scroller)
{
	_ENTER;
	ret_if(!scroller);

	Evas_Object *index = evas_object_data_get(scroller, PRIVATE_DATA_KEY_INDEX);
	if(index) home_custom_index_update(index, scroller, INDEX_BRING_IN_AFTER);
}

// End of this file


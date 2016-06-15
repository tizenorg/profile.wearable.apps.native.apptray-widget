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
#include "app-widget-app.h"

#undef TIZEN_SDK

#include <app.h>
#include <Evas.h>
#include <Elementary.h>
#include <widget_service.h>
#include <widget_errno.h>
#include <feedback.h>
#ifdef TIZEN_SDK
#include <package_manager.h>
#else
#include <pkgmgr-info.h>
#include <package-manager.h>
#endif
#include <bundle.h>
#include "key.h"
#include "util.h"
#include "common/home_custom_scroller.h"
#include "item_info.h"
#include "elm_layout_legacy.h"

#define IDS_IDLE_BODY_APPS "apps"
#define EDJE_FILE "edje/add_to_shortcut.edj"

#define SCHEDULE_ICON_ARABIC "res/schedule_arabic_%s.png"
#define LANGUAGE_ARABIC "ar"

#define  PACKAGE_MANAGER_PKGINFO_PROP_NODISPALY  "PMINFO_PKGINFO_PROP_PACKAGE_NODISPLAY_SETTING"
#define APP_WIDGET_PKGID "org.tizen.app-widget"


#define DEFAULT_APP_ORDER "org.tizen.watch-setting empty empty empty"
#define APPS_PKG "org.tizen.appptray-widget-app"

#define APP_WIDGET_CONTENT_KEY "org.tizen.apptray-widget-content"

typedef struct appdata {
	Evas_Object *edit_win;
	Evas_Object *select_win;
	Ecore_X_Window xwin;
	Evas_Object *select_layout;
	Evas_Object *edit_layout;
	Evas_Object *scroller;
	Evas_Object *title;
	char *widget_name;
	char *index;
	Ecore_Event_Handler *press_handler;
	Ecore_Event_Handler *release_handler;
	Eina_List *app_list;
	Eina_List *item_list;
	char *appid_list[5];
	char *applabel_list[5];
}appdata_s;

char *widget_id = NULL;
int slot_index = 0;
int pressed_index = 0;
int dst_index = 0;
Evas_Object *transit_obj = NULL;
Eina_Bool hide_flag = EINA_FALSE;
Eina_Bool longpress_flag = EINA_FALSE;
Eina_Bool launch_flag = EINA_TRUE;
Eina_Bool transit_go = EINA_FALSE;
Eina_List *font_theme;
Elm_Theme *theme;
Ecore_Timer *longpress_timer;
int empty_count = 0;

static appdata_s *g_info = NULL;

static appdata_s *_get_info(void){
	return g_info;
}




static void _terminate_add_to_shortcut(void);
Evas_Object *_set_app_slot(const char *appid, int pos);
item_info_s *apps_apps_info_create(const char *appid);
item_info_s *apps_recent_info_create(const char *appid);
static void _move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);


void updateContent()
{
	_ENTER;
	char content[255] = {0};
	if(g_info)
	{
	snprintf(content, sizeof(content)-1, "%s %s %s %s", g_info->appid_list[0], g_info->appid_list[1], g_info->appid_list[2], g_info->appid_list[3]);

	int ret = preference_set_string(APP_WIDGET_CONTENT_KEY, content);
	_D("content: %s updated to preference file: ret:%d",content,ret);

	}
	else
	{
		_E("g_info is NULL");
	}
}

char *_get_date(void)
{

	struct tm st;
	time_t tt = time(NULL);
	localtime_r(&tt, &st);

	char str_date[5] = {0,};
	char *date = NULL;

	snprintf(str_date, sizeof(str_date), "%d", st.tm_mday);

	date = strdup(str_date);

	return date;

}

static void _init_theme(void)
{
	theme = elm_theme_new();
}

static void _fini_theme(void)
{
	elm_theme_free(theme);
	theme = NULL;
	if (font_theme) {
		font_theme = NULL;
	}
}

static Eina_Bool _key_release_cb(void *data, int type, void *event)
{
	Evas_Event_Key_Up *ev = event;
	retv_if(NULL == ev, ECORE_CALLBACK_PASS_ON);

	_D("%s", ev->keyname);

	if (!strcmp(ev->keyname, /*KEY_BACK*/"XF86Back") || !strcmp(ev->keyname, /*KEY_POWER*/"XF86PowerOff")) {
		_D("back or home key cb");
		if(evas_object_visible_get(g_info->select_win) == EINA_TRUE){
			evas_object_hide(g_info->select_win);
			evas_object_show(g_info->edit_win);
		}
		else{
			_terminate_add_to_shortcut();
		}
	}


	return ECORE_CALLBACK_PASS_ON;
}

static void _terminate_add_to_shortcut(void){
	_D("");

	appdata_s *info = _get_info();
	int i = 0;

	if(empty_count == 4){
		int ret = 0;
		bundle *b = NULL;
		b = bundle_create();
		if(!b){
			_E("failed to create bundle");
			return;
		}

		bundle_add_str(b, "test", "delete");
		updateContent();
		//ret = widget_service_trigger_update(APP-WIDGET-PKGID, widget_id, b, 1);
		ret = widget_service_trigger_update(APP_WIDGET_PKGID, NULL, b, 1);
		if(WIDGET_ERROR_NONE != ret){
			_E("app-widget widget trigger failed %d", ret);
		}
		if(b){
			free(b);
		}
	}
	else{
		for(i = 0; i < 4 ; i++){
			_D("%s", g_info->appid_list[i]);
		}

		int ret = 0;
		bundle *b = NULL;
		b = bundle_create();
		if(!b){
			_E("failed to create bundle");
			return;
		}
		char content[255] = {0};
		snprintf(content, sizeof(content)-1, "%s %s %s %s", g_info->appid_list[0], g_info->appid_list[1], g_info->appid_list[2], g_info->appid_list[3]);
		bundle_add_str(b, "test", content);
		_D("content : %s", content);
		//ret = widget_service_trigger_update(APP_WIDGET_PKGID, widget_id, b, 1);
		updateContent();
		ret = widget_service_trigger_update(APP_WIDGET_PKGID, NULL, b, 1);

		if(WIDGET_ERROR_NONE != ret){
			_E("app widget widget trigger failed %d", ret);
		}
		if(b){
			free(b);
		}
	}
	evas_object_del(info->edit_win);
	info->edit_win = NULL;

	evas_object_del(info->select_win);
	info->select_win = NULL;


	if (g_info->release_handler) {
		ecore_event_handler_del(g_info->release_handler);
	}

	elm_exit();
}

static void _create_edit_win(appdata_s *info, const char *name, const char *title)
{
	info->edit_win = elm_win_add(NULL, name, ELM_WIN_BASIC);
	ret_if(!info->edit_win);
	evas_object_color_set(info->edit_win, 0, 0, 0, 0);
	evas_object_resize(info->edit_win, 360, 360);

	elm_win_title_set(info->edit_win, title);
	elm_win_borderless_set(info->edit_win, EINA_TRUE);

	_D("create window success");
	return;
}

static void _create_select_win(appdata_s *info, const char *name, const char *title)
{
	info->select_win = elm_win_add(NULL, name, ELM_WIN_BASIC);
	ret_if(!info->select_win);

	evas_object_color_set(info->select_win, 0, 0, 0, 0);
	evas_object_resize(info->select_win, 360, 360);

	elm_win_title_set(info->select_win, title);
	elm_win_borderless_set(info->select_win, EINA_TRUE);

	_D("create window success");
	return;
}

static Eina_Bool _longpress_timer_cb(void *data){
	Evas_Object *item = NULL;
	char index[10] = {0};
	snprintf(index, sizeof(index)-1, "index%d", pressed_index);
	item = elm_object_part_content_unset(g_info->edit_layout, index);
	elm_object_signal_emit(item, "hide", "slot");
	longpress_flag = EINA_TRUE;
	launch_flag = EINA_FALSE;
	feedback_play_type(FEEDBACK_TYPE_VIBRATION, FEEDBACK_PATTERN_HOLD);
	_D("item : %p", item);
	return ECORE_CALLBACK_CANCEL;
}

static void _render_post_cb(void *data, Evas *e , void *event_info){
	_D("render finished");
	Evas_Object *slot = (Evas_Object *)data;

	evas_event_callback_del(e, EVAS_CALLBACK_RENDER_POST, _render_post_cb);
	elm_object_signal_emit(slot, "show_vi", "slot");
}

static void _mouse_clicked_cb(void *data, Evas_Object *o, const char *emission, const char *source){

	_D("icon clicked");

	item_info_s *item_info = evas_object_data_get((Evas_Object *)data, "p_i_n");

	if(!item_info)
		return;

	g_info->appid_list[slot_index] = strdup(item_info->appid);

	Evas_Object *slot = _set_app_slot(item_info->appid, slot_index);
	elm_object_signal_emit(slot, "hide_icon", "slot");
	empty_count--;
	evas_object_hide(g_info->select_win);
	evas_object_show(g_info->edit_win);
	evas_event_callback_add(evas_object_evas_get(g_info->edit_layout), EVAS_CALLBACK_RENDER_POST, _render_post_cb, (void *)slot);
	updateContent();
}

static void _mouse_down_cb(void *data, Evas_Object *o, const char *emission, const char *source){
	_D("icon mouse down");
	Evas_Object *icon = elm_object_part_content_get((Evas_Object *)data, "icon");
	evas_object_color_set(icon, 255, 255, 255, 127);
}

static void _mouse_up_cb(void *data, Evas_Object *o, const char *emission, const char *source){
	_D("icon mouse up");
	Evas_Object *icon = elm_object_part_content_get((Evas_Object *)data, "icon");
	evas_object_color_set(icon, 255, 255, 255, 255);
}

static void _plus_mouse_clicked_cb(void *data, Evas_Object *o, const char *emission, const char *source){
	_D("plus clicked");
	if(launch_flag == EINA_FALSE){
		launch_flag = EINA_TRUE;
		return;
	}
	slot_index = dst_index - 1;
	_D("index:%d", slot_index);
	evas_object_show(g_info->select_win);
}

static void _del_mouse_down_cb(void *data, Evas_Object *o, const char *emission, const char *source){
	_D("del mouse down");
	elm_object_signal_emit((Evas_Object *)data, "pressed", "slot");
}

static void _del_mouse_up_cb(void *data, Evas_Object *o, const char *emission, const char *source){
	_D("del mouse up");
	elm_object_signal_emit((Evas_Object *)data, "released", "slot");
}

static void _del_mouse_clicked_cb(void *data, Evas_Object *o, const char *emission, const char *source){
	_D("del mouse clicked");

	char index[10] = {0};
	Evas_Object *slot = NULL;
	int ret = 0;
	Evas_Coord x,y,w,h;
	int pos = 0;

	if(transit_go){
		_E("transit is now processing");
		return;
	}

	evas_object_geometry_get((Evas_Object *)data, &x, &y, &w, &h);
	_D("X:%d,y:%d,w:%d,h:%d",x,y,w,h);
	if((x == 130 || x==127) &&( y == 21||y==18))
		pos = 1;
	else if((x == 239||x==236) &&( y == 130||y==127))
		pos = 2;
	else if((x == 130||x==127) && (y == 219||y==216))
		pos = 3;
	else if((x == 21||x==18) &&( y == 130||y==127))
		pos = 4;
	else{
		_E("can't reach here.");
		return;
	}
	snprintf(index, sizeof(index)-1, "index%d", pos);
	_D("%s", index);
	slot_index = pos - 1;
	slot = elm_object_part_content_unset(g_info->edit_layout, index);
	if(slot){
		evas_object_del(slot);
		slot = NULL;
	}


	slot = elm_layout_add(g_info->edit_layout);
	char full_path[PATH_MAX] = { 0, };
	_get_resource(EDJE_FILE, full_path, sizeof(full_path));
	_D("full_path:%s",full_path);
	ret = elm_layout_file_set(slot, full_path, "empty_slot");
	if(ret == EINA_FALSE){
		_E("failed to set empty slot");
		return;
	}
	evas_object_size_hint_weight_set(slot, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_resize(slot, 100, 100);
	evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_DOWN, _down_cb, slot);
	evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_UP, _up_cb, slot);
	evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_MOVE, _move_cb, &slot_index);
	elm_object_signal_callback_add(slot, "mouse_clicked", "*", _plus_mouse_clicked_cb, &slot_index);
	elm_object_part_content_set(g_info->edit_layout, index, slot);
	evas_object_show(slot);
	g_info->appid_list[slot_index] = strdup("empty");
	empty_count++;
	updateContent();

}

static void _down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_ENTER;
	Evas_Object *icon = NULL;
	Evas_Coord x,y,w,h;
	evas_object_geometry_get((Evas_Object *)data, &x, &y, &w, &h);
	_D("x:%d, y:%d, w:%d, h:%d",x,y,w,h);
	if(transit_go){
		_E("transit is now processing");
		return;
	}
	if((x == 130 || x==127) && (y == 21|| y==18)){
		pressed_index = 1;
		dst_index = 1;
	}
	else if((x == 239||x==236) && (y == 130||y==127)){
		pressed_index = 2;
		dst_index = 2;
	}
	else if((x == 130||x==127) && (y == 219||y==216)){
		pressed_index = 3;
		dst_index = 3;
	}
	else if((x == 21||x==18) && (y == 130||y==127)){
		pressed_index = 4;
		dst_index = 4;
	}
	else{
		_E("can't reach here.");
		return;
	}
	icon = elm_object_part_content_get(obj, "icon");
	evas_object_color_set(icon, 255, 255, 255, 127);
	elm_object_signal_emit(obj, "pressed", "widget_plus");
	longpress_timer = ecore_timer_add(0.5, _longpress_timer_cb, obj);
}

static void _transit_del_cb(void *data, Elm_Transit *transit){
	_D("transit end");
	char index[10] = {0};
	snprintf(index, sizeof(index)-1, "index%d", pressed_index);
	elm_object_part_content_set(g_info->edit_layout, index, transit_obj);
	pressed_index = (int)data;
	_D("pressed index %d", pressed_index);
	transit_go = EINA_FALSE;
	transit_obj = NULL;
}

static void _anim_switch_item(Evas_Object *item, int src, int dst){
	if(transit_go == EINA_TRUE) return;
	transit_go = EINA_TRUE;
	Elm_Transit *transit = elm_transit_add();
	elm_transit_object_add(transit, item);
	transit_obj = item;
	dst_index = src;
	elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_BEZIER_CURVE);
	double v[] = {0.25, 0.46, 0.45, 1.0};
	elm_transit_tween_mode_factor_n_set(transit, 4, v);
	elm_transit_objects_final_state_keep_set(transit, EINA_TRUE);

	if(dst == 4){
		if(src == 1)
			elm_transit_effect_translation_add(transit, 0, 0, -109, 109);
		else if(src == 2)
			elm_transit_effect_translation_add(transit, 0, 0, -218, 0);
		else if(src == 3)
			elm_transit_effect_translation_add(transit, 0, 0, -109, -89);
	}
	else if(dst == 3){
		if(src == 1)
			elm_transit_effect_translation_add(transit, 0, 0, 0, 198);
		else if(src == 2)
			elm_transit_effect_translation_add(transit, 0, 0, -109, 89);
		else if(src == 4)
			elm_transit_effect_translation_add(transit, 0, 0, 109, 89);

	}
	else if(dst == 2){
		if(src == 1)
			elm_transit_effect_translation_add(transit, 0, 0, 109, 109);
		else if(src == 3)
			elm_transit_effect_translation_add(transit, 0, 0, 109, -89);
		else if(src == 4)
			elm_transit_effect_translation_add(transit, 0, 0, 218, 0);
	}
	else if(dst == 1){
		if(src == 2)
			elm_transit_effect_translation_add(transit, 0, 0, -109, -109);
		else if(src == 3)
			elm_transit_effect_translation_add(transit, 0, 0, 0, -198);
		else if(src == 4)
			elm_transit_effect_translation_add(transit, 0, 0, 109, -109);
	}
	else{
		_E("can't reach here");
	}
	elm_transit_duration_set(transit, 0.3);
	elm_transit_del_cb_set(transit, _transit_del_cb, (void *)src);
	elm_transit_go(transit);
}

static void _move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Move *ev = event_info;
	int cur_x, cur_y;
	char *tmp = NULL;
	cur_x = ev->cur.output.x;
	cur_y = ev->cur.output.y;
	if(longpress_flag){
		evas_object_move(obj, cur_x-80, cur_y-100);
		if(cur_x > 130 && cur_x < 230 && cur_y > 30 && cur_y < 130){
			if(pressed_index != 1 && !transit_go){
				_D("index 1 area");
				Evas_Object *item = elm_object_part_content_unset(g_info->edit_layout, "index1");
				_anim_switch_item(item, 1, pressed_index);
				tmp = strdup(g_info->appid_list[pressed_index-1]);
				g_info->appid_list[pressed_index-1] = strdup(g_info->appid_list[0]);
				g_info->appid_list[0] = strdup(tmp);
				if(tmp) free(tmp);
			}
		}
		else if(cur_x > 230 && cur_x < 330 && cur_y > 130 && cur_y < 230){
			if(pressed_index != 2 && !transit_go){
				_D("index 2 area");
				Evas_Object *item = elm_object_part_content_unset(g_info->edit_layout, "index2");
				_anim_switch_item(item, 2, pressed_index);
				tmp = strdup(g_info->appid_list[pressed_index-1]);
				g_info->appid_list[pressed_index-1] = strdup(g_info->appid_list[1]);
				g_info->appid_list[1] = strdup(tmp);
				if(tmp) free(tmp);
			}
		}
		else if(cur_x > 130 && cur_x < 230 && cur_y > 219 && cur_y < 319){
			if(pressed_index != 3 && !transit_go){
				_D("index 3 area");
				Evas_Object *item = elm_object_part_content_unset(g_info->edit_layout, "index3");
				_anim_switch_item(item, 3, pressed_index);
				tmp = strdup(g_info->appid_list[pressed_index-1]);
				g_info->appid_list[pressed_index-1] = strdup(g_info->appid_list[2]);
				g_info->appid_list[2] = strdup(tmp);
				if(tmp) free(tmp);
			}
		}
		else if(cur_x > 30 && cur_x < 130 && cur_y > 130 && cur_y < 230){
			if(pressed_index != 4 && !transit_go){
				_D("index 4 area");
				Evas_Object *item = elm_object_part_content_unset(g_info->edit_layout, "index4");
				_anim_switch_item(item, 4, pressed_index);
				tmp = strdup(g_info->appid_list[pressed_index-1]);
				g_info->appid_list[pressed_index-1] = strdup(g_info->appid_list[3]);
				g_info->appid_list[3] = strdup(tmp);
				if(tmp) free(tmp);
			}
		}
	}
}

static void _up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_ENTER;
	Evas_Event_Mouse_Up *ev = event_info;
	Evas_Object *icon = NULL;
	int cur_x, cur_y;
	cur_x = ev->output.x;
	cur_y = ev->output.y;
	_D("cur_x:%d,cur_y:%d",cur_x,cur_y);
	char index[10] = {0};
	snprintf(index, sizeof(index)-1, "index%d", dst_index);
	if(longpress_timer){
		ecore_timer_del(longpress_timer);
		longpress_timer = NULL;
	}
	if(longpress_flag == EINA_TRUE){
		longpress_flag = EINA_FALSE;
		elm_object_part_content_set(g_info->edit_layout, index, obj);
		launch_flag = EINA_FALSE;
		_D("dst index %d", dst_index);
	}
	icon = elm_object_part_content_get(obj, "icon");
	evas_object_color_set(icon, 255, 255, 255, 255);
	elm_object_signal_emit(obj, "released", "widget_plus");
	elm_object_signal_emit(obj, "show", "slot");
	updateContent();
}

static Evas_Object *_create_item(Evas_Object *scroller, item_info_s *item_info)
{
	_ENTER;
	Evas_Object *page = NULL;
	Evas_Object *page_rect = NULL;
	Evas_Object *icon = NULL;
	page = elm_layout_add(scroller);
	retv_if(!page, NULL);
	char full_path[PATH_MAX] = { 0, };
	_get_resource(EDJE_FILE, full_path, sizeof(full_path));
	_D("full_path:%s",full_path);
	int ret = elm_layout_file_set(page, full_path, "item");
	_D("ret:%d",ret);

	evas_object_size_hint_weight_set(page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(page, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(page);

	page_rect = evas_object_rectangle_add(evas_object_evas_get(page));
	evas_object_size_hint_min_set(page_rect, (160), (360));
	evas_object_color_set(page_rect, 0, 0, 0, 0);
	evas_object_show(page_rect);
	elm_object_part_content_set(page, "bg", page_rect);

	icon = evas_object_image_add(evas_object_evas_get(page));
	evas_object_repeat_events_set(icon, EINA_TRUE);
	evas_object_image_file_set(icon, item_info->icon, NULL);
	evas_object_image_filled_set(icon, EINA_TRUE);
	evas_object_show(icon);
	elm_object_part_content_set(page, "icon", icon);
	elm_object_part_text_set(page, "name", item_info->name);

	elm_object_signal_callback_add(page, "mouse_clicked", "*", _mouse_clicked_cb, page);
	elm_object_signal_callback_add(page, "mouse_down", "*", _mouse_down_cb, page);
	elm_object_signal_callback_add(page, "mouse_up", "*", _mouse_up_cb, page);

	home_custom_scroller_page_append(scroller, page);
	evas_object_data_set(page, "p_i_n", item_info);
	_EXIT;
	return page;
}

void app_shortcut_show_name(Evas_Object *page){

	appdata_s *info = _get_info();
	item_info_s *item_info = NULL;
	item_info = evas_object_data_get(page, "p_i_n");

	page = home_custom_scroller_get_current_page(info->scroller);

	if(item_info){
		_D("get item info from the page. %s", item_info->name);
		elm_object_part_text_set(info->select_layout, "name", item_info->name);
		elm_object_signal_emit(info->select_layout, "show_name", "name");
		edje_object_message_signal_process(info->select_layout);
		hide_flag = EINA_FALSE;
	}

}

void app_shortcut_hide_name(){

	appdata_s *info = _get_info();
	if(hide_flag == EINA_FALSE){
		_D("hide app name");
		elm_object_signal_emit(info->select_layout, "hide_name", "name");
		hide_flag = EINA_TRUE;
	}

}
#ifdef TIZEN_SDK
Evas_Object *_set_app_slot(const char *appid, int pos){
	package_info_h appinfo_h = NULL;
	_D("%s", appid);
	char *icon_path_tmp = NULL;
	int ret = 0;
	Evas_Object *slot = NULL;
	Evas_Object *icon = NULL;
	char index[10] = {0};
	snprintf(index, sizeof(index)-1, "index%d", pos+1);
	char *label = NULL;

	if(!strcmp(appid, "empty")){
		slot = elm_layout_add(g_info->edit_layout);
		char full_path[PATH_MAX] = { 0, };
		_get_resource(EDJE_FILE, full_path, sizeof(full_path));
		_D("full_path:%s",full_path);
		ret = elm_layout_file_set(slot, full_path, "empty_slot");
		if(ret == EINA_FALSE){
			_E("failed to set empty slot");
			return NULL;
		}
		evas_object_size_hint_weight_set(slot, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_resize(slot, 100, 100);

		evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_DOWN, _down_cb, slot);
		evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_MOVE, _move_cb, &pos);
		evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_UP, _up_cb, slot);
		elm_object_signal_callback_add(slot, "mouse_clicked", "*", _plus_mouse_clicked_cb, &pos);

		elm_object_part_content_set(g_info->edit_layout, index, slot);
		evas_object_show(slot);
		g_info->appid_list[pos] = strdup(appid);
		empty_count++;
	}
	else{
		package_manager_get_package_info(appid, &appinfo_h);

			if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_icon(appinfo_h, &icon_path_tmp)){
				_E("get icon path failed");
			}
			if (icon_path_tmp) {
				if (strlen(icon_path_tmp) > 0) {
				} else {
					icon_path_tmp = strdup(DEFAULT_ICON);
				}
			} else {
				icon_path_tmp = strdup(DEFAULT_ICON);
			}


		if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_label(appinfo_h, &label)){
			_E("get label failed");
			g_info->applabel_list[pos] = strdup("");
		}
		else{
			g_info->applabel_list[pos] = strdup(label);
		}
		_D("icon path in object info %s", icon_path_tmp);

		slot = elm_layout_add(g_info->edit_layout);
		char full_path[PATH_MAX] = { 0, };
		_get_resource(EDJE_FILE, full_path, sizeof(full_path));
		_D("full_path:%s",full_path);
		ret = elm_layout_file_set(slot, full_path, "icon_slot");
		if(ret == EINA_FALSE){
			_E("failed to set empty slot");
			if(icon_path_tmp)
				free(icon_path_tmp);
			return NULL;
		}

		evas_object_size_hint_weight_set(slot, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_resize(slot, 100, 100);
		elm_object_part_content_set(g_info->edit_layout, index, slot);
		evas_object_show(slot);

		icon = evas_object_image_add(evas_object_evas_get(slot));
		evas_object_repeat_events_set(icon, EINA_TRUE);
		evas_object_image_file_set(icon, icon_path_tmp, NULL);
		evas_object_image_filled_set(icon, EINA_TRUE);
		evas_object_show(icon);

		evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_DOWN, _down_cb, slot);
		evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_MOVE, _move_cb, slot);
		evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_UP, _up_cb, slot);

		elm_object_signal_callback_add(slot, "del_mouse_down", "*", _del_mouse_down_cb, slot);
		elm_object_signal_callback_add(slot, "del_mouse_up", "*", _del_mouse_up_cb, slot);
		elm_object_signal_callback_add(slot, "del_mouse_clicked", "*", _del_mouse_clicked_cb, slot);
		elm_object_signal_callback_add(slot, "del_mouse_clicked_icon", "*", _del_mouse_clicked_cb, slot);

		elm_object_part_content_set(slot, "icon", icon);
		elm_object_part_text_set(slot, "name", label);
		evas_object_show(icon);
		g_info->appid_list[pos] = strdup(appid);

		if (appinfo_h) package_info_destroy(appinfo_h);
	}
	return slot;
}
#else

Evas_Object *_set_app_slot(const char *appid, int pos){
	_ENTER;
	pkgmgrinfo_appinfo_h appinfo_h = NULL;
	_D("%s", appid);
	char *icon_path_tmp = NULL;
	int ret = 0;
	Evas_Object *slot = NULL;
	Evas_Object *icon = NULL;
	char index[10] = {0};
	snprintf(index, sizeof(index)-1, "index%d", pos+1);
	char *label = NULL;

	if(!strcmp(appid, "empty")){
		slot = elm_layout_add(g_info->edit_layout);

		char full_path[PATH_MAX] = { 0, };
		_get_resource(EDJE_FILE, full_path, sizeof(full_path));
		_D("full_path:%s",full_path);
		ret = elm_layout_file_set(slot, full_path, "empty_slot");
		if(ret == EINA_FALSE){
			_E("failed to set empty slot");
			return NULL;
		}
		evas_object_size_hint_weight_set(slot, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_resize(slot, 100, 100);

		evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_DOWN, _down_cb, slot);
		evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_MOVE, _move_cb, pos);
		evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_UP, _up_cb, slot);
		elm_object_signal_callback_add(slot, "mouse_clicked", "*", _plus_mouse_clicked_cb, pos);

		elm_object_part_content_set(g_info->edit_layout, index, slot);
		evas_object_show(slot);
		g_info->appid_list[pos] = strdup(appid);
		empty_count++;
	}
	else{
		pkgmgrinfo_appinfo_get_appinfo(appid, &appinfo_h);
			if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_icon(appinfo_h, &icon_path_tmp)){
				_E("get icon path failed");
			}
			if (icon_path_tmp) {
				if (strlen(icon_path_tmp) > 0) {
				} else {
					icon_path_tmp = strdup(DEFAULT_ICON);
				}
			} else {
				icon_path_tmp = strdup(DEFAULT_ICON);
			}
	

		if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_label(appinfo_h, &label)){
			_E("get label failed");
			g_info->applabel_list[pos] = strdup("");
		}
		else{
			g_info->applabel_list[pos] = strdup(label);
		}
		_D("icon path in object info %s", icon_path_tmp);

		slot = elm_layout_add(g_info->edit_layout);
		char full_path[PATH_MAX] = { 0, };
		_get_resource(EDJE_FILE, full_path, sizeof(full_path));
		_D("full_path:%s",full_path);
		ret = elm_layout_file_set(slot, full_path, "icon_slot");
		if(ret == EINA_FALSE){
			_E("failed to set empty slot");
			if(icon_path_tmp)
				free(icon_path_tmp);
			return NULL;
		}

		evas_object_size_hint_weight_set(slot, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_resize(slot, 100, 100);
		elm_object_part_content_set(g_info->edit_layout, index, slot);
		evas_object_show(slot);

		icon = evas_object_image_add(evas_object_evas_get(slot));
		evas_object_repeat_events_set(icon, EINA_TRUE);
		evas_object_image_file_set(icon, icon_path_tmp, NULL);
		evas_object_image_filled_set(icon, EINA_TRUE);
		evas_object_show(icon);

		evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_DOWN, _down_cb, slot);
		evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_MOVE, _move_cb, slot);
		evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_UP, _up_cb, slot);

		elm_object_signal_callback_add(slot, "del_mouse_down", "*", _del_mouse_down_cb, slot);
		elm_object_signal_callback_add(slot, "del_mouse_up", "*", _del_mouse_up_cb, slot);
		elm_object_signal_callback_add(slot, "del_mouse_clicked", "*", _del_mouse_clicked_cb, slot);
		elm_object_signal_callback_add(slot, "del_mouse_clicked_icon", "*", _del_mouse_clicked_cb, slot);

		elm_object_part_content_set(slot, "icon", icon);
		elm_object_part_text_set(slot, "name", label);
		evas_object_show(icon);
		g_info->appid_list[pos] = strdup(appid);

		if (appinfo_h) pkgmgrinfo_appinfo_destroy_appinfo(appinfo_h);
	}
	return slot;
}
#endif
static void _create_edit_layout(appdata_s *info){
	int ret = 0;
	Evas_Object *layout = NULL;
	layout = elm_layout_add(info->edit_win);
	char full_path[PATH_MAX] = { 0, };
	_get_resource(EDJE_FILE, full_path, sizeof(full_path));
	_D("full_path:%s",full_path);
	ret = elm_layout_file_set(layout, full_path, "edit_layout");
	if(ret == EINA_FALSE){
		_E("failed to set layout");
		return;
	}
	info->edit_layout = layout;

	elm_object_signal_emit(info->edit_layout, "dark_theme", "bg");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_resize(layout, 360, 360);
	evas_object_show(layout);
}


static void _create_layout(appdata_s *info){
	Evas_Object *layout = NULL;
	int ret = 0;
	Eina_List *item_info_list = NULL;
	item_info_s *item_info = NULL;
	Evas_Object *page = NULL;

	layout = elm_layout_add(info->select_win);
	char full_path[PATH_MAX] = { 0, };
	_get_resource(EDJE_FILE, full_path, sizeof(full_path));
	_D("full_path:%s",full_path);
	ret = elm_layout_file_set(layout, full_path, "layout");
	if(ret == EINA_FALSE){
		_E("failed to set layout");
		return;
	}
	info->select_layout = layout;

	elm_object_signal_emit(info->select_layout, "dark_theme", "bg");


	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_resize(layout, 360, 360);
	evas_object_show(layout);
	_D("layout create done");
	/* scroller */
	scroller_info_s *scroller_info = home_custom_scroller_info_create();

	scroller_info->page_width = 160;
	scroller_info->page_height = 360;

	scroller_info->edge_width = 100;//76;
	scroller_info->edge_height = 360;//165;

	Evas_Object *scroller = home_custom_scroller_add(layout, scroller_info);
	if(!scroller){
		_E("scroller is NULL");
		free(scroller_info);
		return;
	}
	info->scroller = scroller;
	elm_object_part_content_set(layout, "scroller", scroller);

	
	evas_object_show(scroller);
	_D("scroller create done");
	/* get apps list */
	item_info_list = g_info->app_list;
	//todo : check why this is required
#if 0
	item_info_s *apps_item_info = NULL;
	Evas_Object *apps_item = NULL;
	g_info->item_list = NULL;
	apps_item_info = apps_item_info_create(APPS_PKG);
	if(NULL == apps_item_info){
		_E("failed to create item info");
	}
	else{
		free(apps_item_info->name);
		apps_item_info->name = strdup(IDS_IDLE_BODY_APPS);
		_D("%s,%s,%s", apps_item_info->appid, apps_item_info->icon, apps_item_info->name);
		apps_item = _create_item(scroller, apps_item_info);
		g_info->item_list = eina_list_append(g_info->item_list, apps_item);
	}
#endif
	Eina_List *l = NULL;
	Eina_List *n = NULL;
	EINA_LIST_FOREACH_SAFE(item_info_list, l, n, item_info) {
		Evas_Object *page_item = NULL;
		_D("create item for each pkg pkgid:%s",item_info->pkgid);
		page_item = _create_item(scroller, item_info);
		g_info->item_list = eina_list_append(g_info->item_list, page_item);
	}
	eina_list_free(item_info_list);

	page = home_custom_scroller_get_current_page(scroller);
	elm_object_signal_emit(page, "focus_out_effect", "item");

	Evas_Object *index = home_custom_scroller_index_add(layout, scroller);
	if(!index) return;

	elm_object_part_content_set(layout, "index", index);

	_D("create layout success");
	return;
}

void apps_item_info_destroy(item_info_s *item_info)
{
	ret_if(!item_info);

	if (item_info->pkgid) free(item_info->pkgid);
	if (item_info->appid) free(item_info->appid);
	if (item_info->name) free(item_info->name);
	if (item_info->icon) free(item_info->icon);
	if (item_info->type) free(item_info->type);
	if (item_info) free(item_info);
}

item_info_s *apps_apps_info_create(const char *appid)
{

	item_info_s *item_info = NULL;
	#ifdef TIZEN_SDK
	package_info_h appinfo_h = NULL;
	#else
	pkgmgrinfo_appinfo_h appinfo_h = NULL;
	pkgmgrinfo_pkginfo_h pkghandle = NULL;
	char *pkgid = NULL;
	int support_mode = 0;
	#endif

	char *icon = NULL;
	char *type = NULL;
	bool removable = false;


	retv_if(!appid, NULL);

	item_info = calloc(1, sizeof(item_info_s));
	if (NULL == item_info) {
		return NULL;
	}
#ifdef TIZEN_SDK
	goto_if(0 > package_manager_get_package_info(appid, &appinfo_h), ERROR);
	goto_if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_icon(appinfo_h, &icon), ERROR);
	goto_if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_type(appinfo_h, &type), ERROR);
	if (appid) {
		item_info->pkgid = strdup(appid);
		goto_if(NULL == item_info->pkgid, ERROR);
	}
#else
	goto_if(0 > pkgmgrinfo_appinfo_get_appinfo(appid, &appinfo_h), ERROR);
	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_icon(appinfo_h, &icon), ERROR);
	do {
		break_if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_pkgid(appinfo_h, &pkgid));
		break_if(NULL == pkgid);

		break_if(0 > pkgmgrinfo_pkginfo_get_pkginfo(pkgid, &pkghandle));
		break_if(NULL == pkghandle);
	} while (0);

	goto_if(PMINFO_R_OK != pkgmgrinfo_pkginfo_get_type(pkghandle, &type), ERROR);
	if (pkgid) {
		item_info->pkgid = strdup(pkgid);
		goto_if(NULL == item_info->pkgid, ERROR);
	}
#endif
	
	item_info->appid = strdup(appid);
	goto_if(NULL == item_info->appid, ERROR);

	item_info->name = strdup(_("IDS_IDLE_BODY_APPS"));

	if (type) {
		item_info->type = strdup(type);
		goto_if(NULL == item_info->type, ERROR);
		if (!strncmp(item_info->type, APP_TYPE_WGT, strlen(APP_TYPE_WGT))) {
			item_info->open_app = 1;
		} else {
			item_info->open_app = 0;
		}
	}

	if (icon) {
		if (strlen(icon) > 0) {
			item_info->icon = strdup(icon);
			goto_if(NULL == item_info->icon, ERROR);
		} else {
			item_info->icon = strdup(DEFAULT_ICON);
			goto_if(NULL == item_info->icon, ERROR);
		}
	} else {
		item_info->icon = strdup(DEFAULT_ICON);
		goto_if(NULL == item_info->icon, ERROR);
	}

	item_info->removable = removable;

	#ifdef TIZEN_SDK
		package_info_destroy(appinfo_h);
	#else
		pkgmgrinfo_appinfo_destroy_appinfo(appinfo_h);
	if (pkghandle) pkgmgrinfo_pkginfo_destroy_pkginfo(pkghandle);
	#endif

	return item_info;

ERROR:
	apps_item_info_destroy(item_info);
	#ifdef TIZEN_SDK
		package_info_destroy(appinfo_h);
	#else
		pkgmgrinfo_appinfo_destroy_appinfo(appinfo_h);
	if (pkghandle) pkgmgrinfo_pkginfo_destroy_pkginfo(pkghandle);
	#endif

	return NULL;
}

item_info_s *apps_recent_info_create(const char *appid)
{

	item_info_s *item_info = NULL;
	#ifdef TIZEN_SDK
	package_info_h appinfo_h = NULL;
	#else
	pkgmgrinfo_appinfo_h appinfo_h = NULL;
	pkgmgrinfo_pkginfo_h pkghandle = NULL;
	char *pkgid = NULL;
	int support_mode = 0;
	#endif

	char *icon = NULL;
	char *type = NULL;
	char *name = NULL;
	bool removable = false;


	retv_if(!appid, NULL);

	item_info = calloc(1, sizeof(item_info_s));
	if (NULL == item_info) {
		return NULL;
	}

	#ifdef TIZEN_SDK
	goto_if(0 > package_manager_get_package_info(appid, &appinfo_h), ERROR);
	goto_if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_label(appinfo_h, &name), ERROR);
	goto_if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_icon(appinfo_h, &icon), ERROR);
	goto_if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_label(appinfo_h, &type), ERROR);
	#else
	goto_if(0 > pkgmgrinfo_appinfo_get_appinfo(appid, &appinfo_h), ERROR);
	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_label(appinfo_h, &name), ERROR);
	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_icon(appinfo_h, &icon), ERROR);
	do {
		break_if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_pkgid(appinfo_h, &pkgid));
		break_if(NULL == pkgid);

		break_if(0 > pkgmgrinfo_pkginfo_get_pkginfo(pkgid, &pkghandle));
		break_if(NULL == pkghandle);
	} while (0);
	goto_if(PMINFO_R_OK != pkgmgrinfo_pkginfo_get_type(pkghandle, &type), ERROR);
	if (pkgid) {
		item_info->pkgid = strdup(pkgid);
		goto_if(NULL == item_info->pkgid, ERROR);
	}
	#endif
	item_info->pkgid = strdup(appid);
	item_info->appid = strdup(appid);
	goto_if(NULL == item_info->appid, ERROR);

	if(name){
		item_info->name = strdup(name);
		goto_if(NULL == item_info->name, ERROR);
	}

	if (type) {
		item_info->type = strdup(type);
		goto_if(NULL == item_info->type, ERROR);
		if (!strncmp(item_info->type, APP_TYPE_WGT, strlen(APP_TYPE_WGT))) {
			item_info->open_app = 1;
		} else {
			item_info->open_app = 0;
		}
	}

	if (icon) {
		if (strlen(icon) > 0) {
			item_info->icon = strdup(icon);
			goto_if(NULL == item_info->icon, ERROR);
		} else {
			item_info->icon = strdup(DEFAULT_ICON);
			goto_if(NULL == item_info->icon, ERROR);
		}
	} else {
		item_info->icon = strdup(DEFAULT_ICON);
		goto_if(NULL == item_info->icon, ERROR);
	}

	item_info->removable = removable;
	#ifdef TIZEN_SDK
	package_info_destroy(appinfo_h);
	#else
	pkgmgrinfo_appinfo_destroy_appinfo(appinfo_h);
	if (pkghandle) pkgmgrinfo_pkginfo_destroy_pkginfo(pkghandle);
	#endif

	return item_info;

ERROR:
	apps_item_info_destroy(item_info);
	
	#ifdef TIZEN_SDK
	if (appinfo_h) package_info_destroy(appinfo_h);
	#else
	pkgmgrinfo_appinfo_destroy_appinfo(appinfo_h);
	if (pkghandle) pkgmgrinfo_pkginfo_destroy_pkginfo(pkghandle);
	#endif
	return NULL;
}
#ifdef TIZEN_SDK
item_info_s *apps_item_info_create(const char *appid)
{
	_ENTER;
	item_info_s *item_info = NULL;
	package_info_h appinfo_h = NULL;

	char *name = NULL;
	char *icon = NULL;
	char *type = NULL;

	bool nodisplay = false;
	bool removable = false;

	retv_if(!appid, NULL);

	item_info = calloc(1, sizeof(item_info_s));
	if (NULL == item_info) {
		return NULL;
	}
	package_manager_get_package_info(appid, &appinfo_h);

	goto_if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_label(appinfo_h, &name), ERROR);
	goto_if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_icon(appinfo_h, &icon), ERROR);

	if (nodisplay) goto ERROR;


	goto_if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_type(appinfo_h, &type), ERROR);


	_D("name:%s,type:%s,icon:%s",name,type,icon);
	item_info->pkgid = strdup(appid);
	goto_if(NULL == item_info->pkgid, ERROR);
	item_info->appid = strdup(appid);
	goto_if(NULL == item_info->appid, ERROR);

	if (name) {
			item_info->name = strdup(name);
			goto_if(NULL == item_info->name, ERROR);
	}

	if (type) {
		item_info->type = strdup(type);
		goto_if(NULL == item_info->type, ERROR);
		if (!strncmp(item_info->type, APP_TYPE_WGT, strlen(APP_TYPE_WGT))) {
			item_info->open_app = 1;
		} else {
			item_info->open_app = 0;
		}
	}


		if (icon) {
			if (strlen(icon) > 0) {
				item_info->icon = strdup(icon);
				goto_if(NULL == item_info->icon, ERROR);
			} else {
				item_info->icon = strdup(DEFAULT_ICON);
				goto_if(NULL == item_info->icon, ERROR);
			}
		} else {
			item_info->icon = strdup(DEFAULT_ICON);
			goto_if(NULL == item_info->icon, ERROR);
		}
		_D("name:%s,type:%s,icon:%s",item_info->name,item_info->type,item_info->icon);
	item_info->removable = removable;

	package_info_destroy(appinfo_h);


	return item_info;

ERROR:
	apps_item_info_destroy(item_info);
	package_info_destroy(appinfo_h);

	return NULL;
}
#else

item_info_s *apps_item_info_create(const char *appid)
{
	item_info_s *item_info = NULL;
	pkgmgrinfo_appinfo_h appinfo_h = NULL;
	pkgmgrinfo_pkginfo_h pkghandle = NULL;
	char *pkgid = NULL;
	char *name = NULL;
	char *icon = NULL;
	char *type = NULL;
	bool nodisplay = false;
	bool enabled = false;
	bool removable = false;
	int support_mode = 0;

	retv_if(!appid, NULL);

	item_info = calloc(1, sizeof(item_info_s));
	if (NULL == item_info) {
		return NULL;
	}

	goto_if(0 > pkgmgrinfo_appinfo_get_appinfo(appid, &appinfo_h), ERROR);

	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_label(appinfo_h, &name), ERROR);
	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_icon(appinfo_h, &icon), ERROR);

	do {
		break_if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_pkgid(appinfo_h, &pkgid));
		break_if(NULL == pkgid);

		break_if(0 > pkgmgrinfo_pkginfo_get_pkginfo(pkgid, &pkghandle));
		break_if(NULL == pkghandle);
	} while (0);

	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_is_nodisplay(appinfo_h, &nodisplay), ERROR);
	if (nodisplay) goto ERROR;

	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_is_enabled(appinfo_h, &enabled), ERROR);
	if (!enabled) goto ERROR;

	goto_if(PMINFO_R_OK != pkgmgrinfo_pkginfo_get_type(pkghandle, &type), ERROR);
	
	if (pkgid) {
		item_info->pkgid = strdup(pkgid);
		goto_if(NULL == item_info->pkgid, ERROR);
	}

	item_info->appid = strdup(appid);
	goto_if(NULL == item_info->appid, ERROR);

	if (name) {
		if(!strncmp(appid, APPS_PKG, strlen(APPS_PKG))) {
			item_info->name = strdup(_("IDS_IDLE_BODY_APPS"));
			goto_if(NULL == item_info->name, ERROR);
		}
		else{
			item_info->name = strdup(name);
			goto_if(NULL == item_info->name, ERROR);
		}
	}

	if (type) {
		item_info->type = strdup(type);
		goto_if(NULL == item_info->type, ERROR);
		if (!strncmp(item_info->type, APP_TYPE_WGT, strlen(APP_TYPE_WGT))) {
			item_info->open_app = 1;
		} else {
			item_info->open_app = 0;
		}
	}

	
		if (icon) {
			if (strlen(icon) > 0) {
				item_info->icon = strdup(icon);
				goto_if(NULL == item_info->icon, ERROR);
			} else {
				item_info->icon = strdup(DEFAULT_ICON);
				goto_if(NULL == item_info->icon, ERROR);
			}
		} else {
			item_info->icon = strdup(DEFAULT_ICON);
			goto_if(NULL == item_info->icon, ERROR);
		}
	

	item_info->removable = removable;

	pkgmgrinfo_appinfo_destroy_appinfo(appinfo_h);
	if (pkghandle) pkgmgrinfo_pkginfo_destroy_pkginfo(pkghandle);

	return item_info;

ERROR:
	apps_item_info_destroy(item_info);
	if (appinfo_h) pkgmgrinfo_appinfo_destroy_appinfo(appinfo_h);
	if (pkghandle) pkgmgrinfo_pkginfo_destroy_pkginfo(pkghandle);

	return NULL;
}

#endif

static int _apps_sort_cb(const void *d1, const void *d2)
{
	item_info_s *tmp1 = (item_info_s *) d1;
	item_info_s *tmp2 = (item_info_s *) d2;

	if (NULL == tmp1 || NULL == tmp1->name) return 1;
	else if (NULL == tmp2 || NULL == tmp2->name) return -1;

	return strcmp(tmp1->name, tmp2->name);
}
#ifdef TELEPHONY_DISABLE
#define MESSAGE_PKG "com.samsung.message"
#define VCONFKEY_WMS_HOST_STATUS_VENDOR "db/wms/host_status/vendor"
#endif

#ifdef TIZEN_SDK
bool
package_info_cb(package_info_h handle, void *user_data)
{

	_ENTER;
	Eina_List **list = user_data;
	char *appid = NULL;
	item_info_s *item_info = NULL;

	if(NULL == handle) return false;
	if(NULL == user_data) return false;

	package_info_get_package(handle, &appid);
	if(NULL == appid) return true;
	_D("appid:%s",appid);
	item_info = apps_item_info_create(appid);

	if (NULL == item_info) {
		_E("%s does not have the item_info", appid);
		return false;
	}

	*list = eina_list_append(*list, item_info);
	return true;

}


static Eina_List *_read_all_apps(Eina_List **list)
{
	_ENTER;
	package_manager_filter_h handle = NULL;
	int ret = package_manager_filter_create(&handle);
	if(ret !=PACKAGE_MANAGER_ERROR_NONE )
		{
			_E("Error in createing package manager filter ret :%d",ret);
		}
	ret = package_manager_filter_add_bool(handle,"PMINFO_PKGINFO_PROP_PACKAGE_NODISPLAY_SETTING",0);
	if(ret != PACKAGE_MANAGER_ERROR_NONE )
	{
		_E("Error in adding filter ret:%d",ret);
		goto ERROR;
	}

	ret = package_manager_filter_foreach_package_info(handle,package_info_cb,list);
	if(ret != PACKAGE_MANAGER_ERROR_NONE)
	{
		_E("Error in package_manager_filter_foreach_package_info ret:%d",ret);
		goto ERROR;
	}
	
	*list = eina_list_sort(*list, eina_list_count(*list), _apps_sort_cb);

ERROR:
	if(handle)
		package_manager_filter_destroy(handle);

	return *list;

}
#else
static int _apps_all_cb(pkgmgrinfo_appinfo_h handle, void *user_data)
{
	Eina_List **list = user_data;
	char *appid = NULL;
	item_info_s *item_info = NULL;

	retv_if(NULL == handle, 0);
	retv_if(NULL == user_data, 0);

	pkgmgrinfo_appinfo_get_appid(handle, &appid);
	retv_if(NULL == appid, 0);

	item_info = apps_item_info_create(appid);
	if (NULL == item_info) {
		_D("%s does not have the item_info", appid);
		return 0;
	}

	*list = eina_list_append(*list, item_info);

	return 0;
}

static Eina_List *_read_all_apps(Eina_List **list)
{
	pkgmgrinfo_appinfo_filter_h handle = NULL;

	retv_if(PMINFO_R_OK != pkgmgrinfo_appinfo_filter_create(&handle), NULL);
	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_filter_add_bool(handle, PMINFO_APPINFO_PROP_APP_NODISPLAY, 0), ERROR);
	goto_if(PMINFO_R_OK != pkgmgrinfo_appinfo_filter_foreach_appinfo(handle, _apps_all_cb, list), ERROR);
	*list = eina_list_sort(*list, eina_list_count(*list), _apps_sort_cb);

ERROR:
	if (handle) pkgmgrinfo_appinfo_filter_destroy(handle);

	return *list;
}
#endif
static Eina_Bool _load_list(void* data){
	Eina_List *pkgmgr_list = NULL;
	g_info->app_list = _read_all_apps(&pkgmgr_list);
	_create_layout(g_info);
	return ECORE_CALLBACK_CANCEL;
}

/*
void create_add_to_shortcut(const char *widget_name, const char *index){
	_D("check");
	_create_layout(NULL);

}
*/

static void
app_pause(void *data)
{
	_D("check");
}

static void
app_terminate(void *data)
{
	_D("check");
	_fini_theme();
	feedback_deinitialize();
}

static void
app_resume(void *data)
{
	_D("check");
}

static void app_control(app_control_h service, void *data)
{
	_D("test");
	char *content = NULL;

	char *tmp = NULL;
	char *first = NULL;
	char* save;
	int i = 0;
	int reset = 0;
	int ret = 0;



	//todo: this is requried: check how we are passing data to app control
#if 0
	char *instance_id = NULL;
	app_control_get_extra_data(service, "content", &content);
	app_control_get_extra_data(service, "instance_id", &instance_id);
	_D("content value : %s", content);
	_D("instance_id value : %s", instance_id);
	widget_id = strdup(instance_id);
	free(instance_id);
#else
	//content = strdup(DEFAULT_APP_ORDER);
	//todo: content
	//read from preference key, if not exist then load default app order string else, load the existing app order string
	bool prefkey_exist = false;
	ret = preference_is_existing(APP_WIDGET_CONTENT_KEY, &prefkey_exist);
	if(ret !=PREFERENCE_ERROR_NONE)
	{
		_E("preference_is_existing api failed ret:%d ",ret);
		content = strdup(DEFAULT_APP_ORDER);
	}
	else
	{
		_D("preference_is_existing api success");
		if(prefkey_exist)
		{
			_D("preference key is already exist");
			ret = preference_get_string(APP_WIDGET_CONTENT_KEY, &content);
			if(ret != PREFERENCE_ERROR_NONE)
			{
				_E("preference_get_string api failed, so load default app order ret:%d",ret);
				content = strdup(DEFAULT_APP_ORDER);
			}
		}
		else
		{
			_E("preference_key is not exist. check why key is not present. key should present always");
			ret = preference_set_string(APP_WIDGET_CONTENT_KEY, DEFAULT_APP_ORDER);
			if(ret != PREFERENCE_ERROR_NONE)
			{
				_E("preference_set_string api failed ret:%d",ret);
			}
			content = strdup(DEFAULT_APP_ORDER);
		}
	}
	_D("content: %s",content);
#endif

	if(!content){
		_E("there is no content info.");
		tmp = strdup("empty empty empty empty");
		content = strdup("empty empty empty empty");
	}
	else{
		tmp = strdup(content);
	}

	for(i = 0 ; i < 4 ; i++){
		if(i == 0){
			first = strtok_r(tmp, " ",&save);
			if(content)
			{
				if(!strcmp(first, content)){
					_E("content info format is not proper");
					reset = 1;
					break;
				}
				else{
					_set_app_slot(first, i);
				}
			}
		}
		else{
			_set_app_slot(strtok_r(NULL, " ",&save), i);
		}
	}

	if(reset == 1){
		_E("reset the content value.");
		if(tmp){
			free(tmp);
			tmp = NULL;
		}
		tmp = strdup(DEFAULT_APP_ORDER);
		bundle *b = NULL;
		b = bundle_create();
		if(!b){
			_E("failed to create bundle");
			free(content);
			free(tmp);
			return;
		}

		bundle_add_str(b, "test", tmp);
		_D("content : %s", tmp);
		//ret = widget_service_trigger_update(APP_WIDGET_PKGID, widget_id, b, 1);
		ret = widget_service_trigger_update(APP_WIDGET_PKGID, NULL, b, 1);
		if(WIDGET_ERROR_NONE != ret){
			_E("app-widget trigger failed %x", ret);
		}
		if(b){
			free(b);
		}

		for(i = 0 ; i < 4 ; i++){
			if(i == 0){
				first = strtok_r(tmp, " ",&save);
				_set_app_slot(first, i);
			}
			else{
				_set_app_slot(strtok_r(NULL, " ",&save), i);
			}
		}
	}

	free(content);
	free(tmp);
	ecore_idler_add(_load_list, NULL);
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

static bool app_create(void *data)
{
	_D("check");
	appdata_s *info = data;

	g_info = info;
	_create_edit_win(info, "__ADD_TO_SHORTCUT__", "__ADD_TO_SHORTCUT__");
	_create_select_win(info, "__SELECT_LIST__", "_SELECT_LIST__");
	_init_theme();
	feedback_initialize();
	
	_create_edit_layout(info);
	evas_object_show(info->edit_win);
	

	g_info->release_handler = ecore_event_handler_add(ECORE_EVENT_KEY_UP, _key_release_cb, NULL);
	return true;
}

int main(int argc, char *argv[])
{
	_D("check");
	appdata_s ad = {0,};
		int ret = 0;

		ui_app_lifecycle_callback_s event_callback = {0,};
		app_event_handler_h handlers[5] = {NULL, };

		event_callback.create = app_create;
		event_callback.terminate = app_terminate;
		event_callback.pause = app_pause;
		event_callback.resume = app_resume;
		event_callback.app_control = app_control;

		ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
		ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
		ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
		ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
		ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

		ret = ui_app_main(argc, argv, &event_callback, &ad);
		if (ret != APP_ERROR_NONE) {
			_E("app_main() is failed. err = %d", ret);
		}

		return ret;

	return ret;
}

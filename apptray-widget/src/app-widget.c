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

#undef TIZEN_SDK
#include <Evas.h>
#include <Elementary.h>
#include <dlog.h>
#include <widget_service.h>
#include <widget_errno.h>
#include <app_control.h>

#ifdef TIZEN_SDK
#include <package_manager.h>
#include <app_manager.h>
#else
#include <pkgmgr-info.h>
#include <package-manager.h>
#include <aul.h>
#endif

#include <badge.h>
#include "app-widget_log.h"

#ifndef TELEPHONY_DISABLE
#include <feedback.h>
#endif

#include <unicode/unum.h>
#include <unicode/ustring.h>

#include "app-widget.h"
#include <app_preference.h>

#define PATH_MAX        4096	/* # chars in a path name including nul */



#define PUBLIC __attribute__((visibility("default")))
//#define EDJE_FILE "/usr/apps/com.samsung.app-widget/shared/res/app_shortcut.edj"
#define EDJE_FILE "edje/app_shortcut.edj"
#define DEFAULT_ICON "images/unknown.png"
#define SCHEDULE_ICON "res/schedule_%s.png"
#define SCHEDULE_ICON_ARABIC "res/schedule_arabic_%s.png"
#define LANGUAGE_ARABIC "ar"

#define MESSAGE_PKG "org.tizen.message"
#define WHOME_PKG "org.tizen.w-home"
#define APPS_PKG "org.tizen.apptray-widget-app"
#define APPS_WIDGET "org.tizen.apptray-widget"
#define VCONFKEY_WMS_HOST_STATUS_VENDOR "db/wms/host_status/vendor"
#define APP_TYPE_WGT "wgt"
#define APP_WIDGET_CONTENT_KEY "org.tizen.apptray-widget-content"

#define DEFAULT_APP_ORDER "org.tizen.watch-setting empty empty empty"

static Eina_List *s_list;
//int errno;

//Ea_Theme_Color_Table *color_theme;
Eina_List *font_theme;
Elm_Theme *theme;
char *icon_path;

//samsung_log_manager_h log_manager_h = NULL;

struct info {
	widget_context_h id;
	char *content;
	int size_type;
	int w;
	int h;
	Evas_Object *dbox_win;
	Evas_Object *bg;
	Evas_Object *layout;
	struct object_info *obj[5];
	int need_to_delete;
	int first_loaded;
};

struct object_info {
	char *appid;
	char *icon;
	char *label;
	char *pkgid;
	widget_context_h dbox_id;
	int open_app;
	int index;
	Evas_Object *obj;
};

#define SHORTCUT_W 123
#define SHORTCUT_H 123





void item_badge_unregister_changed_cb(void);
static void _set_app_slot(struct info *item, const char *appid, int pos);


static void _get_resource(const char *file_in, char *file_path_out, int file_path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(file_path_out, file_path_max, "%s%s", res_path, file_in);
		free(res_path);
	}
}


static inline struct info *find_item(widget_context_h id)
{
	struct info *item=NULL;
/*
	Eina_List *l;

	EINA_LIST_FOREACH(s_list, l, item) {
			//if (!strcmp(item->id, id)) {
			if (item->id == id) {
					return item;
			}
	}
	*/
	widget_app_context_get_tag(id, (void**) &item);

	return item;
}

bool _is_arabic(const char *lang){
	char lang_tmp[10] = { 0, };
	char ar[3] = { 0, };
	int i = 0;
	strncpy(lang_tmp, lang,sizeof(lang_tmp));

	for(i = 0 ; i < 2; i++){
		ar[i] = lang_tmp[i];
	}
	ar[2] = '\0';

	if (!strncmp(ar, LANGUAGE_ARABIC, strlen(LANGUAGE_ARABIC))) {
		return true;
	}else{
		return false;
	}
}

static void _init_theme(void)
{
	theme = elm_theme_new();

}



static void _fini_theme(void)
{
	elm_theme_free(theme);
	theme = NULL;
	
}

#define LOCALE_LEN 32
char *util_get_count_str_from_icu(int count)
{
	char *p = NULL;
	char *locale_tmp = NULL;
	char *ret_str = NULL;
	char locale[LOCALE_LEN] = { 0, };
	char res[LOCALE_LEN] = { 0, };


	strncpy(locale, locale_tmp,sizeof(locale));
	free(locale_tmp);

	if(locale[0] != '\0') {
		p = strstr(locale, ".UTF-8");
		if (p) *p = 0;
	}

	ret_str = strdup(res);
	return ret_str;
}
PUBLIC int widget_initialize(const char *pkgname)
{
	_D("dbox initialized");
	/**
	 * @TODO
	 * Do one-time initialize.
	 * This will be called only once right before the first box is creating
	 */
	return 0;
}

PUBLIC int widget_finalize(void)
{
	_D("dbox finalized");
	/**
	 * @TODO
	 * Do one-time finalization.
	 * This will be called only once right after the last box is destroyed
	 */
	_fini_theme();
	return 0;
}


// NOTE: This function is going to be invoked for release all resources
static int widget_destroy(widget_context_h id, widget_app_destroy_type_e reason, bundle *content, void *user_data)
//PUBLIC int widget_destroy(const char *id)
{
	struct info *item;
	_D("[%s:%d]\n", __func__, __LINE__);

	//item = find_item(id);
	widget_app_context_get_tag(id, (void**) &item);
	if (!item) {
			/*!
			 * \NOTE
			 * EXCEPTIONAL CASES
			 */
			 _E("id is invalid");
			return WIDGET_ERROR_NOT_EXIST;
	}
	if(!item->need_to_delete){
		_E("dbox didn't receive the DELETE event");
		if (item->dbox_win) {
				evas_object_del(item->dbox_win);
		}
		return WIDGET_ERROR_NONE;
	}

	s_list = eina_list_remove(s_list, item);

	/* NOTE: You have to clear all resource which are related with
	 *       current instance. If you didn't clear it correctly,
	 *       the live data provider will get suffer from the memory
	 *       pressure. therefore, keep trace on your resources.
	 */
	if (item->dbox_win) {
			evas_object_del(item->dbox_win);
	}
	free(item->content);
	free(item->id);
	free(item);
	preference_unset_changed_cb(APP_WIDGET_CONTENT_KEY);
	return WIDGET_ERROR_NONE;
}

PUBLIC int widget_need_to_update(const widget_context_h id)
{
	struct info *item;

	_D("[%s]\n", id);

	//item = find_item(id);
	widget_app_context_get_tag(id, (void**) &item);
	if (!item) {
		/* Hmm, there is no matched instance. */
		 _E("id is invalid");
		return WIDGET_ERROR_NOT_EXIST;
	}

	return 0;
}

static void _glow_effect_done(void *data, Evas_Object *o, const char *emission, const char *source)
{
	_D("");

	elm_object_signal_emit((Evas_Object *)data, "complete_effect", "widget");

//	edje_object_message_signal_process(ad->ly_tray);
}

void app_shortcut_remove_package(const char *package){
	_D("remove %s", package);
	Eina_List *l;
	struct info *item;
	int update = 0;
	char content[255] = {0};
//	int ret = 0;
	int i = 0;

	EINA_LIST_FOREACH(s_list, l, item) {
		for(i = 0 ; i < 4 ; i++){
			if(strcmp(item->obj[i]->appid, "empty")){
				if(!strcmp(item->obj[i]->pkgid, package)){
					update = 1;
					_set_app_slot(item, "empty", i);
				}
			}
		}
		if(update){
			snprintf(content, sizeof(content)-1, "%s %s %s %s", item->obj[0]->appid, item->obj[1]->appid, item->obj[2]->appid, item->obj[3]->appid);
			//todo: check widget apis
			#if 0
			ret = widget_set_extra_info(item->id, content, NULL, NULL, NULL);
			if(ret != WIDGET_ERROR_NONE){
				_E("set content error %x", ret);
			}
			#endif
			free(item->content);
			item->content = NULL;
			item->content = strdup(content);
			update = 0;
		}
	}
}

int item_badge_count(struct object_info *item)
{
	_ENTER;
	unsigned int is_display = 0;
	unsigned int count = 0;
	badge_error_e err = BADGE_ERROR_NONE;

	retv_if(!item, 0);

	err = badge_get_display(item->appid, &is_display);
	_D("badge_get_display err:%d",err);
	if (BADGE_ERROR_NONE != err) _E("cannot get badge display");

	if (!is_display) return 0;

	err = badge_get_count(item->appid, &count);
	if (BADGE_ERROR_NONE != err) _E("cannot get badge count");

	_D("Badge for app %s : %u", item->appid, count);
	_EXIT;
	return (int) count;
}

#define BADGE_SIGNAL_LEN 16
#define MAX_BADGE_COUNT 999
void item_badge_show(struct object_info *item, int count)
{
	char *str = NULL;
	char badge_signal[16];

	ret_if(!item);

	if (count > MAX_BADGE_COUNT) count = MAX_BADGE_COUNT;

	str = util_get_count_str_from_icu(count);
	elm_object_part_text_set(item->obj, "badge_txt", str);

	if (count <= 0) {
		snprintf(badge_signal, sizeof(badge_signal), "badge,off");
	} else if (count < 10) {
		snprintf(badge_signal, sizeof(badge_signal), "badge,on,1");
	} else if (count < 100) {
		snprintf(badge_signal, sizeof(badge_signal), "badge,on,2");
	} else {
		int nRightPos = ((item->index) == 2);
		_D("nRightPos : %d", nRightPos);
		if(nRightPos) {
			snprintf(badge_signal, sizeof(badge_signal), "badge,on,3r");
		}
		else {
			snprintf(badge_signal, sizeof(badge_signal), "badge,on,3");
		}
	}
	elm_object_signal_emit(item->obj, badge_signal, "slot");

	free(str);
}

void item_badge_hide(struct object_info *item)
{
	ret_if(!item);

	elm_object_signal_emit(item->obj, "badge,off", "slot");
}


static void _badge_change_cb(unsigned int action, const char *appid, unsigned int count, void *data)
{
	unsigned int is_display = 0;
	badge_error_e err = BADGE_ERROR_NONE;
	int i = 0;

	_D("Badge changed, action : %u, appid : %s, count : %u", action, appid, count);

	ret_if(!appid);

	if (BADGE_ACTION_REMOVE == action) {
		count = 0;
		is_display = 0;
	} else {
		err = badge_get_display(appid, &is_display);
		if (BADGE_ERROR_NONE != err) _E("cannot get badge display");
		if (!is_display) count = 0;
	}

	Eina_List *l;
	struct info *item;

	EINA_LIST_FOREACH(s_list, l, item) {
		for(i = 0 ; i < 4 ; i++){
			if(!strcmp(item->obj[i]->appid, appid)){
				if (count) item_badge_show(item->obj[i], count);
				else item_badge_hide(item->obj[i]);
			}
		}
	}
}


void item_badge_register_changed_cb(void)
{
	badge_error_e err;

	err = badge_register_changed_cb(_badge_change_cb, NULL);
	ret_if(BADGE_ERROR_NONE != err);
}

void item_badge_unregister_changed_cb(void)
{
	badge_error_e err;

	err = badge_unregister_changed_cb(_badge_change_cb);
	ret_if(BADGE_ERROR_NONE != err);
}


static void _slot_l_mouse_clicked_cb(void *data, Evas_Object *o, const char *emission, const char *source){
	_D("icon clicked");

}

static void _slot_r_mouse_clicked_cb(void *data, Evas_Object *o, const char *emission, const char *source){
	_D("icon clicked");

}

static void _slot_mouse_clicked_cb(void *data, Evas_Object *o, const char *emission, const char *source){
	_D("icon clicked");


	struct object_info *info = NULL;
	info = data;
	if(!strcmp(info->appid, APPS_PKG)){
		app_control_h service = NULL;
		char *type = "launch_apps";

		ret_if(APP_CONTROL_ERROR_NONE != app_control_create(&service));
		ret_if(NULL == service);

		app_control_set_operation(service, APP_CONTROL_OPERATION_DEFAULT);
		app_control_set_app_id(service, "WHOME_PKG");
		app_control_add_extra_data(service, "home_op", type);

		int ret = app_control_send_launch_request(service, NULL, NULL);
		if (APP_CONTROL_ERROR_NONE != ret) {
			LOGE("error");
			app_control_destroy(service);
			return;
		}

		app_control_destroy(service);
	}
	else{
		_D("launch %s", info->appid);
		if(info->open_app){
			_D("launch wgt");
#ifdef TIZEN_SDK
			int ret_aul  = app_manager_open_app(info->appid);
#else
			int ret_aul  = aul_open_app(info->appid);
#endif
			if (ret_aul < AUL_R_OK) {
				_E("wgt launch failed");
				return;
			}
		}
		else{
			_D("launch normal");
			app_control_h service = NULL;

			ret_if(APP_CONTROL_ERROR_NONE != app_control_create(&service));
			ret_if(NULL == service);

			app_control_set_operation(service, APP_CONTROL_OPERATION_MAIN);

			app_control_set_app_id(service, info->appid);


			int ret = app_control_send_launch_request(service, NULL, NULL);
			if (APP_CONTROL_ERROR_NONE != ret) {
				LOGE("error");
				app_control_destroy(service);
				return;
			}

			app_control_destroy(service);
		}
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

static void _slot_mouse_down_cb(void *data, Evas_Object *o, const char *emission, const char *source){
	_D("icon mouse down");
	evas_object_color_set((Evas_Object *)data, 255, 255, 255, 127);
}

static void _slot_mouse_up_cb(void *data, Evas_Object *o, const char *emission, const char *source){
	_D("icon mouse up");
	evas_object_color_set((Evas_Object *)data, 255, 255, 255, 255);
}



static void _set_app_slot(struct info *item, const char *appid, int pos){
	_ENTER;
	_D("%s %s", appid, item->obj[pos]->appid);

	if(!appid){
		_E("appid is null");
		return;
	}
	if(!strcmp(appid, item->obj[pos]->appid)){
		_D("appid is same, no need to update");
		return;
	}else{
		_D("need to update slot%d", pos+1);
#ifdef TIZEN_SDK
		int ret = 0;
		int badge_count = 0;
		Evas_Object *slot = NULL;
		Evas_Object *icon = NULL;
		package_info_h package_info = NULL;

		package_manager_get_package_info(appid, &package_info);


		char index[10] = {0};
		snprintf(index, sizeof(index)-1, "index%d", pos+1);
		char signal[20] = {0,};
		snprintf(signal, sizeof(signal), "mouse_clicked_%d", pos+1);
		char signal_l[30] = {0,};
		snprintf(signal_l, sizeof(signal_l), "mouse_clicked_l_%d", pos+1);
		char signal_r[30] = {0,};
		snprintf(signal_r, sizeof(signal_r), "mouse_clicked_r_%d", pos+1);
		_D("index:%s, signal:%s, signal_l:%s, signal_r:%s",index,signal,signal_l,signal_r);

		elm_object_part_content_unset(item->layout, index);
		if(item->obj[pos]->obj)
			evas_object_del(item->obj[pos]->obj);
		item->obj[pos]->obj = NULL;

		/*set new slot */
		item->obj[pos]->appid = strdup(appid);

		if(!strcmp(appid, "empty")){
			_W("let it empty slot");
			_D("removing mouse even callbacks from layout");
			elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_l, "*", _slot_l_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_r, "*", _slot_r_mouse_clicked_cb);
			return;
		}
		slot = elm_layout_add(item->layout);
		item->obj[pos]->obj = slot;

		char full_path[PATH_MAX] = { 0, };
		_get_resource(EDJE_FILE, full_path, sizeof(full_path));
		_D("full_path:%s",full_path);
		ret = elm_layout_file_set(slot, full_path, "icon_slot");
		if(ret == EINA_FALSE){
			LOGE("failed to set empty slot");
			_D("removing mouse even callbacks from item layout");
			elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_l, "*", _slot_l_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_r, "*", _slot_r_mouse_clicked_cb);
			return;
		}

		evas_object_size_hint_weight_set(slot, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_resize(slot, SHORTCUT_W, SHORTCUT_H);
		elm_object_part_content_set(item->layout, index, slot);
		evas_object_show(slot);

		char *label = NULL;

		item->obj[pos]->pkgid = strdup(appid);


		if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_label(package_info, &label)){
			_E("get label failed");
			item->obj[pos]->label = strdup("");
		}
		else{
			item->obj[pos]->label = strdup(label);
		}
		_D("label: %s",label);
		elm_object_part_text_set(slot, "name", label);

		char *type = NULL;
		if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_type(package_info, &type)){
			LOGE("get app type failed");
		}
		_D("type :%s,APP_TYPE_WGT:%s",type,APP_TYPE_WGT);
		if (type) {
			if (!strncmp(type, APP_TYPE_WGT, strlen(APP_TYPE_WGT))) {
				item->obj[pos]->open_app = 1;
			} else {
				item->obj[pos]->open_app = 0;
			}
		}

		_D("appid: %s", appid);
		char *icon_path_tmp = NULL;
		if(PACKAGE_MANAGER_ERROR_NONE != package_info_get_icon(package_info, &icon_path_tmp)){
				LOGE("get icon path failed");
			}
		if (icon_path_tmp) {
			if (strlen(icon_path_tmp) > 0) {
				icon_path = strdup(icon_path_tmp);
			} else {
				icon_path = strdup(DEFAULT_ICON);
			}
		} else {
			icon_path = strdup(DEFAULT_ICON);
		}
		_D("icon_path: %s", icon_path);
		item->obj[pos]->icon = strdup(icon_path);
		_D("icon path in object info %s, label %s", item->obj[pos]->icon, item->obj[pos]->label);
		icon = evas_object_image_add(evas_object_evas_get(slot));
		evas_object_repeat_events_set(icon, EINA_TRUE);
		evas_object_image_file_set(icon, icon_path, NULL);
		evas_object_image_filled_set(icon, EINA_TRUE);
		evas_object_show(icon);
		_D("icon is added");
		if(icon_path){
			free(icon_path);
			icon_path = NULL;
		}

		badge_count = item_badge_count(item->obj[pos]);
		if (badge_count) item_badge_show(item->obj[pos], badge_count);
		else item_badge_hide(item->obj[pos]);

		elm_object_part_content_set(slot, "icon", icon);
		//evas_object_show(icon);
		_D("removing mouse even callbacks from layout");
		elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
		elm_object_signal_callback_del(item->layout, signal_l, "*", _slot_l_mouse_clicked_cb);
		elm_object_signal_callback_del(item->layout, signal_r, "*", _slot_r_mouse_clicked_cb);

		_D("adding mouse even callbacks to layout");
		elm_object_signal_callback_add(item->layout, signal, "*", _slot_mouse_clicked_cb, item->obj[pos]);
		elm_object_signal_callback_add(item->layout, signal_l, "*", _slot_l_mouse_clicked_cb, item->obj[pos]);
		elm_object_signal_callback_add(item->layout, signal_r, "*", _slot_r_mouse_clicked_cb, item->obj[pos]);

		_D("adding mouse even callbacks to slot");
		elm_object_signal_callback_add(slot, "mouse_down", "*", _slot_mouse_down_cb, icon);
		elm_object_signal_callback_add(slot, "mouse_up", "*", _slot_mouse_up_cb, icon);
		elm_object_signal_callback_add(slot, "complete,launch_effect", "event", _glow_effect_done, slot);


		package_info_destroy(package_info);
#else
		int ret = 0;
		int badge_count = 0;
		Evas_Object *slot = NULL;
		Evas_Object *icon = NULL;
		pkgmgrinfo_appinfo_h appinfo_h = NULL;
		pkgmgrinfo_pkginfo_h pkghandle = NULL;
		char *pkgid = NULL;

		/*unset pre slot item */
		char index[10] = {0};
		snprintf(index, sizeof(index)-1, "index%d", pos+1);
		char signal[20] = {0,};
		snprintf(signal, sizeof(signal), "mouse_clicked_%d", pos+1);
		char signal_l[30] = {0,};
		snprintf(signal_l, sizeof(signal_l), "mouse_clicked_l_%d", pos+1);
		char signal_r[30] = {0,};
		snprintf(signal_r, sizeof(signal_r), "mouse_clicked_r_%d", pos+1);
		elm_object_part_content_unset(item->layout, index);
		if(item->obj[pos]->obj)
			evas_object_del(item->obj[pos]->obj);
		item->obj[pos]->obj = NULL;

		/*set new slot */
		item->obj[pos]->appid = strdup(appid);

		if(!strcmp(appid, "empty")){
			_W("let it empty slot");
			elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_l, "*", _slot_l_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_r, "*", _slot_r_mouse_clicked_cb);
			return;
		}
		slot = elm_layout_add(item->layout);
		item->obj[pos]->obj = slot;
		char full_path[PATH_MAX] = { 0, };
		_get_resource(EDJE_FILE, full_path, sizeof(full_path));
		_D("full_path:%s",full_path);
		ret = elm_layout_file_set(slot, full_path, "icon_slot");
		if(ret == EINA_FALSE){
			_E("failed to set empty slot");
			elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_l, "*", _slot_l_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_r, "*", _slot_r_mouse_clicked_cb);
			return;
		}

		evas_object_size_hint_weight_set(slot, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_resize(slot, SHORTCUT_W, SHORTCUT_H);
		elm_object_part_content_set(item->layout, index, slot);
		evas_object_show(slot);

		char *label = NULL;
		ret = pkgmgrinfo_appinfo_get_appinfo(appid, &appinfo_h);
		if(ret != PMINFO_R_OK){
			_E("get appinfo failed. let it empty slot, %d", ret);
			elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_l, "*", _slot_l_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_r, "*", _slot_r_mouse_clicked_cb);
			item->obj[pos]->appid = strdup("empty");
			return;
		}

		if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_pkgid(appinfo_h, &pkgid)){
			_E("get pkgid failed. let it empty slot");
			elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_l, "*", _slot_l_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_r, "*", _slot_r_mouse_clicked_cb);
			item->obj[pos]->appid = strdup("empty");
			return;
		}
		item->obj[pos]->pkgid = strdup(pkgid);

		if(0 > pkgmgrinfo_pkginfo_get_pkginfo(pkgid, &pkghandle)){
			_E("get pkghandle faile.");
			elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_l, "*", _slot_l_mouse_clicked_cb);
			elm_object_signal_callback_del(item->layout, signal_r, "*", _slot_r_mouse_clicked_cb);
			item->obj[pos]->appid = strdup("empty");
			return;
		}
		if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_label(appinfo_h, &label)){
			_E("get label failed");
			item->obj[pos]->label = strdup("");
		}
		else{
			item->obj[pos]->label = strdup(label);
		}
		elm_object_part_text_set(slot, "name", label);

		char *type = NULL;
		if(PMINFO_R_OK != pkgmgrinfo_pkginfo_get_type(pkghandle, &type)){
			_E("get app type failed");
		}
		if (type) {
			if (!strncmp(type, APP_TYPE_WGT, strlen(APP_TYPE_WGT))) {
				item->obj[pos]->open_app = 1;
			} else {
				item->obj[pos]->open_app = 0;
			}
		}

		_W("%s", appid);
		char *icon_path_tmp = NULL;
		if(PMINFO_R_OK != pkgmgrinfo_appinfo_get_icon(appinfo_h, &icon_path_tmp)){
				_E("get icon path failed");
			}
			if (icon_path_tmp) {
				if (strlen(icon_path_tmp) > 0) {
					icon_path = strdup(icon_path_tmp);
				} else {
					icon_path = strdup(DEFAULT_ICON);
				}
			} else {
				icon_path = strdup(DEFAULT_ICON);
			}
			item->obj[pos]->icon = strdup(icon_path);
		_W("icon path in object info %s, label %s", item->obj[pos]->icon, item->obj[pos]->label);
		icon = evas_object_image_add(evas_object_evas_get(slot));
		evas_object_repeat_events_set(icon, EINA_TRUE);
		evas_object_image_file_set(icon, icon_path, NULL);
		evas_object_image_filled_set(icon, EINA_TRUE);
		evas_object_show(icon);
		if(icon_path){
			free(icon_path);
			icon_path = NULL;
		}

		badge_count = item_badge_count(item->obj[pos]);
		if (badge_count) item_badge_show(item->obj[pos], badge_count);
		else item_badge_hide(item->obj[pos]);

		elm_object_part_content_set(slot, "icon", icon);
//		evas_object_show(icon);

		elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
		elm_object_signal_callback_del(item->layout, signal_l, "*", _slot_l_mouse_clicked_cb);
		elm_object_signal_callback_del(item->layout, signal_r, "*", _slot_r_mouse_clicked_cb);

		elm_object_signal_callback_add(item->layout, signal, "*", _slot_mouse_clicked_cb, item->obj[pos]);
		elm_object_signal_callback_add(item->layout, signal_l, "*", _slot_l_mouse_clicked_cb, item->obj[pos]);
		elm_object_signal_callback_add(item->layout, signal_r, "*", _slot_r_mouse_clicked_cb, item->obj[pos]);

		elm_object_signal_callback_add(slot, "mouse_down", "*", _slot_mouse_down_cb, icon);
		elm_object_signal_callback_add(slot, "mouse_up", "*", _slot_mouse_up_cb, icon);
		elm_object_signal_callback_add(slot, "complete,launch_effect", "event", _glow_effect_done, slot);

		if (appinfo_h) pkgmgrinfo_appinfo_destroy_appinfo(appinfo_h);
		if (pkghandle) pkgmgrinfo_pkginfo_destroy_pkginfo(pkghandle);

		char log[10] = {0};
		snprintf(log, sizeof(log)-1, "ASS%d", pos+1);
#endif

		_D("slot is added %d",pos+1);

	}
	_EXIT;
	return;

}

PUBLIC int widget_update_content(const widget_context_h id)
{
	_ENTER;
	struct info *item;
	//item = find_item(id);
	widget_app_context_get_tag(id, (void**) &item);
	int i = 0;
	char *tmp = NULL;
	char *first = NULL;
	char* save;
	int ret = 0;
	if (!item) {
		 _E("id is invalid");
		return WIDGET_ERROR_NOT_EXIST;
	}

	_W("content : %s", item->content);

	tmp = strdup(item->content);
	//todo: check widget api
//	ret = widget_set_extra_info(id, tmp, NULL, NULL, NULL);
	if(ret != WIDGET_ERROR_NONE){
		_E("set content error %x", ret);
	}

	for(i = 0 ; i < 4 ; i++){
		if(i == 0){
			first = strtok_r(tmp, " ",&save);
			_set_app_slot(item, first, i);
		}
		else{
			_set_app_slot(item, strtok_r(NULL, " ",&save), i);
		}
	}

	/**
	 * @NOTE
	 * This function have to generate a new content.
	 * If you don't want to generate new content,
	 * return negative values.
	 * or you have to generate the new content. in timeout msec.
	*/

	return WIDGET_ERROR_DISABLED;
}

static char *_set_content_data(char *content, char *updated_data){
	_ENTER;
	return NULL;
}


PUBLIC int widget_set_content_info(const widget_context_h id, bundle *b)
{
	_ENTER;
	struct info *item;
	int ret = 0;
	char uri[256] = {0};

	_E("[%s]\n", id);

	//item = find_item(id);
	widget_app_context_get_tag(id, (void**) &item);
	if (!item) {
		 _E("id is invalid");
			return WIDGET_ERROR_NOT_EXIST;
	}
	const char *content = NULL;
	bundle_get_str(b,"test",(char**)&content);
	if(content){
		if(!strcmp(content, "delete")){
			_D("for test");
			_D("[%s], [%s]", id, item->id);
			snprintf(uri, sizeof(uri) - 1, "file://%s", item->id);
			//ret = widget_provider_send_deleted(APPS_WIDGET, uri);
			if(ret != WIDGET_ERROR_NONE){
				_E("widget_provider_send_deleted error %x", ret);
			}
		}else{
			_W("content : %s", content);
			_E("ex-content info : %s", item->content);

			//todo: check widget api
			#if 0
			tmp = _set_content_data(item->content, content);
			ret = widget_set_extra_info(id, content, NULL, NULL, NULL);
			if(ret != WIDGET_ERROR_NONE){
				_E("set content error %x", ret);
			}
			#endif
			free(item->content);
			item->content = NULL;
			item->content = strdup(content);

			_E("new-content info : %s", item->content);
		}
	}

	/**
	 * @NOTE
	 * This function have to generate a new content.
	 * If you don't want to generate new content,
	 * return negative values.
	 * or you have to generate the new content. in timeout msec.
	 */
	return WIDGET_ERROR_NONE;
}

PUBLIC int widget_clicked(const widget_context_h id, const char *event, double timestamp, double x, double y)
{
	_ENTER;
	_D("dbox clicked");
	struct info *item;
//	item = find_item(id);
	widget_app_context_get_tag(id, (void**) &item);
	if (!item) {
		 _E("id is invalid");
			return WIDGET_ERROR_NOT_EXIST;
	}

	return WIDGET_ERROR_NONE; /* No chages */
}

static void _mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info){
	_ENTER;
	struct object_info *info = NULL;
	info = data;
	elm_object_signal_emit(info->obj, "pressed", "widget_plus");
}

static void _mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info){
	_ENTER;
	struct object_info *info = NULL;
	info = data;
	elm_object_signal_emit(info->obj, "released", "widget_plus");
}

static void _mouse_clicked_cb(void *data, Evas_Object *o, const char *emission, const char *source){
	_ENTER;
	_D("slot clicked");
#if 1
	struct object_info *info = NULL;
	info = data;

	app_control_h service = NULL;
	char *type = "app_shortcut";
	char index[3] = {0};
	snprintf(index, sizeof(index)-1, "%d", info->index);

	ret_if(APP_CONTROL_ERROR_NONE != app_control_create(&service));
	ret_if(NULL == service);

	_D("%s", info->dbox_id);

	app_control_set_operation(service, APP_CONTROL_OPERATION_DEFAULT);
	app_control_set_app_id(service, WHOME_PKG);
	app_control_add_extra_data(service, "home_op", type);
	app_control_add_extra_data(service, "widget_name", info->dbox_id);
	app_control_add_extra_data(service, "index", index);

	int ret = app_control_send_launch_request(service, NULL, NULL);
	if (APP_CONTROL_ERROR_NONE != ret) {
		_E("error");
		app_control_destroy(service);
		return;
	}

	app_control_destroy(service);
#endif
}


static struct object_info *_add_empty_slot(Evas_Object *parent, int pos, struct info *item)
{
	_ENTER;
	_D("add empty slot %d", pos);
	int ret = 0;
	Evas_Object *slot = NULL;
	char index[10] = {0};
	snprintf(index, sizeof(index)-1, "index%d", pos);

	struct object_info *info;

	info = malloc(sizeof(struct object_info));
	if (!info) {
		LOGE("failed to alloc memory");
		return NULL;
	}

	slot = elm_layout_add(parent);

	info->appid = strdup("empty");
	info->dbox_id = item->id;
	info->index = pos;
	info->obj = slot;
	info->icon = NULL;
	info->label = strdup("empty");
	char full_path[PATH_MAX] = { 0, };
	_get_resource(EDJE_FILE, full_path, sizeof(full_path));
	_D("full_path:%s",full_path);
	ret = elm_layout_file_set(slot, full_path, "empty_slot");
	if(ret == EINA_FALSE){
		LOGE("failed to set empty slot");
		free(info);
		return NULL;
	}
	evas_object_size_hint_weight_set(slot, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_resize(slot, SHORTCUT_W, SHORTCUT_H);

	evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, info);
	evas_object_event_callback_add(slot, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, info);
	elm_object_signal_callback_add(slot, "mouse_clicked", "*", _mouse_clicked_cb, info);
	elm_object_signal_callback_add(slot, "complete,launch_effect", "event", _glow_effect_done, slot);

	elm_object_part_content_set(parent, index, slot);
	evas_object_show(slot);

	_D("success");

	return info;
}

static Evas_Object *_create_win(widget_context_h context, int w, int h)
{
	Evas_Object *win = NULL;
	int ret;

	if (context == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get context.");
		return NULL;
	}

	ret = widget_app_get_elm_win(context, &win);
	if (ret != WIDGET_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get window. err = %d", ret);
		return NULL;
	}
	evas_object_resize(win, w, h);
	evas_object_show(win);

	return win;
}

static int widget_resize(widget_context_h id, int w, int h, void *user_data)
{
	struct info *item;
	int ret;
	Evas_Object *layout = NULL;
	Evas_Object *bg = NULL;
	char *tmp = NULL;

	//preference_remove_all();
//	item = find_item((char*)id);
	widget_app_context_get_tag(id,(void**)&item);
	if (!item) {
		 _E("id is invalid");
			return WIDGET_ERROR_NOT_EXIST;
	}

	_D("WIDGET is resized\n");
	item->dbox_win = _create_win(id,w,h);
			if (!item->dbox_win) {
				_E("item dbox win is not found");
					return WIDGET_ERROR_FAULT;
			}

	_init_theme();


	bg = elm_layout_add(item->dbox_win);
	char full_path[PATH_MAX] = { 0, };
	_get_resource(EDJE_FILE, full_path, sizeof(full_path));
	_D("full_path:%s",full_path);
	ret = elm_layout_file_set(bg, full_path, "dbox_bg");

	if(ret == EINA_FALSE){
		LOGE("failed to set layout");
		return WIDGET_ERROR_FAULT;
	}
	_D("bg set");
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_resize(bg, w, h);
	evas_object_show(bg);
	item->bg = bg;

	layout = elm_layout_add(bg);
	item->layout = layout;
	_D("full_path:%s",full_path);
	ret = elm_layout_file_set(layout, full_path, "layout");
	_D("layout added");
	if(ret == EINA_FALSE){
		LOGE("failed to set layout");
		return WIDGET_ERROR_FAULT;
	}
	strncpy(item->content,"default",sizeof(item->content));
	_W("content : %s", item->content);

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_resize(layout, w, h);
	evas_object_show(layout);

	elm_object_part_content_set(bg, "bg_swallow", layout);

	evas_object_resize(item->dbox_win, w, h);
	evas_object_show(item->dbox_win);
	_D("win show");


	_D("item->content:%s",item->content);
	_D("empty dbox created");
	/* load index layout */
	item->obj[0] = _add_empty_slot(layout, 1, item);
	item->obj[1] = _add_empty_slot(layout, 2, item);
	item->obj[2] = _add_empty_slot(layout, 3, item);
	item->obj[3] = _add_empty_slot(layout, 4, item);
	_D("four slots added");
//todo: item->content
	//read from preference key, if not exist then load default app order string else, load the existing app order string
	bool prefkey_exist = false;
	ret = preference_is_existing(APP_WIDGET_CONTENT_KEY, &prefkey_exist);
	if(ret !=PREFERENCE_ERROR_NONE)
	{
		_E("preference_is_existing api failed ret:%d ",ret);
		item->content = strdup(DEFAULT_APP_ORDER);
	}
	else
	{
		_D("preference_is_existing api success");
		if(prefkey_exist)
		{
			_D("preference key is already exist");
			ret = preference_get_string(APP_WIDGET_CONTENT_KEY, &item->content);
			if(ret != PREFERENCE_ERROR_NONE)
			{
				_E("preference_get_string api failed, so load default app order ret:%d",ret);
				item->content = strdup(DEFAULT_APP_ORDER);
			}
		}
		else
		{
			_E("preference_key is not exist. might be first boot so load default app order and store in preference key");
			ret = preference_set_string(APP_WIDGET_CONTENT_KEY, DEFAULT_APP_ORDER);
			if(ret != PREFERENCE_ERROR_NONE)
			{
				_E("preference_set_string api failed ret:%d",ret);
			}
			item->content = strdup(DEFAULT_APP_ORDER);
		}
	}
	_D("item->content :%s",item->content);
//todo: by any chance content string is lost, we should load default apps. this has to be handle
	tmp = strdup(item->content);
	char *first = NULL;
	char* save;
	int i = 0;

	for(i = 0 ; i < 4 ; i++){
		if(i == 0){
			first = strtok_r(tmp, " ",&save);
			_set_app_slot(item, first, i);
		}
		else{
			_set_app_slot(item, strtok_r(NULL, " ",&save), i);
		}
	}

	_D("widget resized to %dx%d\n", w, h);
	item->w =w;
	item->h = h;
	 return WIDGET_ERROR_NONE;
}

void preference_changed_cb_impl(const char *key, void *user_data)
{
	_ENTER;
//	int ret = 0;
	widget_context_h *id= user_data;
	struct info *item;
	//preference_remove_all();
	if(!id)
	{
		_E("id is null");
		return;
	}
//	item = find_item(*id);
	widget_app_context_get_tag(*id, (void**) &item);
	if (!item) {
		 _E("id is invalid");
		_E("item is not found");
		return;
	}
	_D("before content: %s",item->content);
    preference_get_string(key, &item->content);
    _D("after content: %s",item->content);

	_EXIT;
}

// NOTE: This function is going to be invoked for initializing all resources
static int widget_create(widget_context_h id, bundle *content, int w, int h, void *user_data)
{
	_D("WIDGET is created\n");
	struct info *info;

	info = malloc(sizeof(*info));
	if (!info) {
			return WIDGET_ERROR_OUT_OF_MEMORY;
	}

	info->id = id;
//	if (!info->id) {
//			free(info);
//			return WIDGET_ERROR_OUT_OF_MEMORY;
//	}

	//info->content = strdup(content);
//	bundle_get_str(b,"test",&info->content);
//	if (!info->content) {
//			free(info->id);
//			free(info);
//			return WIDGET_ERROR_OUT_OF_MEMORY;
//	}

	/**
	 * @NOTE
	 * cluster == 'user,created'
	 * category == 'default'
	 *
	 * You don't need to care these two values if you don't know what are them
	 */

	info->size_type = WIDGET_SIZE_TYPE_UNKNOWN;
	s_list = eina_list_append(s_list, info);
	info->need_to_delete = 0;
	info->first_loaded = 1;

	/**
	 * @NOTE
	 * You can returns WIDGET_OUTPUT_UPDATED or WIDGET_NEED_TO_SCHEDULE or WIDGET_DONE
	 * You also can use them at same time using '|'
	 *
	 * If your content is updated, from this function, you have to
	 * return WIDGET_OUTPUT_UPDATED;
	 *
	 * If you want to the provider call your widget_update_content function ASAP
	 * return WIDGET_NEED_TO_SCHEDULE;
	 *
	 * If your content is updated and need to call the update_content function ASAP,
	 * return WIDGET_OUTPUT_UPDATED | WIDGET_NEED_TO_SCHEDULE
	 *
	 * Don't have any changes, just
	 * return WIDGET_DONE
	 */

	/**
	 * @NOTE
	 * You create the default output image from here now.
	 * So you HAVE TO return WIDGET_OUTPUT_UPDATED
	 */
	preference_set_changed_cb(APP_WIDGET_CONTENT_KEY,preference_changed_cb_impl,&id);
	widget_app_context_set_tag(id, info);
	//widget_resize(id,w, h, user_data);
	return WIDGET_ERROR_NONE;
}
PUBLIC int widget_need_to_create(const char *cluster, const char *category)
{
	/**
	 * @NOTE
	 * You don't need implement this, if don't know what this is.
	 * return 0 or 1
	 */
	return 0;
}

PUBLIC int widget_change_group(const widget_context_h id, const char *cluster, const char *category)
{
	struct info *item;

	//item = find_item(id);
	widget_app_context_get_tag(id, (void**) &item);
	if (!item) {
		 _E("id is invalid");
			return WIDGET_ERROR_NOT_EXIST;
	}

	/**
	 * @NOTE
	 * If you can generate new content in this function,
	 * Generate a new content.
	 * and return WIDGET_OUTPUT_UPDATED
	 *
	 * In case of you cannot create the updated image in this function directly,
	 * return WIDGET_NEED_TO_SCHEDULE
	 * The provider will call your widget_need_to_update & widget_update_content function.
	 *
	 * I recommend that if you are able to generate new content in this function,
	 * generate it directly. and just returns WIDGET_OUTPUT_UPDATED
	 *
	 * Because if you return WIDGET_NEED_TO_SCHEDULE, the provider will try to update your livebox
	 * But it can be interrupted by other events.
	 * Then you livebox content updating will be delayed
	 */
	return WIDGET_ERROR_NONE;
}

PUBLIC int widget_need_to_destroy(const widget_context_h id)
{
	/**
	 * @NOTE
	 * You don't need implement this, if don't know what this is.
	 * This will be called after call the widget_need_to_update function.
	 * If the widget_need_to_update function returns 0,
	 * The provider will call this.
	 *
	 * If you return 1, the provider will delete your box.
	 */
	return 0;
}

PUBLIC char *widget_pinup(const widget_context_h id, int pinup)
{
	struct info *item;

//	item = find_item(id);
	widget_app_context_get_tag(id, (void**) &item);
	if (!item) {
		 _E("id is invalid");
			return NULL;
	}

	return strdup(item->content);
}

PUBLIC int widget_is_pinned_up(const widget_context_h id)
{
	struct info *item;

//	item = find_item(id);
	widget_app_context_get_tag(id, (void**) &item);
	if (!item) {
			return WIDGET_ERROR_NOT_EXIST;
	}

	/**
	 * @NOTE
	 * If you can generate new content in this function,
	 * Generate a new content.
	 * and return WIDGET_OUTPUT_UPDATED
	 *
	 * In case of you cannot create the updated image in this function directly,
	 * return WIDGET_NEED_TO_SCHEDULE
	 * The provider will call your widget_need_to_update & widget_update_content function.
	 *
	 * I recommend that if you are able to generate new content in this function,
	 * generate it directly. and just returns WIDGET_OUTPUT_UPDATED
	 *
	 * Because if you return WIDGET_NEED_TO_SCHEDULE, the provider will try to update your livebox
	 * But it can be interrupted by other events.
	 * Then you livebox content updating will be delayed
	 */
	return WIDGET_ERROR_NONE;
}

void _set_app_label(struct object_info *obj)
{

}


static void
widget_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	_ENTER;
	/* APP_EVENT_LANGUAGE_CHANGED */
	char *locale = NULL;
	app_event_get_language(event_info, &locale);
	elm_language_set(locale);
	free(locale);
}

static void
widget_app_region_changed(app_event_info_h event_info, void *user_data)
{
	_ENTER;
	/* APP_EVENT_REGION_FORMAT_CHANGED */
}

static int
widget_pause(widget_context_h id, void *user_data)
{
	_ENTER;
	/* Take necessary actions when widget instance becomes invisible. */
	return WIDGET_ERROR_NONE;

}

static int
widget_resume(widget_context_h id, void *user_data)
{
	/* Take necessary actions when widget instance becomes visible. */
	_ENTER;
	return WIDGET_ERROR_NONE;
}

static int
widget_update(widget_context_h id, bundle *content,
                             int force, void *user_data)
{
	_ENTER;
	/* Take necessary actions when widget instance should be updated. */
	return WIDGET_ERROR_NONE;
}
static widget_class_h
widget_app_create(void *user_data)
{
	/* Hook to take necessary actions before main event loop starts.
	   Initialize UI resources.
	   Make a class for widget instance.
	*/
	app_event_handler_h handlers[5] = {NULL, };

	widget_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
		APP_EVENT_LANGUAGE_CHANGED, widget_app_lang_changed, user_data);
	widget_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
		APP_EVENT_REGION_FORMAT_CHANGED, widget_app_region_changed, user_data);

	widget_instance_lifecycle_callback_s ops = {
		.create = widget_create,
		.destroy = widget_destroy,
		.pause = widget_pause,
		.resume = widget_resume,
		.update = widget_update,
		.resize = widget_resize,
	};

	return widget_app_class_create(ops, user_data);
}

static void
widget_app_terminate(void *user_data)
{
	/* Release all resources. */
	_ENTER;
}

int
main(int argc, char *argv[])
{
	widget_app_lifecycle_callback_s ops = {0,};
	int ret;

	ops.create = widget_app_create;
	ops.terminate = widget_app_terminate;

	ret = widget_app_main(argc, argv, &ops, NULL);
	if (ret != WIDGET_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "widget_app_main() is failed. err = %d", ret);
	}

	return ret;
}

/*
 * @brief: This function handle click event.
 */


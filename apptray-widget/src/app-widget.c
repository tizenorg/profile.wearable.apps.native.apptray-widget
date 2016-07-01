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
#include <Evas.h>
#include <Elementary.h>
#include <dlog.h>
#include <widget_service.h>
#include <widget_errno.h>
#include <app_control.h>

#include <pkgmgr-info.h>
#include <package-manager.h>
#include <aul.h>


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
#define APP_WIDGET_CONTENT_KEY "org.tizen.apptray-widget"
#define APP_LAUNCH_KEY "launch_apps"
 #define WHOME_APP_CONTROL "home_op"

#define DEFAULT_APP_ORDER "org.tizen.apptray-widget-app empty org.tizen.watch-setting empty"
#define BUNDLE_CONTENT_KEY "content"
#define BUNDLE_ID_KEY "id"

#define SHORTCUT_W 123
#define SHORTCUT_H 123


Eina_List *font_theme;
Elm_Theme *theme;
char *icon_path;
struct info {
	char* id;
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
	widget_context_h context;
	bundle* content_bundle;
};

struct object_info {
	char *appid;
	char *icon;
	char *label;
	char *pkgid;
	char* dbox_id;
	int open_app;
	int index;
	Evas_Object *obj;
};







static void _set_app_slot(struct info *item, const char *appid, int pos);


static void _get_resource(const char *file_in, char *file_path_out, int file_path_max)
{
	_ENTER;
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(file_path_out, file_path_max, "%s%s", res_path, file_in);
		free(res_path);
	}
}

PUBLIC void widget_update_content(struct info* item);


static void _init_theme(void)
{
	_ENTER;
	theme = elm_theme_new();

}





// NOTE: This function is going to be invoked for release all resources
static int widget_destroy(widget_context_h context, widget_app_destroy_type_e reason, bundle *content, void *user_data)
{
	_ENTER;
	struct info *item = NULL;
	widget_app_context_get_tag(context, (void**)&item);
	if (!item) {
			 _E("id is invalid");
			return WIDGET_ERROR_NOT_EXIST;
	}

	/* NOTE: You have to clear all resource which are related with
	 *       current instance. If you didn't clear it correctly,
	 *       the live data provider will get suffer from the memory
	 *       pressure. therefore, keep trace on your resources.
	 */
	if (item->dbox_win) {
			evas_object_del(item->dbox_win);
	}
	item->first_loaded = 0;
	item->w = item->h = 0;
	free(item->content);
	free(item->id);
	free(item);
	return WIDGET_ERROR_NONE;
}


static void _glow_effect_done(void *data, Evas_Object *o, const char *emission, const char *source)
{
	_ENTER;

	elm_object_signal_emit((Evas_Object *)data, "complete_effect", "widget");
}


static void _slot_mouse_clicked_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
	_D("icon clicked");
	_ENTER;

	struct object_info *info = NULL;
	info = data;
	if (!strcmp(info->appid, APPS_PKG)) {
		app_control_h service = NULL;
		char *type = APP_LAUNCH_KEY;

		ret_if(APP_CONTROL_ERROR_NONE != app_control_create(&service));
		ret_if(NULL == service);

		app_control_set_operation(service, APP_CONTROL_OPERATION_DEFAULT);
		app_control_set_app_id(service, WHOME_PKG);
		app_control_add_extra_data(service, WHOME_APP_CONTROL, type);

		int ret = app_control_send_launch_request(service, NULL, NULL);
		_D("Send Launch Request = %d", ret);
		if (APP_CONTROL_ERROR_NONE != ret) {
			LOGE("error");
			app_control_destroy(service);
			return;
		}

		app_control_destroy(service);
	} else {
		_D("launch %s", info->appid);
		if (info->open_app) {
			_D("launch wgt");
			int ret_aul  = aul_open_app(info->appid);
			if (ret_aul < AUL_R_OK) {
				_E("wgt launch failed");
				return;
			}
		} else {
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

static void _set_app_slot(struct info *item, const char *appid, int pos)
{
	_ENTER;
	_D("%s %s", appid, item->obj[pos]->appid);

	if (!appid) {
		_E("appid is null");
		return;
	}
	if (!strcmp(appid, item->obj[pos]->appid)) {
		_D("appid is same, no need to update");
		return;
	} else {
		_D("need to update slot%d", pos+1);
		int ret = 0;
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
		if (item->obj[pos]->obj)
			evas_object_del(item->obj[pos]->obj);
		item->obj[pos]->obj = NULL;

		/*set new slot */
		item->obj[pos]->appid = strdup(appid);

		if (!strcmp(appid, "empty")) {
			_W("let it empty slot");
			elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
			return;
		}
		slot = elm_layout_add(item->layout);
		item->obj[pos]->obj = slot;
		char full_path[PATH_MAX] = { 0, };
		_get_resource(EDJE_FILE, full_path, sizeof(full_path));
		_D("full_path:%s", full_path);
		ret = elm_layout_file_set(slot, full_path, "icon_slot");
		if (ret == EINA_FALSE) {
			_E("failed to set empty slot");
			elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
			return;
		}

		evas_object_size_hint_weight_set(slot, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_resize(slot, SHORTCUT_W, SHORTCUT_H);
		elm_object_part_content_set(item->layout, index, slot);
		evas_object_show(slot);

		char *label = NULL;
		ret = pkgmgrinfo_appinfo_get_appinfo(appid, &appinfo_h);
		if (ret != PMINFO_R_OK) {
			_E("get appinfo failed. let it empty slot, %d", ret);
			elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
			item->obj[pos]->appid = strdup("empty");
			return;
		}

		if (PMINFO_R_OK != pkgmgrinfo_appinfo_get_pkgid(appinfo_h, &pkgid)) {
			_E("get pkgid failed. let it empty slot");
			elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
			item->obj[pos]->appid = strdup("empty");
			return;
		}
		item->obj[pos]->pkgid = strdup(pkgid);

		if (0 > pkgmgrinfo_pkginfo_get_pkginfo(pkgid, &pkghandle)) {
			_E("get pkghandle faile.");
			elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);
			item->obj[pos]->appid = strdup("empty");
			return;
		}
		if (PMINFO_R_OK != pkgmgrinfo_appinfo_get_label(appinfo_h, &label)) {
			_E("get label failed");
			item->obj[pos]->label = strdup("");
		} else {
			item->obj[pos]->label = strdup(label);
		}
		elm_object_part_text_set(slot, "name", label);

		char *type = NULL;
		if (PMINFO_R_OK != pkgmgrinfo_pkginfo_get_type(pkghandle, &type)) {
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
		if (PMINFO_R_OK != pkgmgrinfo_appinfo_get_icon(appinfo_h, &icon_path_tmp)) {
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
		if (icon_path) {
			free(icon_path);
			icon_path = NULL;
		}

		elm_object_part_content_set(slot, "icon", icon);

		elm_object_signal_callback_del(item->layout, signal, "*", _slot_mouse_clicked_cb);

		elm_object_signal_callback_add(item->layout, signal, "*", _slot_mouse_clicked_cb, item->obj[pos]);

		elm_object_signal_callback_add(slot, "complete,launch_effect", "event", _glow_effect_done, slot);

		if (appinfo_h) pkgmgrinfo_appinfo_destroy_appinfo(appinfo_h);
		if (pkghandle) pkgmgrinfo_pkginfo_destroy_pkginfo(pkghandle);

		char log[10] = {0};
		snprintf(log, sizeof(log)-1, "ASS%d", pos+1);

		_D("slot is added %d", pos+1);

	}
	_EXIT;
	return;

}

PUBLIC void widget_update_content(struct info *item)
{
	_ENTER;
	int i = 0;
	char *tmp = NULL;
	char *first = NULL;
	char* save = NULL;
	int ret = 0;
	if (!item || !item->content) {
		_E("item is null");
		return;
	}
	tmp = strdup(item->content);
	if (ret != WIDGET_ERROR_NONE) {
		_E("set content error %x", ret);
	}

	for (i = 0 ; i < 4 ; i++) {
		if (i == 0) {
			first = strtok_r(tmp, " ", &save);
			_set_app_slot(item, first, i);
		} else {
			_set_app_slot(item, strtok_r(NULL, " ", &save), i);
		}
	}
	if (item->content_bundle) {
		_D("free the existing bundle");
		bundle_free(item->content_bundle);
	}
	item->content_bundle = bundle_create();
	ret = bundle_add_str(item->content_bundle, BUNDLE_ID_KEY, item->id);
	_D("bundle add str BUNDLE_ID_KEY ret:%d", ret);
	ret = bundle_add_str(item->content_bundle, BUNDLE_CONTENT_KEY, item->content);
	_D("bundle add str BUNDLE_CONTENT_KEY ret:%d", ret);
	ret = widget_app_context_set_content_info(item->context, item->content_bundle);
	_D("widget_app_context_set_content_info ret:%d", ret);
}


static void _mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_ENTER;
	struct object_info *info = NULL;
	info = data;
	elm_object_signal_emit(info->obj, "pressed", "widget_plus");
}

static void _mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	_ENTER;
	struct object_info *info = NULL;
	info = data;
	elm_object_signal_emit(info->obj, "released", "widget_plus");
}

static void _mouse_clicked_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
	_ENTER;
	_D("slot clicked");
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
	_D("full_path:%s", full_path);
	ret = elm_layout_file_set(slot, full_path, "empty_slot");
	if (ret == EINA_FALSE) {
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
	_ENTER;
	Evas_Object *win = NULL;
	int ret;

	ret = widget_app_get_elm_win(context, &win);
	if (ret != WIDGET_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get window. err = %d", ret);
		return NULL;
	}
	evas_object_resize(win, w, h);
	evas_object_show(win);

	return win;
}

static int widget_resize(widget_context_h context, int w, int h, void *user_data)
{
	_ENTER;

	struct info *item = NULL;
	int ret;
	Evas_Object *layout = NULL;
	Evas_Object *bg = NULL;
	char *tmp = NULL;
	widget_app_context_get_tag(context, (void**)&item);
	if (!item) {
		 _E("id is invalid");
			return WIDGET_ERROR_NOT_EXIST;
	}

	if (item->first_loaded && w == item->w && h == item->h) {
		_E("no need to update");
		return WIDGET_ERROR_NONE;
	}
	item->first_loaded = 1;


	_D("WIDGET is resized\n");
	item->dbox_win = _create_win(context, w, h);
	if (!item->dbox_win) {
		_E("item dbox win is not found");
			return WIDGET_ERROR_FAULT;
	}

	_init_theme();


	bg = elm_layout_add(item->dbox_win);
	char full_path[PATH_MAX] = { 0, };
	_get_resource(EDJE_FILE, full_path, sizeof(full_path));
	_D("full_path:%s", full_path);
	ret = elm_layout_file_set(bg, full_path, "dbox_bg");

	if (ret == EINA_FALSE) {
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
	_D("full_path:%s", full_path);
	ret = elm_layout_file_set(layout, full_path, "layout");
	_D("layout added");
	if (ret == EINA_FALSE) {
		LOGE("failed to set layout");
		return WIDGET_ERROR_FAULT;
	}


	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_resize(layout, w, h);
	evas_object_show(layout);

	elm_object_part_content_set(bg, "bg_swallow", layout);

	evas_object_resize(item->dbox_win, w, h);
	evas_object_show(item->dbox_win);
	_D("win show");

	_D("empty dbox created");
	/* load index layout */
	item->obj[0] = _add_empty_slot(layout, 1, item);
	item->obj[1] = _add_empty_slot(layout, 2, item);
	item->obj[2] = _add_empty_slot(layout, 3, item);
	item->obj[3] = _add_empty_slot(layout, 4, item);
	_D("four slots added");

	if (!item->content) {
		_D("content is null, so load default app order");
		item->content = strdup(DEFAULT_APP_ORDER);
	} else {
		_D("content is not null, contnet: %s", item->content);
	}

	tmp = strdup(item->content);
	char *first = NULL;
	char* save = NULL;
	int i = 0;

	for (i = 0 ; i < 4 ; i++) {
		if (i == 0) {
			first = strtok_r(tmp, " ", &save);
			_set_app_slot(item, first, i);
		} else {
			_set_app_slot(item, strtok_r(NULL, " ", &save), i);
		}
	}

	_D("widget resized to %dx%d\n", w, h);
	item->w = w;
	item->h = h;
	 return WIDGET_ERROR_NONE;
}

// NOTE: This function is going to be invoked for initializing all resources
static int widget_create(widget_context_h context, bundle *content, int w, int h, void *user_data)
{
	_ENTER;
	struct info *info;
	int ret = 0;

	info = malloc(sizeof(*info));
	if (!info) {
			return WIDGET_ERROR_OUT_OF_MEMORY;
	}
	_D("test1");
	info->context = context;
	info->content = NULL;
	info->content_bundle = NULL;
	info->id = strdup(widget_app_get_id(context));
	if (content) {
		//retrieve the content from bundle and update to info->content.
		_D("content sent by w-home");
		info->content_bundle = bundle_dup(content);
		ret = bundle_del(info->content_bundle, BUNDLE_ID_KEY);
		_D("bundle_del ret:%d", ret);
		ret = bundle_add_str(info->content_bundle, BUNDLE_ID_KEY, info->id);
		_D("bundle add str BUNDLE_ID_KEY id:###%s### ret:%d", info->id, ret);
		ret = bundle_get_str(info->content_bundle, BUNDLE_CONTENT_KEY, &info->content);
		_D("bundle_get_str: content:%s ret:%d", info->content, ret);

		if (!info->content)	{
			_E("bundle get key failed, key is missing from the bundle");
		} else {
			_D("content: %s", info->content);
		}

	} else {
		_D("content is emtpy");
		info->content_bundle = bundle_create();
		if (!info->content_bundle) {
			_E("bundle creation fails");
			return WIDGET_ERROR_OUT_OF_MEMORY;
		}
		info->content = strdup(DEFAULT_APP_ORDER);
		ret = bundle_add_str(info->content_bundle, BUNDLE_ID_KEY, info->id);
		_D("bundle add str BUNDLE_ID_KEY id:###%s### len:%d ret:%d", info->id , strlen(info->id), ret);
		ret = bundle_add_str(info->content_bundle, BUNDLE_CONTENT_KEY, info->content);
		_D("bundle add str BUNDLE_CONTENT_KEY content:%s ret:%d", info->content, ret);

	}
	ret = widget_app_context_set_content_info(context, info->content_bundle);
	_D("widget_app_context_set_content_info ret:%d", ret);
	ret = widget_app_context_set_tag(context, info);
	_D("widget_app_context_set_tag ret:%d", ret);

	info->size_type = WIDGET_SIZE_TYPE_UNKNOWN;
	info->need_to_delete = 0;
	info->first_loaded = 0;

	widget_resize(context, w, h, user_data);

	return WIDGET_ERROR_NONE;
}


void _set_app_label(struct object_info *obj)
{
	_ENTER;
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
widget_pause(widget_context_h context, void *user_data)
{
	_ENTER;
	/* Take necessary actions when widget instance becomes invisible.
	*/
	struct info* item = NULL;
	widget_app_context_get_tag(context, (void**)&item);
	if (!item || !item->content) {
		 _E("id is invalid");
			return WIDGET_ERROR_NOT_EXIST;
	}
	int ret = 0;
	return WIDGET_ERROR_NONE;

}

static int
widget_resume(widget_context_h context, void *user_data)
{
	/* Take necessary actions when widget instance becomes visible. */
	_ENTER;
	return WIDGET_ERROR_NONE;
}

static int
widget_update(widget_context_h context, bundle *content, int force, void *user_data)
{
	_ENTER;
	/* Take necessary actions when widget instance should be updated. */
	struct info* item = NULL;
	int ret = 0;
	widget_app_context_get_tag(context, (void**)&item);
	if (!item) {
		 _E("id is invalid");
			return WIDGET_ERROR_NOT_EXIST;
	}
	if (content) {
		_D("received bundle");
		bundle_get_str(content, BUNDLE_CONTENT_KEY, &item->content);
		if (!item->content) {
			_E("bundle get key failed, key is missing from the bundle");
		} else {
			_D("content: %s", item->content);
		}
	} else {
		_D("bundle is empty");
	}

	widget_update_content(item);
	return WIDGET_ERROR_NONE;
}
static widget_class_h
widget_app_create(void *user_data)
{
	_ENTER;
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
		.create = (void *)widget_create,
		.destroy = (void *)widget_destroy,
		.pause = (void *)widget_pause,
		.resume = (void *)widget_resume,
		.update = (void *)widget_update,
		.resize = (void *)widget_resize,
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
	_ENTER;
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


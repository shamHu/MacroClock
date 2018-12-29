/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <math.h>
#include <watch_app_efl.h>
#include "macroclock.h"
#include "view.h"
#include "view_defines.h"

#define MAIN_EDJ "edje/main.edj"

static struct view_info {
	Evas_Object *win;
	Evas_Object *layout;
	Evas_Object *image;
	Evas_Object *label;
	Evas_Object *label2;
	int w;
	int h;
	int hour;
	int minute;
	char hourText[1];
	char hour2Text[1];
} s_info = {
	.win = NULL,
	.layout = NULL,
	.image = NULL,
	.label = NULL,
	.label2 = NULL,
	.w = 0,
	.h = 0,
	.hour = 0,
	.minute = 0,
	.hourText = "0",
	.hour2Text = "0"
};

static void _message_outside_object_cb(void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg);
//static char *_create_resource_path(const char *file_name);
//static Evas_Object *_create_layout(void);
static void _part_text_set(int val, const char *part);
static int hour_angle;
static int minute_angle;

/*
 * @brief Creates the application view with received size
 * @param width - Application width
 * @param height - Application height
 */
void view_create_with_size(int width, int height) {
	s_info.w = width;
	s_info.h = height;
	view_create();
}

/*
 * @brief: Create Essential Object window, conformant and layout
 */
void view_create(void) {
	/* Create window */
	s_info.win = view_create_win(PACKAGE);
	if (s_info.win == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create a window.");
		return;
	}

	/* Layout */
//	s_info.layout = _create_layout();
//	if (!s_info.layout) {
//		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create main layout.");
//		return;
//	}
	s_info.label = create_label(s_info.win);
	s_info.label2 = create_label(s_info.win);

//	sprintf(s_info.hourText, "%d", s_info.hour);

	evas_object_text_font_set(s_info.label, FONT, FONT_SIZE);
	evas_object_text_font_set(s_info.label2, FONT, FONT_SIZE);
	set_hour_label_text();
	set_hour_label_position();
	evas_object_move(s_info.label, (SCREEN_WIDTH / 2) - 20, (SCREEN_HEIGHT / 2) - 50);
	evas_object_move(s_info.label2, (SCREEN_WIDTH / 2) - 20, (SCREEN_HEIGHT / 2) - 50);
	evas_object_show(s_info.label);
	evas_object_show(s_info.label2);

	/* Show window after main view is set up */
	evas_object_show(s_info.win);
}

Evas_Object *create_label(Evas* window) {
	return evas_object_text_add(window);
}

/*
 * @brief: Make a basic window named package
 * @param[pkg_name]: Name of the window
 */
Evas_Object *view_create_win(const char *pkg_name) {
	Evas_Object *win = NULL;
	int ret;

	/*
	 * Window
	 * Create and initialize elm_win.
	 * elm_win is mandatory to manipulate window
	 */
	ret = watch_app_get_elm_win(&win);
	if (ret != APP_ERROR_NONE)
		return NULL;

	elm_win_title_set(win, pkg_name);
	elm_win_borderless_set(win, EINA_TRUE);
	elm_win_alpha_set(win, EINA_FALSE);
	elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_HIDE);
	elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_BG_TRANSPARENT);
	elm_win_prop_focus_skip_set(win, EINA_TRUE);
	elm_win_role_set(win, "no-effect");

	evas_object_resize(win, s_info.w, s_info.h);
	return win;
}

/*
 * @brief: Make and set a layout to the part
 * @param[parent]: Object to which you want to set this layout
 * @param[file_path]: File path of EDJ will be used
 * @param[group_name]: Group name in EDJ that you want to set to layout
 * @param[part_name]: Part name to which you want to set this layout
 */
Evas_Object *view_create_layout_for_part(Evas_Object *parent, char *file_path, char *group_name, char *part_name) {
	dlog_print(DLOG_INFO, LOG_TAG, "View Create Layout: %s, %s, %s", file_path, group_name, part_name);
	Evas_Object *layout = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return NULL;
	}

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, file_path, group_name);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);
	elm_object_part_content_set(parent, part_name, layout);
	edje_object_message_handler_set(elm_layout_edje_get(layout), _message_outside_object_cb, NULL);

	return layout;
}

/*
 * @brief: Destroy window and free important data to finish this application
 */
void view_destroy(void) {
	if (s_info.win == NULL)
		return;

	evas_object_del(s_info.win);
}

/*
 * @brief Stores the hour value
 * @param hour - New hour value
 */
void view_set_hour(int hour) {
	s_info.hour = hour;
}

/*
 * @brief Stores the minute value
 * @param minute
 */
void view_set_minute(int minute) {
	s_info.minute = minute;
}

/*
 * @brief Updates the second hand edje part state
 * @param second - Current second
 */
void view_set_second(int second) {
//	Edje_Message_Int msg = {0,};
//
//	Evas_Object *edje_obj = elm_layout_edje_get(s_info.layout);
//	if (!edje_obj) {
//		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] edje_obj == NULL", __FILE__, __LINE__);
//		return;
//	}
//
//	msg.val = second;
//	edje_object_message_send(edje_obj, EDJE_MESSAGE_INT, MSG_ID_SECOND_PART, &msg);
}

/*
 * @brief Begins the minute change animation
 */
void view_start_minute_update(void) {
	elm_layout_signal_emit(s_info.layout, SIGNAL_MINUTE_CHANGE_ANIM_START, "");
}

/*
 * @brief Updates the hour and minute values displayed by the edje layout
 * @param hour - New hour value
 * @param minute - New minute value
 */
void view_update_display_time(int hour, int minute) {
	dlog_print(DLOG_INFO, LOG_TAG, "updating view time: %d:%d", hour, minute);

	hour_angle = (hour % 12) * 30;
	minute_angle = (((hour % 12) * 60) + minute) / 2;

//	if (minute == 0) {
		set_hour_label_text();
//	}

	set_hour_label_position();
}

/*
 * @brief Sets the edje layout's ambient mode
 * @param ambient - Ambient mode state
 */
void view_set_ambient_mode(Eina_Bool ambient) {
	if (ambient)
		elm_layout_signal_emit(s_info.layout, SIGNAL_SECOND_STATE_HIDDEN, "");
	else
		elm_layout_signal_emit(s_info.layout, SIGNAL_SECOND_STATE_VISIBLE, "");
}

/*
 * @brief Sets the hour value displayed by the edje layout
 * @param hour - Hour value
 */
void view_reset_display_state(void) {
	elm_layout_signal_emit(s_info.layout, "load", "");
}


/*
 * @brief Callback invoked when a message from the edje is received. Used to update the value of the last 'minute' part
 * @param data - User data
 * @param obj - Edje object which sent the message
 * @param type - The message type
 * @param id - The message id
 * @param msg - The message value
 */
static void _message_outside_object_cb(void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg) {
	Edje_Message_String *edje_msg = NULL;

	if (type != EDJE_MESSAGE_STRING || id != MSG_ID_MINUTE_PART_NEW_OUTSIDE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] Unknown message", __FILE__, __LINE__);
		return;
	}

	edje_msg = (Edje_Message_String *)msg;
	_part_text_set((s_info.minute + 2) % 60, edje_msg->str);
}

/*
 * @brief Sets a number to be displayed by the given edje part
 * @param val - Value to display
 * @param part - Part to update
 */
static void _part_text_set(int val, const char *part) {
	Eina_Stringshare *str = eina_stringshare_printf("%.2d", val);
	elm_layout_text_set(s_info.layout, part, str);
	eina_stringshare_del(str);
}

void set_hour_label_text() {
	int convertedHour = s_info.hour % 12;
	int convertedHour2 = (s_info.hour + 1) % 12;

	if (convertedHour == 0) {
		convertedHour = 12;
	} else if (convertedHour2 == 0) {
		convertedHour2 = 12;
	}

	sprintf(s_info.hourText, "%d", convertedHour);
	sprintf(s_info.hour2Text, "%d", convertedHour2);
	evas_object_text_text_set(s_info.label, s_info.hourText);
	evas_object_text_text_set(s_info.label2, s_info.hour2Text);
}

static double getCos(double angle) {
	return (double) cos(angle);
}

static double getSin(double angle) {
	return (double) sin(angle);
}

void set_hour_label_position() {
	double hour_angle_adj_rad = -((double) hour_angle * 3.14 / 180) + (3.14 / 2);
	double minute_angle_adj_rad = -((double) minute_angle * M_PI / 180) + (M_PI / 2);

	if (hour_angle_adj_rad > (M_PI / 2) && hour_angle_adj_rad < (3 * M_PI / 2)) {
		hour_angle_adj_rad += (2 * M_PI);
	}

	if (minute_angle_adj_rad > (M_PI / 2) && minute_angle_adj_rad < (3 * M_PI / 2)) {
		minute_angle_adj_rad += (2 * M_PI);
	}

	double timeX = getCos(minute_angle_adj_rad) * (double) RADIUS;
	double timeY = getSin(minute_angle_adj_rad) * (double) RADIUS;
	double hourX = getCos(hour_angle_adj_rad) * (double) RADIUS;
	double hourY = getSin(hour_angle_adj_rad) * (double) RADIUS;

	int xPos = -(timeX - hourX) + (SCREEN_WIDTH / 2) - LABEL_X_OFFSET_SMALL;
	int yPos = -(timeY - hourY) + (SCREEN_HEIGHT / 2) - LABEL_Y_OFFSET;

	dlog_print(DLOG_INFO, LOG_TAG, "hour_angle:%d, hour_angle_adj_rad:%f, timeX:%f, hourX:%f, xPos=%d", hour_angle, hour_angle_adj_rad, timeX, hourX, xPos);
	dlog_print(DLOG_INFO, LOG_TAG, "minute_angle:%d, minute_angle_adj_rad:%f, timeY:%f, hourY:%f, yPos=%d", minute_angle, minute_angle_adj_rad, timeY, hourY, yPos);

	double hourX2 = getCos(hour_angle_adj_rad - (0.5236)) * RADIUS;
	double hourY2 = getSin(hour_angle_adj_rad - (0.5236)) * RADIUS;

	int xPos2 = -(timeX - hourX2) + (SCREEN_WIDTH / 2) - LABEL_X_OFFSET_SMALL;
	int yPos2 = -(timeY - hourY2) + (SCREEN_HEIGHT / 2) - LABEL_Y_OFFSET;

	evas_object_move(s_info.label, xPos, yPos);
	evas_object_move(s_info.label2, xPos2, yPos2);
}

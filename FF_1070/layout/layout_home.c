#include "layout_define.h"
#include "layout_common.h"
typedef enum
{
	HOME_TIME_OBJ_ID,
	HOME_MEDIA_OBJ_ID,
	HOME_INTERCOM_OBJ_ID,
	HOME_DISPLAY_OBJ_ID,
	HOME_INTIALIZE_OBJ_ID,
	HOME_SETTING_OBJ_ID,
	HOME_STANDBY_OBJ_ID,
	HOME_TOTAL_BTN,
} home_btn_module;

#define HOME_GEAR_OBJ_ID 10

static custom_area home_btn_area[HOME_TOTAL_BTN] =
	{
		{158, 135, 116, 116},
		{454, 135, 116, 116},
		{750, 135, 116, 116},
		{158, 361, 116, 116},
		{454, 361, 116, 116},
		{750, 361, 116, 116},
		{940,  29,  30,  30},	
	};

// static int new_media_check_count = 0;
// static bool backlight_enable_flag = false;

// static bool manual_enter_monitor = false;
// bool manual_enter_monitor_get(void)
// {
// 	return manual_enter_monitor;
// }

// void manual_enter_monitor_set(bool en)
// {
// 	manual_enter_monitor = en;
// }
// // 在活动屏幕上显示背景图片
lv_obj_t *common_bg_display(lv_obj_t *parent)
{
	static rom_bin_info info = rom_bin_info_get(ROM_UI_BG_BG_PNG);
	lv_obj_set_style_local_pattern_image(parent, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
	return parent;
}
/* 图标按键创建 */
lv_obj_t *common_img_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src)
{
	lv_obj_t *btn_obj = lv_obj_create(parent, NULL);
	lv_obj_set_click(btn_obj, false);
	lv_obj_set_pos(btn_obj, btn_area.x, btn_area.y);
	lv_obj_set_size(btn_obj, btn_area.w, btn_area.h);
	if (icon_src != NULL)
	{
		lv_obj_set_style_local_pattern_image(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
	}

	lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_0);
	lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);

	if (string != NULL)
	{
		lv_obj_set_style_local_value_str(btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, string);
		lv_obj_set_style_local_value_align(btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
		lv_obj_set_style_local_value_ofs_y(btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 4);
		lv_obj_set_style_local_value_font(btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
		lv_obj_set_style_local_value_blend_mode(btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_BLEND_MODE_ADDITIVE);
	}

	
	obj_click_event_listen(btn_obj, btn_pdata);
	return btn_obj;
}

/* time */
// lv_obj_t *home_time_img_btn_create(lv_obj_t *parent, int obj_id, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src)
// {
// 	lv_obj_t *time_btn_obj = lv_obj_create(parent, NULL);
// 	lv_obj_set_click(time_btn_obj, false);
// 	lv_obj_set_pos(time_btn_obj, btn_area.x, btn_area.y);
// 	lv_obj_set_size(time_btn_obj, btn_area.w, btn_area.h);
// 	if (icon_src != NULL)
// 	{
// 		lv_obj_set_style_local_pattern_image(time_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
// 	}

// 	if (obj_id > 0)
// 	{
// 		lv_obj_set_id(time_btn_obj, obj_id);
// 	}

// 	lv_obj_set_style_local_pattern_recolor(time_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
// 	lv_obj_set_style_local_pattern_recolor(time_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
// 	lv_obj_set_style_local_pattern_recolor_opa(time_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_0);
// 	lv_obj_set_style_local_pattern_recolor_opa(time_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);

// 	if (string != NULL)
// 	{
// 		lv_obj_set_style_local_value_str(time_btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, string);
// 		lv_obj_set_style_local_value_align(time_btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
// 		lv_obj_set_style_local_value_ofs_y(time_btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 4);
// 		lv_obj_set_style_local_value_font(time_btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
// 		lv_obj_set_style_local_value_blend_mode(time_btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_BLEND_MODE_ADDITIVE);
// 	}

// 	obj_click_event_listen(time_btn_obj, btn_pdata);

// 	return time_btn_obj;
// }

// /* intercom */
// lv_obj_t *home_media_img_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src, bool underline, bool string_select)
// {
// 	lv_obj_t *media_btn_obj = lv_obj_create(parent, NULL);
// 	lv_obj_set_ext_click_area(media_btn_obj, 12, 12, 10, 15);
// 	lv_obj_set_pos(media_btn_obj, btn_area.x, btn_area.y);
// 	lv_obj_set_size(media_btn_obj, btn_area.w, btn_area.h);
// 	if (icon_src != NULL)
// 	{
// 		lv_obj_set_style_local_pattern_image(media_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
// 		lv_obj_set_style_local_pattern_align(media_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
// 	}

// 	lv_obj_set_style_local_pattern_recolor(media_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
// 	lv_obj_set_style_local_pattern_recolor(media_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));

// 	// 2. 叠加透明度：默认低透明（图标显原始色），按下高透明（图标变深色）
// 	lv_obj_set_style_local_pattern_recolor_opa(media_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);	// 默认无叠加（原始色）
// 	lv_obj_set_style_local_pattern_recolor_opa(media_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50); // 按下叠加50%黑色（深色）

// 	if (string != NULL)
// 	{
// 		lv_obj_set_style_local_value_str(media_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string);
// 		lv_obj_set_style_local_value_align(media_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_BOTTOM_MID);
// 		if (string_select)
// 			lv_obj_set_style_local_value_color(media_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xC52323));
// 		else
// 			lv_obj_set_style_local_value_color(media_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 		lv_obj_set_style_local_value_ofs_y(media_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
// 		lv_obj_set_style_local_value_font(media_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
// 	}

// 	obj_click_event_listen(media_btn_obj, btn_pdata);

// 	return media_btn_obj;
// }

// /* display */
// lv_obj_t *home_display_img_btn_create(lv_obj_t *parent, int obj_id, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src)
// {
// 	lv_obj_t *display_btn_obj = lv_obj_create(parent, NULL);
// 	lv_obj_set_click(display_btn_obj, false);
// 	lv_obj_set_pos(display_btn_obj, btn_area.x, btn_area.y);
// 	lv_obj_set_size(display_btn_obj, btn_area.w, btn_area.h);
// 	if (icon_src != NULL)
// 	{
// 		lv_obj_set_style_local_pattern_image(display_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
// 	}

// 	if (obj_id > 0)
// 	{
// 		lv_obj_set_id(display_btn_obj, obj_id);
// 	}

// 	lv_obj_set_style_local_pattern_recolor(display_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
// 	lv_obj_set_style_local_pattern_recolor(display_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
// 	lv_obj_set_style_local_pattern_recolor_opa(display_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_0);
// 	lv_obj_set_style_local_pattern_recolor_opa(display_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);

// 	if (string != NULL)
// 	{
// 		lv_obj_set_style_local_value_str(display_btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, string);
// 		lv_obj_set_style_local_value_align(display_btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
// 		lv_obj_set_style_local_value_ofs_y(display_btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 4);
// 		lv_obj_set_style_local_value_font(display_btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
// 		lv_obj_set_style_local_value_blend_mode(display_btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_BLEND_MODE_ADDITIVE);
// 	}

// 	obj_click_event_listen(display_btn_obj, btn_pdata);

// 	return display_btn_obj;
// }

// lv_obj_t *home_initalize_img_btn_create(lv_obj_t *parent, int obj_id, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src)
// {
// 	lv_obj_t *initalize_btn_obj = lv_obj_create(parent, NULL);
// 	lv_obj_set_click(initalize_btn_obj, false);
// 	lv_obj_set_pos(initalize_btn_obj, btn_area.x, btn_area.y);
// 	lv_obj_set_size(initalize_btn_obj, btn_area.w, btn_area.h);
// 	if (icon_src != NULL)
// 	{
// 		lv_obj_set_style_local_pattern_image(initalize_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
// 	}

// 	if (obj_id > 0)
// 	{
// 		lv_obj_set_id(initalize_btn_obj, obj_id);
// 	}

// 	lv_obj_set_style_local_pattern_recolor(initalize_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
// 	lv_obj_set_style_local_pattern_recolor(initalize_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
// 	lv_obj_set_style_local_pattern_recolor_opa(initalize_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_0);
// 	lv_obj_set_style_local_pattern_recolor_opa(initalize_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);

// 	if (string != NULL)
// 	{
// 		lv_obj_set_style_local_value_str(initalize_btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, string);
// 		lv_obj_set_style_local_value_align(initalize_btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
// 		lv_obj_set_style_local_value_ofs_y(initalize_btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 4);
// 		lv_obj_set_style_local_value_font(initalize_btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
// 		lv_obj_set_style_local_value_blend_mode(initalize_btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_BLEND_MODE_ADDITIVE);
// 	}

// 	obj_click_event_listen(initalize_btn_obj, btn_pdata);

// 	return initalize_btn_obj;
// }

/* setting */
// lv_obj_t *home_setting_img_btn_create(lv_obj_t *parent, int obj_id, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src)
// {
// 	lv_obj_t *settting_btn_obj = lv_obj_create(parent, NULL);
// 	lv_obj_set_click(settting_btn_obj, false);
// 	lv_obj_set_pos(settting_btn_obj, btn_area.x, btn_area.y);
// 	lv_obj_set_size(settting_btn_obj, btn_area.w, btn_area.h);
// 	if (icon_src != NULL)
// 	{
// 		lv_obj_set_style_local_pattern_image(settting_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
// 	}

// 	if (obj_id > 0)
// 	{
// 		lv_obj_set_id(settting_btn_obj, obj_id);
// 	}

// 	lv_obj_set_style_local_pattern_recolor(settting_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
// 	lv_obj_set_style_local_pattern_recolor(settting_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
// 	lv_obj_set_style_local_pattern_recolor_opa(settting_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_0);
// 	lv_obj_set_style_local_pattern_recolor_opa(settting_btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);

// 	if (string != NULL)
// 	{
// 		lv_obj_set_style_local_value_str(settting_btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, string);
// 		lv_obj_set_style_local_value_align(settting_btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
// 		lv_obj_set_style_local_value_ofs_y(settting_btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 4);
// 		lv_obj_set_style_local_value_font(settting_btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
// 		lv_obj_set_style_local_value_blend_mode(settting_btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_BLEND_MODE_ADDITIVE);
// 	}

// 	obj_click_event_listen(settting_btn_obj, btn_pdata);

// 	return settting_btn_obj;
// }

/* setting 图标 */
static void setting_icon_create(lv_obj_t *parent)
{
    lv_obj_t *setting_icon_obj = lv_img_create(parent, NULL);
    lv_obj_set_pos(setting_icon_obj, 54, 34);
	lv_obj_set_size(setting_icon_obj, 22, 20);
	lv_obj_set_id(setting_icon_obj, HOME_GEAR_OBJ_ID);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_GEAR_PNG);
	lv_obj_set_style_local_pattern_image(setting_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);

    lv_obj_t *Time_label = lv_label_create(parent, NULL);
    lv_obj_set_pos(Time_label, 87, 28);
	lv_obj_set_size(Time_label, 70, 31);
    lv_label_set_text(Time_label, "Menu");
	lv_obj_align(setting_icon_obj, Time_label, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}


static void home_time_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(time));
}

static void home_media_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(photo_list));
}

static void home_intercom_btn_up(lv_obj_t *obj)
{
	 goto_layout(pLAYOUT(intercom));
}

static void home_monitor_btn_up(lv_obj_t *obj)
{
	monitor_channel_set(MON_CH_DOOR1);
	monitor_enter_mask_set(MON_ENTER_MANUAL_DOOR);
	goto_layout(pLAYOUT(camera));
}

static void home_initialize_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(init));
}

static void home_setting_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting));
}

static void home_standby_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(standby));
}

static void home_time_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_time_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_TIME_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_TIME_OBJ_ID], str_get(LAYOUT_SETTING_LANG_TIME_ID), &btn_data, &info);
}

static void home_media_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_media_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_MEDIA_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_MEDIA_OBJ_ID], str_get(COMMON_LANG_HOME_MEDIA), &btn_data, &info);
}

static void home_intercom_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_intercom_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_INTERCOM_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_INTERCOM_OBJ_ID], str_get(COMMON_LANG_INTERCOM_ID), &btn_data, &info);
}

static void home_monitor_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_monitor_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_MONITOR_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_DISPLAY_OBJ_ID], str_get(LAYOUT_HOME_LANG_MINOTOR_ID), &btn_data, &info);
}

static void home_initialize_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_initialize_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_INITIALIZE_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_INTIALIZE_OBJ_ID], str_get(LAYOUT_SETTING_LANG_FORMAT_ID), &btn_data, &info);
}

static void home_setting_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_setting_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_SETTING_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_SETTING_OBJ_ID], str_get(COMMON_LANG_SETTING_ID), &btn_data, &info);
}

static void home_standby_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(home_standby_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_HOME_STANDBY_PNG);
	common_img_btn_create(parent, home_btn_area[HOME_STANDBY_OBJ_ID], NULL, &btn_data, &info);
}

static void LAYOUT_ENTER_FUNC(home)
{

	power_amplifier_enable(true);
	lv_obj_t *parent = common_bg_display(lv_scr_act());
	home_time_btn_create(parent);
	home_media_btn_create(parent);	
	home_intercom_btn_create(parent);
	home_monitor_btn_create(parent);
	home_initialize_btn_create(parent);
	home_setting_btn_create(parent);
	home_standby_btn_create(parent);
	setting_icon_create(parent);
	top_time_date_text_create(parent);
}

static void LAYOUT_QUIT_FUNC(home)
{
	
	lyaout_sd_state_callback_register(layout_sdcard_state_change_default);
	user_data_save();
}

CREATE_LAYOUT(home);

























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
#define HOME_BACKLIGHT_DELAY_MS 300

static lv_task_t *home_backlight_task = NULL;
static lv_obj_t *home_media_btn = NULL;

static custom_area home_btn_area[HOME_TOTAL_BTN] =
	{
		{158, 135, 116, 116},
		{454, 135, 116, 116},
		{750, 135, 116, 116},
		{158, 361, 116, 116},
		{454, 361, 116, 116},
		{750, 361, 116, 116},
		{940,  29,  40,  40},	
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
	lv_obj_set_ext_click_area(btn_obj, 10, 10, 10, 10);
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
    lv_label_set_text(Time_label,str_get(COMMON_LANG_LEFT_HEAD_MENU_ID));
	lv_obj_align(setting_icon_obj, Time_label, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}


static void home_click_down_func(lv_obj_t *obj)
{
	/* Media entry key tone: keep PA warm without restarting AO on click. */
	if (obj == home_media_btn)
	{
		layout_media_keytone_prepare();
		layout_obj_click_down_func(obj);
		return;
	}

	layout_obj_click_down_func(obj);
}

static void home_time_btn_up(lv_obj_t *obj)
{
	if (home_backlight_task != NULL)
	{
		return;
	}
	goto_layout(pLAYOUT(time));
}

static void home_media_btn_up(lv_obj_t *obj)
{
	if (home_backlight_task != NULL)
	{
		return;
	}
	goto_layout(pLAYOUT(photo_list));
}

static void home_intercom_btn_up(lv_obj_t *obj)
{
	if (home_backlight_task != NULL)
	{
		return;
	}
	goto_layout(pLAYOUT(intercom));
}

static void home_monitor_btn_up(lv_obj_t *obj)
{
	if (home_backlight_task != NULL)
	{
		return;
	}
	monitor_channel_set(MON_CH_DOOR1);
	monitor_enter_mask_set(MON_ENTER_MANUAL_DOOR);
	goto_layout(pLAYOUT(camera));
}

static void home_initialize_btn_up(lv_obj_t *obj)
{
	if (home_backlight_task != NULL)
	{
		return;
	}
	goto_layout(pLAYOUT(init));
}

static void home_setting_btn_up(lv_obj_t *obj)
{
	if (home_backlight_task != NULL)
	{
		return;
	}
	goto_layout(pLAYOUT(setting));
}

static void home_standby_btn_up(lv_obj_t *obj)
{
	if (home_backlight_task != NULL)
	{
		return;
	}
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
	home_media_btn = common_img_btn_create(parent, home_btn_area[HOME_MEDIA_OBJ_ID], str_get(COMMON_LANG_HOME_MEDIA), &btn_data, &info);
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

static void home_backlight_task_cb(lv_task_t *task)
{
	home_backlight_task = NULL;
	backlight_enable(true);
	lv_task_del(task);
}

static void home_backlight_task_create(void)
{
	if (home_backlight_task != NULL)
	{
		lv_task_del(home_backlight_task);
		home_backlight_task = NULL;
	}

	home_backlight_task = lv_layout_task_create(home_backlight_task_cb,
												HOME_BACKLIGHT_DELAY_MS,
												LV_TASK_PRIO_LOW,
												NULL);
	home_backlight_task->clean_lock = false;
}

static void LAYOUT_ENTER_FUNC(home)
{
	printf("Entering home layout.\n");
	
	// power_amplifier_enable(true);   lynn 26.3.10
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
	lv_obj_click_down_callback_register(home_click_down_func);
	home_backlight_task_create();
}

static void LAYOUT_QUIT_FUNC(home)
{
	if (home_backlight_task != NULL)
	{
		lv_task_del(home_backlight_task);
		home_backlight_task = NULL;
	}
	lv_obj_click_down_callback_register(layout_obj_click_down_func);
	home_media_btn = NULL;
	
	layout_sd_state_callback_register(layout_sdcard_state_change_default);
	user_data_save();
}

CREATE_LAYOUT(home);


















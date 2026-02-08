#include "layout_define.h"

typedef enum
{
	INTERCOM_IN_HOME_BTN_ID,
	INTERCOM_IN_SOUND_BTN_ID,

	INTERCOM_IN_TOTAL_BTN,
} intercom_in_btn_module;

static custom_area intercom_in_btn_area[INTERCOM_IN_TOTAL_BTN] =
	{
		{24, 12, 54, 54},	// 返回按钮
		{892, 461, 80, 80}, // 声音按钮位置
};

#define INTERCOM_IN_TEXT_LABEL_ID 0
#define INTERCOM_IN_RING_CONT_ID 1
#define INTERCOM_IN_RING_BAR_ID 2
#define INTERCOM_IN_VOL_SLIDER_ID 4
#define INTERCOM_IN_VOL_LABEL_ID 5
#define INTERCOM_IN_COUNTDOWN_LABEL_ID 6
#define INTERCOM_IN_COUNTDOWN_RING_ID 7
#define INTERCOM_IN_LEFT_ARROW_ID 8
#define INTERCOM_IN_RIGHT_ARROW_ID 9
#define INTERCOM_IN_CALL_FROM_LABEL_ID 10
#define INTERCOM_IN_ROOM_NUMBER_LABEL_ID 11
#define INTERCOM_IN_VOL_SLIDER_BG_ID 12
#define INTERCOM_IN_VOL_SLIDER_INDICATOR_ID 13
#define INTERCOM_IN_SOUND_BTN_OBJ_ID 14

static lv_task_t *hung_up_task = NULL;
static lv_task_t *calling_task = NULL;

static int count = 30; // 30秒倒计时
static bool vol_slider_visible = false;

static void intercom_in_hung_up_task_create(void);

static void intercom_in_back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(intercom));
}

static void intercom_in_back_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(intercom_in_back_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_HOME_PNG);
	common_img_btn_create(parent, intercom_in_btn_area[INTERCOM_IN_HOME_BTN_ID], NULL, &btn_data, &info);
}

// 更新声音按钮图片
static void update_sound_btn_image(void)
{
	lv_obj_t *sound_btn = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_SOUND_BTN_OBJ_ID);
	if (sound_btn)
	{
		static rom_bin_info sound_info = rom_bin_info_get(ROM_UI_INTERCOM_SOUND_PNG);
		static rom_bin_info mute_info = rom_bin_info_get(ROM_UI_INTERCOM_MUTE_PNG);

		if (user_data_get()->setting.inter_ring_volume > 0)
		{
			lv_obj_set_style_local_pattern_image(sound_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &sound_info);
		}
		else
		{
			lv_obj_set_style_local_pattern_image(sound_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &mute_info);
		}
	}
}

// 音量滑块事件回调
static void vol_slider_event_cb(lv_obj_t *slider, lv_event_t event)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		int vol = lv_slider_get_value(slider);
		user_data_get()->setting.inter_ring_volume = vol;

		// 更新音量标签
		lv_obj_t *vol_label = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_VOL_LABEL_ID);
		if (vol_label)
		{
			lv_label_set_text_fmt(vol_label, "%d", vol);
		}

		// 更新声音按钮图片
		update_sound_btn_image();

		// 更新滑块指示器高度和位置 - 从底部开始向上增长
		lv_obj_t *slider_indicator = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_VOL_SLIDER_INDICATOR_ID);
		if (slider_indicator)
		{
			// 根据音量值计算指示器高度 (0-4对应0-219高度)
			int indicator_height = (vol * 219) / 4;
			lv_obj_set_height(slider_indicator, indicator_height);
			// 指示器位置从底部开始：背景底部Y = 168 + 219 = 387，指示器Y = 387 - 指示器高度
			lv_obj_set_pos(slider_indicator, 925, 387 - indicator_height);
		}

		ring_volume_set(vol);
	}
}

// 创建音量滑块和标签
static void intercom_in_volume_slider_create(lv_obj_t *parent)
{
	// 创建滑块背景 - 灰色背景，整个219高度
	lv_obj_t *slider_bg = lv_obj_create(parent, NULL);
	lv_obj_set_id(slider_bg, INTERCOM_IN_VOL_SLIDER_BG_ID);
	lv_obj_set_size(slider_bg, 14, 219);
	lv_obj_set_pos(slider_bg, 925, 168);
	lv_obj_set_style_local_bg_color(slider_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x303030));
	lv_obj_set_style_local_radius(slider_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 62);
	lv_obj_set_style_local_bg_opa(slider_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

	// 创建滑块指示器 - 蓝色部分，从底部开始向上增长
	lv_obj_t *slider_indicator = lv_obj_create(parent, NULL);
	lv_obj_set_id(slider_indicator, INTERCOM_IN_VOL_SLIDER_INDICATOR_ID);

	// 根据当前音量计算指示器高度
	int current_vol = user_data_get()->setting.inter_ring_volume;
	int indicator_height = (current_vol * 219) / 4;

	lv_obj_set_size(slider_indicator, 14, indicator_height);
	// 指示器位置从底部开始：背景底部Y = 168 + 219 = 387，指示器Y = 387 - 指示器高度
	lv_obj_set_pos(slider_indicator, 925, 387 - indicator_height);
	lv_obj_set_style_local_bg_color(slider_indicator, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0A84FF));
	lv_obj_set_style_local_radius(slider_indicator, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 62);
	lv_obj_set_style_local_bg_opa(slider_indicator, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

	// 创建音量滑块（用于触摸交互）
	lv_obj_t *slider = lv_slider_create(parent, NULL);
	lv_obj_set_id(slider, INTERCOM_IN_VOL_SLIDER_ID);
	lv_obj_set_size(slider, 28, 219);
	lv_obj_set_pos(slider, 918, 168);
	lv_slider_set_range(slider, 0, 4);
	lv_slider_set_value(slider, current_vol, LV_ANIM_OFF);

	// 设置滑块背景和指示器为透明，因为我们有自己的背景和指示器
	lv_obj_set_style_local_bg_opa(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	// 设置滑块旋钮样式 - 圆形，白色
	lv_obj_set_style_local_radius(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_bg_opa(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_size(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 28);

	// 设置滑块为垂直方向
	lv_slider_set_anim_time(slider, 0); // 禁用动画
	lv_obj_set_event_cb(slider, vol_slider_event_cb);

	// 创建音量标签
	lv_obj_t *vol_label = lv_label_create(parent, NULL);
	lv_obj_set_id(vol_label, INTERCOM_IN_VOL_LABEL_ID);
	lv_obj_set_size(vol_label, 15, 28);
	lv_obj_set_pos(vol_label, 925, 397);
	lv_obj_set_style_local_text_font(vol_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_label_set_text_fmt(vol_label, "%d", current_vol);
	lv_label_set_align(vol_label, LV_LABEL_ALIGN_CENTER);

	// 初始显示音量滑块
	lv_obj_set_hidden(slider_bg, false);
	lv_obj_set_hidden(slider_indicator, false);
	lv_obj_set_hidden(slider, false);
	lv_obj_set_hidden(vol_label, false);
	vol_slider_visible = true;
}

// 声音按钮点击事件 - 显示/隐藏音量滑块
static void intercom_in_sound_btn_up(lv_obj_t *obj)
{
	lv_obj_t *slider_bg = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_VOL_SLIDER_BG_ID);
	lv_obj_t *slider_indicator = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_VOL_SLIDER_INDICATOR_ID);
	lv_obj_t *slider = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_VOL_SLIDER_ID);
	lv_obj_t *vol_label = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_VOL_LABEL_ID);

	if (slider_bg && slider_indicator && slider && vol_label)
	{
		vol_slider_visible = !vol_slider_visible;
		lv_obj_set_hidden(slider_bg, !vol_slider_visible);
		lv_obj_set_hidden(slider_indicator, !vol_slider_visible);
		lv_obj_set_hidden(slider, !vol_slider_visible);
		lv_obj_set_hidden(vol_label, !vol_slider_visible);
	}
}

static void intercom_in_sound_btn_create(lv_obj_t *parent)
{
	// 设置按钮样式
	static obj_click_data btn_data = obj_click_data_up_create(intercom_in_sound_btn_up);
	lv_obj_t *btn = common_img_btn_create(parent, intercom_in_btn_area[INTERCOM_IN_SOUND_BTN_ID], NULL, &btn_data, NULL);
	lv_obj_set_id(btn, INTERCOM_IN_SOUND_BTN_OBJ_ID);
	// 根据当前音量设置按钮图片
	update_sound_btn_image();
}

static void intercom_in_hung_up_btn_up(lv_obj_t *obj)
{
	if (hung_up_task == NULL)
	{
		intercom_state_set(INTERCOM_STATE_HUNG_UP);
		intercom_in_hung_up_task_create();
	}
	if (calling_task != NULL)
	{
		lv_task_del(calling_task);
		calling_task = NULL;
	}
}

// 创建中心倒计时环和标签
static void intercom_in_countdown_create(lv_obj_t *parent)
{
	// 创建倒计时环
	lv_obj_t *countdown_ring = lv_obj_create(parent, NULL);
	lv_obj_set_size(countdown_ring, 270, 270);
	lv_obj_set_pos(countdown_ring, 376, 151);
	static rom_bin_info ring_info = rom_bin_info_get(ROM_UI_INTERCOM_COUNTDOWN_RING_PNG);
	lv_obj_set_style_local_pattern_image(countdown_ring, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &ring_info);

	// 创建倒计时标签 - 立即显示30S
	lv_obj_t *countdown_label = lv_label_create(parent, NULL);
	lv_obj_set_id(countdown_label, INTERCOM_IN_COUNTDOWN_LABEL_ID);
	lv_obj_set_size(countdown_label, 110, 77);
	lv_obj_set_pos(countdown_label, 456, 248);

	// 设置字体样式
	lv_obj_set_style_local_text_font(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(64));
	lv_obj_set_style_local_text_line_space(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_text_letter_space(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);

	// 立即设置为30S
	lv_label_set_text_fmt(countdown_label, "30S");
	lv_label_set_align(countdown_label, LV_LABEL_ALIGN_CENTER);
}

// 创建左右箭头和标签
static void intercom_in_arrows_create(lv_obj_t *parent)
{
	// 左侧箭头
	lv_obj_t *left_arrow = lv_obj_create(parent, NULL);
	lv_obj_set_id(left_arrow, INTERCOM_IN_LEFT_ARROW_ID);
	lv_obj_set_size(left_arrow, 52, 32);
	lv_obj_set_pos(left_arrow, 296, 270);
	static rom_bin_info left_arrow_info = rom_bin_info_get(ROM_UI_INTERCOM_POINT_PNG);
	lv_obj_set_style_local_pattern_image(left_arrow, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &left_arrow_info);

	// 左侧提示语
	lv_obj_t *call_from_label = lv_label_create(parent, NULL);
	lv_obj_set_id(call_from_label, INTERCOM_IN_CALL_FROM_LABEL_ID);
	lv_obj_set_size(call_from_label, 181, 39);
	lv_obj_set_pos(call_from_label, 101, 266);
	lv_obj_set_style_local_text_font(call_from_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(32));
	lv_label_set_text(call_from_label, str_get(LAYOUT_INTERCOM_LANG_RINGING_FROM_ID));

	// 右侧箭头
	lv_obj_t *right_arrow = lv_obj_create(parent, NULL);
	lv_obj_set_id(right_arrow, INTERCOM_IN_RIGHT_ARROW_ID);
	lv_obj_set_size(right_arrow, 52, 32);
	lv_obj_set_pos(right_arrow, 674, 271);
	static rom_bin_info right_arrow_info = rom_bin_info_get(ROM_UI_INTERCOM_POINT_PNG);
	lv_obj_set_style_local_pattern_image(right_arrow, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &right_arrow_info);

	// 右侧房号
	lv_obj_t *room_label = lv_label_create(parent, NULL);
	lv_obj_set_id(room_label, INTERCOM_IN_ROOM_NUMBER_LABEL_ID);
	lv_obj_set_size(room_label, 50, 39);
	lv_obj_set_pos(room_label, 750, 266);
	lv_obj_set_style_local_text_font(room_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(32));

	int call_in_number = intercom_number_get();
	if ((call_in_number == GUARD_INTERCOM_NUMBER) || (call_in_number == GUARD_2_INTERCOM_NUMBER))
	{
		lv_label_set_text(room_label, call_in_number == GUARD_INTERCOM_NUMBER ? str_get(LAYOUT_INTERCOM_LANG_GUARD_1_ID) : str_get(LAYOUT_INTERCOM_LANG_GUARD_2_ID));
	}
	else
	{
		lv_label_set_text_fmt(room_label, "%03d", call_in_number);
	}
}

static void intercom_in_text_icon_create(lv_obj_t *parent)
{
	// 挂断按钮
	lv_obj_t *hung_up_btn = lv_obj_create(parent, NULL);
	lv_obj_set_size(hung_up_btn, 170, 76);
	lv_obj_set_pos(hung_up_btn, 427, 490);
	lv_obj_set_ext_click_area(hung_up_btn, 20, 20, 20, 20);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_INTERCOM_HUNG_UP_PNG);
	lv_obj_set_style_local_pattern_image(hung_up_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
	lv_obj_set_style_local_pattern_recolor(hung_up_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor(hung_up_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor_opa(hung_up_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_0);
	lv_obj_set_style_local_pattern_recolor_opa(hung_up_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
	static obj_click_data btn_data = obj_click_data_up_create(intercom_in_hung_up_btn_up);
	obj_click_event_listen(hung_up_btn, &btn_data);
}

static void intercom_in_hung_up_task(lv_task_t *task_t)
{
	goto_layout(pLAYOUT(intercom));
}

static void intercom_in_hung_up_task_create(void)
{
	lv_obj_t *countdown_label = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_COUNTDOWN_LABEL_ID);
	if (countdown_label != NULL)
	{
		lv_label_set_text(countdown_label, "00S");
	}

	hung_up_task = lv_layout_task_create(intercom_in_hung_up_task, 2000, LV_TASK_PRIO_LOW, NULL);
}

static void intercom_ring_play_start_fun(int index)
{
	ring_volume_set(user_data_get()->setting.inter_ring_volume);
}

static void intercom_ring_play_finish_fun(int index)
{
	power_amplifier_enable(false);
}

static void intercom_in_calling_time_out_task(lv_task_t *task_t)
{
	// 更新倒计时显示
	lv_obj_t *countdown_label = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_COUNTDOWN_LABEL_ID);
	if (countdown_label != NULL)
	{
		lv_label_set_text_fmt(countdown_label, "%02dS", count);
	}

	if (count % 3 == 0)
	{
		if (user_data_get()->setting.inter_ring_volume > 0)
		{
			ringplay_play_form_index(9, 100, intercom_ring_play_start_fun, intercom_ring_play_finish_fun, false);
		}
	}
	if (--count < 0 || intercom_state_get() == INTERCOM_STATE_IDLE)
	{
		if (calling_task != NULL)
		{
			lv_task_del(calling_task);
			calling_task = NULL;
		}
		intercom_state_set(INTERCOM_STATE_IDLE);
		intercom_in_hung_up_task_create();
		return;
	}
}

static void intercom_in_calling_time_out_task_create(void)
{
	count = 30; // 重置为30秒倒计时
	calling_task = lv_layout_task_create(intercom_in_calling_time_out_task, 1000, LV_TASK_PRIO_HIGH, NULL);
}

static void LAYOUT_ENTER_FUNC(intercom_in)
{
	power_amplifier_enable(true);
	standby_timer_close();
	backlight_enable(true);
	lv_obj_t *parent = common_bg_display(lv_scr_act());

	intercom_in_back_btn_create(parent);	  // 返回按钮
	intercom_in_sound_btn_create(parent);	  // 声音按钮
	intercom_in_text_icon_create(parent);	  // 挂断按钮
	intercom_in_volume_slider_create(parent); // 音量滑块
	intercom_in_countdown_create(parent);	  // 倒计时环和标签
	intercom_in_arrows_create(parent);		  // 左右箭头和标签

	lv_obj_click_down_callback_register(NULL);
	intercom_state_set(INTERCOM_STATE_CALLING_IN);
	intercom_in_calling_time_out_task_create();
}

static void LAYOUT_QUIT_FUNC(intercom_in)
{
	hung_up_task = NULL;
	calling_task = NULL;
	standby_timer_restart(true);
	lv_obj_click_down_callback_register(layout_obj_click_down_func);
	user_data_save();
	ringplay_play_stop();
}

CREATE_LAYOUT(intercom_in);
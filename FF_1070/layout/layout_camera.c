/*******************************************************************
 * @Descripttion   :
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-11 11:50
 * @LastEditTime   : 2023-03-15 16:32
 *******************************************************************/
#include "layout_define.h"

#define CAMERA_AUTO_RECORD_ENABLE 0 // 开启自动连续拍照、录像，测试用
#define CAMERA_DISPLAY_DELAY 20		// 监控显示延时（20*100ms=2秒）

enum
{
	RECORD_MODE_INIT = LAYOUT_SETTING_LANG_OFF_ID,
	RECORD_MODE_IMAGE = LAYOUT_RECORD_LANG_IMAGE_ID,
	RECORD_MODE_VIDEO = LAYOUT_RECORD_LANG_VIDEO_ID
};

typedef enum
{
	CAMERA_HOME_BTN_ID,
	CAMERA_SWITCH_BTN_ID,
	CAMERA_COLOR_BTN_ID,
	CAMERA_CAPTURE_BTN_ID,
	CAMERA_RECORD_BTN_ID,
	CAMERA_OPEN_BTN_ID,
	// CAMERA_ANSWER_BTN_ID,
	CAMERA_HANG_UP_BTN_ID,
	CAMERA_ADJUST_RESET_BTN_ID,
	CAMERA_TOTAL_BTN,
} camera_btn_module;

/***
** 日期: 2022-05-13 10:02
** 作者: leo.liu
** 函数作用：进入监控刷新区域
** 返回参数说明：
***/
static void layout_monitor_refresh_1(void)
{
	refresh_area_t area[] = {
		{0, 0, 1024, 70},
		{360, 80, 300, 60},
		{0, 470, 1024, 130}};
	gui_refresh_area(area, sizeof(area) / sizeof(refresh_area_t));
}

/***
** 日期: 2022-05-13 10:02
** 作者: leo.liu
** 函数作用：点击切换通道刷新区域
** 返回参数说明：
***/
static void layout_monitor_refresh_2(void)
{
	refresh_area_t area[] = {
		{0, 0, 1024, 70},
		{0, 470, 1024, 130},
		{378, 90, 300, 50},
		{260, 220, 400, 240}};
	gui_refresh_area(area, sizeof(area) / sizeof(refresh_area_t));
}

/***
** 日期: 2022-05-13 10:02
** 作者: leo.liu
** 函数作用：点击设置刷新区域
** 返回参数说明：
***/
static void layout_monitor_refresh_4(void)
{
	refresh_area_t area[] = {
		{0, 0, 1024, 70},
		{0, 450, 1024, 150},
		{179, 185, 666, 230},
		// {378, 90, 300, 50}
	};
	gui_refresh_area(area, sizeof(area) / sizeof(refresh_area_t));
}

/***
** 日期: 2022-05-13 10:02
** 作者: leo.liu
** 函数作用：在先点击设置的情况下点击录像或者拍照刷新区域
** 返回参数说明：
***/
static void layout_monitor_refresh_5(void)
{
	refresh_area_t area[] = {
		{0, 0, 1024, 70},
		{0, 470, 1024, 130},
		{179, 185, 666, 230},
		// {378, 90, 300, 50}
	};
	gui_refresh_area(area, sizeof(area) / sizeof(refresh_area_t));
}

static custom_area camera_btn_area[CAMERA_TOTAL_BTN] =
	{
		{920, 25, 50, 37},	  // 返回主页按钮
		{127, 450, 120, 120}, // 通道切换按钮
		{257, 450, 120, 120}, // 画面调节按钮
		{387, 450, 120, 120}, // 抓拍按钮
		{517, 450, 120, 120}, // 录像按钮
		{647, 450, 120, 120}, // 开锁按钮
		{777, 450, 120, 120}, // 挂断/对讲按钮
		{942, 12, 54, 54},
};

#define CAMERA_HEAD_CH_LABEL_ID 9					// 9：顶部通道标签（显示“门1/监控1”）
#define CAMERA_HEAD_TIME_LABEL_ID 10				// 10：顶部时间标签
#define CAMERA_HEAD_SDCADR_ICON_LABEL_ID 11			// 11：顶部SD卡图标
#define CAMERA_FUNC_BTN_BG_BLOCK_ID 12				// 12：功能按钮背景块
#define CAMERA_SETTING_WINDOW_ID 13					// 13：参数调节弹窗
#define CAMERA_PROMPT_MESSAGE_LABEL_ID 14			// 14：提示消息标签（比如“录像中 10秒”）
#define CAMERA_ZOOM_SCALE_LABEL_ID 15				// 15：缩放比例标签
#define CAMERA_BRIGHTNESS_CONT_ID 16				// 16：亮度调节容器
#define CAMERA_COLOR_CONT_ID 17						// 17：色度调节容器
#define CAMERA_CONTRAST_CONT_ID 18					// 18：对比度调节容器
#define CAMERA_DISPLAY_DELAY_MASK_OBJ_ID 19			// 19：监控延时遮挡蒙版（刚打开监控时的黑框）
#define CAMERA_CHANGE_SETTING_WINDOW_ID 20			// 20：通道切换弹窗
#define CAMERA_PROMPT_MESSAGE_ION_ID 21				// 21：提示图标（抓拍/开锁的小图标）
#define CAMERA_MONITOR_COUNT_DOWN_ID 23				// 23：顶部监控倒计时
#define CAMERA_CAPTURE_PROMPT_IMG_ID 24				// 24：抓拍提示图片
#define CAMERA_UNLOCK_PROMPT_IMG_ID 25				// 25：开锁提示图片
#define LAYOUT_SETTING_ADJUST_OBJ_MSG_ID 0X26		// 26：调节提示消息
#define LAYOUT_SETTING_ADJUST_OBJ_BTNMATRIX_ID 0x27 // 27：调节按钮矩阵
#define LAYOUT_SETTING_BRIGHTNESS_ID 60
#define LAYOUT_SETTING_COLOR_ID 61
#define LAYOUT_SETTING_CONTRAST_ID 62
#define CAMERA_RECORDING_BG_IMG_ID 63 // 录像倒计时背景图片
// 定时任务枚举
typedef enum
{
	CAMERA_TASK_TIME_DISP,			   // 0：刷新时间任务
	CAMERA_TASK_NO_SDCARD_DISP,		   // 1：无SD卡闪烁提示任务
	CAMERA_TASK_UNLOCK,				   // 2：开锁后恢复状态任务
	CAMERA_TASK_BTN_WIN_HIDDEN,		   // 3：按钮自动隐藏任务
	CAMERA_TASK_RECORD_IMAGE,		   // 4：抓拍后隐藏提示图任务
	CAMERA_TASK_RECORD_VIDEO,		   // 5：录像倒计时任务
	CAMERA_TASK_BTN_CHANGE_WIN_HIDDEN, // 6：通道弹窗隐藏任务
	CAMERA_TASK_TOTAL,				   // 8：任务总数
} ticker_type;

typedef void (*ticker_func)(void);

typedef struct
{
	bool en;
	const int delay;
	int count;
	ticker_func handler_func;
} ticker_task_t;

typedef enum
{
	CAMERA_MODE_MONITOR = 0,
	CAMERA_MODE_ZOOM
} cam_mode_t;

static void layout_camera_door1_call_func(void);
static void layout_camera_door2_call_func(void);
static void layout_camera_callring_finish_default_func(int index);
static void layout_camera_click_down_func(lv_obj_t *obj);
static void layout_camera_open_btn_func(void);
// static void camera_goto_zoom_mode(lv_obj_t *parent);
static void camera_goto_monitor_mode(lv_obj_t *parent);
// static void camera_channel_switch(MON_CH ch);
static void camera_func_btn_diaplay_enable(bool en);
static void camera_btn_and_win_hidden_task_restart(void);
static void camera_bg_btn_click_enable(bool en);
static void camera_record_photo_video(REC_MODE mode);
static void camera_record_photo_video_task(lv_task_t *task);
static void camera_setting_window_display_enable(bool en);
static void camera_display_delay_start(void);
static void camera_switch_btn_create_display(void);

static void camera_ticker_task_stop(ticker_type type);
static void camera_ticker_task_restart(ticker_type type);

// extern bool manual_enter_monitor_get(void);
// extern void manual_enter_monitor_set(bool en);

static bool is_recording = false;
static bool is_opening = false;
static bool func_btn_diaplay_flag = true;
static bool setting_win_diaplay_flag = true;
static bool camera_change_win_diaplay_flag = true;

static cam_mode_t camera_mode = CAMERA_MODE_MONITOR;
static int camera_timeout_val = 90;						// 监控时长90秒
static int camera_display_delay = CAMERA_DISPLAY_DELAY; // * 100ms
static int camera_record_video_count_down = 15;
static lv_task_t *camera_display_delay_task_t = NULL;
static bool camera_in_talk_state = false;
static bool camera_call_ring_active = false;
static unsigned long long camera_call_ring_deadline = 0;

// static bool camera_enter_zoom = false;

static void camera_capture_prompt_img_create(lv_obj_t *parent)
{
	lv_obj_t *img = lv_img_create(parent, NULL);
	lv_obj_set_id(img, CAMERA_CAPTURE_PROMPT_IMG_ID);

	lv_obj_set_pos(img, 422, 180);
	lv_obj_set_size(img, 250, 250);
	lv_obj_set_hidden(img, true);

	lv_img_set_zoom(img, LV_IMG_ZOOM_NONE);
	lv_img_set_angle(img, 0);
	lv_img_set_pivot(img, 0, 0);
}

static void camera_unlock_prompt_img_create(lv_obj_t *parent)
{
	lv_obj_t *img = lv_obj_create(parent, NULL);
	lv_obj_set_id(img, CAMERA_UNLOCK_PROMPT_IMG_ID);
	lv_obj_set_size(img, 250, 250);
	lv_obj_set_pos(img, 422, 170);
	lv_obj_set_style_local_pattern_align(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_bg_opa(img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_hidden(img, true);
}

static void camera_change_setting_display_enable(bool en)
{

	camera_change_win_diaplay_flag = en;
	if (en)
	{
		layout_monitor_refresh_2();
		camera_bg_btn_click_enable(false);
	}
	else
	{
		camera_bg_btn_click_enable(true);
	}

	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_CHANGE_SETTING_WINDOW_ID);
	if (cont == NULL)
	{
		return;
	}

	lv_obj_set_hidden(cont, !en);
}

static void camera_change_door1_btn_up(lv_obj_t *obj)
{

	if (monitor_channel_get() == MON_CH_DOOR1)
	{
		return;
	}
	monitor_enter_mask_set(MON_ENTER_MANUAL_DOOR);
	monitor_channel_set(MON_CH_DOOR1);
	// camera_btn_and_win_hidden_task_restart();
	// user_data_get()->change_channel_enable = true;
	goto_layout(pLAYOUT(camera));
	// camera_channel_switch(MON_CH_DOOR1);
}

static void camera_change_door2_btn_up(lv_obj_t *obj)
{
	if (monitor_channel_get() == MON_CH_DOOR2)
	{
		return;
	}
	monitor_enter_mask_set(MON_ENTER_MANUAL_DOOR);
	monitor_channel_set(MON_CH_DOOR2);
	// camera_btn_and_win_hidden_task_restart();
	// user_data_get()->change_channel_enable = true;
	goto_layout(pLAYOUT(camera));
	// camera_channel_switch(MON_CH_DOOR2);
}

static void camera_change_cctv1_btn_up(lv_obj_t *obj)
{
	printf("1monitor_channel_get() = %d\n", monitor_channel_get());
	if (monitor_channel_get() == MON_CH_CCTV1)
	{
		return;
	}
	// cctv_audio_video_enable_pin_ctrl(false);      lynn 26.3.10
	monitor_enter_mask_set(MON_ENTER_MANUAL_CCTV);
	monitor_channel_set(MON_CH_CCTV1);
	// camera_btn_and_win_hidden_task_restart();
	// user_data_get()->change_channel_enable = true;
	goto_layout(pLAYOUT(camera));
	// camera_channel_switch(MON_CH_CCTV1);
	camera_switch_btn_create_display();
}

static void camera_change_cctv2_btn_up(lv_obj_t *obj)
{
	printf("2monitor_channel_get() = %d\n", monitor_channel_get());
	if (monitor_channel_get() == MON_CH_CCTV2)
	{
		return;
	}
	// cctv_audio_video_enable_pin_ctrl(true);      lynn 26.3.10
	monitor_enter_mask_set(MON_ENTER_MANUAL_CCTV);
	monitor_channel_set(MON_CH_CCTV2);
	// camera_btn_and_win_hidden_task_restart();
	// user_data_get()->change_channel_enable = true;
	goto_layout(pLAYOUT(camera));
	// camera_channel_switch(MON_CH_CCTV2);
	camera_switch_btn_create_display();
}

// 创建切换选择按钮
static void camera_change_select_btn_create(lv_obj_t *parent)
{
	MON_CH ch = monitor_channel_get();

	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_id(cont, CAMERA_CHANGE_SETTING_WINDOW_ID);
	if ((ch == MON_CH_CCTV1) || (ch == MON_CH_CCTV2))
	{
		lv_obj_set_pos(cont, 388, 220);
		lv_obj_set_size(cont, 200, 240);
	}
	else
	{
		lv_obj_set_pos(cont, 260, 220);
		lv_obj_set_size(cont, 200, 240);
	}

	// lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4f4f4f));
	// lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 14);

	lv_obj_t *line = lv_line_create(cont, NULL);
	static lv_point_t points[2] = {{0, 60}, {200, 60}};
	lv_line_set_points(line, points, 2);
	lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_line_opa(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);

	line = lv_line_create(cont, line);
	static lv_point_t points1[2] = {{0, 120}, {200, 120}};
	lv_line_set_points(line, points1, 2);

	line = lv_line_create(cont, line);
	static lv_point_t points2[2] = {{0, 180}, {200, 180}};
	lv_line_set_points(line, points2, 2);

	lv_obj_t *door1_btn = lv_obj_create(cont, NULL);
	lv_obj_set_id(door1_btn, 1);
	lv_obj_set_size(door1_btn, 280, 60);
	lv_obj_set_style_local_bg_opa(door1_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(door1_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(door1_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4f4f4f));
	lv_obj_set_style_local_bg_color(door1_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xC4C4C4));
	lv_obj_set_style_local_value_str(door1_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_DOOR1_ID));
	lv_obj_set_style_local_value_color(door1_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_value_align(door1_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(door1_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	if (monitor_channel_get() == MON_CH_DOOR1)
	{
		lv_obj_set_style_local_bg_opa(door1_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
		lv_obj_set_style_local_bg_opa(door1_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
		lv_obj_set_style_local_bg_color(door1_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xC4C4C4));
		lv_obj_set_style_local_bg_color(door1_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xC4C4C4));
	}
	lv_obj_align(door1_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	static obj_click_data btn_data1 = obj_click_data_up_create(camera_change_door1_btn_up);
	obj_click_event_listen(door1_btn, &btn_data1);

	lv_obj_t *door2_btn = lv_obj_create(cont, door1_btn);
	lv_obj_set_id(door2_btn, 2);
	lv_obj_set_style_local_bg_opa(door2_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(door2_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(door2_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xC4C4C4));

	lv_obj_set_style_local_value_str(door2_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_DOOR2_ID));
	lv_obj_align(door2_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 60);
	if (monitor_channel_get() == MON_CH_DOOR2)
	{
		lv_obj_set_style_local_bg_opa(door2_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
		lv_obj_set_style_local_bg_opa(door2_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
		lv_obj_set_style_local_bg_color(door2_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xC4C4C4));
		lv_obj_set_style_local_bg_color(door2_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xC4C4C4));
	}
	static obj_click_data btn_data2 = obj_click_data_up_create(camera_change_door2_btn_up);
	obj_click_event_listen(door2_btn, &btn_data2);

	lv_obj_t *cctv1_btn = lv_obj_create(cont, door1_btn);
	lv_obj_set_id(cctv1_btn, 3);
	lv_obj_set_style_local_bg_opa(cctv1_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(cctv1_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(cctv1_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xC4C4C4));

	lv_obj_set_style_local_value_str(cctv1_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_CCTV1_ID));
	lv_obj_align(cctv1_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 120);
	if (monitor_channel_get() == MON_CH_CCTV1)
	{
		lv_obj_set_style_local_bg_opa(cctv1_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
		lv_obj_set_style_local_bg_opa(cctv1_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
		lv_obj_set_style_local_bg_color(cctv1_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xC4C4C4));
		lv_obj_set_style_local_bg_color(cctv1_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xC4C4C4));
	}
	static obj_click_data btn_data3 = obj_click_data_up_create(camera_change_cctv1_btn_up);
	obj_click_event_listen(cctv1_btn, &btn_data3);

	lv_obj_t *cctv2_btn = lv_obj_create(cont, door1_btn);
	lv_obj_set_id(cctv2_btn, 4);
	lv_obj_set_style_local_bg_opa(cctv2_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_style_local_bg_opa(cctv2_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
	lv_obj_set_style_local_bg_color(cctv2_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xC4C4C4));

	lv_obj_set_style_local_value_str(cctv2_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_CCTV2_ID));
	lv_obj_align(cctv2_btn, NULL, LV_ALIGN_IN_TOP_MID, 0, 180);
	if (monitor_channel_get() == MON_CH_CCTV2)
	{
		lv_obj_set_style_local_bg_opa(cctv2_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
		lv_obj_set_style_local_bg_opa(cctv2_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
		lv_obj_set_style_local_bg_color(cctv2_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xC4C4C4));
		lv_obj_set_style_local_bg_color(cctv2_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xC4C4C4));
	}
	static obj_click_data btn_data4 = obj_click_data_up_create(camera_change_cctv2_btn_up);
	obj_click_event_listen(cctv2_btn, &btn_data4);
}
static void door_call_auto_camere(lv_task_t *task)
{

	camera_record_photo_video(REC_MODE_AUTO);

	lv_task_del(task);
}

static int camera_call_ring_time_get(void)
{
	int ring_time = user_data_get()->setting.ring_time;
	return ring_time > 0 ? ring_time : 5;
}

static void camera_call_ring_play(int index)
{
	camera_call_ring_active = true;
	camera_call_ring_deadline = user_timestamp_get() + (unsigned long long)camera_call_ring_time_get() * 1000;
	ringplay_play_form_index(index, 100, ringplay_doorcall_start_default_func, layout_camera_callring_finish_default_func, false);
}

static bool camera_call_ring_should_replay(void)
{
	return camera_call_ring_active == true &&
		   monitor_enter_mask_get() == MON_ENTER_CALL &&
		   user_timestamp_get() < camera_call_ring_deadline;
}

static void camera_call_ring_finish_cleanup(void)
{
	camera_call_ring_active = false;
	power_amplifier_enable(false);
	MON_CH ch = monitor_channel_get();
	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, false);
	printf("user_data_get()->setting.record_mode = %d\n", user_data_get()->setting.record_mode);
	uint8_t rec_mode = user_data_get()->setting.record_mode;
	if ((rec_mode == RECORD_MODE_IMAGE || rec_mode == RECORD_MODE_VIDEO))
	{
		lv_layout_task_create(camera_record_photo_video_task, 2000, LV_TASK_PRIO_MID, NULL);
	}
}

// 复位监控倒计时
void camera_timeout_value_reset(void)
{
#if CAMERA_AUTO_RECORD_ENABLE
	camera_timeout_val = 7200; // 3600;
#else
	camera_timeout_val = 90;
#endif
}
#if 1
static void camera_unlock_ringa_start_func(int index)
{
	MON_CH ch = monitor_channel_get();
	ring_volume_set(3);
	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, true);
}

static void camera_unlock_ring_finish_func(int index)
{
	power_amplifier_enable(false);
	MON_CH ch = monitor_channel_get();
	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, false);
}
#endif
static void camera_head_time_display_flush(void)
{
	lv_obj_t *time_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_HEAD_TIME_LABEL_ID);
	struct tm tm = {0};
	user_time_read(&tm);

	if (user_data_get()->setting.calendar == 0) // 0=伊朗历
	{
		struct date temp_date =
			{
				.year = tm.tm_year,
				.month = tm.tm_mon,
				.day = tm.tm_mday};
		temp_date = gregorian2jalali(temp_date);
		lv_label_set_text_fmt(time_label, "%04d-%02d-%02d   %02d:%02d:%02d", temp_date.year, temp_date.month, temp_date.day, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	else
	{
		lv_label_set_text_fmt(time_label, "%04d-%02d-%02d   %02d:%02d:%02d", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}

	lv_obj_set_pos(time_label, 347, 28);
}


static void camera_sdcard_state_display_flush(void)
{
	lv_obj_t *sdcard_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_HEAD_SDCADR_ICON_LABEL_ID);

	if (media_sdcard_insert_check())
	{
		if (sdcard_icon_obj != NULL)
		{
			static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_SDCARD_INSERT_PNG);
			static rom_bin_info info1 = rom_bin_info_get(ROM_UI_CAMERA_SDCARD_ERROR_PNG);
			if (user_data_get()->sd_card_pattion)
			{
				lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info1);
			}
			else
			{
				lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);
			}
			lv_obj_set_hidden(sdcard_icon_obj, false);
			camera_ticker_task_stop(CAMERA_TASK_NO_SDCARD_DISP);
		}
	}
	else
	{
		if (sdcard_icon_obj != NULL)
		{
			static rom_bin_info info1 = rom_bin_info_get(ROM_UI_CAMERA_NO_SDCARD_PNG);
			lv_obj_set_style_local_pattern_image(sdcard_icon_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info1);
			lv_obj_set_hidden(sdcard_icon_obj, false);
			camera_ticker_task_restart(CAMERA_TASK_NO_SDCARD_DISP);
		}
	}
}

// 时间显示刷新任务
static void camera_head_time_display_task(void)
{
	camera_head_time_display_flush();
}
// 无SD卡显示任务
static void camera_sdcard_display_task(void)
{
	lv_obj_t *sdcard_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_HEAD_SDCADR_ICON_LABEL_ID);
	if (sdcard_icon_obj != NULL && media_sdcard_insert_check() == false)
	{
		lv_obj_set_hidden(sdcard_icon_obj, !lv_obj_get_hidden(sdcard_icon_obj));
	}
}
// 开锁任务
static void camera_unlock_task(void)
{
	lv_obj_t *obj1 = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_ION_ID);
	lv_obj_t *unlock_img = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_UNLOCK_PROMPT_IMG_ID);

	// 执行底层开锁关闭逻辑
	monitor_unlcok_close();

	// 恢复开锁按钮的图标样式
	lv_obj_t *open_btn = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_OPEN_BTN_ID);
	if (open_btn != NULL)
	{
		static rom_bin_info original_open_img = rom_bin_info_get(ROM_UI_CAMERA_LOCK_PNG);
		lv_obj_set_style_local_pattern_image(open_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &original_open_img);
	}

	is_opening = false;

	// 无论是否在录像，开锁任务结束时都隐藏录像提示图标(obj1)，
	if (obj1 != NULL)
	{
		lv_obj_set_hidden(obj1, true);
	}

	// 隐藏开锁提示图片
	if (unlock_img != NULL)
	{
		lv_obj_set_hidden(unlock_img, true);
	}

	// 停止当前任务
	camera_ticker_task_stop(CAMERA_TASK_UNLOCK);
}
// 功能键隐藏任务
static void camera_btn_and_win_hidden_task(void)
{

	if (camera_change_win_diaplay_flag)
	{
		camera_change_setting_display_enable(false);
		camera_ticker_task_restart(CAMERA_TASK_BTN_WIN_HIDDEN);

		return;
	}
	else if (func_btn_diaplay_flag)
	{
		camera_func_btn_diaplay_enable(false);
	}
	camera_ticker_task_stop(CAMERA_TASK_BTN_WIN_HIDDEN);
}
// 切换功能键隐藏任务
static void camera_btn_and_select_hidden_task(void)
{
	if (camera_change_win_diaplay_flag)
	{
		camera_change_setting_display_enable(false);
		camera_ticker_task_restart(CAMERA_TASK_BTN_CHANGE_WIN_HIDDEN);
		camera_bg_btn_click_enable(true);
		return;
	}
	else if (func_btn_diaplay_flag)
	{
		camera_func_btn_diaplay_enable(false);
	}
	camera_ticker_task_stop(CAMERA_TASK_BTN_CHANGE_WIN_HIDDEN);
}
// 抓拍结束时的任务
static void camera_record_image_end_task(void)
{
	lv_obj_t *capture_img = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_CAPTURE_PROMPT_IMG_ID);
	lv_obj_t *obj1 = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_ION_ID);
	if (!is_opening && capture_img != NULL)
	{
		lv_obj_set_hidden(capture_img, true);
		lv_obj_set_hidden(obj1, true);
	}
	is_recording = false;
	user_data_get()->new_photo_file_flag = true;
	camera_ticker_task_stop(CAMERA_TASK_RECORD_IMAGE);
}
// 录视频时的倒计时的任务
static void camera_record_video_count_down_task(void)
{
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);
	lv_obj_t *obj1 = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_ION_ID);
	lv_obj_t *rec_bg = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_RECORDING_BG_IMG_ID);
	if (--camera_record_video_count_down < 0 ||
		media_sdcard_insert_check() == false ||
		video_record_status_get() == false ||
		video_input_state_get() == false)
	{
		if (!is_opening && obj != NULL)
		{
			lv_obj_set_hidden(obj, true);   
            lv_obj_set_hidden(obj1, true);  
			lv_obj_set_hidden(rec_bg, true); 
			lv_obj_t *video_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_ION_ID);
			if (video_icon_obj != NULL)
			{
				lv_obj_set_hidden(video_icon_obj, true);
			}
			lv_obj_set_hidden(rec_bg, true);
		}
		// 若SD卡已插入，标记「有新媒体文件」
		if (media_sdcard_insert_check())
		{
			user_data_get()->new_media_file_flag = true;
		}
		// 停止倒计时任务、重置录制状态、关闭视频录制
		camera_ticker_task_stop(CAMERA_TASK_RECORD_VIDEO); // 停止倒计时定时任务
		is_recording = false;							   // 标记录制状态为停止
		record_video_close();							   // 执行关闭视频录制的底层逻辑

		// 恢复录制按钮的原始样式
		lv_obj_t *record_btn = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_RECORD_BTN_ID);
		if (record_btn)
		{
			static rom_bin_info original_info = rom_bin_info_get(ROM_UI_CAMERA_VIDEO_PNG);
			lv_obj_set_style_local_pattern_image(record_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &original_info);
		}
		return;
	}
	camera_ticker_task_restart(CAMERA_TASK_RECORD_VIDEO);
	if (!is_opening && obj != NULL)
	{
		lv_obj_set_hidden(obj, false);
		lv_obj_set_hidden(rec_bg, false);
		if (user_data_get()->setting.language == LANG_ENGLISH)
		{
			lv_label_set_text_fmt(obj, "%02d", camera_record_video_count_down);
		}
		else
		{
			lv_label_set_text_fmt(obj, "%02d ", camera_record_video_count_down);
		}
	}
}

ticker_task_t ticker_task[CAMERA_TASK_TOTAL] = {
	{false, 2, 2, camera_head_time_display_task},
	{false, 4, 4, camera_sdcard_display_task},
	{false, 2, 2, camera_unlock_task},
	{false, 20, 20, camera_btn_and_win_hidden_task},
	{false, 2, 2, camera_record_image_end_task},
	{false, 2, 2, camera_record_video_count_down_task},
	{false, 20, 20, camera_btn_and_select_hidden_task},
};

static void camera_ticker_task_restart(ticker_type type)
{
	ticker_task[type].count = ticker_task[type].delay;
	ticker_task[type].en = true;
}

static void camera_ticker_task_stop(ticker_type type)
{
	if (type == CAMERA_TASK_TOTAL)
	{
		for (int i = 0; i < CAMERA_TASK_TOTAL; i++)
		{
			ticker_task[i].en = false;
		}
	}
	else
	{
		ticker_task[type].en = false;
	}
}

// static void camera_ticker_task(lv_task_t *task_t)
// {
// #if CAMERA_AUTO_RECORD_ENABLE
// 	printf("=================>>> timeout:[%d]\n", camera_timeout_val);
// 	camera_record_photo_video(REC_MODE_AUTO);
// #endif
// 	for (int i = 0; i < CAMERA_TASK_TOTAL; i++)
// 	{
// 		if (ticker_task[i].en)
// 		{
// 			ticker_task[i].count--;
// 			if (ticker_task[i].count <= 0)
// 			{
// 				ticker_task[i].handler_func();
// 			}
// 		}
// 	}
// }

// 新增定时任务，用于更新通话功能按钮图片
// lynn 26.3.23
static void camera_ticker_task(lv_task_t *task_t)
{
#if CAMERA_AUTO_RECORD_ENABLE
    printf("=================>>> timeout:[%d]\n", camera_timeout_val);
    camera_record_photo_video(REC_MODE_AUTO);
#endif
    for (int i = 0; i < CAMERA_TASK_TOTAL; i++)
    {
        if (ticker_task[i].en)
        {
            ticker_task[i].count--;
            if (ticker_task[i].count <= 0)
            {
                ticker_task[i].handler_func();
            }
        }
    }

    // 持续检测听筒状态；按钮图片固定为 hangup，不再在 up/hangup 间切换。
    static bool last_hook_state = false;
    bool current_hook_state = hook_state_get();
    
    if (current_hook_state != last_hook_state)
    {
        last_hook_state = current_hook_state;
        camera_in_talk_state = current_hook_state;
    }
}

static void camera_ticker_task_create(void)
{
	lv_layout_task_create(camera_ticker_task, 500, LV_TASK_PRIO_HIGH, NULL);
}

// SD卡状态显示
static void camera_sdcard_state_change_func(void)
{
	camera_sdcard_state_display_flush();
	if (media_sdcard_insert_check() == false)
	{
		user_data_get()->new_media_file_flag = false;
		user_data_get()->new_photo_file_flag = false;
	}
}
// 创建camera界面顶部lable的背景块
static void camera_func_btn_topbg_block_create(lv_obj_t *parent)
{
	lv_obj_t *obj = lv_obj_create(parent, NULL);
	lv_obj_set_click(obj, false);
	lv_obj_set_pos(obj, 0, 0);
	lv_obj_set_size(obj, 1024, 70);
	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
}
static void camera_head_channel_label_create(lv_obj_t *parent)
{
	lv_obj_t *ch_obj = lv_obj_create(parent, NULL);
	lv_obj_set_id(ch_obj, CAMERA_HEAD_CH_LABEL_ID);
	lv_obj_set_pos(ch_obj, 54, 32);
	MON_CH ch = monitor_channel_get();
	if ((ch == MON_CH_DOOR1) || (ch == MON_CH_DOOR2))
	{
		// lv_label_set_text(ch_obj, monitor_channel_get() == MON_CH_DOOR1 ? str_get(LAYOUT_HOME_LANG_DOOR1_ID) : str_get(LAYOUT_HOME_LANG_DOOR2_ID));
		lv_obj_set_style_local_value_str(ch_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, monitor_channel_get() == MON_CH_DOOR1 ? str_get(LAYOUT_HOME_LANG_DOOR1_ID) : str_get(LAYOUT_HOME_LANG_DOOR2_ID));
	}
	else if ((ch == MON_CH_CCTV1) || (ch == MON_CH_CCTV2))
	{
		if (user_data_get()->setting.language == LANG_PERSIAN)
		{
			lv_obj_set_style_local_value_ofs_x(ch_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 30);
		}
		lv_obj_set_style_local_value_str(ch_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, monitor_channel_get() == MON_CH_CCTV1 ? str_get(LAYOUT_HOME_LANG_CCTV1_ID) : str_get(LAYOUT_HOME_LANG_CCTV2_ID));
		// lv_label_set_text(ch_obj, monitor_channel_get() == MON_CH_CCTV1 ? str_get(LAYOUT_HOME_LANG_CCTV1_ID) : str_get(LAYOUT_HOME_LANG_CCTV2_ID));
	}

	lv_obj_set_style_local_value_color(ch_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_value_font(ch_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
}

static void camera_head_time_label_create(lv_obj_t *parent)
{
	lv_obj_t *time_label = lv_label_create(parent, NULL);
	lv_obj_set_id(time_label, CAMERA_HEAD_TIME_LABEL_ID);
	lv_obj_set_style_local_text_color(time_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_text_font(time_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
	camera_head_time_display_flush();
	camera_ticker_task_restart(CAMERA_TASK_TIME_DISP);
	lv_obj_set_style_local_value_align(time_label, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_MID);
}
// 监控倒计时时间
static void camera_head_monitor_count_flush(lv_task_t* task)
{
	lv_obj_t *time_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_MONITOR_COUNT_DOWN_ID);
	if (time_label == NULL)
	{
		printf("time_label is NULL\n");
		return;
	}
	lv_label_set_text_fmt(time_label, "%02dS", camera_timeout_val);
	lv_obj_set_pos(time_label, 816, 28);
	if (camera_timeout_val-- <= 0)
	{
		goto_layout(pLAYOUT(standby));
		lv_task_del(task);
	}
}
static void camera_head_monitor_count_label_create(lv_obj_t *parent)
{
	lv_obj_t *countdown_label = lv_label_create(parent, NULL);
	lv_obj_set_id(countdown_label, CAMERA_MONITOR_COUNT_DOWN_ID);
	lv_obj_set_style_local_text_color(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_text_font(countdown_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
	lv_layout_task_create(camera_head_monitor_count_flush, 1000, LV_TASK_PRIO_MID, NULL);
}

static void camera_sdcard_icon_create(lv_obj_t *parent)
{
	lv_obj_t *sdcard_icon_obj = lv_obj_create(parent, NULL);
	lv_obj_set_id(sdcard_icon_obj, CAMERA_HEAD_SDCADR_ICON_LABEL_ID);
	lv_obj_set_pos(sdcard_icon_obj, 750, 28);
	lv_obj_set_size(sdcard_icon_obj, 38, 38);
	camera_sdcard_state_display_flush();
}
static void camera_recording_bg_img_create(lv_obj_t *parent)
{
	lv_obj_t *bg_img = lv_obj_create(parent, NULL);
	lv_obj_set_id(bg_img, CAMERA_RECORDING_BG_IMG_ID);
	lv_obj_set_pos(bg_img, 200, 24); // 固定位置
	lv_obj_set_size(bg_img, 48, 48); // 固定大小
	// 设置录像背景图
	static rom_bin_info recording_bg = rom_bin_info_get(ROM_UI_CAMERA_RECORDING_PNG);
	lv_obj_set_style_local_pattern_image(bg_img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &recording_bg);
	lv_obj_set_style_local_pattern_align(bg_img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_bg_opa(bg_img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_hidden(bg_img, true); // 默认隐藏，录像时显示
}
// 提示消息的标签创建（开锁提示、录像提示）
static void camera_prompt_message_label_create(lv_obj_t *parent)
{
	lv_obj_t *label = lv_label_create(parent, NULL);
	lv_obj_set_id(label, CAMERA_PROMPT_MESSAGE_LABEL_ID);
	lv_label_set_text(label, "");
	lv_obj_set_pos(label, 210, 35);
	// lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 30, 66);
	lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_obj_set_hidden(label, true);
}

// 提示消息的ion创建（开锁提示、录像提示）
static void camera_prompt_message_ion_create(lv_obj_t *parent)
{
	lv_obj_t *obj = lv_obj_create(parent, NULL);
	lv_obj_set_id(obj, CAMERA_PROMPT_MESSAGE_ION_ID);
	lv_obj_set_pos(obj, 422, 180);
	lv_obj_set_size(obj, 250, 250);
	lv_obj_set_hidden(obj, true);
	lv_obj_set_style_local_pattern_align(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
}

// 监控视频参数的设置 滑块创建
static void camera_video_param_setting_btn_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, const void *icon_src, const char *str, obj_click_data *slider_data, int value, unsigned int id, unsigned int id_text)
{
	// 创建独立滑块容器
	lv_obj_t *cont = lv_cont_create(parent, NULL);
	lv_obj_set_id(cont, id);
	lv_obj_set_pos(cont, x, y);
	lv_obj_set_size(cont, 606, 34);
	lv_cont_set_fit(cont, LV_FIT_NONE);

	// 容器样式
	lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
	lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 12);
	lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

	// 图标
	if (icon_src != NULL)
	{
		lv_obj_t *icon = lv_obj_create(cont, NULL);
		lv_obj_set_size(icon, 24, 24); // 设置图标大小
		lv_obj_align(icon, cont, LV_ALIGN_IN_LEFT_MID, 16, 0);
		lv_obj_set_style_local_pattern_image(icon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
		lv_obj_set_style_local_pattern_align(icon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
		lv_obj_set_style_local_bg_opa(icon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	}

	// 文字
	lv_obj_set_style_local_value_str(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str);
	lv_obj_set_style_local_value_align(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
	lv_obj_set_style_local_value_ofs_x(cont, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 55);
	lv_obj_set_style_local_value_font(cont, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	// 创建可交互的滑块
	lv_obj_t *slider = lv_slider_create(cont, NULL);
	lv_obj_set_click(slider, true); // 允许滑块被点击拖动
	lv_obj_set_size(slider, 394, 16);
	lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 60, 0);
	lv_slider_set_range(slider, 0, 10);
	lv_slider_set_value(slider, value, LV_ANIM_ON);
	lv_obj_set_adv_hittest(slider, false);

	// 1. 滑块背景（未激活部分）
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	// lv_obj_set_style_local_bg_opa(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_OPA_60);
	lv_obj_set_style_local_radius(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, 6);

	// 2. 进度条（激活部分）：蓝色渐变
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0x00D0FF));
	lv_obj_set_style_local_bg_grad_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0xCCE0FF));
	lv_obj_set_style_local_bg_grad_dir(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_GRAD_DIR_HOR);
	lv_obj_set_style_local_radius(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, 6);

	// 3. 滑块旋钮：白色圆形
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_radius(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
	lv_obj_set_style_local_pad_all(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 5); // 旋钮大小

	// 按下状态：进度条更亮，旋钮不变
	lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_PRESSED, lv_color_hex(0x33B5FF));
	lv_obj_set_style_local_bg_grad_color(slider, LV_SLIDER_PART_INDIC, LV_STATE_PRESSED, lv_color_hex(0x0099FF));

	// 数值
	lv_obj_t *num_label = lv_label_create(cont, NULL);
	lv_obj_set_id(num_label, id_text);
	lv_label_set_text_fmt(num_label, "%02d", value);				// 初始化显示当前通道的参数值
	lv_obj_align(num_label, slider, LV_ALIGN_OUT_RIGHT_MID, 13, 0); // 滑块右侧显示
	lv_obj_set_style_local_text_color(num_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_text_font(num_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

	// 绑定滑块事件回调
	if (slider_data != NULL)
		obj_click_event_listen(slider, slider_data);

	// 保持数据关联
	cont->user_data = slider;
	slider->user_data = cont;
}

static void slider_common_cleanup(lv_obj_t *slider)
{
	camera_btn_and_win_hidden_task_restart();
	lv_obj_clear_state(slider, LV_STATE_PRESSED);
}
// 亮度调节滑块回调
static void camera_brightness_adj_btn_up(lv_obj_t *obj)
{
	// 获取滑块当前亮度值
	int brightness = lv_slider_get_value(obj);
	// 逐层通过ID获取UI对象
	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_SETTING_WINDOW_ID);
	lv_obj_t *brightness_cont = lv_obj_get_child_form_id(cont, CAMERA_BRIGHTNESS_CONT_ID);
	lv_obj_t *label = lv_obj_get_child_form_id(brightness_cont, LAYOUT_SETTING_BRIGHTNESS_ID);
	lv_label_set_text_fmt(label, "%02d", brightness);
	// 设置实际显示亮度+更新显示
	monitor_display_brightness_vol_set(brightness);
	display_bright_adj(brightness, INVALID_FORMAT);
	slider_common_cleanup(obj);
}

// 色彩调节滑块回调
static void camera_color_adj_btn_up(lv_obj_t *obj)
{
	// 获取滑块当前亮度值
	int color = lv_slider_get_value(obj);
	// 逐层通过ID获取UI对象
	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_SETTING_WINDOW_ID);
	lv_obj_t *color_cont = lv_obj_get_child_form_id(cont, CAMERA_COLOR_CONT_ID);
	lv_obj_t *label = lv_obj_get_child_form_id(color_cont, LAYOUT_SETTING_COLOR_ID);
	lv_label_set_text_fmt(label, "%02d", color);
	// 设置色彩值并更新显示
	monitor_display_color_vol_set(color);
	display_color_adj(color, INVALID_FORMAT);
	slider_common_cleanup(obj);
}

// 对比度调节滑块回调
static void camera_contrast_adj_btn_up(lv_obj_t *obj)
{
	// 获取滑块当前值
	int contrast = lv_slider_get_value(obj);
	// 逐层通过ID获取UI对象
	lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_SETTING_WINDOW_ID);
	lv_obj_t *contrast_cont = lv_obj_get_child_form_id(cont, CAMERA_CONTRAST_CONT_ID);
	lv_obj_t *label = lv_obj_get_child_form_id(contrast_cont, LAYOUT_SETTING_CONTRAST_ID);
	lv_label_set_text_fmt(label, "%02d", contrast);
	// 设置对比度值并更新显示
	monitor_display_cont_vol_set(contrast);
	display_const_adj(contrast, INVALID_FORMAT);
	slider_common_cleanup(obj);
}

static rom_bin_info brightness_icon = rom_bin_info_get(ROM_UI_CAMERA_BRIGHTNESS_PNG);
static rom_bin_info color_icon = rom_bin_info_get(ROM_UI_CAMERA_COLOR1_PNG);
static rom_bin_info contrast_icon = rom_bin_info_get(ROM_UI_CAMERA_CONTRAST1_PNG);

// 监控设置窗口创建
static void camera_setting_window_create(lv_obj_t *parent)
{
	lv_obj_t *win_cont = lv_cont_create(parent, NULL);
	lv_obj_set_click(win_cont, false);
	lv_obj_set_id(win_cont, CAMERA_SETTING_WINDOW_ID);
	lv_obj_set_pos(win_cont, 179, 185);
	lv_obj_set_size(win_cont, 666, 230);

	// 弹窗背景样式
	lv_obj_set_style_local_bg_color(win_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_bg_opa(win_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
	lv_obj_set_style_local_radius(win_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
	lv_obj_set_style_local_pad_all(win_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 15); // 弹窗内边距

	// 回调函数
	static obj_click_data slider_data1 = obj_click_data_up_create(camera_brightness_adj_btn_up);
	static obj_click_data slider_data2 = obj_click_data_up_create(camera_color_adj_btn_up);
	static obj_click_data slider_data3 = obj_click_data_up_create(camera_contrast_adj_btn_up);

	// 创建三个参数的滑块控件
	camera_video_param_setting_btn_create(win_cont, 28, 34, &brightness_icon, str_get(LAYOUT_CAMERA_LANG_BRIGHTNESS_ID), &slider_data1, monitor_display_brightness_vol_get(), CAMERA_BRIGHTNESS_CONT_ID, LAYOUT_SETTING_BRIGHTNESS_ID);
	camera_video_param_setting_btn_create(win_cont, 28, 98, &color_icon, str_get(LAYOUT_CAMERA_LANG_COLOR_ID), &slider_data2, monitor_display_color_vol_get(), CAMERA_COLOR_CONT_ID, LAYOUT_SETTING_COLOR_ID);
	camera_video_param_setting_btn_create(win_cont, 28, 162, &contrast_icon, str_get(LAYOUT_CAMERA_LANG_CONTRAST_ID), &slider_data3, monitor_display_cont_vol_get(), CAMERA_CONTRAST_CONT_ID, LAYOUT_SETTING_CONTRAST_ID);
}
// extern bool video_format_is_invalid(void);

// 图标按键创建
lv_obj_t *camera_img_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src)
{
	lv_obj_t *btn_obj = lv_obj_create(parent, NULL);
	lv_obj_set_ext_click_area(btn_obj, 12, 12, 10, 15);
	lv_obj_set_pos(btn_obj, btn_area.x, btn_area.y);
	lv_obj_set_size(btn_obj, btn_area.w, btn_area.h);
	if (icon_src != NULL)
	{
		lv_obj_set_style_local_pattern_image(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
		lv_obj_set_style_local_pattern_align(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	}

	lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50); // 按下叠加50%黑色（深色）

	if (string != NULL)
	{
		lv_obj_set_style_local_value_str(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string);
		lv_obj_set_style_local_value_align(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
		lv_obj_set_style_local_value_ofs_y(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
		lv_obj_set_style_local_value_font(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	}

	obj_click_event_listen(btn_obj, btn_pdata);

	return btn_obj;
}

static void camera_home_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(home));
}
// 创建home按钮
static void camera_home_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(camera_home_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_HOME_PNG);
	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_HOME_BTN_ID], NULL, &btn_data, &info);
	lv_obj_set_id(btn, CAMERA_HOME_BTN_ID);
}
// static void camera_adjust_reset_btn_up(lv_obj_t *obj)
// {
// 	// 定义参数默认值
// 	const int DEFAULT_VALUE = 5;

// 	//  重置亮度到默认值
// 	monitor_display_brightness_vol_set(DEFAULT_VALUE);
// 	display_bright_adj(DEFAULT_VALUE, INVALID_FORMAT); // 应用亮度设置到显示

// 	//  重置色度到默认值
// 	monitor_display_color_vol_set(DEFAULT_VALUE);
// 	display_color_adj(DEFAULT_VALUE, INVALID_FORMAT); // 应用色度设置到显示

// 	//  重置对比度到默认值
// 	monitor_display_cont_vol_set(DEFAULT_VALUE);
// 	display_const_adj(DEFAULT_VALUE, INVALID_FORMAT); // 应用对比度设置到显示

// 	//  同步更新UI滑块显示
// 	lv_obj_t *setting_window = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_SETTING_WINDOW_ID);
// 	// 亮度滑块
// 	lv_obj_t *brightness_cont = lv_obj_get_child_form_id(setting_window, CAMERA_BRIGHTNESS_CONT_ID);
// 	if (brightness_cont && brightness_cont->user_data)
// 	{
// 		lv_slider_set_value((lv_obj_t *)brightness_cont->user_data, DEFAULT_VALUE, LV_ANIM_OFF);
// 	}
// 	// 色度滑块
// 	lv_obj_t *color_cont = lv_obj_get_child_form_id(setting_window, CAMERA_COLOR_CONT_ID);
// 	if (color_cont && color_cont->user_data)
// 	{
// 		lv_slider_set_value((lv_obj_t *)color_cont->user_data, DEFAULT_VALUE, LV_ANIM_OFF);
// 	}
// 	// 对比度滑块
// 	lv_obj_t *contrast_cont = lv_obj_get_child_form_id(setting_window, CAMERA_CONTRAST_CONT_ID);
// 	if (contrast_cont && contrast_cont->user_data)
// 	{
// 		lv_slider_set_value((lv_obj_t *)contrast_cont->user_data, DEFAULT_VALUE, LV_ANIM_OFF);
// 	}
// }
static bool setting_color_change_flag = false;
static void camera_color_btn_up(lv_obj_t *obj)
{
	setting_color_change_flag = true;
	bool is_window_show = setting_win_diaplay_flag;

	if (is_window_show)
	{
		camera_setting_window_display_enable(false);
		// camera_func_btn_diaplay_enable(true);
		// camera_bg_btn_click_enable(true);
		layout_monitor_refresh_1();
	}
	else
	{
		camera_setting_window_display_enable(true);
		// camera_func_btn_diaplay_enable(false);
		camera_bg_btn_click_enable(true);
		layout_monitor_refresh_4(); // 刷新调节界面样式
		printf("画面调节弹窗已显示\n");
	}
}

static void camera_color_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(camera_color_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_COLOR_PNG);
	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_COLOR_BTN_ID], NULL, &btn_data, &info);
	lv_obj_set_id(btn, CAMERA_COLOR_BTN_ID);
}

// 创建adjust_reset按钮
// static void camera_adjust_reset_btn_create(lv_obj_t *parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(camera_adjust_reset_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_SWTICH_PNG);
// 	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_ADJUST_RESET_BTN_ID], NULL, &btn_data, &info);
// 	lv_obj_set_id(btn, CAMERA_ADJUST_RESET_BTN_ID);
// }
static void camera_switch_btn_create_display(void)
{
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_SWITCH_BTN_ID);
	if (obj == NULL)
	{
		return;
	}
	MON_CH ch = monitor_channel_get();
	if ((ch == MON_CH_CCTV1) || (ch == MON_CH_CCTV2))
	{
		static rom_bin_info cctv_info = rom_bin_info_get(ROM_UI_CAMERA_CCTV_CHANNEL_PNG);
		lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &cctv_info);
		lv_obj_set_x(obj, 322);
	}
	else
	{
		static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_SWTICH_PNG);
		lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
		lv_obj_set_x(obj, 127);
	}
}

static void camera_switch_btn_up(lv_obj_t *obj)
{
	// monitor_enter_mask_set(MON_ENTER_MANUAL_DOOR);
	MON_CH ch = monitor_channel_get();
	// ch = ch == MON_CH_DOOR1 ? MON_CH_DOOR2 : MON_CH_DOOR1;
	// camera_channel_switch(ch);
	// camera_setting_window_display_enable(false);
	// camera_change_setting_display_enable(!camera_change_win_diaplay_flag);

	// camera_btn_and_win_hidden_task_restart();
	// camera_bg_btn_click_enable(true);
	switch (ch)
	{
	case MON_CH_DOOR1:
		camera_change_door2_btn_up(NULL);
		break;
	case MON_CH_DOOR2:
		camera_change_cctv1_btn_up(NULL);
		break;
	case MON_CH_CCTV1:
		camera_change_cctv2_btn_up(NULL);
		break;
	case MON_CH_CCTV2:
		camera_change_door1_btn_up(NULL);
		break;
	default:
		camera_change_door1_btn_up(NULL);
		break;
	}
	camera_switch_btn_create_display();
}
// 创建cam1&2按钮
static void camera_switch_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(camera_switch_btn_up);
	MON_CH ch = monitor_channel_get();
	if ((ch == MON_CH_CCTV1) || (ch == MON_CH_CCTV2))
	{
		static rom_bin_info cctv_info = rom_bin_info_get(ROM_UI_CAMERA_CCTV_CHANNEL_PNG);
		lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_SWITCH_BTN_ID], NULL, &btn_data, &cctv_info);
		lv_obj_set_id(btn, CAMERA_SWITCH_BTN_ID);
	}
	else
	{
		static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_SWTICH_PNG);
		lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_SWITCH_BTN_ID], NULL, &btn_data, &info);
		lv_obj_set_id(btn, CAMERA_SWITCH_BTN_ID);
	}

	camera_switch_btn_create_display();
}

static void camera_record_btn_create_display(void)
{
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_RECORD_BTN_ID);
	if (obj == NULL)
	{
		return;
	}
	MON_CH ch = monitor_channel_get();
	if ((ch == MON_CH_CCTV1) || (ch == MON_CH_CCTV2))
	{
		lv_obj_set_x(obj, 582);
	}
}

// 抓拍或录像
static void camera_record_photo_video(REC_MODE mode)
{
	if (video_input_state_get() == false || is_recording == true)
		return;
	printf("=============>> record_mode : [%d] \n", user_data_get()->setting.record_mode);
	
    // 获取UI对象指针
    lv_obj_t *msg_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);
	lv_obj_t *rec_bg = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_RECORDING_BG_IMG_ID);
	lv_obj_t *capture_img = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_CAPTURE_PROMPT_IMG_ID);
    // // 获取录像提示图标容器
    // lv_obj_t *prompt_icon_obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_ION_ID);

	if (msg_label == NULL)
	{
		printf("msg_label == NULL\n");
		return;
	}
	if (rec_bg == NULL)
	{
		printf("rec_bg == NULL\n");
		return;
	}
    
    // 刷新区域
	if (setting_win_diaplay_flag == true)
	{
		layout_monitor_refresh_5();
	}
	else
	{
		layout_monitor_refresh_2();
	}
    
    // ========== 抓拍模式 (或无SD卡) ==========
	if (user_data_get()->setting.record_mode == RECORD_MODE_IMAGE || media_sdcard_insert_check() == false)
	{
		if (record_jpeg_start(mode) == true)
		{
			user_data_get()->media_disp_mode = 0;
			is_recording = true;

            // 设置抓拍图片
			if (capture_img != NULL) {
                static rom_bin_info capture_img_info = rom_bin_info_get(ROM_UI_CAMERA_CAPTURE_PNG);
                lv_img_set_src(capture_img, &capture_img_info);
                lv_obj_set_hidden(capture_img, false);
            }

			lv_obj_set_hidden(msg_label, false);
			camera_ticker_task_restart(CAMERA_TASK_RECORD_IMAGE);
		}
	}
    // ========== 录像模式 ==========
	else if (user_data_get()->setting.record_mode == RECORD_MODE_VIDEO)
	{
		if (record_video_start(mode) == true)
		{
			user_data_get()->media_disp_mode = 1;
			is_recording = true;
			camera_record_video_count_down = 15;
            
            // 设置倒计时文字
			if (user_data_get()->setting.language == LANG_ENGLISH)
			{
				lv_label_set_text_fmt(msg_label, "%02d" , camera_record_video_count_down);
			}
			else
			{
				lv_label_set_text_fmt(msg_label, "%02d ", camera_record_video_count_down);
			}
            // 显示文字和背景
			lv_obj_set_hidden(msg_label, false); 
			if (rec_bg != NULL)
			{
                lv_obj_set_hidden(rec_bg, false); 
            }
			camera_ticker_task_restart(CAMERA_TASK_RECORD_VIDEO);
		}
	}
}
static void camera_record_photo_video_task(lv_task_t *task)
{
	printf("00000000000\n");
	camera_record_photo_video(REC_MODE_AUTO);

	lv_task_del(task);
}

static void camera_record_btn_up(lv_obj_t *obj)
{
	lv_obj_t *msg_label = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_PROMPT_MESSAGE_LABEL_ID);
	lv_obj_t *rec_bg = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_RECORDING_BG_IMG_ID);
	if (record_video_start(REC_MODE_MANUAL) == true)
	{
		user_data_get()->media_disp_mode = 1;
		is_recording = true;
		camera_record_video_count_down = 15;
		if (user_data_get()->setting.language == LANG_ENGLISH)
		{
			lv_label_set_text_fmt(msg_label, "%02d", camera_record_video_count_down);
		}
		else
		{
			lv_label_set_text_fmt(msg_label, "%02d ", camera_record_video_count_down);
		}
		if (rec_bg != NULL)
		{
			lv_obj_set_hidden(rec_bg, false);
		}
		static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_VIDEO_PNG);
		lv_obj_set_style_local_pattern_image(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);
		lv_obj_set_hidden(obj, false);
		lv_obj_set_hidden(msg_label, false);
		camera_ticker_task_restart(CAMERA_TASK_RECORD_VIDEO);
	}
	camera_btn_and_win_hidden_task_restart();
}
// 创建record按钮
static void camera_record_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(camera_record_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_VIDEO_PNG);
	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_RECORD_BTN_ID], NULL, &btn_data, &info);
	lv_obj_set_id(btn, CAMERA_RECORD_BTN_ID);

	camera_record_btn_create_display();
}

static void camera_zoom_btn_create_display(void)
{
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_CAPTURE_BTN_ID);
	if (obj == NULL)
	{
		return;
	}
	MON_CH ch = monitor_channel_get();
	if ((ch == MON_CH_CCTV1) || (ch == MON_CH_CCTV2))
	{
		lv_obj_set_x(obj, 452);
	}
}

static void camera_zoom_btn_up(lv_obj_t *obj)
{
	// camera_enter_zoom = true;
	// camera_goto_zoom_mode(lv_scr_act());
	// layout_monitor_refresh_3();
	if (video_input_state_get() == false || is_recording == true)
		return;
	if (setting_win_diaplay_flag == true)
	{
		layout_monitor_refresh_5();
	}
	else
	{
		layout_monitor_refresh_2();
	}
	lv_obj_t *capture_img = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_CAPTURE_PROMPT_IMG_ID);
	if (record_jpeg_start(REC_MODE_MANUAL) == true)
	{
		printf("------------------[%s]----------[%d]\n", __func__, __LINE__);
		user_data_get()->media_disp_mode = 0;
		is_recording = true;

		static rom_bin_info capture_img_info = rom_bin_info_get(ROM_UI_CAMERA_CAPTURE_PNG);
		lv_img_set_src(capture_img, &capture_img_info);

		// lv_label_set_text(msg_label, str_get(LAYOUT_CAMERA_LANG_RECORD_IMAGE_ID));

		lv_obj_set_hidden(capture_img, false);
		printf("------------------[%s]----------[%d]\n", __func__, __LINE__);
		camera_ticker_task_restart(CAMERA_TASK_RECORD_IMAGE);
		printf("------------------[%s]----------[%d]\n", __func__, __LINE__);
	}
	camera_btn_and_win_hidden_task_restart();
}
// 创建Capture按钮
static void camera_capture_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(camera_zoom_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_PHOTO_PNG);
	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_CAPTURE_BTN_ID], NULL, &btn_data, &info);
	lv_obj_set_id(btn, CAMERA_CAPTURE_BTN_ID);

	camera_zoom_btn_create_display();
}

static void camera_setting_window_display_enable(bool en)
{
	setting_win_diaplay_flag = en;
	if (en)
	{
		camera_bg_btn_click_enable(false);
	}
	else
	{
		camera_bg_btn_click_enable(true);
	}
	lv_obj_t *win = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_SETTING_WINDOW_ID);
	if (win == NULL)
		return;

	lv_obj_t *cont = lv_obj_get_child_form_id(win, CAMERA_BRIGHTNESS_CONT_ID);
	lv_slider_set_value((lv_obj_t *)(cont->user_data), monitor_display_brightness_vol_get(), LV_ANIM_OFF);
	cont = lv_obj_get_child_form_id(win, CAMERA_COLOR_CONT_ID);
	lv_slider_set_value((lv_obj_t *)(cont->user_data), monitor_display_color_vol_get(), LV_ANIM_OFF);
	cont = lv_obj_get_child_form_id(win, CAMERA_CONTRAST_CONT_ID);
	lv_slider_set_value((lv_obj_t *)(cont->user_data), monitor_display_cont_vol_get(), LV_ANIM_OFF);

	layout_monitor_refresh_4();
	lv_obj_set_hidden(win, !en);
}

static void camera_hang_up_btn_click(lv_obj_t *obj)
{
	if (ringplay_ing_check() == true)
	{
		ringplay_play_stop();
	}
	door_audio_talk(AUDIO_CH_CLOSE);
	camera_in_talk_state = false;
	goto_layout(pLAYOUT(standby));
}

// 创建hang up按钮
static void camera_hang_up_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(camera_hang_up_btn_click);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_HANGUP_PNG);
	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_HANG_UP_BTN_ID], NULL, &btn_data, &info);
	lv_obj_set_id(btn, CAMERA_HANG_UP_BTN_ID);
	camera_in_talk_state = false;
}

static void camera_open_btn_up(lv_obj_t *obj)
{
	MON_CH ch = monitor_channel_get();
	if ((is_opening) || (ch == MON_CH_CCTV1 || ch == MON_CH_CCTV2 || (video_input_state_get() == false)))
		return;
	if (setting_win_diaplay_flag == true)
	{
		camera_setting_window_display_enable(false);
		layout_monitor_refresh_5();
	}
	else
	{
		layout_monitor_refresh_2();
	}
	lv_obj_t *unlock_img = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_UNLOCK_PROMPT_IMG_ID);
	if (unlock_img != NULL)
	{
		static rom_bin_info unlock_img_info = rom_bin_info_get(ROM_UI_CAMERA_OPEN_LOCK_PNG);
		lv_obj_set_style_local_pattern_image(unlock_img, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &unlock_img_info);
		lv_obj_set_hidden(unlock_img, false);
	}

	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_LOCK_PNG);
	lv_obj_set_style_local_pattern_image(obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);
	lv_obj_set_hidden(obj, false);

	camera_btn_and_win_hidden_task_restart();
	camera_ticker_task_restart(CAMERA_TASK_UNLOCK);
	is_opening = true;
	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, true);
	monitor_unlock_open(0, ch);
	ringplay_play_form_index(7, 100, camera_unlock_ringa_start_func, camera_unlock_ring_finish_func, false);
}
// 创建open按钮
static void camera_open_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(camera_open_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_LOCK_PNG);
	lv_obj_t *btn = camera_img_btn_create(parent, camera_btn_area[CAMERA_OPEN_BTN_ID], NULL, &btn_data, &info);
	lv_obj_set_id(btn, CAMERA_OPEN_BTN_ID);
}

// 图标隐藏任务重新倒计时
static void camera_btn_and_win_hidden_task_restart(void)
{
	// if(setting_win_diaplay_flag)
	// {
	camera_ticker_task_restart(CAMERA_TASK_BTN_WIN_HIDDEN);
	// }
	// else if(camera_change_win_diaplay_flag)
	// {
	// 	camera_ticker_task_restart(CAMERA_TASK_BTN_CHANGE_WIN_HIDDEN);
	// }
}

// 功能键显示使能
static void camera_func_btn_diaplay_enable(bool en)
{
	func_btn_diaplay_flag = en;
	// lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_FUNC_BTN_BG_BLOCK_ID);
	// if (obj == NULL)
	// 	return;
	// lv_obj_set_hidden(obj, !en);
	for (int i = 1; i < 7; i++)
	{
		lv_obj_set_hidden(lv_obj_get_child_form_id(lv_scr_act(), i), !en);
	}

	MON_CH ch = monitor_channel_get();
	if ((ch == MON_CH_CCTV1) || (ch == MON_CH_CCTV2))
	{
		lv_obj_set_hidden(lv_obj_get_child_form_id(lv_scr_act(), 2), true);
		lv_obj_set_hidden(lv_obj_get_child_form_id(lv_scr_act(), 5), true);
		lv_obj_set_hidden(lv_obj_get_child_form_id(lv_scr_act(), 6), true);
	}
}

static void camera_bg_btn_up(lv_obj_t *obj)
{
	camera_func_btn_diaplay_enable(!func_btn_diaplay_flag);
	// printf("==========func_btn_diaplay_flag[%d]==\n", func_btn_diaplay_flag);
	if (func_btn_diaplay_flag)
	{
		camera_ticker_task_restart(CAMERA_TASK_BTN_WIN_HIDDEN);
	}
	else
	{
		camera_ticker_task_stop(CAMERA_TASK_BTN_WIN_HIDDEN);
	}
	camera_btn_and_win_hidden_task_restart();
}
// 背景的点击使能
static void camera_bg_btn_click_enable(bool en)
{
	if (en)
	{
		static obj_click_data bg_btn_data = obj_click_data_up_create(camera_bg_btn_up);
		obj_click_event_listen(lv_scr_act(), &bg_btn_data);
	}
	else
	{
		obj_click_event_listen(lv_scr_act(), NULL);
	}
}
// 进入监控模式
static void camera_goto_monitor_mode(lv_obj_t *parent)
{
	camera_mode = CAMERA_MODE_MONITOR; /*选择是进入平常模式*/
	video_input_display_zoom_set(100);
	video_input_display_offset_set(0, 0);
	lv_obj_clean(parent);
	camera_bg_btn_click_enable(true); /*背景的点击使能，点击任务的reset和stop*/

	camera_func_btn_topbg_block_create(parent);
	camera_head_channel_label_create(parent);		/*顶部通道label的显示*/
	camera_head_time_label_create(parent);			/*顶部时间label的显示*/
	camera_sdcard_icon_create(parent);				/*顶部SD卡ui状态的显示*/
	camera_head_monitor_count_label_create(parent); /*顶部监控倒计时的显示*/
	camera_prompt_message_label_create(parent);		/*开锁/录像/抓拍之后label的显示*/
	camera_prompt_message_ion_create(parent);		/*开锁/录像/抓拍之后ui的显示*/

	// camera_func_btn_bg_block_create(parent); /*创建底部的背景*/
	camera_home_btn_create(parent); /*创建返回home界面的按钮*/
	// if (user_data_get()->setting.window_display_enable)   // lynn
	// camera_adjust_reset_btn_create(parent); /*创建恢复adjust的按钮*/
	camera_color_btn_create(parent);
	camera_switch_btn_create(parent);  /*创建切换通道的按钮*/
	camera_capture_btn_create(parent); /*创建抓拍的按钮*/
	camera_record_btn_create(parent);  /*创建录像的按钮*/
	camera_open_btn_create(parent);	   /*创建开锁的按钮*/
	// camera_answer_btn_create(parent);		/*创建的通话按钮*/
	camera_hang_up_btn_create(parent); /*创建挂断的按钮*/
	camera_capture_prompt_img_create(parent);
	camera_unlock_prompt_img_create(parent);
	camera_func_btn_diaplay_enable(true); /*底部按钮随时间的显示与否*/

	camera_setting_window_create(parent);		 /*创建一个容器和在之上创建亮度等的ui*/
	camera_setting_window_display_enable(false); /*win容器和亮度等的ui显示与否*/

	camera_change_select_btn_create(parent);	 /*创建切换按钮等的ui*/
	camera_change_setting_display_enable(false); /*切换按钮容器和按键等的ui显示与否*/
	camera_switch_btn_create_display();
	camera_recording_bg_img_create(parent);
	camera_btn_and_win_hidden_task_restart();
	layout_monitor_refresh_1();

	layout_sd_state_callback_register(camera_sdcard_state_change_func);
}

#if 0
static void camera_door_call_ring_play_timer(lv_task_t *pt)
{
	if (pt == NULL)
	{
		return;
	}
	if ((video_input_state_get() == false) && (pt->repeat_count > 1))
	{
		return;
	}

	// if (/*(monitor_enter_mask_get() == MON_ENTER_TALK) || */(monitor_enter_mask_get() == MON_ENTER_CALL))
	{
		MON_CH ch = monitor_channel_get();
		if (ch == MON_CH_DOOR1)
		{
			ringplay_play_form_index(user_data_get()->setting.door1_tone, 100, ringplay_doorcall_start_default_func, layout_camera_callring_finish_default_func, false);
		}
		else if (ch == MON_CH_DOOR2)
		{
			ringplay_play_form_index(user_data_get()->setting.door2_tone, 100, ringplay_doorcall_start_default_func, layout_camera_callring_finish_default_func, false);
		}
	}

	lv_task_del(pt);
}
#endif
static void LAYOUT_ENTER_FUNC(camera)
{
	/*清状态*/
	layout_monitor_refresh_1();
	MON_CH ch = monitor_channel_get();

	if (ch == MON_CH_DOOR1 || ch == MON_CH_DOOR2)
	{
		if (user_data_get()->setting.door_ring_volume == 0)//lynn 26.3.13
		{
			power_amplifier_enable(false);
		}
	}

	// camera_enter_zoom = false;
	monitor_open(true, 0x03);
	printf("==============[%d]:[%s]\n", __LINE__, __func__);
	audio_input_capture_enable(true); // 延时打开ai，ai打开的同时，铃声开始播放，会有顿一下
	jpg_encode_capture_enable(true);

	standby_timer_close();
	is_recording = false;
	is_opening = false;

	lv_obj_t *parent = lv_scr_act();
	lv_obj_set_style_local_pattern_image(parent, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, NULL);
	camera_goto_monitor_mode(parent);
	// camera_screen_adjust_enable(user_data_get()->setting.window_display_enable);
	/* 主界面插卡的时候进入监控，有ui残留 */
	fb_gui_layer_rect_fill(0x00, 0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX);

	/*注册回调*/
	layout_door1_call_callback_register(layout_camera_door1_call_func);
	layout_door2_call_callback_register(layout_camera_door2_call_func);
	lv_obj_click_down_callback_register(layout_camera_click_down_func);
	layout_gate_open_callback_register(layout_camera_open_btn_func);

	ak_sleep_ms(500); // 延时播放铃声，ai打开的同时，铃声开始播放，会有顿一下

	// camera_auto_record_task_create();
	/*启动定时任务*/
	camera_timeout_value_reset();
	camera_ticker_task_create();
	camera_display_delay_start();

	if ((monitor_enter_mask_get() == MON_ENTER_CALL))
	{
		// lv_task_t *pt = lv_layout_task_create(camera_door_call_ring_play_timer, 100, LV_TASK_PRIO_MID, NULL);
		// lv_task_set_repeat_count(pt, 30);
		MON_CH ch = monitor_channel_get();
		// if (call_record_start(true, ch, 0))
		// {
		// 	printf("Call record created for door station 1\n");
		// }
		printf("user_data_get()->setting.door_ring_volume = %d\n", user_data_get()->setting.door_ring_volume);
		if (ch == MON_CH_DOOR1 && user_data_get()->setting.door_ring_volume != 0)
		{
			camera_call_ring_play(user_data_get()->setting.door1_tone);
		}
		else if (ch == MON_CH_DOOR2 && user_data_get()->setting.door_ring_volume != 0)
		{
			camera_call_ring_play(user_data_get()->setting.door2_tone);
		}
		if (user_data_get()->setting.door_ring_volume == 0)
		{
			lv_layout_task_create(door_call_auto_camere, 3000, LV_TASK_PRIO_MID, NULL);
		}
	}
	if (hook_state_get() == true)
	{
		if (ch == MON_CH_DOOR1)
		{
			printf("=========================>>>> door1 talking \n");
			if (ringplay_ing_check() == true)
			{
				ringplay_play_stop();
			}
			// if (call_record_answered(CALL_DOOR_STATION_1))
			// {
			// 	printf("Call answered for door station 1\n");
			// }
			door_audio_talk(AUDIO_CH_DOOR1);
		}
		else if (ch == MON_CH_DOOR2)
		{
			if (ringplay_ing_check() == true)
			{
				ringplay_play_stop();
			}
			printf("=========================>>>> door2 talking \n");
			// if (call_record_answered(CALL_DOOR_STATION_2))
			// {
			// 	printf("Call answered for door station 1\n");
			// }
			door_audio_talk(AUDIO_CH_DOOR2);
		}
		monitor_enter_mask_set(MON_ENTER_TALK);
	}

	camera_in_talk_state = hook_state_get();
	
	// lv_layout_task_create(camera_bell_detaction_task, 10, LV_TASK_PRIO_HIGH, NULL);
}
static void LAYOUT_QUIT_FUNC(camera)
{
	// if(user_data_get()->change_channel_enable == false)
	// {
	// 	manual_enter_monitor_set(false);
	// }
	ringplay_play_stop();
	camera_call_ring_active = false;
	camera_call_ring_deadline = 0;
	video_input_display_zoom_set(100);
	video_input_display_offset_set(0, 0);
	layout_door1_call_callback_register(layout_door1_call_default);
	layout_door2_call_callback_register(layout_door2_call_default);
	lv_obj_click_down_callback_register(layout_obj_click_down_func);
	layout_gate_open_callback_register(NULL);
	camera_ticker_task_stop(CAMERA_TASK_TOTAL);
	audio_input_capture_enable(false);
	camera_bg_btn_click_enable(false);

	layout_sd_state_callback_register(layout_sdcard_state_change_default);

	door_audio_talk(AUDIO_CH_CLOSE);
	monitor_close();
	record_video_close();
	record_jpeg_close();


	call_record_end(CALL_DOOR_STATION_1);
	call_record_end(CALL_DOOR_STATION_2);

	// audio_to_outdoor_pin_ctrl(false);
	user_data_save();
	standby_timer_restart(true);
	monitor_unlcok_close();
	camera_display_delay_task_t = NULL;
}

// 监控界面点击按键音处理函数
static void layout_camera_click_down_func(lv_obj_t *obj)
{
	//  if(hook_state_get() == true)
	//  {
	//  	return;
	//  }
	//  MON_CH ch = monitor_channel_get();
	//  call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, false);
	//  touch_sound_play(NULL, NULL);
}

// 监控界面door1 call机处理函数
static void layout_camera_door1_call_func(void)
{
	// user_data_get()->change_channel_enable = false;

	monitor_valid_channel_set(MON_CH_DOOR1, true);
	monitor_enter_mask_set(MON_ENTER_CALL);
	MON_CH ch = monitor_channel_get();
	if (ch != MON_CH_DOOR1)
	{
		// camera_channel_switch(MON_CH_DOOR1);
		monitor_channel_set(MON_CH_DOOR1);
		goto_layout(pLAYOUT(camera));
		return;
	}
	// if (camera_enter_zoom == true)
	// {
	// 	camera_enter_zoom = false;
	// 	camera_goto_monitor_mode(lv_scr_act());
	// }
	if (user_data_get()->setting.door_ring_volume != 0){
		camera_call_ring_play(user_data_get()->setting.door1_tone);
	}
		
}

// 监控界面door2 call机处理函数
static void layout_camera_door2_call_func(void)
{
	// user_data_get()->change_channel_enable = false;

	monitor_valid_channel_set(MON_CH_DOOR2, true);
	monitor_enter_mask_set(MON_ENTER_CALL);
	MON_CH ch = monitor_channel_get();
	if (ch != MON_CH_DOOR2)
	{
		// camera_channel_switch(MON_CH_DOOR2);
		monitor_channel_set(MON_CH_DOOR2);
		goto_layout(pLAYOUT(camera));
		return;
	}
	// if (camera_enter_zoom == true)
	// {
	// 	camera_enter_zoom = false;
	// 	camera_goto_monitor_mode(lv_scr_act());
	// }
	printf("user_data_get()->setting.door_ring_volume = %d\n", user_data_get()->setting.door_ring_volume);
	if (user_data_get()->setting.door_ring_volume != 0){
		camera_call_ring_play(user_data_get()->setting.door2_tone);
	}	
}

static void layout_camera_callring_finish_default_func(int index)
{
	if (camera_call_ring_should_replay() == true)
	{
		ringplay_play_form_index(index, 100, ringplay_doorcall_start_default_func, layout_camera_callring_finish_default_func, false);
		return;
	}
	camera_call_ring_finish_cleanup();
}

// 监控延时显示任务
static void camera_display_delay_task(lv_task_t *task_t)
{
	if (--camera_display_delay < 0 || video_input_state_get())
	{
		lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), CAMERA_DISPLAY_DELAY_MASK_OBJ_ID);
		if (obj != NULL)
		{
			lv_obj_del(obj);
		}
		lv_task_del(task_t);
		camera_display_delay_task_t = NULL;
		backlight_enable(true);
	}
}
// 监控延时显示开始
static void camera_display_delay_start(void)
{
	backlight_enable(false);
	camera_display_delay = CAMERA_DISPLAY_DELAY;
	if (camera_display_delay_task_t == NULL)
	{
		camera_display_delay_task_t = lv_layout_task_create(camera_display_delay_task, 100, LV_TASK_PRIO_HIGH, NULL);
		lv_obj_t *mask_obj = lv_obj_create(lv_scr_act(), NULL);
		lv_obj_set_id(mask_obj, CAMERA_DISPLAY_DELAY_MASK_OBJ_ID);
		lv_obj_set_pos(mask_obj, 0, 0);
		lv_obj_set_size(mask_obj, 1024, 600);
	}
}

static void layout_camera_open_btn_func(void)
{
	camera_open_btn_up(NULL);
}

CREATE_LAYOUT(camera);

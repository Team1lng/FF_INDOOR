/*******************************************************************
 * @Descripttion   :
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-19 11:15
 * @LastEditTime   : 2022-11-24 16:36
 *******************************************************************/
#include "layout_define.h"
#include "lvgl/lv_task.h"
static lv_task_t *motion_move_check_task_t = NULL;
static lv_task_t *motion_timing_check_task_t = NULL;
static lv_task_t *motion_delay_start_task_t = NULL;
static bool motion_detection_inited = false;
static bool motion_detection_started = false;
static bool standby_entering_motion_detection = false;
static bool standby_wakeup_in_progress = false;

#define MOTION_CAMERA_CCTV1 2
#define MOTION_CAMERA_CCTV2 3
/***移动侦测相关函数***/
static void standby_resource_release_sfunc(bool motion)
{
	if (motion)
	{
		if (motion_delay_start_task_t != NULL)
		{
			lv_task_del(motion_delay_start_task_t);
			motion_delay_start_task_t = NULL;
		}
		if (motion_move_check_task_t != NULL)
		{
			lv_task_del(motion_move_check_task_t);
			motion_move_check_task_t = NULL;
		}
		if (motion_timing_check_task_t != NULL)
		{
			lv_task_del(motion_timing_check_task_t);
			motion_timing_check_task_t = NULL;
		}
	}
}

static void standby_motion_detection_stop_and_destroy(void)
{
	if (motion_detection_started)
	{
		motion_detection_stop();
		motion_detection_started = false;
	}
	if (motion_detection_inited)
	{
		motion_detection_destory();
		motion_detection_inited = false;
	}
}

static bool standby_intercom_busy(void)
{
	callSIGN tag = GetIntercomCallTag();
	return intercom_state_get() != INTERCOM_STATE_IDLE ||
		   tag == OCR ||
		   tag == ACR;
}
static MON_CH standby_motion_monitor_channel_get(void)
{
	if (user_data_get()->motion.select_camera != MOTION_CAMERA_CCTV1 &&
		user_data_get()->motion.select_camera != MOTION_CAMERA_CCTV2)
	{
		printf("motion select camera invalid for new board: %d, fallback to CCTV1\n",
			   user_data_get()->motion.select_camera);
		user_data_get()->motion.select_camera = MOTION_CAMERA_CCTV1;
		user_data_save();
	}

	return user_data_get()->motion.select_camera == MOTION_CAMERA_CCTV2 ? MON_CH_CCTV2 : MON_CH_CCTV1;
}

static void layout_motion_monitor_open(void)
{
	monitor_channel_set(standby_motion_monitor_channel_get());
}

// 移动侦测检查任务
static void motion_move_check_task(lv_task_t *task)
{
	if (standby_intercom_busy())
	{
		motion_move_check_task_t = NULL;
		lv_task_del(task);
		standby_motion_detection_stop_and_destroy();
		return;
	}

	if (motion_detection_check() == false)
	{
		return;
	}

	if (motion_delay_start_task_t != NULL)
	{
		lv_task_del(motion_delay_start_task_t);
		motion_delay_start_task_t = NULL;
	}
	if (motion_timing_check_task_t != NULL)
	{
		lv_task_del(motion_timing_check_task_t);
		motion_timing_check_task_t = NULL;
	}
	motion_move_check_task_t = NULL;
	standby_entering_motion_detection = true;

	// 移动侦测触发后的处理
	printf("Motion detected! Taking action...\n");
	MON_CH motion_ch = standby_motion_monitor_channel_get();
	monitor_channel_set(motion_ch);
	monitor_valid_channel_set(motion_ch, true);
	monitor_enter_mask_set(MON_ENTER_NONE);
	// door_audio_select_pin_ctrl();
	lv_task_del(task);
	task = NULL;
	printf("--------------------&&&&&&&&&&&&&&&&>1");
	goto_layout(pLAYOUT(motion_detection));
}

// 初始化移动侦测参数
static void motion_init(void)
{
	// 根据灵敏度设置参数
	switch (user_data_get()->motion.sensivity)
	{
	case 0: // 低灵敏度
		motion_detection_sensivity(40, 128);
		break;
	case 1: // 中灵敏度
		motion_detection_sensivity(30, 96);
		break;
	case 2: // 高灵敏度
		motion_detection_sensivity(20, 64);
		break;
	default:
		motion_detection_sensivity(30, 96);
		break;
	}
}

// 定时检查任务
static void motion_timing_check_task(lv_task_t *task)
{
	// 如果启用了时间段控制
	if (user_data_get()->motion.timer_en)
	{
		// 获取当前时间
		time_t now = time(NULL);
		struct tm *now_tm = localtime(&now);
		int current_minutes = now_tm->tm_hour * 60 + now_tm->tm_min;

		// 计算开始和结束时间的分钟数
		int start_minutes = user_data_get()->motion.start.tm_hour * 60 + user_data_get()->motion.start.tm_min;
		int end_minutes = user_data_get()->motion.end.tm_hour * 60 + user_data_get()->motion.end.tm_min;

		// 检查当前时间是否在设定的时间段内
		bool in_time_range = false;
		if (start_minutes <= end_minutes)
		{
			// 时间段在同一天内
			in_time_range = (current_minutes >= start_minutes && current_minutes <= end_minutes);
		}
		else
		{
			// 时间段跨越午夜
			in_time_range = (current_minutes >= start_minutes || current_minutes <= end_minutes);
		}

		if (in_time_range)
		{
			if (motion_move_check_task_t == NULL)
			{
				motion_init();
				motion_detection_init();
				motion_detection_inited = true;
				motion_detection_start();
				motion_detection_started = true;
				printf("启动移动侦测任务\n");
				motion_move_check_task_t = lv_layout_task_create(motion_move_check_task, 100, LV_TASK_PRIO_HIGH, NULL);
			}
		}
		else
		{
			if (motion_move_check_task_t)
			{
				lv_task_del(motion_move_check_task_t);
				motion_move_check_task_t = NULL;
				motion_detection_stop();
				motion_detection_started = false;
			}
		}
	}
	else
	{
		lv_task_del(task);
		motion_timing_check_task_t = NULL;
		// 没有启用时间段控制，直接启动移动侦测
		printf("启动移动侦测任务\n");
		motion_init();
		motion_detection_init();
		motion_detection_inited = true;
		motion_detection_start();
		motion_detection_started = true;
		motion_move_check_task_t = lv_layout_task_create(motion_move_check_task, 100, LV_TASK_PRIO_HIGH, NULL);
	}
}

// 延时启动移动侦测任务
static void motion_delay_start_task(lv_task_t *task)
{

	if (motion_delay_start_task_t != NULL)
	{
		lv_task_del(motion_delay_start_task_t);
		motion_delay_start_task_t = NULL;
	}
	if (user_data_get()->motion.enable)
	{
		if (standby_intercom_busy())
		{
			return;
		}

		refresh_area_t area = {0, 0, 1024, 600};
		gui_refresh_area(&area, 1);
		layout_motion_monitor_open();
		if (monitor_open(false, 0x01) == false)
		{
			printf("Motion detection monitor open failed.\n");
			return;
		}
		video_input_skip_frame_count_set(15);

		// 启动定时检查任务
		motion_timing_check_task_t = lv_layout_task_create(motion_timing_check_task, 100, LV_TASK_PRIO_HIGH, NULL);
	}
}

static void standby_bg_click_event_cb(lv_obj_t *obj)
{
	if (standby_wakeup_in_progress)
	{
		return;
	}

	standby_wakeup_in_progress = true;
	obj_click_event_listen(lv_scr_act(), NULL);
	printf("Standby background clicked, returning to home layout.\n");
	printf("[ui_audio_trace] %llu standby wake click -> goto home\n", user_timestamp_get());
	goto_layout(pLAYOUT(home));
}

static void LAYOUT_ENTER_FUNC(standby)
{
	printf("[ui_audio_trace] %llu standby enter\n", user_timestamp_get());
	printf("Entering standby layout.\n");
	standby_wakeup_in_progress = false;
	standby_timer_close();
	backlight_enable(false);
	printf("[ui_audio_trace] %llu standby enter backlight off\n", user_timestamp_get());
	video_display_preview_enable(false);
	printf("[ui_audio_trace] %llu standby enter video preview off\n", user_timestamp_get());
	lv_video_mode_enable(false);
	fb_gui_layer_rect_fill(0x00, 0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	refresh_area_t area = {0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX};
	gui_refresh_area(&area, 1);
	printf("[ui_audio_trace] %llu standby enter fill black + refresh\n", user_timestamp_get());

	// 创建背景点击事件
	static obj_click_data bg_btn_data = obj_click_data_create(standby_bg_click_event_cb, NULL);
	obj_click_event_listen(lv_scr_act(), &bg_btn_data);
	/* Standby wakes on press, so do not queue a key tone during the layout switch. */
	lv_obj_click_down_callback_register(NULL);

	// 如果移动侦测启用，延时启动
	if (user_data_get()->motion.enable)
	{
		printf("Motion detection enabled.\n");
		motion_delay_start_task_t = lv_layout_task_create(motion_delay_start_task, 2000, LV_TASK_PRIO_HIGH, NULL);
	}
}

void delay_backlight_open_task(lv_task_t *task)
{
	backlight_enable(true);
	lv_task_del(task);
}



static void LAYOUT_QUIT_FUNC(standby)
{
	printf("[ui_audio_trace] %llu standby quit\n", user_timestamp_get());
	printf("Exiting standby layout.\n");
	// 清理移动侦测资源
	standby_resource_release_sfunc(true);
	if (!standby_entering_motion_detection)
	{
		standby_motion_detection_stop_and_destroy();
		monitor_close();
	}
	else
	{
		motion_detection_started = false;
		motion_detection_inited = false;
	}
	standby_entering_motion_detection = false;

	obj_click_event_listen(lv_scr_act(), NULL);
	lv_obj_click_down_callback_register(layout_obj_click_down_func);
	printf("[ui_audio_trace] %llu standby quit restore click cb\n", user_timestamp_get());
	standby_timer_restart(true);
}

CREATE_LAYOUT(standby);

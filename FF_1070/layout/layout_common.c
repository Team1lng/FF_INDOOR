#include "layout_define.h"
#include "ringplay.h"
#include "audio_output.h"

#define HOME_TOP_TIME_DATE_ID 0x1001

static void layout_prepare_intercom_in_black_transition(void)
{
	const layout *cur = cur_layout_get();

	if (cur != pLAYOUT(standby) && cur != pLAYOUT(motion_detection))
	{
		return;
	}

	printf("[intercom_in] black transition before incoming intercom\n");
	if (cur == pLAYOUT(motion_detection))
	{
		layout_motion_detection_prepare_intercom_in();
	}
	backlight_enable(false);
	video_display_preview_enable(false);
	fb_gui_layer_rect_fill(0x00, 0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	lv_obj_clean(lv_scr_act());
	refresh_area_t area = {0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX};
	gui_refresh_area(&area, 1);
}

/***
** 日期: 2022-05-20 17:30
** 作者: leo.liu
** 函数作用：铃声结束的默认处理
** 返回参数说明：
***/
void ringplay_keysound_finish_default_func(int index)
{
	(void)index;
	/* Touch tone finish must not toggle PA; layout exit/talk state owns PA off. */
}
void ringplay_keysound_start_default_func(int index)
{
	(void)index;
	printf("[media_audio_trace] %llu touch-tone start: layout=%p ao_remain=%d\n",
		   user_timestamp_get(), cur_layout_get(), audio_output_buffer_query());
	/* Enable PA (and cancel any pending PA-off) for the key tone. Without this
	 * the amplifier stays off on every page except home-media/photo-list, so the
	 * click sound is silent. */
	layout_media_keytone_prepare();
}
/***
** 日期: 2022-04-28 09:53
** 作者: leo.liu
** 函数作用：控件被按下执行的回调函数
** 返回参数说明：
***/
void layout_obj_click_down_func(lv_obj_t *obj)
{
	//***** 控制声音流向 *****/
	printf("[ui_audio_trace] %llu click_down layout=%p ao_remain=%d\n",
		   user_timestamp_get(), cur_layout_get(), audio_output_buffer_query());
	touch_sound_play(ringplay_keysound_start_default_func, ringplay_keysound_finish_default_func);
}

/***
** 日期: 2022-05-21 08:12
** 作者: leo.liu
** 函数作用：门口机铃声开始
** 返回参数说明：
***/
void ringplay_doorcall_start_default_func(int index)
{
	MON_CH ch = monitor_channel_get();
	ring_volume_set(user_data_get()->setting.inter_ring_volume);
	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, true);
}
/***
** 日期: 2022-05-21 08:12
** 作者: leo.liu
** 函数作用：门口机铃声结束
** 返回参数说明：
***/
void ringplay_doorcall_finish_default_func(int index)
{
	MON_CH ch = monitor_channel_get();
	if (hook_state_get() == true && (ch == MON_CH_DOOR1 || ch == MON_CH_DOOR2))
	{
		door_audio_talk(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2);
		return;
	}

	/***** 开启功放 *****/
	power_amplifier_enable(false);
	call_ring_to_outdoor_ctrl(ch == MON_CH_DOOR1 ? AUDIO_CH_DOOR1 : AUDIO_CH_DOOR2, false);
}
/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：door1 call 默认处理函数
** 返回参数说明：
***/
/* External door events preempt intercom immediately. Do not wait for the
 * current intercom ringtone to drain before switching the page. */
static void layout_interrupt_intercom_for_door_call(void)
{
	const layout *cur = cur_layout_get();
	if (cur != pLAYOUT(intercom_in) &&
		cur != pLAYOUT(intercom_out) &&
		cur != pLAYOUT(intercom_talk))
	{
		return;
	}

	printf("[door_call] interrupt intercom before entering camera\n");
	ringplay_play_stop_async();
	MsgCallEnd();
	/* The current call must close its route first. Only stale callbacks after
	 * MsgCallEnd() are prevented from stopping the upcoming door-call ring. */
	intercom_door_call_audio_hold_set(true);
	intercom_state_set(INTERCOM_STATE_IDLE);
	intercom_hangup_flag_get_and_clear();
	intercom_remote_ack_get_and_clear();
	intercom_busy_flag_get_and_clear();
	intercom_unack_flag_get_and_clear();
}

void layout_door1_call_default(void)
{
	monitor_valid_channel_set(MON_CH_DOOR1, true);
	if (cur_layout_get() != pLAYOUT(camera) /* && cur_layout_get() != pLAYOUT(intercom_talk) */)
	{
		if (cur_layout_get() == pLAYOUT(motion_detection))
		{
			layout_motion_detection_prepare_camera_in();
		}

		layout_interrupt_intercom_for_door_call();
		intercom_state_set(IDLE_WAITING);  /* 修改：替换为IDLE_WAITING */
		monitor_channel_set(MON_CH_DOOR1);
		monitor_enter_mask_set(MON_ENTER_CALL);
		printf("[motion_to_call] door1 call armed: record_mode=%d enter=%d\n",
			   user_data_get()->setting.record_mode, monitor_enter_mask_get());
		goto_layout(pLAYOUT(camera));
		// if (hook_state_get() == false)
		// {
		// 	ringplay_play_form_index(user_data_get()->setting.door1_tone, user_data_get()->setting.door_ring_volume, ringplay_doorcall_start_default_func, ringplay_doorcall_finish_default_func, false);
		// }
	}
}

/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：door2 call 默认处理函数
** 返回参数说明：通道设置必须放在goto后面，
***/
void layout_door2_call_default(void)
{
	monitor_valid_channel_set(MON_CH_DOOR2, true);
	if (cur_layout_get() != pLAYOUT(camera) /* && cur_layout_get() != pLAYOUT(intercom_talk) */)
	{
		if (cur_layout_get() == pLAYOUT(motion_detection))
		{
			layout_motion_detection_prepare_camera_in();
		}
		layout_interrupt_intercom_for_door_call();
		intercom_state_set(IDLE_WAITING);  /* 修改：替换为IDLE_WAITING */
		monitor_channel_set(MON_CH_DOOR2);
		monitor_enter_mask_set(MON_ENTER_CALL);
		printf("[motion_to_call] door2 call armed: record_mode=%d enter=%d\n",
			   user_data_get()->setting.record_mode, monitor_enter_mask_get());
		goto_layout(pLAYOUT(camera));
		// if (hook_state_get() == false)
		// {
		// ringplay_play_form_index(user_data_get()->setting.door2_tone, user_data_get()->setting.door_ring_volume, ringplay_doorcall_start_default_func, ringplay_doorcall_finish_default_func, false);
		// }
	}
}

/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：call camera 默认处理函数
** 返回参数说明：
***/
void layout_incoming_intercom_call_default(void)
{
	layout_prepare_intercom_in_black_transition();
	goto_layout(pLAYOUT(intercom_in));
}

void layout_call_camera_default(void)
{
	layout_interrupt_intercom_for_door_call();

	if (cur_layout_get() == pLAYOUT(motion_detection))
	{
		layout_motion_detection_prepare_camera_in();
	}

	power_amplifier_enable(false);
	ringplay_play_stop_async();
	goto_layout(pLAYOUT(calling));

} // lynn 26.3.14

#define GATE_OPEN_DELAY 1000 // ms
#define ELEVATOR_CALL_DELAY 1000
static lv_task_t *gate_open_task_t = NULL;
static void gate_open_task(lv_task_t *task_t)
{
	gate_unlock_pin_ctrl(false);
	lv_task_del(gate_open_task_t);
	gate_open_task_t = NULL;
}

// static lv_task_t *elevator_call_task_t = NULL;

// static void elevator_call_task(lv_task_t *task_t)
// {
// 	elevator_on_pin_ctrl(false);
// 	lv_task_del(elevator_call_task_t);
// 	elevator_call_task_t = NULL;
// }
/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：室内机开锁键按下 默认处理函数
** 返回参数说明：
***/
void layout_gate_open_default(void)
{
	if (gate_open_task_t == NULL)
	{
		gate_unlock_pin_ctrl(true); // gpio68 大门锁：开锁不播放铃声
		gate_open_task_t = lv_layout_task_create(gate_open_task, GATE_OPEN_DELAY, LV_TASK_PRIO_LOW, NULL);
		gate_open_task_t->clean_lock = false;
	}
}
/***
** 日期: 2022-05-20 14:29
** 作者: leo.liu
** 函数作用：室内机呼梯键按下 默认处理函数
** 返回参数说明：
***/
// void layout_elevator_call_default(void)
// {
// 	if (elevator_call_task_t == NULL)
// 	{
// 		elevator_on_pin_ctrl(true);
// 		ringplay_play_form_index(7, 100, open_ring_play_start_default_func, open_ring_play_finish_default_func, false);
// 		elevator_call_task_t = lv_layout_task_create(elevator_call_task, ELEVATOR_CALL_DELAY, LV_TASK_PRIO_LOW, NULL);
// 		elevator_call_task_t->clean_lock = false;
// 	}
// }
/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：hook state change 默认处理函数(听筒状态改变)
** 返回参数说明：
***/
bool layout_hook_state_change_default(unsigned int cmd, unsigned int arg)
{
	if (cur_layout_get() == pLAYOUT(logo))
	{
		return true;
	}
	else if (cur_layout_get() == pLAYOUT(standby))
	{
		if (cmd == true)
		{
			/* Do not swallow the handset event while the screen is idle. */
			printf("=============>>> standby hook pickup, enter home\n");
			goto_layout(pLAYOUT(home));
		}
		return true;
	}
	else if (cur_layout_get() == pLAYOUT(photo_list) ||
			 cur_layout_get() == pLAYOUT(memory_video))
	{
		if (cmd == true)
		{
			power_amplifier_enable(true);
		}
		else
		{
			goto_layout(pLAYOUT(standby));
		}
	}
	else if (cur_layout_get() == pLAYOUT(camera))
	{
		if (cmd)
		{
			layout_camera_hook_answer();
		}
		else
		{
			layout_camera_hook_hangup();
		}
	}
	else if (cur_layout_get() == pLAYOUT(intercom_in))
	{
		/* 摘机接听：须调 MsgCallAccept 与底层协议一致 */
		if (cmd == true && intercom_state_get() == INTERCOM_STATE_CALLING_IN)
		{
			/* Let the ring worker stop in the background before entering talk. */
			ringplay_play_stop_async();
			call_record_answered(0);
			printf("=============>>> intercom in talking (hook)\n");
			MsgCallAccept();
			goto_layout(pLAYOUT(intercom_talk));
		}
		else if (cmd == false)
		{
			printf("=============>>> 挂断\n");
			guard_talking_pin_ctrl(false);
			MsgCallEnd();
			intercom_state_set(INTERCOM_STATE_IDLE);
			goto_layout(pLAYOUT(standby));
		}
	}
	else if (cur_layout_get() == pLAYOUT(intercom_out))
	{
		if (cmd == false)
		{
			printf("=============>>> 挂断 \n");
			guard_talking_pin_ctrl(false);
			MsgCallEnd();
			intercom_state_set(INTERCOM_STATE_IDLE);
			goto_layout(pLAYOUT(standby));
		}
	}
	else if (cur_layout_get() == pLAYOUT(intercom_talk))
	{
		if (cmd == false)
		{
			printf("=============>>> 挂断 \n");
			guard_talking_pin_ctrl(false);
			MsgCallEnd();
			intercom_state_set(INTERCOM_STATE_IDLE);
			goto_layout(pLAYOUT(standby));
		}
	}
	else
	{

		if (cmd == false)
		{
			printf("=============>>> 待机 \n");
			goto_layout(pLAYOUT(standby));
		}
	}
	return true;
}

void layout_intercom_out_default(unsigned int send_id, unsigned int cmd)
{
		printf("=============>>> huchu \n");
		guard_talking_pin_ctrl(false);
		intercom_state_set(INTERCOM_STATE_CALL_OUT);
		goto_layout(pLAYOUT(intercom_out));
}

/***
** 日期: 2022-05-13 11:05
** 作者: leo.liu
** 函数作用：sd卡状态改变默认回调
** 返回参数说明：
***/
void layout_sdcard_state_change_default(void)
{
	if (media_sdcard_insert_check() == false)
	{
		user_data_get()->new_media_file_flag = false;
		user_data_get()->new_photo_file_flag = false;
		user_data_save();
	}
}

static MON_ENTER_FLG layout_monitor_enter_flag = MON_ENTER_MANUAL_DOOR;
/***
** 日期: 2022-05-13 11:05
** 作者: leo.liu
** 函数作用：设置进入监控的标志位
** 返回参数说明：
***/
void monitor_enter_mask_set(MON_ENTER_FLG flg)
{
	layout_monitor_enter_flag = flg;
}

/***
** 日期: 2022-05-13 11:05
** 作者: leo.liu
** 函数作用：获取进入监控的标志位
** 返回参数说明：
***/
MON_ENTER_FLG monitor_enter_mask_get(void)
{
	return layout_monitor_enter_flag;
}

/***
** 日期: 2022-05-17 08:13
** 作者: leo.liu
** 函数作用：获取当前通道的亮度值
** 返回参数说明：
***/
int monitor_display_brightness_vol_get(void)
{
	MON_CH channel = monitor_channel_get();
	if (channel == MON_CH_DOOR1)
	{
		return user_data_get()->camera.door1.bright;
	}
	else if (channel == MON_CH_DOOR2)
	{
		return user_data_get()->camera.door2.bright;
	}
	else if (channel == MON_CH_CCTV1)
	{
		return user_data_get()->camera.cctv1.bright;
	}
	else
	{
		return user_data_get()->camera.cctv2.bright;
	}
}

/***
** 日期: 2022-05-17 08:15
** 作者: leo.liu
** 函数作用：设置亮度值
** 返回参数说明：
***/
void monitor_display_brightness_vol_set(int vol)
{
	MON_CH channel = monitor_channel_get();
	if (channel == MON_CH_DOOR1)
	{
		user_data_get()->camera.door1.bright = vol;
	}
	else if (channel == MON_CH_DOOR2)
	{
		user_data_get()->camera.door2.bright = vol;
	}
	else if (channel == MON_CH_CCTV1)
	{
		user_data_get()->camera.cctv1.bright = vol;
	}
	else if (channel == MON_CH_CCTV2)
	{
		user_data_get()->camera.cctv2.bright = vol;
	}
	user_data_save();
}

/***
** 日期: 2022-05-17 08:13
** 作者: leo.liu
** 函数作用：获取当前通道对比度值
** 返回参数说明：
***/
int monitor_display_cont_vol_get(void)
{
	MON_CH channel = monitor_channel_get();
	if (channel == MON_CH_DOOR1)
	{
		return user_data_get()->camera.door1.cont;
	}
	else if (channel == MON_CH_DOOR2)
	{
		return user_data_get()->camera.door2.cont;
	}
	else if (channel == MON_CH_CCTV1)
	{
		return user_data_get()->camera.cctv1.cont;
	}
	else
	{
		return user_data_get()->camera.cctv2.cont;
	}
}

/***
** 日期: 2022-05-17 08:15
** 作者: leo.liu
** 函数作用：设置对比度值
** 返回参数说明：
***/
void monitor_display_cont_vol_set(int vol)
{
	MON_CH channel = monitor_channel_get();
	if (channel == MON_CH_DOOR1)
	{
		user_data_get()->camera.door1.cont = vol;
	}
	else if (channel == MON_CH_DOOR2)
	{
		user_data_get()->camera.door2.cont = vol;
	}
	else if (channel == MON_CH_CCTV1)
	{
		user_data_get()->camera.cctv1.cont = vol;
	}
	else if (channel == MON_CH_CCTV2)
	{
		user_data_get()->camera.cctv2.cont = vol;
	}
	user_data_save();
}

/***
** 日期: 2022-05-17 08:13
** 作者: leo.liu
** 函数作用：获取当前通道的色度值
** 返回参数说明：
***/
int monitor_display_color_vol_get(void)
{
	MON_CH channel = monitor_channel_get();
	if (channel == MON_CH_DOOR1)
	{
		return user_data_get()->camera.door1.color;
	}
	else if (channel == MON_CH_DOOR2)
	{
		return user_data_get()->camera.door2.color;
	}
	else if (channel == MON_CH_CCTV1)
	{
		return user_data_get()->camera.cctv1.color;
	}
	else
	{
		return user_data_get()->camera.cctv2.color;
	}
}

/***
** 日期: 2022-05-17 08:15
** 作者: leo.liu
** 函数作用：设置亮度值
** 返回参数说明：
***/
void monitor_display_color_vol_set(int vol)
{
	MON_CH channel = monitor_channel_get();
	if (channel == MON_CH_DOOR1)
	{
		user_data_get()->camera.door1.color = vol;
	}
	else if (channel == MON_CH_DOOR2)
	{
		user_data_get()->camera.door2.color = vol;
	}
	else if (channel == MON_CH_CCTV1)
	{
		user_data_get()->camera.cctv1.color = vol;
	}
	else if (channel == MON_CH_CCTV2)
	{
		user_data_get()->camera.cctv2.color = vol;
	}
	user_data_save();
}

// 电源指示灯默认处理函数
// void power_led_handler_default_func(void)
// {
// 	static unsigned long long timestemp = 0;
// 	static bool led_state = false;
// 	unsigned long long curr_times = user_timestamp_get();
// 	if (curr_times - timestemp > 1000)
// 	{
// 		timestemp = curr_times;
// 		if ((user_data_get()->new_media_file_flag) || (user_data_get()->new_photo_file_flag))
// 		{
// 			led_state = !led_state;
// 			power_led_enable(led_state);
// 		}
// 		else
// 		{
// 			led_state = true;
// 			power_led_enable(true);
// 		}
// 	}
// }

// 重新封装铃声播放函数
void ring_play(int index, int volume, ringplay_callback start, ringplay_callback finish, bool loop)
{
	const layout *cur = cur_layout_get();
	bool hook_media_play = hook_state_get() == true &&
						   cur != pLAYOUT(camera) &&
						   cur != pLAYOUT(intercom_in) &&
						   cur != pLAYOUT(intercom_out) &&
						   cur != pLAYOUT(intercom_talk);

	layout_media_power_amplifier_hold();

	if (user_data_get()->setting.inter_ring_volume == 0)
	{
		power_amplifier_enable(false);
	}
	else if (hook_media_play)
	{
		power_amplifier_enable(false);
	}
	else
	{
		power_amplifier_enable(true);
	}
	ringplay_play_form_index(index, volume, start, finish, loop);
}

static lv_task_t *media_pa_release_task = NULL;
/* Extra settle ticks after key-tone/AO drain before hard PA-off (100ms task). */
static int media_pa_release_settle_count = 0;
#define MEDIA_PA_RELEASE_SETTLE_TICKS 4

static bool layout_media_keep_power_amplifier(void)
{
	const layout *cur = cur_layout_get();

	/* Only pages that intentionally keep PA warm. Camera is NOT included:
	 * manual monitor must not force PA on just by entering. */
	return cur == pLAYOUT(photo_list) ||
		   cur == pLAYOUT(memory_video) ||
		   cur == pLAYOUT(memory_photo) ||
		   cur == pLAYOUT(calling) ||
		   cur == pLAYOUT(intercom_in) ||
		   cur == pLAYOUT(intercom_out) ||
		   cur == pLAYOUT(intercom_talk);
}

static void layout_media_pa_release_task_stop(void)
{
	if (media_pa_release_task == NULL)
	{
		media_pa_release_settle_count = 0;
		return;
	}

	lv_task_del(media_pa_release_task);
	printf("[media_audio_trace] %llu media PA release task canceled: settle=%d\n",
		   user_timestamp_get(), media_pa_release_settle_count);
	media_pa_release_task = NULL;
	media_pa_release_settle_count = 0;
}

static void layout_media_power_amplifier_release_task(lv_task_t *task)
{
	if (media_pa_release_task != task)
	{
		lv_task_del(task);
		return;
	}

	/* Re-entered media/intercom: keep PA and drop the pending off. */
	if (layout_media_keep_power_amplifier())
	{
		printf("[media_audio_trace] %llu media PA release aborted: re-entered layout=%p\n",
			   user_timestamp_get(), cur_layout_get());
		layout_media_pa_release_task_stop();
		return;
	}

	/* Wait until key tone finishes. */
	if (ringplay_ing_check())
	{
		printf("[media_audio_trace] %llu media PA release waiting: ringplay active\n",
			   user_timestamp_get());
		media_pa_release_settle_count = 0;
		return;
	}

	/*
	 * Wait AO residual empty before hard GPIO off.
	 * Closing PA while DAC still has samples causes the audible "啪".
	 * buffer_query < 0 means AO already closed -> treat as empty.
	 */
	{
		int remain = audio_output_buffer_query();
		if (remain > 0)
		{
			printf("[media_audio_trace] %llu media PA release waiting: ao_remain=%d\n",
				   user_timestamp_get(), remain);
			media_pa_release_settle_count = 0;
			return;
		}
	}

	/* Settle window after buffer reports empty (~400ms). */
	if (media_pa_release_settle_count < MEDIA_PA_RELEASE_SETTLE_TICKS)
	{
		media_pa_release_settle_count++;
		printf("[media_audio_trace] %llu media PA release settling: %d/%d\n",
			   user_timestamp_get(), media_pa_release_settle_count,
			   MEDIA_PA_RELEASE_SETTLE_TICKS);
		return;
	}

	/* End on a known zero-level DAC output before the PA-off test. */
	audio_output_silence_drain(1024, 120);
	/* Diagnostic only: keep PA_EN asserted to isolate GPIO9-off pop noise. */
	printf("[media_audio_trace] %llu media PA release test: GPIO9 remains on\n",
		   user_timestamp_get());
	printf("[media_audio_trace] %llu media PA release complete\n", user_timestamp_get());
	layout_media_pa_release_task_stop();
}

void layout_media_power_amplifier_hold(void)
{
	/* Cancel pending Media leave PA-off without forcing PA on. */
	if (media_pa_release_task != NULL)
	{
		printf("[media_audio_trace] %llu media PA hold requested: layout=%p\n",
			   user_timestamp_get(), cur_layout_get());
	}
	layout_media_pa_release_task_stop();
}

void layout_media_keytone_prepare(void)
{
	/*
	 * UI click path must not restart AO: ringplay may already be playing or
	 * queued, and ak_ao_cancel()/restart can make the touch tone double/cut.
	 */
	layout_media_pa_release_task_stop();
	power_amplifier_enable(true);
	ring_volume_set(TOUCH_TONE_VOL);
}

void layout_media_audio_prepare(void)
{
	/* Cancel any pending PA-off from previous leave-media/home transition. */
	layout_media_pa_release_task_stop();

	/*
	 * Media pages keep PA on for key-tone stability. Do not restart AO here:
	 * entering the next media page can happen while the previous page's touch
	 * tone is still draining, and cancel/restart would cut that tone abruptly.
	 * audio_output_open() still handles a required parameter switch.
	 */
	audio_output_open(AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000);
	power_amplifier_enable(true);
	ring_volume_set(TOUCH_TONE_VOL);
}

void layout_media_power_amplifier_release(void)
{
	if (layout_media_keep_power_amplifier())
	{
		layout_media_pa_release_task_stop();
		return;
	}

	if (media_pa_release_task != NULL)
	{
		return;
	}

	media_pa_release_settle_count = 0;
	media_pa_release_task = lv_layout_task_create(layout_media_power_amplifier_release_task,
												  100,
												  LV_TASK_PRIO_LOW,
												  NULL);
	if (media_pa_release_task != NULL)
	{
		media_pa_release_task->clean_lock = false;
	}
}

// 文件列表图标按键创建
lv_obj_t *photo_and_video_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src, bool underline, bool string_select)
{
	lv_obj_t *btn_obj = lv_obj_create(parent, NULL);
	// lv_obj_set_ext_click_area(btn_obj, 12, 12, 10, 15);
	lv_obj_set_pos(btn_obj, btn_area.x, btn_area.y);
	lv_obj_set_size(btn_obj, btn_area.w, btn_area.h);
	if (icon_src != NULL)
	{
		lv_obj_set_style_local_pattern_image(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
		lv_obj_set_style_local_pattern_align(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	}

	lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));

	// 2. 叠加透明度：默认低透明（图标显原始色），按下高透明（图标变深色）
	// lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);	// 默认无叠加（原始色）
	// lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50); // 按下叠加50%黑色（深色）

	lv_obj_set_style_local_bg_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF)); // 白色背景（或与父容器同色）
	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);				  // 完全透明，默认不显示

	lv_obj_set_style_local_bg_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xAAAAAA)); // 深灰色背景
	lv_obj_set_style_local_bg_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_100);				  // 完全不透明，显示区域

	if (string != NULL)
	{
		lv_obj_set_style_local_value_str(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string);
		lv_obj_set_style_local_value_align(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
		if (string_select)
			lv_obj_set_style_local_value_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
		else
			lv_obj_set_style_local_value_color(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
		lv_obj_set_style_local_value_ofs_y(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
		lv_obj_set_style_local_value_font(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	}

	// lv_obj_set_style_local_border_width(btn_obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
	// lv_obj_set_style_local_border_color(btn_obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x32, 0x32, 0x37));
	// lv_obj_set_style_local_border_color(btn_obj, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27));
	// // lv_obj_set_style_local_border_side(btn_obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_RIGHT);
	// // 创建底部下划线
	// if (underline)
	// {

	// 	lv_obj_set_style_local_border_side(btn_obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_RIGHT);
	// }

	obj_click_event_listen(btn_obj, btn_pdata);

	return btn_obj;
}

static lv_obj_t *time_cont = NULL;
static void home_date_display(struct tm *time)
{
	static char str[32] = {0};
	if (user_data_get()->setting.calendar == 0)
	{
		struct date temp_date =
			{
				.year = time->tm_year + 1900,
				.month = time->tm_mon + 1,
				.day = time->tm_mday};
		temp_date = gregorian2jalali(temp_date);
		sprintf(str, "%04d-%02d-%02d   %02d:%02d:%02d", temp_date.year, temp_date.month, temp_date.day, time->tm_hour, time->tm_min, time->tm_sec);
	}
	else
	{
		sprintf(str, "%04d-%02d-%02d   %02d:%02d:%02d", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec);
	}
	lv_obj_set_style_local_value_str(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str);
}

static void home_time_display_task(lv_task_t *task_t)
{
	time_t seconds = time(NULL);
	struct tm tm = {0};
	static struct tm prev_tm = {0};
	localtime_r(&seconds, &tm);

	if (prev_tm.tm_hour != tm.tm_hour || prev_tm.tm_min != tm.tm_min || prev_tm.tm_sec != tm.tm_sec || task_t == NULL)
	{
		home_date_display(&tm);
	}
	if (prev_tm.tm_year != tm.tm_year || prev_tm.tm_mon != tm.tm_mon || prev_tm.tm_mday != tm.tm_mday || task_t == NULL)
	{
		home_date_display(&tm);
	}

	prev_tm = tm;
}

bool top_time_date_text_create(lv_obj_t *parent) // 正上方时间显示 非sdandby时间
{
	/***** 创建时间容器 *****/
	time_cont = lv_cont_create(parent, NULL);
	lv_obj_set_pos(time_cont, 300, 35);
	lv_obj_set_size(time_cont, 314, 29);
	lv_obj_set_id(time_cont, HOME_TOP_TIME_DATE_ID);
	// lv_obj_align(time_cont, NULL, LV_ALIGN_IN_TOP_MID, -82, 10);

	/***** 创建日期,用value显示 *****/
	lv_obj_set_style_local_value_align(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
	lv_obj_set_style_local_value_ofs_x(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 40); //-27
	lv_obj_set_style_local_value_font(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
	lv_obj_set_style_local_value_color(time_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	time_t seconds = time(NULL);
	struct tm tm = {0};
	localtime_r(&seconds, &tm);
	home_date_display(&tm);
	lv_layout_task_create(home_time_display_task, 1000, LV_TASK_PRIO_LOWEST, NULL);
	home_time_display_task(NULL);
	return true;
}

// // 新增:左上角简写接口
// lv_obj_t *setting_left_icon_create(lv_obj_t *parent, layout_lang_id id)
// {
//     lv_obj_t *setting_icon_obj = lv_img_create(parent, NULL);
//     if(setting_icon_obj != NULL) {
//         lv_obj_set_pos(setting_icon_obj, 54, 34);
//         lv_obj_set_size(setting_icon_obj, 22, 20);
//         static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_GEAR_PNG);
//         lv_obj_set_style_local_pattern_image(setting_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
//     }

//     lv_obj_t *left_head_label = lv_label_create(parent, NULL);
//     if(left_head_label != NULL) {
//         lv_label_set_text(left_head_label, str_get(id));
//         lv_obj_set_style_local_text_color(left_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
//         lv_obj_set_style_local_text_font(left_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
//         lv_obj_align(left_head_label, setting_icon_obj, LV_ALIGN_OUT_LEFT_MID, -5, 0);
//     }
//     return setting_icon_obj;
// }

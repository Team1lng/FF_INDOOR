#include "user_data.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "stdio.h"
#include "stdlib.h"

#define USER_DATA_PATH "/app/data/user_data.cfg"

static user_data_info user_data;

static const user_data_info user_data_default = {

	.setting.language = 0,
	.setting.time_display_enable = true,
	.setting.clock_style = 0,
	.setting.intercom_receive_enable = true,

	.setting.calendar = 1,
	// .setting.window_display_enable = false,
	.setting.record_mode = 0,
	.setting.door1_tone = 1,
	.setting.door2_tone = 1,
	.setting.door_ring_volume = 2,
	// .setting.door2_ring_volume = 2,lynn 26.3.13
	.setting.key_tone_enable = true,
	.setting.inter_ring_volume = 2,
	.setting.ring_time = 5,

	.motion = {
		.enable = false,
		.select_camera = 0,
		.saving_fmt = 0,
		.sensivity = 1,
		.timer_en = false,
		.lcd_en = false,
		.record_duration = 10,
		.start =
			{
				.tm_year = 2023,
				.tm_mon = 1,
				.tm_mday = 1,
				.tm_hour = 0,
				.tm_min = 0,
				.tm_sec = 0},
		.end =
			{
				.tm_year = 2030,
				.tm_mon = 1,
				.tm_mday = 1,
				.tm_hour = 0,
				.tm_min = 0,
				.tm_sec = 0}},

	.camera.door1.bright = 5,
	.camera.door1.color = 5,
	.camera.door1.cont = 5,

	.camera.door2.bright = 5,
	.camera.door2.color = 5,
	.camera.door2.cont = 5,

/* 	.camera.door.bright = 5,  lynn 26.3.12
	.camera.door.color = 5,
	.camera.door.cont = 5, */

	.camera.cctv1.bright = 5,
	.camera.cctv1.color = 5,
	.camera.cctv1.cont = 5,

	.camera.cctv2.bright = 5,
	.camera.cctv2.color = 5,
	.camera.cctv2.cont = 5,

	.media_disp_mode = 0,

	.new_media_file_flag = false,
	.new_call_record_flag = false,

	.etc = {
		.language = 0,
		.deive_id = 0,
		.door_1_open_time = 1,
		.door_2_open_time = 1,
		.password = {"1234"},
	},
	.alarm = {
		.auto_record = false,
		.alarm_1_enable = false,
		.alarm_1_trigger = false,
		.alarm_2_enable = false,
		.alarm_2_trigger = false,
	},
	.device_id = {0x00, 0x01,},
	.other_id = {0x00, 0x01,},

	.upgrade_success_flag = false,

	.password = 1234,

	.room_no_flag = false,

	.sd_card_pattion = false,

	.new_photo_file_flag = false,

	// .change_channel_enable = false,
};

bool user_data_save(void)
{
	int fd = open(USER_DATA_PATH, O_WRONLY | O_CREAT);
	if (fd < 0)
	{
		printf("write open %s fail \n", USER_DATA_PATH);
		return false;
	}

	write(fd, &user_data, sizeof(user_data_info));

	close(fd);
	system("sync");
	return true;
}

#define user_data_check_range_out(cur, min, max)            \
	if ((user_##data.cur < min) || (user_##data.cur > max)) \
	{                                                       \
		user_##data.cur = user_##data##_default.cur;        \
		printf("user data error \n");                       \
	}

#define user_data_motion_check_range_out(x, min, max) user_data_check_range_out(motion.x, min, max)

#define user_data_audio_check_range_out(x, min, max) user_data_check_range_out(audio.x, min, max)

#define user_data_display_check_range_out(x, min, max) user_data_check_range_out(display.x, min, max)

#define user_data_etc_check_range_out(x, min, max) user_data_check_range_out(etc.x, min, max)

#define user_data_alarm_check_range_out(x, min, max) user_data_check_range_out(alarm.x, min, max)


bool user_data_init(void)
{
	int fd = open(USER_DATA_PATH, O_RDONLY);
	if (fd < 0)
	{
		printf("read open %s fail \n", USER_DATA_PATH);
		user_data = user_data_default;
		return false;
	}
	read(fd, &user_data, sizeof(user_data_info));

	close(fd);
	return true;
}

user_data_info *user_data_get(void)
{
	return &user_data;
}

void user_data_reset(void)
{
	user_data = user_data_default;
	user_data_save();
	system("sync");
}

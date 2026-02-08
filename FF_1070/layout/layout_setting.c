/*******************************************************************
 * @Descripttion   :
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-09 18:00
 * @LastEditTime   : 2022-11-24 14:59
 *******************************************************************/
#include "layout_define.h"
#include "user_data.h"
#include "lvgl/lvgl.h"
typedef enum
{
	SETTING_MEMORY_BTN_ID,
	SETTING_RINGTONE_BTN_ID,
	SETTING_LANGUAGE_BTN_ID,
	SETTING_COPY_DELETE_BTN_ID,
	SETTING_TOTAL_BTN,
} setting_btn_module;

static custom_area setting_btn_area[SETTING_TOTAL_BTN] =
	{
		{0, 0, 200, 80},
		{0, 80, 200, 80},
		{0, 160, 200, 80},
		{0, 240, 200, 80},
};


#define SETTING_AUTO_RECORD 0x06
#define SETTING_MOTION_DETECT 0x07
#define HOME_BACK_OBJ_ID 0X12
#define HOME_GEAR_OBJ_ID 0X13
#define SETTING_AUTO_PHOTO_ID 0X14
#define SETTING_AUTO_VIDEO_ID 0X15
#define SETTING_MOTION_CCTV1_ID 0X16
#define SETTING_MOTION_CCTV2_ID 0X17
#define SETTING_TICK_ID 0X18

//static int start_angle = 270;			 // 起始角度
static bool setting_change_flag = false; // 修改标记（退出时判断是否保存）
static void back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(home));
}
static void back_btn_create(lv_obj_t *parent)
{
    lv_obj_t *back_icon_obj = lv_img_create(parent, NULL);
    lv_obj_set_pos(back_icon_obj, 920, 25);
	lv_obj_set_size(back_icon_obj, 50, 37);
	lv_obj_set_id(back_icon_obj, HOME_BACK_OBJ_ID);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_TIME_BACK_PNG);
	lv_obj_set_style_local_pattern_image(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
    static obj_click_data btn_data = obj_click_data_up_create(back_btn_up);
    obj_click_event_listen(back_icon_obj, &btn_data);
}

static void setting_icon_create(lv_obj_t *parent)
{
    lv_obj_t *setting_icon_obj = lv_img_create(parent, NULL);
    lv_obj_set_pos(setting_icon_obj, 54, 34);
	lv_obj_set_size(setting_icon_obj, 22, 20);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_GEAR_PNG);
	lv_obj_set_id(setting_icon_obj, HOME_GEAR_OBJ_ID);
	lv_obj_set_style_local_pattern_image(setting_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);

    lv_obj_t *Setting_label = lv_label_create(parent, NULL);
    lv_obj_set_pos(Setting_label, 87, 28);
	lv_obj_set_size(Setting_label, 90, 31);
    lv_label_set_text(Setting_label, "Setting");
	lv_obj_align(setting_icon_obj, Setting_label, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}


static void Memory_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting));
}


static void setting_memory_btn_create(lv_obj_t *left_col_cont)
{
	static obj_click_data btn_data = obj_click_data_up_create(Memory_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_RECTANGLE_PNG);
	lv_obj_t *btn = photo_and_video_btn_create(left_col_cont, setting_btn_area[SETTING_MEMORY_BTN_ID], str_get(COMMON_LANG_MEMORY_ID), &btn_data, &info, true, true);
	lv_obj_set_id(btn, SETTING_MEMORY_BTN_ID);
}

static void setting_ringtone_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting_sound));
}


static void setting_ringtone_btn_create(lv_obj_t *left_col_cont)
{
	static obj_click_data btn_data = obj_click_data_up_create(setting_ringtone_btn_up);
	lv_obj_t *btn = photo_and_video_btn_create(left_col_cont, setting_btn_area[SETTING_RINGTONE_BTN_ID], str_get(LAYOUT_BELL_LANG_RING_ID), &btn_data, NULL, true, false);
	lv_obj_set_id(btn, SETTING_RINGTONE_BTN_ID);
}

static void setting_language_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(language));
}

static void setting_language_btn_create(lv_obj_t *left_col_cont)
{
	static obj_click_data btn_data = obj_click_data_up_create(setting_language_btn_up);
	lv_obj_t *btn = photo_and_video_btn_create(left_col_cont, setting_btn_area[SETTING_LANGUAGE_BTN_ID], str_get(LAYOUT_LANGUAGE_LANG_SELECT_ID), &btn_data, NULL, false, false);
	lv_obj_set_id(btn, SETTING_LANGUAGE_BTN_ID);
}

static void setting_copy_delete_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(sd));
}

static void setting_copy_delete_btn_create(lv_obj_t *left_col_cont)
{
	static obj_click_data btn_data = obj_click_data_up_create(setting_copy_delete_btn_up);
	lv_obj_t *btn = photo_and_video_btn_create(left_col_cont, setting_btn_area[SETTING_COPY_DELETE_BTN_ID], str_get(COMMON_LANG_COPY_DELETE_ID), &btn_data, NULL, false, false);
	lv_obj_set_id(btn, SETTING_COPY_DELETE_BTN_ID);
}



/***
** 日期: 2024-05-20 10:30
** 作者: leo.liu
** 函数作用：获取自动录像模式的显示字符串
** 返回参数说明：返回当前自动录像模式的字符串指针（关闭/视频/照片）
***/
static const char *setting_auto_record_mode_string_get(void)
{

	// 已开启，根据auto_record_mode判断是视频还是照片模式
	switch (user_data_get()->setting.record_mode)
	{
	case LAYOUT_SETTING_LANG_OFF_ID:				 // 失能
		return str_get(LAYOUT_SETTING_LANG_OFF_ID);	 // 复用现有guanbi模式字符串
	case LAYOUT_RECORD_LANG_IMAGE_ID:				 // 照片模式
		return str_get(LAYOUT_RECORD_LANG_IMAGE_ID); // 复用现有照片模式字符串
	case LAYOUT_RECORD_LANG_VIDEO_ID:				 // 视频模式
		return str_get(LAYOUT_RECORD_LANG_VIDEO_ID); // 复用现有视频模式字符串

	default: // 异常mode值：默认返回“关闭”
		return str_get(LAYOUT_SETTING_LANG_OFF_ID);
	}
}
static void setting_auto_record_id_btn_up(lv_obj_t *obj)
{
	printf("Right arrow clicked - previous auto_record_id\n");
	setting_change_flag = true;
	// 切换上一个模式
	switch (user_data_get()->setting.record_mode)
	{
	case LAYOUT_SETTING_LANG_OFF_ID: // 当前是失能模式→切换到照片模式
		user_data_get()->setting.record_mode = LAYOUT_RECORD_LANG_IMAGE_ID;
		break;
	case LAYOUT_RECORD_LANG_IMAGE_ID: // 当前是照片模式→切换到视频模式
		user_data_get()->setting.record_mode = LAYOUT_RECORD_LANG_VIDEO_ID;
		break;
	case LAYOUT_RECORD_LANG_VIDEO_ID: // 当前是视频模式→切换到关闭模式
		user_data_get()->setting.record_mode = LAYOUT_SETTING_LANG_OFF_ID;
		break;
	default: // 异常mode→默认切换到关闭模式
		user_data_get()->setting.record_mode = LAYOUT_SETTING_LANG_OFF_ID;
		break;
	}

	//更新UI显示
	lv_obj_t *auto_obj = lv_obj_get_child_form_id(lv_scr_act(), 100);
	if (auto_obj == NULL)
	{
    	printf("obj not found\n");
    	return;
	}
	lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(auto_obj, SETTING_AUTO_RECORD), 
									LV_CONT_PART_MAIN, LV_STATE_DEFAULT, setting_auto_record_mode_string_get());
	user_data_save();
}
static void setting_auto_record_id_right_btn_up(lv_obj_t *obj)
{
	printf("Right arrow clicked - next auto_record_id\n");
	setting_change_flag = true;
	// 切换下一个模式
	switch (user_data_get()->setting.record_mode)
	{
	case LAYOUT_SETTING_LANG_OFF_ID: // 当前是关闭模式→切换到视频模式
		user_data_get()->setting.record_mode = LAYOUT_RECORD_LANG_VIDEO_ID;
		break;
	case LAYOUT_RECORD_LANG_VIDEO_ID: // 当前是视频模式→切换到照片模式
		user_data_get()->setting.record_mode = LAYOUT_RECORD_LANG_IMAGE_ID;
		break;
	case LAYOUT_RECORD_LANG_IMAGE_ID: // 当前是照片模式→切换到关闭模式
		user_data_get()->setting.record_mode = LAYOUT_SETTING_LANG_OFF_ID;
		break;
	default: // 异常mode→默认切换到关闭模式
		user_data_get()->setting.record_mode = LAYOUT_SETTING_LANG_OFF_ID;
		break;
	}

	// 更新UI显示
	lv_obj_t *auto_obj = lv_obj_get_child_form_id(lv_scr_act(), 100);
	lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(auto_obj, SETTING_AUTO_RECORD), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, setting_auto_record_mode_string_get());
	if (auto_obj == NULL)
	{
		printf("obj not found\n");
		return;
	}
	user_data_save();
}

static void setting_motion_detection_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting_motion_detection));
}


/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建auto_record设置按钮
** 返回参数说明：
***/
static bool setting_auto_photo_btn_create(lv_obj_t *center_cont)
{
	static obj_click_data click_data = obj_click_data_up_create(setting_auto_record_id_btn_up);
	static obj_click_data right_click_data = obj_click_data_up_create(setting_auto_record_id_right_btn_up);
	setting_double_arrow_btn_create(center_cont, 220, 120 + (50 * 0), 425, 32,
								str_get(SETTING_LANG_AUTO_RECORD_ID),
								setting_auto_record_mode_string_get(),
								&click_data,
								&right_click_data,
								SETTING_AUTO_RECORD);

	return true;
}

static bool setting_motion_detection_btn_create(lv_obj_t *center_cont)
{
	static obj_click_data right_click_data = obj_click_data_up_create(setting_motion_detection_btn_up);
	lv_obj_t *btn = setting_double_arrow_btn_create(center_cont, 220, 193 + (50 * 0), 425, 32,
									str_get(LAYOUT_SETTING_LANG_MOTION_DETECTION_ID),
									user_data_get()->motion.enable ? str_get(LAYOUT_SETTING_LANG_ON_ID) : str_get(LAYOUT_SETTING_LANG_OFF_ID),
									&right_click_data,
									NULL,
									SETTING_MOTION_DETECT);
	return true;
	lv_obj_set_ext_click_area(btn, 200, 50, 20, 20);

}


// static bool setting_motion_detection_btn_create(lv_obj_t *center_cont)
// {
// 	static obj_click_data click_data = obj_click_data_up_create(setting_motion_detection_btn_up);
// 	setting_right_btn_base_create(center_cont, 220, 193 + (50 * 0), 390, 32,
// 								  str_get(LAYOUT_SETTING_LANG_MOTION_DETECTION_ID),
// 								  user_data_get()->motion.enable ? str_get(LAYOUT_SETTING_LANG_ON_ID) : str_get(LAYOUT_SETTING_LANG_OFF_ID),
// 								  &click_data,
// 								  SETTING_MOTION_DETECT);
// 	return true;
// }





static void LAYOUT_ENTER_FUNC(setting)
{
	lv_obj_set_style_local_text_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	//整体容器
	setting_change_flag = false;
	lv_obj_t *parent = common_bg_display(lv_scr_act());
    lv_obj_t *center_cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(center_cont, 664, 336);
	lv_obj_set_pos(center_cont, 180, 141);  
    lv_obj_set_id(center_cont, 100);
    lv_obj_set_style_local_bg_color(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x838383));
    lv_obj_set_style_local_radius(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_bg_opa(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
	lv_obj_set_style_local_border_width(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
	//左边容器
	lv_obj_t *left_col_cont=lv_cont_create(center_cont,NULL);
	lv_obj_set_size(left_col_cont, 200, 336);
	lv_obj_set_pos(left_col_cont, 0, 0);
	lv_obj_set_style_local_bg_color(left_col_cont,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,lv_color_hex(0x616688));
	lv_obj_set_style_local_radius(left_col_cont,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,10);
	lv_obj_set_style_local_bg_opa(left_col_cont,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_40);
	lv_obj_set_style_local_border_width(left_col_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(left_col_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
	
	back_btn_create(parent);
	setting_icon_create(parent);
	top_time_date_text_create(parent);

	setting_memory_btn_create(left_col_cont);
	setting_ringtone_btn_create(left_col_cont);
	setting_language_btn_create(left_col_cont);
	setting_copy_delete_btn_create(left_col_cont);
	setting_auto_photo_btn_create(center_cont);
	setting_motion_detection_btn_create(center_cont);
}

static void LAYOUT_QUIT_FUNC(setting)
{
	if(setting_change_flag )
	{
	user_data_save();
	}
}

CREATE_LAYOUT(setting);





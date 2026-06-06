/*******************************************************************
 * @Descripttion   :
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-09 18:00
 * @LastEditTime   : 2022-11-24 14:59
 *******************************************************************/
#include "layout_define.h"
#include "lv_msg_event.h"
#include "lvgl/lvgl.h"
#include "lvgl/lv_obj.h"
#include "lvgl/lv_label.h"
#include "language.h"
#include "stdio.h"

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


#define HOME_BACK_OBJ_ID 0x10
#define HOME_GEAR_OBJ_ID 0x11
#define SETTING_VOLUME_ID 0x12
#define SETTING_CAM1_ID 0x13
#define SETTING_CAM2_ID 0X14


static char volume_str[3] = {0};
static char ring_cctv1_str[3] = {0};
static char ring_cctv2_str[3] = {0};

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
    lv_obj_set_style_local_pattern_recolor(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor_opa(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50); 
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
    lv_label_set_text(Setting_label,str_get(COMMON_LANG_LEFT_HEAD_SETTING_ID));
	lv_obj_align(setting_icon_obj, Setting_label, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}

// 获取当前主音量
static int door_ring_volume_get(void)
{
    return user_data_get()->setting.door_ring_volume;
}

// 设置音量
static void door_ring_volume_set(int vol)
{
    user_data_get()->setting.door_ring_volume = vol;

}

static void door_ring_play_start_cb(int index)
{
    ring_volume_set(user_data_get()->setting.door_ring_volume);
}

static void door_ring_play_finish_cb(int index)
{
    // 播放完成处理
}

// 铃声音量调节回调（右箭头 - 音量加）
static void setting_sound_ring_volume_btn_right_btn_up(lv_obj_t *obj)
{
    int bell_volume = door_ring_volume_get();
    if (bell_volume < 4)
    {
        bell_volume++;
    }
    door_ring_volume_set(bell_volume);
    ring_volume_set(door_ring_volume_get());
    // 先格式化字符串，再更新显示
    sprintf(volume_str, "%02d", bell_volume);
    
    lv_obj_t *center_container = lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_VOLUME_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, volume_str);
    lv_obj_invalidate(center_container);

    // 如果音量大于0，播放铃声预览
    if (bell_volume > 0)
    {
        // 预览时默认播放门口机1的铃声，或者根据需要调整
        int preview_index = user_data_get()->setting.door1_tone; 
        ringplay_play_form_index(preview_index, 100, door_ring_play_start_cb, door_ring_play_finish_cb, false);
    }
}

// 铃声音量调节回调（左箭头 - 音量减）
static void setting_sound_ring_volume_btn_left_btn_up(lv_obj_t *obj)
{
    int bell_volume = door_ring_volume_get();
    if (bell_volume > 0)
    {
        bell_volume--;
    }
    door_ring_volume_set(bell_volume);
    ring_volume_set(door_ring_volume_get());
    // 先格式化字符串，再更新显示
    sprintf(volume_str, "%02d", bell_volume);

    lv_obj_t *center_container = lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_VOLUME_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, volume_str);
    lv_obj_invalidate(center_container);

    if (bell_volume > 0)
    {
        int preview_index = user_data_get()->setting.door1_tone;
        ringplay_play_form_index(preview_index, 100, door_ring_play_start_cb, door_ring_play_finish_cb, false);
    }
}

// 门口机1铃声选择（右箭头）
static void setting_cam1_sound_song_right_btn_up(lv_obj_t *obj)
{
    // 直接读取和设置 door1_tone
    int index = user_data_get()->setting.door1_tone;
    if (index >= 1 && index <= 6)
    {
        index = (index >= 6) ? 1 : index + 1;
        user_data_get()->setting.door1_tone = index;

        // 先格式化字符串
        sprintf(ring_cctv1_str, "%02d", index);

        lv_obj_t *center_container = lv_obj_get_child_form_id(lv_scr_act(), 100);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_CAM1_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, ring_cctv1_str);
        lv_obj_invalidate(center_container);

        if (door_ring_volume_get() > 0)
        {
            ringplay_play_form_index(index, 100, door_ring_play_start_cb, door_ring_play_finish_cb, false);
        }
    }
}

// 门口机1铃声选择（左箭头）
static void setting_cam1_sound_song_left_btn_up(lv_obj_t *obj)
{
    // 直接读取和设置 door1_tone
    int index = user_data_get()->setting.door1_tone;
    if (index >= 1 && index <= 6)
    {
        index = (index <= 1) ? 6 : index - 1;
        user_data_get()->setting.door1_tone = index;

        // 先格式化字符串
        sprintf(ring_cctv1_str, "%02d", index);

        lv_obj_t *center_container = lv_obj_get_child_form_id(lv_scr_act(), 100);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_CAM1_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, ring_cctv1_str);
        lv_obj_invalidate(center_container);

        if (door_ring_volume_get() > 0)
        {
            ringplay_play_form_index(index, 100, door_ring_play_start_cb, door_ring_play_finish_cb, false);
        }
    }
}

// 门口机2铃声选择（右箭头）
static void setting_cam2_sound_song_right_btn_up(lv_obj_t *obj)
{
    // 直接读取和设置 door2_tone
    int index = user_data_get()->setting.door2_tone;
    index = (index >= 6) ? 1 : index + 1;
    user_data_get()->setting.door2_tone = index;

    // 先格式化字符串
    sprintf(ring_cctv2_str, "%02d", index);

    // 更新显示
    lv_obj_t *obj_door2_tone = lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(obj_door2_tone, SETTING_CAM2_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, ring_cctv2_str);
    lv_obj_invalidate(obj_door2_tone);

    if (user_data_get()->setting.door_ring_volume > 0)
    {
        ringplay_play_form_index(index, 100, door_ring_play_start_cb, door_ring_play_finish_cb, false);
    }
}

// 门口机2铃声选择（左箭头）
static void setting_cam2_sound_song_left_btn_up(lv_obj_t *obj)
{
    // 直接读取和设置 door2_tone
    int index = user_data_get()->setting.door2_tone;
    if (index >= 1 && index <= 6)
    {
        index = (index <= 1) ? 6 : index - 1;
        user_data_get()->setting.door2_tone = index;

        // 先格式化字符串
        sprintf(ring_cctv2_str, "%02d", index);

        lv_obj_t *center_container = lv_obj_get_child_form_id(lv_scr_act(), 100);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_CAM2_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, ring_cctv2_str);
        lv_obj_invalidate(center_container);

        if (door_ring_volume_get() > 0)
        {
            ringplay_play_form_index(index, 100, door_ring_play_start_cb, door_ring_play_finish_cb, false);
        }
    }
}

static void Memory_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting));
}

static void setting_memory_btn_create(lv_obj_t *left_col_cont)
{
	static obj_click_data btn_data = obj_click_data_up_create(Memory_btn_up);
	lv_obj_t *btn = photo_and_video_btn_create(left_col_cont, setting_btn_area[SETTING_MEMORY_BTN_ID], str_get(COMMON_LANG_MEMORY_ID), &btn_data, NULL, true, true);
	lv_obj_set_id(btn, SETTING_MEMORY_BTN_ID);
}

static void setting_ringtone_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(setting_sound));
}

static void setting_ringtone_btn_create(lv_obj_t *left_col_cont)
{
	static obj_click_data btn_data = obj_click_data_up_create(setting_ringtone_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_RECTANGLE_PNG);
	lv_obj_t *btn = photo_and_video_btn_create(left_col_cont, setting_btn_area[SETTING_RINGTONE_BTN_ID], str_get(LAYOUT_BELL_LANG_RING_ID), &btn_data, &info, true, false);
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

static bool setting_sound_ring_volume_btn_create(lv_obj_t *center_cont)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_sound_ring_volume_btn_right_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_sound_ring_volume_btn_left_btn_up);

    int ring_volume = user_data_get()->setting.door_ring_volume;
    sprintf(volume_str, "%02d", ring_volume);
    setting_double_arrow_btn_create(center_cont, 230, 70 + (75* 0), 415, 32,
                                    str_get(COMMON_LANG_SETTING_VOLUME_ID),
                                    volume_str,
                                    &click_data,
                                    &left_click_data,
                                    SETTING_VOLUME_ID);
    return true;
}

static bool setting_cam1_melody_btn_create(lv_obj_t *center_cont)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_cam1_sound_song_right_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_cam1_sound_song_left_btn_up);

    int ring_index  = user_data_get()->setting.door1_tone;
    sprintf(ring_cctv1_str, "%02d", ring_index );
    setting_double_arrow_btn_create(center_cont, 230, 70 + (75 * 1), 415, 32,
                                    str_get(COMMON_LANG_CAM1_MELODY_ID),
                                    ring_cctv1_str,
                                    &click_data,
                                    &left_click_data,
                                    SETTING_CAM1_ID);
    return true;
}

static bool setting_cam2_melody_btn_create(lv_obj_t *center_cont)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_cam2_sound_song_right_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_cam2_sound_song_left_btn_up);

    int ring_index = user_data_get()->setting.door2_tone;
    sprintf(ring_cctv2_str, "%02d", ring_index);
    
    setting_double_arrow_btn_create(center_cont, 230, 70 + (75 * 2), 415, 32,
                                    str_get(COMMON_LANG_CAM2_MELODY_ID),
                                    ring_cctv2_str, 
                                    &click_data,
                                    &left_click_data,
                                    SETTING_CAM2_ID);
    return true;
}

static void LAYOUT_ENTER_FUNC(setting_sound)
{
	lv_obj_t *parent = common_bg_display(lv_scr_act());
    lv_obj_t *center_cont = lv_cont_create(parent, NULL);
    lv_obj_set_id(center_cont, 100);
    if(center_cont == NULL)
    {
        return;
    }
    lv_obj_set_size(center_cont, 664, 336); 
	lv_obj_set_pos(center_cont, 180, 141); 
    lv_obj_set_style_local_bg_color(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x838383)); 
    lv_obj_set_style_local_radius(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10); 
    lv_obj_set_style_local_bg_opa(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40); 

	lv_obj_t *left_col_cont=lv_cont_create(center_cont,NULL);
	lv_obj_set_size(left_col_cont, 200, 336); 
	lv_obj_set_pos(left_col_cont, 0, 0);
	lv_obj_set_style_local_bg_color(left_col_cont,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,lv_color_hex(0x616688)); 
	lv_obj_set_style_local_radius(left_col_cont,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,10); 
	lv_obj_set_style_local_bg_opa(left_col_cont,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_40); 
	lv_obj_set_style_local_border_opa(left_col_cont,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_TRANSP); 

	setting_memory_btn_create(left_col_cont);    
	setting_ringtone_btn_create(left_col_cont);  
	setting_language_btn_create(left_col_cont);  
	setting_copy_delete_btn_create(left_col_cont);

	back_btn_create(parent);
	setting_icon_create(parent);
	top_time_date_text_create(parent);

    setting_sound_ring_volume_btn_create(center_cont);
    setting_cam1_melody_btn_create(center_cont);
    setting_cam2_melody_btn_create(center_cont);
}

static void LAYOUT_QUIT_FUNC(setting_sound)
{
    // 退出时停止铃声播放
    ringplay_play_stop();
    // 保存用户数据
    user_data_save();
}

CREATE_LAYOUT(setting_sound);
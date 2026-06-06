#include "layout_define.h"

#define HOME_BACK_OBJ_ID 0x10
#define HOME_GEAR_OBJ_ID 0x11
#define SETTING_CP_SD_BIN_ID 0X12
#define LAYOUT_SETTING_ADJUST_OBJ_MSG_ID 0X13
#define LAYOUT_SETTING_ADJUST_OBJ_BTNMATRIX_ID 0X14
#define SETTING_SDCARD_BACE_BTN_ID 0x15
#define CENTER_CONT_ID 0X16

static int delete_media_total = 0;
static bool setting_change_flag = false; 
static file_type delete_file_type = FILE_TYPE_NONE;
static int exit_time_count = 0;

static void back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(sd));
}
static void back_btn_create(lv_obj_t *parent)
{
    lv_obj_t *back_icon_obj = lv_img_create(parent, NULL);
    lv_obj_set_pos(back_icon_obj, 920, 25);
	lv_obj_set_size(back_icon_obj, 50, 37);
	lv_obj_set_id(back_icon_obj, HOME_BACK_OBJ_ID);
    lv_obj_set_style_local_pattern_recolor(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor_opa(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50); // 按下叠加50%黑色（深色）
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
extern void setting_string_format_switch(char *dst_str, const char *src_str, int num);


static void delete_yes_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(sd));
}

static void delete_file_state_display_task(lv_task_t *task_t)
{
    if (delete_file_type != FILE_TYPE_NONE)
    {
        // char str[40] = {0};
        if (!media_file_delete_all_state())
        {
            // setting_string_format_switch(str, str_get(LAYOUT_DELETE_LANG_DELETING_ID), 100);
            // lv_label_set_text(task_t->user_data, str);
            delete_file_type = FILE_TYPE_NONE;
            exit_time_count = 1;
        }
        // else
        // {
        //     int cur_total = high_speed_media_file_total_get(delete_file_type);
        //     setting_string_format_switch(str, str_get(LAYOUT_DELETE_LANG_DELETING_ID), (int)((delete_media_total - cur_total) * 100 / delete_media_total));
        //     lv_label_set_text(task_t->user_data, str);
        // }
    }
    else
    {
        if (--exit_time_count <= 0)
        {
            goto_layout(pLAYOUT(setting));
        }
    }
}

static void delete_file_state_display_task_create(const char *state_info)
{
    standby_timer_close();
    // lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    // lv_label_set_text(label, state_info);
    // lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    // lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 160);
    exit_time_count = 1;
    lv_layout_task_create(delete_file_state_display_task, 500, LV_TASK_PRIO_LOWEST, NULL);

    if (delete_file_type != FILE_TYPE_NONE)
    {
        media_file_delete_all_start(delete_file_type);
    }
}

static void delete_picture_yes_btn_up(lv_obj_t *obj)
{
    if (media_file_delete_all_state())
    {
        return;
    }
    lv_obj_set_click(lv_obj_get_child_form_id(obj->parent, 1), false);                           // yes按键禁止点击
    lv_obj_set_click(lv_obj_get_child_form_id(obj->parent, 2), false);                           // no按键禁止点击
    //lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), SETTING_DELETE_BACE_BTN_ID), false); // 退出按键禁止点击

    if (media_sdcard_insert_check() == true)
    {
        delete_file_type = FILE_TYPE_PHOTO;
    }
    else
    {
        delete_file_type = FILE_TYPE_FLASH_PHOTO;
    }
    int total;
    media_file_total_get(delete_file_type, &total, NULL);
    delete_media_total = total;
    // char str[40] = {0};
    // if (total == 0)
    // {
    //     delete_file_type = FILE_TYPE_NONE;
    //     setting_string_format_switch(str, str_get(LAYOUT_DELETE_LANG_DELETING_ID), 100);
    //     delete_file_state_display_task_create(str);
    // }
    // else
    // {
    //     setting_string_format_switch(str, str_get(LAYOUT_DELETE_LANG_DELETING_ID), 0);
    //     delete_file_state_display_task_create(str);
    // }
    if (total == 0)
    {
        delete_file_type = FILE_TYPE_NONE;
    }

    delete_file_state_display_task_create(NULL);
    user_data_get()->new_photo_file_flag = 0;   
}

static void delete_picture_msg_box_create(lv_obj_t *center_cont)
{
    static obj_click_data btn_data = obj_click_data_up_create(delete_yes_btn_up);
    static obj_click_data btn_data1 = obj_click_data_up_create(delete_picture_yes_btn_up);
    message_box_create(lv_scr_act(), str_get(SETTING_DELETE_SD_PHOTO_ID), &btn_data1, &btn_data);
}

// static void sdcard_no_btn_up(lv_obj_t *obj)
// {
//     goto_layout(pLAYOUT(sd));
// }


// void  create_delete_sd_photo(lv_obj_t *center_cont)
// {
    
//    	static obj_click_data btn_data = obj_click_data_up_create(sdcard_copy_yes_btn_up);
// 	static obj_click_data btn_data2 = obj_click_data_up_create(sdcard_no_btn_up);
// 	message_box_create(center_cont, str_get(COMMON_LANG_CP_SD_ID), &btn_data, &btn_data2);
// 	 lv_obj_t *dialog = lv_obj_get_child(center_cont, NULL); // 获取第一个子控件
//     if (dialog != NULL) {
//         lv_obj_set_id(dialog, LAYOUT_SETTING_ADJUST_OBJ_MSG_ID); // 绑定弹窗ID
//         printf("dialog addr: %p, bind ID: %d\n", dialog, LAYOUT_SETTING_ADJUST_OBJ_MSG_ID);
//     } else {
//         printf("dialog create failed in create_cp_sd\n");
//     }
// }


static void LAYOUT_ENTER_FUNC(delete_sd_photo)
{
	lv_obj_t *parent = common_bg_display(lv_scr_act());
	back_btn_create(parent);
	setting_icon_create(parent);
	top_time_date_text_create(parent);
    delete_picture_msg_box_create(parent);
	//create_cp_sd(center_cont);
}

static void LAYOUT_QUIT_FUNC(delete_sd_photo)
{
    if (setting_change_flag == true)
	user_data_save();

}

CREATE_LAYOUT(delete_sd_photo);

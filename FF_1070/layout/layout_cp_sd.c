#include "layout_define.h"

#define HOME_BACK_OBJ_ID 0x10
#define HOME_GEAR_OBJ_ID 0x11
#define SETTING_CP_SD_BIN_ID 0X12
#define LAYOUT_SETTING_ADJUST_OBJ_MSG_ID 0X13
#define LAYOUT_SETTING_ADJUST_OBJ_BTNMATRIX_ID 0X14
#define SETTING_SDCARD_BACE_BTN_ID 0x15
#define CENTER_CONT_ID 0X16

static int exit_time_count = 0;
//static int delete_media_total = 0;
static bool setting_change_flag = false; 
static int delete_media_total = 0;

extern void setting_string_format_switch(char *dst_str, const char *src_str, int num);


static void sdcard_copy_state_display_task(lv_task_t *task_t)
{
    char str[40] = {0};
    int copied_photo = 0;
    if (media_copy_flash_photo_to_sd_state(&copied_photo) == false)
    {
        if(media_sdcard_insert_check())
        {
            setting_string_format_switch(str, str_get(LAYOUT_SD_LANG_COPYING_ID), 100);
            lv_label_set_text(task_t->user_data, str);
            lv_obj_align(task_t->user_data, NULL, LV_ALIGN_IN_TOP_MID, 0, 165);
        }
        if (--exit_time_count <= 0)
        {
            goto_layout(pLAYOUT(setting));
        }
    }
    else
    {
        setting_string_format_switch(str, str_get(LAYOUT_SD_LANG_COPYING_ID), (int)(copied_photo * 100 / delete_media_total));
        printf("=====================>>> 已拷贝照片:[%d]\n", copied_photo);
        lv_label_set_text(task_t->user_data, str);
        lv_obj_align(task_t->user_data, NULL, LV_ALIGN_IN_TOP_MID, 0, 165);
    }
}

/**
 * @brief  SD卡复制确认按钮点击处理函数（优化版）
 * @param  obj: 按钮对象指针
 */
static void sdcard_copy_yes_btn_up(lv_obj_t *obj)
{
    if (media_copy_flash_photo_to_sd_state(NULL))
    {
        return;
    }
    // lv_obj_set_click(lv_obj_get_child_form_id(obj->parent, 1), false);//yes按键禁止点击
    // lv_obj_set_click(lv_obj_get_child_form_id(obj->parent, 2), false);//no按键禁止点击
    // lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), SETTING_SDCARD_BACE_BTN_ID), false);//退出按键禁止点击

    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    if(media_sdcard_insert_check())
    {
        delete_media_total = high_speed_media_file_total_get(FILE_TYPE_FLASH_PHOTO);
        char str[40] = {0};
        if (delete_media_total == 0)
        {
            setting_string_format_switch(str, str_get(LAYOUT_SD_LANG_COPYING_ID), 100);
            lv_label_set_text(label, str);
        }
        else
        {
            setting_string_format_switch(str, str_get(LAYOUT_SD_LANG_COPYING_ID), 0);
            lv_label_set_text(label, str);
            media_copy_flash_photo_to_sd();
        }
    }
    else
    {
        lv_label_set_text(label, str_get(COMMON_LANG_NO_SD_ID));
    }
    
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 165);
    exit_time_count = 3;
    lv_layout_task_create(sdcard_copy_state_display_task, 200, LV_TASK_PRIO_LOWEST, label);
}




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




static void sdcard_no_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(sd));
}


void  create_cp_sd(lv_obj_t *center_cont)
{
    
   	static obj_click_data btn_data = obj_click_data_up_create(sdcard_copy_yes_btn_up);
	static obj_click_data btn_data2 = obj_click_data_up_create(sdcard_no_btn_up);
	message_box_create(center_cont, str_get(COMMON_LANG_CP_SD_ID), &btn_data, &btn_data2);
	 lv_obj_t *dialog = lv_obj_get_child(center_cont, NULL); // 获取第一个子控件
    if (dialog != NULL) {
        lv_obj_set_id(dialog, LAYOUT_SETTING_ADJUST_OBJ_MSG_ID); // 绑定弹窗ID
        printf("dialog addr: %p, bind ID: %d\n", dialog, LAYOUT_SETTING_ADJUST_OBJ_MSG_ID);
    } else {
        printf("dialog create failed in create_cp_sd\n");
    }
}




static void LAYOUT_ENTER_FUNC(cp_sd)
{
	lv_obj_t *parent = common_bg_display(lv_scr_act());
    lv_obj_t *center_cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(center_cont, 466, 158);  
    lv_obj_set_id(center_cont, CENTER_CONT_ID);
    lv_obj_align(center_cont, parent,LV_ALIGN_CENTER, 0, 0);  
    lv_obj_set_style_local_bg_color(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x333355));
    lv_obj_set_style_local_radius(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_bg_opa(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
	printf("center_cont addr: %p, bind ID: %d\n", center_cont, CENTER_CONT_ID);
	back_btn_create(parent);
	setting_icon_create(parent);
	top_time_date_text_create(parent);
	create_cp_sd(center_cont);
}



static void LAYOUT_QUIT_FUNC(cp_sd)
{
    if (setting_change_flag == true)
	user_data_save();

}

CREATE_LAYOUT(cp_sd);

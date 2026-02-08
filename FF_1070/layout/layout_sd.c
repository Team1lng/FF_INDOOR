#include "layout_define.h"

typedef enum
{
	SETTING_MEMORY_BTN_ID,
	SETTING_RINGTONE_BTN_ID,
	SETTING_LANGUAGE_BTN_ID,
	SETTING_COPY_DELETE_BTN_ID,
    SETTING_COPY_SD_BIN_ID,
    SETTING_DELETE_VIDEO_BIN_ID,
    SETTING_DELETE_SD_PHOTO_BINT_ID,
    SETTING_DELETE_FLASH_PHOTO_BINT_ID,
	SETTING_TOTAL_BTN,
} setting_btn_module;

static custom_area setting_btn_area[SETTING_TOTAL_BTN] =
	{
		{0, 0, 200, 80},
		{0, 80, 200, 80},
		{0, 160, 200, 80},
		{0, 240, 200, 80},
        // {230, 60, 280, 31},
        // {230, 125, 280, 31},
        // {230, 190, 280, 31},
        // {230, 255, 280, 31},
};

#define HOME_BACK_OBJ_ID 0x10
#define HOME_GEAR_OBJ_ID 0x11

static bool setting_change_flag = false;

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
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_RECTANGLE_PNG);
	lv_obj_t *btn = photo_and_video_btn_create(left_col_cont, setting_btn_area[SETTING_COPY_DELETE_BTN_ID], str_get(COMMON_LANG_COPY_DELETE_ID), &btn_data, &info, false, false);
	lv_obj_set_id(btn, SETTING_COPY_DELETE_BTN_ID);
}




static void setting_cp_sd_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(cp_sd));
}


static void setting_delete_video_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(delete_video));
}


static void setting_delete_sd_photo_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(delete_sd_photo));
}

static void setting_delete_flash_photo_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(delete_flash_photo));
}

void create_setting_cp_delete_page(lv_obj_t *center_cont)
{
    static obj_click_data copy_sd_click_data = obj_click_data_up_create(setting_cp_sd_btn_up);
    custom_area area1 = {.x = 285, .y = 60+(65*0), .w = 280, .h = 31};
    setting_right_btn_base_create(center_cont, area1.x, area1.y, area1.w, area1.h, 
                                  str_get(COMMON_LANG_COPY_SD_ID),
                                  NULL,
                                  &copy_sd_click_data, 
                                  SETTING_COPY_SD_BIN_ID);

    static obj_click_data delete_video_click_data = obj_click_data_up_create(setting_delete_video_btn_up);
    custom_area area2 = {.x = 285, .y = 60+(65*1), .w = 280, .h = 31};
    setting_right_btn_base_create(center_cont, area2.x, area2.y, area2.w, area2.h, 
                                  str_get(COMMON_LANG_DELETE_VIDEO_ID),
                                  NULL,
                                  &delete_video_click_data, 
                                  SETTING_DELETE_VIDEO_BIN_ID);

    static obj_click_data delete_sd_photo_click_data = obj_click_data_up_create(setting_delete_sd_photo_btn_up);
    custom_area area3 = {.x = 285, .y = 60+(65*2), .w = 280, .h = 31};
    setting_right_btn_base_create(center_cont, area3.x, area3.y, area3.w, area3.h, 
                                  str_get(COMMON_LANG_DELETE_SD_PHOTO_ID),
                                  NULL,
                                  &delete_sd_photo_click_data, 
                                  SETTING_DELETE_SD_PHOTO_BINT_ID);
                                
    static obj_click_data delete_flash_photo_click_data = obj_click_data_up_create(setting_delete_flash_photo_btn_up);
    custom_area area4 = {.x = 285, .y = 60+(65*3), .w = 280, .h = 31};
    setting_right_btn_base_create(center_cont, area4.x, area4.y, area4.w, area4.h, 
                                  str_get(COMMON_LANG_DELETE_FLASH_PHOTO_ID),
                                  NULL,
                                  &delete_flash_photo_click_data, 
                                  SETTING_DELETE_FLASH_PHOTO_BINT_ID);
}


static void LAYOUT_ENTER_FUNC(sd)
{
	lv_obj_t *parent = common_bg_display(lv_scr_act());
	setting_change_flag = false;
    lv_obj_t *center_cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(center_cont, 664, 336); 
	lv_obj_set_pos(center_cont, 180, 141); 
    lv_obj_set_id(center_cont, 100); 

    lv_obj_set_style_local_bg_color(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x838383));
    lv_obj_set_style_local_radius(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_bg_opa(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);

	lv_obj_t *left_col_cont = lv_cont_create(center_cont, NULL);
	lv_obj_set_size(left_col_cont, 200, 336); 
	lv_obj_set_pos(left_col_cont, 0, 0);
	lv_obj_set_style_local_bg_color(left_col_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x616688));
	lv_obj_set_style_local_radius(left_col_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10); 
	lv_obj_set_style_local_bg_opa(left_col_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
	lv_obj_set_style_local_border_opa(left_col_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    back_btn_create(parent);
	setting_icon_create(parent);
	top_time_date_text_create(parent);

    setting_memory_btn_create(left_col_cont);
	setting_ringtone_btn_create(left_col_cont);
	setting_language_btn_create(left_col_cont);
	setting_copy_delete_btn_create(left_col_cont);

    create_setting_cp_delete_page(center_cont);
}

static void LAYOUT_QUIT_FUNC(sd)
{
    user_data_save();
}

CREATE_LAYOUT(sd);
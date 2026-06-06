#include "layout_define.h"

#define HOME_BACK_OBJ_ID 0X10
#define HOME_GEAR_OBJ_ID 0X11
#define LANGUAGE_ENGLISH_ID 0X12
#define LANGUAGE_POLISH_ID 0X13
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

static bool english_checked_state = false;
static bool persian_checked_state = false;

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
    lv_obj_set_style_local_pattern_recolor(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor_opa(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50); // 按下叠加50%黑色（深色）
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
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_RECTANGLE_PNG);
	lv_obj_t *btn = photo_and_video_btn_create(left_col_cont, setting_btn_area[SETTING_LANGUAGE_BTN_ID], str_get(LAYOUT_LANGUAGE_LANG_SELECT_ID), &btn_data, &info, false, false);
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

// static void language_english_checkbox_click_btn_up(lv_obj_t *obj)
// {    
// 	static rom_bin_info image_off_d = rom_bin_info_get(ROM_UI_MEDIA_LIST_CHECKBOX_PNG);
//     static rom_bin_info image_on_d = rom_bin_info_get(ROM_UI_MEDIA_LIST_CHECKBOX_FOCUS_PNG);
// 	if (obj == NULL) {
//         printf("Invalid checkbox object!\n");
//         return;
//     }
//     bool *is_showing_on = (bool *)lv_checkbox_is_checked(obj);
// 	if (is_showing_on == NULL) {
//         printf("No state data found for checkbox!\n");
//         return;
//     }

//     if (*is_showing_on) {
//         lv_obj_set_style_local_pattern_image(obj, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, &image_off_d);
//         *is_showing_on = false; 
//     } else {
        
//         lv_obj_set_style_local_pattern_image(obj, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, &image_on_d);
//         *is_showing_on = true;

//     lv_obj_invalidate(obj);  
//     }
//     if(user_data_get()->setting.language == LANG_ENGLISH) return;
//     if(user_data_get()->setting.language == LANG_PERSIAN)
//     {
//         user_data_get()->setting.language = LANG_ENGLISH;
// 	    lv_ft_font_set_type(user_data_get()->setting.language);
//         lv_font_afresh_init();
//         goto_layout(pLAYOUT(language));
//     }
// }

#include "layout_define.h"


/***** 設置字庫 *****/
extern void lv_ft_font_set_type(int type);
/***** 重新初始化字庫 *****/
extern void lv_font_afresh_init(void);
static void language_english_btn_up(lv_obj_t *obj)
{
    static rom_bin_info img_unchecked = rom_bin_info_get(ROM_UI_MEDIA_LIST_CHECKBOX_PNG);
    static rom_bin_info img_checked  = rom_bin_info_get(ROM_UI_MEDIA_LIST_CHECKBOX_FOCUS_PNG);

    if(user_data_get()->setting.language == LANG_ENGLISH) return;
    english_checked_state = true;
    lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img_checked);
    lv_obj_invalidate(obj);

    lv_obj_t *persian_btn = lv_obj_get_child_form_id(lv_obj_get_parent(obj),2); // 通过ID获取波斯语按钮
    if(persian_btn != NULL)
    {
        persian_checked_state = false;
        lv_obj_set_style_local_pattern_image(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img_unchecked);
        lv_obj_invalidate(persian_btn);
    }

    user_data_get()->setting.language = LANG_ENGLISH;
    lv_ft_font_set_type(user_data_get()->setting.language);
    lv_font_afresh_init();
    user_data_save();

    goto_layout(pLAYOUT(language));
}


static void language_persian_btn_up(lv_obj_t *obj)
{
    static rom_bin_info img_unchecked = rom_bin_info_get(ROM_UI_MEDIA_LIST_CHECKBOX_PNG);
    static rom_bin_info img_checked  = rom_bin_info_get(ROM_UI_MEDIA_LIST_CHECKBOX_FOCUS_PNG);

    if(user_data_get()->setting.language == LANG_PERSIAN) return;
    persian_checked_state = true;
    lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img_checked);
    lv_obj_invalidate(obj);

     lv_obj_t *english_btn = lv_obj_get_child_form_id(lv_obj_get_parent(obj), 1); // 通过ID获取英语按钮
    if(english_btn != NULL)
    {
        english_checked_state = false;
        lv_obj_set_style_local_pattern_image(english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img_unchecked);
        lv_obj_invalidate(english_btn);
    }

        user_data_get()->setting.language = LANG_PERSIAN;
        lv_ft_font_set_type(user_data_get()->setting.language);
        lv_font_afresh_init();
        user_data_save();
        goto_layout(pLAYOUT(language));
}

static void init_language_btn_state(lv_obj_t *english_btn, lv_obj_t *persian_btn)
{
    static rom_bin_info img_unchecked = rom_bin_info_get(ROM_UI_MEDIA_LIST_CHECKBOX_PNG);
    static rom_bin_info img_checked  = rom_bin_info_get(ROM_UI_MEDIA_LIST_CHECKBOX_FOCUS_PNG);

    // 根据当前语言设置初始化按钮图片
    if(user_data_get()->setting.language == LANG_ENGLISH)
    {
        english_checked_state = true;
        persian_checked_state = false;
        lv_obj_set_style_local_pattern_image(english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img_checked);
        lv_obj_set_style_local_pattern_image(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img_unchecked);
    }
    else if(user_data_get()->setting.language == LANG_PERSIAN)
    {
        english_checked_state = false;
        persian_checked_state = true;
        lv_obj_set_style_local_pattern_image(english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img_unchecked);
        lv_obj_set_style_local_pattern_image(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img_checked);
    }

    // 刷新按钮显示
    lv_obj_invalidate(english_btn);
    lv_obj_invalidate(persian_btn);
}


//创建Language选择按钮
static void language_select_btn_create(lv_obj_t * center_cont)
{
	lv_obj_t *language_english_btn = lv_btn_create(center_cont, NULL);
	lv_obj_set_pos(language_english_btn, 470, 251);
	lv_obj_set_size(language_english_btn, 34, 34);
	lv_obj_set_id(language_english_btn, 1);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_CHECKBOX_PNG);
	lv_obj_set_style_local_pattern_image(language_english_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info);
	lv_obj_set_style_local_bg_opa(language_english_btn, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_ext_click_area(language_english_btn, 20, 100, 10, 10);
	static obj_click_data btn_data = obj_click_data_up_create(language_english_btn_up);

	lv_obj_t *language_english_label = lv_label_create(center_cont, NULL);
    lv_obj_set_pos(language_english_label, 568, 252);
	lv_obj_set_size(language_english_label, 88, 31);
    lv_label_set_text(language_english_label, "English");
    lv_obj_align(language_english_btn, language_english_label, LV_ALIGN_OUT_LEFT_MID, -30, 0);
	obj_click_event_listen(language_english_btn, &btn_data);


    lv_obj_t *persian_btn = lv_btn_create(center_cont, NULL);
	lv_obj_set_pos(persian_btn, 470, 319);
	lv_obj_set_size(persian_btn, 34, 34);
	lv_obj_set_id(persian_btn, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_MEDIA_LIST_CHECKBOX_PNG);
	lv_obj_set_style_local_pattern_image(persian_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &info1);
	lv_obj_set_style_local_bg_opa(persian_btn, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_ext_click_area(persian_btn, 20, 100, 10, 10);
	static obj_click_data btn_data1 = obj_click_data_up_create(language_persian_btn_up);

	lv_obj_t *language_persian_label = lv_label_create(center_cont, NULL);
    lv_obj_set_pos(language_persian_label, 568, 323);
	lv_obj_set_size(language_persian_label, 88, 31);
    lv_label_set_text(language_persian_label, "فارسی");
	lv_obj_align(persian_btn, language_persian_label, LV_ALIGN_OUT_LEFT_MID, -30, 0);
	obj_click_event_listen(persian_btn, &btn_data1);

    if(user_data_get()->setting.language == LANG_ENGLISH)
    {
        lv_obj_set_style_local_value_color(language_english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
        lv_obj_set_style_local_value_color(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
    }
    else if(user_data_get()->setting.language == LANG_PERSIAN)
    {
        lv_obj_set_style_local_value_color(language_english_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8A8A8A));
        lv_obj_set_style_local_value_color(persian_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x46CC00));
    }    

    init_language_btn_state(language_english_btn, persian_btn);
}

static void LAYOUT_ENTER_FUNC(language)
{
	lv_obj_t *parent = common_bg_display(lv_scr_act());
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
    language_select_btn_create(parent); 
}

static void LAYOUT_QUIT_FUNC(language)
{
    user_data_save();
}

CREATE_LAYOUT(language);
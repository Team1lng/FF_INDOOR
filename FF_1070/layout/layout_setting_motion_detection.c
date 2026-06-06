#include "layout_define.h"

#define SETTING_MOTION_CAMERA_SELECT_ID 0x05
#define SETTING_MOTION_STORAGE_MODE_ID 0x06
#define SETTING_MOTION_SENSITIVITY_ID 0x07
#define SETTING_MOTION_RECORD_DURATION_ID 0x08
#define SETTING_MOTION_BRIGHT_SCREEN_ID 0x09
#define LAYOUT_SETTING_MOTION_OBJ_CONT 0x10
#define HOME_BACK_OBJ_ID 0x11
#define HOME_GEAR_OBJ_ID 0x12

// 全局变量用于存储当前显示文本
static char camera_select_text[32] = {0};
static char storage_mode_text[32] = {0};
static char sensitivity_text[32] = {0};
static char record_duration_text[32] = {0};
static char bright_screen_text[32] = {0};

static void back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting));
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
    lv_label_set_text(Setting_label, str_get(COMMON_LANG_LEFT_HEAD_MEMORY_ID));
    lv_obj_align(setting_icon_obj, Setting_label, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}

// 切换运动检测总开关
static void setting_detectiong_open_detection_btn_up(lv_obj_t *obj)
{
    bool state = (lv_checkbox_get_state(obj) & LV_STATE_CHECKED) ? true : false;
    user_data_get()->motion.enable = state;
    user_data_save();
    lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_SETTING_MOTION_OBJ_CONT);
    if (cont == NULL)
    {
        printf("find LAYOUT_SETTING_MOTION_OBJ_CONT failed \n");
        return;
    }
    lv_obj_set_hidden(cont, !state);
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建Open detection设置按钮
** 返回参数说明：成功创建返回true
***/
static bool setting_detectiong_open_detection_btn_create(lv_obj_t *parent)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_detectiong_open_detection_btn_up);
    setting_sub_btn_base_create(parent, 172, 122 + (50 * 0), 680, 46,
                                str_get(LAYOUT_SET_DETECTIONG_OPEN_DETECTION_ID),
                                &click_data, user_data_get()->motion.enable, 3);
    return true;
}
/***
** 日期: 2022-05-05 16:23
** 作者: leo.liu
** 函数作用：移动侦测参数按钮设置的容器
** 返回参数说明：
***/
static lv_obj_t *setting_motion_cont_create(void)
{
    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    if (cont == NULL)
    {
        printf("motion detect setting cont create failed \n");
        return NULL;
    }
    lv_obj_set_id(cont, LAYOUT_SETTING_MOTION_OBJ_CONT);
    lv_obj_set_pos(cont, 175, 190);
    lv_obj_set_size(cont, 680, 60 * 5);

    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    lv_obj_set_hidden(cont, user_data_get()->motion.enable == true ? false : true);
    return cont;
}

// 更新摄像头选择显示文本
static void update_camera_select_display(void)
{
    lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_SETTING_MOTION_OBJ_CONT);
    if (cont == NULL)
    {
        printf("motion cont not found\n");
        return;
    }
    switch (user_data_get()->motion.select_camera)
    {
    case 0:
        sprintf(camera_select_text, "%s", str_get(LAYOUT_HOME_LANG_DOOR1_ID));
        break;
    case 1:
        sprintf(camera_select_text, "%s", str_get(LAYOUT_HOME_LANG_DOOR2_ID));
        break;
    case 2:
        sprintf(camera_select_text, "%s", str_get(LAYOUT_HOME_LANG_CCTV1_ID));
        break;
    case 3:
        sprintf(camera_select_text, "%s", str_get(LAYOUT_HOME_LANG_CCTV2_ID));
        break;
    default:
        sprintf(camera_select_text, "%s", str_get(LAYOUT_HOME_LANG_DOOR1_ID));
        break;
    }

    // 使用正确的方式更新UI显示
    lv_obj_t *obj = lv_obj_get_child_form_id(cont, SETTING_MOTION_CAMERA_SELECT_ID);
    if (obj == NULL)
    {
        printf("camera select obj not found\n");
        return;
    }
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, camera_select_text);
}

// 按钮点击回调函数 - 摄像头选择
static void setting_detectiong_camera_select_btn_up(lv_obj_t *obj)
{
    // 循环切换摄像头选择：0->1->2->3->0
    user_data_get()->motion.select_camera = (user_data_get()->motion.select_camera + 1) % 4;
    user_data_save();
    update_camera_select_display();
}

static void setting_detectiong_camera_select_left_btn_up(lv_obj_t *obj)
{
    // 向左切换摄像头选择
    user_data_get()->motion.select_camera = (user_data_get()->motion.select_camera - 1 + 4) % 4;
    user_data_save();
    update_camera_select_display();
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建camera_select设置按钮
** 返回参数说明：成功创建返回true
***/
static bool setting_detectiong_camera_select_btn_create(lv_obj_t *parent)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_detectiong_camera_select_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_detectiong_camera_select_left_btn_up);

    lv_obj_t *btn = setting_double_arrow_btn_create(parent, 0, 60 * 0, 680, 60,
                                                    str_get(LAYOUT_SET_DETECTIONG_CAMERA_SELECT_ID),
                                                    camera_select_text,
                                                    &click_data,
                                                    &left_click_data,
                                                    SETTING_MOTION_CAMERA_SELECT_ID);
    if (btn != NULL)
    {
        lv_obj_set_style_local_border_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
        lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
        lv_obj_set_style_local_border_side(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
        lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x5E5E5E));
    }
    // 初始化显示文本
    update_camera_select_display();
    return btn != NULL;
}

// 更新存储格式显示文本
static void update_storage_mode_display(void)
{
    lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_SETTING_MOTION_OBJ_CONT);
    if (cont == NULL)
    {
        printf("motion cont not found\n");
        return;
    }
    switch (user_data_get()->motion.saving_fmt)
    {
    case 0:
        sprintf(storage_mode_text, "%s", str_get(LAYOUT_MEMORY_LANG_IMAGE_ID));
        break;
    case 1:
        sprintf(storage_mode_text, "%s", str_get(LAYOUT_MEMORY_LANG_VIDEO_ID));
        break;
    default:
        sprintf(storage_mode_text, "%s", str_get(LAYOUT_MEMORY_LANG_IMAGE_ID));
        break;
    }

    // 使用正确的方式更新UI显示
    lv_obj_t *obj = lv_obj_get_child_form_id(cont, SETTING_MOTION_STORAGE_MODE_ID);
    if (obj == NULL)
    {
        printf("storage mode obj not found\n");
        return;
    }
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, storage_mode_text);
}

// 按钮点击回调函数 - 存储格式
static void setting_detectiong_storage_mode_btn_up(lv_obj_t *obj)
{
    // 循环切换存储格式：0->1->0
    user_data_get()->motion.saving_fmt = (user_data_get()->motion.saving_fmt + 1) % 2;
    user_data_save();
    update_storage_mode_display();
}

static void setting_detectiong_storage_mode_left_btn_up(lv_obj_t *obj)
{
    // 向左切换存储格式
    user_data_get()->motion.saving_fmt = (user_data_get()->motion.saving_fmt - 1 + 2) % 2;
    user_data_save();
    update_storage_mode_display();
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建storage mode设置按钮
** 返回参数说明：成功创建返回true
***/
static bool setting_detectiong_storage_mode_btn_create(lv_obj_t *parent)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_detectiong_storage_mode_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_detectiong_storage_mode_left_btn_up);

    lv_obj_t *btn = setting_double_arrow_btn_create(parent, 0, 60 * 1, 680, 60,
                                                    str_get(LAYOUT_SET_DETECTIONG_STORAGE_MODE_ID),
                                                    storage_mode_text,
                                                    &click_data,
                                                    &left_click_data,
                                                    SETTING_MOTION_STORAGE_MODE_ID);
    if (btn != NULL)
    {
        lv_obj_set_style_local_border_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
        lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
        lv_obj_set_style_local_border_side(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
        lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x5E5E5E));
    }
    update_storage_mode_display();
    return btn != NULL;
}

// 更新灵敏度显示文本
static void update_sensitivity_display(void)
{
    lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_SETTING_MOTION_OBJ_CONT);
    if (cont == NULL)
    {
        printf("motion cont not found\n");
        return;
    }

    switch (user_data_get()->motion.sensivity)
    {
    case 0:
        sprintf(sensitivity_text, "%s", str_get(LAYOUT_MOTION_DETECTION_LOW_ID));
        break;
    case 1:
        sprintf(sensitivity_text, "%s", str_get(LAYOUT_MOTION_DETECTION_MIDDLE_ID));
        break;
    case 2:
        sprintf(sensitivity_text, "%s", str_get(LAYOUT_MOTION_DETECTION_HIGH_ID));
        break;
    default:
        sprintf(sensitivity_text, "%s", str_get(LAYOUT_MOTION_DETECTION_MIDDLE_ID));
        break;
    }

    // 使用正确的方式更新UI显示
    lv_obj_t *obj = lv_obj_get_child_form_id(cont, SETTING_MOTION_SENSITIVITY_ID);
    if (obj == NULL)
    {
        printf("sensitivity obj not found\n");
        return;
    }
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, sensitivity_text);
}

// 按钮点击回调函数 - 灵敏度设置
static void setting_detectiong_sensitivity_setting_btn_up(lv_obj_t *obj)
{
    // 循环切换灵敏度：0->1->2->0
    user_data_get()->motion.sensivity = (user_data_get()->motion.sensivity + 1) % 3;
    user_data_save();
    update_sensitivity_display();
}

static void setting_detectiong_sensitivity_setting_left_btn_up(lv_obj_t *obj)
{
    // 向左切换灵敏度
    user_data_get()->motion.sensivity = (user_data_get()->motion.sensivity - 1 + 3) % 3;
    user_data_save();
    update_sensitivity_display();
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建sensitivity setting设置按钮
** 返回参数说明：成功创建返回true
***/
static bool setting_detectiong_sensitivity_setting_btn_create(lv_obj_t *parent)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_detectiong_sensitivity_setting_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_detectiong_sensitivity_setting_left_btn_up);

    lv_obj_t *btn = setting_double_arrow_btn_create(parent, 0, (60 * 2), 680, 60,
                                                    str_get(LAYOUT_SET_DETECTIONG_SENSITIVITY_ID),
                                                    sensitivity_text,
                                                    &click_data,
                                                    &left_click_data,
                                                    SETTING_MOTION_SENSITIVITY_ID);
    if (btn != NULL)
    {
        lv_obj_set_style_local_border_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
        lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
        lv_obj_set_style_local_border_side(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
        lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x5E5E5E));
    }
    update_sensitivity_display();
    return btn != NULL;
}

// 更新录像时长显示文本
static void update_record_duration_display(void)
{
    lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_SETTING_MOTION_OBJ_CONT);
    if (cont == NULL)
    {
        printf("motion cont not found\n");
        return;
    }
    switch (user_data_get()->motion.record_duration)
    {
    case 0:
        sprintf(record_duration_text, "10s");
        break;
    case 1:
        sprintf(record_duration_text, "20s");
        break;
    case 2:
        sprintf(record_duration_text, "30s");
        break;
    default:
        sprintf(record_duration_text, "20s");
        break;
    }

    // 使用正确的方式更新UI显示
    lv_obj_t *obj = lv_obj_get_child_form_id(cont, SETTING_MOTION_RECORD_DURATION_ID);
    if (obj == NULL)
    {
        printf("record duration obj not found\n");
        return;
    }
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, record_duration_text);
}

// 按钮点击回调函数 - 录像时长
static void setting_detectiong_recording_duration_btn_up(lv_obj_t *obj)
{
    // 循环切换录像时长
    user_data_get()->motion.record_duration = (user_data_get()->motion.record_duration + 1) % 3;
    user_data_save();
    update_record_duration_display();
}

static void setting_detectiong_recording_duration_left_btn_up(lv_obj_t *obj)
{
    // 向左切换录像时长
    user_data_get()->motion.record_duration = (user_data_get()->motion.record_duration - 1 + 3) % 3;
    user_data_save();
    update_record_duration_display();
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建Recording duration设置按钮
** 返回参数说明：成功创建返回true
***/
static bool setting_detectiong_recording_duration_btn_create(lv_obj_t *parent)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_detectiong_recording_duration_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_detectiong_recording_duration_left_btn_up);

    lv_obj_t *btn = setting_double_arrow_btn_create(parent, 0, (60 * 3), 680, 60,
                                                    str_get(LAYOUT_SET_DETECTIONG_RECORD_DURATION_ID),
                                                    record_duration_text,
                                                    &click_data,
                                                    &left_click_data,
                                                    SETTING_MOTION_RECORD_DURATION_ID);
    if (btn != NULL)
    {
        lv_obj_set_style_local_border_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
        lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
        lv_obj_set_style_local_border_side(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
        lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x5E5E5E));
    }
    update_record_duration_display();
    return btn != NULL;
}

// 更新亮屏显示文本
static void update_bright_screen_display(void)
{
    lv_obj_t *cont = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_SETTING_MOTION_OBJ_CONT);
    if (cont == NULL)
    {
        printf("motion cont not found\n");
        return;
    }
    if (user_data_get()->motion.lcd_en)
    {
        sprintf(bright_screen_text, "%s", str_get(LAYOUT_SETTING_LANG_ON_ID));
    }
    else
    {
        sprintf(bright_screen_text, "%s", str_get(LAYOUT_SETTING_LANG_OFF_ID));
    }

    // 使用正确的方式更新UI显示
    lv_obj_t *obj = lv_obj_get_child_form_id(cont, SETTING_MOTION_BRIGHT_SCREEN_ID);
    if (obj == NULL)
    {
        printf("bright screen obj not found\n");
        return;
    }
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, bright_screen_text);
}

// 按钮点击回调函数 - 亮屏设置
static void setting_detectiong_bright_screen_btn_up(lv_obj_t *obj)
{
    // 切换亮屏使能
    user_data_get()->motion.lcd_en = !user_data_get()->motion.lcd_en;
    user_data_save();
    update_bright_screen_display();
}

static void setting_detectiong_bright_screen_left_btn_up(lv_obj_t *obj)
{
    // 向左切换亮屏使能
    user_data_get()->motion.lcd_en = !user_data_get()->motion.lcd_en;
    user_data_save();
    update_bright_screen_display();
}

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建Bright screen 设置按钮
** 返回参数说明：成功创建返回true
***/
static bool setting_detectiong_bright_screen_btn_create(lv_obj_t *parent)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_detectiong_bright_screen_btn_up);
    static obj_click_data left_click_data = obj_click_data_up_create(setting_detectiong_bright_screen_left_btn_up);

    lv_obj_t *btn = setting_double_arrow_btn_create(parent, 0, (60 * 4), 680, 60,
                                                    str_get(LAYOUT_SET_DETECTIONG_BRIGHT_SCREEN_ID),
                                                    bright_screen_text,
                                                    &click_data,
                                                    &left_click_data,
                                                    SETTING_MOTION_BRIGHT_SCREEN_ID);
    // 初始化显示文本
    if (btn != NULL)
    {
        lv_obj_set_style_local_border_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
        lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
        lv_obj_set_style_local_border_side(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
        lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x5E5E5E));
    }
    update_bright_screen_display();
    return btn != NULL;
}

// // 按钮点击回调函数
// static void setting_detectiong_time_period_control_btn_up(lv_obj_t *obj)
// {
//     goto_layout(pLAYOUT(setting_time_period_control));
// }

/***
** 日期: 2022-04-29 08:19
** 作者: leo.liu
** 函数作用：创建Time period control设置按钮
** 返回参数说明：成功创建返回true
***/
// static bool setting_detectiong_time_period_control_btn_create(lv_obj_t *parent)
// {
//     static obj_click_data click_data = obj_click_data_up_create(setting_detectiong_time_period_control_btn_up);
//     setting_right_btn_base_create(parent, 0, (50 * 5), 680, 40,
//                                   str_get(LAYOUT_SET_DETECTIONG_TIME_PETIOD_COUNT_ID),
//                                   user_data_get()->motion.timer_en ? str_get(LAYOUT_SETTING_LANG_ON_ID) : str_get(LAYOUT_SETTING_LANG_OFF_ID),
//                                   &click_data,
//                                   0);
//     return true;
// }

static void LAYOUT_ENTER_FUNC(setting_motion_detection)
{
    lv_obj_t *parent = common_bg_display(lv_scr_act());
    back_btn_create(parent);
    setting_icon_create(parent);
    top_time_date_text_create(parent);

    setting_detectiong_open_detection_btn_create(parent);
    lv_obj_t *cont = setting_motion_cont_create();

    setting_detectiong_camera_select_btn_create(cont);
    setting_detectiong_storage_mode_btn_create(cont);
    setting_detectiong_sensitivity_setting_btn_create(cont);
    setting_detectiong_recording_duration_btn_create(cont);
    setting_detectiong_bright_screen_btn_create(cont);
    // setting_detectiong_time_period_control_btn_create(cont);
}

static void LAYOUT_QUIT_FUNC(setting_motion_detection)
{
    // 保存用户数据
    user_data_save();
}

CREATE_LAYOUT(setting_motion_detection);
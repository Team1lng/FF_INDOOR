#include "layout_define.h"

typedef enum
{
    SETTING_TIME_PRTIOD_HOME_BTN_ID,
    SETTING_TIME_PRTIOD_SYSTEM_BTN_ID,
    SETTING_TIME_PRTIOD_TIME_BTN_ID,
    SETTING_TIME_PRTIOD_SOUND_BTN_ID,
    SETTING_TIME_PRTIOD_TOTAL_BTN,
} setting_btn_module;

static custom_area setting_btn_area[SETTING_TIME_PRTIOD_TOTAL_BTN] =
    {
        {24, 12, 54, 54},
        {30, 70, 155, 170},
        {30, 240, 155, 170},
        {28, 410, 157, 170},
};

#define LAYOUT_SETTING_TIME_PERIOD_OBJ_CONT 0x01
#define LAYOUT_SETTING_TIME_PERIOD_OBJ_START_HOUR 0X02
#define LAYOUT_SETTING_TIME_PERIOD_OBJ_START_MIN 0X03
#define LAYOUT_SETTING_TIME_PERIOD_OBJ_START_SEC 0X04
#define LAYOUT_SETTING_TIME_PERIOD_OBJ_END_HOUR 0X05
#define LAYOUT_SETTING_TIME_PERIOD_OBJ_END_MIN 0X06
#define LAYOUT_SETTING_TIME_PERIOD_OBJ_END_SEC 0X07

// 全局变量保存容器指针
static lv_obj_t *g_time_period_cont = NULL;

static void setting_time_prtiod_home_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting_motion_detection));
}

static void setting_time_prtiod_home_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_prtiod_home_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_HOME_PNG);
    common_img_btn_create(parent, setting_btn_area[SETTING_TIME_PRTIOD_HOME_BTN_ID], NULL, &btn_data, &info);
}

static void setting_time_prtiod_system_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting));
}

static void setting_time_prtiod_system_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_prtiod_system_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_SYSTEM_RED_PNG);
    lv_obj_t *btn = photo_and_video_btn_create(parent, setting_btn_area[SETTING_TIME_PRTIOD_SYSTEM_BTN_ID], str_get(LAYOUT_SETTING_LANG_SYSTEM_ID), &btn_data, &info, true, true);
    lv_obj_set_id(btn, SETTING_TIME_PRTIOD_SYSTEM_BTN_ID);
}

static void setting_time_prtiod_time_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting_time));
}

static void setting_time_prtiod_time_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_prtiod_time_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_TIME_PNG);
    lv_obj_t *btn = photo_and_video_btn_create(parent, setting_btn_area[SETTING_TIME_PRTIOD_TIME_BTN_ID], str_get(LAYOUT_SETTING_LANG_TIME_ID), &btn_data, &info, true, false);
    lv_obj_set_id(btn, SETTING_TIME_PRTIOD_TIME_BTN_ID);
}

static void setting_time_prtiod_sound_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting_sound));
}

static void setting_time_prtiod_sound_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_prtiod_sound_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_SOUND_PNG);
    lv_obj_t *btn = photo_and_video_btn_create(parent, setting_btn_area[SETTING_TIME_PRTIOD_SOUND_BTN_ID], str_get(LAYOUT_SETTING_LANG_SOUND_ID), &btn_data, &info, false, false);
    lv_obj_set_id(btn, SETTING_TIME_PRTIOD_SOUND_BTN_ID);
}

// 核心功能：根据timer_en状态设置按钮和滚筒容器
static void setting_time_period_enable_btn_up(lv_obj_t *obj)
{
    bool state = (lv_checkbox_get_state(obj) & LV_STATE_CHECKED) ? true : false;

    user_data_get()->motion.timer_en = state;
    user_data_save();

    if (g_time_period_cont == NULL)
    {
        printf("g_time_period_cont is NULL\n");
        return;
    }
    lv_obj_set_hidden(g_time_period_cont, !state);
    printf("设置容器隐藏状态: %s\n", !state ? "隐藏" : "显示");
}

/***
** 函数作用：创建时间段开关按钮
***/
static void setting_time_period_enable_btn_create(void)
{
    static obj_click_data click_data = obj_click_data_up_create(setting_time_period_enable_btn_up);
    setting_sub_btn_base_create(NULL, 241, 84 + (50 * 0), 680, 40,
                                str_get(LAYOUT_SET_DETECTIONG_OPEN_DETECTION_ID),
                                &click_data,
                                user_data_get()->motion.timer_en, 3);
}

/***
** 函数作用：创建时间段设置容器（专门用于放置滚筒和图标）
***/
static lv_obj_t *setting_time_period_cont_create(void)
{
    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    if (cont == NULL)
    {
        printf("time period setting cont create failed \n");
        return NULL;
    }
    lv_obj_set_id(cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_CONT);
    lv_obj_set_pos(cont, 241, 84 + (50 * 1));
    lv_obj_set_size(cont, 680, 450);

    // 保存全局指针
    g_time_period_cont = cont;

    /***** 判定滚筒是否修改 *****/
    static bool modidy = false;
    modidy = false;
    cont->user_data = &modidy;

    lv_obj_set_hidden(cont, !user_data_get()->motion.timer_en);
    printf("容器初始隐藏状态: %s\n", !user_data_get()->motion.timer_en ? "隐藏" : "显示");
    return cont;
}

/***
** 函数作用：创建时间滚筒（按照相对位置设置）
***/
static bool setting_time_period_hour_rooler_create(lv_obj_t *parent)
{
    // 开始时间滚筒 - 相对位置
    setting_time_roller_base(parent, 289, 40, 66, 122, 0, 23, user_data_get()->motion.start.tm_hour, LAYOUT_SETTING_TIME_PERIOD_OBJ_START_HOUR);
    setting_time_roller_base(parent, 377, 40, 66, 122, 0, 59, user_data_get()->motion.start.tm_min, LAYOUT_SETTING_TIME_PERIOD_OBJ_START_MIN);
    setting_time_roller_base(parent, 465, 40, 66, 122, 0, 59, user_data_get()->motion.start.tm_sec, LAYOUT_SETTING_TIME_PERIOD_OBJ_START_SEC);

    // 结束时间滚筒 - 相对位置
    setting_time_roller_base(parent, 289, 253, 66, 122, 0, 23, user_data_get()->motion.end.tm_hour, LAYOUT_SETTING_TIME_PERIOD_OBJ_END_HOUR);
    setting_time_roller_base(parent, 377, 253, 66, 122, 0, 59, user_data_get()->motion.end.tm_min, LAYOUT_SETTING_TIME_PERIOD_OBJ_END_MIN);
    setting_time_roller_base(parent, 465, 253, 66, 122, 0, 59, user_data_get()->motion.end.tm_sec, LAYOUT_SETTING_TIME_PERIOD_OBJ_END_SEC);

    return true;
}

/*************************************************************************
 * @brief  创建图标和分隔符（按照相对位置设置）
 **************************************************************************/
static void setting_time_period_icon_display(lv_obj_t *parent)
{
    /* 开始的图标 - 相对位置 */
    lv_obj_t *obj = lv_obj_create(parent, NULL);
    lv_obj_set_pos(obj, 161, 71);
    lv_obj_set_size(obj, 62, 62);
    static rom_bin_info img_start = rom_bin_info_get(ROM_UI_SETTING_TIME_START_PNG);
    lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img_start);

    /* 结束的图标 - 相对位置 */
    obj = lv_obj_create(parent, obj);
    lv_obj_set_pos(obj, 163, 285);
    static rom_bin_info img_end = rom_bin_info_get(ROM_UI_SETTING_TIME_END_PNG);
    lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &img_end);

    /* 时间中间的冒号 - 开始时间 - 相对位置 */
    obj = lv_obj_create(parent, NULL);
    static char string_2[] = {":"};
    lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, string_2);
    lv_obj_set_style_local_value_font(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
    lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4A90E2)); // 蓝色

    // 开始时间的第一个冒号（时:分）- 相对位置
    lv_obj_set_pos(obj, 348, 86);
    lv_obj_set_size(obj, 22, 27);

    // 开始时间的第二个冒号（分:秒）- 相对位置
    obj = lv_obj_create(parent, obj);
    lv_obj_set_pos(obj, 435, 86);
    lv_obj_set_size(obj, 22, 27);

    /* 时间中间的冒号 - 结束时间 - 相对位置 */
    obj = lv_obj_create(parent, obj);
    // 结束时间的第一个冒号（时:分）- 相对位置
    lv_obj_set_pos(obj, 348, 302);
    lv_obj_set_size(obj, 22, 27);

    // 结束时间的第二个冒号（分:秒）- 相对位置
    obj = lv_obj_create(parent, obj);
    lv_obj_set_pos(obj, 435, 302);
    lv_obj_set_size(obj, 22, 27);

    /* 创建分割线 - 相对位置 */
    obj = lv_obj_create(parent, NULL);
    lv_obj_set_pos(obj, 22, 208);
    lv_obj_set_size(obj, 658, 1);
    lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x444466));
    lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_border_width(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
}

static void LAYOUT_ENTER_FUNC(setting_time_period_control)
{
    printf("============================enter_setting_time_prtiod\n");
    lv_obj_t *parent = common_bg_display(lv_scr_act());

    // 创建左侧导航按钮
    setting_time_prtiod_home_btn_create(parent);
    setting_time_prtiod_system_btn_create(parent);
    setting_time_prtiod_time_btn_create(parent);
    setting_time_prtiod_sound_btn_create(parent);

    // 创建时间段开关按钮
    setting_time_period_enable_btn_create();

    // 创建时间段设置容器（包含滚筒和图标）
    lv_obj_t *cont = setting_time_period_cont_create();

    // 创建时间滚筒和图标
    setting_time_period_hour_rooler_create(cont);
    setting_time_period_icon_display(cont);
}

static void LAYOUT_QUIT_FUNC(setting_time_period_control)
{

    if (g_time_period_cont == NULL)
    {
        printf("退出时容器为空，尝试查找...\n");
        g_time_period_cont = lv_obj_get_child_form_id(lv_scr_act(), LAYOUT_SETTING_TIME_PERIOD_OBJ_CONT);
    }
    bool modify = *((bool *)g_time_period_cont->user_data);
    printf("退出时修改标志: %s\n", modify ? "已修改" : "未修改");
    if (modify == true)
    {

        if (g_time_period_cont != NULL)
        {
            char buffer[8] = {0};
            lv_obj_t *obj = NULL;

            /***** 保存开始时间 *****/
            obj = lv_obj_get_child_form_id(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_START_HOUR);
            if (obj != NULL)
            {
                lv_roller_get_selected_str(obj, buffer, 8);
                int hour;
                sscanf(buffer, "%d", &hour);
                user_data_get()->motion.start.tm_hour = hour;
                printf("保存开始小时: %d\n", hour);
            }

            obj = lv_obj_get_child_form_id(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_START_MIN);
            if (obj != NULL)
            {
                lv_roller_get_selected_str(obj, buffer, 8);
                int min;
                sscanf(buffer, "%d", &min);
                user_data_get()->motion.start.tm_min = min;
                printf("保存开始分钟: %d\n", min);
            }

            obj = lv_obj_get_child_form_id(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_START_SEC);
            if (obj != NULL)
            {
                lv_roller_get_selected_str(obj, buffer, 8);
                int sec;
                sscanf(buffer, "%d", &sec);
                user_data_get()->motion.start.tm_sec = sec;
                printf("保存开始秒钟: %d\n", sec);
            }

            /***** 保存结束时间 *****/
            obj = lv_obj_get_child_form_id(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_END_HOUR);
            if (obj != NULL)
            {
                lv_roller_get_selected_str(obj, buffer, 8);
                int hour;
                sscanf(buffer, "%d", &hour);
                user_data_get()->motion.end.tm_hour = hour;
                printf("保存结束小时: %d\n", hour);
            }

            obj = lv_obj_get_child_form_id(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_END_MIN);
            if (obj != NULL)
            {
                lv_roller_get_selected_str(obj, buffer, 8);
                int min;
                sscanf(buffer, "%d", &min);
                user_data_get()->motion.end.tm_min = min;
                printf("保存结束分钟: %d\n", min);
            }

            obj = lv_obj_get_child_form_id(g_time_period_cont, LAYOUT_SETTING_TIME_PERIOD_OBJ_END_SEC);
            if (obj != NULL)
            {
                lv_roller_get_selected_str(obj, buffer, 8);
                int sec;
                sscanf(buffer, "%d", &sec);
                user_data_get()->motion.end.tm_sec = sec;
                printf("保存结束秒钟: %d\n", sec);
            }

            user_data_save();
            printf("用户数据已保存\n");
        }
        else
        {
            printf("退出时未找到容器\n");
        }
    }

    // 重置全局变量
    g_time_period_cont = NULL;
}

CREATE_LAYOUT(setting_time_period_control);
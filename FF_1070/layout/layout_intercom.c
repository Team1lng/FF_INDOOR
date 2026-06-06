/**
 * @file    layout_intercom.c
 * @brief   内线对讲主页
 *
 *  功能：
 *    1. 数字键盘输入被叫房号（1~99）或呼叫保安（0/0）
 *    2. 密码验证后修改本机房号
 *    3. 呼叫按钮：若已摘机直接发起握手，否则提示摘机等待
 */

#include "layout_define.h"
#include "intercom.h"
#include "user_intercom.h"

#define INTERCOM_LOCAL_ADDR_ID     1
#define CALL_OUT_INIT_TIMEOUT      100 
#define ROOM_LEN                   2

typedef enum
{
    INTERCOM_HOME_BTN_ID,
    INTERCOM_GUARD_BTN_ID,
    INTERCOM_INTER_BTN_ID,
    HOME_BACK_OBJ_ID,
    HOME_GEAR_OBJ_ID,
    HOME_IMG_OBJ_ID,
    INTERCOM_TOTAL_BTN,
} intercom_btn_module;

static unsigned int kb_click_num = 0;
static bool kb_input_room_flag = false;
static lv_task_t *wait_hook_on_task = NULL;
static lv_obj_t *ta = NULL;  /* 被叫号码输入框 */
static lv_obj_t *tt = NULL;  /* 房号设置输入框 */
static bool textarea_input_select = false;
static char call_num_save[20] = {0};
static unsigned char call_room[2] = {0};

static void intercom_wait_hook_on_task_create(void);
static void intercom_call_out_result_task_create(const char *str);
static void intercom_room_out_result_task_create(const char *str);

/* ---------------------------------------------------------------
 * 通用输入框创建
 * --------------------------------------------------------------- */
static void intercom_textarea_create_common(lv_obj_t *parent,
                                             lv_obj_t **textarea_obj,
                                             int x, int y,
                                             int cont_w, int cont_h,
                                             int ta_w, int ta_h,
                                             int delete_btn_x,
                                             int max_length,
                                             obj_click_data *btn_data,
                                             obj_click_data *delete_click_data)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_pos(cont, x, y);
    lv_obj_set_size(cont, cont_w, cont_h);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 4);
    lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00CCFF));
    lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_border_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

    *textarea_obj = lv_textarea_create(cont, NULL);
    lv_obj_set_pos(*textarea_obj, 0, 0);
    lv_obj_set_size(*textarea_obj, ta_w, ta_h);
    lv_textarea_set_text(*textarea_obj, "");
    lv_textarea_set_cursor_hidden(*textarea_obj, true);
    lv_textarea_set_one_line(*textarea_obj, true);
    lv_textarea_set_text_align(*textarea_obj, LV_LABEL_ALIGN_CENTER);
    lv_textarea_set_scrollbar_mode(*textarea_obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_local_border_width(*textarea_obj, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_text_font(*textarea_obj, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, FONT_SIZE(30));
    lv_obj_set_style_local_text_color(*textarea_obj, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_color(*textarea_obj, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_value_font(*textarea_obj, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_set_style_local_pad_top(*textarea_obj, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 7);
    lv_obj_set_style_local_pad_bottom(*textarea_obj, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, 7);
    lv_textarea_set_max_length(*textarea_obj, max_length);
    obj_click_event_listen(*textarea_obj, btn_data);

    lv_obj_t *del_btn = lv_obj_create(cont, NULL);
    lv_obj_set_pos(del_btn, delete_btn_x, 8);
    lv_obj_set_size(del_btn, 48, 48);
    static rom_bin_info del_info = rom_bin_info_get(ROM_UI_INTERCOM_DELETE_PNG);
    lv_obj_set_style_local_pattern_image(del_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &del_info);
    lv_obj_set_style_local_pattern_recolor(del_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    lv_obj_set_style_local_pattern_recolor(del_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
    lv_obj_set_style_local_pattern_recolor_opa(del_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_pattern_recolor_opa(del_btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
    obj_click_event_listen(del_btn, delete_click_data);
}

/* ---------------------------------------------------------------
 * 返回按钮
 * --------------------------------------------------------------- */
static void back_btn_up(lv_obj_t *obj)
{
    if (user_data_get()->room_no_flag == false)
        goto_layout(pLAYOUT(home));
    else
        goto_layout(pLAYOUT(intercom));
}

static void back_btn_create(lv_obj_t *parent)
{
    lv_obj_t *back_icon = lv_img_create(parent, NULL);
    lv_obj_set_pos(back_icon, 920, 25);
    lv_obj_set_size(back_icon, 50, 37);
    lv_obj_set_style_local_pattern_recolor(back_icon, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
    lv_obj_set_style_local_pattern_recolor_opa(back_icon, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
    lv_obj_set_id(back_icon, HOME_BACK_OBJ_ID);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_TIME_BACK_PNG);
    lv_obj_set_style_local_pattern_image(back_icon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
    static obj_click_data btn_data = obj_click_data_up_create(back_btn_up);
    obj_click_event_listen(back_icon, &btn_data);
}

/* ---------------------------------------------------------------
 * 设置图标 + 本机房号显示
 * --------------------------------------------------------------- */
static void setting_icon_create(lv_obj_t *parent)
{
    lv_obj_t *icon = lv_img_create(parent, NULL);
    lv_obj_set_pos(icon, 54, 34);
    lv_obj_set_size(icon, 22, 20);
    static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_GEAR_PNG);
    lv_obj_set_id(icon, HOME_GEAR_OBJ_ID);
    lv_obj_set_style_local_pattern_image(icon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);

    lv_obj_t *label = lv_label_create(parent, NULL);
    lv_obj_set_pos(label, 87, 28);
    lv_obj_set_size(label, 70, 31);
    lv_obj_set_id(label, INTERCOM_LOCAL_ADDR_ID);
    static char str[40] = {0};
    if (user_data_get()->device_id[0] == 0 && user_data_get()->device_id[1] == 0)
        sprintf(str, "%s : %s", str_get(LAYOUT_INTERCOM_LANG_ROOM_NO_ID), str_get(LAYOUT_INTERCOM_LANG_GUARD_1_ID));
    else
        sprintf(str, "%s : %hhu%hhu", str_get(LAYOUT_INTERCOM_LANG_ROOM_NO_ID),
        user_data_get()->device_id[0], user_data_get()->device_id[1]);

    lv_label_set_text(label, str);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(22));
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_align(icon, label, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}

static void intercom_select_btn_create(lv_obj_t *parent)
{
    lv_obj_t *lbl = lv_label_create(parent, NULL);
    lv_obj_set_pos(lbl, 607, 179);
    lv_obj_set_size(lbl, 147, 29);
    lv_label_set_text_fmt(lbl, "%s :", str_get(LAYOUT_INTERCOM_LANG_CALL_NUMBER_ID));
    lv_obj_set_style_local_text_font(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_set_style_local_text_color(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
}

static void set_no_select_btn_create(lv_obj_t *parent)
{
    lv_obj_t *lbl = lv_label_create(parent, NULL);
    lv_obj_set_pos(lbl, 607, 324);
    lv_obj_set_size(lbl, 101, 29);
    lv_label_set_text_fmt(lbl, "%s :", str_get(COMMON_LANG_SET_NO));
    lv_obj_set_style_local_text_font(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_set_style_local_text_color(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
}

/* ---------------------------------------------------------------
 * 输入框事件
 * --------------------------------------------------------------- */
static void intercom_ta_up(lv_obj_t *obj)
{
    textarea_input_select = false;
    lv_textarea_set_text(ta, "");
    lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
}

static void intercom_tt_up(lv_obj_t *obj)
{
    textarea_input_select = true;
    lv_textarea_set_text(tt, "");
    lv_obj_set_style_local_value_opa(tt, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
    intercom_room_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_INPUT_PASSWORD_ID));
}

static void delete_button_click(lv_obj_t *obj)       
{ 
    lv_textarea_del_char(ta); 
}
static void room_delete_button_click(lv_obj_t *obj)  
{ 
    lv_textarea_del_char(tt); 
}

static void intercom_number_textarea_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(intercom_ta_up);
    static obj_click_data del_click_data = obj_click_data_up_create(delete_button_click);
    intercom_textarea_create_common(parent, &ta, 607, 226, 280, 64, 280, 64, 218, 3, &btn_data, &del_click_data);
}

static void intercom_room_textarea_create(lv_obj_t *parent)
{
    static obj_click_data btn_data2 = obj_click_data_up_create(intercom_tt_up);
    static obj_click_data del_click_data = obj_click_data_up_create(room_delete_button_click);
    intercom_textarea_create_common(parent, &tt, 607, 371, 300, 64, 298, 62, 236, 5, &btn_data2, &del_click_data);
}

/* ---------------------------------------------------------------
 * 键盘
 * --------------------------------------------------------------- */
static lv_obj_t *create_keyboard_button(lv_obj_t *parent, int x, int y,
                                         const char *text,
                                         obj_click_data *click_data,
                                         bool is_icon_button)
{
    lv_obj_t *btn = lv_btn_create(parent, NULL);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, 93, 93);
    lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    static rom_bin_info circle_info = rom_bin_info_get(ROM_UI_INTERCOM_CIRCLE_PNG);
    lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &circle_info);
    lv_obj_set_style_local_pattern_recolor(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    lv_obj_set_style_local_pattern_recolor(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
    lv_obj_set_style_local_pattern_recolor_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_pattern_recolor_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);

    if (!is_icon_button && text != NULL) {
        lv_obj_t *lbl = lv_label_create(btn, NULL);
        lv_label_set_text(lbl, text);
        lv_obj_set_style_local_text_font(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
        lv_obj_set_style_local_text_color(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
        lv_obj_set_style_local_text_color(lbl, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xCCCCCC));
        lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 0);
    }
    if (click_data != NULL) {
        obj_click_event_listen(btn, click_data);
    }
    return btn;
}

static void keyboard_button_click(lv_obj_t *obj)
{
    /* 摘机等待期间禁止继续输入 */
    if (wait_hook_on_task != NULL) {
        return;
    }

    lv_obj_t *lbl = lv_obj_get_child(obj, NULL);
    if (lbl == NULL) {
        return;
    }
    const char *txt = lv_label_get_text(lbl);
    if (txt == NULL) {
        return;
    }

    if (textarea_input_select) {
        lv_textarea_add_char(tt, txt[0]);
        lv_obj_set_style_local_value_opa(tt, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
    } else {
        lv_textarea_add_char(ta, txt[0]);
        lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
    }
}

/* ---------------------------------------------------------------
 * 房号设置 OK 处理
 * --------------------------------------------------------------- */
static void intercom_ok_button_click(lv_obj_t *obj)
{
    static char str[20] = {0};
    static unsigned char input_room[2] = {0};
    user_data_info *ud = user_data_get();
    if (ud->password == 0) ud->password = 1234;

    kb_click_num = atoi(lv_textarea_get_text(tt));

    if (!kb_input_room_flag) {
        /* 密码验证 */
        if (kb_click_num == ud->password) {
            lv_textarea_set_text(tt, "");
            intercom_room_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_INPUT_ROOM_NO_ID));
            kb_input_room_flag = true;
            lv_textarea_set_max_length(tt, 5);
        } else {
            lv_textarea_set_text(tt, "");
            intercom_room_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_ERROR_ID));
        }
    } else {
        /* 房号设置 */
        if (kb_click_num < 1 || kb_click_num > 99) {
            intercom_room_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_ERROR_ID));
            lv_textarea_set_text(tt, "");
            kb_input_room_flag = false;
            return;
        }
        input_room[0] = (unsigned char)(kb_click_num / 10);
        input_room[1] = (unsigned char)(kb_click_num % 10);

        if (input_room[0] == ud->device_id[0] && input_room[1] == ud->device_id[1]) {
            intercom_room_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_LOCAL_ID));
        } else {
            ud->device_id[0] = input_room[0];
            ud->device_id[1] = input_room[1];
            /* 同步更新底层本机 ID */
            MsgUpdateNativeId(input_room[0] * 10 + input_room[1]);
            intercom_room_out_result_task_create(str_get(LAYOUT_ROOM_ID_LANG_MODIFY_SUCCESSFUL_ID));
            lv_obj_t *addr_obj = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_LOCAL_ADDR_ID);
            if (addr_obj) {
                sprintf(str, "%s : %hhu%hhu", str_get(LAYOUT_INTERCOM_LANG_ROOM_NO_ID),
                        ud->device_id[0], ud->device_id[1]);
                lv_label_set_text(addr_obj, str);
            }
        }
        lv_textarea_set_text(tt, "");
        kb_input_room_flag = false;
    }
}

static void do_call_out(void)
{
    user_data_info *ud = user_data_get();

    /* 标记为主叫 */
    intercom_call_in_flag = false;

    /* 更新 other_id 供 intercom_out 界面读取显示 */
    ud->other_id[0] = call_room[0];
    ud->other_id[1] = call_room[1];

    /* 组合为单字节房间号传给底层 */
    unsigned char dest_id = call_room[0] * 10 + call_room[1];

    printf("[intercom] do_call_out: dest=%u\n", dest_id);

    /* 初始化底层并发起握手 */
    MsgCallRequest(dest_id);

    /* 跳转呼出等待界面 */
    goto_layout(pLAYOUT(intercom_out));
}

static void intercom_button_click(lv_obj_t *obj)
{
    if (wait_hook_on_task != NULL) {
        return;
    }

    if (textarea_input_select) {
        intercom_ok_button_click(obj);
        return;
    }

    unsigned int call_num = atoi(lv_textarea_get_text(ta));
    memset(call_num_save, 0, sizeof(call_num_save));
    sprintf(call_num_save, "%s", lv_textarea_get_text(ta));

    if (call_num < 1 || call_num > 99) {
        intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_ERROR_ID));
        lv_textarea_set_text(ta, "");
        return;
    }

    call_room[0] = (unsigned char)(call_num / 10);
    call_room[1] = (unsigned char)(call_num % 10);

    user_data_info *ud = user_data_get();
    /* 不能呼叫自己 */
    if (call_room[0] == ud->device_id[0] && call_room[1] == ud->device_id[1]) {
        intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_LOCAL_ID));
        lv_textarea_set_text(ta, "");
        return;
    }

    lv_textarea_set_text(ta, "");

    if (hook_state_get() == true) {
        /* 已摘机，直接呼出 */
        do_call_out();
    } else {
        /* 未摘机，提示摘机并等待 */
        intercom_wait_hook_on_task_create();
    }
}

static void guard_button_click(lv_obj_t *obj)
{
    if (wait_hook_on_task != NULL) {
        return;
    }

    call_room[0] = 0;
    call_room[1] = 0;
    memset(call_num_save, 0, sizeof(call_num_save));
    sprintf(call_num_save, "%s", str_get(LAYOUT_INTERCOM_LANG_GUARD_ID));

    user_data_info *ud = user_data_get();
    if (call_room[0] == ud->device_id[0] && call_room[1] == ud->device_id[1]) {
        intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_LOCAL_ID));
        return;
    }

    guard_talking_pin_ctrl(true);
    lv_textarea_set_text(ta, "");

    if (hook_state_get() == true) {
        do_call_out();
    } else {
        intercom_wait_hook_on_task_create();
    }
}

static void intercom_number_kb_create(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_pos(cont, 150, 135);
    lv_obj_set_size(cont, 355, 395);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);

    static obj_click_data kb_click = obj_click_data_up_create(keyboard_button_click);
    static obj_click_data intercom_clk = obj_click_data_up_create(intercom_button_click);
    static obj_click_data guard_clk = obj_click_data_up_create(guard_button_click);

    create_keyboard_button(cont,   0,   0, "1", &kb_click, false);
    create_keyboard_button(cont, 128,   0, "2", &kb_click, false);
    create_keyboard_button(cont, 256,   0, "3", &kb_click, false);
    create_keyboard_button(cont,   0,  98, "4", &kb_click, false);
    create_keyboard_button(cont, 128,  98, "5", &kb_click, false);
    create_keyboard_button(cont, 256,  98, "6", &kb_click, false);
    create_keyboard_button(cont,   0, 196, "7", &kb_click, false);
    create_keyboard_button(cont, 128, 196, "8", &kb_click, false);
    create_keyboard_button(cont, 256, 196, "9", &kb_click, false);

    lv_obj_t *itc_btn = create_keyboard_button(cont, 0, 294, "", &intercom_clk, true);
    static rom_bin_info itc_info = rom_bin_info_get(ROM_UI_INTERCOM_CALL_PNG);
    lv_obj_set_style_local_pattern_image(itc_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &itc_info);

    create_keyboard_button(cont, 128, 294, "0", &kb_click, false);

    if (user_data_get()->room_no_flag == false) {
        lv_obj_t *gd_btn = create_keyboard_button(cont, 256, 294, "", &guard_clk, true);
        static rom_bin_info gd_info = rom_bin_info_get(ROM_UI_INTERCOM_GUARD_PNG);
        lv_obj_set_style_local_pattern_image(gd_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &gd_info);
    }
}

/* ---------------------------------------------------------------
 * 提示文本
 * --------------------------------------------------------------- */
static void intercom_call_out_result_task_create(const char *str)
{
    lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, str);
    lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
}

static void intercom_room_out_result_task_create(const char *str)
{
    lv_obj_set_style_local_value_str(tt, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, str);
    lv_obj_set_style_local_value_opa(tt, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
}

/* ---------------------------------------------------------------
 * 等待摘机任务（约 30 秒超时）
 * --------------------------------------------------------------- */
static void intercom_wait_hook_on_task_cb(lv_task_t *task_t)
{
    static int time_out = 0;
    if (time_out == 0)
    time_out = 60; 

    if (--time_out < 0) {
        guard_talking_pin_ctrl(false);
        lv_task_del(task_t);
        wait_hook_on_task = NULL;
        time_out = 0;
        lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
        return;
    }
    if (hook_state_get() == true) {
        lv_task_del(task_t);
        wait_hook_on_task = NULL;
        time_out = 0;
        do_call_out();
        return;
    }
    static bool blink = false;
    blink = !blink;
    lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT,
                                     blink ? call_num_save : str_get(LAYOUT_INTERCOM_LANG_TAKE_HANDSET_ID));
    lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
}

static void intercom_wait_hook_on_task_create(void)
{
    lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT,
                                     str_get(LAYOUT_INTERCOM_LANG_TAKE_HANDSET_ID));
    lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
    wait_hook_on_task = lv_layout_task_create(intercom_wait_hook_on_task_cb, 500, LV_TASK_PRIO_LOW, NULL);
}

/* ---------------------------------------------------------------
 * 布局生命周期
 * --------------------------------------------------------------- */
static void LAYOUT_ENTER_FUNC(intercom)
{
    lv_obj_t *parent = common_bg_display(lv_scr_act());
    textarea_input_select = false;
    back_btn_create(parent);
    setting_icon_create(parent);
    intercom_select_btn_create(parent);
    set_no_select_btn_create(parent);
    intercom_number_kb_create(parent);
    intercom_number_textarea_create(parent);
    intercom_room_textarea_create(parent);
    top_time_date_text_create(parent);
}

static void LAYOUT_QUIT_FUNC(intercom)
{
    user_data_get()->room_no_flag = false;
    user_data_save();

    if (wait_hook_on_task != NULL) {
        const layout *next = cur_layout_get();
        if (next != pLAYOUT(intercom_in)  &&
            next != pLAYOUT(intercom_talk) &&
            next != pLAYOUT(intercom_out)) {
            guard_talking_pin_ctrl(false);
        }
    }
    wait_hook_on_task  = NULL;
    kb_input_room_flag = false;
}

CREATE_LAYOUT(intercom);
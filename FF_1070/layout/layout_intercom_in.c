/**
 * @file    layout_intercom_in.c
 * @brief   内线来电等待界面
 *
 *  进入条件：
 *    底层收到 RQ_HANDSHAKE → Out_ACKFun() → intercom_call_in_flag=true
 *    → goto_layout(pLAYOUT(intercom_in))
 *
 *  状态机：
 *    进入时：intercom_state = INTERCOM_STATE_CALLING_IN
 *    摘机时：MsgCallAccept() → 底层发 RQ_TALKING → TalkAccpetFun()
 *            → intercom_state = INTERCOM_STATE_TALKING
 *            → calling_task 检测到后跳转 intercom_talk
 *    拒接时：MsgCallEnd() → 底层发 RQ_OFFLINE → In_HandupFun()
 *            → intercom_state = INTERCOM_STATE_IDLE
 *            → 跳转 standby
 *    对端挂断：In_HandupFun() → intercom_state = INTERCOM_STATE_IDLE
 *              → calling_task 检测到后跳转 standby
 *    30秒超时：自动调 MsgCallEnd() 拒接
 */

#include "layout_define.h"
#include "intercom.h"
#include "user_intercom.h"

typedef enum
{
    INTERCOM_IN_HOME_BTN_ID,
    INTERCOM_IN_SOUND_BTN_ID,
    HOME_GEAR_OBJ_ID,
    INTERCOM_IN_TOTAL_BTN,
} intercom_in_btn_module;

static custom_area intercom_in_btn_area[INTERCOM_IN_TOTAL_BTN] =
{
    {920, 25, 54, 54},   /* 返回/拒接按钮 */
    {892, 461, 80, 80},  /* 声音按钮 */
};

#define INTERCOM_IN_TEXT_LABEL_ID           0
#define INTERCOM_IN_RING_CONT_ID            1
#define INTERCOM_IN_RING_BAR_ID             2
#define INTERCOM_IN_VOL_SLIDER_ID           4
#define INTERCOM_IN_VOL_LABEL_ID            5
#define INTERCOM_IN_COUNTDOWN_LABEL_ID      6
#define INTERCOM_IN_COUNTDOWN_RING_ID       7
#define INTERCOM_IN_CALL_FROM_LABEL_ID      8
#define INTERCOM_IN_ROOM_NUMBER_LABEL_ID    9
#define INTERCOM_IN_VOL_SLIDER_BG_ID        10
#define INTERCOM_IN_VOL_SLIDER_INDICATOR_ID 11
#define INTERCOM_IN_SOUND_BTN_OBJ_ID        12
#define INTERCOM_LOCAL_ADDR_ID              13
#define INTERCOM_IN_GUARD_IMG_ID            14
#define GUARD_INTERCOM_NUMBER   0

static lv_task_t *hung_up_task  = NULL;
static lv_task_t *calling_task  = NULL;
static int count = 30;
static bool vol_slider_visible = false;

static void intercom_in_hung_up_task_create(void);

/* ---------------------------------------------------------------
 * 拒接（返回按钮 / 挂断按钮）
 * --------------------------------------------------------------- */
static void do_hang_up(void)
{
    if (hung_up_task != NULL) return; /* 防止重复触发 */

    MsgCallEnd();  /* 通知底层发送 RQ_OFFLINE */
    intercom_state_set(INTERCOM_STATE_IDLE);
    intercom_hangup_flag_get_and_clear();      

    if (calling_task != NULL) {
        lv_task_del(calling_task);
        calling_task = NULL;
    }
    intercom_in_hung_up_task_create();
}

static void intercom_in_back_btn_up(lv_obj_t *obj) { do_hang_up(); }

static void intercom_in_back_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(intercom_in_back_btn_up);
    static rom_bin_info   info     = rom_bin_info_get(ROM_UI_CAMERA_HOME_PNG);
    common_img_btn_create(parent, intercom_in_btn_area[INTERCOM_IN_HOME_BTN_ID], NULL, &btn_data, &info);
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

    lv_obj_t *lbl = lv_label_create(parent, NULL);
    lv_obj_set_pos(lbl, 87, 28);
    lv_obj_set_size(lbl, 70, 31);
    lv_obj_set_id(lbl, INTERCOM_LOCAL_ADDR_ID);
    static char str[40] = {0};
    if (user_data_get()->device_id[0] == 0 && user_data_get()->device_id[1] == 0)
    sprintf(str, "%s : %s", str_get(LAYOUT_INTERCOM_LANG_ROOM_NO_ID), str_get(LAYOUT_INTERCOM_LANG_GUARD_1_ID));
    else
    sprintf(str, "%s : %hhu%hhu", str_get(LAYOUT_INTERCOM_LANG_ROOM_NO_ID),
    user_data_get()->device_id[0], user_data_get()->device_id[1]);
    lv_label_set_text(lbl, str);
    lv_obj_set_style_local_text_font(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(22));
    lv_obj_set_style_local_text_color(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_align(icon, lbl, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}

/* ---------------------------------------------------------------
 * 音量控件
 * --------------------------------------------------------------- */
static void update_sound_btn_image(void)
{
    lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_SOUND_BTN_OBJ_ID);
    if (!btn)
    return;
    static rom_bin_info sound_info = rom_bin_info_get(ROM_UI_INTERCOM_SOUND_PNG);
    static rom_bin_info mute_info  = rom_bin_info_get(ROM_UI_INTERCOM_MUTE_PNG);
    lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,
    user_data_get()->setting.inter_ring_volume > 0 ? &sound_info : &mute_info);
}

static void vol_slider_event_cb(lv_obj_t *slider, lv_event_t event)
{
    if (event != LV_EVENT_VALUE_CHANGED) return;
    int vol = lv_slider_get_value(slider);
    user_data_get()->setting.inter_ring_volume = vol;
    lv_obj_t *lbl = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_VOL_LABEL_ID);
    if (lbl) lv_label_set_text_fmt(lbl, "%d", vol);
    update_sound_btn_image();
    lv_obj_t *ind = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_VOL_SLIDER_INDICATOR_ID);
    if (ind) 
    {
        int h = (vol * 219) / 4;
        lv_obj_set_height(ind, h);
        lv_obj_set_pos(ind, 925, 387 - h);
    }
    ring_volume_set(vol);
}

static void intercom_in_volume_slider_create(lv_obj_t *parent)
{
    lv_obj_t *bg = lv_obj_create(parent, NULL);
    lv_obj_set_id(bg, INTERCOM_IN_VOL_SLIDER_BG_ID);
    lv_obj_set_size(bg, 14, 219);
    lv_obj_set_pos(bg, 925, 168);
    lv_obj_set_style_local_bg_color(bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x303030));
    lv_obj_set_style_local_radius(bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 62);
    lv_obj_set_style_local_bg_opa(bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

    int cur = user_data_get()->setting.inter_ring_volume;
    int h = (cur * 219) / 4;

    lv_obj_t *ind = lv_obj_create(parent, NULL);
    lv_obj_set_id(ind, INTERCOM_IN_VOL_SLIDER_INDICATOR_ID);
    lv_obj_set_size(ind, 14, h);
    lv_obj_set_pos(ind, 925, 387 - h);
    lv_obj_set_style_local_bg_color(ind, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0A84FF));
    lv_obj_set_style_local_radius(ind, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 62);
    lv_obj_set_style_local_bg_opa(ind, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

    lv_obj_t *slider = lv_slider_create(parent, NULL);
    lv_obj_set_id(slider, INTERCOM_IN_VOL_SLIDER_ID);
    lv_obj_set_size(slider, 28, 219);
    lv_obj_set_pos(slider, 918, 168);
    lv_slider_set_range(slider, 0, 4);
    lv_slider_set_value(slider, cur, LV_ANIM_OFF);
    lv_obj_set_style_local_bg_opa(slider, LV_SLIDER_PART_BG,    LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_radius(slider, LV_SLIDER_PART_KNOB,  LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_bg_opa(slider, LV_SLIDER_PART_KNOB,  LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_size(slider, LV_SLIDER_PART_KNOB,    LV_STATE_DEFAULT, 28);
    lv_slider_set_anim_time(slider, 0);
    lv_obj_set_event_cb(slider, vol_slider_event_cb);

    lv_obj_t *lbl = lv_label_create(parent, NULL);
    lv_obj_set_id(lbl, INTERCOM_IN_VOL_LABEL_ID);
    lv_obj_set_size(lbl, 15, 28);
    lv_obj_set_pos(lbl, 925, 403);
    lv_obj_set_style_local_text_font(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
    lv_label_set_text_fmt(lbl, "%d", cur);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);

    lv_obj_set_hidden(bg , false);
    lv_obj_set_hidden(ind, false);
    lv_obj_set_hidden(slider, false);
    lv_obj_set_hidden(lbl, false);
    vol_slider_visible = true;
}

static void intercom_in_sound_btn_up(lv_obj_t *obj)
{
    lv_obj_t *bg  = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_VOL_SLIDER_BG_ID);
    lv_obj_t *ind = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_VOL_SLIDER_INDICATOR_ID);
    lv_obj_t *sl  = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_VOL_SLIDER_ID);
    lv_obj_t *lbl = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_VOL_LABEL_ID);
    if (!bg || !ind || !sl || !lbl) return;
    vol_slider_visible = !vol_slider_visible;
    lv_obj_set_hidden(bg,  !vol_slider_visible);
    lv_obj_set_hidden(ind, !vol_slider_visible);
    lv_obj_set_hidden(sl,  !vol_slider_visible);
    lv_obj_set_hidden(lbl, !vol_slider_visible);
}

static void intercom_in_sound_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(intercom_in_sound_btn_up);
    lv_obj_t *btn = common_img_btn_create(parent, intercom_in_btn_area[INTERCOM_IN_SOUND_BTN_ID], NULL, &btn_data, NULL);
    lv_obj_set_id(btn, INTERCOM_IN_SOUND_BTN_OBJ_ID);
    update_sound_btn_image();
}

/* ---------------------------------------------------------------
 * 挂断按钮
 * --------------------------------------------------------------- */
static void intercom_in_hung_up_btn_up(lv_obj_t *obj) { do_hang_up(); }

static void intercom_in_text_icon_create(lv_obj_t *parent)
{
    lv_obj_t *btn = lv_obj_create(parent, NULL);
    lv_obj_set_size(btn, 170, 76);
    lv_obj_set_pos(btn, 427, 490);
    lv_obj_set_ext_click_area(btn, 20, 20, 20, 20);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_INTERCOM_HUNG_UP_PNG);
    lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
    lv_obj_set_style_local_pattern_recolor(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
    lv_obj_set_style_local_pattern_recolor_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
    static obj_click_data btn_data = obj_click_data_up_create(intercom_in_hung_up_btn_up);
    obj_click_event_listen(btn, &btn_data);
}

/* ---------------------------------------------------------------
 * 倒计时环 + 秒数标签
 * --------------------------------------------------------------- */
static void intercom_in_countdown_create(lv_obj_t *parent)
{
    lv_obj_t *ring = lv_obj_create(parent, NULL);
    lv_obj_set_size(ring, 270, 270);
    lv_obj_set_pos(ring, 376, 151);
    static rom_bin_info ring_info = rom_bin_info_get(ROM_UI_INTERCOM_COUNTDOWN_RING_PNG);
    lv_obj_set_style_local_pattern_image(ring, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &ring_info);

    lv_obj_t *lbl = lv_label_create(parent, NULL);
    lv_obj_set_id(lbl, INTERCOM_IN_COUNTDOWN_LABEL_ID);
    lv_obj_set_size(lbl, 110, 77);
    lv_obj_set_pos(lbl, 456, 248);
    lv_obj_set_style_local_text_font(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(64));
    lv_obj_set_style_local_text_line_space(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_text_letter_space(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_label_set_text_fmt(lbl, "30S");
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
}

/* ---------------------------------------------------------------
 * 来电方房号显示
 * --------------------------------------------------------------- */
static void intercom_in_arrows_create(lv_obj_t *parent)
{
    lv_obj_t *from_lbl = lv_label_create(parent, NULL);
    lv_obj_set_id(from_lbl, INTERCOM_IN_CALL_FROM_LABEL_ID);
    lv_obj_set_size(from_lbl, 181, 39);
    lv_obj_set_pos(from_lbl, 101, 266);
    lv_obj_set_style_local_text_font(from_lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(32));
    lv_label_set_text(from_lbl, str_get(LAYOUT_INTERCOM_LANG_RINGING_FROM_ID));

    lv_obj_t *guard_img = lv_obj_create(parent, NULL);
    lv_obj_set_id(guard_img, INTERCOM_IN_GUARD_IMG_ID);
    lv_obj_set_size(guard_img, 100, 100);
    lv_obj_set_pos(guard_img, 746, 233);
    static rom_bin_info guard_info = rom_bin_info_get(ROM_UI_INTERCOM_GUARD_TALKING_PNG);
    lv_obj_set_style_local_pattern_image(guard_img, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &guard_info);

    lv_obj_t *room_lbl = lv_label_create(parent, NULL);
    lv_obj_set_id(room_lbl, INTERCOM_IN_ROOM_NUMBER_LABEL_ID);
    lv_obj_set_size(room_lbl, 100, 39);
    lv_obj_set_pos(room_lbl, 750, 266);
    lv_obj_set_style_local_text_font(room_lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(32));

    /* 从高层状态获取来电方房号 */
    unsigned int call_in_number = intercom_number_get();
    if (call_in_number == GUARD_INTERCOM_NUMBER) {
        lv_obj_set_hidden(room_lbl,  true);
        lv_obj_set_hidden(guard_img, false);
    } else {
        lv_obj_set_hidden(guard_img, true);
        lv_obj_set_hidden(room_lbl,  false);
        lv_label_set_text_fmt(room_lbl, "%03u", call_in_number);
    }
}

/* ---------------------------------------------------------------
 * 挂断后延迟跳转
 * --------------------------------------------------------------- */
static void intercom_in_hung_up_task_cb(lv_task_t *task_t)
{
    goto_layout(pLAYOUT(standby));
}

static void intercom_in_hung_up_task_create(void)
{
    lv_obj_t *lbl = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_COUNTDOWN_LABEL_ID);
    if (lbl) 
    lv_label_set_text(lbl, "00S");
    hung_up_task = lv_layout_task_create(intercom_in_hung_up_task_cb, 2000, LV_TASK_PRIO_LOW, NULL);
}

/* ---------------------------------------------------------------
 * 铃声回调
 * --------------------------------------------------------------- */
static void intercom_ring_play_start_fun(int index)
{
    ring_volume_set(user_data_get()->setting.inter_ring_volume);
    power_amplifier_enable(true);
}

static void intercom_ring_play_finish_fun(int index)
{
    power_amplifier_enable(false);
}

/* ---------------------------------------------------------------
 * 来电等待周期任务（每 1 秒）
 * --------------------------------------------------------------- */
static void intercom_in_calling_time_out_task(lv_task_t *task_t)
{
    /* ① 对端已接听 / 通话建立（TalkAccpetFun 已置位 remote_ack） */
    if (intercom_remote_ack_get_and_clear()) {
        printf("[intercom_in] remote accepted, goto intercom_talk\n");
        if (calling_task != NULL) { lv_task_del(calling_task); calling_task = NULL; }
        goto_layout(pLAYOUT(intercom_talk));
        return;
    }

    /* ② 挂断事件 */
    if (intercom_hangup_flag_get_and_clear()) {
        printf("[intercom_in] hung up detected, goto standby\n");
        if (calling_task != NULL) { lv_task_del(calling_task); calling_task = NULL; }
        if (hung_up_task == NULL) intercom_in_hung_up_task_create();
        return;
    }

    /* ③ 用户摘机 → 接听 */
    if (hook_state_get() == true &&
        intercom_state_get() == INTERCOM_STATE_CALLING_IN) {
        printf("[intercom_in] hook on, accepting call\n");
        intercom_state_set(INTERCOM_STATE_TALKING);
        MsgCallAccept(); 
        return;
    }

    /* ④ 更新倒计时显示 */
    lv_obj_t *lbl = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_IN_COUNTDOWN_LABEL_ID);
    if (lbl) lv_label_set_text_fmt(lbl, "%02dS", count);

    /* ⑤ 每 3 秒响铃一次 */
    if (count % 3 == 0 && user_data_get()->setting.inter_ring_volume > 0) {
        ringplay_play_form_index(9, 100, intercom_ring_play_start_fun, intercom_ring_play_finish_fun, false);
    }

    /* ⑥ 倒计时结束，自动拒接 */
    if (--count < 0) {
        printf("[intercom_in] timeout, auto refuse\n");
        if (calling_task != NULL) { lv_task_del(calling_task); calling_task = NULL; }
        MsgCallEnd();
        intercom_state_set(INTERCOM_STATE_IDLE);
        intercom_in_hung_up_task_create();
    }
}

static void intercom_in_calling_time_out_task_create(void)
{
    count = 30;
    calling_task = lv_layout_task_create(intercom_in_calling_time_out_task, 1000, LV_TASK_PRIO_HIGH, NULL);
}

/* ---------------------------------------------------------------
 * 布局生命周期
 * --------------------------------------------------------------- */
static void LAYOUT_ENTER_FUNC(intercom_in)
{
    power_amplifier_enable(true);
    standby_timer_close();
    backlight_enable(true);
    lv_obj_t *parent = common_bg_display(lv_scr_act());

    intercom_in_back_btn_create(parent);
    setting_icon_create(parent);
    intercom_in_sound_btn_create(parent);
    intercom_in_text_icon_create(parent);
    intercom_in_volume_slider_create(parent);
    intercom_in_countdown_create(parent);
    intercom_in_arrows_create(parent);
    top_time_date_text_create(parent);
    lv_obj_click_down_callback_register(NULL);

    intercom_remote_ack_get_and_clear();
    intercom_hangup_flag_get_and_clear();
    intercom_busy_flag_get_and_clear();
    intercom_unack_flag_get_and_clear();

    /* 标记当前为被叫侧 */
    intercom_call_in_flag = true;
    intercom_state_set(INTERCOM_STATE_CALLING_IN);

    /* 通知底层 UI 已就绪 */
    UiIntercomStateNormal();

    /* 启动来电等待倒计时任务 */
    intercom_in_calling_time_out_task_create();
}

static void LAYOUT_QUIT_FUNC(intercom_in)
{
    hung_up_task = NULL;
    calling_task = NULL;
    standby_timer_restart(true);
    lv_obj_click_down_callback_register(layout_obj_click_down_func);
    user_data_save();
    ringplay_play_stop();
}

CREATE_LAYOUT(intercom_in);
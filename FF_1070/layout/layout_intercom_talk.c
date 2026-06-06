/**
 * @file    layout_intercom_talk.c
 * @brief   内线通话界面（双方已接通）
 *
 *  进入条件：
 *    主叫侧：intercom_out 的 calling_task 检测到 remote_ack 后跳转
 *    被叫侧：intercom_in 的 calling_task 检测到 remote_ack 后跳转
 *
 *  状态机：
 *    进入时：intercom_state = INTERCOM_STATE_TALKING（已由 TalkAccpetFun 设置）
 *            开启音频通道
 *    用户挂机（放回听筒）：hook_state_get() == false → MsgCallEnd()
 *    用户点击挂断按钮：MsgCallEnd()
 *    对端挂断：In_HandupFun() → intercom_state = IDLE → talking_task 检测跳转
 *    3分钟通话上限：自动挂断
 */

#include "layout_define.h"
#include "intercom.h"
#include "user_intercom.h"

typedef enum
{
    INTERCOM_TALK_HOME_BTN_ID,
    INTERCOM_TALK_SOUND_BTN_ID,
    HOME_GEAR_OBJ_ID,
    INTERCOM_TALK_TOTAL_BTN,
} intercom_talk_btn_module;

static custom_area intercom_talk_btn_area[INTERCOM_TALK_TOTAL_BTN] =
{
    {920, 25, 54, 54},   /* 返回/挂断按钮 */
    {892, 461, 80, 80},  /* 声音按钮      */
};

#define INTERCOM_TALK_TEXT_LABEL_ID           0
#define INTERCOM_TALK_RING_CONT_ID            1
#define INTERCOM_TALK_RING_BAR_ID             2
#define INTERCOM_TALK_VOL_SLIDER_ID           4
#define INTERCOM_TALK_VOL_LABEL_ID            5
#define INTERCOM_TALK_TIME_COUNT_LABEL_ID     6
#define INTERCOM_TALK_COUNTDOWN_RING_ID       7
#define INTERCOM_TALK_CALL_FROM_LABEL_ID      8
#define INTERCOM_TALK_ROOM_NUMBER_LABEL_ID    9
#define INTERCOM_TALK_VOL_SLIDER_BG_ID        10
#define INTERCOM_TALK_VOL_SLIDER_INDICATOR_ID 11
#define INTERCOM_TALK_SOUND_BTN_OBJ_ID        12
#define INTERCOM_LOCAL_ADDR_ID                13

/* 管理机保留号码 */
#define GUARD_INTERCOM_NUMBER   0

static lv_task_t *hung_up_task  = NULL;
static lv_task_t *talking_task  = NULL;
static int count = 180; 
static bool vol_slider_visible = false;

static void intercom_talk_hung_up_task_create(void);

/* ---------------------------------------------------------------
 * 挂断
 * --------------------------------------------------------------- */
static void do_hang_up(void)
{
    if (hung_up_task != NULL) return;

    MsgCallEnd();
    intercom_state_set(INTERCOM_STATE_IDLE);
    intercom_hangup_flag_get_and_clear();

    if (talking_task != NULL) { lv_task_del(talking_task); talking_task = NULL; }
    intercom_talk_hung_up_task_create();
}

static void intercom_talk_back_btn_up(lv_obj_t *obj)  { do_hang_up(); }

static void intercom_talk_back_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(intercom_talk_back_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_HOME_PNG);
    common_img_btn_create(parent, intercom_talk_btn_area[INTERCOM_TALK_HOME_BTN_ID], NULL, &btn_data, &info);
}

/* ---------------------------------------------------------------
 * 设置图标 + 本机房号
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
    lv_obj_t *btn = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_TALK_SOUND_BTN_OBJ_ID);
    if (!btn) return;
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
    lv_obj_t *lbl = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_TALK_VOL_LABEL_ID);
    if (lbl) lv_label_set_text_fmt(lbl, "%d", vol);
    update_sound_btn_image();
    lv_obj_t *ind = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_TALK_VOL_SLIDER_INDICATOR_ID);
    if (ind) {
        int h = (vol * 219) / 4;
        lv_obj_set_height(ind, h);
        lv_obj_set_pos(ind, 925, 387 - h);
    }
    ring_volume_set(vol);
}

static void intercom_talk_volume_slider_create(lv_obj_t *parent)
{
    lv_obj_t *bg = lv_obj_create(parent, NULL);
    lv_obj_set_id(bg, INTERCOM_TALK_VOL_SLIDER_BG_ID);
    lv_obj_set_size(bg, 14, 219);
    lv_obj_set_pos(bg, 925, 168);
    lv_obj_set_style_local_bg_color(bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x303030));
    lv_obj_set_style_local_radius(bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 62);
    lv_obj_set_style_local_bg_opa(bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

    int cur = user_data_get()->setting.inter_ring_volume;
    int h = (cur * 219) / 4;

    lv_obj_t *ind = lv_obj_create(parent, NULL);
    lv_obj_set_id(ind, INTERCOM_TALK_VOL_SLIDER_INDICATOR_ID);
    lv_obj_set_size(ind, 14, h);
    lv_obj_set_pos(ind, 925, 387 - h);
    lv_obj_set_style_local_bg_color(ind, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0A84FF));
    lv_obj_set_style_local_radius(ind, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 62);
    lv_obj_set_style_local_bg_opa(ind, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);

    lv_obj_t *slider = lv_slider_create(parent, NULL);
    lv_obj_set_id(slider, INTERCOM_TALK_VOL_SLIDER_ID);
    lv_obj_set_size(slider, 28, 219);
    lv_obj_set_pos(slider, 918, 168);
    lv_slider_set_range(slider, 0, 4);
    lv_slider_set_value(slider, cur, LV_ANIM_OFF);
    lv_obj_set_style_local_bg_opa(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(slider, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_radius(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_bg_color(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_bg_opa(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_size(slider, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 28);
    lv_slider_set_anim_time(slider, 0);
    lv_obj_set_event_cb(slider, vol_slider_event_cb);

    lv_obj_t *lbl = lv_label_create(parent, NULL);
    lv_obj_set_id(lbl, INTERCOM_TALK_VOL_LABEL_ID);
    lv_obj_set_size(lbl, 15, 28);
    lv_obj_set_pos(lbl, 925, 403);
    lv_obj_set_style_local_text_font(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
    lv_label_set_text_fmt(lbl, "%d", cur);
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);

    lv_obj_set_hidden(bg, true);
    lv_obj_set_hidden(ind, true);
    lv_obj_set_hidden(slider, true);
    lv_obj_set_hidden(lbl, true);
    vol_slider_visible = false;
}

static void intercom_talk_sound_btn_up(lv_obj_t *obj)
{
    lv_obj_t *bg  = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_TALK_VOL_SLIDER_BG_ID);
    lv_obj_t *ind = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_TALK_VOL_SLIDER_INDICATOR_ID);
    lv_obj_t *sl  = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_TALK_VOL_SLIDER_ID);
    lv_obj_t *lbl = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_TALK_VOL_LABEL_ID);
    if (!bg || !ind || !sl || !lbl) return;
    vol_slider_visible = !vol_slider_visible;
    lv_obj_set_hidden(bg, !vol_slider_visible);
    lv_obj_set_hidden(ind, !vol_slider_visible);
    lv_obj_set_hidden(sl, !vol_slider_visible);
    lv_obj_set_hidden(lbl, !vol_slider_visible);
}

static void intercom_talk_sound_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(intercom_talk_sound_btn_up);
    lv_obj_t *btn = common_img_btn_create(parent, intercom_talk_btn_area[INTERCOM_TALK_SOUND_BTN_ID], NULL, &btn_data, NULL);
    lv_obj_set_id(btn, INTERCOM_TALK_SOUND_BTN_OBJ_ID);
    update_sound_btn_image();
}

/* ---------------------------------------------------------------
 * 挂断按钮
 * --------------------------------------------------------------- */
static void intercom_talk_hung_up_btn_up(lv_obj_t *obj) 
{ 
    do_hang_up(); 
}

static void intercom_talk_text_icon_create(lv_obj_t *parent)
{
    lv_obj_t *btn = lv_obj_create(parent, NULL);
    lv_obj_set_size(btn, 170, 76);
    lv_obj_set_pos(btn, 427, 490);
    lv_obj_set_ext_click_area(btn, 20, 20, 20, 20);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_INTERCOM_HUNG_UP_PNG);
    lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info);
    lv_obj_set_style_local_pattern_recolor(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
    lv_obj_set_style_local_pattern_recolor_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
    static obj_click_data btn_data = obj_click_data_up_create(intercom_talk_hung_up_btn_up);
    obj_click_event_listen(btn, &btn_data);
}

/* ---------------------------------------------------------------
 * 通话时长计时环 + 标签（正计时）
 * --------------------------------------------------------------- */
static void intercom_talk_countdown_create(lv_obj_t *parent)
{
    lv_obj_t *ring = lv_obj_create(parent, NULL);
    lv_obj_set_id(ring, INTERCOM_TALK_COUNTDOWN_RING_ID);
    lv_obj_set_size(ring, 270, 270);
    lv_obj_set_pos(ring, 376, 151);
    static rom_bin_info ring_info = rom_bin_info_get(ROM_UI_INTERCOM_COUNTDOWN_RING_PNG);
    lv_obj_set_style_local_pattern_image(ring, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &ring_info);

    lv_obj_t *lbl = lv_label_create(parent, NULL);
    lv_obj_set_id(lbl, INTERCOM_TALK_TIME_COUNT_LABEL_ID);
    lv_obj_set_size(lbl, 200, 77);
    lv_obj_set_pos(lbl, 465, 260);
    lv_obj_set_style_local_text_font(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(40));
    lv_obj_set_style_local_text_line_space(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_text_letter_space(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_label_set_text(lbl, "00:00");
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
}

/* ---------------------------------------------------------------
 * 对端房号显示
 * --------------------------------------------------------------- */
static void intercom_talk_arrows_create(lv_obj_t *parent)
{
    lv_obj_t *with_lbl = lv_label_create(parent, NULL);
    lv_obj_set_id(with_lbl, INTERCOM_TALK_CALL_FROM_LABEL_ID);
    lv_obj_set_size(with_lbl, 181, 39);
    lv_obj_set_pos(with_lbl, 101, 266);
    lv_obj_set_style_local_text_font(with_lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(32));
    lv_label_set_text(with_lbl, str_get(LAYOUT_INTERCOM_LANG_TALKING_WITH_ID));

    lv_obj_t *room_lbl = lv_label_create(parent, NULL);
    lv_obj_set_id(room_lbl, INTERCOM_TALK_ROOM_NUMBER_LABEL_ID);
    lv_obj_set_size(room_lbl, 150, 39);
    lv_obj_set_pos(room_lbl, 750, 266);
    lv_obj_set_style_local_text_font(room_lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(32));

    unsigned int talking_number = intercom_number_get();
    if (talking_number == GUARD_INTERCOM_NUMBER) {
        lv_label_set_text(room_lbl, str_get(LAYOUT_INTERCOM_LANG_GUARD_1_ID));
    } else {
        lv_label_set_text_fmt(room_lbl, "%03u", talking_number);
    }
}

static void intercom_talk_hung_up_task_cb(lv_task_t *task_t)
{
    if (intercom_call_in_flag) {
        goto_layout(pLAYOUT(standby));
    } else {
        goto_layout(pLAYOUT(intercom));
    }
}

static void intercom_talk_hung_up_task_create(void)
{
    lv_obj_t *lbl = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_TALK_TIME_COUNT_LABEL_ID);
    if (lbl) lv_label_set_text(lbl, "00:00");
    hung_up_task = lv_layout_task_create(intercom_talk_hung_up_task_cb, 2000, LV_TASK_PRIO_LOW, NULL);
}

static void intercom_talk_talking_task(lv_task_t *task_t)
{
    /* ① 物理挂机（听筒放回） */
    if (hook_state_get() == false) {
        printf("[intercom_talk] hook down, hang up\n");
        do_hang_up();
        return;
    }

    /* ② 对端挂断 */
    if (intercom_hangup_flag_get_and_clear() ||
        intercom_state_get() == INTERCOM_STATE_IDLE) {
        printf("[intercom_talk] remote hung up\n");
        if (talking_task != NULL) { lv_task_del(talking_task); talking_task = NULL; }
        if (hung_up_task == NULL) intercom_talk_hung_up_task_create();
        return;
    }

    /* ③ 通话时长上限 */
    if (--count < 0) {
        printf("[intercom_talk] talk time limit reached\n");
        do_hang_up();
        return;
    }

    /* ④ 正计时显示：elapsed = 180 - count */
    int elapsed = 180 - count;
    lv_obj_t *lbl = (lv_obj_t *)task_t->user_data;
    if (lbl) {
        lv_label_set_text_fmt(lbl, "%02d:%02d", elapsed / 60, elapsed % 60);
    }
}

static void intercom_talk_talking_task_create(void)
{
    if (talking_task != NULL) { 
        lv_task_del(talking_task); talking_task = NULL; 
    }

    lv_obj_t *lbl = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_TALK_TIME_COUNT_LABEL_ID);
    if (lbl) {
        lv_label_set_text(lbl, "00:00");
    }

    count = 180;
    talking_task = lv_layout_task_create(intercom_talk_talking_task, 1000, LV_TASK_PRIO_LOW, lbl);

    unsigned int num = intercom_number_get();
    if (num == GUARD_INTERCOM_NUMBER) {
        door_audio_talk(AUDIO_CH_GUARD);
    } else {
        door_audio_talk(AUDIO_CH_INTER);
    }
}

/* ---------------------------------------------------------------
 * 布局生命周期
 * --------------------------------------------------------------- */
static void LAYOUT_ENTER_FUNC(intercom_talk)
{
    power_amplifier_enable(true);
    standby_timer_close();
    backlight_enable(true);
    lv_obj_t *parent = common_bg_display(lv_scr_act());

    intercom_talk_back_btn_create(parent);
    setting_icon_create(parent);
    intercom_talk_sound_btn_create(parent);
    intercom_talk_text_icon_create(parent);
    intercom_talk_volume_slider_create(parent);
    intercom_talk_countdown_create(parent);
    intercom_talk_arrows_create(parent);
    top_time_date_text_create(parent);
    intercom_remote_ack_get_and_clear();
    intercom_hangup_flag_get_and_clear();

    /* 确认状态为通话中 */
    intercom_state_set(INTERCOM_STATE_TALKING);

    /* 通知底层 UI 正常 */
    UiIntercomStateNormal();

    /* 启动通话计时 + 开音频通道 */
    intercom_talk_talking_task_create();
}

static void LAYOUT_QUIT_FUNC(intercom_talk)
{
    hung_up_task = NULL;
    talking_task = NULL;
    standby_timer_restart(true);
    door_audio_talk(AUDIO_CH_CLOSE); /* 关闭音频通道 */
    user_data_save();
}

CREATE_LAYOUT(intercom_talk);
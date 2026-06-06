/*******************************************************************
 * @Descripttion   : 来电呼叫界面 (Calling UI)
 * @version        : 1.0.0
 * @Author         : lynn
 * @Date           : 2026-03-16
 *******************************************************************/
#include "layout_define.h"
#include "lv_msg_event.h"
#include "lvgl/lvgl.h"

#define CALLING_LABEL_ID 1
#define LAYOUT_LANG_confirm_delete_ID 2
// static lv_task_t * g_calling_task = NULL;

static void calling_play_start_cb(int index)
{
    ring_volume_set(user_data_get()->setting.door_ring_volume);
}

static void calling_play_finish_cb(int index)
{
}

// static void calling_task_cb(lv_task_t *task)
// {
//     ringplay_play_stop();
//     goto_layout(pLAYOUT(home)); 
// }

static void calling_back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(home)); 
}

// 创建返回按钮
static void calling_back_btn_create(lv_obj_t *parent)
{
    lv_obj_t *back_icon_obj = lv_img_create(parent, NULL);
    lv_obj_set_pos(back_icon_obj, 920, 25);
    lv_obj_set_size(back_icon_obj, 50, 37);
    static rom_bin_info info1 = rom_bin_info_get(ROM_UI_TIME_BACK_PNG);
    lv_obj_set_style_local_pattern_image(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
    lv_obj_set_style_local_pattern_recolor(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
    lv_obj_set_style_local_pattern_recolor_opa(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
    static obj_click_data btn_data = obj_click_data_up_create(calling_back_btn_up);
    obj_click_event_listen(back_icon_obj, &btn_data);
}


lv_obj_t *calling_container_create(lv_obj_t *parent, obj_click_data *yes_btn_data, layout_lang_id id)
{
    // 1. 整体消息框容器 
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(cont, 466, 158);
    lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x838383)); 
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00D0FF));
    lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
    lv_obj_set_click(cont, false);

    // 2. 提示语区域 (上2/3，460*96) - 禁用点击
    lv_obj_t *msg_area = lv_cont_create(cont, NULL);
    lv_obj_set_size(msg_area, 466, 70);
    lv_obj_align(msg_area, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
    lv_obj_set_style_local_bg_opa(msg_area, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_pad_all(msg_area, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 15);
    lv_obj_set_click(msg_area, false); 

    // 提示语文本
    lv_obj_t *msg_label = lv_label_create(msg_area, NULL);
    lv_label_set_text(msg_label, str_get(LAYOUT_LANG_calling_ID)); // 删除提示语
    lv_obj_set_style_local_text_color(msg_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_text_font(msg_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_label_set_align(msg_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(msg_label, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_click(msg_label, false); // 禁用文本点击

    // 4. 按钮区域容器 (下1/3，460*60)
    lv_obj_t *btn_area = lv_cont_create(cont, NULL);
    lv_obj_set_size(btn_area, 466, 80);
    lv_obj_align(btn_area, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_local_bg_opa(btn_area, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_pad_all(btn_area, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_click(btn_area, false); // 禁用按钮区域容器点击，仅单个按钮可交互

    // 5. 确认按钮 (230*60) - 唯一可点击的区域之一
    lv_obj_t *yes_btn = lv_obj_create(btn_area, NULL);
    lv_obj_align(yes_btn, NULL, LV_ALIGN_CENTER, 0, 0);

    // 底层：TRUE 图标（子对象，仅显示，禁用点击）
    lv_obj_t *yes_icon = lv_obj_create(yes_btn, NULL);
    lv_obj_align(yes_icon, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_bg_opa(yes_icon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    static rom_bin_info info_yes = rom_bin_info_get(ROM_UI_SETTING_TRUE_PNG);
    lv_obj_set_style_local_pattern_image(yes_icon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info_yes);
    lv_obj_set_style_local_pattern_opa(yes_icon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_pattern_align(yes_icon, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
    lv_obj_set_click(yes_icon, false); // 禁用点击，穿透到yes_btn

    // 光晕效果绑定到yes_btn的PRESSED状态
    static rom_bin_info info_glow = rom_bin_info_get(ROM_UI_SETTING_GLOOM_PNG);
    lv_obj_set_style_local_pattern_image(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info_glow);
    lv_obj_set_style_local_pattern_opa(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_pattern_opa(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_COVER);
    lv_obj_set_style_local_pattern_align(yes_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);

    if (yes_btn_data != NULL)
        obj_click_event_listen(yes_btn, yes_btn_data); // 绑定确认按钮点击回调

    return cont;
}

static void yes_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(home));
}

static obj_click_data yes_btn_data = obj_click_data_up_create(yes_btn_up);
static void LAYOUT_ENTER_FUNC(calling)
{
    lv_obj_t *parent = common_bg_display(lv_scr_act());
    calling_back_btn_create(parent);
    calling_container_create(parent, &yes_btn_data, LAYOUT_LANG_calling_ID);
    int system_vol = user_data_get()->setting.door_ring_volume;
    ring_volume_set(system_vol);
    ringplay_play_form_index(1, 100, 
                             calling_play_start_cb, 
                             calling_play_finish_cb, 
                             false); 
    // g_calling_task = lv_task_create(calling_task_cb, 30000, LV_TASK_PRIO_MID, NULL);
}

static void LAYOUT_QUIT_FUNC(calling)
{
    ringplay_play_stop();
    standby_timer_restart(true);
    // if(g_calling_task != NULL)
    // {
    //     lv_task_del(g_calling_task);
    //     g_calling_task = NULL;
    // }
}
CREATE_LAYOUT(calling);
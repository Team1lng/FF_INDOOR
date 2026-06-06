#include "layout_define.h"
#include "lv_msg_event.h"
#include "lvgl/lvgl.h"
#include "lvgl/lv_obj.h"
#include "lvgl/lv_label.h"
#include "language.h"
#include "lvgl/lv_task.h"  

#define HOME_BACK_OBJ_ID 0x10
#define HOME_GEAR_OBJ_ID 0x11
#define SETTING_format_SD_BIN_ID 0x12
#define SETTING_SD_CHECK_LABEL_ID 0x13

static bool is_no_sd_scenario = false; // 是否处于无SD卡场景，控制SD卡检测任务的显示逻辑
static lv_obj_t *no_sd_label = NULL;   // No SD标签指针
static int exit_time_count = 0;// 退出计时器
lv_obj_t *label_sd_format = NULL;// 格式化状态标签指针
static bool is_sd_formatting = false;  // 是否正在格式化SD卡

static void sd_card_detect_task(lv_task_t *task)
{

    // 格式化中不检测SD卡状态，避免No SD标签错误显示
    if (is_sd_formatting)
    {
        return; 
    }
    // 1. 获取当前SD卡实际状态
    bool sd_inserted = media_sdcard_insert_check();
    printf("SD卡状态:%d\n", sd_inserted);
    // 2. 场景1：无SD卡（拔卡）→ 创建/显示No SD标签
    if (!sd_inserted)
    {
        is_no_sd_scenario = true;
        if (no_sd_label == NULL)
        {
            no_sd_label = lv_label_create(lv_scr_act(), NULL);
            if (no_sd_label == NULL)
            {
                printf("Failed to create No SD label!\n");
                return;
            }
            lv_label_set_text(no_sd_label, str_get(COMMON_LANG_NO_SD_ID));
            lv_obj_set_style_local_text_font(no_sd_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
            lv_obj_set_style_local_text_color(no_sd_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF3030));
            lv_obj_align(no_sd_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 160);
        }
    }
    else
    {
        is_no_sd_scenario = false;
        // 标签存在时才删除（避免空指针）
        if (no_sd_label != NULL)
        {
            lv_obj_del(no_sd_label);
            no_sd_label = NULL;
        }
    }
}

/* setting 图标 */
static void setting_icon_create(lv_obj_t *parent)
{
    lv_obj_t *setting_icon_obj = lv_img_create(parent, NULL);
    lv_obj_set_pos(setting_icon_obj, 54, 34);
	lv_obj_set_size(setting_icon_obj, 22, 20);
	lv_obj_set_id(setting_icon_obj, HOME_GEAR_OBJ_ID);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_GEAR_PNG);
	lv_obj_set_style_local_pattern_image(setting_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);

    lv_obj_t *Time_label = lv_label_create(parent, NULL);
    lv_obj_set_pos(Time_label, 87, 28);
	lv_obj_set_size(Time_label, 70, 31);
    lv_label_set_text(Time_label, "Init");
	lv_obj_align(setting_icon_obj, Time_label, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}

static void back_btn_up(lv_obj_t *obj)
{
    // 重置格式化状态标记
    is_sd_formatting = false;
    
    // 清理No SD标签
    if (no_sd_label != NULL)
    {
        lv_obj_del(no_sd_label);
        no_sd_label = NULL;
    }
    // 清理格式化相关标签
    if (label_sd_format != NULL)
    {
        lv_obj_del(label_sd_format);
        label_sd_format = NULL;
    }
    // 重置状态标记
    is_no_sd_scenario = false;
    exit_time_count = 0;
    
    goto_layout(pLAYOUT(home));
}

static void reset_system_btn_up(lv_obj_t *obj)
{
    uint8_t lang = user_data_get()->setting.language;
    user_data_reset();
    if(user_data_get()->setting.language != lang)
    {
        /***** 设置字库 *****/
        extern void lv_ft_font_set_type(int type);
        lv_ft_font_set_type(user_data_get()->setting.language);
        
        extern void lv_font_afresh_init(void);
        lv_font_afresh_init();
    }
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
    lv_obj_set_style_local_pattern_recolor(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
    lv_obj_set_style_local_pattern_recolor_opa(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50); // 按下叠加50%黑色
    static obj_click_data btn_data = obj_click_data_up_create(back_btn_up);
    obj_click_event_listen(back_icon_obj, &btn_data);
}
  
static void sd_format_state_display_task(lv_task_t *task_t)
{
    if (task_t == NULL)
    {
        printf("Error: task is NULL!\n");
        return;
    }

    lv_obj_t *state_label = task_t->user_data;
    if (state_label == NULL)
    {
        printf("Error: state_label is NULL!\n");
        lv_task_del(task_t);
        return;
    }

    // No SD场景：标签常驻，不做清理
    if (is_no_sd_scenario)
    {
        return;
    }

    // 格式化完成/失败（media_format_sd_state()返回false）
    if (media_format_sd_state() == false)
    {
        // 格式化完成，重置状态标记
        is_sd_formatting = false;
        
        // 显示格式化成功提示（仅SD卡插入时）
        if (media_sdcard_insert_check() && exit_time_count > 3)
        {
            lv_label_set_text(label_sd_format, "");  // 清空加载动画
            lv_label_set_text(state_label, str_get(LAYOUT_SD_LANG_FORMAT_OK_ID));
            lv_obj_align(state_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 165);
            exit_time_count = 3;  // 倒计时3秒跳转
        }

        // 先判断exit_time_count>0再自减，避免负数导致死循环
        if (exit_time_count > 0)
        {
            exit_time_count--;
            if (exit_time_count <= 0)
            {
                // 清理所有资源后再跳转
                lv_obj_del(state_label);
                if (label_sd_format != NULL)
                {
                    lv_obj_del(label_sd_format);
                    label_sd_format = NULL;
                }
                lv_task_del(task_t);  // 删除任务
                goto_layout(pLAYOUT(home));
            }
        }
        else
        {
            // 无倒计时时直接清理资源
            lv_obj_del(state_label);
            if (label_sd_format != NULL)
            {
                lv_obj_del(label_sd_format);
                label_sd_format = NULL;
            }
            lv_task_del(task_t);
        }
    }
    // 格式化中（media_format_sd_state()返回true）
    else
    {
        static char loading_str[3][10] = {".", "..", "..."};
        lv_label_set_text(state_label, str_get(LAYOUT_SD_LANG_FORMATING_ID));
        lv_label_set_text(label_sd_format, loading_str[exit_time_count % 3]);
        lv_obj_align(label_sd_format, state_label, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
        exit_time_count++;

        // 防止exit_time_count溢出（避免整数溢出导致逻辑异常）
        if (exit_time_count > 1000)
        {
            exit_time_count = 0;
        }
    }
}

static void no_sd_label_display(lv_obj_t *center_cont)
{
    // 格式化中不显示No SD标签
    if (is_sd_formatting)
    {
        return;
    }
    
    if (!media_sdcard_insert_check())
    {
        is_no_sd_scenario = true;
        no_sd_label = lv_label_create(lv_scr_act(), NULL);
        if (no_sd_label == NULL)
        {
            printf("Failed to create No SD label!\n");
            return;
        }
        lv_label_set_text(no_sd_label, str_get(COMMON_LANG_NO_SD_ID));
        lv_obj_set_style_local_text_font(no_sd_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
        lv_obj_set_style_local_text_color(no_sd_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF5555));
        lv_obj_align(no_sd_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 160);
    }
    else
    {
        is_no_sd_scenario = false;
        if (no_sd_label != NULL)
        {
            lv_obj_del(no_sd_label);
            no_sd_label = NULL;
        }
    }
}

// 格式化按钮点击回调
static void format_sd_btn_up(lv_obj_t *obj)
{
    // 1. 正在格式化时禁止重复点击
    if (media_format_sd_state() || is_sd_formatting)
    {
        printf("Format is in progress, ignore click!\n");
        return;
    }

    // 2. 清理所有残留资源
    if (no_sd_label != NULL)
    {
        lv_obj_del(no_sd_label);
        no_sd_label = NULL;
    }
    if (label_sd_format != NULL)
    {
        lv_obj_del(label_sd_format);
        label_sd_format = NULL;
    }
    is_no_sd_scenario = false;
    exit_time_count = 0;

    
    if (media_sdcard_insert_check())
    {
        // 标记为格式化中
        is_sd_formatting = true;
        
        // SD卡已插入：创建格式化相关标签
        lv_obj_t *state_label = lv_label_create(lv_scr_act(), NULL);
        if (state_label == NULL)
        {
            printf("Failed to create format state label!\n");
            is_sd_formatting = false; // 失败时重置标记
            return;
        }

        // 加载动画标签（已在create_init_page中创建，这里只需重置）
        if (label_sd_format == NULL)
        {
            label_sd_format = lv_label_create(lv_scr_act(), NULL);
            if (label_sd_format == NULL)
            {
                printf("Failed to create loading label!\n");
                lv_obj_del(state_label);
                is_sd_formatting = false; // 失败时重置标记
                return;
            }
            lv_obj_set_style_local_text_font(label_sd_format, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
        }

        // 配置格式化中显示
        lv_label_set_text(state_label, str_get(LAYOUT_SD_LANG_FORMATING_ID));
        lv_obj_set_style_local_text_font(state_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
        lv_obj_align(state_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 160);
        lv_obj_align(label_sd_format, state_label, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

        // 关键：如果media_format_sd()是阻塞函数，建议用LVGL任务异步执行（避免界面卡死）
        // 这里用lv_task_once创建一次性任务执行格式化，不阻塞主线程
        media_format_sd();  // 执行格式化（阻塞操作放在子任务）

        exit_time_count = 3 + 1;  // 初始化倒计时（+1满足exit_time_count>3判断）
        lv_layout_task_create(sd_format_state_display_task, 200, LV_TASK_PRIO_LOWEST, state_label);
    }
    else
    {
        is_no_sd_scenario = true;
        no_sd_label = lv_label_create(lv_scr_act(), NULL);
        if (no_sd_label == NULL)
        {
            printf("Failed to create No SD label!\n");
            return;
        }

        lv_label_set_text(no_sd_label, str_get(COMMON_LANG_NO_SD_ID));
        lv_obj_set_style_local_text_font(no_sd_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
        lv_obj_set_style_local_text_color(no_sd_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF5555));
        lv_obj_align(no_sd_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 160);

        lv_layout_task_create(sd_format_state_display_task, 1000, LV_TASK_PRIO_LOWEST, no_sd_label);
    }
}


void create_init_page(lv_obj_t *center_cont)
{
    // 格式化按钮
    static obj_click_data format_sd_click_data = obj_click_data_up_create(format_sd_btn_up);
    custom_area area1 = {.x = 102, .y = 100, .w = 330, .h = 30};
    setting_right_btn_base_create(center_cont, area1.x, area1.y, area1.w, area1.h, 
                                  str_get(LAYOUT_SD_LANG_FORMAT_ID),
                                  NULL,
                                  &format_sd_click_data, 
                                  SETTING_format_SD_BIN_ID);

    static obj_click_data reset_system_click_data = obj_click_data_up_create(reset_system_btn_up);
    custom_area area2 = {.x = 102, .y = 167, .w = 330, .h = 50};
    setting_right_btn_base_create(center_cont, area2.x, area2.y, area2.w, area2.h, 
                                  str_get(COMMON_LANG_RESET_SYSTEM),
                                  NULL,
                                  &reset_system_click_data, 
                                  SETTING_format_SD_BIN_ID);
                                  
    label_sd_format = lv_label_create(center_cont, NULL);
    lv_obj_set_pos(label_sd_format, 0, 190);
    lv_obj_set_size(label_sd_format, 100, 30);
    lv_label_set_text(label_sd_format, "");
    lv_obj_set_style_local_text_font(label_sd_format, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
}

static void LAYOUT_ENTER_FUNC(init)
{
    lv_obj_t *parent = common_bg_display(lv_scr_act());
    lv_obj_t *center_cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(center_cont, 534, 294);  
    lv_obj_set_id(center_cont, 100);
    lv_obj_align(center_cont, parent, LV_ALIGN_CENTER, 0, 0);  
    lv_obj_set_style_local_bg_color(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x838383));
    lv_obj_set_style_local_radius(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_bg_opa(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
    back_btn_create(parent);
    setting_icon_create(parent);
    top_time_date_text_create(parent);
    create_init_page(center_cont);
    
    if (label_sd_format != NULL) {
        lv_obj_del(label_sd_format);
        label_sd_format = NULL;
    }

    no_sd_label_display(center_cont);
    exit_time_count = 0;

    lv_layout_task_create(sd_card_detect_task, 500, LV_TASK_PRIO_LOWEST, NULL);

}

// 布局退出时清理所有资源
static void LAYOUT_QUIT_FUNC(init)
{
    is_sd_formatting = false;
    is_no_sd_scenario = false;
    exit_time_count = 0;


}

CREATE_LAYOUT(init);
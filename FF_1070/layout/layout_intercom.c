#include "layout_define.h"

#define INTERCOM_LOCAL_ADDR_ID     1       // 本地地址ID
#define ADMIN_PASSWORD            "1234"   // 管理员密码
#define ADMIN_PASSWORD_LEN        4        // 管理员密码长度

// 对讲按钮模块ID枚举
typedef enum
{
    INTERCOM_HOME_BTN_ID,      // 主页按钮ID
    INTERCOM_GUARD_BTN_ID,     // 保安按钮ID
    INTERCOM_INTER_BTN_ID,     // 对讲按钮ID
    HOME_BACK_OBJ_ID,          // 返回按钮ID
    HOME_GEAR_OBJ_ID,          // 设置按钮ID
    HOME_IMG_OBJ_ID,           // 图片对象ID
    INTERCOM_TOTAL_BTN,        // 按钮总数
} intercom_btn_module;

// 密码验证状态枚举
typedef enum 
{
    PWD_STATE_UNVERIFIED,      // 未验证
    PWD_STATE_SUCCESS,         // 验证成功
    PWD_STATE_FAILED           // 验证失败
} pwd_verify_state_t;

// 对讲按钮功能枚举
typedef enum 
{
    INTERCOM_BTN_FUNC_CALL,    // 呼叫功能
    INTERCOM_BTN_FUNC_CONFIRM  // 确认功能
} intercom_btn_func_t;

static unsigned int kb_click_num = 0;                // 键盘点击次数/输入数值
static bool kb_input_room_flag = false;              // 输入标识：true-房号 false-密码
static unsigned int call_num = 0;                    // 内线拨打号码
static int time_out = 0;                             // 超时计数
static lv_task_t *call_out_wait_task = NULL;         // 外线呼叫等待任务句柄
static lv_task_t *wait_hook_on_task = NULL;          // 等待挂机任务句柄
static lv_obj_t *ta = NULL;                          // 呼叫号码输入框对象
static lv_obj_t *tt = NULL;                          // 房号/密码输入框对象
static bool textarea_input_select = false;           // 输入框选中标识：true-tt false-ta
static char call_num_save[20] = {0};                 // 保存呼叫号码

static void intercom_wait_hook_on_task_create(void);
static void intercom_call_out_result_task_create(const char *str);
static void intercom_call_out_wait_task_create(void);
static void intercom_room_out_result_task_create(const char *str);

// 通用输入框创建函数
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
    // 1. 创建输入框背景容器
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_pos(cont, x, y);
    lv_obj_set_size(cont, cont_w, cont_h);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_bg_color(cont,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,lv_color_hex(0x000000));
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 4);
    lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00CCFF));
    lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_border_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    
    // 2. 创建文本域
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
    
    // 3. 绑定输入框点击事件
    obj_click_event_listen(*textarea_obj, btn_data);
    
    // 4. 创建删除按钮
    lv_obj_t *cont_delete_btn = lv_obj_create(cont, NULL);
    lv_obj_set_pos(cont_delete_btn, delete_btn_x, 8);
    lv_obj_set_size(cont_delete_btn, 48, 48);
    static rom_bin_info cont_delete_info = rom_bin_info_get(ROM_UI_INTERCOM_DELETE_PNG);
    lv_obj_set_style_local_pattern_image(cont_delete_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &cont_delete_info);
	lv_obj_set_style_local_pattern_recolor(cont_delete_btn,LV_BTN_PART_MAIN,LV_STATE_DEFAULT,lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor(cont_delete_btn,LV_BTN_PART_MAIN,LV_STATE_PRESSED,lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor_opa(cont_delete_btn,LV_BTN_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_0);
	lv_obj_set_style_local_pattern_recolor_opa(cont_delete_btn,LV_BTN_PART_MAIN,LV_STATE_PRESSED,LV_OPA_50);
    obj_click_event_listen(cont_delete_btn, delete_click_data);
}

// 返回按钮点击事件
static void back_btn_up(lv_obj_t *obj)
{
    if(user_data_get()->room_no_flag == false)
    {
        goto_layout(pLAYOUT(home));
    }
    else
    {
        goto_layout(pLAYOUT(intercom));
    }
}

// 创建返回按钮
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

// 创建设置图标和房号显示
static void setting_icon_create(lv_obj_t *parent)
{
    // 1. 创建设置图标
    lv_obj_t *setting_icon_obj = lv_img_create(parent, NULL);
    lv_obj_set_pos(setting_icon_obj, 54, 34);
    lv_obj_set_size(setting_icon_obj, 22, 20);
    static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_GEAR_PNG);
    lv_obj_set_id(setting_icon_obj, HOME_GEAR_OBJ_ID);
    lv_obj_set_style_local_pattern_image(setting_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);

    // 2. 创建房号显示标签
    lv_obj_t *Time_label = lv_label_create(parent, NULL);
    lv_obj_set_pos(Time_label, 87, 28);
    lv_obj_set_size(Time_label, 70, 31);
    lv_obj_set_id(Time_label, INTERCOM_LOCAL_ADDR_ID);
    static char str[40] = {0};
    
    // 3. 根据设备类型显示不同文本
    if ((user_data_get()->device_id == GUARD_INTERCOM_NUMBER) || (user_data_get()->device_id == GUARD_2_INTERCOM_NUMBER))
    {
        sprintf(str, "%s : %s", str_get(LAYOUT_INTERCOM_LANG_ROOM_NO_ID),
        (user_data_get()->device_id == GUARD_INTERCOM_NUMBER) ? str_get(LAYOUT_INTERCOM_LANG_GUARD_1_ID) : str_get(LAYOUT_INTERCOM_LANG_GUARD_2_ID));
    }
    else
    {
        sprintf(str, "%s : %03d", str_get(LAYOUT_INTERCOM_LANG_ROOM_NO_ID), user_data_get()->device_id);
    }
    
    lv_label_set_text(Time_label, str);
    lv_obj_set_style_local_text_font(Time_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(22));
    lv_obj_set_style_local_text_color(Time_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_align(setting_icon_obj, Time_label, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}

// 创建呼叫号码标签
static void intercom_select_btn_create(lv_obj_t *parent)
{
    lv_obj_t *call_label = lv_label_create(parent, NULL);
    lv_obj_set_pos(call_label, 607, 179);
    lv_obj_set_size(call_label, 147, 29);
    lv_label_set_text_fmt(call_label, "%s :", str_get(LAYOUT_INTERCOM_LANG_CALL_NUMBER_ID));
    lv_obj_set_style_local_text_font(call_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_set_style_local_text_color(call_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
}

// 创建房号设置标签
static void set_no_select_btn_create(lv_obj_t *parent)
{
    lv_obj_t *call_label = lv_label_create(parent, NULL);
    lv_obj_set_pos(call_label, 607, 324);
    lv_obj_set_size(call_label, 101, 29);
    lv_label_set_text_fmt(call_label, "%s :", str_get(COMMON_LANG_SET_NO));
    lv_obj_set_style_local_text_font(call_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    lv_obj_set_style_local_text_color(call_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
}

/************************** 输入框事件处理 **************************/
// 呼叫号码输入框点击事件
static void intercom_ta_up(lv_obj_t *obj)
{
    textarea_input_select = false;
    lv_textarea_set_text(ta, "");
    lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
}

// 房号/密码输入框点击事件
static void intercom_tt_up(lv_obj_t *obj)
{
    textarea_input_select = true;
    lv_textarea_set_text(tt, "");
    lv_obj_set_style_local_value_opa(tt, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
}

// 呼叫号码删除按钮点击事件
static void delete_button_click(lv_obj_t *obj)
{
    lv_textarea_del_char(ta);
}

// 房号/密码删除按钮点击事件
static void room_delete_button_click(lv_obj_t *obj)
{
    lv_textarea_del_char(tt);
}

// 创建呼叫号码输入框
static void intercom_number_textarea_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(intercom_ta_up);
    static obj_click_data delete_click_data = obj_click_data_up_create(delete_button_click);
    
    intercom_textarea_create_common(parent, 
                                   &ta,                // 指向call输入框的指针
                                   607, 226,           // 位置x,y
                                   280, 64,            // 容器宽高
                                   280, 64,            // 文本域宽高
                                   218,                // 删除按钮x坐标
                                   3,                  // 最大输入长度
                                   &btn_data,          // 点击回调
                                   &delete_click_data); // 删除回调
}

// 创建房号/密码输入框
static void intercom_room_textarea_create(lv_obj_t *parent)
{
    static obj_click_data btn_data2 = obj_click_data_up_create(intercom_tt_up);
    static obj_click_data delete_click_data = obj_click_data_up_create(room_delete_button_click);
    
    intercom_textarea_create_common(parent, 
                                   &tt,                // 指向room输入框的指针
                                   607, 371,           // 位置x,y
                                   300, 64,            // 容器宽高
                                   298, 62,            // 文本域宽高
                                   236,                // 删除按钮x坐标
                                   5,                  // 最大输入长度
                                   &btn_data2,         // 点击回调
                                   &delete_click_data); // 删除回调
}

/************************** 键盘相关函数 **************************/
// 创建单个键盘按钮
static lv_obj_t *create_keyboard_button(lv_obj_t *parent, int x, int y, const char *text, obj_click_data *click_data, bool is_icon_button)
{
    lv_obj_t *btn = lv_btn_create(parent, NULL);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, 93, 93);
    lv_obj_set_style_local_bg_opa(btn,LV_BTN_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_0);
    lv_obj_set_style_local_bg_color(btn,LV_BTN_PART_MAIN,LV_STATE_DEFAULT,lv_color_hex(0x000000));
    // 按钮默认样式
    static rom_bin_info info1 = rom_bin_info_get(ROM_UI_INTERCOM_CIRCLE_PNG);
    lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
	// 1. 图标颜色叠加：默认和按下都叠加黑色（0x000000）
	lv_obj_set_style_local_pattern_recolor(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor(btn,LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));

	// 2. 叠加透明度：默认低透明（图标显原始色），按下高透明（图标变深色）
	lv_obj_set_style_local_pattern_recolor_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);	
	lv_obj_set_style_local_pattern_recolor_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50); 

    
    // 非图标按钮设置文本
    if (!is_icon_button && text != NULL)
    {
        lv_obj_t *label = lv_label_create(btn, NULL);
        lv_label_set_text(label, text);
        lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));
        lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
		lv_obj_set_style_local_text_color(label,LV_LABEL_PART_MAIN,LV_STATE_PRESSED,lv_color_hex(0xCCCCCC));
        lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
    }

    // 绑定点击事件
    if (click_data != NULL)
    {
        obj_click_event_listen(btn, click_data);
    }

    return btn;
}

// 键盘数字按钮点击事件
static void keyboard_button_click(lv_obj_t *obj)
{
    // 有等待任务时不响应
    if ((call_out_wait_task != NULL) || (wait_hook_on_task != NULL))
        return;

    // 获取按钮文本
    lv_obj_t *label_obj = lv_obj_get_child(obj, NULL);
    if (label_obj == NULL) {
        printf("Keyboard button is icon type, no text label\n");
        return;
    }

    const char *txt = lv_label_get_text(lv_obj_get_child(obj, NULL));
    if(txt == NULL){
        return;
    }
    
    printf("Keyboard button pressed: %s\n", txt);
    
    // 根据选中的输入框添加字符
    if (textarea_input_select)
    {
        lv_textarea_add_char(tt, txt[0]);
        lv_obj_set_style_local_value_opa(tt, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
    }
    else
    {
        lv_textarea_add_char(ta, txt[0]);
        lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
    }
}

// 密码/房号确认按钮点击事件
static void intercom_ok_button_click(lv_obj_t *obj)
{
    static char str[20] = {0};
    user_data_info *user_data = user_data_get(); 
    
    // 初始化默认密码
    if (user_data->password == 0) { 
        user_data->password = 1234; 
        printf("默认密码已初始化:1234\n");
    }
    
    // 获取输入的数值
    kb_click_num = atoi(lv_textarea_get_text(tt));
    printf("kb_click_num: %d\n", kb_click_num);
    
    // 密码验证阶段
    if(!kb_input_room_flag)
    {
        if (kb_click_num == user_data_get()->password)
        {
            printf("密码验证成功: %d\n", user_data->password);
            lv_textarea_set_text(tt, "");
            intercom_room_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_INPUT_ROOM_NO_ID));
            kb_input_room_flag = true;
            lv_textarea_set_max_length(tt, 5);
            return;
        }
        else
        {
            printf("密码验证失败: %d\n", user_data->password);
            lv_textarea_set_text(tt, "");
            intercom_room_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_ERROR_ID));
            return;
        }
    }
    // 房号设置阶段
    else 
    {
        if (kb_click_num == user_data_get()->device_id)
        {
            intercom_room_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_LOCAL_ID));
        }
        else if (kb_click_num > 0 && kb_click_num < 9999)
        {
            // 更新设备ID
            user_data_get()->device_id = kb_click_num;
            intercom_room_out_result_task_create(str_get(LAYOUT_ROOM_ID_LANG_MODIFY_SUCCESSFUL_ID));

            // 更新房号显示
            lv_obj_t *local_addr_obj = lv_obj_get_child_form_id(lv_scr_act(), INTERCOM_LOCAL_ADDR_ID);
            if (local_addr_obj != NULL) {
                sprintf(str, "%s : %03d", str_get(LAYOUT_INTERCOM_LANG_ROOM_NO_ID), user_data_get()->device_id);
                lv_label_set_text(local_addr_obj, str);
                printf("kb_click_num setting sucess,user_data_get()->device_id: %d\n", user_data_get()->device_id);
            }
        }
        else 
        {
            intercom_room_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_ERROR_ID));
        }
        
        lv_textarea_set_text(tt, "");
        kb_input_room_flag = false;
    }
}   

// 对讲按钮点击事件
static void intercom_button_click(lv_obj_t *obj)
{
    // 有等待任务时不响应
    if (call_out_wait_task != NULL || wait_hook_on_task != NULL)
        return;
    
    // 选中房号输入框时执行确认逻辑
    if (textarea_input_select) {
        intercom_ok_button_click(obj);
        return;
    }

    // 获取呼叫号码
    call_num = atoi(lv_textarea_get_text(ta));
    memset(call_num_save, 0, sizeof(call_num_save));
    sprintf(call_num_save, "%s", lv_textarea_get_text(ta));
    printf("Call number: %d\n", call_num);

    // 验证呼叫号码
    if (call_num == user_data_get()->device_id)
    {
        intercom_room_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_LOCAL_ID));
    }
    else if (call_num > 0 && call_num < 257)
    {
        // 手柄已拿起
        if (hook_state_get() == true)
        {
            intercom_call_out_wait_task_create();
        }
        // 等待拿起手柄
        else
        {
            intercom_wait_hook_on_task_create();
        }
    }
    else
    {
        intercom_room_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_ERROR_ID));
    }
    
    lv_textarea_set_text(ta, "");
}

// 呼叫保安按钮点击事件
static void guard_button_click(lv_obj_t *obj)
{
    // 有等待任务时不响应
    if (call_out_wait_task != NULL || wait_hook_on_task != NULL)
        return;

    // 设置保安号码
    call_num = GUARD_INTERCOM_NUMBER;
    memset(call_num_save, 0, sizeof(call_num_save));
    sprintf(call_num_save, "%s", str_get(LAYOUT_INTERCOM_LANG_GUARD_ID));
    printf("Call guard: %d\n", call_num);

    // 验证呼叫号码
    if (call_num == user_data_get()->device_id)
    {
        intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_LOCAL_ID));
    }
    else
    {
        // 手柄已拿起
        if (hook_state_get() == true)
        {
            intercom_call_out_wait_task_create();
        }
        // 等待拿起手柄
        else
        {
            intercom_wait_hook_on_task_create();
        }
    }
    
    lv_textarea_set_text(ta, "");
}

// 创建数字键盘
static void intercom_number_kb_create(lv_obj_t *parent)
{
    // 1. 创建键盘容器
    lv_obj_t *kb_cont = lv_cont_create(parent, NULL);
    lv_obj_set_pos(kb_cont, 150, 135);
    lv_obj_set_size(kb_cont, 355, 395);
    lv_obj_set_style_local_bg_opa(kb_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);

    // 2. 定义按钮点击回调
    static obj_click_data kb_click_data = obj_click_data_up_create(keyboard_button_click);
    static obj_click_data intercom_click_data = obj_click_data_up_create(intercom_button_click);
    static obj_click_data guard_click_data = obj_click_data_up_create(guard_button_click);

    // 3. 创建数字按钮 - 第一行
    create_keyboard_button(kb_cont, 0, 0, "1", &kb_click_data, false);
    create_keyboard_button(kb_cont, 128, 0, "2", &kb_click_data, false);
    create_keyboard_button(kb_cont, 256, 0, "3", &kb_click_data, false);

    // 4. 创建数字按钮 - 第二行
    create_keyboard_button(kb_cont, 0, 98, "4", &kb_click_data, false);
    create_keyboard_button(kb_cont, 128, 98, "5", &kb_click_data, false);
    create_keyboard_button(kb_cont, 256, 98, "6", &kb_click_data, false);

    // 5. 创建数字按钮 - 第三行
    create_keyboard_button(kb_cont, 0, 196, "7", &kb_click_data, false);
    create_keyboard_button(kb_cont, 128, 196, "8", &kb_click_data, false);
    create_keyboard_button(kb_cont, 256, 196, "9", &kb_click_data, false);

    // 6. 创建功能按钮 - 第四行
    // 对讲按钮
    lv_obj_t *intercom_btn = create_keyboard_button(kb_cont, 0, 294, "", &intercom_click_data, true);
    static rom_bin_info intercom_info = rom_bin_info_get(ROM_UI_INTERCOM_CALL_PNG);
    lv_obj_set_style_local_pattern_image(intercom_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &intercom_info);
    lv_obj_set_style_local_pattern_recolor(intercom_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0DFF00));
    lv_obj_set_style_local_pattern_recolor_opa(intercom_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    
    // 0按钮
    create_keyboard_button(kb_cont, 128, 294, "0", &kb_click_data, false);

    // 管理员/保安按钮
    if (user_data_get()->room_no_flag == false)
    {
        lv_obj_t *admin_btn = create_keyboard_button(kb_cont, 256, 294, "", &guard_click_data, true);
        static rom_bin_info admin_info = rom_bin_info_get(ROM_UI_INTERCOM_GUARD_PNG);
        lv_obj_set_style_local_pattern_image(admin_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &admin_info);
        lv_obj_set_style_local_pattern_recolor(admin_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF0000));
        lv_obj_set_style_local_pattern_recolor_opa(admin_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
        obj_click_event_listen(admin_btn, &guard_click_data);
    }
}

/************************** 任务相关函数 **************************/
// 设置呼叫结果提示文本
static void intercom_call_out_result_task_create(const char *str)
{
    lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, str);
    lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
}

// 设置房号/密码操作结果提示文本
static void intercom_room_out_result_task_create(const char *str)
{
    lv_obj_set_style_local_value_str(tt, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, str);
    lv_obj_set_style_local_value_opa(tt, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
}

// 外线呼叫等待任务处理函数
static void intercom_call_out_wait_task(lv_task_t *task_t)
{
    // 超时处理
    if (--time_out < 20)
    {
        intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_NO_RESPONSE_ID));
        lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
        intercom_state_set(INTERCOM_STATE_IDLE);
    }
    
    // 任务结束
    if (--time_out < 0)
    {
        lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, "");
        lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
        lv_task_del(task_t);
        call_out_wait_task = NULL;
    }

    // 设备忙处理
    if (intercom_unit_busy_state_get())
    {
        intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_UNIT_BUSY_ID));
        lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
        lv_task_del(task_t);
        call_out_wait_task = NULL;
        intercom_state_set(INTERCOM_STATE_IDLE);
    }

    // 线路忙处理
    if (intercom_line_busy_state_get())
    {
        intercom_call_out_result_task_create(str_get(LAYOUT_INTERCOM_LANG_BUS_BUSY_ID));
        lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
        lv_task_del(task_t);
        call_out_wait_task = NULL;
        intercom_state_set(INTERCOM_STATE_IDLE);
    }
}

// 创建外线呼叫等待任务
static void intercom_call_out_wait_task_create(void)
{
    // 设置呼叫中提示
    lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, str_get(LAYOUT_INTERCOM_LANG_CALLING_ID));
    lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
    time_out = 100;

    // 重置忙状态
    extern void intercom_busy_state_reset(void);
    intercom_busy_state_reset();

    // 创建任务
    call_out_wait_task = lv_layout_task_create(intercom_call_out_wait_task, 100, LV_TASK_PRIO_LOW, NULL);
    intercom_number_set(call_num);
    intercom_state_set(INTERCOM_STATE_CALL);
}

// 等待挂机任务处理函数
static void intercom_wait_hook_on_task(lv_task_t *task_t)
{
    // 超时结束任务
    if (--time_out < 0)
    {
        lv_task_del(task_t);
        wait_hook_on_task = NULL;
        lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
        return;
    }
    
    // 检测到手柄拿起
    if (hook_state_get() == true)
    {
        lv_task_del(task_t);
        wait_hook_on_task = NULL;
        intercom_call_out_wait_task_create();
        return;
    }
    
    // 闪烁提示拿起手柄
    static bool hidden_flag = false;
    hidden_flag = !hidden_flag;
    printf("Waiting for handset: %s\n", lv_textarea_get_text(ta));
    lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, hidden_flag ? call_num_save : str_get(LAYOUT_INTERCOM_LANG_TAKE_HANDSET_ID));
    lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
}

// 创建等待挂机任务
static void intercom_wait_hook_on_task_create(void)
{
    time_out = 60;
    lv_obj_set_style_local_value_str(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, str_get(LAYOUT_INTERCOM_LANG_TAKE_HANDSET_ID));
    lv_obj_set_style_local_value_opa(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
    wait_hook_on_task = lv_layout_task_create(intercom_wait_hook_on_task, 500, LV_TASK_PRIO_LOW, NULL);
}

/************************** 布局生命周期函数 **************************/
// 进入对讲布局
static void LAYOUT_ENTER_FUNC(intercom)
{
    lv_obj_t *parent = common_bg_display(lv_scr_act());
    textarea_input_select = false;
    
    // 创建UI元素
    back_btn_create(parent);                  // 返回按钮
    setting_icon_create(parent);              // 设置图标和房号显示
    intercom_select_btn_create(parent);       // 呼叫号码标签
    set_no_select_btn_create(parent);         // 房号设置标签
    intercom_number_kb_create(parent);        // 数字键盘
    intercom_number_textarea_create(parent);  // 呼叫号码输入框
    intercom_room_textarea_create(parent);    // 房号/密码输入框
    top_time_date_text_create(parent);        // 顶部时间日期
}

// 退出对讲布局
static void LAYOUT_QUIT_FUNC(intercom)
{
    // 重置房号标识
    user_data_get()->room_no_flag = false;
    user_data_save();
    
    // 停止对讲任务
    if (call_out_wait_task != NULL &&
        cur_layout_get() != pLAYOUT(intercom_in) &&
        cur_layout_get() != pLAYOUT(intercom_out) &&
        cur_layout_get() != pLAYOUT(intercom_talk))
        intercom_state_set(INTERCOM_STATE_HUNG_UP);

    // 清空任务句柄
    call_out_wait_task = NULL;
    wait_hook_on_task = NULL;

    // 重置输入标识
    kb_input_room_flag = false;
}

// 创建对讲布局
CREATE_LAYOUT(intercom);
#include "layout_define.h"

// 全局临时变量
static struct tm temp_tm;             // 临时时间结构体（时/分存储）
static struct date temp_date;         // 临时日期结构体（年/月/日存储）
static bool time_change_flag = false; // 时间修改标记（退出时判断是否保存）
// 时间显示字符串缓冲区
static char str_year[5] = {0};
static char str_month[5] = {0};
static char str_day[5] = {0};
static char str_hour[5] = {0};
static char str_min[5] = {0};

//  UI侧边功能按钮（Home/System/Time/Sound）-
typedef enum
{
    SETTING_TIME_HOME_BTN_ID,
    SETTING_TIME_SYSTEM_BTN_ID,
    SETTING_TIME_TIME_BTN_ID,
    SETTING_TIME_SOUND_BTN_ID,
    SETTING_TIME_TOTAL_BTN,
} setting_btn_module;

static custom_area setting_btn_area[SETTING_TIME_TOTAL_BTN] =
    {
        {24, 12, 54, 54},
        {30, 70, 155, 170},
        {30, 240, 155, 170},
        {28, 410, 157, 170},
};

#define SETTING_TIME_YEAR_ID 0x05
#define SETTING_TIME_MONTH_ID 0x06
#define SETTING_TIME_DAY_ID 0x07
#define SETTING_TIME_HOUR_ID 0x08
#define SETTING_TIME_MINUTE_ID 0x09
// 日历切换开关ID
#define SETTING_TIME_CALENDAR_SW_ID 0x10
// Home按钮：跳转首页
static void setting_time_home_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(home));
}
static void setting_time_home_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_home_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_HOME_PNG);
    common_img_btn_create(parent, setting_btn_area[SETTING_TIME_HOME_BTN_ID], NULL, &btn_data, &info);
}

// System按钮：跳转系统设置
static void setting_time_system_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting));
}
static void setting_time_system_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_system_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_SYSTEM_PNG);
    lv_obj_t *btn = photo_and_video_btn_create(parent, setting_btn_area[SETTING_TIME_SYSTEM_BTN_ID], str_get(LAYOUT_SETTING_LANG_SYSTEM_ID), &btn_data, &info, true, false);
    lv_obj_set_id(btn, SETTING_TIME_SYSTEM_BTN_ID);
}

// Time按钮：跳转当前时间设置（自身）
static void setting_time_time_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting_time));
}
static void setting_time_time_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_time_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_TIME_RED_PNG);
    lv_obj_t *btn = photo_and_video_btn_create(parent, setting_btn_area[SETTING_TIME_TIME_BTN_ID], str_get(LAYOUT_SETTING_LANG_TIME_ID), &btn_data, &info, true, true);
    lv_obj_set_id(btn, SETTING_TIME_TIME_BTN_ID);
}

// Sound按钮：跳转声音设置
static void setting_time_sound_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(setting_sound));
}
static void setting_time_sound_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(setting_time_sound_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_SETTING_SOUND_PNG);
    lv_obj_t *btn = photo_and_video_btn_create(parent, setting_btn_area[SETTING_TIME_SOUND_BTN_ID], str_get(LAYOUT_SETTING_LANG_SOUND_ID), &btn_data, &info, false, false);
    lv_obj_set_id(btn, SETTING_TIME_SOUND_BTN_ID);
}
// 开关回调：处理公历/波斯历切换逻辑（状态变化时触发）
static void setting_time_calendar_sw_value_change(lv_obj_t *obj, lv_event_t event)
{
    if (event != LV_EVENT_VALUE_CHANGED)
        return;

    // 获取开关当前状态（on=公历，off=波斯历）
    bool is_gregorian = lv_switch_get_state(obj);
    // 更新用户数据中的日历模式
    user_data_get()->setting.calendar = is_gregorian ? 1 : 0;
    time_change_flag = true; // 标记时间已修改

    // 保存当前日期，避免重复转换导致数据错误
    struct date old_date = temp_date;
    // 日历转换：根据开关状态切换公历/波斯历
    if (is_gregorian)
    {
        // 波斯历 → 公历
        temp_date = jalali2gregorian(old_date);
    }
    else
    {
        // 公历 → 波斯历
        temp_date = gregorian2jalali(old_date);
    }

    //  刷新年/月/日显示（确保切换后文本同步）
    // 年显示更新
    sprintf(str_year, "%04d", temp_date.year);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_YEAR_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_year);

    // 月显示更新
    sprintf(str_month, "%02d", temp_date.month);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_MONTH_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_month);

    // 日显示更新（需校验当月天数，避免转换后日期超出范围）
    int mon_last_day = is_gregorian ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day; // 日期修正为当月最后一天
    }
    sprintf(str_day, "%02d", temp_date.day);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
}

// 创建日历切换开关（调用你已封装的setting_sub_btn_base_create，type=3对应开关）
static bool setting_time_calendar_sw_create(void)
{
    //  绑定开关回调（状态变化时触发切换逻辑）
    static obj_click_data sw_click_data = obj_click_data_anything_create(setting_time_calendar_sw_value_change);

    // 确定开关初始状态（根据当前日历模式：公历=on，波斯历=off）
    bool init_is_gregorian = (user_data_get()->setting.calendar == 1);

    // 调用已封装的开关创建函数（type=3触发setting_btn_sub_switch_create）
    lv_obj_t *sw_btn = setting_sub_btn_base_create(
        NULL,                             // 父对象：NULL→默认屏幕
        241,                              // x坐标：与时间按钮统一
        84 + (50 * 0),                    // y坐标：第一位
        680,                              // 宽度：与时间按钮一致
        40,                               // 高度：与时间按钮一致
        str_get(COMMON_LANG_CALENDAR_ID), // 开关文本："日历"
        &sw_click_data,                   // 回调函数：处理切换逻辑
        init_is_gregorian,                // 初始状态：公历=开，波斯历=关
        3                                 // type=3：对应你封装的开关类型
    );

    // 设置开关ID
    if (sw_btn != NULL)
    {
        lv_obj_set_id(sw_btn, SETTING_TIME_CALENDAR_SW_ID);
    }
    return (sw_btn != NULL);
}

// 年/月/日/时/分 双箭头控制逻辑（左减右加）
// ------------------------------ 年按钮逻辑 ------------------------------
// 左箭头：年份减1（公历2022-2037，波斯历1400-1415循环）
static void setting_time_year_left_btn_up(lv_obj_t *obj)
{
    if (user_data_get()->setting.calendar == 1) // 公历
    {
        if (--temp_date.year < 2022)
            temp_date.year = 2037;
    }
    else // 波斯历
    {
        if (--temp_date.year < 1400)
            temp_date.year = 1415;
    }
    time_change_flag = true; // 标记时间已修改
    // 更新显示文本
    sprintf(str_year, "%04d", temp_date.year);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_YEAR_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_year);
    // 校验日期合法性（年变化可能影响当月天数）
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}

// 右箭头：年份加1（同左箭头范围）
static void setting_time_year_btn_up(lv_obj_t *obj)
{
    if (user_data_get()->setting.calendar == 1) // 公历
    {
        if (++temp_date.year > 2037)
            temp_date.year = 2022;
    }
    else // 波斯历
    {
        if (++temp_date.year > 1415)
            temp_date.year = 1400;
    }
    time_change_flag = true;
    sprintf(str_year, "%04d", temp_date.year);

    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_YEAR_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_year);
    // 校验日期合法性
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}

// 创建年按钮（传入当前年份文本）
static bool setting_time_year_btn_create(void)
{
    static obj_click_data right_data = obj_click_data_up_create(setting_time_year_btn_up);     // 右箭头（加）
    static obj_click_data left_data = obj_click_data_up_create(setting_time_year_left_btn_up); // 左箭头（减）
    setting_double_arrow_btn_create(
        NULL,
        241, 84 + (60 * 2), 680, 40,
        str_get(LAYOUT_CALENDAR_LANG_YEAR_ID),
        str_year, // 初始显示当前年份
        &right_data,
        &left_data,
        SETTING_TIME_YEAR_ID);
    return true;
}

// ------------------------------ 月按钮逻辑 ------------------------------
// 左箭头：月份减1（1-12循环）
static void setting_time_month_letf_btn_up(lv_obj_t *obj)
{
    if (--temp_date.month < 1)
        temp_date.month = 12;
    time_change_flag = true;
    // 更新显示
    sprintf(str_month, "%02d", temp_date.month);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_MONTH_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_month);
    // 校验日期合法性（月变化影响当月天数）
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}

// 右箭头：月份加1（1-12循环）
static void setting_time_month_btn_up(lv_obj_t *obj)
{
    if (++temp_date.month > 12)
        temp_date.month = 1;
    time_change_flag = true;
    sprintf(str_month, "%02d", temp_date.month);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_MONTH_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_month);
    // 校验日期合法性
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}

// 创建月按钮（传入当前月份文本）
static bool setting_time_month_btn_create(void)
{
    static obj_click_data right_data = obj_click_data_up_create(setting_time_month_btn_up);
    static obj_click_data left_data = obj_click_data_up_create(setting_time_month_letf_btn_up);
    setting_double_arrow_btn_create(
        NULL,
        241, 84 + (60 * 3), 680, 40,
        str_get(LAYOUT_CALENDAR_LANG_MONTH_ID),
        str_month, // 初始显示当前月份
        &right_data,
        &left_data,
        SETTING_TIME_MONTH_ID);
    return true;
}

// ------------------------------ 日按钮逻辑 ------------------------------
// 左箭头：日期减1（1-当月最后一天循环）
static void setting_time_day_left_btn_up(lv_obj_t *obj)
{
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);

    if (--temp_date.day < 1)
        temp_date.day = mon_last_day;

    time_change_flag = true;
    sprintf(str_day, "%02d", temp_date.day);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
}

// 右箭头：日期加1（1-当月最后一天循环）
static void setting_time_day_btn_up(lv_obj_t *obj)
{
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);

    if (++temp_date.day > mon_last_day)
        temp_date.day = 1;

    time_change_flag = true;
    sprintf(str_day, "%02d", temp_date.day);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
}

// 创建日按钮（传入当前日期文本）
static bool setting_time_day_btn_create(void)
{
    static obj_click_data right_data = obj_click_data_up_create(setting_time_day_btn_up);
    static obj_click_data left_data = obj_click_data_up_create(setting_time_day_left_btn_up);
    setting_double_arrow_btn_create(
        NULL,
        241, 84 + (60 * 4), 680, 40,
        str_get(LAYOUT_CALENDAR_LANG_DAY_ID),
        str_day, // 初始显示当前日期
        &right_data,
        &left_data,
        SETTING_TIME_DAY_ID);
    return true;
}

// ------------------------------ 时按钮逻辑 ------------------------------
// 左箭头：小时减1（0-23循环）
static void setting_time_hour_left_btn_up(lv_obj_t *obj)
{
    if (--temp_tm.tm_hour < 0)
        temp_tm.tm_hour = 23;
    time_change_flag = true;
    sprintf(str_hour, "%02d", temp_tm.tm_hour);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_HOUR_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_hour);
}

// 右箭头：小时加1（0-23循环）
static void setting_time_hour_btn_up(lv_obj_t *obj)
{
    if (++temp_tm.tm_hour > 23)
        temp_tm.tm_hour = 0;
    time_change_flag = true;
    sprintf(str_hour, "%02d", temp_tm.tm_hour);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_HOUR_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_hour);
}

// 创建时按钮（传入当前小时文本）
static bool setting_time_hour_btn_create(void)
{
    static obj_click_data right_data = obj_click_data_up_create(setting_time_hour_btn_up);
    static obj_click_data left_data = obj_click_data_up_create(setting_time_hour_left_btn_up);
    setting_double_arrow_btn_create(
        NULL,
        241, 84 + (60 * 5), 680, 40,
        str_get(LAYOUT_CALENDAR_LANG_HOUR_ID),
        str_hour, // 初始显示当前小时
        &right_data,
        &left_data,
        SETTING_TIME_HOUR_ID);
    return true;
}

// ------------------------------ 分按钮逻辑 ------------------------------
// 左箭头：分钟减1（0-59循环）
static void setting_time_minute_left_btn_up(lv_obj_t *obj)
{
    if (--temp_tm.tm_min < 0)
        temp_tm.tm_min = 59;
    time_change_flag = true;
    sprintf(str_min, "%02d", temp_tm.tm_min);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_MINUTE_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_min);
}

// 右箭头：分钟加1（0-59循环）
static void setting_time_minute_btn_up(lv_obj_t *obj)
{
    if (++temp_tm.tm_min > 59)
        temp_tm.tm_min = 0;
    time_change_flag = true;
    sprintf(str_min, "%02d", temp_tm.tm_min);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(lv_scr_act(), SETTING_TIME_MINUTE_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_min);
}

// 创建分按钮（传入当前分钟文本）
static bool setting_time_minute_btn_create(void)
{
    static obj_click_data right_data = obj_click_data_up_create(setting_time_minute_btn_up);
    static obj_click_data left_data = obj_click_data_up_create(setting_time_minute_left_btn_up);
    setting_double_arrow_btn_create(
        NULL,
        241, 84 + (60 * 6), 680, 40,
        str_get(LAYOUT_CALENDAR_LANG_MIN_ID),
        str_min, // 初始显示当前分钟
        &right_data,
        &left_data,
        SETTING_TIME_MINUTE_ID);
    return true;
}

// 布局进入：初始化时间数据+创建所有按钮
static void LAYOUT_ENTER_FUNC(setting_time)
{
    printf("============================enter_setting_time\n");
    lv_obj_t *parent = common_bg_display(lv_scr_act());
    // 初始化：读取当前系统时间，填充临时变量与显示字符串
    time_change_flag = false; // 重置修改标记
    user_time_read(&temp_tm); // 读取当前时间到temp_tm
    // 同步日期到temp_date
    temp_date.year = temp_tm.tm_year;
    temp_date.month = temp_tm.tm_mon;
    temp_date.day = temp_tm.tm_mday;
    // 时间合法性校验（确保年份不小于2022）
    if (temp_date.year < 2022)
    {
        temp_date.year = 2022;
        temp_date.month = 1;
        temp_date.day = 1;
        temp_tm.tm_year = 2022;
        temp_tm.tm_mon = 1;
        temp_tm.tm_mday = 1;
    }
    // 若当前是波斯历，转换为波斯历日期
    if (user_data_get()->setting.calendar == 0)
    {
        temp_date = gregorian2jalali(temp_date);
    }
    // 填充显示字符串
    sprintf(str_year, "%04d", temp_date.year);
    sprintf(str_month, "%02d", temp_date.month);
    sprintf(str_day, "%02d", temp_date.day);
    sprintf(str_hour, "%02d", temp_tm.tm_hour);
    sprintf(str_min, "%02d", temp_tm.tm_min);

    // 创建侧边功能按钮
    setting_time_home_btn_create(parent);
    setting_time_system_btn_create(parent);
    setting_time_time_btn_create(parent);
    setting_time_sound_btn_create(parent);
    // 创建时间设置按钮（传入初始化后的显示文本）
    setting_time_calendar_sw_create();
    setting_time_year_btn_create();
    setting_time_month_btn_create();
    setting_time_day_btn_create();
    setting_time_hour_btn_create();
    setting_time_minute_btn_create();
}

// 布局退出：保存修改后的时间
static void LAYOUT_QUIT_FUNC(setting_time)
{
    if (time_change_flag == true) // 仅当时间被修改时执行保存
    {
        // 若当前是波斯历，先转换为公历再保存
        if (user_data_get()->setting.calendar == 0)
        {
            temp_date = jalali2gregorian(temp_date);
        }
        // 同步临时日期到temp_tm
        temp_tm.tm_year = temp_date.year;
        temp_tm.tm_mon = temp_date.month;
        temp_tm.tm_mday = temp_date.day;
        temp_tm.tm_sec = 0; // 秒重置为0
        // 关闭待机定时器->设置新时间->重启定时器
        standby_timer_close();
        user_time_set(&temp_tm);
        // 二次校验时间（防止异常）
        user_time_read(&temp_tm);
        if (temp_tm.tm_year < 2022)
        {
            temp_tm.tm_year = 2022;
            user_time_set(&temp_tm);
        }
        standby_timer_restart(true);
        // 保存用户数据（日历/时间设置）
        user_data_save();
        // 更新RTC校准时间戳
        extern unsigned long long calibrate_rtc_timestamp;
        calibrate_rtc_timestamp = user_timestamp_get();
    }
}

CREATE_LAYOUT(setting_time);
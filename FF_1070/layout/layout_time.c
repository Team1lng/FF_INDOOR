#include "layout_define.h"
#include "lv_msg_event.h"
#include "lvgl/lvgl.h"
#include "lvgl/lv_obj.h"
#include "lvgl/lv_label.h"
#include "language.h"

#define wxj

typedef enum 
{
	TIME_YEAR_BTN_ID,
	TIME_MONTH_BTN_ID,
	TIME_DAY_BTN_ID,
	TIME_HOUR_BTN_ID,
	TIME_MIN_BTN_ID,
    HOME_GEAR_OBJ_ID,
    HOME_BACK_OBJ_ID,
	TIME_TOTAL_BTN,
}time_btn_module;

// static custom_area time_btn_area[TIME_TOTAL_BTN] = 
// {
// 	{319, 196, 55, 31},
// 	{319, 252, 79, 31},
// 	{319, 309, 48, 31},
// 	{319, 366, 60, 31},
// 	{319, 423, 85, 31},
// };

//#define TIME_CALENDAR_PERSIAN_BTN_ID 0  //波斯日历
#define TIME_CALENDAR_WEATERN_BTN_ID 1  //西方日历

static char str_year[5] = {0};
static char str_month[5] = {0};
static char str_day[5] = {0};
static char str_hour[5] = {0};
static char str_min[5] = {0};

static struct tm temp_tm;
static struct date temp_date;
static bool time_change_flag = false;


#define SETTING_TIME_YEAR_ID 0x10
#define SETTING_TIME_MONTH_ID 0x11
#define SETTING_TIME_DAY_ID 0x12
#define SETTING_TIME_HOUR_ID 0x13
#define SETTING_TIME_MINUTE_ID 0x14
#define SETTING_TIME_CALENDAR_SW_ID 0x15

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
    time_change_flag = true; 
   
    sprintf(str_year, "%04d", temp_date.year);
    
    lv_obj_t * center_container= lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_YEAR_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_year);
    lv_obj_invalidate(center_container);
    // 校验日期合法性（年变化可能影响当月天数）
    int mon_last_day =  user_western_calendar_month_last_day(temp_date.year, temp_date.month) ;
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}
// 右箭头：年份加1（同左箭头范围）
static void setting_time_year_btn_up(lv_obj_t *obj)
{
    if(user_data_get()->setting.calendar == 1)
    {
    if (++temp_date.year > 2037)
        temp_date.year = 2022;
    }

    time_change_flag = true;
    sprintf(str_year, "%04d", temp_date.year);
   
    lv_obj_t * center_container= lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_YEAR_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_year);
    lv_obj_invalidate(center_container);
    // 校验日期合法性
    int mon_last_day = user_western_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}

static bool years_btn_create(lv_obj_t *center_cont)
{
    static obj_click_data right_data = obj_click_data_up_create(setting_time_year_btn_up);     // 右箭头（加）
    static obj_click_data left_data = obj_click_data_up_create(setting_time_year_left_btn_up); // 左箭头（减）
	setting_double_arrow_btn_create(center_cont, 130, 55 + (50 * 0), 400, 35,
									str_get(LAYOUT_CALENDAR_LANG_YEAR_ID),
                                    // 当前年份显示
									//!user_data_get()->setting.language ? "English" : "بالعربية", // 当前语言显示
                                    str_year,
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
   
    lv_obj_t * center_container= lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_MONTH_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_month);
     lv_obj_invalidate(center_container);
    // 校验日期合法性（月变化影响当月天数）
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}

// 右箭头：月份加1（1-12循环）
static void setting_time_month_btn_up(lv_obj_t *obj)
{
    if (++temp_date.month > 12)
        temp_date.month = 1;
    time_change_flag = true;
    sprintf(str_month, "%02d", temp_date.month);

    lv_obj_t * center_container= lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_MONTH_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_month);
     lv_obj_invalidate(center_container);
    // 校验日期合法性
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);
    if (temp_date.day > mon_last_day)
    {
        temp_date.day = mon_last_day;
        sprintf(str_day, "%02d", temp_date.day);
        lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    }
}

static bool month_btn_create(lv_obj_t *center_cont)
{
	static obj_click_data left_click_data = obj_click_data_up_create(setting_time_month_letf_btn_up);
	static obj_click_data right_click_data = obj_click_data_up_create(setting_time_month_btn_up);

	setting_double_arrow_btn_create(center_cont, 130, 55 + (50 * 1), 400, 35,
									str_get(LAYOUT_CALENDAR_LANG_MONTH_ID),
									//!user_data_get()->setting.language ? "English" : "بالعربية", // 当前语言显示
                                    str_month,
                                    &right_click_data,
									&left_click_data,
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
   
    lv_obj_t * center_container= lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    lv_obj_invalidate(center_container);
}

// 右箭头：日期加1（1-当月最后一天循环）
static void setting_time_day_btn_up(lv_obj_t *obj)
{
    int mon_last_day = (user_data_get()->setting.calendar == 1) ? user_western_calendar_month_last_day(temp_date.year, temp_date.month) : user_persian_calendar_month_last_day(temp_date.year, temp_date.month);

    if (++temp_date.day > mon_last_day)
        temp_date.day = 1;

    time_change_flag = true;
    sprintf(str_day, "%02d", temp_date.day);
    
    lv_obj_t * center_container= lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_DAY_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_day);
    lv_obj_invalidate(center_container);
}

static bool day_btn_create(lv_obj_t *center_cont)
{
	static obj_click_data left_click_data = obj_click_data_up_create(setting_time_day_left_btn_up);
	static obj_click_data right_click_data = obj_click_data_up_create(setting_time_day_btn_up);

	setting_double_arrow_btn_create(center_cont, 130, 55 + (50 * 2), 400, 35,
									str_get(LAYOUT_CALENDAR_LANG_DAY_ID),
									//!user_data_get()->setting.language ? "English" : "بالعربية", // 当前语言显示
                                    str_day,
                                    &right_click_data,
									&left_click_data,
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
  
    lv_obj_t * center_container= lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_HOUR_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_hour);
    lv_obj_invalidate(center_container);
}

// 右箭头：小时加1（0-23循环）
static void setting_time_hour_btn_up(lv_obj_t *obj)
{
    if (++temp_tm.tm_hour > 23)
        temp_tm.tm_hour = 0;
    time_change_flag = true;
    sprintf(str_hour, "%02d", temp_tm.tm_hour);
    
    lv_obj_t * center_container= lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_HOUR_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_hour);
    lv_obj_invalidate(center_container);
}


static bool hour_btn_create(lv_obj_t *center_cont)
{
	static obj_click_data left_click_data = obj_click_data_up_create(setting_time_hour_left_btn_up);
	static obj_click_data right_click_data = obj_click_data_up_create(setting_time_hour_btn_up);

	setting_double_arrow_btn_create(center_cont, 130, 55 + (50 * 3), 400, 35,
									str_get(LAYOUT_CALENDAR_LANG_HOUR_ID),
									//!user_data_get()->setting.language ? "English" : "بالعربية", // 当前语言显示
                                    str_hour,
                                    &right_click_data, 
									&left_click_data,
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

    lv_obj_t * center_container= lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_MINUTE_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_min);
    lv_obj_invalidate(center_container);
}

// 右箭头：分钟加1（0-59循环）
static void setting_time_minute_btn_up(lv_obj_t *obj)
{
    if (++temp_tm.tm_min > 59)
        temp_tm.tm_min = 0;
    time_change_flag = true;
    sprintf(str_min, "%02d", temp_tm.tm_min);
   
    lv_obj_t * center_container= lv_obj_get_child_form_id(lv_scr_act(), 100);
    lv_obj_set_style_local_value_str(lv_obj_get_child_form_id(center_container, SETTING_TIME_MINUTE_ID), LV_CONT_PART_MAIN, LV_STATE_DEFAULT, str_min);
    lv_obj_invalidate(center_container);
}

static bool min_btn_create(lv_obj_t *center_cont)
{
	static obj_click_data left_click_data = obj_click_data_up_create(setting_time_minute_left_btn_up);
	static obj_click_data right_click_data = obj_click_data_up_create(setting_time_minute_btn_up);

	setting_double_arrow_btn_create(center_cont, 130, 55 + (50 * 4), 400, 35,
									str_get(LAYOUT_CALENDAR_LANG_MIN_ID),
									//!user_data_get()->setting.language ? "English" : "بالعربية", // 当前语言显示
                                    str_min,
                                    &right_click_data,
									&left_click_data,						 
                                    SETTING_TIME_MINUTE_ID);
	return true;
}

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

/* setting 图标 */
static void setting_icon_create(lv_obj_t *parent)
{
    lv_obj_t *setting_icon_obj = lv_img_create(parent, NULL);
    lv_obj_set_pos(setting_icon_obj, 54, 34);
	lv_obj_set_size(setting_icon_obj, 22, 20);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_GEAR_PNG);
	lv_obj_set_id(setting_icon_obj, HOME_GEAR_OBJ_ID);
	lv_obj_set_style_local_pattern_image(setting_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);

    lv_obj_t *Time_label = lv_label_create(parent, NULL);
    lv_obj_set_pos(Time_label, 87, 28);
	lv_obj_set_size(Time_label, 70, 31);
    lv_label_set_text(Time_label,str_get(COMMON_LANG_LEFT_HEAD_TIME_ID));
	lv_obj_align(setting_icon_obj, Time_label, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}

static void time_setting_btn_display(void)
{
    user_time_read(&temp_tm);
    
    temp_date.year = temp_tm.tm_year;
    temp_date.month = temp_tm.tm_mon;
    temp_date.day = temp_tm.tm_mday;

    if(temp_date.year < 2022)
    {
        temp_date.year = 2022;
        temp_date.month = 1;
        temp_date.day = 1;
    }
}


static void LAYOUT_ENTER_FUNC(time)
{
    time_change_flag = false;
    time_setting_btn_display();
    sprintf(str_year, "%04d", temp_date.year);
    sprintf(str_month, "%02d", temp_date.month);
    sprintf(str_day, "%02d", temp_date.day);
    sprintf(str_hour, "%02d", temp_tm.tm_hour);
    sprintf(str_min, "%02d", temp_tm.tm_min);
 
	lv_obj_t *parent = common_bg_display(lv_scr_act());
    lv_obj_t *center_cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(center_cont, 664, 336);  
    lv_obj_set_id(center_cont, 100);
    lv_obj_align(center_cont, parent,LV_ALIGN_CENTER, 0, 0);  
    lv_obj_set_style_local_bg_color(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x838383));
    lv_obj_set_style_local_radius(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_bg_opa(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
    back_btn_create(parent);
    
    years_btn_create(center_cont);
    month_btn_create(center_cont); 
    day_btn_create(center_cont);
    hour_btn_create(center_cont);
    min_btn_create(center_cont);
    setting_icon_create(parent);
    top_time_date_text_create(parent);
    
}


static void LAYOUT_QUIT_FUNC(time)
{
    if(time_change_flag == true)
    {
        temp_tm.tm_year = temp_date.year ;
        temp_tm.tm_mon = temp_date.month ;
        temp_tm.tm_mday = temp_date.day;
        temp_tm.tm_sec = 0;
        standby_timer_close();
        user_time_set(&temp_tm);

        user_time_read(&temp_tm);
        if((temp_tm.tm_year) < 2022)
        {
            temp_tm.tm_year = 2022  ;
            user_time_set(&temp_tm);
        }

        standby_timer_restart(true);
    }    
    user_data_save();
    extern unsigned long long calibrate_rtc_timestamp;
    calibrate_rtc_timestamp = user_timestamp_get();

}

CREATE_LAYOUT(time);
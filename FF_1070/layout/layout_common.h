#ifndef _LAYOUT_COMMON_H_
#define _LAYOUT_COMMON_H_

#include "ringplay.h"

#define TOUCH_TONE_VOL 2 // 触摸按键音音量
#define OPEN_TONE_VOL 2	 // 开锁铃声音量

// 区域的起始坐标和宽高
typedef struct
{
	lv_coord_t x;
	lv_coord_t y;
	lv_coord_t w;
	lv_coord_t h;
} custom_area;

//

/***
** 日期: 2022-04-28 09:53
** 作者: leo.liu
** 函数作用：控件被按下执行的回调函数
** 返回参数说明：
***/
void layout_obj_click_down_func(lv_obj_t *obj);
/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：door1 call 默认处理函数
** 返回参数说明：
***/
void layout_door1_call_default(void);
/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：door2 call 默认处理函数
** 返回参数说明：
***/
void layout_door2_call_default(void);
/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：室内机开锁键按下 默认处理函数
** 返回参数说明：
***/
void layout_gate_open_default(void);
/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：门禁机呼梯默认处理函数
** 返回参数说明：
***/
void layout_elevator_call_default(void);
/***
** 日期: 2022-05-12 10:27
** 作者: leo.liu
** 函数作用：hook state change 默认处理函数(听筒状态改变)
** 返回参数说明：
***/
bool layout_hook_state_change_default(unsigned int cmd, unsigned int arg);
/***
** 日期: 2022-05-13 11:05
** 作者: leo.liu
** 函数作用：sd卡状态改变默认回调
** 返回参数说明：
***/
void layout_sdcard_state_change_default(void);

typedef enum
{
	MON_ENTER_NONE,
	MON_ENTER_MANUAL_DOOR,
	MON_ENTER_MANUAL_CCTV,
	MON_ENTER_CALL,
	MON_ENTER_TALK,
	MON_ENTER_DISPLAY,
} MON_ENTER_FLG;
/***
** 日期: 2022-05-13 11:05
** 作者: leo.liu
** 函数作用：设置进入监控的标志位
** 返回参数说明：
***/
void monitor_enter_mask_set(MON_ENTER_FLG flg);
/***
** 日期: 2022-05-13 11:05
** 作者: leo.liu
** 函数作用：获取进入监控的标志位
** 返回参数说明：
***/
MON_ENTER_FLG monitor_enter_mask_get(void);
/***
** 日期: 2022-05-17 08:13
** 作者: leo.liu
** 函数作用：获取当前通道的亮度值
** 返回参数说明：
***/
int monitor_display_brightness_vol_get(void);
/***
** 日期: 2022-05-17 08:15
** 作者: leo.liu
** 函数作用：设置亮度值
** 返回参数说明：
***/
void monitor_display_brightness_vol_set(int vol);
/***
** 日期: 2022-05-17 08:13
** 作者: leo.liu
** 函数作用：获取当前通道的亮度值
** 返回参数说明：
***/
int monitor_display_cont_vol_get(void);
/***
** 日期: 2022-05-17 08:15
** 作者: leo.liu
** 函数作用：设置亮度值
** 返回参数说明：
***/
void monitor_display_cont_vol_set(int vol);
/***
** 日期: 2022-05-17 08:13
** 作者: leo.liu
** 函数作用：获取当前通道的色度值
** 返回参数说明：
***/
int monitor_display_color_vol_get(void);
/***
** 日期: 2022-05-17 08:15
** 作者: leo.liu
** 函数作用：设置亮度值
** 返回参数说明：
***/
void monitor_display_color_vol_set(int vol);
/***
** 日期: 2022-05-20 17:30
** 作者: leo.liu
** 函数作用：铃声播放其实回调函数
** 返回参数说明：
***/
void ringplay_keysound_start_default_func(int index);
/***
** 日期: 2022-05-20 17:30
** 作者: leo.liu
** 函数作用：铃声结束的默认处理
** 返回参数说明：
***/
void ringplay_keysound_finish_default_func(int index);
/***
** 日期: 2022-05-21 08:12
** 作者: leo.liu
** 函数作用：门口机铃声开始
** 返回参数说明：
***/
void ringplay_doorcall_start_default_func(int index);
/***
** 日期: 2022-05-21 08:12
** 作者: leo.liu
** 函数作用：门口机铃声结束
** 返回参数说明：
***/
void ringplay_doorcall_finish_default_func(int index);
// 电源指示灯默认处理函数
void power_led_handler_default_func(void);

bool top_time_date_text_create(lv_obj_t *parent);

lv_obj_t *common_bg_display(lv_obj_t *parent);
lv_obj_t *common_img_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src);
lv_obj_t *common_gate_img_btn_create(lv_obj_t *parent, int obj_id, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src);
lv_obj_t *photo_and_video_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src, bool underline, bool string_select);
lv_obj_t *camera_img_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src);
lv_obj_t *message_box_create(lv_obj_t *parent, const char *title, obj_click_data *yes_btn_data, obj_click_data *no_btn_data);
lv_obj_t *memory_message_box_create(lv_obj_t *parent, obj_click_data *yes_btn_data, obj_click_data *no_btn_data, layout_lang_id id);
lv_obj_t *page_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src);
lv_obj_t *memory_delete_box_create(lv_obj_t *parent, obj_click_data *yes_btn_data, obj_click_data *no_btn_data, layout_lang_id id);
// lv_obj_t *setting_left_icon_create(lv_obj_t *parent,layout_lang_id id);

void ring_play(int index, int volume, ringplay_callback start, ringplay_callback finish, bool loop);
void get_curr_relesse_date(int *day, int *month, int *year);

#endif

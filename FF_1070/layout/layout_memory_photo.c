/*******************************************************************
 * @Descripttion   :
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-11 18:14
 * @LastEditTime   : 2023-03-30 17:10
 *******************************************************************/
/*
 *当前的文件管理机制，是把sd卡所有的 图片 和 视频 放在media文件
 *但是项目需求是能够单独预览 图片 和 视频，所以需要将media里的文件分类（不改动文件管理机制）
 *方法1：预览前根据文件类型去筛选，翻页时再筛选，直接显示（已实现）
 *方法2：预览前根据文件类型去筛选并将文件索引保存到动态内存，翻页时只需从动态内存中拿出索引即可
 */
#include "layout_define.h"
#include "media_thumb.h"

typedef enum
{
	MEDIA_PHOTO_TYPE = 0,
	MEDIA_VIDEO_TYPE
} memory_media_type;

typedef enum
{
	MEMORY_HOME_BTN_ID,
	// MEMORY_MODE_BTN_ID,
	MEMORY_PREV_BTN_ID,
	MEMORY_PLAY_BTN_ID,
	MEMORY_NEXT_BTN_ID,
	MEMORY_DELETE_BTN_ID,
	MEMORY_TOTAL_BTN,
} photo_btn_module;

static custom_area photo_btn_area[MEMORY_TOTAL_BTN] =
	{
		{582, 450, 120, 120}, 
		{192, 450, 120, 120},
		{322, 450, 120, 120},
		{452, 450, 120, 120},
		{712, 450, 120, 120},
};
#define MEMORY_FUNC_BTN_BG_BLOCK_ID 8
#define MEMORY_HEAD_CH_LABEL_ID 9
#define MEMORY_HEAD_TIME_LABEL_ID 10
#define MEMORY_HEAD_INDEX_LABEL_ID 11
// #define MEMORY_HEAD_INDEX_NUM_LABEL_ID 12
#define MEMORY_PHOTO_TIMEOUT_DURATION 120 // 显示时长 *

static int memory_photo_timeout_val = MEMORY_PHOTO_TIMEOUT_DURATION;
// 静态变量存储遮罩层，供后续删除（确认/取消时）
static lv_obj_t *dim_mask = NULL;
static bool func_btn_diaplay_flag = true;
extern int photo_index_get(void);
extern void photo_index_set(int index);
extern void photo_total_set(int index);
extern file_type photo_state_get(void);

static int photo_total = 0;
// static int photo_index = 0;//1 ～ photo_total

static int media_total = 0;
static int media_index = 0; // 0 ～ media_total-1
// static const file_info *p_media_info = NULL;
static file_type photo_file_type = FILE_TYPE_PHOTO;
static char photo_file_path[20] = {0};

extern void video_index_reset(void);

static void layout_memory_photo_load(void);
static void memory_photo_param_init(void);
static void photo_next_btn_up(lv_obj_t *obj);

// 复位显示倒计时
void memory_photo_timeout_value_reset(void)
{
	memory_photo_timeout_val = MEMORY_PHOTO_TIMEOUT_DURATION;
}

static int photo_index_new = 0;  // 本地图片索引（0~photo_total-1)

void photo_index_reset(void)
{
	photo_index_new = 0;
}
// static void memory_func_btn_topbg_block_create(lv_obj_t *parent)
// {
// 	lv_obj_t *obj = lv_obj_create(parent, NULL);
// 	lv_obj_set_click(obj, false);
// 	lv_obj_set_pos(obj, 0, 0);
// 	lv_obj_set_size(obj, 1024, 70);
// 	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
// 	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
// }
static void photo_head_label_create(lv_obj_t *parent)
{
	lv_obj_t *label1 = lv_label_create(parent, NULL);
	lv_obj_set_id(label1, MEMORY_HEAD_CH_LABEL_ID);
	lv_obj_set_pos(label1, 54, 28);
	lv_label_set_text(label1, " ");
	lv_obj_set_style_local_text_color(label1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
	lv_obj_set_style_local_text_font(label1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(30));

	lv_obj_t *label2 = lv_label_create(parent, label1);
	lv_obj_set_id(label2, MEMORY_HEAD_TIME_LABEL_ID);
	lv_label_set_text(label2, " ");

	lv_obj_t *label3 = lv_label_create(parent, label1);
	lv_obj_set_id(label3, MEMORY_HEAD_INDEX_LABEL_ID);
	lv_label_set_text(label3, " ");

	// lv_obj_t *label4 = lv_label_create(parent, label1);
	// lv_obj_set_id(label4, MEMORY_HEAD_INDEX_NUM_LABEL_ID);
	// lv_label_set_text(label4, " ");

	lv_obj_set_pos(label2, 347, 28);
	lv_obj_set_pos(label3, 827, 28);
	// lv_obj_set_pos(label4, 520, 16);
}
static void photo_head_label_display(const file_info *pinfo)
{
	lv_obj_t *label1 = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_CH_LABEL_ID);
	MON_CH ch = pinfo->ch;
	lv_label_set_text(label1, ch == MON_CH_DOOR1 ? str_get(LAYOUT_HOME_LANG_DOOR1_ID) : ch == MON_CH_DOOR2 ? str_get(LAYOUT_HOME_LANG_DOOR2_ID)
																					: ch == MON_CH_CCTV1   ? str_get(LAYOUT_HOME_LANG_CCTV1_ID)
																										   : str_get(LAYOUT_HOME_LANG_CCTV2_ID));

	char str[5] = {0};

	memset(str, 0, 5);
	memcpy(str, pinfo->file_name, 2);
	int year = atoi(str) + (atoi(str) < 37 ? 2000 : 1900);

	memset(str, 0, 5);
	memcpy(str, &(pinfo->file_name[2]), 2);
	int month = atoi(str);

	memset(str, 0, 5);
	memcpy(str, &(pinfo->file_name[4]), 2);
	int day = atoi(str);

	memset(str, 0, 5);
	memcpy(str, &(pinfo->file_name[7]), 2);
	int hour = atoi(str);

	memset(str, 0, 5);
	memcpy(str, &(pinfo->file_name[9]), 2);
	int min = atoi(str);

	memset(str, 0, 5);
	memcpy(str, &(pinfo->file_name[11]), 2);
	int second = atoi(str);
	printf("==============>> [%04d-%02d-%02d   %02d:%02d:%02d]\n", year, month, day, hour, min, second);
	lv_obj_t *label2 = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_TIME_LABEL_ID);
	if (user_data_get()->setting.calendar == 0)// 0=波斯历，1=公历
	{
		struct date temp_date =
			{
				.year = year,
				.month = month,
				.day = day};
		temp_date = gregorian2jalali(temp_date);
		lv_label_set_text_fmt(label2, "%04d-%02d-%02d %02d:%02d:%02d", temp_date.year, temp_date.month, temp_date.day, hour, min, second);
	}
	else
	{
		lv_label_set_text_fmt(label2, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min, second);	}

	lv_obj_t *label3 = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_INDEX_LABEL_ID);
	char str1[30] = {0};
	sprintf(str1, "%04d/%04d", photo_total-photo_index_new, photo_total);
	lv_label_set_text(label3, str1);

	// lv_obj_t *label4 = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_INDEX_NUM_LABEL_ID);
	// char str2[30] = {0};
	// sprintf(str2, "%s :", str_get(LAYOUT_MEMORY_LANG_IMAGE_ID));
	// lv_label_set_text(label4, str2);

	// lv_obj_set_pos(label2, 347, 28);

	// if (user_data_get()->setting.language == LANG_PERSIAN)
	// {
	// 	// lv_obj_set_pos(label4, 640, 16);
	// 	lv_obj_set_pos(label3, 827, 28);
	// }
	// else
	// {
	// 	lv_obj_set_pos(label3, 827, 28);
	// 	// lv_obj_set_pos(label4, 520, 16);
	// }
}

// 创建camera界面功能按键的背景块
//  static void photo_func_btn_bg_block_create(lv_obj_t * parent)
//  {
//  	lv_obj_t *obj = lv_obj_create(parent, NULL);
//  	lv_obj_set_pos(obj, 0, 470);
//  	lv_obj_set_size(obj, 1024, 130);
//  	lv_obj_set_id(obj, MEMORY_FUNC_BTN_BG_BLOCK_ID);
//  	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x4F4F4F));
//  	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
//  }

static void photo_home_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(photo_list));
}
// 创建home按钮
static void photo_home_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(photo_home_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEMORY_HOME_PNG);
	lv_obj_t *btn = camera_img_btn_create(parent, photo_btn_area[MEMORY_HOME_BTN_ID], NULL, &btn_data, &info);
	lv_obj_set_id(btn, MEMORY_HOME_BTN_ID);
}

#if 0
static void photo_mode_btn_up(lv_obj_t *obj)
{
	user_data_get()->media_disp_mode = MEDIA_VIDEO_TYPE;
	goto_layout(pLAYOUT(memory_video));
}
//创建mode按钮
static void photo_mode_btn_create(lv_obj_t * parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(photo_mode_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEMORY_IMAGE_MODE_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_MEMORY_VIDEO_MODE_PNG);
	lv_obj_t *btn = camera_img_btn_create(parent, photo_btn_area[MEMORY_MODE_BTN_ID], str_get(LAYOUT_MEMORY_LANG_MODE_ID), &btn_data, user_data_get()->media_disp_mode == MEDIA_PHOTO_TYPE ? &info1 : &info);
	lv_obj_set_id(btn, MEMORY_MODE_BTN_ID);
}

#endif

static void photo_prev_btn_up(lv_obj_t *obj)
{
    if (photo_total <= 1)
        return;
    if (photo_file_type == FILE_TYPE_PHOTO) // SD卡照片
    {
        photo_index_new--;
        if (photo_index_new < 0)
        {
            photo_index_new = photo_total - 1;
        }
    }
    else if (photo_file_type == FILE_TYPE_FLASH_PHOTO) // Flash照片
    {
        photo_index_new++;
        if (photo_index_new > (photo_total - 1))
        {
            photo_index_new = 0;
        }
    }
	
    photo_index_set(photo_index_get() + 1);
    if (photo_index_get() >= photo_total)
    {
        photo_index_set(0);
    }
    printf("==============>>>>>>>photo_index_get()[%d], file_type:[%d]\n", photo_index_get(), photo_file_type);
    layout_memory_photo_load();
}
// 创建prev按钮
static void photo_prev_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(photo_prev_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEMORY_PREV_PNG);
	lv_obj_t *btn = camera_img_btn_create(parent, photo_btn_area[MEMORY_PREV_BTN_ID], NULL, &btn_data, &info);
	lv_obj_set_id(btn, MEMORY_PREV_BTN_ID);
}

static lv_task_t *photo_play_task_t = NULL;

static void memory_photo_ticker_task(lv_task_t *task_t)
{
	if (memory_photo_timeout_val-- <= 0)
	{
		if (dim_mask != NULL)
		{
			lv_obj_del(dim_mask);
			dim_mask = NULL;
		}
		goto_layout(pLAYOUT(standby));
	}
	if (photo_play_task_t != NULL)
	{
		lv_task_del(task_t);
	}
}

static void photo_play_task(lv_task_t *task_t)
{
	photo_next_btn_up(NULL);
}
static void memory_func_btn_diaplay_enable(bool en)
{
	func_btn_diaplay_flag = en;
	for (int i = 0; i <= 4; i++)
	{
		lv_obj_set_hidden(lv_obj_get_child_form_id(lv_scr_act(), i), !en);
	}
}
static void photo_play_btn_up(lv_obj_t *obj)
{
	if (photo_total <= 1)
		return;
	memory_func_btn_diaplay_enable(!func_btn_diaplay_flag);
	if (photo_play_task_t == NULL)
	{
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HOME_BTN_ID), false);
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_PREV_BTN_ID), false);
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_NEXT_BTN_ID), false);
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_DELETE_BTN_ID), false);
		standby_timer_close();
		photo_play_task_t = lv_layout_task_create(photo_play_task, 3000, LV_TASK_PRIO_LOW, NULL);

		memory_photo_timeout_value_reset();
	}
	else
	{
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HOME_BTN_ID), true);
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_PREV_BTN_ID), true);
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_NEXT_BTN_ID), true);
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_DELETE_BTN_ID), true);
		// standby_timer_restart(true);
		lv_task_del(photo_play_task_t);
		photo_play_task_t = NULL;

		memory_photo_timeout_value_reset();
		lv_layout_task_create(memory_photo_ticker_task, 500, LV_TASK_PRIO_HIGH, NULL);
	}
}
// 创建play按钮
static void photo_play_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(photo_play_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEMORY_PLAY_PNG);
	lv_obj_t *btn = camera_img_btn_create(parent, photo_btn_area[MEMORY_PLAY_BTN_ID], NULL, &btn_data, &info);
	lv_obj_set_id(btn, MEMORY_PLAY_BTN_ID);
}

static void photo_next_btn_up(lv_obj_t *obj)
{
    if (photo_total <= 1)
        return;
    if (photo_file_type == FILE_TYPE_PHOTO) // SD卡照片：原逻辑不变
    {
        photo_index_new++;
        if (photo_index_new > (photo_total - 1))
        {
            photo_index_new = 0;
        }
    }
    else if (photo_file_type == FILE_TYPE_FLASH_PHOTO) // Flash照片：反向逻辑
    {
        photo_index_new--;
        if (photo_index_new < 0)
        {
            photo_index_new = photo_total - 1;
        }
    }
    photo_index_set(photo_index_get() - 1);
    if (photo_index_get() < 0)
    {
        photo_index_set(photo_total - 1);
    }

    printf("==============>>>>>>>photo_index_get_next()[%d], file_type:[%d]\n", photo_index_get(), photo_file_type);
    layout_memory_photo_load();
}
// 创建next按钮
static void photo_next_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(photo_next_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEMORY_NEXT_PNG);
	lv_obj_t *btn = camera_img_btn_create(parent, photo_btn_area[MEMORY_NEXT_BTN_ID], NULL, &btn_data, &info);
	lv_obj_set_id(btn, MEMORY_NEXT_BTN_ID);
}

static void memory_bg_btn_up(lv_obj_t *obj)
{
	if (photo_total <= 1)
		return;
	memory_func_btn_diaplay_enable(!func_btn_diaplay_flag);
	if (photo_play_task_t == NULL)
	{
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HOME_BTN_ID), false);
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_PREV_BTN_ID), false);
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_NEXT_BTN_ID), false);
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_DELETE_BTN_ID), false);
		standby_timer_close();
		photo_play_task_t = lv_layout_task_create(photo_play_task, 3000, LV_TASK_PRIO_LOW, NULL);

		memory_photo_timeout_value_reset();
	}
	else
	{
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HOME_BTN_ID), true);
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_PREV_BTN_ID), true);
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_NEXT_BTN_ID), true);
		lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_DELETE_BTN_ID), true);
		// standby_timer_restart(true);
		lv_task_del(photo_play_task_t);
		photo_play_task_t = NULL;

		memory_photo_timeout_value_reset();
		lv_layout_task_create(memory_photo_ticker_task, 500, LV_TASK_PRIO_HIGH, NULL);
	}
}
static void memory_bg_btn_click_enable(bool en)
{
	if (en)
	{
		static obj_click_data bg_btn_data = obj_click_data_up_create(memory_bg_btn_up);
		obj_click_event_listen(lv_scr_act(), &bg_btn_data);
	}
	else
	{
		obj_click_event_listen(lv_scr_act(), NULL);
	}
}

static void photo_delete_yes_btn_up(lv_obj_t *obj)
{
	if (dim_mask != NULL)
	{
		lv_obj_del(dim_mask);
		dim_mask = NULL;
	}
	media_file_delete(photo_file_type, photo_index_new);
	photo_index_set(photo_index_get() - 1);
	goto_layout(pLAYOUT(memory_photo));
}
static void photo_delete_no_btn_up(lv_obj_t *obj)
{
	memory_bg_btn_click_enable(true);
	if (dim_mask != NULL)
	{
		lv_obj_del(dim_mask);
		dim_mask = NULL;
	}
	lv_obj_t *btn_area = obj->parent;
	lv_obj_t *msg_box_cont = btn_area->parent;
	lv_obj_del(msg_box_cont);
	lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HOME_BTN_ID), true);
	lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_PREV_BTN_ID), true);
	lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_PLAY_BTN_ID), true);
	lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_NEXT_BTN_ID), true);
	lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_DELETE_BTN_ID), true);
}
static void create_dim_mask()
{
	// 先删除旧遮罩（防止重复创建）
	if (dim_mask != NULL)
	{
		lv_obj_del(dim_mask);
		dim_mask = NULL;
	}

	// 创建遮罩层，父对象为当前屏幕
	dim_mask = lv_obj_create(lv_scr_act(), NULL);
	// 尺寸覆盖全屏
	lv_obj_set_size(dim_mask, 1024, 600);
	// 对齐方式：全屏覆盖
	lv_obj_align(dim_mask, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	// 背景色：黑色，不透明度 50%（半透明，实现变暗效果）
	lv_obj_set_style_local_bg_color(dim_mask, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	lv_obj_set_style_local_bg_opa(dim_mask, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50); // 50%不透明
	// 禁用遮罩层点击（避免干扰弹窗操作）
	lv_obj_set_click(dim_mask, false);
}
static void photo_delete_btn_up(lv_obj_t *obj)
{
	if (photo_total == 0)
		return;
	memory_bg_btn_click_enable(false);
	lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HOME_BTN_ID), false);
	lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_PREV_BTN_ID), false);
	lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_PLAY_BTN_ID), false);
	lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_NEXT_BTN_ID), false);
	lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_DELETE_BTN_ID), false);
	// 创建全屏遮罩层（屏幕变暗）
	create_dim_mask();
	static obj_click_data btn_data = obj_click_data_up_create(photo_delete_yes_btn_up);
	static obj_click_data btn_data1 = obj_click_data_up_create(photo_delete_no_btn_up);
	memory_delete_box_create(lv_scr_act(), &btn_data, &btn_data1, LAYOUT_MEMORY_LANG_DELETE_PICTURE_ID);
}
// 创建delete按钮
static void photo_delete_btn_create(lv_obj_t *parent)
{
	static obj_click_data btn_data = obj_click_data_up_create(photo_delete_btn_up);
	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEMORY_DELETE_PNG);
	lv_obj_t *btn = camera_img_btn_create(parent, photo_btn_area[MEMORY_DELETE_BTN_ID], NULL, &btn_data, &info);
	lv_obj_set_id(btn, MEMORY_DELETE_BTN_ID);
}

// 创建back按钮
// static void photo_back_btn_up(lv_obj_t *obj)
// {
// 	// photo_index_set(photo_index_get() + 1);
// 	goto_layout(pLAYOUT(photo_list));
// }

// static void photo_back_btn_create(lv_obj_t *parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(photo_back_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEMORY_BACK_PNG);
// 	lv_obj_t *btn = camera_img_btn_create(parent, photo_btn_area[MEMORY_BACK_BTN_ID], str_get(COMMON_LANG_BACK_ID), &btn_data, &info);
// 	lv_obj_set_id(btn, MEMORY_BACK_BTN_ID);
// }

static void layout_memory_photo_load(void)
{
	thumb_media_buffer_clear();
	const file_info *pinfo = media_file_info_get(photo_file_type, photo_index_new);
	char file[128] = {0};
	strcpy(file, photo_file_path);
	strcat(file, pinfo->file_name);

	printf("=================>> 图片索引:[%d]   索引:[%d]   文件路径:[%s] \n", photo_index_new, media_index, file);

	thumb_media_load(0, 0, 1024, 600, file);
	printf("======%d=======[][][%d]\n",pinfo->is_new, photo_index_new);
	if (pinfo->is_new == true)
	{
		printf("11111111111111111111111\n");
		media_file_new_clear(pinfo->type, photo_index_new);
	}
	photo_head_label_display(pinfo);
	lv_obj_invalidate(lv_scr_act());
}

static void memory_photo_param_init(void)
{
	if ((media_sdcard_insert_check() == true) && (photo_state_get() == FILE_TYPE_PHOTO))
	{
		photo_file_type = FILE_TYPE_PHOTO;
		sprintf(photo_file_path, "%s", SD_PHOTO_PATH);
		sd_media_all_file_total_get(&media_total, NULL);
		printf("====================>> sd卡媒体文件总数:[%d]\n", media_total);
	}
	else
	{
		photo_file_type = FILE_TYPE_FLASH_PHOTO;
		sprintf(photo_file_path, "%s", FLASH_PHOTO_PATH);
		media_file_total_get(photo_file_type, &media_total, NULL);
		printf("====================>> flash媒体文件总数:[%d]\n", media_total);
	}

	media_file_total_get(photo_file_type, &photo_total, NULL);
	printf("====================>> 图片文件总数:[%d]\n", photo_total);

	if (photo_total == 0) // 没有图片
	{
		goto_layout(pLAYOUT(photo_list));
		return;
	}
	else
	{
		lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
	}

	// printf("===========photo_index=[%d]\n",photo_index_get());
	if (photo_index_get() < 0)
	{
		photo_index_set(0);
		photo_index_new = photo_total - 1;
	}
	else
	{
		photo_index_new = photo_total - (photo_index_get()) - 1;
	}
	if (photo_index_new < 0)
	{
		photo_index_new = photo_total - 1;
	}
	printf("===========photo_index=[%d]\n", photo_index_get());
	thumb_media_open();
	layout_memory_photo_load();
}

static void memory_photo_sdcard_state_change_event_cb(void)
{
	photo_index_reset();
	video_index_reset();
	photo_index_set(0);
	goto_layout(pLAYOUT(memory_photo));
}

static void LAYOUT_ENTER_FUNC(memory_photo)
{
	printf("come in memory_photo\n");
	// standby_timer_close();
	lv_obj_t *parent = lv_scr_act();

	// memory_func_btn_topbg_block_create(parent);

	photo_head_label_create(parent);

	// photo_func_btn_bg_block_create(parent);
	photo_home_btn_create(parent);

	// photo_mode_btn_create(parent);                       /*选择图片或者视频切换模式*/
	photo_prev_btn_create(parent); /*点击搜索上一页面的图片*/

	photo_play_btn_create(parent); /*播放按钮*/

	photo_next_btn_create(parent); /*点击搜索下一页面的图片*/

	photo_delete_btn_create(parent); /*删除图片按钮*/

	// photo_back_btn_create(parent);                      	/*返回列表按钮*/
	memory_bg_btn_click_enable(true);

	lyaout_sd_state_callback_register(memory_photo_sdcard_state_change_event_cb);

	memory_photo_param_init(); /*初始化*/
}
static void LAYOUT_QUIT_FUNC(memory_photo)
{
	if (photo_play_task_t != NULL)
	{
		lv_task_del(photo_play_task_t);
		photo_play_task_t = NULL;
	}
	memory_bg_btn_click_enable(false);
	lv_obj_set_style_local_value_str(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "");
	lyaout_sd_state_callback_register(layout_sdcard_state_change_default);
	standby_timer_restart(true);

	if (cur_layout_get() != pLAYOUT(memory_photo) && cur_layout_get() != pLAYOUT(photo_list))
	{
		printf("===================1215465465\n");
		photo_index_reset();
		video_index_reset();

		photo_index_set(0);
	}
	thumb_media_close();
}
CREATE_LAYOUT(memory_photo);
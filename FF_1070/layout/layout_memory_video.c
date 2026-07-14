/*******************************************************************
 * @Descripttion   :
 * @version        : 1.0.0
 * @Author         : wxj
 * @Date           : 2022-11-18 10:48
 * @LastEditTime   : 2023-03-30 17:05
 *******************************************************************/
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
} video_btn_module;

static custom_area video_btn_area[MEMORY_TOTAL_BTN] =
{
    {712, 450, 120, 120},
    {192, 450, 120, 120},
    {322, 450, 120, 120},
    {452, 450, 120, 120},
    {582, 450, 120, 120},
};
#define MEMORY_FUNC_BTN_BG_BLOCK_ID 8
#define MEMORY_HEAD_CH_LABEL_ID 9
#define MEMORY_HEAD_TIME_LABEL_ID 10
#define MEMORY_HEAD_INDEX_LABEL_ID 11
#define MEMORY_HEAD_PLAY_TIME_LABEL_ID 12
#define MEMORY_HEAD_INDEX_NUM_LABEL_ID 13
#define MEMORY_VIDEO_PROGRESS_BAR_ID 14
static int memory_video_timeout_val = 60;

static bool func_btn_diaplay_flag = true;
static bool memory_video_finish_handled = false;
static bool memory_video_delete_dialog_active = false;
static bool memory_video_sdcard_removing = false;
static lv_task_t *memory_video_timeout_task = NULL;
static lv_task_t *memory_video_play_state_task = NULL;
static lv_obj_t *dim_mask = NULL;
static lv_obj_t *memory_video_delete_box = NULL;

extern int video_index_get(void);
extern void video_index_set(int index);
extern void photo_list_page_set(int index);

static int video_total = 0;
static int video_index = 0;

static int media_total = 0;
// static int media_index = 0;

extern void photo_index_reset(void);

static void layout_memory_video_load(void);
static void layout_play_state_task(lv_task_t *task_t);

void memory_video_timeout_value_reset(int num)
{
    memory_video_timeout_val = num;
}

static void memory_video_ticker_task(lv_task_t *task_t)
{
    if (task_t != memory_video_timeout_task)
    {
        lv_task_del(task_t);
        return;
    }

    printf(">>>>>>>>>>>>>>>>>[%d]\n", memory_video_timeout_val);
    if (memory_video_timeout_val-- <= 0)
    {
        if (dim_mask != NULL)
        {
            lv_obj_del(dim_mask);
            dim_mask = NULL;
        }
        memory_video_timeout_task = NULL;
        goto_layout(pLAYOUT(standby));
    }
    if (video_play_status_get() == VIDEO_PLAY_STATE_PLAY)
    {
        memory_video_timeout_task = NULL;
        lv_task_del(task_t);
    }
}

static void memory_video_timeout_task_stop(void)
{
    if (memory_video_timeout_task != NULL)
    {
        lv_task_del(memory_video_timeout_task);
        memory_video_timeout_task = NULL;
    }
}

static void memory_video_timeout_task_start(void)
{
    if (memory_video_timeout_task == NULL)
    {
        memory_video_timeout_task = lv_layout_task_create(memory_video_ticker_task, 1000, LV_TASK_PRIO_MID, NULL);
    }
}

static void memory_video_user_activity_reset(void)
{
    memory_video_timeout_value_reset(60);
    if (video_play_status_get() != VIDEO_PLAY_STATE_PLAY)
    {
        memory_video_timeout_task_start();
    }
}

static void memory_video_play_state_task_stop(void)
{
    if (memory_video_play_state_task != NULL)
    {
        lv_task_del(memory_video_play_state_task);
        memory_video_play_state_task = NULL;
    }
}

static void memory_video_play_state_task_start(void)
{
    if (memory_video_play_state_task != NULL)
    {
        return;
    }

    memory_video_play_state_task = lv_layout_task_create(layout_play_state_task, 100, LV_TASK_PRIO_LOW, NULL);
}

void video_index_reset(void)
{
    video_index = 0;
}

static void video_head_label_create(lv_obj_t *parent)
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

    lv_obj_t *label4 = lv_label_create(parent, NULL);
    lv_obj_set_id(label4, MEMORY_HEAD_PLAY_TIME_LABEL_ID);
    lv_obj_set_pos(label4, 870, 552);
    lv_obj_set_style_local_text_color(label4, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_text_font(label4, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
    lv_label_set_text(label4, "");

    lv_obj_set_pos(label2, 347, 28);
    lv_obj_set_pos(label3, 827, 28);
}

static void video_progress_bar_create(lv_obj_t *parent)
{
    lv_obj_t *progress_bar = lv_bar_create(parent, NULL);
    if (progress_bar == NULL)
        return;

    lv_obj_set_id(progress_bar, MEMORY_VIDEO_PROGRESS_BAR_ID);
    lv_obj_set_pos(progress_bar, 150, 562);
    lv_obj_set_size(progress_bar, 700, 9);

    lv_obj_set_style_local_bg_color(progress_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_bg_opa(progress_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);

    lv_obj_set_style_local_bg_color(progress_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0x00D0FF));
    lv_obj_set_style_local_bg_opa(progress_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_OPA_100);

    lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF);
}

static void video_head_info_label_display(const file_info *pinfo)
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

    lv_obj_t *label2 = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_TIME_LABEL_ID);
    if (user_data_get()->setting.calendar == 0)
    {
        struct date temp_date =
        {
            .year = year,
            .month = month,
            .day = day
        };
        temp_date = gregorian2jalali(temp_date);
        lv_label_set_text_fmt(label2, "%04d-%02d-%02d %02d:%02d:%02d", temp_date.year, temp_date.month, temp_date.day, hour, min, second);
    }
    else
    {
        lv_label_set_text_fmt(label2, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min, second);
    }

    lv_obj_t *label3 = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_INDEX_LABEL_ID);
    char str1[30] = {0};
    sprintf(str1, "%04d/%04d", video_index + 1, video_total); // 显示索引不变：0→0001
    lv_label_set_text_fmt(label3, str1);
}

static void video_head_play_time_label_display(int play_time, int play_total)
{
    lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_HEAD_PLAY_TIME_LABEL_ID);
    // 修复：原代码play_time取余60错误，应该是play_time % 60
    lv_label_set_text_fmt(label, "%02d:%02d / %02d:%02d", play_time / 60, play_time % 60, play_total / 60, play_total % 60);
}

static void video_progress_bar_update(int play_time, int play_total)
{
    lv_obj_t *progress_bar = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_VIDEO_PROGRESS_BAR_ID);
    if (progress_bar == NULL)
    {
        printf("进度条未找到：MEMORY_VIDEO_PROGRESS_BAR_ID\n");
        return;
    }

    if (play_total <= 0 || play_time < 0)
    {
        lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF);
        return;
    }
    if (play_time >= play_total)
    {
        lv_bar_set_value(progress_bar, 100, LV_ANIM_OFF);
        return;
    }

    int progress = (play_time * 100) / play_total;
    lv_bar_set_value(progress_bar, progress, LV_ANIM_ON);
}

static void video_home_btn_up(lv_obj_t *obj)
{
    memory_video_user_activity_reset();
    photo_list_page_set(video_index);
    goto_layout(pLAYOUT(photo_list));
}

static void video_home_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(video_home_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_MEMORY_HOME_PNG);
    lv_obj_t *btn = camera_img_btn_create(parent, video_btn_area[MEMORY_HOME_BTN_ID], NULL, &btn_data, &info);
    lv_obj_set_id(btn, MEMORY_HOME_BTN_ID);
}

// 修复：上一个按钮逻辑（显示索引反向）
static void video_prev_btn_up(lv_obj_t *obj)
{
    memory_video_user_activity_reset();
    if (video_total <= 1)
    return;

    // 显示索引：下一个→数值-1（total-1→total-2→...→0）
    video_index--;
    if (video_index < 0)
    {
        video_index = video_total - 1;
    }

    video_index_set(video_index); // 同步到全局索引
    printf("--------------next_video_index_get[%d] (显示索引)，实际播放索引[%d]\n", video_index, video_total - 1 - video_index);

    video_progress_bar_update(0, 0);
    video_play_stop();
    layout_memory_video_load();
    memory_video_user_activity_reset();
}

static void video_prev_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(video_prev_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_MEMORY_PREV_PNG);
    lv_obj_t *btn = camera_img_btn_create(parent, video_btn_area[MEMORY_PREV_BTN_ID], NULL, &btn_data, &info);
    lv_obj_set_id(btn, MEMORY_PREV_BTN_ID);
}

static void memory_video_btn_click_set(int obj_id, bool en)
{
    lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), obj_id);
    if (obj != NULL)
    {
        lv_obj_set_click(obj, en);
    }
}

static void video_play_btn_state_display(bool is_playing)
{
    lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), MEMORY_PLAY_BTN_ID);
    if (obj == NULL)
    {
        return;
    }

    static rom_bin_info play_info = rom_bin_info_get(ROM_UI_MEMORY_PLAY_PNG);
    static rom_bin_info start_info = rom_bin_info_get(ROM_UI_MEMORY_START_PNG);
    rom_bin_info *info = is_playing ? &start_info : &play_info;

    lv_obj_set_style_local_pattern_image(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, info);
    lv_obj_invalidate(obj);

    if (is_playing)
    {
        memory_video_btn_click_set(MEMORY_HOME_BTN_ID, true);
        memory_video_btn_click_set(MEMORY_PREV_BTN_ID, false);
        memory_video_btn_click_set(MEMORY_NEXT_BTN_ID, false);
        memory_video_btn_click_set(MEMORY_DELETE_BTN_ID, false);
    }
    else
    {
        memory_video_btn_click_set(MEMORY_HOME_BTN_ID, true);
        memory_video_btn_click_set(MEMORY_PREV_BTN_ID, true);
        memory_video_btn_click_set(MEMORY_NEXT_BTN_ID, true);
        memory_video_btn_click_set(MEMORY_DELETE_BTN_ID, true);
    }
}

static void memory_func_btn_diaplay_enable(bool en)
{
    func_btn_diaplay_flag = en;
    for (int i = 0; i <= 4; i++)
    {
        lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), i);
        if (obj != NULL)
        {
            lv_obj_set_hidden(obj, !en);
        }
    }
}

static void video_play_btn_up(lv_obj_t *obj)
{
    memory_video_user_activity_reset();
    if (video_total <= 0)
        return;

    VIDEO_PLAY_STATUS status = video_play_status_get();

    if (status == VIDEO_PLAY_STATE_IDLE)
    {
        // 开始播放
        int real_play_index = video_total - 1 - video_index;
        const file_info *pinfo = media_file_info_get(FILE_TYPE_VIDEO, real_play_index);
        char file[128] = {0};
        strcpy(file, SD_VIDEO_PATH);
        strcat(file, pinfo->file_name);
        video_play_start(file);
        memory_video_finish_handled = false;

        memory_func_btn_diaplay_enable(true);
        standby_timer_close();
        memory_video_timeout_task_stop();
        video_play_btn_state_display(true);
        return;
    }

    if (status == VIDEO_PLAY_STATE_PLAY)
    {
        // 播放中 → 暂停
        video_play_pause();
        memory_func_btn_diaplay_enable(true);
        video_play_btn_state_display(false);
        memory_video_timeout_value_reset(60);
        return;
    }

    if (status == VIDEO_PLAY_STATE_PAUSE)
    {
        // 暂停中恢复播放，video_play_pause() 内部负责 PAUSE/PLAY 状态切换。
        video_play_pause();
        memory_video_finish_handled = false;

        memory_func_btn_diaplay_enable(true);
        standby_timer_close();
        memory_video_timeout_task_stop();
        video_play_btn_state_display(true);
        return;
    }
}

static void video_play_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(video_play_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_MEMORY_PLAY_PNG);
    lv_obj_t *btn = camera_img_btn_create(parent, video_btn_area[MEMORY_PLAY_BTN_ID], NULL, &btn_data, &info);
    lv_obj_set_id(btn, MEMORY_PLAY_BTN_ID);
    lv_obj_set_style_local_pattern_image(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, &info);
    lv_obj_set_style_local_pattern_recolor_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_TRANSP);
}

// 修复：下一个按钮逻辑（显示索引反向）
static void video_next_btn_up(lv_obj_t *obj)
{
    memory_video_user_activity_reset();

	if (video_total <= 1)
    return;

    // 显示索引：上一个→数值+1（0→1→2→...→total-1）
    video_index++;
    if (video_index >= video_total)
    {
        video_index = 0;
    }

    video_index_set(video_index); // 同步到全局索引
    printf("--------------per_video_index_get[%d] (显示索引)，实际播放索引[%d]\n", video_index, video_total - 1 - video_index);

    video_progress_bar_update(0, 0);
    video_play_stop();
    layout_memory_video_load();
    memory_video_user_activity_reset();
}

static void video_next_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(video_next_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_MEMORY_NEXT_PNG);
    lv_obj_t *btn = camera_img_btn_create(parent, video_btn_area[MEMORY_NEXT_BTN_ID], NULL, &btn_data, &info);
    lv_obj_set_id(btn, MEMORY_NEXT_BTN_ID);
}

static void create_dim_mask()
{
    if (dim_mask != NULL)
    {
        lv_obj_del(dim_mask);
        dim_mask = NULL;
    }

    dim_mask = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_size(dim_mask, 1024, 600);
    lv_obj_align(dim_mask, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_obj_set_style_local_bg_color(dim_mask, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    lv_obj_set_style_local_bg_opa(dim_mask, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
    lv_obj_set_click(dim_mask, false);
}

static void video_delete_yes_btn_up(lv_obj_t *obj)
{
    memory_video_user_activity_reset();
    video_play_stop();
    memory_video_finish_handled = false;
    memory_video_delete_dialog_active = false;

    if (dim_mask != NULL)
    {
        lv_obj_del(dim_mask);
        dim_mask = NULL;
    }

    // 修复：删除时使用实际播放索引
    int real_play_index = video_total - 1 - video_index;
    media_file_delete(FILE_TYPE_VIDEO, real_play_index);

    printf("====================删除索引：显示[%d]，实际[%d]\n", video_index, real_play_index);

    // 删除后重置显示索引（默认回到最新视频）
    video_total--;
    if (video_total <= 0)
    {
        video_index = 0;
    }
    else if (video_index >= video_total)
    {
        video_index = video_total - 1;
    }

    video_index_set(video_index);
    goto_layout(pLAYOUT(memory_video));
}

static void memory_bg_btn_up(lv_obj_t *obj)
{
    memory_video_user_activity_reset();
    if (video_total <= 0)
        return;

    VIDEO_PLAY_STATUS status = video_play_status_get();

    if (status == VIDEO_PLAY_STATE_PLAY)
    {
        memory_func_btn_diaplay_enable(!func_btn_diaplay_flag);
        video_play_btn_state_display(true);
        standby_timer_close();
        memory_video_timeout_task_stop();
        return;
    }

    if (status == VIDEO_PLAY_STATE_PAUSE)
    {
        memory_func_btn_diaplay_enable(true);
        video_play_btn_state_display(false);
        return;
    }

    if (status == VIDEO_PLAY_STATE_IDLE)
    {
        // 修复：实际播放索引 = 总数-1-显示索引
        int real_play_index = video_total - 1 - video_index;
        const file_info *pinfo = media_file_info_get(FILE_TYPE_VIDEO, real_play_index);
        char file[128] = {0};
        strcpy(file, SD_VIDEO_PATH);
        strcat(file, pinfo->file_name);
        video_play_start(file);
    }
    if (video_play_status_get() == VIDEO_PLAY_STATE_PLAY)
    {
        memory_func_btn_diaplay_enable(false);
        standby_timer_close();
        memory_video_timeout_task_stop();
        video_play_btn_state_display(true);
    }
    else
    {
        memory_func_btn_diaplay_enable(true);
        video_play_btn_state_display(false);
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

static void video_delete_no_btn_up(lv_obj_t *obj)
{
    memory_video_user_activity_reset();
    memory_video_delete_dialog_active = false;
    memory_bg_btn_click_enable(true);
    if (dim_mask != NULL)
    {
        lv_obj_del(dim_mask);
        dim_mask = NULL;
    }
    if (memory_video_delete_box != NULL)
    {
        lv_obj_t *delete_box = memory_video_delete_box;
        memory_video_delete_box = NULL;
        lv_obj_del(delete_box);
    }

    memory_video_btn_click_set(MEMORY_PREV_BTN_ID, true);
    memory_video_btn_click_set(MEMORY_PLAY_BTN_ID, true);
    memory_video_btn_click_set(MEMORY_NEXT_BTN_ID, true);
    memory_video_btn_click_set(MEMORY_DELETE_BTN_ID, true);
    memory_video_btn_click_set(MEMORY_HOME_BTN_ID, true);
    video_play_btn_state_display(false);
    memory_video_user_activity_reset();
    memory_video_play_state_task_start();
}

static void video_delete_btn_up(lv_obj_t *obj)
{
    memory_video_user_activity_reset();
    if (video_total == 0 || memory_video_delete_dialog_active == true)
        return;

    memory_video_delete_dialog_active = true;
    memory_video_timeout_task_stop();
    memory_video_finish_handled = false;
    memory_video_play_state_task_stop();

    video_play_stop();
    video_input_resident_bzero();
    layout_memory_video_load();
    memory_func_btn_diaplay_enable(true);
    video_play_btn_state_display(false);

    memory_bg_btn_click_enable(false);

    memory_video_btn_click_set(MEMORY_PREV_BTN_ID, false);
    memory_video_btn_click_set(MEMORY_PLAY_BTN_ID, false);
    memory_video_btn_click_set(MEMORY_NEXT_BTN_ID, false);
    memory_video_btn_click_set(MEMORY_DELETE_BTN_ID, false);
    memory_video_btn_click_set(MEMORY_HOME_BTN_ID, false);

    create_dim_mask();
    static obj_click_data btn_data = obj_click_data_up_create(video_delete_yes_btn_up);
    static obj_click_data btn_data1 = obj_click_data_up_create(video_delete_no_btn_up);
    memory_video_delete_box = memory_delete_box_create(lv_scr_act(), &btn_data, &btn_data1, LAYOUT_MEMORY_LANG_DELETE_VIDEO_ID);
    lv_obj_move_foreground(dim_mask);
    lv_obj_move_foreground(memory_video_delete_box);
}

static void video_delete_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(video_delete_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_MEMORY_DELETE_PNG);
    lv_obj_t *btn = camera_img_btn_create(parent, video_btn_area[MEMORY_DELETE_BTN_ID], NULL, &btn_data, &info);
    lv_obj_set_id(btn, MEMORY_DELETE_BTN_ID);
}

static void layout_play_state_task(lv_task_t *task_t)
{
    if (task_t != memory_video_play_state_task)
    {
        lv_task_del(task_t);
        return;
    }

    if (memory_video_delete_dialog_active == true)
    {
        return;
    }

    int cur = -1, total = -1;
    static VIDEO_PLAY_STATUS prev_statu = VIDEO_PLAY_STATE_IDLE;
    VIDEO_PLAY_STATUS statu = video_play_status_get();
    bool statu_changed = (prev_statu != statu);
    bool duration_valid = video_play_duration_get(&cur, &total);
    bool play_finished = video_play_eof_check();

    if (play_finished == true && total > 0)
    {
        video_head_play_time_label_display(total / 1000, total / 1000);
        video_progress_bar_update(total, total);
    }
    else if (cur != total && cur != -1 && total != -1)
    {
        video_head_play_time_label_display(cur / 1000, total / 1000);
        video_progress_bar_update(cur, total);
    }
    else if (total != -1)
    {
        video_head_play_time_label_display(0, total / 1000);
        video_progress_bar_update(0, 0);
    }

    if (play_finished == true && memory_video_finish_handled == false)
    {
        memory_video_finish_handled = true;
        prev_statu = statu;
        video_play_stop();
        layout_memory_video_load();
        video_play_btn_state_display(false);
        memory_func_btn_diaplay_enable(true);
        memory_video_user_activity_reset();
        return;
    }

    if (statu == VIDEO_PLAY_STATE_PLAY)
    {
        memory_video_finish_handled = false;
        prev_statu = statu;
        if (statu_changed)
        {
            video_play_btn_state_display(true);
        }
        standby_timer_close();
        return;
    }

    if (statu_changed)
    {
        prev_statu = statu;
        video_play_btn_state_display(false);
        memory_func_btn_diaplay_enable(true);
        memory_video_user_activity_reset();
    }
}

static void layout_memory_video_load(void)
{
    thumb_media_buffer_clear();
    // 修复：实际播放索引 = 总数-1-显示索引
    int real_play_index = video_total - 1 - video_index;
    const file_info *pinfo = media_file_info_get(FILE_TYPE_VIDEO, real_play_index);
    printf("=================>> 显示索引:[%d]   实际播放索引:[%d]   文件名:[%s] \n", video_index, real_play_index, pinfo->file_name);

    char file[128] = {0};
    strcpy(file, SD_VIDEO_PATH);
    strcat(file, pinfo->file_name);
    thumb_media_load(0, 0, 1024, 600, file);

    if (pinfo->is_new == true)
    {
        media_file_new_clear(pinfo->type, real_play_index); // 修复：使用实际索引
    }

    int total_duration_ms = video_get_duration_without_play(file);
    video_head_info_label_display(pinfo);
    video_head_play_time_label_display(0, total_duration_ms / 1000);
    video_play_btn_state_display(false);
    lv_obj_invalidate(lv_scr_act());
}

static void memory_video_param_init(void)
{
    memory_video_timeout_task = NULL;
    if (media_sdcard_insert_check() == true)
    {
        sd_media_all_file_total_get(&media_total, NULL);
        printf("====================>> sd卡媒体文件总数:[%d]\n", media_total);
        media_file_total_get(FILE_TYPE_VIDEO, &video_total, NULL);
        printf("====================>> 视频文件总数:[%d]\n", video_total);
    }
    else
    {
        media_total = 0;
        video_total = 0;
    }

    if (video_total == 0)
    {
        goto_layout(pLAYOUT(photo_list));
        return;
    }
    else
    {
        lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    }

    printf("--------------video_index_get()=[%d]\n", video_index_get());
    // 修复：显示索引默认从0开始（对应最新视频）
    if (video_index_get() < 0 || video_index_get() >= video_total)
    {
        video_index = 0; // 显示0001 → 最新视频
    }
    else
    {
        video_index = video_index_get();
    }
    printf("----------显示索引video_index----[%d]，实际播放索引[%d]\n", video_index, video_total - 1 - video_index);

    thumb_media_open();
    layout_memory_video_load();

    memory_video_play_state_task_start();
    memory_video_user_activity_reset();
}

static void memory_video_sdcard_state_change_event_cb(void)
{
    photo_index_reset();
    video_index_reset();
    if (media_sdcard_insert_check() == true)
    {
        goto_layout(pLAYOUT(memory_video));
    }
    else
    {
        if (memory_video_sdcard_removing == true)
        {
            return;
        }

        memory_video_sdcard_removing = true;
        memory_bg_btn_click_enable(false);
        memory_video_timeout_task_stop();
        memory_video_play_state_task_stop();

        /* Stop all SD-backed playback before the layout is destroyed. */
        video_play_stop();
        video_input_resident_bzero();
        goto_layout(pLAYOUT(memory_video));
    }
}

static void memory_video_click_down_func(lv_obj_t *obj)
{
    if (video_play_status_get() == VIDEO_PLAY_STATE_PLAY)
    {
        return;
    }
    layout_obj_click_down_func(obj);
}

static void LAYOUT_ENTER_FUNC(memory_video)
{
    lv_obj_click_down_callback_register(memory_video_click_down_func);
    printf("come in memory_video\n");
    memory_video_sdcard_removing = false;

    lv_obj_t *parent = lv_scr_act();
    video_head_label_create(parent);
    video_progress_bar_create(parent);

    video_home_btn_create(parent);
    video_prev_btn_create(parent);
    video_play_btn_create(parent);
    video_next_btn_create(parent);
    video_delete_btn_create(parent);

    memory_bg_btn_click_enable(true);

    layout_sd_state_callback_register(memory_video_sdcard_state_change_event_cb);

    memory_video_param_init();
}

static void LAYOUT_QUIT_FUNC(memory_video)
{
    lv_obj_click_down_callback_register(layout_obj_click_down_func);
    memory_bg_btn_click_enable(false);
    memory_video_timeout_task_stop();
    memory_video_play_state_task_stop();

    // 删除挂在 lv_scr_act 上的弹窗对象，避免布局切换后残留导致下次进入崩溃
    if (dim_mask != NULL)
    {
        lv_obj_del(dim_mask);
    }
    if (memory_video_delete_box != NULL)
    {
        lv_obj_del(memory_video_delete_box);
    }

    memory_video_delete_dialog_active = false;
    dim_mask = NULL;
    memory_video_delete_box = NULL;
    memory_video_timeout_task = NULL;
    memory_video_play_state_task = NULL;
    memory_video_finish_handled = false;
    video_play_stop();

    if (cur_layout_get() == pLAYOUT(camera))
    {
        backlight_enable(false);
    }

    lv_obj_set_style_local_value_str(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "");
    layout_sd_state_callback_register(layout_sdcard_state_change_default);
    standby_timer_restart(true);

    if (cur_layout_get() != pLAYOUT(photo_list) && cur_layout_get() != pLAYOUT(memory_video))
    {
        photo_index_reset();
        video_index_reset();
        video_index_set(0);
    }
    thumb_media_close();
}

CREATE_LAYOUT(memory_video);

#include "layout_define.h"
#include "media_thumb.h"

// 按钮ID定义
#define PHOTO_LIST_HOME_BTN_ID 1
#define PHOTO_LIST_FLASH_PHOTO_BTN_ID 2
#define PHOTO_LIST_SD_PHOTO_BTN_ID 3
#define PHOTO_LIST_VIDEO_BTN_ID 4
#define PHOTO_LIST_PHOTO_PAGE_BTN_ID 5
#define PHOTO_LIST_PHOTO_TOTAL_BTN_ID 6
#define PHOTO_LIST_DELETE_BTN_ID 7
#define VIDEO_LIST_PHOTO_PAGE_BTN_ID 8
#define VIDEO_LIST_PHOTO_TOTAL_BTN_ID 9

// 按钮枚举
typedef enum
{
    PHOTO_LIST_HOME_BTN,
    PHOTO_LIST_FLASH_PHOTO_BTN,
    PHOTO_LIST_SD_PHOTO_BTN,
    PHOTO_LIST_VIDEO_BTN,
    PHOTO_LIST_DELETE_BTN,
    PHOTO_LIST_TOTAL_BTN,
} media_btn_module;
static custom_area media_btn_area[PHOTO_LIST_TOTAL_BTN] =
{
    {920, 25, 50, 37},         // HOME按钮
    {0, 0, 218, 168},          // Flash照片按钮
    {0, 168, 218, 168},        // SD照片按钮
    {0, 336, 218, 168},        // Video按钮
    {845, 20, 48, 48},         // 删除按钮
};

static lv_obj_t *parent_bg = NULL;

// 照片相关变量
static file_type file_type_photo = FILE_TYPE_FLASH_PHOTO;
static lv_obj_t *photo_page = NULL;
static char photo_file_path[20] = {0};
static int photo_list_curr_page_num = 1;
static int media_total = 0;
static int exit_time_count = 0;
static int photo_index = 0;
static int photo_total;
static int last_selected_btn_id=PHOTO_LIST_FLASH_PHOTO_BTN_ID;

// 视频相关变量
static int video_total;
static char *cur_video_path = SD_MEDIA_PATH;
static file_type video_file_type = FILE_TYPE_VIDEO;
static int video_index = 0;
static int video_list_curr_page_num = 1;

// 通用删除相关
static file_type delete_file_type = FILE_TYPE_NONE;
static int start_angle = 270;// 加载圆弧的起始角度

// UI容器相关
static lv_obj_t *main_func_cont = NULL;    
static lv_obj_t *media_switch_cont = NULL;
static lv_obj_t *flash_photo_btn = NULL;
static lv_obj_t *sd_photo_btn = NULL;
static lv_obj_t *video_btn = NULL;
static lv_obj_t *video_page = NULL;
static lv_obj_t *no_sd_label = NULL;


// 照片/视频项创建函数声明
static lv_obj_t *photo_list_page_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int file_global_index);
static lv_obj_t *video_list_page_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int file_global_index);

static void list_no_sd_create(lv_obj_t *parent);

// 分页显示/SD卡提示函数声明
static void playback_total_label_display(int type);

/************************** 照片相关接口函数 **************************/
file_type photo_state_get(void){
    return file_type_photo;
}


int photo_index_get(void)
{
    return photo_index;
}

void photo_index_set(int index)
{
    photo_index = index;
}

int photo_total_get(void)
{
    return photo_total;
}

void photo_total_set(int index)
{
    photo_total = index;
}

/************************** 视频相关接口函数 **************************/
int video_index_get(void)
{
    return video_index;
}

void video_index_set(int index)
{
    video_index = index;
}

void video_total_set(int total)
{
    video_total = total;
}

int video_total_get(void)
{
    return video_total;
}

static void media_btn_style_reset(void)
{
    lv_color_t default_bg = lv_color_hex(0x616688);
    if(flash_photo_btn) {
        lv_obj_set_style_local_bg_color(flash_photo_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, default_bg);
        lv_obj_set_style_local_bg_opa(flash_photo_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
    }
    if(sd_photo_btn) {
        lv_obj_set_style_local_bg_color(sd_photo_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, default_bg);
        lv_obj_set_style_local_bg_opa(sd_photo_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
    }
    if(video_btn) {
        lv_obj_set_style_local_bg_color(video_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, default_bg);
        lv_obj_set_style_local_bg_opa(video_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
    }
}

static void set_selected_media_btn(lv_obj_t *selected_btn)
{
    media_btn_style_reset();
    if(selected_btn) {
        lv_obj_set_style_local_bg_color(selected_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x007AFF));
        lv_obj_set_style_local_bg_opa(selected_btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    }
}

//通用页面按钮创建函数
lv_obj_t *page_btn_create(lv_obj_t *parent, custom_area btn_area, const char *string, obj_click_data *btn_pdata, const void *icon_src)
{
    lv_obj_t *btn_obj = lv_obj_create(parent, NULL);
    lv_obj_set_click(btn_obj, false);
    lv_obj_set_pos(btn_obj, btn_area.x, btn_area.y);
    lv_obj_set_size(btn_obj, btn_area.w, btn_area.h);
    
    lv_obj_set_style_local_radius(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_border_width(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    if (icon_src != NULL)
    {
        lv_obj_set_style_local_pattern_image(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, icon_src);
    }

    lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
    lv_obj_set_style_local_pattern_recolor(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
    lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_0);
    lv_obj_set_style_local_pattern_recolor_opa(btn_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);

    if (string != NULL)
    {
        lv_obj_set_style_local_value_str(btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, string);
        lv_obj_set_style_local_value_align(btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
        lv_obj_set_style_local_value_ofs_y(btn_obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, -40);
        lv_obj_set_style_local_value_font(btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
        lv_obj_set_style_local_value_blend_mode(btn_obj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_BLEND_MODE_ADDITIVE);
    }

    obj_click_event_listen(btn_obj, btn_pdata);
    return btn_obj;
}

/************************** 照片初始化函数 **************************/
void photo_play_parameter_init(void)
{
    photo_total = 0;
    if(file_type_photo == FILE_TYPE_FLASH_PHOTO)
    {
        sprintf(photo_file_path,"%s",FLASH_PHOTO_PATH);
        media_file_total_get(file_type_photo,&media_total,NULL);
        printf("===================>>flash媒体文件总数:[%d]\n",media_total);
    }
    else if(file_type_photo == FILE_TYPE_PHOTO)
    {
        if(media_sdcard_insert_check()==true)
        {
            sprintf(photo_file_path,"%s",SD_PHOTO_PATH);
            sd_media_all_file_total_get(&media_total,NULL);
            printf("===================>>SDCARD媒体文件总数:[%d]\n",media_total);
        }
        else
        {
            media_total=0;
        }
    }
    media_file_total_get(file_type_photo,&photo_total,NULL);
    printf("=======================>>当前介质图片文件总数:[%d]\n", photo_total);
}

/************************** 视频初始化函数 **************************/
void video_play_parameter_init(void)
{
    if (media_sdcard_insert_check() == true)
    {
        cur_video_path = SD_VIDEO_PATH;
        video_file_type = FILE_TYPE_VIDEO;
        media_file_total_get(video_file_type, &video_total, NULL);
        printf("==================================[视频总数:%d]\n", video_total);
    }
    else
    {
        video_total = 0;
    }
    video_list_curr_page_num = 1;
}

/************************** 照片按钮回调 **************************/
static void photo_list_flash_photo_btn_up(lv_obj_t *obj)
{
    file_type_photo=FILE_TYPE_FLASH_PHOTO;
    photo_list_curr_page_num = 1;
    photo_play_parameter_init();
    // 先更新选中按钮ID
    last_selected_btn_id=PHOTO_LIST_FLASH_PHOTO_BTN_ID;

    if(no_sd_label != NULL) lv_obj_del(no_sd_label), no_sd_label = NULL;
    if(photo_page!=NULL)
    {
        lv_obj_clean(photo_page);
        // 隐藏视频页面，显示照片页面
        if(video_page) lv_obj_set_hidden(video_page, true);
        lv_obj_set_hidden(photo_page, false);
        
        for(int i=0;i<6;i++)
        {
            int file_global_index = (photo_list_curr_page_num-1)*6+i;
            if(file_global_index>=photo_total) break;
            photo_list_page_btn_create(photo_page,0,i*62,800,62,file_global_index);
        }
    }
    set_selected_media_btn(flash_photo_btn);
    playback_total_label_display(0); // 0表示照片类型
}

static void photo_list_sd_photo_btn_up(lv_obj_t *obj)
{
    if (media_sdcard_insert_check() == false)
	{
		list_no_sd_create(parent_bg);
	}
    else
    {
        // SD卡已插入，确保提示被删除
        if(no_sd_label != NULL) {
            lv_obj_del(no_sd_label);
            no_sd_label = NULL;
        }
    }
    
    file_type_photo = FILE_TYPE_PHOTO;
    photo_list_curr_page_num = 1;
    photo_play_parameter_init();
    
    // 先更新选中按钮ID
    last_selected_btn_id=PHOTO_LIST_SD_PHOTO_BTN_ID;
    
    if(photo_page != NULL)
    {
        lv_obj_clean(photo_page);
        // 隐藏视频页面，显示照片页面
        if(video_page) lv_obj_set_hidden(video_page, true);
        lv_obj_set_hidden(photo_page, false);
        
        for(int i=0;i<6;i++){
            int file_global_index = (photo_list_curr_page_num - 1) * 6 + i;
            if (file_global_index >= photo_total) break;
            photo_list_page_btn_create(photo_page, 0, i * 62, 800, 62, file_global_index);
        }
    }
    set_selected_media_btn(sd_photo_btn);
    playback_total_label_display(0); // 0表示照片类型
}

static void photo_list_home_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(home));
}

/************************** 视频按钮回调 **************************/
static void photo_list_video_btn_up(lv_obj_t *obj)
{
    if (media_sdcard_insert_check() == false)
	{
		list_no_sd_create(parent_bg);
	}
    else
    {
        // SD卡已插入，确保提示被删除
        if(no_sd_label != NULL) {
            lv_obj_del(no_sd_label);
            no_sd_label = NULL;
        }
    }

    video_play_parameter_init();
    
    // 先更新选中按钮ID
    last_selected_btn_id=PHOTO_LIST_VIDEO_BTN_ID;
    
    if(video_page!=NULL)
    {
        lv_obj_clean(video_page);
        // 隐藏照片页面，显示视频页面
        if(photo_page) lv_obj_set_hidden(photo_page, true);
        lv_obj_set_hidden(video_page, false);
        
        for(int i=0;i<6;i++){
            int file_global_index = (video_list_curr_page_num - 1) * 6 + i;
            if (file_global_index >= video_total) break;
            video_list_page_btn_create(video_page, 0, i * 62, 800, 62, file_global_index);
        }
    }
    set_selected_media_btn(video_btn);
    playback_total_label_display(1); // 1表示视频类型
}

static void photo_list_btn_up(lv_obj_t *obj)
{
    photo_index = obj->obj_id;
    printf("==========@@@@@@@@==[照片索引:%d]======\n", photo_index);
    if (photo_total == 0)
    {
        photo_index = -1;
    }
    goto_layout(pLAYOUT(memory_photo));
}

static void video_list_btn_up(lv_obj_t *obj)
{
    video_index = obj->obj_id;
    printf("==========@@@@@@@@==[视频索引:%d]======\n", video_index);
    if (video_total == 0)
    {
        video_index = -1;
    }
    goto_layout(pLAYOUT(memory_video));
}

/************************** 通用媒体项创建函数**************************/
typedef enum {
    MEDIA_TYPE_PHOTO,
    MEDIA_TYPE_VIDEO
} media_item_type;

static lv_obj_t *media_list_page_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int file_global_index, media_item_type media_type)
{
    int total = (media_type == MEDIA_TYPE_PHOTO) ? photo_total : video_total;
    file_type cur_file_type = (media_type == MEDIA_TYPE_PHOTO) ? file_type_photo : video_file_type;
    const file_info *media_info = NULL;
    obj_click_data *click_data = NULL;

    static obj_click_data photo_click_data = obj_click_data_up_create(photo_list_btn_up);
    static obj_click_data video_click_data = obj_click_data_up_create(video_list_btn_up);
    click_data = (media_type == MEDIA_TYPE_PHOTO) ? &photo_click_data : &video_click_data;

    printf("当前页内序号：%d | 全局索引：%d\n", file_global_index % 6, file_global_index);
    lv_obj_t *btn = lv_obj_create(parent, NULL);
    lv_obj_set_id(btn, file_global_index);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x4f4f4f));
    lv_obj_set_style_local_radius(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    obj_click_event_listen(btn, click_data);

    int real_file_index = total - 1 - file_global_index;
    if (real_file_index < 0 || real_file_index >= total) {
        lv_obj_del(btn);
        return NULL;
    }
    media_info = media_file_info_get(cur_file_type, real_file_index); 
    if (media_info == NULL) {
        lv_obj_del(btn);
        return NULL;
    }
    printf("当前显示的文件索引：%d      %d\n", real_file_index, media_info->is_new);

    // lv_color_t new_file_color_def = lv_color_hex(0xC52323);
    // lv_color_t normal_color_def = lv_color_hex(0xFFFFFF);
    // lv_color_t curr_color_def = (media_info->is_new) ? new_file_color_def : normal_color_def;
    lv_color_t text_color=lv_color_hex(0xFFFFFF);

    char year[32] = {0};
    char month[32] = {0};
    char day[32] = {0};
    char hour[32] = {0};
    char min[32] = {0};
    char second[32] = {0};
    static char file_year[64] = {0};
    strncpy(year, media_info->file_name, 2);
    strncpy(month, media_info->file_name + 2, 2);
    strncpy(day, media_info->file_name + 4, 2);
    strncpy(hour, media_info->file_name + 7, 2);
    strncpy(min, media_info->file_name + 9, 2);
    strncpy(second, media_info->file_name + 11, 2);

    struct date temp_date = {
        .year = atoi(year) + 2000,
        .month = atoi(month),
        .day = atoi(day)
    };
    temp_date = gregorian2jalali(temp_date);

    if (user_data_get()->setting.calendar == 1) {
        sprintf(file_year, "20%s-%s-%s  %s:%s:%s", year, month, day, hour, min, second);
    } else {
        sprintf(file_year, "%d-%02d-%02d  %s:%s:%s",
                temp_date.year, temp_date.month, temp_date.day, hour, min, second);
    }

    lv_obj_set_style_local_value_align(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
    lv_obj_set_style_local_value_ofs_x(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 73);
    lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24)); 

    static rom_bin_info red_info=rom_bin_info_get(ROM_UI_MEDIA_RED_PNG);
    lv_obj_t *red_dot_img=lv_img_create(btn,NULL);
    lv_img_set_src(red_dot_img,&red_info);
    lv_obj_align(red_dot_img,btn,LV_ALIGN_IN_LEFT_MID,15,0);
    lv_obj_set_hidden(red_dot_img,!media_info->is_new);

    lv_obj_t *obj = lv_obj_create(btn, NULL);
    if (media_info->ch == MON_CH_DOOR1) {
        lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_DOOR1_ID));
    } else if (media_info->ch == MON_CH_DOOR2) {
        lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_DOOR2_ID));
    } else if (media_info->ch == MON_CH_CCTV1) {
        lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_CCTV1_ID));
    } else if (media_info->ch == MON_CH_CCTV2) {
        lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_CCTV2_ID));
    }
    lv_obj_set_style_local_value_color(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, text_color);
    lv_obj_set_style_local_value_font(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    
    // 通道文本对齐
    if (user_data_get()->setting.language == LANG_ENGLISH) {
        lv_obj_set_style_local_value_align(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
    } else {
        lv_obj_set_style_local_value_align(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);
    }
    lv_obj_align(obj, btn, LV_ALIGN_IN_LEFT_MID, 390, 5);
    lv_obj_set_click(obj, false);

    // 时间标签
    lv_obj_t *label2 = lv_label_create(btn, NULL);
    lv_label_set_text(label2, file_year);
    lv_label_set_align(label2, LV_LABEL_ALIGN_LEFT);
    lv_obj_set_style_local_text_color(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, text_color);
    lv_obj_align(label2, btn, LV_ALIGN_IN_LEFT_MID, 30, 0);
    lv_obj_set_style_local_text_font(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

    return btn;
}
static lv_obj_t *photo_list_page_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int file_global_index)
{
    return media_list_page_btn_create(parent, x, y, w, h, file_global_index, MEDIA_TYPE_PHOTO);
}
static lv_obj_t *video_list_page_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int file_global_index)
{
    return media_list_page_btn_create(parent, x, y, w, h, file_global_index, MEDIA_TYPE_VIDEO);   
}

/************************** 通用分页显示函数**************************/
static void playback_total_label_display(int type) // type:0=照片，1=视频
{
    static char str[20];
    lv_obj_t *label = NULL;
    int curr_page = 0;
    int total = 0;
    
    // 根据类型获取对应的标签和分页信息
    if(type == 0) {
        label = lv_obj_get_child_form_id(main_func_cont, PHOTO_LIST_PHOTO_TOTAL_BTN_ID);
        curr_page = photo_list_curr_page_num;
        total = photo_total;
    } else {
        label = lv_obj_get_child_form_id(main_func_cont, PHOTO_LIST_PHOTO_TOTAL_BTN_ID); // 复用同一个标签
        curr_page = video_list_curr_page_num;
        total = video_total;
    }

    int total_page_num = 0;
    if (total > 0) {
        total_page_num = (total - 1) / 6 + 1;
    }
    if (curr_page < 1) {
        curr_page = 1;
    } else if (curr_page > total_page_num) {
        curr_page = total_page_num;
    }
    sprintf(str, "%04d/%04d", curr_page, total_page_num);
    lv_label_set_text(label, str);
}

/************************** 照片分页按钮回调 **************************/
static void photo_list_prev_btn_up(lv_obj_t *obj)
{
    // 动态判断当前是照片还是视频模式，统一处理分页
    if(last_selected_btn_id == PHOTO_LIST_VIDEO_BTN_ID) {
        int total_page_num = (video_total > 0) ? ((video_total - 1) / 6 + 1) : 0;
        if (total_page_num == 0) return;

        if (video_list_curr_page_num <= 1) {
            video_list_curr_page_num = total_page_num;
        } else {
            video_list_curr_page_num--;
        }

        playback_total_label_display(1);
        if (video_page != NULL) {
            lv_obj_clean(video_page);
            for (int i = 0; i < 6; i++)
            {
                int file_global_index = (video_list_curr_page_num - 1) * 6 + i;
                if (file_global_index >= video_total) break;
                int real_file_index = video_total - 1 - file_global_index;
                if(real_file_index >=0 && real_file_index < video_total){
                    video_list_page_btn_create(video_page, 0, i * 62, 800, 62, file_global_index);
                }
            }
        }
    } else {
        int total_page_num = (photo_total > 0) ? ((photo_total - 1) / 6 + 1) : 0;
        if (total_page_num == 0) return;

        if (photo_list_curr_page_num <= 1) {
            photo_list_curr_page_num = total_page_num;
        } else {
            photo_list_curr_page_num--;
        }
        playback_total_label_display(0);
        if (photo_page != NULL) {
            lv_obj_clean(photo_page);
            for (int i = 0; i < 6; i++)
            {
                int file_global_index = (photo_list_curr_page_num - 1) * 6 + i;
                if (file_global_index >= photo_total) break;
                int real_file_index = photo_total - 1 - file_global_index;
                if(real_file_index >=0 && real_file_index < photo_total){
                    photo_list_page_btn_create(photo_page, 0, i * 62, 800, 62, file_global_index);
                }
            }
        }
    }
}

// 下一页按钮
static void photo_list_next_btn_up(lv_obj_t *obj)
{
    // 动态判断当前是照片还是视频模式，统一处理分页
    if(last_selected_btn_id == PHOTO_LIST_VIDEO_BTN_ID) {
        int total_page_num = (video_total > 0) ? ((video_total - 1) / 6 + 1) : 0;
        if (total_page_num == 0) return;

        if (video_list_curr_page_num >= total_page_num) {
            video_list_curr_page_num = 1;
        } else {
            video_list_curr_page_num++;
        }

        playback_total_label_display(1);
        if (video_page != NULL) {
            lv_obj_clean(video_page);
            for (int i = 0; i < 6; i++)
            {
                int file_global_index = (video_list_curr_page_num - 1) * 6 + i;
                if (file_global_index >= video_total) break;
                int real_file_index = video_total - 1 - file_global_index;
                if(real_file_index >=0 && real_file_index < video_total){
                    video_list_page_btn_create(video_page, 0, i * 62, 800, 62, file_global_index);
                }
            }
        }
    } else {
        int total_page_num = (photo_total > 0) ? ((photo_total - 1) / 6 + 1) : 0;
        if (total_page_num == 0) return;

        if (photo_list_curr_page_num >= total_page_num) {
            photo_list_curr_page_num = 1;
        } else {
            photo_list_curr_page_num++;
        }

        playback_total_label_display(0);
        if (photo_page != NULL) {
            lv_obj_clean(photo_page);
            for (int i = 0; i < 6; i++)
            {
                int file_global_index = (photo_list_curr_page_num - 1) * 6 + i;
                if (file_global_index >= photo_total) break;
                int real_file_index = photo_total - 1 - file_global_index;
                if(real_file_index >=0 && real_file_index < photo_total){
                    photo_list_page_btn_create(photo_page, 0, i * 62, 800, 62, file_global_index);
                }
            }
        }
    }
}

/************************** 页面显示函数 **************************/
static void media_list_page_display(lv_obj_t *parent)
{
    // 创建照片页面
    photo_page = lv_cont_create(main_func_cont, NULL);
    lv_obj_set_id(photo_page, PHOTO_LIST_PHOTO_PAGE_BTN_ID);
    lv_obj_set_pos(photo_page, 285, 56);
    lv_obj_set_size(photo_page, 533, 380);

    // 创建视频页面
    video_page = lv_cont_create(main_func_cont, NULL);
    lv_obj_set_id(video_page, VIDEO_LIST_PHOTO_PAGE_BTN_ID);
    lv_obj_set_pos(video_page, 285, 56); // 和照片页面同位置
    lv_obj_set_size(video_page, 533, 380);
    lv_obj_set_hidden(video_page, true); // 默认隐藏视频页面

    // 初始化照片页面
    if (photo_index_get() < 0)
    {
        photo_index_set(0);
    }
    photo_list_curr_page_num = 1;
    printf("当前页：%d | 照片总数：%d\n", photo_list_curr_page_num, photo_total);
    for (int i = 0; i < 6; i++)
    {
        int file_global_index = (photo_list_curr_page_num - 1) * 6 + i;
        if (file_global_index >= photo_total) {
            break;
        }
        photo_list_page_btn_create(photo_page, 0, i * 62, 800, 62, file_global_index);
    }

    // 创建通用分页按钮
    // if(photo_total>0 || video_total>0){
        // 上一页按钮
        lv_obj_t *prev_btn = lv_obj_create(main_func_cont, NULL);
        static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_PREV_PNG);
        lv_obj_set_pos(prev_btn, 420, 460);
        lv_obj_set_size(prev_btn, 40, 40);
        lv_obj_set_click(prev_btn,true);
        lv_obj_set_style_local_pattern_align(prev_btn,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_ALIGN_CENTER);
        lv_obj_set_style_local_pattern_image(prev_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &info);
        static obj_click_data btn_data1 = obj_click_data_up_create(photo_list_prev_btn_up);
        obj_click_event_listen(prev_btn, &btn_data1);

        // 下一页按钮
        lv_obj_t *next_btn = lv_obj_create(main_func_cont, NULL);
        static rom_bin_info info1 = rom_bin_info_get(ROM_UI_MEDIA_LIST_NEXT_PNG);
        lv_obj_set_pos(next_btn, 656, 460);
        lv_obj_set_size(next_btn, 40, 40);
        lv_obj_set_click(next_btn,true);
        lv_obj_set_style_local_pattern_align(next_btn,LV_CONT_PART_MAIN,LV_STATE_DEFAULT,LV_ALIGN_CENTER);
        lv_obj_set_style_local_pattern_image(next_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &info1);
        static obj_click_data btn_data2 = obj_click_data_up_create(photo_list_next_btn_up);
        obj_click_event_listen(next_btn, &btn_data2);
    // }
}


/************************** 删除相关函数**************************/
static void delete_file_state_display_task(lv_task_t *task_t)
{
    lv_obj_t *arc = (lv_obj_t *)task_t->user_data;
    arc_loader_update(arc, &start_angle);
    if (delete_file_type != FILE_TYPE_NONE)
    {
        if (!media_file_delete_all_state())
        {
            delete_file_type = FILE_TYPE_NONE;
            exit_time_count = 1;
        }
    }
    else
    {
        if (--exit_time_count <= 0)
        {
            arc_loader_delete(arc);
            goto_layout(pLAYOUT(photo_list));
        }
    }
}

// 照片删除确认
static void photo_list_delete_yes_btn_up(lv_obj_t *obj)
{
    setting_msgdialog_msg_bg_delete(LAYOUT_MEMORY_LANG_DELETE_ALL_PICTURE_ID);
    lv_obj_t *cont = setting_msgdialog_msg_bg_create(0, 100, 100);
    lv_arc_loader_create(
        cont,
        delete_file_state_display_task,
        120,
        lv_color_hex(0xFFFFFF),
        lv_color_hex(0x007AFF),
        11,
        50,
        0, 0);
    if (last_selected_btn_id == PHOTO_LIST_FLASH_PHOTO_BTN_ID) {
        delete_file_type = FILE_TYPE_FLASH_PHOTO;
    } else if (last_selected_btn_id == PHOTO_LIST_SD_PHOTO_BTN_ID) {
        if (media_sdcard_insert_check() == false) {
            delete_file_type = FILE_TYPE_NONE;
            exit_time_count = 1;
            printf("SD卡未插入，无法删除SD照片！\n");
            return;
        }
        delete_file_type = FILE_TYPE_PHOTO;
    } else {
        delete_file_type = FILE_TYPE_FLASH_PHOTO;
    }
    int total;
    media_file_total_get(delete_file_type, &total, NULL);
    if (total == 0)
    {
        delete_file_type = FILE_TYPE_NONE;
        exit_time_count = 1;
    }else{
        exit_time_count = 1;
        media_file_delete_all_start(delete_file_type);
    }
    user_data_get()->new_media_file_flag = 0;
}

// 视频删除确认
static void video_list_delete_yes_btn_up(lv_obj_t *obj)
{
    setting_msgdialog_msg_bg_delete(LAYOUT_MEMORY_LANG_DELETE_ALL_VIDEO_ID);
    lv_obj_t *cont = setting_msgdialog_msg_bg_create(0, 100, 100);
    lv_arc_loader_create(
        cont,
        delete_file_state_display_task,
        120,
        lv_color_hex(0xFFFFFF),
        lv_color_hex(0x007AFF),
        11,
        50,
        0, 0);
    if (media_sdcard_insert_check() == true)
    {
        delete_file_type = FILE_TYPE_VIDEO;
        int total;
        media_file_total_get(delete_file_type, &total, NULL);
        if (total == 0)
        {
            delete_file_type = FILE_TYPE_NONE;
            exit_time_count = 1;
        }
        else
        {
            exit_time_count = 1;
            media_file_delete_all_start(delete_file_type);
        }
    }
    else
    {
        delete_file_type = FILE_TYPE_NONE;
        exit_time_count = 1;
    }
    user_data_get()->new_media_file_flag = 0;
}

// 通用删除取消
static void media_list_delete_no_btn_up(lv_obj_t *obj)
{
    printf("no clicked\n");
    // 根据当前选中类型删除对应对话框
    if(last_selected_btn_id == PHOTO_LIST_VIDEO_BTN_ID) {
        setting_msgdialog_msg_bg_delete(LAYOUT_MEMORY_LANG_DELETE_ALL_VIDEO_ID);
    } else {
        setting_msgdialog_msg_bg_delete(LAYOUT_MEMORY_LANG_DELETE_ALL_PICTURE_ID);
    }
}

// 通用删除按钮回调
static void media_list_delete_btn_up(lv_obj_t *obj)
{
    // 判断当前选中的是照片还是视频
    int total = 0;
    int lang_id = 0;
    obj_click_data *yes_btn_data = NULL;
    
    if(last_selected_btn_id == PHOTO_LIST_VIDEO_BTN_ID) {
        total = video_total;
        lang_id = LAYOUT_MEMORY_LANG_DELETE_ALL_VIDEO_ID;
        static obj_click_data yes_data = obj_click_data_up_create(video_list_delete_yes_btn_up);
        yes_btn_data = &yes_data;
    } else {
        if (last_selected_btn_id == PHOTO_LIST_FLASH_PHOTO_BTN_ID) {
            media_file_total_get(FILE_TYPE_FLASH_PHOTO, &total, NULL);
        } else if (last_selected_btn_id == PHOTO_LIST_SD_PHOTO_BTN_ID) {
            media_file_total_get(FILE_TYPE_PHOTO, &total, NULL);
        }
        lang_id = LAYOUT_MEMORY_LANG_DELETE_ALL_PICTURE_ID;
        static obj_click_data yes_data = obj_click_data_up_create(photo_list_delete_yes_btn_up);
        yes_btn_data = &yes_data;
    }

    if (total == 0)
        return;
    lv_obj_t *cont = setting_msgdialog_msg_bg_create(lang_id, 460, 156);
    static obj_click_data no_data = obj_click_data_up_create(media_list_delete_no_btn_up);
    memory_message_box_create(cont, yes_btn_data, &no_data, lang_id);
}

/************************** 按钮创建函数 **************************/
static void photo_list_home_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(photo_list_home_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_HOME_PNG);
    camera_img_btn_create(parent, media_btn_area[PHOTO_LIST_HOME_BTN], NULL, &btn_data, &info);
}

static void photo_list_flash_photo_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data=obj_click_data_up_create(photo_list_flash_photo_btn_up);
    static rom_bin_info info=rom_bin_info_get(ROM_UI_MEDIA_PHOTO_PNG);
    flash_photo_btn = page_btn_create(
        media_switch_cont,
        media_btn_area[PHOTO_LIST_FLASH_PHOTO_BTN],
        str_get(LAYOUT_MEMORY_LANG_PHOTO_FLASH_ID),
        &btn_data,
        &info);
    lv_obj_set_id(flash_photo_btn,PHOTO_LIST_FLASH_PHOTO_BTN_ID);
}

static void photo_list_sd_photo_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data=obj_click_data_up_create(photo_list_sd_photo_btn_up);
    static rom_bin_info info=rom_bin_info_get(ROM_UI_MEDIA_PHOTO_PNG);
    sd_photo_btn = page_btn_create(
        media_switch_cont,
        media_btn_area[PHOTO_LIST_SD_PHOTO_BTN],
        str_get(LAYOUT_MEMORY_LANG_PHOTO_SD_ID),
        &btn_data,
        &info);
    lv_obj_set_id(sd_photo_btn,PHOTO_LIST_SD_PHOTO_BTN_ID);
}

static void photo_list_video_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(photo_list_video_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_VIDEO_PNG);
    video_btn = page_btn_create(media_switch_cont, media_btn_area[PHOTO_LIST_VIDEO_BTN], str_get(LAYOUT_MEMORY_LANG_VIDEO_LISE_ID), &btn_data, &info);
    lv_obj_set_id(video_btn,PHOTO_LIST_VIDEO_BTN_ID);
}

static void media_list_delete_btn_create(lv_obj_t *parent)
{
    static obj_click_data btn_data = obj_click_data_up_create(media_list_delete_btn_up);
    static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_DELETE_PNG);
    lv_obj_t *btn = camera_img_btn_create(parent, media_btn_area[PHOTO_LIST_DELETE_BTN], NULL, &btn_data, &info);
    lv_obj_set_id(btn, PHOTO_LIST_DELETE_BTN_ID);
}

static void playback_total_label_create(void)
{
    lv_obj_t *label = lv_label_create(main_func_cont, NULL);
    lv_obj_set_id(label, PHOTO_LIST_PHOTO_TOTAL_BTN_ID);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
    lv_obj_set_pos(label, 483, 458);
    lv_obj_set_size(label, 133, 29);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style_local_text_color(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
    playback_total_label_display(0); // 默认显示照片分页
}

static void list_no_sd_create(lv_obj_t *parent)
{
    if(no_sd_label!=NULL)
    {
        lv_obj_del(no_sd_label);
        no_sd_label=NULL;
    }
	lv_obj_t *label = lv_label_create(parent, NULL);
	lv_obj_align(label, NULL, LV_ALIGN_CENTER, -25, 0);
	lv_label_set_text(label, str_get(LAYOUT_MEMORY_LANG_NO_SD_ID));
	lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFCE08));
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(40));
    no_sd_label = label; 
}

static void media_list_sdcard_state_change_func(void)
{
    // 重新初始化媒体参数
    photo_play_parameter_init();
    video_play_parameter_init();
    
    // 更新页面显示
    if(photo_page != NULL && (last_selected_btn_id != PHOTO_LIST_FLASH_PHOTO_BTN_ID)){
        lv_obj_clean(photo_page);
        for(int i=0;i<6;i++){
            int file_global_index=(photo_list_curr_page_num-1)*6+i;
            if(file_global_index>=photo_total)break;
            photo_list_page_btn_create(photo_page,0,i*62,800,62,file_global_index);
        }
    }
    
    if(video_page != NULL){
        lv_obj_clean(video_page);
        for(int i=0;i<6;i++){
            int file_global_index=(video_list_curr_page_num-1)*6+i;
            if(file_global_index>=video_total)break;
            video_list_page_btn_create(video_page,0,i*62,800,62,file_global_index);
        }
    }
    
    playback_total_label_display(last_selected_btn_id == PHOTO_LIST_VIDEO_BTN_ID ? 1 : 0);
    lv_obj_t *parent =lv_scr_act();
    if (media_sdcard_insert_check() == false&&last_selected_btn_id != PHOTO_LIST_FLASH_PHOTO_BTN_ID)
    {
        list_no_sd_create(parent);
    }
    else{
        if(no_sd_label != NULL) {
            lv_obj_del(no_sd_label);
            no_sd_label = NULL;
        }
    }
}

/************************** 布局入口/退出函数**************************/
static void LAYOUT_ENTER_FUNC(photo_list)
{
    printf("============================enter_photo_list\n");
    file_type_photo=FILE_TYPE_FLASH_PHOTO;
    user_data_get()->new_photo_file_flag = false;
    photo_play_parameter_init();
    video_play_parameter_init(); // 同时初始化视频参数
    photo_list_curr_page_num=1;
    
    parent_bg = common_bg_display(lv_scr_act());
    lv_obj_set_style_local_text_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));

    // 创建第一份的UI容器
    main_func_cont = lv_cont_create(parent_bg, NULL);
    lv_obj_set_pos(main_func_cont, 51, 74);
    lv_obj_set_size(main_func_cont, 922, 504);
    lv_obj_set_style_local_bg_color(main_func_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x838383));
    lv_obj_set_style_local_radius(main_func_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_bg_opa(main_func_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
    lv_obj_set_style_local_border_width(main_func_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(main_func_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

    media_switch_cont = lv_cont_create(main_func_cont, NULL);
    lv_obj_set_pos(media_switch_cont, 0, 0);
    lv_obj_set_size(media_switch_cont, 218, 504);
    lv_obj_set_style_local_bg_color(media_switch_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x616688));
    lv_obj_set_style_local_radius(media_switch_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_bg_opa(media_switch_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
    lv_obj_set_style_local_border_width(media_switch_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_outline_width(media_switch_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

    // 创建所有按钮
    photo_list_home_btn_create(parent_bg);
    photo_list_flash_photo_btn_create(media_switch_cont);
    photo_list_sd_photo_btn_create(media_switch_cont);
    photo_list_video_btn_create(media_switch_cont); // 视频按钮
    media_list_page_display(main_func_cont); // 整合的页面显示
    media_list_delete_btn_create(parent_bg); // 通用删除按钮
    playback_total_label_create();
    top_time_date_text_create(parent_bg);

    // 恢复上次选中的按钮状态
    switch(last_selected_btn_id){
        case PHOTO_LIST_FLASH_PHOTO_BTN_ID:
            photo_list_flash_photo_btn_up(NULL);
            break;
        case PHOTO_LIST_SD_PHOTO_BTN_ID:
            photo_list_sd_photo_btn_up(NULL);
            break;
        case PHOTO_LIST_VIDEO_BTN_ID:
            photo_list_video_btn_up(NULL);
            break;
        default:
            photo_list_flash_photo_btn_up(NULL);
            break;
    }
    
    if (media_sdcard_insert_check() == false&&last_selected_btn_id != PHOTO_LIST_FLASH_PHOTO_BTN_ID)
	{
		list_no_sd_create(parent_bg);
	}
    else
    {
        // SD卡已插入，确保提示被删除
        if(no_sd_label != NULL) {
            lv_obj_del(no_sd_label);
            no_sd_label = NULL;
        }
    }
    
    lyaout_sd_state_callback_register(media_list_sdcard_state_change_func);
}

static void LAYOUT_QUIT_FUNC(photo_list)
{
    const layout *cur_layout = cur_layout_get();
    if ((cur_layout != pLAYOUT(memory_photo)) && (cur_layout != pLAYOUT(memory_video)))
    {
        thumb_media_close();
    }

    lyaout_sd_state_callback_register(NULL);
    if ((cur_layout != pLAYOUT(memory_photo)) && (cur_layout != pLAYOUT(memory_video)) && (cur_layout != pLAYOUT(photo_list))) {
        photo_index_set(0);
        video_index_set(0);
    }
    if(no_sd_label != NULL) {
        lv_obj_del(no_sd_label);
        no_sd_label = NULL;
    }
    
    if(main_func_cont) {
        lv_obj_del(main_func_cont);
        main_func_cont = NULL;
    }
    
    // 清空对象引用
    media_switch_cont = NULL;
    flash_photo_btn = NULL;
    sd_photo_btn = NULL;
    video_btn = NULL;
    photo_page = NULL;
    video_page = NULL;
    // last_selected_btn_id = PHOTO_LIST_FLASH_PHOTO_BTN_ID;
}

CREATE_LAYOUT(photo_list);
// #include "layout_define.h"
// #include "media_thumb.h"

// #define PHOTO_LIST_BG_FUNC_BTN_BG_BLOCK_ID 1
// #define PHOTO_LIST_HOEM_BTN_ID 2
// #define PHOTO_LIST_IMAGE_BTN_ID 3
// #define PHOTO_LIST_VIDEO_BTN_ID 4
// #define PHOTO_LIST_HEAD_LABEI_BTN_ID 5
// #define PHOTO_LIST_PHOTO_PAGE_BTN_ID 6
// #define PHOTO_LIST_PHOTO_TATOL_BTN_ID 7
// #define PHOTO_LIST_PHOTO_LEFT_BTN_ID 8
// #define PHOTO_LIST_PHOTO_RIGHT_BTN_ID 9
// #define PHOTO_LIST_CALL_BTN_ID 10
// #define HOME_GEAR_OBJ_ID 11
// #define HOME_BACK_OBJ_ID 12

// static file_type photo_file_type = FILE_TYPE_VIDEO;
// static file_type delete_file_type = FILE_TYPE_NONE;

// static const file_info *photo_info;
// static lv_obj_t *photo_page = NULL;
// static char photo_file_path[20] = {0};

// static int photo_list_curr_page_num = 1; // 当前页
// static int start_angle = 270;			 // 起始角度

// static int media_total = 0;
// static int exit_time_count = 0;
// // static int video_total = 0;

// static int photo_index = 0;
// static int photo_total;

// int photo_index_get(void)
// {
// 	return photo_index;
// }
// void photo_index_set(int index)
// {
// 	photo_index = index;
// }

// int photo_total_get(void)
// {
// 	return photo_total;
// }
// void photo_total_set(int index)
// {
// 	photo_index = index;
// }

// static int page_num = 0;

// typedef enum
// {
// 	PHOTO_LIST_HOME_BTN,
// 	PHOTO_LIST_IMAGE_BTN,
// 	PHOTO_LIST_VIDEO_BTN,
// 	PHOTO_LIST_CALL_BTN,
// 	PHOTO_LIST_DELETE_BTN_ID,
// 	PHOTO_LIST_TOTAL_BTN,
// } photo_btn_module;

// static custom_area photo_list_btn_area[PHOTO_LIST_TOTAL_BTN] =
// 	{
// 		{24, 12, 54, 54},
// 		{28, 410, 157, 170},
// 		{30, 240, 155, 170},
// 		{30, 70, 155, 170},
// 		{954, 12, 48, 48},
// };

// static void photo_flash_create(lv_obj_t *parent, custom_area btn_area, const char *str, char *value, obj_click_data *prev_btn_data, obj_click_data *next_btn_data, unsigned int id)
// {
// lv_obj_t *photo_flash_btn = lv_btn_create(parent, NULL);
// lv_obj_set_size(photo_flash_btn, 460, 40);
// //lv_obj_align(photo_flash_btn, LV_ALIGN_CENTER, 0, 0);

// // 1. photo_flash_icon
// lv_obj_t *photo_flash_icon = lv_img_create(photo_flash_btn, NULL);
// //lv_img_set_src(photo_flash_icon, ROM_UI_HOME_CAMERA_PNG);
// lv_obj_set_size(photo_flash_icon, 30, 30);
// //lv_obj_align(photo_flash_icon, LV_ALIGN_LEFT_MID, 10, 0);

// // 2. photo_flash_text
// lv_obj_t *photo_flash_text_label = lv_label_create(photo_flash_btn, NULL);
// lv_label_set_text(photo_flash_text_label, "Photo 1 (Flash)");
// //lv_obj_align(photo_flash_text_label, LV_ALIGN_LEFT_MID, 50, 0);

// // 3. photo_flash_arrow
// lv_obj_t *photo_flash_arrow = lv_img_create(photo_flash_btn, NULL);
// //lv_img_set_src(photo_flash_arrow, ROM_UI_HOME_CAMERA_PNG);
// lv_obj_set_size(photo_flash_arrow, 20, 20);
// //lv_obj_align(photo_flash_arrow, LV_ALIGN_RIGHT_MID, -10, 0);

// // 4. photo_flash_arrow
// lv_obj_t *photo_flash_label = lv_img_create(photo_flash_btn, NULL);
// //lv_img_set_src(photo_flash_label, ROM_UI_HOME_CAMERA_PNG);
// lv_obj_set_size(photo_flash_label, 20, 20);
// //lv_obj_align(photo_flash_arrow, LV_ALIGN_RIGHT_MID, -10, 0);


// // 5. photo_flash_number
// lv_obj_t *photo_flash_num_label = lv_label_create(photo_flash_btn, NULL);
// lv_label_set_text(photo_flash_num_label, "0");  
// //lv_obj_align(photo_flash_num_label, LV_ALIGN_RIGHT_MID, -60, 0);
// }
// void photo_play_parameter_init(void)
// {

// 	if (media_sdcard_insert_check() == true)
// 	{
// 		photo_file_type = FILE_TYPE_PHOTO;
// 		sprintf(photo_file_path, "%s", SD_PHOTO_PATH);
// 		sd_media_all_file_total_get(&media_total, NULL);
// 		printf("====================>> sd卡媒体文件总数:[%d]\n", media_total);
// 	}
// 	else
// 	{
// 		photo_file_type = FILE_TYPE_FLASH_PHOTO;
// 		sprintf(photo_file_path, "%s", FLASH_PHOTO_PATH);
// 		media_file_total_get(photo_file_type, &media_total, NULL);
// 		printf("====================>> flash媒体文件总数:[%d]\n", media_total);
// 	}

// 	media_file_total_get(photo_file_type, &photo_total, NULL);
// 	printf("====================>> 图片文件总数:[%d]\n", photo_total);
// }

// static void photo_list_home_btn_up(lv_obj_t *obj)
// {
// 	goto_layout(pLAYOUT(home));
// }
// // 创建home按钮
// static void photo_list_home_btn_create(lv_obj_t *parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(photo_list_home_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_HOME_PNG);
// 	camera_img_btn_create(parent, photo_list_btn_area[PHOTO_LIST_HOME_BTN], NULL, &btn_data, &info);
// }

// static void photo_list_call_btn_up(lv_obj_t *obj)
// {
// 	goto_layout(pLAYOUT(call_list));
// }
// // 创建call按钮
// static void photo_list_call_btn_create(lv_obj_t *parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(photo_list_call_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_CALL_PNG);
// 	lv_obj_t *btn = photo_and_video_btn_create(parent, photo_list_btn_area[PHOTO_LIST_CALL_BTN], str_get(LAYOUT_MEMORY_LANG_CALL_ID), &btn_data, &info, true, false);
// 	lv_obj_set_id(btn, PHOTO_LIST_CALL_BTN_ID);
// }

// static void photo_list_image_btn_up(lv_obj_t *obj)
// {
// 	goto_layout(pLAYOUT(photo_list));
// }
// // 创建image按钮
// static void photo_list_image_btn_create(lv_obj_t *parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(photo_list_image_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_PHOTO_RED_PNG);
// 	photo_and_video_btn_create(parent, photo_list_btn_area[PHOTO_LIST_IMAGE_BTN], str_get(LAYOUT_MEMORY_LANG_IMAGE_LISE_ID), &btn_data, &info, false, true);
// }

// static void photo_list_video_btn_up(lv_obj_t *obj)
// {
// 	goto_layout(pLAYOUT(video_list));
// }
// // 创建video按钮
// static void photo_list_video_btn_create(lv_obj_t *parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(photo_list_video_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_VIDEO_PNG);
// 	photo_and_video_btn_create(parent, photo_list_btn_area[PHOTO_LIST_VIDEO_BTN], str_get(LAYOUT_MEMORY_LANG_VIDEO_LISE_ID), &btn_data, &info, true, false);
// }

// static void photo_list_btn_up(lv_obj_t *obj)
// {
// 	photo_index = obj->obj_id;
// 	printf("==========@@@@@@@@==[%d]======\n", photo_index);
// 	if (photo_total == 0)
// 	{
// 		photo_index = -1;
// 	}

// 	goto_layout(pLAYOUT(memory_photo));
// }

// static lv_obj_t *photo_list_page_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int i)
// {
// 	printf("#########################111\n");

// 	lv_obj_t *btn = lv_obj_create(parent, NULL);
// 	lv_obj_set_id(btn, i);

// 	lv_obj_set_drag_parent(btn, true);
// 	printf("#########################222\n");
// 	// lv_btn_set_layout(btn, LV_FIT_NONE);
// 	lv_obj_set_pos(btn, x, y);
// 	lv_obj_set_size(btn, w, h);
// 	// set_location(btn, x, y, w, h);
// 	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
// 	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
// 	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x3BD741));
// 	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x4f4f4f));

// 	// static rom_bin_info info_pressed = rom_bin_info_get(ROM_UI_MEDIA_LIST_NEW_PNG);
// 	// lv_obj_set_style_local_pattern_image(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, &info_pressed);
// 	// lv_obj_set_style_local_pattern_align(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
// 	// lv_obj_set_style_local_pattern_opa(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);
// 	static obj_click_data click_data = obj_click_data_up_create(photo_list_btn_up);
// 	obj_click_event_listen(btn, &click_data);

// 	photo_info = media_file_info_get(photo_file_type, photo_total - i - 1);
// 	printf("#########################333\n");
// 	// if (photo_info->is_new)
// 	// {
// 	// 	lv_obj_set_style_local_value_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xff1111));
// 	// 	lv_obj_set_style_local_value_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xff1111));
// 	// }
// 	// else
// 	// {
// 	// 	lv_obj_set_style_local_value_color(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	// 	lv_obj_set_style_local_value_color(btn, LV_LABEL_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0xFFFFFF));
// 	// 	// lv_obj_set_style_local_pattern_image(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, NULL);
// 	// 	// lv_obj_set_style_local_pattern_image(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, NULL);
// 	// }
// 	lv_color_t new_file_color_def = lv_color_hex(0xC52323); // 新文件-默认字体色
// 	lv_color_t normal_color_def = lv_color_hex(0xFFFFFF);	// 非新文件-默认字体色

// 	// 根据“是否为新文件”选择对应的颜色组
// 	lv_color_t curr_color_def = (photo_info->is_new) ? new_file_color_def : normal_color_def;

// 	char year[32] = {0};
// 	char month[32] = {0};
// 	char day[32] = {0};
// 	char hour[32] = {0};
// 	char min[32] = {0};
// 	char second[32] = {0};
// 	static char file_year[64] = {0};
// 	// static char file_time[64] = {0};
// 	strncpy(year, photo_info->file_name, 2);
// 	strncpy(month, photo_info->file_name + 2, 2);
// 	strncpy(day, photo_info->file_name + 4, 2);
// 	strncpy(hour, photo_info->file_name + 7, 2);
// 	strncpy(min, photo_info->file_name + 9, 2);
// 	strncpy(second, photo_info->file_name + 11, 2);

// 	printf("------year[%s]\n", photo_info->file_name);
// 	printf("------year[%s]\n", year);
// 	printf("------month[%s]\n", month);
// 	printf("------day[%s]\n", day);
// 	printf("------hour[%s]\n", hour);
// 	printf("------min[%s]\n", min);
// 	printf("------second[%s]\n", second);

// 	struct date temp_date =
// 		{
// 			.year = atoi(year) + 2000,
// 			.month = atoi(month),
// 			.day = atoi(day)};
// 	temp_date = gregorian2jalali(temp_date);

// 	if (user_data_get()->setting.calendar == 1)
// 	{
// 		sprintf(file_year, "20%s-%s-%s %s:%s", year, month, day, hour, min);
// 	}
// 	else
// 	{
// 		sprintf(file_year, "%d-%02d-%02d %s:%s",
// 				temp_date.year, temp_date.month, temp_date.day,
// 				hour, min);
// 	}
// 	// sprintf(file_time, "%s:%s:%s", hour, min, second);
// 	printf("---------------------file_year[%s]\n", file_year);
// 	// printf("---------------------file_time[%s]\n", file_time);

// 	lv_obj_set_style_local_value_align(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
// 	lv_obj_set_style_local_value_ofs_x(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 73);
// 	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

// 	lv_obj_t *obj = lv_obj_create(btn, NULL);
// 	if (photo_info->ch == MON_CH_DOOR1)
// 	{
// 		lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_DOOR1_ID));
// 	}
// 	else if (photo_info->ch == MON_CH_DOOR2)
// 	{
// 		lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_DOOR2_ID));
// 	}
// #if 1
// 	else if (photo_info->ch == MON_CH_CCTV1)
// 	{
// 		lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_CCTV1_ID));
// 	}
// 	else if (photo_info->ch == MON_CH_CCTV2)
// 	{
// 		lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_CCTV2_ID));
// 	}
// #endif
// 	lv_obj_set_style_local_value_color(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, curr_color_def); // 默认状态
// 	// 其他样式
// 	lv_obj_set_style_local_value_font(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
// 	if (user_data_get()->setting.language == LANG_ENGLISH)
// 	{
// 		lv_obj_set_style_local_value_align(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
// 	}
// 	else
// 	{
// 		lv_obj_set_style_local_value_align(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);
// 	}
// 	// 位置和点击属性
// 	lv_obj_align(obj, btn, LV_ALIGN_IN_LEFT_MID, 390, 5);
// 	lv_obj_set_click(obj, false);

// 	lv_obj_t *label2 = lv_label_create(btn, NULL);
// 	lv_label_set_text(label2, file_year);			 // 设置日期文本
// 	lv_label_set_align(label2, LV_LABEL_ALIGN_LEFT); // 文本左对齐
// 	// 【核心样式】设置“默认”和“按下”状态的字体颜色
// 	lv_obj_set_style_local_text_color(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, curr_color_def); // 默认状态
// 																									 // 位置
// 	lv_obj_align(label2, btn, LV_ALIGN_IN_LEFT_MID, 30, 0);
// 	// 其他样式
// 	lv_obj_set_style_local_text_font(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

// 	lv_obj_t *label3 = lv_label_create(btn, NULL);

// 	// 根据mode值设置录制模式文本（手动/自动/运动检测）
// 	const char *mode_text = NULL;
// 	int mode_align = 625;
// 	switch (photo_info->mode)
// 	{
// 	case REC_MODE_MANUAL:
// 		mode_text = str_get(LAYOUT_MEMORY_LANG_MANUAL_ID); // 手动录制
// 		break;
// 	case REC_MODE_AUTO:
// 		mode_text = str_get(LAYOUT_MEMORY_LANG_AUTO_ID); // 自动录制
// 		if (user_data_get()->setting.language == LANG_PERSIAN)
// 		{
// 			mode_align = 605;
// 		}
// 		else
// 		{
// 			mode_align = 645;
// 		}
// 		break;
// 	case REC_MODE_MOTION:
// 		mode_text = str_get(LAYOUT_MEMORY_LANG_MOTION); // 运动检测录制
// 		break;
// 	default:
// 		mode_text = str_get(LAYOUT_MEMORY_LANG_MANUAL_ID); // 手动录制
// 		break;
// 	}
// 	lv_label_set_text(label3, mode_text); // 标签文本设置
// 	// 其他样式
// 	lv_label_set_align(label3, LV_LABEL_ALIGN_LEFT);
// 	// 位置
// 	lv_obj_align(label3, btn, LV_ALIGN_IN_LEFT_MID, mode_align, 0);
// 	lv_obj_set_style_local_text_font(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

// 	lv_obj_set_style_local_text_color(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, curr_color_def); // 默认状态

// 	lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
// 	lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x32, 0x32, 0x37));
// 	lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27));
// 	lv_obj_set_style_local_border_side(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
// 	// if (photo_info->is_new)
// 	// {
// 	// 	lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	// 	lv_obj_set_style_local_text_color(label2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	// 	lv_obj_set_style_local_text_color(label3, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	// }
// 	// else
// 	// {
// 	// 	lv_obj_set_style_local_value_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	// 	lv_obj_set_style_local_text_color(label2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	// 	lv_obj_set_style_local_text_color(label3, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	// }

// 	return btn;
// }


// static void photo_flash_label_create(lv_obj_t *center_cont)
// {
//     // 1. 创建按钮容器
//     lv_obj_t *btn_cont = lv_obj_create(center_cont, NULL);
//     v_obj_set_pos(btn_cont, 100, 105);
// 	lv_obj_set_size(btn_cont, 465, 50);
//     lv_obj_set_style_local_value_color(btn_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x201F1F));
// 	lv_obj_set_style_local_value_align(btn_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_RIGHT);
// 	lv_obj_set_style_local_value_ofs_x(btn_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 50);
// 	lv_obj_set_style_local_value_font(btn_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
//     // 2. 添加图标
//     lv_obj_t *img = lv_img_create(center_cont, NULL);
//     static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_PREV_PNG);
//     lv_obj_set_size(img, 56, 37);
//     lv_obj_set_pos(img, 100, 105);
//     lv_obj_set_style_local_pattern_image(img, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &info);
    
//     lv_obj_t *label = lv_label_create(center_cont, NULL);
//     lv_label_set_text(label, "Photo 1 (Flash) | 101");
//     lv_obj_set_size(img, 56, 37);
//     lv_obj_set_pos(img, 188, 110);

//     lv_obj_t *icon = lv_img_create(center_cont, NULL);
//     static rom_bin_info icon_info = rom_bin_info_get(ROM_UI_MEDIA_LIST_PREV_PNG);
//     lv_obj_set_size(icon, 0, 38);
//     lv_obj_set_pos(icon, 405, 105);
//     lv_obj_set_style_local_pattern_image(icon, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &icon_info);
    

//     lv_obj_t *arrow = lv_img_create(center_cont, NULL);
//     static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_NEXT_PNG);
//     lv_obj_set_size(arrow, 56, 37);
//     lv_obj_set_pos(arrow, 100, 105);
//     lv_obj_set_style_local_pattern_image(img, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &info);

//     static obj_click_data btn_data2 = obj_click_data_up_create(photo_list_next_btn_up);
// 	obj_click_event_listen(btn_cont, &btn_data2);
// }

// /***
// **   日期:2022-05-23 16:28:24
// **   作者: leo.liu
// **   函数作用：页面显示
// **   参数说明:
// ***/
// static void playback_total_label_display(void)
// {
// 	static char str[20];
// 	lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), PHOTO_LIST_PHOTO_TATOL_BTN_ID);
// 	int total_page_num = (photo_total == 0) ? 0 : (photo_total / 6 + ((photo_total % 6) ? 1 : 0));
// 	if (photo_total == 0)
// 	{
// 		photo_list_curr_page_num = 0;
// 	}
// 	sprintf(str, "%04d/%04d", photo_list_curr_page_num, total_page_num);
// 	lv_label_set_text(label, str);
// 	lv_obj_set_style_local_text_color(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
// }
// /***
// **   日期:2022-05-23 16:25:07
// **   作者: leo.liu
// **   函数作用：显示页面创建
// **   参数说明:
// ***/
// static void playback_total_label_create(void)
// {
// 	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
// 	lv_obj_set_id(label, PHOTO_LIST_PHOTO_TATOL_BTN_ID);
// 	lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
// 	lv_obj_set_pos(label, 468, 539);
// 	lv_obj_set_size(label, 222, 34);
// 	// lv_obj_align(label, NULL, LV_ALIGN_IN_BOTTOM_MID, 36, -71);
// 	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
// 	playback_total_label_display();
// }

// static void photo_list_prev_btn_up(lv_obj_t *obj)
// {
// 	if (--photo_list_curr_page_num <= 0)
// 	{
// 		photo_list_curr_page_num = (photo_total == 0) ? 1 : (photo_total / 6 + ((photo_total % 6) ? 1 : 0));
// 		if (photo_list_curr_page_num == 1)
// 			return;
// 	}
// 	playback_total_label_display();
// 	lv_obj_clean(photo_page);
// 	for (int i = 0; ((page_num - photo_list_curr_page_num) * 6 + i) < photo_total; i++)
// 	{
// 		if (photo_list_page_btn_create(photo_page, 0, ((i) * 62), 800, 62, (page_num - photo_list_curr_page_num) * 6 + i) == NULL)
// 		{
// 			i--;
// 		}
// 		if (i >= 6)
// 			break;
// 	}
// }

// static void photo_list_next_btn_up(lv_obj_t *obj)
// {
// 	if (++photo_list_curr_page_num > ((photo_total == 0) ? 0 : (photo_total / 6 + ((photo_total % 6) ? 1 : 0))))
// 	{
// 		photo_list_curr_page_num = 1;
// 		if (((photo_total == 0) ? 1 : (photo_total / 6 + ((photo_total % 6) ? 1 : 0))) == 1)
// 			return;
// 	}
// 	playback_total_label_display();
// 	lv_obj_clean(photo_page);
// 	for (int i = 0; ((page_num - photo_list_curr_page_num) * 6 + i) < photo_total; i++)
// 	{
// 		if (photo_list_page_btn_create(photo_page, 0, ((i) * 62), 800, 62, (page_num - photo_list_curr_page_num) * 6 + i) == NULL)
// 		{
// 			i--;
// 		}
// 		if (i >= 6)
// 			break;
// 	}
// }

// static void photo_list_page_display(lv_obj_t *parent)
// {

// 	photo_page = lv_cont_create(parent, NULL);
// 	lv_obj_set_id(photo_page, PHOTO_LIST_PHOTO_PAGE_BTN_ID);
// 	lv_obj_set_pos(photo_page, 200, 131);
// 	lv_obj_set_size(photo_page, 790, 373);
// 	// set_location(photo_page, 180, 89, 748, 426);
// 	lv_obj_set_style_local_value_color(photo_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x201F1F));
// 	lv_obj_set_style_local_value_align(photo_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_RIGHT);
// 	lv_obj_set_style_local_value_ofs_x(photo_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 50);
// 	lv_obj_set_style_local_value_font(photo_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

// 	if (photo_index_get() < 0)
// 	{
// 		photo_index_set(0);
// 	}
// 	int select_index = photo_index_get();
// 	page_num = (photo_total / 6 + ((photo_total % 6) ? 1 : 0));
// 	photo_list_curr_page_num = ((select_index <= 6) ? page_num : (page_num - (select_index / 6)));
// 	printf("+++++++++++++++++>>>>>>>>>111111[%d]====[%d]\n", photo_list_curr_page_num, select_index);
// 	for (int i = 0; ((page_num - photo_list_curr_page_num) * 6 + i) < photo_total; i++)
// 	{
// 		if (photo_list_page_btn_create(photo_page, 0, ((i) * 62), 800, 62, (page_num - photo_list_curr_page_num) * 6 + i) == NULL)
// 		{
// 			i--;
// 		}
// 		if (i >= 6)
// 			break;
// 	}

// 	lv_obj_t *prev_btn = lv_obj_create(lv_scr_act(), NULL);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_PREV_PNG);
// 	lv_obj_set_pos(prev_btn, 404, 528);
// 	lv_obj_set_size(prev_btn, 72, 72);
// 	// lv_obj_set_size(prev_btn, 72, 72);
// 	// lv_obj_align(prev_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, -158, 0);
// 	lv_obj_set_style_local_pattern_image(prev_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &info);

// 	static obj_click_data btn_data1 = obj_click_data_up_create(photo_list_prev_btn_up);
// 	obj_click_event_listen(prev_btn, &btn_data1);

// 	lv_obj_t *next_btn = lv_obj_create(lv_scr_act(), NULL);
// 	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_MEDIA_LIST_NEXT_PNG);
// 	lv_obj_set_pos(next_btn, 670, 528);
// 	lv_obj_set_size(next_btn, 72, 72);
// 	// lv_obj_set_size(next_btn, 72, 72);
// 	// lv_obj_align(next_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 158, 0);
// 	lv_obj_set_style_local_pattern_image(next_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &info1);

// 	static obj_click_data btn_data2 = obj_click_data_up_create(photo_list_next_btn_up);
// 	obj_click_event_listen(next_btn, &btn_data2);
// }
// static void delete_file_state_display_task(lv_task_t *task_t)
// {
// 	lv_obj_t *arc = (lv_obj_t *)task_t->user_data;
// 	arc_loader_update(arc, &start_angle);
// 	if (delete_file_type != FILE_TYPE_NONE)
// 	{

// 		if (!media_file_delete_all_state())
// 		{
// 			delete_file_type = FILE_TYPE_NONE;
// 			exit_time_count = 1;
// 		}
// 	}
// 	else
// 	{
// 		if (--exit_time_count <= 0)
// 		{
// 			arc_loader_delete(arc);
// 			goto_layout(pLAYOUT(photo_list));
// 		}
// 	}
// }
// static void photo_list_delete_yes_btn_up(lv_obj_t *obj)
// {
// 	setting_msgdialog_msg_bg_delete(LAYOUT_MEMORY_LANG_DELETE_ALL_PICTURE_ID);
// 	lv_obj_t *cont = setting_msgdialog_msg_bg_create(0, 100, 100);
// 	// 2. 调用封装函数创建环形进度条
// 	lv_arc_loader_create(
// 		cont,
// 		delete_file_state_display_task,
// 		120,
// 		lv_color_hex(0xFFFFFF),
// 		lv_color_hex(0x007AFF),
// 		11,
// 		50,
// 		0, 0);
// 	if (media_sdcard_insert_check() == true)
// 	{
// 		delete_file_type = FILE_TYPE_PHOTO;
// 	}
// 	else
// 	{
// 		delete_file_type = FILE_TYPE_FLASH_PHOTO;
// 	}
// 	int total;
// 	media_file_total_get(delete_file_type, &total, NULL);

// 	if (total == 0)
// 	{
// 		delete_file_type = FILE_TYPE_NONE;
// 		exit_time_count = 1;
// 	}
// 	else
// 	{
// 		exit_time_count = 1;
// 		media_file_delete_all_start(delete_file_type);
// 	}
// 	user_data_get()->new_media_file_flag = 0;
// }

// static void photo_list_delete_no_btn_up(lv_obj_t *obj)
// {
// 	printf("no clicked\n");
// 	setting_msgdialog_msg_bg_delete(LAYOUT_MEMORY_LANG_DELETE_ALL_PICTURE_ID);
// }

// static void photo_list_delete_btn_up(lv_obj_t *obj)
// {
// 	if (photo_total == 0)
// 		return;
// 	lv_obj_t *cont = setting_msgdialog_msg_bg_create(LAYOUT_MEMORY_LANG_DELETE_ALL_PICTURE_ID, 460, 156);
// 	static obj_click_data btn_data = obj_click_data_up_create(photo_list_delete_yes_btn_up);
// 	static obj_click_data btn_data1 = obj_click_data_up_create(photo_list_delete_no_btn_up);
// 	memory_message_box_create(cont, &btn_data, &btn_data1, LAYOUT_MEMORY_LANG_DELETE_ALL_PICTURE_ID);
// }
// // 创建delete按钮
// static void photo_list_delete_btn_create(lv_obj_t *parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(photo_list_delete_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_DELETE_PNG);
// 	lv_obj_t *btn = camera_img_btn_create(parent, photo_list_btn_area[PHOTO_LIST_DELETE_BTN_ID], NULL, &btn_data, &info);
// 	lv_obj_set_id(btn, PHOTO_LIST_DELETE_BTN_ID);
// }

// // static lv_task_t *sdcard_insert_detect = NULL;
// // static bool prev_insert_state = true;
// // static void sdcard_insert_detect_task(lv_task_t *task_t)
// // {
// // 	bool curr_insert_state = media_sdcard_insert_check();
// // 	if (curr_insert_state != prev_insert_state)
// // 	{
// // 		prev_insert_state = curr_insert_state;
// // 		photo_index = 0;
// // 		goto_layout(pLAYOUT(photo_list));
// // 	}
// // }

// // SD卡拔插状态
// static void photo_list_sdcard_state_change_func(void)
// {
// 	if (media_sdcard_insert_check() == false)
// 	{
// 		photo_index = 0;
// 		goto_layout(pLAYOUT(photo_list));
// 	}
// 	else
// 	{
// 		photo_index = 0;
// 		goto_layout(pLAYOUT(photo_list));
// 	}
// }

// // // 插入SD卡但没有照片文案显示
// // static void photo_list_sd_image_num_label_create(lv_obj_t *parent)
// // {
// // 	lv_obj_t *label = lv_label_create(parent, NULL);
// // 	// lv_obj_set_pos(label,511,256);
// // 	// lv_obj_set_size(label,128,45);
// // 	lv_obj_align(label, NULL, LV_ALIGN_CENTER, -50, 0);
// // 	lv_label_set_text(label, str_get(LAYOUT_MEMORY_LANG_NO_IMAGE_ID));
// // 	lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFCE08));
// // 	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(40));
// // }

// /* setting 图标 */
// static void setting_icon_create(lv_obj_t *parent)
// {
//     lv_obj_t *setting_icon_obj = lv_img_create(parent, NULL);
//     lv_obj_set_pos(setting_icon_obj, 54, 34);
// 	lv_obj_set_size(setting_icon_obj, 22, 20);
// 	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_GEAR_PNG);
// 	lv_obj_set_id(setting_icon_obj, HOME_GEAR_OBJ_ID);
// 	lv_obj_set_style_local_pattern_image(setting_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);

//     lv_obj_t *Time_label = lv_label_create(parent, NULL);
//     lv_obj_set_pos(Time_label, 87, 28);
// 	lv_obj_set_size(Time_label, 70, 31);
//     lv_label_set_text(Time_label, "Media");
// 	lv_obj_align(setting_icon_obj, Time_label, LV_ALIGN_OUT_LEFT_MID, -5, 0);
// }

// static void back_btn_up(lv_obj_t *obj)
// {
//     goto_layout(pLAYOUT(home));
// }
// static void back_btn_create(lv_obj_t *parent)
// {
//     lv_obj_t *back_icon_obj = lv_img_create(parent, NULL);
//     lv_obj_set_pos(back_icon_obj, 920, 25);
// 	lv_obj_set_size(back_icon_obj, 50, 37);
// 	lv_obj_set_id(back_icon_obj, HOME_BACK_OBJ_ID);
// 	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_TIME_BACK_PNG);
// 	lv_obj_set_style_local_pattern_image(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);
//     static obj_click_data btn_data = obj_click_data_up_create(back_btn_up);
//     obj_click_event_listen(back_icon_obj, &btn_data);
// }

// static void LAYOUT_ENTER_FUNC(photo_list)
// {
// 	printf("============================enter_photo_list\n");

// 	user_data_get()->new_photo_file_flag = false;
// 	photo_play_parameter_init();
// 	lv_obj_t *parent = common_bg_display(lv_scr_act());

// 	// photo_list_func_btn_bg_block_create(parent);

//     lv_obj_t *center_cont = lv_cont_create(parent, NULL);
//     lv_obj_set_size(center_cont, 664, 336);  
//     lv_obj_set_id(center_cont, 100);
//     lv_obj_align(center_cont, parent,LV_ALIGN_CENTER, 0, 0);  
//     lv_obj_set_style_local_bg_color(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x333355));
//     lv_obj_set_style_local_radius(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
//     lv_obj_set_style_local_bg_opa(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);

//     top_time_date_text_create(parent);
 
//     back_btn_create(parent);





// 	photo_list_home_btn_create(parent);

// 	photo_list_image_btn_create(parent);

// 	photo_list_video_btn_create(parent);

// 	photo_list_call_btn_create(parent);

// 	photo_list_head_label_create(parent);

// 	photo_list_page_display(parent); // 在当前活跃的屏幕上创建容器

// 	photo_list_delete_btn_create(parent);

// 	playback_total_label_create();

// 	lyaout_sd_state_callback_register(photo_list_sdcard_state_change_func);
	
//     setting_icon_create(parent);
// }



// static void LAYOUT_QUIT_FUNC(photo_list)
// {

// 	const layout *cur_layout = cur_layout_get();
// 	if ((cur_layout != pLAYOUT(memory_photo)) /*&& (cur_layout != pLAYOUT(memory_video))*/)
// 	{
// 		thumb_media_close();
// 	}

// 	// if (sdcard_insert_detect != NULL)
// 	// {
// 	// 	lv_task_del(sdcard_insert_detect);
// 	// 	sdcard_insert_detect = NULL;
// 	// }
// 	lyaout_sd_state_callback_register(NULL);
// 	if ((cur_layout != pLAYOUT(memory_photo)) && (cur_layout != pLAYOUT(photo_list)))
// 		photo_index_set(0);
// }
// CREATE_LAYOUT(photo_list);  
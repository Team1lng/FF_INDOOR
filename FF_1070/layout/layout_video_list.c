// #include "layout_define.h"
// #include "media_thumb.h"

// #define VIDEO_LIST_BG_FUNC_BTN_BG_BLOCK_ID 1
// #define VIDEO_LIST_HOEM_BTN_ID 2
// #define VIDEO_LIST_IMAGE_BTN_ID 3
// #define VIDEO_LIST_VIDEO_BTN_ID 4
// #define VIDEO_LIST_HEAD_LABEI_BTN_ID 5
// #define VIDEO_LIST_PHOTO_PAGE_BTN_ID 6
// #define VIDEO_LIST_PHOTO_TATOL_BTN_ID 7
// #define VIDEO_LIST_PHOTO_LEFT_BTN_ID 8
// #define VIDEO_LIST_PHOTO_RIGHT_BTN_ID 9

// typedef enum
// {
// 	VIDEO_LIST_HOME_BTN,
// 	VIDEO_LIST_IMAGE_BTN,
// 	VIDEO_LIST_VIDEO_BTN,
// 	VIDEO_LIST_CALL_BTN,
// 	VIDEO_LIST_DELETE_BTN_ID,
// 	VIDEO_LIST_TOTAL_BTN,
// } photo_btn_module;

// static custom_area video_list_btn_area[VIDEO_LIST_TOTAL_BTN] =
// 	{
// 		{24, 12, 54, 54},
// 		{28, 410, 157, 170},
// 		{30, 240, 155, 170},
// 		{30, 70, 155, 170},
// 		{954, 12, 48, 48},
// };

// static int video_total;
// static char *cur_video_path = SD_MEDIA_PATH;
// static file_type video_file_type = FILE_TYPE_VIDEO;
// static file_type delete_file_type = FILE_TYPE_NONE;
// static lv_obj_t *video_page = NULL;
// const file_info *video_info = NULL;

// static int video_index = 0;
// static int exit_time_count = 0;
// static int video_list_curr_page_num = 1; // 当前页（默认1）
// static int start_angle = 270;			 // 起始角度

// // 【修复】原有video_total_set函数错误（把video_index赋值，现在修正）
// int video_index_get(void)
// {
// 	return video_index;
// }
// void video_index_set(int index)
// {
// 	video_index = index;
// }

// // 【修复】修正video_total_set的逻辑（原有是错的）
// void video_total_set(int total)
// {
// 	video_total = total;
// }

// int video_total_get(void)
// {
// 	return video_total;
// }

// // 【删除】未使用的page_num全局变量
// // static int page_num = 0;

// void video_play_parameter_init(void)
// {
// 	if (media_sdcard_insert_check() == true)
// 	{
// 		cur_video_path = SD_VIDEO_PATH;
// 		video_file_type = FILE_TYPE_VIDEO;
// 		media_file_total_get(video_file_type, &video_total, NULL);
// 		printf("==================================[视频总数:%d]\n", video_total);
// 	}
// 	else
// 	{
// 		video_total = 0;
// 	}
// 	// 重置当前页为1
// 	video_list_curr_page_num = 1;
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
// 	lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), VIDEO_LIST_PHOTO_TATOL_BTN_ID);
// 	// 【修改】精准计算总页数（和图片列表一致）
// 	int total_page_num = 0;
// 	if (video_total > 0) {
// 		total_page_num = (video_total - 1) / 6 + 1; // 通用公式：(总数-1)/每页数量 +1
// 	}
// 	// 边界保护
// 	if (video_list_curr_page_num < 1) {
// 		video_list_curr_page_num = 1;
// 	} else if (video_list_curr_page_num > total_page_num) {
// 		video_list_curr_page_num = total_page_num;
// 	}
// 	// 格式化显示（比如13个视频→0001/0003）
// 	sprintf(str, "%04d/%04d", video_list_curr_page_num, total_page_num);
// 	lv_label_set_text(label, str);
// 	lv_obj_set_style_local_text_color(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
// 	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
// }

// static void video_list_btn_up(lv_obj_t *obj)
// {
// 	printf("=====================================1546543545\n");
// 	video_index = obj->obj_id;
// 	printf("================video_index==[%d]\n", video_index);
// 	if (video_total == 0)
// 	{
// 		video_index = -1;
// 	}

// 	goto_layout(pLAYOUT(memory_video));
// }

// static lv_obj_t *video_list_page_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int file_global_index)
// {
// 	lv_obj_t *btn = lv_obj_create(parent, NULL);
// 	lv_obj_set_id(btn, file_global_index);

// 	lv_obj_set_drag_parent(btn, true);
// 	lv_obj_set_pos(btn, x, y);
// 	lv_obj_set_size(btn, w, h);
// 	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
// 	lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
// 	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x3BD741));
// 	lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x4f4f4f));

// 	static obj_click_data click_data = obj_click_data_up_create(video_list_btn_up);
// 	obj_click_event_listen(btn, &click_data);

// 	// 【核心修改】最新视频在前（和图片列表逻辑一致）
// 	// file_global_index：当前页的全局索引（第1页0-5，第2页6-11...）
// 	// real_file_index：反向取，保证最新的视频在第一条
// 	int real_file_index = video_total - 1 - file_global_index;
// 	// 边界保护
// 	if (real_file_index < 0 || real_file_index >= video_total) {
// 		lv_obj_del(btn);
// 		return NULL;
// 	}
// 	video_info = media_file_info_get(video_file_type, real_file_index);
// 	// 空指针保护
// 	if (video_info == NULL) {
// 		lv_obj_del(btn);
// 		return NULL;
// 	}

// 	lv_color_t new_file_color_def = lv_color_hex(0xC52323); // 新文件-默认字体色
// 	lv_color_t normal_color_def = lv_color_hex(0xFFFFFF);	// 非新文件-默认字体色
// 	lv_color_t curr_color_def = (video_info->is_new) ? new_file_color_def : normal_color_def;

// 	char year[32] = {0};
// 	char month[32] = {0};
// 	char day[32] = {0};
// 	char hour[32] = {0};
// 	char min[32] = {0};
// 	char second[32] = {0};
// 	static char file_year[64] = {0};
// 	strncpy(year, video_info->file_name, 2);
// 	strncpy(month, video_info->file_name + 2, 2);
// 	strncpy(day, video_info->file_name + 4, 2);
// 	strncpy(hour, video_info->file_name + 7, 2);
// 	strncpy(min, video_info->file_name + 9, 2);
// 	strncpy(second, video_info->file_name + 11, 2);

// 	printf("------year[%s]\n", video_info->file_name);
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

// 	lv_obj_set_style_local_value_align(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
// 	lv_obj_set_style_local_value_ofs_x(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 73);
// 	lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

// 	lv_obj_t *obj = lv_obj_create(btn, NULL);
// 	if (video_info->ch == MON_CH_DOOR1)
// 	{
// 		lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_DOOR1_ID));
// 	}
// 	else if (video_info->ch == MON_CH_DOOR2)
// 	{
// 		lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_DOOR2_ID));
// 	}
// #if 1
// 	else if (video_info->ch == MON_CH_CCTV1)
// 	{
// 		lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_CCTV1_ID));
// 	}
// 	else if (video_info->ch == MON_CH_CCTV2)
// 	{
// 		lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_CCTV2_ID));
// 	}
// #endif
// 	lv_obj_set_style_local_value_color(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, curr_color_def);
// 	lv_obj_align(obj, btn, LV_ALIGN_IN_LEFT_MID, 390, 4);
// 	lv_obj_set_click(obj, false);
// 	lv_obj_set_style_local_value_font(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
// 	if (user_data_get()->setting.language == LANG_ENGLISH)
// 	{
// 		lv_obj_set_style_local_value_align(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
// 	}
// 	else
// 	{
// 		lv_obj_set_style_local_value_align(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);
// 	}

// 	lv_obj_t *label2 = lv_label_create(btn, NULL);
// 	lv_label_set_text(label2, file_year);
// 	lv_label_set_align(label2, LV_LABEL_ALIGN_LEFT);
// 	lv_obj_set_style_local_text_color(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, curr_color_def);
// 	lv_obj_align(label2, btn, LV_ALIGN_IN_LEFT_MID, 30, 0);
// 	lv_obj_set_style_local_text_font(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

// 	lv_obj_t *label3 = lv_label_create(btn, NULL);
// 	const char *mode_text = NULL;
// 	int mode_align = 625;
// 	switch (video_info->mode)
// 	{
// 	case REC_MODE_MANUAL:
// 		mode_text = str_get(LAYOUT_MEMORY_LANG_MANUAL_ID);
// 		break;
// 	case REC_MODE_AUTO:
// 		mode_text = str_get(LAYOUT_MEMORY_LANG_AUTO_ID);
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
// 		mode_text = str_get(LAYOUT_MEMORY_LANG_MOTION);
// 		break;
// 	default:
// 		mode_text = str_get(LAYOUT_MEMORY_LANG_MANUAL_ID);
// 		break;
// 	}

// 	lv_label_set_text(label3, mode_text);
// 	lv_label_set_align(label3, LV_LABEL_ALIGN_LEFT);
// 	lv_obj_align(label3, btn, LV_ALIGN_IN_LEFT_MID, mode_align, 0);
// 	lv_obj_set_style_local_text_font(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
// 	lv_obj_set_style_local_text_color(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, curr_color_def);

// 	lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
// 	lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x32, 0x32, 0x37));
// 	lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27));
// 	lv_obj_set_style_local_border_side(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);

// 	return btn;
// }

// static void video_list_home_btn_up(lv_obj_t *obj)
// {
// 	goto_layout(pLAYOUT(home));
// }
// static void video_list_home_btn_create(lv_obj_t *parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(video_list_home_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_HOME_PNG);
// 	lv_obj_t *btn = camera_img_btn_create(parent, video_list_btn_area[VIDEO_LIST_HOME_BTN], NULL, &btn_data, &info);
// 	lv_obj_set_id(btn, VIDEO_LIST_HOEM_BTN_ID);
// }
// static void video_list_call_btn_up(lv_obj_t *obj)
// {
// 	goto_layout(pLAYOUT(call_list));
// }
// static void video_list_call_btn_create(lv_obj_t *parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(video_list_call_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_CALL_PNG);
// 	page_btn_create(parent, video_list_btn_area[VIDEO_LIST_CALL_BTN], str_get(LAYOUT_MEMORY_LANG_CALL_ID), &btn_data, &info);
// }
// static void video_list_image_btn_up(lv_obj_t *obj)
// {
// 	goto_layout(pLAYOUT(photo_list));
// }
// static void video_list_image_btn_create(lv_obj_t *parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(video_list_image_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_PHOTO_PNG);
// 	page_btn_create(parent, video_list_btn_area[VIDEO_LIST_IMAGE_BTN], str_get(LAYOUT_MEMORY_LANG_IMAGE_LISE_ID), &btn_data, &info);
// }

// static void video_list_video_btn_up(lv_obj_t *obj)
// {
// 	goto_layout(pLAYOUT(video_list));
// }
// static void video_list_video_btn_create(lv_obj_t *parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(video_list_video_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_VIDEO_RED_PNG);
// 	page_btn_create(parent, video_list_btn_area[VIDEO_LIST_VIDEO_BTN], str_get(LAYOUT_MEMORY_LANG_VIDEO_LISE_ID), &btn_data, &info);
// }

// static void video_list_head_label_create(lv_obj_t *parent)
// {
// 	lv_obj_t *set_page = lv_cont_create(parent, NULL);
// 	lv_obj_set_pos(set_page, 200, 90);
// 	lv_obj_set_size(set_page, 790, 45);
// 	lv_obj_set_style_local_value_color(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x201F1F));
// 	lv_obj_set_style_local_value_align(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_RIGHT);
// 	lv_obj_set_style_local_value_ofs_x(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 50);
// 	lv_obj_set_style_local_value_font(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	
// 	lv_obj_t *label = lv_label_create(set_page, NULL);
// 	lv_obj_set_pos(label, 40, 0);
// 	lv_obj_set_size(label, 176, 29);
// 	lv_label_set_text(label, str_get(LAYOUT_MEMORY_LANG_RECORD_TIME_ID));
// 	lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x697074));
// 	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

// 	lv_obj_t *label2 = lv_label_create(set_page, NULL);
// 	lv_obj_set_pos(label2, 388, 0);
// 	lv_obj_set_size(label2, 171, 29);
// 	lv_label_set_text(label2, str_get(LAYOUT_MEMORY_LANG_DEVICE_ID));
// 	lv_obj_set_style_local_text_color(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x697074));
// 	lv_obj_set_style_local_text_font(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

// 	lv_obj_t *label3 = lv_label_create(set_page, NULL);
// 	lv_obj_set_pos(label3, 591, 0);
// 	lv_obj_set_size(label3, 167, 29);
// 	lv_label_set_text(label3, str_get(LAYOUT_MEMORY_LANG_RECORD_MODE_ID));
// 	lv_obj_set_style_local_text_color(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x697074));
// 	lv_obj_set_style_local_text_font(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

// 	lv_obj_set_style_local_border_width(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
// 	lv_obj_set_style_local_border_color(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x32, 0x32, 0x37));
// 	lv_obj_set_style_local_border_color(set_page, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27));
// 	lv_obj_set_style_local_border_side(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
// }

// // 【完全重写】上一页按钮逻辑（和图片列表一致，循环切换）
// static void video_list_prev_btn_up(lv_obj_t *obj)
// {
// 	printf("the_prev_1\n");
// 	// 精准计算总页数
// 	int total_page_num = (video_total > 0) ? ((video_total - 1) / 6 + 1) : 0;
// 	if (total_page_num == 0) return; // 无视频直接返回

// 	// 循环逻辑：第一页点上一页→最后一页
// 	if (video_list_curr_page_num <= 1) {
// 		video_list_curr_page_num = total_page_num;
// 	} else {
// 		video_list_curr_page_num--;
// 	}

// 	printf("the_prev_3: 当前页=%d, 总页数=%d\n", video_list_curr_page_num, total_page_num);
// 	playback_total_label_display();
// 	lv_obj_clean(video_page);

// 	// 重新渲染当前页的6个视频项
// 	for (int i = 0; i < 6; i++)
// 	{
// 		int file_global_index = (video_list_curr_page_num - 1) * 6 + i;
// 		if (file_global_index >= video_total) break;
// 		video_list_page_btn_create(video_page, 0, i * 62, 800, 62, file_global_index);
// 	}
// }

// // 【完全重写】下一页按钮逻辑（和图片列表一致，循环切换）
// static void video_list_next_btn_up(lv_obj_t *obj)
// {
// 	printf("==============================================>>>>[%s]\n", __func__);
// 	// 精准计算总页数
// 	int total_page_num = (video_total > 0) ? ((video_total - 1) / 6 + 1) : 0;
// 	if (total_page_num == 0) return; // 无视频直接返回

// 	// 循环逻辑：最后一页点下一页→第一页
// 	if (video_list_curr_page_num >= total_page_num) {
// 		video_list_curr_page_num = 1;
// 	} else {
// 		video_list_curr_page_num++;
// 	}

// 	playback_total_label_display();
// 	lv_obj_clean(video_page);

// 	// 重新渲染当前页的6个视频项
// 	for (int i = 0; i < 6; i++)
// 	{
// 		int file_global_index = (video_list_curr_page_num - 1) * 6 + i;
// 		if (file_global_index >= video_total) break;
// 		video_list_page_btn_create(video_page, 0, i * 62, 800, 62, file_global_index);
// 	}
// }

// // 【完全重写】视频列表页面显示逻辑（默认第一页，最新在前）
// static void video_list_page_display(lv_obj_t *parent)
// {
// 	video_page = lv_cont_create(parent, NULL);
// 	lv_obj_set_id(video_page, VIDEO_LIST_PHOTO_PAGE_BTN_ID);
// 	lv_obj_set_pos(video_page, 200, 131);
// 	lv_obj_set_size(video_page, 790, 373);
// 	lv_obj_set_style_local_value_color(video_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x201F1F));
// 	lv_obj_set_style_local_value_align(video_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_RIGHT);
// 	lv_obj_set_style_local_value_ofs_x(video_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 50);
// 	lv_obj_set_style_local_value_font(video_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

// 	// 【核心】强制默认第一页（删除原有复杂的select_index/page_num计算）
// 	video_list_curr_page_num = 1;
// 	printf("视频列表初始页：%d, 视频总数：%d\n", video_list_curr_page_num, video_total);

// 	// 渲染第一页的6个视频项（最新在前）
// 	for (int i = 0; i < 6; i++)
// 	{
// 		int file_global_index = (video_list_curr_page_num - 1) * 6 + i;
// 		if (file_global_index >= video_total) break;
// 		video_list_page_btn_create(video_page, 0, i * 62, 800, 62, file_global_index);
// 	}

// 	// 创建上一页按钮
// 	lv_obj_t *prev_btn = lv_obj_create(lv_scr_act(), NULL);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_PREV_PNG);
// 	lv_obj_set_pos(prev_btn, 404, 528);
// 	lv_obj_set_size(prev_btn, 72, 72);
// 	lv_obj_set_style_local_pattern_image(prev_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &info);
// 	static obj_click_data btn_data1 = obj_click_data_up_create(video_list_prev_btn_up);
// 	obj_click_event_listen(prev_btn, &btn_data1);

// 	// 创建下一页按钮
// 	lv_obj_t *next_btn = lv_obj_create(lv_scr_act(), NULL);
// 	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_MEDIA_LIST_NEXT_PNG);
// 	lv_obj_set_pos(next_btn, 670, 528);
// 	lv_obj_set_size(next_btn, 72, 72);
// 	lv_obj_set_style_local_pattern_image(next_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &info1);
// 	static obj_click_data btn_data2 = obj_click_data_up_create(video_list_next_btn_up);
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
// 			goto_layout(pLAYOUT(video_list));
// 		}
// 	}
// }

// static void video_list_delete_yes_btn_up(lv_obj_t *obj)
// {
// 	setting_msgdialog_msg_bg_delete(LAYOUT_MEMORY_LANG_DELETE_ALL_VIDEO_ID);
// 	lv_obj_t *cont = setting_msgdialog_msg_bg_create(0, 100, 100);
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
// 		delete_file_type = FILE_TYPE_VIDEO;
// 		int total;
// 		media_file_total_get(delete_file_type, &total, NULL);
// 		if (total == 0)
// 		{
// 			delete_file_type = FILE_TYPE_NONE;
// 			exit_time_count = 1;
// 		}
// 		else
// 		{
// 			exit_time_count = 1;
// 			media_file_delete_all_start(delete_file_type);
// 		}
// 	}
// 	else
// 	{
// 		delete_file_type = FILE_TYPE_NONE;
// 		exit_time_count = 1;
// 	}
// 	user_data_get()->new_media_file_flag = 0;
// }

// static void video_list_delete_no_btn_up(lv_obj_t *obj)
// {
// 	printf("no clicked\n");
// 	setting_msgdialog_msg_bg_delete(LAYOUT_MEMORY_LANG_DELETE_ALL_VIDEO_ID);
// }
// static void video_list_delete_btn_up(lv_obj_t *obj)
// {
// 	if (video_total == 0)
// 		return;
// 	lv_obj_t *cont = setting_msgdialog_msg_bg_create(LAYOUT_MEMORY_LANG_DELETE_ALL_VIDEO_ID, 460, 156);
// 	static obj_click_data btn_data = obj_click_data_up_create(video_list_delete_yes_btn_up);
// 	static obj_click_data btn_data1 = obj_click_data_up_create(video_list_delete_no_btn_up);
// 	memory_message_box_create(cont, &btn_data, &btn_data1, LAYOUT_MEMORY_LANG_DELETE_ALL_VIDEO_ID);
// }
// static void video_list_delete_btn_create(lv_obj_t *parent)
// {
// 	static obj_click_data btn_data = obj_click_data_up_create(video_list_delete_btn_up);
// 	static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_DELETE_PNG);
// 	lv_obj_t *btn = camera_img_btn_create(parent, video_list_btn_area[VIDEO_LIST_DELETE_BTN_ID], NULL, &btn_data, &info);
// 	lv_obj_set_id(btn, VIDEO_LIST_DELETE_BTN_ID);
// }

// static void playback_total_label_create(void)
// {
// 	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
// 	lv_obj_set_id(label, VIDEO_LIST_PHOTO_TATOL_BTN_ID);
// 	lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
// 	lv_obj_set_pos(label, 468, 539);
// 	lv_obj_set_size(label, 222, 34);
// 	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
// 	playback_total_label_display();
// }

// static void video_list_no_sd_create(lv_obj_t *parent)
// {
// 	lv_obj_t *label = lv_label_create(parent, NULL);
// 	lv_obj_align(label, NULL, LV_ALIGN_CENTER, -25, 0);
// 	lv_label_set_text(label, str_get(LAYOUT_MEMORY_LANG_NO_SD_ID));
// 	lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFCE08));
// 	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(40));
// }

// static void video_list_sdcard_state_change_func(void)
// {
// 	const layout *cur_layout = cur_layout_get();
// 	if ((media_sdcard_insert_check() == false) && (cur_layout != pLAYOUT(video_list)))
// 	{
// 		video_index = 0;
// 		goto_layout(pLAYOUT(photo_list));
// 	}
// 	else
// 	{
// 		video_index = 0;
// 		goto_layout(pLAYOUT(video_list));
// 	}
// }

// static void LAYOUT_ENTER_FUNC(video_list)
// {
// 	printf("============================enter_video_list\n");
// 	video_play_parameter_init();

// 	lv_obj_t *parent = common_bg_display(lv_scr_act());

// 	video_list_home_btn_create(parent);
// 	video_list_image_btn_create(parent);
// 	video_list_video_btn_create(parent);
// 	video_list_call_btn_create(parent);
// 	video_list_head_label_create(parent);
// 	video_list_page_display(parent);
// 	video_list_delete_btn_create(parent);
// 	playback_total_label_create();

// 	if (media_sdcard_insert_check() == false)
// 	{
// 		video_list_no_sd_create(parent);
// 	}

// 	lyaout_sd_state_callback_register(video_list_sdcard_state_change_func);
// }

// static void LAYOUT_QUIT_FUNC(video_list)
// {
// 	const layout *cur_layout = cur_layout_get();
// 	if ((cur_layout != pLAYOUT(memory_video)))
// 	{
// 		thumb_media_close();
// 	}
// 	lyaout_sd_state_callback_register(NULL);
// 	if ((cur_layout != pLAYOUT(memory_video)) && (cur_layout != pLAYOUT(video_list)))
// 		video_index_set(0);
// }

// CREATE_LAYOUT(video_list);
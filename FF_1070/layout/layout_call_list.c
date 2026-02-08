// #include "layout_define.h"
// #include "media_thumb.h"

// static int call_total;
// // static int call_new_total;

// // static char *cur_call_path = SD_MEDIA_PATH;

// static lv_obj_t *call_page = NULL;
// // file_info *call_info = NULL;
// static int call_index = 0;
// static int exit_time_count = 0;
// static int call_list_curr_page_num = 1; // 当前页
// static int start_angle = 270;           // 起始角度
// static int page_num = 0;                // 添加page_num声明

// const file_info *call_info = NULL;
// static file_type delete_file_type = FILE_TYPE_NONE;

// #define CALL_LIST_BG_FUNC_BTN_BG_BLOCK_ID 1
// #define CALL_LIST_HOEM_BTN_ID 2
// #define CALL_LIST_IMAGE_BTN_ID 3
// #define CALL_LIST_CALL_BTN_ID 4
// #define CALL_LIST_HEAD_LABEI_BTN_ID 5
// #define CALL_LIST_PHOTO_PAGE_BTN_ID 6
// #define CALL_LIST_PHOTO_TATOL_BTN_ID 7
// #define CALL_LIST_PHOTO_LEFT_BTN_ID 8
// #define CALL_LIST_PHOTO_RIGHT_BTN_ID 9

// int call_index_get(void)
// {
//     return call_index;
// }
// void call_index_set(int index)
// {
//     call_index = index;
// }

// void call_total_set(int index)
// {
//     call_index = index;
// }

// int call_total_get(void)
// {
//     return call_total;
// }

// typedef enum
// {
//     CALL_LIST_HOME_BTN,
//     CALL_LIST_IMAGE_BTN,
//     CALL_LIST_VIDEO_BTN,
//     CALL_LIST_CALL_BTN,
//     CALL_LIST_DELETE_BTN_ID,
//     CALL_LIST_TOTAL_BTN,
// } photo_btn_module;

// static custom_area call_list_btn_area[CALL_LIST_TOTAL_BTN] =
//     {
//         {24, 12, 54, 54},
//         {28, 410, 157, 170},
//         {30, 240, 155, 170},
//         {30, 70, 155, 170},
//         {954, 12, 48, 48},
// };

// static void call_list_home_btn_up(lv_obj_t *obj)
// {
//     goto_layout(pLAYOUT(home));
// }
// // 创建home按钮
// static void call_list_home_btn_create(lv_obj_t *parent)
// {
//     static obj_click_data btn_data = obj_click_data_up_create(call_list_home_btn_up);
//     static rom_bin_info info = rom_bin_info_get(ROM_UI_CAMERA_HOME_PNG);
//     camera_img_btn_create(parent, call_list_btn_area[CALL_LIST_HOME_BTN], NULL, &btn_data, &info);
// }
// /***
// **   日期:2022-05-23 16:28:24
// **   作者: leo.liu
// **   函数作用：页面显示
// **   参数说明:
// ***/
// static void playback_total_label_display(void)
// {
//     static char str[20];
//     lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), CALL_LIST_PHOTO_TATOL_BTN_ID);
//     int total_page_num = (call_total == 0) ? 0 : (call_total / 6 + ((call_total % 6) ? 1 : 0));
//     if (call_total == 0)
//     {
//         call_list_curr_page_num = 0;
//     }
//     sprintf(str, "%04d/%04d", call_list_curr_page_num, total_page_num);
//     lv_label_set_text(label, str);
//     lv_obj_set_style_local_text_color(label, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
//     lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(31));
// }

// static void call_list_image_btn_up(lv_obj_t *obj)
// {
//     goto_layout(pLAYOUT(photo_list));
// }
// // 创建image按钮
// static void call_list_image_btn_create(lv_obj_t *parent)
// {
//     static obj_click_data btn_data = obj_click_data_up_create(call_list_image_btn_up);
//     static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_PHOTO_PNG);
//     page_btn_create(parent, call_list_btn_area[CALL_LIST_IMAGE_BTN], str_get(LAYOUT_MEMORY_LANG_IMAGE_LISE_ID), &btn_data, &info);
// }

// static void call_list_video_btn_up(lv_obj_t *obj)
// {
//     goto_layout(pLAYOUT(video_list));
// }
// // 创建video按钮
// static void call_list_video_btn_create(lv_obj_t *parent)
// {
//     static obj_click_data btn_data = obj_click_data_up_create(call_list_video_btn_up);
//     static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_VIDEO_PNG);
//     page_btn_create(parent, call_list_btn_area[CALL_LIST_VIDEO_BTN], str_get(LAYOUT_MEMORY_LANG_VIDEO_LISE_ID), &btn_data, &info); // 修改显示文字为视频
// }

// static void call_list_call_btn_up(lv_obj_t *obj)
// {
//     goto_layout(pLAYOUT(call_list));
// }
// // 创建call按钮
// static void call_list_call_btn_create(lv_obj_t *parent)
// {
//     static obj_click_data btn_data = obj_click_data_up_create(call_list_call_btn_up);
//     static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_CALL_RED_PNG);
//     page_btn_create(parent, call_list_btn_area[CALL_LIST_CALL_BTN], str_get(LAYOUT_MEMORY_LANG_CALL_ID), &btn_data, &info);
// }

// // 顶部文案显示
// static void call_list_head_label_create(lv_obj_t *parent)
// {
//     lv_obj_t *set_page = lv_cont_create(parent, NULL);
//     lv_obj_set_pos(set_page, 200, 90);
//     lv_obj_set_size(set_page, 790, 45);
//     // set_location(photo_page, 180, 89, 748, 426);
//     lv_obj_set_style_local_value_color(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x201F1F));
//     lv_obj_set_style_local_value_align(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_RIGHT);
//     lv_obj_set_style_local_value_ofs_x(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 50);
//     lv_obj_set_style_local_value_font(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
//     lv_obj_t *label = lv_label_create(set_page, NULL);
//     lv_obj_set_pos(label, 60, 0);
//     lv_obj_set_size(label, 176, 29);
//     // lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 9, 44);
//     // if(user_data_get()->media_disp_mode == 0)
//     // {
//     lv_label_set_text(label, str_get(LAYOUT_MEMORY_LANG_CALL_TIME_ID));
//     // }
//     // else
//     // {
//     // 	lv_label_set_text(label, str_get(LAYOUT_MEMORY_LANG_CALL_LISE_ID));
//     // }

//     lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x697074));
//     lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

//     lv_obj_t *label2 = lv_label_create(set_page, NULL);
//     lv_obj_set_pos(label2, 388, 0);
//     lv_obj_set_size(label2, 171, 29);

//     lv_label_set_text(label2, str_get(LAYOUT_MEMORY_LANG_DEVICE_ID));

//     lv_obj_set_style_local_text_color(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x697074));
//     lv_obj_set_style_local_text_font(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

//     lv_obj_t *label3 = lv_label_create(set_page, NULL);
//     lv_obj_set_pos(label3, 630, 0);
//     lv_obj_set_size(label3, 167, 29);

//     lv_label_set_text(label3, str_get(LAYOUT_MEMORY_LANG_STATE_ID));

//     lv_obj_set_style_local_text_color(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x697074));
//     lv_obj_set_style_local_text_font(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

//     lv_obj_set_style_local_border_width(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
//     lv_obj_set_style_local_border_color(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x32, 0x32, 0x37));
//     lv_obj_set_style_local_border_color(set_page, LV_CONT_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27));
//     lv_obj_set_style_local_border_side(set_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
// }
// static lv_obj_t *call_list_page_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int i);
// // 添加点击事件处理函数
// static void call_list_btn_up(lv_obj_t *obj)
// {
//     int index = obj->obj_id;
//     printf("==========@@@@@@@@==[%d]======\n", index);

//     if (call_total == 0)
//     {
//         return;
//     }

//     // 清除新文件标志
//     int actual_index = call_total - index - 1;
//     printf("Clearing new flag for call record index: %d\n", actual_index);
//     call_record_new_clear(actual_index);

//     // 可选：立即刷新当前页面显示
//     // 这会让新文件立即变为旧文件的显示样式
//     lv_obj_clean(call_page);
//     for (int i = 0; ((page_num - call_list_curr_page_num) * 6 + i) < call_total; i++)
//     {
//         if (call_list_page_btn_create(call_page, 0, ((i) * 62), 800, 62, (page_num - call_list_curr_page_num) * 6 + i) == NULL)
//         {
//             i--;
//         }
//         if (i >= 6)
//             break;
//     }

//     // 这里可以根据需要决定是否跳转到其他页面
//     // goto_layout(pLAYOUT(some_other_layout));
// }
// static char phone_num_pool[6][32] = {
//     {0},
//     {0},
//     {0},
//     {0},
//     {0},
//     {0}};
// static int pool_index = 0; // 当前使用的缓冲区索引
// static lv_obj_t *call_list_page_btn_create(lv_obj_t *parent, int x, int y, int w, int h, int i)
// {

//     lv_obj_t *btn = lv_obj_create(parent, NULL);
//     lv_obj_set_id(btn, i);

//     lv_obj_set_drag_parent(btn, true);

//     lv_obj_set_pos(btn, x, y);
//     lv_obj_set_size(btn, w, h);
//     lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
//     lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
//     lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x3BD741));
//     lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x4f4f4f));

//     // 为按钮添加点击事件
//     static obj_click_data click_data = obj_click_data_up_create(call_list_btn_up);
//     obj_click_event_listen(btn, &click_data);

//     // 获取call记录信息
//     call_info = call_record_info_get(call_total - i - 1);

//     if (call_info == NULL)
//     {
//         printf("Failed to get call record info for index %d\n", call_total - i - 1);
//         return btn;
//     }

//     lv_color_t new_file_color_def = lv_color_hex(0xC52323); // 新文件-默认字体色
//     lv_color_t normal_color_def = lv_color_hex(0xFFFFFF);   // 非新文件-默认字体色

//     // 根据"是否为新文件"选择对应的颜色组
//     lv_color_t curr_color_def = (call_info->is_new) ? new_file_color_def : normal_color_def;

//     char year[32] = {0};
//     char month[32] = {0};
//     char day[32] = {0};
//     char hour[32] = {0};
//     char min[32] = {0};
//     char second[32] = {0};
//     static char file_year[64] = {0};

//     // 解析文件名中的时间信息
//     // 文件名格式: yymmdd-hhmmss-ch-mode.CAL
//     strncpy(year, call_info->file_name, 2);
//     strncpy(month, call_info->file_name + 2, 2);
//     strncpy(day, call_info->file_name + 4, 2);
//     strncpy(hour, call_info->file_name + 7, 2);
//     strncpy(min, call_info->file_name + 9, 2);
//     strncpy(second, call_info->file_name + 11, 2);

//     printf("------year[%s]\n", call_info->file_name);
//     printf("------year[%s]\n", year);
//     printf("------month[%s]\n", month);
//     printf("------day[%s]\n", day);
//     printf("------hour[%s]\n", hour);
//     printf("------min[%s]\n", min);
//     printf("------second[%s]\n", second);

//     struct date temp_date =
//         {
//             .year = atoi(year) + 2000,
//             .month = atoi(month),
//             .day = atoi(day)};
//     temp_date = gregorian2jalali(temp_date);

//     if (user_data_get()->setting.calendar == 1)
//     {
//         sprintf(file_year, "20%s-%s-%s %s:%s", year, month, day, hour, min);
//     }
//     else
//     {
//         sprintf(file_year, "%d-%02d-%02d %s:%s",
//                 temp_date.year, temp_date.month, temp_date.day,
//                 hour, min);
//     }

//     printf("---------------------file_year[%s]\n", file_year);

//     lv_obj_set_style_local_value_align(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
//     lv_obj_set_style_local_value_ofs_x(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 73);
//     lv_obj_set_style_local_value_font(btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));

//     // 显示门口机信息
//     lv_obj_t *obj = lv_obj_create(btn, NULL);
//     if (call_info->is_door_station)
//     {
//         if (call_info->ch == '1') // 门口机1
//         {
//             lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_DOOR1_ID));
//         }
//         else if (call_info->ch == '2') // 门口机2
//         {
//             lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, str_get(LAYOUT_HOME_LANG_DOOR2_ID));
//         }
//         else
//         {
//             lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Unknown");
//         }
//         lv_obj_align(obj, btn, LV_ALIGN_IN_LEFT_MID, 390, 5);
//     }
//     else
//     {
//         int current_index = pool_index % 6;
//         pool_index++; // 下次使用下一个缓冲区
//         if (user_data_get()->setting.language == LANG_ENGLISH)
//         {
//             sprintf(phone_num_pool[current_index], "%s%03d", str_get(COMMON_LANG_INTERCOM_ID), call_info->call_num);
//             lv_obj_align(obj, btn, LV_ALIGN_IN_LEFT_MID, 350, 5);
//         }
//         else
//         {
//             sprintf(phone_num_pool[current_index], "%03d%s", call_info->call_num, str_get(COMMON_LANG_INTERCOM_ID));
//             lv_obj_align(obj, btn, LV_ALIGN_IN_LEFT_MID, 440, 10);
//         }
//         printf("phone_num[%s],call_num[%d]\n", phone_num_pool[current_index], call_info->call_num);
//         lv_obj_set_style_local_value_str(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, phone_num_pool[current_index]);
//     }
//     lv_obj_set_style_local_value_color(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, curr_color_def); // 默认状态

//     lv_obj_set_click(obj, false);
//     lv_obj_set_style_local_value_font(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
//     if (user_data_get()->setting.language == LANG_ENGLISH)
//     {
//         lv_obj_set_style_local_value_align(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_LEFT_MID);
//     }
//     else
//     {
//         lv_obj_set_style_local_value_align(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_IN_RIGHT_MID);
//     }

//     // 显示时间
//     lv_obj_t *label2 = lv_label_create(btn, NULL);
//     lv_label_set_text(label2, file_year);
//     lv_label_set_align(label2, LV_LABEL_ALIGN_LEFT);
//     lv_obj_set_style_local_text_color(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, curr_color_def); // 默认状态
//     lv_obj_align(label2, btn, LV_ALIGN_IN_LEFT_MID, 30, 0);
//     lv_obj_set_style_local_text_font(label2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

//     // 显示接听状态
//     lv_obj_t *label3 = lv_label_create(btn, NULL);
//     lv_label_set_align(label3, LV_LABEL_ALIGN_CENTER);

//     if (call_info->answered)
//     {
//         lv_label_set_text(label3, str_get(LAYOUT_MEMORY_LANG_ANSWERED_ID)); // "已接听"
//         lv_obj_align(label3, btn, LV_ALIGN_IN_LEFT_MID, 605, 0);
//     }
//     else
//     {
//         lv_label_set_text(label3, str_get(LAYOUT_MEMORY_LANG_REJECT_ID)); // "未接听"
//         lv_obj_align(label3, btn, LV_ALIGN_IN_LEFT_MID, 625, 0);
//     }

//     lv_obj_set_style_local_text_font(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
//     lv_obj_set_style_local_text_color(label3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, curr_color_def); // 默认状态

//     lv_obj_set_style_local_border_width(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
//     lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0x32, 0x32, 0x37));
//     lv_obj_set_style_local_border_color(btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(0x22, 0x22, 0x27));
//     lv_obj_set_style_local_border_side(btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);

//     return btn;
// }

// static void call_list_prev_btn_up(lv_obj_t *obj)
// {
//     printf("the_prev_1\n");
//     if (--call_list_curr_page_num <= 0)
//     {
//         printf("the_prev_2\n");
//         call_list_curr_page_num = (call_total == 0) ? 1 : (call_total / 6 + ((call_total % 6) ? 1 : 0));
//         if (call_list_curr_page_num == 1)
//             return;
//     }
//     printf("the_prev_3\n");
//     playback_total_label_display();
//     lv_obj_clean(call_page);
//     printf("the_prev_4\n");
//     for (int i = 0; ((page_num - call_list_curr_page_num) * 6 + i) < call_total; i++)
//     {
//         printf("the_prev_5\n");
//         if (call_list_page_btn_create(call_page, 0, ((i) * 62), 800, 62, (page_num - call_list_curr_page_num) * 6 + i) == NULL)
//         {
//             i--;
//         }
//         if (i >= 6)
//             break;
//     }
// }

// static void call_list_next_btn_up(lv_obj_t *obj)
// {
//     printf("==============================================>>>>[%s]\n", __func__);
//     if (++call_list_curr_page_num > ((call_total == 0) ? 0 : (call_total / 6 + ((call_total % 6) ? 1 : 0))))
//     {
//         call_list_curr_page_num = 1;
//         if (((call_total == 0) ? 1 : (call_total / 6 + ((call_total % 6) ? 1 : 0))) == 1)
//             return;
//     }
//     playback_total_label_display();
//     lv_obj_clean(call_page);
//     for (int i = 0; ((page_num - call_list_curr_page_num) * 6 + i) < call_total; i++)
//     {
//         if (call_list_page_btn_create(call_page, 0, ((i) * 62), 800, 62, (page_num - call_list_curr_page_num) * 6 + i) == NULL)
//         {
//             i--;
//         }
//         if (i >= 6)
//             break;
//     }
// }

// static void call_list_page_display(lv_obj_t *parent)
// {
//     call_page = lv_cont_create(parent, NULL);
//     lv_obj_set_id(call_page, CALL_LIST_PHOTO_PAGE_BTN_ID);
//     lv_obj_set_pos(call_page, 200, 131);
//     lv_obj_set_size(call_page, 790, 373);
//     // set_location(call_page, 180, 89, 748, 426);
//     lv_obj_set_style_local_value_color(call_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x201F1F));
//     lv_obj_set_style_local_value_align(call_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_RIGHT);
//     lv_obj_set_style_local_value_ofs_x(call_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 50);
//     lv_obj_set_style_local_value_font(call_page, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));

//     // 获取call记录总数
//     call_record_total_get(&call_total, NULL);

//     int select_index = call_index_get() + 1;
//     page_num = (call_total / 6 + ((call_total % 6) ? 1 : 0));
//     call_list_curr_page_num = ((select_index <= 6) ? page_num : (page_num - ((select_index % 6) ? ((select_index / 6)) : (select_index / 6) - 1)));

//     for (int i = 0; ((page_num - call_list_curr_page_num) * 6 + i) < call_total; i++)
//     {
//         if (call_list_page_btn_create(call_page, 0, ((i) * 62), 800, 62, (page_num - call_list_curr_page_num) * 6 + i) == NULL)
//         {
//             i--;
//         }
//         if (i >= 6)
//             break;
//     }

//     lv_obj_t *prev_btn = lv_obj_create(lv_scr_act(), NULL);
//     static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_PREV_PNG);
//     lv_obj_set_pos(prev_btn, 404, 528);
//     lv_obj_set_size(prev_btn, 72, 72);
//     lv_obj_set_style_local_pattern_image(prev_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &info);

//     static obj_click_data btn_data1 = obj_click_data_up_create(call_list_prev_btn_up);
//     obj_click_event_listen(prev_btn, &btn_data1);

//     lv_obj_t *next_btn = lv_obj_create(lv_scr_act(), NULL);
//     static rom_bin_info info1 = rom_bin_info_get(ROM_UI_MEDIA_LIST_NEXT_PNG);
//     lv_obj_set_pos(next_btn, 670, 528);
//     lv_obj_set_size(next_btn, 72, 72);
//     lv_obj_set_style_local_pattern_image(next_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &info1);

//     static obj_click_data btn_data2 = obj_click_data_up_create(call_list_next_btn_up);
//     obj_click_event_listen(next_btn, &btn_data2);
// }

// static void delete_file_state_display_task(lv_task_t *task_t)
// {
//     lv_obj_t *arc = (lv_obj_t *)task_t->user_data;
//     arc_loader_update(arc, &start_angle);
//     if (delete_file_type != FILE_TYPE_NONE)
//     {

//         if (!media_file_delete_all_state())
//         {
//             delete_file_type = FILE_TYPE_NONE;
//             exit_time_count = 1;
//         }
//     }
//     else
//     {
//         if (--exit_time_count <= 0)
//         {
//             arc_loader_delete(arc);
//             goto_layout(pLAYOUT(call_list));
//         }
//     }
// }

// static void call_list_delete_yes_btn_up(lv_obj_t *obj)
// {
//     setting_msgdialog_msg_bg_delete(LAYOUT_MEMORY_LANG_DELETE_ALL_CALL_ID);
//     lv_obj_t *cont = setting_msgdialog_msg_bg_create(0, 100, 100);
//     // 2. 调用封装函数创建环形进度条
//     lv_arc_loader_create(
//         cont,
//         delete_file_state_display_task,
//         120,
//         lv_color_hex(0xFFFFFF),
//         lv_color_hex(0x007AFF),
//         11,
//         50,
//         0, 0);
//     if (media_sdcard_insert_check() == true)
//     {
//         delete_file_type = FILE_TYPE_CALL;
//         int total;
//         media_file_total_get(delete_file_type, &total, NULL);
//         if (total == 0)
//         {
//             delete_file_type = FILE_TYPE_NONE;
//             exit_time_count = 1;
//         }
//         else
//         {
//             exit_time_count = 1;
//             media_file_delete_all_start(delete_file_type);
//         }
//     }
//     else
//     {
//         delete_file_type = FILE_TYPE_NONE;
//         exit_time_count = 1;
//     }
//     user_data_get()->new_call_record_flag = 0;
// }

// static void call_list_delete_no_btn_up(lv_obj_t *obj)
// {
//     printf("no clicked\n");
//     setting_msgdialog_msg_bg_delete(LAYOUT_MEMORY_LANG_DELETE_ALL_CALL_ID);
// }
// static void call_list_delete_btn_up(lv_obj_t *obj)
// {
//     if (call_total == 0)
//         return;
//     lv_obj_t *cont = setting_msgdialog_msg_bg_create(LAYOUT_MEMORY_LANG_DELETE_ALL_CALL_ID, 460, 156);
//     static obj_click_data btn_data = obj_click_data_up_create(call_list_delete_yes_btn_up);
//     static obj_click_data btn_data1 = obj_click_data_up_create(call_list_delete_no_btn_up);
//     memory_message_box_create(cont, &btn_data, &btn_data1, LAYOUT_MEMORY_LANG_DELETE_ALL_CALL_ID);
// }

// // 创建delete按钮
// static void call_list_delete_btn_create(lv_obj_t *parent)
// {
//     static obj_click_data btn_data = obj_click_data_up_create(call_list_delete_btn_up);
//     static rom_bin_info info = rom_bin_info_get(ROM_UI_MEDIA_LIST_DELETE_PNG);
//     lv_obj_t *btn = camera_img_btn_create(parent, call_list_btn_area[CALL_LIST_DELETE_BTN_ID], NULL, &btn_data, &info);
//     lv_obj_set_id(btn, CALL_LIST_DELETE_BTN_ID);
// }

// /***
// **   日期:2022-05-23 16:25:07
// **   作者: leo.liu
// **   函数作用：显示页面创建
// **   参数说明:
// ***/
// static void playback_total_label_create(void)
// {
//     lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
//     lv_obj_set_id(label, CALL_LIST_PHOTO_TATOL_BTN_ID);
//     lv_label_set_long_mode(label, LV_LABEL_LONG_CROP);
//     lv_obj_set_pos(label, 468, 539);
//     lv_obj_set_size(label, 222, 34);
//     lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
//     playback_total_label_display();
// }

// static void LAYOUT_ENTER_FUNC(call_list)
// {
//     printf("============================enter_call_list\n");
//     user_data_get()->new_call_record_flag = false;

//     lv_obj_t *parent = common_bg_display(lv_scr_act());

//     call_list_home_btn_create(parent);

//     call_list_image_btn_create(parent);

//     call_list_video_btn_create(parent);

//     call_list_call_btn_create(parent);

//     call_list_head_label_create(parent);

//     call_list_page_display(parent);

//     call_list_delete_btn_create(parent);

//     playback_total_label_create();
// }

// static void LAYOUT_QUIT_FUNC(call_list)
// {
// }

// CREATE_LAYOUT(call_list);
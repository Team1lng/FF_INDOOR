#include "layout_define.h"
#include <stdio.h>
#include "lv_msg_event.h"
#include "lvgl/lvgl.h"
#include "lvgl/lv_obj.h"
#include "lvgl/lv_label.h"
#include "language.h"
#include "lvgl/lv_task.h"

#define HOME_BACK_OBJ_ID 0x10
#define HOME_GEAR_OBJ_ID 0x11
#define SETTING_format_SD_BIN_ID 0x12
#define SETTING_SD_CHECK_LABEL_ID 0x13

static bool is_no_sd_scenario = false; /* 是否处于无SD卡场景 */
static lv_obj_t *no_sd_label = NULL;
static int format_anim_count = 0;  /* Formating 点动画计数 */
static int format_exit_count = 0;  /* Format OK 后倒计时 */
static bool is_sd_formatting = false;
static lv_obj_t *format_state_label = NULL;
static lv_task_t *format_state_task = NULL;

static void format_status_text_set(lv_obj_t *label, int anim_idx)
{
	static const char *dots[3] = {".  ", ".. ", "..."};
	char buf[64];

	if (label == NULL)
	{
		return;
	}

	snprintf(buf, sizeof(buf), "%s%s",
			 str_get(LAYOUT_SD_LANG_FORMATING_ID),
			 dots[anim_idx % 3]);
	lv_label_set_text(label, buf);
	/* Fixed width: avoid "." / ".." / "..." width jump/flicker. */
	lv_obj_set_width(label, 220);
	lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_MID, 0, 160);
}

static void format_state_clear(void)
{
	if (format_state_task != NULL)
	{
		lv_task_del(format_state_task);
		format_state_task = NULL;
	}

	if (format_state_label != NULL)
	{
		lv_obj_del(format_state_label);
		format_state_label = NULL;
	}

	is_sd_formatting = false;
	format_anim_count = 0;
	format_exit_count = 0;
}

static void no_sd_label_show(void)
{
	is_no_sd_scenario = true;
	if (no_sd_label != NULL)
	{
		return;
	}

	no_sd_label = lv_label_create(lv_scr_act(), NULL);
	if (no_sd_label == NULL)
	{
		printf("Failed to create No SD label!\n");
		return;
	}

	lv_label_set_text(no_sd_label, str_get(COMMON_LANG_NO_SD_ID));
	lv_obj_set_style_local_text_font(no_sd_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_set_style_local_text_color(no_sd_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF3030));
	lv_obj_align(no_sd_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 160);
}

static void sd_card_detect_task(lv_task_t *task)
{
	bool sd_inserted = media_sdcard_insert_check();

	if (is_sd_formatting)
	{
		return;
	}

	if (!sd_inserted)
	{
		no_sd_label_show();
	}
	else
	{
		is_no_sd_scenario = false;
		if (no_sd_label != NULL)
		{
			lv_obj_del(no_sd_label);
			no_sd_label = NULL;
		}
	}
}

/* setting 图标 */
static void setting_icon_create(lv_obj_t *parent)
{
	lv_obj_t *setting_icon_obj = lv_img_create(parent, NULL);
	lv_obj_set_pos(setting_icon_obj, 54, 34);
	lv_obj_set_size(setting_icon_obj, 22, 20);
	lv_obj_set_id(setting_icon_obj, HOME_GEAR_OBJ_ID);
	static rom_bin_info info1 = rom_bin_info_get(ROM_UI_HOME_GEAR_PNG);
	lv_obj_set_style_local_pattern_image(setting_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info1);

	lv_obj_t *Time_label = lv_label_create(parent, NULL);
	lv_obj_set_pos(Time_label, 87, 28);
	lv_obj_set_size(Time_label, 70, 31);
	lv_label_set_text(Time_label, "Init");
	lv_obj_align(setting_icon_obj, Time_label, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}

static void back_btn_up(lv_obj_t *obj)
{
	format_state_clear();

	if (no_sd_label != NULL)
	{
		lv_obj_del(no_sd_label);
		no_sd_label = NULL;
	}
	is_no_sd_scenario = false;
	format_anim_count = 0;
	format_exit_count = 0;

	goto_layout(pLAYOUT(home));
}

static void reset_system_btn_up(lv_obj_t *obj)
{
	uint8_t lang = user_data_get()->setting.language;
	user_data_reset();
	if (user_data_get()->setting.language != lang)
	{
		extern void lv_ft_font_set_type(int type);
		lv_ft_font_set_type(user_data_get()->setting.language);

		extern void lv_font_afresh_init(void);
		lv_font_afresh_init();
	}
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
	lv_obj_set_style_local_pattern_recolor(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_hex(0x000000));
	lv_obj_set_style_local_pattern_recolor_opa(back_icon_obj, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
	static obj_click_data btn_data = obj_click_data_up_create(back_btn_up);
	obj_click_event_listen(back_icon_obj, &btn_data);
}

static void sd_format_state_display_task(lv_task_t *task_t)
{
	if (task_t == NULL)
	{
		printf("Error: task is NULL!\n");
		return;
	}

	lv_obj_t *state_label = task_t->user_data;
	if (state_label == NULL)
	{
		printf("Error: state_label is NULL!\n");
		lv_task_del(task_t);
		format_state_task = NULL;
		return;
	}

	/* No SD tip stays on screen. */
	if (is_no_sd_scenario)
	{
		return;
	}

	if (media_format_sd_state() == true)
	{
		/* Still formatting: only refresh dots on one fixed-width label. */
		format_status_text_set(state_label, format_anim_count++);
		if (format_anim_count > 1000)
		{
			format_anim_count = 0;
		}
		return;
	}

	/* Format finished / failed. */
	if (media_sdcard_insert_check() == false)
	{
		/* Card gone during/after format: go home, avoid No SD flash overlap. */
		format_state_clear();
		if (no_sd_label != NULL)
		{
			lv_obj_del(no_sd_label);
			no_sd_label = NULL;
		}
		is_no_sd_scenario = false;
		goto_layout(pLAYOUT(home));
		return;
	}

	is_sd_formatting = false;

	if (format_exit_count <= 0)
	{
		/* Show Format OK once, then count down. */
		lv_label_set_text(state_label, str_get(LAYOUT_SD_LANG_FORMAT_OK_ID));
		lv_obj_set_width(state_label, 220);
		lv_label_set_align(state_label, LV_LABEL_ALIGN_CENTER);
		lv_obj_align(state_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 160);
		format_exit_count = 15; /* ~3s @ 200ms */
		return;
	}

	format_exit_count--;
	if (format_exit_count > 0)
	{
		return;
	}

	format_state_clear();
	goto_layout(pLAYOUT(home));
}

static void no_sd_label_display(lv_obj_t *center_cont)
{
	if (is_sd_formatting)
	{
		return;
	}

	if (!media_sdcard_insert_check())
	{
		is_no_sd_scenario = true;
		no_sd_label = lv_label_create(lv_scr_act(), NULL);
		if (no_sd_label == NULL)
		{
			printf("Failed to create No SD label!\n");
			return;
		}
		lv_label_set_text(no_sd_label, str_get(COMMON_LANG_NO_SD_ID));
		lv_obj_set_style_local_text_font(no_sd_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
		lv_obj_set_style_local_text_color(no_sd_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF5555));
		lv_obj_align(no_sd_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 160);
	}
	else
	{
		is_no_sd_scenario = false;
		if (no_sd_label != NULL)
		{
			lv_obj_del(no_sd_label);
			no_sd_label = NULL;
		}
	}
}

/* 格式化按钮点击回调 */
static void format_sd_btn_up(lv_obj_t *obj)
{
	if (media_format_sd_state() || is_sd_formatting)
	{
		printf("Format is in progress, ignore click!\n");
		return;
	}

	if (no_sd_label != NULL)
	{
		lv_obj_del(no_sd_label);
		no_sd_label = NULL;
	}
	format_state_clear();
	is_no_sd_scenario = false;
	format_anim_count = 0;
	format_exit_count = 0;

	if (media_sdcard_insert_check())
	{
		is_sd_formatting = true;

		/* Single fixed-width label: Formating. / Formating.. / Formating... */
		lv_obj_t *state_label = lv_label_create(lv_scr_act(), NULL);
		if (state_label == NULL)
		{
			printf("Failed to create format state label!\n");
			is_sd_formatting = false;
			return;
		}
		format_state_label = state_label;
		lv_obj_set_style_local_text_font(state_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
		format_status_text_set(state_label, 0);

		/* Blocking format call; UI text is already set. */
		media_format_sd();

		format_anim_count = 1;
		format_exit_count = 0;
		format_state_task = lv_layout_task_create(sd_format_state_display_task, 200, LV_TASK_PRIO_LOWEST, state_label);
	}
	else
	{
		is_no_sd_scenario = true;
		no_sd_label = lv_label_create(lv_scr_act(), NULL);
		if (no_sd_label == NULL)
		{
			printf("Failed to create No SD label!\n");
			return;
		}

		lv_label_set_text(no_sd_label, str_get(COMMON_LANG_NO_SD_ID));
		lv_obj_set_style_local_text_font(no_sd_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
		lv_obj_set_style_local_text_color(no_sd_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF5555));
		lv_obj_align(no_sd_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 160);

		lv_layout_task_create(sd_format_state_display_task, 1000, LV_TASK_PRIO_LOWEST, no_sd_label);
	}
}

void create_init_page(lv_obj_t *center_cont)
{
	static obj_click_data format_sd_click_data = obj_click_data_up_create(format_sd_btn_up);
	custom_area area1 = {.x = 102, .y = 100, .w = 330, .h = 30};
	setting_right_btn_base_create(center_cont, area1.x, area1.y, area1.w, area1.h,
								  str_get(LAYOUT_SD_LANG_FORMAT_ID),
								  NULL,
								  &format_sd_click_data,
								  SETTING_format_SD_BIN_ID);

	static obj_click_data reset_system_click_data = obj_click_data_up_create(reset_system_btn_up);
	custom_area area2 = {.x = 102, .y = 167, .w = 330, .h = 50};
	setting_right_btn_base_create(center_cont, area2.x, area2.y, area2.w, area2.h,
								  str_get(COMMON_LANG_RESET_SYSTEM),
								  NULL,
								  &reset_system_click_data,
								  SETTING_format_SD_BIN_ID);
}

static void LAYOUT_ENTER_FUNC(init)
{
	lv_obj_t *parent = common_bg_display(lv_scr_act());
	lv_obj_t *center_cont = lv_cont_create(parent, NULL);
	lv_obj_set_size(center_cont, 534, 294);
	lv_obj_set_id(center_cont, 100);
	lv_obj_align(center_cont, parent, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_local_bg_color(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x838383));
	lv_obj_set_style_local_radius(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
	lv_obj_set_style_local_bg_opa(center_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_40);
	back_btn_create(parent);
	setting_icon_create(parent);
	top_time_date_text_create(parent);
	create_init_page(center_cont);

	no_sd_label_display(center_cont);
	format_anim_count = 0;
	format_exit_count = 0;

	lv_layout_task_create(sd_card_detect_task, 500, LV_TASK_PRIO_LOWEST, NULL);
}

static void LAYOUT_QUIT_FUNC(init)
{
	format_state_clear();
	is_no_sd_scenario = false;
	format_anim_count = 0;
	format_exit_count = 0;
}

CREATE_LAYOUT(init);

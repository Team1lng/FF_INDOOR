#!/bin/sh
set -eu

home_file="${1:-layout/layout_home.c}"
common_file="${2:-layout/layout_common.c}"
photo_list_file="${3:-layout/layout_photo_list.c}"

extract_func() {
	file="$1"
	name="$2"
	awk -v name="$name" '
		{
			line = $0
			sub(/\r$/, "", line)
			tmp = line
			sub(/^[[:space:]]*/, "", tmp)
			if (((index(tmp, name "(") == 1) || (tmp == name)) && line !~ /;/) printing = 1
		}
		printing {
			print line
			if (line == "}") exit
		}
	' "$file"
}

prepare_body=$(extract_func "$common_file" "void layout_media_audio_prepare")
release_body=$(extract_func "$common_file" "void layout_media_power_amplifier_release")
home_click_body=$(extract_func "$home_file" "static void home_click_down_func")
photo_click_body=$(extract_func "$photo_list_file" "static void photo_list_click_down_func")
photo_enter_body=$(extract_func "$photo_list_file" "static void LAYOUT_ENTER_FUNC(photo_list)")
photo_quit_body=$(extract_func "$photo_list_file" "static void LAYOUT_QUIT_FUNC(photo_list)")

printf '%s\n' "$prepare_body" | grep -q "power_amplifier_enable(true)"
printf '%s\n' "$prepare_body" | grep -q "audio_output_open"
printf '%s\n' "$release_body" | grep -q "lv_layout_task_create"
printf '%s\n' "$release_body" | grep -q "clean_lock = false"
! printf '%s\n' "$release_body" | grep -q "power_amplifier_enable(false)"

prepare_line=$(printf '%s\n' "$home_click_body" | grep -n "layout_media_audio_prepare()" | cut -d: -f1)
key_line=$(printf '%s\n' "$home_click_body" | grep -n "layout_obj_click_down_func(obj)" | cut -d: -f1)
[ "$prepare_line" -lt "$key_line" ]

photo_prepare_line=$(printf '%s\n' "$photo_click_body" | grep -n "layout_media_audio_prepare()" | cut -d: -f1)
photo_key_line=$(printf '%s\n' "$photo_click_body" | grep -n "layout_obj_click_down_func(obj)" | cut -d: -f1)
[ "$photo_prepare_line" -lt "$photo_key_line" ]

printf '%s\n' "$photo_enter_body" | grep -q "photo_list_click_down_func"
printf '%s\n' "$photo_quit_body" | grep -q "layout_media_power_amplifier_release"
grep -q "layout_media_power_amplifier_release_task" "$common_file"

echo "check_media_audio_transition: OK"

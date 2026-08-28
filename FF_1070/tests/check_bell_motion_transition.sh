#!/bin/sh
set -eu

common_file="${1:-layout/layout_common.c}"

call_body=$(awk '
    { sub(/\r$/, "") }
    /^void layout_call_camera_default\(void\)$/ { printing = 1 }
    printing { print }
    printing && /^}/ { exit }
' "$common_file")

prepare_line=$(printf '%s\n' "$call_body" | grep -n 'layout_motion_detection_prepare_camera_in()' | cut -d: -f1)
stop_line=$(printf '%s\n' "$call_body" | grep -n 'ringplay_play_stop_async()' | cut -d: -f1)

[ -n "$prepare_line" ]
[ "$prepare_line" -lt "$stop_line" ]

prepare_body=$(awk '
    /^void layout_motion_detection_prepare_camera_in\(void\)$/ { printing = 1 }
    printing { print }
    printing && /^}/ { exit }
' layout/layout_motion_detection.c)
printf '%s\n' "$prepare_body" | grep -q 'video_display_preview_enable(false)'
printf '%s\n' "$prepare_body" | grep -q 'lv_video_mode_enable(false)'
printf '%s\n' "$prepare_body" | grep -q 'video_input_resident_bzero()'
printf '%s\n' "$prepare_body" | grep -q 'screen_force_refresh()'

camera_enter_body=$(awk '
    /^static void LAYOUT_ENTER_FUNC\(camera\)$/ { printing = 1 }
    printing { print }
    printing && /^}/ { exit }
' layout/layout_camera.c)
mask_line=$(printf '%s\n' "$camera_enter_body" | grep -n 'camera_display_delay_start()' | cut -d: -f1)
monitor_line=$(printf '%s\n' "$camera_enter_body" | grep -n 'monitor_open(true' | cut -d: -f1)
[ -n "$mask_line" ]
[ -n "$monitor_line" ]
[ "$mask_line" -lt "$monitor_line" ]

echo "check_bell_motion_transition: OK"

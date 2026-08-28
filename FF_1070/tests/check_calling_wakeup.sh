#!/bin/sh
set -eu

calling_file="${1:-layout/layout_call_camera.c}"

extract_func() {
    awk -v name="$2" '
        { sub(/\r$/, "") }
        index($0, name) { printing = 1 }
        printing { print }
        printing && /^}/ { exit }
    ' "$1"
}

calling_enter=$(extract_func "$calling_file" "static void LAYOUT_ENTER_FUNC(calling)")
calling_container=$(extract_func "$calling_file" "lv_obj_t *calling_container_create")
calling_start=$(extract_func "$calling_file" "static void calling_play_start_cb")

printf '%s\n' "$calling_enter" | grep -Fq 'backlight_enable(true);'
printf '%s\n' "$calling_container" | grep -Fq 'lv_obj_set_size(yes_btn, 466, 80);'
printf '%s\n' "$calling_start" | grep -Fq 'power_amplifier_enable(true);'

echo "check_calling_wakeup: OK"

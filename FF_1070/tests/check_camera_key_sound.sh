#!/bin/sh
set -eu

camera_file="${1:-layout/layout_camera.c}"

camera_click_body=$(awk '
	{ sub(/\r$/, "") }
	/static void layout_camera_click_down_func\(lv_obj_t \*obj\)$/ { found = 1; next }
	found && /^\{/ { body = 1; next }
	body && /^\}/ { exit }
	body { print }
' "$camera_file")

printf '%s\n' "$camera_click_body" | grep -Fq 'return;'
if printf '%s\n' "$camera_click_body" | grep -Fq 'layout_obj_click_down_func'; then
	echo "camera monitor must not play local key tones" >&2
	exit 1
fi

echo "check_camera_key_sound: OK"

#!/bin/sh
set -eu

common_file="${1:-layout/layout_common.c}"

interrupt_body=$(awk '
    { sub(/\r$/, "") }
    /^static void layout_interrupt_intercom_for_door_call\(void\)$/ { printing = 1 }
    printing { print }
    printing && /^}/ { exit }
' "$common_file")

calling_body=$(awk '
    { sub(/\r$/, "") }
    /^void layout_call_camera_default\(void\)$/ { printing = 1 }
    printing { print }
    printing && /^}/ { exit }
' "$common_file")

printf '%s\n' "$interrupt_body" | grep -Fq 'ringplay_play_stop_async();'
printf '%s\n' "$interrupt_body" | grep -Fq 'MsgCallEnd();'
printf '%s\n' "$calling_body" | grep -Fq 'layout_interrupt_intercom_for_door_call();'
printf '%s\n' "$calling_body" | grep -Fq 'ringplay_play_stop_async();'

for file in layout/layout_intercom_in.c layout/layout_intercom_out.c; do
    grep -Fq 'ringplay_play_stop_async();' "$file"
done

camera_call_body=$(awk '
    { sub(/\r$/, "") }
    /^static void camera_door_call_switch\(MON_CH target_ch\)$/ { printing = 1 }
    printing { print }
    printing && /^}/ { exit }
' layout/layout_camera.c)

printf '%s\n' "$camera_call_body" | grep -Fq 'ringplay_play_stop_async();'
if printf '%s\n' "$camera_call_body" | grep -Fq 'ringplay_play_stop();'; then
    echo "camera door call path still waits synchronously for ring playback" >&2
    exit 1
fi

echo "check_external_call_preempts_intercom: OK"

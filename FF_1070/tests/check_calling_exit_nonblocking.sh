#!/bin/sh
set -eu

file="${1:-layout/layout_call_camera.c}"

body=$(awk '
    {
        line = $0
        tmp = line
        sub(/^[[:space:]]*/, "", tmp)
        if (index(tmp, "static void LAYOUT_QUIT_FUNC(calling)") == 1 && line !~ /;/) printing = 1
    }
    printing {
        print line
        if (line == "}") exit
    }
' "$file")

if printf '%s\n' "$body" | grep -q 'ringplay_play_stop('; then
    echo "calling layout quit must not synchronously wait for ring playback" >&2
    exit 1
fi

printf '%s\n' "$body" | grep -q 'ringplay_play_stop_async()'
printf '%s\n' "$body" | grep -q 'layout_media_power_amplifier_release()'
printf '%s\n' "$body" | grep -q 'obj_click_event_listen(lv_scr_act(), NULL)'

enter_body=$(awk '
    {
        line = $0
        tmp = line
        sub(/^[[:space:]]*/, "", tmp)
        if (index(tmp, "static void LAYOUT_ENTER_FUNC(calling)") == 1 && line !~ /;/) printing = 1
    }
    printing {
        print line
        if (line == "}") exit
    }
' "$file")
printf '%s\n' "$enter_body" | grep -q 'obj_click_event_listen(parent, &yes_btn_data)'

echo "check_calling_exit_nonblocking: OK"

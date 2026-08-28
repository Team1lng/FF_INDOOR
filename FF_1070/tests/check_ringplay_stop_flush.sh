#!/bin/sh
set -eu

ringplay_file="${1:-common/ringplay.c}"

# A stop must invalidate messages already taken by the worker, discard queued
# messages, and reset AO when a ringtone has already entered the device.
grep -q 'generation' "$ringplay_file"
grep -q 'ringplay_queue_clear_locked' "$ringplay_file"

stop_body=$(awk '
    /^void ringplay_play_stop_async\(void\)$/ { printing = 1 }
    printing {
        print
        if ($0 == "}") exit
    }
' "$ringplay_file")

printf '%s\n' "$stop_body" | grep -q 'ringplay_queue_clear_locked'
printf '%s\n' "$stop_body" | grep -q 'audio_output_device_restart'

play_body=$(awk '
    /^static bool ringplay_playbase\(/ { printing = 1 }
    printing {
        print
        if ($0 == "}") exit
    }
' "$ringplay_file")

printf '%s\n' "$play_body" | grep -q 'ringplay_queue_clear_locked'
printf '%s\n' "$play_body" | grep -q 'ringplay_request_generation'

# Replacing an active ring must not block the caller (normally LVGL).  The
# request is queued, then the ringplay worker performs the AO reset.
grep -q 'bool restart_output;' "$ringplay_file"
printf '%s\n' "$play_body" | grep -q 'info.msg.restart_output = is_ringplay_playing'
if printf '%s\n' "$play_body" | grep -q 'audio_output_device_restart'; then
	echo "ringplay_playbase must not restart AO in the caller thread" >&2
	exit 1
fi

task_body=$(awk '
    /^static void \*ringplay_task\(void \*arg\)$/ { printing = 1 }
    printing {
        print
        if ($0 ~ /play_data\.msg\.restart_output/)
            saw_restart = 1
        if (saw_restart && $0 ~ /audio_output_device_restart/)
            found = 1
    }
    END { if (!found) exit 1 }
' "$ringplay_file") || {
	echo "ringplay task must reset AO for a queued replacement" >&2
	exit 1
}

# MP3 decoding runs beside the single LVGL thread.  Every AO write must yield
# so a long ringtone cannot starve countdown rendering.
grep -q 'static void ringplay_audio_write' "$ringplay_file"
grep -q 'usleep(1000)' "$ringplay_file"

echo "check_ringplay_stop_flush: OK"

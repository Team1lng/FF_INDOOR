#!/bin/sh
set -eu

camera_file="${1:-layout/layout_camera.c}"
common_file="${2:-layout/layout_common.c}"

grep -q "camera_channel_switch_request" "$camera_file"
grep -q "camera_channel_switch_task" "$camera_file"
grep -q "CAMERA_CHANNEL_SWITCH_POLL_MS" "$camera_file"

extract_func_from() {
	file="$1"
	func_name="$2"
	awk -v name="$func_name" '
		{
			line = $0
			sub(/\r$/, "", line)
		}
		index(line, name) && line !~ /;/ {
			printing = 1
		}
		printing {
			print line
			if (line == "}") {
				exit
			}
		}
	' "$file"
}

extract_func() {
	extract_func_from "$camera_file" "$1"
}

manual_body=$(extract_func "static void camera_channel_switch_internal")
call_body=$(extract_func "static void camera_door_call_switch")
switch_task_body=$(extract_func "static void camera_channel_switch_task")
auto_record_body=$(extract_func "static void door_call_auto_camere")
auto_record_ready_body=$(extract_func "static bool camera_call_auto_record_start_if_ready")
switch_complete_body=$(extract_func "static void camera_channel_switch_complete")
ring_start_body=$(extract_func "static void camera_call_ring_start")
ring_play_body=$(extract_func "static void camera_call_ring_play")
ring_replay_body=$(extract_func "static bool camera_call_ring_should_replay")
ring_finish_body=$(extract_func "static void layout_camera_callring_finish_default_func")
unlock_ring_finish_body=$(extract_func "static void camera_unlock_ring_finish_func")
countdown_body=$(extract_func "static void camera_head_monitor_count_flush")
camera_ticker_body=$(extract_func "static void camera_ticker_task")
hook_answer_body=$(extract_func "void layout_camera_hook_answer")
hook_hangup_body=$(extract_func "void layout_camera_hook_hangup")
hook_event_body=$(extract_func_from "$common_file" "bool layout_hook_state_change_default")

printf '%s\n' "$manual_body" | grep -q "camera_channel_switch_request"
printf '%s\n' "$call_body" | grep -q "camera_channel_switch_request"

if printf '%s\n%s\n' "$manual_body" "$call_body" | grep -Eq 'while \(!video_input_state_get\(\)|usleep\('; then
	echo "blocking VI wait remains in a channel switch entry point" >&2
	exit 1
fi

complete_count=$(printf '%s\n' "$switch_task_body" | grep -c "camera_channel_switch_complete" || true)
if [ "$complete_count" -lt 2 ]; then
	echo "channel switch timeout does not restore preview and remove the mask" >&2
	exit 1
fi

printf '%s\n' "$auto_record_body" | grep -q "camera_call_auto_record_start_if_ready"
printf '%s\n' "$auto_record_ready_body" | grep -q "camera_auto_record_ready"
printf '%s\n' "$auto_record_ready_body" | grep -q "camera_call_auto_record_task_t = NULL"
printf '%s\n' "$auto_record_ready_body" | grep -q "camera_timeout_value_reset"
printf '%s\n' "$switch_complete_body" | grep -q "camera_call_auto_record_start_if_ready"
if printf '%s\n' "$switch_complete_body" | grep -q "camera_call_auto_record_start();"; then
	echo "channel switch completion starts recording before video readiness is confirmed" >&2
	exit 1
fi
if printf '%s\n' "$ring_start_body" | grep -q "CAMERA_CALL_RING_DELAY_MS"; then
	echo "door-call ringtone still waits for a fixed start delay" >&2
	exit 1
fi
if printf '%s\n' "$ring_start_body" | grep -q "camera_call_auto_record_task_t"; then
	echo "door-call ringtone still waits for automatic recording readiness" >&2
	exit 1
fi
printf '%s\n' "$ring_start_body" | grep -q "camera_call_ring_play"
if printf '%s\n' "$ring_play_body" | grep -q "ringplay_play_form_index"; then
	echo "initial door-call ringtone bypasses the indoor volume guard" >&2
	exit 1
fi
printf '%s\n' "$ring_play_body" | grep -q "ring_play(index, 100"
printf '%s\n' "$call_body" | grep -q "camera_call_ring_start"
printf '%s\n' "$ring_finish_body" | grep -q "camera_call_ring_should_replay"
if printf '%s\n' "$ring_finish_body" | grep -q "ringplay_play_form_index"; then
	echo "door-call ringtone replay bypasses the indoor volume guard" >&2
	exit 1
fi
printf '%s\n' "$ring_finish_body" | grep -q "ring_play(index, 100"
if printf '%s\n' "$unlock_ring_finish_body" | grep -q "camera_call_ring_should_replay"; then
	echo "door-call ringtone replay leaked into the unlock-tone callback" >&2
	exit 1
fi
if printf '%s\n' "$ring_replay_body" | grep -q "hook_state_get() == false"; then
	echo "raised handset prevents the configured ringtone duration from completing" >&2
	exit 1
fi
printf '%s\n' "$switch_complete_body" | grep -q "reason == CAMERA_CHANNEL_SWITCH_MANUAL"
if printf '%s\n' "$switch_complete_body" | grep -q "ringplay_play_stop"; then
	echo "channel readiness stops the ringtone when the handset is raised" >&2
	exit 1
fi
printf '%s\n' "$countdown_body" | grep -q "camera_call_auto_record_task_t"
printf '%s\n' "$countdown_body" | grep -q "video_input_state_get"

if [ -z "$hook_answer_body" ] || [ -z "$hook_hangup_body" ]; then
	echo "camera hook answer/hangup entry points are missing" >&2
	exit 1
fi
answer_mark_line=$(printf '%s\n' "$hook_answer_body" | grep -n "camera_call_ring_answered = true" | head -n 1 | cut -d: -f1)
answer_stop_line=$(printf '%s\n' "$hook_answer_body" | grep -n "ringplay_play_stop" | head -n 1 | cut -d: -f1)
if [ "$answer_mark_line" -ge "$answer_stop_line" ]; then
	echo "camera answer stops playback before disabling ringtone replay" >&2
	exit 1
fi
printf '%s\n' "$hook_answer_body" | grep -q "call_ring_to_outdoor_ctrl"
printf '%s\n' "$hook_answer_body" | grep -q "door_audio_talk"
printf '%s\n' "$hook_hangup_body" | grep -q "camera_call_ring_cancel"
printf '%s\n' "$hook_hangup_body" | grep -q "ringplay_play_stop"
printf '%s\n' "$hook_hangup_body" | grep -q "camera_call_auto_record_task_cancel"
printf '%s\n' "$hook_hangup_body" | grep -q "camera_channel_switch_cancel"
printf '%s\n' "$hook_hangup_body" | grep -q "door_audio_talk(AUDIO_CH_CLOSE)"
printf '%s\n' "$hook_hangup_body" | grep -q "video_display_preview_enable(false)"
printf '%s\n' "$hook_hangup_body" | grep -q "goto_layout(pLAYOUT(standby))"
printf '%s\n' "$hook_event_body" | grep -q "layout_camera_hook_answer()"
printf '%s\n' "$hook_event_body" | grep -q "layout_camera_hook_hangup()"
if printf '%s\n' "$camera_ticker_body" | grep -q "hook_state_get"; then
	echo "camera ticker still duplicates immediate handset event handling" >&2
	exit 1
fi

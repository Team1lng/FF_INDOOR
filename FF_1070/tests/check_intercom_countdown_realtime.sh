#!/bin/sh
set -eu

check_file() {
	file="$1"
	func="$2"

	body=$(awk -v name="$func" '
		{
			line = $0
			tmp = line
			sub(/^[[:space:]]*/, "", tmp)
			if (index(tmp, name) == 1 && line !~ /;/) printing = 1
		}
		printing {
			print line
			if (line == "}") exit
		}
	' "$file")

	printf '%s\n' "$body" | grep -q 'lv_tick_elaps'
	printf '%s\n' "$body" | grep -q 'INTERCOM_COUNTDOWN_TOTAL_S'
	printf '%s\n' "$body" | grep -q 'countdown_last_display_tick'
	printf '%s\n' "$body" | grep -q 'task_t'
	printf '%s\n' "$body" | grep -q 'lv_tick_elaps(countdown_last_display_tick) >= 1000'
	printf '%s\n' "$body" | grep -q 'countdown_last_display_tick = lv_tick_get()'
	printf '%s\n' "$body" | grep -q 'lv_label_set_text_fmt(lbl, "%02dS", count)'
	if ! printf '%s\n' "$body" | grep -q -- 'count--'; then
		echo "$file: countdown display must advance one second at a time" >&2
		exit 1
	fi

}

check_file layout/layout_intercom_out.c 'static void intercom_out_calling_time_out_task'
check_file layout/layout_intercom_in.c 'static void intercom_in_calling_time_out_task'

grep -Fq '#define INTERCOM_COUNTDOWN_TASK_PERIOD_MS    100' layout/layout_intercom_out.c
grep -Fq '#define INTERCOM_COUNTDOWN_TASK_PERIOD_MS 100' layout/layout_intercom_in.c
grep -Fq 'INTERCOM_COUNTDOWN_TASK_PERIOD_MS' layout/layout_intercom_out.c
grep -Fq 'INTERCOM_COUNTDOWN_TASK_PERIOD_MS' layout/layout_intercom_in.c

# The incoming ringtone must be queued when the callee page enters, not only
# after the first three-second countdown interval.
in_enter_body=$(awk '
    /^static void LAYOUT_ENTER_FUNC\(intercom_in\)$/ { printing = 1 }
    printing {
        print
        if ($0 == "}") exit
    }
' layout/layout_intercom_in.c)
printf '%s\n' "$in_enter_body" | grep -q 'intercom_in_ring_play()'

ring_line=$(grep -n 'intercom_in_ring_play();' layout/layout_intercom_in.c | tail -1 | cut -d: -f1)
parent_line=$(grep -n 'lv_obj_t \*parent = common_bg_display' layout/layout_intercom_in.c | tail -1 | cut -d: -f1)
if [ "$ring_line" -ge "$parent_line" ]; then
	 echo "layout_intercom_in: ringtone must be queued before UI creation" >&2
	 exit 1
fi

echo "check_intercom_countdown_realtime: OK"

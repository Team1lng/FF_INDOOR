#!/bin/sh
set -eu

camera_file="${1:-layout/layout_camera.c}"

extract_func() {
	file="$1"
	name="$2"
	awk -v name="$name" '
		{
			line = $0
			sub(/\r$/, "", line)
			tmp = line
			sub(/^[[:space:]]*/, "", tmp)
			if (index(tmp, name) == 1 && line !~ /;/) printing = 1
		}
		printing {
			print line
			if (line == "}") exit
		}
	' "$file"
}

camera_quit=$(extract_func "$camera_file" "static void LAYOUT_QUIT_FUNC(camera)")

if printf '%s\n' "$camera_quit" | grep -Eq 'while|usleep|ringplay_play_stop\('; then
	echo "camera layout quit must not synchronously wait on media shutdown" >&2
	exit 1
fi

if printf '%s\n' "$camera_quit" | grep -Eq 'user_data_save|system[[:space:]]*\('; then
	echo "camera layout quit must not synchronously sync persistent data" >&2
	exit 1
fi

printf '%s\n' "$camera_quit" | grep -q "monitor_close()"
printf '%s\n' "$camera_quit" | grep -q "record_video_close()"

echo "check_camera_exit_nonblocking: OK"

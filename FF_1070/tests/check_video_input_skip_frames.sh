#!/bin/sh
set -eu

video_input_file="${1:-common/video_input.c}"

grep -q "#define VIDEO_INPUT_STARTUP_SKIP_FRAMES 2" "$video_input_file"
grep -q "video_input_skip_frame_count = VIDEO_INPUT_STARTUP_SKIP_FRAMES" "$video_input_file"

if grep -q "video_input_skip_frame_count = 5" "$video_input_file"; then
	echo "video input startup still skips five frames" >&2
	exit 1
fi

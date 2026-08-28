#!/bin/sh
set -eu

ringplay_file="${1:-common/ringplay.c}"

# Index 0 must use the packaged MP3 file, not the legacy ROM PCM descriptor.
grep -Fq '{"0.mp3", 1, AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 0}' "$ringplay_file"

# Short key tones must be written even when they never fill the long-ring cache.
grep -q 'is_touch_sound' "$ringplay_file"
grep -Fq 'audio_output_write((unsigned char *)sample_buffer, sample_read_size)' "$ringplay_file"

echo "check_key_sound_mp3: OK"

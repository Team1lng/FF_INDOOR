#!/bin/sh
set -eu

camera_file="${1:-layout/layout_camera.c}"

hangup_body=$(awk '
    /void layout_camera_hook_hangup\(void\)/ { printing = 1 }
    printing {
        print
        opens = gsub(/\{/, "{", $0)
        closes = gsub(/\}/, "}", $0)
        depth += opens - closes
        if (depth == 0 && opens > 0) {
            exit
        }
    }
' "$camera_file")

if ! printf '%s\n' "$hangup_body" | awk '
    /door_audio_talk\(AUDIO_CH_CLOSE\)/ && !in_talk_branch {
        exit 1
    }
    /if \(camera_in_talk_state\)/ {
        in_talk_branch = 1
    }
    /door_audio_talk\(AUDIO_CH_CLOSE\)/ && in_talk_branch {
        close_in_talk_branch = 1
    }
    END {
        exit close_in_talk_branch ? 0 : 1
    }
'; then
    echo "manual camera exit still closes PA through AUDIO_CH_CLOSE" >&2
    exit 1
fi

printf '%s\n' "$hangup_body" | grep -q 'camera_in_talk_state = false'

#!/bin/sh
set -eu

check_no_sync_save_in_quit() {
    file="$1"
    layouts="${2:-intercom intercom_in intercom_out intercom_talk}"
    for layout in $layouts; do
        body=$(awk -v name="static void LAYOUT_QUIT_FUNC($layout)" '
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

        if printf '%s\n' "$body" | grep -Eq 'user_data_save|system[[:space:]]*\('; then
            echo "$file: $layout quit must not synchronously save or sync data" >&2
            exit 1
        fi
    done
}

check_no_sync_save_in_quit layout/layout_intercom.c
check_no_sync_save_in_quit layout/layout_intercom_in.c
check_no_sync_save_in_quit layout/layout_intercom_out.c
check_no_sync_save_in_quit layout/layout_intercom_talk.c
check_no_sync_save_in_quit layout/layout_home.c home

msg_call_end=$(awk '
    /^void MsgCallEnd\(void\)/ { printing = 1 }
    printing {
        print
        if ($0 == "}") exit
    }
' layout/intercom.c)

if printf '%s\n' "$msg_call_end" | grep -Eq 'BusDelay|usleep'; then
    echo "MsgCallEnd must not sleep in the LVGL event thread" >&2
    exit 1
fi

echo "check_interactive_exit_nonblocking: OK"

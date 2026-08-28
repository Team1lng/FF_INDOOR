#!/bin/sh
set -eu

wait_file="${1:-layout/page_ITC_WAIT.c}"
common_file="${2:-layout/layout_common.c}"
camera_file="${3:-layout/layout_camera.c}"
intercom_file="${4:-layout/intercom.c}"
interface_file="${5:-layout/intercom_interface.c}"

close_body=$(awk '
    /^void IntercomColseAudio\(void\)$/ { printing = 1 }
    printing {
        print
        opens = gsub(/\{/, "{", $0)
        closes = gsub(/\}/, "}", $0)
        depth += opens - closes
        if (depth == 0 && opens > 0) exit
    }
' "$wait_file")

printf '%s\n' "$close_body" | grep -Fq 'intercom_door_call_audio_hold_get()'
printf '%s\n' "$close_body" | grep -Fq 'CH OFF ignored: door call owns audio'

interrupt_body=$(awk '
    /^static void layout_interrupt_intercom_for_door_call\(void\)$/ { printing = 1 }
    printing {
        print
        opens = gsub(/\{/, "{", $0)
        closes = gsub(/\}/, "}", $0)
        depth += opens - closes
        if (depth == 0 && opens > 0) exit
    }
' "$common_file")
printf '%s\n' "$interrupt_body" | grep -Fq 'intercom_door_call_audio_hold_set(true);'
end_line=$(awk '/^static void layout_interrupt_intercom_for_door_call\(void\)$/ { in_func=1 } in_func && /MsgCallEnd\(\);/ { print NR; exit }' "$common_file")
hold_line=$(awk '/^static void layout_interrupt_intercom_for_door_call\(void\)$/ { in_func=1 } in_func && /intercom_door_call_audio_hold_set\(true\);/ { print NR; exit }' "$common_file")
[ "$end_line" -lt "$hold_line" ]

camera_quit_body=$(awk '
    /^static void LAYOUT_QUIT_FUNC\(camera\)$/ { printing = 1 }
    printing {
        print
        opens = gsub(/\{/, "{", $0)
        closes = gsub(/\}/, "}", $0)
        depth += opens - closes
        if (depth == 0 && opens > 0) exit
    }
' "$camera_file")
printf '%s\n' "$camera_quit_body" | grep -Fq 'intercom_door_call_audio_hold_set(false);'

request_body=$(awk '
    /^void MsgCallRequest\(unsigned char id\)$/ { printing = 1 }
    printing {
        print
        opens = gsub(/\{/, "{", $0)
        closes = gsub(/\}/, "}", $0)
        depth += opens - closes
        if (depth == 0 && opens > 0) exit
    }
' "$intercom_file")
printf '%s\n' "$request_body" | grep -Fq 'intercom_door_call_audio_hold_set(0);'

uart_body=$(awk '
    /^static void \*uartItcTask\(void \*arg\)$/ { printing = 1 }
    printing {
        print
        opens = gsub(/\{/, "{", $0)
        closes = gsub(/\}/, "}", $0)
        depth += opens - closes
        if (depth == 0 && opens > 0) exit
    }
' "$interface_file")
printf '%s\n' "$uart_body" | grep -Fq '[intercom_uart] alive'

echo "check_intercom_door_call_audio: OK"

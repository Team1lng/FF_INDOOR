# Camera Channel Async Switch Design

## Goal

Optimize Door1/Door2 manual and incoming-call channel switching without showing
stale frames, blocking the LVGL thread, or changing the existing hardware power
selection policy.

During a switch, the target channel UI must appear immediately while only the
video area remains black. The new video is displayed after VI reports a stable
frame.

## Current Problem

`layout_camera.c` has separate manual-switch and door-call-switch paths. Both
paths synchronously poll `video_input_state_get()` with `usleep()`, which can
block LVGL for up to one second. This makes the current frame appear frozen and
delays UI refresh.

Door1 and Door2 use the same video format, but they share one TP9950/VI path.
The current power-selection function also disables the inactive door station,
so true parallel target-channel pre-initialization is not available.

## Proposed Design

### Shared asynchronous state machine

Manual switching and incoming-call switching use one switch controller with
these stages:

1. Cancel any previous switch task and store the new target channel and reason.
2. Stop preview and clear the resident video buffer.
3. Reset recording, capture, unlock, color-window, and transient camera UI.
4. Select the target channel, refresh its label/buttons, create a black mask
   covering only the video area, and force one LVGL refresh.
5. Close transient resources and call the existing `monitor_open(false, 0x03)`
   interface to select and detect the target TP9950 input.
6. Poll `video_input_state_get()` from an LVGL task instead of blocking the UI.
7. When VI is ready, enable preview, remove the video mask, and run the
   reason-specific completion action.

### Timing and retry policy

- Poll interval: 20 ms.
- First attempt timeout: 700 ms.
- On first timeout: reselect the target VIN, restart TP9950 detection, clear the
  resident buffer, and retry once.
- Final timeout: 1,400 ms total. Remove the switch task but retain the black
  video area if no valid signal is available.
- Keep the existing five-frame VI warm-up. Reducing it risks unstable or stale
  frames and is outside this change.

### Completion behavior

For a manual switch, restore raised-handset talk only for Door1/Door2; CCTV
continues to reject talk audio.

For an incoming door call, start the automatic capture/record task after the
target video is ready. Then start the existing delayed ringtone flow so audio
routing is stable before ringtone playback.

If another call or manual switch arrives while switching, the newest target
replaces the pending request. Stale task callbacks must verify a generation ID
before changing preview, UI, audio, or recording state.

## Scope

Primary changes are limited to `FF_1070/layout/layout_camera.c`. Existing
`monitor_open()`, TP9950 selection, video-input readiness, power control, and
five-frame warm-up behavior remain unchanged.

## Validation

- Repeated Door1 to Door2 and Door2 to Door1 manual switching.
- Door2 calls while Door1 is displaying, and the reverse direction.
- Repeated alternating calls while a previous channel is still initializing.
- Raised handset switches between Door1/Door2 and then to CCTV.
- Automatic photo and video recording after an incoming call.
- Target UI remains responsive and visible while only the video area is black.
- No old frame, large black overlay, frozen countdown, or stale recording UI.

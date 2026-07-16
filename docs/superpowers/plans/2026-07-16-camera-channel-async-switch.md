# Camera Channel Async Switch Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace blocking Door1/Door2 channel switching with one asynchronous flow that refreshes the target UI immediately and reveals video only after VI is ready.

**Architecture:** `layout_camera.c` will own one file-local switch state and LVGL task. Manual switches and incoming-call switches submit requests to the same controller; request setup handles UI and channel selection, while the task handles readiness polling, one retry, timeout, and reason-specific completion.

**Tech Stack:** C, LVGL tasks, TP9950/VI monitor APIs, CMake/Make firmware build

---

### Task 1: Add a structural regression check

**Files:**
- Create: `FF_1070/tests/check_camera_async_switch.sh`
- Test: `FF_1070/layout/layout_camera.c`

- [ ] **Step 1: Create the failing structural test**

```sh
#!/bin/sh
set -eu

camera_file="${1:-layout/layout_camera.c}"

grep -q "camera_channel_switch_request" "$camera_file"
grep -q "camera_channel_switch_task" "$camera_file"
grep -q "CAMERA_CHANNEL_SWITCH_POLL_MS" "$camera_file"

manual_body=$(sed -n '/static void camera_channel_switch_internal/,/^}/p' "$camera_file")
call_body=$(sed -n '/static void camera_door_call_switch/,/^}/p' "$camera_file")

printf '%s\n' "$manual_body" | grep -q "camera_channel_switch_request"
printf '%s\n' "$call_body" | grep -q "camera_channel_switch_request"

if printf '%s\n%s\n' "$manual_body" "$call_body" | grep -Eq 'while \(!video_input_state_get\(\)|usleep\('; then
    echo "blocking VI wait remains in a channel switch entry point" >&2
    exit 1
fi
```

- [ ] **Step 2: Run the check and verify it fails before implementation**

Run: `cd FF_1070 && sh tests/check_camera_async_switch.sh`

Expected: failure because `camera_channel_switch_request` and `camera_channel_switch_task` do not exist yet.

### Task 2: Introduce the shared asynchronous switch controller

**Files:**
- Modify: `FF_1070/layout/layout_camera.c`
- Test: `FF_1070/tests/check_camera_async_switch.sh`

- [ ] **Step 1: Add timing constants and switch state**

Add these constants near the existing Camera timing constants:

```c
#define CAMERA_CHANNEL_SWITCH_POLL_MS 20
#define CAMERA_CHANNEL_SWITCH_RETRY_MS 700
#define CAMERA_CHANNEL_SWITCH_TIMEOUT_MS 1400
```

Add a file-local reason and state:

```c
typedef enum
{
	CAMERA_CHANNEL_SWITCH_MANUAL = 0,
	CAMERA_CHANNEL_SWITCH_CALL,
} CAMERA_CHANNEL_SWITCH_REASON;

typedef struct
{
	lv_task_t *task;
	MON_CH target_ch;
	CAMERA_CHANNEL_SWITCH_REASON reason;
	int tone_index;
	int elapsed_ms;
	bool retried;
} camera_channel_switch_state_t;

static camera_channel_switch_state_t camera_channel_switch_state = {
	.task = NULL,
	.target_ch = MON_CH_NONE,
};
```

- [ ] **Step 2: Add cancellation and mask helpers**

Implement helpers that delete only the current switch task, reset its fields,
start `camera_display_delay_mask_start()` in video-only mode, and remove
`CAMERA_DISPLAY_DELAY_MASK_OBJ_ID` without changing the backlight.

```c
static void camera_channel_switch_cancel(void)
{
	if (camera_channel_switch_state.task != NULL)
	{
		lv_task_del(camera_channel_switch_state.task);
		camera_channel_switch_state.task = NULL;
	}
	camera_channel_switch_state.target_ch = MON_CH_NONE;
	camera_channel_switch_state.elapsed_ms = 0;
	camera_channel_switch_state.retried = false;
}
```

- [ ] **Step 3: Add reason-specific completion**

For `CAMERA_CHANNEL_SWITCH_MANUAL`, restore `door_audio_talk()` only when the
handset is raised and the target is Door1/Door2. For CCTV or an on-hook handset,
close door talk audio.

For `CAMERA_CHANNEL_SWITCH_CALL`, start `camera_call_auto_record_task_create()`
and `camera_call_ring_delay_task_create()` after preview is enabled. Do not call
`camera_call_ring_play()` directly from the request entry point.

- [ ] **Step 4: Add the LVGL polling task**

The task must reject stale callbacks using pointer identity:

```c
if (task != camera_channel_switch_state.task)
{
	lv_task_del(task);
	return;
}
```

When `video_input_state_get()` becomes true, clear the state task pointer,
enable preview, remove the video mask, run completion, and delete the task.

At 700 ms, call `monitor_open(false, 0x03)` once more to reselect the same VIN
and restart TP9950 detection. At 1,400 ms, cancel the task and retain the black
video area so invalid input cannot expose an old frame.

- [ ] **Step 5: Add the shared request function**

`camera_channel_switch_request(target_ch, reason, tone_index)` must:

```c
camera_channel_switch_cancel();
camera_channel_transient_ui_reset();
camera_channel_switch_ui_prepare(target_ch);
camera_channel_transient_resource_close();
camera_display_delay_mask_start(CAMERA_CHANNEL_SWITCH_TIMEOUT_MS /
						CAMERA_CHANNEL_SWITCH_POLL_MS, false, true);
monitor_open(false, 0x03);
camera_channel_switch_state.task = lv_layout_task_create(
		camera_channel_switch_task,
		CAMERA_CHANNEL_SWITCH_POLL_MS,
		LV_TASK_PRIO_HIGH,
		NULL);
```

Store the target, reason, tone, elapsed time, and retry state before creating
the task. Force the target UI refresh before starting hardware selection.

- [ ] **Step 6: Run the structural check**

Run: `cd FF_1070 && sh tests/check_camera_async_switch.sh`

Expected: PASS with exit code 0.

### Task 3: Route both switch entry points through the controller

**Files:**
- Modify: `FF_1070/layout/layout_camera.c`
- Test: `FF_1070/tests/check_camera_async_switch.sh`

- [ ] **Step 1: Replace the manual blocking path**

Keep the same-channel early return and existing ringtone/audio cleanup in
`camera_channel_switch_internal()`, then replace `monitor_open()`, the blocking
VI loop, preview enable, and immediate talk restoration with:

```c
camera_channel_switch_request(target_ch, CAMERA_CHANNEL_SWITCH_MANUAL, 0);
```

- [ ] **Step 2: Replace the incoming-call blocking path**

Keep incoming-call preemption state cleanup in `camera_door_call_switch()`. If
the target differs, submit:

```c
camera_channel_switch_request(target_ch,
						 CAMERA_CHANNEL_SWITCH_CALL,
						 tone_index);
```

If the target is already active and VI is ready, retain the existing fast call
path without restarting TP9950. Start automatic recording and the delayed
ringtone through the same completion helper.

- [ ] **Step 3: Cancel pending switches on Camera exit**

Call `camera_channel_switch_cancel()` before `monitor_close()` in the Camera
quit path. Also cancel a pending request before answer, hangup, unlock, or a new
channel request can alter audio/recording state.

- [ ] **Step 4: Run source checks**

Run:

```bash
cd FF_1070
sh tests/check_camera_async_switch.sh
git diff --check -- layout/layout_camera.c tests/check_camera_async_switch.sh
```

Expected: both commands exit 0 and neither switch entry point contains a
blocking VI wait.

### Task 4: Build and board validation

**Files:**
- Verify: `FF_1070/build/FF.BIN`
- Verify: `FF_1070/upgrade/app/FF.BIN`

- [ ] **Step 1: Build the firmware and upgrade image**

Run: `cd FF_1070 && make`

Expected: `[100%] Built target FF.BIN`, packaging completes, and the command
exits 0.

- [ ] **Step 2: Verify generated application artifacts**

Run:

```bash
test -s FF_1070/build/FF.BIN
test -s FF_1070/upgrade/app/FF.BIN
```

Expected: exit code 0.

- [ ] **Step 3: Validate on the FF_1070 board**

Verify repeated Door1/Door2 manual switching, Door1/Door2 alternating calls,
new calls arriving during initialization, raised-handset Door/CCTV switching,
and call-triggered photo/video recording. The target UI must appear immediately,
only the video area may remain black, and no old frame or frozen Camera UI may
appear.

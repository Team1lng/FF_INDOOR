# Camera Hook Audio Stop Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Stop indoor and outdoor call ringtones immediately on answer, and close call audio/video immediately on handset hangup.

**Architecture:** Keep camera call-state ownership in `layout_camera.c`. Expose one answer entry point and one hangup entry point through `layout_common.h`, and make the global hook callback delegate to them so ringtone flags are updated before `ringplay_play_stop()` can invoke its finish callback.

**Tech Stack:** C, LVGL task/layout framework, existing shell structural regression test, CMake/Make.

---

### Task 1: Add Hook Transition Regression Checks

**Files:**
- Modify: `tests/check_camera_async_switch.sh`

- [ ] Extract the camera answer/hangup functions and the camera branch of `layout_hook_state_change_default()`.
- [ ] Assert answer marks the ringtone answered before stopping playback, disables outdoor ringtone routing, and starts door talk.
- [ ] Assert hangup cancels ringtone state, closes talk/audio routes and recording before navigating to standby.
- [ ] Run `sh tests/check_camera_async_switch.sh` and confirm it fails before production changes.

### Task 2: Centralize Camera Answer and Hangup

**Files:**
- Modify: `layout/layout_camera.c`
- Modify: `layout/layout_common.h`

- [ ] Convert the existing camera answer helper into the public `layout_camera_hook_answer()` entry point.
- [ ] Add `layout_camera_hook_hangup()` to cancel ringtone/record tasks, close audio and media, then enter standby.
- [ ] Remove the duplicate 500 ms handset transition handling from the camera ticker.

### Task 3: Delegate Global Hook Events

**Files:**
- Modify: `layout/layout_common.c`

- [ ] Replace duplicated camera answer logic with `layout_camera_hook_answer()`.
- [ ] Replace duplicated camera hangup logic with `layout_camera_hook_hangup()`.
- [ ] Preserve existing behavior for non-camera layouts.

### Task 4: Verify

**Files:**
- Test: `tests/check_camera_async_switch.sh`

- [ ] Run the structural regression test and confirm it passes.
- [ ] Run `git -c core.whitespace=cr-at-eol diff --check` for touched files.
- [ ] Build `FF.BIN` with `cmake --build build --target FF.BIN -j16`.
- [ ] Run full `make` and compare `build/FF.BIN` with `upgrade/app/FF.BIN`.

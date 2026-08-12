# Home Media Audio Transition Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Prevent abnormal key tones when entering Media from Home while keeping the media list and video playback amplifier enabled.

**Architecture:** Add one media audio preparation helper that enables the external amplifier before the key tone is queued. Invoke it from a Home-specific click-down callback and reuse it in media layout entry points.

**Tech Stack:** C, LVGL layout callbacks, ringplay, shell structural tests, CMake/Make.

---

### Task 1: Add Transition Ordering Test

**Files:**
- Create: `tests/check_media_audio_transition.sh`

- [ ] Assert `layout_media_audio_prepare()` enables GPIO9 without toggling the shared AO speaker path.
- [ ] Assert the Home Media click-down callback calls the helper before the generic key-tone callback.
- [ ] Assert Home registers and restores its click-down callback.
- [ ] Assert media list and video playback entries use the shared helper.
- [ ] Run the test and confirm it fails before implementation.

### Task 2: Implement Shared Audio Preparation

**Files:**
- Modify: `layout/layout_common.c`
- Modify: `layout/layout_common.h`

- [ ] Add `layout_media_audio_prepare()`.
- [ ] Enable the SDK speaker first, then GPIO9.

### Task 3: Stabilize Home to Media Transition

**Files:**
- Modify: `layout/layout_home.c`

- [ ] Store the Home Media button object.
- [ ] Register a Home-specific click-down callback.
- [ ] Prepare media audio before queuing the Media key tone.
- [ ] Restore the default callback on Home exit.

### Task 4: Reuse Preparation in Media Layouts

**Files:**
- Modify: `layout/layout_photo_list.c`
- Modify: `layout/layout_memory_video.c`

- [ ] Replace direct amplifier enable calls with the shared helper.

### Task 5: Verify

**Files:**
- Test: `tests/check_media_audio_transition.sh`

- [ ] Run regression tests and whitespace checks.
- [ ] Build `FF.BIN` and run full packaging.

# Door Call Zero Volume Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep the indoor unit silent during a door Call when `inter_ring_volume` is zero without muting the outdoor unit or changing talk volume.

**Architecture:** Reuse the existing `ring_play()` wrapper, which applies the indoor amplifier state before queuing audio while retaining the door-call start callback that routes ringtone audio to the outdoor unit. Apply the wrapper to both initial playback and replay.

**Tech Stack:** C, existing ringplay queue, shell structural regression test, CMake/Make.

---

### Task 1: Add Regression Checks

**Files:**
- Modify: `tests/check_camera_async_switch.sh`

- [ ] Extract `camera_call_ring_play()` and assert it uses `ring_play()` rather than direct queue submission.
- [ ] Assert the ringtone replay branch also uses `ring_play()`.
- [ ] Run the test and confirm the current implementation fails.

### Task 2: Route Door Call Playback Through Indoor Volume Guard

**Files:**
- Modify: `layout/layout_camera.c`

- [ ] Replace the initial direct `ringplay_play_form_index()` call with `ring_play()`.
- [ ] Replace the replay direct call with `ring_play()`.
- [ ] Keep `ringplay_doorcall_start_default_func` unchanged so outdoor ringtone routing remains active.

### Task 3: Verify

**Files:**
- Test: `tests/check_camera_async_switch.sh`

- [ ] Run the structural test and whitespace check.
- [ ] Build `FF.BIN`.
- [ ] Run full packaging and compare copied application binaries.

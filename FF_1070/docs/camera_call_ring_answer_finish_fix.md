# camera call 铃声接听后 finish 回调修复说明

## 针对问题

门口机 call 机响铃过程中，拿起话筒会调用 `ringplay_play_stop()` 停止当前铃声，然后马上进入 `door_audio_talk()` 打开门口机通话音频通路。

但 `ringplay` 播放线程在停止铃声后仍会继续执行当前铃声的 `finish_func`。原来的 camera call 铃声 finish 回调最终会进入 `camera_call_ring_finish_cleanup()`，执行以下清理动作：

- `power_amplifier_enable(false)`
- `call_ring_to_outdoor_ctrl(..., false)`

这会造成一个竞态风险：刚接听并打开通话音频通路，随后铃声 finish 回调又把功放和门口机音频通路关闭，导致接听后通话声音异常。

## 修改内容

修改文件：

- `FF_1070/layout/layout_camera.c`

新增状态：

- `camera_call_ring_answered`

具体修改：

- call 铃声开始播放时，将 `camera_call_ring_answered` 重置为 `false`。
- 拿起话筒接听时，将 `camera_call_ring_answered` 置为 `true`，然后停止铃声并进入 `door_audio_talk()`。
- camera call 铃声 finish 回调 `layout_camera_callring_finish_default_func()` 中，如果检测到 `camera_call_ring_answered == true`，直接返回，不再执行 `camera_call_ring_finish_cleanup()`。
- 正常未接听的铃声结束流程保持不变，仍会按原逻辑关闭功放、关闭门口机音频通路，并触发自动拍照/录像逻辑。
- 退出 camera 界面时重置 `camera_call_ring_answered`，避免状态残留到下一次 call。

## 不影响的功能

触摸按键音不使用 `layout_camera_callring_finish_default_func()` 这个 finish 回调。

触摸按键音链路仍然是：

```text
layout_obj_click_down_func()
-> touch_sound_play()
-> ringplay_keysound_start_default_func()
-> ring_volume_set(TOUCH_TONE_VOL)
```

因此本次修改不会改变触摸按键音打开功放的逻辑。

## 影响范围

主要影响 camera 界面的门口机 call 响铃接听流程：

- call 机响铃时拿起话筒。
- `ringplay_play_stop()` 停止铃声后，避免铃声 finish 回调误关闭已进入通话的音频通路。

## 验证结果

已执行：

```sh
cd FF_1070 && make
```

验证结果：

- 编译通过，`FF_1070/build/FF.BIN` 生成成功。
- 打包流程通过，`FF_1070/upgrade/app/` 和升级相关镜像重新生成。

## 建议提交信息

```text
fix: keep door audio route after answering camera call
```

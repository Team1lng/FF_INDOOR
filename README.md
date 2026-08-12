# FF_1070

# FF 项目编译、文件拷贝及升级镜像制作全流程指南

本文档用于指导完成 FF 项目的编译、文件拷贝及升级镜像制作全流程，通过自动化脚本完成镜像打包、文件同步及外部 SDK 编译，最终生成设备可使用的升级镜像文件。

## 前置条件

执行编译操作前，请务必确认满足以下所有条件。

### 1. 环境依赖

已安装以下基础编译/打包工具：

- `cmake`
- `make`
- `squashfs-tools`，需确保 `mksquashfs` 工具可用
- `tar`

### 2. 路径检查

当前工作区主要路径如下：

1. APP 工程路径：`/home/leo/workspace/FF_Indoor/FF_1070`
2. 外部 SDK 路径：`/home/leo/workspace/FF_Indoor/AK37E_SDK_V1.05`
3. APP 打包输出目录：`/home/leo/workspace/FF_Indoor/FF_1070/upgrade`
4. SDK 升级包输出目录：`/home/leo/workspace/FF_Indoor/AK37E_SDK_V1.05/upgrade`

路径不一致时，需要检查 `FF_1070/Makefile` 和 `upgrade/make_image.sh` 中的硬编码路径。

### 3. 文件依赖

`FF_1070` 目录下需存在以下目录及文件：

- `build`：CMake 编译输出目录
- `res/rom.bin`：UI 资源包
- `res/rom.h`：UI 资源宏定义
- `res/sat_leo.ttf`
- `res/persian.ttf`
- `res/rings`

## 编译步骤

### 1. 基础编译

执行以下命令完成 APP 编译、资源拷贝和升级镜像打包：

```sh
cd /home/leo/workspace/FF_Indoor/FF_1070
make
```

该命令会生成：

- `FF_1070/build/FF.BIN`
- `FF_1070/upgrade/app/`
- `FF_1070/upgrade/platform/app.sqsh4`
- `AK37E_SDK_V1.05/upgrade/SAT_FFOS`

### 2. 清理编译产物

```sh
cd /home/leo/workspace/FF_Indoor/FF_1070
make clean
```

## 补充说明

- 常用升级包在 `AK37E_SDK_V1.05/upgrade` 目录下生成。
- `rom.bin` 必须通过正常资源打包流程同步到 `FF_1070/upgrade/app/` 和 SDK rootfs，否则板端可能显示旧图或错图。
- APP 版本日期来自 `layout_logo.c` 的 `__DATE__`，需要确保 `layout_logo.c.o` 每次 make 时重新编译。

## 修改记录

### 20260628

1. 门口机响铃输出通路修复
   问题：`call_ring_to_outdoor_ctrl(AUDIO_CH ch, bool en)` 关闭分支之前直接返回，后续又只保留关闭逻辑，导致 `en == true` 时可能没有打开 DOOR1/DOOR2 音频输出通路。
   涉及文件：`FF_1070/layout/user_gpio.c`
   修改内容：恢复 `call_ring_to_outdoor_ctrl()` 的开启分支，`AUDIO_CH_DOOR1` 打开 `audio_to_outdoor1_pin_ctrl(true)`，`AUDIO_CH_DOOR2` 打开 `audio_to_outdoor2_pin_ctrl(true)`；关闭分支按通道关闭对应 GPIO；异常通道兜底关闭两个门口机音频输出；函数末尾补齐 `return true`，避免非 void 函数无返回值。

2. 门口机音频 GPIO 默认低电平
   问题：APP 启动初始化后 GPIO56、GPIO36 默认高电平，可能让门口机音频输出通路在空闲状态下默认打开。
   涉及文件：`FF_1070/layout/user_gpio.c`
   修改内容：在 `layout_gpio_init()` 中将 GPIO56 `AUDIO_DOOR1_PIN` 和 GPIO36 `AUDIO_DOOR2_PIN` 初始化后的默认输出改为低电平；只改启动默认状态，不改变 `door_audio_talk()`、`call_ring_to_outdoor_ctrl()`、`monitor_record_pin_enable()` 的运行态打开逻辑。

3. camera 听筒状态跟踪修复
   问题：`camera_ticker_task()` 内部使用 `static bool last_hook_state`，页面退出后不会重置，多次进入 camera 界面后可能导致听筒拿起检测不准。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：将函数内部静态变量改为文件级 `camera_last_hook_state`；进入 camera 页面时读取 `hook_state_get()` 并同步到 `camera_in_talk_state` 和 `camera_last_hook_state`；退出页面时重置 `camera_last_hook_state`，避免旧状态影响下一次进入。

4. camera call 接听后铃声 finish 回调修复
   问题：门口机 call 机响铃时拿起话筒会停止铃声并进入 `door_audio_talk()`，但铃声线程后续仍可能执行 finish 回调，把功放和门口机音频通路关闭，导致接听后通话声音异常。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：新增 `camera_call_ring_answered` 状态；call 铃声开始时清零；拿起话筒接听时置位；`layout_camera_callring_finish_default_func()` 检测到已接听后直接返回，不再执行 `camera_call_ring_finish_cleanup()`；退出 camera 页面时重置该状态，避免影响下一次 call。

5. 升级包版本字段拼写修复
   问题：升级打包脚本写入字段 `uprade_image_version`，少了字母 `g`，后续按 `upgrade_image_version` 读取时会读不到升级包版本。
   涉及文件：`AK37E_SDK_V1.05/upgrade/make_image.sh`
   修改内容：将 env 临时配置里写入的字段名改为 `upgrade_image_version`；只修改脚本字段名，不改变 APP 版本显示逻辑和升级分区结构。

6. APP 版本日期不更新修复
   问题：设置页版本日期来自 `layout_logo.c` 的 `__DATE__`，如果 CMake 复用旧的 `layout_logo.c.o`，重新打包 app 后日期仍是旧日期。
   涉及文件：`FF_1070/Makefile`
   修改内容：在 `make` 流程开始时删除当前工程路径下的 `build/CMakeFiles/FF.BIN.dir$(CURDIR)/layout/layout_logo.c.o`，强制 `layout_logo.c` 每次重新编译，让 `__DATE__` 使用本次编译日期。

### 20260629

1. CCTV/门口机来电切换 UI 错位修复
   问题：手动进入 CCTV 监控界面后，门口机 call 机会切到 door 通道，但界面上可能残留 CCTV 图标；再次点击屏幕后 CCTV 和 door 图标同时显示。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：新增 camera 通道 UI 刷新逻辑；来电切通道后统一刷新当前通道名、按钮坐标和通道选择弹窗状态；`switch / record / capture` 按钮从 CCTV 切回 door 时恢复 door 坐标；刷新后对相关对象执行 invalidate，减少旧图标残留。

2. 门口机来电切换清屏和自动拍照/录像提前
   问题：door1 监控中被 door2 呼叫切换时偶发花屏或旧 UI 残留；自动拍照/录像要等铃声播放结束后才开始；过短的呼叫录像也可能进入媒体列表。
   涉及文件：`FF_1070/layout/layout_camera.c`、`FF_1070/common/video_record.c`
   修改内容：页内通道切换前增加 GUI 层清理和监控区域刷新；新增 `camera_call_auto_record_start()` 及短间隔重试任务，来电后立即尝试自动拍照/录像；铃声结束回调只保留音频清理，不再负责启动记录；新来电打断旧来电时先关闭旧抓拍/录像任务再按新通道启动；录像保存门槛改为大于等于 3000ms 且有有效帧。

3. 呼叫录像时退出监控卡住修复
   问题：门口机呼叫并自动录像时，铃声播放期间点击返回 Home 或挂断返回待机，界面会卡到铃声播放结束后才退出。
   涉及文件：`FF_1070/common/ringplay.c`、`FF_1070/include/common/ringplay.h`、`FF_1070/layout/layout_camera.c`
   修改内容：新增 `ringplay_play_stop_async()`，只设置停止标志，不等待音频 buffer 清空；camera 的返回 Home、挂断返回待机、页面退出流程改用异步停止；新增 `camera_call_ring_cancel()`，退出时统一取消来电铃声状态，避免退出后 finish 回调继续按来电流程清理。

4. 两台门口机呼叫互相打断切换
   问题：门口机1呼叫播放铃声时，门口机2再呼叫需要等待门口机1铃声播放完成，门口机2通道和铃声才会出来。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：调整 camera 来电切换流程，新门口机来电时先取消旧门口机铃声和旧 call 状态，再立即切换当前监控通道、刷新通道 UI、播放新通道铃声，并按新通道处理后续自动拍照/录像。

5. 视频暂停恢复音频异常修复
   问题：视频第一次播放声音正常，点击暂停再播放后声音异常；之前点击屏幕触发暂停/播放时也会导致音频变怪。
   涉及文件：`FF_1070/common/video_play.c`、`FF_1070/include/common/video_play.h`、`FF_1070/layout/layout_memory_video.c`
   修改内容：暂停状态下不再继续读取视频帧；暂停时记录当前视频帧位置、清理 AO 缓冲并关闭 AVI 句柄；恢复播放时重新打开 AVI 文件，按视频帧同步音频读位置并重建播放时钟；带音频视频打开时重启 AO；视频界面点击屏幕只控制按钮显示/隐藏，不再直接触发暂停/继续，暂停/播放统一由播放按钮处理。

6. 视频播放按钮 `play.png/start.png` 切换
   问题：视频播放界面原来只有 `play.png`，靠按下变暗区分状态不直观；加入 `start.png` 后又出现不切换、暂停不回切、点击视频文件卡住等问题。
   涉及文件：`FF_1070/layout/layout_memory_video.c`、`FF_1070/res/ui/memory/start.png`、`FF_1070/res/rom.bin`、`FF_1070/res/rom.h`
   修改内容：新增 `video_play_btn_state_display()`，播放态显示 `ROM_UI_MEMORY_START_PNG`，暂停/停止/结束态显示 `ROM_UI_MEMORY_PLAY_PNG`；运行时只更新 `LV_STATE_DEFAULT`，避免 pressed 状态覆盖默认图标；创建按钮时 pressed 图固定为 `play.png` 并关闭 recolor；增加 `memory_video_btn_click_set()` 空指针保护，避免对象未创建时调用 `lv_obj_set_click(NULL, ...)`；资源包同步加入 `start.png` 对应宏和数据。

7. 视频播放结束后图标和画面异常修复
   问题：视频自然播放结束后图标能切回 `play.png`，但视频界面会弹出奇怪显示或旧画面叠加。
   涉及文件：`FF_1070/common/video_play.c`、`FF_1070/include/common/video_play.h`、`FF_1070/layout/layout_memory_video.c`
   修改内容：新增 `video_play_eof_check()` 读取底层 EOF 标志；页面层不再用 `cur >= total` 提前判定结束，避免最后一帧附近过早关闭视频层；结束时先把进度显示为 100%，再执行 `video_play_stop()`、`layout_memory_video_load()` 重载当前视频缩略图，并把按钮切回 `play.png`；开始播放和停止播放时清理 EOF 标志，避免下一次播放误判已结束。

8. Flash/SD 图片界面播放按钮同步
   问题：视频界面改为 `play.png/start.png` 后，Flash 图片界面和 SD 图片界面的播放按钮行为不一致，点击播放按钮可能没有反应。
   涉及文件：`FF_1070/layout/layout_memory_photo.c`
   修改内容：新增 `photo_play_btn_state_display()`，图片轮播播放时显示 `start.png`，停止时显示 `play.png`；新增 `memory_photo_btn_click_set()` 做按钮空指针保护；播放和停止轮播时保持按钮可见并同步切换图标；删除弹窗相关按钮恢复点击时也改为安全 helper，避免空对象崩溃。

9. 视频播放结束后再次播放音频异常处理
   问题：视频播放完毕后再次点击播放，音频可能再次异常，怀疑 AO 或 AVI 状态没有清理干净。
   涉及文件：`FF_1070/common/video_play.c`
   修改内容：视频 EOF 后重启 AO、关闭 AVI 句柄、清空 jpg 解码缓冲；恢复或重新播放时重新打开 AVI 文件，重新计算视频帧和音频帧对应关系，并重新设置音频读位置，避免复用播放完成后的旧音频状态。

10. 资源打包同步说明
    问题：新增 `start.png` 或替换 UI 资源后，如果 `rom.bin/rom.h` 没同步或资源包没打进 app，板端会显示旧图、错图或按钮不变化。
    涉及文件：`FF_1070/res/ui/memory/start.png`、`FF_1070/res/rom.bin`、`FF_1070/res/rom.h`、`FF_1070/upgrade/app/rom.bin`、`AK37E_SDK_V1.05/rootfs/resource/app/app/rom.bin`
    修改内容：通过正常 `make` 打包流程同步资源，不手工只补 `rom.h` 宏；确保 `res/rom.bin`、`upgrade/app/rom.bin`、SDK rootfs 里的 app 资源包保持一致，避免编译通过但板端显示错资源。

### 20260630

1. 设置页音量改为室内机铃声音量
   问题：`setting_sound_ring_volume_btn_create()` 原来使用 `door_ring_volume`，会把设置页音量同时影响到门口机相关音量；需求是设置页只控制室内机铃声/预览音量，不直接控制门口机通话音量。
   涉及文件：`FF_1070/layout/layout_setting_sound.c`、`FF_1070/layout/layout_common.c`
   修改内容：设置页音量读取和保存改为 `user_data_get()->setting.inter_ring_volume`；相关 helper 从 `door_ring_volume_get/set()` 调整为 `inter_ring_volume_get/set()`；门口机1/2铃声预览统一按 `inter_ring_volume > 0` 判断，音量为 0 时设置页不播放预览；门口机来电铃声启动时使用 `inter_ring_volume` 控制室内机功放音量。

2. 音量 0 时室内机静音但门口机仍有声音
   问题：室内机铃声音量设置为 0 时，室内机听筒/喇叭仍能听到铃声；同时门口机侧仍需要能听到呼叫铃声，不能因为室内机音量为 0 就跳过铃声播放链路。
   涉及文件：`FF_1070/layout/user_gpio.c`、`FF_1070/layout/layout_camera.c`、`FF_1070/layout/layout_common.c`
   修改内容：修复 `power_amplifier_enable(bool en)`，由原先无论 `true/false` 都拉高 GPIO9，改为按参数控制高低电平，使 `ring_volume_set(0)` 能真正关闭室内机功放；来电 call 铃声不再因为 `inter_ring_volume == 0` 而跳过播放，仍执行 `camera_call_ring_play()` 和 `call_ring_to_outdoor_ctrl()`，保证门口机侧呼叫声链路正常。

3. 开锁提示音打断门口机呼叫铃声
   问题：室内机待机进入门口机 call 界面后，门口机正在播放呼叫铃声时点击开锁，开锁声应该打断呼叫铃声，但原逻辑会让呼叫铃声继续播放或后续 finish 回调再次影响音频通道。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：在 `camera_open_btn_up()` 中点击开锁前检测当前是否有 call 铃声播放；若正在播放，则增加 `camera_call_ring_ignore_finish_count` 并调用 `camera_call_ring_cancel()` 取消旧铃声状态；随后重新打开当前门口机音频输出通道、执行 `monitor_unlock_open()` 并播放开锁提示音，使开锁声优先于呼叫铃声。

4. 摘机通话期间提示音结束后恢复通话音频
   问题：拿起话筒后本应正常通话，但如果期间触发 call 铃声或开锁提示音，提示音播放结束回调会执行 `power_amplifier_enable(false)` 和 `call_ring_to_outdoor_ctrl(..., false)`，把门口机与室内机通话音频通道关闭，导致双方无音频。
   涉及文件：`FF_1070/layout/layout_common.c`、`FF_1070/layout/layout_camera.c`
   修改内容：在 `ringplay_doorcall_finish_default_func()` 和 `camera_unlock_ring_finish_func()` 中增加摘机通话判断；当 `hook_state_get() == true` 且当前通道为 `MON_CH_DOOR1/MON_CH_DOOR2` 时，不关闭功放和门口机通道，而是调用 `door_audio_talk(AUDIO_CH_DOOR1/2)` 恢复对应门口机通话音频链路；非通话状态保持原有关闭逻辑。

5. 视频播放暂停后删除卡住修复
   问题：进入视频播放界面后，直接删除视频正常；但播放后点击暂停，再点击删除，界面可能卡住，按钮无法继续操作，只能等 `memory_video_timeout_val` 从 60 倒计时到 0 后返回待机界面恢复。
   涉及文件：`FF_1070/layout/layout_memory_video.c`
   修改内容：在 `video_delete_btn_up()` 入口先调用 `video_play_stop()` 退出暂停态播放器，避免 AVI 句柄和视频显示模式继续占用；新增 `memory_video_timeout_task_stop()`，点击删除时停止 60 秒待机倒计时；新增 `memory_video_delete_dialog_active` 标志，删除确认弹窗期间让 `layout_play_state_task()` 直接返回，不再重建倒计时或修改按钮状态；确认删除和取消删除时清除该标志。

6. 视频暂停后删除弹窗画面残留修复
   问题：播放视频后暂停，再点击删除，删除确认弹窗能正常出现，但弹窗下方会残留暂停视频层或异常画面。
   涉及文件：`FF_1070/layout/layout_memory_video.c`
   修改内容：点击删除按钮时，在 `video_play_stop()` 后调用 `video_input_resident_bzero()` 清空视频 resident buffer，并调用 `layout_memory_video_load()` 重新加载当前视频缩略图，然后再创建半透明遮罩和删除确认弹窗，避免弹窗底部保留暂停时的视频层画面。

7. 监控调节窗口切门口机通道残留修复
   问题：进入监控界面后点击 `camera_color_btn_create` 打开亮度/色度/对比度调节窗口，此时门口机 call 切换通道，调节窗口 UI 会残留并跟随新通道显示。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：在 `camera_door_call_switch()` 入口检测 `setting_win_diaplay_flag`，若调节窗口正在显示，则先调用 `camera_setting_window_display_enable(false)` 关闭调节窗口，再执行通道切换、GUI 层清理、视频预览关闭和通道 UI 刷新，避免设置窗口残留到新通道。

8. 监控调节滑块点击范围优化
   问题：亮度、色度、对比度三个滑动进度条范围为 0-10，拖动能到 10，但点击时通常只能点到 0-9，最右侧最大值 10 不容易点到。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：新增 `camera_slider_value_get()`，在 slider 点击回调中读取当前触摸释放坐标和 slider 坐标；当点击位置靠近右端 8px 范围时强制设置为 10，靠近左端 8px 范围时强制设置为 0；亮度、色度、对比度三个回调统一使用该 helper 获取值，保留原拖动逻辑。

9. 门口机 call 切换清视频层和 UI 层
   问题：门口机 call 触发监控通道切换时，旧视频帧或 GUI 层偶发残留，表现为一瞬间花屏或旧通道 UI 残影。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：在 `camera_door_call_switch()` 中当前通道切到目标通道前，先关闭视频预览 `video_display_preview_enable(false)`，清空 resident buffer，填充清理 GUI 层，刷新监控区域；切换 `monitor_channel_set()` 并重新 `monitor_open()` 后再次清空 resident buffer，再刷新通道名和通道 UI。

### 20260701

1. 门口机重复 call 后通话音频恢复
   问题：已进入门口机通话状态后，如果门口机再次 call 机播放铃声，铃声播放结束回调仍可能按呼叫铃声流程关闭功放或门口机音频通道，导致原本正在通话的门口机和室内机没有声音。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：在 `layout_camera_callring_finish_default_func()` 中增加摘机通话判断；当 `hook_state_get() == true` 且当前通道为 `MON_CH_DOOR1/MON_CH_DOOR2` 时，清除 call 铃声状态，设置 `MON_ENTER_TALK`，并重新调用 `door_audio_talk(AUDIO_CH_DOOR1/2)` 恢复对应门口机通话链路；只有非通话状态才继续执行铃声重播或结束清理逻辑。

2. 监控调节滑块点击值优化
   问题：亮度、色度、对比度滑块范围为 0-10，拖动正常，但点击时会出现比例计算抖动，例如 `ratio=0.72` 直接到 7，`ratio=0.78` 先闪 7 再到 8；另外 9 到 10 仍不稳定。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：将滑块取值统一改为根据触摸点和滑块坐标计算 0-1 比例，再通过四舍五入映射到 0-10，并限制最终范围；亮度、色度、对比度拆分为 `*_update()` 和 `*_event()`，在 `LV_EVENT_VALUE_CHANGED`、`LV_EVENT_CLICKED`、`LV_EVENT_RELEASED` 中统一刷新数值、标签和实际显示参数，点击释放后再清理 pressed 状态和重启隐藏倒计时。

3. 视频停止时同步关闭 AVI 句柄
   问题：视频播放、暂停、删除、播放结束等流程中，如果 `video_play_stop()` 只切状态但没有立即关闭 AVI 句柄，后台播放线程和页面层可能同时处理旧播放资源，导致再次播放、删除弹窗或界面刷新时出现异常。
   涉及文件：`FF_1070/common/video_play.c`
   修改内容：在 `video_play_stop()` 中将状态切到 `VIDEO_PLAY_STATE_IDLE` 后，如果 `avi_handle_id` 仍存在，立即调用 `video_play_device_close()` 同步关闭 AVI 句柄；同时保留恢复旧 JPEG 解码回调、恢复屏幕背景、关闭视频模式和关闭视频预览的原有流程，避免暂停/删除后底层播放资源残留。

4. 视频播放暂停后删除弹窗概率死机修复
   问题：进入视频播放界面后，点击播放再暂停，然后点击删除按钮，删除弹窗可能出现异常残影；如果先打开删除弹窗再被门口机 call 机打断，返回后再次播放、暂停、删除，仍有概率死机。
   涉及文件：`FF_1070/layout/layout_memory_video.c`
   修改内容：新增 `memory_video_play_state_task` 记录播放状态刷新任务，并提供 start/stop helper；点击删除时先停止播放状态任务、超时任务和上一首/下一首等待任务，再停止视频播放、清空 resident buffer、重载当前视频缩略图、恢复按钮显示状态，最后创建遮罩和删除确认弹窗；删除弹窗对象新增 `memory_video_delete_box` 句柄，取消时直接按句柄删除，不再通过 `obj->parent->parent` 查找父容器；弹窗创建后调用 `lv_obj_move_foreground()` 保证遮罩和弹窗位于最上层；退出视频界面时清理删除状态、弹窗句柄和任务指针，避免 call 机打断后旧任务继续刷新已销毁对象。

5. 移动侦测中 call 机切监控死机修复
   问题：移动侦测界面正在使用 VI/VENC/AI、录像或抓拍资源时，门口机 call 机触发切到监控界面，旧界面退出流程先关闭监控再停采集和录制，可能造成硬件资源关闭顺序冲突并卡死。
   涉及文件：`FF_1070/layout/layout_motion_detection.c`
   修改内容：调整 `LAYOUT_QUIT_FUNC(motion_detection)` 的资源释放顺序；先记录是否需要 `monitor_close()`，随后停止铃声、移动侦测任务、JPEG 采集和音频采集，再关闭录像和 JPEG 录制，最后按需关闭 monitor；之后再恢复 SD 卡回调、销毁移动侦测对象、清理点击事件、保存用户数据和重启待机计时，避免 call 机切界面时采集/编码/监控关闭互相抢占。

### 20260706

1. 媒体视频界面操作后重置待机倒计时
   问题：视频文件播放界面原本 60 秒无操作返回待机，但进入界面后如果倒计时已经衰减到一半，再点击播放、暂停、上一首、下一首、删除或背景区域，只是暂停/继续旧倒计时，没有按用户操作重新从 60 秒开始。
   涉及文件：`FF_1070/layout/layout_memory_video.c`
   修改内容：新增 `memory_video_timeout_task_start()` 和 `memory_video_user_activity_reset()`，统一把倒计时复位到 60 秒；播放、暂停、上一首、下一首、返回列表、删除确认/取消、背景点击、播放结束等入口都调用该复位逻辑；播放状态下停止待机倒计时，暂停或停止后重新启动倒计时；删除旧的 `memory_video_timeout_kuaijin`、上一首/下一首等待任务和对应计数变量，避免多套倒计时任务互相抢占。

2. 媒体图片界面操作后重置待机倒计时
   问题：Flash 图片界面和 SD 卡图片界面同样存在无操作返回待机逻辑，用户点击上一张、下一张、播放轮播、删除弹窗等操作后，需要重新按完整时间计时，而不是沿用旧倒计时。
   涉及文件：`FF_1070/layout/layout_memory_photo.c`
   修改内容：新增 `memory_photo_timeout_task` 记录当前待机任务，新增 `memory_photo_timeout_task_start()` 和 `memory_photo_user_activity_reset()`；进入图片界面、点击 home、上一张、下一张、播放/停止轮播、背景、删除确认和取消时统一复位倒计时；轮播播放期间删除待机任务，停止轮播后重新启动待机任务；退出页面时删除待机任务和残留弹窗，避免旧任务刷新已销毁对象。

3. 房号 0 支持作为管理员
   问题：设置本机房号时，原逻辑要求房号必须大于等于 1，导致房号 `0` 无法保存；实际需求是房号 0 表示管理员/管理机。
   涉及文件：`FF_1070/layout/layout_intercom.c`
   修改内容：房号设置校验从 `kb_click_num < 1 || kb_click_num > 99` 改为只拦截空输入和大于 99 的值，允许输入并保存 `00`；保存成功后同步调用 `MsgUpdateNativeId()` 更新底层内线本机 ID；界面显示时如果房号为 `00`，显示管理员文字，否则按两位房号格式显示，避免管理员被显示成普通房号。

4. 内线通话接听状态修正
   问题：新板和客样内线通话时，存在一端拿起听筒后另一端自动挂断、主叫未真正等到对端确认就进入通话、挂断后对端界面不退出等异常，根因之一是接听、通话请求和通话响应状态处理过早切换音频或误处理重复帧。
   涉及文件：`FF_1070/layout/intercom.c`、`FF_1070/layout/layout_intercom_out.c`
   修改内容：`MsgCallAccept()` 中先进入请求通话状态并启动超时重发，不再立即执行 `Intercom.Accept()` 和切换音频通道；`RP_ReceiveResponseTalking()` 收到对端通话响应后才停止重发、执行接听、切换对应音频通道并进入 `RP_TALKING`；被叫侧收到重复或串线的 `RQ_TALKING` 时改为忽略，避免误调用挂断并清空状态；主叫呼出界面只根据 `intercom_remote_ack_get_and_clear()` 跳转通话界面，不再仅凭本地 `INTERCOM_STATE_TALKING` 提前进入通话。

5. 内线忙线提示文本修正
   问题：内线呼出等待过程中收到忙线标志时，界面使用了普通 busy 文本 ID，可能和当前语言表里的总线忙线提示不一致。
   涉及文件：`FF_1070/layout/layout_intercom_out.c`
   修改内容：忙线提示从 `LAYOUT_INTERCOM_LANG_BUSY_ID` 改为 `LAYOUT_INTERCOM_LANG_BUS_BUSY_ID`，让内线总线忙时显示对应语言文案。

6. camera 退出时隐藏通道标签
   问题：从 camera/监控界面切换到内线或其它界面时，CCTV/door 通道文字有概率残留到新界面，表现为旧通道名跟随页面显示。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：在 `LAYOUT_QUIT_FUNC(camera)` 中查找 `CAMERA_HEAD_CH_LABEL_ID` 并隐藏该对象，先把通道标签从 GUI 层移除，再继续执行原有页面退出流程，减少跨页面 UI 残留。

7. UART 发送调试打印清理
   问题：内线串口发送函数之前每发送一帧都会打印 fd、长度、首字节和 errno，频繁通信时日志量过大，容易干扰实际问题分析。
   涉及文件：`FF_1070/common/uart_ctrl.c`
   修改内容：移除 `uart_write()` 中的逐帧调试打印和额外 `errno.h` 引用，函数恢复为直接 `write()` 并返回实际发送长度，保留原有串口发送行为不变。

### 20260707

1. 待机黑屏移动侦测内线来电闪帧处理
   问题：开启移动侦测后，室内机进入待机黑屏，另一台室内机拨打内线时，进入内线来电界面前会先闪一帧移动侦测画面；原因是待机黑屏后移动侦测任务已经启动，内线来电切页时旧视频/GUI 层和背光打开时序存在竞争。
   涉及文件：`FF_1070/layout/layout_standby.c`、`FF_1070/layout/layout_motion_detection.c`、`FF_1070/layout/layout_common.c`、`FF_1070/layout/layout_intercom_in.c`
   修改内容：在 standby 移动侦测任务中增加内线忙碌判断，检测到内线状态或来电标志时不再进入移动侦测页面，并停止/销毁已启动的移动侦测资源；退出 standby 时根据是否真正进入移动侦测决定是否释放资源；移动侦测页面进入和退出时增加内线抢占处理，检测到内线来电则跳过移动侦测界面并转入 `intercom_in`；内线来电默认入口增加黑屏过渡，当前处于 standby 或 motion_detection 时先关闭背光、关闭视频预览、清 GUI 层并刷新整屏，再进入内线来电界面；`intercom_in` 去掉进入页面时立即开背光，改为 UI 创建完成并通知底层后延时打开背光，退出页面时清理未执行的背光任务。

2. SD/媒体文件列表快速翻页闪屏处理
   问题：进入 SD 卡图片/视频文件列表后，快速点击上一页、下一页切换页码时，列表区域偶尔闪一下；之前尝试隐藏整个列表容器再重建内容，虽然能减少闪屏，但会导致 SD 文件列表保持隐藏，看不到文件。
   涉及文件：`FF_1070/layout/layout_photo_list.c`
   修改内容：统一通过 `media_list_page_rebuild()` 重建照片/视频列表项；翻页时不再先 `lv_obj_set_hidden(page, true)` 隐藏整个列表容器，而是只清理并重建子项，减少整块空白刷新窗口；重建结束后强制 `lv_obj_set_hidden(page, false)`，保证从 Flash/SD/Video 切换回来时当前列表容器可见；Flash 图片、SD 图片、视频列表按钮切换和分页回调统一走该重建函数，减少重复清理和重绘逻辑。

3. 监控界面进入时左上角 `Text` 闪烁修复
   问题：刚进入监控通道时，屏幕左上角会短暂闪一下默认 `Text`，前几个版本没有该现象；原因是监控倒计时 label 创建后没有立即设置文本和位置，LVGL 默认文本可能在 UI 初始化或背光打开前短暂显示。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：`camera_head_monitor_count_label_create()` 创建倒计时 label 后立即设置为当前倒计时文本并放到右上角，避免默认 `Text` 出现在左上角；进入 camera 页面开始时先关闭背光，后续通过原有延时显示流程再打开背光，避免 UI 初始化中间状态直接暴露到屏幕。

4. 门口机交换 call 切通道过渡调整
   问题：door1 监控/呼叫过程中 door2 再 call 时，原先切换逻辑会停留在 door1 当前画面，等待新视频流准备好后才跳到 door2，体验上像卡住；中间尝试用 LVGL 黑色遮罩模拟视频黑底，但会在屏幕中间出现大黑块，遮挡 UI。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：门口机交换 call 时不再创建 LVGL 黑色遮罩对象，避免 UI 层出现大黑块；切换流程改为先关闭旧视频预览、退出视频显示模式并清空 resident 视频 buffer，然后立即把 `monitor_channel` 切到目标门口机，刷新通道名、底部按钮和监控 UI，并强制刷新屏幕，让界面先显示目标门口机 UI 和黑色视频区域；随后再重新打开目标通道并等待 VI 就绪，最后开启视频预览显示新通道画面。

5. 监控初始显示遮罩逻辑拆分
   问题：进入监控页面和门口机交换 call 都需要处理视频流未就绪的过渡，但两者效果不同；进入监控可以整页黑屏等待，交换 call 不能遮住 UI。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：保留进入 camera 页面时的整屏延时遮罩和背光控制，用于防止刚进入监控时旧帧或脏帧外露；门口机交换 call 的 `camera_channel_switch_delay_start()` 改为不创建遮罩，只依赖关闭视频预览和清空视频 buffer 实现黑色视频区域过渡，避免把整页遮罩逻辑误用到页内通道切换。

### 20260708

1. 监控倒计时首帧显示 89S 修复
   问题：监控界面倒计时应从 90S 开始，但进入监控时偶尔会先闪一下 89S，再显示 90S；原因是倒计时复位在监控 UI 创建之后执行，倒计时 label 或刷新任务可能先拿到上一次已经自减后的旧值。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：在 `LAYOUT_ENTER_FUNC(camera)` 中将 `camera_timeout_value_reset()` 提前到 `camera_goto_monitor_mode(parent)` 之前执行，让监控倒计时 label 创建时直接使用复位后的 90S；删除 UI 创建后重复复位的调用，避免任务和 label 初始化顺序不一致导致首帧显示旧值。

2. 待机唤醒进入 Home 的 UI 残留处理
   问题：室内机待机黑屏久了以后点击屏幕进入 Home，亮屏瞬间可能看到上一界面 UI 残留或半帧旧内容；原因是待机页面只关闭背光，没有清 GUI 层，且退出待机时过早打开背光，Home UI 还没完全刷新到屏幕。
   涉及文件：`FF_1070/layout/layout_standby.c`、`FF_1070/layout/layout_home.c`
   修改内容：进入 standby 时关闭背光后立即调用 `fb_gui_layer_rect_fill()` 清黑整屏，并刷新整屏 GUI 区域；待机退出时不再提前打开背光；Home 页面创建完成后不再立即开背光，而是新增 `home_backlight_task` 延时打开背光，给 LVGL 背景和按钮刷新留出时间，减少亮屏瞬间旧 UI 外露。

3. 待机点击唤醒防重入和连续点击死机修复
   问题：待机黑屏后连续快速点击屏幕，可能重复触发 `goto_layout(home)`，或在 Home 延迟亮屏任务未执行前再次切页，导致 LVGL task 指针悬空、重复删除任务，严重时室内机死机或长时间黑屏。
   涉及文件：`FF_1070/layout/layout_standby.c`、`FF_1070/layout/layout_home.c`
   修改内容：待机背景点击从松手 `CLICKED` 改为按下 `PRESSED` 即响应，减少黑屏唤醒时 release 丢失导致的无响应；新增 `standby_wakeup_in_progress` 标志，第一次点击后立即取消 standby 屏幕点击回调，防止唤醒过程中重复进入 `goto_layout(home)`；Home 的延时亮屏任务设置 `clean_lock = false`，不再由全局 `lv_task_clean()` 自动删除，改为在 `LAYOUT_QUIT_FUNC(home)` 中手动删除并置空，避免悬空指针；Home 亮屏任务未完成前，Home 上的时间、媒体、内线、监控、初始化、设置、待机按钮回调全部直接返回，避免黑屏延迟期间连续点击误触发其它页面切换。

4. 待机移动侦测误触发处理
   问题：开启移动侦测后进入待机，移动侦测检查任务启动后会直接进入 “Motion detected” 跳转流程，没有先判断是否真正检测到移动，可能导致待机唤醒、内线来电或其它切页时被移动侦测流程抢占，出现闪帧、延迟响应或黑屏异常。
   涉及文件：`FF_1070/layout/layout_standby.c`
   修改内容：在 `motion_move_check_task()` 中增加 `motion_detection_check()` 判断；当没有检测到移动时直接返回，只有检测到真实移动后才清理延时任务/定时任务、设置 `standby_entering_motion_detection`，并进入移动侦测页面；保留内线忙碌时停止并销毁移动侦测资源的原有保护逻辑。

### 20260709

1. 视频回放退出待机后黑屏处理
   问题：回放视频后挂断退到待机，重复几次后室内机可能一直黑屏；触摸有按键音、串口也有反应，说明主循环仍在，但显示层或 LVGL task 状态异常。
   涉及文件：`FF_1070/common/video_play.c`、`FF_1070/layout/layout_standby.c`、`FF_1070/common/lv_msg_event.c`、`FF_1070/share/lvgl/src/lv_misc/lv_task.c`
   修改内容：`video_play_stop()` 改为即使当前已经是 `VIDEO_PLAY_STATE_IDLE`，也会兜底关闭功放、关闭 AVI 句柄、恢复 JPEG 解码回调、关闭 `lv_video_mode_enable(false)` 和 `video_display_preview_enable(false)`，避免视频层残留影响 GUI 刷新；进入 standby 时增加关闭 video preview/video mode 和清黑 GUI 层；`goto_layout()` 调整为先执行旧页面 `quit()`，再做全局 task/动画/对象清理，避免页面自己的 task 指针被全局清理提前删掉后再次删除；修复 `lv_task_clean()` 遍历链表时边遍历边删除当前节点的问题，改为删除前先保存 next 节点，避免访问已释放 task 节点导致任务链表异常。

2. 待机摘挂机不亮屏及功放异常处理
   问题：待机黑屏时拿起话筒会亮屏，放下话筒会挂机黑屏；拿起话筒期间播放 call 铃声或视频文件，挂掉话筒时功放会瞬间打开，表现为声音突然放大。
   涉及文件：`FF_1070/layout/layout_common.c`、`FF_1070/common/video_play.c`、`FF_1070/layout/layout_memory_video.c`
   修改内容：`layout_hook_state_change_default()` 在 standby 页面收到摘机/挂机事件时只打印并返回，不再 `goto_layout(home)`，避免待机摘机亮屏；camera 页面挂机分支先停止铃声、关闭门口机铃声通道并关闭功放，再退 standby；`ring_play()` 增加摘机但非通话页面的保护，避免媒体或普通提示音在摘机状态主动打开功放；`video_play_start()` 根据 `hook_state_get()` 决定功放状态，摘机时保持功放关闭；`layout_memory_video` 退出时删除无条件 `power_amplifier_enable(true)`，避免视频页退出后把功放重新打开。

3. 录像倒计时开锁时 UI 卡住修复
   问题：监控界面正在录像倒计时时点击开锁，实际倒计时仍在运行，但界面上的倒计时数字会停住，直到开锁状态结束或界面切换后才恢复。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：`camera_record_video_count_down_task()` 不再用 `is_opening` 屏蔽录像倒计时 UI 刷新；开锁期间录像倒计时 label 和录像图标仍持续显示、更新并执行 `lv_obj_invalidate()`；录像结束或异常结束时也不再受开锁状态影响，统一隐藏倒计时 label、录像图标和相关提示对象，避免旧倒计时或提示残留。

4. 待机移动侦测不触发修复
   问题：进入待机后移动侦测看起来不触发；原因是 `motion_detection_check()` 依赖 `video_input_state_get() == true`，但 standby 移动侦测启动时只设置了监控通道，没有打开 VIN，导致没有视频帧可用于移动侦测。
   涉及文件：`FF_1070/layout/layout_standby.c`
   修改内容：在 `motion_delay_start_task()` 中设置移动侦测通道后调用 `monitor_open(false, 0x01)`，让待机黑屏状态下打开视频输入但不显示预览；移动侦测专用启动后把 `video_input_skip_frame_count_set(15)`，避免 `monitor_open()` 默认 1000 帧过滤导致长时间不检测；退出 standby 且不是进入移动侦测页面时调用 `monitor_close()`，避免待机后台视频输入残留。

5. 移动侦测点击回 Home 闪旧画面修复
   问题：进入移动侦测界面后点击屏幕返回 Home，正常流程应先黑屏再进入 Home，但实际在跳转到 Home 前会闪一下移动侦测旧画面，表现为 UI 残留。
   涉及文件：`FF_1070/layout/layout_motion_detection.c`
   修改内容：移动侦测背景点击回调中先取消屏幕点击回调、关闭背光、关闭 video preview、关闭 video mode、清黑 GUI 层并刷新整屏，再执行 `goto_layout(home)`；`LAYOUT_QUIT_FUNC(motion_detection)` 中也补充关闭 `lv_video_mode_enable(false)`，并统一执行 `monitor_close()` 和后续资源等待，避免点击回 Home 时跳过关闭等待导致视频层回刷旧帧。

6. 拍照提示图概率残留修复
   问题：监控界面拍照时，拍照提示图片有概率一直显示在屏幕上，直到退出监控界面才消失；开锁期间抓拍更容易出现，因为旧逻辑在 `is_opening == true` 时不隐藏抓拍提示。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：`camera_record_image_end_task()` 不再受 `is_opening` 限制；拍照结束任务触发后，只要抓拍提示图和提示容器存在，就直接隐藏并执行 `lv_obj_invalidate()`，随后清除 `is_recording` 并停止 `CAMERA_TASK_RECORD_IMAGE`，保证抓拍提示不会因为开锁状态或刷新窗口错过隐藏。

### 20260713

1. 监控画面调节窗口与拍照/录像提示重叠修复
   问题：监控界面打开色彩、亮度、对比度调节窗口后，仍可触发抓拍、录像或开锁；原流程只刷新局部区域而没有关闭调节窗口，导致滑块窗口与抓拍相机图、录像提示或开锁提示同时显示，并可能留下旧图层。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：新增 `camera_setting_window_close_for_action()`，在自动抓拍、手动抓拍、手动录像和开锁前统一关闭画面调节窗口；调节窗口显示函数调整为先更新对象隐藏状态再刷新对应区域，并为亮度、色彩、对比度滑块容器增加空指针保护，避免窗口对象未完整创建时访问异常。

2. 摘机状态下监控切换门口机通话修复
   问题：拿起话筒进入监控后，当前门口机可以直接对讲；切换到另一台门口机时，原对讲路由已关闭但目标通道没有重新建立通话，导致切换后无声音。切换到 CCTV 时也必须确保不能保留门口机对讲。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：`camera_channel_switch_internal()` 切换前统一关闭旧门口机对讲路由并清除当前通话状态；切换完成后仅当听筒仍处于拿起状态且目标通道为 `DOOR1/DOOR2` 时，调用对应 `door_audio_talk()` 建立目标门口机通话并同步通话状态；目标为 `CCTV1/CCTV2` 或听筒已放下时保持音频关闭。

3. 视频媒体页 SD 卡状态切换资源清理
   问题：视频媒体页收到 SD 卡拔出事件时，如果播放状态任务、超时任务或视频缓冲仍在运行，页面切换期间可能继续访问已经失效的 SD 媒体资源。
   涉及文件：`FF_1070/layout/layout_memory_video.c`
   修改内容：新增 `memory_video_sdcard_removing` 防止重复拔卡事件重复进入切页流程；拔卡处理先禁用页面操作、停止超时和播放状态任务，调用 `video_play_stop()` 关闭媒体播放，再清理视频显示缓冲后刷新媒体页面；重新进入视频页时复位该状态标志。

4. SD 图片循环播放拔卡概率死机修复
   问题：SD 图片循环播放时拔卡，定时任务可能已经进入下一张图片加载；文件索引会失效，JPEG 解码可能读取到不完整数据。日志表现为 `mmcblk0 error -110`、`JPEGDEC_INCREASE_INPUT_BUFFER`、`thumb media wait decode finish timeout` 和 `find sd media index failed`，随后可能访问空的图片信息导致死机。
   涉及文件：`FF_1070/layout/layout_memory_photo.c`、`FF_1070/common/media_thumb.c`
   修改内容：新增统一的 `memory_photo_sdcard_exit()`，在拔卡回调、循环播放任务、下一张翻页和图片加载入口复用，停止循环/超时任务、关闭 JPEG 缩略图解码、重置索引并退出到媒体列表；图片加载前增加 SD 状态和 `media_file_info_get()` 返回值检查，避免空指针访问；`thumb_media_jpg_load()` 增加内存分配及完整读取校验，SD 读取失败或短读时不再把残缺数据送入 JPEG 解码器；缩略图解码启动失败或等待超时时清理解码缓存并返回失败，由图片页立即结束 SD 图片播放流程。

5. 移动侦测拍照图标常驻显示优化
   问题：移动侦测设置为拍照模式，或未插 SD 卡自动降级为拍照时，进入移动侦测页面后拍照图标一直显示，不符合实际“触发一次拍照”的状态，也影响界面观感。
   涉及文件：`FF_1070/layout/layout_motion_detection.c`
   修改内容：拍照模式创建移动侦测页面时默认隐藏拍照图标；`record_jpeg_start(REC_MODE_MOTION)` 成功后才调用 `motion_detection_photo_icon_flash()` 短暂显示拍照图标；现有拍照结束任务触发时隐藏并刷新图标。录像模式继续常驻显示录像图标和倒计时，不改变原录像提示逻辑。

### 20260716

1. 门口机 Call 铃声响应速度及连续呼叫切换优化
   问题：门口机 Call 后，室内机铃声需要等待视频通道或自动录像准备，导致首次呼叫和连续 Call 响应迟钝；Door1 正在呼叫时 Door2 再次呼叫，也需要及时停止旧铃声、切换目标通道并播放新铃声，不能继续等待上一段铃声或录像流程结束。
   涉及文件：`FF_1070/layout/layout_camera.c`、`FF_1070/tests/check_camera_async_switch.sh`
   修改内容：移除 Call 铃声启动对固定延时、视频就绪和自动录像任务状态的依赖，新增 `camera_call_ring_start()`，在门口机 Call 进入监控、同通道再次 Call 或 Door1/ Door2 切换呼叫时立即按当前目标通道播放对应铃声；自动拍照/录像继续通过独立任务等待有效视频帧，不再阻塞铃声启动；连续 Call 时先停止旧通道铃声、关闭旧门口机铃声输出并复位旧 Call 状态，再启动新通道切换、自动录像等待和新铃声；修正门口机铃声重复播放判断的位置，确保重播逻辑只在 Call 铃声结束回调中执行，不会误进入开锁提示音结束回调。

2. 门口机 Call 接听后继续响铃及挂机清理优化
   问题：门口机 Call 后拿起话筒接听，原流程会先调用 `ringplay_play_stop()`，但 Call 铃声状态仍标记为活动，铃声结束回调会判断呼叫尚未结束并重新播放，导致接听后室内机或门口机仍继续响铃；挂下听筒时也需要立即停止铃声并关闭通话音频、视频预览和自动录像等待，不能继续停留在监控或通话状态。
   涉及文件：`FF_1070/layout/layout_camera.c`、`FF_1070/layout/layout_common.c`、`FF_1070/layout/layout_common.h`、`FF_1070/tests/check_camera_async_switch.sh`
   修改内容：新增统一的 `layout_camera_hook_answer()` 接听入口，在停止播放前先将 Call 铃声设置为已接听并清除重播截止时间，随后关闭 Door1、Door2 的门口机铃声输出、同步停止室内机铃声，再建立当前门口机通话路由；接听后如果门口机再次 Call，会重新建立新的 Call 状态并重新播放铃声。新增 `layout_camera_hook_hangup()` 挂机入口，统一取消通道切换和自动录像等待任务、取消并同步停止铃声、关闭门口机音频路由和音频采集、禁用视频预览并返回待机，由监控退出流程继续关闭 VI、录像和拍照资源；物理话筒事件与监控界面挂断按钮统一调用这两个入口，并移除监控定时任务中重复的 500ms 话筒状态轮询处理，避免接听和挂机逻辑重复执行或状态不同步。

### 20260718

1. Home 监控和 camera 通道切换触摸音/功放时序回退
   问题：前面为处理触摸音和功放时序，把 Home 监控图标和 camera 通道切换按钮接入了新的按键音/功放等待逻辑，导致点击 Home 监控图标、监控界面切换 Door/CCTV 通道时触摸音异常，并且通道切换响应被额外任务影响。
   涉及文件：`FF_1070/layout/layout_home.c`、`FF_1070/layout/layout_camera.c`
   修改内容：删除 Home 监控按钮的延后进入任务，恢复为点击后直接设置 `MON_CH_DOOR1`、`MON_ENTER_MANUAL_DOOR` 并进入 `camera`；移除 camera 通道切换的延后任务和取消逻辑，Door1、Door2、CCTV1、CCTV2 按钮恢复为直接调用 `camera_channel_switch_internal()`；监控界面的 `layout_camera_click_down_func()` 恢复为空实现，不再让通道切换按钮走新的触摸音/功放时序；保留媒体页相关按键音优化，不影响 Home 进入媒体的处理。

2. 视频播放界面返回视频文件列表响应速度优化
   问题：进入视频播放界面后点击返回视频文件列表，界面响应明显变慢；原因是 `memory_video` 退出时无条件调用 `thumb_media_close()`，会关闭 JPEG/缩略图解码资源，而返回 `photo_list` 属于媒体模块内部跳转，不需要立即关闭该资源。
   涉及文件：`FF_1070/layout/layout_memory_video.c`
   修改内容：调整 `LAYOUT_QUIT_FUNC(memory_video)` 的缩略图资源释放条件，只有真正离开媒体模块时才调用 `thumb_media_close()`；当目标页面是 `photo_list`、`memory_photo` 或 `memory_video` 时保留缩略图资源，减少关闭解码器导致的阻塞；保留 SD 卡拔出路径中的播放停止和资源清理，避免拔卡后继续访问无效媒体文件。

3. Flash 图片和 SD 图片播放界面返回列表响应速度补齐
   问题：视频播放页返回列表已经变快，但 Flash 图片界面和 SD 图片界面返回媒体列表仍然较慢；原因是 `memory_photo` 退出时仍无条件关闭 `thumb_media`，图片播放页没有按视频播放页同样的媒体内部跳转规则处理。
   涉及文件：`FF_1070/layout/layout_memory_photo.c`
   修改内容：将 `LAYOUT_QUIT_FUNC(memory_photo)` 中的 `thumb_media_close()` 改为条件执行；返回 `photo_list`、`memory_photo` 或 `memory_video` 时不关闭缩略图解码资源，只有真正离开媒体模块时才关闭；SD 卡拔出处理路径继续保留强制 `thumb_media_close()`，防止循环播放或图片加载任务继续读取已拔出的 SD 卡。

4. 触摸音异常和触摸音后跟随“啪”声处理
   问题：媒体相关页面为解决按键音异常，引入了页面常开功放、延迟关功放和点击前预处理，但点击路径里调用 `audio_output_device_restart()` 会和 ringplay 触摸音播放线程抢 AO，导致触摸音双响、变调或被截断；随后触摸音结束回调又触发延迟关功放任务，GPIO9 关闭功放时会在触摸音后跟随一声“啪”。
   涉及文件：`FF_1070/layout/layout_common.c`、`FF_1070/layout/layout_common.h`、`FF_1070/layout/layout_home.c`、`FF_1070/layout/layout_photo_list.c`
   修改内容：新增 `layout_media_keytone_prepare()`，用于 UI 点击前只取消待执行的功放关闭任务、打开功放并设置触摸音音量，不再重启 AO；Home 进入 Media 的点击预处理和媒体列表点击预处理改用该轻量函数，避免每次点击都执行 `ak_ao_cancel()`/`ak_ao_restart()`；保留 `layout_media_audio_prepare()` 在媒体页面进入时初始化 AO，不影响媒体播放页的音频准备；将 `ringplay_keysound_finish_default_func()` 改为不再调用 `layout_media_power_amplifier_release()`，触摸音播放完成不再主动关闭功放，避免每次触摸音后产生功放关闭“啪”声；功放关闭仍由媒体页退出、通话结束、铃声结束等状态边界负责。

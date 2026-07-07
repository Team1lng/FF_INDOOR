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

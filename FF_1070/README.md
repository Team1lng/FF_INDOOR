## 20260628
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

## 20260629
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

### 20260702-03

1. 监控通道切换 quit/enter 交织死机修复
   问题：`camera_change_door1/2_btn_up` 和 `camera_change_cctv1/2_btn_up` 通过 `goto_layout(pLAYOUT(camera))` 切换通道，触发 `LAYOUT_QUIT_FUNC(camera)` → `LAYOUT_ENTER_FUNC(camera)` 完整退出/进入流程，两边的硬件操作（关闭 VI/音频 vs 打开音频/VI）在异步线程中交织执行导致死机。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：新增 `camera_channel_switch_internal(MON_CH target_ch)` 页内切换函数，直接关闭预览 → 停止录像 → 切换通道 → monitor_open → 等待 VI 就绪 → 刷新 UI，不走 layout 退出/进入；四个 `camera_change_*_btn_up` 改为调用该函数。

2. 门口机呼叫切换通道花屏修复
   问题：两台门口机快速切换通道时，视频预览在 `monitor_open()` 中立即开启，但 TP9950/VI 在异步线程中复位重新初始化，视频层显示缓冲区零值或旧数据导致花屏。
   涉及文件：`FF_1070/layout/layout_camera.c`、`FF_1070/common/video_input.c`
   修改内容：`camera_door_call_switch()` 中 `monitor_open()` 改为 `monitor_open(false, 0x03)` 暂不开启预览，等待 `video_input_state_get()` 返回 true（VI 已打开+跳帧完成+格式有效）后再 `video_display_preview_enable(true)`；`video_input_resident_bzero()` 修复缓冲区格式 bug（原按 4 字节 ARGB 写 `0xFF000000` 到 3 字节 RGB888 缓冲区，溢出且产生彩色条纹），改为 `memset(buffer, 0, W*H*3)` 并加 `video_main_display_lock()` 防半屏黑。

3. I2C 写失败修复
   问题：`tp9950_and_isp_device_enable(false)` 中 `tp9950_power_off()` 断电后仍调用 `tp9950_vin_enable(ch, true)` 写 I2C，芯片无响应导致 `i2c addr:44 write failed` 反复打印；`en=true` 时 `comm_init()` 后立即 `vin_enable`，芯片未就绪也概率失败。
   涉及文件：`FF_1070/layout/tp9950.c`
   修改内容：将 `tp9950_vin_enable()` 移入 `if (en == true)` 块内，并在 `tp9950_comm_init()` 后加 `usleep(10ms)` 等待芯片启动稳定；`en=false` 时不再调用 `tp9950_vin_enable`。

4. 录像按钮与拍照按钮互斥修复
   问题：`camera_record_btn_up()`（录像）入口没有 `is_recording` 检查，先点拍照再点录像时两者并发执行。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：`camera_record_btn_up()` 入口添加 `if (video_input_state_get() == false || is_recording == true) return;`

5. 同通道重复呼叫不打断录像
   问题：door1 录像中 door1 再次呼叫，`camera_door_call_switch()` 无条件停止录像再重新创建，导致录像被重置。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：将录像停止/重启逻辑移入 `if (current_ch != target_ch)` 块内，仅在不同通道切换时执行；同通道呼叫时录像继续不受影响。

6. 切换通道取消录制
   问题：录像中点击切换通道按钮，录像继续运行，应该取消录制。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：`camera_channel_switch_internal()` 入口处添加录像/拍照停止 + UI 清理（隐藏倒计时标签、录制背景、恢复按钮图标）。

7. 切换通道后图标不显示修复
   问题：图标自动隐藏后（`func_btn_diaplay_flag = false`），call 机切通道调用 `camera_channel_ui_refresh()` 传入 `false`，图标保持隐藏。
   涉及文件：`FF_1070/layout/layout_camera.c`
   修改内容：`camera_door_call_switch()` 中切通道后调用 `camera_func_btn_diaplay_enable(true)` + `camera_btn_and_win_hidden_task_restart()` 强制显示图标并重启自动隐藏计时器。

8. 内线通话房号显示 2 位
   问题：嵌入式 C 库不支持 `%hhu` 格式，`%hhu%hhu` 被当作 `%u` 处理，房号 01 只显示 "0"；被叫/通话界面使用 `%03u` 显示 3 位如 "001"。
   涉及文件：`FF_1070/layout/layout_intercom_in.c`、`layout_intercom_out.c`、`layout_intercom_talk.c`
   修改内容：本机房号 `%hhu%hhu` 改为 `%02u`，参数改为 `device_id[0] * 10 + device_id[1]`；对方房号 `%03u` 改为 `%02u`。

9. 内线通话挂断延迟优化
   问题：挂断后 `hung_up_task` 延迟 2000ms 才退出界面，用户感觉卡顿。
   涉及文件：`FF_1070/layout/layout_intercom_in.c`、`layout_intercom_out.c`、`layout_intercom_talk.c`
   修改内容：延迟从 2000ms 改为 300ms。

10. 内线通话音量调节按钮屏蔽
    问题：音量调节在内线通话中无实际效果（GPIO 控制的是铃声功放电路，不控制内线音频路径），用户要求去掉。
    涉及文件：`FF_1070/layout/layout_intercom_in.c`、`layout_intercom_out.c`、`layout_intercom_talk.c`
    修改内容：注释掉 `*_sound_btn_create(parent)` 和 `*_volume_slider_create(parent)` 调用。

11. 呼出状态提示居中修复
    问题：`LAYOUT_INTERCOM_LANG_NO_ANSWER_ID` 状态提示 x=350 偏左。
    涉及文件：`FF_1070/layout/layout_intercom_out.c`
    修改内容：状态标签宽度 300→400，x=350→450。

12. 视频播放界面删除弹窗残留导致死机修复
    问题：视频播放→暂停→删除弹窗打开时被门口机 call 打断，`LAYOUT_QUIT_FUNC(memory_video)` 只将 `dim_mask` 和 `memory_video_delete_box` 指针置 NULL 但未删除 LVGL 对象（对象挂在 `lv_scr_act()` 上不会随布局自动销毁），再次进入 memory_video 时 `create_dim_mask()` 创建新对象 → 旧对象泄漏累积 → 崩溃。
    涉及文件：`FF_1070/layout/layout_memory_video.c`、`layout_memory_photo.c`
    修改内容：`LAYOUT_QUIT_FUNC` 中先调用 `lv_obj_del(dim_mask)` 和 `lv_obj_del(memory_video_delete_box)` 再置 NULL；`memory_photo` 同样修复 `dim_mask` 泄漏。

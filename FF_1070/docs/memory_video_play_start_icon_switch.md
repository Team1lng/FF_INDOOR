# 视频播放按钮图标切换说明

## 问题需求

`memory_video` 视频播放界面原来只有一个 `play` 图标，通过按下变暗和恢复来区分播放/暂停状态，不够直观。

现在需要改成两个图标状态：

- 进入界面时显示 `play.png`。
- 点击播放后显示 `start.png`。
- 再次点击暂停后恢复为 `play.png`。
- 继续点击恢复播放后再次显示 `start.png`。
- 视频播放完毕后恢复为 `play.png`。

## 修改内容

- `video_play_btn_state_display(true)` 改为显示 `ROM_UI_MEMORY_START_PNG`。
- `video_play_btn_state_display(false)` 保持显示 `ROM_UI_MEMORY_PLAY_PNG`。
- 播放按钮创建后默认刷新为 `play` 图标。
- 取消播放按钮按下时的黑色暗沉效果，避免和图标状态混淆。

## 点击播放后仍不切换的修复

板端 UI 恢复正常后，点击 `play` 按钮仍没有切到 `start`。检查代码后确认资源宏和按钮换图接口都已经存在，问题在 `layout_play_state_task()` 的状态轮询逻辑。

原逻辑里有一段被注释破坏的 `if/else` 结构，播放状态变化后可能继续执行“暂停、停止、播放结束”的分支，把按钮重新设置成 `play.png`，导致点击后看起来没有切换。

本次只修改 `layout_memory_video.c`：

- 将 `layout_play_state_task()` 改为明确按 `VIDEO_PLAY_STATE_PLAY / PAUSE / IDLE` 三态处理。
- 播放态固定调用 `video_play_btn_state_display(true)`，显示 `start.png`。
- 暂停或结束态只在状态发生变化时调用 `video_play_btn_state_display(false)`，显示 `play.png`。
- 增加 `memory_video_timeout_task` 句柄，避免暂停/结束后每 100ms 重复创建待机计时任务。
- 页面进入、退出和超时跳转时清空 `memory_video_timeout_task`，避免旧任务被布局清理后留下脏指针。

本次没有重新生成或修改 `rom.bin / rom.h`，避免再次影响其它 UI 资源。

## 点击视频文件后卡住的修复

增加 `start.png` 切换后，`video_play_btn_state_display()` 不只负责换图，还会同步开关 HOME/PREV/NEXT/DELETE 等按钮点击状态。

原实现直接写成：

```c
lv_obj_set_click(lv_obj_get_child_form_id(lv_scr_act(), MEMORY_xxx_BTN_ID), true);
```

如果页面创建过程中某个按钮对象还没创建完成，或者 ID 查找失败，`lv_obj_set_click()` 会收到 `NULL`，LVGL 内部会断言/访问空指针，表现为点击视频文件进入页面时卡住。

本次修复：

- 新增 `memory_video_btn_click_set()`，只有对象存在时才调用 `lv_obj_set_click()`。
- `memory_func_btn_diaplay_enable()` 批量隐藏/显示按钮时也增加空指针保护。
- 去掉 `video_play_btn_create()` 内部立即调用 `video_play_btn_state_display(false)`，避免播放按钮刚创建时就操作其它还没创建的按钮。

这次修复只处理 `start.png` 改动引入的 UI 对象访问风险，不改资源文件和视频预览流程。

## 暂停和播放结束后不回切 play 的修复

现象：

- 进入界面显示 `play.png` 正常。
- 点击播放后能切到 `start.png`。
- 点击 `start.png` 暂停后没有切回 `play.png`。
- 播放结束后也没有自动切回 `play.png`。

原因是 `video_play_btn_state_display()` 同时给 `LV_STATE_DEFAULT` 和 `LV_STATE_PRESSED` 设置了图标。点击 `start.png` 时按钮处于 pressed 状态，pressed 状态里仍保存着 `start.png`，会覆盖默认状态刚设置的 `play.png`，看起来就没有切换。

本次修复：

- 运行时只给 `LV_STATE_DEFAULT` 设置当前状态图标。
- 播放按钮创建时把 `LV_STATE_PRESSED` 固定为 `play.png`，避免按住/释放期间继续显示 `start.png`。
- 不再运行时调用 `lv_obj_remove_style_local_prop()` 删除 pressed 样式；该接口参数容易误用，错误传参会导致进入视频界面异常卡住。
- `layout_play_state_task()` 播放态只在状态变化时刷新 `start.png`，避免 100ms 轮询反复覆盖按钮回调刚设置的图标。

修复后预期：

- 点击 `play.png` 开始播放，按钮显示 `start.png`。
- 点击 `start.png` 暂停，按钮显示 `play.png`。
- 再点击 `play.png` 继续播放，按钮显示 `start.png`。
- 视频播放结束后，状态变为非播放态，按钮自动恢复 `play.png`。

## 资源说明

代码使用的资源宏为：

- `ROM_UI_MEMORY_PLAY_PNG`
- `ROM_UI_MEMORY_START_PNG`

如果后续替换 `start.png`，需要重新生成 `rom.bin` 并打包 app 分区。

## 编译报错修复

本次编译报错点是 `layout_memory_video.c` 引用了 `ROM_UI_MEMORY_START_PNG`，但 `res/rom.h` 中没有同步生成该资源宏。

已重新同步 `res/rom.bin` 和 `res/rom.h`：

- `res/ui/memory/start.png` 已写入 `rom.bin`。
- `rom.h` 已生成 `ROM_UI_MEMORY_START_PNG` 和 `ROM_UI_MEMORY_START_PNG_SIZE`。
- `start.png` 当前资源尺寸为 `120x120`，和播放按钮区域一致。

注意：`rom.h` 里的 offset 必须和 `rom.bin` 完全对应，不能只手工补一个宏名，否则可能编译通过但运行时显示错图。

## 验证方式

```sh
cd FF_1070
make
```

当前验证结果：

- `cd FF_1070 && make` 编译通过。
- 已生成 `FF_1070/build/FF.BIN`。
- 已生成 `FF_1070/upgrade/platform/app.sqsh4`。
- 已同步生成 `AK37E_SDK_V1.05/upgrade/platform/app.sqsh4`。

板端验证：

1. 进入视频播放界面，确认按钮显示 `play.png`。
2. 点击播放，确认按钮切为 `start.png`。
3. 点击暂停，确认按钮切回 `play.png`。
4. 再次点击播放，确认按钮再次切为 `start.png`。
5. 视频播放结束后，确认按钮自动恢复为 `play.png`。

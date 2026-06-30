# camera 听筒状态跟踪修复说明

## 针对问题

camera 界面定时任务 `camera_ticker_task()` 中，之前使用函数内部 `static bool last_hook_state` 记录上一次听筒状态。

函数内 `static` 变量不会随着 camera 页面退出自动重置。如果多次进入/退出 camera 界面，或者进入界面前听筒已经处于拿起状态，旧的 `last_hook_state` 可能残留，导致以下风险：

- 新一次进入 camera 界面时，听筒状态变化检测不准确。
- 拿起话筒接听逻辑 `camera_hook_answer_call()` 可能不触发，或者状态和页面实际状态不一致。
- 页面内 `camera_in_talk_state` 与听筒真实状态容易出现不同步。

## 修改内容

修改文件：

- `FF_1070/layout/layout_camera.c`

新增状态：

- `camera_last_hook_state`

具体修改：

- 将 `camera_ticker_task()` 内部的 `static bool last_hook_state` 改为文件级状态 `camera_last_hook_state`。
- `camera_ticker_task()` 使用 `camera_last_hook_state` 判断听筒状态是否变化。
- camera 页面进入时，读取当前 `hook_state_get()`，同步到 `camera_in_talk_state` 和 `camera_last_hook_state`。
- camera 页面退出时，将 `camera_last_hook_state` 重置为 `false`，避免状态残留到下一次进入。

## 影响范围

主要影响 camera 界面内听筒状态检测和接听触发逻辑。

不改变以下逻辑：

- 听筒拿起后仍调用 `camera_hook_answer_call()`。
- `camera_hook_answer_call()` 内部停止铃声、进入 `door_audio_talk()` 的流程保持不变。
- 挂断按钮和退出 camera 页面逻辑保持原有行为。

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
fix: reset camera hook state tracking per entry
```

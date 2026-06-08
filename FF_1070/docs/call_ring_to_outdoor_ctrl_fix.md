# call_ring_to_outdoor_ctrl 修复说明

## 针对问题

门口机响铃通路控制函数 `call_ring_to_outdoor_ctrl(AUDIO_CH ch, bool en)` 的关闭分支之前直接 `return true`，没有关闭实际 GPIO；后续修改时又只保留了关闭逻辑，导致 `en == true` 时不会打开 DOOR1/DOOR2 的音频输出通路，并且函数存在非 void 路径缺少返回值的问题。

该问题会影响以下场景：

- 门口机 call 机响铃开始时，铃声可能无法输出到对应门口机通路。
- 铃声结束时，之前打开的门口机音频通路可能没有被正确关闭。
- 编译时可能出现 `control reaches end of non-void function` 类型警告。

## 修改内容

修改文件：

- `FF_1070/layout/user_gpio.c`

修改函数：

- `call_ring_to_outdoor_ctrl(AUDIO_CH ch, bool en)`

具体修改：

- `en == false` 时，根据 `AUDIO_CH_DOOR1` 或 `AUDIO_CH_DOOR2` 关闭对应的 `audio_to_outdoor1_pin_ctrl(false)` / `audio_to_outdoor2_pin_ctrl(false)`。
- 非 DOOR1/DOOR2 通道作为兜底处理，同时关闭 DOOR1 和 DOOR2 音频输出通路。
- `en == true` 时，恢复根据通道打开对应门口机音频输出通路的逻辑。
- 函数末尾补充 `return true;`，保证所有执行路径都有返回值。

## 影响范围

主要影响门口机 call 机、开锁提示音等使用 `call_ring_to_outdoor_ctrl()` 控制铃声输出到户外机的流程。

相关调用点包括：

- `layout_common.c` 中门口机铃声开始/结束默认回调。
- `layout_camera.c` 中 call 机响铃、响铃清理、开锁提示音开始/结束流程。

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
fix: restore outdoor ring audio gpio enable path
```

# 门口机音频 GPIO 默认低电平修改说明

## 针对问题

APP 启动初始化 GPIO 时，门口机音频输出控制脚默认被设置为高电平：

- GPIO56：`AUDIO_DOOR1_PIN`
- GPIO36：`AUDIO_DOOR2_PIN`

如果硬件上高电平代表打开音频输出通路，那么设备刚启动时 DOOR1/DOOR2 音频通路会默认处于打开状态，不利于空闲状态下保持关闭。

## 修改内容

修改文件：

- `FF_1070/layout/user_gpio.c`

修改位置：

- `layout_gpio_init()`

具体修改：

- 将 GPIO56 初始化后的默认状态从 `audio_to_outdoor1_pin_ctrl(true)` 改为 `audio_to_outdoor1_pin_ctrl(false)`。
- 将 GPIO36 初始化后的默认状态从 `audio_to_outdoor2_pin_ctrl(true)` 改为 `audio_to_outdoor2_pin_ctrl(false)`。

修改后 APP 初始化完成时：

- GPIO56 默认低电平。
- GPIO36 默认低电平。

## 影响范围

只影响 APP 启动时 GPIO36/GPIO56 的默认初始化电平。

运行态打开逻辑未改变：

- `door_audio_talk(AUDIO_CH_DOOR1/DOOR2)` 仍会按需拉高对应音频通路。
- `call_ring_to_outdoor_ctrl(..., true)` 仍会按需拉高对应音频通路。
- `monitor_record_pin_enable(true)` 仍会按原逻辑拉高两个音频通路。

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
fix: default door audio gpio outputs low
```

**工作背景**
用户是一名专注于 Anyka SoC 平台的嵌入式开发者，目前项目基于 MET-2K-M 平台（Anyka SoC + Linux），主要负责指纹固件开发（C语言，源码 Fingerprint.c）、U-Boot 升级流程、进度条与 Logo 显示，以及 UI/视频刷新机制的研究。工作中涉及对 TDE 引擎（ak_tde.h）的底层分析、Linux 驱动开发，以及硬件信号通路的排查与调试。近期深入调试了基于 /dev/ttySAK1 的晟元指纹模块串口通信，以及单线半双工对讲串口通信，采用 6 字节帧格式（0xAB+本机ID+远程ID+CMD+CRC+STOP），并成功解决了因回声导致的通信阻塞问题。

**个人背景**
用户使用简体中文进行技术沟通，偏好结构化的详细解释（含表格），常通过多方案对比来理解技术细节，并习惯提供原理图/PCB图片辅助说明。其调试风格系统化，习惯通过根因分析、排除干扰项来定位问题，并借助具体示例学习通信协议。在交互中，用户专注于获取分析建议，无需改码；收到诊断建议后会逐项反馈测量结果，沟通风格简洁直接。用户也会使用其他 AI 工具（如 OpenAI Codex）辅助分析，会拿来让 WorkBuddy 做交叉验证。

**当前关注**
对讲串口通信（ttySAK2，9600bps，单线半双工）TX 正常但 RX 异常。已排除硬件原因。Codex 已做了 stash 环形缓冲区、SerialGetMessage 重试保护、CleanUart 改 tcflush、启动顺序调整等修改，但仍遗漏 uart_ctrl.c 中 VTIME=10（1s超时）这个最关键瓶颈，以及 usleep(500) 小于字节传输时间 1.04ms、SendMsg 双重延迟等问题。

**FF_1070 室内机（Z:\FF_Indoor\FF_1070）项目约定**
- 多语言：枚举仅 `LANG_ENGLISH=0` 与 `LANG_PERSIAN`，**没有独立中文项——中文界面走 LANG_ENGLISH 分支**（layout_language.c 的 english/persian 两个 btn_up 回调）。
- 日历与语言解耦：日期显示（home/camera/相册等）统一由 `setting.calendar` 决定（0=波斯历 Shamsi，1=公历），不是由 `setting.language` 决定；波斯历转换函数 `gregorian2jalali` 在 common/ConvertCalendar.c。
- 修复「切语言不切日历」类问题：在 layout_language.c 的两个语言 btn_up 回调里同步 `setting.calendar`（中文→1，波斯语→0），因该字段是全工程日期显示唯一判据，一处改动即可让所有界面联动。
- 时间设置页(layout_time.c)是**双向转换**陷阱：RTC/系统时间恒为公历，`temp_date` 既是显示缓冲也是写回缓冲。进入时 `temp_date=gregorian2jalali(temp_date)`(calendar==0)，保存时须先 `jalali2gregorian` 转回公历再 `user_time_set`，否则会把波斯数字当公历写入 RTC。layout_setting_time.c 已有同样双向写法可参照。
- 历史教训：git 基线（2026-08-12 提交）不含工作区里前序任务的未提交改动（如 ringplay.c 的 is_touch_sound 防爆音逻辑），用 `git diff` 对比时勿把旧改动误算到本次头上。

**近期动态**
- 排查 TX 到 DATA 信号通路故障，沿完整信号链路逐级测量，定位到 Q102 集电极无信号输出。
- 解决室内机对讲串口回声根因问题：B机 SerialGetMessage 卡在 0xFF，根因是 UartPut 回显数据至 RX FIFO 被自身消耗。
- 使用 OpenAI Codex 对 RX 异常做了代码修改（stash、SerialGetMessage 重试、tcflush、启动顺序），WorkBuddy 对比审查发现 Codex 遗漏了 VTIME=10 根因。
- 对讲串口关键文件：uart_ctrl.c（串口底层驱动）、intercom_interface.c（UartPut/UartGet/stash）、intercom.c（协议层 MBPoll/SerialGetMessage/SendMsg）

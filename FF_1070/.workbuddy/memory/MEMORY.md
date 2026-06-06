**工作背景**
用户是一名专注于 Anyka SoC 平台的嵌入式开发者，目前项目基于 MET-2K-M 平台（Anyka SoC + Linux），主要负责指纹固件开发（C语言，源码 Fingerprint.c）、U-Boot 升级流程、进度条与 Logo 显示，以及 UI/视频刷新机制的研究。工作中涉及对 TDE 引擎（ak_tde.h）的底层分析、Linux 驱动开发，以及硬件信号通路的排查与调试。近期深入调试了基于 /dev/ttySAK1 的晟元指纹模块串口通信，以及单线半双工对讲串口通信，采用 6 字节帧格式（0xAB+本机ID+远程ID+CMD+CRC+STOP），并成功解决了因回声导致的通信阻塞问题。

**个人背景**
用户使用简体中文进行技术沟通，偏好结构化的详细解释（含表格），常通过多方案对比来理解技术细节，并习惯提供原理图/PCB图片辅助说明。其调试风格系统化，习惯通过根因分析、排除干扰项来定位问题，并借助具体示例学习通信协议。在交互中，用户专注于获取分析建议，无需改码；收到诊断建议后会逐项反馈测量结果，沟通风格简洁直接。用户也会使用其他 AI 工具（如 OpenAI Codex）辅助分析，会拿来让 WorkBuddy 做交叉验证。

**当前关注**
对讲串口通信（ttySAK2，9600bps，单线半双工）TX 正常但 RX 异常。已排除硬件原因。Codex 已做了 stash 环形缓冲区、SerialGetMessage 重试保护、CleanUart 改 tcflush、启动顺序调整等修改，但仍遗漏 uart_ctrl.c 中 VTIME=10（1s超时）这个最关键瓶颈，以及 usleep(500) 小于字节传输时间 1.04ms、SendMsg 双重延迟等问题。

**近期动态**
- 排查 TX 到 DATA 信号通路故障，沿完整信号链路逐级测量，定位到 Q102 集电极无信号输出。
- 解决室内机对讲串口回声根因问题：B机 SerialGetMessage 卡在 0xFF，根因是 UartPut 回显数据至 RX FIFO 被自身消耗。
- 使用 OpenAI Codex 对 RX 异常做了代码修改（stash、SerialGetMessage 重试、tcflush、启动顺序），WorkBuddy 对比审查发现 Codex 遗漏了 VTIME=10 根因。
- 对讲串口关键文件：uart_ctrl.c（串口底层驱动）、intercom_interface.c（UartPut/UartGet/stash）、intercom.c（协议层 MBPoll/SerialGetMessage/SendMsg）

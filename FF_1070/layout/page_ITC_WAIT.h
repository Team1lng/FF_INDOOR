/**
 * @file    page_ITC_WAIT.h
 * @brief   底层协议栈回调函数声明（注册到 MBINTERCOM 结构体）
 *
 *  这些函数是底层 intercom.c 驱动 UI 层的唯一入口。
 *  每个函数对应协议栈中的一个事件：
 *
 *    底层事件          回调函数              说明
 *    ─────────────────────────────────────────────────────────────
 *    Wait（被叫等待）  Out_ACKFun            必须调用UiIntercomStateNormal()
 *    Accept（接通）    TalkAccpetFun         双方进入通话状态
 *    HangUp（挂断）    In_HandupFun          任意方挂断
 *    Busy（忙线）      OutWill_BusyFun       对方忙线
 *    Fail（失败）      IntercomUnAckFun      超时无应答
 *    AudioCH0          IntercomAD3Audio      普通内线音频通道
 *    AudioCH1          IntercomAD4Audio      管理机音频通道
 *    AudioOFF          IntercomColseAudio    关闭音频
 */

#ifndef __PAGE_ITC_WAIT_MENU_H__
#define __PAGE_ITC_WAIT_MENU_H__


/** 主叫侧：发出握手请求后等待对端应答 */
void Out_ACKFun(void);

/** 被叫侧：对端已接听 / 主叫侧：对端已应答通话 */
void TalkAccpetFun(void);

/** 任意方：通话挂断 */
void In_HandupFun(void);

/** 主叫侧：对端忙线 */
void OutWill_BusyFun(void);

/** 门口机专用：门口机主动挂断 */
void OutDoorCallHangUp(void);

/** 主叫侧：对端拒接/忙线 */
void BusyForbitCall(void);

/** 主叫侧：呼叫超时无应答 */
void IntercomUnAckFun(void);

/** 空闲状态下收到其它机器的通话请求 */
void IDLE_ACKFun(void);

void IntercomAD3Audio(void);

void IntercomAD4Audio(void);

void IntercomColseAudio(void);

unsigned long IntercomGetTimerSec(void);

void IntercomSwitchAudioChannel(unsigned char ch);

unsigned long IntercomGetTimerMsec(void);

#endif
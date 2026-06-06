/**
 * @file    page_ITC_WAIT.c
 * @brief   底层协议栈回调实现 —— 将底层事件桥接到 UI 高层状态
 *
 *  主叫侧（呼出方）：
 *    1. layout_intercom.c 调用 MsgCallRequest(dest_id)
 *    2. 底层 timer 触发 CallRequestPrcess()
 *    3. MessageSend(NativeId, dest, RQ_HANDSHAKE, 0)  ← 串口发出
 *    4. 等待被叫应答（RP_HANDSHAKE）
 *       → 收到后 Intercom.Wait() → Out_ACKFun()
 *         Out_ACKFun 调用 UiIntercomStateNormal() + 设置UI状态
 *    5. 等待被叫接听（RP_TALKING）
 *       → 收到后 Intercom.Accept() → TalkAccpetFun()
 *         TalkAccpetFun 设置 intercom_remote_ack，layout 检测后跳转通话页
 *
 *  被叫侧（来电方）：
 *    1. 收到 RQ_HANDSHAKE → RQ_IdleReceiveHandshakeRequest()
 *    2. StartRDelayInstruction(CalledCaller, RP_HANDSHAKE)
 *    3. Timer 触发 MessageSend(RP_HANDSHAKE)  ← 串口发出
 *    4. SendMsgProcess(RP_HANDSHAKE) → Intercom.Wait() → Out_ACKFun()
 *       Out_ACKFun 调用 UiIntercomStateNormal()
 *    5. 用户摘机 → layout 调用 MsgCallAccept()
 *    6. MessageSend(RQ_TALKING)  ← 串口发出
 *    7. 收到 RP_TALKING → Intercom.Accept() → TalkAccpetFun()
 *
 *  挂断流程（任意方）：
 *    → MsgCallEnd() → MessageSend(RQ_OFFLINE)
 *    → 对端收到后 MessageSend(RP_OFFLINE)
 *    → Intercom.HangUp() → In_HandupFun()
 *    → 设置 intercom_hangup_flag，layout 检测后跳转
 *
 *  ═══════════════════════════════════════════════════════════════
 */

#include "page_ITC_WAIT.h"
#include "user_intercom.h"
#include "intercom.h"
#include "layout_define.h" 
#include <stdio.h>

void IntercomSwitchAudioChannel(unsigned char ch)
{
    switch (ch) {
        case 0: /* CH_OFF */
            IntercomColseAudio();
            break;
        case 1: /* CH0_ON —— 普通内线 */
            IntercomAD3Audio();
            break;
        case 2: /* CH1_ON —— 管理机/保安 */
            IntercomAD4Audio();
            break;
        default:
            IntercomColseAudio();
            break;
    }
}

void IntercomAD3Audio(void)
{
    printf("[intercom_audio] CH0 ON (intercom line)\n");
    door_audio_talk(AUDIO_CH_INTER); 
}

void IntercomAD4Audio(void)
{
    printf("[intercom_audio] CH1 ON (guard line)\n");
    door_audio_talk(AUDIO_CH_GUARD); 
}

void IntercomColseAudio(void)
{
    printf("[intercom_audio] CH OFF\n");
    door_audio_talk(AUDIO_CH_CLOSE);
}

/* ================================================================
 *  时间戳获取
 * ================================================================ */

unsigned long IntercomGetTimerSec(void)
{
    return user_sec_get();
}

unsigned long IntercomGetTimerMsec(void)
{
    return user_timestamp_get();
}


/* ================================================================
 *  底层协议栈回调实现
 * ================================================================ */

void Out_ACKFun(void)
{
    printf("[intercom_cb] Out_ACKFun: RP_HANDSHAKE sent, UI ready\n");

    /*
     * 告知底层"UI 页面已就绪"
     * 底层 UiStateCheck() 检查此标志，若不设置则超时后 ParamInit()
     */
    UiIntercomStateNormal();

    /*
     * 更新对端号码（来电侧显示来电方房号）
     * GetCalledCallerNumber() 返回底层记录的 CalledCaller（对端号码）
     */
    intercom_number_set((unsigned int)GetCalledCallerNumber());

    /*
     * 角色判定：不要依赖 page-local 的 intercom_call_in_flag，
     * 而是直接使用底层协议栈的 CallTag。
     *
     *  - CallTag == OCR / ACR → 被叫（来电），进入 intercom_in
     *  - CallTag == OCO / ACO → 主叫（呼出），保持 CALL_OUT
     */
    callSIGN tag = GetIntercomCallTag();
    printf("[intercom_cb] Out_ACKFun: CallTag=%d\n", (int)tag);

    if (tag == OCR || tag == ACR) {
        intercom_state_set(INTERCOM_STATE_CALLING_IN);

        /* 若当前不在来电页，则立即跳到 intercom_in */
        const layout *cur = cur_layout_get();
        if (cur != pLAYOUT(intercom_in)) {
            printf("[intercom_cb] Out_ACKFun: goto intercom_in\n");
            goto_layout(pLAYOUT(intercom_in));
        }
    } else {
        intercom_state_set(INTERCOM_STATE_CALL_OUT);
    }
}

/**
 * @brief
 *
 *  调用路径：
 *    被叫侧：MsgCallAccept() → Intercom.Accept()
 *    主叫侧：RQ_ReceiveRequestTalking() → Intercom.Accept()
 */
void TalkAccpetFun(void)
{
    printf("[intercom_cb] TalkAccpetFun: call accepted, entering talk\n");

    intercom_state_set(INTERCOM_STATE_TALKING);

    /*
     * 置位"远端已接听"标志。
     * 主叫侧的 layout_intercom_out 的周期任务检测到此标志后
     * 跳转至 intercom_talk 页面。
     * 被叫侧已在 intercom_in，收到接听后由 intercom_in 的任务处理跳转。
     */
    intercom_remote_ack_set();

    /* 更新对端号码 */
    intercom_number_set((unsigned int)GetCalledCallerNumber());
}

/**
 * @brief  任意一方挂断时触发
 *
 *  调用路径：
 *    MsgCallEnd() → Intercom.HangUp()
 *    CallFinish()  → Intercom.HangUp()
 *    RQ_ReceiveRequestTalking()（被叫被其他事件中断）→ Intercom.HangUp()
 */
void In_HandupFun(void)
{
    printf("[intercom_cb] In_HandupFun: call hung up\n");

    intercom_state_set(INTERCOM_STATE_IDLE);

    /* 置位挂断事件标志，各 layout 的周期任务检测后执行页面跳转 */
    intercom_hangup_flag_set();

    /* 关闭音频 */
    IntercomColseAudio();
}

/**
 * @brief  门口机专用挂断（OutDoorCall 场景，普通内线不涉及）
 */
void OutDoorCallHangUp(void)
{
    printf("[intercom_cb] OutDoorCallHangUp\n");
    intercom_state_set(INTERCOM_STATE_IDLE);
    intercom_hangup_flag_set();
    IntercomColseAudio();
}

/**
 * @brief  主叫侧收到忙线拒接时触发
 *
 *  调用路径：
 *    RP_ReceiveBusyRefuse() → Intercom.Busy()
 *    SendMsgProcess(RP_BUSY_REFUSE) → Intercom.Busy()
 */
void OutWill_BusyFun(void)
{
    printf("[intercom_cb] OutWill_BusyFun: remote busy\n");

    intercom_state_set(INTERCOM_STATE_IDLE);

    /* 置位忙线标志，layout_intercom_out 检测后显示忙线提示并返回 */
    intercom_busy_flag_set(true);

    IntercomColseAudio();
}

/**
 * @brief  对端拒接
 */
void BusyForbitCall(void)
{
    printf("[intercom_cb] BusyForbitCall: call refused\n");
    OutWill_BusyFun();
}

/**
 * @brief  呼叫超时，对端无应答时触发
 *
 *  调用路径：
 *    CallFail() → Intercom.Fail()
 */
void IntercomUnAckFun(void)
{
    printf("[intercom_cb] IntercomUnAckFun: call timeout, no answer\n");

    intercom_state_set(INTERCOM_STATE_IDLE);

    /* 置位无应答标志，layout_intercom_out 检测后显示"无应答"并返回 */
    intercom_unack_flag_set();

    IntercomColseAudio();
}

/**
 * @brief  UiCallWait → 被叫侧：底层已发 RP_HANDSHAKE，进入来电界面等待摘机接听
 */
void IDLE_ACKFun(void)
{
    printf("[intercom_cb] IDLE_ACKFun: callee ring, open intercom_in\n");
    goto_layout(pLAYOUT(intercom_in));
}
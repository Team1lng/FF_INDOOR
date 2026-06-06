/**
 * @file    user_intercom.h
 * @brief   内线对讲高层状态管理 —— UI 层与底层协议栈的桥接接口
 *
 *  分层说明
 *  ┌─────────────────────────────────────────────┐
 *  │  layout_intercom / layout_intercom_in / ... │  UI 页面层
 *  ├─────────────────────────────────────────────┤
 *  │  user_intercom  (本文件)                    │  状态桥接层
 *  ├─────────────────────────────────────────────┤
 *  │  page_ITC_WAIT  (底层回调注册)               │  回调适配层
 *  ├─────────────────────────────────────────────┤
 *  │  intercom.c / intercom_timer.c              │  协议栈层（串口指令收发）
 *  └─────────────────────────────────────────────┘
 *
 *  关键约定
 *  1. UI 层只读/写 intercom_state_t，不直接操作底层 mbStatus。
 *  2. 底层通过 page_ITC_WAIT.c 中注册的回调（Wait/Accept/HangUp/Busy/Fail）
 *     通知 UI，回调内部调用本文件的接口更新状态。
 *  3. Out_ACKFun（Wait 回调）**必须** 调用 UiIntercomStateNormal()，
 *     否则底层超时保护会 ParamInit() 重置所有状态。
 */

#ifndef USER_INTERCOM_H_
#define USER_INTERCOM_H_

#include <stdbool.h>

/* ---------------------------------------------------------------
 * 高层对讲状态枚举
 * --------------------------------------------------------------- */
typedef enum {
    INTERCOM_STATE_IDLE = 0,        ///< 空闲
    INTERCOM_STATE_CALLING_IN,      ///< 被呼入
    INTERCOM_STATE_CALL_OUT,        ///< 主动呼出
    INTERCOM_STATE_TALKING,         ///< 通话中
    INTERCOM_STATE_HUNG_UP,         ///< 挂断处理中
} intercom_state_t;

/** true = 当前是来电，false = 当前是主动呼出 */
extern bool intercom_call_in_flag;

/* ---------------------------------------------------------------
 * 状态读写
 * --------------------------------------------------------------- */
void intercom_state_set(intercom_state_t state);
intercom_state_t intercom_state_get(void);

/* ---------------------------------------------------------------
 * 对端房号
 *   来电时：CalledCallerNumber
 *   呼出时：目标房间号
 * --------------------------------------------------------------- */
void intercom_number_set(unsigned int num);
unsigned int intercom_number_get(void);

/* ---------------------------------------------------------------
 * 忙线标志
 *   由 OutWill_BusyFun / BusyForbitCall 置位
 *   由 layout_intercom_out 轮询后清除
 * --------------------------------------------------------------- */
void intercom_busy_flag_set(bool busy);
bool intercom_busy_flag_get_and_clear(void);

/* ---------------------------------------------------------------
 * 无应答标志
 *   由 IntercomUnAckFun 置位
 *   由 layout_intercom_out 轮询后清除
 * --------------------------------------------------------------- */
void intercom_unack_flag_set(void);
bool intercom_unack_flag_get_and_clear(void);

/* ---------------------------------------------------------------
 * 远端已接听标志
 *   由 TalkAccpetFun 置位
 *   由 layout_intercom_out 轮询后清除，触发跳转至 intercom_talk
 * --------------------------------------------------------------- */
void intercom_remote_ack_set(void);
bool intercom_remote_ack_get_and_clear(void);

/* ---------------------------------------------------------------
 * 挂断事件标志
 *   由 In_HandupFun 置位
 *   由各 layout 轮询后清除
 * --------------------------------------------------------------- */
void intercom_hangup_flag_set(void);
bool intercom_hangup_flag_get_and_clear(void);

#endif /* USER_INTERCOM_H_ */
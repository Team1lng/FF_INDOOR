/**
 * @file    user_intercom.c
 * @brief   内线对讲高层状态管理实现
 *
 *  本文件是 UI 页面层与底层协议栈（intercom.c）之间的状态桥接层。
 *  所有状态变量均在此集中管理，UI 页面只通过本文件的接口读写状态，
 *  底层回调（page_ITC_WAIT.c）也只通过本文件更新状态。
 */

#include "user_intercom.h"
#include <stdio.h>

bool intercom_call_in_flag = false; // true=被叫, false=主叫

/* ---------------------------------------------------------------
 * 内部状态变量
 * --------------------------------------------------------------- */
static intercom_state_t s_state = INTERCOM_STATE_IDLE;
static unsigned int s_intercom_num = 0; ///< 对端房号

static volatile bool s_busy_flag = false; ///< 忙线标志
static volatile bool s_unack_flag = false; ///< 无应答标志
static volatile bool s_remote_ack = false; ///< 远端已接听标志
static volatile bool s_hangup_flag = false; ///< 挂断事件标志

/* ---------------------------------------------------------------
 * 状态读写
 * --------------------------------------------------------------- */

/**
 * @brief 设置高层对讲状态
 * @note  每次状态变化都打印日志，方便调试
 */
void intercom_state_set(intercom_state_t state)
{
    printf("[intercom] state %d -> %d\n", (int)s_state, (int)state);
    s_state = state;
}

intercom_state_t intercom_state_get(void)
{
    return s_state;
}

/* ---------------------------------------------------------------
 * 对端房号
 * --------------------------------------------------------------- */

void intercom_number_set(unsigned int num)
{
    printf("[intercom] remote number = %u\n", num);
    s_intercom_num = num;
}

unsigned int intercom_number_get(void)
{
    return s_intercom_num;
}

/* ---------------------------------------------------------------
 * 忙线标志
 * --------------------------------------------------------------- */

void intercom_busy_flag_set(bool busy)
{
    s_busy_flag = busy;
}

bool intercom_busy_flag_get_and_clear(void)
{
    bool v = s_busy_flag;
    s_busy_flag = false;
    return v;
}

/* ---------------------------------------------------------------
 * 无应答标志
 * --------------------------------------------------------------- */

void intercom_unack_flag_set(void)
{
    s_unack_flag = true;
}

bool intercom_unack_flag_get_and_clear(void)
{
    bool v = s_unack_flag;
    s_unack_flag = false;
    return v;
}

/* ---------------------------------------------------------------
 * 远端已接听标志
 * --------------------------------------------------------------- */

void intercom_remote_ack_set(void)
{
    s_remote_ack = true;
}

bool intercom_remote_ack_get_and_clear(void)
{
    bool v = s_remote_ack;
    s_remote_ack = false;
    return v;
}

/* ---------------------------------------------------------------
 * 挂断事件标志
 * --------------------------------------------------------------- */

void intercom_hangup_flag_set(void)
{
    s_hangup_flag = true;
}

bool intercom_hangup_flag_get_and_clear(void)
{
    bool v = s_hangup_flag;
    s_hangup_flag = false;
    return v;
}
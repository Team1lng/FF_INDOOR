#include "intercom.h"
#include "user_intercom.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
static lv_task_t *s_intercom_poll_task = NULL;
static lv_task_t *s_intercom_watchdog_task = NULL;
static void intercom_poll_task_cb(lv_task_t *task)
{
    static int cnt = 0;
    (void)task;

    if (++cnt % 50 == 0) {
        printf("[intercom_poll] alive\n");
    }

    /* modbusIntercomListener() 已由 uartItcTask 线程负责调用（intercom_interface.c），
     * 此处不再重复调用，避免并发导致 MessageSend 被执行两次。*/
    // modbusIntercomListener();
}
static void intercom_watchdog_cb(lv_task_t *task)
{
    (void)task;
    static intercom_state_t last_state = INTERCOM_STATE_IDLE;
    intercom_state_t now = intercom_state_get();
    if (now != last_state)
    {
        printf("[intercom_service] state %d -> %d\n", last_state, now);
        last_state = now;
    }
}
void intercom_service_start(void)
{
    if (!s_intercom_poll_task)
    {
        s_intercom_poll_task = lv_task_create(
            intercom_poll_task_cb,
            20,
            LV_TASK_PRIO_HIGH,
            NULL);
    }
    if (!s_intercom_watchdog_task)
    {
        s_intercom_watchdog_task = lv_task_create(
            intercom_watchdog_cb,
            200,
            LV_TASK_PRIO_LOW,
            NULL);
    }
    printf("[intercom_service] started\n");
}
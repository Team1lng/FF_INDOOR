/**
  *@file     main.c
  *@brief    SAT serial Intercom process User-implemented functions
  *@details  Including intercom UI display, serial port sending and receiving,
  *          millisecond and second acquisition,audio channel switching.
  *@author   MichaelHe Any question please send Email to hemaokang@foxmail.com
  *@date     2020/06/14
  *@version  v1.0
  *@copyright Copyright (c) 2020 SAT
  ******************************************************************************
  */

#include "intercom.h"
#include "intercom_timer.h"
#include "intercom.h"

#include "time.h"
#include "serial300.h"
#include "page_ITC_WAIT.h"
#include "user_time.h"
#include "raw_uart.h"
#include "user_intercom.h"
#include "lv_msg_event.h"
#include <pthread.h>
#include <errno.h>
#include <termios.h>

struct ItcTask {
    pthread_t id;
    pthread_attr_t attr;
    pthread_mutex_t mutex;
    bool running;
    int uartFd;
};

static struct ItcTask uartIntercom = {
    .id = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .running = false,
    .uartFd = 0,
};

#define UART_RX_STASH_SIZE 16
static unsigned char s_rx_stash[UART_RX_STASH_SIZE];
static unsigned char s_rx_stash_w = 0;
static unsigned char s_rx_stash_r = 0;

static void stash_clear(void)
{
    s_rx_stash_w = 0;
    s_rx_stash_r = 0;
}

static void stash_push(unsigned char c)
{
    unsigned char next = (unsigned char)((s_rx_stash_w + 1) % UART_RX_STASH_SIZE);
    if (next == s_rx_stash_r) {
        s_rx_stash_r = (unsigned char)((s_rx_stash_r + 1) % UART_RX_STASH_SIZE);
    }
    s_rx_stash[s_rx_stash_w] = c;
    s_rx_stash_w = next;
}

static int stash_pop(unsigned char *c)
{
    if (s_rx_stash_r == s_rx_stash_w) {
        return 0;
    }
    *c = s_rx_stash[s_rx_stash_r];
    s_rx_stash_r = (unsigned char)((s_rx_stash_r + 1) % UART_RX_STASH_SIZE);
    return 1;
}

static void UartPut(const char c)
{
    // char echo;
    // int ret;

    if (uartIntercom.uartFd <= 0) {
        return;
    }

    uart_write(uartIntercom.uartFd, &c, 1);
    // usleep(500);

    // ret = uart_read(uartIntercom.uartFd, &echo, 1);
    // if (ret == 1 && (unsigned char)echo != (unsigned char)c) {
    //     stash_push((unsigned char)echo);
    // }
}

static int UartGet(void)
{
    unsigned char cached = 0;
    char c = 0;
    int ret;

    if (uartIntercom.uartFd <= 0) {
        return -1;
    }

    if (stash_pop(&cached)) {
        return cached;
    }

    ret = uart_read(uartIntercom.uartFd, &c, 1);
    if (ret <= 0) {
        return -1;
    }

    return (unsigned char)c;
}

static void UartClean(void)
{
    stash_clear();
    if (uartIntercom.uartFd > 0) {
        tcflush(uartIntercom.uartFd, TCIFLUSH);
    }
}

/**
  *@brief Call waiting user interface
  *@param None
  *@retval None
  */
void UiCallWait(void)
{
    IDLE_ACKFun();
    mbprintf(">>>Wait answer.\n");
}

/**
  *@brief Call failure user interface
  *@param None
  *@retval None
  */
void UiCallFail(void)
{
    IntercomUnAckFun();
    mbprintf(">>>Call failure.\n");
}

/**
  *@brief Call busy user interface
  *@param None
  *@retval None
  */
void UiCallBusy(void)
{
    OutWill_BusyFun();
    mbprintf(">>>Call Busy.\n");
}

/**
  *@brief Call accepted user interface
  *@param None
  *@retval None
  */
void UiCallAccept(void)
{
    TalkAccpetFun();
    mbprintf(">>>Call Accept.\n");
}

/**
  *@brief Call Hang up user interface
  *@param None
  *@retval None
  */
void UiCallHangUp(void)
{
    In_HandupFun();
    mbprintf(">>>Call hang up.\n");
}

/**
  *@brief Switch call audio channel
  *@param channel: CH0_OFF CH0_ON CH1_OFF CH1_ON
  *@retval None
  */
void SwitchAudioCh(unsigned char channel)
{
    switch (channel)
    {
        case CH_OFF:
            IntercomColseAudio();
            break;
        case CH0_ON:
            IntercomAD3Audio();
            break;
        case CH1_ON:
            IntercomAD4Audio();
            break;
        default:
            break;
    }
}

static void *uartItcTask(void *arg)
{
    struct ItcTask *task = (struct ItcTask *)arg;
    if (task == NULL) {
        goto exitUartTask;
    }

    uartIntercom.uartFd = uart_open("ttySAK2", 9600, 8, 1, 'n');
    printf("[intercom] uart_open fd=%d\n", uartIntercom.uartFd);
    if (uartIntercom.uartFd < 0) {
        fprintf(stderr, "[intercom] Open uart fail\n");
        goto exitUartTask;
    }

    UartClean();

    while (task->running) {
        modbusIntercomListener();
        usleep(1000);
    }

exitUartTask:
    if (task != NULL && uartIntercom.uartFd > 0) {
        printf("LIU------------->>>>uart_close\n");
        uart_close(uartIntercom.uartFd);
        uartIntercom.uartFd = 0;
    }
    pthread_exit(NULL);
}

/**
  *@brief Initilaze modbus Intercom base interface
  *@param None
  *@retval None
  */
void mbIntercomInterfaceInit(void)
{
    printf("[INTERCOM] 接口注册初始化成功！！\n");
    InitIntercomBaseFunc(
        UartPut,
        UartGet,
        UiCallWait,
        UiCallFail,
        UiCallBusy,
        UiCallAccept,
        UiCallHangUp,
        IntercomSwitchAudioChannel,
        IntercomGetTimerSec,
        IntercomGetTimerMsec,
        UartClean
    );
}

int IntercomUartTaskStart(void)
{
    if (uartIntercom.running == false) {
        uartIntercom.running = true;
        pthread_attr_init(&uartIntercom.attr);
        if (pthread_create(&uartIntercom.id, &uartIntercom.attr, 
            uartItcTask, &uartIntercom)) 
            {
                uartIntercom.running = false;
                perror("[intercom] Create uart thread fail");
                return EXIT_FAILURE;
            }
    }
    return EXIT_SUCCESS;
}

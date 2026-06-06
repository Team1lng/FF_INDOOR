/**
  *@file     intercom_timer.c
  *@brief    SAT serial Intercom process timer functions
  *@details  Handshake timeout, delayed transmission, timeout retransmission,
  *			 and talk time control.
  *@author   MichaelHe Any question please send Email to hemaokang@foxmail.com
  *@date     2020/06/14
  *@version  v1.0
  *@copyright Copyright (c) 2020 SAT
  ******************************************************************************
  */

#ifndef _INTERCOM_TIMER_H_
#define _INTERCOM_TIMER_H_
#include "intercom.h"

typedef unsigned  long(*CPU_TIMER)(void);
typedef void (*timerCallback)(void *argv);

typedef struct
{
    BOOL enable;
    unsigned  long startTime;
    unsigned  long tCount;
    unsigned  long delay;
    timerCallback timerHandle;
    void *argv;
} MB_Timer;

typedef void (*InitFunc)(void);
typedef BOOL (*StartFunc)(unsigned char, unsigned long, timerCallback, void *);
typedef BOOL (*StopFunc)(unsigned char);
typedef void (*StopAllFunc)(void);
typedef BOOL (*CheakFunc)(unsigned char);

typedef struct _MBTimer
{
    InitFunc init;
    StartFunc start;
    StopFunc stop;
    StopAllFunc stopAll;
    CheakFunc check;
}MODBUSTimer;

enum
{
    msTIMER_0 = 0,
    msTIMER_1 = 1,
    msTIMER_2 = 2,
    sTIMER_0  = 3,
    sTIMER_1  = 4,
    sTIMER_2  = 5,
    MB_TIMER_NUMBER = 6///< Please do not add a timer here
};
	
extern MODBUSTimer modbusTimer;

/*Timer initialization and monitoring*/
BOOL PortTimerInit(void);
void mbTimerListener(void);
/*Random delay of message sending*/
void BusDelay(void);
void CheckRetransDelay(void);
/*Timeout retransmission timer*/
void EnableTimeoutRetrans(void);
 void StartTimeoutRetransTimer(unsigned char client, unsigned char message);
 void StopTimeoutRetransTimer(void);
/*Instruction random delay sending timer*/
 void StartRDelayInstruction(unsigned char client, unsigned char message);
 void StopRDelayInstruction(void);
/*Handshake timeout timer*/
 void StartConnectWaitTimer(void);
 void StopConnectWaitTimer(void);
/*Connection hold timeout timer*/
 void StartConnectHoldTimer(void);
 void StopConnectHoldTimer(void);
/*Talking hold timeout Timer*/
 void StartTalkTimer(void);
 void StopTalkTimer(void);
void StopCallRequestTimer(void);


#endif // _INTERCOM_TIMER_H_


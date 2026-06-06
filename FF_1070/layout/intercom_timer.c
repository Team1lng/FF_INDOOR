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

#include "intercom.h"
#include "intercom_timer.h"



CPU_TIMER GetMsecond;
CPU_TIMER GetSecond;


static BOOL TimerInitalizeFlag = FALSE;

static MB_Timer mbTimer[MB_TIMER_NUMBER];

static  unsigned char dsClientId = 0; 				///< Delay send message client id
static  unsigned char dsMessage  = IDLE_WAITING;	///< Delay send message

static unsigned char RetransClientId = 0; 			///< Timeout retransmission client id.
static unsigned char RetransMessage = IDLE_WAITING; ///< Timeout retransmission message.
static unsigned char RetransEnableCnt = 0; 			///< Timeout retransmission enable flag.

extern unsigned char NativeId;
extern BOOL UiIsNormal;

/**
  *@brief Register reading system millisecond function
  *@param msFunc:millisecond function
  *@retval None
  */
  
void RegisterReadMsecondFunc(CPU_TIMER msFunc)
{
    GetMsecond = msFunc;
}

/**
  *@brief Register reading system  seconds  function
  *@param sFunc: seconds  function
  *@retval None
  */
void RegisterReadSecondFunc(CPU_TIMER sFunc)
{
    GetSecond = sFunc;
}

/**
  *@brief Get millisecond
  *@param None
  *@retval millisecond
  */
unsigned  long GetCPUms(void)
{
    return GetMsecond();
}

/**
  *@brief Get seconds
  *@param None
  *@retval seconds
  */
unsigned  long GetCPUs(void)
{
    return GetSecond();
}

/**
  *@brief Timer initialization
  *@param None
  *@retval None
  */
static void mbTimerInit(void)
{
    int i = 0;
    for (i = 0 ; i < MB_TIMER_NUMBER ; i++)
    {
        mbTimer[i].enable = FALSE;
        mbTimer[i].delay = 0;
        mbTimer[i].startTime = 0;
        mbTimer[i].timerHandle = NULL;
    }
    TimerInitalizeFlag = TRUE;
    RegisterReadMsecondFunc(Intercom.ReadmSecond);
    RegisterReadSecondFunc(Intercom.ReadSecond);
}
/**
  *@brief Timer start
  *@param id,delay,callback,param
  *@retval state
  */
static BOOL mbTimerStart(unsigned char id, unsigned long Delay, timerCallback callback, void *argv)
{
    printf("[mbTimerStart]: id %d, delay %d, callback %p, argv %p\n\r", id, Delay, callback, argv);
    if (TimerInitalizeFlag == FALSE)
    {
        printf("timer not init \n\r");
        return FALSE;
    }
    if (id >= MB_TIMER_NUMBER)
    {
        printf("timerStart fail : id %d >= max %d\n\r", id, MB_TIMER_NUMBER);
        return FALSE;
    }
    if (Delay == 0)
    {
        printf("start timer fail delay == 0\n\r");
        return FALSE;
    }
    if (callback == NULL)
    {
        printf("start timer fail callback == NULL \n\r");
        return FALSE;
    }
    if (id == msTIMER_0 || id == msTIMER_1 || id == msTIMER_2)
    {
        mbTimer[id].startTime = GetCPUms();
    }
    else
    {
        mbTimer[id].startTime = GetCPUs();
    }

    mbTimer[id].enable = TRUE;
    mbTimer[id].delay = Delay;
    mbTimer[id].tCount = mbTimer[id].startTime + mbTimer[id].delay;
    mbTimer[id].timerHandle = callback;
    mbTimer[id].argv = argv;
    return TRUE;
}
/**
  *@brief Timer stop
  *@param id
  *@retval state
  */
static BOOL mbTimerStop(unsigned char id)
{
    if (TimerInitalizeFlag == FALSE)
    {
        //printf("timer not init \n\r");
        return FALSE;
    }
    if (id >= MB_TIMER_NUMBER)
    {
        //printf("timerStart fail : id %d >= max %d\n\r", id, MB_TIMER_NUMBER);
        return FALSE;
    }
    mbTimer[id].enable = FALSE;
    mbTimer[id].delay = 0;
    mbTimer[id].startTime = 0;
    mbTimer[id].argv = NULL;
    mbTimer[id].timerHandle = NULL;
    return TRUE;
}

/**
  *@brief Timer stop all
  *@param None
  *@retval None
  */
static void mbTimerStopAll(void)
{
    int i;
    for (i = 0; i < MB_TIMER_NUMBER; i++)
    {
        mbTimerStop(i);
    }
}

/**
  *@brief Timer check
  *@param id
  *@retval None
  */
static BOOL mbTimerCheck(unsigned char TimeId)
{
    BOOL reslut = FALSE;
    int DelayCnt = 0;

    if (TimerInitalizeFlag == FALSE)
    {
        //printf("timer not init \n\r");
        return reslut;
    }


    if (mbTimer[TimeId].enable == TRUE)
    {
        if (TimeId == msTIMER_0 || TimeId == msTIMER_1  || TimeId == msTIMER_2)
        {
            DelayCnt = (int)(GetCPUms() - mbTimer[TimeId].tCount);
            if (DelayCnt <= 0)
            {
                return reslut;
            }
        }
        else
        {
            DelayCnt = (int)(GetCPUs() - mbTimer[TimeId].tCount);
            if (DelayCnt <= 0)
            {
                return reslut;
            }
        }

        /* 先保存回调和参数，再 stop，防止 one-shot 定时器被重复触发 */
        timerCallback cb = mbTimer[TimeId].timerHandle;
        void *argv = mbTimer[TimeId].argv;
        mbTimerStop(TimeId);
        if (cb)
        {
            cb(argv);
        }
        reslut = TRUE;
    }
    return reslut;
}

/**
  *@brief Timer structure
  *@param None
  *@retval None
  */
MODBUSTimer modbusTimer =
{
    mbTimerInit,
    mbTimerStart,
    mbTimerStop,
    mbTimerStopAll,
    mbTimerCheck
};

/**
  *@brief Timer monitoring
  *@param None
  *@retval None
  */
void mbTimerListener(void)
{
    unsigned char cnt;

    for (cnt = 0; cnt < MB_TIMER_NUMBER; cnt++)
    {
        modbusTimer.check(cnt);
    }
}

/**
  *@brief Timer initialization
  *@param None
  *@retval None
  */
BOOL PortTimerInit(void)
{
    modbusTimer.init();
    return TRUE;
}

/**
  *@brief General request random delay
  *@param None
  *@retval None
  */
void BusDelay(void)
{
    srand(user_timestamp_get());
    unsigned int Delay = (rand() % 50) + 1;///< max 816ms
	
    usleep(_msgCYCLE * Delay*1000);
}

/**
  *@brief Message check retransmisson random delay
  *@param None
  *@retval None
  */
void CheckRetransDelay(void)
{
	srand(user_timestamp_get());
    unsigned int Delay = (rand() % 2);
	
    usleep(_msgCYCLE * Delay*2*1000);///< max:1616ms
}


void StartCallRequestTimer(void)
{
	srand(user_timestamp_get());
    unsigned int Delay = (rand() % 20) + 1;

    printf("[timer] start call request timer, delay=%u\n", Delay);

	///< max:1616ms
	modbusTimer.start(msTIMER_2, (_msgCYCLE * Delay*4), (timerCallback)CallRequestPrcess, NULL);
}

void StopCallRequestTimer(void)
{
	modbusTimer.stop(msTIMER_2);
}

/**
  *@brief Send command timeout, resend
  *@param None
  *@retval None
  */
void TimeoutRetrans(void)
{
	if(RetransMessage != IDLE_WAITING)
	{
	    MessageSend(NativeId, RetransClientId, RetransMessage,(RETRANC_NUMBER - RetransEnableCnt));
		if(RetransEnableCnt)
		{
			modbusTimer.start(msTIMER_0, RETRANS_WAIT_MS, (timerCallback)TimeoutRetrans, NULL);
			RetransEnableCnt--;
		}
		else
		{
			RetransMessage = IDLE_WAITING;
		}
	}
}

/**
  *@brief Enable timeout resend
  *@param None
  *@retval None
  */
void EnableTimeoutRetrans(void)
{
	RetransEnableCnt = RETRANC_NUMBER;
}

/**
  *@brief Start timeout retransmission timer
  *@param client:Receiver message:Instruction
  *@retval None
  */
 void StartTimeoutRetransTimer(unsigned char client, unsigned char message)
{
	if(RetransEnableCnt)
	{
		RetransClientId = client;
		RetransMessage = message;
		modbusTimer.start(msTIMER_0, RETRANS_WAIT_MS, (timerCallback)TimeoutRetrans, NULL);
		RetransEnableCnt--;
	}
}

/**
  *@brief Stop timeout retransmission timer
  *@param None
  *@retval None
  */
 void StopTimeoutRetransTimer(void)
{
	RetransEnableCnt = 0;
	RetransClientId = 0;
	RetransMessage = IDLE_WAITING;
	modbusTimer.stop(msTIMER_0);
}

/**
  *@brief Avoid sending command conflicts and delay sending
  *@param None
  *@retval None
  */
void DelayExecute(void)
{
	if(dsMessage != IDLE_WAITING)
	{
    printf("------------------>>>>>>>>>>>>>>>>>DelayExecute ============>>>>>>>>>>>>>>dsMessage:%d\n", dsMessage);
	    MessageSend(NativeId, dsClientId, dsMessage, 0);
		dsMessage = IDLE_WAITING;
		StopRDelayInstruction();	
	}
}

/**
  *@brief Start delay send timer
  *@param client:Receiver message:Instruction
  *@retval None
  */
 void StartRDelayInstruction( unsigned char client, unsigned char message)
{
    srand(user_timestamp_get());
    unsigned int Delay = (rand() % 50 + 1) * _msgCYCLE * 1 + 100; ///< max :916ms 50*1
	dsClientId = client;
	dsMessage  = message;
    printf("[intercom_timer] StartRDelayInstruction: client=%u msg=0x%02X delay=%u\n",
           (unsigned)client, (unsigned)message, (unsigned)Delay);
    modbusTimer.start(msTIMER_1, Delay, (timerCallback)DelayExecute, NULL);
}

/**
  *@brief Stop delay send timer
  *@param None
  *@retval None
  */
 void StopRDelayInstruction(void)
{
	dsClientId = 0;
	dsMessage = IDLE_WAITING;
    modbusTimer.stop(msTIMER_1);
}

/**
  *@brief Start handshake timeout timer
  *@param None
  *@retval None
  */
 void StartConnectWaitTimer(void)
{
    modbusTimer.start(sTIMER_0, CALL_CONNECT_TIME_S, (timerCallback)CallFail, NULL);
}

/**
  *@brief Stop handshake timeout timer
  *@param None
  *@retval None
  */
 void StopConnectWaitTimer(void)
{
    modbusTimer.stop(sTIMER_0);
}

/**
  *@brief Start wait answer timeout timer
  *@param None
  *@retval None
  */
 void StartConnectHoldTimer(void)
{	
	StopConnectWaitTimer();///< Successful handshake, end handshake timeout timer
    modbusTimer.start(sTIMER_1, CALL_WAIT_TIME_S, (timerCallback)CallFinish, NULL);
}

/**
  *@brief Stop wait answer timeout timer
  *@param None
  *@retval None
  */
 void StopConnectHoldTimer(void)
{
    modbusTimer.stop(sTIMER_1);
}

/**
  *@brief Start talking timeout timer
  *@param None
  *@retval None
  */
 void StartTalkTimer(void)
{
	StopConnectHoldTimer();///< Start talking, end connection and wait for timeout
	modbusTimer.start(sTIMER_2, CALL_TALK_TIME_S, (timerCallback)CallFinish, NULL);
}
/**
  *@brief Stop talking timeout timer
  *@param None
  *@retval None
  */
 void StopTalkTimer(void)
{
	modbusTimer.stop(sTIMER_2);
}



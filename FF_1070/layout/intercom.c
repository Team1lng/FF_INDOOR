/**
  *@file     main.c
  *@brief    SAT serial Intercom process
  *@details  It mainly includes SAT serial port intercom communication protocol,
  *          instruction coding.
  *@author   MichaelHe Any question please send Email to hemaokang@foxmail.com
  *@date     2020/06/14
  *@version  v1.0
  *@copyright Copyright (c) 2020 SAT
  ******************************************************************************
  *@attention Communication Protocol.
  *@note
  *<table>
  *<tr><th>Start byte  <th>MSG <th>Source address <th>Destination address <th>Check code <th>Stop byte
  *<tr><td>0xAA        <td>0X00 <td>0X00          <td>0X01                <td>0X01       <td>0xFF
  *</table>
  */
#include "intercom.h"
#include "intercom_timer.h"

unsigned char NativeId = 0;  	///< Native room id
static unsigned char RemoteId = 0;  	///< Remote room id

static unsigned char CalledCaller = 0;  ///< called room id  or caller room id
static unsigned char DestinationId = 0; ///< Command destination

static unsigned char Message = IDLE_WAITING;

volatile unsigned char Txmsg[_MSG];
volatile unsigned char MsgRTA;	///< Message retransmission times attribute
volatile unsigned char TempCallId = 0xff;

BOOL RequestIntercom = FALSE;
BOOL UiIsNormal = FALSE;

static mbStatus ConnectStatus = IDLE_WAITING;	///< Intercom Connect status
volatile callSIGN CallTag = CIS; 				///< Call tag

MBINTERCOM Intercom = {NULL}; 					///< Intercom interface function structure

/**
*@brief UI status normal mark
*@param None
*@return None
*/
void UiIntercomStateNormal(void)
{
    UiIsNormal = TRUE;
}

/**
*@brief Intercom related status reset.
*@param None
*@return None
*/
void ParamInit(void)
{
    UiIsNormal = FALSE;
    RequestIntercom = FALSE;
    RemoteId = 0;
    //TempCallId = 0;
    //CalledCaller = 0;
    DestinationId = 0;
    Message = IDLE_WAITING;
    ConnectStatus = IDLE_WAITING;
    CallTag = CIS;
    Intercom.SwitchAudioCh(CH_OFF);

    StopRDelayInstruction();
    StopConnectWaitTimer();
    StopConnectHoldTimer();
    StopTalkTimer();
}
/**
*@brief Intercom UI status check.
*@param None
*@return None
*/
void UiStateCheck(void)
{
    /*UI Exception timeout*/
    static unsigned long nowSecond = 0;
    unsigned char offset = 0;
    ///< 不在呼叫等待页，通话状态非空闲
    if ((!UiIsNormal) && (ConnectStatus != IDLE_WAITING))
    {
        offset = (unsigned char)(GetCPUs() - nowSecond);
        if (offset >= UI_ABNORMAL_TIMEOUT)
        {
            //Intercom.SendByte(0xAA);
            // Intercom.SendByte(offset);
            ParamInit();
        }
    }
    else
    {
        nowSecond = GetCPUs();
    }
}
/**
  *@brief Register intercom interface function structure
  *@param User function
  *@retval None
  */
void InitIntercomBaseFunc(FuncSerialSend send,
                          FuncSerialRead read,
                          FuncUI wait,
                          FuncUI fail,
                          FuncUI busy,
                          FuncUI accept,
                          FuncUI hangup,
                          FuncAudio audio,
                          FuncGetSecond second,
                          FuncGetSecond msecond,
                          FuncCleanUart cleanuart)
{

    Intercom.SendByte = send;
    Intercom.ReadByte = read;
    Intercom.Wait = wait;
    Intercom.Fail = fail;
    Intercom.Busy = busy;
    Intercom.Accept = accept;
    Intercom.HangUp = hangup;
    Intercom.SwitchAudioCh = audio;
    Intercom.ReadSecond = second;
    Intercom.ReadmSecond = msecond;
    Intercom.CleanUart = cleanuart;
    ModbusPhoneInit();

}

/**
  *@brief Serial Send message
  *@param msg:unsigned char message, length
  *@return None
  */
void SendMsg(unsigned char *msg, unsigned char length)
{
    // usleep(BYTE_DELAY * 1000);
    unsigned char cnt;
    int data[_MSG];  // + 新增数组
    for (cnt = 0; cnt < length; cnt++)
    {
        // printf("[SendMsg] %02X\n", msg[cnt]);
        Intercom.SendByte(msg[cnt]);
        if (BYTE_DELAY)
        {
            usleep(BYTE_DELAY * 1000);
        }
        // usleep(2 * BYTE_DELAY * 1000);
    }
    usleep(2 * BYTE_DELAY * 1000);
    ReadMsg(data, length); 
    /* 收包统一由 MBPoll/SerialGetMessage 处理；发后 ReadMsg 会误读或丢包，已移除 */
}

/**
*@brief Serial Read message
*@param msg:int message, length
*@return None
*/
void ReadMsg(int *msg, unsigned char length)
{
    unsigned char cnt;
    for (cnt = 0; cnt < length; cnt++)
    {
        // printf("[ReadMsg] %02X\n", msg[cnt]);
        msg[cnt] = Intercom.ReadByte();
        if (BYTE_DELAY)
        {
            usleep(BYTE_DELAY * 1000);
        }
    }
}

/**
  *@brief CRC-8
  *@param ptr, Value and Value length to be verified
  *@retval crc CRC value obtained;
  */
/*
unsigned char CRC(unsigned char *ptr, unsigned char length)
{
   unsigned char cnt;
   unsigned char crc = 0x00;

   while (length--)
   {
       crc ^= *ptr++;
       for (cnt = 8; cnt > 0; --cnt)
       {
           if (crc & 0x80)
           {
               crc = (crc << 1) ^ 0x31;
           }
           else
           {
               crc = (crc << 1);
           }
       }
   }
   return (crc);
}
*/
/**
*@brief Message stop byte, get the number of retransmissions, and check whether the message is valid.
*@param stopB:Message stop byte
*@return TRUE:Valid message
*/

BOOL CheckStopByte(unsigned char stopB)
{
    unsigned char Retrans;

    for (Retrans = 0 ; Retrans <= RETRANC_NUMBER; Retrans++)
    {
        if ((stopB - Retrans) == MSG_STOP)
        {
            MsgRTA = Retrans;
            return TRUE;
        }
        else
        {
            MsgRTA = 0;
        }
    }
    return FALSE;
}
/**
*@brief Get a complete instruction
*@param ServerId: Sender ClientId:Receiver msg:Instruction
*@return TRUE:new message
*/
BOOL SerialGetMessage(unsigned char *ServerId, unsigned char *ClientId, unsigned char *msg)
{
    unsigned char CR_ANTOR;
    int startByte = -1;
    int data[_MSG - 1];
    BOOL ValidMsg;
    int cnt;
    int retry;

    for (retry = 0; retry < 100; retry++)
    {
        startByte = Intercom.ReadByte();
        if (startByte >= 0)
        {
            break;
        }
        usleep(200);
    }

    if (startByte != MSG_START)
    {
        return FALSE;
    }

    for (cnt = 0; cnt < (_MSG - 1); cnt++)
    {
        data[cnt] = -1;
        for (retry = 0; retry < 100; retry++)
        {
            data[cnt] = Intercom.ReadByte();
            if (data[cnt] >= 0)
            {
                break;
            }
            usleep(200);
        }
        if (data[cnt] < 0)
        {
            return FALSE;
        }
    }

    printf("[SGM] frame: %02X %02X %02X %02X %02X\n",
        (unsigned char)data[0], (unsigned char)data[1],
        (unsigned char)data[2], (unsigned char)data[3],
        (unsigned char)data[4]);
    ValidMsg = CheckStopByte(data[_STOP - 1]);

    if (!ValidMsg)
    {
        printf("[SGM] STOP byte FAIL: 0x%02X\n", (unsigned char)data[_STOP - 1]);
        return FALSE;
    }

    CR_ANTOR = (unsigned char)(data[_NATIVE - 1] + data[_REMOTE - 1] + data[_CMD - 1]);
    if (CR_ANTOR != data[_CRC - 1])
    {
        printf("[SGM] CRC FAIL: calc=0x%02X got=0x%02X\n",
               CR_ANTOR, (unsigned char)data[_CRC - 1]);
        return FALSE;
    }

    *ServerId = data[_NATIVE - 1];
    *ClientId = data[_REMOTE - 1];
    *msg = data[_CMD - 1];
    return TRUE;
}

// BOOL SerialGetMessage(unsigned char *ServerId, unsigned char *ClientId, unsigned char *msg)
// {
//     int frame[6];
//     int i;

//     for (i = 0; i < 6; i++)
//     {
//         int retry = 0;
//         frame[i] = -1;
//         while (retry < 100) {
//             frame[i] = Intercom.ReadByte();
//             if (frame[i] >= 0) break;
//             usleep(200);
//             retry++;
//         }
//     }

//     printf("[SGM] frame: %02X %02X %02X %02X %02X %02X\n",
//         (unsigned char)frame[0], (unsigned char)frame[1],
//         (unsigned char)frame[2], (unsigned char)frame[3],
//         (unsigned char)frame[4], (unsigned char)frame[5]);

//     /* 校验帧头 */
//     if (frame[0] != MSG_START) {
//         return FALSE;
//     }

//     /* 校验 STOP 字节 */
//     if (!CheckStopByte(frame[_STOP])) {
//         printf("[SGM] STOP byte FAIL: 0x%02X\n", (unsigned char)frame[_STOP]);
//         return FALSE;
//     }

//     /* 校验 CRC */
//     unsigned char CR_ANTOR = (unsigned char)(frame[_NATIVE] + frame[_REMOTE] + frame[_CMD]);
//     if (CR_ANTOR != frame[_CRC]) {
//         printf("[SGM] CRC FAIL: calc=0x%02X got=0x%02X\n", CR_ANTOR, (unsigned char)frame[_CRC]);
//         return FALSE;
//     }

//     *ServerId = frame[_NATIVE];
//     *ClientId = frame[_REMOTE];
//     *msg = frame[_CMD];
//     printf("[SGM] OK: src=%02X dst=%02X cmd=%02X\n", *ServerId, *ClientId, *msg);
//     return TRUE;
// }


/**
*@brief Send a complete instruction
*@param SourceId: Sender DestId:Receiver msg:Instruction
*@return None
*/
void MessageSend(unsigned char SourceId, unsigned char DestId, unsigned char msg, unsigned char retransTimes)
{
    printf("[intercom_tx] src=%d, dest=%d, cmd=0x%02X, param=%d\n", SourceId, DestId, msg, retransTimes);
    unsigned char CR_ANTOR;
    Txmsg[_START] =  MSG_START;
    Txmsg[_NATIVE] = SourceId;
    Txmsg[_REMOTE] = DestId;
    Txmsg[_CMD] = msg;
    CR_ANTOR = (unsigned char)(Txmsg[_NATIVE] + Txmsg[_REMOTE] + Txmsg[_CMD]);
    Txmsg[_CRC] = CR_ANTOR;
    Txmsg[_STOP] = (MSG_STOP + retransTimes);

    SendMsg((unsigned char *)Txmsg, (unsigned char) _MSG);
    SendMsgProcess(Txmsg[_REMOTE], Txmsg[_CMD]);
}

/**
  *@brief Update native id
  *@param roomid: Room ID
  *@retval None
  */
void MsgUpdateNativeId(unsigned char roomid)
{
    NativeId = roomid;
    printf("[intercom] MsgUpdateNativeId: NativeId=%u\n", (unsigned)NativeId);
}

void CallRequestPrcess(void)
{
    printf("[intercom] CallRequestPrcess: tx handshake to id:%d (was status %d)\n", TempCallId, ConnectStatus);
    RemoteId = TempCallId;
    CalledCaller = TempCallId;
    ConnectStatus = RQ_HANDSHAKE;

    if (NativeId == 0 || RemoteId == 0)
    {
        CallTag = ACO;
        //Intercom.SwitchAudioCh(CH1_ON);
    }
    else
    {
        CallTag = OCO;
        //Intercom.SwitchAudioCh(CH0_ON);
    }
    ///< 尽量避免多台机器同时抬起听筒通话造成串口冲突;
    ///< Try to avoid the serial port conflict caused by multiple machines lifting
    ///< the handset to talk at the same time;
    EnableTimeoutRetrans();
    MessageSend(NativeId, CalledCaller, ConnectStatus, 0);
    StopCallRequestTimer();
}

/**
  *@brief Send Call request message
  *@param Called room id
  *@retval None
  */
void MsgCallRequest(unsigned char id)
{
    printf("[intercom] MsgCallRequest: schedule handshake to %u (local id=%u, status=%d)\n",
           (unsigned)id, (unsigned)NativeId, ConnectStatus);
    TempCallId = id;
    RequestIntercom = TRUE;
    StartCallRequestTimer();
    UiIntercomStateNormal();
}

/**
  *@brief Send Call Accept  message
  *@param None
  *@retval None
  */
void MsgCallAccept(void)
{
    printf("[intercom] MsgCallAccept:  CallTag:%d  ConnectStatus:%d\n", CallTag,ConnectStatus);
    if (CallTag == ACR || CallTag == OCR)
    {
        ConnectStatus = RQ_TALKING;
        
        Intercom.Accept();
		if (CallTag == OCR)
        {
            Intercom.SwitchAudioCh(CH0_ON);
        }
        else if (CallTag == ACR)
        {
            Intercom.SwitchAudioCh(CH1_ON);
        }
        ///< 尽量避免相同房号的被叫同时抬起听筒通话造成串口冲突
        ///< Try to avoid the serial number conflict caused by the callers with
        ///< the same room number raising the handset to talk at the same time
        BusDelay();

        EnableTimeoutRetrans();
        MessageSend(NativeId, CalledCaller, ConnectStatus, 0);
    }
}

/**
  *@brief Send Call end request  message
  *@param None
  *@retval None
  */
void MsgCallEnd(void)
{
    Intercom.SwitchAudioCh(CH_OFF);

    ///if (ConnectStatus != IDLE_WAITING)
    {
        ///< 防止被叫不存在时挂断重发占线
        ///< To prevent the called party from hanging up and resending the busy line
        if (ConnectStatus != RQ_HANDSHAKE)
        {
            EnableTimeoutRetrans();
        }
        Intercom.HangUp();
        TempCallId = 0xff;
        ///< 尽量避免通话的双方同时挂断通话造成串口冲突
        ///< Try to avoid the serial port conflict caused by both parties hanging up the call
        BusDelay();
        ConnectStatus = RQ_OFFLINE;
        MessageSend(NativeId, CalledCaller, ConnectStatus, 0);
        ParamInit();
    }
}

void OutDoorCallEnd(void)
{
    printf("[intercom] OutDoorCallEnd: Ending call, resetting state ConnectStatus:%d\n", ConnectStatus);
    Intercom.SwitchAudioCh(CH_OFF);
    if (ConnectStatus != RQ_HANDSHAKE)
    {
        EnableTimeoutRetrans();
    }
    TempCallId = 0xff;
    BusDelay();
    ConnectStatus = RQ_OFFLINE;
    MessageSend(NativeId, CalledCaller, ConnectStatus, 0);
    ParamInit();
}
/**
  *@brief Call failure handling.
  *@param None
  *@retval None
  */
void CallFail(void)
{
    Intercom.Fail();
    StopTimeoutRetransTimer();
    ParamInit();
    TempCallId = 0xff;
}

/**
  *@brief Call completion processing.
  *@param None
  *@retval None
  */
void CallFinish(void)
{
    Intercom.HangUp();
    TempCallId = 0xff;
    StopTimeoutRetransTimer();
    ParamInit();
}

/**
  *@brief None never run here
  *@param msg:None
  *@retval None
  */
mbStatus IdleWatingWarning(unsigned char msg)
{
    //mbprintf("Should never run here.");
    //mbprintf("\n%d-%s:0x%02x\n", __LINE__, __FUNCTION__, ConnectStatus);
    return IDLE_WAITING;
}

/**
  *@brief Call Handshake request processing
  *@param msg:Instruction
  *@retval None
  */
mbStatus RQ_IdleReceiveHandshakeRequest(unsigned char msg)
{
    if (ConnectStatus == IDLE_WAITING || MsgRTA)
    {
        ///< 空闲状态收到与本机的通话请求
        ///< Idle to receive a call request with this unit

        if (TempCallId == RemoteId)
        {
            StopCallRequestTimer();
        }

        if (NativeId == 0 || RemoteId == 0)
        {
            CallTag = ACR;
        }
        else
        {
            CallTag = OCR;
        }
        CalledCaller = RemoteId;
        ConnectStatus = RP_HANDSHAKE;
        //Intercom.SendByte(0xa1);
        printf("[intercom_rx] RQ_HANDSHAKE: NativeId=%u RemoteId=%u TempCallId=%u -> send RP_HANDSHAKE to %u\n",
               (unsigned)NativeId, (unsigned)RemoteId, (unsigned)TempCallId, (unsigned)CalledCaller);
        StartRDelayInstruction(CalledCaller, RP_HANDSHAKE);
        return RP_HANDSHAKE;
    }
    else if (DestinationId == NativeId)
    {
        ///< 忙线：本机已处于通话状态
        ///< Busy line: the machine is already in a call
        //Intercom.SendByte(0xaa);
        if (RemoteId == CalledCaller &&  ConnectStatus == RP_HANDSHAKE && (DestinationId == NativeId))
        {
            ///< 甲乙连接时，新甲打乙，接通所有主叫
            ConnectStatus = RP_HANDSHAKE;
            //Intercom.SendByte(0xa2);
            StartRDelayInstruction(CalledCaller, RP_HANDSHAKE);
        }
        else if (CalledCaller != NativeId ||
                 RequestIntercom) ///< 相同房号甲甲通话，乙拨打甲，由呼出的甲拒接
        {
            //Intercom.SendByte(0xa1);
            //msleep(5);
            StartRDelayInstruction(RemoteId, RP_BUSY_REFUSE);
        }
        return RP_BUSY_REFUSE;
    }

    return IDLE_WAITING;
}

/**
  *@brief Call busy response processing
  *@param msg:Instruction
  *@retval None
  */
mbStatus RP_ReceiveBusyRefuse(unsigned char msg)
{
    if (ConnectStatus == RQ_TALKING || ConnectStatus == RP_TALKING)
    {
        return IDLE_WAITING;
    }

    StopTimeoutRetransTimer();
    if (ConnectStatus == RQ_HANDSHAKE)
    {
        ///<忙线: 主叫失败.
        ///<Busy Line: The caller failed.
        if (RemoteId != CalledCaller)
        {
            MessageSend(NativeId, CalledCaller, RQ_OFFLINE, 0);
        }
        Intercom.Busy();
        ParamInit();
        TempCallId = 0xff;
    }
    else
    {
        ///< 忙线：被叫不再响应主叫.
        ///< Busy line: The called party no longer responds to the calling party.
        if (ConnectStatus == RP_HANDSHAKE)
        {
            StopTimeoutRetransTimer();
            Intercom.Busy();
        }
        ParamInit();
        TempCallId = 0xff;
    }
    return IDLE_WAITING;
}

/**
  *@brief Call handshake response processing
  *@param msg:Instruction
  *@retval None
  */
mbStatus RP_ReceiveHandshakeResponse(unsigned char msg)
{
    printf("[intercom] RP_ReceiveHandshakeResponse: UiIsNormal:%d, ConnectStatus:%d\n", UiIsNormal, ConnectStatus);
    printf("[intercom] RP_ReceiveHandshakeResponse: TempCallId:%d, RequestIntercom:%d\n", TempCallId, RequestIntercom);
    if (ConnectStatus == RQ_HANDSHAKE)
    {
        ///< 被叫号码应答，清除连接等待超时
        ///< The called number answers, clears the connection and waits for timeout
        StopTimeoutRetransTimer();
        StartConnectHoldTimer();
        ConnectStatus = RP_HANDSHAKE;
    }
    else if (ConnectStatus == RP_HANDSHAKE && (!UiIsNormal))
    {
        ///< 未进入等待状态进入等待状态
        StopRDelayInstruction();
        StartConnectHoldTimer();
        Intercom.Wait();
        UiIntercomStateNormal();
    }
    else if (ConnectStatus == RP_HANDSHAKE && (TempCallId == DestinationId) && RequestIntercom)
    {
        //单个甲打乙，同时乙打甲(存在多个甲乙)
        MsgCallAccept();
    }

    return IDLE_WAITING;
}

/**
  *@brief Talking request processing
  *@param msg:Instruction
  *@retval None
  */
mbStatus RQ_ReceiveRequestTalking(unsigned char msg)
{
    if (ConnectStatus == RP_HANDSHAKE || (ConnectStatus == RP_TALKING && MsgRTA))
    {
        if (CallTag == ACR || CallTag == OCR)
        {
            ///< 被叫收到相同房号的通话请求，结束等待接听（仅被叫可以发起通话请求，主叫响应该请求）
            ///< The called party receives a call request with the same room number
            ///< and ends waiting for answering (only the called party can initiate a call request,
            ///< and the caller responds to the request)
            Intercom.HangUp();
            StopTimeoutRetransTimer();
            ParamInit();
            TempCallId = 0xff;
        }
        else
        {
            ///< 主叫响应被叫的通话请求
            ///< The caller responds to the call request of the called party
            Intercom.Accept();
            if (CallTag == ACO)
            {
                Intercom.SwitchAudioCh(CH1_ON);
            }
            else if (CallTag == OCO)
            {
                Intercom.SwitchAudioCh(CH0_ON);
            }
            ConnectStatus = RP_TALKING;
            StartTalkTimer();
            MessageSend(NativeId, CalledCaller, RP_TALKING, 0);
            return RP_TALKING;
        }
    }
    return IDLE_WAITING;
}

/**
  *@brief Talking response processing
  *@param msg:Instruction
  *@retval None
  */
mbStatus RP_ReceiveResponseTalking(unsigned char msg)
{
    if (ConnectStatus == RQ_TALKING)
    {
        ///< 被叫收到主叫的通话请求响应被叫
        ///< The called party responds to the called party by receiving
        ///< the call request from the calling party
        StopTimeoutRetransTimer();
        StartTalkTimer();
        ConnectStatus = RP_TALKING;
    }
    return IDLE_WAITING;
}

/**
  *@brief Offline request processing
  *@param msg:Instruction
  *@retval None
  */
mbStatus RQ_ReceiveOfflineRequest(unsigned char msg)
{
    if (ConnectStatus != IDLE_WAITING || MsgRTA)
    {
        Intercom.HangUp();
        TempCallId = 0xff;

        if (RemoteId == NativeId)
        {
            ///< 被叫号码收到与本机相同号码的挂断请求
            ///< The called number received a hangup request
            ///< with the same number as the local
            StopTimeoutRetransTimer();
            //ParamInit();
            ///< 多台相同房号机器同时通信，无法分别消息来源 anyway
            StartRDelayInstruction(CalledCaller, RP_OFFLINE);
        }
        else
        {
            ///< 响应挂断请求
            ///< respond to hang up request
            ConnectStatus = RP_OFFLINE;
            StopTimeoutRetransTimer();
            StartRDelayInstruction(CalledCaller, RP_OFFLINE);
            return RP_OFFLINE;
        }
    }
    return IDLE_WAITING;
}

/**
  *@brief Offline response processing
  *@param msg:Instruction
  *@retval None
  */
mbStatus RP_ReceiveOfflineResponse(unsigned char msg)
{
    StopTimeoutRetransTimer();
    if (RemoteId == NativeId)
    {
        ///< 被叫收到相同房号的挂断响应，离线
        ///< The called party receives a hangup response with the same room number, offline
        StopRDelayInstruction();
        if (CallTag == ACR || CallTag == OCR)
        {
            Intercom.HangUp();
        }
        ParamInit();
    }
    else
    {
        ///< 主叫收到离线响应
        ///< caller receives offline response
        ParamInit();
    }
    TempCallId = 0xff;
    return IDLE_WAITING;
}

/* An array of Modbus functions handlers which associates Modbus function
 * codes with implementing functions.
 */
static xMBFunctionHandler xFuncHandlers[MB_FUNC_HANDLERS_MAX] =
{
    {IDLE_WAITING,		 IdleWatingWarning},			 ///<0x00
    {RQ_HANDSHAKE,		 RQ_IdleReceiveHandshakeRequest},///<0x01
    {RP_HANDSHAKE,		 RP_ReceiveHandshakeResponse},	 ///<0x02
    {RQ_TALKING, 		 RQ_ReceiveRequestTalking},		 ///<0x03
    {RP_TALKING, 		 RP_ReceiveResponseTalking},	 ///<0x04
    {RQ_OFFLINE, 		 RQ_ReceiveOfflineRequest},		 ///<0x05
    {RP_OFFLINE, 		 RP_ReceiveOfflineResponse}, 	 ///<0x06
    {RP_BUSY_REFUSE,	 RP_ReceiveBusyRefuse}			 ///<0x07
};

/**
  *@brief Instruction processing function registration
  *@param ucFunctionCode:Instruction pxHandler:Instruction process
  *@retval None
  */
#if 0
MBErrorCode MBRegisterCB(unsigned char ucFunctionCode, pxMBFunctionHandler pxHandler)
{
    int i;
    MBErrorCode eStatus;

    if ((0 < ucFunctionCode) && (ucFunctionCode <= 127))
    {
        if (pxHandler != NULL)
        {
            for (i = 0; i < MB_FUNC_HANDLERS_MAX; i++)
            {
                if ((xFuncHandlers[i].pxHandler == NULL) || (xFuncHandlers[i].pxHandler == pxHandler))
                {
                    xFuncHandlers[i].ucFunctionCode = ucFunctionCode;
                    xFuncHandlers[i].pxHandler = pxHandler;
                    break;
                }
            }
            eStatus = (i != MB_FUNC_HANDLERS_MAX) ? MB_ENOERR : MB_EMSG;
        }
        else
        {
            for (i = 0; i < MB_FUNC_HANDLERS_MAX; i++)
            {
                if (xFuncHandlers[i].ucFunctionCode == ucFunctionCode)
                {
                    xFuncHandlers[i].ucFunctionCode = 0;
                    xFuncHandlers[i].pxHandler = NULL;
                    break;
                }
            }
            /* Remove can't fail. */
            eStatus = MB_ENOERR;
        }
    }
    else
    {
        eStatus = MB_EMSG;
    }
    return eStatus;
}
#endif
/**
  *@brief Instruction processing
  *@param request:Instruction
  *@retval None
  */
unsigned char RequestProcess(unsigned char request)
{
    unsigned char i;
    unsigned char Respone = IDLE_WAITING;
    for (i = RQ_HANDSHAKE; i < MB_FUNC_HANDLERS_MAX; i++)
    {
        /* No more function handlers registered. Abort. */
        if (xFuncHandlers[i].ucFunctionCode == IDLE_WAITING)
        {
            break;
        }
        else if (xFuncHandlers[i].ucFunctionCode == request)
        {
            Respone = xFuncHandlers[i].pxHandler(request);
            break;
        }
    }
    return Respone;
}

/**
  *@brief Command sending completion processing function
  *@param Clinet:Receiver msg:Instruction
  *@retval None
  */
void SendMsgProcess(unsigned char client, unsigned char msg)
{
    printf("[SendMsgProcess] client:%d  msg:%02X\n", client, msg);
    switch (msg)
    {
        case RQ_HANDSHAKE:
        {
            ///< 请求通话等待
            ///< request call waiting
            StartTimeoutRetransTimer(client, msg);
            StartConnectWaitTimer();
        }
        break;
        case RP_HANDSHAKE:
            if (!UiIsNormal)
            {
                ///< 应答通话，等待接听
                ///< 未进入等待状态进入等待状态
                ///< answer the call, wait for answer
                StartConnectHoldTimer();
                Intercom.Wait();
                UiIntercomStateNormal();
            }
            else
            {
                ///< Intercom.SendByte(0xaa);
                ///< Intercom.SendByte(TempCallId);
                ///< Intercom.SendByte(RemoteId);
                ///< Intercom.SendByte(RequestIntercom);
                if ((TempCallId == RemoteId) && RequestIntercom)
                {
                    //单个甲打乙，同时乙打甲
                    MsgCallAccept();
                }
            }
            break;
        case RQ_TALKING:
        {
            ///< 发送接听信号后,等待通话超时挂机
            ///< After sending the answer signal,
            ///< wait for the call to time out and hang up
            StartTimeoutRetransTimer(client, msg);
            StartTalkTimer(); ///< maybe need this
        }
        break;
        case RP_TALKING:
        {
            ///< 接收到接听信号后,等待通话超时挂机
            ///< After receiving the answer signal,
            ///< wait for the call to time out and hang up
            StartTalkTimer();
        }
        break;
        case RQ_OFFLINE:
        {
            ///< 请求挂断，超时后重发请求
            ///< Request hangs up, re-send request after timeout
            StartTimeoutRetransTimer(client, msg);
            ParamInit();
        }
        break;
        case RP_OFFLINE:
        {
            ///< 已经响应离线请求，离线
            ///< has responded to the offline request, offline
            ParamInit();
            StopCallRequestTimer();
            StopTimeoutRetransTimer();
        }
        break;
        case RP_BUSY_REFUSE:
            ///< None,maybe neend this
            ///< 甲呼叫乙 新甲同时呼叫丙
            if (RemoteId == NativeId && (ConnectStatus == RQ_HANDSHAKE || ConnectStatus == RP_HANDSHAKE) &&
                    (DestinationId != CalledCaller))
            {
                ///< 相同房号打同一被叫未接听时，所有机器忙线（忙线接通2选一）
                Intercom.Busy();
                ParamInit();
                TempCallId = 0xff;
            }
            break;
        default:
            break;
    }
}

/**
  *@brief Conflict instruction processing
  *@param message:Instruction
  *@retval None
  */
void OtherMessageProcess(unsigned char message)
{
    switch (message)
    {
        case RQ_HANDSHAKE:
            if ((CallTag == OCO) && (RemoteId != 0 && DestinationId != 0) && DestinationId != CalledCaller)
            {
                ///< 通话发起方收到发送到其它机器的一般通话握手请求
                ///< The call originator receives a general call handshake request sent to other machines
                ///< 忙线：已存在一组普通通话
                ///< Busy line: a group of ordinary calls already exists
                //StartRDelayInstruction(RemoteId, RP_BUSY_REFUSE);

                if (DestinationId != NativeId || RequestIntercom) ///< 相同房号甲甲通话，乙拨打甲，由呼出的甲拒接
                {
                    //Intercom.SendByte(0xa2);
                    //msleep(5);
                    MessageSend(NativeId, RemoteId, RP_BUSY_REFUSE, 0);
                }
                if (RemoteId == TempCallId)
                {
                    ///< 被叫正在呼出
                    Intercom.Busy();
                    StopTimeoutRetransTimer();
                    ParamInit();
                    TempCallId = 0xff;
                }
            }
            break;
        case RP_HANDSHAKE:
            if ((CallTag == ACR || CallTag == OCR) && (RemoteId == NativeId))
            {
                ///< 收到相同房号的握手响应
                RequestProcess(message);
            }
            break;
        case RQ_TALKING:
            if (RemoteId == NativeId)
            {
                ///< 被叫收到相同房号的通话请求
                ///< The called party receives a call request with the same room number
                RequestProcess(message);
            }
            break;
        case RQ_OFFLINE:
            if ((CallTag == ACR || CallTag == OCR) && (RemoteId == NativeId))
            {
                ///< 被叫收到离线请求
                ///< The called party received an offline request
                RequestProcess(message);
            }
            else if ((CallTag == ACO || CallTag == OCO) && DestinationId == CalledCaller)
            {
                ///< 本机的被叫收到挂断信号
                ///< The called party of this machine receives the hang up signal
                Intercom.Busy();
                ParamInit();
                StopTimeoutRetransTimer();
                TempCallId = 0xff;
            }
            break;
        case RP_OFFLINE:
            if ((CallTag == ACR || CallTag == OCR) && (RemoteId == NativeId))
            {
                ///< 收到相同房号的离线响应
                RequestProcess(message);
            }
            break;
        case RP_BUSY_REFUSE:
            if (DestinationId == CalledCaller)
            {
                ///< 未接通的被叫收到忙线信息
                ///< The called party who is not connected receives the busy message
                RequestProcess(message);
            }
            break;
        default:
            break;
    }
}

/**
  *@brief Intercom enable
  *@param None
  *@retval None
  */
MBErrorCode MBEnable(void)
{
    MBErrorCode eStatus = MB_ENOERR;
    if (eMBState != STATE_ENABLED)
    {
        Intercom.CleanUart();
        eMBState = STATE_ENABLED;
    }
    return eStatus;
}

/**
  *@brief Intercom disable
  *@param None
  *@retval None
  */
MBErrorCode MBDisable(void)
{
    MBErrorCode    eStatus;

    if (eMBState == STATE_ENABLED)
    {
        modbusTimer.init();
        ParamInit();
        eMBState = STATE_DISABLED;
        eStatus = MB_ENOERR;
    }
    return eStatus;
}

/**
  *@brief Intercom initialization
  *@param None
  *@retval None
  */
void ModbusPhoneInit(void)
{
    PortTimerInit();
    MBEnable();
}

/**
  *@brief Intercom processing entrance
  *@param None
  *@retval None
  */
MBErrorCode MBPoll(void)
{
    static unsigned char   ServerId;
    static unsigned char   ClientId;
    static unsigned char   RxMesssage;

    MBErrorCode eStatus = MB_ENOERR;

    /* Check if the protocol stack is ready. */
    if (eMBState != STATE_ENABLED)
    {
        return MB_EILLSTATE;
    }

    /*获取串口收到的消息*/
    BOOL msgResult = SerialGetMessage(&ServerId, &ClientId, &RxMesssage);
    // printf("[intercom_rx] SerialGetMessage returned %s\n", msgResult ? "TRUE" : "FALSE");
    if (msgResult)
    {
        RemoteId = ServerId;
        DestinationId = ClientId;

        /* 只在握手请求包上打关键接收日志，方便定位对端为什么不回 RP_HANDSHAKE */
        if (RxMesssage == RQ_HANDSHAKE)
        {
            printf("[intercom_rx] MBPoll: got RQ_HANDSHAKE src=%u dest=%u localNativeId=%u -> %s\n",
                   (unsigned)RemoteId, (unsigned)DestinationId, (unsigned)NativeId,
                   (NativeId == DestinationId) ? "RequestProcess" : "OtherMessageProcess");
        }

        if (NativeId == DestinationId)
        {
            ///< 发送到本机的指令
            ///< command sent to this machine
            RequestProcess(RxMesssage);
        }
        else
        {
            ///< 可能冲突的指令
            ///< instructions that may conflict
            OtherMessageProcess(RxMesssage);
        }
    }

    mbTimerListener();
    UiStateCheck();
    return eStatus;
}

/**
  *@brief Intercom event monitoring
  *@param None
  *@retval None
  */
int modbusIntercomListener(void)
{
    MBErrorCode retval;
    retval = MBPoll();
    return retval;
}

/**
  *@brief  Get Intercom state
  *@param None
  *@retval state
  */
unsigned char GetMbIntercomStatus(void)
{
    return ConnectStatus;
}

/**
  *@brief  Get Intercom Caller or Called
  *@param None
  *@retval Caller or called
  */
unsigned char GetCalledCallerNumber(void)
{
    return CalledCaller;
}

/**
 * @brief Get current intercom call role tag.
 *  - Used by UI layer to decide "主叫/被叫" without relying on page-local flag.
 */
callSIGN GetIntercomCallTag(void)
{
    return CallTag;
}


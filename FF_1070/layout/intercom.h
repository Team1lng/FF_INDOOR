/**
  *@file     intercom.h
  *@brief    OMID单线串口指令对讲的一些函数和状态
  *@author   MichaelHe Any question please send Email to hemaokang@foxmail.com
  *@date     2020/06/14
  *@version  v1.0
  *@copyright Copyright (c) 2020 SAT
  ******************************************************************************
  *@attention 发送指令可能产生的冲突，需要随机延时
  *			  A.请求
  *				1.握手请求：多台机器同时抬起听筒发起握手，此时会产生串口冲突
  *				2.接听请求：相同房号的被叫同时抬起听筒接听，此时会产生串口冲突
  *				3.挂断请求：1）通话双方同时挂断；2）相同房号的被叫在通话等待时，
  *							被其它事件中断通话。
  *			  B.应答
  *				1.握手应答：1）相同房号的被叫同时响应握手请求。2）被叫的应答与
  *							其它通话中的机器发送给主叫的忙线拒接冲突。（收到握
  *							手请求后等待一段时间，主叫未被忙线拒接后响应）
  *				2.接听应答：相同房号的被叫收到其中一台的接听响应，立即进入空闲，
  *				  		    主叫仅响应一次，无冲突。
  *				3.挂断应答：通话等待中，主叫挂断，相同房号的被叫同时应答（相同
  *				         	房号的被叫收到其中一台的挂断响应，立即进入空闲）
  *				4.忙线应答：同时仅通话的发起方或被叫本机应答，当存在多个相同房
  *  						号的被叫本机时，存在同时应答。
  *				页面异常超时：INTERCOM 非空闲状态2S未进入
  * @attention The conflict that may occur when sending instructions requires a random delay
  *			   A. Request
  *				1. Handshake request: multiple machines simultaneously raise the handset to 
  *				   initiate a handshake, at this time a serial port conflict will occur
  *				2. Answering request: The callee with the same room number raises the handset
  *				   to answer at the same time, and serial port conflict will occur at this time
  *				3. Hang-up request: 1) Both parties of the call hang up at the same time; 
  *				   2) When the callee with the same room number is waiting for the call,
  *				   The call was interrupted by other events.
  *				B. Response
  *				 1. Handshake response: 1) Callees with the same room number respond to the handshake 
  *					request at the same time. 2) The called responds with
  *				    The busy line sent to the caller by the machine in the other call refuses to conflict. 
  *					(Received grip Wait for a while after the hand request, the caller responds after 
  *					being rejected by the busy line)
  *				2. Answering answer: The callee with the same room number receives one of the answering 
  *					answers and immediately enters idle.
  *				    The caller only responds once without conflicts.
  *				3. Hang up answer: During the call waiting, the caller hangs up, and the callee with 
  *					the same room number answers at the same time (same
  *				    The called of the room number receives the hang-up response from one of them and 
  *					immediately enters idle.)
  *				4. Busy answer: only the originator or called party of the call answers at the same time,
  *					when there are multiple same rooms
  *				   There is a simultaneous answer when the called party of the number is called.
  *				   Page abnormal timeout: INTERCOM is not idle 2S did not enter
  *@par
  *<table>
  *<tr><th>Date       <th>Version <th>Author    <th>Description
  *<tr><td>2020/06/14 <td>1.0     <td>MichaelHe <td>Initial version
  *</table>
  */

#ifndef _INTERCOM_H_
#define _INTERCOM_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys.h>
#include "debug.h"
#include "time.h"
#include "unistd.h"

//#include "intercom_crc.h" ///< 582 NOT SUPPORT


/*User configuration*/
#define _msgCYCLE					 (18) 	 ///< Instruction Cycle 16ms)
#define BYTE_DELAY					 (2)	 ///< ms

#define RETRANC_NUMBER				 (3)	 ///< Number of retransmissions
#define RETRANS_WAIT_MS				 (1500)  ///< Response signal maximum delay:916ms

#define UI_ABNORMAL_TIMEOUT			 (6)	 ///< Page abnormal timeout(second)
#define CALL_CONNECT_TIME_S 		 (6)	 ///< Handshake timeout (second)
#define CALL_WAIT_TIME_S    		 (40)	 ///< Call waiting timeout(second)
#define CALL_TALK_TIME_S			 (60)	 ///< Intercom wait timeout(second)

/*Intercom Software configuration*/
#define MB_FUNC_HANDLERS_MAX         (0x0F)
#define MB_FUNC_ERROR                (0x80)
#define MB_ADDRESS_BROADCAST		 (0xFF)

#define MSG_START               	 (0XAB)  ///< message start byte
#define MSG_STOP                	 (0XE0)  ///< message stop byte

#define _HEADER                 	 (0X01)  ///< message Header length of header
#define _LENGTH                 	 (0X05)  ///< message length of MSG-stop
#define _CRCLEN                 	 (0X03)  ///< Value length to be verified

#define mbprintf(fmt, arg...)		 if(0)printf(fmt, ##arg) 

extern int rand();
extern void srand(unsigned int seed);
//extern void __mips32__ cli32(void);
//extern void __mips32__ sti32(void);


#ifndef TRUE
    #define TRUE            1
#endif

#ifndef FALSE
    #define FALSE           0
#endif

typedef unsigned char   BOOL;


typedef void(*FuncSerialSend)(const char);
typedef int(*FuncSerialRead)(void);
typedef void(*FuncUI)(void);
typedef void(*FuncAudio)(unsigned char);
typedef unsigned long (*FuncGetSecond)(void);
typedef unsigned long (*FuncGetmSecond)(void);
typedef void(*FuncCleanUart)(void);

typedef struct _MBINTERCOM
{
    FuncSerialSend SendByte;
    FuncSerialRead ReadByte;
    FuncUI Wait;
    FuncUI Fail;
    FuncUI Busy;
    FuncUI Accept;
    FuncUI HangUp;
    FuncAudio SwitchAudioCh;
    FuncGetSecond ReadSecond;
    FuncGetmSecond ReadmSecond;
	FuncCleanUart CleanUart; 
} MBINTERCOM;

extern MBINTERCOM Intercom;

void InitIntercomBaseFunc(FuncSerialSend send,
                          FuncSerialRead read,
                          FuncUI wait,
                          FuncUI fail,
                          FuncUI busy,
                          FuncUI accept,
                          FuncUI hangup,
                          FuncAudio audio,
                          FuncGetSecond second,
                          FuncGetmSecond msecond,
                          FuncCleanUart cleanuart);

enum _MessageLocal
{
    _START,
    _NATIVE,
    _REMOTE,
    _CMD,
    _CRC,
    _STOP,
    _MSG        ///< message length
};

// typedef enum _Message
// {
// 	/* Master-slave mutipoint competition communication*/
// 	IDLE_WAITING 	  = 0X00, 		///< 0x00 Idle state waitting for handshake request.
	
// 	RQ_HANDSHAKE 	  = 0X01, 		///< 0x01 Idle Request to create a connection.
// 	RP_HANDSHAKE 	  = 0X02, 		///< 0x02 Response handshake request.
// 	/*Start Master-slave peer-to-peer communication.*/
// 	RQ_TALKING 		  = 0X03,		///< 0x03 Request start talk.
// 	RP_TALKING 		  = 0X04, 		///< 0x04 Response talk request.
// 	/*Stop Master-slave peer-to-peer communication.*/
// 	RQ_OFFLINE 		  = 0X05,		///< 0x05 Requst offline.
// 	RP_OFFLINE 		  = 0X06,		///< 0x06 Send Allow offline;
	
// 	RP_BUSY_REFUSE 	  = 0X07		///< 0x07 Be notified of successful connection.
// }mbStatus;

typedef enum _Message {
    IDLE_WAITING = 0x00, // 空闲
    RQ_HANDSHAKE = 0x01, // 请求握手
    RP_HANDSHAKE = 0x02, // 响应握手
    RQ_TALKING = 0x03, // 请求通话
    RP_TALKING = 0x04, // 响应通话
    RQ_OFFLINE = 0x05, // 请求挂断
    RP_OFFLINE = 0x06, // 响应挂断
    RP_BUSY_REFUSE = 0x07 // 忙线拒接
} mbStatus;


typedef enum _CallType
{
	CIS,				///< Call idel state
	OCO,				///< Ordinary call originator
	OCR,				///< Ordinary call receiver
	ACO,				///< Admin call originator
	ACR 				///< Admin call receiver
}callSIGN;


/*! \ingroup modbus
 * \brief Errorcodes used by all function in the protocol stack.
 */
typedef enum
{
    MB_ENOERR,                  /*!< no error. */
    MB_ESERVER,                 /*!< illegal server address. */
    MB_ECLIENT,                 /*!< illegal slave address. */
    MB_EMSG,                    /*!< illegal message. */
    MB_EILLSTATE,               /*!< protocol stack in illegal state. */
    MB_ETIMEDOUT                /*!< timeout error occurred. */
} MBErrorCode;

typedef mbStatus(*pxMBFunctionHandler)(unsigned char pucFrame);

typedef struct
{
    unsigned char ucFunctionCode;
    pxMBFunctionHandler pxHandler;
} xMBFunctionHandler;

static enum
{
    STATE_DISABLED,
    STATE_ENABLED
    
} eMBState = STATE_DISABLED;

enum _audio_channel
{
    CH_OFF,
    CH0_ON,
    CH1_ON
};

/*The user does not need to care about the following functions*/

void ModbusPhoneInit(void);

/*Check and set, UI status during intercom.*/
void UiStateCheck(void);
void UiIntercomStateNormal(void);
/*Message send and receive*/
void ReadMsg(int *msg, unsigned char length);
void SendMsg(unsigned char *msg, unsigned char length);
void MessageSend(unsigned char SourceId, unsigned char DestId, unsigned char msg,unsigned char retransTimes);
void SendMsgProcess(unsigned char client,unsigned char msg);
void CallRequestPrcess(void);
/*Event handling used by timers*/
void CallFail(void);
void CallFinish(void);

/*Enable intercom function*/
MBErrorCode MBEnable(void);
/*Disable intercom function*/
MBErrorCode MBDisable(void);
/*Intercom event monitoring*/
int modbusIntercomListener(void);
/*Update the machine room number*/
void MsgUpdateNativeId(unsigned char roomid);
/*Initiate a call request*/
void MsgCallRequest(unsigned char id);
/*Send an answer request*/
void MsgCallAccept(void);
/*Send hang up request*/
void MsgCallEnd(void);
void OutDoorCallEnd(void);

/*Get current call status*/
unsigned char GetMbIntercomStatus(void);
/*Get the calling number*/
unsigned char GetCalledCallerNumber(void);
/*Get current call role tag (for UI role判定)*/
callSIGN GetIntercomCallTag(void);
#endif // _INTERCOM_H_


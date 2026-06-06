/**
  *@file     main.c
  *@brief    SAT serial Intercom process User-implemented functions
  *@details  Including intercom UI display, serial port sending and receiving,
  *			 millisecond and second acquisition,audio channel switching.
  *@author   MichaelHe Any question please send Email to hemaokang@foxmail.com
  *@date     2020/06/14
  *@version  v1.0
  *@copyright Copyright (c) 2020 SAT
  ******************************************************************************
  */

#ifndef _INTERCOM_INTERFACE_H_
#define _INTERCOM_INTERFACE_H_
// #include "intercom.h"

#define AFC_INTERCOM	(1)
void UiCallWait(void);
void UiCallFail(void);
void UiCallBusy(void);
void UiCallAccept(void);
void UiCallHangUp(void);
void SwitchAudioCh(unsigned char channel);



// /*Enable intercom function*/
// MBErrorCode MBEnable(void);
// /*Disable intercom function*/
// MBErrorCode MBDisable(void);
/*Intercom function interface initialization*/
void mbIntercomInterfaceInit(void);
// /*Intercom event monitoring*/
// int modbusIntercomListener(void);
// /*Update the machine room number*/
// void MsgUpdateNativeId(unsigned char roomid);
// /*Initiate a call request*/
// void MsgCallRequest(unsigned char id);
// /*Send an answer request*/
// void MsgCallAccept(void);
// /*Send hang up request*/
// void MsgCallEnd(void);
// void OutDoorCallEnd(void);

// /*Get current call status*/
// unsigned char GetMbIntercomStatus(void);
// /*Get the calling number*/
// unsigned char GetCalledCallerNumber(void);
#endif // _INTERCOM_INTERFACE_H_


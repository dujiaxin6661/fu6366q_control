/**************************** (C) COPYRIGHT 2017 Fortiortech Shenzhen ******************************
* File Name          : UART_Debug.h
* Author             : Any Lin
* Version            : V1.0
* Date               : 2018-02-01
* Description        :
****************************************************************************************************
* All Rights Reserved
***************************************************************************************************/

#ifndef __UART_DEBUG_H__
#define __UART_DEBUG_H__

/**************************************************************************************************///Including Header Files
/**************************************************************************************************///Define Macro
#define BUFFLEN                         128
/**************************************************************************************************///Define Global Symbols
/**************************************************************************************************///Function Subject
extern void           InitBuff_UARTDBG(unsigned char Type);
extern void           LoadBuff16_UARTDBG(unsigned short Data);
extern void           LoadBuff8_UARTDBG(unsigned char Data);
extern unsigned char  SendReady_UARTDBG(void);
extern unsigned char* GetAddr_UARTDBG(void);




#endif
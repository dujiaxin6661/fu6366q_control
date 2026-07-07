/* --------------------------- (C) COPYRIGHT 2020 Fortiortech ShenZhen -----------------------------
    File Name      : FlashWrite.h
    Author         : Fortiortech  Appliction Team
    Version        : V1.0
    Date           : 2020-10-08
    Description    : This file contains Flash Read&Write parameter used for Motor Control.
----------------------------------------------------------------------------------------------------
                                       All Rights Reserved
------------------------------------------------------------------------------------------------- */

/* Define to prevent recursive inclusion -------------------------------------------------------- */
#ifndef __FLASHWRITE_H_
#define __FLASHWRITE_H_


#define OFFSETCODEADDRESS 0X3F00

/* Exported types --------------------------------------------------------------------------------*/
typedef struct
{
    uint8 x5a;
    uint8 x1f;
}FlashSecurityKey;                           // 安全密匙

/* Exported variables ----------------------------------------------------------------------------*/
extern FlashSecurityKey data FlashKey;

/* Exported functions ----------------------------------------------------------------------------*/
extern uint16 Get2ByteFromFlash(uint8 xdata *BlockStartAddr);
extern uint32 Get4ByteFromFlash(uint8 xdata *BlockStartAddr);
extern void Write2Byte2Flash(uint16 BlockStartAddr,uint16 NewData2Flash);
extern void Write4Byte2Flash(uint16 BlockStartAddr,uint32 NewData2Flash);

extern void FlashRead(void);
extern void OffsetWrite(void);

extern void Flash_Sector_Erase(uint16 FlashAddress, uint8 Key5a, uint8 Key1f);
extern void Flash_Sector_Write(uint16 FlashAddress, uint8 FlashData, uint8 Key5a, uint8 Key1f);
#endif

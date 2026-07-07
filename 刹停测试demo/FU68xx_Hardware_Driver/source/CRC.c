/**
 * @copyright (C) COPYRIGHT 2022 Fortiortech Shenzhen
 * @file      main.c
 * @author    Fortiortech  Appliction Team
 * @since     Create:2023-07
 * @date      Last modify:2023-07
 * @note      Last modify author is Kobe Peng
 * @brief      
 */

#include <Myproject.h>
#include "CRC.h"

/****************************************************************************
CRC16_CCITT_FALSE	硬件实现
****************************************************************************/
unsigned short CRC_Check(unsigned char start_sector , unsigned char offset_sector)
{
    unsigned short crcresult = 0x0000;

    unsigned short tempH = 0x00;
    unsigned short tempL = 0x00;

    SetBit(CRC_CR , CRCVAL);		//0-->0x0000  1-->0xffff
    SetBit(CRC_CR , CRCDINI);		//1-->init success

    CRC_BEG = start_sector;	    //起始扇区
    CRC_CNT = offset_sector;	//扇区偏移量	0-->1个扇区

    //1个空扇区-->0x41E8	 2个空扇区-->0x1634
    //1个0xFF  -->0x5B2F	 0x00~0xFF-->0x3FBD
    SetBit(CRC_CR , AUTOINT);		//自动计算使能
    /***************************************************/

    SetBit(CRC_CR , CRCPNT);	
    tempH = CRC_DR;
    ClrBit(CRC_CR , CRCPNT);	
    tempL = CRC_DR;
    crcresult = (unsigned short)(tempH << 8) + tempL;
    return crcresult;
}

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
#include <DMA.h>
#include <FU68xx_2_DMA.h>
#include <FU68xx_2_MCU.h>

/* -------------------------------------------------------------------------------------------------
    Function Name  : Init_DMA
    Description    :  
*   DMA模块的配置（DMA0/1通用）
*
*   @param IEMod  DMA中断配置，可用参数如下
*                   ENIE     启用DMA中断
*                   DISIE    禁用DMA中断
*   @param ENDIAN DMA传输顺序（发送十六位数据时适用）
*                   FLSB     低8位先发（发送8位数据时必须选用该项）
*                   FMSB     高8位先发（发送8位数据时必须选用该项）
    Date           : 2020-08-07
    Parameter      : IEMod   : [输入] 
**			         FirstMod: [输入] 
------------------------------------------------------------------------------------------------- */
//void Init_DMA(uint8 IEMod, uint8 FirstMod)
//{
//    DMA0_CR0 = IEMod | FirstMod;
//}

/* -------------------------------------------------------------------------------------------------
    Function Name  : Set_DMA
    Description    : 

 *  DMA设置
 *
 *  @param   Ch         DMA通道
 *  @param   Pipe       DMA管道，可选参数如下
 *                         UART_XDATA    UART->XDATA
 *                         XDATA_UART    UART<-XDATA
 *                         I2C_XDATA     I2C ->XDATA
 *                         XDATA_I2C     I2C <-XDATA
 *                         SPI_XDATA     SPI ->XDATA
 *                         XDATA_SPI     SPI <-XDATA
 *  @param   Addr       传输首地址，可取地址范围：0x0000~0x0317
 *  @param   Len        数据包大小(1~64)
 
    Date           : 2020-08-07
    Parameter      : Ch:    [输入] 
 **       			 Pipe:  [输入] 
 **       			 Addr:  [输入] 
 **       			 Len:   [输入] 
------------------------------------------------------------------------------------------------- */

void Set_DBG_DMA(uint16 DMAAddr)
{
    ClrBit(DMA1_CR0, DMAEN);             
    SetBit(DMA1_CR0, (DBGEN | (uint8)DRAM_SPI));                   
    if (!ReadBit(DMAAddr, 0x4000)) SetBit(DMA1_CR0, DBGSW); 

    DMA1_LEN = 7;                                     
    DMA1_BA = DMAAddr & 0x07ff;                           

    Switch_DMA(1);                                                              
}


void Conf_DMA(uint8 DMAx, uint8 DMAPipe, uint16 DMAAddr, uint8 DMALen)
{
    Wait_DMA(DMAx);                                                   // 等待DMAx传输结束
    ClrBit(*(&DMA0_CR0 + DMAx), DMAEN);                               // 禁止DMAx的传输

    SetReg(*(&DMA0_CR0 + DMAx), DMA_PIPE, DMAPipe);                   // 设置DMAx的传输管道

    *(&DMA0_LEN + DMAx) = DMALen - 1;                                 // 设置DMAx传输数量
    *(&DMA0_BA + DMAx) = DMAAddr & 0x07ff;                           // 设置DMAx传输首地址
    
}

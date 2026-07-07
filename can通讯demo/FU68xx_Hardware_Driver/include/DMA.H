#ifndef __DMA_H__
#define __DMA_H__

#include "FU68xx_6_MCU.h"

/*************************************************************************************/
 #define DMA_PIPE        (DMACFG2 | DMACFG1 | DMACFG0)
 #define UART1_XDATA     0x00                                         // DMA¹ÜµÀ--UART1-->XDATA
 #define XDATA_UART1     (DMACFG0)   
 
typedef enum
{
    UART_DRAM  = 0,
    DRAM_UART  = DMACFG0,
    I2C_DRAM   = DMACFG1,
    DRAM_I2C   = DMACFG1 | DMACFG0,
    SPI_DRAM   = DMACFG2,
    DRAM_SPI   = DMACFG2 | DMACFG0,
    UART2_DRAM = DMACFG2 | DMACFG1,
    DRAM_UART2 = DMACFG2 | DMACFG1 | DMACFG0
}eType_DMA_PIPE;

typedef enum
{
    DMA_IRQ_L1 = 0,
    DMA_IRQ_L2 = 0x40,
    DMA_IRQ_L3 = 0x80,
    DMA_IRQ_L4 = 0xc0
}eType_DMA_IRQ;

typedef enum
{
    BIG_ENDIAN    = 0,
    LITTLE_ENDIAN = ENDIAN
}eType_DMA_Endian;

typedef enum
{
    DMA_BSY = DMABSY,
    DMA_IRQ = DMAIF
}eType_DMA_Statu;

/******************************************************************************///Function Subject
extern void Conf_DMA(uint8 DMAx, uint8 DMAPipe, uint16 DMAAddr, uint8 DMALen);
extern void Set_DBG_DMA(uint16 DMAAddr);

#endif


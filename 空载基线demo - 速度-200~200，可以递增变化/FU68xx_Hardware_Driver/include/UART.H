#ifndef __UART_H__
#define __UART_H__

#include <Customer_Debug.h>

/*************************************************************************************///External Function
#define TxBUFFLEN  20
#define RxBUFFLEN  12

 #ifdef UART_DBG   // 串口调试模式 500000bps  SCLK-RX

typedef struct
{
    uint8   T_DATA[TxBUFFLEN];
}TypeDef_UartANO;


typedef union 
{
  float FloatData;
  int8  ByteData[4];
}FLOATtoByet;

 
 typedef struct  
{
  FLOATtoByet Data[4];
}SENDDATA;

extern SENDDATA      xdata Send;
extern TypeDef_UartANO     xdata UART_ANO;
extern void Send_4DataFrame(void); //为了效率固定传输4个数据 40us

#endif

extern void UART1_Init(uint32 uart1bps);

#endif

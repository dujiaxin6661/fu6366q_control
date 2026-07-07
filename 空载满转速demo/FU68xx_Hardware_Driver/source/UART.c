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
#include <FU68xx_6_MCU.h>

/** 
 * @brief        Uart2初始化函数
 * @param        None
 * @return       none
 * @author       Marcel
 * @date         2022-07-13
 * @version      1.0   
 * @property     Public
*/
void UART2_Init(void)
{
    SetBit(PH_SEL , UART2EN);   //P3[6]as UART2_RXD; P3[7]as UART2_TXD

#if 0
    ClrBit(P3_OE , P36);        //输入使能
    SetBit(P3_PU , P36);        //上拉电阻
    SetBit(P3_OE , P37);        //输出使能            
    SetBit(P3_PU , P37);        //上拉电阻 	
#endif

    ClrBit(UT2_CR , UT2MOD1);	//00-->单线制8bit		01-->8bit uart(波特率可设置)
    SetBit(UT2_CR , UT2MOD0);	//10-->单线制9bit	    11-->9bit uart(波特率可设置)

    ClrBit(UT2_CR , UT2SM2);    //0-->单机通讯			1-->多机通讯；
    SetBit(UT2_CR , UT2REN);    //0-->不允许串行输入	1-->允许串行输入，软件清0;
    ClrBit(UT2_CR , UT2TB8);	//模式2/3下数据发送第9位，在多机通信中，可用于判断当前数据帧的数据是地址还是数据，TB8=0为数据，TB8=1为地址
    ClrBit(UT2_CR , UT2RB8);	//模式2/3下数据接收第9位，若SM2=0,作为停止位

    ClrBit(IP3 , PSPI_UT21);	//中断优先级时最低
    ClrBit(IP3 , PSPI_UT20);	//中断优先级时最低
//    PSPI_UT21 = 0;              //中断优先级时最低
//    PSPI_UT20 = 0;

    ClrBit(UT2_BAUD , BAUD2_SEL);//倍频使能0-->Disable  1-->Enable
//    ClrBit(UT2_BAUD , UART2CH);  //UART2端口功能转移使能0：P36->RXD P37->TXD 1:P01->RXD P00->TXD
    ClrBit(UT2_BAUD , UART2IEN); //UART2中断使能0-->Disable  1-->Enable
    UT2_BAUD = 0x000c;           //波特率可设置 = 24000000/(16/(1+ UT_BAUD[BAUD_SEL]))/(UT_BAUD+1)
                                 //9B-->9600 0x000c-->115200 0x0005-->256000
}

//void UART1_Init(void)
//{
//    SetBit(PH_SEL, UART1EN); 
//    UT_MOD1 = 0;    //00-->单线制8bit        01-->8bit uart(波特率可设置)
//    UT_MOD0 = 1;    //10-->单线制9bit        11-->9bit uart(波特率可设置)
//    SM2 = 0;        //0-->单机通讯          1-->多机通讯；
//    REN = 1;        //0-->不允许串行输入 1-->允许串行输入，软件清0;
//    TB8 = 0;        //模式2/3下数据发送第9位，在多机通信中，可用于判断当前数据帧的数据是地址还是数据，TB8=0为数据，TB8=1为地址
//    RB8 = 0;        //模式2/3下数据接收第9位，若SM2=0,作为停止位
//	
//	  ClrBit(IP3 , PI2C_UT11);      //HFI    //中断优先级时最低
//    ClrBit(IP3 , PI2C_UT10);      //HFI
//	
//    ClrBit(UT_BAUD, UART_2xBAUD);   //倍频使能0-->Disable  1-->Enable
//    ES0 = 0;                        //UART1中断使能0-->Disable  1-->Enable
//    UT_BAUD = 0x000c;//波特率可设置 = 24000000/(16/(1+ UT_BAUD[BAUD_SEL]))/(UT_BAUD+1)
//}



void UART1_Init(uint32 uart1bps)
{
    SetBit(PH_SEL , UART1EN);   //P0[6]as UART2_RXD; P0[5]as UART2_TXD

#if 0
    ClrBit(P0_OE , P06);        //输入使能
    SetBit(P0_PU , P06);        //上拉电阻
    SetBit(P0_OE , P05);        //输出使能            
    SetBit(P0_PU , P05);        //上拉电阻 	
#endif

    /* UART模式 UT_MOD1,UT_MOD0: 
    00  模式0，移位寄存器波特率为 系统时钟/12
    01  模式1，8-bit UART 波特率为 fcpu_clk / ( (16 / (1+ UT2_BAUD[BAUD2_SEL]) ) / (UT2_BAUD+1) )
    10  模式2，9-bit UART 波特率为 fcpu_clk / ( 32 – 16* UT2_BAUD[BAUD2_SEL])
    11  模式3，9-bit UART 波特率为 fcpu_clk / ( (16 / (1+ UT2_BAUD[BAUD2_SEL])) / (UT2_BAUD+1) )   */
    UT_MOD1 = 0;	
    UT_MOD0 = 1;	
    
    /* Does not allow multi-thread cpu operation */
    /* UT2_SM2 为 0 - 单机通讯
               为 1 - 多机通讯 */
    SM2 = 0;
    
    /* UT2_REN 为 0 - 禁止接收
               为 1 - 使能接收 */
    REN = 0;
    
    TB8 = 0;	                     //模式2/3下数据发送第9位，在多机通信中，可用于判断当前数据帧的数据是地址还是数据，TB8=0为数据，TB8=1为地址
    
    RB8 = 0;	                     //模式2/3下数据接收第9位，若SM2=0,作为停止位


    /* Uart1设置中断优先级 */
    ClrBit(IP3, PI2C_UT11);
    ClrBit(IP3, PI2C_UT10);
 
    UT_BAUD = 24000000/15 / uart1bps - 1;
    
//  SetBit(UT_BAUD , UART_2xBAUD);    // 倍频使能0-->Disable  1-->Enable
    ES0 = 0;                          // UART1中断使能0-->Disable  1-->Enable
}




void UART1_Init_Debuger(void)
{
    SetBit(PH_SEL , UART1EN);   // P0[6]as UART2_RXD; P0[5]as UART2_TXD
    UT_MOD1 = 0;                // 8bit波特率可变UART模式
    UT_MOD0 = 1;		
    SM2     = 0;                // 
    REN     = 1;		        // 使能接收
    ES0 = 0;                    // 发送/接受中断使能
    UT_BAUD = 0x000c;           // 9B-->9600 0x000c-->115200 0x0005-->256000 0x0006-->250000
}

//void Float2Char(float* DataTemp,uint8* P,uint8 length) //适配Vofa上位机
//{
//  unsigned char j  = 0;
//  for(j=0;j<length;j++)
//  {
//       P[j*4+3] = *((uint8 *)(DataTemp+j));
//       P[j*4+2] = *((uint8 *)(DataTemp+j)+1);
//       P[j*4+1] = *((uint8 *)(DataTemp+j)+2);
//       P[j*4] = *((uint8 *)(DataTemp+j)+3);
//  }
//}

#ifdef UART_DBG   // 串口调试模式 500000bps  SCLK-RX

SENDDATA xdata Send;
TypeDef_UartANO     xdata UART_ANO;

void Send_4DataFrame(void) //为了效率固定传输4个数据 40us
{

	UART_ANO.T_DATA[0] = Send.Data[0].ByteData[3]; //第一个数据
	UART_ANO.T_DATA[1] = Send.Data[0].ByteData[2]; //第一个数据
	UART_ANO.T_DATA[2] = Send.Data[0].ByteData[1]; //第一个数据
  UART_ANO.T_DATA[3] = Send.Data[0].ByteData[0]; //第一个数据
	
	UART_ANO.T_DATA[4] = Send.Data[1].ByteData[3]; //第二个数据
	UART_ANO.T_DATA[5] = Send.Data[1].ByteData[2]; //第二个数据
	UART_ANO.T_DATA[6] = Send.Data[1].ByteData[1]; //第二个数据
  UART_ANO.T_DATA[7] = Send.Data[1].ByteData[0]; //第二个数据
	
	UART_ANO.T_DATA[8] =  Send.Data[2].ByteData[3]; //第三个数据
	UART_ANO.T_DATA[9] =  Send.Data[2].ByteData[2]; //第三个数据
	UART_ANO.T_DATA[10] = Send.Data[2].ByteData[1]; //第三个数据
  UART_ANO.T_DATA[11] = Send.Data[2].ByteData[0]; //第三个数据
	
	UART_ANO.T_DATA[12] = Send.Data[3].ByteData[3]; //第四个数据
	UART_ANO.T_DATA[13] = Send.Data[3].ByteData[2]; //第四个数据
	UART_ANO.T_DATA[14] = Send.Data[3].ByteData[1]; //第四个数据
  UART_ANO.T_DATA[15] = Send.Data[3].ByteData[0]; //第四个数据
		
  UART_ANO.T_DATA[16] = 0x00;            //写帧尾数据
  UART_ANO.T_DATA[17] = 0x00;
  UART_ANO.T_DATA[18] = 0x80;
  UART_ANO.T_DATA[19] = 0x7f; 
}


//			  #ifdef UART_DBG   // 串口调试模式 500000bps  SCLK-RX
//        {
//				  Wait_DMA(1);
//					Send.Data[0].FloatData =  mcFocCtrl.PostionLoopOut  ;
//          Send.Data[1].FloatData =  mcFocCtrl.SpeedFlt ;
//          Send.Data[2].FloatData =  MR.HallB ;
//          Send.Data[3].FloatData =  MR.FlagCircle;
//					Send_4DataFrame();
//			    Switch_DMA(1); 					     
//        }
//        #endif
#endif
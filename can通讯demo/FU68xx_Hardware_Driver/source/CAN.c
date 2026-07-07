#include <Myproject.h>
#include <FU68xx_6_MCU.h>

uint8 xdata CANdata[8] = {5, 7, 8, 10, 1, 3, 5, 7};//
 
CanRxMsg CanRxdata = {0};
/*  ----------------------------------------------------------------------------------------------*/
/*  Function Name  : CAN_Init can外设初始化
	/*  Description    :
	/*  Date           : 2022-08-26
	/*  Parameter      : 注意屏蔽位后面尽量要去匹配，不然太多数据会加大单片机负载
	/*  ----------------------------------------------------------------------------------------------*/
void CAN_Init(void)
{
	uint16 BRP;
	uint8 SEG2;
	uint8 SEG1;

	ClrBit(CAN_CR0, CANEN);	 /* 清除总线使能 */
	SetBit(CAN_CR0, RSTMOD); /* 进入复位模式 */
	/***************************清0避免写入bug**********************************/
	CAN_BTR0 = 0;
	CAN_BTR1 = 0;
	CAN_BTR2 = 0;
	
	ClrBit(CAN_CR0, CAN_CH); /* 端口转移：0:txd→P00,rxd→P01	1:txd→P05,rxd→P06 */
	SetBit(CAN_CR0, WAKMOD); /* 自动唤醒使能*/
	
	ClrBit(CAN_CR0, WAKMOD);
	ClrBit(CAN_CR0, SELFTST); /* 回环模式 */
	ClrBit(CAN_CR0, LISTEN);  /* 监听模式 */	

	/* 功能：CAN模块波特率设置
	CAN波特率 = 24M /((BRP+1)*( SEG1+SEG2+3) )
	
	采样点=（SEG1+2）/（SEG1+SEG2+3）*100%  ----注意采样点要保持在80%以上*/
	
	BRP =  3;  /* 范围0-0x3FF		时间单元tq=(BRP+1)/24MHz  125K==15;  250K==7; 500K==3  */
	SEG2 = 1; /* 范围0-0x07			时间段2时间长度tSEG2=tq*(SEG2+1) */
	SEG1 = 8; /* 范围0-0x0E			时间段1时间长度tSEG1=tq*(SEG1+1) */

	CAN_BTR0 |= (BRP >> 8);
	CAN_BTR1 = BRP;
	CAN_BTR2 |= (SEG2 << 4) | SEG1;
 

	SetBit(CAN_IER, ETYIE | BOFIE | PERIE | ERWIE | OVIE | ARBIE | TXIE | RXIE); /* 中断开启选择 */

 			 
	/*在复位模式下，CAN控制器处于复位状态，不可以接收或发送报文。对CAN_CR0[RSTMOD]清‘0’以进入复位模式*/
	ClrBit(CAN_CR0, RSTMOD);

	SetBit(CAN_CR0, FILMOD); /* 单过滤模式 */
	
	CAN_FIR0 = 0xFF;
	CAN_FIR1 = 0xF7;//只接受标准帧
	CAN_FIR2 = 0xFF;
	CAN_FIR3 = 0xFF;

	CAN_FMR0 = 0xFF; /* 不过滤 */
	CAN_FMR1 = 0xFF; 
	CAN_FMR2 = 0xFF;
	CAN_FMR3 = 0xFF;
	
	SetBit(CAN_CR0, CANEN); /*  CAN模块使能 */
	
  //CAN收发器芯片使能控制I/O配置
  SetBit(P2_OE, PIN6);
	ClrBit(P2_PU, PIN6); /*  收发器休眠管脚  500k  */
	GP26 = 0;			 /*  使能CAN收发器   */
	
}
/*  ----------------------------------------------------------------------------------------------*/
/*  Function Name  : CAN_Send can数据发送
	/*  Description    : 输入CheckMode:校验模式 ;
							 ID:发送的id号;
							 pBuffer:数据数组;
							 len：发送数据长度 0发送广播帧;
							 IDE: 发送ID类型 1扩展帧；0标准帧;
	/*  Date           : 2022-08-26
	/*  Parameter      :
	/*  ----------------------------------------------------------------------------------------------*/
void CAN_Send(uint32 ID, uint8 *pBuffer, uint8 len)
{
	//while (!ReadBit(CAN_SR, TXSUC)); /* 等待发送成功 */
		
	CAN_TXCR = len;//标准数据帧

	CAN_TXID0 = (ID >> 3)& 0xff ;
	CAN_TXID1 = (ID << 5)& 0xff ;
	CAN_TXID2 = 0;
	CAN_TXID3 = 0;
	
	CAN_TXDR0 = pBuffer[0];
	CAN_TXDR1 = pBuffer[1];
	CAN_TXDR2 = pBuffer[2];
	CAN_TXDR3 = pBuffer[3];
	CAN_TXDR4 = pBuffer[4];
	CAN_TXDR5 = pBuffer[5];
	CAN_TXDR6 = pBuffer[6];
	CAN_TXDR7 = pBuffer[7];

	SetBit(CAN_CR1, TXREQ);
}

/*  ----------------------------------------------------------------------------------------------*/
/*  Function Name  : CAN_Read can数据接收
	/*  Date           : 2022-08-26
	/*  Parameter      : 从邮箱中获取接收的数据值
	/*  ----------------------------------------------------------------------------------------------*/

void CAN_Read(void)
{
	uint8 rdptr;
	
	uint32 id[4] = 0;
	
	rdptr = CAN_CR2;
	
	rdptr = rdptr>>6; //获取最高两位
	
	switch (rdptr)
	{
	  case 0:
		
		CanRxdata.CAN_IDE = (CAN_RX0CR >> 7) & 0x1;
		CanRxdata.CAN_RTR = (CAN_RX0CR >> 6) & 0x1;
		CanRxdata.CAN_DLC = CAN_RX0CR & 0x0F;
	
		id[0] = CAN_RX0ID0;
		id[1] = CAN_RX0ID1;
		id[2] = CAN_RX0ID2;
		id[3] = CAN_RX0ID3;
	
		CanRxdata.CANID = 0;
		CanRxdata.CANID |= id[0] << 3;
		CanRxdata.CANID |= id[1] >> 5;
 
		CanRxdata.Data[0] = CAN_RX0DR0;
		CanRxdata.Data[1] = CAN_RX0DR1;
		CanRxdata.Data[2] = CAN_RX0DR2;
		CanRxdata.Data[3] = CAN_RX0DR3;
		CanRxdata.Data[4] = CAN_RX0DR4;
		CanRxdata.Data[5] = CAN_RX0DR5;
		CanRxdata.Data[6] = CAN_RX0DR6;
		CanRxdata.Data[7] = CAN_RX0DR7;
		
		break;

	case 1:
		
		CanRxdata.CAN_IDE = (CAN_RX1CR >> 7) & 0x1;
		CanRxdata.CAN_RTR = (CAN_RX1CR >> 6) & 0x1;
		CanRxdata.CAN_DLC = CAN_RX1CR & 0x0F;
	
		id[0] = CAN_RX1ID0;
		id[1] = CAN_RX1ID1;
		id[2] = CAN_RX1ID2;
		id[3] = CAN_RX1ID3;
	 
		CanRxdata.CANID = 0;
		CanRxdata.CANID |= id[0] << 3;
		CanRxdata.CANID |= id[1] >> 5;
 
		CanRxdata.Data[0] = CAN_RX1DR0;
		CanRxdata.Data[1] = CAN_RX1DR1;
		CanRxdata.Data[2] = CAN_RX1DR2;
		CanRxdata.Data[3] = CAN_RX1DR3;
		CanRxdata.Data[4] = CAN_RX1DR4;
		CanRxdata.Data[5] = CAN_RX1DR5;
		CanRxdata.Data[6] = CAN_RX1DR6;
		CanRxdata.Data[7] = CAN_RX1DR7;
		
		break;

	case 2:
		
		CanRxdata.CAN_IDE = (CAN_RX2CR >> 7) & 0x1;
		CanRxdata.CAN_RTR = (CAN_RX2CR >> 6) & 0x1;
		CanRxdata.CAN_DLC = CAN_RX2CR & 0x0F;
		id[0] = CAN_RX2ID0;
		id[1] = CAN_RX2ID1;
		id[2] = CAN_RX2ID2;
		id[3] = CAN_RX2ID3;
		
		CanRxdata.CANID = 0;
		CanRxdata.CANID |= id[0] << 3;
		CanRxdata.CANID |= id[1] >> 5;
	
		CanRxdata.Data[0] = CAN_RX2DR0;
		CanRxdata.Data[1] = CAN_RX2DR1;
		CanRxdata.Data[2] = CAN_RX2DR2;
		CanRxdata.Data[3] = CAN_RX2DR3;
		CanRxdata.Data[4] = CAN_RX2DR4;
		CanRxdata.Data[5] = CAN_RX2DR5;
		CanRxdata.Data[6] = CAN_RX2DR6;
		CanRxdata.Data[7] = CAN_RX2DR7;
		
		break;
	default:
		break;
	}
	if(!CanRxdata.CAN_IDE && (CanRxdata.CANID ==0X01))
	{
    CanRxdata.CAN_ResonFlag  = 1;
  }
}

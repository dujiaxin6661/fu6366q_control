#ifndef CAN_H
#define CAN_H

#define IDNum          0x01
#define CAN_COMMAND_ID 0x200

typedef struct
{
		//  : 表示位域，即这个变量只占用几位，后面是1就存储1位，是4就存储4位
		uint8 CAN_ResonFlag :1;  // 接收响应标志位，CAN_Read() 从硬件邮箱里读出一帧数据后，把数据放进 CanRxdata，然后把 CAN_ResonFlag 置 1
		uint8 CAN_IDE :1;
    uint8 CAN_RTR :1;				 // 0 代表标准帧（11位ID），1 代表扩展帧（29位ID）。
    uint8 CAN_DLC :4;				 // 记录接下来的 Data 数组里到底有多少个有效字节（0~8）。
    uint32 CANID;						 // CAN ID（占 32 位/4字节）。用来存放比如 0x180、0x200 这样的帧 ID。
    uint8 Data[8];    			 // 数据实体，占用8字节
} CanRxMsg;   							 // 接收缓冲区，用来保存收到的一帧CAN数据

extern uint8 xdata CANdata[8];  // 发送缓冲区。要发送一帧 CAN，先把数据放进去
extern CanRxMsg CanRxdata;   // 结构体重命名

void CAN_Read(void);
void CAN_Send(uint32 ID, uint8 *pBuffer, uint8 len);
void CAN_Init(void);

#endif

#ifndef CAN_H
#define CAN_H

#define IDNum          0x01
#define CAN_COMMAND_ID 0x200

typedef struct
{
    uint8 CAN_ResonFlag :1;
    uint8 CAN_IDE :1;
    uint8 CAN_RTR :1;
    uint8 CAN_DLC :4;
    uint32 CANID;
    uint8 Data[8];
} CanRxMsg;

extern uint8 xdata CANdata[8];
extern CanRxMsg CanRxdata;

void CAN_Read(void);
void CAN_Send(uint32 ID, uint8 *pBuffer, uint8 len);
void CAN_Init(void);

#endif

/*
 * CAN application layer.
 *
 * RX:
 *   0x200 上位机发送给 FU6366 的命令
 *   0x001 上位机发送给 FU6366 的测试，收到后原样返回
 *
 * TX:
 *   0x180~0x183：电机主动汇报状态给上位机
 *   0x184：电机回复主控，表示命令收到
 */

#include <Myproject.h>
#include "Customer.h"

#define CAN_TELEMETRY_PERIOD_MS     (5)           // 5 毫秒
#define CAN_ID_BASELINE_ADC         (0x180)		  // FU6366 → 上位机：电压、电流、Iq、Id
#define CAN_ID_BASELINE_FOC         (0x181)		  // FU6366 → 上位机：Iq 给定、速度、温度、故障、电机状态
#define CAN_ID_BASELINE_ANGLE       (0x182)		  // FU6366 → 上位机：实际角度、目标角度、角度误差、Iq 峰值
#define CAN_ID_BASELINE_SPEED       (0x183)		  // FU6366 → 上位机：速度斜坡、按键状态、控制模式、CAN 状态
#define CAN_ID_COMMAND_ACK          (0x184)		  // FU6366 → 上位机：对 0x200 控制命令的应答

static volatile uint8 data sg_ucTelemetryTick = 0;     // 1 ms计时器
static volatile uint8 data sg_ucTelemetryPending = 0;
static uint8 data sg_ucTelemetryFrame = 0;
static uint8 xdata sg_ucTelemetryData[8];
static uint8 xdata sg_ucAckData[8];
static volatile uint16 data sg_uiIqAbsPeakRaw = 0;

// 打包函数：把变量拆成 CAN 的 8 个字节。CAN 一帧最多 8 字节，但是你的变量很多是 uint16、int16，一个变量占 2 字节。
static void CAN_PackU16(uint8 index, uint16 value)
{
    sg_ucTelemetryData[index] = (uint8)(value & 0x00FF);
    sg_ucTelemetryData[index + 1] = (uint8)(value >> 8);
}

// CAN_PackU16 是对无符号变量的打包，这个是对有符号变量的打包，因为电流电压可能会有负值
static void CAN_PackS16(uint8 index, int16 value)
{
    CAN_PackU16(index, (uint16)value);
}

// 计算 int16 的绝对值。计算实际 Iq 电流的绝对值时候会用到
static uint16 CAN_AbsS16(int16 value)
{
    if(value < 0)
    {
        return (uint16)(-value);
    }
    return (uint16)value;
}

// 从接收到的 CAN 数据里，读取两个字节，组合成一个 int16。
static int16 CAN_ReadS16(uint8 index)
{
    uint16 value;

    value = (uint16)CanRxdata.Data[index];
    value |= ((uint16)CanRxdata.Data[index + 1] << 8);
    return (int16)value;
}

static void CAN_AckPackU16(uint8 index, uint16 value)
{
    sg_ucAckData[index] = (uint8)(value & 0x00FF);
    sg_ucAckData[index + 1] = (uint8)(value >> 8);
}


/*
1ms 定时函数
*/
void CAN_TelemetryTick1ms(void)
{
    uint16 iq_abs;

    iq_abs = CAN_AbsS16(FOC__IQ);        // 计算 Iq 电流的绝对值
    if(iq_abs > sg_uiIqAbsPeakRaw)
    {
        sg_uiIqAbsPeakRaw = iq_abs;      // 每 1ms 看一下当前实际 Iq 电流，如果比之前记录的峰值还大，就更新峰值。
    }

    sg_ucTelemetryTick++;
    if(sg_ucTelemetryTick >= CAN_TELEMETRY_PERIOD_MS)
    {
        sg_ucTelemetryTick = 0;
        if(sg_ucTelemetryPending <= 4)   // 每 5ms 准备发送 4 帧遥测
        {
            sg_ucTelemetryPending += 4;
        }
    }
}

// 发送 ID = 0x180 的 CAN 帧
static void CAN_SendBaselineAdc(void)
{
    int16 ibus_delta;

    ibus_delta = (int16)AdcSampleValue.ADCIbusFlt - mcCurOffset.Ibus_FltOffset;
    CAN_PackU16(0, AdcSampleValue.ADCDcbusFlt);
    CAN_PackS16(2, ibus_delta);
    CAN_PackS16(4, FOC__IQ);
    CAN_PackS16(6, FOC__ID);
    CAN_Send(CAN_ID_BASELINE_ADC, sg_ucTelemetryData, 8);   
}

// 发送 0x181
static void CAN_SendBaselineFoc(void)
{
    CAN_PackS16(0, FOC_IQREF);
    CAN_PackS16(2, mcFocCtrl.SpeedFlt);
    CAN_PackU16(4, AdcSampleValue.ADCTmosFlt);
    sg_ucTelemetryData[6] = (uint8)mcFaultSource;          // 故障来源
    sg_ucTelemetryData[7] = (uint8)mcState;				   // 电机状态
    CAN_Send(CAN_ID_BASELINE_FOC, sg_ucTelemetryData, 8);
}

// 发送 0x182
static void CAN_SendBaselineAngle(void)
{
    int16 angle_error;

    angle_error = _Q15Limit(mcFocCtrl.TargetAngle.s32 - mcFocCtrl.SensorAngle.s32);
    CAN_PackU16(0, (uint16)mcFocCtrl.SensorAngle.s16[1]);
    CAN_PackU16(2, (uint16)mcFocCtrl.TargetAngle.s16[1]);
    CAN_PackS16(4, angle_error);
    CAN_PackU16(6, sg_uiIqAbsPeakRaw);
    sg_uiIqAbsPeakRaw = CAN_AbsS16(FOC__IQ);
    CAN_Send(CAN_ID_BASELINE_ANGLE, sg_ucTelemetryData, 8);
}

// 发送 0x183
static void CAN_SendBaselineSpeed(void)
{
    uint8 can_mode;
    uint8 can_status;
    uint8 can_seq;
    uint16 can_age;

    Apply_CAN_GetStatus(&can_mode, &can_status, &can_seq, &can_age);

    CAN_PackS16(0, mcSpeedRamp.TargetValue);
    CAN_PackS16(2, mcSpeedRamp.ActualValue);
    sg_ucTelemetryData[4] = 0;
    sg_ucTelemetryData[5] = (uint8)mcFocCtrl.CtrlMode;
    sg_ucTelemetryData[6] = can_mode;
    sg_ucTelemetryData[7] = can_status;

    if(GP33 == 0)
    {
        sg_ucTelemetryData[4] |= 0x01;
    }

    if(GP45 == 0)
    {
        sg_ucTelemetryData[4] |= 0x02;
    }

    if(mcSpeedRamp.FlagONOFF)
    {
        sg_ucTelemetryData[4] |= 0x04;
    }

    CAN_Send(CAN_ID_BASELINE_SPEED, sg_ucTelemetryData, 8);
}

// 轮流发送遥测帧
static void CAN_TelemetryProcess(void)
{
    if(sg_ucTelemetryPending == 0)   // 如果没有遥测帧等待发送，就直接退出
    {
        return;
    }

    if(ReadBit(CAN_SR, TXING))       // 如果 CAN 还在发上一帧，就直接退出，防止发送冲突。
    {
        return;
    }

    if(sg_ucTelemetryFrame == 0)
    {
        CAN_SendBaselineAdc();
        sg_ucTelemetryFrame = 1;
    }
    else if(sg_ucTelemetryFrame == 1)
    {
        CAN_SendBaselineFoc();
        sg_ucTelemetryFrame = 2;
    }
    else if(sg_ucTelemetryFrame == 2)
    {
        CAN_SendBaselineAngle();
        sg_ucTelemetryFrame = 3;
    }
    else
    {
        CAN_SendBaselineSpeed();
        sg_ucTelemetryFrame = 0;
    }

    sg_ucTelemetryPending--;
}

// 发送 0x184 应答帧
static void CAN_SendCommandAck(uint8 rx_seq)
{
    uint8 mode;
    uint8 status;
    uint8 applied_seq;
    uint16 age_ms;

    Apply_CAN_GetStatus(&mode, &status, &applied_seq, &age_ms);

    sg_ucAckData[0] = mode;
    sg_ucAckData[1] = status;
    sg_ucAckData[2] = (uint8)mcFaultSource;
    sg_ucAckData[3] = (uint8)mcState;
    sg_ucAckData[4] = (uint8)mcFocCtrl.CtrlMode;
    sg_ucAckData[5] = rx_seq;
    CAN_AckPackU16(6, age_ms);
    CAN_Send(CAN_ID_COMMAND_ACK, sg_ucAckData, 8);
}

// 处理 0x200 控制命令
static void CAN_HandleCommandFrame(void)
{
    uint8 mode;
    uint8 flags;
    uint8 seq;
    uint16 timeout_ms;
    int16 target;
    int16 limit_ma;

    if(CanRxdata.CAN_DLC < 8)   // 检查长度，因为 CAN 命令帧要求 8 个字节
    {
        return;
    }
	
	// 解析 8 个数据字节
    mode = CanRxdata.Data[0];
    flags = CanRxdata.Data[1];
    target = CAN_ReadS16(2);
    limit_ma = CAN_ReadS16(4);
    timeout_ms = ((uint16)CanRxdata.Data[6]) * 50;
    seq = CanRxdata.Data[7];

	// 把命令交给应用层
    Apply_CAN_SetCommand(mode, flags, target, limit_ma, timeout_ms, seq);   
    CAN_SendCommandAck(seq);    // 收到命令后，立刻回复一帧 0x184。
}


// CAN 应用层主入口
void CAN_Process(void)
{
    if(CanRxdata.CAN_ResonFlag == 1)   // 检查 CAN_Read() 是否已经从硬件里读到一帧数据，并放进 CanRxdata 了
    {
        CanRxdata.CAN_ResonFlag = 0;   // 如果收到新帧，清零当前接收标志位

        if(!CanRxdata.CAN_IDE && CanRxdata.CANID == CAN_COMMAND_ID)   // 如果收到的是标准帧，并且ID == 0x200（控制命令ID），那这帧数据就是控制命令
        {
            CAN_HandleCommandFrame();
            return;
        }

        if(!CanRxdata.CAN_IDE && CanRxdata.CANID == IDNum)  // 如果收到ID为0x01的CAN帧（测试帧），就将接收到的数据原样发回去，用来测试CAN通讯
        {
            CAN_Send(IDNum, CanRxdata.Data, 8);
            return;
        }
    }

    CAN_TelemetryProcess();       // 如果没有收到 0x200 或 0x001 命令，就继续发送遥测帧数据。
}

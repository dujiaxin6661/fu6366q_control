/**
 * ============================================================================
 * @file      ControlChat.c
 * @brief     CAN总线通信处理 —— 接收上位机指令并应答
 * @note      本文件处理CAN通信的应用层逻辑：
 *
 *              CAN_Process() —— 在主循环中调用
 *              检查CAN接收标志(CAN_ResonFlag)，收到消息后将数据原样回传（echo）
 *              使用CAN_Send()发送8字节数据到IDNum指定的CAN ID
 *
 *              注意：这是原厂demo的极简实现（仅回传数据），实际产品中需要：
 *              - 解析CAN指令格式（如ID=0x200设置控制模式，ID=0x201设置目标值）
 *              - 根据指令调用对应的电机控制函数
 *              - 定期上报状态（速度/电流/角度/故障码）
 *
 *              全局变量（在CAN驱动中定义）：
 *              - CanRxdata.CAN_ResonFlag : 接收完成标志
 *              - IDNum                     : 要发送的CAN ID
 *              - CANdata[8]               : 发送数据缓冲区
 * ============================================================================
 */

#include <Myproject.h>
#include "Customer.h"

#define CAN_TELEMETRY_PERIOD_MS     (5)
#define CAN_ID_BASELINE_ADC         (0x180)
#define CAN_ID_BASELINE_FOC         (0x181)
#define CAN_ID_BASELINE_ANGLE       (0x182)
#define CAN_ID_BASELINE_SPEED       (0x183)

static volatile uint8 data sg_ucTelemetryTick = 0;
static volatile uint8 data sg_ucTelemetryPending = 0;
static uint8 data sg_ucTelemetryFrame = 0;
static uint8 xdata sg_ucTelemetryData[8];
static volatile uint16 data sg_uiIqAbsPeakRaw = 0;

static void CAN_PackU16(uint8 index, uint16 value)
{
    sg_ucTelemetryData[index] = (uint8)(value & 0x00FF);
    sg_ucTelemetryData[index + 1] = (uint8)(value >> 8);
}

static void CAN_PackS16(uint8 index, int16 value)
{
    CAN_PackU16(index, (uint16)value);
}

static uint16 CAN_AbsS16(int16 value)
{
    if(value < 0)
    {
        return (uint16)(-value);
    }
    return (uint16)value;
}

void CAN_TelemetryTick1ms(void)
{
    uint16 iq_abs;

    iq_abs = CAN_AbsS16(FOC__IQ);
    if(iq_abs > sg_uiIqAbsPeakRaw)
    {
        sg_uiIqAbsPeakRaw = iq_abs;
    }

    sg_ucTelemetryTick++;
    if(sg_ucTelemetryTick >= CAN_TELEMETRY_PERIOD_MS)
    {
        sg_ucTelemetryTick = 0;
        if(sg_ucTelemetryPending <= 4)
        {
            sg_ucTelemetryPending += 4;
        }
    }
}

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

static void CAN_SendBaselineFoc(void)
{
    CAN_PackS16(0, FOC_IQREF);
    CAN_PackS16(2, mcFocCtrl.SpeedFlt);
    CAN_PackU16(4, AdcSampleValue.ADCTmosFlt);
    sg_ucTelemetryData[6] = (uint8)mcFaultSource;
    sg_ucTelemetryData[7] = (uint8)mcState;
    CAN_Send(CAN_ID_BASELINE_FOC, sg_ucTelemetryData, 8);
}

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

static void CAN_SendBaselineSpeed(void)
{
    CAN_PackS16(0, mcSpeedRamp.TargetValue);
    CAN_PackS16(2, mcSpeedRamp.ActualValue);
    sg_ucTelemetryData[4] = 0;
    sg_ucTelemetryData[5] = (uint8)mcFocCtrl.CtrlMode;
    sg_ucTelemetryData[6] = 0;
    sg_ucTelemetryData[7] = 0;

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

static void CAN_TelemetryProcess(void)
{
    if(sg_ucTelemetryPending == 0)
    {
        return;
    }

    if(ReadBit(CAN_SR, TXING))
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

/**
 * @brief      CAN_Process() —— CAN通信应用层处理（由主循环每周期调用）
 * @note       当前功能：收到CAN消息后原样回传（echo/loopback测试）
 *              CAN_Send(IDNum, CANdata, 8) → 将8字节数据发送到IDNum
 *
 *              扩展建议：
 *              - 定义协议格式：bytes[0]=命令类型, bytes[1-2]=目标值(16位), bytes[3]=CRC
 *              - 命令类型：0x01=力矩控制, 0x02=速度控制, 0x03=位置控制
 *              - 状态上报：定时发送电机状态（速度/电流/角度/温度/故障码）
 */
void CAN_Process(void)
{
    if(CanRxdata.CAN_ResonFlag == 1)                     // ★ 有CAN消息到达
    {
        CanRxdata.CAN_ResonFlag = 0;                     // 清除标志（允许接收下一条）

        /* 原样回传：将接收到的8字节数据发送回主机 */
        CAN_Send(IDNum, CANdata, 8);                     // IDNum=接收时的CAN ID, CANdata=8字节数据
        return;
    }

    CAN_TelemetryProcess();
}

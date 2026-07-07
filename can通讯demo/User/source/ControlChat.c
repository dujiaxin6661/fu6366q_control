/*
 * CAN application layer.
 *
 * RX:
 *   0x200 command frame from PC/main controller.
 *   0x001 legacy ping frame, echoed for link tests.
 *
 * TX:
 *   0x180..0x183 telemetry frames.
 *   0x184 command acknowledgement frame.
 */

#include <Myproject.h>
#include "Customer.h"

#define CAN_TELEMETRY_PERIOD_MS     (5)
#define CAN_ID_BASELINE_ADC         (0x180)
#define CAN_ID_BASELINE_FOC         (0x181)
#define CAN_ID_BASELINE_ANGLE       (0x182)
#define CAN_ID_BASELINE_SPEED       (0x183)
#define CAN_ID_COMMAND_ACK          (0x184)

static volatile uint8 data sg_ucTelemetryTick = 0;
static volatile uint8 data sg_ucTelemetryPending = 0;
static uint8 data sg_ucTelemetryFrame = 0;
static uint8 xdata sg_ucTelemetryData[8];
static uint8 xdata sg_ucAckData[8];
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

static void CAN_HandleCommandFrame(void)
{
    uint8 mode;
    uint8 flags;
    uint8 seq;
    uint16 timeout_ms;
    int16 target;
    int16 limit_ma;

    if(CanRxdata.CAN_DLC < 8)
    {
        return;
    }

    mode = CanRxdata.Data[0];
    flags = CanRxdata.Data[1];
    target = CAN_ReadS16(2);
    limit_ma = CAN_ReadS16(4);
    timeout_ms = ((uint16)CanRxdata.Data[6]) * 50;
    seq = CanRxdata.Data[7];

    Apply_CAN_SetCommand(mode, flags, target, limit_ma, timeout_ms, seq);
    CAN_SendCommandAck(seq);
}

void CAN_Process(void)
{
    if(CanRxdata.CAN_ResonFlag == 1)
    {
        CanRxdata.CAN_ResonFlag = 0;

        if(!CanRxdata.CAN_IDE && CanRxdata.CANID == CAN_COMMAND_ID)
        {
            CAN_HandleCommandFrame();
            return;
        }

        if(!CanRxdata.CAN_IDE && CanRxdata.CANID == IDNum)
        {
            CAN_Send(IDNum, CanRxdata.Data, 8);
            return;
        }
    }

    CAN_TelemetryProcess();
}

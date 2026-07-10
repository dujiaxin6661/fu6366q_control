/*
 * CAN command + key fallback control.
 *
 * CAN command frame 0x200:
 *   byte0 mode:
 *     0 = disable/coast
 *     1 = current, target is Iq in mA
 *     2 = speed, target is rpm
 *     3 = position, target is 0.01 degree
 *     4 = brake_to_hold
 *     5 = hold, current angle unless flags bit1 is set
 *   byte1 flags:
 *     bit0 = enable CAN control
 *     bit1 = relative angle for position, or use target angle for hold
 *   byte2..3 target_s16 little endian
 *   byte4..5 limit_s16 little endian, current limit in mA
 *   byte6 timeout in 50 ms units, 0 means default timeout
 *   byte7 sequence number
 *
 * If no valid CAN command arrives before timeout, the motor brakes to 0 rpm
 * and then holds the current angle.
 */

#include <Myproject.h>
#include "Customer.h"

#define KEY_JOG_SPEED_RPM             (50)
#define KEY_JOG_IQ_LIMIT_A            (0.5)

#define CAN_DEFAULT_TIMEOUT_MS        (200)     // 200 ms
#define CAN_MIN_TIMEOUT_MS            (50)
#define CAN_MAX_TIMEOUT_MS            (5000)
#define CAN_DEFAULT_IQ_LIMIT_MA       (500)
#define CAN_MAX_IQ_LIMIT_MA           (600)
#define CAN_CURRENT_MAX_MA            (600)
#define CAN_SPEED_MAX_RPM             (500)
#define CAN_POSITION_MAX_RPM          (80)
#define CAN_HOLD_MAX_RPM              (80)
#define CAN_LOCK_SPEED_RPM            (8)

#define CAN_FLAG_ENABLE               (0x01)
#define CAN_FLAG_RELATIVE             (0x02)

#define APPLY_CAN_STATUS_READY        (0x01)
#define APPLY_CAN_STATUS_ACTIVE       (0x02)
#define APPLY_CAN_STATUS_TIMEOUT      (0x04)
#define APPLY_CAN_STATUS_BRAKING      (0x08)

#define APPLY_IQ_RAW_PER_A            (I_Value(1.0))

typedef enum
{
    ApplyMode_Hold = 0,
    ApplyMode_JogLeft,
    ApplyMode_JogRight,
    ApplyMode_StopToHold,
    ApplyMode_CanDisable,
    ApplyMode_CanCurrent,
    ApplyMode_CanSpeed,
    ApplyMode_CanPosition,
    ApplyMode_CanBrakeToHold,
    ApplyMode_CanHold
} ApplyModeType;

APPLYTypeDef xdata ApplyState;

static ApplyModeType xdata s_applyMode;
static uint8 xdata s_lastJogActive;

static uint8 xdata s_canActive;
static uint8 xdata s_canMode;
static uint8 xdata s_canFlags;
static uint8 xdata s_canSeq;
static uint8 xdata s_canTimeoutLatch;
static uint16 xdata s_canAgeMs;
static uint16 xdata s_canTimeoutMs;
static int16 xdata s_canTarget;
static int16 xdata s_canLimitMa;
static uint8 xdata s_canCommandDirty;
static uint8 xdata s_canLastAppliedSeq;

static int16 Apply_LimitS16(int16 value, int16 min_value, int16 max_value)
{
    if(value > max_value)
    {
        return max_value;
    }
    if(value < min_value)
    {
        return min_value;
    }
    return value;
}

static int16 Apply_LimitAbsMa(int16 limit_ma)
{
    if(limit_ma < 0)
    {
        limit_ma = -limit_ma;
    }
    if(limit_ma == 0)
    {
        limit_ma = CAN_DEFAULT_IQ_LIMIT_MA;
    }
    if(limit_ma > CAN_MAX_IQ_LIMIT_MA)
    {
        limit_ma = CAN_MAX_IQ_LIMIT_MA;
    }
    return limit_ma;
}

static int16 Apply_MilliAmpToIqRaw(int16 current_ma)
{
    int32 raw;

    raw = ((int32)current_ma * (int32)APPLY_IQ_RAW_PER_A) / 1000L;
    return (int16)MaxMinLimit(32767, -32768, raw);
}

static int16 Apply_RpmToRaw(int16 rpm)
{
    int32 raw;

    raw = ((int32)rpm * 32767L) / 4000L;
    return (int16)MaxMinLimit(32767, -32768, raw);
}

static int32 Apply_CentiDegToRaw(int16 centi_deg)
{
    return ((int32)centi_deg * 65536L) / 36000L;
}

static void Apply_ForceCurrentLoopHardware(void)
{
    ClrBit(FOC_CR2, UQD);
    ClrBit(FOC_CR2, UDD);
    FOC_IDREF = 0;
    MOE = 1;
}

static void Apply_ConfigSpeedOutputRaw(int16 target_speed_raw, int16 iq_limit_raw)
{
    mcFocCtrl.CtrlMode = SpeedLoopMode;
    mcFocCtrl.CurrentLoopEn = 1;
    mcFocCtrl.AngleMode = AngleLoop;
    s_PI.Uk_max = iq_limit_raw;
    s_PI.Uk_min = -iq_limit_raw;
    mcSpeedRamp.TargetValue = target_speed_raw;
    mcSpeedRamp.ActualValue = target_speed_raw;
    mcSpeedRamp.FlagONOFF = 1;
    Apply_ForceCurrentLoopHardware();
}

static void Apply_ConfigSpeedOutput(int16 target_speed)
{
    Apply_ConfigSpeedOutputRaw(target_speed, I_Value(KEY_JOG_IQ_LIMIT_A));
}

static void Apply_ConfigCurrentOutput(int16 iq_ref)
{
    mcFocCtrl.CtrlMode = CurrentLoopMode;
    mcFocCtrl.CurrentLoopEn = 1;
    mcFocCtrl.AngleMode = AngleLoop;
    mcFocCtrl.Iqref = iq_ref;
    mcSpeedRamp.TargetValue = 0;
    mcSpeedRamp.ActualValue = 0;
    mcSpeedRamp.FlagONOFF = 1;
    Apply_ForceCurrentLoopHardware();
    FOC_IQREF = iq_ref;
}

static void Apply_ConfigPositionOutput(int16 max_rpm, int16 iq_limit_raw)
{
    mcFocCtrl.CtrlMode = PostionLoopMode;
    mcFocCtrl.CurrentLoopEn = 1;
    mcFocCtrl.AngleMode = AngleLoop;
    mcFocCtrl.PostionLoopOut = 0;
    mcFocCtrl.SpeedErr = 0;
    PI_Position.Err = 0;
    PI_Position.ErrLast = 0;
    PI_Position.Uk_max = Apply_RpmToRaw(max_rpm);
    PI_Position.Uk_min = Apply_RpmToRaw(-max_rpm);
    s_PI.Uk_max = iq_limit_raw;
    s_PI.Uk_min = -iq_limit_raw;
    mcSpeedRamp.TargetValue = 0;
    mcSpeedRamp.ActualValue = 0;
    mcSpeedRamp.FlagONOFF = 1;
    Apply_ForceCurrentLoopHardware();
}

static void Apply_HoldCurrentAngle(void)
{
    Apply_ConfigPositionOutput(CAN_HOLD_MAX_RPM, I_Value(KEY_JOG_IQ_LIMIT_A));
    mcFocCtrl.TargetAngle.s32 = mcFocCtrl.SensorAngle.s32;
    s_applyMode = ApplyMode_Hold;
    s_lastJogActive = 0;
}

static void Apply_StartJog(int16 target_speed, ApplyModeType mode)
{
    Apply_ConfigSpeedOutput(target_speed);
    s_applyMode = mode;
    s_lastJogActive = 1;
}

static void Apply_StartStopToHold(void)
{
    Apply_ConfigSpeedOutput(0);
    s_applyMode = ApplyMode_StopToHold;
    s_lastJogActive = 0;
}

static uint8 Apply_IsStopped(void)
{
    return (Abs_F16(mcFocCtrl.SpeedFlt) <= Apply_RpmToRaw(CAN_LOCK_SPEED_RPM));
}

static uint8 Apply_IsReady(void)
{
    return (MRSyS.MRState == MRFinish && mcState == mcRun);
}

static void Apply_DisableOutput(void)
{
    mcFocCtrl.CtrlMode = CurrentLoopMode;
    mcFocCtrl.CurrentLoopEn = 1;
    mcFocCtrl.AngleMode = AngleLoop;
    mcFocCtrl.Iqref = 0;
    mcSpeedRamp.TargetValue = 0;
    mcSpeedRamp.ActualValue = 0;
    mcSpeedRamp.FlagONOFF = 1;
    Apply_ForceCurrentLoopHardware();
    FOC_IQREF = 0;
    s_applyMode = ApplyMode_CanDisable;
    s_lastJogActive = 0;
}

// 真正的电机控制函数
static void Apply_ExecuteCanCommand(void)
{
    int16 limit_ma;
    int16 iq_limit_raw;
    int16 target;
    int16 speed_raw;
    int32 angle_raw;

    if(!s_canActive && s_canCommandDirty == 0)
    {
        return;
    }

    if(!Apply_IsReady())
    {
        return;
    }

    if(s_canTimeoutLatch)
    {
        if(s_applyMode != ApplyMode_CanBrakeToHold && s_applyMode != ApplyMode_CanHold && s_applyMode != ApplyMode_Hold)
        {
            Apply_ConfigSpeedOutputRaw(0, Apply_MilliAmpToIqRaw(CAN_DEFAULT_IQ_LIMIT_MA));
            s_applyMode = ApplyMode_CanBrakeToHold;
        }
        if(s_applyMode == ApplyMode_CanBrakeToHold && Apply_IsStopped())
        {
            Apply_ConfigPositionOutput(CAN_HOLD_MAX_RPM, Apply_MilliAmpToIqRaw(CAN_DEFAULT_IQ_LIMIT_MA));
            mcFocCtrl.TargetAngle.s32 = mcFocCtrl.SensorAngle.s32;
            s_applyMode = ApplyMode_CanHold;
        }
        return;
    }

    if(s_canCommandDirty == 0 && s_canTimeoutLatch == 0)
    {
        if(s_applyMode == ApplyMode_CanBrakeToHold && Apply_IsStopped())
        {
            Apply_ConfigPositionOutput(CAN_HOLD_MAX_RPM, Apply_MilliAmpToIqRaw(CAN_DEFAULT_IQ_LIMIT_MA));
            mcFocCtrl.TargetAngle.s32 = mcFocCtrl.SensorAngle.s32;
            s_applyMode = ApplyMode_CanHold;
        }
        return;
    }

    limit_ma = Apply_LimitAbsMa(s_canLimitMa);
    iq_limit_raw = Apply_MilliAmpToIqRaw(limit_ma);
    target = s_canTarget;
    s_canCommandDirty = 0;
    s_canLastAppliedSeq = s_canSeq;

    if(s_canMode == 0)
    {
        Apply_DisableOutput();
    }
    else if(s_canMode == 1)
    {
        target = Apply_LimitS16(target, -CAN_CURRENT_MAX_MA, CAN_CURRENT_MAX_MA);
        target = Apply_LimitS16(target, -limit_ma, limit_ma);
        Apply_ConfigCurrentOutput(Apply_MilliAmpToIqRaw(target));
        s_applyMode = ApplyMode_CanCurrent;
    }
    else if(s_canMode == 2)
    {
        target = Apply_LimitS16(target, -CAN_SPEED_MAX_RPM, CAN_SPEED_MAX_RPM);
        speed_raw = Apply_RpmToRaw(target);
        Apply_ConfigSpeedOutputRaw(speed_raw, iq_limit_raw);
        s_applyMode = ApplyMode_CanSpeed;
    }
    else if(s_canMode == 3)
    {
        Apply_ConfigPositionOutput(CAN_POSITION_MAX_RPM, iq_limit_raw);
        angle_raw = Apply_CentiDegToRaw(target);
        if(s_canFlags & CAN_FLAG_RELATIVE)
        {
            mcFocCtrl.TargetAngle.s32 = mcFocCtrl.SensorAngle.s32 + angle_raw;
        }
        else
        {
            mcFocCtrl.TargetAngle.s32 = angle_raw;
        }
        s_applyMode = ApplyMode_CanPosition;
    }
    else if(s_canMode == 4)
    {
        Apply_ConfigSpeedOutputRaw(0, iq_limit_raw);
        s_applyMode = ApplyMode_CanBrakeToHold;
    }
    else if(s_canMode == 5)
    {
        Apply_ConfigPositionOutput(CAN_HOLD_MAX_RPM, iq_limit_raw);
        if(s_canFlags & CAN_FLAG_RELATIVE)
        {
            mcFocCtrl.TargetAngle.s32 = Apply_CentiDegToRaw(target);
        }
        else
        {
            mcFocCtrl.TargetAngle.s32 = mcFocCtrl.SensorAngle.s32;
        }
        s_applyMode = ApplyMode_CanHold;
    }
}

static void Apply_KeyFallbackControl(void)
{
    if(GP33 == 0 && GP45 == 0)
    {
        if(s_applyMode == ApplyMode_JogLeft || s_applyMode == ApplyMode_JogRight || s_lastJogActive)
        {
            Apply_StartStopToHold();
        }
    }
    else if(GP45 == 0)
    {
        if(s_applyMode != ApplyMode_JogRight)
        {
            Apply_StartJog(Apply_RpmToRaw(KEY_JOG_SPEED_RPM), ApplyMode_JogRight);
        }
    }
    else if(GP33 == 0)
    {
        if(s_applyMode != ApplyMode_JogLeft)
        {
            Apply_StartJog(Apply_RpmToRaw(-KEY_JOG_SPEED_RPM), ApplyMode_JogLeft);
        }
    }
    else
    {
        if(s_applyMode == ApplyMode_JogLeft || s_applyMode == ApplyMode_JogRight || s_lastJogActive)
        {
            Apply_StartStopToHold();
        }
        else if(s_applyMode == ApplyMode_StopToHold)
        {
            if(Apply_IsStopped())
            {
                Apply_HoldCurrentAngle();
            }
        }
    }
}

/*
只负责“接收并登记命令”，不直接控制电机。
*/ 
void Apply_CAN_SetCommand(uint8 mode, uint8 flags, int16 target, int16 limit_ma, uint16 timeout_ms, uint8 seq)
{
    if(timeout_ms == 0)
    {
        timeout_ms = CAN_DEFAULT_TIMEOUT_MS;   // 上位机发送0时，不表示永不超时，而是使用默认值 200ms。
    }
    else if(timeout_ms < CAN_MIN_TIMEOUT_MS)
    {
        timeout_ms = CAN_MIN_TIMEOUT_MS;       // 低于 50ms 时强制改成 50ms，避免上位机稍有延迟就误触发超时。
    }
    else if(timeout_ms > CAN_MAX_TIMEOUT_MS)
    {
        timeout_ms = CAN_MAX_TIMEOUT_MS;	   // 大于 5000ms 时限制为 5000ms，防止通信已经断开，电机还长时间保持旧的速度或电流命令。
    }

	// 保存数据到内部变量
    s_canMode = mode;
    s_canFlags = flags;
    s_canTarget = target;
    s_canLimitMa = limit_ma;
    s_canTimeoutMs = timeout_ms;
    s_canSeq = seq;
	
    s_canAgeMs = 0;           // 表示距离最近一次有效命令过去了多少毫秒。
    s_canTimeoutLatch = 0;

    if(flags & CAN_FLAG_ENABLE)
    {
        if(mode == 1 || mode == 2)
        {
            s_canActive = 1;
        }
        else
        {
            s_canActive = 0;
        }
        s_canCommandDirty = 1;
    }
    else
    {
        s_canActive = 0;
        s_canCommandDirty = 1;
        s_canMode = 0;
    }
}

// 超时函数，速度模式设置 5000ms 后，如果5秒内没有新命令：自动刹停
void Apply_CAN_Tick1ms(void)
{
    if(s_canActive)
    {
        if(s_canAgeMs < 60000)
        {
            s_canAgeMs++;
        }
        if(s_canAgeMs > s_canTimeoutMs)
        {
            s_canTimeoutLatch = 1;
        }
    }
}

/*
它不改变控制状态，只负责读取并打包状态。
*/
void Apply_CAN_GetStatus(uint8 *mode, uint8 *status, uint8 *seq, uint16 *age_ms)
{
    uint8 state;
    state = 0;   // 清空状态字节，state 是8位状态字，每一位表示一种状态。
	
    if(Apply_IsReady())   // 如果MR传感器校准完成并且电机控制状态机进入 mcRun，就state |= 0x01 表示把 bit0 设置为1，同时不影响其他位。
    {
        state |= APPLY_CAN_STATUS_READY;
    }
    if(s_canActive)
    {
        state |= APPLY_CAN_STATUS_ACTIVE;
    }
    if(s_canTimeoutLatch)
    {
        state |= APPLY_CAN_STATUS_TIMEOUT;
    }
    if(s_applyMode == ApplyMode_CanBrakeToHold || s_applyMode == ApplyMode_StopToHold)
    {
        state |= APPLY_CAN_STATUS_BRAKING;
    }

    *mode = s_canMode;
    *status = state;
    *seq = s_canLastAppliedSeq;
    *age_ms = s_canAgeMs;
}

void ApplyFun_Conrtrol(void)
{
    switch (ApplyState.SystemState)
    {
        case PowerOn:
            if(Apply_IsReady())
            {
                Apply_HoldCurrentAngle();
                ApplyState.SystemState = Run;
            }
            break;

        case Run:
            if(s_canActive || s_canCommandDirty)
            {
                Apply_ExecuteCanCommand();
            }
            else
            {
                Apply_KeyFallbackControl();
            }

            if(s_applyMode == ApplyMode_CanBrakeToHold && Apply_IsStopped())
            {
                Apply_ConfigPositionOutput(CAN_HOLD_MAX_RPM, Apply_MilliAmpToIqRaw(CAN_DEFAULT_IQ_LIMIT_MA));
                mcFocCtrl.TargetAngle.s32 = mcFocCtrl.SensorAngle.s32;
                s_applyMode = ApplyMode_CanHold;
            }
            break;

        default:
            ApplyState.SystemState = PowerOn;
            break;
    }
}

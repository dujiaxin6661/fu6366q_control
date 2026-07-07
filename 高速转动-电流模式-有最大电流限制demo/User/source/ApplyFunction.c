/*
选择电流控制模式，不会根据速度误差进行反馈，并且添加了电流限制，不会导致过流保护关PWM
*/

#include <Myproject.h>
#include "Customer.h"

#define BASELINE_KEY_LOCK_MS     (250)
#define POSITION_HOLD_TARGET_DEG (30L)
#define POSITION_HOLD_MAX_RPM    (80)
#define POSITION_HOLD_IQ_LIMIT_A (0.8)
#define CURRENT_DIAG_ENABLE      (1)       // 创建 #if #else 分支
#define CURRENT_DIAG_IQ_A        (0.30)    // 限制最大电流为0.3A
#define POSITION_DEG_TO_RAW(deg) ((int32)((deg) * 65536L / 360L))

APPLYTypeDef xdata ApplyState;

static void Apply_ConfigCurrentDiagOutput(void)
{
    mcFocCtrl.CtrlMode = CurrentLoopMode;     // 电流模式，
    mcFocCtrl.CurrentLoopEn = 1;
    mcFocCtrl.AngleMode = AngleLoop;
    mcFocCtrl.Iqref = 0;
    mcSpeedRamp.TargetValue = 0;
    mcSpeedRamp.ActualValue = 0;
    mcSpeedRamp.FlagONOFF = 1;
    ClrBit(FOC_CR2, UQD);
    ClrBit(FOC_CR2, UDD);
    FOC_IDREF = 0;
    FOC_IQREF = 0;
    MOE = 1;
}

static void Apply_SetCurrentDiagIq(int16 iq_ref)
{
    mcFocCtrl.Iqref = iq_ref;
    FOC_IQREF = iq_ref;
}

static void Apply_ConfigPositionHoldOutput(void)
{
    mcFocCtrl.CtrlMode = PostionLoopMode;
    mcFocCtrl.CurrentLoopEn = 1;
    mcFocCtrl.AngleMode = AngleLoop;
    mcFocCtrl.PostionLoopOut = 0;
    mcFocCtrl.SpeedErr = 0;
    PI_Position.Err = 0;
    PI_Position.ErrLast = 0;
    PI_Position.Uk_max = S_Value(POSITION_HOLD_MAX_RPM);
    PI_Position.Uk_min = S_Value(-POSITION_HOLD_MAX_RPM);
    s_PI.Uk_max = I_Value(POSITION_HOLD_IQ_LIMIT_A);
    s_PI.Uk_min = I_Value(-POSITION_HOLD_IQ_LIMIT_A);
    mcSpeedRamp.TargetValue = 0;
    mcSpeedRamp.ActualValue = 0;
    mcSpeedRamp.FlagONOFF = 1;
    ClrBit(FOC_CR2, UQD);
    ClrBit(FOC_CR2, UDD);
    FOC_IDREF = 0;
    FOC_IQREF = 0;
    MOE = 1;
}

static void Apply_HoldCurrentAngle(void)
{
    Apply_ConfigPositionHoldOutput();
    mcFocCtrl.TargetAngle.s32 = mcFocCtrl.SensorAngle.s32;
}

static void Apply_MoveHoldTarget(int32 delta_deg)
{
    Apply_ConfigPositionHoldOutput();
    mcFocCtrl.TargetAngle.s32 = mcFocCtrl.SensorAngle.s32 + POSITION_DEG_TO_RAW(delta_deg);
}

void ApplyFun_Conrtrol(void)
{
#if CURRENT_DIAG_ENABLE
    switch (ApplyState.SystemState)
    {
        case PowerOn:
            if(MRSyS.MRState == MRFinish && mcState == mcRun)
            {
                Apply_ConfigCurrentDiagOutput();
                ApplyState.SystemState = Run;
            }
            break;

        case Run:
            if(GP45 == 0 && GP33 == 1)
            {
                Apply_SetCurrentDiagIq(I_Value(CURRENT_DIAG_IQ_A));     // 根据按键给不同的电流
            }
            else if(GP33 == 0 && GP45 == 1)
            {
                Apply_SetCurrentDiagIq(I_Value(-CURRENT_DIAG_IQ_A));
            }
            else
            {
                Apply_SetCurrentDiagIq(0);
            }
            break;

        default:
            ApplyState.SystemState = PowerOn;
            break;
    }
#else    // 进入位置保持模式
    switch (ApplyState.SystemState)
    {
        case PowerOn:
            if(MRSyS.MRState == MRFinish && mcState == mcRun)
            {
                Apply_HoldCurrentAngle();
                ApplyState.SystemState = Run;
            }
            break;

        case Run:
            if(GP33 == 0 && GP45 == 0)
            {
                Apply_HoldCurrentAngle();
                ApplyState.SystemState = Wait;
                MRSyS.State_Count = BASELINE_KEY_LOCK_MS;
            }
            else if(GP45 == 0)
            {
                Apply_MoveHoldTarget(POSITION_HOLD_TARGET_DEG);
                ApplyState.SystemState = Wait;
                MRSyS.State_Count = BASELINE_KEY_LOCK_MS;
            }
            else if(GP33 == 0)
            {
                Apply_MoveHoldTarget(-POSITION_HOLD_TARGET_DEG);
                ApplyState.SystemState = Wait;
                MRSyS.State_Count = BASELINE_KEY_LOCK_MS;
            }
            break;

        case Wait:
            if(MRSyS.State_Count == 0)
            {
                if(GP33 == 1 && GP45 == 1)
                {
                    ApplyState.SystemState = Run;
                }
            }
            break;

        default:
            ApplyState.SystemState = PowerOn;
            break;
    }
#endif
}

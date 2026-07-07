/*
按下按键以JOG_SPEED_RPM速度转动，松开按键立马刹车
静止时候，转动电机，点击可以抗绕，自动回到静止保持位置

按键时候是速度模式，松开按键进入零速制动，等点击速度降到很低的时候，在切换到位置保持模式，锁住当前角度进行抗扰
*/

#include <Myproject.h>
#include "Customer.h"

#define JOG_SPEED_RPM             (500)     // 点动控制的目标速度（按下按键就转，松开立马就停）
#define JOG_IQ_LIMIT_A            (0.5)			// 限制最大电流 
#define HOLD_MAX_RPM              (500)			// 位置保持模式下，位置环输出的最大速度限制
#define HOLD_IQ_LIMIT_A           (0.5)
#define HOLD_LOCK_SPEED_RPM       (8)       // 当点电机速度低于 8 rpm的时候，才切换到位置保持模式

typedef enum
{
    ApplyMode_Hold = 0,
    ApplyMode_JogLeft,
    ApplyMode_JogRight,
    ApplyMode_StopToHold,
} ApplyModeType;

APPLYTypeDef xdata ApplyState;
static ApplyModeType xdata s_applyMode;
static uint8 xdata s_lastJogActive;

static void Apply_ForceCurrentLoopHardware(void)
{
    ClrBit(FOC_CR2, UQD);
    ClrBit(FOC_CR2, UDD);
    FOC_IDREF = 0;
    MOE = 1;
}

static void Apply_ConfigSpeedOutput(int16 target_speed)
{
    mcFocCtrl.CtrlMode = SpeedLoopMode;
    mcFocCtrl.CurrentLoopEn = 1;
    mcFocCtrl.AngleMode = AngleLoop;
    s_PI.Uk_max = I_Value(JOG_IQ_LIMIT_A);
    s_PI.Uk_min = I_Value(-JOG_IQ_LIMIT_A);
    mcSpeedRamp.TargetValue = target_speed;
    mcSpeedRamp.ActualValue = target_speed;
    mcSpeedRamp.FlagONOFF = 1;
    Apply_ForceCurrentLoopHardware();
}

/*
位置保持模式，点电机停在某个目标角度，被外力推开后，允许产生恢复力矩，但是限制回正速度和最大电流，避免过载保护
*/
static void Apply_ConfigPositionHoldOutput(void)
{
    mcFocCtrl.CtrlMode = PostionLoopMode;        // positon control
    mcFocCtrl.CurrentLoopEn = 1;                 // 启用电流环，速度环的输出变成电流指令，由电流指令调整PWM，而不是由速度环的输出调整
    mcFocCtrl.AngleMode = AngleLoop;             // 角度控制模式，根据角度误差生成速度指令
    mcFocCtrl.PostionLoopOut = 0;
    mcFocCtrl.SpeedErr = 0;
    PI_Position.Err = 0;
    PI_Position.ErrLast = 0;
    PI_Position.Uk_max = S_Value(HOLD_MAX_RPM);
    PI_Position.Uk_min = S_Value(-HOLD_MAX_RPM);
    s_PI.Uk_max = I_Value(HOLD_IQ_LIMIT_A);
    s_PI.Uk_min = I_Value(-HOLD_IQ_LIMIT_A);
    mcSpeedRamp.TargetValue = 0;
    mcSpeedRamp.ActualValue = 0;
    mcSpeedRamp.FlagONOFF = 1;
    Apply_ForceCurrentLoopHardware();
}

static void Apply_HoldCurrentAngle(void)
{
    Apply_ConfigPositionHoldOutput();
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

// 判断是否停稳了的函数
static uint8 Apply_IsStopped(void)
{
    return (Abs_F16(mcFocCtrl.SpeedFlt) <= S_Value(HOLD_LOCK_SPEED_RPM));
}

void ApplyFun_Conrtrol(void)
{
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
                if(s_applyMode == ApplyMode_JogLeft || s_applyMode == ApplyMode_JogRight || s_lastJogActive)
                {
                    Apply_StartStopToHold();
                }
            }
            else if(GP45 == 0)
            {
                if(s_applyMode != ApplyMode_JogRight)
                {
                    Apply_StartJog(S_Value(JOG_SPEED_RPM), ApplyMode_JogRight);
                }
            }
            else if(GP33 == 0)
            {
                if(s_applyMode != ApplyMode_JogLeft)
                {
                    Apply_StartJog(S_Value(-JOG_SPEED_RPM), ApplyMode_JogLeft);
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
            break;

        default:
            ApplyState.SystemState = PowerOn;
            break;
    }
}

/**
 * @file ApplyFunction.c
 * @brief Key-triggered speed-step control for no-load baseline tests.
 *
 * Behavior:
 * - After MR calibration finishes and mcRun is reached, stop and wait for a key.
 * - First GP45 press starts +30 rpm. Later GP45 presses add +10 rpm.
 * - First GP33 press starts -30 rpm. Later GP33 presses subtract 10 rpm.
 * - Target speed is clamped to -200..+200 rpm.
 * - The motor keeps running after the key is released.
 */

#include <Myproject.h>
#include "Customer.h"

#define BASELINE_START_RPM      (30)
#define BASELINE_STEP_RPM       (10)
#define BASELINE_MAX_RPM        (200)
#define BASELINE_MIN_RPM        (-200)
#define BASELINE_KEY_LOCK_MS    (250)

APPLYTypeDef xdata ApplyState;

static int16 data sg_iBaselineTargetRpm = 0;

static void Apply_ConfigSpeedLoop(void)
{
    mcFocCtrl.CtrlMode = SpeedLoopMode;
    mcFocCtrl.CurrentLoopEn = 1;

    if(mcFocCtrl.CurrentLoopEn)
    {
        s_PI.Uk_max = SOUTMAX_Cur;
        s_PI.Uk_min = SOUTMIN_Cur;   // 
    }
    else
    {
        s_PI.Uk_max = SOUTMAX;
        s_PI.Uk_min = SOUTMIN;
    }
}

static void Apply_SetBaselineSpeed(int16 target_rpm, uint8 reset_ramp)
{
    if(target_rpm > BASELINE_MAX_RPM)
    {
        target_rpm = BASELINE_MAX_RPM;
    }
    else if(target_rpm < BASELINE_MIN_RPM)
    {
        target_rpm = BASELINE_MIN_RPM;
    }

    sg_iBaselineTargetRpm = target_rpm;
    Apply_ConfigSpeedLoop();
    mcSpeedRamp.TargetValue = S_Value(target_rpm);

    if(reset_ramp)
    {
        mcSpeedRamp.ActualValue = 0;
    }

    mcSpeedRamp.FlagONOFF = 1;
}

static void Apply_HandleSpeedStep(int16 delta_rpm)
{
    uint8 reset_ramp;
    int16 next_rpm;

    reset_ramp = (mcSpeedRamp.FlagONOFF == 0) ? 1 : 0;

    if(sg_iBaselineTargetRpm == 0 && mcSpeedRamp.FlagONOFF == 0)
    {
        next_rpm = (delta_rpm > 0) ? BASELINE_START_RPM : -BASELINE_START_RPM;
    }
    else
    {
        next_rpm = sg_iBaselineTargetRpm + delta_rpm;
    }

    Apply_SetBaselineSpeed(next_rpm, reset_ramp);
}

void ApplyFun_Conrtrol(void)
{
    switch (ApplyState.SystemState)
    {
        case PowerOn:
            if(MRSyS.MRState == MRFinish && mcState == mcRun)
            {
                mcSpeedRamp.FlagONOFF = 0;
                sg_iBaselineTargetRpm = 0;
                ApplyState.SystemState = Run;
            }
            break;

        case Run:
            if(GP45 == 0)
            {
                Apply_HandleSpeedStep(BASELINE_STEP_RPM);
                ApplyState.SystemState = Wait;
                MRSyS.State_Count = BASELINE_KEY_LOCK_MS;
            }
            else if(GP33 == 0)
            {
                Apply_HandleSpeedStep(-BASELINE_STEP_RPM);
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
    }
}

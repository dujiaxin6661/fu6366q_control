/**
 * ============================================================================
 * @file      Protect.c
 * @brief     电机故障保护系统 —— 5段轮询检测 + 故障处理 + 自动恢复
 * @author    Fortiortech Application Team (Kobe Peng)
 * @since     2023-07
 * @note      本文件实现完整的故障检测与保护机制，在TickCycle_1ms中每个1ms调用一次：
 *
 *              故障检测（5段轮询，每1ms执行一段，5ms一个完整周期）：
 *                Segment 0: 软件过流检测（Is > OverSoftCurrentValue）
 *                Segment 1: 过压/欠压检测（母线电压超限）
 *                Segment 2: 启动失败检测（霍尔错误导致反转）
 *                Segment 3: 堵转检测（速度≈0但Q轴电压饱和）
 *                Segment 4: (保留)
 *
 *              故障处理：
 *                FaultProcess() → 关断PWM输出 → mcState=mcFault
 *
 *              自动恢复：
 *                过流恢复：在mcFault状态下计时OverCurrentRecoverTime后自动清除故障
 *
 *              全局变量：
 *                mcFaultDect (FaultVarible)  : 故障检测中间变量（计数器/标志/段号）
 *                mcFaultSource                : 当前故障源类型（枚举）
 * ============================================================================
 */

#include <Myproject.h>

FaultVarible        xdata       mcFaultDect;            /**< 故障检测变量：段号/计数器/标志位 */
FaultStateType      xdata       mcFaultSource;          /**< ★ 当前故障源（FaultNoSource=正常） */

/**
 * @brief      Fault_Detection() —— 故障检测5段轮询调度
 * @note       每1ms调用一次，segment变量在0~4之间循环，每5ms完成一轮全部检测
 *             优先检测过流（最快保护），其次是过欠压、启动、堵转
 */
void Fault_Detection(void)
{
    mcFaultDect.segment++;                               // 轮询段号+1

    if (mcFaultDect.segment >= 5)                        // 5段轮询，到5归0
        mcFaultDect.segment = 0;

    if (mcFaultDect.segment == 0)                        /* 段0：软件过流检测（最高频） */
    {
        if(OverSoftCurrentProtectEnable == 1)            // 过流保护使能（Customer_Debug.h中定义）
            Fault_OverCurrent();
    }
    else if (mcFaultDect.segment == 1)                   /* 段1：过压/欠压检测 */
    {
        if (VoltageProtectEnable == 1)                   // 电压保护使能
            Fault_OverUnderVoltage();
    }
    else if (mcFaultDect.segment == 2)                   /* 段2：启动失败检测 */
    {
        if(StartProtectEnable == 1)                      // 启动保护使能
            Fault_Start();
     }
    else if (mcFaultDect.segment == 3)                   /* 段3：堵转检测 */
    {
        if (StallProtectEnable == 1)                     // 堵转保护使能
            Fault_Stall();
    }
    else if (mcFaultDect.segment == 4)                   /* 段4：(保留扩展) */
    {

    }
}

/**
 * @brief      Fault_OverCurrentRecover() —— 软件过流自动恢复
 * @note       在过流故障状态下，每1ms累加恢复计数器
 *             达到OverCurrentRecoverTime后，CurrentPretectTimes+1
 *             若CurrentPretectTimes<5（最多可自动恢复5次），清除故障源回到mcReady
 *             第5次后不再自动恢复，需要断电重启
 */
void Fault_OverCurrentRecover(void)
{
    if ((mcState == mcFault) &&                                             // 必须在故障态
        ((mcFaultSource == FaultSoftOVCurrent) || (mcFaultSource == FaultHardOVCurrent)) &&
        (mcFaultDect.CurrentPretectTimes < 5))                              // 最多恢复5次
    {
        mcFaultDect.CurrentRecoverCnt++;

        if (mcFaultDect.CurrentRecoverCnt >= OverCurrentRecoverTime)        // 恢复延时到（默认1000ms×5=5s）
        {
            mcFaultDect.CurrentRecoverCnt = 0;
            mcFaultDect.CurrentPretectTimes++;                               // 恢复次数+1
            mcFaultSource = FaultNoSource;                                   // ★ 清除故障源 → 回到mcReady
        }
    }
}


/**
 * @brief      Fault_OverCurrent() —— 软件过流检测
 * @note       在运行态/启动态/预定位态中检测
 *             从FOC硬件读取IA和IBET，通过Atan_Us_MDU计算合成电流Is
 *             若Is > OverSoftCurrentValue（默认8A），累加过流计数器
 *             连续OverSoftCurrent_DectTime次超限 → 判定过流故障
 *             若电流恢复，计数器递减（迟滞防抖）
 */
void Fault_OverCurrent(void)
{
    if (mcState == mcRun || mcState == mcStart ||                           // 仅在电机转动态检测
        mcState == mcFirstAlign || mcState == mcSecondAlign)
    {
        Atan_Us_MDU(FOC__IA, FOC__IBET, mcFaultDect.Is);                   // 硬件MDU计算合成电流幅值

        if (mcFaultDect.Is >= OverSoftCurrentValue)                         // ★ 电流超过软件过流阈值（Q15格式）
        {
            mcFaultDect.OverCurCnt++;

            if (mcFaultDect.OverCurCnt >= OverSoftCurrent_DectTime)         // 持续超限 → 确认过流
            {
                mcFaultSource     = FaultSoftOVCurrent;                     // 故障源=软件过流
                FaultProcess();                                              // ★ 立即关闭输出
                mcFaultDect.OverCurCnt = 0;
            }
        }
        else if (mcFaultDect.OverCurCnt > 0)                                 // 电流恢复 → 递减计数（迟滞）
        {
            mcFaultDect.OverCurCnt--;
        }
    }

   #if (CurrentRecoverEnable)                                                // 过流自动恢复使能
    {
        Fault_OverCurrentRecover();
    }
    #endif
}


/**
 * @brief      Fault_OverUnderVoltage() —— 过压/欠压检测
 * @note       监测滤波后的母线电压（AdcSampleValue.ADCDcbusFlt，Q15格式）
 *             过压（>36V）：连续50次（250ms）→ 故障
 *             欠压（<12V）：连续50次（250ms）→ 故障
 *             电压在正常范围内时计数器递减（迟滞防抖）
 *             过欠压恢复功能当前已注释（需要时可打开）
 */
void Fault_OverUnderVoltage(void)
{
    if (mcFaultSource == FaultNoSource)                                      // 没有其他故障才检测
    {
        /* ── 过压检测：母线电压>OVER_PROTECT_VALUE（默认36V对应的Q15值）── */
        if (AdcSampleValue.ADCDcbusFlt > OVER_PROTECT_VALUE)
        {
            mcFaultDect.OverVoltDetecCnt++;

            if (mcFaultDect.OverVoltDetecCnt > 50)                           // 持续250ms → 确认过压
            {
                mcFaultDect.OverVoltDetecCnt = 0;
                mcFaultSource = FaultOverVoltage;                             // 故障源=过压
                FaultProcess();                                               // ★ 关闭输出
            }
        }
        else if (mcFaultDect.OverVoltDetecCnt > 0)                           // 电压恢复 → 递减
        {
            mcFaultDect.OverVoltDetecCnt--;
        }

        /* ── 欠压检测：母线电压<UNDER_PROTECT_VALUE（默认12V对应的Q15值）── */
        if (AdcSampleValue.ADCDcbusFlt < UNDER_PROTECT_VALUE)
        {
            mcFaultDect.UnderVoltDetecCnt++;

            if (mcFaultDect.UnderVoltDetecCnt > 50)                           // 持续250ms → 确认欠压
            {
                mcFaultDect.UnderVoltDetecCnt = 0;
                mcFaultSource = FaultUnderVoltage;                            // 故障源=欠压
                FaultProcess();                                               // ★ 关闭输出
            }
        }
        else if (mcFaultDect.UnderVoltDetecCnt > 0)
        {
            mcFaultDect.UnderVoltDetecCnt--;
        }
    }

    /******* 过压欠压自动恢复（当前已注释）*********/
    // 恢复条件：电压回到正常范围（OV_RECOVER < Vdc < UV_RECOVER）持续1s
}

/**
 * @brief      Fault_Start() —— 启动失败保护（霍尔传感器接线错误或角度偏置错误）
 * @note       在运行初期（RunTimeCnt<2000ms且>5ms）检测
 *             若Q轴电压很高（>1000或<-1000）但速度很低（<20RPM）→ 电机反转/堵转
 *             连续15次检测到此异常 → 启动失败故障
 *             这通常说明霍尔传感器信号与电机相序不匹配
 */
void Fault_Start(void)
{
    if((mcState == mcRun) && (mcFocCtrl.RunTimeCnt < 2000) && (mcFocCtrl.RunTimeCnt > 5))
    {
        /* 条件：Q轴电压>1000但速度<20RPM（正向打满但转不起来） */
        if(((FOC__UQ > 1000) && (mcFocCtrl.SpeedFlt < S_Value(-20))) ||      // 正向异常
           ((FOC__UQ < -1000) && (mcFocCtrl.SpeedFlt > S_Value(20))))        // 反向异常
        {
            mcFaultDect.HallErrCnt++;
            if(mcFaultDect.HallErrCnt > 15)                                   // 连续15次 → 确认故障
            {
                mcFaultDect.HallErrCnt = 0;
                mcFaultSource = FaultStart;                                    // 故障源=启动失败
                FaultProcess();                                                // ★ 关闭输出
            }
        }
        else
        {
            mcFaultDect.HallErrCnt = 0;                                        // 正常 → 清零
        }
    }
}

/**
 * @brief      Fault_Stall() —— 堵转保护
 * @note       仅在mcRun态检测
 *             条件：速度<最小堵转阈值 且 Q轴电压>PI上限-300（接近饱和）
 *             连续300次（1500ms）→ 堵转故障
 *             速度恢复时计数器递减（迟滞防抖）
 */
void Fault_Stall(void)
{
    if (mcState == mcRun)
    {
        if((Abs_F16(mcFocCtrl.SpeedFlt) < MOTOR_SPEED_STAL_MIN_RPM) &&       // 速度≈0
           (Abs_F16(FOC__UQ) > (s_PI.Uk_max - 300)))                         // Q轴电压接近饱和
        {
            mcFaultDect.StallCnt++;
            if(mcFaultDect.StallCnt > 300)                                    // 持续1500ms → 确认堵转
            {
                mcFaultDect.StallFlag = 1;                                    // 置位堵转标志
                mcFaultDect.StallCnt = 0;
                mcFaultSource = FaultStall;                                    // 故障源=堵转
                FaultProcess();                                                // ★ 关闭输出
            }
        }
        else
        {
            if(mcFaultDect.StallCnt > 0)                                       // 正常 → 递减
                mcFaultDect.StallCnt--;
        }
    }
    else
    {
        mcFaultDect.StallCnt = 0;                                              // 非运行态清零
    }
}


/**
 * @brief      Fault_Start_Process() —— 启动失败故障处理
 * @note       自动翻转FlagCircle（半圈偏移），尝试纠正霍尔角度错误
 *             如果之后仍失败，可能需要手动检查接线
 */
void Fault_Start_Process(void)
{
    switch (mcFaultSource)
    {
        case FaultStart:
            if(MR.FlagCircle)                                    // 如果FlagCircle=32768 → 清为0
                MR.FlagCircle = 0;
            else                                                  // 如果FlagCircle=0 → 设为32768（半圈偏移）
                MR.FlagCircle = 32768;

            mcFaultSource = FaultNoSource;                        // 清除故障源，回到mcReady重试
            break;
    }
}


/**
 * ============================================================================
 * @brief      FaultProcess() —— ★ 故障处理：立即关断PWM输出
 * @note       设置mcState=mcFault → FOC控制停止
 *             设置MOE=0 → 6路PWM全部关断（电机惯性滑行）
 *             故障源由各处检测函数在进入mcFault前设置
 * ============================================================================
 */
void FaultProcess(void)
{
    mcState = mcFault;                                       // ★ 状态机进入故障态
    MOE     = 0;                                              // ★ 立即关断PWM输出
}

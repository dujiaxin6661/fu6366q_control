/**
 * ============================================================================
 * @file      AddFunction.c
 * @brief     核心控制环函数 —— 力矩/速度/位置三种控制模式的分发与执行
 * @author    Fortiortech Application Team (Kobe Peng)
 * @since     2023-07
 * @note      本文件包含控制系统的核心运行时函数：
 *              - Speed_response()   : ★ 三种控制模式的数据流分发（由DRV_ISR每PWM周期调用）
 *              - Posi_response()    : 位置环PD控制器（由SYStick_INT每1ms调用）
 *              - mc_ramp()         : 速度斜坡函数（TargetValue按Inc/Dec渐进到ActualValue）
 *              - TickCycle_1ms()   : 1ms周期任务调度（状态计时/母线电压/故障检测/位置环）
 *              - TimeCountDec()    : 状态计时器递减
 *
 *              全局变量：
 *              - mcFocCtrl (FOCCTRL)     : 主控制结构体（CtrlMode/CurrentLoopEn/Iqref/TargetAngle等）
 *              - AdcSampleValue           : ADC采样值（母线电压/电流/温度）
 *              - mcSpeedRamp (MCRAMP)     : 速度斜坡结构体（TargetValue/ActualValue/IncValue/DecValue）
 *              - PI_Position (PI_TypeDef) : 位置环PD结构体（Err/ErrLast/Uk_max/Uk_min）
 * ============================================================================
 */

#include <FU68xx_2.h>
#include <Myproject.h>

/* Private variables ---------------------------------------------------------*/
FOCCTRL            xdata   mcFocCtrl;          /**< ★ 主控制结构体：集中管理所有控制参数和状态 */
ADCSample          xdata   AdcSampleValue;     /**< ADC采样值：母线电压/母线电流/MOS温度 */
MCRAMP             xdata   mcSpeedRamp;        /**< 速度斜坡结构体：TargetValue→ActualValue渐进 */
PI_TypeDef         idata   PI_Position;        /**< 位置环PI：Err(误差)/ErrLast(上次误差)/Kp/Kd */

extern uint16 xdata spidebug[4];

/**
 * @brief      Abs_F16 —— 16位有符号数取绝对值
 */
uint16 Abs_F16(int16 value)
{
    if (value < 0) { return (-value); }
    else           { return (value);  }
}

/**
 * @brief      Abs_F32 —— 32位有符号数取绝对值
 */
uint32 Abs_F32(int32 value)
{
    if (value < 0) { return (-value); }
    else           { return (value);  }
}

/*---------------------------------------------------------------------------*/
/* Name     :   void Speed_response(void)
/* Description: 速度响应函数，可根据需求加入控制环，如恒转矩控制、恒转速控制、恒功率控制,运行时间20us
/*---------------------------------------------------------------------------*/
/**
 * ============================================================================
 * @brief      Speed_response() —— ★ 控制环主分发函数（由DRV_ISR每个PWM周期调用）
 * @note       根据 mcFocCtrl.CtrlMode 选择3种控制模式之一：
 *
 *              【力矩控制模式 CurrentLoopMode】
 *                目标电流(mcFocCtrl.Iqref) → 直接赋值 FOC_IQREF → 硬件电流PI → SVPWM
 *                (最简单：直接Q轴电流控制，无外环)
 *
 *              【速度控制模式 SpeedLoopMode】
 *                目标速度→mc_ramp()斜坡→速度PI(HW_One_PI2)→FOC_IQREF→硬件电流PI→SVPWM
 *                (双环：外环速度PI + 内环电流PI)
 *
 *              【位置控制模式 PostionLoopMode】
 *                位置环输出(mcFocCtrl.PostionLoopOut) - 速度反馈→速度PI→FOC_IQREF→硬件电流PI→SVPWM
 *                (三环：外环位置PD(1ms) + 中环速度PI(41.7μs) + 内环电流PI(硬件))
 *
 *              速度PI使用硬件PI2（HW_One_PI2），增益来自s_PI（SKP/SKI/SKF）
 *              输出可走电流环(FOC_IQREF)或电压环(FOC__UQ)，由CurrentLoopEn选择
 * ============================================================================
 */
void Speed_response(void)
{
    if(mcSpeedRamp.FlagONOFF)                            // ★ 总开关：FlagONOFF==1才执行控制环
    {
        /* ── ★ 力矩控制模式：直接设定Q轴电流目标 ── */
        if(mcFocCtrl.CtrlMode == CurrentLoopMode)
        {
            if(mcFocCtrl.CurrentLoopEn)
            {
                FOC_IQREF = mcFocCtrl.Iqref;             // Q轴电流参考 = 用户设定值（标幺值）
                                                         // 正值=正转转矩，负值=反转转矩
            }
        }
        /* ── ★ 速度控制模式：斜坡+速度PI ── */
        else if(mcFocCtrl.CtrlMode == SpeedLoopMode)
        {
            mc_ramp();                                   // 速度斜坡：ActualValue→TargetValue渐进
            mcFocCtrl.SpeedErr  = mcSpeedRamp.ActualValue - mcFocCtrl.SpeedFlt;  // 速度误差=参考-反馈
        }
        /* ── ★ 位置控制模式：位置环输出作为速度给定 ── */
        else if(mcFocCtrl.CtrlMode == PostionLoopMode)
        {
            /*──  速度环：位置环输出(PostionLoopOut)作为速度参考，减去速度反馈 ──*/
            mcFocCtrl.SpeedErr  = mcFocCtrl.PostionLoopOut - mcFocCtrl.SpeedFlt;
        }

        /* ── ★ 配置硬件PI2参数（来自s_PI全局结构体）── */
        PI2_KP     = s_PI.Kp;                            // 速度环Kp（_Q12，默认2.2）
        PI2_KI     = s_PI.Ki;                            // 速度环Ki（_Q15，默认0.01）
        PI2_UKMAX  = s_PI.Uk_max;                        // PI输出上限
        PI2_UKMIN  = s_PI.Uk_min;                        // PI输出下限

        /* ── ★ 硬件PI2执行一次计算，输出Uk ── */
        mcFocCtrl.UQValueTemp = HW_One_PI2(mcFocCtrl.SpeedErr);  // 速度环PI输出

        /* ── 输出到电流环 或 电压环 ── */
        if(mcFocCtrl.CurrentLoopEn)                      // CurrentLoopEn==1 → 走电流内环
        {
            FOC_IQREF = mcFocCtrl.UQValueTemp;           // PI输出作为Q轴电流参考 → 硬件电流PI
        }
        else                                             // CurrentLoopEn==0 → 直接电压输出
        {
            FOC__UQ = mcFocCtrl.UQValueTemp;             // PI输出直接作为Q轴电压 → SVPWM
        }
    }
}

/*---------------------------------------------------------------------------
 * Name     :   void Posi_response(void)
 * Description: 位置环PD控制器（由SYStick_INT每1ms调用）,执行时间14.6us
 * @note       使用PD控制器（比例+微分），无积分项（避免位置过冲和积分饱和）
 *              计算流程：
 *                Err = _Q15Limit(TargetAngle - SensorAngle)  // 位置误差（32位→Q15限幅）
 *                if(|Err| < 10) Err = 0                      // ★ 死区±10：到达目标后停止
 *                Uk = PKP_Sw × Err                           // 比例项（_Q12乘法，>>12还原）
 *                Dk = PKD_Sw × (Err - ErrLast)               // 微分项（抑制过冲）
 *                PostionLoopOut = MaxMinLimit(POUTMAX, POUTMIN, Uk+Dk)  // 限幅到±300RPM
 *
 *              输出 mcFocCtrl.PostionLoopOut 会作为速度环的参考输入
 * ============================================================================
 */
void Posi_response(void)
{
    S32tS16 Temp;                                        // MDU乘法结果暂存（64位→2×32位）
    int32   Temp_Uk;                                     // 比例项计算结果
    int32   Temp_Dk;                                     // 微分项计算结果
    int32   Temp_Theta, Temp_Theta1;

    /* ── 双重读取确保数据一致性（防止中断中SensorAngle被更新）── */
    Temp_Theta = mcFocCtrl.SensorAngle.s32;              // 读取32位传感器角度
    Temp_Theta1 = mcFocCtrl.SensorAngle.s32;             // 再次读取
    if(Temp_Theta1 != Temp_Theta)                        // 两次不一致（发生了跨圈中断更新）
        Temp_Theta = mcFocCtrl.SensorAngle.s32;          // 第三次读取保证一致

    /* ── ★ 计算位置误差 = 目标位置 - 当前位置 ── */
    PI_Position.Err = _Q15Limit(mcFocCtrl.TargetAngle.s32 - Temp_Theta);  // 32位差→Q15限幅(±32767)

		/* 越小精度越高，但是可能会导致震荡 */
    if(PI_Position.Err <= 10 && PI_Position.Err >= -10)   // ★ 死区 ±10 LSB：到位后强制误差=0
        PI_Position.Err = 0;                              // 避免到位后微小振荡

    /* ── ★ Kp运算：比例项 = PKP_Sw × Err ── */
    MuiltS_MDU0(PKP_Sw, PI_Position.Err, Temp.s16[0], Temp.s16[1]);  // PKP_Sw(_Q12) × Err → 硬件MDU0
    Temp_Uk = Temp.s32 >> 12;                             // >>12恢复Q12乘法后的标度

    /* ── ★ Kd运算：微分项 = PKD_Sw × (Err - ErrLast) ── */
    MuiltS_MDU0(PKD_Sw, (PI_Position.Err - PI_Position.ErrLast), Temp.s16[0], Temp.s16[1]);
    Temp_Dk = Temp.s32 >> 12;                             // >>12恢复Q12乘法后的标度

    Temp_Uk += Temp_Dk;                                   // PD输出 = Kp项 + Kd项
    PI_Position.ErrLast = PI_Position.Err;                // 保存本次误差供下次计算微分

    /* ── ★ 位置环输出限幅 → 作为速度环的速度参考 ── */
    mcFocCtrl.PostionLoopOut = MaxMinLimit(
																					PI_Position.Uk_max, 
																					PI_Position.Uk_min, 
																					Temp_Uk);       // PD 算出的输出值
}                                                         // 限幅范围：POUTMAX ~ POUTMIN (默认±300RPM)


/**
 * @brief      mc_ramp() —— 速度斜坡函数
 * @note       将ActualValue向TargetValue平滑过渡
 *             加速时：ActualValue每次+IncValue直到达到TargetValue
 *             减速时：ActualValue每次-DecValue直到达到TargetValue
 *             IncValue/DecValue在VariablesPreInit中初始化为SPEED_INC/SPEED_DEC(各100)
 *             调用频率：每个PWM周期（24kHz），100LSB×24000=约2.4M LSB/s的斜坡速率
 */
void mc_ramp(void)
{
    if (mcSpeedRamp.ActualValue < mcSpeedRamp.TargetValue)       // 需要加速
    {
        if (mcSpeedRamp.ActualValue + mcSpeedRamp.IncValue < mcSpeedRamp.TargetValue)
        {
            mcSpeedRamp.ActualValue += mcSpeedRamp.IncValue;     // 完整步进
        }
        else
        {
            mcSpeedRamp.ActualValue = mcSpeedRamp.TargetValue;   // 直接到达目标
        }
    }
    else                                                         // 需要减速
    {
        if (mcSpeedRamp.ActualValue - mcSpeedRamp.DecValue > mcSpeedRamp.TargetValue)
        {
            mcSpeedRamp.ActualValue -= mcSpeedRamp.DecValue;     // 完整步退
        }
        else
        {
            mcSpeedRamp.ActualValue = mcSpeedRamp.TargetValue;   // 直接到达目标
        }
    }
}

/**
 * @brief      TimeCountDec() —— 状态计时器递减（由1ms节拍调用）
 * @note       递减两个计数器：
 *              - mcFocCtrl.State_Count : 电机状态机计时器（>0时每ms减1）
 *              - MRSyS.State_Count     : MR传感器状态机计时器
 *              - mcFocCtrl.RunTimeCnt  : 运行时间计数（递增，上限50000ms≈50s）
 */
void TimeCountDec(void)
{
    if (mcFocCtrl.State_Count > 0){
				mcFocCtrl.State_Count--;      // 电机计时器递减
		}      
    if (MRSyS.State_Count > 0){
				MRSyS.State_Count--;          // 传感器计时器递减
		}          
    if (mcState == mcRun)
    {
        if(mcFocCtrl.RunTimeCnt < 50000){
						mcFocCtrl.RunTimeCnt++;     // 运行时间累加（上限50s）
				}  
    }
}

/*---------------------------------------------------------------------------*/
/* Name     :  void  TickCycle_1ms(void)
/* Description:  默认1ms周期服务函数，运行信号采样，调速信号处理，闭环控制，故障检测,ATO爬坡函数
/*---------------------------------------------------------------------------*/
/**
 * @brief      TickCycle_1ms() —— 1ms周期任务（由SYStick_INT调用）
 * @note       执行顺序：
 *              1. TimeCountDec()            : 状态计时器递减
 *              2. 母线电压采样+低通滤波      : ADC2读取→LPF滤波系数30
 *              3. Fault_Detection()         : ★ 5段故障检测轮询
 *              4. Posi_response()           : ★ 位置环（仅位置模式+运行态）
 */
void  TickCycle_1ms(void)
{
    TimeCountDec();

    /* ── 母线电压采样 + 低通滤波（滤波系数30，平滑电压读数）── */
    AdcSampleValue.ADCDcbus = ADC2_DR;                                   // 读取ADC2通道（母线电压）
    AdcSampleValue.ADCDcbusFlt = LPFFunction0(AdcSampleValue.ADCDcbus,
                                               AdcSampleValue.ADCDcbusFlt, 30);  // 一阶低通滤波
    AdcSampleValue.ADCIbus = ADC3_DR;
    AdcSampleValue.ADCIbusFlt = LPFFunction0(AdcSampleValue.ADCIbus,
                                             AdcSampleValue.ADCIbusFlt, 30);
    AdcSampleValue.ADCTmos = ADC6_DR;
    AdcSampleValue.ADCTmosFlt = LPFFunction0(AdcSampleValue.ADCTmos,
                                             AdcSampleValue.ADCTmosFlt, 30);
    CAN_TelemetryTick1ms();

    /* ── ★ 故障保护检测：5段轮询（过流→过欠压→启动→堵转→保留）── */
    Fault_Detection();

    /* ── ★ 位置环PD计算（1kHz，仅位置模式+运行态）── */
    if(mcState == mcRun && ApplyState.SystemState != PowerOn)
    {
        if(mcFocCtrl.CtrlMode == PostionLoopMode)
        {
            Posi_response();                                             // 执行位置环PD
        }
    }
}

/**
 * @brief      FUNC_Control() —— 应用层控制入口（转发到按键/指令处理）
 */
void FUNC_Control(void)
{
    ApplyFun_Conrtrol();                                 // 见 ApplyFunction.c
}

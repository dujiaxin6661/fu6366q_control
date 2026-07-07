/**
 * ============================================================================
 * @file      FocControlFunction.c
 * @brief     FOC硬件配置与电机各阶段控制函数
 * @note      本文件包含FOC硬件寄存器配置和电机启动各阶段的执行函数：
 *              - FOC_Init()          : ★ 核心——配置FU68xx硬件FOC引擎的所有寄存器
 *              - Motor_Charge()      : 三步预充电（U→U+V→U+V+W，10%占空比）
 *              - Motor_Align()      : 预定位（D轴电流锁0°→90°→0°，两段法找零偏）
 *              - Motor_Open()       : 开环启动（角度斜坡加速）
 *              - GetCurrentOffset() : ADC电流偏置校准（上电执行~300次）
 *              - MC_Stop()         : 停止电机
 *              - MotorcontrolInit() : 控制变量初始化
 *              - VariablesPreInit() : 保护变量+位置环+斜坡初始化
 * ============================================================================
 */

#include <FU68xx_2.h>
#include <Myproject.h>

CurrentOffset xdata mcCurOffset;    /**< ADC电流偏置校准结构体（偏置值+累加器+标志位） */

/*---------------------------------------------------------------------------*/
/* Name     :   void FOC_Init(void)
/* Input    :   NO
/* Output   :   NO
/* Description: mcInit状态下，对FOC的相关寄存器进行配置,先清理寄存器，后配置，最后使能
/*---------------------------------------------------------------------------*/
/**
 * ============================================================================
 * @brief      FOC_Init() —— 配置FU68xx硬件FOC引擎的所有寄存器
 * @note       这是最核心的硬件配置函数，在多个阶段被调用（启动、预定位、开环）
 *             配置内容包括：
 *               1. PWM输出使能 + 互补模式配置
 *               2. FOC模块使能 + 母线电压ADC通道选择
 *               3. 速度滤波系数
 *               4. 清空所有FOC寄存器（CR1/CR2/IDREF/IQREF/THETA/RTHEACC/...）
 *               5. 电流环参数：D/Q轴PI的KP/KI + D/Q轴输出限幅(DOUTMAX/MIN, QOUTMAX/MIN)
 *               6. 估算器参数：EKP/EKI（观测器PLL PI增益）+ FBASE（电角度增量系数）+ 速度滤波
 *               7. 电压模式配置：SetBit(UQD) + SetBit(UDD) → 使用FOC__UD/FOC__UQ直接控制电压
 *               8. SVPWM模式 + 死区配置
 *               9. 旋转方向（DDIR）
 *               10. 过调制（可选）
 *               11. ★ 电流采样配置：分4种模式（单电阻/新单电阻/双电阻/三电阻）
 *                  每种模式配置采样窗口、触发延迟、死区补偿、下桥最小脉宽
 *                  并选择SVPWM段数（5段/7段）和采样周期数（1周期/2周期）
 *               12. ★ 电流偏置校正：将GetCurrentOffset()采集的偏置写入FOC硬件寄存器
 *               13. PWM来源切到FOC模块（OCS=1）
 *               14. 关闭估算器强制输出/强拉/角度控制模式
 * ============================================================================
 */

void FOC_Init(void)
{
    MOE = 0;                                            // ★ 先关闭PWM输出，防止配置过程中误触发
    DRV_CMR         = 0x0ABF;                           // UH/VH/WH UL/VL/WL 互补并使能（0x0ABF=1010_1111）

    /* ── 使能FOC模块 ── */
    SetBit(DRV_CR, FOCEN);

    #if (VOLTAGE_MODE == INTERNAL)                      // 如果使用内部分压测母线电压
    {
        SetBit(FOC_CR0, UCSEL);                         // UDC采样通道选择：0=AD2  1=AD15
    }
    #endif

    /* ── 速度滤波系数（Q8格式，1.0=不滤波）── */
    FOC_EOMEKLPF    = _Q8(1.0);                         // FOC内部估算速度的低通滤波系数

    /* ── ★ 清空所有FOC控制/状态寄存器 ── */
    FOC_CR1         = 0;                                // 控制寄存器1（采样模式、SVPWM等）
    FOC_CR2         = 0;                                // 控制寄存器2（电压/电流模式、采样顺序等）
    FOC_IDREF       = 0;                                // D轴电流参考值
    FOC_IQREF       = 0;                                // Q轴电流参考值
    FOC__THETA      = 0;                                // 电角度
    FOC_RTHEACC     = 0;                                // 爬坡初始加速度
    FOC__RTHESTEP   = 0;                                // 爬坡步长（角度增量）
    FOC_RTHECNT     = 0;                                // 爬坡计数
    FOC_THECOMP     = 0;                                // SMO估算角度补偿值
    FOC_THECOR      = 0x00;                             // 角度误差补偿（微调）

    /* ── ★ 电流环PI的D/Q轴输出限幅（来自Customer.h）── */
    FOC_DMAX        = DOUTMAX;                          // D轴输出上限（默认±60%占空比）
    FOC_DMIN        = DOUTMIN;                          // D轴输出下限
    FOC_QMAX        = QOUTMAX;                          // Q轴输出上限（默认±98%占空比）
    FOC_QMIN        = QOUTMIN;                          // Q轴输出下限

    /* ── 估算器使能 ── */
    SetBit(FOC_CR0, EDIS);                              // 使能估算器

    /* ── ★ 估算器PLL的PI增益（决定角度估算的响应速度）── */
    FOC_EKP = OBSW_KP_GAIN;                             // 估算器比例增益（_Q12格式）
    FOC_EKI = OBSW_KI_GAIN;                             // 估算器积分增益（_Q15格式）

    /* ── 速度→角度增量系数 + 速度低通滤波 ── */
    FOC_FBASE     = MR_FBASE1;                          // 将速度(标幺值)转换为每PWM周期的角度增量
    FOC_OMEKLPF   = SPEED_KLPF;                         // 估算器内部速度低通滤波系数

    /* ── ★ 电压控制模式：FOC__UD/FOC__UQ直接控制D/Q轴电压（非电流PI模式）── */
    SetBit(FOC_CR2, UQD);                               // Q轴：电压模式
    SetBit(FOC_CR2, UDD);                               // D轴：电压模式

    FOC__UQ = 0;                                        // Q轴电压清零
    FOC__UQ = 0;                                        // （原代码重复，保留不变）
    FOC_IDREF = 0;                                      // D轴电流参考清零
    FOC_IQREF = 0;                                      // Q轴电流参考清零

    /* ── SVPWM模式 + 死区 ── */
    FOC_TGLI      = PWM_TGLI_LOAD;                      // 窄脉宽消除+死区配置
    SetBit(FOC_CR1, SVPWMEN);                           // ★ 使能SVPWM（空间矢量调制）

    /* ── 旋转方向 ── */
    if(mcFocCtrl.FRStatus)
    {
        SetBit(DRV_CR, DDIR);                           // 设置反转标志（交换U/V相序）
    }

    /* ── (可选) 过调制 ── */
    #if (OverModulation == Enable)
    {
        SetBit(FOC_CR1, OVMDL);                         // 使能过调制（提高母线电压利用率~15%）
    }
    #endif

    /* ── ★★★ 电流采样模式配置 ★★★ ── */
    #if (Shunt_Resistor_Mode == New_Single_Resistor)     /* 新单电阻采样 */
    {
        SetReg(FOC_CR1, CSM0 | CSM1, CSM1);             // 采样模式选择
        FOC_TSMIN  = PWM_TS_LOAD;                       // 最小采样窗口（死区+0.9μs）
        FOC_TRGDLY = 0x0C;                              // ADC触发延迟12个clock（对齐PWM中点）
        ClrBit(FOC_CR2, F5SEG);                         // 7段SVPWM（单电阻仅支持7段）
    }
    #elif (Shunt_Resistor_Mode == Single_Resistor)       /* 单电阻采样 */
    {
        SetReg(FOC_CR1, CSM0 | CSM1, 0x00);             // 采样模式
        FOC_TSMIN  = PWM_TS_LOAD;                       // 最小采样窗口
        FOC_TRGDLY = 0x0C;                              // ADC触发延迟
        ClrBit(FOC_CR2, F5SEG);                         // 7段SVPWM
    }
    #elif (Shunt_Resistor_Mode == Double_Resistor)       /* ★ 当前使用：双电阻采样 */
    {
        SetReg(FOC_CR1, CSM0 | CSM1, CSM0);             // 双电阻采样模式
        FOC_TSMIN       = PWM_DT_LOAD;                  // 死区补偿值（作为采样窗口）
        FOC_TRGDLY      = 0x02;                         // ADC触发时刻在PWM计数器零点附近
        FOC_TBLO        = PWM_DLOWL_TIME;               // 下桥臂最小脉宽（保证足够采样时间）

        #if (SVPMW_Mode == SVPWM_7_Segment)
            ClrBit(FOC_CR2, F5SEG);                     // 7段SVPWM
        #elif (SVPMW_Mode == SVPWM_5_Segment)
            SetBit(FOC_CR2, F5SEG);                     // 5段SVPWM（开关损耗更低）
        #endif

        #if (DouRes_Sample_Mode == DouRes_1_Cycle)
            ClrBit(FOC_CR2, DSS);                       // 1周期采样完Ia和Ib
        #elif (DouRes_Sample_Mode == DouRes_2_Cycle)
            SetBit(FOC_CR2, DSS);                       // 2周期交替采样Ia/Ib
        #endif
    }
    #elif (Shunt_Resistor_Mode == Three_Resistor)        /* 三电阻采样 */
    {
        SetReg(FOC_CR1, CSM0 | CSM1, CSM0 | CSM1);      // 三电阻模式
        FOC_TSMIN  = PWM_DT_LOAD;                       // 死区补偿
        FOC_TRGDLY = 0x06;                              // ADC触发延迟
        FOC_TBLO = PWM_OVERMODULE_TIME;                  // 过调制时调整TB脉宽保证采样

        #if (SVPMW_Mode == SVPWM_7_Segment)
            ClrBit(FOC_CR2, F5SEG);
        #elif (SVPMW_Mode == SVPWM_5_Segment)
            SetBit(FOC_CR2, F5SEG);
        #endif

        #if (DouRes_Sample_Mode == DouRes_1_Cycle)
            ClrBit(FOC_CR2, DSS);
        #elif (DouRes_Sample_Mode == DouRes_2_Cycle)
            SetBit(FOC_CR2, DSS);
        #endif
    }
    #endif

    /* ── ★★★ 电流偏置校正：将GetCurrentOffset()采集的ADC偏置写入FOC硬件 ★★★ ── */
    #if (CalibENDIS == Enable)                           // 若使能了电流校正
    {
        if (mcCurOffset.OffsetFlag == 1)                 // 校正已完成
        {
            #if ((Shunt_Resistor_Mode == Single_Resistor)||(Shunt_Resistor_Mode == New_Single_Resistor))
            {
                SetReg(FOC_CR2, CSOC0 | CSOC1, 0x00);
                FOC_CSO = mcCurOffset.Iw_busOffset;     // 写入母线电流偏置
            }
            #elif (Shunt_Resistor_Mode == Double_Resistor)  /* ★ 当前使用 */
            {
                /* 写入IA相电流偏置 */
                SetReg(FOC_CR2, CSOC0 | CSOC1, CSOC0);
                FOC_CSO  = mcCurOffset.IuOffset;        // IA通道偏置值

                /* 写入IB相电流偏置 */
                SetReg(FOC_CR2, CSOC0 | CSOC1, CSOC1);
                FOC_CSO  = mcCurOffset.IvOffset;        // IB通道偏置值
            }
            #elif (Shunt_Resistor_Mode == Three_Resistor)
            {
                SetReg(FOC_CR2, CSOC0 | CSOC1, CSOC0);
                FOC_CSO = mcCurOffset.IuOffset;
                SetReg(FOC_CR2, CSOC0 | CSOC1, CSOC1);
                FOC_CSO = mcCurOffset.IvOffset;
                SetReg(FOC_CR2, CSOC0 | CSOC1, 0x00);
                FOC_CSO = mcCurOffset.Iw_busOffset;
            }
            #endif
        }
    }
    #endif

    /* ── ★ PWM来源切到FOC模块（OCS=1→使用SVPWM输出）── */
    SetBit(DRV_CR, DRVEN);                              // 使能PWM计数器
    SetBit(DRV_CR, OCS);                                // 输出控制源选FOC/SVPWM（而非DRV_COMR）

    /* ── 关闭估算器强制输出/强拉模式/角度控制 ── */
    ClrBit(FOC_CR1, EFAE);                              // 估算器不强制输出（正常模式）
    ClrBit(FOC_CR1, RFAE);                              // 禁止强拉模式
    ClrBit(FOC_CR1, ANGM);                              // 角度控制模式关闭
}

/*---------------------------------------------------------------------------*/
/* Name     :   void Motor_Charge(void)
/* Input    :   NO
/* Output   :   NO
/* Description: 预充电，当一直处于预充电状态下，不接电机，可用于验证IPM或者Mos。
预充电分三步，第一步是对U相进行预充电，第二步是对U,V两相进行预充电;第三步是对U、V、W三相进行预充电。
/*---------------------------------------------------------------------------*/
/**
 * @brief      Motor_Charge() —— 三步预充电（给自举电容充电）
 * @note       分三个阶段依次导通下桥臂，以10%占空比对母线电容充电：
 *              Step1: 只导通U相下桥臂（0x01）
 *              Step2: 导通U+V相下桥臂（0x01|0x04）
 *              Step3: 导通U+V+W相下桥臂（0x01|0x04|0x10）
 *              完成后恢复OCS=1，切回SVPWM模式
 *              时间分配：State_Count 从 Charge_Time 递减
 *                >2/3 Charge_Time → Step1
 *                >1/3 Charge_Time → Step2
 *                >0              → Step3
 */
void Motor_Charge(void)
{
    if (McStaSet.SetFlag.ChargeSetFlag == 0)             // 首次进入：初始化
    {
        McStaSet.SetFlag.ChargeSetFlag = 1;
        DRV_DR = 0.01 * DRV_ARR;                         // ★ 下桥臂10%占空比（低压预充电）
        ClrBit(DRV_CR, OCS);                             // PWM来源切到DRV_COMR（手动控制）
        mcFocCtrl.ChargeStep = 0;
     }

    /* Step 1: U相下桥臂导通（0x01 = bit0）*/
    if ((mcFocCtrl.State_Count < Charge_Time) && (mcFocCtrl.ChargeStep == 0))
    {
        mcFocCtrl.ChargeStep = 1;
        DRV_CMR |= 0x01;                                 // 使能U相下桥臂输出
        MOE = 1;                                         // 开启PWM输出
    }

    /* Step 2: U+V相下桥臂同时导通（0x01|0x04）*/
   else if ((mcFocCtrl.State_Count <= (Charge_Time/3.0*2)) && (mcFocCtrl.ChargeStep == 1))
    {
        mcFocCtrl.ChargeStep = 2;
        DRV_CMR |= 0x04;                                 // 使能V相下桥臂输出
    }

    /* Step 3: U+V+W三相下桥臂同时导通（0x01|0x04|0x10）*/
   else if ((mcFocCtrl.State_Count <= (Charge_Time/3.0*1)) && (mcFocCtrl.ChargeStep == 2))
    {
        mcFocCtrl.ChargeStep = 3;
        DRV_CMR |= 0x10;                                 // 使能W相下桥臂输出
    }

   else if(mcFocCtrl.ChargeStep == 3 && mcFocCtrl.State_Count == 0)
    {
        MOE = 0;                                         // 关闭输出
        SetBit(DRV_CR, OCS);                             // ★ PWM来源切回FOC/SVPWM
    }
}

/*---------------------------------------------------------------------------*/
/* Name     :   void Motor_Align(void)
/* Input    :   NO
/* Output   :   NO
/* Description: 预定位函数，当无逆风判断时，采用预定位固定初始位置;当有逆风判断时，采用预定位刹车
/*---------------------------------------------------------------------------*/

/**
 * ============================================================================
 * @brief      Motor_Align() —— 预定位/角度零偏校准
 * @note       使用两段定位法（仅在mcFocCtrl.AlignMode==1时执行）：
 *              阶段1: D轴电压锁到0°，保持Align_Time的2/3时间
 *              阶段2: 读到0°对应的传感器角度后，切换到90°电角度
 *              阶段3: 验证角度差≈90°（误差<±40°），再切回0°
 *              阶段4: 计算零偏 = (0°时传感器角度 + 反转后0°时传感器角度)/2
 *                     将该零偏存入mcFocCtrl.SensorAngleOffset，并写入Flash
 *
 *              如果AlignMode==0（单次定位），则在mcFirstAlign中只锁到0°不动
 *
 *              预定位期间使用专用的定位PI增益（DQKP_Alignment/DQKI_Alignment）
 *              D轴电压为0.15占空比（_Q15(0.15)），提供足够的锁定电流但不过大
 * ============================================================================
 */

void Motor_Align(void)
{
    static int16  Temp_Align_Angle;                      // 暂存0°预定位时的FOC角度

    if (McStaSet.SetFlag.AlignSetFlag == 0)              // ★ 首次进入：配置预定位参数
    {
        McStaSet.SetFlag.AlignSetFlag = 1;
        mcFocCtrl.AlignFlag = 0;
        mcFocCtrl.State_Count = Align_Time;              // 设置预定位时间（默认5000ms）

        FOC_Init();                                      // ★ 重新初始化FOC硬件（清空寄存器）

        /* ── 使用预定位专用的PI增益（比运行增益小，防止过冲）── */
        FOC_QKP = DQKP_Alignment;                        // D轴PI的Kp（预定位专用）
        FOC_DKP = DQKP_Alignment;
        FOC_QKI = DQKI_Alignment;                        // D轴PI的Ki（预定位专用）
        FOC_DKI = DQKI_Alignment;

        /* ── 电压模式：D轴施加0.15占空比 → 产生锁定转矩 ── */
        SetBit(FOC_CR2, UQD);
        SetBit(FOC_CR2, UDD);
        FOC__UQ = 0;                                     // Q轴电压=0
        FOC__UD = _Q15(0.15);                            // D轴电压=15%占空比（提供定位电流）

        /* ── 锁到0°电角度 ── */
        FOC__THETA  =  Align_Theta_0;                    // 目标电角度=0°

        /* ── 使能三相输出 ── */
        DRV_CMR |= 0x3F;                                 // U/V/W三相全输出（0x3F=0011_1111）
        MOE = 1;                                         // 开启PWM
    }
    else                                                 // ★ 后续执行：两段定位流程
    {
      if(mcFocCtrl.AlignMode)                            // AlignMode==1 → 两段式定位
      {
            /* ★ 阶段1：0°定位保持2/3时间后，记录传感器角度，切到90° */
            if((mcFocCtrl.AlignFlag == 0) && (mcFocCtrl.State_Count < (Align_Time/3.0*2)))
            {
                Temp_Align_Angle = mcFocCtrl.FOCTheta;    // 读取0°时对应的传感器角度
                mcFocCtrl.AlignFlag = 1;
                mcFocCtrl.AlignTheta1 = mcFocCtrl.SensorAngle.s16[1];  // 记录第一次对准的传感器角度

                FOC__THETA = Align_Theta_90;             // ★ 切换到90°电角度定位
            }
            /* ★ 阶段2：验证角度变化≈90°（允许±40°误差），再切回0° */
            else if((mcFocCtrl.AlignFlag == 1) && (mcFocCtrl.State_Count < (Align_Time/3.0*1)))
            {
                /* 判断：当前FOC角度 - 之前0°角度 - 90°理论值 的绝对值 < 40° */
                if(Abs_F16(mcFocCtrl.FOCTheta - Temp_Align_Angle - _Q15(90.0/180)) < _Q15(40.0/180))
                {
                    Temp_Align_Angle = mcFocCtrl.FOCTheta;        // 记录90°时的角度
                    FOC__THETA = Align_Theta_0;                   // ★ 切回0°
                    mcFocCtrl.AlignFlag = 2;
                }
                else
                {
                    mcFaultSource = FaultAlign;                   // 预定位失败 → 进入故障
                    FaultProcess();
                    mcState = mcFault;
                }
            }
            /* ★ 阶段3：计时结束后，验证从90°回到0°的角度变化也≈90° */
            else if((mcFocCtrl.AlignFlag == 2) && (mcFocCtrl.State_Count == 0))
            {
                if(Abs_F16(Temp_Align_Angle - mcFocCtrl.FOCTheta - _Q15(90.0/180)) < _Q15(40.0/180))
                {
                    Temp_Align_Angle = mcFocCtrl.FOCTheta;
                    mcFocCtrl.AlignTheta2 = mcFocCtrl.SensorAngle.s16[1];  // 记录第二次的传感器角度
                    mcFocCtrl.AlignFlag = 3;
                }
                else
                {
                    mcFaultSource = FaultAlign;
                    FaultProcess();
                    mcState = mcFault;
                }
            }
            /* ★ 阶段4：计算零偏 + 写入Flash */
            else if(mcFocCtrl.AlignFlag == 3)
            {
                FOC_IDREF = 0;                           // 清零电流参考
                FOC_IQREF = 0;

                MR.AngleOffsetInitFlag = 1;              // 标记角度零偏已校准

                /* 零偏 = (第一次传感器角度 + 第二次传感器角度)/2，取模65535 */
                mcFocCtrl.SensorAngleOffset = (int32)((int32)mcFocCtrl.AlignTheta1 + mcFocCtrl.AlignTheta2) % 65535 / 2;

                OffsetWrite();                            // ★ 将校准结果写入Flash永久保存
            }
        }
    }
}


/*---------------------------------------------------------------------------*/
/* Name     :   void Motor_Open(void)
/* Input    :   NO
/* Output   :   NO
/* Description: 开环启动的参数配置
/*---------------------------------------------------------------------------*/
/**
 * @brief      Motor_Open() —— 开环启动配置
 * @note       在mcStart状态下调用：
 *              - 重新FOC_Init()，使用运行PI增益（DQKP/DQKI）
 *              - 如果是AngleOpen模式：设置FOC__RTHESTEP=Open_Speed（开环角度爬坡）
 *              - 如果是AngleLoop模式：爬坡步长=0（由MR传感器角度直接闭环）
 *              - 电压模式：D轴=0.15占空比保持励磁，等待校准完成
 *              - 最后使能输出 MOE=1
 */
void Motor_Open(void)
{
    if (McStaSet.SetFlag.StartSetFlag == 0)              // 首次进入
    {
        McStaSet.SetFlag.StartSetFlag = 1;

        FOC_Init();                                      // ★ 重新初始化FOC

        /* ── 使用运行PI增益（非预定位增益）── */
        FOC_QKP = DQKP;                                  // 电流环PI - Kp（Q12格式，默认2.0）
        FOC_DKP = DQKP;
        FOC_QKI = DQKI;                                  // 电流环PI - Ki（Q15格式，默认0.03）
        FOC_DKI = DQKI;

        /* ── 开环角度爬坡速度 ── */
        if(mcFocCtrl.AngleMode == AngleOpen)
            FOC__RTHESTEP = Open_Speed;                  // ★ 角度开环：每PWM周期增加的角度步长
        else
            FOC__RTHESTEP = 0;                           // 角度闭环：不需要爬坡

        /* ── 关闭估算器强制输出/强拉/角度控制 ── */
        ClrBit(FOC_CR1, EFAE);
        ClrBit(FOC_CR1, RFAE);
        ClrBit(FOC_CR1, ANGM);

        /* ── 启动时的电压/电流配置 ── */
        if(mcFocCtrl.AngleMode == AngleOpen)             // 角度开环模式
        {
            SetBit(FOC_CR2, UQD);                        // 电压模式
            SetBit(FOC_CR2, UDD);
            FOC__UQ = 0;
            FOC__UD = _Q15(0.15);                        // D轴15%占空比（提供励磁）
        }
        else                                             // 角度闭环模式
        {
            if(mcFocCtrl.CurrentLoopEn)
            {
                ClrBit(FOC_CR2, UQD);                    // 电流模式
                ClrBit(FOC_CR2, UDD);
                FOC_IDREF = 0;
                FOC_IQREF = 0;
            }
            else
            {
                SetBit(FOC_CR2, UQD);                    // 电压模式
                SetBit(FOC_CR2, UDD);
                FOC__UD = 0;
                FOC__UQ = 0;
            }
        }
    }

    MOE = 1;                                             // ★ 使能PWM输出
}


/*---------------------------------------------------------------------------*/
/* Name     :   void MC_Stop(void)
/* Description: 停止电机——关闭PWM输出，回到初始化状态
/*---------------------------------------------------------------------------*/
void MC_Stop(void)
{
    MOE     = 0;                                         // ★ 立即关断PWM输出
    SetBit(DRV_CR, FOCEN);                               // 保持FOC使能
    mcState = mcInit;                                    // 状态回到mcInit
}

/**
 * @brief      MotorcontrolInit() —— 控制变量初始化（在SoftwareInit中调用）
 * @note       初始化：状态机标志清零、电流偏置累加器初值16383（ADC中值）、
 *              PI参数赋值（SKP/SKI/SKF）、Flash安全密钥
 */
void MotorcontrolInit(void)
{
    /* ── 状态机时序变量清零 ── */
    McStaSet.SetMode                   = 0;

    /* ── 电流偏置校准累加器初始值 = 16383（16位ADC中值）── */
    mcCurOffset.IuOffsetSum            = 16383;
    mcCurOffset.IvOffsetSum            = 16383;
    mcCurOffset.Iw_busOffsetSum        = 16383;

    /* ── 速度环PI初始化（使用Customer.h中定义的SKP/SKI/SKF）── */
    memset(&s_PI, 0, sizeof(PI_Typedef));                // PI结构体清零
    s_PI.Kp = SKP;                                       // 速度环Kp（_Q12格式）
    s_PI.Ki = SKI;                                       // 速度环Ki（_Q15格式）
    s_PI.Kf = SKF;                                       // 速度环Kf（前馈系数，暂未使用）
    s_PI.Uk_max = SOUTMAX;                               // PI输出上限（占空比模式）
    s_PI.Uk_min = SOUTMIN;                               // PI输出下限
    s_PI.Ki_max = SOUTMAX * 1.2;                          // 积分项上限（比输出限幅高20%防windup）
    s_PI.Ki_min = SOUTMIN * 1.2;                          // 积分项下限

    /* ── Flash安全密钥（擦写Flash时必须先写入这两个key）── */
    FlashKey.x5a = 0x5a;
    FlashKey.x1f = 0x1f;
}

/**
 * @brief      VariablesPreInit() —— 运行前变量初始化
 * @note       清零：故障源/故障检测变量/位置环PI/斜坡增量
 *             设置：位置环输出限幅=POUTMAX/POUTMIN（±300RPM）
 *                   斜坡增量=SPEED_INC(100)，减量=SPEED_DEC(100)
 *                   状态标志=0x01（只置位CalibFlag）
 */
void VariablesPreInit(void)
{
    /*********** 保护变量清零 ******************/
    mcFaultSource = 0;                                   // 故障源清零
    memset(&mcFaultDect, 0, sizeof(FaultVarible));       // 故障检测结构体全部清零

	// 这两个参数都在customer.h文件中今次那个了预定义，初始化时赋值一次，当按键按下的时候覆盖初始值
    /***** 位置环PI初始化 *******/
    memset(&PI_Position, 0, sizeof(PI_TypeDef));         // 位置环PI清零
    PI_Position.Uk_max = POUTMAX;                        // 位置环输出上限（正向最大速度）
    PI_Position.Uk_min = POUTMIN;                        // 位置环输出下限（反向最大速度）

    /***** 速度斜坡参数 *******/
    mcSpeedRamp.IncValue = SPEED_INC;                    // 加速增量（默认100 LSB/周期）
    mcSpeedRamp.DecValue = SPEED_DEC;                    // 减速减量（默认100 LSB/周期）

    /***** 状态标志位：仅位置0（CalibFlag）=1，其余清零 *******/
    McStaSet.SetMode  = 0x01;                            // bit0=CalibFlag=1（进入mcReady时已校准完成）

    mcFocCtrl.SpeedFlt = 0;                              // 滤波后速度初始化为0
}

/*---------------------------------------------------------------------------*/
/* Name     :   void GetCurrentOffset(void)
/* Description: 上电时，先对硬件电路的电流进行采集，写入对应的校准寄存器中。
               调试时，需观察mcCurOffset结构体中对应变量是否在范围内。采集结束后，OffsetFlag置1。
/*---------------------------------------------------------------------------*/
/**
 * @brief      GetCurrentOffset() —— ADC电流偏置自动校准
 * @note       在主循环中持续调用，直到OffsetFlag==1（校准完成）
 *             采集Calib_Time次（默认300次），累加后右移4位（÷16）得到平均值
 *             使用 &0x7ff8 掩码去掉低3位噪声
 *             预期偏置值在16383±2000范围内（16位ADC中值=16383）
 *             校准完成后OffsetFlag置1，后续调用直接跳过
 *
 *             ★ 重要：如果偏置值偏差太大会影响电流采样精度，导致FOC控制异常
 */
void GetCurrentOffset(void)
{
    if (!mcCurOffset.OffsetFlag)                         // 仅在校准未完成时执行
    {
        SetBit(ADC_CR, ADCBSY);                          // 触发一次ADC采样
        while (ReadBit(ADC_CR, ADCBSY));                 // 等待采样完成

        #if ((Shunt_Resistor_Mode == Single_Resistor)||(Shunt_Resistor_Mode == New_Single_Resistor))
            /* 单电阻模式：只校准母线电流(Ibus) */
            mcCurOffset.Iw_busOffsetSum += ((ADC4_DR & 0x7ff8));     // 累加，去掉低3位
            mcCurOffset.Iw_busOffset     = mcCurOffset.Iw_busOffsetSum >> 4;  // 右移4位=÷16
            mcCurOffset.Iw_busOffsetSum -= mcCurOffset.Iw_busOffset;            // 减去已取出的值

            mcCurOffset.Ibus_FltOffsetSum += ((ADC11_DR & 0x7ff8));
            mcCurOffset.Ibus_FltOffset     = mcCurOffset.Ibus_FltOffsetSum >> 4;
            mcCurOffset.Ibus_FltOffsetSum -= mcCurOffset.Ibus_FltOffset;
        #elif (Shunt_Resistor_Mode == Double_Resistor)   /* ★ 当前使用：双电阻模式 */
            /* 校准IA相电流偏置（ADC0通道）*/
            mcCurOffset.IuOffsetSum     += ((ADC0_DR & 0x7ff8));
            mcCurOffset.IuOffset         = mcCurOffset.IuOffsetSum >> 4;
            mcCurOffset.IuOffsetSum     -= mcCurOffset.IuOffset;

            /* 校准IB相电流偏置（ADC1通道）*/
            mcCurOffset.IvOffsetSum     += ((ADC1_DR & 0x7ff8));
            mcCurOffset.IvOffset         = mcCurOffset.IvOffsetSum >> 4;
            mcCurOffset.IvOffsetSum     -= mcCurOffset.IvOffset;

            /* 校准母线电流偏置（ADC3通道）*/
            mcCurOffset.Ibus_FltOffsetSum += ((ADC3_DR & 0x7ff8));
            mcCurOffset.Ibus_FltOffset     = mcCurOffset.Ibus_FltOffsetSum >> 4;
            mcCurOffset.Ibus_FltOffsetSum  -= mcCurOffset.Ibus_FltOffset;
        #elif (Shunt_Resistor_Mode == Three_Resistor)
            mcCurOffset.IuOffsetSum     += ((ADC0_DR & 0x7ff8));
            mcCurOffset.IuOffset         = mcCurOffset.IuOffsetSum >> 4;
            mcCurOffset.IuOffsetSum     -= mcCurOffset.IuOffset;
            mcCurOffset.IvOffsetSum     += ((ADC1_DR & 0x7ff8));
            mcCurOffset.IvOffset         = mcCurOffset.IvOffsetSum >> 4;
            mcCurOffset.IvOffsetSum     -= mcCurOffset.IvOffset;
            mcCurOffset.Iw_busOffsetSum += ((ADC4_DR & 0x7ff8));
            mcCurOffset.Iw_busOffset     = mcCurOffset.Iw_busOffsetSum >> 4;
            mcCurOffset.Iw_busOffsetSum -= mcCurOffset.Iw_busOffset;
        #endif

        mcCurOffset.OffsetCount++;                       // 校准计数器+1

        if (mcCurOffset.OffsetCount > Calib_Time)        // ★ 达到校准次数（默认300次）
        {
            mcCurOffset.OffsetFlag = 1;                  // ★ 校准完成标志置1
        }
    }
}

/**
 * @brief      Motor_Ready() —— 就绪态初始化
 * @note       首次进入时：使能FOC模块，关闭PWM输出（安全状态）
 */
void Motor_Ready(void)
{
    if (McStaSet.SetFlag.CalibFlag == 0)
    {
        McStaSet.SetFlag.CalibFlag = 1;
        SetBit(DRV_CR, FOCEN);                           // 使能FOC模块
        MOE = 0;                                         // 确保PWM输出关闭
    }
}

/**
 * @brief      Motor_Init() —— 初始化态：变量+PI初始化
 */
void Motor_Init(void)
{
    VariablesPreInit();                                  // 保护变量、位置环、斜坡参数初始化
    PI0_Init();                                          // 硬件PI0（角速度环PI）初始化
}

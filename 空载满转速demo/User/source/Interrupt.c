/**
 * ============================================================================
 * @file      Interrupt.c
 * @brief     中断服务程序（ISR）集合 —— FOC控制所有实时响应的入口
 * @author    Fortiortech Application Team (Kobe Peng)
 * @since     2023-07
 * @note      本文件包含6个中断服务函数，按优先级从高到低：
 *              interrupt 0  : LVW_TSD_INT   — 低电压/热关断警告
 *              interrupt 1  : FO_INT        — 硬件FO过流保护（已禁用，改用CMP3）
 *              interrupt 3  : DRV_ISR       — ★ FOC核心中断（每PWM周期，24kHz≈41.7μs）
 *              interrupt 8  : CAN_INT       — CAN总线收发中断
 *              interrupt 10 : SYStick_INT   — 1ms系统节拍（保护检测、位置环）
 *              interrupt 12 : CMP3_INT      — 硬件比较器过流保护（优先级最高）
 * ============================================================================
 */

#include <Myproject.h>
#include <Customer_Debug.h>

/** SPI调试缓冲区（4通道）：[FOC__ETHETA, FOC__UD, FOC__UQ, FOC__THETA] */
uint16 xdata spidebug[4] = { 0 };

/**
 * @brief      LVW & TSD 中断服务 (interrupt 0)
 * @note       低电压警告(LVW)和热关断警告(TSD)
 *             优先级最低，仅做标志清除，不参与电机控制
 */
void LVW_TSD_INT(void) interrupt 0  //LVW & TSD interrupt
{
    if (ReadBit(LVSR, LVWIF))
    {
        if (ReadBit(LVSR, LVWF))
        {
            #if 0
            XorBit(P0, P03);          // 调试用：翻转GPIO指示LVW状态
            #endif
            ClrBit(LVSR, LVWF);       // 清除低电压警告标志
        }

        ClrBit(LVSR, LVWIF);          // 清除LVW中断标志
    }

    if (TSDIF)
    {
        TSDIF = 0;                    // 清除热关断中断标志
    }
}

/*-------------------------------------------------------------------------------------------------
    Function Name : void FO_INT(void)
    Description   : FO_INT interrupt，硬件FO过流保护，关断输出，中断优先级最高
    Input         : 无
    Output        : 无
    @note          当前已被 #if 0 禁用，改用 CMP3_INT (interrupt 12) 做硬件过流保护
                   因为CMP3响应更快且不依赖FOC模块
-------------------------------------------------------------------------------------------------*/
#if 0
void FO_INT(void) interrupt 1               // 硬件FO过流中断，关闭输出
{
    if (IF0)
    {
        mcFaultSource = FaultHardOVCurrent; // 记录故障源：硬件过流
        FaultProcess();                     // 立即关闭PWM输出（MOE=0）
        mcState = mcFault;                  // 状态机进入故障态
        IF0 = 0;                            // 清除P00中断标志
    }
}
#endif


/**
 * ============================================================================
 * @brief      DRV_ISR —— FOC驱动中断 (interrupt 3)  ★ 最核心的中断
 * @note       每个PWM载波周期触发一次（24kHz ≈ 41.7μs）
 *             中断优先级第二高（仅次于CMP3硬件过流保护）
 *             执行流程：
 *               1. 读取MR传感器Sin/Cos的ADC值并减去偏置
 *               2. (可选) Sin/Cos峰值归一化 —— 修正传感器幅值不对称
 *               3. 送入FOC硬件：FOC__EALP=Cos, FOC__EBET=Sin
 *               4. 根据传感器状态机做不同处理：
 *                  - 校准态(MRCalibra)：调用MR_Offset_Calibration_GETDATA()采集偏置数据
 *                  - 运行态：计算电角度→更新FOC__THETA→记录旋转圈数
 *               5. 读取速度反馈 FOC__OMEEST，调用 Speed_response() 执行控制环
 *               6. (可选) 将调试数据写入 spidebug[] 供SPI输出
 *             使用了MDU2（乘法除法单元2），注意主循环用MDU0，避免冲突
 * ============================================================================
 */
void DRV_ISR(void) interrupt 3
{
    static  uint8 SpeedClcCnt = 0;      // 速度计算分频计数器（每10次PWM计算一次速度方向）
    int16 data Temp;

    GP04 = 1;                           // ★ GPIO调试标记：DRV_ISR入口（示波器可测中断执行时间）

    if (ReadBit(DRV_SR, DCIF))          // 等待DC比较中断标志置位
    {
        /* --- 触发一次ADC采样，等待完成 --- */
        SetBit(ADC_CR, ADCBSY);
        while (ReadBit(ADC_CR, ADCBSY));

        /* --- 读取MR传感器Sin/Cos原始值，减去已校准的偏置 --- */
        MR.iSin = ADC4_DR - MR.SinOffset;    // ADC4通道：Sin信号（经过偏置校正）
        MR.iCos = ADC12_DR - MR.CosOffset;   // ADC12通道：Cos信号（经过偏置校正）

        /* --- (可选) Sin/Cos峰值归一化，修正传感器幅值不对称 --- */
        #if (MR_NormaLizationEnable)

            if(MROffset.iPeakNorlizationFinshFlag)
            {
                /***归一化处理****/
                if(MROffset.iPeakNorlizationFlag == 0) // 以Sin幅度为基准，调整Cos
                {
                    MuiltS1_H_MDU2(MR.iCos, (MROffset.iPeakNorlizationCoff>>1), MROffset.iPeakNorDeltaVlaue);
                    MR.iCos  = MR.iCos + MROffset.iPeakNorDeltaVlaue;
                }
                else // 以Cos幅度为基准，调整Sin
                {
                    MuiltS1_H_MDU2(MR.iSin, (MROffset.iPeakNorlizationCoff>>1), MROffset.iPeakNorDeltaVlaue);
                    MR.iSin = MR.iSin + MROffset.iPeakNorDeltaVlaue;
                }
            }
        #endif

        /* --- 将校正后的Sin/Cos值送入FOC硬件模块（用于后续角度估算）--- */
        FOC__EALP = MR.iCos;            // α轴分量 = Cos
        FOC__EBET = MR.iSin;            // β轴分量 = Sin

        if(MRSyS.MRState == MRCalibra)                      // ★ MR传感器校准状态
        {
            MR_Offset_Calibration_GETDATA();                 // 采集一个PWM周期的Sin/Cos偏置数据

            if(++SpeedClcCnt > 10)                           // 每10次PWM（~417μs）计算一次
            {
                SpeedClcCnt = 0;

                /* 用硬件MDU2计算反正切，获取当前180°范围内的角度 */
                Atan_Theta_MDU2(MR.iCos, MR.iSin, MR.Theta180);

                /* 根据角度变化判断电机旋转方向 */
                Temp = ((int32)MR.Theta180 - MR.Theta180Old) % 65535;
                if(Temp > 0)                                 // 角度递增 → 正转
                {
                    if(MR.DirCnt < 30000)
                        MR.DirCnt++;                         // 正转计数+
                }
                else                                         // 角度递减 → 反转
                {
                    if(MR.DirCnt > (-30000))
                        MR.DirCnt--;                         // 反转计数-
                }
                MR.Theta180Old = MR.Theta180;                // 保存本次角度，供下次比较
            }
        }
        else if(MRSyS.MRState != MRWait)                     // ★ 非等待状态（正常运行/就绪）
        {
            /*----------------------- 角度计算 --------------------------*/
            /* 从FOC硬件读出电角度，减去霍尔偏置，得到MR传感器机械角度 */
            MR.Theta180 = FOC__ETHETA - MR.HallAngleOffset;

            /*----------------------- Hall相位校准 --------------------------*/
            #if (HALLNUM == TWOHALL)
                if(MRSyS.MRState == MRHallOffset)
                {
                    Hall_MR_Offset_Check();                 // 霍尔偏置校准（找霍尔跳变沿对应的MR角度）
                }
            #endif

            /* --- 360°圆圈跟踪：检测角度是否跨过0°/360°边界 --- */
            /* 5536 ≈ 30°, 60000 ≈ 330°（在16位角度空间 0~65535） */
            if(((MR.Theta180 < 5536) && (MR.Theta180Old > 60000)) ||
               ((MR.Theta180 > 60000) && (MR.Theta180Old < 5536)))
            {
                MR.FlagCircle ^= 0x8000;                    // 跨圈时翻转半圈标志（32768=0x8000）
            }
            MR.Theta180Old = MR.Theta180;

            /* --- 构造32位角度值 --- */
            /* s16[1] = 低16位：0~65535 的360°单圈机械角度 */
            /* s16[0] = 高16位：±32767 的累积圈数 */
            mcFocCtrl.SensorAngle.s16[1] = (MR.Theta180 >> 1) + MR.FlagCircle;   // /2得到360°范围
            mcFocCtrl.MotorAngle360 = mcFocCtrl.SensorAngle.s16[1] - mcFocCtrl.SensorAngleOffset;

            /* 电角度 = 机械角度 × 极对数（硬件MDU2做乘法） */
            Muilt_L_MDU2(mcFocCtrl.MotorAngle360, Pole_Pairs, mcFocCtrl.FOCTheta);

            /* --- 运行态时，将计算好的电角度写入FOC硬件 --- */
            if((ApplyState.SystemState != PowerOn) && (mcState == mcRun))
            {
                FOC__THETA = mcFocCtrl.FOCTheta;
            }

            /****************************** 记录旋转圈数（32位累计）*******************************************/
            /* 当传感器单圈角度跨过零点时，s16[0]（圈数计数器）+1或-1 */
            if((((uint16)mcFocCtrl.SensorAngle.s16[1]) < 5536) && (mcFocCtrl.SensorTheta360Old > 60000))
            {
                if(mcFocCtrl.SensorAngle.s16[0] < 30000)     // 正向圈数上限 ±30000
                    mcFocCtrl.SensorAngle.s16[0]++;          // 正转1圈
            }
            else if((((uint16)mcFocCtrl.SensorAngle.s16[1]) > 60000) && (mcFocCtrl.SensorTheta360Old < 5536))
            {
                if(mcFocCtrl.SensorAngle.s16[0] > -30000)    // 反向圈数下限
                    mcFocCtrl.SensorAngle.s16[0]--;          // 反转1圈
            }
            mcFocCtrl.SensorTheta360Old = mcFocCtrl.SensorAngle.s16[1];  // 保存本次单圈角度
        }

        /****************** 速度反馈与控制环响应 ***********************************************/
        /* 仅在非PowerOn状态且电机运行中（或停止中）才执行控制环 */
        if((ApplyState.SystemState != PowerOn) && (mcState == mcRun || mcState == mcStop))
        {
            mcFocCtrl.SpeedFlt = FOC__OMEEST;               // 从FOC硬件读取估算的机械速度（标幺值）

            if(mcFocCtrl.SpeedFlt > -20 && mcFocCtrl.SpeedFlt < 20)
                mcFocCtrl.SpeedFlt = 0;                     // 速度死区：±20LSB以内视为静止

            /* ★ 执行控制环：根据CtrlMode分发到力矩/速度/位置控制 */
            Speed_response();
        }
        else
        {
            mcFocCtrl.SpeedFlt = 0;                         // 非运行态速度清零
        }

        /* --- SPI调试输出（软件调试模式）：将关键变量写入调试缓冲区 --- */
        #if defined (SPI_DBG_SW)
        {
            spidebug[0] = FOC__ETHETA;                      // 估算电角度
            spidebug[1] = FOC__UD;                           // D轴输出电压
            spidebug[2] = FOC__UQ;                           // Q轴输出电压
            spidebug[3] = FOC__THETA;                        // 当前FOC使用的电角度
        }
        #endif

        /* 清除DCIF中断标志，同时置位SYSTIF（为1ms tick提供时基） */
        DRV_SR = (DRV_SR | SYSTIF) & (~DCIF);
    }

    GP04 = 0;                           // ★ GPIO调试标记：DRV_ISR出口
}


/*---------------------------------------------------------------------------*/
/* Name     :   void CAN_INT(void) interrupt 8
/* Input    :   NO
/* Output   :   NO
/* Description: CAN总线中断服务 —— 处理CAN消息的接收/发送/错误
/*              中断优先级：中等（低于FOC中断和FO中断）
/*---------------------------------------------------------------------------*/
/**
 * @brief      CAN中断服务 (interrupt 8)
 * @note       处理5种CAN中断事件：
 *              - RXIF  接收中断 → 调用CAN_Read()读取数据 → 释放接收缓冲区
 *              - TXIF  发送成功中断 → 清除标志
 *              - OVIF  溢出中断 → 清除标志（数据丢失）
 *              - ARBIF 仲裁中断 → 清除标志（总线竞争失败）
 *              - ERRIF 错误中断 → 若接收错误计数>128进入主动错误态→复位CAN模块
 */
void Can_INT(void)    interrupt 8
{
    if (ReadBit(CAN_IFR, RXIF))          // 接收中断：有CAN消息到达
    {
        CAN_Read();                      // 读取CAN消息到CanRxdata缓冲区
        SetBit(CAN_CR1, BUFRLS);        // 释放接收缓冲区，允许接收下一条
    }

    if (ReadBit(CAN_IFR, TXIF))          // 发送成功中断
    {
        ClrBit(CAN_IFR, TXIF);          // 清除发送中断标志
    }

    if (ReadBit(CAN_IFR, OVIF))          // 溢出中断：接收缓冲区未及时释放导致溢出
    {
        ClrBit(CAN_IFR, OVIF);          // 清除溢出标志（消息已丢失）
    }

    if (ReadBit(CAN_IFR, ARBIF))         // 仲裁中断：CAN总线仲裁失败
    {
        ClrBit(CAN_IFR, ARBIF);         // 清除仲裁标志
    }

    if (ReadBit(CAN_IFR, ERRIF))         // 错误中断
    {
        if (CAN_REC > 128)              // 接收错误计数 > 128 —— 进入Error Passive状态
        {
            CAN_Init();                 // 复位CAN控制器
        }
        ClrBit(CAN_IFR, ERRIF);         // 清除错误中断标志
    }
}



/*---------------------------------------------------------------------------*/
/* Name     :   void TIM4S_INT(void) interrupt 10
/* Input    :   NO
/* Output   :   NO
/* Description: 1ms定时器中断（SYS TICK中断），用于处理附加功能，如控制环路响应、各种保护等。
               中断优先级低于FO中断和FOC中断。
/*---------------------------------------------------------------------------*/
/**
 * @brief      1ms系统节拍中断 (interrupt 10) —— 低频控制任务
 * @note       由DRV_ISR中置位的SYSTIF标志触发，约1ms执行一次
 *             执行内容：
 *               1. TickCycle_1ms() —— 状态计时器递减、母线电压采样滤波、
 *                  故障保护检测（5段轮询）、位置环PD计算（仅位置模式）
 *             注意：使用了MDU0组，避免与DRV_ISR中的MDU2冲突
 */
void SYStick_INT(void) interrupt 10
{
    if (ReadBit(DRV_SR, SYSTIF))         // 等待SYS TICK中断标志
    {
//        GP04 = 1;                      // (可选) GPIO调试标记入口
        TickCycle_1ms();                 // ★ 执行1ms周期任务（保护检测+位置环）
                                         // 注意：用了第0组MDU

        /* 清除SYSTIF标志，同时保持DCIF（防止丢失FOC中断触发） */
        DRV_SR = (DRV_SR | DCIF) & (~SYSTIF);
//        GP04 = 0;                      // (可选) GPIO调试标记出口
    }
}

/*-------------------------------------------------------------------------------------------------
    Function Name : void CMP3_INT(void)
    Description   : CMP3：硬件比较器过流保护，关断输出，中断优先级最高
    Input         : 无
    Output        : 无
-------------------------------------------------------------------------------------------------*/
/**
 * @brief      硬件比较器过流保护中断 (interrupt 12) ★ 优先级最高
 * @note       当相电流超过DAC设定的硬件阈值时，CMP3比较器立即触发此中断
 *             处理：立即关断PWM输出 → 记录故障源 → 进入mcFault状态
 *             这是最后一道硬件保护，响应速度最快（无需软件判断）
 */
void CMP3_INT(void)  interrupt 12
{
    if(ReadBit(CMP_SR, CMP3IF))          // 检查CMP3比较器中断标志
    {
        FaultProcess();                  // ★ 立即关闭PWM输出（MOE=0）
        mcFaultSource = FaultHardOVCurrent;  // 记录故障源：硬件过流
        mcState       = mcFault;             // 状态机进入故障态（停止一切控制）
        ClrBit(CMP_SR, CMP3IF);          // 清除CMP3中断标志
    }
}

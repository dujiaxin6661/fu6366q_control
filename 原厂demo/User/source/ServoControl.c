/**
 * ============================================================================
 * @file      ServoControl.c
 * @brief     MR磁阻传感器信号处理 —— Sin/Cos偏置校准、霍尔偏置校准、峰值归一化
 * @author    Fortiortech Application Team (Kobe Peng)
 * @since     2023-07
 * @note      本文件包含三个核心函数：
 *              - MR_Offset_Calibration_GETDATA() : ★ 实时采集Sin/Cos偏置数据（状态机方式）
 *              - MR_Offset_Calibration_PROCESSDATA() : 冒泡排序+去头去尾平均+峰值归一化
 *              - Hall_MR_Offset_Check() : 霍尔传感器偏置校准（找跳变沿→角度补偿）
 *              - Hall_MR_StartCheck() : 霍尔传感器初始状态检测（消抖确认高低电平）
 *
 *              全局变量：
 *              - s_PI (PI_Typedef)          : 速度环PI参数
 *              - MR (MRSensor_Typedef)       : MR传感器实时数据（iSin/iCos/Theta180/DirCnt等）
 *              - MROffset (MRSensorOffset_Typedef) : 校准中间数据（偏置数组/峰值数组/状态机）
 *
 *              Sin/Cos信号路径：ADC4→iSin, ADC12→iCos → 减去偏置 → FOC__EALP/FOC__EBET
 * ============================================================================
 */

#include <FU68xx_4.h>                                    // FU68xx 第4组外设（ADC/MDU相关）
#include <Myproject.h>

PI_Typedef              xdata  s_PI;                     /**< 速度环PI参数（Kp/Ki/Kf/Uk_max/Uk_min） */
MRSensor_Typedef        idata  MR;                        /**< MR传感器实时数据 */
MRSensorOffset_Typedef  xdata  MROffset;                 /**< MR校准中间数据和状态机 */

/**
 * ============================================================================
 * @brief      MR_Offset_Calibration_GETDATA() —— MR传感器Sin/Cos偏置实时采集
 * @note       在DRV_ISR中每个PWM周期调用（仅校准态MRCalibra）
 *             使用状态机分别对Sin和Cos信号进行偏置采集：
 *
 *             Sin通道状态机（SinState）：
 *               State0: 等待Sin>0且<300 → State1（正过零检测）
 *               State1: 等待Sin<-300（负半周）→ State2（进入负半周）
 *               State2/3: 等待Sin>300 → 记录峰值iSinMax → State3
 *               State3/4: 等待Sin<-600 → 记录谷值iSinMin → State4
 *               State4: 等待Sin>-100且<0 → 计算本次偏置=(Max+Min)/2, 峰值=(Max-Min)/2
 *                       SinCnt++; 未达到OffsetCalibraTimes→回到State2继续采集
 *                       SinCnt满→State5(完成)
 *
 *             Cos通道同理（CosState），只是判断阈值不同
 *
 *             每个通道采集 OffsetCalibraTimes 组偏置和峰值数据
 *             采集完成后由 MR_Offset_Calibration_PROCESSDATA() 做统计处理
 * ============================================================================
 */
void MR_Offset_Calibration_GETDATA(void)
{
    /* ── Sin通道偏置采集状态机 ── */
    if((MR.iSin > 0) && (MR.iSin < 300) && (MROffset.SinState == 0))     // 状态0→1：正过零点
    {
        MROffset.SinState = 1;
    }
    else if((MR.iSin < -300) && (MROffset.SinState == 1))                // 状态1→2：进入负半周
    {
        MROffset.SinState = 2;
    }
    else if((MR.iSin > 300) && ((MROffset.SinState == 2) || (MROffset.SinState == 3)))  // 状态2/3→3：记录正向峰值
    {
        MROffset.SinState = 3;
        if(MROffset.iSinMax < MR.iSin)                                   // 更新正弦波正向最大值
            MROffset.iSinMax = MR.iSin;
    }
    else if((MR.iSin < -600) && ((MROffset.SinState == 3) || (MROffset.SinState == 4))) // 状态3/4→4：记录负向谷值
    {
        MROffset.SinState = 4;
        if(MROffset.iSinMin > MR.iSin)                                   // 更新正弦波负向最小值
            MROffset.iSinMin = MR.iSin;
    }
    else if((MR.iSin > -100) && (MR.iSin < 0) && (MROffset.SinState == 4)) // 状态4→完成一周期
    {
        /* ★ 计算本次采集的偏置和峰值 */
        MROffset.iSinOffset[MROffset.SinCnt] = (MROffset.iSinMax + MROffset.iSinMin) / 2;   // 偏置 = (Max+Min)/2
        MROffset.iSinPeak[MROffset.SinCnt] = (MROffset.iSinMax - MROffset.iSinMin) / 2;    // 峰值 = (Max-Min)/2
        MROffset.SinCnt++;                                               // 采集次数+1

        if(MROffset.SinCnt < OffsetCalibraTimes)                         // 未达到采集次数
        {
            MROffset.iSinMax = 0;                                        // 清零，准备下一轮
            MROffset.iSinMin = 0;
            MROffset.SinState = 2;                                       // 回到State2继续
        }
        else
        {
            MROffset.SinState = 5;                                       // ★ Sin通道采集完成
        }
    }

    /* ── Cos通道偏置采集状态机（与Sin通道完全对称）── */
    if((MR.iCos > 0) && (MR.iCos < 300) && (MROffset.CosState == 0))
    {
        MROffset.CosState = 1;
    }
    else if((MR.iCos < -300) && (MROffset.CosState == 1))
    {
        MROffset.CosState = 2;
    }
    else if((MR.iCos > 300) && ((MROffset.CosState == 2) || (MROffset.CosState == 3)))
    {
        MROffset.CosState = 3;
        if(MROffset.iCosMax < MR.iCos)
            MROffset.iCosMax = MR.iCos;
    }
    else if((MR.iCos < -600) && ((MROffset.CosState == 3) || (MROffset.CosState == 4)))
    {
        MROffset.CosState = 4;
        if(MROffset.iCosMin > MR.iCos)
            MROffset.iCosMin = MR.iCos;
    }
    else if((MR.iCos > -100) && (MR.iCos < 0) && (MROffset.CosState == 4))
    {
        MROffset.iCosOffset[MROffset.CosCnt] = (MROffset.iCosMax + MROffset.iCosMin) / 2;
        MROffset.iCosPeak[MROffset.CosCnt] = (MROffset.iCosMax - MROffset.iCosMin) / 2;
        MROffset.CosCnt++;
        if(MROffset.CosCnt < OffsetCalibraTimes)
        {
            MROffset.iCosMax = 0;
            MROffset.iCosMin = 0;
            MROffset.CosState = 2;
        }
        else
        {
            MROffset.CosState = 5;                                       // ★ Cos通道采集完成
        }
    }
}


/**
 * @brief      Hall_MR_Offset_Check() —— 双霍尔传感器偏置校准
 * @note       在开环启动期间执行（MRState==MRHallOffset）
 *             找霍尔A信号的跳变沿，记录跳变时刻的MR传感器角度作为霍尔偏置(HallAngleOffset)
 *             通过5个_nop_()延时做消抖，确保电平稳定
 *             校准完成后清零MR传感器角度计数器，从新的零点开始
 */
void Hall_MR_Offset_Check(void)
{
    static uint8  HallReadState = 0;                     // 状态：0=等待霍尔低电平, 1=等待霍尔高电平（跳变沿）

    MR.HallA = HALLA;                                    // 读取霍尔A信号（HALLA是GPIO宏）

    if(MR.HallCalibrationFlag == 0)                       // 尚未校准
    {
        if(HallReadState == 0)                           // ★ 阶段0：等待霍尔=0
        {
            if(MR.HallA == 0)                            // 读到低电平
            {
                _nop_();_nop_();_nop_();_nop_();_nop_(); // ★ 消抖延时（~5个指令周期）
                MR.HallA = HALLA;                        // 重新读取确认

                if(MR.HallA == 0)                        // 确认低电平
                {
                    MR.HallState = 0;                    // 记录霍尔状态
                    HallReadState = 1;                   // 进入阶段1（等待上升沿）
                }
            }
        }
        else                                             // ★ 阶段1：等待霍尔=1（上升沿）
        {
            if(MR.HallA == 1)                            // 读到高电平（上升沿！）
            {
                _nop_();_nop_();_nop_();_nop_();_nop_(); // ★ 消抖延时
                MR.HallA = HALLA;                        // 重新读取确认

                if(MR.HallA == 1)                        // 确认高电平
                {
                    /* ★ 记录跳变时刻的MR角度作为霍尔偏置 */
                    MR.HallAngleOffset = MR.Theta180;    // 保存霍尔跳变沿对应的MR传感器角度

                    MR.FlagCircle  = 0;                  // 清零圆圈标志
                    MR.Theta180 = 0;                     // 清零角度（从该跳变点重新开始）
                    MR.Theta180Old = 0;

                    MR.HallCalibrationFlag = 1;          // ★ 霍尔校准完成标志
                }
            }
        }
    }
}


/**
 * @brief      MR_Offset_Calibration_PROCESSDATA() —— 偏置数据后处理
 * @note       当Sin和Cos通道都完成OffsetCalibraTimes次采集后调用
 *             处理流程：
 *              1. 冒泡排序 → 将偏置数组从小到大排列
 *              2. 去头去尾求平均 → 去掉最大和最小值后取平均（减少异常点影响）
 *              3. 累加到MR.SinOffset/CosOffset（历史偏置的累加，多次校准取平均）
 *
 *              如果启用了峰值归一化(MR_NormaLizationEnable)：
 *              4. 计算Sin和Cos的平均峰值
 *              5. 以峰值较大的通道为基准，计算归一化系数
 *              6. 在DRV_ISR中对较小的通道做幅值补偿
 */
void MR_Offset_Calibration_PROCESSDATA(void)
{
    uint8 i, j;
    int32 OffsetTemp;

    /* ── ★ Sin偏置：冒泡排序（从大到小）── */
    for(i = 0; i < (OffsetCalibraTimes - 1); i++)
    {
        for(j = 0; j < (OffsetCalibraTimes - i - 1); j++)
        {
            if(MROffset.iSinOffset[j] < MROffset.iSinOffset[j + 1])
            {
                OffsetTemp = MROffset.iSinOffset[j + 1];       // 交换
                MROffset.iSinOffset[j + 1] = MROffset.iSinOffset[j];
                MROffset.iSinOffset[j] = OffsetTemp;
            }
        }
    }

    /* ── 去头去尾求平均（去掉最大和最小各1个）── */
    OffsetTemp = 0;
    for(i = 0; i < (OffsetCalibraTimes - 2); i++)
    {
        OffsetTemp += MROffset.iSinOffset[i + 1];             // 从第2个到倒数第2个
    }
    MR.SinOffset = MR.SinOffset + OffsetTemp / (OffsetCalibraTimes - 2);  // 累加到全局偏置（多次校准取均值）

    /* ── ★ Cos偏置：同样的冒泡排序+去头去尾平均 ── */
    for(i = 0; i < (OffsetCalibraTimes - 1); i++)
    {
        for(j = 0; j < (OffsetCalibraTimes - i - 1); j++)
        {
            if(MROffset.iCosOffset[j] < MROffset.iCosOffset[j + 1])
            {
                OffsetTemp = MROffset.iCosOffset[j + 1];
                MROffset.iCosOffset[j + 1] = MROffset.iCosOffset[j];
                MROffset.iCosOffset[j] = OffsetTemp;
            }
        }
    }

    OffsetTemp = 0;
    for(i = 0; i < (OffsetCalibraTimes - 2); i++)
    {
        OffsetTemp += MROffset.iCosOffset[i + 1];
    }
    MR.CosOffset = MR.CosOffset + OffsetTemp / (OffsetCalibraTimes - 2);


#if (MR_NormaLizationEnable)                              // ── ★ (可选) Sin/Cos峰值归一化 ──

    /* 计算Sin平均峰值 */
    OffsetTemp = 0;
    for(i = 0; i < OffsetCalibraTimes; i++)
    {
        OffsetTemp += MROffset.iSinPeak[i];
    }
    MROffset.iSinPeakValue = OffsetTemp / OffsetCalibraTimes;

    /* 计算Cos平均峰值 */
    OffsetTemp = 0;
    for(i = 0; i < OffsetCalibraTimes; i++)
    {
        OffsetTemp += MROffset.iCosPeak[i];
    }
    MROffset.iCosPeakValue = OffsetTemp / OffsetCalibraTimes;

    /* ★ 以峰值较大的通道为基准，对较小通道做归一化 */
    if(MROffset.iSinPeakValue >= MROffset.iCosPeakValue)  // Sin峰值更大 → 以Sin为基准
    {
        /* 计算归一化系数 = (Sin峰值 - Cos峰值) / Cos峰值 */
        DivQ_L_MDU((MROffset.iSinPeakValue - MROffset.iCosPeakValue), 0,
                    MROffset.iCosPeakValue, MROffset.iPeakNorlizationCoff);
        MROffset.iPeakNorlizationFlag = 0;               // 0=对Cos做归一化
    }
    else                                                   // Cos峰值更大 → 以Cos为基准
    {
        DivQ_L_MDU((MROffset.iCosPeakValue - MROffset.iSinPeakValue), 0,
                    MROffset.iSinPeakValue, MROffset.iPeakNorlizationCoff);
        MROffset.iPeakNorlizationFlag = 1;               // 1=对Sin做归一化
    }
    MROffset.iPeakNorlizationFinshFlag = 1;               // 归一化系数计算完成
#endif

}


/**
 * @brief      Hall_MR_StartCheck() —— 霍尔传感器初始状态检测
 * @note       在Flash读取到校准数据后调用（SenconrdPowerOn==1时）
 *             通过消抖读取确认HALLA和HALLB的稳定电平（0或1）
 *             然后根据当前的MR角度判断电机处于哪个霍尔扇区
 *             最后设置FlagCircle（影响角度方向的正负号）
 *
 *             消抖方式：读取+5个nop+再读取+5个nop+再读取，三次一致才确认
 */
void Hall_MR_StartCheck(void)
{
    uint16 RetryCnt;                                     // ★ 超时保护计数器

    MR.HallA = 2;                                        // 初始化为无效值
    MR.HallB = 2;

    /* ── 读取并消抖HALLA ── */
    RetryCnt = 0;
    while(MR.HallA == 2)
    {
        MR.HallA = HALLA;                                // 第一次读取

        if(MR.HallA == 1)                                // 读到高
        {
            _nop_();_nop_();_nop_();_nop_();_nop_();
            _nop_();_nop_();_nop_();_nop_();_nop_();
            MR.HallA = HALLA;                            // 重新读取
            if(MR.HallA == 1)  { MR.HallA = 1; }          // 确认高电平
            else               { MR.HallA = 2; }          // 抖动，重新读取
        }
        else if(MR.HallA == 0)                           // 读到低
        {
            _nop_();_nop_();_nop_();_nop_();_nop_();
            _nop_();_nop_();_nop_();_nop_();_nop_();
            MR.HallA = HALLA;
            if(MR.HallA == 0)  { MR.HallA = 0; }          // 确认低电平
            else               { MR.HallA = 2; }
        }

        /* ★ 超时保护：霍尔引脚异常时强制退出，默认为低电平 */
        if(++RetryCnt > 10000)
        {
            MR.HallA = 0;                                // 异常时默认低电平，防止死循环
            break;
        }
    }

    /* ── 读取并消抖HALLB（逻辑同上）── */
    RetryCnt = 0;
    while(MR.HallB == 2)
    {
        MR.HallB = HALLB;

        if(MR.HallB == 1)
        {
            _nop_();_nop_();_nop_();_nop_();_nop_();
            _nop_();_nop_();_nop_();_nop_();_nop_();
            MR.HallB = HALLB;
            if(MR.HallB == 1)  { MR.HallB = 1; }
            else               { MR.HallB = 2; }
        }
        else if(MR.HallB == 0)
        {
            _nop_();_nop_();_nop_();_nop_();_nop_();
            _nop_();_nop_();_nop_();_nop_();_nop_();
            MR.HallB = HALLB;
            if(MR.HallB == 0)  { MR.HallB = 0; }
            else               { MR.HallB = 2; }
        }

        /* ★ 超时保护：霍尔引脚异常时强制退出，默认为低电平 */
        if(++RetryCnt > 10000)
        {
            MR.HallB = 0;                                // 异常时默认低电平，防止死循环
            break;
        }
    }

    /* ── 延时10ms确认稳定后，根据角度判断扇区 ── */
    MRSyS.State_Count = 20;
    while(MRSyS.State_Count != 0);                       // 轮询等待（阻塞式，约10ms）
    MR.Theta180 = FOC__ETHETA - MR.HallAngleOffset;      // 读取当前MR角度
    MR.Theta180Old = MR.Theta180;

    /* ★ 根据当前角度判断霍尔扇区 */
    if((MR.Theta180 > (16383)) && (MR.Theta180 < (49152)))    // 角度在90°~270°之间
    {
        MR.HallState = !MR.HallA;                        // 使用HALLA的反相
    }
    else
    {
        if(((int16)MR.Theta180) > 0)                     // 角度>0°且<90°或>270°
            MR.HallState = MR.HallB;                     // 使用HALLB
        else
            MR.HallState = !MR.HallB;                    // 使用HALLB的反相
    }

    MR.FlagCircle = (uint16)MR.HallState * 32768;        // ★ 设置圆圈标志（0或0x8000）
                                                          // 0x8000=32768 即半圈偏移量
}

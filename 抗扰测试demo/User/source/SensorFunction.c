/**
 * ============================================================================
 * @file      SensorFunction.c
 * @brief     MR磁阻传感器状态机 —— 管理传感器从校准到就绪的完整生命周期
 * @note      核心状态机 MR_Control() 包含7个状态：
 *              MRWait(等待)→MRCalibra(校准)→MRHallOffset(霍尔偏置)→MRRun(运行)→MRNLC(非线性补偿)→MRReady(就绪)→MRFinish(完成)
 *
 *              全局变量：
 *              - MRSyS (MRSySTypeDef) : MR传感器状态机结构体
 *                 - MRState    		 	 : 当前状态
 *                 - State_Count       : 状态计时器（1ms递减）
 *                 - MRStaSet          : 标志位（StartCalibraSetFlag/OverCalibraSetFlag等）
 * ============================================================================
 */

#include <FU68xx_2.h>
#include <Myproject.h>

MRSySTypeDef    xdata   MRSyS;                           /**< MR传感器状态机全局变量 */

/**
 * ============================================================================
 * @brief      MR_Control() —— MR传感器状态机（由主循环每周期调用）
 * @note       状态切换流程：
 *              MRWait : 等待启动指令
 *                       - StartCalibraSetFlag==1 → MRCalibra（开始校准，角度开环）
 *                       - OverCalibraSetFlag==1 → MRReady（Flash已有校准数据，直接跳过校准）
 *
 *              MRCalibra : ★ Sin/Cos偏置校准
 *                        - 等待SinState==5且CosState==5（两个通道都完成OffsetCalibraTimes次采集）
 *                        - 等待FOC角度稳定在0°附近（FOC__THETA<|100|）
 *                        - 调用MR_Offset_Calibration_PROCESSDATA()做统计处理
 *                        - 根据DirCnt判断旋转方向（<-1000=反转 → 设置FRStatus位）
 *                        - → MRHallOffset（霍尔偏置校准阶段）
 *
 *              MRHallOffset : 霍尔传感器偏置校准
 *                           - 无霍尔(NONEHALL)：直接→MRRun
 *                           - 双霍尔(TWOHALL)：等HallCalibrationFlag==1且稳定→MRRun
 *
 *              MRRun : 短暂延时10ms后 → MRNLC
 *              MRNLC : 短暂延时10ms后 → MRReady（保留给非线性补偿扩展）
 *              MRReady : 等待预定位完成(AngleOffsetInitFlag==1) → MRFinish
 *              MRFinish : 校准全部完成，等待应用层使用
 * ============================================================================
 */
void MR_Control(void)
{
    switch (MRSyS.MRState)
    {
        case MRWait:
            /* ── 等待启动校准指令 ── */
            if(MRSyS.MRStaSet.SetFlag.StartCalibraSetFlag == 1)     // ★ 开始校准（首次上电）
            {
                MRSyS.MRState  = MRCalibra;
                mcFocCtrl.AngleMode = AngleOpen;                    // 切换到角度开环模式（开环旋转采集数据）
            }
            else if(MRSyS.MRStaSet.SetFlag.OverCalibraSetFlag == 1) // ★ 已经校准过（Flash有数据）
            {
                MRSyS.MRState = MRReady;
                mcFocCtrl.AngleMode = AngleLoop;                    // 直接切换到角度闭环模式
            }
            break;

        case MRCalibra:
            /* ── ★ 等待Sin和Cos双通道都完成偏置采集 ── */
            if((MROffset.CosState == 5) && (MROffset.SinState == 5))
            {
                /* 等待FOC电角度稳定在0°附近（预定位锁到了0°）再处理数据 */
                if(FOC__THETA < 100 && FOC__THETA > -100)
                {
                    MR_Offset_Calibration_PROCESSDATA();            // ★ 冒泡排序+去头去尾平均
                    MR.SinCosOffsetInitFlag = 1;                    // 标记Sin/Cos偏置已校准

                    /* 根据方向计数器判断旋转方向 */
                    if(MR.DirCnt < (-1000))                         // 方向计数<-1000 → 反转
                    {
                        mcFocCtrl.FRStatus = 1;                     // 设置反转标志
                        SetBit(DRV_CR, DDIR);                       // 硬件设置反转（交换U/V相序）
                    }

                    MRSyS.MRState = MRHallOffset;                   // ★ 进入霍尔偏置校准
                    MRSyS.State_Count = 5000;                       // 给5s时间完成霍尔校准
                }
            }
            break;

        case MRHallOffset:
            /* ── 霍尔偏置校准阶段 ── */
            #if (HALLNUM == NONEHALL)                                // 无霍尔传感器
                mcFocCtrl.AlignMode = 1;                             // 使能两段预定位
                mcFocCtrl.AngleMode = AngleLoop;                     // 切换到角度闭环
                MRSyS.MRState = MRRun;
            #elif (HALLNUM == TWOHALL)                               // ★ 双霍尔传感器
                /* 等待霍尔校准完成 + 角度稳定 + 计时到 */
                if(MR.HallCalibrationFlag == 1 &&
                   FOC__THETA < 100 && FOC__THETA > -100 &&
                   MRSyS.State_Count == 0)
                {
                    MR.HallAngleOffsetInitFlag = 1;                 // 标记霍尔偏置已校准

                    mcFocCtrl.AlignMode = 1;                         // 使能两段预定位
                    mcFocCtrl.AngleMode = AngleLoop;                 // 切换到角度闭环

                    MRSyS.MRState = MRRun;

                    MRSyS.State_Count = 100;                         // 延时100ms
                }
            #endif
            break;

        case MRRun:
            /* ── 运行过渡态：延时10ms ── */
            if(MRSyS.State_Count == 0)
            {
                MRSyS.State_Count = 10;
                MRSyS.MRState = MRNLC;                               // → 非线性补偿
            }
            break;

        case MRNLC:
            /* ── NLC（非线性补偿）过渡态：延时10ms ── */
            /* 当前为空，预留给非线性补偿功能扩展（如LUT查表修正角度误差）*/
            if(MRSyS.State_Count == 0)
            {
                MRSyS.State_Count = 10;
                MRSyS.MRState = MRReady;
            }
            break;

        case MRReady:
            /* ── 就绪态：等待第二次预定位完成 ── */
            if(MRSyS.State_Count == 0)
            {
                if(MR.AngleOffsetInitFlag == 1)                      // 等待角度零偏预定位完成
                {
                    MRSyS.MRState = MRFinish;                        // ★ 所有校准完毕
                }
            }
            break;

        case MRFinish:
            /* ── ★ 完成态：传感器校准全部完成，应用层可以使用 ── */
            break;
    }
}

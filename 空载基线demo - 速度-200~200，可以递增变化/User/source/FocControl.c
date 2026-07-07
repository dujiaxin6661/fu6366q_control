/**
 * ============================================================================
 * @file      FocControl.c
 * @brief     电机控制状态机 —— 管理电机从启动到运行的完整生命周期
 * @note      核心状态机 MC_Control() 包含9个状态：
 *              mcReady(就绪)→mcInit(初始化)→mcCharge(预充电)→mcFirstAlign(第一次预定位)
 *              →mcStart(开环启动)→mcSecondAlign(第二次预定位)→mcRun(运行)
 *              →mcStop(停止)→mcFault(故障)
 *
 *              状态切换由 mcFocCtrl.State_Count 计时器驱动（1ms递减）
 *              每个状态调用对应的 Motor_Xxx() 函数完成该阶段的硬件配置
 * ============================================================================
 */

#include <FU68xx_2.h>
#include <Myproject.h>

/* Private variables ----------------------------------------------------------------------------*/
MotStaType idata mcState;       /**< 当前电机状态（mcReady/mcInit/mcCharge/.../mcFault） */
MotStaM    McStaSet;            /**< 状态机标志位集合：CalibFlag/ChargeSetFlag/AlignSetFlag/StartSetFlag等 */

/**
 * ============================================================================
 * @brief      MC_Control() —— 电机控制主状态机（由主循环每周期调用）
 * @note       状态切换流程：
 *              mcReady 		 : 等待电流偏置校准完成 + FlagONOFF=1 → mcInit
 *              mcInit   		 : 调用Motor_Init()初始化变量和PI，计时到→mcCharge
 *              mcCharge     : 三步预充电（U→U+V→U+V+W），完成后判断：
 *                             - 已校准（Flash有数据）→ 直接mcStart
 *                             - 未校准（首次上电）→ mcFirstAlign
 *              mcFirstAlign : 预定位到0°电角度，计时到 → mcStart（开始传感器校准）
 *              mcStart  		 : 开环加速运行，等待SinCos+Hall校准完成：
 *                         		 - 首次上电→mcSecondAlign（第二次预定位找零偏）
 *                         		 - 第二次上电→mcRun（直接进入闭环）
 *              mcSecondAlign : 第二次预定位找传感器与电角度的零偏 → mcRun
 *              mcRun    		 : 正常运行，FlagONOFF=0→mcStop
 *              mcStop  		 : 关闭输出，FlagONOFF=1→mcReady（重新启动）
 *              mcFault  		 : 故障态，故障清除→mcReady
 * ============================================================================
 */
void MC_Control(void)
{
    switch (mcState)
    {
        case mcReady:
            /* --- 就绪态：关闭输出，等待电流偏置校准完成 --- */
            Motor_Ready();

            /* OffsetFlag==1: 电流偏置校准完成；FlagONOFF==1: 启动指令已给出 */
            if(mcCurOffset.OffsetFlag == 1 && mcSpeedRamp.FlagONOFF == 1)
            {
                mcState = mcInit;
            }
            break;

        case mcInit:
            /* --- 初始化态：调用VariablesPreInit()和PI0_Init()初始化变量和PI --- */
            Motor_Init();

            if(mcFocCtrl.State_Count == 0)
            {
                mcState = mcCharge;
                mcFocCtrl.State_Count  = Charge_Time;   // 设置预充电时间（默认10ms）
            }
            break;

          case mcCharge:
            /* --- 预充电态：三步充电U→U+V→U+V+W（10%占空比）给自举电容充电 --- */
            Motor_Charge();

            if(mcFocCtrl.State_Count == 0)
            {
                if(MR.SinCosOffsetInitFlag == 1)         // ★ Sin/Cos偏置已从Flash读取成功
                {
                    #if (HALLNUM == TWOHALL)

                        if(MR.HallAngleOffsetInitFlag == 1)  // 霍尔偏置也已校准
                        {
                            MRSyS.MRStaSet.SetFlag.OverCalibraSetFlag = 1;  // 标记"已校准过"
                            mcState = mcStart;                               // 跳过预定位，直接开环
                        }
                        else                                 // ★ 霍尔偏置未校准 → 需要走完整校准流程
                        {
                            mcState = mcFirstAlign;         // 进入预定位，后续在mcStart中校准霍尔
                        }

                    #elif (HALLNUM == NONEHALL)

                        MRSyS.MRStaSet.SetFlag.OverCalibraSetFlag = 1;      // 标记"已校准过"
                        mcState = mcStart;                                   // 跳过预定位

                    #endif
                }
                else
                {
                    mcState = mcFirstAlign;              // ★ 首次运行 → 需要预定位
                }
            }
            break;

        case mcFirstAlign:
            /* --- 第一次预定位：D轴电流钳位到0°电角度，锁定转子初始位置 --- */
            Motor_Align();

            if((mcFocCtrl.State_Count == 0))
            {
                mcState = mcStart;
                MRSyS.MRStaSet.SetFlag.StartCalibraSetFlag = 1;  // 触发MR传感器校准开始
            }
            break;

        case mcStart:
            /* --- 开环启动态：角度开环斜坡加速，同时后台执行MR校准 --- */
            Motor_Open();

            #if (HALLNUM == TWOHALL)

                if(MR.SinCosOffsetInitFlag == 1 && MR.HallAngleOffsetInitFlag == 1) // 开环期间校准完成
                {
                    if(MR.SenconrdPowerOn == 0)          // ★ 首次上电：Flash无校准数据
                    {
                        mcState = mcSecondAlign;         // 需要第二次预定位找传感器零偏
                        McStaSet.SetFlag.AlignSetFlag = 0;
                    }
                    else if(MR.SenconrdPowerOn == 1)     // ★ 再次上电：Flash已有校准数据
                    {
                        mcState = mcRun;                 // 直接进入闭环运行
                    }
                }

            #elif (HALLNUM == NONEHALL)

                if(MR.SinCosOffsetInitFlag == 1)
                {
                    if(MR.SenconrdPowerOn == 0)
                    {
                        mcState = mcSecondAlign;         // 首次：需要第二次预定位
                        McStaSet.SetFlag.AlignSetFlag = 0;
                    }
                    else if(MR.SenconrdPowerOn == 1)
                    {
                        mcState = mcRun;                 // 非首次：直接运行
                    }
                }
            #endif
            break;

          case mcSecondAlign:
            /* --- 第二次预定位：通过两次定位(0°/90°)计算MR传感器与电角度的零偏 --- */
            if(MR.AngleOffsetInitFlag == 0)
            {
                Motor_Align();                           // 执行预定位流程
            }
            else                                         // 预定位完成，得到零偏
            {
                /* ★ 预定位完成后重新初始化FOC硬件，从电压锁轴模式切到运行模式 */
                FOC_Init();                              // 清空FOC寄存器并恢复基础配置
                FOC_QKP = DQKP;                          // 恢复运行PI增益
                FOC_DKP = DQKP;
                FOC_QKI = DQKI;
                FOC_DKI = DQKI;

                if(mcFocCtrl.CurrentLoopEn)              // 电流闭环模式
                {
                    ClrBit(FOC_CR2, UQD);                // Q轴 → 电流模式（读取FOC_IQREF）
                    ClrBit(FOC_CR2, UDD);                // D轴 → 电流模式（读取FOC_IDREF）
                    FOC_IDREF = 0;
                    FOC_IQREF = 0;
                }
                else                                     // 电压模式
                {
                    SetBit(FOC_CR2, UQD);
                    SetBit(FOC_CR2, UDD);
                    FOC__UD = 0;
                    FOC__UQ = 0;
                }

                MOE = 1;                                 // 使能PWM输出
                mcState = mcRun;                         // ★ 进入正常运行
            }

            break;

         case mcRun:
            /* --- ★ 正常运行态：控制环激活（力矩/速度/位置），由DRV_ISR和SYStick驱动 --- */
            if (mcSpeedRamp.FlagONOFF == 0)              // 收到停止指令
            {
                MOE = 0;                                 // 关断PWM输出
                mcState = mcStop;
            }
            break;

        case mcStop:
            /* --- 停止态：等待重新启动指令 --- */
            if (mcSpeedRamp.FlagONOFF == 1)              // 收到重启指令
            {
                mcState = mcReady;                       // 返回就绪态，重新走启动流程
            }
            break;

        case mcFault:
            /* --- 故障态：输出已关闭，等待故障源清除 --- */
            FaultProcess();                              // 确保输出关闭

            if(mcFaultSource == FaultNoSource)           // 故障清除（如过流自动恢复）
            {
                mcState = mcReady;                       // 返回就绪态
            }
          break;

        default:
            break;
    }
}

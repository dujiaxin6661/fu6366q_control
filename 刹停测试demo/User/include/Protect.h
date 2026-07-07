/*  -------------------------- (C) COPYRIGHT 2020 Fortiortech ShenZhen ---------------------------*/
/*  File Name      : Protect.h
/*  Author         : Fortiortech  Appliction Team
/*  Version        : V1.0
/*  Date           : 2020-08-18
/*  Description    : 主要用于电机运行保护条件参数设置.
/*  ----------------------------------------------------------------------------------------------*/
/*                                     All Rights Reserved
/*  ----------------------------------------------------------------------------------------------*/

/*  Define to prevent recursive inclusion --------------------------------------------------------*/
#ifndef __PROTECT_H_
#define __PROTECT_H_

#include <Customer.h>
#include <Parameter.h>
#include <AddFunction.h>
#include "FU68xx_4_Type.h"

/*硬件过流保护DAC值*/

/*保护参数值-------------------------------------------------------------------*/
 /*硬件过流保护*/
 #define Hardware_FO_Protect            (1)                                     // 硬件FO过流保护使能，适用于IPM有FO保护的场合
 #define Hardware_CMP_Protect           (2)                                     // 硬件CMP比较过流保护使能，适用于MOS管应用场合
 #define Hardware_FO_CMP_Protect        (3)                                     // 硬件CMP比较和FO过流保护都使能
 #define Hardware_Protect_Disable       (4)                                     // 硬件过流保护禁止，用于测试
 #define HardwareCurrent_Protect        (Hardware_CMP_Protect)              	// 硬件过流保护实现方式


 /*硬件过流保护比较值来源*/
 #define Compare_DAC                    (0)                                     // DAC设置硬件过流值
 #define Compare_Hardware               (1)                                     // 硬件设置硬件过流值
 #define Compare_Mode                   (Compare_DAC)                      		// 硬件过流值的来源

 #define OverHardcurrentValue           (10.0)                                	// (A) DAC模式下的硬件过流值

 /*软件过流保护*/
 #define OverSoftCurrent_DectTime       (5)                                    ///< (ms)软件过流检测时间8
 #define OverSoftCurrentValue           I_Value(8.0)                           // (A) 软件过流值

 /*过欠压保护*/
 #define Over_Protect_Voltage           (36.0)                                  // (V) 直流电压过压保护值
 #define Over_Recover_Vlotage           (30.0)                                  // (V) 直流电压过压保护恢复值
 #define Under_Protect_Voltage          (12.0)                                 	// (V) 直流电压欠压保护值
 #define Under_Recover_Vlotage          (10.0)                                  // (V) 直流电压欠压保护恢复值

 /*堵转保护*/
 #define MOTOR_SPEED_STAL_MAX_RPM       S_Value(2400.0)                                // (RPM) 堵转保护最大转速
 #define MOTOR_SPEED_STAL_MIN_RPM       S_Value(100.0)                                 // (RPM) 堵转保护最小转

 /*  保护重启参数设置  */

 #define OverCurrentRecoverTime           (1000)                                       // (5ms) 过流保护恢复时间

 /*保护使能*/
 #define OverSoftCurrentProtectEnable   (1)                                    // 软件过流保护，0,不使能；1，使能
 #define VoltageProtectEnable           (1)                                    // 电压保护，0,不使能；1，使能
 #define StallProtectEnable             (0)                                    // 堵转保护，0,不使能；1，使能
 #define StartProtectEnable             (1)                                    // 当电机为奇数对极，且没有hall时，会有一半几率会反转转，这个检测反转然后保护
 
 
 typedef enum
{
    FaultNoSource      = 0,                                                     // 无故障
    FaultHardOVCurrent = 1,                                                     // 硬件过流
    FaultSoftOVCurrent = 2,                                                     // 软件过流
    FaultUnderVoltage  = 3,                                                     // 欠压保护
    FaultOverVoltage   = 4,                                                     // 过压保护
    FaultLossPhase     = 5,                                                     // 缺相保护
    FaultStall         = 6,                                                     // 堵转保护
    FaultAlign         = 8,                                                     // 预定位失败，防止角度误差保护
    FaultStart         = 9,                                                     // Hall故障保护，防止Hall出现异常
	  FaultMROffset      = 10, 
} FaultStateType;


typedef struct
{
    uint16 segment;                                                             // 分段执行
    
    //voltage protect
    uint16 OverVoltDetecCnt;                                                    // 过压检测计数
    uint16 UnderVoltDetecCnt;                                                   // 欠压检测计数
    uint16 VoltRecoverCnt;                                                      // 过压恢复计数
	
    uint8  OverCurCnt;                                                          // 软件过流计数
  	int16  Is;
	
    //OVER Current protect recover
    uint16 CurrentRecoverCnt;                                                   // 过流保护恢复计数
    uint16 CurrentPretectTimes;      

    //stall protect
    uint16 StallCnt;                                                            // 堵转延迟判断计时
    uint8  StallFlag; 
    // hall err protect
    uint8  HallErrCnt;

} FaultVarible;


extern FaultVarible         xdata   mcFaultDect;
extern FaultStateType       xdata   mcFaultSource;

extern void   Fault_Detection(void);
extern void   Fault_OverCurrentRecover(void);
extern void   Fault_OverUnderVoltage(void);
extern void   Fault_Stall(void);
extern void   Fault_OverCurrent(void);
extern void   Fault_Start(void);
extern void   FaultProcess(void);

#endif


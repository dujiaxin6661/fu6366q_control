/**************************** (C) COPYRIGHT 2018 Fortiortech shenzhen *****************************
* File Name          : AddFunction.h
* Author             : Bruce, Fortiortech Hardware
* Version            : V1.0
* Date               : 2018-02-11
* Description        : This file contains all the common data types used for
*                      Motor Control.
***************************************************************************************************
* All Rights Reserved
**************************************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __AddFunction_H_
#define __AddFunction_H_

/******************************************************************************/
#include <FU68xx_6_Type.h>
/******************************************************************************/

/* Exported types ------------------------------------------------------------*/


typedef struct
{
    uint16 ADCDcbus;                                                           // 母线电压
	  uint16 ADCDcbusFlt;                                                        // 母线电压滤波高16位
	
    uint16 ADCIbus;                                                            // 母线电流
	  uint16 ADCIbusFlt;                                                         // 母线电流滤波高16位
	
    uint16 ADCTmos;                                                            // Mos温度
	  uint16 ADCTmosFlt;                                                         // Mos温度滤波高16位
	
} ADCSample;

// 电机的工作模式
typedef enum
{
	  CurrentLoopMode        = 1,       // 力矩控制模式
    SpeedLoopMode          = 2,       // 速度控制模式
    PostionLoopMode        = 3,       // 位置控制模式

}CONTROLModeType;

typedef union 
{
    int32  s32;                                                                 // 比较值标幺化的值
    int16  s16[2];
}S32tS16;

typedef union 
{
    int32  s32;                                                                 // 比较值标幺化的值
    int8   s8[4];
}S32tS8;

typedef union 
{
    uint32  u32;                                                                // 比较值标幺化的值
    uint16  u16[2];
}U32tU16;

typedef union 
{
    int16  s16;                                                                 // 比较值标幺化的值
    int8   s8[2];
}S16tS8;

typedef union 
{
    uint16  u16;                                                                 // 比较值标幺化的值
    uint8   u8[2];
}U16tU8;

typedef union
{
    uint16 ByteMode;                                                            // 整个配置模式使能位
    struct
    {
        uint16 bit15    :1;                                                     // 第15位
        uint16 bit14    :1;                                                     // 第14位
        uint16 bit13    :1;                                                     // 第13位
        uint16 bit12    :1;                                                     // 第12位
        uint16 bit11    :1;                                                     // 第11位
        uint16 bit10    :1;                                                     // 第10位
        uint16 bit9     :1;                                                     // 第9位
        uint16 bit8     :1;                                                     // 第8位
        uint16 bit7     :1;                                                     // 第7位
        uint16 bit6     :1;                                                     // 第6位
        uint16 bit5     :1;                                                     // 第5位
        uint16 bit4     :1;                                                     // 第4位
        uint16 bit3     :1;                                                     // 第3位
        uint16 bit2     :1;                                                     // 第2位
        uint16 bit1     :1;                                                     // 第1位
        uint16 bit0     :1;                                                     // 第0位
    } BitMode;
}U16tBit;


typedef struct
{
    S32tS16 PIUK;
    int16   _Q15PIUK;
    int16   Uk_max;
    int16   Uk_min;
    int32   Err;
	  int32   ErrLast;
}PI_TypeDef;

typedef enum
{
    AngleOpen           = 1,      // 角度开环OpenMode
    AngleLoop           = 2,      // 角度闭环SensorMode
}AngleModeType;

typedef struct
{
	  uint8   CtrlMode;                                                           // 控制模式,0为速度模式，1为舵机模式
    uint8   CurrentLoopEn;                                                      // 电流闭环使能
    uint8   FRStatus;                                                           // MR方向
    int16   mcDcbusFlt;                                                         // 母线电压滤波高16位
    int16   SpeedFlt;                                                           // 当前速度滤波后的值
    uint16  ChargeStep;                                                         // 预充电的步骤
    volatile uint16  State_Count;                                               // 电机各个状态的时间计数
    uint16  SensorTheta360Old;                                                  // 传感器360°角度历史值
    uint8   AlignMode;                                                          // 预定位模式（0：一次预定位；1：多次预定位 2：开环预定位）
    uint8   AlignFlag;                                                          // 预定位状态
    int16   AlignTheta1;                                                        // 第一次预定位0°时候的角度
    int16   AlignTheta2;                                                        // 第二次预定位0°时候的角度
    int16   FOCTheta;                                                           // 给FOC寄存器幅值变量
    int16   FOCThetaCompensation;                                               // 角度补偿
    int16   SpeedErr;
    int16   PostionLoopOut ;                                                    // 位置环输出                        
    int16   UQValueTemp;                                                        // 速度环输出        
    int16   Iqref;
    uint16  RunTimeCnt;
    int16   MotorAngle360;                                                      // 增加传感器0点和电角度0点之后的角度
    int16   SensorAngleOffset;                                                  // 传感器角度偏置
    S32tS16 TargetAngle;                                                        // 目标圈数
    S32tS16 SensorAngle;                                                        // 传感器角度
    AngleModeType   AngleMode;                                                  // 角度控制模式
}FOCCTRL;


typedef struct
{
    int16   TargetValue;
    int16   ActualValue;
    int16   IncValue;
    int16   DecValue;
    int8    FlagONOFF;
}MCRAMP;

/* Exported variables ---------------------------------------------------------------------------*/
extern ADCSample            xdata   AdcSampleValue;
extern FOCCTRL              xdata   mcFocCtrl;
extern MCRAMP               xdata   mcSpeedRamp;
extern PI_TypeDef           idata   PI_Position;

/* Exported functions ---------------------------------------------------------------------------*/

extern void   VariablesPreInit(void);
extern void   Posi_response(void);
extern void   Speed_response(void);
extern void   mc_ramp(void);
extern uint16 Abs_F16(int16 value);
extern uint32 Abs_F32(int32 value);
extern void   TimeCountDec(void);
extern void   FUNC_Control(void);
extern void   TickCycle_1ms(void);
extern void   mc_rampLim(void);

#endif
/**
  ******************************************************************************
  * @file    FocControl.h
  * @author  Fortior Application Team
  * @version V1.0.0
  * @date    10-Apr-2017
  * @brief   define motor contorl parameter
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __FOCCONTROL_H_
#define __FOCCONTROL_H_


/* Exported types -------------------------------------------------------------------------------*/

typedef enum
{
    mcReady     = 0,
    mcInit      = 1,
    mcCharge    = 2,
    mcFirstAlign = 3,
    mcSecondAlign =4,
    mcStart     = 5,
    mcRun       = 6,
    mcStop      = 7,
    mcFault     = 8,
    mcBrake     = 9,
}MotStaType;

typedef union
{
    uint8 SetMode;                                                              // 整个配置模式使能位
    struct
    {
        uint8 CalibFlag        :1;                                              // 电流校准的标志位
        uint8 ChargeSetFlag    :1;                                              // 预充电配置标志位
        uint8 AlignSetFlag     :1;                                              // 预定位配置标志位
        uint8 StartSetFlag     :1;                                              // 启动配置标志位
        uint8 HallGetFlag      :1;                                              // HALL读取标志
		  	uint8 PosCheckFlag     :1; 
    } SetFlag;
}MotStaM;



/* Exported variables ---------------------------------------------------------------------------*/
extern MotStaType idata mcState;
//extern TimeCnt Time;
//extern MotStaTim  MotorStateTime;
extern MotStaM    McStaSet;
//extern TailWindSet   xdata mcTailwind;
extern uint16     TimeCnt;

/* Exported functions ---------------------------------------------------------------------------*/
extern void MC_Control(void);
extern void MotorcontrolInit(void);
extern void McTailWindDealwith(void);
extern void Motor_PllStart(void);

#endif

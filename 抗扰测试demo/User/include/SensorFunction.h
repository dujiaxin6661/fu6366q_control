/* --------------------------- (C) COPYRIGHT 2021 Fortiortech ShenZhen -----------------------------
    File Name      : SensorFunction.h
    Author         : Fortiortech  Appliction Team
    Version        : V1.0
    Date           : 2021-12-06
    Description    : This file contains Sensor parameter used for Motor Control.
----------------------------------------------------------------------------------------------------
                                       All Rights Reserved
------------------------------------------------------------------------------------------------- */

/* Define to prevent recursive inclusion -------------------------------------------------------- */
#ifndef __SENSOR_FUNCTION_H_
#define __SENSOR_FUNCTION_H_

/******************************************************************************/
#include <FU68xx_2_Type.h>
/******************************************************************************/
typedef enum
{
    MRWait          = 0,
    MRCalibra       = 1,
    MRHallOffset    = 2,                                                    // 带Hall偏置校准态
    MRRun           = 3,
    MRNLC           = 4,
    MRReady         = 5,
    MRFinish        = 6
}SensorStaType;

typedef union
{
    uint8 SetMode;                                                          // 整个配置模式使能位
    struct
    {
			  uint8 StartCalibraSetFlag   :1;                                   // 开始校准
			  uint8 OverCalibraSetFlag    :1;                                    // 校准完成
        uint8 NLCSetFlag    :1;                                             // 电流校准的标志位
    } SetFlag;
}MRStaM;

typedef struct
{
    SensorStaType     MRState;
    MRStaM            MRStaSet;
	  uint16            State_Count;
}MRSySTypeDef;

extern  MRSySTypeDef    xdata   MRSyS;
extern  void MR_Control(void);

#endif


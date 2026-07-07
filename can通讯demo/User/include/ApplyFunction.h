/* --------------------------- (C) COPYRIGHT 2022 Fortiortech ShenZhen -----------------------------
    File Name      : ElectricToolFunction.h
    Author         : Fortiortech  Appliction Team
    Version        : V1.0
    Date           : 2022-01-07
    Description    : This file contains XX-XX-XX parameter used for Motor Control.
----------------------------------------------------------------------------------------------------
                                       All Rights Reserved
------------------------------------------------------------------------------------------------- */

/* Define to prevent recursive inclusion -------------------------------------------------------- */
#ifndef __APPLY_FUNCTION_H_
#define __APPLY_FUNCTION_H_

/******************************************************************************/
#include <FU68xx_2_Type.h>
/******************************************************************************/

#define PowerOn  0  
#define Run      1  
#define Wait     2


//typedef enum
//{
//    PowerOn,      
//    Run  ,          
//	  Wait          
//}ApplyStaType;


typedef struct
{
    uint8 SystemState;
	
}APPLYTypeDef;

//typedef struct
//{
//    ApplyStaType SystemState;
//	
//}APPLYTypeDef;

 
extern APPLYTypeDef xdata ApplyState;

extern void ApplyFun_Conrtrol(void);
extern void Apply_CAN_SetCommand(uint8 mode, uint8 flags, int16 target, int16 limit_ma, uint16 timeout_ms, uint8 seq);
extern void Apply_CAN_Tick1ms(void);
extern void Apply_CAN_GetStatus(uint8 *mode, uint8 *status, uint8 *seq, uint16 *age_ms);

#endif


/**
 * @copyright (C) COPYRIGHT 2022 Fortiortech Shenzhen
 * @file      main.c
 * @author    Fortiortech  Appliction Team
 * @since     Create:2023-07
 * @date      Last modify:2023-07
 * @note      Last modify author is Kobe Peng
 * @brief      
 */
 
#include "AddFunction.h"
#include "Driver.h"
#include "Parameter.h"
#include "customer.h"
#include <ServoControl.h>

void Driver_Init(void)
{
    DRV_ARR = PWM_VALUE_LOAD ;     	// 载波频率的周期值
    DRV_DTR = PWM_LOAD_DEADTIME;	// 死区时间
    DRV_DR  = 0;
    DRV_CMR = 0x0A80;               // UH/VH/WH UL/VL/WL 互补
    DRV_OUT = 0x00;                 // 空闲电平，默认高电平有效

	/*驱动有效电平和空闲电平*/
	#if (PWM_LEVEL_MODE == HIGH_LEVEL)
	{
        ClrBit(PI_CR , HINV);      // 反相关闭
        ClrBit(PI_CR , LINV);      // 反相关闭
	}
	#elif (PWM_LEVEL_MODE == LOW_LEVEL)
	{
        SetBit(PI_CR , HINV);  // 反相打开
        SetBit(PI_CR , LINV);  // 反相打开
	}
	#elif (PWM_LEVEL_MODE == UP_H_DOWN_L)
	{
        ClrBit(PI_CR , HINV);  // 反相关闭
        SetBit(PI_CR , LINV);  // 反相打开
	}
	#elif (PWM_LEVEL_MODE == UP_L_DOWN_H)
	{
        SetBit(PI_CR , HINV);  // 反相打开
        ClrBit(PI_CR , LINV);  // 反相关闭
	}
	#endif //end PWM_Level_Mode   
        
    ClrBit(DRV_SR , FGIE);  //FG中断使能			0-->Disable		1-->Enable

    
    /**************************************************
    DRV比较匹配中断模式
    当计数值等于DRV_COMR时，根据DCIM的设置判断是否产生中断标记
    00：不产生中断        01：上升方向
    10：下降方向          11：上升/下降方向
    *************************************************/
    ClrBit(DRV_SR , DCIM1);
    SetBit(DRV_SR , DCIM0);

    /*设置DRV计数器的比较匹配值，当DRV计数值与COMR相等时，根据DRV_SR寄存器的DCIM是否产生比较匹配事件*/
    DRV_COMR = (PWM_VALUE_LOAD >> 2)-100;

    SetBit(IP0, PDRV1);  //中断优先级设置为1，优先级低于FO硬件过流    2
    ClrBit(IP0, PDRV0);
   SetBit(DRV_SR , DCIP);	//0-->1个计数周期产生中断  1-->2个计数周期产生中断

		SetBit(DRV_CR, FOCEN);	 
    /*MESEL为0，ME模块工作在BLDC模式
    MESEL为1，ME模块工作在FOC/SVPWM/SPWM模式*/
    SetBit(DRV_CR , MESEL);

    SetBit(DRV_CR , DRVEN);	// 计数器使能		0-->Disable 1-->Enable
    ClrBit(DRV_CR , DRPE);	// 计数器比较值预装载使能 0-->Disable		        1-->Enable
    SetBit(DRV_CR , DRVOE);	// Driver输出使能0-->Disable		1-->Enable

  	SetBit(CCFG2, VBB_VSEL);  //VBB_LDO设置 0：10V 1：12V
  	ClrBit(CCFG1, VBB_DIS);  //VBB_LDO使能输出后需要延时等待1ms以上

    FOC_EKP = OBSW_KP_GAIN;//OBSW_KP_GAIN_RUN4;      _Q12(3.0)       
    FOC_EKI = OBSW_KI_GAIN;//OBSW_KI_GAIN_RUN4;      _Q15(0.1)
		
		FOC_FBASE     = MR_FBASE1;                            // 由速度计算角度增量的系数
    FOC_OMEKLPF   = SPEED_KLPF;
		
		SetBit(DRV_CR, OCS);	     //开始PLL计算
    SetBit(DRV_CR, FOCEN);	   //开始PLL计算
}


 

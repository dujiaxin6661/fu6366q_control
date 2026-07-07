/**
 * @copyright (C) COPYRIGHT 2022 Fortiortech Shenzhen
 * @file      main.c
 * @author    Fortiortech  Appliction Team
 * @since     Create:2023-07
 * @date      Last modify:2023-07
 * @note      Last modify author is Kobe Peng
 * @brief      
 */

#include "CMP.h"
int16 DAC_OvercurrentValue_Temp = 0;

void CMP3_Init(void)
{
	  SetBit(CMP_CR4, CMP3P4M_FS);     //P3.4复用为比较器3的过流端口
    SetBit(P3_AN, P34);

    ClrBit(CMP_CR1 , CMP3MOD1);     //00-->P27-单比较器模式    01-->P20/P23-双比较器模式
    ClrBit(CMP_CR1 , CMP3MOD0);     //1X-->P20/P23/P27-三比较器模式

    #if (Compare_Mode == Compare_Hardware)
    {
        /* -----P2.6使能其模拟功能，使能数字输出----- */
        SetBit(P2_AN, P26);
        SetBit(P2_OE, P26);
        ClrBit(DAC_CR, DAC0_1EN);
    }
    #else
    {
//			SetBit(P2_AN, P26);
//			SetBit(P2_OE, P26);
        /* -----使能DAC----- */
         SetBit(DAC_CR, DAC0_1EN );
        /*  -------------------------------------------------------------------------------------------------
            1、DAC电压模式配置
            2、0: 正常模式，DAC输出电压范围为0到VREF，适用于硬件过流保护
            3、1: 半电压转换模式，DAC输出电压范围为VHALF到VREF
            -------------------------------------------------------------------------------------------------*/
        ClrBit(DAC_CR, DACMOD );
    
        /* -----设置DAC过流值----- */
        DAC0_DR = DAC_OverCurrentValue; 
    }
    #endif  //end Compare_Mode
		
    SetBit(CMP_CR1, CMP3HYS);                                       // 比较器3有迟滞电压
    /*  -------------------------------------------------------------------------------------------------
        1、选择母线电流保护触发信号源，外部中断0或者比较器3中断。
        2、1-INT0, 0-CMP3
        -------------------------------------------------------------------------------------------------*/
    ClrBit(EVT_FILT, INT0_MOE_EN);
    /*  -------------------------------------------------------------------------------------------------
        1、触发硬件保护后硬件关闭驱动输出MOE配置
        2、00--MOE不自动清零
        3、01--MOE自动清零
        -------------------------------------------------------------------------------------------------*/
    SetReg(EVT_FILT, MOEMD0 | MOEMD1, MOEMD0);
    //  SetReg(EVT_FILT, MOEMD0 | MOEMD1, 0x00);
    /*  -------------------------------------------------------------------------------------------------
        1、母线电流保护时间滤波宽度
        2、00-不滤波    01-4cpu clock    10-8cpu clock    11-16cpu clock
        -------------------------------------------------------------------------------------------------*/
    SetReg(EVT_FILT, EFDIV0 | EFDIV1, EFDIV0 | EFDIV1);
    /*比较器使能-------------------------------------------------------------------------------------*/
    SetBit(CMP_CR1, CMP3EN);
}


void CMP3_Interrupt_Init(void)
{
    ClrBit(CMP_SR , CMP3IF);
    /*------------------------------------------------------------------------
    比较器中断模式配置
    00: 不产生中断  
    01: 上升沿产生中断  
    10: 下降沿产生中断  
    11: 上升/下降沿产生中断
    ------------------------------------------------------------------------*/
    ClrBit(CMP_CR0 , CMP3IM1);
    SetBit(CMP_CR0 , CMP3IM0);

    SetBit(IP3 , PCMP31);    // 中断优先级别3
    SetBit(IP3 , PCMP30);                                                   // 中断优先级别3
}

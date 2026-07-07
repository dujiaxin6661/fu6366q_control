/**
 * @copyright (C) COPYRIGHT 2022 Fortiortech Shenzhen
 * @file      main.c
 * @author    Fortiortech  Appliction Team
 * @since     Create:2023-07
 * @date      Last modify:2023-07
 * @note      Last modify author is Kobe Peng
 * @brief      
 */
 
#include <Myproject.h>
#include "ADC.h"

void ADC_Init(void)
{
		 /* ***********************VREF&VHALF Config*********************** */
    #if (HW_ADC_VREF == VREF3_0)
    {
        SetBit(VREF_VHALF_CR, VRVSEL1);             //00-->4.5V   01-->VDD5
        ClrBit(VREF_VHALF_CR, VRVSEL0);             //10-->3.0V   11-->4.0V
    }
    #elif (HW_ADC_VREF == VREF4_0)
    {
        SetBit(VREF_VHALF_CR, VRVSEL1);             //00-->4.5V   01-->VDD5
        SetBit(VREF_VHALF_CR, VRVSEL0);             //10-->3.0V   11-->4.0V
    }
    #elif (HW_ADC_VREF == VREF4_5)
    {
        ClrBit(VREF_VHALF_CR, VRVSEL1);             //00-->4.5V   01-->VDD5
        ClrBit(VREF_VHALF_CR, VRVSEL0);             //10-->3.0V   11-->4.0V
    }
    #elif (HW_ADC_VREF == VREF5_0)
    {
        ClrBit(VREF_VHALF_CR, VRVSEL1);             //00-->4.5V   01-->VDD5
        SetBit(VREF_VHALF_CR, VRVSEL0);             //10-->3.0V   11-->4.0V
    }
	  #else
    {
      #error " VHALF MODE Err "
    }
	 #endif
	
	  #if (VREF_OUT_EN)
      SetBit(P3_AN, PIN5);                         //VREF Voltage -->P35 Output 是否输出到P35引脚，需同步配置输出
      SetBit(P3_OE, PIN5);                         //VREF Voltage -->P35 Output 是否输出到P35引脚
      SetBit(VREF_VHALF_CR, VREFEN);
    #endif
		
	  #if (VHALF_OUT_EN)
      SetBit(P3_AN, P32);
      SetBit(VREF_VHALF_CR, VHALFEN);
    #endif
    
    /********************ADC 端口模拟功能设置************************/
    SetBit(P2_AN , PIN0);	//AD0 P20 CH open--IU--固定
    SetBit(P2_AN , PIN3);	//AD1 P23 CH open--IV--固定 
    SetBit(P2_AN , PIN4);	//AD2 P24 CH open--母线电压，注意分压比
		
    SetBit(P2_AN , PIN7);	//AD4  P27   MR_SIN
    SetBit(P1_AN , PIN3);	//AD12 P13   MR_SCOS
	
		SetBit(P2_AN , PIN5);	//AD3 P25 CH open   Ibus
		// P3.3 is used as GP33 key input in the speed-step baseline test.
		// Do not enable AD6 here, otherwise the key may not read reliably.
		SetBit(P3_AN , PIN4);	//AD7 P34 CH open   Icp
		
	/****************************************************************/
    SetBit(ADC_MASK , CH0EN | CH1EN | CH2EN | CH4EN | CH12EN | CH3EN | CH7EN);//相应通道使能

    SetBit(ADC_CR , ADCALIGN); 		//ADC数据次高位对齐使能0-->Disable	1-->Enable
    ClrBit(ADC_CR , ADCIE); 		  //ADC中断使能
    SetBit(ADC_CR , ADCEN);       //Enable ADC0  
}

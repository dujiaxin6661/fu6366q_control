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
#include "AMP.h"


void Current_AMP_Init(void) //相电流运放初始化，默认FOC采集UV两相电流
{
    #if (Shunt_Resistor_Mode == Double_Resistor)
    {
        /*****AMP 端口模拟功能设置******/
        SetBit(P3_AN , P31);            //AMP0 Pin设置为模拟模式  +
        SetBit(P3_AN , P30);            //AMP0 Pin设置为模拟模式  -
        SetBit(P2_AN , P27);            //AMP0 Pin设置为模拟模式  O

        SetBit(AMP_CR0 , AMP0EN);        //AMP0 Enable
        
        SetBit(P1_AN , P16);            //AMP1 Pin设置为模拟模式  +
        SetBit(P1_AN , P17);            //AMP1 Pin设置为模拟模式  -
        SetBit(P2_AN , P20);            //AMP1 Pin设置为模拟模式  O

        SetBit(P2_AN , P21);            //AMP2 Pin设置为模拟模式  +
        SetBit(P2_AN , P22);            //AMP2 Pin设置为模拟模式  -
        SetBit(P2_AN , P23);            //AMP2 Pin设置为模拟模式  O
        ClrBit(P2_OE , P23);            //P23_OE需要强制为0，禁止DA1输出至PAD  

        SetBit(AMP_CR0 , AMP1EN);        //AMP1 Enable
        SetBit(AMP_CR0 , AMP2EN);        //AMP2 Enable
    }
    #endif
	
		#if (AMP_MODE ==INTERNAL)
    {
        //000-->Genaral AMP 001-->2X 010-->4X 011-->8X 100-->16X  
			
        /**********AMP1、2 PGA SET************/
        #if (HW_AMPGAIN == AMP2x)
        {
            ClrBit(AMP_CR1, AMP_PH_GAIN2);  //0x80
            ClrBit(AMP_CR1, AMP_PH_GAIN1);  //0x40
            SetBit(AMP_CR1, AMP_PH_GAIN0);  //0x20
        }
        #elif (HW_AMPGAIN == AMP4x)
        {
            ClrBit(AMP_CR1, AMP_PH_GAIN2);  //0x80
            SetBit(AMP_CR1, AMP_PH_GAIN1);  //0x40
            ClrBit(AMP_CR1, AMP_PH_GAIN0);  //0x20		
        }
        #elif (HW_AMPGAIN == AMP8x)
        {
            ClrBit(AMP_CR1, AMP_PH_GAIN2);  //0x80
            SetBit(AMP_CR1, AMP_PH_GAIN1);  //0x40
            SetBit(AMP_CR1, AMP_PH_GAIN0);  //0x20
        }
        #elif (HW_AMPGAIN == AMP16x)
        {
            SetBit(AMP_CR1, AMP_PH_GAIN2);
            ClrBit(AMP_CR1, AMP_PH_GAIN1);
            ClrBit(AMP_CR1, AMP_PH_GAIN0); 
        }
        #endif
    }
    #else		
    {	
   			ClrBit(AMP_CR1, AMP_PH_GAIN2);  //放大倍数由外部硬件电路决定
				ClrBit(AMP_CR1, AMP_PH_GAIN1);
				ClrBit(AMP_CR1, AMP_PH_GAIN0);			
    }
    #endif			
}


void MR_AMP_Init(void)//MR检测需要运放进行放大，初始化运放0以及运放3
{
		/*****AMP0 端口模拟功能设置******/
		SetBit(P3_AN , P31);            //AMP0 Pin设置为模拟模式  +
		SetBit(P3_AN , P30);            //AMP0 Pin设置为模拟模式  -
		SetBit(P2_AN , P27);            //AMP0 Pin设置为模拟模式  O
		SetBit(AMP_CR0 , AMP0EN);       //AMP0 Enable
        
		/*****AMP3 端口模拟功能设置******/
		SetBit(P1_AN , P15);            //AMP3 Pin设置为模拟模式  +
		SetBit(P1_AN , P14);            //AMP3 Pin设置为模拟模式  -
		SetBit(P1_AN , P13);            //AMP3 Pin设置为模拟模式  O
		SetBit(AMP_CR0 , AMP3EN);       //AMP3 Enable
	
	 /*******设置放大倍数********/
	  //000-->Genaral AMP 001-->2X 010-->4X 011-->8X 100-->16X  
	
		SetBit(AMP_CR1, AMP0_GAIN2);
		ClrBit(AMP_CR1, AMP0_GAIN1);
		ClrBit(AMP_CR1, AMP0_GAIN0); //运放0放大16倍
	
		SetBit(AMP_CR2, AMP3_GAIN2);
		ClrBit(AMP_CR2, AMP3_GAIN1);
		ClrBit(AMP_CR2, AMP3_GAIN0); //运放3放大16倍
}




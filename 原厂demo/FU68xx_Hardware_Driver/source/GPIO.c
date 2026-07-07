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
#include "GPIO.h"


void GPIO_Init(void)
{
    SetBit(P0_OE , PIN4);
    SetBit(P0_PU , PIN4);

    ClrBit(P0_OE , PIN2); 
    SetBit(P0_PU , PIN2);
	
		SetBit(P0_OE , PIN4); 
    SetBit(P0_PU , PIN4); 
	
	  ClrBit(P3_OE , PIN6); 
    SetBit(P3_PU , PIN6);  
	
    ClrBit(P4_OE , PIN5); //KeyUp 
    SetBit(P4_PU , PIN5);	//PullUp
	
	  ClrBit(P3_OE , PIN3); //KeyDown 
    SetBit(P3_PU , PIN3);	//PullUp 			
}

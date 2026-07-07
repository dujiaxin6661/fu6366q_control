/**
 * @copyright (C) COPYRIGHT 2022 Fortiortech Shenzhen
 * @file      main.c
 * @author    Fortiortech  Appliction Team
 * @since     Create:2023-07
 * @date      Last modify:2023-07
 * @note      Last modify author is Kobe Peng
 * @brief      
 */
 /* Includes -------------------------------------------------------------------------------------*/
#include <Myproject.h>
 
/*  -------------------------------------------------------------------------------------------------
    Function Name  : LPFFunction
    Description    : 低通滤波函数
    Date           : 2021-08-08
    Parameter      : Xn1: [输入]
**                   Xn0: [输入]
**                   K: [输入]
    ------------------------------------------------------------------------------------------------- */
int16 LPFFunction0(int16 Xn1, int16 Xn0, int8 K)
{
    LPF0_K = K << 8;
    LPF0_X = Xn1;
    LPF0_YH = Xn0;
    SMDU_RunBlock(0, LPF);
    return LPF0_YH;
}
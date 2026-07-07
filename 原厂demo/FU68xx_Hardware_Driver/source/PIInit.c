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

void PI0_Init(void)
{
    PI0_KP  = SKP;
    PI0_KI  = SKI;
    PI0_UKH = 0;
    PI0_UKL = 0;
    PI0_EK  = 0;
    PI0_EK1 = 0;
    PI0_UKMAX = SOUTMAX;
    PI0_UKMIN = SOUTMIN;
    SMDU_RunBlock(0, PI);
}

void PI2_Init(void)
{
    PI2_KP  = SKP;
    PI2_KI  = SKI;
    PI2_UKH = 0;
    PI2_UKL = 0;
    PI2_EK  = 0;
    PI2_EK1 = 0;
    PI2_UKMAX = SOUTMAX;
    PI2_UKMIN = SOUTMIN;
    SMDU_RunBlock(2, PI);
}

void PI1_Init(void)
{	
	  PI1_KP  = SKP;
    PI1_KI  = SKI;
    PI1_UKH = 0;
    PI1_UKL = 0;
    PI1_EK  = 0;
    PI1_EK1 = 0;
    PI1_UKMAX = SOUTMAX;
    PI1_UKMIN = SOUTMIN;
    SMDU_RunBlock(1, PI);  
}


/*-------------------------------------------------------------------------------------------------
    Function Name : int16 HW_One_PI(int16 Xn1, int16 Yn0, int16 Xn2)
    Description   : PI控制
    Input         : Xn1--E(K)
    Output        : PI_UK--当前PI输出值,执行时间us
-------------------------------------------------------------------------------------------------*/
int16 HW_One_PI1(int16 Xn1)
{
    PI1_EK =  Xn1;                                                                               //填入EK
    SMDU_RunBlock(1, PI);
    return PI1_UKH;
}

/*-------------------------------------------------------------------------------------------------
    Function Name : int16 HW_One_PI(int16 Xn1, int16 Yn0, int16 Xn2)
    Description   : PI控制
    Input         : Xn1--E(K)
    Output        : PI_UK--当前PI输出值,执行时间us
-------------------------------------------------------------------------------------------------*/
int16 HW_One_PI0(int16 Xn1)
{
    PI0_EK =  Xn1;                                                                               //填入EK
    SMDU_RunBlock(0, PI);
    return PI0_UKH;
}
/*-------------------------------------------------------------------------------------------------
    Function Name : int16 HW_One_PI(int16 Xn1, int16 Yn0, int16 Xn2)
    Description   : PI控制
    Input         : Xn1--E(K)
    Output        : PI_UK--当前PI输出值,执行时间us
-------------------------------------------------------------------------------------------------*/
int16 HW_One_PI2(int16 Xn1)
{
    PI2_EK =  Xn1;                                                                               //填入EK
    SMDU_RunBlock(2, PI);
    return PI2_UKH;
}

/*-------------------------------------------------------------------------------------------------
    Function Name : int16 HW_One_PI(int16 Xn1, int16 Yn0, int16 Xn2)
    Description   : PI控制
    Input         : Xn1--E(K)
    Output        : PI_UK--当前PI输出值,执行时间us
-------------------------------------------------------------------------------------------------*/
int16 HW_One_PI3(int16 Xn1)
{
    PI3_EK =  Xn1;                                                                               //填入EK
    SMDU_RunBlock(3, PI);
    return PI3_UKH;
}
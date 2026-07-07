/**************************** (C) COPYRIGHT 2015 Fortiortech shenzhen *****************************
* File Name          : PIInit.h
* Author             : Fortiortech  Market Dept
* Version            : V1.0
* Date               : 01/07/2015
* Description        : This file contains all the common data types used for Motor Control.
***************************************************************************************************
* All Rights Reserved
**************************************************************************************************/ 

/* Define to prevent recursive inclusion --------------------------------------------------------*/
#ifndef _PIINIT_H_
#define _PIINIT_H_

/* Exported functions ---------------------------------------------------------------------------*/
extern void PI0_Init(void);
extern void PI1_Init(void);
extern void PI2_Init(void);
extern void PI3_Init(void);
extern int16  HW_One_PI0(int16 Xn1);
extern int16  HW_One_PI2(int16 Xn1);
extern int16  HW_One_PI3(int16 Xn1);
extern int16  HW_One_PI1(int16 Xn1);

#endif
#ifndef __TIMER_H__
#define __TIMER_H__



 #define TIM0_Fre                       (24000000.0)                              // TIM0计数频率24MHz
 #define TIM1_Fre                       (750000.0)                                // TIM1计数频率750KHz
 #define TIM2_Fre                       (24000000.0)                               // TIM2计数频率12MHz
 #define TIM3_Fre                       (375000.0)                              // TIM3计数频率1500KHz
 #define TIM4_Fre                       (750000.0)                                // TIM4计数频率24MHz
 
 #define TIM3_FreL                      (0xE360)
 #define TIM3_FreH                      (0x16)  
/*************************************************************************************///External Function
extern void Timer1_Init(void);
extern void Timer2_Init(void);
extern void Timer2_QEP_Init(void);
extern void Timer2_Step_Init(void);
extern void Timer3_Init(void);
extern void Timer4_Init(void);

#endif

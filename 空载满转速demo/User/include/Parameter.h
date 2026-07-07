/**************************** (C) COPYRIGHT 2018 Fortiortech shenzhen *****************************
* File Name          : Parameter.h
* Author             : Bruce,Fortiortech  RD
* Version            : V1.0
* Date               : 2017-12-20
* Description        : This file contains all FOC debug parameter.
***************************************************************************************************
* All Rights Reserved
**************************************************************************************************/

/* Define to prevent recursive inclusion --------------------------------------------------------*/

#ifndef __Parameter_H_
#define __Parameter_H_

/* Define to prevent recursive inclusion -------------------------------------*/
/* Q format define ---------------------------------------------------------------------------------*/
#define _Q7(A)                          (int8) ((A) * 0x7f)                                              // Q7 format
#define _Q8(A)                          (int16)((A) * 0xff)                                              // Q8 format
#define _Q9(A)                          (int16) (A  * 0x1ff)                                             // Q9 format
#define _Q10(A)                         (int16)((A) * 0x3ff)                                             // Q10 format
#define _Q11(A)                         (int16)((A) * 0x7ff)                                             // Q11 format
#define _Q12(A)                         (int16)((A) * 0xfff)                                             // Q12 format
#define _Q13(A)                         (int16)((A) * 0x1fff)                                            // Q13 format
#define _Q14(A)                         (int16)((A) * 0x3fff)                                            // Q14 format
#define _Q15(A)                         (int16)((A) * 0x7fff)                                            // Q15 format
#define _2PI                            (3.1415926  * 2)                                                 // 2PI value
#define _Q16                            (65535.0)                                                        // Q16 format 
                                                   

#define _Q15Limit(A)                    (A>(int32)32767)?32767:((A<(int32)(-32768))?-32768:A)
#define MaxMinLimit(Max,Min,A)          (A>(int32)Max)?Max:((A<(int32)(Min))?Min:A)
/* Private define ------------------------------------------------------------*/
#define SystemPowerUpTime               (uint16)(60000)                                       // 上电等待时间，cpu计数时钟

#define INTERNAL                        (0)   
#define EXTERNAL                        (1)   

#define Single_Resistor                 (0)                                     // 单电阻电流采样模式
#define Double_Resistor                 (1)                                     // 双电阻电流采样模式
#define Three_Resistor                  (2)                                     // 三电阻电流采样模式
#define New_Single_Resistor             (3)                                     //新单电阻

#define VREF4_5		      	              (0)
#define VREF5_0			                    (1)
#define VREF3_0                         (2)
#define VREF4_0			                    (3)

/* 内部PGA放大倍数选择 */
#define AMP2x                           (2)           
#define AMP4x                           (4)
#define AMP8x                           (8)
#define AMP16x                          (16)
#define AMP32x                          (32)

/*芯片参数值------------------------------------------------------------------*/
/*CPU and PWM Parameter*/
#define PWM_CYCLE                       (1000.0 / PWM_FREQUENCY)                      // 周期us
#define SAMP_FREQ                       (PWM_FREQUENCY * 1000)                        // 采样频率(HZ)
#define TPWM_VALUE                      (1.0 / SAMP_FREQ)                             // 载波周期(S)
#define PWM_VALUE_LOAD                  (uint16)(MCU_CLOCK * 500 *4/ PWM_FREQUENCY)     // PWM 定时器重载值 

/*double resistor sample Parameter*/
#define DLL_TIME                        (2.8)//(0.2)            // 双电阻最小脉宽设置(us),建议值为死区时间值+0.2us以上 //低压的时候变为0最好，噪声问题
/*three resistor overmodule Parameter*/
#define OVERMOD_TIME                    (2.0)            // 三电阻过调制时间(us)，建议值2.0
/*deadtime compensation*/
#define DT_TIME                         (0.0)            // 死区补偿时间(us)，适用于双电阻和三电阻，建议值是1/2死区时间
/*min pulse*/
#define GLI_TIME                        (0.0)            // 桥臂窄脉宽消除(us),建议值0.5

/*deadtime Parameter*/
#define PWM_LOAD_DEADTIME               (PWM_DEADTIME * MCU_CLOCK)             // 死区设置值

#define PWM_OVERMODULE_TIME             (OVERMOD_TIME * MCU_CLOCK / 2)         // 过调制时间
#define PWM_DLOWL_TIME                  (DLL_TIME * MCU_CLOCK / 2)             //下桥臂最小时间
/*single resistor sample Parameter*/
#define PWM_TS_LOAD                     (uint16)(_Q16 / PWM_CYCLE * MIN_WIND_TIME / 16)            // 单电阻采样设置值
#define PWM_DT_LOAD                     (uint16)(_Q16 / PWM_CYCLE * DT_TIME / 16)                  // 死区补偿值
#define PWM_TGLI_LOAD                   (0)//(uint16)(_Q16 / PWM_CYCLE * (GLI_TIME + PWM_DEADTIME) / 16)  // 最小脉冲

/*硬件板子参数设置值------------------------------------------------------------------*/
/*hardware current sample Parameter*/
/*电流基准的电路参数*/
#define HW_BOARD_CURR_MAX               (HW_ADC_REF / 2 / HW_AMPGAIN / HW_RSHUNT) // 最大采样电流,2.702A
#define HW_BOARD_CURR_MIN               (-HW_BOARD_CURR_MAX)                                        // 最小采样电流,-2.702A
#define HW_BOARD_CURR_BASE              (HW_BOARD_CURR_MAX * 2)                                       // 电流基准//5.4A

/*hardware voltage sample Parameter*/
/*母线电压采样分压电路参数*/
#define HW_BOARD_VOLTAGE_BASE           (HW_BOARD_VOLT_MAX / 1.732)                              // 电压基准
#define HW_BOARD_VOLTAGE_VC             ((RV1 + RV2 + RV3 * VC1) / (RV3 * VC1))
#define HW_BOARD_VOLTAGE_BASE_Start     (HW_ADC_REF * HW_BOARD_VOLTAGE_VC / 1.732)               // 电压基准

/*硬件过流保护DAC值*/
#define DAC_OverCurrentValue              _Q9(I_ValueBus(OverHardcurrentValue))   ///< 该结果是进行了右移一位的结果

#define Align_Theta_0                     _Q15((float)Align_Angle_0 / 180.0)
#define Align_Theta_90                    _Q15((float)Align_Angle_90 / 180.0)
#define Align_Theta_270                    _Q15((float)Align_Angle_270 / 180.0)

#define BASE_FREQ                       ((MOTOR_SPEED_BASE / 60) * Pole_Pairs)  // 基准频率

/*保护参数值------------------------------------------------------------------*/
/* protect value */
#define OVER_PROTECT_VALUE              _Q15(Over_Protect_Voltage  / HW_BOARD_VOLT_MAX)
#define LIMIT_PROTECT_VALUE             _Q15(Limit_Protect_Voltage / HW_BOARD_VOLT_MAX)
#define UNDER_PROTECT_VALUE             _Q15(Under_Protect_Voltage / HW_BOARD_VOLT_MAX)
#define OVER_RECOVER_VALUE              _Q15(Over_Recover_Vlotage  / HW_BOARD_VOLT_MAX)
#define UNDER_RECOVER_VALUE             _Q15(Under_Recover_Vlotage / HW_BOARD_VOLT_MAX)

/* motor speed set value */
#define S_Value(Speed)					    _Q15(Speed/(float)MOTOR_SPEED_BASE) 
#define Motor_Open_Ramp_ACC             _Q15(MOTOR_OPEN_ACC     / MOTOR_SPEED_BASE)
#define Motor_Open_Ramp_Min             _Q15(MOTOR_OPEN_ACC_MIN / MOTOR_SPEED_BASE)

#define Motor_Omega_Ramp_Min            _Q15(MOTOR_OMEGA_ACC_MIN / MOTOR_SPEED_BASE)
#define Motor_Omega_Ramp_End            _Q15(MOTOR_OMEGA_ACC_END / MOTOR_SPEED_BASE)

#define Motor_Loop_Speed                _Q15(MOTOR_LOOP_RPM / MOTOR_SPEED_BASE)
#define Motor_Switch_Speed              _Q15(Hall_switch_Sensorless_RPM / MOTOR_SPEED_BASE)

#define Motor_Posicheck_Speed           _Q15(MOTOR_SPEED_POSICHECK_RPM   / MOTOR_SPEED_BASE)
#define Motor_Max_Speed                 _Q15(MOTOR_SPEED_MAX_RPM   / MOTOR_SPEED_BASE)
#define Motor_Min_Speed                 _Q15(MOTOR_SPEED_MIN_RPM   / MOTOR_SPEED_BASE)


#define Motor_Limit_Speed               _Q15(MOTOR_SPEED_LIMIT_RPM / MOTOR_SPEED_BASE)
#define Motor_Stop_Speed                _Q15(MOTOR_SPEED_STOP_RPM  / MOTOR_SPEED_BASE)

#define Motor_Stall_Min_Speed           _Q15(MOTOR_SPEED_STAL_MIN_RPM / MOTOR_SPEED_BASE)
#define Motor_Stall_Max_Speed           _Q15(MOTOR_SPEED_STAL_MAX_RPM / MOTOR_SPEED_BASE)

/*外环增量*/
//#define Motor_Speed_Inc                 _Q15(SPEED_INC / MOTOR_SPEED_BASE)
//#define Motor_Speed_Dec                 _Q15(SPEED_DEC / MOTOR_SPEED_BASE)

#define SPEED_FreK                      _Q15(Speed_PerFre / MOTOR_SPEED_BASE)
#define SPEED_K                         ((float)(Motor_Max_Speed-Motor_Min_Speed)/(float)(MAXPWMDuty-MINPWMDuty))
#define POWER_K                         ((float)(Motor_Max_Power-Motor_Min_Power)/(float)(MAXPWMDuty-MINPWMDuty))
#define CURRENT_K                       ((float)(QOUTMAXVALUE-QOUTMINVALUE)/(float)(MAXPWMDuty-MINPWMDuty))

#define Wc                              (5000.0)
#define I_KP_Suggest                    LD*Wc/(Run_Voltage*HW_RSHUNT*HW_AMPGAIN/HW_ADC_REF)
#define I_KI_Suggest                    RS*Wc/SAMP_FREQ/(Run_Voltage*HW_RSHUNT*HW_AMPGAIN/HW_ADC_REF)

#define I_KP_Suggest1                   LD*Wc/(Run_Voltage/2.45*HW_RSHUNT*HW_AMPGAIN/HW_ADC_REF)
#define I_KI_Suggest1                   RS*Wc/SAMP_FREQ/(Run_Voltage/2.45*HW_RSHUNT*HW_AMPGAIN/HW_ADC_REF)



/* obsever parameter set value */
#define MAX_BEMF_VOLTAGE                ((MOTOR_SPEED_BASE*Ke)/(1000.0))
#define MAX_OMEG_RAD_SEC                ((float)(_2PI*BASE_FREQ))


#if (!AdapEnable)
    #define OBS_K1T                         _Q15(LD/(LD+RS*TPWM_VALUE))
    #define OBS_K2T                         _Q13(1.732*(TPWM_VALUE/(LD+RS*TPWM_VALUE))*(HW_BOARD_VOLTAGE_BASE_Start/HW_BOARD_CURR_BASE))
    #define OBS_K3T                         _Q15((TPWM_VALUE/(LD+RS*TPWM_VALUE))*(MAX_BEMF_VOLTAGE/HW_BOARD_CURR_BASE))
    #define OBS_K4T                         _Q15(((LD-LQ)*TPWM_VALUE*MAX_OMEG_RAD_SEC)/(LD+RS*TPWM_VALUE))
#else
	#define OBS_K1T                         _Q11((1.0*3.0/(125.0*1.0))*(LQ/TPWM_VALUE)*(HW_BOARD_CURR_BASE/HW_BOARD_VOLTAGE_BASE))
	#define OBS_K2T                         _Q11(1.0*RS*HW_BOARD_CURR_BASE/HW_BOARD_VOLTAGE_BASE)
	//#define OBS_K2T_SMO                     _Q11(1.0*RS*HW_BOARD_CURR_BASE/HW_BOARD_VOLTAGE_BASE)
	#define OBS_K3T                         _Q8(2.5)//2.5
	#define OBS_K4T                         _Q15(((LD-LQ)*TPWM_VALUE*MAX_OMEG_RAD_SEC)/(LD+RS*TPWM_VALUE))
#endif


#define OBSW_KP_GAIN                    _Q12(2*_2PI*ATT_COEF*ATO_BW/BASE_FREQ)
#define OBSW_KI_GAIN                    _Q15(_2PI*ATO_BW*ATO_BW*TPWM_VALUE/BASE_FREQ)

#define OBS_FBASE                       _Q15(BASE_FREQ*TPWM_VALUE) 
#define SPEED_KLPF                      _Q15(_2PI*SPD_BW*TPWM_VALUE)
#define SPD_BW                         (10.0)           ///< 速度带宽的滤波值，经典值为5.0-40.0  


//#define OBS_KLPF                        _Q15(_2PI*BASE_FREQ*TPWM_VALUE)    // 2PI*Fbase*Tpwm
//#define SPEED_KLPF                      _Q15(_2PI*SPD_BW*TPWM_VALUE)       // 2PI*SPD_BW*Tpwm
//#define OBS_EA_KS                       _Q15((2*MOTOR_SPEED_SMOMIN_RPM*_2PI*BASE_FREQ*TPWM_VALUE)/MOTOR_SPEED_BASE)     // SMO的最小速度

//#define OBSE_PLLKP_GAIN                 _Q11(((2*ATT_COEF*_2PI*E_BW*LD - RS)*HW_BOARD_CURR_BASE)/HW_BOARD_VOLTAGE_BASE)
//#define OBSE_PLLKI_GAIN                 _Q11((_2PI*E_BW*_2PI*E_BW*LD*TPWM_VALUE*HW_BOARD_CURR_BASE)/HW_BOARD_VOLTAGE_BASE)


//#define OBSE_PLLKP_GAIN_1                 _Q11(((2*ATT_COEF*_2PI*E_BW_1*LD - RS)*HW_BOARD_CURR_BASE)/HW_BOARD_VOLTAGE_BASE)
//#define OBSE_PLLKI_GAIN_1                 _Q11((_2PI*E_BW*_2PI*E_BW_1*LD*TPWM_VALUE*HW_BOARD_CURR_BASE)/HW_BOARD_VOLTAGE_BASE)

///*逆风判断时的估算算法设置值------------------------------------------------------------*/
//#define SPEED_KLPF_WIND                 _Q15(_2PI*SPD_BW_Wind*TPWM_VALUE)           // 2PI*SPD_BW_Wind*Tpwm
//#define OBSW_KP_GAIN_WIND               _Q12(2*_2PI*ATT_COEF*ATO_BW_Wind/BASE_FREQ)
////#define   OBSW_KI_GAIN_WIND           _Q15(_2PI*0.5*ATO_BW_Wind*ATO_BW_Wind*TPWM_VALUE/BASE_FREQ)//---SMO
//#define OBSW_KI_GAIN_WIND               _Q15(_2PI*ATO_BW_Wind*ATO_BW_Wind*TPWM_VALUE/BASE_FREQ)//---PLL

/*Current Calib:enable or disable*/
#define Disable                         (0)                              //
#define Enable                          (1)                              //
#define CalibENDIS                      (Enable)

/*SVPWM mode*/
#define SVPWM_5_Segment                 (0)                              // 五段式SVPWM
#define SVPWM_7_Segment                 (1)                              // 七段式SVPWM
#define SVPMW_Mode                      (SVPWM_7_Segment)

/*double resistor sample mode*/
#define DouRes_1_Cycle                  (0)                              // 1 周期采样完 ia, ib
#define DouRes_2_Cycle                  (1)                              // 交替采用ia, ib, 2周期采样完成
#define DouRes_Sample_Mode              (DouRes_1_Cycle)

#endif
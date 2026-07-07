/**
 * @copyright (C) COPYRIGHT 2022 Fortiortech Shenzhen
 * @file      Parameter.h
 * @author    Kobe.Peng
 * @note      Last modify author is Kobe Peng
 * @since     2023-07
 * @date      2023-07
 * @brief       
 */
 
#ifndef __CUSTOMER_H_
#define __CUSTOMER_H_

 #define I_Value_Ibus(Curr_Value)       (_Q15(I_ValueBus(Curr_Value)))
 #define I_ValueBus(Curr_Value)          (Curr_Value * HW_BusRSHUNT / (HW_ADC_REF))          // 注意没有放大倍数

 #define I_ValueX(Curr_Value)           (Curr_Value * HW_RSHUNT * HW_AMPGAIN / (HW_ADC_REF))
 #define S_Value(Speed_Value)           _Q15(Speed_Value / MOTOR_SPEED_BASE)                // 输入单位：rpm/min,输出单位：标幺后转速
 #define ACC_TraPlan_Value(ACC_Value)   (65536*ACC_Value/1000/1000)                         // 输入单位：rpm/(s)²,输出单位：Q16(rpm/ms²)
 #define I_Value(Curr_Value)            _Q15(I_ValueX(Curr_Value))
 #define I_Value_0_1A(Curr_Value)       (3276.7*I_ValueX(Curr_Value))
 #define A_Value(Angle_Value)           _Q15(Angle_Value/ 180.0)
 #define A_Value_Half(Angle_Value)      _Q15(Angle_Value/ 360.0)
 #define ACC_Value(Speed_Value)         (int16)(Speed_Value/60.0*65536*Pole_Pairs/SAMP_FREQ)
 #define IActural_0_1A(Curr_Value)      (Curr_Value*HW_ADC_REF/3276.7/HW_RSHUNT/HW_AMPGAIN) // 输入单位：AD采样值，输出单位：实际电流：0.1A
 #define SActural_RPM(Speed_Value)      (Speed_Value*MOTOR_SPEED_BASE/32767.0)              // 输入单位：标幺后速度，输出单位：实际速度


/*芯片参数值-------------------------------------------------------------------*/
 /*CPU and PWM Parameter*/
 #define MCU_CLOCK                      (24.0)                                  // (MHz) 主频
 #define PWM_FREQUENCY                  (24.0)                                  // (kHz) 载波频率15

 /*deadtime Parameter*/
 #define PWM_DEADTIME                   (1.0)                                   // (us) 死区时间

 /*single resistor sample Parameter*/
 #define MIN_WIND_TIME                  (PWM_DEADTIME + 0.9)                    // (us) 单电阻最小采样窗口，建议值死区时间+0.9us


 /*电机参数值-------------------------------------------------------------------*/ 
 #define Pole_Pairs                     (11)                                     // 极对数
 
 /*电机参数值-----------有感FOC可以不需要以下电机参数-----------------------------*/ 
 
 #define RS                             (0.35)                                  // (Ω)  相电阻 ,测量两根相线之间电阻/2, ohm
 #define LD                             (0.0008)                                // (H)   D轴相电感 ,测量两根相线之间电感/2,H    
 #define LQ                             (0.0008)                                // (H)   Q轴电感
 
 
 #define KeVpp                          (5.84 * 1.0)                            // (V)   反电动势测量的峰峰值
 #define KeT                            (46.60)                                 // (ms)  反电动势测量的周期
 #define Ke                             (8.0)    // 反电动势常数, V/KRPM

 #define MOTOR_SPEED_BASE               (4000.0)                                // (RPM) 速度基准

/*硬件板子参数设置值------------------------------------------------------------*/

 /*电机电流采样相关硬件参数*/
 #define AMP_MODE                       (INTERNAL)                               // AMP0选择使用输入通道放大倍数来源，INTERNAL内部放大倍数  EXTERNAL外部放大倍数
 #define HW_RSHUNT                      (0.005)                                  // (Ω)  相电流采样电阻  
 #define HW_ADC_REF                     (4.5)                                    // (V)  ADC参考电压
 #define HW_AMPGAIN                     (AMP8x)                                  // 运放放大倍数

/*电机母线电流采样相关硬件参数*/
 #define HW_BusRSHUNT                   (0.020)                                  // (Ω)  母线电流采样电阻  
 

 /*电机驱动母线电压采样相关硬件参数*/
 #define VOLTAGE_MODE                   (EXTERNAL)                              // 母线电压选择使用分压通道，INTERNAL内部分压  EXTERNAL外部分压
 #define RV1                            (100.0)                                  // (kΩ) 母线电压分压电阻1   
 #define RV2                            (0.0)                                   // (kΩ) 母线电压分压电阻2
 #define RV3                            (4.7)                                   // (kΩ) 母线电压分压电阻3   
 #define VC1                            (1.0)                                   // 电压补偿系数
 #define RV                             ((RV1 + RV2 + RV3) / RV3)               // 分压比
 
 #define HW_BOARD_VOLT_MAX              (HW_ADC_REF * RV)                       // (V)  ADC可测得的最大母线电压


/**
 * 基准电压VREF对外输出使能
 * @param (Disable)       禁止
 * @param (Enable)       使能
 */
 
 
#define VREF_OUT_EN                     (Enable)         ///< 基准电压VREF对外输出使能

/**
 * 参考电压设置
 * @param (VREF3_0)       参考电压设置为3.0V
 * @param (VREF4_0)       参考电压设置为4.0V
 * @param (VREF4_5)       参考电压设置为4.5V
 * @param (VREF5_0)       参考电压设置为5.0V
 */
#define HW_ADC_VREF                    (VREF4_5)        ///< (V)  ADC参考电压

/**
 * 电流采样模式选择
 * @param (Single_Resistor)       单电阻电流采样模式
 * @param (Double_Resistor)       双电阻电流采样模式
 * @param (Three_Resistor)        三电阻电流采样模式
 * @param (New_Single_Resistor)   新单电阻电流采样模式
 */
 
 /**
 * VHALF输出使能
 * @param (Disable)       禁止
 * @param (Enable)        使能
 */
 
#define VHALF_OUT_EN                    (Enable)        ///< VHALF输出使能

 /*电流采样模式*/
 #define Shunt_Resistor_Mode            (Double_Resistor)


/*时间设置值-------------------------------------------------------------------*/
 #define Calib_Time                     (300)                                   // 校正次数，固定1000次，单位:次
 #define Charge_Time                    (10)                                  	// (ms) 预充电时间，单位：ms

 
 /***预定位的Kp、Ki****/
 #define DQKP_Alignment                 _Q12(0.5)                               // 预定位的KP
 #define DQKI_Alignment                 _Q15(0.004)                             // 预定位的KI
 #define Align_Time                     (5000)                                  // (ms) 预定位时间，单位：ms原来3000

 #define ID_Align_CURRENT               I_Value(2.0)                            // (A) D轴定位电流
 #define IQ_Align_CURRENT               I_Value(0.0)                            // (A) Q轴定位电流 

 #define Align_Angle_0                   (0.0)                                  // (°) 预定位角度
 #define Align_Angle_90                  (90.0)                                 // (°) 预定位角度

 /***开环电流****/
 
 #define ID_Open_CURRENT               I_Value(2.0)                             // (A) D轴开环电流
 #define IQ_Open_CURRENT               I_Value(0.0)                            	// (A) Q轴开环电流
 #define Open_Speed                    ACC_Value(30)	                          // (rpm)强拉速度


 /* motor run speed value */
 #define MOTOR_SPEED_POSICHECK_RPM      (100.0)                                 // (RPM) 限位校准转速  
 #define MOTOR_SPEED_MIN_RPM            (1000.0)                               	// (RPM) 运行最小转速  
 #define MOTOR_SPEED_MAX_RPM            (2000.0)                                // (RPM) 运行最大转速
 #define MOTOR_SPEED_STOP_RPM           (20.0)                                  // (RPM) 刹车停止速度
  
 /*电流环参数设置值--------------------------------------------------------------*/
 #define DQKP                           _Q12(2.0)                               //  DQ轴KP参数
 #define DQKI                           _Q15(0.03)                             //  DQ轴KI参数

 /* D轴参数设置 */
 #define DOUTMAX                        _Q15(0.60)                              // D轴最大限幅值，单位：输出占空比
 #define DOUTMIN                        _Q15(-0.60)                             // D轴最小限幅值，单位：输出占空比
 /* Q轴参数设置，默认0.99即可 */
 #define QOUTMAX                        _Q15(0.98)                              // Q轴最大限幅值，单位：输出占空比 
 #define QOUTMIN                        _Q15(-0.98)                             // Q轴最小限幅值，单位：输出占空比


/*外环参数设置值----------------------------------------------------------------*/
 //---------------------------------------速度环参数----------------------------------------*/

 #define SKP                            _Q12(2.2)                              // 外环KP   
 #define SKF                            _Q12(0.0)
 #define SKI                            _Q15(0.01)                            // 外环KI   

 #define SOUTMAX                        _Q15(0.96)                              // (A) 外环最大限幅值
 #define SOUTMIN                        _Q15(-0.96)                             // (A) 外环最小限幅值
 
 #define SOUTMAX_Cur                    I_Value(5.0)                            // (A) 外环最大限幅值
 #define SOUTMIN_Cur                    I_Value(-5.0)                           // (A) 外环最小限幅值

/*
  这两个值作用于 AddFunction.c 的 mc_ramp() 函数，每个 PWM 周期（约 41.7μs@ 24kHz）执行一次：
  - SPEED_INC = 100：每个周期加 100 LSB，24000 次/秒 →约 2.4M LSB/秒
  - 从 0 到 300 RPM（≈S_Value(300) ≈_Q15(300/4000) ≈2457 LSB），只需 2457/100 ≈25 个周期 ≈1ms
*/ 
 #define SPEED_INC                      (1)                                     // 速度环增量：加速步长（每个速度环周期增加的量）
 #define SPEED_DEC                      (1)                                     // 速度环减量：减速步长
 
/*  ---------------------------------------位置环路PD参数----------------------------------------
_Q12是一个定点数转换宏。因为很多单片机算浮点数开销较大，所以程序会把小数放大成整数来计算。Q12的意思可以简单理解为：把小数乘以 2^12
PKP_Sw 越大，位置响应越快，越容易抖动；PKP_Sw 越小，位置响应越慢，但是更平稳
	位置环一般会计算：位置误差 = 目标位置 - 实际位置
	然后位置环输出：位置环输出 = KP × 位置误差 + KD × 误差变化率
*/ 
 #define PKP_Sw                         _Q12(0.8)                              // 位置环的比例P参数，也就是KP
 #define PKD_Sw                         _Q12(0.1)                              // 位置环的微分D参数，也就是KD
 
 
/*
POUTMAX和POUTMIN是位置环的输出限幅，相当于控制位置环输出速度指令的最大值
S_Value(300) 内部数值表示转速rpm/min.S_Value函数的作用就是把实际速度单位 rpm 转换成程序内部速度单位
*/ 
 #define POUTMAX                        S_Value(10)                           // (RPM) 
 #define POUTMIN                        S_Value(-10)                          // (RPM) 
 
 
 /** 
 * 硬件PCBA参数设置根据驱动芯片的类型选择，6866为 HIGH_LEVEL 
 * @param (HIGH_LEVEL)   驱动高电平有效 
 * @param (LOW_LEVEL)    驱动高电平有效 
 * @param (UP_H_DOWN_L)  驱动高电平有效 
 * @param (UP_L_DOWN_H)  驱动高电平有效 
 */ 
#define PWM_LEVEL_MODE                  (HIGH_LEVEL)    ///< 驱动电平设置 

/* --驱动电平设置-- */
#define HIGH_LEVEL                      (0xA0)          ///< 驱动高电平有效
#define LOW_LEVEL                       (0xB0)          ///< 驱动低电平有效
#define UP_H_DOWN_L                     (0xC0)          ///< 上桥臂高电平有效，下桥臂低电平有效
#define UP_L_DOWN_H                     (0xD0)          ///< 上桥臂低电平有效，下桥臂高电平有效

#define ID_Start_CURRENT                I_Value(0.0)     ///< (A) D轴启动电流  
#define IQ_Start_CURRENT                I_Value(0.0)     ///< (A) Q轴启动电流  
 
#define ATT_COEF                        (0.85)            // 
#define ATO_BW                          (500.0)           // 观测器带宽的滤波值，经典值为1.0-200.0
  
#endif

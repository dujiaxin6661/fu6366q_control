/**
 * @copyright (C) COPYRIGHT 2019, Fortior Technology Co., Ltd.
 * @file      FU68xx_6.h
 * @author    
 * @note      Last modify author is Bruce HW&RD
 * @since     2022-09-07
 * @date      2022-09-07
 * @brief     Created file
 */

#ifndef __FU68XX_6_MCU_H__
#define __FU68XX_6_MCU_H__

/*******************************************************************************///Including Header Files
#include "FU68xx_6_Type.h"
/*******************************************************************************///Define Macro
#define FREC                            (24000000)                              // MCU工作频率
/*******************************************************************************///Function Subject
#define SetReg(reg, val1, val2)         ((reg) = (reg) & (~(val1)) | (val2))    // 将reg中val1对应的位写val2
#define SetBit(reg, val)                ((reg) |=  (val))                       // 将reg中val对应的位写1
#define ClrBit(reg, val)                ((reg) &= ~(val))                       // 将reg中val对应的位写0
#define XorBit(reg, val)                ((reg) ^=  (val))                       // 将reg中val对应的位取反
#define ReadBit(reg, val)               (((reg) & (val)) != 0)                  // 判断reg中val对应的位是否为1
/*******************************************************************************///Register Map
// 搜索快照寄存器请在Keil搜索"[0-9A-Z]__[A-Z]"，勾选"Match Case"和"Regular Expressior"
// 快速搜索外设方法:在搜索对话框中输入"//"+外设名称.例:搜索TIMER1,则输入"//TIM1"

/******************************************************************************///SFR
 sfr16   DPTR                           = 0x82;                                 // RV:0000H  8051指针
 sfr     PSW                            = 0xd0;                                 // RV:  00H  8051状态寄存器
 sbit    CY                                       = PSW^7;                      //               rw-- 进/借位标志
 sbit    AC                                       = PSW^6;                      //               rw-- BCD进/借位标志
 sbit    F0                                       = PSW^5;                      //               rw-- 用户自定义标志0
 sbit    RS1                                      = PSW^4;                      //               rw-- R寄存器换页位1
 sbit    RS0                                      = PSW^3;                      //               rw-- R寄存器换页位0
 sbit    OV                                       = PSW^2;                      //               rw-- 算术运算溢出标志
 sbit    F1                                       = PSW^1;                      //               rw-- 用户自定义标志1
 sbit    P                                        = PSW^0;                      //               rw-- 奇偶标志

 sfr     ACC                            = 0xe0;                                 // RV:  00H  8051 A寄存器
/******************************************************************************///RST
 sfr     RST_SR                         = 0xc9;                                 // RV:  xxH  复位状态寄存器
 #define RSTPOW                                   0x80                          //               r--- 上电复位标志
 #define RSTCLR                                   0x80                          //               w1-- 复位标志清零
 #define RSTEXT                                   0x40                          //               r--- 外部复位标志
 #define RSTLVD                                   0x20                          //               r--- 低压复位标志
 #define RSTEOS                                   0x10                          //               r--- 电应力过度复位标志
 #define RSTWDT                                   0x08                          //               r--- 看门狗复位标志
 #define RSTFED                                   0x04                          //               r--- 代码保护复位
 #define RSTDBG                                   0x02                          //               r--- 调试接口复位
 #define SOFTR                                    0x01                          //               rw1- 清除复位标志
/******************************************************************************///RTC
 #define RTC_TM                         *(_IO uint16 xdata *)0x402c             // RV:FFFFH  RTC计数寄存器
 #define RTC_STA                        *(_IO  uint8 xdata *)0x402e             // RV:  00H  RTC控制寄存器
 #define RTC_EN                                   0x80                          //               rw-- RTC使能
 #define RTC_IF                                   0x40                          //               rw-- RTC中断标志
 #define SCKSEL                                   0x20                          //               rw-- RTC慢时钟来源选择
 #define ISOSCEN                                  0x10                          //               rw-- 内部慢时钟使能
 #define ESOAE                                    0x08                          //               rw-- 外部慢时钟晶体PIN模拟输入使能
 #define ESOEN                                    0x04                          //               rw-- 外部慢时钟使能
 #define ESCKSEL                                  0x02                          //               rw-- 时钟校准输入源选择P11使能
 #define ESCKEN                                   0x01                          //               rw-- 外部慢时钟数字输入使能
/******************************************************************************///OSC
 sfr     OSC_CFG                        = 0xf1;                                 // RV:  00H  外部快时钟相关寄存器
 #define EFCKEN                                   0x80                          //               rw-- 外部快时钟强制使能
 #define IFCKEN                                   0x40                          //               rw-- 内部快时钟强制使能
 #define CK_FLAG                                  0x20                          //               r--- 系统时钟标志
 #define MCD_RET                                  0x20                          //               w1-- 尝试退出MCD状态
/******************************************************************************///CLOCK_CAL
 #define CAL_CR0                        *(_IO  uint8 xdata *)0x4044             // RV:  00H  时钟校准寄存器
 #define CAL_STA                                  0x80                          //               w-- CLOCK_CAL使能, r-- 0-->Done 1-->ING
 #define CAL_CR1                        *(_IO  uint8 xdata *)0x4045             // RV:  00H  时钟校准寄存器
/******************************************************************************///IRQ
 // IRQ Channel
 // LVW/TSD/MCD     interrupt 0
 // INT0            interrupt 1
 // INT1            interrupt 2
 // DRV             interrupt 3
 // TIM2            interrupt 4
 // TIM1            interrupt 5
 // ADC             interrupt 6
 // CMP/HALL        interrupt 7
 // RTC/CAN         interrupt 8
 // TIM3            interrupt 9
 // SysTick         interrupt 10
 // TIM4            interrupt 11
 // CMP3            interrupt 12
 // I2C/UART1       interrupt 13
 // SPI/UART2/LIN   interrupt 14
 // DMA             interrupt 15

 sfr     TCON                           = 0x88;                                 // RV:  00H  8051中断控制寄存器
 sbit    MCDIF                                    = TCON^6;                     //               rw-- MCD时钟缺失中断标志
 sbit    TSDIF                                    = TCON^5;                     //               rw-- TSD温度感应中断标志
 sbit    IT11                                     = TCON^4;                     //               rw-- 外部中断1触发配置1
 sbit    IT10                                     = TCON^3;                     //               rw-- 外部中断1触发配置0
 sbit    IF0                                      = TCON^2;                     //               rw-- 外部中断0标志
 sbit    IT01                                     = TCON^1;                     //               rw-- 外部中断0触发配置1
 sbit    IT00                                     = TCON^0;                     //               rw-- 外部中断0触发配置0

 sfr     IE                             = 0xa8;                                 // RV:  00H  8051 中断开关
 sbit    EA                                       = IE^7;                       //               rw-- MCU总中断使能
 sbit    RTCIE                                    = IE^6;                       //               rw-- RTC中断使能
 sbit    MCDIE                                    = IE^5;                       //               rw-- MCD时钟缺失中断使能
 sbit    ES0                                      = IE^4;                       //               rw-- UART中断使能
 sbit    SPIIE                                    = IE^3;                       //               rw-- SPI中断使能
 sbit    EX1                                      = IE^2;                       //               rw-- 外部中断1使能
 sbit    TSDIE                                    = IE^1;                       //               rw-- TSD中断使能
 sbit    EX0                                      = IE^0;                       //               rw-- 外部中断0使能

 sfr     IP0                            = 0x8a;                                 // RV:  00H  外设中断优先级设置G0
 #define PDRV1                                    0x80                          //               rw-- DRV优先级配置1
 #define PDRV0                                    0x40                          //               rw-- DRV优先级配置0
 #define PX11                                     0x20                          //               rw-- 外部中断1优先级配置1
 #define PX10                                     0x10                          //               rw-- 外部中断1优先级配置0
 #define PX01                                     0x08                          //               rw-- 外部中断0优先级配置1
 #define PX00                                     0x04                          //               rw-- 外部中断0优先级配置0
 #define PLVW1                                    0x02                          //               rw-- LVW/TSD优先级配置1
 #define PLVW0                                    0x01                          //               rw-- LVW/TSD优先级配置0

 sfr     IP1                            = 0x8b;                                 // RV:  00H  外设中断优先级设置G1
 #define PCMP1                                    0x80                          //               rw-- CMP优先级配置1
 #define PCMP0                                    0x40                          //               rw-- CMP优先级配置0
 #define PADC1                                    0x20                          //               rw-- ADC优先级配置1
 #define PADC0                                    0x10                          //               rw-- ADC优先级配置0
 #define PTIM11                                   0x08                          //               rw-- TIM1优先级配置1
 #define PTIM10                                   0x04                          //               rw-- TIM1优先级配置0
 #define PTIM21                                   0x02                          //               rw-- TIM2优先级配置1
 #define PTIM20                                   0x01                          //               rw-- TIM2优先级配置0

 sfr     IP2                            = 0x8c;                                 // RV:  00H  外设中断优先级设置G2
 #define PTIM41                                   0x80                          //               rw-- TIM4优先级配置1
 #define PTIM40                                   0x40                          //               rw-- TIM4优先级配置0
 #define PSYSTICK1                                0x20                          //               rw-- SysTick优先级配置1
 #define PSYSTICK0                                0x10                          //               rw-- SysTick优先级配置0
 #define PTIM31                                   0x08                          //               rw-- TIM3优先级配置1
 #define PTIM30                                   0x04                          //               rw-- TIM3优先级配置0
 #define PRTC1                                    0x02                          //               rw-- RTC优先级配置1
 #define PRTC0                                    0x01                          //               rw-- RTC优先级配置0

 sfr     IP3                            = 0x8d;                                 // RV:  00H  外设中断优先级设置G3
 #define PDMA1                                    0x80                          //               rw-- DMA优先级配置1
 #define PDMA0                                    0x40                          //               rw-- DMA优先级配置0
 #define PSPI_UT21                                0x20                          //               rw-- SPI/UT2优先级配置1
 #define PSPI_UT20                                0x10                          //               rw-- SPI/UT2优先级配置0
 #define PI2C_UT11                                0x08                          //               rw-- I2C/UT1优先级配置1
 #define PI2C_UT10                                0x04                          //               rw-- I2C/UT1优先级配置0
 #define PCMP31                                   0x02                          //               rw-- CMP3优先级配置1
 #define PCMP30                                   0x01                          //               rw-- CMP3优先级配置0
/******************************************************************************///I2C
 #define I2C_CR                         *(_IO  uint8 xdata *)0x4028             // RV:  00H  I2C控制寄存器
 #define I2CEN                                    0x80                          //               rw-- I2C使能
 #define I2CMS                                    0x40                          //               rw-- 主/从机模式选择
 #define I2CDMANAKINT                             0x10                          //               rw-- 使能后当DMA传输时忽略NAK应答引起的传输中断
 #define I2CDMAAUTO                               0x08                          //               rw-- 使能后当DMA传输时不用手动发送第一个数据字节
 #define I2CSPD1                                  0x04                          //               rw-- 速度配置1
 #define I2CSPD0                                  0x02                          //               rw-- 速度配置0
 #define I2CIE                                    0x01                          //               rw-- 中断控制

 #define I2C_SR                         *(_IO  uint8 xdata *)0x402b             // RV:  00H  I2C状态寄存器
 #define I2CBSY                                   0x80                          //               r--- I2C忙标志
 #define DMOD                                     0x40                          //               rw-- I2C读写控制
 #define I2CSTA                                   0x10                          //               rw-- START信号控制
 #define I2CSTP                                   0x08                          //               rw-- STOP信号控制
 #define STR                                      0x04                          //               rw-- I2C事件完成标志
 #define NACK                                     0x02                          //               rw-- 应答信号控制
 #define I2CIF                                    0x01                          //               r--- 中断标志

 #define I2C_ID                         *(_IO  uint8 xdata *)0x4029             // RV:  AAH  本机2C地址
 #define GC                                       0x01                          //               广播模式

 #define I2C_DR                         *(_IO  uint8 xdata *)0x402a             // RV:  00H  I2C数据寄存器
/******************************************************************************///SPI
 #define SPI_CR0                        *(_IO  uint8 xdata *)0x4030             // RV:  0bH  SPI控制寄存器1
 #define SPIBSY                                   0x80                          //               r--- 忙标志
 #define SPIMS                                    0x40                          //               rw-- 主/从机配置
 #define CPHA                                     0x20                          //               rw-- 时钟相位
 #define CPOL                                     0x10                          //               rw-- 时钟空闲电平
 #define SLVSEL                                   0x08                          //               r--- NSS处理后的信号
 #define NSSIN                                    0x04                          //               r--- NSS即时信号
 #define SRMT                                     0x02                          //               r--- 移位寄存器空标志
 #define RXBMT                                    0x01                          //               r--- 接收缓冲器空标志

 #define SPI_CR1                        *(_IO  uint8 xdata *)0x4031             // RV:  02H  SPI控制寄存器2
 #define SPIIF                                    0x80                          //               rw0- 中断标志
 #define WCOL                                     0x40                          //               rw0- 写冲突标志
 #define MODF                                     0x20                          //               rw0- 模式错误标志
 #define RXOVRN                                   0x10                          //               rw0- 接收溢出标志
 #define NSSMOD1                                  0x08                          //               rw-- SPI模式配置位1
 #define NSSMOD0                                  0x04                          //               rw-- SPI模式配置位0
 #define TXBMT                                    0x02                          //               r--- 发送缓冲器空标志
 #define SPIEN                                    0x01                          //               rw-- SPI使能

 #define SPI_CLK                        *(_IO  uint8 xdata *)0x4032             // RV:  00H  SPI时钟配置寄存器
 #define SPI_DR                         *(_IO  uint8 xdata *)0x4033             // RV:  00H  SPI数据寄存器
/******************************************************************************///UART
 sfr     UT_CR                          = 0x98;                                 // RV:  00H  UART控制寄存器
 sbit    UT_MOD1                                  = UT_CR^7;                    //               rw-- UART模式配置1
 sbit    UT_MOD0                                  = UT_CR^6;                    //               rw-- UART模式配置0
 sbit    SM2                                      = UT_CR^5;                    //               rw-- 多机通信使能
 sbit    REN                                      = UT_CR^4;                    //               rw-- 接收使能
 sbit    TB8                                      = UT_CR^3;                    //               rw-- 9位模式下发送的第9位
 sbit    RB8                                      = UT_CR^2;                    //               rw-- 9位模式下接收的第9位
 sbit    TI                                       = UT_CR^1;                    //               rw-- 发送完成标志
 sbit    RI                                       = UT_CR^0;                    //               rw-- 接收完成标志

 sfr     UT_DR                          = 0x99;                                 // RV:  00H  UART数据寄存器

 sfr16   UT_BAUD                        = 0x9a;                                 // RV:009bH  UART波特率控制寄存器
 #define UART_2xBAUD                              0x8000                        //               rw--倍频使能
 #define UART_RX_INV                              0x4000                        //               rw-- RxD反相使能
 #define UART_TX_INV                              0x2000                        //               rw-- TxD反相使能
 #define UART_CH                                  0x1000                        //               rw-- 串口端口转移(P05-->P10 P06-->P11)

 sfr     UT2_CR                         = 0xd8;                                 // RV:  00H  UART2控制寄存器
 sbit    UT2MOD1                                  = UT2_CR^7;                   //               rw-- UART2模式配置1
 sbit    UT2MOD0                                  = UT2_CR^6;                   //               rw-- UART2模式配置0
 sbit    UT2SM2                                   = UT2_CR^5;                   //               rw-- 多机通信使能
 sbit    UT2REN                                   = UT2_CR^4;                   //               rw-- 接收使能
 sbit    UT2TB8                                   = UT2_CR^3;                   //               rw-- 9位模式下发送的第9位
 sbit    UT2RB8                                   = UT2_CR^2;                   //               rw-- 9位模式下接收的第9位
 sbit    UT2TI                                    = UT2_CR^1;                   //               rw-- 发送完成标志
 sbit    UT2RI                                    = UT2_CR^0;                   //               rw-- 接收完成标志

 #define UT2_BAUD                       *(_IO uint16 xdata *)0x4042             // RV:009bH  UART2波特率控制寄存器
 #define BAUD2_SEL                                0x8000                        //               rw--倍频使能
 #define UART2_RX_INV                             0x4000                        //               rw-- RxD反相使能
 #define UART2_TX_INV                             0x2000                        //               rw-- TxD反相使能
 #define UART2IEN                                 0x1000                        //               rw--UART2中断使能
 sfr     UT2_DR                         = 0x89;                                 // RV:  00H  UART2数据寄存器
/******************************************************************************///LIN
 sfr     LIN_CR                         = 0xb9;                                 // RV:0000H  LIN控制寄存器
 #define DMASEL                                   0x40                          //               rw-- DMA选择
 #define CHSEL1                                   0x20                          //               rw-- 通道选择1
 #define CHSEL0                                   0x10                          //               rw-- 通道选择0
 #define LINIE                                    0x08                          //               rw-- LIN中断使能
 #define CHKMOD                                   0x04                          //               rw-- 校验模式配置
 #define LINRW                                    0x02                          //               rw-- 读写模式配置
 #define AUTOSIZE                                 0x01                          //               rw-- 根据ID自动设置数据长度使能

 sfr LIN_SR                             = 0xba;                                 // RV:0000H  LIN状态寄存器
 #define ERRSYNC                                  0x80                          //               r--- 同步错误, 同步超时或同步过快
 #define ERRCHK                                   0x40                          //               r--- 数据校验错误
 #define ERRPRTY                                  0x20                          //               r--- ID校验错误
 #define ABORT                                    0x10                          //               r--- 传输被中断, 如果传输时收到帧头或者LIN_CR[LIN_STOP]置1, 那么该位置1
 #define LINACT                                   0x08                          //               r--- LIN总线上处于活动状态
 #define LINIDLE                                  0x04                          //               rw0- LIN总线上处于空闲状态并持续4s
 #define LINDONE                                  0x02                          //               rw-- LIN传输完成
 #define LINREQ                                   0x01                          //               rw-- LIN收到帧头

 sfr     LIN_CSR                        = 0xbb;                                 // RV:0000H  LIN状态控制寄存器
 #define ERRBIT                                   0x80                          //               r--- 位冲突错误, 发送的信号与总线上的信号不一致
 #define RDBAKDIS                                 0x40                          //               rw-- 位冲突检查禁止
 #define LINSLP                                   0x20                          //               rw-- LIN的睡眠状态
 #define CLRERR                                   0x10                          //               rw0- 清除错误标志
 #define LINWAKEUP                                0x08                          //               rw1- 读: LIN是否被唤醒; 写: 发送唤醒信号
 #define LINACK                                   0x04                          //               w1-- 用于响应帧头, 在该位置1前必须将校验模式, 读写模式, 数据, 数据长度准备好
 #define LINSTOP                                  0x02                          //               w1-- 中断当前的传输
 #define LINEN                                    0x01                          //               rw-- LIN使能

 sfr     LIN_ID                         = 0xbc;                                 // RV:0000H  LIN收到的ID的寄存器
 sfr     LIN_SIZE                       = 0xbd;                                 // RV:0000H  LIN数据长度寄存器
 sfr16   LIN_BAUD                       = 0xbe;                                 // RV:04afH  LIN波特率寄存器
/******************************************************************************///CAN
 sfr     CAN_CR0                        = 0xb1;                                 // RV:  01H  CAN控制寄存器0
 #define CANEN                                    0x80                          //               rw-- CAN使能
 #define CAN_CH                                   0x40                          //               rw-- CAN功能转移(P01-->P06 P00-->P05)
 #define CANREV                                   0x20                          //               rw-- CAN版本
 #define WAKMOD                                   0x10                          //               rw-- 唤醒模式
 #define FILMOD                                   0x08                          //               rw-- 过滤模式
 #define SELFTST                                  0x04                          //               rw-- 自测模式
 #define LISTEN                                   0x02                          //               rw-- 监听模式
 #define RSTMOD                                   0x01                          //               rw-- 复位模式

 sfr     CAN_CR1                        = 0xb2;                                 // RV:  00H  CAN控制寄存器1
 #define SLPREQ                                   0x20                          //               rw-- 睡眠请求
 #define TXSELF                                   0x10                          //               rw-- 自发自收
 #define BUFRLS                                   0x04                          //               rw-- 释放接收缓存
 #define NOART                                    0x02                          //               rw-- 禁止自动重传
 #define TXREQ                                    0x01                          //               rw-- 发送请求

 sfr     CAN_SR                         = 0xb3;                                 // RV:  0CH CAN状态寄存器
 #define SLPACK                                   0x40                          //               r--- 睡眠确认标志
 #define TXING                                    0x20                          //               r--- TXD正处于发送状态
 #define RXING                                    0x10                          //               r--- RXD正处于接收状态
 #define TXSUC                                    0x08                          //               r--- 发送成功
 #define TXDONE                                   0x04                          //               r--- 发送结束
 #define BUFOV                                    0x02                          //               r--- FIFO溢出
 #define NOEMP                                    0x01                          //               r--- FIFO非空

 sfr     CAN_IER                        = 0xb4;                                 // RV:  00H CAN中断使能寄存器
 #define ETYIE                                    0x80                          //               rw-- 错误类型中断使能
 #define BOFIE                                    0x40                          //               rw-- 总线关闭中断使能
 #define PERIE                                    0x20                          //               rw-- 被动错误中断使能
 #define ERWIE                                    0x10                          //               rw-- 错误预警中断使能
 #define OVIE                                     0x08                          //               rw-- 溢出中断使能
 #define ARBIE                                    0x04                          //               rw-- 仲裁中断使能
 #define TXIE                                     0x02                          //               rw-- 发送中断使能
 #define RXIE                                     0x01                          //               rw-- 接收中断使能

 sfr     CAN_CR2                        = 0xb5;                                 // RV:  00H CAN控制寄存器2
 #define RDPTR1                                   0x80                          //               r--- FIFO读指针位1
 #define RDPTR0                                   0x40                          //               r--- FIFO读指针位0
 #define MESCNT1                                  0x20                          //               r--- 有效报文数位1
 #define MESCNT0                                  0x10                          //               r--- 有效报文数位0
 #define WKUIE                                    0x04                          //               rw-- 唤醒中断使能
 #define SLPIE                                    0x02                          //               rw-- 睡眠中断使能
 #define ERRIE                                    0x01                          //               rw-- 错误总中断使能

 sfr     CAN_IFR                        = 0xb6;                                 // RV:  00H CAN中断标志寄存器
 #define WKUIF                                    0x40                          //               rw-- 唤醒中断标志
 #define SLPIF                                    0x20                          //               rw-- 睡眠中断标志
 #define ERRIF                                    0x10                          //               rw-- 错误中断标志
 #define OVIF                                     0x08                          //               rw-- 溢出中断标志
 #define ARBIF                                    0x04                          //               rw-- 仲裁中断标志
 #define TXIF                                     0x02                          //               rw-- 发送中断标志
 #define RXIF                                     0x01                          //               rw-- 接收中断标志

 sfr     CAN_BTR0                       = 0xb7;                                 // RV:  00H CAN位时间寄存器0
 #define SJW1                                     0x08                          //               rw-- 重新同步跳跃宽度配置1
 #define SJW0                                     0x04                          //               rw-- 重新同步跳跃宽度配置0
 sfr     CAN_BTR1                       = 0xc2;                                 // RV:  00H CAN位时间寄存器1
 sfr     CAN_BTR2                       = 0xc3;                                 // RV:  00H CAN位时间寄存器2
 #define TTCM                                     0x80                          //               rw-- 时间触发采样模式

 sfr     CAN_FIR0                       = 0xc4;                                 // RV:  00H CAN滤波标识符寄存器0
 sfr     CAN_FIR1                       = 0xc5;                                 // RV:  00H CAN滤波标识符寄存器1
 sfr     CAN_FIR2                       = 0xc6;                                 // RV:  00H CAN滤波标识符寄存器2
 sfr     CAN_FIR3                       = 0xc7;                                 // RV:  00H CAN滤波标识符寄存器3
 sfr     CAN_FMR0                       = 0xca;                                 // RV:  00H CAN滤波屏蔽位寄存器0
 sfr     CAN_FMR1                       = 0xcb;                                 // RV:  00H CAN滤波屏蔽位寄存器1
 sfr     CAN_FMR2                       = 0xcc;                                 // RV:  00H CAN滤波屏蔽位寄存器2
 sfr     CAN_FMR3                       = 0xcd;                                 // RV:  00H CAN滤波屏蔽位寄存器3

 sfr     CAN_ALC                        = 0xce;                                 // RV:  00H CAN仲裁丢失位置寄存器
 sfr     CAN_ESR                        = 0xcf;                                 // RV:  00H CAN错误寄存器
 #define ETY2                                     0x80                          //               rw-- 错误类型位2
 #define ETY1                                     0x40                          //               rw-- 错误类型位1
 #define ETY0                                     0x20                          //               rw-- 错误类型位0
 #define BOFF                                     0x04                          //               r--- 总线关闭标志
 #define PERF                                     0x02                          //               rw-- 被动错误标志
 #define ERWF                                     0x01                          //               r--- 错误预警标志
 sfr     CAN_LIM                        = 0xdd;                                 // RV:  60H CAN错误预警门限寄存器
 sfr     CAN_TEC                        = 0xde;                                 // RV:  00H CAN发送错误计数寄存器
 sfr     CAN_REC                        = 0xdf;                                 // RV:  00H CAN接收错误计数寄存器

 #define CAN_TXCR                       *(_IO  uint8 xdata *)0x0ed7             // RV:  00H CAN发送控制寄存器
 #define IDE                                      0x80
 #define RTR                                      0x40

 #define CAN_TXID0                      *(_IO  uint8 xdata *)0x0ed6             // RV:  XXH CAN发送ID寄存器0
 #define CAN_TXID1                      *(_IO  uint8 xdata *)0x0ed5             // RV:  XXH CAN发送ID寄存器1
 #define CAN_TXID2                      *(_IO  uint8 xdata *)0x0ed4             // RV:  XXH CAN发送ID寄存器2
 #define CAN_TXID3                      *(_IO  uint8 xdata *)0x0ed3             // RV:  XXH CAN发送ID寄存器3
 #define CAN_TXDR0                      *(_IO  uint8 xdata *)0x0ed2             // RV:  XXH CAN发送数据寄存器0
 #define CAN_TXDR1                      *(_IO  uint8 xdata *)0x0ed1             // RV:  XXH CAN发送数据寄存器1
 #define CAN_TXDR2                      *(_IO  uint8 xdata *)0x0ed0             // RV:  XXH CAN发送数据寄存器2
 #define CAN_TXDR3                      *(_IO  uint8 xdata *)0x0ecf             // RV:  XXH CAN发送数据寄存器3
 #define CAN_TXDR4                      *(_IO  uint8 xdata *)0x0ece             // RV:  XXH CAN发送数据寄存器4
 #define CAN_TXDR5                      *(_IO  uint8 xdata *)0x0ecd             // RV:  XXH CAN发送数据寄存器5
 #define CAN_TXDR6                      *(_IO  uint8 xdata *)0x0ecc             // RV:  XXH CAN发送数据寄存器6
 #define CAN_TXDR7                      *(_IO  uint8 xdata *)0x0ecb             // RV:  XXH CAN发送数据寄存器7

 #define CAN_RX0CR                      *(_IO  uint8 xdata *)0x0eca             // RV:  XXH CAN接收缓冲区0控制寄存器
 #define CAN_RX0ID0                     *(_IO  uint8 xdata *)0x0ec9             // RV:  XXH CAN接收缓冲区0ID寄存器0
 #define CAN_RX0ID1                     *(_IO  uint8 xdata *)0x0ec8             // RV:  XXH CAN接收缓冲区0ID寄存器1
 #define CAN_RX0ID2                     *(_IO  uint8 xdata *)0x0ec7             // RV:  XXH CAN接收缓冲区0ID寄存器2
 #define CAN_RX0ID3                     *(_IO  uint8 xdata *)0x0ec6             // RV:  XXH CAN接收缓冲区0ID寄存器3
 #define CAN_RX0DR0                     *(_IO  uint8 xdata *)0x0ec5             // RV:  XXH CAN接收缓冲区0数据寄存器0
 #define CAN_RX0DR1                     *(_IO  uint8 xdata *)0x0ec4             // RV:  XXH CAN接收缓冲区0数据寄存器1
 #define CAN_RX0DR2                     *(_IO  uint8 xdata *)0x0ec3             // RV:  XXH CAN接收缓冲区0数据寄存器2
 #define CAN_RX0DR3                     *(_IO  uint8 xdata *)0x0ec2             // RV:  XXH CAN接收缓冲区0数据寄存器3
 #define CAN_RX0DR4                     *(_IO  uint8 xdata *)0x0ec1             // RV:  XXH CAN接收缓冲区0数据寄存器4
 #define CAN_RX0DR5                     *(_IO  uint8 xdata *)0x0ec0             // RV:  XXH CAN接收缓冲区0数据寄存器5
 #define CAN_RX0DR6                     *(_IO  uint8 xdata *)0x0ebf             // RV:  XXH CAN接收缓冲区0数据寄存器6
 #define CAN_RX0DR7                     *(_IO  uint8 xdata *)0x0ebe             // RV:  XXH CAN接收缓冲区0数据寄存器7

 #define CAN_RX1CR                      *(_IO  uint8 xdata *)0x0ebd             // RV:  XXH CAN接收缓冲区1控制寄存器
 #define CAN_RX1ID0                     *(_IO  uint8 xdata *)0x0ebc             // RV:  XXH CAN接收缓冲区1ID寄存器0
 #define CAN_RX1ID1                     *(_IO  uint8 xdata *)0x0ebb             // RV:  XXH CAN接收缓冲区1ID寄存器1
 #define CAN_RX1ID2                     *(_IO  uint8 xdata *)0x0eba             // RV:  XXH CAN接收缓冲区1ID寄存器2
 #define CAN_RX1ID3                     *(_IO  uint8 xdata *)0x0eb9             // RV:  XXH CAN接收缓冲区1ID寄存器3
 #define CAN_RX1DR0                     *(_IO  uint8 xdata *)0x0eb8             // RV:  XXH CAN接收缓冲区1数据寄存器0
 #define CAN_RX1DR1                     *(_IO  uint8 xdata *)0x0eb7             // RV:  XXH CAN接收缓冲区1数据寄存器1
 #define CAN_RX1DR2                     *(_IO  uint8 xdata *)0x0eb6             // RV:  XXH CAN接收缓冲区1数据寄存器2
 #define CAN_RX1DR3                     *(_IO  uint8 xdata *)0x0eb5             // RV:  XXH CAN接收缓冲区1数据寄存器3
 #define CAN_RX1DR4                     *(_IO  uint8 xdata *)0x0eb4             // RV:  XXH CAN接收缓冲区1数据寄存器4
 #define CAN_RX1DR5                     *(_IO  uint8 xdata *)0x0eb3             // RV:  XXH CAN接收缓冲区1数据寄存器5
 #define CAN_RX1DR6                     *(_IO  uint8 xdata *)0x0eb2             // RV:  XXH CAN接收缓冲区1数据寄存器6
 #define CAN_RX1DR7                     *(_IO  uint8 xdata *)0x0eb1             // RV:  XXH CAN接收缓冲区1数据寄存器7

 #define CAN_RX2CR                      *(_IO  uint8 xdata *)0x0eb0             // RV:  XXH CAN接收缓冲区2控制寄存器
 #define CAN_RX2ID0                     *(_IO  uint8 xdata *)0x0eaf             // RV:  XXH CAN接收缓冲区2ID寄存器0
 #define CAN_RX2ID1                     *(_IO  uint8 xdata *)0x0eae             // RV:  XXH CAN接收缓冲区2ID寄存器1
 #define CAN_RX2ID2                     *(_IO  uint8 xdata *)0x0ead             // RV:  XXH CAN接收缓冲区2ID寄存器2
 #define CAN_RX2ID3                     *(_IO  uint8 xdata *)0x0eac             // RV:  XXH CAN接收缓冲区2ID寄存器3
 #define CAN_RX2DR0                     *(_IO  uint8 xdata *)0x0eab             // RV:  XXH CAN接收缓冲区2数据寄存器0
 #define CAN_RX2DR1                     *(_IO  uint8 xdata *)0x0eaa             // RV:  XXH CAN接收缓冲区2数据寄存器1
 #define CAN_RX2DR2                     *(_IO  uint8 xdata *)0x0ea9             // RV:  XXH CAN接收缓冲区2数据寄存器2
 #define CAN_RX2DR3                     *(_IO  uint8 xdata *)0x0ea8             // RV:  XXH CAN接收缓冲区2数据寄存器3
 #define CAN_RX2DR4                     *(_IO  uint8 xdata *)0x0ea7             // RV:  XXH CAN接收缓冲区2数据寄存器4
 #define CAN_RX2DR5                     *(_IO  uint8 xdata *)0x0ea6             // RV:  XXH CAN接收缓冲区2数据寄存器5
 #define CAN_RX2DR6                     *(_IO  uint8 xdata *)0x0ea5             // RV:  XXH CAN接收缓冲区2数据寄存器6
 #define CAN_RX2DR7                     *(_IO  uint8 xdata *)0x0ea4             // RV:  XXH CAN接收缓冲区2数据寄存器7
/******************************************************************************///MDU
 sfr     MDU_CR                         = 0xc1;                                 // RV:  00H  MDU控制寄存器
 #define MDUBSY                                   0x80                          //               r--- 当使能SMDU时, 用于表示MDU繁忙
 #define MDUSTA3                                  0x40                          //               rw-- 当使能SMDU时, 用于表示启动计算单元3
 #define MDUSTA2                                  0x20                          //               rw-- 当使能SMDU时, 用于表示启动计算单元2
 #define MDUSTA1                                  0x10                          //               rw-- 当使能SMDU时, 用于表示启动计算单元1
 #define MDUSTA0                                  0x08                          //               rw-- 当使能SMDU时, 用于表示启动计算单元0
 #define MDUMOD2                                  0x04                          //               rw-- 当使能SMDU时, 用于表示MDU运行模式配置2
 #define MDUMOD1                                  0x02                          //               rw-- 当使能SMDU时, 用于表示MDU运行模式配置1
 #define MDUMOD0                                  0x01                          //               rw-- 当使能SMDU时, 用于表示MDU运行模式配置0

 sfr     PI_CR                          = 0xf9;                                 // RV:  00H  PI控制寄存器
 #define T2TSS                                    0x80                          //               rw-- TIM2步进模式输入使能&PRD使能
 #define DRV_MD                                   0x04                          //               rw-- 0:中央对齐 1:锯齿波
 #define HINV                                     0x02                          //               rw-- 上管反相使能
 #define LINV                                     0x01                          //               rw-- 下管反相使能

/******************************************************************************///SMDU
 #define MUL_LEN                        8                                       // 乘法单元所占字节数
 #define DIV_LEN                        12                                      // 除法单元所占字节数
 #define LPF_LEN                        8                                       // 低通滤波单元所占字节数
 #define SCAT_LEN                       10                                      // Sin/Cos/ATan单元所占字节数
 #define PI_LEN                         16                                      // PI单元所占字节数
 #define PID_LEN                        20                                      // PID单元所占字节数

 #define SCAT3_BADR                     0x0ef8                                  // SMDU基地址/Sin/Cos/ATan单元3首地址
 #define SCAT2_BADR                     (SCAT3_BADR + SCAT_LEN)                 // Sin/Cos/ATan单元2首地址
 #define SCAT1_BADR                     (SCAT2_BADR + SCAT_LEN)                 // Sin/Cos/ATan单元1首地址
 #define SCAT0_BADR                     (SCAT1_BADR + SCAT_LEN)                 // Sin/Cos/ATan单元0首地址
 #define DIV3_BADR                      (SCAT0_BADR + SCAT_LEN)                 // 除法单元3首地址
 #define DIV2_BADR                      (DIV3_BADR  + DIV_LEN)                  // 除法单元2首地址
 #define MUL3_BADR                      (DIV2_BADR  + DIV_LEN)                  // 乘法单元3首地址
 #define MUL2_BADR                      (MUL3_BADR  + MUL_LEN)                  // 乘法单元2首地址
 #define PI3_BADR                       (MUL2_BADR  + MUL_LEN)                  // PI单元3首地址
 #define PI2_BADR                       (PI3_BADR   + PID_LEN)                  // PI单元2首地址
 #define LPF3_BADR                      (PI2_BADR   + PID_LEN)                  // 低通滤波单元3首地址
 #define LPF2_BADR                      (LPF3_BADR  + LPF_LEN)                  // 低通滤波单元2首地址
 #define DIV1_BADR                      (LPF2_BADR  + LPF_LEN)                  // 除法单元 1首地址
 #define DIV0_BADR                      (DIV1_BADR  + DIV_LEN)                  // 除法单元0首地址
 #define MUL1_BADR                      (DIV0_BADR  + DIV_LEN)                  // 乘法单元1首地址
 #define MUL0_BADR                      (MUL1_BADR  + MUL_LEN)                  // 乘法单元0首地址
 #define PI1_BADR                       (MUL0_BADR  + MUL_LEN)                  // PI单元1首地址
 #define PI0_BADR                       (PI1_BADR   + PI_LEN)                   // PI单元0首地址
 #define LPF1_BADR                      (PI0_BADR   + PI_LEN)                   // 低通滤波单元1首地址
 #define LPF0_BADR                      (LPF1_BADR  + LPF_LEN)                  // 低通滤波单元0首地址

 #define SCAT3_COS                      *(_IO uint16 xdata *)(SCAT3_BADR + 0x00)// rw-- Cos
 #define SCAT3_SIN                      *(_IO uint16 xdata *)(SCAT3_BADR + 0x02)// rw-- Sin
 #define SCAT3_THE                      *(_IO uint16 xdata *)(SCAT3_BADR + 0x04)// rw-- Angle
 #define SCAT3_RES1                     *(_IO uint16 xdata *)(SCAT3_BADR + 0x06)// rw-- Sin/Cos模式下为Cos; ATan模式下为幅值
 #define SCAT3_RES2                     *(_IO uint16 xdata *)(SCAT3_BADR + 0x08)// rw-- Sin/Cos模式下为Sin; ATan模式下为角度

 #define SCAT2_COS                      *(_IO uint16 xdata *)(SCAT2_BADR + 0x00)// rw-- Cos
 #define SCAT2_SIN                      *(_IO uint16 xdata *)(SCAT2_BADR + 0x02)// rw-- Sin
 #define SCAT2_THE                      *(_IO uint16 xdata *)(SCAT2_BADR + 0x04)// rw-- Angle
 #define SCAT2_RES1                     *(_IO uint16 xdata *)(SCAT2_BADR + 0x06)// rw-- Sin/Cos模式下为Cos; ATan模式下为幅值
 #define SCAT2_RES2                     *(_IO uint16 xdata *)(SCAT2_BADR + 0x08)// rw-- Sin/Cos模式下为Sin; ATan模式下为角度

 #define SCAT1_COS                      *(_IO uint16 xdata *)(SCAT1_BADR + 0x00)// rw-- Cos
 #define SCAT1_SIN                      *(_IO uint16 xdata *)(SCAT1_BADR + 0x02)// rw-- Sin
 #define SCAT1_THE                      *(_IO uint16 xdata *)(SCAT1_BADR + 0x04)// rw-- Angle
 #define SCAT1_RES1                     *(_IO uint16 xdata *)(SCAT1_BADR + 0x06)// rw-- Sin/Cos模式下为Cos; ATan模式下为幅值
 #define SCAT1_RES2                     *(_IO uint16 xdata *)(SCAT1_BADR + 0x08)// rw-- Sin/Cos模式下为Sin; ATan模式下为角度

 #define SCAT0_COS                      *(_IO uint16 xdata *)(SCAT0_BADR + 0x00)// rw-- Cos
 #define SCAT0_SIN                      *(_IO uint16 xdata *)(SCAT0_BADR + 0x02)// rw-- Sin
 #define SCAT0_THE                      *(_IO uint16 xdata *)(SCAT0_BADR + 0x04)// rw-- Angle
 #define SCAT0_RES1                     *(_IO uint16 xdata *)(SCAT0_BADR + 0x06)// rw-- Sin/Cos模式下为Cos; ATan模式下为幅值
 #define SCAT0_RES2                     *(_IO uint16 xdata *)(SCAT0_BADR + 0x08)// rw-- Sin/Cos模式下为Sin; ATan模式下为角度

 #define DIV3_DAH                       *(_IO uint16 xdata *)(DIV3_BADR  + 0x00)// rw-- 被除数高16位
 #define DIV3_DAL                       *(_IO uint16 xdata *)(DIV3_BADR  + 0x02)// rw-- 被除数低16位
 #define DIV3_DB                        *(_IO uint16 xdata *)(DIV3_BADR  + 0x04)// rw-- 除数
 #define DIV3_DQH                       *(_IO uint16 xdata *)(DIV3_BADR  + 0x06)// rw-- 商高16位
 #define DIV3_DQL                       *(_IO uint16 xdata *)(DIV3_BADR  + 0x08)// rw-- 商低16位
 #define DIV3_DR                        *(_IO uint16 xdata *)(DIV3_BADR  + 0x0a)// rw-- 余数

 #define DIV2_DAH                       *(_IO uint16 xdata *)(DIV2_BADR  + 0x00)// rw-- 被除数高16位
 #define DIV2_DAL                       *(_IO uint16 xdata *)(DIV2_BADR  + 0x02)// rw-- 被除数低16位
 #define DIV2_DB                        *(_IO uint16 xdata *)(DIV2_BADR  + 0x04)// rw-- 除数
 #define DIV2_DQH                       *(_IO uint16 xdata *)(DIV2_BADR  + 0x06)// rw-- 商高16位
 #define DIV2_DQL                       *(_IO uint16 xdata *)(DIV2_BADR  + 0x08)// rw-- 商低16位
 #define DIV2_DR                        *(_IO uint16 xdata *)(DIV2_BADR  + 0x0a)// rw-- 余数

 #define DIV1_DAH                       *(_IO uint16 xdata *)(DIV1_BADR  + 0x00)// rw-- 被除数高16位
 #define DIV1_DAL                       *(_IO uint16 xdata *)(DIV1_BADR  + 0x02)// rw-- 被除数低16位
 #define DIV1_DB                        *(_IO uint16 xdata *)(DIV1_BADR  + 0x04)// rw-- 除数
 #define DIV1_DQH                       *(_IO uint16 xdata *)(DIV1_BADR  + 0x06)// rw-- 商高16位
 #define DIV1_DQL                       *(_IO uint16 xdata *)(DIV1_BADR  + 0x08)// rw-- 商低16位
 #define DIV1_DR                        *(_IO uint16 xdata *)(DIV1_BADR  + 0x0a)// rw-- 余数

 #define DIV0_DAH                       *(_IO uint16 xdata *)(DIV0_BADR  + 0x00)// rw-- 被除数高16位
 #define DIV0_DAL                       *(_IO uint16 xdata *)(DIV0_BADR  + 0x02)// rw-- 被除数低16位
 #define DIV0_DB                        *(_IO uint16 xdata *)(DIV0_BADR  + 0x04)// rw-- 除数
 #define DIV0_DQH                       *(_IO uint16 xdata *)(DIV0_BADR  + 0x06)// rw-- 商高16位
 #define DIV0_DQL                       *(_IO uint16 xdata *)(DIV0_BADR  + 0x08)// rw-- 商低16位
 #define DIV0_DR                        *(_IO uint16 xdata *)(DIV0_BADR  + 0x0a)// rw-- 余数

 #define MUL3_MA                        *(_IO uint16 xdata *)(MUL3_BADR  + 0x00)// rw-- 被乘数
 #define MUL3_MB                        *(_IO uint16 xdata *)(MUL3_BADR  + 0x02)// rw-- 乘数
 #define MUL3_MCH                       *(_IO uint16 xdata *)(MUL3_BADR  + 0x04)// rw-- 积高16位
 #define MUL3_MCL                       *(_IO uint16 xdata *)(MUL3_BADR  + 0x06)// rw-- 积低16位

 #define MUL2_MA                        *(_IO uint16 xdata *)(MUL2_BADR  + 0x00)// rw-- 被乘数
 #define MUL2_MB                        *(_IO uint16 xdata *)(MUL2_BADR  + 0x02)// rw-- 乘数
 #define MUL2_MCH                       *(_IO uint16 xdata *)(MUL2_BADR  + 0x04)// rw-- 积高16位
 #define MUL2_MCL                       *(_IO uint16 xdata *)(MUL2_BADR  + 0x06)// rw-- 积低16位

 #define MUL1_MA                        *(_IO uint16 xdata *)(MUL1_BADR  + 0x00)// rw-- 被乘数
 #define MUL1_MB                        *(_IO uint16 xdata *)(MUL1_BADR  + 0x02)// rw-- 乘数
 #define MUL1_MCH                       *(_IO uint16 xdata *)(MUL1_BADR  + 0x04)// rw-- 积高16位
 #define MUL1_MCL                       *(_IO uint16 xdata *)(MUL1_BADR  + 0x06)// rw-- 积低16位

 #define MUL0_MA                        *(_IO uint16 xdata *)(MUL0_BADR  + 0x00)// rw-- 被乘数
 #define MUL0_MB                        *(_IO uint16 xdata *)(MUL0_BADR  + 0x02)// rw-- 乘数
 #define MUL0_MCH                       *(_IO uint16 xdata *)(MUL0_BADR  + 0x04)// rw-- 积高16位
 #define MUL0_MCL                       *(_IO uint16 xdata *)(MUL0_BADR  + 0x06)// rw-- 积低16位

 #define LPF3_K                         *(_IO  int16 xdata *)(LPF3_BADR  + 0x00)// rw-- 滤波系数[32767, 0]
 #define LPF3_X                         *(_IO  int16 xdata *)(LPF3_BADR  + 0x02)// rw-- 滤波输入
 #define LPF3_YH                        *(_IO uint16 xdata *)(LPF3_BADR  + 0x04)// rw-- 滤波结果高16位
 #define LPF3_YL                        *(_IO uint16 xdata *)(LPF3_BADR  + 0x06)// rw-- 滤波结果低16位

 #define LPF2_K                         *(_IO  int16 xdata *)(LPF2_BADR  + 0x00)// rw-- 滤波系数[32767, 0]
 #define LPF2_X                         *(_IO  int16 xdata *)(LPF2_BADR  + 0x02)// rw-- 滤波输入
 #define LPF2_YH                        *(_IO uint16 xdata *)(LPF2_BADR  + 0x04)// rw-- 滤波结果高16位
 #define LPF2_YL                        *(_IO uint16 xdata *)(LPF2_BADR  + 0x06)// rw-- 滤波结果低16位

 #define LPF1_K                         *(_IO  int16 xdata *)(LPF1_BADR  + 0x00)// rw-- 滤波系数[32767, 0]
 #define LPF1_X                         *(_IO  int16 xdata *)(LPF1_BADR  + 0x02)// rw-- 滤波输入
 #define LPF1_YH                        *(_IO uint16 xdata *)(LPF1_BADR  + 0x04)// rw-- 滤波结果高16位
 #define LPF1_YL                        *(_IO uint16 xdata *)(LPF1_BADR  + 0x06)// rw-- 滤波结果低16位

 #define LPF0_K                         *(_IO  int16 xdata *)(LPF0_BADR  + 0x00)// rw-- 滤波系数[32767, 0]
 #define LPF0_X                         *(_IO  int16 xdata *)(LPF0_BADR  + 0x02)// rw-- 滤波输入
 #define LPF0_YH                        *(_IO uint16 xdata *)(LPF0_BADR  + 0x04)// rw-- 滤波结果高16位
 #define LPF0_YL                        *(_IO uint16 xdata *)(LPF0_BADR  + 0x06)// rw-- 滤波结果低16位

 #define PI3_KP                         *(_IO  int16 xdata *)(PI3_BADR   + 0x00)// rw-- Kp系数, Q12格式, 有效范围[32767, 0]
 #define PI3_EK1                        *(_IO  int16 xdata *)(PI3_BADR   + 0x02)// rw-- 上一次的误差
 #define PI3_EK                         *(_IO  int16 xdata *)(PI3_BADR   + 0x04)// rw-- 当前的误差
 #define PI3_KI                         *(_IO  int16 xdata *)(PI3_BADR   + 0x06)// rw-- Ki系数, Q15格式, 有效范围[32767, 0]
 #define PI3_UKH                        *(_IO  int16 xdata *)(PI3_BADR   + 0x08)// rw-- PI计算结果高16位
 #define PI3_UKL                        *(_IO  int16 xdata *)(PI3_BADR   + 0x0a)// rw-- PI计算结果低16位
 #define PI3_UKMAX                      *(_IO  int16 xdata *)(PI3_BADR   + 0x0c)// rw-- UK限幅最大值
 #define PI3_UKMIN                      *(_IO  int16 xdata *)(PI3_BADR   + 0x0e)// rw-- UK限幅最小值
 #define PI3_KD                         *(_IO  int16 xdata *)(PI3_BADR   + 0x10)// rw-- Kd系数, Q12格式, 有效范围[32767, 0]
 #define PI3_EK2                        *(_IO  int16 xdata *)(PI3_BADR   + 0x12)// rw-- 上两次的误差

 #define PI2_KP                         *(_IO  int16 xdata *)(PI2_BADR   + 0x00)// rw-- Kp系数, Q12格式, 有效范围[32767, 0]
 #define PI2_EK1                        *(_IO  int16 xdata *)(PI2_BADR   + 0x02)// rw-- 上一次的误差
 #define PI2_EK                         *(_IO  int16 xdata *)(PI2_BADR   + 0x04)// rw-- 当前的误差
 #define PI2_KI                         *(_IO  int16 xdata *)(PI2_BADR   + 0x06)// rw-- Ki系数, Q15格式, 有效范围[32767, 0]
 #define PI2_UKH                        *(_IO  int16 xdata *)(PI2_BADR   + 0x08)// rw-- PI计算结果高16位
 #define PI2_UKL                        *(_IO  int16 xdata *)(PI2_BADR   + 0x0a)// rw-- PI计算结果低16位
 #define PI2_UKMAX                      *(_IO  int16 xdata *)(PI2_BADR   + 0x0c)// rw-- UK限幅最大值
 #define PI2_UKMIN                      *(_IO  int16 xdata *)(PI2_BADR   + 0x0e)// rw-- UK限幅最小值
 #define PI2_KD                         *(_IO  int16 xdata *)(PI2_BADR   + 0x10)// rw-- Kd系数, Q12格式, 有效范围[32767, 0]
 #define PI2_EK2                        *(_IO  int16 xdata *)(PI2_BADR   + 0x12)// rw-- 上两次的误差

 #define PI1_KP                         *(_IO  int16 xdata *)(PI1_BADR   + 0x00)// rw-- Kp系数, Q12格式, 有效范围[32767, 0]
 #define PI1_EK1                        *(_IO  int16 xdata *)(PI1_BADR   + 0x02)// rw-- 上一次的误差
 #define PI1_EK                         *(_IO  int16 xdata *)(PI1_BADR   + 0x04)// rw-- 当前的误差
 #define PI1_KI                         *(_IO  int16 xdata *)(PI1_BADR   + 0x06)// rw-- Ki系数, Q15格式, 有效范围[32767, 0]
 #define PI1_UKH                        *(_IO  int16 xdata *)(PI1_BADR   + 0x08)// rw-- PI计算结果高16位
 #define PI1_UKL                        *(_IO  int16 xdata *)(PI1_BADR   + 0x0a)// rw-- PI计算结果低16位
 #define PI1_UKMAX                      *(_IO  int16 xdata *)(PI1_BADR   + 0x0c)// rw-- UK限幅最大值
 #define PI1_UKMIN                      *(_IO  int16 xdata *)(PI1_BADR   + 0x0e)// rw-- UK限幅最小值

 #define PI0_KP                         *(_IO  int16 xdata *)(PI0_BADR   + 0x00)// rw-- Kp系数, Q12格式, 有效范围[32767, 0]
 #define PI0_EK1                        *(_IO  int16 xdata *)(PI0_BADR   + 0x02)// rw-- 上一次的误差
 #define PI0_EK                         *(_IO  int16 xdata *)(PI0_BADR   + 0x04)// rw-- 当前的误差
 #define PI0_KI                         *(_IO  int16 xdata *)(PI0_BADR   + 0x06)// rw-- Ki系数, Q15格式, 有效范围[32767, 0]
 #define PI0_UKH                        *(_IO  int16 xdata *)(PI0_BADR   + 0x08)// rw-- PI计算结果高16位
 #define PI0_UKL                        *(_IO  int16 xdata *)(PI0_BADR   + 0x0a)// rw-- PI计算结果低16位
 #define PI0_UKMAX                      *(_IO  int16 xdata *)(PI0_BADR   + 0x0c)// rw-- UK限幅最大值
 #define PI0_UKMIN                      *(_IO  int16 xdata *)(PI0_BADR   + 0x0e)// rw-- UK限幅最小值
/******************************************************************************///FOC
 #define FOC_CR0                        *(_IO  uint8 xdata *)0x409f             // RV:  00H  FOC控制寄存器0
 #define OMIF                                     0x80                          //               r--- 当前转速<FOC_EFREQMIN为1
 #define OMAF                                     0x40                          //               r--- 当前转速>FOC_EFREQMAX为1
 #define MERRS1                                   0x20                          //               rw-- 滑膜算法的MAX ERR选择
 #define MERRS0                                   0x10                          //               rw-- 滑膜算法的MAX ERR选择
 #define UCSEL                                    0x08                          //               rw-- UDC采样通道选择0：AD2 1:AD15
 #define OMAS                                     0x04                          //               rw-- 估算速度过大时输出选择位
 #define ESCMS                                    0x02                          //               rw-- omega计算模式为ATAN
 #define EDIS                                     0x01                          //               rw-- EALPHA/EBETA自动更新禁止

 #define FOC_CR1                        *(_IO  uint8 xdata *)0x40a0             // RV:  00H  FOC控制寄存器1
 #define OVMDL                                    0x80                          //               rw-- 过调制使能
 #define EFAE                                     0x40                          //               rw-- 估算器强制角度使能
 #define RFAE                                     0x20                          //               rw-- 强制爬坡角度使能
 #define ANGM                                     0x10                          //               rw-- 角度模式选择
 #define CSM1                                     0x08                          //               rw-- 电流采样模式配置1
 #define CSM0                                     0x04                          //               rw-- 电流采样模式配置0
 #define SPWMSEL                                  0x02                          //               rw-- SPWM模式选择
 #define SVPWMEN                                  0x01                          //               rw-- SVPWM/SPWM选择

 #define FOC_CR2                        *(_IO  uint8 xdata *)0x40a1             // RV:  00H  FOC控制寄存器2
 #define ESEL                                     0x80                          //               rw-- 估算器模式
 #define ICLR                                     0x40                          //               rw-- FOC__IA/B/CMAX清零
 #define F5SEG                                    0x20                          //               rw-- 7/5段式SVPWM模式
 #define DSS                                      0x10                          //               rw-- 2/3电阻采样模式
 #define CSOC1                                    0x08                          //               rw-- 电流采样基准校准配置1
 #define CSOC0                                    0x04                          //               rw-- 电流采样基准校准配置0
 #define UQD                                      0x02                          //               rw-- 禁能Q轴PI
 #define UDD                                      0x01                          //               rw-- 禁能D轴PI

 #define FOC_CR3                        *(_IO  uint8 xdata *)0x409e             // RV:  00H  FOC控制寄存器3
 #define EFAM                                     0x80                          //               rw-- 新算法
 #define TAMD                                     0x40                          //               rw-- Theta ATAN MODE
 #define MFP_EN                                   0x20                          //               rw-- MFP使能
 #define FOC_THECOMP_DIS                          0x10                          //               rw-- 不加补偿角算法
 #define FOCFEN                                   0x08                          //               rw-- FOC强制使能
 #define HALL_PLL_EN                              0x04                          //               rw-- PLL模式下HALL滤波处理
 #define TSMINH9                                  0x02                          //               rw-- 单电阻模式下的ADC采样窗口  或  双三电阻模式下死区补偿
 #define TSMINH8                                  0x01                          //               rw-- 单电阻模式下的ADC采样窗口  或  双三电阻模式下死区补偿

 #define FOC_CR4                        *(_IO  uint8 xdata *)0x409d             // RV:  00H  FOC控制寄存器4, 高频注入模式用
 #define HFI_ST                                   0x80                          //               rw-- 手动正负UD控制
 #define HFI_DEL_MAN_EN                           0x40                          //               rw-- 手动填写delta_theta使能
 #define HFI_SMO_EN                               0x20                          //               rw-- PLL前补偿角度使能
 #define HFI_THE_SEL                              0x10                          //               rw-- atan的计算源选择
 #define HFI_MAN_EN                               0x08                          //               rw-- 手动填写atan计算源使能
 #define HFI_OME_EN                               0x04                          //               rw-- 估算角度的计算源选择
 #define HFI_TOG_EN                               0x02                          //               rw-- 自动正负UD控制使能
 #define HFI_EN                                   0x01                          //               rw-- 高频注入使能

 #define FOC_CR5                        *(_IO  uint8 xdata *)0x409c             // RV:  00H  FOC控制寄存器5
 #define SAM_MD                                   0x80                          //               rw-- 三电阻采样模式选择
 #define HFI_FDB_SEL                              0x40                          //               rw-- 角度补偿加入位置选择
 #define DT_X2_EN                                 0x20                          //               rw-- 高频注入的角度分频使能
 #define VAB_CPS_EN                               0x10                          //               rw-- 高频注入位置选择
 #define SMO_REV_EN                               0x08                          //               rw-- PLL前角度补偿量选择
 #define HFI_MD                                   0x04                          //               rw-- 高频注入模式选择
 #define OME_ABS_EN                               0x02                          //               rw-- 估算角度反转使能
 #define CAL2_EN                                  0x01                          //               rw-- 角度估算周期控制

 #define FOC_EKP                        *(_IO  int16 xdata *)0x4074             // RV:0000H  估算器的PI的P参数
 #define FOC_EKI                        *(_IO  int16 xdata *)0x4076             // RV:0000H  估算器的PI的I参数
 #define FOC_KSLIDE                     *(_IO  int16 xdata *)0x4078             // RV:0000H  估算器SMO模式下的KSLIDE  或  PLL模式下的KP
 #define FOC_EKLPFMIN                   *(_IO  int16 xdata *)0x407a             // RV:0000H  估算器SMO模式下EMF的LPF系数最小值  或  PLL模式下的KI
 #define FOC_OMEKLPF                    *(_IO  int16 xdata *)0x407e             // RV:0000H  估算器速度计算的LPF系数
 #define FOC_FBASE                      *(_IO  int16 xdata *)0x4080             // RV:0000H  估算器角度增量(OMEGA)的系数
 #define FOC_EFREQACC                   *(_IO uint16 xdata *)0x4082             // RV:0000H  估算器在强制角度模式下的OMEGA增量
 #define FOC_EFREQMIN                   *(_IO  int16 xdata *)0x4084             // RV:0000H  估算器在强制角度模式下的OMEGA最小值
 #define FOC_EFREQHOLD                  *(_IO  int16 xdata *)0x4086             // RV:0000H  估算器在强制角度模式下的OMEGA保持值
 #define FOC_EFREQMAX                   *(_IO  uint8 xdata *)0x406f             // RV:  7FH  估算器的强制角度模式下的OMEGA最大值
 #define FOC_EK3                        *(_IO  int16 xdata *)0x4088             // RV:0000H  估算器估算电流的系数3
 #define FOC_EK4                        *(_IO  int16 xdata *)0x408a             // RV:0000H  估算器估算电流的系数4
 #define FOC_EK1                        *(_IO  int16 xdata *)0x408c             // RV:0000H  估算器估算电流的系数1
 #define FOC_EK2                        *(_IO  int16 xdata *)0x408e             // RV:0000H  估算器估算电流的系数2
 #define FOC_IDREF                      *(_IO  int16 xdata *)0x4090             // RV:0000H  D轴电流参考值
 #define FOC_IQREF                      *(_IO  int16 xdata *)0x4092             // RV:0000H  Q轴电流参考值
 #define FOC_QKP                        *(_IO  int16 xdata *)0x4094             // RV:0000H  Q轴PI的比例参数
 #define FOC_QKI                        *(_IO  int16 xdata *)0x4096             // RV:0000H  Q轴PI的积分参数
 #define FOC_DKP                        *(_IO  int16 xdata *)0x4070             // RV:0000H  D轴PI的比例参数
 #define FOC_DKI                        *(_IO  int16 xdata *)0x407c             // RV:0000H  D轴PI的积分参数
 #define FOC__UDCFLT                    *(_I   int16 xdata *)0x4098             // RV:0000H  滤波后的母线电压
 #define FOC_TSMIN                      *(_IO  uint8 xdata *)0x40a2             // RV:  00H  单电阻模式下的ADC采样窗口  或  双三电阻模式下死区补偿
 #define FOC_TGLI                       *(_IO  uint8 xdata *)0x40a3             // RV:  00H  上桥导通窄脉冲消除
 #define FOC_TBLO                       *(_IO  uint8 xdata *)0x40a4             // RV:  00H  三电阻电流采样的屏蔽时间
 #define FOC_TRGDLY                     *(_IO   int8 xdata *)0x40a5             // RV:  00H  单电阻模式下ADC采样触发延迟  或  双三电阻模式下电流采样时机
 #define FOC_CSO                        *(_IO  int16 xdata *)0x40a6             // RV:4000H  电流采样基准值
 #define FOC__RTHESTEP                  *(_IO  int16 xdata *)0x40a8             // RV:0000H  爬坡速度
 #define FOC_RTHEACC                    *(_O   int16 xdata *)0x40aa             // RV:0000H  爬坡加速度
 #define FOC__EOMELPF                   *(_I   int16 xdata *)0x40aa             // RV:0000H  滤波后速度
 #define FOC_RTHECNT                    *(_IO  uint8 xdata *)0x40ac             // RV:  00H  爬坡次数
 #define FOC_THECOR                     *(_IO  uint8 xdata *)0x40ad             // RV:  01H  爬坡模式切换到估算模式的角度修正值
 #define FOC__EMF                       *(_I   int16 xdata *)0x40ae             // RV:0000H  估算器估算的反电动势EMF
 #define FOC_THECOMP                    *(_O   int16 xdata *)0x40ae             // RV:0000H  角度补偿值，用于补偿估算器估算出的角度
 #define FOC_DMAX                       *(_O   int16 xdata *)0x40b0             // RV:0000H  D轴PI控制器的输出上限
 #define FOC__OMEEST                    *(_I   int16 xdata *)0x40b0             // RV:0000H  FOC估算的速度
 #define FOC_DMIN                       *(_O   int16 xdata *)0x40b2             // RV:0000H  D轴PI控制器的输出下限
 #define FOC_QMAX                       *(_IO  int16 xdata *)0x40b4             // RV:0000H  Q轴PI控制器的输出上限
 #define FOC_QMIN                       *(_IO  int16 xdata *)0x40b6             // RV:0000H  Q轴PI控制器的输出下限
 #define FOC__UD                        *(_IO  int16 xdata *)0x40b8             // RV:0000H  D轴电压输出值
 #define FOC__UQ                        *(_IO  int16 xdata *)0x40ba             // RV:0000H  Q轴电压输出值
 #define FOC__ID                        *(_I   int16 xdata *)0x40bc             // RV:0000H  D轴反馈电流值
 #define FOC__IQ                        *(_I   int16 xdata *)0x40be             // RV:0000H  Q轴反馈电流值
 #define FOC_IQ_LPFK                    *(_O   uint8 xdata *)0x40c0             // RV:  FFH  FOC IQ低通滤波系数
 #define FOC_ID_LPFK                    *(_O   uint8 xdata *)0x40c1             // RV:  FFH  FOC ID低通滤波系数
 #define FOC__IBET                      *(_I   int16 xdata *)0x40c0             // RV:0000H  β轴采样电流值
 #define FOC__VBET                      *(_I   int16 xdata *)0x40c2             // RV:0000H  β轴电压输出值
 #define FOC_UDCPS                      *(_O   int16 xdata *)0x40c2             // RV:0000H  D轴的电压前馈补偿
 #define FOC__VALP                      *(_I   int16 xdata *)0x40c4             // RV:0000H  α轴电压输出值
 #define FOC_UQCPS                      *(_O   int16 xdata *)0x40c4             // RV:0000H  Q轴的电压前馈补偿
 #define FOC__IC                        *(_I   int16 xdata *)0x40c6             // RV:0000H  C相电流
 #define FOC_FLUX                       *(_O   int16 xdata *)0x40c6             // RV:0000H  电机磁链
 #define FOC__IB                        *(_I   int16 xdata *)0x40c8             // RV:0000H  B相电流
 #define FOC_LQ                         *(_O   int16 xdata *)0x40c8             // RV:0000H
 #define FOC__IA                        *(_I   int16 xdata *)0x40ca             // RV:0000H  A相电流
 #define FOC_LD                         *(_O   int16 xdata *)0x40ca             // RV:0000H
 #define FOC__THETA                     *(_IO  int16 xdata *)0x40cc             // RV:0000H  电机电角度
 #define FOC__ETHETA                    *(_IO  int16 xdata *)0x40ce             // RV:0000H  估算器估算的角度
 #define FOC__EALP                      *(_IO  int16 xdata *)0x40d0             // RV:0000H  估算器估算反电动势的α轴值
 #define FOC__EBET                      *(_IO  int16 xdata *)0x40d2             // RV:0000H  估算器估算反电动势的β轴值
 #define FOC__EOME                      *(_IO  int16 xdata *)0x40d4             // RV:0000H  估算器估算的速度
 #define FOC__UQEX                      *(_I   int16 xdata *)0x40d6             // RV:0000H  Q轴的电压溢出值，用于弱磁控制
 #define FOC_KFG                        *(_O  uint16 xdata *)0x40d6             // RV:0000H  FG计算系数
 #define FOC__POW                       *(_I   int16 xdata *)0x40d8             // RV:0000H  电机当前运行功率
 #define FOC_EOMEKLPF                   *(_O   uint8 xdata *)0x40d8             // RV:0000H  EOMELPF的滤波系数
 #define FOC__IAMAX                     *(_I   int16 xdata *)0x40da             // RV:0000H  A相电流最大值
 #define FOC__IBMAX                     *(_I   int16 xdata *)0x40dc             // RV:0000H  B相电流最大值
 #define FOC__ICMAX                     *(_I   int16 xdata *)0x40de             // RV:0000H  C相电流最大值
 #define HFI_KP                         *(_IO  int16 xdata *)0x40e0             // RV:0000H  高频注入PLL锁相环PI调节器KP
 #define HFI_KI                         *(_IO  int16 xdata *)0x40e2             // RV:0000H  高频注入PLL锁相环PI调节器KI
 #define OMEGA_HFI                      *(_IO  int16 xdata *)0x40e4             // RV:0000H  高频注入算法: 估算转速
 #define HFI_ALPHA                      *(_IO  int16 xdata *)0x40e6             // RV:0000H  高频注入算法: 计算到的alpha轴高频电流
 #define HFI_BETA                       *(_IO  int16 xdata *)0x40e8             // RV:0000H  高频注入算法: 计算到的belta轴高频电流
 #define ID0                            *(_IO  int16 xdata *)0x40ea             // RV:0000H  当前的ID
 #define ID1                            *(_IO  int16 xdata *)0x40ec             // RV:0000H  上一次的ID
 #define DELTA_THETA                    *(_IO  int16 xdata *)0x40ee             // RV:0000H  delta_theta控制, 手动模式有效
 #define IQ0                            *(_IO  int16 xdata *)0x40f0             // RV:0000H  当前的IQ
 #define IQ1                            *(_IO  int16 xdata *)0x40f2             // RV:0000H  上一次的IQ
 #define HFI_I1                         *(_IO  int16 xdata *)0x40f4             // RV:0000H
 #define HFI_I2                         *(_IO  int16 xdata *)0x40f6             // RV:0000H
 #define HFI_I3                         *(_IO  int16 xdata *)0x40f8             // RV:0000H
 #define K_HFI                          *(_IO  int16 xdata *)0x409b             // RV:0000H
/******************************************************************************///TIM1
 #define TIM1_CR0                       *(_IO  uint8 xdata *)0x4068             // RV:  00H  TIMER1控制寄存器0
 #define T1RWEN                                   0x80                          //               r0w- T1RCEN操作允许
 #define T1CFLT1                                  0x40                          //               rw-- 换相滤波配置1
 #define T1CFLT0                                  0x20                          //               rw-- 换相滤波配置0
 #define T1FORC                                   0x10                          //               rw-- 60°强制换相
 #define T1OPS1                                   0x08                          //               rw-- 数据传输方式配置1
 #define T1OPS0                                   0x04                          //               rw-- 数据传输方式配置0
 #define T1BCEN                                   0x02                          //               rw-- 基本定时器计数使能
 #define T1RCEN                                   0x01                          //               rw-- 重载定时器计数使能

 #define TIM1_CR1                       *(_IO  uint8 xdata *)0x4069             // RV:  00H  TIMER1控制寄存器1
 #define T1BAPE                                   0x80                          //               rw-- TIM1__BARR自动装载使能

 #define TIM1_CR2                       *(_IO  uint8 xdata *)0x406a             // RV:  00H  TIMER1控制寄存器2
 #define T1BRS                                    0x80                          //               rw-- 基本定时器复位源选择

 #define TIM1_CR3                       *(_IO  uint8 xdata *)0x406b             // RV:  00H  TIMER1控制寄存器3
 #define T1COM_MD                                 0x80                          //               rw-- 方波换相模式选择
 #define T1PSC2                                   0x40                          //               rw-- 定时器分频配置2
 #define T1PSC1                                   0x20                          //               rw-- 定时器分频配置1
 #define T1PSC0                                   0x10                          //               rw-- 定时器分频配置0
 #define T1TIS1                                   0x08                          //               rw-- 输入源选择配置1
 #define T1TIS0                                   0x04                          //               rw-- 输入源选择配置0
 #define T1INM1                                   0x02                          //               rw-- 输入源噪声滤波配置1
 #define T1INM0                                   0x01                          //               rw-- 输入源噪声滤波配置0

 #define TIM1_CR4                       *(_IO  uint8 xdata *)0x406c             // RV:  00H  TIMER1控制寄存器4
 #define T1CST2                                   0x04                          //               rw-- 换相状态机配置2
 #define T1CST1                                   0x02                          //               rw-- 换相状态机配置1
 #define T1CST0                                   0x01                          //               rw-- 换相状态机配置0

 #define TIM1_CR5                       *(_IO  uint8 xdata *)0x4072             // RV:  00H  TIMER1控制寄存器5
 #define T1POP                                    0x80                          //               rw-- DRV计数器上溢事件触发数据传输使能
 #define T1WTS                                    0x40                          //               rw-- PWM OFF时候换相，消除窄脉冲
 #define ITRIP_DIS                                0x08                          //               rw-- 方波忽略ITRIP电流采样
 #define UCOP_DIS                                 0x04                          //               rw-- 方波忽略导通相电压
 #define T1AFL1                                   0x02                          //               rw-- ADC采样点压计算结果滤波
 #define T1AFL0                                   0x01                          //               rw-- ADC采样点压计算结果滤波

 #define TIM1_IER                       *(_IO  uint8 xdata *)0x406d             // RV:  00H  TIMER1中断控制
 #define T1UPD                                    0x80                          //               r0w- OPS=00时，触发数据传输
 #define T1MAME                                   0x40                          //               rw-- TIM1手动模式使能
 #define T1ADIE                                   0x20                          //               rw-- ADC检测中断使能
 #define T1BOIE                                   0x10                          //               rw-- 基本定时器上溢中断使能
 #define T1ROIE                                   0x08                          //               rw-- 重载定时器上溢中断使能
 #define T1WTIE                                   0x04                          //               rw-- 写入时序中断使能
 #define T1PDIE                                   0x02                          //               rw-- 位置检测中断使能
 #define T1BDIE                                   0x01                          //               rw-- 屏蔽续流结束中断使能

 #define TIM1_SR                        *(_IO  uint8 xdata *)0x406e             // RV:  00H  TIMER1状态
 #define T1ADIF                                   0x20                          //               rw0- ADC检测中断标志
 #define T1BOIF                                   0x10                          //               rw0- 基本定时器上溢中断标志
 #define T1ROIF                                   0x08                          //               rw0- 重载定时器上溢中断标志
 #define T1WTIF                                   0x04                          //               rw0- 写入时序中断标志
 #define T1PDIF                                   0x02                          //               rw0- 位置检测中断标志
 #define T1BDIF                                   0x01                          //               rw0- 屏蔽续流结束中断标志

 #define TIM1_DBR1                      *(_IO uint16 xdata *)0x4074             // RV:0000H  TIMER1在CST=1的数据
 #define TIM1_DBR2                      *(_IO uint16 xdata *)0x4076             // RV:0000H  TIMER1在CST=2的数据
 #define TIM1_DBR3                      *(_IO uint16 xdata *)0x4078             // RV:0000H  TIMER1在CST=3的数据
 #define TIM1_DBR4                      *(_IO uint16 xdata *)0x407a             // RV:0000H  TIMER1在CST=4的数据
 #define TIM1_DBR5                      *(_IO uint16 xdata *)0x407c             // RV:0000H  TIMER1在CST=5的数据
 #define TIM1_DBR6                      *(_IO uint16 xdata *)0x407e             // RV:0000H  TIMER1在CST=6的数据
 #define TIM1_DBR7                      *(_IO uint16 xdata *)0x4080             // RV:0000H  TIMER1在CST=7的数据
 #define T1CPE2                                   0x4000                        //               rw-- 输入沿极性和比较器选择配置2
 #define T1CPE1                                   0x2000                        //               rw-- 输入沿极性和比较器选择配置1
 #define T1CPE0                                   0x1000                        //               rw-- 输入沿极性和比较器选择配置0
 #define T1WHP                                    0x0800                        //               rw-- W相上管反相使能
 #define T1WLP                                    0x0400                        //               rw-- W相下管反相使能
 #define T1VHP                                    0x0200                        //               rw-- V相上管反相使能
 #define T1VLP                                    0x0100                        //               rw-- V相下管反相使能
 #define T1UHP                                    0x0080                        //               rw-- U相上管反相使能
 #define T1ULP                                    0x0040                        //               rw-- U相下管反相使能
 #define T1WHE                                    0x0020                        //               rw-- W相上管输出使能
 #define T1WLE                                    0x0010                        //               rw-- W相下管输出使能
 #define T1VHE                                    0x0008                        //               rw-- V相上管输出使能
 #define T1VLE                                    0x0004                        //               rw-- V相下管输出使能
 #define T1UHE                                    0x0002                        //               rw-- U相上管输出使能
 #define T1ULE                                    0x0001                        //               rw-- U相下管输出使能

 #define TIM1_BCOR                      *(_IO uint16 xdata *)0x4070             // RV:0000H  捕获基本定时器计数值滤波值
 #define TIM1__BCNTR                    *(_IO uint16 xdata *)0x4082             // RV:0000H  基本定时器计数值
 #define TIM1__BCCR                     *(_IO uint16 xdata *)0x4084             // RV:0000H  捕获基本定时器计数值
 #define TIM1__BARR                     *(_IO uint16 xdata *)0x4086             // RV:0000H  基本定时器自动重载值
 #define TIM1__RARR                     *(_IO uint16 xdata *)0x4088             // RV:0000H  重载定时器自动重载值
 #define TIM1__RCNTR                    *(_IO uint16 xdata *)0x408a             // RV:ffffH  重载定时器计数值
 #define TIM1__ITRIP                    *(_I  uint16 xdata *)0x4098             // RV:0000H  滤波后的母线电流
 #define TIM1__UCOP                     *(_IO uint16 xdata *)0x408c             // RV:0000H  导通相电压
 #define TIM1__UFLP                     *(_IO uint16 xdata *)0x408e             // RV:0000H  悬空相电压
 #define TIM1__URES                     *(_IO  int16 xdata *)0x4090             // RV:0000H  计算结果，公式发生符号变化即产生换相信号
 #define TIM1__KRMAX                    *(_IO  uint8 xdata *)0x4092             // RV:  00H  上升沿系数最大值
 #define TIM1__KFMIN                    *(_IO  uint8 xdata *)0x4093             // RV:  00H  下降沿系数最小值
 #define TIM1_KF                        *(_IO uint16 xdata *)0x4094             // RV:0000H  下降沿系数
 #define TIM1_KR                        *(_IO uint16 xdata *)0x4096             // RV:0000H  上升沿系数
/******************************************************************************///TIM2
 sfr     TIM2_CR0                       = 0xa1;                                 // RV:  00H  TIMER2控制寄存器0
 #define T2PSC2                                   0x80                          //               rw-- 定时器分频配置2
 #define T2PSC1                                   0x40                          //               rw-- 定时器分频配置1
 #define T2PSC0                                   0x20                          //               rw-- 定时器分频配置0
 #define T2OCM                                    0x10                          //               rw-- 定时器比较匹配模式/QEP&步进模式配置
 #define T2IRE                                    0x08                          //               rw-- 比较匹配/脉宽检测/方向改变中断使能
 #define T2CES                                    0x04                          //               rw-- 边沿模式选择
 #define T2MOD1                                   0x02                          //               rw-- 定时器模式配置1
 #define T2MOD0                                   0x01                          //               rw-- 定时器模式配置0

 sfr     TIM2_CR1                       = 0xa9;                                 // RV:  00H  TIMER2控制寄存器1
 #define T2IR                                     0x80                          //               rw0- 比较匹配标志
 #define T2IP                                     0x40                          //               rw0- 周期检测/计数匹配标志
 #define T2IF                                     0x20                          //               rw0- 计数上溢标志
 #define T2IPE                                    0x10                          //               rw-- 周期检测/计数匹配中断使能
 #define T2IFE                                    0x08                          //               rw-- 上溢中断使能
 #define T2FE                                     0x04                          //               rw-- 噪声滤波使能
 #define T2DIR                                    0x02                          //               r--- QEP/步进方向信号
 #define T2CEN                                    0x01                          //               rw-- 定时器使能

 sfr16   TIM2__DR                       = 0xac;                                 // RV:0000H  TIMER2匹配值
 sfr16   TIM2__ARR                      = 0xae;                                 // RV:0000H  TIMER2重载值
 sfr16   TIM2__CNTR                     = 0xaa;                                 // RV:0000H  TIMER2计数值
/******************************************************************************///TIM3
 sfr     TIM3_CR0                       = 0x9c;                                 // RV:  00H  TIMER3控制寄存器0
 #define T3PSC2                                   0x80                          //               rw-- 定时器分频配置2
 #define T3PSC1                                   0x40                          //               rw-- 定时器分频配置1
 #define T3PSC0                                   0x20                          //               rw-- 定时器分频配置0
 #define T3OCM                                    0x10                          //               rw-- 定时器比较匹配/捕获脉宽模式配置
 #define T3IRE                                    0x08                          //               rw-- 比较匹配/捕获脉宽中断
 #define T3OPM                                    0x02                          //               rw-- 单次模式使能
 #define T3MOD                                    0x01                          //               rw-- 定时器模式配置0

 sfr     TIM3_CR1                       = 0x9d;                                 // RV:  00H  TIMER3控制寄存器1
 #define T3IR                                     0x80                          //               rw0- 比较匹配/捕获脉宽标志
 #define T3IP                                     0x40                          //               rw0- 周期检测标志
 #define T3IF                                     0x20                          //               rw0- 计数上溢标志
 #define T3IPE                                    0x10                          //               rw-- 周期检测中断使能
 #define T3IFE                                    0x08                          //               rw-- 上溢中断使能
 #define T3NM1                                    0x04                          //               rw-- 噪音滤波配置1
 #define T3NM0                                    0x02                          //               rw-- 噪音滤波配置0
 #define T3EN                                     0x01                          //               rw-- 定时器使能

 sfr16   TIM3__DR                       = 0xa4;                                 // RV:0000H  TIMER3匹配值
 sfr16   TIM3__ARR                      = 0xa6;                                 // RV:0000H  TIMER3重载值
 sfr16   TIM3__CNTR                     = 0xa2;                                 // RV:0000H  TIMER3计数值
/******************************************************************************///TIM4
 sfr     TIM4_CR0                       = 0x9e;                                 // RV:  00H  TIMER4控制寄存器0
 #define T4PSC2                                   0x80                          //               rw-- 定时器分频配置2
 #define T4PSC1                                   0x40                          //               rw-- 定时器分频配置1
 #define T4PSC0                                   0x20                          //               rw-- 定时器分频配置0
 #define T4OCM                                    0x10                          //               rw-- 定时器比较匹配/捕获脉宽模式配置
 #define T4IRE                                    0x08                          //               rw-- 比较匹配/捕获脉宽中断
 #define T4OPM                                    0x02                          //               rw-- 单次模式使能
 #define T4MOD                                    0x01                          //               rw-- 定时器模式配置0

 sfr     TIM4_CR1                       = 0x9f;                                 // RV:  00H  TIMER4控制寄存器1
 #define T4IR                                     0x80                          //               rw0- 比较匹配/捕获脉宽标志
 #define T4IP                                     0x40                          //               rw0- 周期检测标志
 #define T4IF                                     0x20                          //               rw0- 计数上溢标志
 #define T4IPE                                    0x10                          //               rw-- 周期检测中断使能
 #define T4IFE                                    0x08                          //               rw-- 上溢中断使能
 #define T4NM1                                    0x04                          //               rw-- 噪音滤波配置1
 #define T4NM0                                    0x02                          //               rw-- 噪音滤波配置0
 #define T4EN                                     0x01                          //               rw-- 定时器使能

 sfr16   TIM4__DR                       = 0x94;                                 // RV:0000H  TIMER4匹配值
 sfr16   TIM4__ARR                      = 0x96;                                 // RV:0000H  TIMER4重载值
 sfr16   TIM4__CNTR                     = 0x92;                                 // RV:0000H  TIMER4计数值
/******************************************************************************///SYSTICK
 #define SYST_ARR                       *(_IO uint16 xdata *)0x4064             // RV:5dbfH  SysTick重载值
/******************************************************************************///DRV
 sfr     DRV_OUT                        = 0xf8;                                 // RV:  00H  驱动输出控制寄存器
 sbit    MOE                                      = DRV_OUT^7;                  //               rw-- 主输出使能
 sbit    OISWL                                    = DRV_OUT^5;                  //               rw-- WL & XL输出空闲电平
 sbit    OISWH                                    = DRV_OUT^4;                  //               rw-- WH & XH输出空闲电平
 sbit    OISVL                                    = DRV_OUT^3;                  //               rw-- VL输出空闲电平
 sbit    OISVH                                    = DRV_OUT^2;                  //               rw-- VH输出空闲电平
 sbit    OISUL                                    = DRV_OUT^1;                  //               rw-- UL输出空闲电平
 sbit    OISUH                                    = DRV_OUT^0;                  //               rw-- UH输出空闲电平

 #define DRV_DR                         *(_IO uint16 xdata *)0x4058             // RV:0000H  手动控制输出时设定的比较值
 #define DRV_COMR                       *(_IO uint16 xdata *)0x405a             // RV:0000H  计数器的比较匹配值

 #define DRV_CMR                        *(_IO uint16 xdata *)0x405c             // RV:0000H  输出配置寄存器
 #define XHP                                      0x8000                        //               rw-- X相上管反相使能
 #define XLP                                      0x4000                        //               rw-- X相下管反相使能
 #define XHE                                      0x2000                        //               rw-- X相上管输出使能
 #define XLE                                      0x1000                        //               rw-- X相下管输出使能
 #define WHP                                      0x0800                        //               rw-- W相上管反相使能
 #define WLP                                      0x0400                        //               rw-- W相下管反相使能
 #define VHP                                      0x0200                        //               rw-- V相上管反相使能
 #define VLP                                      0x0100                        //               rw-- V相下管反相使能
 #define UHP                                      0x0080                        //               rw-- U相上管反相使能
 #define ULP                                      0x0040                        //               rw-- U相下管反相使能
 #define WHE                                      0x0020                        //               rw-- W相上管输出使能
 #define WLE                                      0x0010                        //               rw-- W相下管输出使能
 #define VHE                                      0x0008                        //               rw-- V相上管输出使能
 #define VLE                                      0x0004                        //               rw-- V相下管输出使能
 #define UHE                                      0x0002                        //               rw-- U相上管输出使能
 #define ULE                                      0x0001                        //               rw-- U相下管输出使能

 #define DRV_ARR                        *(_IO uint16 xdata *)0x405e             // RV:0000H  计数器重载值
 #define DRV_DTR                        *(_IO  uint8 xdata *)0x4060             // RV:  00H  上下管死区时间
 #define DRV__CNTR                      *(_IO uint16 xdata *)0x4066             // RV:0000H  计数器值

 #define DRV_SR                         *(_IO  uint8 xdata *)0x4061             // RV:  00H  Driver状态寄存器
 #define SYSTIF                                   0x80                          //               rw0- SysTick中断标志
 #define SYSTIE                                   0x40                          //               rw-- SysTick中断使能
 #define FGIF                                     0x20                          //               rw0- FG中断标志
 #define DCIF                                     0x10                          //               rw0- 比较匹配中断标志
 #define FGIE                                     0x08                          //               rw-- FG中断使能
 #define DCIP                                     0x04                          //               rw-- 产生比较匹配中断的间隔
 #define DCIM1                                    0x02                          //               rw-- 比较匹配中断模式配置1
 #define DCIM0                                    0x01                          //               rw-- 比较匹配中断模式配置0

 #define DRV_CR                         *(_IO  uint8 xdata *)0x4062             // RV:  00H  Driver控制寄存器
 #define DRVEN                                    0x80                          //               rw-- 计数器使能
 #define DDIR                                     0x40                          //               rw-- 输出方向
 #define FOCEN                                    0x20                          //               rw-- FOC/SVPWM/SPWM模块使能
 #define DRPE                                     0x10                          //               rw-- DRV_DR预装载使能
 #define OCS                                      0x08                          //               rw-- 比较值来源选择
 #define MESEL                                    0x04                          //               rw-- ME核工作模式选择
 #define DRVOE                                    0x01                          //               rw-- Driver输出使能
/******************************************************************************///WDT
 #define WDT_CR                         *(_IO  uint8 xdata *)0x4026             // RV:  00H  看门狗控制寄存器
 #define WDTRF                                    0x01                          //               rw-- 看门狗初始化标志

 #define WDT_ARR                        *(_IO  uint8 xdata *)0x4027             // RV:  00H  看门狗重载计数器
/******************************************************************************///GPIO
 sfr     P0                             = 0x80;                                 // RV:  00H  Port 0
 sbit    GP00                                     = P0^0;
 sbit    GP01                                     = P0^1;
 sbit    GP02                                     = P0^2;
 sbit    GP03                                     = P0^3;
 sbit    GP04                                     = P0^4;
 sbit    GP05                                     = P0^5;
 sbit    GP06                                     = P0^6;
 sbit    GP07                                     = P0^7;

 sfr     P1                             = 0x90;                                 // RV:  00H  Port 1
 sbit    GP10                                     = P1^0;
 sbit    GP11                                     = P1^1;
 sbit    GP12                                     = P1^2;
 sbit    GP13                                     = P1^3;
 sbit    GP14                                     = P1^4;
 sbit    GP15                                     = P1^5;
 sbit    GP16                                     = P1^6;
 sbit    GP17                                     = P1^7;

 sfr     P2                             = 0xa0;                                 // RV:  00H  Port 2
 sbit    GP20                                     = P2^0;
 sbit    GP21                                     = P2^1;
 sbit    GP22                                     = P2^2;
 sbit    GP23                                     = P2^3;
 sbit    GP24                                     = P2^4;
 sbit    GP25                                     = P2^5;
 sbit    GP26                                     = P2^6;
 sbit    GP27                                     = P2^7;

 sfr     P3                             = 0xb0;                                 // RV:  00H  Port 3
 sbit    GP30                                     = P3^0;
 sbit    GP31                                     = P3^1;
 sbit    GP32                                     = P3^2;
 sbit    GP33                                     = P3^3;
 sbit    GP34                                     = P3^4;
 sbit    GP35                                     = P3^5;
 sbit    GP36                                     = P3^6;
 sbit    GP37                                     = P3^7;

 sfr     P4                             = 0xb8;                                 // RV:  00H  Port 4
 sbit    GP41                                     = P4^1;
 sbit    GP42                                     = P4^2;
 sbit    GP43                                     = P4^3;
 sbit    GP44                                     = P4^4;
 sbit    GP45                                     = P4^5;
 sbit    GP46                                     = P4^6;

 sfr     P0_OE                          = 0xfc;                                 // RV:  00H  P0输出使能寄存器
 #define P0_PU                          *(_IO  uint8 xdata *)0x4053             // RV:  00H  P0上拉使能寄存器
 #define P07                                      0x80                          //               rw-- 使能端口7
 #define P06                                      0x40                          //               rw-- 使能端口6
 #define P05                                      0x20                          //               rw-- 使能端口5
 #define P04                                      0x10                          //               rw-- 使能端口4
 #define P03                                      0x08                          //               rw-- 使能端口3
 #define P02                                      0x04                          //               rw-- 使能端口2
 #define P01                                      0x02                          //               rw-- 使能端口1
 #define P00                                      0x01                          //               rw-- 使能端口0

 sfr     P1_IE                          = 0xd1;                                 // RV:  00H  P1外部中断使能寄存器
 sfr     P1_IF                          = 0xd2;                                 // RV:  00H  P1外部中断状态寄存器
 sfr     P1_OE                          = 0xfd;                                 // RV:  00H  P1输出使能寄存器
 #define P1_PU                          *(_IO  uint8 xdata *)0x4054             // RV:  00H  P1上拉使能寄存器
 #define P17                                      0x80                          //               rw-- 使能端口7
 #define P16                                      0x40                          //               rw-- 使能端口6
 #define P15                                      0x20                          //               rw-- 使能端口5
 #define P14                                      0x10                          //               rw-- 使能端口4
 #define P13                                      0x08                          //               rw-- 使能端口3
 #define P12                                      0x04                          //               rw-- 使能端口2
 #define P11                                      0x02                          //               rw-- 使能端口1
 #define P10                                      0x01                          //               rw-- 使能端口0
 #define P11HV_EN                                 0x02                          //               rw-- P11高压PAD使能位
 #define P10HV_EN                                 0x01                          //               rw-- P10高压PAD使能位

 #define P1_AN                          *(_IO  uint8 xdata *)0x4050             // RV:  00H  P1模拟使能寄存器
 #define P17                                      0x80                          //               rw-- 使能端口7
 #define P16                                      0x40                          //               rw-- 使能端口6
 #define P15                                      0x20                          //               rw-- 使能端口5
 #define P14                                      0x10                          //               rw-- 使能端口4
 #define HBMOD                                    0x08                          //               rw-- 端口P13模式配置
 #define HDIO                                     0x04                          //               rw-- PWM输出驱动能力选择
 #define ODE1                                     0x02                          //               rw-- P01漏极开路使能
 #define ODE0                                     0x01                          //               rw-- P00漏极开路使能

 sfr     P2_OE                          = 0xfe;                                 // RV:  00H  P2输出使能寄存器
 #define P2_PU                          *(_IO  uint8 xdata *)0x4055             // RV:  00H  P2上拉使能寄存器
 #define P2_AN                          *(_IO  uint8 xdata *)0x4051             // RV:  00H  P2模拟使能寄存器
 #define P27                                      0x80                          //               rw-- 使能端口7
 #define P26                                      0x40                          //               rw-- 使能端口6
 #define P25                                      0x20                          //               rw-- 使能端口5
 #define P24                                      0x10                          //               rw-- 使能端口4
 #define P23                                      0x08                          //               rw-- 使能端口3
 #define P22                                      0x04                          //               rw-- 使能端口2
 #define P21                                      0x02                          //               rw-- 使能端口1
 #define P20                                      0x01                          //               rw-- 使能端口0

 sfr     P3_OE                          = 0xff;                                 // RV:  00H  P3输出使能寄存器
 #define P3_PU                          *(_IO  uint8 xdata *)0x4056             // RV:  00H  P3上拉使能寄存器
 #define P37                                      0x80                          //               rw-- 使能端口7
 #define P36                                      0x40                          //               rw-- 使能端口6
 #define P35                                      0x20                          //               rw-- 使能端口5
 #define P34                                      0x10                          //               rw-- 使能端口4
 #define P33                                      0x08                          //               rw-- 使能端口3
 #define P32                                      0x04                          //               rw-- 使能端口2
 #define P31                                      0x02                          //               rw-- 使能端口1
 #define P30                                      0x01                          //               rw-- 使能端口0

 #define P3_AN                          *(_IO  uint8 xdata *)0x4052             // RV:  00H  P3模拟使能寄存器
 #define P11_PL                                   0x80                          //               rw-- P11下拉使能
 #define P01_PL                                   0x40                          //               rw-- P01下拉使能
 #define P35                                      0x20                          //               rw-- 使能端口5
 #define P34                                      0x10                          //               rw-- 使能端口4
 #define P33                                      0x08                          //               rw-- 使能端口3
 #define P32                                      0x04                          //               rw-- 使能端口2
 #define P31                                      0x02                          //               rw-- 使能端口1
 #define P30                                      0x01                          //               rw-- 使能端口0

 sfr     P4_IE                          = 0xd3;                                 // RV:  00H  P4外部中断使能寄存器
 sfr     P4_IF                          = 0xd4;                                 // RV:  00H  P4外部中断状态寄存器
 sfr     P4_OE                          = 0xe9;                                 // RV:  00H  P4输出使能寄存器
 #define P4_PU                          *(_IO  uint8 xdata *)0x4057             // RV:  00H  P4上拉使能寄存器
 #define P4_AN                          *(_IO  uint8 xdata *)0x4047             // RV:  00H  P4模拟使能寄存器
 #define P46                                      0x40                          //               rw-- 使能端口6
 #define P45                                      0x20                          //               rw-- 使能端口5
 #define P44                                      0x10                          //               rw-- 使能端口4
 #define P43                                      0x08                          //               rw-- 使能端口3
 #define P42                                      0x04                          //               rw-- 使能端口2
 #define P41                                      0x02                          //               rw-- 使能端口1

 #define PIN7                                     0x80                          //               rw-- PIN7, 用于替代上面的端口编号
 #define PIN6                                     0x40                          //               rw-- PIN6, 用于替代上面的端口编号
 #define PIN5                                     0x20                          //               rw-- PIN5, 用于替代上面的端口编号
 #define PIN4                                     0x10                          //               rw-- PIN4, 用于替代上面的端口编号
 #define PIN3                                     0x08                          //               rw-- PIN3, 用于替代上面的端口编号
 #define PIN2                                     0x04                          //               rw-- PIN2, 用于替代上面的端口编号
 #define PIN1                                     0x02                          //               rw-- PIN1, 用于替代上面的端口编号
 #define PIN0                                     0x01                          //               rw-- PIN0, 用于替代上面的端口编号

 #define PH_SEL                         *(_IO  uint8 xdata *)0x404c             // RV:  00H  端口复用
 #define SPITMOD                                  0x80                          //               rw-- SPI从机发送端在发送完成后变为高阻态
 #define UART1EN                                  0x40                          //               rw-- 端口复用为UART1
 #define UART2EN                                  0x20                          //               rw-- 端口复用为UART2
 #define T4SEL                                    0x10                          //               rw-- 端口复用为TIM4
 #define T3SEL                                    0x08                          //               rw-- 端口复用为TIM3
 #define T2SEL                                    0x04                          //               rw-- 端口复用为TIM2
 #define T2SSEL                                   0x02                          //               rw-- 端口复用为TIM2S
 #define XOE                                      0x01                          //               rw-- 端口复用为X相驱动端

 #define PH_SEL1                        *(_IO  uint8 xdata *)0x404d             // RV:  00H  端口复用
 #define UART2CH1                                 0x80                          //               rw-- UART2功能转移使能（P36-->P01 P37-->P00）
 #define UART2CH0                                 0x40                          //               rw-- UART2功能转移使能（P37-->P12）
 #define CMPXO_P34                                0x20                          //               rw-- P01比较器输出功能转移-->P34
 #define SPICT0                                   0x10                          //               rw-- SPI单线调试功能转移(P05-->P00)
 #define T4CT1                                    0x08                          //               rw-- TIM4功能转移
 #define T4CT0                                    0x04                          //               rw-- TIM4功能转移
 #define T3CT0                                    0x01                          //               rw-- TIM3功能转移

 #define PH_SEL2                        *(_IO  uint8 xdata *)0x4049             // RV:  00H  端口复用
 #define ADC_SCYCH7                               0x20                          //               rw-- ADC_SCYCH[7:4] AD2&AD15 通道采样时间设置
 #define ADC_SCYCH6                               0x10                          //               rw--
 #define ADC_SCYCH5                               0x08                          //               rw--
 #define ADC_SCYCH4                               0x04                          //               rw--
 #define I2C_FS                                   0x02                          //               rw-- I2C功能转移
 #define SPICT1                                   0x01                          //               rw-- SPI单线调试功能转移(P05-->P34)
/******************************************************************************///ADC
 #define ADC_MASK                       *(_IO uint16 xdata *)0x4036             // RV:0000H  ADC通道使能
 #define CH15EN                                   0x8000                        //               rw-- ADC通道15使能
 #define CH14EN                                   0x4000                        //               rw-- ADC通道14使能
 #define CH13EN                                   0x2000                        //               rw-- ADC通道13使能
 #define CH12EN                                   0x1000                        //               rw-- ADC通道12使能
 #define CH11EN                                   0x0800                        //               rw-- ADC通道11使能
 #define CH10EN                                   0x0400                        //               rw-- ADC通道10使能
 #define CH9EN                                    0x0200                        //               rw-- ADC通道9使能
 #define CH8EN                                    0x0100                        //               rw-- ADC通道8使能
 #define CH7EN                                    0x0080                        //               rw-- ADC通道7使能
 #define CH6EN                                    0x0040                        //               rw-- ADC通道6使能
 #define CH5EN                                    0x0020                        //               rw-- ADC通道5使能
 #define CH4EN                                    0x0010                        //               rw-- ADC通道4使能
 #define CH3EN                                    0x0008                        //               rw-- ADC通道3使能
 #define CH2EN                                    0x0004                        //               rw-- ADC通道2使能
 #define CH1EN                                    0x0002                        //               rw-- ADC通道1使能
 #define CH0EN                                    0x0001                        //               rw-- ADC通道0使能

 #define ADC_SCYC                       *(_IO  uint8 xdata *)0x4038             // RV:  33H  ADC通道采样时间配置

 #define ADC_CR                         *(_IO  uint8 xdata *)0x4039             // RV:  00H  ADC控制寄存器
 #define ADCEN                                    0x80                          //               rw-- 使能ADC
 #define ADCBSY                                   0x40                          //               rw1- ADC启动 & ADC忙标志
 #define ADCRATIO                                 0x20                          //               rw-- AD15采VCC电压使用的分压比
 #define ADCTM1                                   0x10                          //               rw-- DRV触发ADC启动模式
 #define ADCTM0                                   0x08                          //               rw-- 00不触发01上升沿10下降沿11双沿
 #define ADCALIGN                                 0x04                          //               rw-- ADC数据次高位对齐
 #define ADCIE                                    0x02                          //               rw-- ADC中断使能
 #define ADCIF                                    0x01                          //               rw-- ADC中断标志

 #define ADC0_DR                        *(_I  uint16 xdata *)0x0fd8             // RV:0000H  ADC通道0转换结果
 #define ADC1_DR                        *(_I  uint16 xdata *)0x0fda             // RV:0000H  ADC通道1转换结果
 #define ADC2_DR                        *(_I  uint16 xdata *)0x0fdc             // RV:0000H  ADC通道2转换结果
 #define ADC3_DR                        *(_I  uint16 xdata *)0x0fde             // RV:0000H  ADC通道3转换结果
 #define ADC4_DR                        *(_I  uint16 xdata *)0x0fe0             // RV:0000H  ADC通道4转换结果
 #define ADC5_DR                        *(_I  uint16 xdata *)0x0fe2             // RV:0000H  ADC通道5转换结果
 #define ADC6_DR                        *(_I  uint16 xdata *)0x0fe4             // RV:0000H  ADC通道6转换结果
 #define ADC7_DR                        *(_I  uint16 xdata *)0x0fe6             // RV:0000H  ADC通道7转换结果
 #define ADC8_DR                        *(_I  uint16 xdata *)0x0fe8             // RV:0000H  ADC通道8转换结果
 #define ADC9_DR                        *(_I  uint16 xdata *)0x0fea             // RV:0000H  ADC通道9转换结果
 #define ADC10_DR                       *(_I  uint16 xdata *)0x0fec             // RV:0000H  ADC通道10转换结果
 #define ADC11_DR                       *(_I  uint16 xdata *)0x0fee             // RV:0000H  ADC通道11转换结果
 #define ADC12_DR                       *(_I  uint16 xdata *)0x0ff0             // RV:0000H  ADC通道12转换结果
 #define ADC13_DR                       *(_I  uint16 xdata *)0x0ff2             // RV:0000H  ADC通道13转换结果
 #define ADC14_DR                       *(_I  uint16 xdata *)0x0ff4             // RV:0000H  ADC通道14转换结果
 #define ADC15_DR                       *(_I  uint16 xdata *)0x0ff6             // RV:0000H  ADC通道15转换结果
/*****************************************************************************///DAC
 #define DAC_CR                         *(_IO  uint8 xdata *)0x4035             // RV:  0CH  DAC控制寄存器
 #define DAC0_1EN                                 0x80                          //               rw-- DAC0&1使能
 #define DACMOD                                   0x40                          //               rw-- DAC模式配置
 #define ADC_SCYCH3                               0x20                          // ADC_SCYCH[3:0]
 #define ADC_SCYCH2                               0x10                          // CH8/9/10/11/12/13采样时间配置
 #define ADC_SCYCH1                               0x08                          //
 #define ADC_SCYCH0                               0x04                          //
 //#define DAC2EN                                   0x02                          //               rw-- DAC2使能

 #define DAC0_DR                        *(_IO  uint8 xdata *)0x404b             // RV:  00H  DAC0输出值[8:1]
 #define DAC1_DR                        *(_IO  uint8 xdata *)0x404a             // RV:  00H  DAC1输出值
 #define DAC0_DR_0                                0x80                          // RV:  00H  DAC0输出值[0]
 //#define DAC2_DR                        *(_IO  uint8 xdata *)0x4049             // RV:  00H  DAC2输出值

/******************************************************************************///DMA
 #define DMA0_CR0                       *(_IO  uint8 xdata *)0x403a             // RV:  00H  DMA通道0配置 & DMA公共配置
 #define DMA1_CR0                       *(_IO  uint8 xdata *)0x403b             // RV:  00H  DMA通道1配置 & Debug设置
 #define DMAEN                                    0x80                          //               rw-- DMA通道0/1使能
 #define DMABSY                                   0x40                          //               rw1- DMA通道0/1状态/启动
 #define DMACFG2                                  0x20                          //               rw-- DMA通道0/1外设与方向选择位2
 #define DMACFG1                                  0x10                          //               rw-- DMA通道0/1外设与方向选择位1
 #define DMACFG0                                  0x08                          //               rw-- DMA通道0/1外设与方向选择位0
 #define DBGSW                                    0x04                          //               rw-- DMA通道1  DBG模式指向区域
 #define DBGEN                                    0x02                          //               rw-- DMA通道1  DBG模式使能
 #define DMAIE                                    0x04                          //               rw-- DMA中断使能,位于DMA_CR0
 #define ENDIAN                                   0x02                          //               rw-- DMA数据传输顺序,位于DMA_CR0
 #define DMAIF                                    0x01                          //               rw0- DMA通道0/1中断标志位

 #define DMA0_LEN                       *(_IO  uint8 xdata *)0x403c             // RV:  00H  DMA通道0传输长度配置
 #define DMA1_LEN                       *(_IO  uint8 xdata *)0x403d             // RV:  00H  DMA通道1传输长度配置

 #define DMA0_BA                        *(_IO uint16 xdata *)0x403e             // RV:0000H  DMA通道0传输地址配置
 #define DMA1_BA                        *(_IO uint16 xdata *)0x4040             // RV:0000H  DMA通道1传输地址配置
/******************************************************************************///VREF & VHALF
 #define VREF_VHALF_CR                  *(_IO  uint8 xdata *)0x404f             // RV:  06H  VREF & VHALF控制寄存器
 #define VRVSEL1                                  0x80                          //               rw-- VREF电压配置1
 #define VRVSEL0                                  0x40                          //               rw-- VREF电压配置0
 #define VREFEN                                   0x10                          //               rw-- VREF使能
 #define VHALFSEL1                                0x04                          //               rw-- VHALF电压选择VREF系数
 #define VHALFSEL0                                0x02                          //               rw-- 00:1/8 01:1/4 10:25/64 11:1/2(default)
 #define VHALFEN                                  0x01                          //               rw-- VHALF使能
/******************************************************************************///AMP
 #define AMP_CR0                        *(_IO  uint8 xdata *)0x404e             // RV:  00H  运放控制寄存器0
 #define CP_SEL                                   0x80                          //               rw-- 使能后P27通过50k电阻到P33,需CP_EN=1
 #define CP_EN                                    0x40                          //               rw-- 使能后P27通过50k电阻到P34
 #define AMP12M_GND                               0x20                          //               rw-- PGA模式运放12负输入端接GND
 #define AMP0M_GND                                0x10                          //               rw-- PGA模式运放0负输入端接GND
 #define AMP3EN                                   0x08                          //               rw-- 运放3使能
 #define AMP2EN                                   0x04                          //               rw-- 运放2使能
 #define AMP1EN                                   0x02                          //               rw-- 运放1使能
 #define AMP0EN                                   0x01                          //               rw-- 运放0使能

 #define AMP_CR1                        *(_IO  uint8 xdata *)0x4034             // RV:  00H  运放控制寄存器1
 #define AMP_PH_GAIN2                             0x80                          //               rw-- AMP1/2 PGA增益控制位2
 #define AMP_PH_GAIN1                             0x40                          //               rw-- AMP1/2 PGA增益控制位1
 #define AMP_PH_GAIN0                             0x20                          //               rw-- AMP1/2 PGA增益控制位0
 #define AMP0_ISCALE1                             0x10                          //               rw-- 保留
 #define AMP0_ISCALE0                             0x08                          //               rw-- 保留
 #define AMP0_GAIN2                               0x04                          //               rw-- AMP0 PGA增益控制位2
 #define AMP0_GAIN1                               0x02                          //               rw-- AMP0 PGA增益控制位1
 #define AMP0_GAIN0                               0x01                          //               rw-- AMP0 PGA增益控制位0

 #define AMP_CR2                        *(_IO  uint8 xdata *)0x4046             // RV:  00H  运放控制寄存器2
 #define AMP3_GAIN2                               0x04                          //               rw-- AMP3 PGA增益控制位2
 #define AMP3_GAIN1                               0x02                          //               rw-- AMP3 PGA增益控制位1
 #define AMP3_GAIN0                               0x01                          //               rw-- AMP3 PGA增益控制位0
/******************************************************************************///CMP
 sfr     CMP_CR0                        = 0xd5;                                 // RV:  00H  比较器控制寄存器0
 #define CMP3IM1                                  0x80                          //               rw-- CMP3中断模式配置1
 #define CMP3IM0                                  0x40                          //               rw-- CMP3中断模式配置0
 #define CMP2IM1                                  0x20                          //               rw-- CMP2中断模式配置1
 #define CMP2IM0                                  0x10                          //               rw-- CMP2中断模式配置0
 #define CMP1IM1                                  0x08                          //               rw-- CMP1中断模式配置1
 #define CMP1IM0                                  0x04                          //               rw-- CMP1中断模式配置0
 #define CMP0IM1                                  0x02                          //               rw-- CMP0中断模式配置1
 #define CMP0IM0                                  0x01                          //               rw-- CMP0中断模式配置0

 sfr     CMP_CR1                        = 0xd6;                                 // RV:  00H  比较器控制寄存器1
 #define CMP3MOD1                                 0x40                          //               rw-- CMP3正输入端选择配置1
 #define CMP3MOD0                                 0x20                          //               rw-- CMP3正输入端选择配置0
 #define CMP3EN                                   0x10                          //               rw-- CMP3使能
 #define CMP3HYS                                  0x08                          //               rw-- CMP3迟滞使能
 #define CMP0HYS1                                 0x02                          //               rw-- CMP0迟滞电压配置1
 #define CMP0HYS0                                 0x01                          //               rw-- CMP0迟滞电压配置0

 sfr     CMP_CR2                        = 0xda;                                 // RV:  00H  比较器控制寄存器2
 #define CMP4EN                                   0x80                          //               rw-- CMP4使能
 #define CMP0MOD1                                 0x40                          //               rw-- CMP0模式配置1
 #define CMP0MOD0                                 0x20                          //               rw-- CMP0模式配置0
 #define CMP0SEL1                                 0x10                          //               rw-- CMP0端口组合配置1
 #define CMP0SEL0                                 0x08                          //               rw-- CMP0端口组合配置0
 #define CMP0CSEL1                                0x04                          //               rw-- CMP0轮询速度配置1
 #define CMP0CSEL0                                0x02                          //               rw-- CMP0轮询速度配置0
 #define CMP0EN                                   0x01                          //               rw-- CMP0使能

 sfr     CMP_CR3                        = 0xdc;                                 // RV:  00H  比较器控制寄存器3
 #define CMPDTEN                                  0x80                          //               rw-- 比较器死区采样使能
 #define DBGSEL1                                  0x40                          //               rw-- DBG信号配置1
 #define DBGSEL0                                  0x20                          //               rw-- DBG信号配置0
 #define SAMSEL1                                  0x10                          //               rw-- CMP0/1/2 & ADC 采样时机配置1
 #define SAMSEL0                                  0x08                          //               rw-- CMP0/1/2 & ADC 采样时机配置0
 #define CMPSEL2                                  0x04                          //               rw-- 比较器输出选择配置2
 #define CMPSEL1                                  0x02                          //               rw-- 比较器输出选择配置1
 #define CMPSEL0                                  0x01                          //               rw-- 比较器输出选择配置0

 sfr     CMP_CR4                        = 0xe1;                                 // RV:  00H  比较器控制寄存器4
 #define CMP4OUT                                  0x80                          //               r--- CMP4比较结果
 #define CMP3P4M_FS                               0x10                          //               rw-- P27(CMP3P和CMP4M)功能转移至P34
 #define FAEN                                     0x04                          //               rw-- 滤波采样系数扩大使能
 #define CMP0_FS                                  0x02                          //               rw-- CMP1/2功能转移使能

 sfr     HALL_CR                        = 0xe2;                                 // RV:  00H  HALL控制寄存器
 #define HALL_IF                                  0x80                          //               rw-- HALL中断触发标志位
 #define HALL_IE                                  0x40                          //               rw-- HALL中断使能
 #define HALLSEL                                  0x08                          //               rw-- 霍尔信号输入选择
 #define HALL2                                    0x04                          //               rw-- HALL2滤波后输出电平
 #define HALL1                                    0x02                          //               rw-- HALL1滤波后输出电平
 #define HALL0                                    0x01                          //               rw-- HALL0滤波后输出电平

 sfr     CMP_SR                         = 0xd7;                                 // RV:  00H  比较器状态寄存器
 #define CMP3IF                                   0x80                          //               rw0- CMP3中断标志
 #define CMP2IF                                   0x40                          //               rw0- CMP2中断标志
 #define CMP1IF                                   0x20                          //               rw0- CMP1中断标志
 #define CMP0IF                                   0x10                          //               rw0- CMP0中断标志
 #define CMP3OUT                                  0x08                          //               r--- CMP3比较结果
 #define CMP2OUT                                  0x04                          //               r--- CMP2比较结果
 #define CMP1OUT                                  0x02                          //               r--- CMP1比较结果
 #define CMP0OUT                                  0x01                          //               r--- CMP0比较结果

 sfr     EVT_FILT                       = 0xd9;                                 // RV:  00H  驱动保护功能
 #define MOEMD1                                   0x10                          //               rw-- MOE自动控制配置1
 #define MOEMD0                                   0x08                          //               rw-- MOE自动控制配置0
 #define INT0_MOE_EN                              0x04                          //               rw-- INT0中断触发MOE关闭使能
 #define EFDIV1                                   0x02                          //               rw-- 母线电流保护滤波配置1
 #define EFDIV0                                   0x01                          //               rw-- 母线电流保护滤波配置0

 #define CMP_SAMR                       *(_IO  uint8 xdata *)0x40ad             // RV:  00H  比较器采样配置
/******************************************************************************///FLASH
 sfr     FLA_KEY                        = 0x84;                                 // RV:  00H  FLASH解锁寄存器
 #define FLAKSTA1                                 0x02                          //               rw-- FLASH解锁状态位1
 #define FLAKSTA0                                 0x01                          //               rw-- FLASH解锁状态位0

 sfr     FLA_CR                         = 0x85;                                 // RV:  00H  FLASH控制寄存器
 #define PAGE_EN                                  0x20                          //               rw-- FLASH页操作使能
 #define FLAERR                                   0x10                          //               r--- FLASH错误标志位
 #define FLAPRE                                   0x04                          //               rw-- FLASH预编程使能
 #define FLAERS                                   0x02                          //               rw-- FLASH清除使能
 #define FLAEN                                    0x01                          //               rw-- FLASH编程使能
/******************************************************************************///CRC
 #define CRC_DIN                        *(_O   uint8 xdata *)0x4021             // RV:  00H  CRC数据输入寄存器

 #define CRC_CR                         *(_IO  uint8 xdata *)0x4022             // RV:  11H  CRC控制寄存器
 #define CRCDONE                                  0x10                          //               r1-- 自动CRC完成标志
 #define CRCDINI                                  0x08                          //               r0w1 CRC初始化
 #define CRCVAL                                   0x04                          //               rw-- CRC初始化的值
 #define AUTOINT                                  0x02                          //               rw-- CRC自动计算使能
 #define CRCPNT                                   0x01                          //               rw-- CRC结果访问位置

 #define CRC_DR                         *(_IO  uint8 xdata *)0x4023             // RV:  6bH  CRC结果输出寄存器
 #define CRC_BEG                        *(_IO  uint8 xdata *)0x4024             // RV:  00H  CRC自动计算的起始位置
 #define CRC_CNT                        *(_IO  uint8 xdata *)0x4025             // RV:  00H  CRC块数计数器
/******************************************************************************///TSD
 #define TSD_CR                         *(_IO  uint8 xdata *)0x402f             // RV:  0DH  TSD控制寄存器
 #define TSDEN                                    0x80                          //               rw-- TSD使能
 #define TSEN_EN                                  0x40                          //               rw-- 温度传感器使能位
 #define TSEN_HYS                                 0x20                          //               rw-- 温度解码调整步进值：0-->1℃； 1-->2℃
 #define TSDADJ3                                  0x08                          //               rw-- TSD温度调节
 #define TSDADJ2                                  0x04                          //               rw-- TSD温度调节
 #define TSDADJ1                                  0x02                          //               rw-- TSD温度调节
 #define TSDADJ0                                  0x01                          //               rw-- TSD温度调节

 #define TSEN_DR                        *(_IO  uint8 xdata *)0x4048             // RV:  00H  TSENSOR数据寄存器
/******************************************************************************///POWER
 sfr     PCON                           = 0x87;                                 // RV:  00H  MCU功率控制寄存器
 #define GF3                                      0x20                          //               rw-- 通用标志3
 #define GF2                                      0x10                          //               rw-- 通用标志2
 #define GF1                                      0x08                          //               rw-- 通用标志1
 #define STOP                                     0x02                          //               rw-- MCU睡眠使能
 #define IDLE                                     0x01                          //               rw-- MCU待机使能

 sfr     LVSR                           = 0xdb;                                 // RV:  00H  状态寄存器
 #define EXT0CFG2                                 0x20                          //               rw-- 外部中断0端口配置2
 #define EXT0CFG1                                 0x10                          //               rw-- 外部中断0端口配置1
 #define EXT0CFG0                                 0x08                          //               rw-- 外部中断0端口配置0
 #define TSDF                                     0x04                          //               rw-- 过温标志位
 #define LVWF                                     0x02                          //               rw-- VCC低电压预警标志
 #define LVWIF                                    0x01                          //               rw-- VCC低电压预警中断标志

 #define CCFG1                          *(_O   uint8 xdata *)0x401E             // RV:  00H
 #define LVW_EN_B                                 0x80                          //               rw-- VCC低电压预警使能，复位使能
 #define LVW_IE                                   0x40                          //               rw-- VCC低电压预警中断使能
 #define WDT_EN                                   0x20                          //               rw-- WDT使能
 #define VBB_DIS                                  0x10                          //               rw-- VBB LDO关闭
 
 #define CCFG2                          *(_O   uint8 xdata *)0x401D             // RV:  00H
 #define VBB_VSEL                                 0x10                          //               rw-- 0：VBB-10V   1：VBB-12V
 
#endif


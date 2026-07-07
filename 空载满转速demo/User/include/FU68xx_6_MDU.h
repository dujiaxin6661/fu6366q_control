/**
 * @copyright (C) COPYRIGHT 2022 Fortiortech Shenzhen
 * @file      main.c
 * @author    Fortiortech  Appliction Team
 * @since     Create:2023-07
 * @date      Last modify:2023-07
 * @note      Last modify author is Kobe Peng
 * @brief      
 */

#ifndef __FU68XX_6_SMDU_H__
#define __FU68XX_6_SMDU_H__

/******************************************************************************///Including Header Files
#include <FU68xx_6_MCU.h>
/******************************************************************************///Define Macro
/******************************************************************************///Define Type
/**
 * @brief SMDU的模式类型
 *
 * @note 使用@ref SMDU_RunNoBlock时, 其中的mode参数可以直接使用本枚举的内容
 * @note 使用@ref SMDU_RunBlock时, 其中的mode参数可以直接使用本枚举的内容
 */
typedef enum
{
    S1MUL   = 0, /**< 有符号乘法, 计算结果左移1位 ,最小时钟9*/
    SMUL    = 1, /**< 有符号乘法 ,最小时钟9*/
    UMUL    = 2, /**< 无符号乘法 ,最小时钟9*/
    DIV     = 3, /**< 32/16无符号除法 ,最小时钟31*/
    SIN_COS = 4, /**< Sin/Cos ,最小时钟30*/
    ATAN    = 5, /**< ATan ,最小时钟28*/
    LPF     = 6, /**< 低通滤波 ,最小时钟15*/
    PI      = 7  /**< PI ,最小时钟28*/
} ETypeSMDUMode;
/******************************************************************************///External Symbols
/******************************************************************************///External Function

//#define SMDU_Init()                 SetBit(TIM234_CTRL,MDU_EN_N);
                                      
/**
 * @brief 运行SMDU且不等待运行结束
 *
 * @param  stan (0-3) 要启动的计算单元编号
 * @param  mode (0-7) 指定计算单元的模式, 可使用@ref ETypeSMDUMode 作为计算模式的设置\n
 * @ref S1MUL   有符号乘法, 计算结果左移1位 \n
 * @ref SMUL    有符号乘法 \n
 * @ref UMUL    无符号乘法 \n
 * @ref DIV     32/16无符号除法 \n
 * @ref SIN_COS Sin/Cos \n
 * @ref ATAN    ATan \n
 * @ref LPF     低通滤波 \n
 * @ref PI      PI \n
 */
#define SMDU_RunNoBlock(stan, mode)   do                                                  \
                                      {                                                   \
                                          MDU_CR = MDUSTA0 << stan | (unsigned char)mode; \
                                      } while (0)

/**
 * @brief 运行SMDU且等待运行结束
 *
 * @param  stan (0-3) 要启动的计算单元编号
 * @param  mode (0-7) 指定计算单元的模式, 可使用@ref ETypeSMDUMode 作为计算模式的设置\n
 * @ref S1MUL   有符号乘法, 计算结果左移1位 \n
 * @ref SMUL    有符号乘法 \n
 * @ref UMUL    无符号乘法 \n
 * @ref DIV     32/16无符号除法 \n
 * @ref SIN_COS Sin/Cos \n
 * @ref ATAN    ATan \n
 * @ref LPF     低通滤波 \n
 * @ref PI      PI \n
 */
#define SMDU_RunBlock(stan, mode)   do                                       \
                                    {                                        \
                                        SMDU_RunNoBlock(stan, mode);         \
                                        while (MDU_CR & MDUBSY);             \
                                    } while (0);

/**
 * 无符号乘法
 *
 * @CodeLen 0x0033(Xdata) 0x0029(Idata) 0x0021(Data)
 * @RunTime 2.88us(Xdata) 1.96us(Idata) 1.46us(Data)
 * @Exp     C = A * B
 *
 * @param   wA   (uint16)  被乘数
 * @param   wB   (uint16)  乘数
 * @param   wCh  (uint16)  积的高16位
 * @param   wCl  (uint16)  积的低16位
 */
#define Muilt_MDU(wA, wB, wCh, wCl)                         do { \
                                                                MUL0_MA  = wA;\
                                                                MUL0_MB  = wB;\
                                                                SMDU_RunBlock(0, UMUL);\
                                                                wCh = MUL0_MCH;\
                                                                wCl = MUL0_MCL;\
                                                                } while (0)

/**
 * 无符号乘法,只取高位
 *
 * @CodeLen 0x002a(Xdata) 0x0024(Idata) 0x001c(Data)
 * @RunTime 2.30us(Xdata) 1.71us(Idata) 1.21us(Data)
 * @Exp     C = A * B
 *
 * @param   wA   (uint16)  被乘数
 * @param   wB   (uint16)  乘数
 * @param   wCh  (uint16)  积的高16位
 */
#define Muilt_H_MDU(wA, wB, wCh)                            do { \
                                                                MUL0_MA  = wA;\
                                                                MUL0_MB  = wB;\
                                                                SMDU_RunBlock(0, UMUL);\
                                                                wCh = MUL0_MCH;\
                                                                } while (0)
                                    
                                    
/**
 * 无符号乘法,只取低位
 *
 * @CodeLen 0x002a(Xdata) 0x0024(Idata) 0x001c(Data)
 * @RunTime 2.30us(Xdata) 1.71us(Idata) 1.21us(Data)
 * @Exp     C = A * B
 *
 * @param   wA   (uint16)  被乘数
 * @param   wB   (uint16)  乘数
 * @param   wCl  (uint16)  积的低16位
 */
#define Muilt_L_MDU(wA, wB, wCl)                            do { \
                                                                MUL0_MA  = wA;\
                                                                MUL0_MB  = wB;\
                                                                SMDU_RunBlock(0, UMUL);\
                                                                wCl = MUL0_MCL;\
                                                                } while (0)

/**
 * 有符号乘法，结果左移一位,只取高位
 *
 * @CodeLen 0x002a(Xdata) 0x0024(Idata) 0x001c(Data)
 * @RunTime 2.25us(Xdata) 1.71us(Idata) 1.21us(Data)
 * @Exp     C = A * B * 2
 *
 * @param   iA   (int16)  被乘数
 * @param   iB   (int16)  乘数
 * @param   iCh  (int16)  积的高16位
 */
#define MuiltS1_H_MDU(iA, iB, iCh)                          do { \
                                                                MUL0_MA  = iA;\
                                                                MUL0_MB  = iB;\
                                                                SMDU_RunBlock(0, S1MUL);\
                                                                iCh = MUL0_MCH;\
                                                            } while (0)

/**
 * 有符号乘法
 *
 * @CodeLen 0x0033(Xdata) 0x0029(Idata) 0x0021(Data)
 * @RunTime 2.88us(Xdata) 1.96us(Idata) 1.46us(Data)
 * @Exp     C = A * B
 *
 * @param   iA   (int16)  被乘数
 * @param   iB   (int16)  乘数
 * @param   iCh  (int16)  积的高16位
 * @param   iCl  (int16)  积的低16位
 */
#define MuiltS_MDU0(iA, iB, iCh, iCl)                        do { \
                                                                MUL0_MA  = iA;\
                                                                MUL0_MB  = iB;\
                                                                SMDU_RunBlock(0, SMUL);\
                                                                iCh = MUL0_MCH;\
                                                                iCl = MUL0_MCL;\
                                                            } while (0)

/**
 * 有符号乘法,只取高位
 *
 * @CodeLen 0x002a(Xdata) 0x0024(Idata) 0x001c(Data)
 * @RunTime 2.30us(Xdata) 1.71us(Idata) 1.21us(Data)
 * @Exp     C = A * B
 *
 * @param   iA   (int16)  被乘数
 * @param   iB   (int16)  乘数
 * @param   iCh  (int16)  积的高16位
 */
#define MuiltS_H_MDU(iA, iB, iCh)                           do { \
                                                                MUL0_MA  = iA;\
                                                                MUL0_MB  = iB;\
                                                                SMDU_RunBlock(0, SMUL);\
                                                                iCh = MUL0_MCH;\
                                                            } while (0)

/**
 * 32Bit/16Bit除法----求商
 *
 * @CodeLen 0x0043(Xdata) 0x003a(Idata) 0x002c(Data)
 * @RunTime 4.46us(Xdata) 3.50us(Idata) 2.67us(Data)
 * @Exp     C = A / B
 *
 * @param   wAh  (uint16)  被除数的高16位
 * @param   wAl  (uint16)  被除数的低16位
 * @param   wB   (uint16)  除数
 * @param   wCh  (uint16)  商的高16位
 * @param   wCl  (uint16)  商的低16位
 */
#define DivQ_MDU(wAh, wAl, wB, wCh, wCl)                    do {\
                                                                DIV0_DAH  = wAh;\
                                                                DIV0_DAL  = wAl;\
                                                                DIV0_DB   = wB;\
                                                                SMDU_RunBlock(0, DIV);\
                                                                wCh    = DIV0_DQH;\
                                                                wCl    = DIV0_DQL;\
                                                            } while (0)

/**
 * 32Bit/16Bit除法----求商,只取低位
 *
 * @CodeLen 0(Xdata) 0(Idata) 0(Data)
 * @RunTime 0(Xdata) 0(Idata) 0(Data)
 * @Exp     C = A / B
 *
 * @param   wAh  (uint16)  被除数的高16位
 * @param   wAl  (uint16)  被除数的低16位
 * @param   wB   (uint16)  除数
 * @param   wCl  (uint16)  商的低16位
 */
#define DivQ_L_MDU(wAh, wAl, wB, wCl)                       do {\
                                                                DIV0_DAH  = wAh;\
                                                                DIV0_DAL  = wAl;\
                                                                DIV0_DB   = wB;\
                                                                SMDU_RunBlock(0, DIV);\
                                                                wCl    = DIV0_DQL;\
                                                              } while (0)

/**
 * 低通滤波器
 *
 * @CodeLen 0x0049(Xdata) 0x003d(Idata) 0x002f(Data)
 * @RunTime 4.21us(Xdata) 2.88us(Idata) 2.29us(Data)
 * @Exp     Y += K * (X - Y)
 *
 * @param   iYh  (int16)  滤波输出的高16位(填入上一次计算的输出，取出新的输出)
 * @param   iYl  (int16)  滤波输出的低16位(填入上一次计算的输出，取出新的输出)
 * @param   iX   (int16)  滤波输入
 * @param   ucK  (uint8)  滤波系数(Q8格式：0~255)
 */
#define LPF_MDU2(iX, ucK, iYh, iYl)                          do { \
                                                                LPF2_K = ucK;\
                                                                LPF2_X = iX;\
                                                                LPF2_YH = iYh;\
                                                                LPF2_YL = iYl;\
                                                                SMDU_RunBlock(2, LPF);\
                                                                iYh    = LPF2_YH;\
                                                                iYl    = LPF2_YL;\
                                                            } while (0)

#define LPF_MDU1(iX, ucK, iYh, iYl)                          do { \
                                                                LPF1_K = ucK;\
                                                                LPF1_X = iX;\
                                                                LPF1_YH = iYh;\
                                                                LPF1_YL = iYl;\
                                                                SMDU_RunBlock(1, LPF);\
                                                                iYh    = LPF1_YH;\
                                                                iYl    = LPF1_YL;\
                                                            } while (0)
/**
 * Sin/Cos计算与坐标转换
 *
 * @CodeLen 0x0043(Xdata) 0x003a(Idata) 0x002c(Data)
 * @RunTime 4.46us(Xdata) 3.38us(Idata) 2.67us(Data)
 * @Exp     Sin = Cos0 * Sin(Theta) + Sin0 * Cos(Theta)
 *          Cos = Cos0 * Cos(Theta) - Sin0 * Sin(Theta)
 *
 * @param   iCos0   (int16)  初始Cos值
 * @param   iTheta  (int16)  角度
 * @param   iSin0   (int16)  初始Sin值
 * @param   iCos    (int16)  Cos计算结果
 * @param   iSin    (int16)  Sin计算结果
 */
#define SinCos_MDU(iCos0, iTheta, iSin0, iCos, iSin)        do {\
                                                                SCAT0_COS  = iCos0;\
                                                                SCAT0_SIN  = iSin0;\
                                                                SCAT0_THE  = iTheta;\
                                                                SMDU_RunBlock(0,SIN_COS);\
                                                                iCos   = SCAT0_RES1;\
                                                                iSin   = SCAT0_RES2;\
                                                            } while (0)

/**
 * 角度计算
 *
 * @CodeLen 0(Xdata) 0(Idata) 0(Data)
 * @RunTime 0(Xdata) 0(Idata) 0(Data)
 * @Exp     Theta = Atan(Sin / Cos)
 *
 * @param   iCos    (int16)  输入的Cos值
 * @param   iSin    (int16)  输入的Sin值
 * @param   iTheta  (int16)  角度计算结果
 */
#define Atan_Theta_MDU2(iCos, iSin, iTheta)                  do {\
                                                                SCAT2_COS  = iCos;\
                                                                SCAT2_SIN  = iSin;\
                                                                SMDU_RunBlock(2, ATAN);\
                                                                iTheta    = SCAT2_RES2;\
                                                            } while (0)
#define Atan_Theta_MDU1(iCos, iSin, iTheta)                  do {\
                                                                SCAT1_COS  = iCos;\
                                                                SCAT1_SIN  = iSin;\
                                                                SMDU_RunBlock(1, ATAN);\
                                                                iTheta    = SCAT1_RES2;\
                                                            } while (0)

/**
 * 幅值计算
 *
 * @CodeLen 0(Xdata) 0(Idata) (Data)
 * @RunTime 0(Xdata) 0(Idata) (Data)
 * @Exp     Us    = Sqrt(Sin^2 + Cos^2)
 *
 * @param   iCos    (int16)  输入的Cos值
 * @param   iSin    (int16)  输入的Sin值
 * @param   iUs     (int16)  幅值计算结果
 */
#define Atan_Us_MDU(iCos, iSin, iUs)                        do {\
                                                                SCAT0_COS  = iCos;\
                                                                SCAT0_SIN  = iSin;\
                                                                SMDU_RunBlock(0, ATAN);\
                                                                iUs    = SCAT0_RES1;\
                                                            } while (0)


#define Atan_Us_MDUPR3(iCos, iSin, iUs)                        do {\
                                                                SCAT3_COS  = iCos;\
                                                                SCAT3_SIN  = iSin;\
                                                                SMDU_RunBlock(3, ATAN);\
                                                                iUs    = SCAT3_RES1;\
                                                            } while (0)

/**
 * 幅值低通滤波器
 *
 * @CodeLen 0x0039(Xdata) 0x0033(Idata) 0x0026(Data)
 * @RunTime 6.00us(Xdata) 4.71us(Idata) 3.63us(Data)
 * @Exp     Y += K * (Sqrt(Sin^2 + Cos^2) - Y)
 *
 * @param   iCos  (int16)  输入的Cos值
 * @param   iSin  (int16)  输入的Sin值
 * @param   ucK   (uint8)  滤波系数(Q8格式：0~255)
 * @param   iYh   (int16)  滤波输出的高16位(填入上一次计算的输出，取出新的输出)
 * @param   iYl   (int16)  滤波输出的低16位(填入上一次计算的输出，取出新的输出)
 */
#define Atan_LPF_MDU(iCos, iSin, ucK, iYh, iYl)             do {\
                                                                SCAT0_COS  = iCos;\
                                                                SCAT0_SIN  = iSin;\
                                                                SMDU_RunBlock(0, ATAN);\
                                                                LPF0_K = ucK;\
                                                                LPF0_X = SCAT0_RES1;\
                                                                LPF0_YH = iYh;\
                                                                LPF0_YL = iYl;\
                                                                SMDU_RunBlock(0, LPF);\
                                                                iYh    = LPF0_YH;\
                                                                iYl    = LPF0_YL;\
                                                            } while (0)


/**
 * 有符号乘法
 *
 * @CodeLen 0x0033(Xdata) 0x0029(Idata) 0x0021(Data)
 * @RunTime 2.88us(Xdata) 1.96us(Idata) 1.46us(Data)
 * @Exp     C = A * B
 *
 * @param   iA   (int16)  被乘数
 * @param   iB   (int16)  乘数
 * @param   iCh  (int16)  积的高16位
 * @param   iCl  (int16)  积的低16位
 */
#define MuiltS_MDU0(iA, iB, iCh, iCl)                        do { \
                                                                MUL0_MA  = iA;\
                                                                MUL0_MB  = iB;\
                                                                SMDU_RunBlock(0, SMUL);\
                                                                iCh = MUL0_MCH;\
                                                                iCl = MUL0_MCL;\
                                                            } while (0)

/**
 * 无符号乘法,只取高位
 *
 * @CodeLen 0x002a(Xdata) 0x0024(Idata) 0x001c(Data)
 * @RunTime 2.30us(Xdata) 1.71us(Idata) 1.21us(Data)
 * @Exp     C = A * B
 *
 * @param   wA   (uint16)  被乘数
 * @param   wB   (uint16)  乘数
 * @param   wCh  (uint16)  积的高16位
 */
#define Muilt_H_MDU2(wA, wB, wCh)                           do { \
                                                                MUL2_MA  = wA;\
                                                                MUL2_MB  = wB;\
                                                                SMDU_RunBlock(2, UMUL);\
                                                                wCh = MUL2_MCH;\
                                                                } while (0)

/**
 * 有符号乘法，结果左移一位,只取高位
 *
 * @CodeLen 0x002a(Xdata) 0x0024(Idata) 0x001c(Data)
 * @RunTime 2.25us(Xdata) 1.71us(Idata) 1.21us(Data)
 * @Exp     C = A * B * 2
 *
 * @param   iA   (int16)  被乘数
 * @param   iB   (int16)  乘数
 * @param   iCh  (int16)  积的高16位
 */
#define MuiltS1_H_MDU2(iA, iB, iCh)                         do { \
                                                                MUL2_MA  = iA;\
                                                                MUL2_MB  = iB;\
                                                                SMDU_RunBlock(2, S1MUL);\
                                                                iCh = MUL2_MCH;\
                                                            } while (0)

#define MuiltS1_H_MDU2_NoBlock(iA, iB, iCh)                 do { \
                                                                MUL2_MA  = iA;\
                                                                MUL2_MB  = iB;\
                                                                SMDU_RunNoBlock(2, S1MUL);\
                                                                iCh = MUL2_MCH;\
                                                            } while (0)

/**
 * Sin/Cos计算与坐标转换
 *
 * @CodeLen 0x0043(Xdata) 0x003a(Idata) 0x002c(Data)
 * @RunTime 4.46us(Xdata) 3.38us(Idata) 2.67us(Data)
 * @Exp     Sin = Cos0 * Sin(Theta) + Sin0 * Cos(Theta)
 *          Cos = Cos0 * Cos(Theta) - Sin0 * Sin(Theta)
 *
 * @param   iCos0   (int16)  初始Cos值
 * @param   iTheta  (int16)  角度
 * @param   iSin0   (int16)  初始Sin值
 * @param   iCos    (int16)  Cos计算结果
 * @param   iSin    (int16)  Sin计算结果
 */
#define SinCos_MDU2(iCos0, iTheta, iSin0, iCos, iSin)       do {\
                                                                SCAT2_COS  = iCos0;\
                                                                SCAT2_SIN  = iSin0;\
                                                                SCAT2_THE  = iTheta;\
                                                                SMDU_RunBlock(2,SIN_COS);\
                                                                iCos   = SCAT2_RES1;\
                                                                iSin   = SCAT2_RES2;\
                                                            } while (0)



/*------------------------------------------------------------------------------------------*/
/**
 * 无符号乘法
 *
 * @CodeLen 0x0033(Xdata) 0x0029(Idata) 0x0021(Data)
 * @RunTime 2.88us(Xdata) 1.96us(Idata) 1.46us(Data)
 * @Exp     C = A * B
 *
 * @param   wA   (uint16)  被乘数
 * @param   wB   (uint16)  乘数
 * @param   wCh  (uint16)  积的高16位
 * @param   wCl  (uint16)  积的低16位
 */
#define Muilt_MDU2(wA, wB, wCh, wCl)                        do { \
                                                                MUL2_MA  = wA;\
                                                                MUL2_MB  = wB;\
                                                                SMDU_RunBlock(2, UMUL);\
                                                                wCh = MUL2_MCH;\
                                                                wCl = MUL2_MCL;\
                                                                } while (0)

/**
 * 无符号乘法,只取高位
 *
 * @CodeLen 0x002a(Xdata) 0x0024(Idata) 0x001c(Data)
 * @RunTime 2.30us(Xdata) 1.71us(Idata) 1.21us(Data)
 * @Exp     C = A * B
 *
 * @param   wA   (uint16)  被乘数
 * @param   wB   (uint16)  乘数
 * @param   wCh  (uint16)  积的高16位
 */
#define Muilt_H_MDU2(wA, wB, wCh)                           do { \
                                                                MUL2_MA  = wA;\
                                                                MUL2_MB  = wB;\
                                                                SMDU_RunBlock(2, UMUL);\
                                                                wCh = MUL2_MCH;\
                                                                } while (0)
                                    
                                    
/**
 * 无符号乘法,只取低位
 *
 * @CodeLen 0x002a(Xdata) 0x0024(Idata) 0x001c(Data)
 * @RunTime 2.30us(Xdata) 1.71us(Idata) 1.21us(Data)
 * @Exp     C = A * B
 *
 * @param   wA   (uint16)  被乘数
 * @param   wB   (uint16)  乘数
 * @param   wCl  (uint16)  积的低16位
 */
#define Muilt_L_MDU2(wA, wB, wCl)                           do { \
                                                                MUL2_MA  = wA;\
                                                                MUL2_MB  = wB;\
                                                                SMDU_RunBlock(2, UMUL);\
                                                                wCl = MUL2_MCL;\
                                                                } while (0)


/**
 * 无符号乘法,只取低位
 *
 * @CodeLen 0x002a(Xdata) 0x0024(Idata) 0x001c(Data)
 * @RunTime 2.30us(Xdata) 1.71us(Idata) 1.21us(Data)
 * @Exp     C = A * B
 *
 * @param   wA   (uint16)  被乘数
 * @param   wB   (uint16)  乘数
 * @param   wCl  (uint16)  积的低16位
 */
#define Muilt_L_MDU1(wA, wB, wCl)                           do { \
                                                                MUL1_MA  = wA;\
                                                                MUL1_MB  = wB;\
                                                                SMDU_RunBlock(1, UMUL);\
                                                                wCl = MUL2_MCL;\
                                                                } while (0)
/**
 * 有符号乘法，结果左移一位,只取高位
 *
 * @CodeLen 0x002a(Xdata) 0x0024(Idata) 0x001c(Data)
 * @RunTime 2.25us(Xdata) 1.71us(Idata) 1.21us(Data)
 * @Exp     C = A * B * 2
 *
 * @param   iA   (int16)  被乘数
 * @param   iB   (int16)  乘数
 * @param   iCh  (int16)  积的高16位
 */
#define MuiltS1_H_MDU2(iA, iB, iCh)                         do { \
                                                                MUL2_MA  = iA;\
                                                                MUL2_MB  = iB;\
                                                                SMDU_RunBlock(2, S1MUL);\
                                                                iCh = MUL2_MCH;\
                                                            } while (0)

/**
 * 有符号乘法
 *
 * @CodeLen 0x0033(Xdata) 0x0029(Idata) 0x0021(Data)
 * @RunTime 2.88us(Xdata) 1.96us(Idata) 1.46us(Data)
 * @Exp     C = A * B
 *
 * @param   iA   (int16)  被乘数
 * @param   iB   (int16)  乘数
 * @param   iCh  (int16)  积的高16位
 * @param   iCl  (int16)  积的低16位
 */
#define MuiltS_MDU2(iA, iB, iCh, iCl)                       do { \
                                                                MUL2_MA  = iA;\
                                                                MUL2_MB  = iB;\
                                                                SMDU_RunBlock(2, SMUL);\
                                                                iCh = MUL2_MCH;\
                                                                iCl = MUL2_MCL;\
                                                            } while (0)

/**
 * 有符号乘法,只取高位
 *
 * @CodeLen 0x002a(Xdata) 0x0024(Idata) 0x001c(Data)
 * @RunTime 2.30us(Xdata) 1.71us(Idata) 1.21us(Data)
 * @Exp     C = A * B
 *
 * @param   iA   (int16)  被乘数
 * @param   iB   (int16)  乘数
 * @param   iCh  (int16)  积的高16位
 */
#define MuiltS_H_MDU2(iA, iB, iCh)                          do { \
                                                                MUL2_MA  = iA;\
                                                                MUL2_MB  = iB;\
                                                                SMDU_RunBlock(2, SMUL);\
                                                                iCh = MUL2_MCH;\
                                                            } while (0)

/**
 * 32Bit/16Bit除法----求商
 *
 * @CodeLen 0x0043(Xdata) 0x003a(Idata) 0x002c(Data)
 * @RunTime 4.46us(Xdata) 3.50us(Idata) 2.67us(Data)
 * @Exp     C = A / B
 *
 * @param   wAh  (uint16)  被除数的高16位
 * @param   wAl  (uint16)  被除数的低16位
 * @param   wB   (uint16)  除数
 * @param   wCh  (uint16)  商的高16位
 * @param   wCl  (uint16)  商的低16位
 */
#define DivQ_MDU2(wAh, wAl, wB, wCh, wCl)                   do {\
                                                                DIV2_DAH  = wAh;\
                                                                DIV2_DAL  = wAl;\
                                                                DIV2_DB   = wB;\
                                                                SMDU_RunBlock(2, DIV);\
                                                                wCh    = DIV2_DQH;\
                                                                wCl    = DIV2_DQL;\
                                                            } while (0)

/**
 * 32Bit/16Bit除法----求商,只取低位
 *
 * @CodeLen 0(Xdata) 0(Idata) 0(Data)
 * @RunTime 0(Xdata) 0(Idata) 0(Data)
 * @Exp     C = A / B
 *
 * @param   wAh  (uint16)  被除数的高16位
 * @param   wAl  (uint16)  被除数的低16位
 * @param   wB   (uint16)  除数
 * @param   wCl  (uint16)  商的低16位
 */
#define DivQ_L_MDU2(wAh, wAl, wB, wCl)                      do {\
                                                                DIV2_DAH  = wAh;\
                                                                DIV2_DAL  = wAl;\
                                                                DIV2_DB   = wB;\
                                                                SMDU_RunBlock(2, DIV);\
                                                                wCl    = DIV2_DQL;\
                                                              } while (0)

/**
 * 低通滤波器
 *
 * @CodeLen 0x0049(Xdata) 0x003d(Idata) 0x002f(Data)
 * @RunTime 4.21us(Xdata) 2.88us(Idata) 2.29us(Data)
 * @Exp     Y += K * (X - Y)
 *
 * @param   iYh  (int16)  滤波输出的高16位(填入上一次计算的输出，取出新的输出)
 * @param   iYl  (int16)  滤波输出的低16位(填入上一次计算的输出，取出新的输出)
 * @param   iX   (int16)  滤波输入
 * @param   ucK  (uint8)  滤波系数(Q8格式：0~255)
 */
#define LPF_MDU2(iX, ucK, iYh, iYl)                         do { \
                                                                LPF2_K = ucK;\
                                                                LPF2_X = iX;\
                                                                LPF2_YH = iYh;\
                                                                LPF2_YL = iYl;\
                                                                SMDU_RunBlock(2, LPF);\
                                                                iYh    = LPF2_YH;\
                                                                iYl    = LPF2_YL;\
                                                            } while (0)

/**
 * Sin/Cos计算与坐标转换
 *
 * @CodeLen 0x0043(Xdata) 0x003a(Idata) 0x002c(Data)
 * @RunTime 4.46us(Xdata) 3.38us(Idata) 2.67us(Data)
 * @Exp     Sin = Cos0 * Sin(Theta) + Sin0 * Cos(Theta)
 *          Cos = Cos0 * Cos(Theta) - Sin0 * Sin(Theta)
 *
 * @param   iCos0   (int16)  初始Cos值
 * @param   iTheta  (int16)  角度
 * @param   iSin0   (int16)  初始Sin值
 * @param   iCos    (int16)  Cos计算结果
 * @param   iSin    (int16)  Sin计算结果
 */
#define SinCos_MDU2(iCos0, iTheta, iSin0, iCos, iSin)       do {\
                                                                SCAT2_COS  = iCos0;\
                                                                SCAT2_SIN  = iSin0;\
                                                                SCAT2_THE  = iTheta;\
                                                                SMDU_RunBlock(2,SIN_COS);\
                                                                iCos   = SCAT2_RES1;\
                                                                iSin   = SCAT2_RES2;\
                                                            } while (0)

/**
 * 角度计算
 *
 * @CodeLen 0(Xdata) 0(Idata) 0(Data)
 * @RunTime 0(Xdata) 0(Idata) 0(Data)
 * @Exp     Theta = Atan(Sin / Cos)
 *
 * @param   iCos    (int16)  输入的Cos值
 * @param   iSin    (int16)  输入的Sin值
 * @param   iTheta  (int16)  角度计算结果
 */
#define Atan_Theta_MDU2(iCos, iSin, iTheta)                 do {\
                                                                SCAT2_COS  = iCos;\
                                                                SCAT2_SIN  = iSin;\
                                                                SMDU_RunBlock(2, ATAN);\
                                                                iTheta    = SCAT2_RES2;\
                                                            } while (0)

/**
 * 幅值计算
 *
 * @CodeLen 0(Xdata) 0(Idata) (Data)
 * @RunTime 0(Xdata) 0(Idata) (Data)
 * @Exp     Us    = Sqrt(Sin^2 + Cos^2)
 *
 * @param   iCos    (int16)  输入的Cos值
 * @param   iSin    (int16)  输入的Sin值
 * @param   iUs     (int16)  幅值计算结果
 */
#define Atan_Us_MDU2(iCos, iSin, iUs)                       do {\
                                                                SCAT2_COS  = iCos;\
                                                                SCAT2_SIN  = iSin;\
                                                                SMDU_RunBlock(2, ATAN);\
                                                                iUs    = SCAT2_RES1;\
                                                            } while (0)

/**
 * 幅值低通滤波器
 *
 * @CodeLen 0x0039(Xdata) 0x0033(Idata) 0x0026(Data)
 * @RunTime 6.00us(Xdata) 4.71us(Idata) 3.63us(Data)
 * @Exp     Y += K * (Sqrt(Sin^2 + Cos^2) - Y)
 *
 * @param   iCos  (int16)  输入的Cos值
 * @param   iSin  (int16)  输入的Sin值
 * @param   ucK   (uint8)  滤波系数(Q8格式：0~255)
 * @param   iYh   (int16)  滤波输出的高16位(填入上一次计算的输出，取出新的输出)
 * @param   iYl   (int16)  滤波输出的低16位(填入上一次计算的输出，取出新的输出)
 */
#define Atan_LPF_MDU2(iCos, iSin, ucK, iYh, iYl)            do {\
                                                                SCAT2_COS  = iCos;\
                                                                SCAT2_SIN  = iSin;\
                                                                SMDU_RunBlock(2, ATAN);\
                                                                LPF2_K = ucK;\
                                                                LPF2_X = SCAT0_RES1;\
                                                                LPF2_YH = iYh;\
                                                                LPF2_YL = iYl;\
                                                                SMDU_RunBlock(2, LPF);\
                                                                iYh    = LPF2_YH;\
                                                                iYl    = LPF2_YL;\
                                                            } while (0)

#endif

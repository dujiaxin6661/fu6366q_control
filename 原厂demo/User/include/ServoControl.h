#ifndef __SERVOCONTROL_H_
#define __SERVOCONTROL_H_

/******************************************************************************/
#include <FU68xx_4_Type.h>
/******************************************************************************/

typedef struct
{
    uint8   HallCalibrationFlag;
    int16   DirCnt;
    S32tS16 AngleErr;
    int16   FlagCircle;
    int16   iSin;
    int16   iCos;
    uint16  Theta180;                        // MR出来角度，一圈180°
    uint16  Theta180Old;
    int16   HallAngleOffset;                 // HALL和MR过零点偏差
    int16   ThetaOld;
    int16   SinOffset;
    int16   CosOffset;
    uint8   HallState;                      // Hall状态
    uint8   AngleOffsetInitFlag;            // 角度偏置获取标志
    uint8   SinCosOffsetInitFlag;           // 模拟偏置获取标志
    uint8   HallAngleOffsetInitFlag;        // Hall和MR角度偏置校准
    uint8   SenconrdPowerOn;  
    uint8   HallA;  
		uint8   HallB;  
}MRSensor_Typedef;

#define OffsetCalibraTimes              (4) // 取4次平均  至少四次

typedef struct
{
    uint8   SinState;                       // Sin校准状态
    uint8   CosState;                       // Cos校准状态
    uint8   SinCnt;
    uint8   CosCnt;
    int16   iSinMax;                        // Sin最大值存储
    int16   iCosMax;                        // Cos最大值存储
    int16   iSinMin;                        // Sin最小值存储
    int16   iCosMin;                        // Cos最小值存储
    int16   iSinOffset[OffsetCalibraTimes]; // Sin偏置存储
    int16   iCosOffset[OffsetCalibraTimes]; // Cos偏置存储
    int16   iSinPeak[OffsetCalibraTimes];   // Sin偏置存储
    int16   iCosPeak[OffsetCalibraTimes];   // Cos偏置存储
	
	  int16   iSinPeakValue;   // Sin峰值存储
	  int16   iCosPeakValue;   // Cos峰值存储
	
	  int16   iPeakNorlizationCoff;   // 归一化系数
		int16   iPeakNorDeltaVlaue;     // 归一化增量
	  uint8   iPeakNorlizationFlag;   // 归一化标志
	  uint8   iPeakNorlizationFinshFlag;   // 归一化标志
	
}MRSensorOffset_Typedef;


typedef struct
{
    int16   Err;
    int16   Err_last;
    int16   Ref_last;
    int16   Fb_last;
    int32   Integral;
    int32   Uk;
    int16   Uk_max;
    int16   Uk_min;
    int32   Ki_max;
    int32   Ki_min;
    uint16  Kp;
    uint16  Ki;
    uint16  Kf;
	  uint16  Kd;
}PI_Typedef;


#define MR_NormaLizationEnable          (Disable)                                                // MR归一化处理    

#define HALLNUM                         (TWOHALL)                                               // 位置检测HALL数量
#define NONEHALL                        (0)                                                      // 没有HALL
#define TWOHALL                         (1)                                                      // 有2个HALL

#define Pole                            (2)                                                     // 一个机械周期对应sin周期信号数
#define MR_ATT_COEF                     (0.85)
#define MR_Speed_n                      (2500)                                                  // rpm/min
#define MR_Omega_n                      (MR_Speed_n*_2PI/60)                                    // 自然角频率

#define MR_BASE_FREQ1                   ((MOTOR_SPEED_BASE / 60)*Pole)                        // 将rad/s转为标幺的rpm/min
#define PLL_KP                          _Q12(2*MR_ATT_COEF*MR_Omega_n/MR_BASE_FREQ1)
#define PLL_KI                          _Q15(MR_Omega_n*MR_Omega_n*TPWM_VALUE/MR_BASE_FREQ1)

#define MR_FBASE1                       _Q15(MR_BASE_FREQ1*TPWM_VALUE)                        // 两个载波中断执行一次

extern PI_Typedef             xdata     s_PI;
extern MRSensor_Typedef       idata    MR;
extern MRSensorOffset_Typedef xdata    MROffset;

extern void MR_Offset_Calibration_GETDATA(void);
extern void MR_HALL_Offset_Calibration_GETDATA(void);
extern void MR_Offset_Calibration_PROCESSDATA(void);
extern void Hall_MR_Offset_Check(void);
extern void Hall_MR_StartCheck(void);

#endif

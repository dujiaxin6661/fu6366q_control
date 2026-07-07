/**
 * ============================================================================
 * @file      main.c
 * @brief     主函数入口 —— 电机控制固件的主循环
 * @author    Fortiortech Application Team (Kobe Peng)
 * @since     2023-07
 * @note      上电后依次执行硬件初始化、变量初始化、电流偏置校准，
 *            然后进入无限循环：电流校准 → 电机状态机 → MR传感器状态机 → 应用控制 → CAN通信
 * ============================================================================
 */

/*-------------------------------------------------------------------------------
    Header Definition
-------------------------------------------------------------------------------*/
#include <Myproject.h>

/**
 * @brief      Flash预存偏置常量数组（地址 0x3F00）
 * @note       上电时直接将这些值填入电流偏置寄存器，省去每次上电重新校准
 *             数据顺序：IuOffset, IvOffset, IwOffset, FRStatus, AngleOffset, HallOffsetFlag, HallOffset
 */
code uint16 OffsetInitial[7] = {0x3EE8, 0x4232, 0x0000, 0X0001, 0XFAC2, 0X0001, 0x2DBB}; // ?CO?MAIN(C:0x3f00)
                                                                                           // 用于直接填入偏置至内存，省掉初始上电整定内存

/**
 * @brief      main() —— 电机控制固件主函数
 * @note       执行流程：
 *               1. 上电延时等待（SystemPowerUpTime 个计数周期）
 *               2. HardwareInit() —— 初始化MCU外设（GPIO/ADC/PWM/定时器等）
 *               3. SoftwareInit() —— 初始化控制变量和状态机
 *               4. 主循环（while(1)）：
 *                  - GetCurrentOffset()    : ADC电流偏置校准（仅上电首次执行，校准完OffsetFlag=1后跳过）
 *                  - MC_Control()          : 电机9状态状态机（Ready→Init→...→Run→Stop）
 *                  - MR_Control()          : MR磁阻传感器校准状态机
 *                  - FUNC_Control()        : 应用层控制（按键处理、CAN指令响应）
 *                  - CAN_Process()         : CAN总线通信处理
 */
void main(void)
{
    uint16 PowerUpCnt = 0;

    /*********上电等待——等待电源稳定*********/
    for(PowerUpCnt = 0; PowerUpCnt < SystemPowerUpTime; PowerUpCnt++){};
        PowerUpCnt = OffsetInitial[0];

    /* ----- 硬件初始化，配置MCU外设（GPIO/ADC/PWM/定时器/CAN等）----- */
    HardwareInit();

    /* ----- 变量初始化（控制结构体清零、PI参数赋值、保护变量初始化）----- */
    SoftwareInit();

    /* -----debug配置(SPI调试)，量产程序可以删除----- */
//    SPIDebugInit();

    /**************************** 总中断使能 **************************/
    mcSpeedRamp.FlagONOFF = 1;          // 启动标志置1，允许电机进入启动流程

    while (1)
    {
        /* ------ ADC电流偏置校准（首次上电执行~300次后OffsetFlag置1，后续跳过）------ */
        GetCurrentOffset();

        /* ------ 电机状态机：9状态切换（mcReady→mcInit→mcCharge→...→mcRun）------ */
        MC_Control();

        /* ------ MR磁阻传感器状态机：校准→就绪→运行------ */
        MR_Control();

        /* ------ 应用层控制：按键检测 / 模式切换 / CAN指令执行------ */
        FUNC_Control();

        /* ------ CAN总线通信：接收主机指令并应答------ */
        CAN_Process();
    }
}

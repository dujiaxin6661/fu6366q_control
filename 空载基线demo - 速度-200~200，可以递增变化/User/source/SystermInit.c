/**
 * @copyright (C) COPYRIGHT 2022 Fortiortech Shenzhen
 * @file      main.c
 * @author    Fortiortech  Appliction Team
 * @since     Create:2023-07
 * @date      Last modify:2023-07
 * @note      Last modify author is Kobe Peng
 * @brief      
 */


#include <Myproject.h>

/*-------------------------------------------------------------------------------------------------
        Function Name : void DebugSet(void)
        Description   : 调试模式配置
        Input         : 无
        Output        : 无
-------------------------------------------------------------------------------------------------*/

void DebugSet(void)
{
    /*******************Observation signal Select*********************/
    SetReg(CMP_CR3, DBGSEL0 | DBGSEL1, GP01_DBG_Conf);
    SetReg(CMP_CR3, CMPSEL0 | CMPSEL1 | CMPSEL2, GP07_DBG_Conf);
}

void SPIDebugInit(void)
{
    /************************硬件外设初始化**************************/
    #if defined (SPI_DBG_HW)        // 硬件调试模式
    {
        SPI_Init();
        Set_DBG_DMA(&HARD_SPIDATA);
    }
    #elif defined (SPI_DBG_SW)      // 软件调试模式
    {
        SPI_Init();
        Set_DBG_DMA(spidebug);
    }
    #elif defined (UART_DBG)        // UART调试模式    这个模式似乎不可用
    {
        UART1_Init(500000);            // 监控数据串口初始化
        Conf_DMA(1, XDATA_UART1, UART_ANO.T_DATA, 20);
    }
    #endif
    DebugSet();
}

/*-------------------------------------------------------------------------------------------------
    Function Name : void HardwareInit(void)
    Description   : 硬件初始化，初始化需要使用的硬件设备配置，FOC必须配置的是运放电压、运放初始化、ADC初始化、Driver初始化
                    ，其他的可根据实际需求加。
    Input         : 无
    Output        : 无
-------------------------------------------------------------------------------------------------*/

void HardwareInit(void)
{
    GPIO_Init();
	
    /*----- ADC参考电压电压配置 ------*/
    ADC_Init();
	
	  CAN_Init();//CAN初始化

    /*********硬件FO过流，比较器初始化，用于硬件过流比较保护*********/
    #if (HardwareCurrent_Protect == Hardware_FO_Protect)
        INT0_Init();
    #elif (HardwareCurrent_Protect == Hardware_CMP_Protect)
        CMP3_Init();
    #elif (HardwareCurrent_Protect == Hardware_FO_CMP_Protect)
        INT0_Init();
        CMP3_Init();
    #endif
    
    Driver_Init();
	
    /*****运放初始化********************/
    Current_AMP_Init();//电流采样运放
		
    MR_AMP_Init();//MR采样运放初始化 
  
    /******比较器中断配置**********/
    CMP3_Interrupt_Init();
    
  /*************************SYSTICK定时器配置**********************/
    ClrBit(IP2, PTIM41);    //4K中断
    ClrBit(IP2, PTIM40);
    SetBit(DRV_SR, SYSTIE);   
    EA = 1;		
}

/*-------------------------------------------------------------------------------------------------
    Function Name : void SoftwareInit(void)
    Description   : 软件初始化，初始化所有定义变量，按键初始化扫描
    Input         : 无
    Output        : 无
-------------------------------------------------------------------------------------------------*/

void SoftwareInit(void)
{
    MotorcontrolInit();
    PI0_Init();        // 速度环硬件PI参数初始化
    mcState = mcReady;
    mcFaultSource = 0;
    FlashRead();
}

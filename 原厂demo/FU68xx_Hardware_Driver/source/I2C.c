/********************************************************************************

 **** Copyright (C), 2019, Fortior Technology Co., Ltd.                      ****

 ********************************************************************************
 * File Name     : I2C.c
 * Author        : Bruce HW&RD
 * Date          : 2019-03-27
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 * 
 * Record        :
 * 1.Date        : 2019-03-27
 *   Author      : Bruce HW&RD
 *   Modification: Created file

********************************************************************************/

#include <FU68xx_4.h>
#include <Myproject.h>

#if (COMMUNICATE_MODE == IICMODE)
volatile IICTypeDef idata IIC;
#endif

/*-------------------------------------------------------------------------------------------------
    Function Name  : Init_I2C
    Description    : I2C初始化配置
    Date           : 2020-09-23
    Parameter      : None
------------------------------------------------------------------------------------------------- */
void Init_I2C(void)
{
    I2C_CR = 0x00;                                     //初始化IIC功能寄存器
    /*  -------------------------------------------------------------------------------------------------
        主/从机配置
        1：将6832配置为主机
        0：将6832配置为从机
        -------------------------------------------------------------------------------------------------*/
    #if (I2C_MASTER_SLAVE == I2C_MASTER)
    {
        SetBit(I2C_CR, I2CMS);
    }
    #elif (I2C_MASTER_SLAVE == I2C_SLAVE)
    {
        ClrBit(I2C_CR, I2CMS);
    }
    #endif
    /*  -------------------------------------------------------------------------------------------------
        I2C传输速率配置
        00：100KHz传输速率
        01：400KHz传输速率
        10：1MHz传输速率
        -------------------------------------------------------------------------------------------------*/
    #if (I2C_SPEED == I2C_SPD_100k)
    {
        ClrBit(I2C_CR, I2CSPD1);
        ClrBit(I2C_CR, I2CSPD0);
    }
    #elif (I2C_SPEED == I2C_SPD_400k)
    {
        ClrBit(I2C_CR, I2CSPD1);
        SetBit(I2C_CR, I2CSPD0);
    }
    #elif (I2C_SPEED == I2C_SPD_1M)
    {
        SetBit(I2C_CR, I2CSPD1);
        ClrBit(I2C_CR, I2CSPD0);
    }
    #endif
    /*  -------------------------------------------------------------------------------------------------
        I2C中断配置
        0：不使能中断
        1：使能中断
        -------------------------------------------------------------------------------------------------*/
    #if (I2C_IRQ == DIS_IRQ_I2C)
    {
        ClrBit(I2C_CR, I2CIE);
    }
    #elif (I2C_IRQ == EN_IRQ_I2C)
    {
        SetBit(I2C_CR, I2CIE);
        PI2C_UT11 = 1;
        PI2C_UT10 = 0;
    }
    #endif
    /*  -------------------------------------------------------------------------------------------------
        MCU作为从机时地址设置
        -------------------------------------------------------------------------------------------------*/
    #if (I2C_MASTER_SLAVE == I2C_SLAVE)
    {
        I2C_ID = I2C_ADDR7 << 1;
    }
    #endif
    
    Set_Addr_I2C(0x06);                             //填写从机地址
    Enable_I2C();                                   //I2C使能
}



/* -------------------------------------------------------------------------------------------------
    Function Name  : IIC_EncoderSensor_ReadStart
    Description    : 启动角度读取
    Date           : 2021-02-26
    Parameter      : None
------------------------------------------------------------------------------------------------- */
void IIC_EncoderSensor_ReadStart(void)
{
    if(IIC.ReadStatus == 0)
    {
        I2C_SR = I2CSTA;                //启动IIC
        IIC.Mode = 0;
        IIC.ReadStatus = 1;
    }
}

/*  -------------------------------------------------------------------------------------------------
    Function Name  : IIC_Write_MPU_Start
    Description    : 通过IIC写MPU寄存器函数
    Date           : 2020-09-28
    Parameter      : Reg: [寄存器]
**                   Data: [数据]
    ------------------------------------------------------------------------------------------------- */
void IIC_Write_EncoderSensor_Start(uint8 Reg, uint8 Data1, uint8 Data2)
{
    IIC.WriteFinishStatus = 0;
    
    while (!IIC.WriteFinishStatus)
    {
        if (IIC.WriteStatus == 0)
        {
            if(IIC.WriteFinishStatus==0)
            {
                IIC.WriteStatus = 1;
                IIC.Mode = 1;
                IIC.WriteReg = Reg;
                IIC.WriteData1 = Data1;
                IIC.WriteData2 = Data2;
                IIC.WriteFinishStatus = 0;
                I2C_SR = I2CSTA;
            }
        }
    }
}


/* -------------------------------------------------------------------------------------------------
    Function Name  : IIC_Read_EncoderSensor_Process
    Description    : 角度读取程序
    Date           : 2021-02-26
    Parameter      : None
------------------------------------------------------------------------------------------------- */
void IIC_Read_EncoderSensor_Process(void)
{    
    if(ReadBit(I2C_SR,STR))
    {
        switch(IIC.ReadStatus)
        {
            case 1:
                I2C_DR = 0x03;
                ClrBit(I2C_SR, STR);
                IIC.ReadStatus = 2;
                break;
            
            case 2:
                I2C_SR = I2CSTA | DMOD;
                IIC.ReadStatus = 3;
                break;
            
            case 3:
                SetBit(I2C_SR, NACK);
                ClrBit(I2C_SR, STR);
                IIC.ReadStatus = 4;
                break;
            
            case 4:
                IIC.Angle.AngleU8.AngleH = I2C_DR;
                ClrBit(I2C_SR, DMOD);
                SetBit(I2C_SR, I2CSTP|I2CSTA);
                ClrBit(I2C_SR, STR);
                IIC.ReadStatus = 5;
                break;
            
            case 5:
                I2C_DR = 0x04;
                ClrBit(I2C_SR, STR);
                IIC.ReadStatus = 6;
                break;
            
            case 6:
                I2C_SR = I2CSTA | DMOD;
                IIC.ReadStatus = 7;
                break;
            
            case 7:
                SetBit(I2C_SR, NACK);
                ClrBit(I2C_SR, STR);
                IIC.ReadStatus = 8;
                break;
            
            case 8:
                IIC.Angle.AngleU8.AngleL = I2C_DR;
                SetBit(I2C_SR, I2CSTP);
                ClrBit(I2C_SR, STR);
                IIC.AngleEnd = IIC.Angle.AngleU16;
                IIC.ReadFinishStatus = 1;
                IIC.ReadStatus = 0;
            
            default:
                IIC.ReadStatus = 0;
                break;
        }

    }
    else
    {
        IIC.ReadStatus = 0;
    }
}

/* -------------------------------------------------------------------------------------------------
    Function Name  : IIC_Write_EncoderSensor_Process
    Description    : 磁编数据配置
    Date           : 2021-02-26
    Parameter      : None
------------------------------------------------------------------------------------------------- */
void IIC_Write_EncoderSensor_Process(void)
{    
    if ((ReadBit(I2C_SR, STR)) && (!ReadBit(I2C_SR, NACK)))
    {
        switch(IIC.WriteStatus)
        {
            case 1:
                I2C_DR = IIC.WriteReg;
                ClrBit(I2C_SR, STR);
                IIC.WriteStatus = 2;
                break;
            
            case 2:
                I2C_DR = IIC.WriteData1;
                ClrBit(I2C_SR, STR);
                IIC.WriteStatus = 3;
                break;
            
            case 3:
                I2C_DR = IIC.WriteData2;
                ClrBit(I2C_SR, STR);
                IIC.WriteStatus = 4;
                break;
            
            case 4:
                SetBit(I2C_SR, I2CSTP);
                ClrBit(I2C_SR, STR);
                IIC.WriteStatus = 0;
                IIC.WriteFinishStatus = 1;
                break;
            
            default:
                IIC.WriteStatus = 0;
                _nop_();
                break;
        }
    }
    else
    {
        IIC.WriteStatus = 0;
    }
}

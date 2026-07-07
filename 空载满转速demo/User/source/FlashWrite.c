/**
 * ============================================================================
 * @file      FlashWrite.c
 * @brief     Flash存储器读写 —— 校准参数的持久化存储
 * @author    Fortiortech Application Team (Kobe Peng)
 * @since     2023-07
 * @note      使用FU68xx内部Flash扇区（地址0x3E80~0x3F80，即OFFSETCODEADDRESS=0x3F00）
 *             存储校准参数，断电不丢失，上电后直接读取避免重复校准：
 *
 *             存储布局（每个值2字节，地址递增）：
 *               OFFSETCODEADDRESS+0  : MR.SinOffset                (Sin偏置值)
 *               OFFSETCODEADDRESS+2  : MR.CosOffset                (Cos偏置值)
 *               OFFSETCODEADDRESS+4  : mcFocCtrl.FRStatus          (旋转方向)
 *               OFFSETCODEADDRESS+6  : MR.AngleOffsetInitFlag      (角度零偏校准标志)
 *               OFFSETCODEADDRESS+8  : mcFocCtrl.SensorAngleOffset (传感器角度零偏)
 *               OFFSETCODEADDRESS+10 : MR.HallAngleOffsetInitFlag  (霍尔偏置校准标志) [仅TWOHALL]
 *               OFFSETCODEADDRESS+12 : MR.HallAngleOffset          (霍尔角度偏置) [仅TWOHALL]
 *
 *             Flash操作需要安全密钥（0x5A, 0x1F）解锁，用中断关闭(EA=0)保护写时序
 *             每次Write前先Erase扇区（整扇区擦除），然后逐个写入2字节值
 *
 *             FlashRead()在SoftwareInit()中被调用，判断SenconrdPowerOn标志决定启动流程
 * ============================================================================
 */

#include <MyProject.h>

FlashSecurityKey data FlashKey;                          /**< Flash安全密钥 {0x5A, 0x1F} */

/**
 * @brief      WriteXByte2Flash() —— 向Flash写入X字节数据
 * @param      FlashStartAddr : Flash目标地址
 * @param      NewData2Flash  : 要写入的32位数据（取低X字节）
 * @param      XByte          : 写入字节数（1~255）
 * @note       逐字节将32位数据的低X字节写入Flash
 */
void WriteXByte2Flash(uint16 FlashStartAddr, uint32 NewData2Flash, uint8 XByte)
{
    uint8 i;
    uint32 tempofFlashData = 0;
    uint32 tempofNewFlashData = 0;
    tempofNewFlashData = NewData2Flash;

    for(i = 0; i < XByte; i++)
    {
        tempofFlashData = (tempofNewFlashData >> (8 * (XByte - i - 1))) & 0x000000ff;  // 提取第i个字节
        Flash_Sector_Write((FlashStartAddr + i), (uint8)tempofFlashData,
                            FlashKey.x5a, FlashKey.x1f);                                 // ★ 写入Flash
        _nop_();                                                                          // 等待Flash写完成
    }
}

/**
 * @brief      Get2ByteFromFlash() —— 从Flash读取2字节
 * @param      BlockStartAddr : Flash扇区起始地址
 * @return     读取到的16位数据
 */
uint16 Get2ByteFromFlash(uint8 xdata *BlockStartAddr)
{
    uint8 xdata *FlashStartAddr = BlockStartAddr;
    uint16 tempofFlashData;

    tempofFlashData = *(uint16 code *)(FlashStartAddr);   // ★ 从code区（Flash映射）直接读2字节
    return tempofFlashData;
}

/**
 * @brief      Write2Byte2Flash() —— 向Flash写入2字节数据
 * @param      BlockStartAddr : Flash目标地址
 * @param      NewData2Flash  : 要写入的16位数据
 */
void Write2Byte2Flash(uint16 BlockStartAddr, uint16 NewData2Flash)
{
    uint8 xdata *FlashStartAddr = BlockStartAddr;
    WriteXByte2Flash((FlashStartAddr), NewData2Flash, 2); // 调用通用写入函数
}

/**
 * @brief      Get4ByteFromFlash() —— 从Flash读取4字节（轮询找到最后一次写入的有效数据）
 * @param      BlockStartAddr : Flash扇区起始地址
 * @return     读取到的32位数据
 * @note       32字节区域内，每4字节为一个slot
 *             遍历32个slot，遇到全0的位置说明前面的slot是最后一次有效写入
 *             若32个slot全满 → 返回最后一个slot的数据
 *             经典应用：32组数据循环写入（磨损均衡）
 */
uint32 Get4ByteFromFlash(uint8 xdata *BlockStartAddr)
{
    uint8 xdata *FlashStartAddr = BlockStartAddr;
    uint8 i;
    uint32 tempofFlashData;

    for(i = 0; i < 32; i++)                               // 遍历32个4字节slot
    {
        tempofFlashData = *(uint32 code *)(FlashStartAddr + 4 * i);
        if(tempofFlashData == 0)                           // 遇到空slot
        {
            if(i != 0)                                     // 非第一个slot → 前一个slot有效
            {
                tempofFlashData = *(uint32 code *)(FlashStartAddr + 4 * (i - 1));
                return tempofFlashData;
            }
            else                                           // 第一个就是空 → 无数据
            {
                return 0;
            }
        }
        else
        {
            if(i == 31)                                    // 32个slot全满 → 返回最后一个
            {
                return tempofFlashData;
            }
        }
    }
    return 0;
}

/**
 * @brief      Write4Byte2Flash() —— 向Flash写入4字节（含磨损均衡）
 * @param      BlockStartAddr : Flash目标地址
 * @param      NewData2Flash  : 要写入的32位数据
 * @note       在32个slot中找第一个空slot写入
 *             若32个slot全满 → 擦除整扇区 → 在第一个slot写入
 */
void Write4Byte2Flash(uint16 BlockStartAddr, uint32 NewData2Flash)
{
    uint8 xdata *FlashStartAddr = BlockStartAddr;
    uint32 tempofFlashData = 0;
    uint8 i;

    for(i = 0; i < 32; i++)                               // 找第一个空slot
    {
        tempofFlashData = *(uint32 code *)(FlashStartAddr + 4 * i);
        if(tempofFlashData == 0)                           // 找到空位
        {
            WriteXByte2Flash((FlashStartAddr + 4 * i), NewData2Flash, 4);
            break;
        }
        else
        {
            if(i == 31)                                    // 全满 → 擦除整扇区
            {
                Flash_Sector_Erase(FlashStartAddr, FlashKey.x5a, FlashKey.x1f);
                _nop_();
                WriteXByte2Flash((FlashStartAddr), NewData2Flash, 4);
            }
        }
    }
}


/**
 * ============================================================================
 * @brief      FlashRead() —— ★ 上电时从Flash读取全部校准数据
 * @note       读取顺序：SinOffset/CosOffset/FRStatus/AngleOffset/SensorAngleOffset
 *                        [+ HallAngleOffset/HallAngleOffset(仅双霍尔)]
 *
 *              ★ 关键判断：
 *              若SinOffset或CosOffset==0（Flash无有效数据）→ 首次上电：
 *                - SinCosOffsetInitFlag=0（触发校准流程）
 *                - AngleOffsetInitFlag=0
 *                - HallAngleOffsetInitFlag=0
 *                - SenconrdPowerOn=0（标记为首次上电）
 *                - 偏置设置默认值16383
 *
 *              若数据都有效 → 非首次上电：
 *                - SenconrdPowerOn=1（跳过第一次预定位，直接开环→运行）
 *                - SinCosOffsetInitFlag=1
 *                - 调用Hall_MR_StartCheck()确认当前霍尔扇区
 * ============================================================================
 */
void FlashRead(void)
{
    MR.SinOffset = Get2ByteFromFlash(OFFSETCODEADDRESS);               // 读Sin偏置
    MR.CosOffset = Get2ByteFromFlash(OFFSETCODEADDRESS + 2);           // 读Cos偏置

    if((MR.SinOffset == 0) || (MR.CosOffset == 0))                     // ★ 偏置数据无效 → 首次上电
    {
        MR.SinOffset = 16383;                                           // 设置ADC中值为默认值（16位ADC的½）
        MR.CosOffset = 16383;

        MR.SinCosOffsetInitFlag = 0;                                    // 标记Sin/Cos未校准
        MR.AngleOffsetInitFlag = 0;                                     // 标记角度零偏未校准
        MR.HallAngleOffsetInitFlag = 0;                                 // 标记霍尔偏置未校准
        MR.SenconrdPowerOn = 0;                                         // ★ 标记为首次上电（需要走完整校准流程）
        mcSpeedRamp.FlagONOFF = 1;                                      // 自动触发启动
    }
    else                                                                // ★ 偏置数据有效 → 已有校准数据
    {
        MR.SenconrdPowerOn = 1;                                         // ★ 标记为非首次上电（跳过部分校准）
        MR.SinCosOffsetInitFlag = 1;                                    // Sin/Cos偏置已校准
        mcFocCtrl.FRStatus = Get2ByteFromFlash(OFFSETCODEADDRESS + 4);  // 读取旋转方向

        MR.AngleOffsetInitFlag = Get2ByteFromFlash(OFFSETCODEADDRESS + 6);       // 读取角度零偏标志
        mcFocCtrl.SensorAngleOffset = Get2ByteFromFlash(OFFSETCODEADDRESS + 8);   // 读取传感器角度零偏

       #if (HALLNUM == TWOHALL)                                        // 仅双霍尔模式读取霍尔数据
        {
            MR.HallAngleOffsetInitFlag = Get2ByteFromFlash(OFFSETCODEADDRESS + 10);  // 霍尔偏置标志
            MR.HallAngleOffset = Get2ByteFromFlash(OFFSETCODEADDRESS + 12);           // 霍尔角度偏置值
            Hall_MR_StartCheck();                                      // ★ 调用霍尔初始状态检测（确认扇区）
        }
       #endif
    }
}


/**
 * @brief      OffsetWrite() —— ★ 将所有校准数据写入Flash永久保存
 * @note       调用场景：第二次预定位完成后（Motor_Align中AlignFlag==3时）
 *             写入前先擦除整扇区，然后依次写入12~14字节数据
 */
void OffsetWrite(void)
{
    Flash_Sector_Erase(OFFSETCODEADDRESS, FlashKey.x5a, FlashKey.x1f);  // ★ 先擦除整扇区

    Write2Byte2Flash(OFFSETCODEADDRESS,     MR.SinOffset);                       // Sin偏置
    Write2Byte2Flash(OFFSETCODEADDRESS + 2, MR.CosOffset);                       // Cos偏置
    Write2Byte2Flash(OFFSETCODEADDRESS + 4, mcFocCtrl.FRStatus);                 // 旋转方向

    Write2Byte2Flash(OFFSETCODEADDRESS + 6, MR.AngleOffsetInitFlag);             // 角度零偏标志
    Write2Byte2Flash(OFFSETCODEADDRESS + 8, mcFocCtrl.SensorAngleOffset);        // 角度零偏值

    #if (HALLNUM == TWOHALL)
    {
        Write2Byte2Flash(OFFSETCODEADDRESS + 10, MR.HallAngleOffsetInitFlag);    // 霍尔偏置标志
        Write2Byte2Flash(OFFSETCODEADDRESS + 12, MR.HallAngleOffset);            // 霍尔偏置值
    }
    #endif
}


/**
 * @brief      Flash_Sector_Erase() —— Flash扇区擦除
 * @param      FlashAddress : 扇区起始地址（必须在0x3E80~0x3F80范围内）
 * @param      Key5a, Key1f : 安全密钥（0x5A, 0x1F）
 * @note       擦除前关全局中断(EA=0)，防止中断干扰Flash时序
 *             擦除后上锁(FLA_CR=0x08)
 */
void Flash_Sector_Erase(uint16 FlashAddress, uint8 Key5a, uint8 Key1f)
{
    bool    TempEA;                                        // 暂存EA（全局中断使能位）
    uint8   TempKey5a;
    uint8   TempKey1f;
    uint16  TempFlashAddress;

    TempKey5a = Key5a;
    TempKey1f = Key1f;
    TempFlashAddress = FlashAddress;
    TempEA = EA;
    EA = 0;                                                // ★ 关闭全局中断（保护Flash时序）

    if ((FlashAddress >= 0X3E80) && (FlashAddress < 0x3f80))   // ★ 限制擦除地址范围
    {
        FLA_CR = 0x03;                                     // Flash配置：擦除模式
        FLA_KEY = TempKey5a;                               // ★ 写入安全密钥1
        FLA_KEY = TempKey1f;                               // ★ 写入安全密钥2（两次写入解锁）
        _nop_();
        *(uint8 xdata *)TempFlashAddress = 0xff;            // ★ 对目标地址写0xFF触发扇区擦除
        FLA_CR = 0x08;                                     // Flash配置：上锁
    }

    EA = TempEA;                                           // 恢复中断
}

/**
 * @brief      Flash_Sector_Write() —— Flash字节写入
 * @param      FlashAddress : 目标地址
 * @param      FlashData    : 要写入的单字节数据
 * @param      Key5a, Key1f : 安全密钥
 * @note       同样关全局中断保护时序
 */
void Flash_Sector_Write(uint16 FlashAddress, uint8 FlashData, uint8 Key5a, uint8 Key1f)
{
    bool TempEA;
    uint8   TempKey5a;
    uint8   TempKey1f;
    data uint16  TempFlashAddress;

    TempKey5a = Key5a;
    TempKey1f = Key1f;
    TempFlashAddress = FlashAddress;
    TempEA = EA;
    EA = 0;                                                // ★ 关闭全局中断

    if ((FlashAddress >= 0X3E80) && (FlashAddress < 0x3f80))
    {
        FLA_CR = 0x01;                                     // Flash配置：写入模式
        FLA_KEY = TempKey5a;
        FLA_KEY = TempKey1f;
        _nop_();
        *(uint8 xdata *)TempFlashAddress = FlashData;      // ★ 写入单字节
        FLA_CR = 0x08;                                     // 上锁
    }

    EA = TempEA;                                           // 恢复中断
}

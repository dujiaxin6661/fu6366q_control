# FU6366Q 电机测试项目交接说明

本文档用于把当前 FU6366Q 电机测试项目的背景、已完成工作、关键结论、代码状态和后续计划交接给其他 AI 或工程协作者。

## 1. 项目目标

本项目最初目标是：

1. 将 FU6366Q 电机驱动板上的运行数据通过 CAN 总线传输到电脑。
2. 使用 PCAN-USB 将 CAN 数据转成 USB，电脑作为上位机采集、记录和可视化数据。
3. 主要关注电机旋转时的：
   - 电流
   - Iq / Id
   - 转速
   - 目标转速
   - 母线电压
   - 故障状态
   - 后续可能扩展到力矩估算
4. 最终服务于云台项目，判断电机是否能满足真实负载下的静态保持、连续跟踪、点击转动/急停需求。

现在项目目标已从“只调通通信”扩展为：

```text
建立一套电机能力评估流程：
CAN 通信采集 -> 空载基线 -> 最高转速识别 -> 电机型号推断 -> 力矩估算 -> 云台负载可行性判断 -> 后续整机验证规划
```

## 2. 当前工作目录和重要文件

工作目录：

```text
C:\Users\Easyj\Documents\fu6366q电机
```

重要目录：

```text
原厂demo
```

当前主要固件工程，适合继续做负载测试。按键速度范围为 `-200 ~ +200 rpm`，每次按键变化 `10 rpm`。

```text
空载基线demo - 速度-200~200，可以递增变化
```

备份的空载基线版本，不建议修改。

```text
空载满转速demo
```

用于电机本体空载最高转速测试。按键速度范围为 `-1800 ~ +1800 rpm`，每次按键变化 `100 rpm`。

```text
pc_visualizer
```

电脑端 Python 上位机程序，负责 PCAN-USB 采集、CSV 记录、曲线绘制、转速监视窗口。

重要数据文件：

```text
pc_visualizer\logs\fu6366q_telemetry.csv
pc_visualizer\logs\fu6366q_latest_speed_summary.csv
pc_visualizer\logs\空载基线版本数据记录.csv
```

其他图片：

```text
构造图1.jpg
构造图2.jpg
```

这两张图展示了实际工程中可能使用同步带：小轮带大轮。

## 3. 硬件信息

### 3.1 CAN 通信硬件

电脑识别到的 USB-CAN 模块是：

```text
PCAN-USB
```

在 Windows 设备管理器中显示为：

```text
CAN-Hardware -> PCAN-USB
```

因此上位机使用 `python-can` 的 `pcan` interface。

常用命令：

```powershell
cd C:\Users\Easyj\Documents\fu6366q电机\pc_visualizer
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000
```

如果需要打印原始 CAN 帧：

```powershell
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000 --print-raw
```

注意：`--print-raw` 已经做了限速打印，避免 PCAN 接收队列被 PowerShell 打印拖慢。

### 3.2 CAN 接线

CANH 接 CANH，CANL 接 CANL，不交叉。

```text
PCAN CANH -> FU6366Q CANH
PCAN CANL -> FU6366Q CANL
GND 建议共地
```

### 3.3 FU6366Q 板子

根据原理图，FU6366Q 板上有 CAN 收发器：

```text
SN65HVD1050
```

CAN 相关引脚：

```text
FU6366Q P0.0/P0.1 -> CAN CTX/CRX
```

板上有 CANH/CANL 和 120R 终端电阻。

## 4. 电机候选参数

曾经找到两张可能对应当前电机的参数表。

### 4.1 GB3506

```text
额定电压：12 V
额定电流：1.1 A
额定扭矩：0.09 N·m
额定转速：467 rpm
峰值转速：1610 rpm
峰值扭矩：0.15 N·m
峰值电流：1.9 A
相间电阻：6.56 Ω
相间电感：1.27 mH
Kv：134 rpm/V
Kt：0.08 N·m/A
极对数：11
带编码器重量：71 g
带编码器尺寸：Ø40 × 21 mm
编码器：AS5048A / 5600
```

### 4.2 GB3510

```text
额定电压：12 V
额定电流：0.53 A
额定扭矩：0.11 N·m
额定转速：363 rpm
峰值转速：965 rpm
峰值扭矩：0.16 N·m
峰值电流：0.8 A
相间电阻：8.53 Ω
相间电感：1.90 mH
Kv：80 rpm/V
Kt：0.2 N·m/A
极对数：11
带编码器重量：88.9 g
带编码器尺寸：Ø40 × 25 mm
编码器：AS5048A / 5600
```

### 4.3 当前判断

通过电机本体空载最高转速测试，实际最高稳定速度约：

```text
800 ~ 850 rpm
```

这更接近 GB3510 的峰值转速 `965 rpm`，而不是 GB3506 的 `1610 rpm`。

因此当前阶段优先按：

```text
Kt = 0.2 N·m/A
```

进行后续估算。

仍可通过以下方式进一步确认型号：

```text
测带编码器厚度：约 25 mm 更像 GB3510，约 21 mm 更像 GB3506
测带编码器重量：约 89 g 更像 GB3510，约 71 g 更像 GB3506
测相间电阻：约 8.53 Ω 更像 GB3510，约 6.56 Ω 更像 GB3506
```

## 5. 固件改动概述

### 5.1 主动 CAN 遥测

原厂 demo 最初只在收到 CAN ID `0x01` 时 echo 回复。已经修改为主动发送遥测。

当前 CAN 协议：

```text
0x180:
u16 vbus_raw
s16 ibus_delta_raw
s16 iq_actual_raw
s16 id_actual_raw

0x181:
s16 iq_ref_raw
s16 speed_raw
u16 tmos_raw
u8 fault_state
u8 mc_state

0x182:
u16 actual_angle_raw
u16 target_angle_raw
s16 angle_error_raw
u16 iq_abs_peak_raw

0x183:
s16 target_speed_raw
s16 ramp_speed_raw
u8 key_state
u8 ctrl_mode
u8 reserved0
u8 reserved1
```

发送周期约为：

```text
5 ms
```

### 5.2 按键阶梯调速

当前 `原厂demo`：

```text
上电校准完成后：停住，等待按键
第一次 GP45：+30 rpm
之后 GP45：每次 +10 rpm
第一次 GP33：-30 rpm
之后 GP33：每次 -10 rpm
速度限制：-200 ~ +200 rpm
松开按键后：速度保持
```

对应文件：

```text
原厂demo\User\source\ApplyFunction.c
```

当前 `空载满转速demo`：

```text
上电校准完成后：停住，等待按键
第一次 GP45：+300 rpm
之后 GP45：每次 +100 rpm
第一次 GP33：-300 rpm
之后 GP33：每次 -100 rpm
速度限制：-1800 ~ +1800 rpm
松开按键后：速度保持
```

对应文件：

```text
空载满转速demo\User\source\ApplyFunction.c
```

### 5.3 GP33 和温度 ADC 冲突

原厂代码里：

```text
GP33
```

被用作按键。

之前一度尝试把 `P3.3 / AD6` 作为 MOS 温度采样，但这会影响 GP33 按键，因此当前版本为了保证按键可靠，关闭了 `P3.3/AD6` 的模拟输入。

结果：

```text
tmos_raw 当前不可用于温度分析
```

## 6. 上位机程序

路径：

```text
pc_visualizer\can_motor_plotter.py
```

功能：

1. 使用 PCAN-USB 接收 CAN 帧。
2. 解析 `0x180~0x183` 遥测。
3. 实时绘图。
4. 保存 CSV。
5. 弹出转速监视窗口。
6. 处理 PCAN 接收队列过慢 warning，不再崩溃。

常用运行命令：

```powershell
cd C:\Users\Easyj\Documents\fu6366q电机\pc_visualizer
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000
```

带原始帧打印：

```powershell
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000 --print-raw
```

降低打印频率：

```powershell
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000 --print-raw --raw-print-hz 5
```

关闭转速监视窗口：

```powershell
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000 --no-monitor
```

模拟数据测试：

```powershell
python .\can_motor_plotter.py --simulate
```

### 6.1 PowerShell 中的提示

如果出现：

```text
uptime library not available, timestamps are relative to boot time and not to Epoch UTC
```

这是 `python-can` 的提示，不影响采集、绘图、CSV。

意思是 PCAN 底层时间戳是相对电脑开机时间，不是 UTC。当前程序使用的是本次采集开始后的相对 `time_s`，所以不影响分析。

### 6.2 PCAN receive queue warning

之前出现过：

```text
The receive queue was read too late
```

原因是 `--print-raw` 每帧打印到 PowerShell，打印太慢导致 PCAN 接收队列堆积。

已修复：

```text
--print-raw 默认限速打印
遇到 PCAN receive queue warning 不再崩溃
```

正式长时间采集建议不加 `--print-raw`。

## 7. 已完成测试和主要结论

### 7.1 CAN 通信调通

已完成：

```text
PCAN-USB 被电脑识别
python-can 可接收 PCAN 数据
固件可主动发送 CAN 遥测
上位机可保存 CSV 并绘制曲线
```

### 7.2 空载低速基线

测试范围：

```text
-200 rpm ~ +200 rpm
```

当前采用现有版本数据作为空载基线。

主要结论：

```text
正向空载 Iq 大约 +5 ~ +8 raw
反向空载 Iq 大约 -9 ~ -13 raw
平均速度基本跟目标速度一致
瞬时速度波动较大，但平均不偏
```

稳定段总体：

```text
平均速度误差：约 +0.29 rpm
平均绝对速度误差：约 10.82 rpm
95% 绝对误差：约 28.57 rpm
最大瞬时误差：约 69.95 rpm
```

说明：

```text
speed_rpm_est 是瞬时估算速度，低速/轻载下会抖动。
监视窗口看到的不吻合更多是瞬时波动，不是平均速度偏离。
```

### 7.3 空载最高转速测试

使用：

```text
空载满转速demo
```

程序目标上限：

```text
±1800 rpm
```

注意：

```text
±1800 rpm 是程序目标速度限制，不是电机自然极限。
```

实测结果：

```text
目标 -700 rpm：实际基本可跟随
目标 -800 rpm：实际平均约 -779 rpm，最高约 -838 rpm
目标 -900 rpm：实际仍约 -786 rpm
目标 -1000 到 -1800 rpm：实际基本卡在 -786 rpm 左右
```

因此：

```text
电机本体空载最高稳定转速约 800 ~ 850 rpm
更接近 GB3510，而不是 GB3506
```

当前推断：

```text
电机型号更可能为 GB3510
Kt 优先取 0.2 N·m/A
```

### 7.4 异常停机测试

曾经在一次测试最后做过非正常停机。

记录到：

```text
mc_state = 8
fault_state = 3
```

代码含义：

```text
mc_state = 8 -> mcFault
fault_state = 3 -> FaultUnderVoltage 欠压保护
```

说明系统可通过 CAN 遥测捕捉故障状态。

## 8. 当前关于同步带和工程结构的理解

用户后续实际工程会使用同步带：

```text
小轮约 16 齿
大轮约 88 齿
```

设计齿比：

```text
88 / 16 ≈ 5.5
```

考虑齿数可能误差 ±2：

```text
最小约 86 / 18 ≈ 4.78
最大约 90 / 14 ≈ 6.43
```

先按：

```text
减速比 ≈ 5.5:1
```

进行估算。

大轮中心到小轮中心约：

```text
5 cm
```

但注意：

```text
大小轮中心距不是负载力臂。
```

真正的负载力臂是：

```text
负载整体重心 到 大轮/输出轴中心 的垂直距离
```

当前所有空载基线和满转速测试都是电机本体直驱测试：

```text
不带同步带
不考虑减速比
speed_rpm_est 是电机轴转速
```

实际工程加同步带后：

```text
输出轴转速 ≈ 电机转速 / 5.5
输出轴力矩 ≈ 电机力矩 × 5.5 × 效率
```

同步带效率可先保守取：

```text
0.8 ~ 0.85
```

## 9. 云台速度和力矩估算

用户预计云台实际输出转速：

```text
20 ~ 30 rpm
```

这是合理的云台速度范围。

如果使用 5.5:1 同步带：

```text
电机 120 rpm -> 输出约 22 rpm
电机 160 rpm -> 输出约 29 rpm
电机 200 rpm -> 输出约 36 rpm
```

这说明：

```text
云台目标速度 20~30 rpm 对应电机约 110~165 rpm。
这个速度区间在已验证的 ±200 rpm 空载低速基线范围内。
```

按 GB3510：

```text
电机额定力矩：0.11 N·m
电机峰值力矩：0.16 N·m
Kt：0.2 N·m/A
```

通过 5.5:1 同步带，效率按 0.85：

```text
输出额定力矩 ≈ 0.11 × 5.5 × 0.85 ≈ 0.51 N·m
输出峰值力矩 ≈ 0.16 × 5.5 × 0.85 ≈ 0.75 N·m
```

对 1kg 负载：

```text
T = m × g × r = 1 × 9.8 × r
```

举例：

```text
r = 20 mm -> T ≈ 0.196 N·m
r = 50 mm -> T ≈ 0.49 N·m
r = 80 mm -> T ≈ 0.78 N·m
```

因此：

```text
若 1kg 负载重心控制在输出轴 50mm 以内，理论上可行。
若重心到 80mm 左右，则接近或超过峰值能力，风险较高。
```

## 10. 当前阶段建议

用户当前硬件周期快结束，且同步带机构还没完善。因此当前阶段不强求整机全链路打通，而是尽可能用电机本体测试形成完整工程结论。

建议述职主线：

```text
我建立了一套面向云台电机选型的测试平台。
完成了 CAN 通信、主动遥测、上位机实时可视化、CSV 记录。
完成了电机本体空载低速基线、最高转速边界测试、故障状态记录。
结合最高转速测试判断当前电机更接近 GB3510。
结合 Kt=0.2 N·m/A 和设计同步带 5.5:1，估算输出额定力矩约 0.51 N·m，峰值约 0.75 N·m。
对 1kg 负载，若重心在 50mm 内理论可行，超过 80mm 风险较高。
下一阶段需在同步带机构完成后进行整机负载验证。
```

## 11. 后续如果继续开展工作

如果后续硬件完善，建议按以下顺序：

1. 使用 `原厂demo`，不是 `空载满转速demo`。
2. 装同步带和大轮，但不加负载，测“机构空载基线”。
3. 加 1kg 真实负载，测目标电机速度：

```text
60 rpm  -> 输出约 11 rpm
100 rpm -> 输出约 18 rpm
120 rpm -> 输出约 22 rpm
160 rpm -> 输出约 29 rpm
200 rpm -> 输出约 36 rpm
```

4. 对比三种数据：

```text
电机本体空载 Iq
机构空载 Iq
机构带载 Iq
```

5. 计算：

```text
机构摩擦/同步带额外 Iq = 机构空载 Iq - 电机本体空载 Iq
真实负载额外 Iq = 机构带载 Iq - 机构空载 Iq
```

6. 使用 Kt 和齿比估算输出力矩：

```text
电机力矩 = Iq_A × 0.2
输出力矩 ≈ 电机力矩 × 5.5 × 0.85
```

7. 判断：

```text
连续力矩是否低于输出额定力矩
峰值力矩是否低于输出峰值力矩
是否有 1.5~2 倍安全余量
是否出现欠压、过流、失速或明显速度误差
```

## 12. 当前不确定项

仍需注意：

1. `Iq raw -> 实际电流 A` 的换算尚未完全标定。
2. `Kt = 0.2 N·m/A` 目前基于电机更接近 GB3510 的判断和厂家参数，尚未通过力臂实测标定。
3. `tmos_raw` 当前不可用，因为为了保留 GP33 按键，关闭了 P3.3/AD6 温度采样。
4. `ibus_delta_raw` 在稳定测试中多数为 0，目前不适合作为母线电流分析依据。
5. 当前所有空载数据都是电机本体数据，不能直接代表最终同步带机构。
6. 同步带齿数目前是估算，实际大轮小轮齿数可能有 ±2 齿误差。

## 13. 给后续 AI 的协助方向

后续 AI 可以优先协助：

1. 根据现有 CSV 自动生成述职报告图表。
2. 把 `fu6366q_telemetry.csv` 自动分段分析，输出速度/Iq/峰值/故障摘要。
3. 写一个 `analyze_motor_data.py`，自动生成：

```text
速度档位统计表
Iq 空载基线表
最高稳定转速判断
电机型号推断
同步带输出力矩估算
1kg 负载可行性估算
```

4. 帮助生成述职 PPT 大纲。
5. 如果后续有整机同步带数据，帮助做：

```text
机构空载 vs 电机本体空载
机构带载 vs 机构空载
输出力矩估算
安全余量判断
```

## 14. 一句话总结

当前项目已经不只是“调通 CAN 通信和采集数据”，而是完成了一个电机本体能力评估闭环：

```text
FU6366Q 主动遥测 + PCAN 上位机 + 空载基线 + 最高转速测试 + 电机型号推断 + Kt 初步采用 + 同步带输出能力估算 + 云台负载可行性判断
```

当前最重要的工程结论：

```text
电机更接近 GB3510，优先采用 Kt = 0.2 N·m/A。
电机本体空载最高稳定转速约 800~850 rpm。
实际云台输出 20~30 rpm 对应电机约 110~165 rpm，在已验证稳定工作区间内。
若后续使用 5.5:1 同步带，理论输出额定力矩约 0.51 N·m，峰值约 0.75 N·m。
1kg 负载若重心控制在输出轴 50mm 内理论可行，80mm 左右风险较高。
```

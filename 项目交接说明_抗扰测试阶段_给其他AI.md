# FU6366Q 电机项目交接说明：抗扰测试阶段

本文档用于让新的 AI 快速接上当前 FU6366Q 电机调试上下文。工作目录：

`C:\Users\Easyj\Documents\fu6366q电机`

请注意：用户明确要求，后续如果继续改代码，优先只改当前指定文件夹，不要误改其他 demo 文件夹。

## 一、项目目标

当前项目面向云台相机电机控制。最终目标不是简单让电机转，而是实现类似云台需要的运动控制能力：

- 上位机通过 CAN 发指令，控制电机转到某个角度停止。
- 按指定角速度持续旋转。
- 松开/刹停时尽快停止，尽量不要惯性滑行或抖动。
- 静止时有保持力，受到外力扰动后能自动回到原来的锁定角度。
- 当前先以电脑作为上位机，通过 CAN 做算法和功能验证。

厂家给的电机参数是 GB3510：

- 额定电压：12 V
- 额定电流：0.53 A
- 额定扭矩：0.11 N·m
- 额定转速：363 rpm
- 最大转速：965 rpm
- 堵转扭矩：0.16 N·m
- 堵转电流：0.8 A
- 相间电阻：8.53 Ω
- 相间电感：1.90 mH
- 转速常数：80 rpm/V
- 扭矩常数：0.2 N·m/A
- 极对数：11
- 编码器：AS5048a/5600

Iq 电流换算采用：

```text
Iq_A = Iq_raw * 4.5 / (32767 * 0.005 * 8)
     = Iq_raw * 0.003433

Torque_Nm = Iq_A * 0.2
```

## 二、CAN 遥测格式

当前 PC 采集脚本是：

`C:\Users\Easyj\Documents\fu6366q电机\pc_visualizer\can_motor_plotter.py`

数据一般保存到：

`C:\Users\Easyj\Documents\fu6366q电机\pc_visualizer\logs\fu6366q_telemetry.csv`

已用到的遥测帧：

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
s32 actual_angle_raw
s32 target_angle_raw
s32 angle_error_raw
s16 iq_abs_peak_raw

0x183:
s16 target_speed_raw
s16 ramp_speed_raw
u8 key_state
u8 ctrl_mode
u8 reserved0
u8 reserved1
```

`key_state` 位含义：

- bit0 = GP33
- bit1 = GP45
- bit2 = ramp/FlagONOFF

常见值：

- `4`：无按键，ramp 开
- `5`：GP33 按下
- `6`：GP45 按下
- `7`：GP33 + GP45 同时按下

状态含义：

- `mc_state=6`：正常运行
- `mc_state=8`：故障
- `fault_state=0`：无故障
- `fault_state=3`：欠压保护
- `ctrl_mode=1`：CurrentLoopMode，直接电流/力矩模式
- `ctrl_mode=2`：SpeedLoopMode，速度环
- `ctrl_mode=3`：PostionLoopMode，位置环

## 三、前面几版程序失败原因总结

用户之前反复遇到：

```text
上电自转/辨识阶段结束
↓
代码逻辑开始执行：电机转动
↓
代码让电机静止
↓
手动碰一下电机
↓
后面的电机动作不再执行，只能断电恢复
```

后来通过数据分析确认，问题不是按键没读到，也不是上层逻辑完全没跑。

### 1. 按键、目标角度、控制模式其实都正常

在旧的静止保持/抗扰动测试数据中看到：

- `fault_state=0`
- `mc_state=6`
- `ctrl_mode=3`
- `key_state=5/6` 能正常出现
- 目标角度能在约 `62.95°` 和 `122.95°` 之间切换

说明 GP33/GP45 读入正常，程序也确实在改目标角度。

### 2. 关键问题是参考电流有了，实际电流没起来

旧位置保持版本里：

- `iq_ref_raw` 达到 `±233`，约等于 `±0.8A`
- 但 `iq_actual_raw` 大多只有几十以内，实际电流约 `0.02A~0.10A`
- 实际角度几乎不动

结论：上层位置环已经要求出力，但底层没有真正打出力矩。

### 3. CurrentLoopMode 原始逻辑有覆盖问题

`AddFunction.c::Speed_response()` 原来在 `CurrentLoopMode` 分支里写了：

```c
FOC_IQREF = mcFocCtrl.Iqref;
```

但后面没有 `return`，程序继续执行速度 PI：

```c
mcFocCtrl.UQValueTemp = HW_One_PI2(mcFocCtrl.SpeedErr);
FOC_IQREF = mcFocCtrl.UQValueTemp;
```

这会覆盖直接电流命令。后来在 `抗扰测试demo\User\source\AddFunction.c` 中给 `CurrentLoopMode` 加了 `return`，这个修正应保留。

### 4. FOC 硬件电流模式可能没有被强制切入

仅设置：

```c
mcFocCtrl.CurrentLoopEn = 1;
```

不一定足够，因为 FOC 硬件的 Q/D 轴电流/电压模式由 `FOC_CR2` 的 `UQD/UDD` 位控制。

后续稳定版本中需要显式：

```c
ClrBit(FOC_CR2, UQD);
ClrBit(FOC_CR2, UDD);
FOC_IDREF = 0;
MOE = 1;
```

否则可能出现“软件变量看起来在跑，实际 PWM/电流环没真正按电流模式输出”的情况。

### 5. 欠压保护阈值太贴近 12V

用户 12V 供电时，多份日志中出现过：

- `fault_state=3`
- `mc_state=8`

这表示欠压保护或故障态。故障态下，即使按键和参考值变化，电机也不会正常输出。

因此在当前 `抗扰测试demo` 中，已把欠压保护从 `12V` 调低到 `10.5V`，恢复阈值改为 `11.0V`。这个改动只做在 `抗扰测试demo` 里。

## 四、直接电流诊断的结论

为了判断底层是否真的能出力，曾把 `抗扰测试demo` 临时改成直接电流模式：

- GP45：`+0.30A`
- GP33：`-0.30A`
- 松开：`0A`

测试现象：

- GP33 按下，电机向左高速转
- GP45 按下，电机向右高速转
- 松开后会靠惯性继续转几圈

数据结论：

- `ctrl_mode=1`
- `iq_ref_raw=±87`，约 `±0.30A`
- 速度最高约 `+875 rpm`，反向约 `-827 rpm`

这说明：

- 电机本体没坏
- 编码器/角度链路基本可用
- 功率级/PWM 能输出
- 电流环直接给定时能产生力矩

松手后有惯性是正常的，因为直接电流诊断版只是把 `Iqref=0`，相当于不给油，不是主动刹车。

## 五、当前最新代码状态

用户后来新建了文件夹：

`C:\Users\Easyj\Documents\fu6366q电机\抗扰测试demo`

用户明确要求：只在这个文件夹基础上调整，之前其他文件夹不要改。

当前已经修改的文件只有：

```text
C:\Users\Easyj\Documents\fu6366q电机\抗扰测试demo\User\source\ApplyFunction.c
C:\Users\Easyj\Documents\fu6366q电机\抗扰测试demo\User\include\Protect.h
```

没有修改旧的 `项目交接说明_给其他AI.md`。

### 1. ApplyFunction.c 当前目标

当前 `ApplyFunction.c` 已改成综合状态机：

- 上电且 MR 校准结束、进入 `mcRun` 后：锁定当前角度，进入位置保持。
- GP33 按下：低速左转。
- GP45 按下：低速右转。
- 松开按键：先切到速度环目标 `0 rpm`，主动刹停。
- 当速度降到阈值以内：切换到位置环，并把当前角度作为锁定角度。
- 静止保持时，如果手动拧动电机，应自动回到锁定角度。
- 后续再次按 GP33/GP45，不影响按键逻辑继续执行。

关键参数：

```c
#define JOG_SPEED_RPM             (50)
#define JOG_IQ_LIMIT_A            (0.5)
#define HOLD_MAX_RPM              (50)
#define HOLD_IQ_LIMIT_A           (0.5)
#define HOLD_LOCK_SPEED_RPM       (8)
```

含义：

- `JOG_SPEED_RPM=50`：按键点动速度，几十 rpm，先求稳。
- `JOG_IQ_LIMIT_A=0.5`：点动速度环最大电流。
- `HOLD_MAX_RPM=50`：位置保持纠偏时的位置环最大速度输出。
- `HOLD_IQ_LIMIT_A=0.5`：位置保持最大电流。
- `HOLD_LOCK_SPEED_RPM=8`：松手刹停后，速度低于 8 rpm 才锁角度。

### 2. ApplyFunction.c 当前核心逻辑

强制 FOC 进入电流模式：

```c
static void Apply_ForceCurrentLoopHardware(void)
{
    ClrBit(FOC_CR2, UQD);
    ClrBit(FOC_CR2, UDD);
    FOC_IDREF = 0;
    MOE = 1;
}
```

按键旋转走速度环：

```c
static void Apply_ConfigSpeedOutput(int16 target_speed)
{
    mcFocCtrl.CtrlMode = SpeedLoopMode;
    mcFocCtrl.CurrentLoopEn = 1;
    mcFocCtrl.AngleMode = AngleLoop;
    s_PI.Uk_max = I_Value(JOG_IQ_LIMIT_A);
    s_PI.Uk_min = I_Value(-JOG_IQ_LIMIT_A);
    mcSpeedRamp.TargetValue = target_speed;
    mcSpeedRamp.ActualValue = target_speed;
    mcSpeedRamp.FlagONOFF = 1;
    Apply_ForceCurrentLoopHardware();
}
```

静止保持走位置环：

```c
static void Apply_ConfigPositionHoldOutput(void)
{
    mcFocCtrl.CtrlMode = PostionLoopMode;
    mcFocCtrl.CurrentLoopEn = 1;
    mcFocCtrl.AngleMode = AngleLoop;
    mcFocCtrl.PostionLoopOut = 0;
    mcFocCtrl.SpeedErr = 0;
    PI_Position.Err = 0;
    PI_Position.ErrLast = 0;
    PI_Position.Uk_max = S_Value(HOLD_MAX_RPM);
    PI_Position.Uk_min = S_Value(-HOLD_MAX_RPM);
    s_PI.Uk_max = I_Value(HOLD_IQ_LIMIT_A);
    s_PI.Uk_min = I_Value(-HOLD_IQ_LIMIT_A);
    mcSpeedRamp.TargetValue = 0;
    mcSpeedRamp.ActualValue = 0;
    mcSpeedRamp.FlagONOFF = 1;
    Apply_ForceCurrentLoopHardware();
}
```

锁定当前角度：

```c
static void Apply_HoldCurrentAngle(void)
{
    Apply_ConfigPositionHoldOutput();
    mcFocCtrl.TargetAngle.s32 = mcFocCtrl.SensorAngle.s32;
    s_applyMode = ApplyMode_Hold;
    s_lastJogActive = 0;
}
```

松手刹停：

```c
static void Apply_StartStopToHold(void)
{
    Apply_ConfigSpeedOutput(0);
    s_applyMode = ApplyMode_StopToHold;
    s_lastJogActive = 0;
}
```

停稳判断：

```c
static uint8 Apply_IsStopped(void)
{
    return (Abs_F16(mcFocCtrl.SpeedFlt) <= S_Value(HOLD_LOCK_SPEED_RPM));
}
```

主状态机：

```c
case Run:
    if(GP33 == 0 && GP45 == 0)
    {
        if(s_applyMode == ApplyMode_JogLeft || s_applyMode == ApplyMode_JogRight || s_lastJogActive)
        {
            Apply_StartStopToHold();
        }
    }
    else if(GP45 == 0)
    {
        if(s_applyMode != ApplyMode_JogRight)
        {
            Apply_StartJog(S_Value(JOG_SPEED_RPM), ApplyMode_JogRight);
        }
    }
    else if(GP33 == 0)
    {
        if(s_applyMode != ApplyMode_JogLeft)
        {
            Apply_StartJog(S_Value(-JOG_SPEED_RPM), ApplyMode_JogLeft);
        }
    }
    else
    {
        if(s_applyMode == ApplyMode_JogLeft || s_applyMode == ApplyMode_JogRight || s_lastJogActive)
        {
            Apply_StartStopToHold();
        }
        else if(s_applyMode == ApplyMode_StopToHold)
        {
            if(Apply_IsStopped())
            {
                Apply_HoldCurrentAngle();
            }
        }
    }
    break;
```

### 3. Protect.h 当前改动

在：

`C:\Users\Easyj\Documents\fu6366q电机\抗扰测试demo\User\include\Protect.h`

欠压保护改成：

```c
#define Under_Protect_Voltage          (10.5)
#define Under_Recover_Vlotage          (11.0)
```

注意：有一次终端显示这两行被挤在同一行，但用 Python 读取真实文本时确认 `抗扰测试demo` 里是两行。后续如果编译报宏错误，优先检查这两行。

## 六、当前还没有做的事

这轮对话里没有调用 Keil 编译，所以：

```text
C:\Users\Easyj\Documents\fu6366q电机\抗扰测试demo\KeilC51\Objects\FU6366_MR.hex
```

仍可能是旧产物。用户需要在 Keil 里重新 Build，再烧录新的 hex。

## 七、烧录后的预期现象

烧录最新 `抗扰测试demo` 后，预期：

1. 上电完成自转/辨识/MR 校准后，电机进入静止保持。
2. 按 GP33：左转，速度大约几十 rpm，不应像直接电流诊断版那样高速飞转。
3. 按 GP45：右转，速度大约几十 rpm。
4. 松开按键：先主动刹到 `0 rpm`，惯性滑行应明显减少。
5. 停稳后自动锁住当前角度。
6. 静止状态下手动扭动电机，应自动回到锁住的位置。
7. 再次按 GP33/GP45，仍能正常进入点动，不应出现“手动碰一下后后续动作不执行”。

## 八、烧录后建议采集并重点看这些数据

烧录后建议继续用：

`pc_visualizer\can_motor_plotter.py`

采集：

`pc_visualizer\logs\fu6366q_telemetry.csv`

重点检查：

- 正常时：`fault_state=0`
- 正常时：`mc_state=6`
- 按键点动时：`ctrl_mode=2`
- 松手刹停后：`ctrl_mode` 应从 `2` 切到 `3`
- GP33：`key_state=5`
- GP45：`key_state=6`
- 无按键：`key_state=4`
- 按键期间：`target_speed_rpm_est` 约 `±50 rpm`
- 松手后：`target_speed_rpm_est=0`
- 切到保持后：`actual_angle_deg` 被扰动后应回到 `target_angle_deg`
- `iq_actual_raw` 应能跟随动作变化，而不是长期接近 0

如果烧录后仍然“按键没反应”，优先判断：

1. 是否重新 Build 生成了新的 `FU6366_MR.hex`。
2. CSV 里是否进入了 `fault_state=3` / `mc_state=8`。
3. `ctrl_mode` 是否按预期在 `2` 和 `3` 之间切换。
4. `iq_ref_raw` 有变化但 `iq_actual_raw` 没变化时，继续怀疑 FOC 电流模式或硬件保护。

## 九、后续调参建议

如果按键转得还是太快：

```c
#define JOG_SPEED_RPM (30)
```

如果松手后还有明显滑行：

- 先观察松手时是否进入 `ctrl_mode=2` 且 `target_speed=0`。
- 如果已经进入但停得慢，可以把 `JOG_IQ_LIMIT_A` 从 `0.5` 提到 `0.6`，但不要超过厂家堵转电流 `0.8A`。
- 也可以把 `HOLD_LOCK_SPEED_RPM` 从 `8` 降到 `5`，让它更晚切入位置保持。

如果静止保持太软、手动扭后回不去：

```c
#define HOLD_IQ_LIMIT_A (0.6)
#define HOLD_MAX_RPM    (80)
```

如果静止保持抖动：

- 降低 `HOLD_IQ_LIMIT_A`
- 降低 `HOLD_MAX_RPM`
- 或后续调整 `Customer.h` 中位置环参数 `PKP_Sw` / `PKD_Sw`

## 十、重要提醒

- 当前项目中存在多个 demo 文件夹，很多文件名相同，例如 `ApplyFunction.c`、`AddFunction.c`、`Protect.h`。后续修改前必须确认路径。
- 用户这次明确说：只基于 `抗扰测试demo` 调整，之前的文件夹不要改。
- 不要修改现有 Markdown 文件，除非用户明确要求。本文件是本轮新建的阶段性交接文档。

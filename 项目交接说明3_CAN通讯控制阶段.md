# FU6366Q 电机项目交接说明：CAN 通讯控制阶段

本文档用于让新的 AI 快速接上当前 FU6366Q 电机项目上下文。工作目录：

```text
C:\Users\Easyj\Documents\fu6366q电机
```

当前阶段新增工程目录：

```text
C:\Users\Easyj\Documents\fu6366q电机\can通讯demo
```

重要约束：后续改代码优先只改 `can通讯demo`，不要误改之前的参考 demo 文件夹。

## 一、项目目标

项目面向云台相机电机控制。前期已经在 FU6366Q 上验证过三类控制能力：

- 电流/力矩控制
- 速度控制
- 位置控制/静止保持

并且已经做过：

- 抗扰测试
- 刹停测试
- 空载速度阶跃测试
- 直接电流模式测试
- CAN 遥测打通

最终目标是：后续云台主控芯片通过 CAN 给 FU6366Q 发命令，FU6366Q 根据命令控制电机运动。当前先用电脑模拟主控芯片，通过 PCAN-USB 验证 CAN 控制协议和固件执行逻辑。

## 二、参考 demo

本轮设计 CAN 控制 demo 时参考了以下文件夹里的实现：

```text
高速转动-电流模式-有最大电流限制demo
抗扰测试demo
空载基线demo - 速度-200~200，可以递增变化
刹停测试demo
```

各自提供的经验：

- `高速转动-电流模式-有最大电流限制demo`：直接 CurrentLoopMode，按键给定 `Iqref`，验证电流环和力矩输出。
- `抗扰测试demo`：按键低速点动，松手速度环 0 rpm 刹停，停稳后位置环锁当前位置。
- `空载基线demo - 速度-200~200，可以递增变化`：速度模式阶跃、目标速度限幅。
- `刹停测试demo`：双键刹停到 0 rpm，然后切位置保持。

## 三、当前 can通讯demo 的主要内容

`can通讯demo` 是从 `抗扰测试demo` 复制出来的新工程，再加入 CAN 控制入口和 PC 上位机。

主要新增/修改文件：

```text
can通讯demo\User\source\ApplyFunction.c
can通讯demo\User\include\ApplyFunction.h
can通讯demo\User\source\ControlChat.c
can通讯demo\User\source\AddFunction.c
can通讯demo\FU68xx_Hardware_Driver\include\CAN.H
can通讯demo\FU68xx_Hardware_Driver\source\CAN.c
can通讯demo\pc_host\can_motor_gui.py
can通讯demo\启动CAN上位机.bat
can通讯demo\安装Python依赖.bat
can通讯demo\CAN协议说明.md
```

Keil 工程：

```text
can通讯demo\KeilC51\FTC6806.uvproj
```

用户每次修改固件后需要在 Keil 中重新 Build 并烧录。

## 四、CAN 协议

### 1. 遥测帧

保留前期已有遥测帧：

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
u8 can_mode
u8 can_status
```

状态含义：

```text
mc_state=6：正常运行
mc_state=8：故障
fault_state=0：无故障
fault_state=3：欠压保护
ctrl_mode=1：CurrentLoopMode
ctrl_mode=2：SpeedLoopMode
ctrl_mode=3：PostionLoopMode
```

`can_status` bit：

```text
bit0 ready
bit1 active
bit2 timeout
bit3 braking
```

### 2. 控制命令帧

PC/主控 -> 电机：

```text
CAN ID = 0x200
```

8 字节，标准帧，小端：

```text
byte0     mode
byte1     flags
byte2-3   target_s16
byte4-5   limit_s16
byte6     timeout_50ms
byte7     seq
```

`mode`：

```text
0 = disable/coast
1 = current，target 单位 mA
2 = speed，target 单位 rpm
3 = position，target 单位 0.01 degree
4 = brake_to_hold
5 = hold current angle
```

`flags`：

```text
bit0 = enable CAN control
bit1 = relative angle，position 模式下 target 为相对角度
```

`limit_s16` 当前作为电流限制，单位 mA。固件内最大限制为 `600mA`。

`timeout_50ms` 单位为 50ms。填 0 时固件使用默认 200ms。GUI 填 `Timeout ms=5000` 时，发送值为 100，固件内部也允许最大 5000ms。

### 3. 命令应答帧

电机 -> PC：

```text
CAN ID = 0x184
```

8 字节：

```text
byte0 mode
byte1 status
byte2 fault_state
byte3 mc_state
byte4 ctrl_mode
byte5 seq
byte6-7 age_ms
```

## 五、当前安全机制

固件安全策略：

- 未完成 `MRFinish && mcState == mcRun` 前，不执行运动命令。
- 电流命令最大限制为 `±600mA`。
- 速度命令最大限制为 `±500rpm`。
- 位置模式最大纠偏速度限制为 `80rpm`。
- 位置/保持模式电流限制最大为 `600mA`。
- `Current` 和 `Speed` 属于连续模式，依赖超时保护；超时后自动 `brake_to_hold`。
- `Position`、`Brake Hold`、`Hold` 是一次性命令，不依赖心跳续发。
- `Disable` 目前实现为 0 电流输出，控制链路保持活着，后续命令应能恢复控制。

注意：实际停车优先使用 `Brake Hold`，不要把 `Disable` 当常规停车命令。

## 六、Python 上位机

GUI 文件：

```text
can通讯demo\pc_host\can_motor_gui.py
```

双击启动：

```text
can通讯demo\启动CAN上位机.bat
```

依赖安装：

```text
can通讯demo\安装Python依赖.bat
```

默认连接参数：

```text
interface = pcan
channel   = PCAN_USBBUS1
bitrate   = 500000
```

GUI 按钮含义：

- `Hold`：锁住当前位置。
- `Speed`：按 `Speed rpm` 输入框目标转速运行。
- `Brake Hold`：速度环目标 0 rpm 主动刹停，停稳后锁当前位置。
- `Position`：按 `Position deg` 输入框走位置命令。勾选 `relative` 时表示相对当前位置运动。
- `Current`：直接给 Q 轴电流，单位 mA，主要用于调试力矩/电流环。
- `Disable`：0 电流输出，不主动保持、不主动刹停。
- `Ping`：旧 `0x01` 链路测试，不控制电机。

GUI 会自动记录 CAN raw 日志：

```text
can通讯demo\pc_host\logs\can_host_YYYYMMDD_HHMMSS.csv
```

当前日志格式包含：

```text
pc_time_s,direction,can_id,data_hex
```

其中 `direction=tx` 的 `0x200` 是 GUI 发出的控制命令，`direction=rx` 是接收到的遥测/应答。

## 七、本轮发现并修复的问题

### 1. pc_visualizer 没记录到数据

用户曾尝试用：

```text
pc_visualizer\can_motor_plotter.py
```

与 GUI 同时采集数据，但生成的：

```text
pc_visualizer\logs\fu6366q_telemetry.csv
```

只有表头，没有样本行。

原因大概率是 GUI 已经占用了 `PCAN_USBBUS1`，`pc_visualizer` 没收到帧。

后续建议直接使用 GUI 自带日志 `pc_host\logs\can_host_*.csv`，不要同时打开两个 CAN 程序抢同一个 PCAN 通道。

### 2. Position 不按输入角度运动

最开始 GUI 的 Position 会受 `repeat command` 和 timeout 影响，导致相对位置命令反复刷新或过早超时。

已修复：

- Position 变成一次性命令。
- Position/Brake/Hold 不再依赖心跳续发。
- GUI 对相对 Position 加入 `-180°..180°` 范围提醒。

重要限制：现有位置环误差使用 `_Q15Limit`，单次误差最大约 `±180°`。因此相对位置命令超过 180° 时，表现可能像都在转半圈左右。大角度运动建议拆成多次命令，或使用速度模式连续转动。

日志曾确认 Position 实际生效，例如目标角度从约 `321.5°` 变为 `21.5°`，相当于相对转动约 `+60°`。

### 3. Disable 后后续按钮不好使

最初 `Disable` 把 `mcSpeedRamp.FlagONOFF` 关掉了，导致后续控制恢复不顺。

已修复：

- Disable 改成 CurrentLoopMode 下 0 电流输出。
- `mcSpeedRamp.FlagONOFF` 保持开启。
- 控制链路保持活着，后续 `Speed/Hold/Position` 应能恢复。

### 4. Timeout ms=5000 实际不到 5000ms

用户反馈：不勾选 repeat command，速度模式下 `Timeout ms=5000`，实际坚持不到 5000ms。

日志确认：速度命令大约 1 秒左右进入 timeout/brake。

原因：

- 原协议 `byte6` 单位是 10ms，1 字节最大只能表达 2550ms。
- 固件内 `CAN_MAX_TIMEOUT_MS` 还写成了 1000ms。

已修复：

- `byte6` 改为 `timeout_50ms`。
- 固件最大 timeout 改为 5000ms。
- GUI 按 50ms 单位发送 timeout。

注意：这个修复需要重新 Keil Build 并烧录固件，仅重启 GUI 不够。

## 八、推荐测试流程

烧录最新 `can通讯demo` 后，建议按以下顺序测试：

```text
1. 打开 GUI，Connect。
2. 点 Hold，确认电机锁住当前位置。
3. Position deg = 30.0，勾选 relative，点 Position。
4. 点 Brake Hold，确认主动刹停并保持。
5. Speed rpm = 50，勾选 repeat command，点 Speed。
6. 点 Brake Hold。
7. 取消 repeat command，Timeout ms = 5000，点 Speed，观察是否约 5 秒后自动刹停保持。
8. 点 Disable，再点 Hold 或 Speed 50，确认可以恢复控制。
```

如果出现异常，保存并分析：

```text
can通讯demo\pc_host\logs\can_host_*.csv
```

重点看：

- `tx 0x200`：GUI 实际发送的 mode/target/timeout/seq。
- `rx 0x184`：固件是否应答命令。
- `rx 0x183`：`ctrl_mode`、`can_mode`、`can_status` 是否按预期变化。
- `rx 0x181`：`fault_state`、`mc_state` 是否正常。
- `rx 0x182`：`actual_angle` 和 `target_angle` 是否变化。

## 九、当前仍需注意的点

- 所有新增内容都在 `can通讯demo`，之前 demo 文件夹不要动。
- 每次修改固件后必须 Keil Build 并烧录。
- `Current` 是直接力矩/电流模式，不管速度和位置，调试时建议小电流、短时间使用。
- 云台实际应用主要会用 `Speed`、`Position`、`Hold`、`Brake Hold`，电流模式更多是底层诊断。
- `Brake Hold` 比 `Disable` 更适合做常规停车。
- 如果 12V 电源偏低，仍需注意 `fault_state=3` 欠压保护。


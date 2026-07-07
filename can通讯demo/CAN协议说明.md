# FU6366Q CAN 控制协议

本 demo 基于 `抗扰测试demo` 复制生成，新增 PC 通过 CAN 控制电机的命令通道。

## 发送方向

- PC -> 电机：`0x200`
- 电机 -> PC 命令应答：`0x184`
- 电机 -> PC 遥测：继续保留 `0x180`、`0x181`、`0x182`、`0x183`

## 0x200 控制命令

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

`limit_s16` 当前作为电流限制，单位 mA。固件内限制最大为 600 mA。

`timeout_50ms` 为命令超时时间，单位 50 ms；填 0 时固件使用默认 200 ms。

## 安全机制

- 未完成 `MRFinish && mcRun` 前，固件不执行运动命令。
- 速度命令最大限制为 `±500 rpm`。
- 电流命令和速度/位置模式电流限制最大为 `600 mA`。
- CAN 命令超时后，固件进入 `brake_to_hold`：先速度环目标 0 rpm，再停稳后锁当前位置。
- GUI 里的 `Timeout ms` 最大可用到 5000 ms；固件内部也会限制到 5000 ms。
- 上位机 GUI 对 current/speed 默认每 50 ms 周期续发命令。
- position/brake_to_hold/hold 是一次性命令，不依赖心跳续发。
- disable 是 0 电流输出，不主动保持位置，后续命令仍可直接恢复控制。
- relative position 单次建议使用 `-180°..180°`；当前位置环误差本身按半圈限幅，超过该范围应拆成多次位置命令或使用速度模式。

## Python 上位机

双击：

```text
启动CAN上位机.bat
```

GUI 连接 CAN 后会自动记录 raw CAN 日志：

```text
pc_host\logs\can_host_YYYYMMDD_HHMMSS.csv
```

日志会记录 `rx` 和 `tx`，其中 `tx 0x200` 就是 GUI 发出的控制命令。

默认参数：

```text
interface = pcan
channel   = PCAN_USBBUS1
bitrate   = 500000
```

如果缺少依赖：

```powershell
python -m pip install --user -r .\pc_host\requirements.txt
```

# Iq actual current conversion demo

这个目录是独立的上位机测试版本，用来把 FU6366Q 上报的 `Iq raw` 换算为实际电流 `A`，并基于 `Kt = 0.2 N*m/A` 估算电机本体输出力矩。

原始 `pc_visualizer` 目录没有被修改。

## 默认换算参数

这些参数来自当前 `原厂demo` 的 `Customer.h` / `Parameter.h`：

```text
HW_ADC_REF   = 4.5 V
HW_RSHUNT    = 0.005 ohm
HW_AMPGAIN   = AMP8x = 8
Kt           = 0.2 N*m/A
```

换算关系：

```text
Iq_A = Iq_raw * 4.5 / (32767 * 0.005 * 8)
     = Iq_raw * 0.003433

Torque_Nm = Iq_A * 0.2
```

例子：

```text
Iq_raw = 100  -> Iq = 0.343 A -> torque = 0.0687 N*m
Iq_raw = 300  -> Iq = 1.030 A -> torque = 0.2060 N*m
```

## 运行

```powershell
cd C:\Users\Easyj\Documents\fu6366q电机\Iq实际电流换算demo
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000 --print-raw
```

CSV 默认保存到：

```text
logs/fu6366q_telemetry.csv
```

相比旧版 CSV，这版新增：

```text
iq_actual_A
id_actual_A
iq_ref_A
torque_actual_Nm
torque_ref_Nm
iq_abs_peak_A
torque_abs_peak_Nm
```

实时窗口新增 `Current / torque` 曲线，速度监视框会显示：

```text
Iq=...A T=...Nm Tpk=...Nm
```

如果后面确认 Kt 或采样参数不同，可以直接用命令行覆盖，例如：

```powershell
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000 --torque-constant-nm-per-a 0.08
```

# FU6366Q CAN telemetry plotter

This folder contains the PC-side live plotter and CSV logger for FU6366Q baseline and load tests.

## Firmware protocol

The modified firmware actively sends standard CAN frames every 5 ms. Multi-byte values are little-endian.

| CAN ID | Bytes | Meaning |
| --- | --- | --- |
| `0x180` | `u16 vbus_raw, s16 ibus_delta_raw, s16 iq_actual_raw, s16 id_actual_raw` | DC bus, bus current delta, Q/D current feedback |
| `0x181` | `s16 iq_ref_raw, s16 speed_raw, u16 tmos_raw, u8 fault_state, u8 mc_state` | FOC current reference, actual speed, raw MOS temperature, states |
| `0x182` | `u16 actual_angle_raw, u16 target_angle_raw, s16 angle_error_raw, u16 iq_abs_peak_raw` | angle telemetry and 5 ms Iq peak |
| `0x183` | `s16 target_speed_raw, s16 ramp_speed_raw, u8 key_state, u8 ctrl_mode, u8 reserved0, u8 reserved1` | speed-step target, ramp output, key bits, control mode |

`key_state` bits:

| Bit | Meaning |
| --- | --- |
| `0x01` | GP33 is pressed |
| `0x02` | GP45 is pressed |
| `0x04` | motor speed ramp is enabled |

The logger records raw values first. It also adds estimated speed in rpm and angle in degrees for quick inspection. Convert current and torque only after confirming calibration constants and Kt.

## Speed-step baseline test

The current firmware in `原厂demo` uses this no-load baseline behavior:

| Action | Result |
| --- | --- |
| Power on and calibration finishes | Stop and wait for a key |
| First `GP45` press | Start speed loop at `+30 rpm` |
| Later `GP45` press | Increase target speed by `10 rpm` |
| First `GP33` press | Start speed loop at `-30 rpm` |
| Later `GP33` press | Decrease target speed by `10 rpm` |
| Hold a key | Only one step is applied until the key is released |
| Limit | Target speed is clamped to `-200..+200 rpm` |

During a baseline sweep, hold each speed point for several seconds before pressing the next step. The CSV fields `target_speed_rpm_est`, `ramp_speed_rpm_est`, `speed_rpm_est`, `iq_actual_raw`, and `iq_abs_peak_raw` are the main fields for grouping and analysis.

## Run with PCAN-USB

```powershell
cd C:\Users\Easyj\Documents\fu6366q电机\pc_visualizer
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000 --print-raw
```

The plot window should show repeated frames from `0x180`, `0x181`, `0x182`, and `0x183`. CSV data is saved to:

```text
logs/fu6366q_telemetry.csv
```

By default, the same command also opens a separate speed monitor window. It shows actual speed, target speed, ramp speed, current step hold time, motor state, fault state, key state, and frame counts.

`--print-raw` is rate-limited by default so PowerShell printing does not block CAN reception. The default limit is 20 lines per second, while CSV logging and plotting still process all received frames.

To change the raw print rate:

```powershell
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000 --print-raw --raw-print-hz 5
```

To print every frame for a short diagnostic run:

```powershell
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000 --print-raw --print-raw-all
```

To disable the speed monitor:

```powershell
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000 --print-raw --no-monitor
```

## Echo test

The original factory demo only replied after receiving standard CAN ID `0x01`. This command is still useful for checking the CAN link:

```powershell
python .\can_motor_plotter.py --interface pcan --channel PCAN_USBBUS1 --bitrate 500000 --print-raw --send-ping
```

For active telemetry collection, do not use `--send-ping`.

## Simulated data

```powershell
python .\can_motor_plotter.py --simulate
```

This only tests the PC plotter and does not read the motor.

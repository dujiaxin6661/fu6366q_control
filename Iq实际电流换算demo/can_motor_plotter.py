import argparse
import csv
import math
import random
import struct
import time
from collections import deque
from dataclasses import dataclass, replace
from pathlib import Path

import matplotlib.animation as animation
import matplotlib.pyplot as plt


TELEMETRY_ADC_ID = 0x180
TELEMETRY_FOC_ID = 0x181
TELEMETRY_ANGLE_ID = 0x182
TELEMETRY_SPEED_ID = 0x183
MOTOR_SPEED_BASE_RPM = 4000.0
DEFAULT_CURRENT_ADC_REF_V = 4.5
DEFAULT_CURRENT_SHUNT_OHM = 0.005
DEFAULT_CURRENT_AMP_GAIN = 8.0
DEFAULT_TORQUE_CONSTANT_NM_PER_A = 0.2


@dataclass
class Sample:
    t: float
    vbus_raw: int | None = None
    ibus_delta_raw: int | None = None
    iq_actual_raw: int | None = None
    id_actual_raw: int | None = None
    iq_ref_raw: int | None = None
    speed_raw: int | None = None
    target_speed_raw: int | None = None
    ramp_speed_raw: int | None = None
    tmos_raw: int | None = None
    fault_state: int | None = None
    mc_state: int | None = None
    key_state: int | None = None
    ctrl_mode: int | None = None
    actual_angle_raw: int | None = None
    target_angle_raw: int | None = None
    angle_error_raw: int | None = None
    iq_abs_peak_raw: int | None = None


class TelemetryState:
    def __init__(self) -> None:
        self.latest = Sample(t=0.0)

    def update_from_can(self, arbitration_id: int, data: bytes, t: float) -> Sample | None:
        if arbitration_id == TELEMETRY_ADC_ID and len(data) >= 8:
            vbus_raw, ibus_delta_raw, iq_actual_raw, id_actual_raw = struct.unpack_from("<Hhhh", data)
            self.latest = replace(
                self.latest,
                t=t,
                vbus_raw=vbus_raw,
                ibus_delta_raw=ibus_delta_raw,
                iq_actual_raw=iq_actual_raw,
                id_actual_raw=id_actual_raw,
            )
            return self.latest

        if arbitration_id == TELEMETRY_FOC_ID and len(data) >= 8:
            iq_ref_raw, speed_raw, tmos_raw, fault_state, mc_state = struct.unpack_from("<hhHBB", data)
            self.latest = replace(
                self.latest,
                t=t,
                iq_ref_raw=iq_ref_raw,
                speed_raw=speed_raw,
                tmos_raw=tmos_raw,
                fault_state=fault_state,
                mc_state=mc_state,
            )
            return self.latest

        if arbitration_id == TELEMETRY_ANGLE_ID and len(data) >= 8:
            actual_angle_raw, target_angle_raw, angle_error_raw, iq_abs_peak_raw = struct.unpack_from("<HHhH", data)
            self.latest = replace(
                self.latest,
                t=t,
                actual_angle_raw=actual_angle_raw,
                target_angle_raw=target_angle_raw,
                angle_error_raw=angle_error_raw,
                iq_abs_peak_raw=iq_abs_peak_raw,
            )
            return self.latest

        if arbitration_id == TELEMETRY_SPEED_ID and len(data) >= 8:
            target_speed_raw, ramp_speed_raw, key_state, ctrl_mode, _reserved0, _reserved1 = struct.unpack_from(
                "<hhBBBB", data
            )
            self.latest = replace(
                self.latest,
                t=t,
                target_speed_raw=target_speed_raw,
                ramp_speed_raw=ramp_speed_raw,
                key_state=key_state,
                ctrl_mode=ctrl_mode,
            )
            return self.latest

        return None


class CsvLogger:
    def __init__(
        self,
        path: Path | None,
        current_adc_ref_v: float,
        current_shunt_ohm: float,
        current_amp_gain: float,
        torque_constant_nm_per_a: float,
    ) -> None:
        self.file = None
        self.writer = None
        self.current_adc_ref_v = current_adc_ref_v
        self.current_shunt_ohm = current_shunt_ohm
        self.current_amp_gain = current_amp_gain
        self.torque_constant_nm_per_a = torque_constant_nm_per_a
        if path is not None:
            path.parent.mkdir(parents=True, exist_ok=True)
            self.file = path.open("w", newline="", encoding="utf-8")
            self.writer = csv.writer(self.file)
            self.writer.writerow(
                [
                    "time_s",
                    "vbus_raw",
                    "ibus_delta_raw",
                    "iq_actual_raw",
                    "iq_actual_A",
                    "id_actual_raw",
                    "id_actual_A",
                    "iq_ref_raw",
                    "iq_ref_A",
                    "torque_actual_Nm",
                    "torque_ref_Nm",
                    "speed_raw",
                    "speed_rpm_est",
                    "target_speed_raw",
                    "target_speed_rpm_est",
                    "ramp_speed_raw",
                    "ramp_speed_rpm_est",
                    "tmos_raw",
                    "fault_state",
                    "mc_state",
                    "key_state",
                    "ctrl_mode",
                    "actual_angle_raw",
                    "target_angle_raw",
                    "angle_error_raw",
                    "actual_angle_deg",
                    "target_angle_deg",
                    "angle_error_deg",
                    "iq_abs_peak_raw",
                    "iq_abs_peak_A",
                    "torque_abs_peak_Nm",
                ]
            )

    def write(self, sample: Sample) -> None:
        if self.writer is None:
            return
        self.writer.writerow(
            [
                f"{sample.t:.6f}",
                sample.vbus_raw,
                sample.ibus_delta_raw,
                sample.iq_actual_raw,
                current_raw_to_a(
                    sample.iq_actual_raw,
                    self.current_adc_ref_v,
                    self.current_shunt_ohm,
                    self.current_amp_gain,
                ),
                sample.id_actual_raw,
                current_raw_to_a(
                    sample.id_actual_raw,
                    self.current_adc_ref_v,
                    self.current_shunt_ohm,
                    self.current_amp_gain,
                ),
                sample.iq_ref_raw,
                current_raw_to_a(
                    sample.iq_ref_raw,
                    self.current_adc_ref_v,
                    self.current_shunt_ohm,
                    self.current_amp_gain,
                ),
                torque_from_current_raw(
                    sample.iq_actual_raw,
                    self.current_adc_ref_v,
                    self.current_shunt_ohm,
                    self.current_amp_gain,
                    self.torque_constant_nm_per_a,
                ),
                torque_from_current_raw(
                    sample.iq_ref_raw,
                    self.current_adc_ref_v,
                    self.current_shunt_ohm,
                    self.current_amp_gain,
                    self.torque_constant_nm_per_a,
                ),
                sample.speed_raw,
                speed_raw_to_rpm(sample.speed_raw),
                sample.target_speed_raw,
                speed_raw_to_rpm(sample.target_speed_raw),
                sample.ramp_speed_raw,
                speed_raw_to_rpm(sample.ramp_speed_raw),
                sample.tmos_raw,
                sample.fault_state,
                sample.mc_state,
                sample.key_state,
                sample.ctrl_mode,
                sample.actual_angle_raw,
                sample.target_angle_raw,
                sample.angle_error_raw,
                angle_raw_to_deg(sample.actual_angle_raw),
                angle_raw_to_deg(sample.target_angle_raw),
                angle_error_raw_to_deg(sample.angle_error_raw),
                sample.iq_abs_peak_raw,
                current_raw_to_a(
                    sample.iq_abs_peak_raw,
                    self.current_adc_ref_v,
                    self.current_shunt_ohm,
                    self.current_amp_gain,
                ),
                torque_from_current_raw(
                    sample.iq_abs_peak_raw,
                    self.current_adc_ref_v,
                    self.current_shunt_ohm,
                    self.current_amp_gain,
                    self.torque_constant_nm_per_a,
                ),
            ]
        )

    def close(self) -> None:
        if self.file is not None:
            self.file.close()


def simulated_messages(start: float):
    while True:
        now = time.monotonic() - start
        vbus = int(15000 + 100 * math.sin(now * 0.5) + random.randint(-20, 20))
        ibus = int(700 * math.sin(now * 2.0) + random.randint(-40, 40))
        iq = int(1200 + 300 * math.sin(now * 1.4))
        id_value = int(60 * math.sin(now * 0.9))
        iq_ref = int(1300 + 200 * math.sin(now * 0.8))
        speed_raw = int(32767 * (900 + 350 * math.sin(now * 0.35)) / MOTOR_SPEED_BASE_RPM)
        target_speed_raw = int(32767 * (1000 + 200 * math.sin(now * 0.12)) / MOTOR_SPEED_BASE_RPM)
        ramp_speed_raw = int(0.8 * target_speed_raw + 0.2 * speed_raw)
        tmos = int(7800 + 50 * math.sin(now * 0.15))
        actual_angle = int((now * 3000) % 65536)
        target_angle = int((actual_angle + 1200 * math.sin(now * 0.7)) % 65536)
        angle_error = int(1200 * math.sin(now * 0.7))
        iq_peak = abs(iq) + random.randint(0, 80)

        yield TELEMETRY_ADC_ID, struct.pack("<Hhhh", vbus, ibus, iq, id_value), now
        yield TELEMETRY_FOC_ID, struct.pack("<hhHBB", iq_ref, speed_raw, tmos, 0, 6), now
        yield TELEMETRY_ANGLE_ID, struct.pack("<HHhH", actual_angle, target_angle, angle_error, iq_peak), now
        yield TELEMETRY_SPEED_ID, struct.pack("<hhBBBB", target_speed_raw, ramp_speed_raw, 0x04, 1, 0, 0), now
        time.sleep(0.02)


def can_messages(
    interface: str,
    channel: str,
    bitrate: int,
    print_raw: bool,
    print_raw_all: bool,
    raw_print_hz: float,
    send_ping: bool,
    ping_id: int,
    ping_interval: float,
):
    import can

    with can.Bus(interface=interface, channel=channel, bitrate=bitrate) as bus:
        start = time.monotonic()
        next_ping = start
        next_raw_print = start
        next_error_print = start
        skipped_raw_prints = 0
        raw_print_interval = 0.0 if raw_print_hz <= 0 else 1.0 / raw_print_hz
        while True:
            now_abs = time.monotonic()
            if send_ping and now_abs >= next_ping:
                msg = can.Message(
                    arbitration_id=ping_id,
                    data=[0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07],
                    is_extended_id=False,
                )
                try:
                    bus.send(msg, timeout=0.0)
                    if print_raw:
                        print(f"{now_abs - start:10.6f} TX ID=0x{ping_id:X} DLC=8 DATA=00 01 02 03 04 05 06 07", flush=True)
                except can.CanError as exc:
                    if print_raw:
                        print(f"{now_abs - start:10.6f} TX FAILED: {exc}", flush=True)
                next_ping = now_abs + ping_interval

            try:
                msg = bus.recv(timeout=0.0)
            except can.CanOperationError as exc:
                if print_raw and now_abs >= next_error_print:
                    print(f"{now_abs - start:10.6f} RX WARNING: {exc}", flush=True)
                    next_error_print = now_abs + 1.0
                yield None
                continue
            if msg is None:
                yield None
                continue
            if msg.is_error_frame:
                if print_raw:
                    now = time.monotonic() - start
                    print(f"{now:10.6f} ERROR FRAME", flush=True)
                continue
            if msg.is_remote_frame:
                if print_raw:
                    now = time.monotonic() - start
                    print(f"{now:10.6f} REMOTE FRAME ID=0x{msg.arbitration_id:X}", flush=True)
                continue
            now = time.monotonic() - start
            if print_raw:
                if print_raw_all or now_abs >= next_raw_print:
                    data_text = " ".join(f"{byte:02X}" for byte in msg.data)
                    direction = "RX" if getattr(msg, "is_rx", True) else "TX"
                    skipped_text = f" skipped_prints={skipped_raw_prints}" if skipped_raw_prints else ""
                    print(
                        f"{now:10.6f} {direction} ID=0x{msg.arbitration_id:X} DLC={msg.dlc} DATA={data_text}{skipped_text}",
                        flush=True,
                    )
                    skipped_raw_prints = 0
                    next_raw_print = now_abs + raw_print_interval
                else:
                    skipped_raw_prints += 1
            yield msg.arbitration_id, bytes(msg.data), now


def none_to_nan(value: int | None) -> float:
    return float("nan") if value is None else float(value)


def current_raw_to_a(
    value: int | None,
    adc_ref_v: float = DEFAULT_CURRENT_ADC_REF_V,
    shunt_ohm: float = DEFAULT_CURRENT_SHUNT_OHM,
    amp_gain: float = DEFAULT_CURRENT_AMP_GAIN,
) -> float | None:
    if value is None:
        return None
    return value * adc_ref_v / (32767.0 * shunt_ohm * amp_gain)


def torque_from_current_a(value: float | None, torque_constant_nm_per_a: float) -> float | None:
    if value is None:
        return None
    return value * torque_constant_nm_per_a


def torque_from_current_raw(
    value: int | None,
    adc_ref_v: float = DEFAULT_CURRENT_ADC_REF_V,
    shunt_ohm: float = DEFAULT_CURRENT_SHUNT_OHM,
    amp_gain: float = DEFAULT_CURRENT_AMP_GAIN,
    torque_constant_nm_per_a: float = DEFAULT_TORQUE_CONSTANT_NM_PER_A,
) -> float | None:
    return torque_from_current_a(current_raw_to_a(value, adc_ref_v, shunt_ohm, amp_gain), torque_constant_nm_per_a)


def speed_raw_to_rpm(value: int | None) -> float | None:
    if value is None:
        return None
    return value * MOTOR_SPEED_BASE_RPM / 32767.0


def angle_raw_to_deg(value: int | None) -> float | None:
    if value is None:
        return None
    return value * 360.0 / 65536.0


def angle_error_raw_to_deg(value: int | None) -> float | None:
    if value is None:
        return None
    return value * 360.0 / 65536.0


def format_optional(value: float | None, digits: int = 1) -> str:
    if value is None:
        return "--"
    return f"{value:.{digits}f}"


def target_speed_bin(value: float | None, bin_rpm: float) -> float | None:
    if value is None:
        return None
    return round(value / bin_rpm) * bin_rpm


def format_key_state(value: int | None) -> str:
    if value is None:
        return "--"
    parts = []
    if value & 0x01:
        parts.append("GP33")
    if value & 0x02:
        parts.append("GP45")
    if value & 0x04:
        parts.append("RUN")
    return "+".join(parts) if parts else "IDLE"


def run(args: argparse.Namespace) -> None:
    state = TelemetryState()
    logger = CsvLogger(
        args.csv,
        args.current_adc_ref_v,
        args.current_shunt_ohm,
        args.current_amp_gain,
        args.torque_constant_nm_per_a,
    )
    samples: deque[Sample] = deque(maxlen=args.window_samples)
    raw_frames = 0
    parsed_frames = 0
    start_wall = time.monotonic()
    current_target_bin: float | None = None
    target_hold_start: float | None = None

    if args.simulate:
        source = simulated_messages(time.monotonic())
    else:
        source = can_messages(
            args.interface,
            args.channel,
            args.bitrate,
            args.print_raw,
            args.print_raw_all,
            args.raw_print_hz,
            args.send_ping,
            args.ping_id,
            args.ping_interval,
        )

    fig, (ax_adc, ax_current, ax_speed, ax_angle) = plt.subplots(4, 1, sharex=True, figsize=(11, 9))
    fig.canvas.manager.set_window_title("FU6366Q current conversion telemetry")

    adc_lines = {
        "Vbus raw": ax_adc.plot([], [], label="Vbus raw", linewidth=1.2)[0],
        "Ibus delta raw": ax_adc.plot([], [], label="Ibus delta raw", linewidth=1.2)[0],
        "Tmos raw": ax_adc.plot([], [], label="Tmos raw", linewidth=1.2)[0],
    }
    current_lines = {
        "Iq actual A": ax_current.plot([], [], label="Iq actual A", linewidth=1.3)[0],
        "Iq ref A": ax_current.plot([], [], label="Iq ref A", linewidth=1.1)[0],
        "Id actual A": ax_current.plot([], [], label="Id actual A", linewidth=1.0)[0],
        "Torque actual Nm": ax_current.plot([], [], label="Torque actual Nm", linewidth=1.1, linestyle="--")[0],
        "Torque peak Nm": ax_current.plot([], [], label="Torque peak Nm", linewidth=1.0, linestyle=":")[0],
    }
    speed_lines = {
        "Speed rpm est": ax_speed.plot([], [], label="Speed rpm est", linewidth=1.1)[0],
        "Target speed rpm": ax_speed.plot([], [], label="Target speed rpm", linewidth=1.1, linestyle="--")[0],
        "Ramp speed rpm": ax_speed.plot([], [], label="Ramp speed rpm", linewidth=1.0, linestyle=":")[0],
    }
    angle_lines = {
        "Actual angle deg": ax_angle.plot([], [], label="Actual angle deg", linewidth=1.1)[0],
        "Target angle deg": ax_angle.plot([], [], label="Target angle deg", linewidth=1.1)[0],
        "Angle error deg": ax_angle.plot([], [], label="Angle error deg", linewidth=1.1)[0],
    }

    ax_adc.set_ylabel("ADC raw")
    ax_current.set_ylabel("A / Nm")
    ax_speed.set_ylabel("rpm")
    ax_angle.set_ylabel("Angle deg")
    ax_angle.set_xlabel("Time(s)")
    ax_adc.grid(True, alpha=0.25)
    ax_current.grid(True, alpha=0.25)
    ax_speed.grid(True, alpha=0.25)
    ax_angle.grid(True, alpha=0.25)
    ax_adc.legend(loc="upper right")
    ax_current.legend(loc="upper right")
    ax_speed.legend(loc="upper right")
    ax_angle.legend(loc="upper right")
    status_text = fig.text(0.01, 0.01, "Starting...", fontsize=9)

    monitor_artists = []
    if not args.no_monitor:
        monitor_fig, monitor_ax = plt.subplots(figsize=(5.4, 3.4))
        monitor_fig.canvas.manager.set_window_title("FU6366Q speed monitor")
        monitor_ax.set_axis_off()
        monitor_ax.set_facecolor("#f7f8fa")
        monitor_fig.patch.set_facecolor("#f7f8fa")
        monitor_title = monitor_ax.text(
            0.04, 0.92, "FU6366Q Speed Monitor", fontsize=13, weight="bold", transform=monitor_ax.transAxes
        )
        monitor_speed = monitor_ax.text(
            0.04, 0.68, "Actual -- rpm", fontsize=30, weight="bold", transform=monitor_ax.transAxes
        )
        monitor_target = monitor_ax.text(
            0.04, 0.50, "Target -- rpm   Ramp -- rpm", fontsize=14, transform=monitor_ax.transAxes
        )
        monitor_hold = monitor_ax.text(
            0.04, 0.36, "Hold -- s", fontsize=18, weight="bold", transform=monitor_ax.transAxes
        )
        monitor_detail = monitor_ax.text(
            0.04, 0.18, "state=-- fault=-- key=-- raw=0 parsed=0", fontsize=11, transform=monitor_ax.transAxes
        )
        monitor_hint = monitor_ax.text(
            0.04, 0.07, "Hold time resets when target speed step changes.", fontsize=9, color="#58606a", transform=monitor_ax.transAxes
        )
        monitor_artists = [
            monitor_title,
            monitor_speed,
            monitor_target,
            monitor_hold,
            monitor_detail,
            monitor_hint,
        ]

    def update(_frame):
        nonlocal raw_frames, parsed_frames, current_target_bin, target_hold_start
        deadline = time.monotonic() + 0.03
        while time.monotonic() < deadline:
            item = next(source)
            if item is None:
                break
            arbitration_id, data, t = item
            raw_frames += 1
            sample = state.update_from_can(arbitration_id, data, t)
            if sample is not None:
                parsed_frames += 1
                samples.append(sample)
                logger.write(sample)

        if not samples:
            elapsed = time.monotonic() - start_wall
            status_text.set_text(
                f"Waiting for CAN frames... elapsed={elapsed:.1f}s raw={raw_frames} parsed={parsed_frames}"
            )
            if monitor_artists:
                monitor_artists[1].set_text("Actual -- rpm")
                monitor_artists[2].set_text("Target -- rpm   Ramp -- rpm")
                monitor_artists[3].set_text("Hold -- s")
                monitor_artists[4].set_text(f"Waiting... elapsed={elapsed:.1f}s raw={raw_frames} parsed={parsed_frames}")
            return [status_text, *monitor_artists]

        xs = [s.t for s in samples]
        adc_lines["Vbus raw"].set_data(xs, [none_to_nan(s.vbus_raw) for s in samples])
        adc_lines["Ibus delta raw"].set_data(xs, [none_to_nan(s.ibus_delta_raw) for s in samples])
        adc_lines["Tmos raw"].set_data(xs, [none_to_nan(s.tmos_raw) for s in samples])
        current_lines["Iq actual A"].set_data(
            xs,
            [
                none_to_nan(current_raw_to_a(s.iq_actual_raw, args.current_adc_ref_v, args.current_shunt_ohm, args.current_amp_gain))
                for s in samples
            ],
        )
        current_lines["Iq ref A"].set_data(
            xs,
            [
                none_to_nan(current_raw_to_a(s.iq_ref_raw, args.current_adc_ref_v, args.current_shunt_ohm, args.current_amp_gain))
                for s in samples
            ],
        )
        current_lines["Id actual A"].set_data(
            xs,
            [
                none_to_nan(current_raw_to_a(s.id_actual_raw, args.current_adc_ref_v, args.current_shunt_ohm, args.current_amp_gain))
                for s in samples
            ],
        )
        current_lines["Torque actual Nm"].set_data(
            xs,
            [
                none_to_nan(
                    torque_from_current_raw(
                        s.iq_actual_raw,
                        args.current_adc_ref_v,
                        args.current_shunt_ohm,
                        args.current_amp_gain,
                        args.torque_constant_nm_per_a,
                    )
                )
                for s in samples
            ],
        )
        current_lines["Torque peak Nm"].set_data(
            xs,
            [
                none_to_nan(
                    torque_from_current_raw(
                        s.iq_abs_peak_raw,
                        args.current_adc_ref_v,
                        args.current_shunt_ohm,
                        args.current_amp_gain,
                        args.torque_constant_nm_per_a,
                    )
                )
                for s in samples
            ],
        )
        speed_lines["Speed rpm est"].set_data(xs, [none_to_nan(speed_raw_to_rpm(s.speed_raw)) for s in samples])
        speed_lines["Target speed rpm"].set_data(xs, [none_to_nan(speed_raw_to_rpm(s.target_speed_raw)) for s in samples])
        speed_lines["Ramp speed rpm"].set_data(xs, [none_to_nan(speed_raw_to_rpm(s.ramp_speed_raw)) for s in samples])
        angle_lines["Actual angle deg"].set_data(xs, [none_to_nan(angle_raw_to_deg(s.actual_angle_raw)) for s in samples])
        angle_lines["Target angle deg"].set_data(xs, [none_to_nan(angle_raw_to_deg(s.target_angle_raw)) for s in samples])
        angle_lines["Angle error deg"].set_data(xs, [none_to_nan(angle_error_raw_to_deg(s.angle_error_raw)) for s in samples])

        left = max(0.0, xs[-1] - args.window_seconds)
        right = max(args.window_seconds, xs[-1])
        ax_adc.set_xlim(left, right)

        for ax in (ax_adc, ax_current, ax_speed, ax_angle):
            ax.relim()
            ax.autoscale_view(scalex=False, scaley=True)

        status_text.set_text(
            f"raw={raw_frames} parsed={parsed_frames} latest={xs[-1]:.3f}s "
            f"fault={samples[-1].fault_state} state={samples[-1].mc_state} "
            f"target={format_optional(speed_raw_to_rpm(samples[-1].target_speed_raw))}rpm"
        )

        latest = samples[-1]
        actual_rpm = speed_raw_to_rpm(latest.speed_raw)
        target_rpm = speed_raw_to_rpm(latest.target_speed_raw)
        ramp_rpm = speed_raw_to_rpm(latest.ramp_speed_raw)
        iq_actual_a = current_raw_to_a(
            latest.iq_actual_raw, args.current_adc_ref_v, args.current_shunt_ohm, args.current_amp_gain
        )
        torque_actual_nm = torque_from_current_a(iq_actual_a, args.torque_constant_nm_per_a)
        torque_peak_nm = torque_from_current_raw(
            latest.iq_abs_peak_raw,
            args.current_adc_ref_v,
            args.current_shunt_ohm,
            args.current_amp_gain,
            args.torque_constant_nm_per_a,
        )
        latest_target_bin = target_speed_bin(target_rpm, args.hold_bin_rpm)
        if latest_target_bin != current_target_bin:
            current_target_bin = latest_target_bin
            target_hold_start = latest.t
        hold_seconds = None if target_hold_start is None else max(0.0, latest.t - target_hold_start)

        if monitor_artists:
            monitor_artists[1].set_text(f"Actual {format_optional(actual_rpm, 1)} rpm")
            monitor_artists[2].set_text(
                f"Target {format_optional(target_rpm, 1)} rpm   Ramp {format_optional(ramp_rpm, 1)} rpm"
            )
            monitor_artists[3].set_text(
                f"Hold {format_optional(hold_seconds, 1)} s   Step {format_optional(current_target_bin, 0)} rpm"
            )
            monitor_artists[4].set_text(
                f"state={latest.mc_state} fault={latest.fault_state} key={format_key_state(latest.key_state)} "
                f"Iq={format_optional(iq_actual_a, 3)}A T={format_optional(torque_actual_nm, 3)}Nm "
                f"Tpk={format_optional(torque_peak_nm, 3)}Nm raw={raw_frames} parsed={parsed_frames}"
            )
            monitor_artists[0].figure.canvas.draw_idle()

        return [*adc_lines.values(), *current_lines.values(), *speed_lines.values(), *angle_lines.values(), status_text, *monitor_artists]

    try:
        ani = animation.FuncAnimation(fig, update, interval=50, blit=False, cache_frame_data=False)
        plt.show()
    finally:
        logger.close()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="FU6366Q CAN telemetry plotter")
    parser.add_argument("--simulate", action="store_true", help="show fake data without CAN hardware")
    parser.add_argument("--interface", default="pcan", help="python-can interface name")
    parser.add_argument("--channel", default="PCAN_USBBUS1", help="CAN adapter channel")
    parser.add_argument("--bitrate", type=int, default=500000, help="CAN bitrate")
    parser.add_argument("--print-raw", action="store_true", help="print received CAN frames with rate limiting")
    parser.add_argument("--print-raw-all", action="store_true", help="print every received CAN frame; not recommended for long runs")
    parser.add_argument("--raw-print-hz", type=float, default=20.0, help="maximum lines per second printed by --print-raw")
    parser.add_argument("--send-ping", action="store_true", help="periodically send a standard CAN test frame")
    parser.add_argument("--ping-id", type=lambda value: int(value, 0), default=0x01, help="standard CAN ID used by --send-ping")
    parser.add_argument("--ping-interval", type=float, default=0.5, help="seconds between ping frames")
    parser.add_argument("--csv", type=Path, default=Path("logs/fu6366q_telemetry.csv"))
    parser.add_argument("--window-seconds", type=float, default=10.0)
    parser.add_argument("--window-samples", type=int, default=5000)
    parser.add_argument("--no-monitor", action="store_true", help="disable the separate speed monitor window")
    parser.add_argument("--hold-bin-rpm", type=float, default=10.0, help="target-speed bin size used for hold-time reset")
    parser.add_argument(
        "--current-adc-ref-v",
        type=float,
        default=DEFAULT_CURRENT_ADC_REF_V,
        help="ADC reference voltage used by FU6366Q current conversion",
    )
    parser.add_argument(
        "--current-shunt-ohm",
        type=float,
        default=DEFAULT_CURRENT_SHUNT_OHM,
        help="phase-current shunt resistance in ohms",
    )
    parser.add_argument(
        "--current-amp-gain",
        type=float,
        default=DEFAULT_CURRENT_AMP_GAIN,
        help="phase-current amplifier gain",
    )
    parser.add_argument(
        "--torque-constant-nm-per-a",
        type=float,
        default=DEFAULT_TORQUE_CONSTANT_NM_PER_A,
        help="motor torque constant Kt in N*m/A",
    )
    return parser.parse_args()


if __name__ == "__main__":
    run(parse_args())

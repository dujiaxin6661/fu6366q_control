from __future__ import annotations

import queue
import csv
import struct
import threading
import time
import tkinter as tk
from dataclasses import dataclass, field
from datetime import datetime
from pathlib import Path
from tkinter import messagebox, ttk

try:
    import can
except Exception:  # pragma: no cover - shown in GUI at runtime
    can = None


CMD_ID = 0x200
ACK_ID = 0x184
TEL_ADC_ID = 0x180
TEL_FOC_ID = 0x181
TEL_ANGLE_ID = 0x182
TEL_SPEED_ID = 0x183

FLAG_ENABLE = 0x01
FLAG_RELATIVE = 0x02

MODE_NAMES = {
    0: "disable/coast",
    1: "current",
    2: "speed",
    3: "position",
    4: "brake_to_hold",
    5: "hold",
}

CTRL_MODE_NAMES = {
    1: "current",
    2: "speed",
    3: "position",
}

STATUS_BITS = [
    (0x01, "ready"),
    (0x02, "active"),
    (0x04, "timeout"),
    (0x08, "braking"),
]


@dataclass
class Telemetry:
    connected: bool = False
    raw_frames: int = 0
    parsed_frames: int = 0
    last_rx_s: float = 0.0
    vbus_raw: int | None = None
    iq_actual_raw: int | None = None
    iq_ref_raw: int | None = None
    speed_raw: int | None = None
    target_speed_raw: int | None = None
    actual_angle_raw: int | None = None
    target_angle_raw: int | None = None
    fault_state: int | None = None
    mc_state: int | None = None
    key_state: int | None = None
    ctrl_mode: int | None = None
    can_mode: int | None = None
    can_status: int | None = None
    ack: dict[str, int] = field(default_factory=dict)

    @staticmethod
    def speed_rpm(raw: int | None) -> float | None:
        if raw is None:
            return None
        return raw * 4000.0 / 32767.0

    @staticmethod
    def angle_deg(raw: int | None) -> float | None:
        if raw is None:
            return None
        return raw * 360.0 / 65536.0

    @staticmethod
    def iq_a(raw: int | None) -> float | None:
        if raw is None:
            return None
        return raw * 0.003433


class MotorCanGui(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title("FU6366Q CAN Host")
        self.geometry("760x520")
        self.minsize(720, 480)

        self.rx_queue: queue.Queue[tuple[int, bytes, float]] = queue.Queue()
        self.stop_event = threading.Event()
        self.rx_thread: threading.Thread | None = None
        self.bus = None
        self.log_handle = None
        self.log_writer = None
        self.seq = 0
        self.last_command: tuple[int, int, int, int, int] | None = None
        self.last_command_time = 0.0
        self.telemetry = Telemetry()

        self.interface_var = tk.StringVar(value="pcan")
        self.channel_var = tk.StringVar(value="PCAN_USBBUS1")
        self.bitrate_var = tk.IntVar(value=500000)
        self.current_var = tk.IntVar(value=100)
        self.speed_var = tk.IntVar(value=50)
        self.position_var = tk.DoubleVar(value=30.0)
        self.limit_var = tk.IntVar(value=500)
        self.timeout_var = tk.IntVar(value=200)
        self.relative_var = tk.BooleanVar(value=True)
        self.repeat_var = tk.BooleanVar(value=True)
        self.status_var = tk.StringVar(value="not connected")
        self.last_tx_var = tk.StringVar(value="none")

        self.value_vars: dict[str, tk.StringVar] = {}
        self._build_ui()
        self.after(50, self._pump_rx)
        self.after(50, self._periodic_send)
        self.protocol("WM_DELETE_WINDOW", self._on_close)

    def _build_ui(self) -> None:
        outer = ttk.Frame(self, padding=12)
        outer.pack(fill=tk.BOTH, expand=True)

        conn = ttk.LabelFrame(outer, text="CAN")
        conn.pack(fill=tk.X)
        ttk.Label(conn, text="Interface").grid(row=0, column=0, sticky="w", padx=6, pady=6)
        ttk.Entry(conn, textvariable=self.interface_var, width=12).grid(row=0, column=1, sticky="w", padx=6)
        ttk.Label(conn, text="Channel").grid(row=0, column=2, sticky="w", padx=6)
        ttk.Entry(conn, textvariable=self.channel_var, width=18).grid(row=0, column=3, sticky="w", padx=6)
        ttk.Label(conn, text="Bitrate").grid(row=0, column=4, sticky="w", padx=6)
        ttk.Entry(conn, textvariable=self.bitrate_var, width=10).grid(row=0, column=5, sticky="w", padx=6)
        ttk.Button(conn, text="Connect", command=self.connect).grid(row=0, column=6, padx=6)
        ttk.Button(conn, text="Disconnect", command=self.disconnect).grid(row=0, column=7, padx=6)
        ttk.Label(conn, textvariable=self.status_var).grid(row=1, column=0, columnspan=8, sticky="w", padx=6, pady=(0, 6))

        cmd = ttk.LabelFrame(outer, text="Command")
        cmd.pack(fill=tk.X, pady=(10, 0))
        ttk.Label(cmd, text="Current mA").grid(row=0, column=0, sticky="w", padx=6, pady=6)
        ttk.Entry(cmd, textvariable=self.current_var, width=10).grid(row=0, column=1, padx=6)
        ttk.Label(cmd, text="Speed rpm").grid(row=0, column=2, sticky="w", padx=6)
        ttk.Entry(cmd, textvariable=self.speed_var, width=10).grid(row=0, column=3, padx=6)
        ttk.Label(cmd, text="Position deg").grid(row=0, column=4, sticky="w", padx=6)
        ttk.Entry(cmd, textvariable=self.position_var, width=10).grid(row=0, column=5, padx=6)
        ttk.Checkbutton(cmd, text="relative", variable=self.relative_var).grid(row=0, column=6, padx=6)

        ttk.Label(cmd, text="Iq limit mA").grid(row=1, column=0, sticky="w", padx=6, pady=6)
        ttk.Entry(cmd, textvariable=self.limit_var, width=10).grid(row=1, column=1, padx=6)
        ttk.Label(cmd, text="Timeout ms").grid(row=1, column=2, sticky="w", padx=6)
        ttk.Entry(cmd, textvariable=self.timeout_var, width=10).grid(row=1, column=3, padx=6)
        ttk.Checkbutton(cmd, text="repeat command", variable=self.repeat_var).grid(row=1, column=4, columnspan=2, sticky="w", padx=6)
        ttk.Label(cmd, textvariable=self.last_tx_var).grid(row=1, column=6, columnspan=2, sticky="w", padx=6)

        buttons = ttk.Frame(cmd)
        buttons.grid(row=2, column=0, columnspan=8, sticky="ew", padx=6, pady=(4, 8))
        for idx in range(8):
            buttons.columnconfigure(idx, weight=1)

        ttk.Button(buttons, text="Disable", command=self.send_disable).grid(row=0, column=0, sticky="ew", padx=3)
        ttk.Button(buttons, text="Current", command=self.send_current).grid(row=0, column=1, sticky="ew", padx=3)
        ttk.Button(buttons, text="Speed", command=self.send_speed).grid(row=0, column=2, sticky="ew", padx=3)
        ttk.Button(buttons, text="Position", command=self.send_position).grid(row=0, column=3, sticky="ew", padx=3)
        ttk.Button(buttons, text="Brake Hold", command=self.send_brake_hold).grid(row=0, column=4, sticky="ew", padx=3)
        ttk.Button(buttons, text="Hold", command=self.send_hold).grid(row=0, column=5, sticky="ew", padx=3)
        ttk.Button(buttons, text="Ping", command=self.send_ping).grid(row=0, column=6, sticky="ew", padx=3)

        tel = ttk.LabelFrame(outer, text="Telemetry")
        tel.pack(fill=tk.BOTH, expand=True, pady=(10, 0))
        fields = [
            "rx",
            "fault/mc",
            "ctrl/can",
            "speed",
            "target speed",
            "angle",
            "target angle",
            "iq actual",
            "iq ref",
            "ack",
        ]
        for i, name in enumerate(fields):
            ttk.Label(tel, text=name).grid(row=i, column=0, sticky="w", padx=8, pady=4)
            var = tk.StringVar(value="-")
            self.value_vars[name] = var
            ttk.Label(tel, textvariable=var).grid(row=i, column=1, sticky="w", padx=8, pady=4)
        tel.columnconfigure(1, weight=1)

    def connect(self) -> None:
        if can is None:
            messagebox.showerror("python-can missing", "Please install python-can first: pip install python-can")
            return
        if self.bus is not None:
            return
        try:
            self.bus = can.Bus(
                interface=self.interface_var.get().strip(),
                channel=self.channel_var.get().strip(),
                bitrate=int(self.bitrate_var.get()),
            )
        except Exception as exc:
            messagebox.showerror("CAN connect failed", str(exc))
            return
        self.stop_event.clear()
        self.rx_thread = threading.Thread(target=self._rx_loop, daemon=True)
        self.rx_thread.start()
        self.telemetry.connected = True
        self._open_log()
        self.status_var.set("connected")

    def disconnect(self) -> None:
        self.last_command = None
        self.stop_event.set()
        if self.rx_thread is not None:
            self.rx_thread.join(timeout=0.5)
            self.rx_thread = None
        if self.bus is not None:
            try:
                self.bus.shutdown()
            except Exception:
                pass
            self.bus = None
        self._close_log()
        self.telemetry.connected = False
        self.status_var.set("not connected")

    def _open_log(self) -> None:
        log_dir = Path(__file__).resolve().parent / "logs"
        log_dir.mkdir(exist_ok=True)
        stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        path = log_dir / f"can_host_{stamp}.csv"
        self.log_handle = path.open("w", newline="", encoding="utf-8")
        self.log_writer = csv.writer(self.log_handle)
        self.log_writer.writerow(["pc_time_s", "direction", "can_id", "data_hex"])
        self.log_handle.flush()
        self.last_tx_var.set(f"log={path.name}")

    def _close_log(self) -> None:
        if self.log_handle is not None:
            try:
                self.log_handle.flush()
                self.log_handle.close()
            except Exception:
                pass
        self.log_handle = None
        self.log_writer = None

    def _log_frame(self, arbitration_id: int, data: bytes, stamp: float) -> None:
        if self.log_writer is None:
            return
        self.log_writer.writerow([f"{stamp:.6f}", "rx", f"0x{arbitration_id:03X}", data.hex(" ")])
        if self.telemetry.raw_frames % 100 == 0 and self.log_handle is not None:
            self.log_handle.flush()

    def _log_tx_frame(self, arbitration_id: int, data: bytes) -> None:
        if self.log_writer is None:
            return
        self.log_writer.writerow([f"{time.time():.6f}", "tx", f"0x{arbitration_id:03X}", data.hex(" ")])
        if self.log_handle is not None:
            self.log_handle.flush()

    def _rx_loop(self) -> None:
        while not self.stop_event.is_set():
            try:
                msg = self.bus.recv(timeout=0.1) if self.bus is not None else None
            except Exception as exc:
                self.rx_queue.put((-1, str(exc).encode(errors="ignore"), time.time()))
                break
            if msg is None:
                continue
            if msg.is_extended_id:
                continue
            self.rx_queue.put((msg.arbitration_id, bytes(msg.data), time.time()))

    def _pump_rx(self) -> None:
        while True:
            try:
                arbitration_id, data, stamp = self.rx_queue.get_nowait()
            except queue.Empty:
                break
            if arbitration_id == -1:
                self.status_var.set(f"RX error: {data.decode(errors='ignore')}")
                self.disconnect()
                break
            self._parse_frame(arbitration_id, data, stamp)
        self._refresh_telemetry()
        self.after(50, self._pump_rx)

    def _parse_frame(self, arbitration_id: int, data: bytes, stamp: float) -> None:
        self.telemetry.raw_frames += 1
        self.telemetry.last_rx_s = stamp
        self._log_frame(arbitration_id, data, stamp)
        try:
            if arbitration_id == TEL_ADC_ID and len(data) >= 8:
                vbus, _ibus, iq_actual, _id_actual = struct.unpack_from("<Hhhh", data)
                self.telemetry.vbus_raw = vbus
                self.telemetry.iq_actual_raw = iq_actual
            elif arbitration_id == TEL_FOC_ID and len(data) >= 8:
                iq_ref, speed, _tmos, fault, mc_state = struct.unpack_from("<hhHBB", data)
                self.telemetry.iq_ref_raw = iq_ref
                self.telemetry.speed_raw = speed
                self.telemetry.fault_state = fault
                self.telemetry.mc_state = mc_state
            elif arbitration_id == TEL_ANGLE_ID and len(data) >= 8:
                actual, target, _err, _peak = struct.unpack_from("<HHhH", data)
                self.telemetry.actual_angle_raw = actual
                self.telemetry.target_angle_raw = target
            elif arbitration_id == TEL_SPEED_ID and len(data) >= 8:
                target_speed, _ramp, key_state, ctrl_mode, can_mode, can_status = struct.unpack_from("<hhBBBB", data)
                self.telemetry.target_speed_raw = target_speed
                self.telemetry.key_state = key_state
                self.telemetry.ctrl_mode = ctrl_mode
                self.telemetry.can_mode = can_mode
                self.telemetry.can_status = can_status
            elif arbitration_id == ACK_ID and len(data) >= 8:
                mode, status, fault, mc_state, ctrl_mode, seq, age = struct.unpack_from("<BBBBBBH", data)
                self.telemetry.ack = {
                    "mode": mode,
                    "status": status,
                    "fault": fault,
                    "mc_state": mc_state,
                    "ctrl_mode": ctrl_mode,
                    "seq": seq,
                    "age": age,
                }
            else:
                return
            self.telemetry.parsed_frames += 1
        except struct.error:
            return

    def _refresh_telemetry(self) -> None:
        tel = self.telemetry
        age = time.time() - tel.last_rx_s if tel.last_rx_s else 0.0
        self.value_vars["rx"].set(f"raw={tel.raw_frames} parsed={tel.parsed_frames} age={age:.2f}s")
        self.value_vars["fault/mc"].set(f"fault={tel.fault_state} mc={tel.mc_state}")
        ctrl = CTRL_MODE_NAMES.get(tel.ctrl_mode, str(tel.ctrl_mode))
        can_mode = MODE_NAMES.get(tel.can_mode, str(tel.can_mode))
        status = self._status_text(tel.can_status)
        self.value_vars["ctrl/can"].set(f"ctrl={ctrl} can={can_mode} [{status}]")
        self.value_vars["speed"].set(self._fmt_float(Telemetry.speed_rpm(tel.speed_raw), " rpm"))
        self.value_vars["target speed"].set(self._fmt_float(Telemetry.speed_rpm(tel.target_speed_raw), " rpm"))
        self.value_vars["angle"].set(self._fmt_float(Telemetry.angle_deg(tel.actual_angle_raw), " deg"))
        self.value_vars["target angle"].set(self._fmt_float(Telemetry.angle_deg(tel.target_angle_raw), " deg"))
        self.value_vars["iq actual"].set(self._fmt_float(Telemetry.iq_a(tel.iq_actual_raw), " A"))
        self.value_vars["iq ref"].set(self._fmt_float(Telemetry.iq_a(tel.iq_ref_raw), " A"))
        if tel.ack:
            ack_mode = MODE_NAMES.get(tel.ack["mode"], str(tel.ack["mode"]))
            ack_status = self._status_text(tel.ack["status"])
            self.value_vars["ack"].set(
                f"seq={tel.ack['seq']} mode={ack_mode} status=[{ack_status}] fault={tel.ack['fault']} age={tel.ack['age']}ms"
            )
        else:
            self.value_vars["ack"].set("-")

    @staticmethod
    def _fmt_float(value: float | None, suffix: str) -> str:
        if value is None:
            return "-"
        return f"{value:.2f}{suffix}"

    @staticmethod
    def _status_text(value: int | None) -> str:
        if value is None:
            return "-"
        names = [name for bit, name in STATUS_BITS if value & bit]
        return ",".join(names) if names else "none"

    def _send(self, mode: int, flags: int, target: int, limit_ma: int, timeout_ms: int, repeat: bool) -> None:
        if self.bus is None:
            messagebox.showwarning("Not connected", "Connect CAN first.")
            return
        self.seq = (self.seq + 1) & 0xFF
        timeout_units = max(0, min(255, int(round(int(timeout_ms) / 50.0))))
        payload = struct.pack(
            "<BBhhBB",
            mode & 0xFF,
            flags & 0xFF,
            max(-32768, min(32767, int(target))),
            max(-32768, min(32767, int(limit_ma))),
            timeout_units,
            self.seq,
        )
        msg = can.Message(arbitration_id=CMD_ID, data=payload, is_extended_id=False)
        try:
            self.bus.send(msg, timeout=0.05)
            self._log_tx_frame(CMD_ID, payload)
        except Exception as exc:
            self.status_var.set(f"TX error: {exc}")
            return
        self.last_tx_var.set(
            f"tx seq={self.seq} {MODE_NAMES.get(mode, mode)} target={target} limit={limit_ma}mA"
        )
        self.last_command_time = time.time()
        self.last_command = (mode, flags, target, limit_ma, timeout_ms) if repeat else None

    def _periodic_send(self) -> None:
        if self.bus is not None and self.repeat_var.get() and self.last_command is not None:
            if time.time() - self.last_command_time >= 0.05:
                mode, flags, target, limit_ma, timeout_ms = self.last_command
                self._send(mode, flags, target, limit_ma, timeout_ms, repeat=True)
        self.after(50, self._periodic_send)

    def send_disable(self) -> None:
        self.last_command = None
        self._send(0, 0, 0, 0, self.timeout_var.get(), repeat=False)

    def send_current(self) -> None:
        self._send(1, FLAG_ENABLE, self.current_var.get(), self.limit_var.get(), self.timeout_var.get(), repeat=True)

    def send_speed(self) -> None:
        self._send(2, FLAG_ENABLE, self.speed_var.get(), self.limit_var.get(), self.timeout_var.get(), repeat=True)

    def send_position(self) -> None:
        self.last_command = None
        flags = FLAG_ENABLE | (FLAG_RELATIVE if self.relative_var.get() else 0)
        centi_deg = int(round(float(self.position_var.get()) * 100.0))
        if self.relative_var.get() and abs(centi_deg) > 18000:
            messagebox.showwarning("Position range", "Relative position is limited to -180..180 degrees per command.")
            return
        self._send(3, flags, centi_deg, self.limit_var.get(), self.timeout_var.get(), repeat=False)

    def send_brake_hold(self) -> None:
        self.last_command = None
        self._send(4, FLAG_ENABLE, 0, self.limit_var.get(), self.timeout_var.get(), repeat=False)

    def send_hold(self) -> None:
        self.last_command = None
        self._send(5, FLAG_ENABLE, 0, self.limit_var.get(), self.timeout_var.get(), repeat=False)

    def send_ping(self) -> None:
        if self.bus is None:
            messagebox.showwarning("Not connected", "Connect CAN first.")
            return
        msg = can.Message(arbitration_id=0x01, data=bytes([1, 2, 3, 4, 5, 6, 7, 8]), is_extended_id=False)
        try:
            self.bus.send(msg, timeout=0.05)
            self._log_tx_frame(0x01, bytes(msg.data))
            self.last_tx_var.set("tx ping 0x01")
        except Exception as exc:
            self.status_var.set(f"TX error: {exc}")

    def _on_close(self) -> None:
        self.disconnect()
        self.destroy()


if __name__ == "__main__":
    app = MotorCanGui()
    app.mainloop()

#!/usr/bin/env python3
import argparse
import math
import sys
import time

try:
    import serial
except ImportError:
    print("Missing dependency: pyserial\nInstall with: pip install pyserial", file=sys.stderr)
    raise


HEADER = "critical_timestamp_ms,chassis_timestamp_ms,wheel_speed_timestamp_ms,temp1_c,temp2_c,flow1_lpm,flow2_lpm,susp1,susp2,susp3,susp4,steering_angle_deg,speed_kmh"


def parse_args():
    parser = argparse.ArgumentParser(description="Send test lines over a telemetry radio serial port.")
    parser.add_argument("--port", required=True, help="Serial port (e.g. COM7 or /dev/cu.usbserial-D30AF7R6)")
    parser.add_argument("--baud", type=int, default=57600, help="UART baud rate")
    parser.add_argument("--message", default="laptop telemetry test ping", help="Line to send")
    parser.add_argument("--csv", action="store_true", help="Send fake DAQ CSV rows instead of the message text")
    parser.add_argument("--send-header", action="store_true", help="Send the CSV header before data rows")
    parser.add_argument("--interval", type=float, default=0.1, help="Seconds between sends")
    parser.add_argument("--count", type=int, default=0, help="Number of lines to send, or 0 forever")
    return parser.parse_args()


def fake_csv_row(elapsed_s):
    timestamp_ms = int(elapsed_s * 1000)
    temp1_c = 42.0 + 4.0 * math.sin(elapsed_s / 4.0)
    temp2_c = 39.0 + 3.0 * math.sin(elapsed_s / 5.0)
    flow1_lpm = 12.0 + 2.0 * math.sin(elapsed_s / 3.0)
    flow2_lpm = 11.0 + 1.5 * math.sin(elapsed_s / 3.5)
    susp1 = 1.8 + 0.5 * math.sin(elapsed_s * 2.0)
    susp2 = 1.7 + 0.5 * math.sin(elapsed_s * 2.1)
    susp3 = 1.9 + 0.5 * math.sin(elapsed_s * 1.9)
    susp4 = 1.6 + 0.5 * math.sin(elapsed_s * 2.2)
    steering_angle_deg = 180.0 + 90.0 * math.sin(elapsed_s / 2.0)
    speed_kmh = max(0.0, 45.0 + 25.0 * math.sin(elapsed_s / 6.0))

    return (
        f"{timestamp_ms},{timestamp_ms},{timestamp_ms},"
        f"{temp1_c:.1f},{temp2_c:.1f},"
        f"{flow1_lpm:.2f},{flow2_lpm:.2f},"
        f"{susp1:.3f},{susp2:.3f},{susp3:.3f},{susp4:.3f},"
        f"{steering_angle_deg:.1f},{speed_kmh:.2f}"
    )


def main():
    args = parse_args()
    sent = 0

    print(f"Opening serial: port={args.port} baud={args.baud}")
    with serial.Serial(args.port, args.baud, timeout=1) as ser:
        start = time.monotonic()
        if args.csv and args.send_header:
            ser.write((HEADER + "\n").encode("utf-8"))
            ser.flush()
            print(f"sent: {HEADER}")

        while args.count == 0 or sent < args.count:
            if args.csv:
                line = fake_csv_row(time.monotonic() - start)
            else:
                line = f"{args.message} {sent + 1}"
            ser.write((line + "\n").encode("utf-8"))
            ser.flush()
            print(f"sent: {line}")
            sent += 1
            time.sleep(args.interval)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
import argparse
import datetime as dt
import os
import sys
import time

try:
    import serial
except ImportError:
    print("Missing dependency: pyserial\nInstall with: pip install pyserial", file=sys.stderr)
    raise

EXPECTED_FIELDS = 13
HEADER = "critical_timestamp_ms,chassis_timestamp_ms,wheel_speed_timestamp_ms,temp1_c,temp2_c,flow1_lpm,flow2_lpm,susp1,susp2,susp3,susp4,steering_angle_deg,speed_kmh"


def parse_args():
    parser = argparse.ArgumentParser(description="Receive DAQ telemetry CSV over serial and log to file.")
    parser.add_argument("--port", required=True, help="Serial port (e.g. COM7 or /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=57600, help="UART baud rate")
    parser.add_argument("--outdir", default="antenna/logs", help="Directory for output CSV files")
    parser.add_argument("--prefix", default="telemetry", help="Output filename prefix")
    parser.add_argument("--no-print", action="store_true", help="Disable live console printing")
    return parser.parse_args()


def make_output_path(outdir, prefix):
    os.makedirs(outdir, exist_ok=True)
    stamp = dt.datetime.now().strftime("%Y%m%d_%H%M%S")
    return os.path.join(outdir, f"{prefix}_{stamp}.csv")


def main():
    args = parse_args()
    out_path = make_output_path(args.outdir, args.prefix)

    print(f"Opening serial: port={args.port} baud={args.baud}")
    print(f"Logging to: {out_path}")

    with serial.Serial(args.port, args.baud, timeout=1) as ser, open(out_path, "w", encoding="utf-8", newline="") as f:
        f.write(HEADER + "\n")
        f.flush()

        valid_rows = 0
        dropped_rows = 0

        while True:
            try:
                raw = ser.readline()
                if not raw:
                    continue

                line = raw.decode("utf-8", errors="replace").strip()
                if not line:
                    continue

                parts = line.split(",")
                if len(parts) != EXPECTED_FIELDS:
                    dropped_rows += 1
                    if not args.no_print:
                        print(f"[drop {dropped_rows}] {line}")
                    continue

                f.write(line + "\n")
                valid_rows += 1

                if valid_rows % 10 == 0:
                    f.flush()

                if not args.no_print:
                    print(line)

            except KeyboardInterrupt:
                print("\nStopped by user.")
                break
            except serial.SerialException as exc:
                print(f"Serial error: {exc}")
                print("Retrying in 2 seconds...")
                time.sleep(2)
                break

        f.flush()
        print(f"Saved {valid_rows} valid rows to {out_path}")
        print(f"Dropped {dropped_rows} malformed rows")


if __name__ == "__main__":
    main()

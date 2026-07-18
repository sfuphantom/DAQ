#!/usr/bin/env python3
import argparse
import sys
import time

try:
    import serial
except ImportError:
    print("Missing dependency: pyserial\nInstall with: pip install pyserial", file=sys.stderr)
    raise


def parse_args():
    parser = argparse.ArgumentParser(description="Print raw telemetry serial lines.")
    parser.add_argument("--port", required=True, help="Serial port, for example /dev/cu.usbserial-D30AF4AU")
    parser.add_argument("--baud", type=int, default=57600, help="UART baud rate")
    parser.add_argument("--hex", action="store_true", help="Print received bytes as hex instead of text")
    return parser.parse_args()


def main():
    args = parse_args()

    print(f"Opening serial: port={args.port} baud={args.baud}")
    print("Printing raw received telemetry. Press Ctrl+C to stop.")

    try:
        with serial.Serial(args.port, args.baud, timeout=1) as ser:
            while True:
                raw = ser.readline()
                if not raw:
                    continue

                if args.hex:
                    print(raw.hex(" "))
                else:
                    print(raw.decode("utf-8", errors="replace").rstrip())

    except KeyboardInterrupt:
        print("\nStopped by user.")
    except serial.SerialException as exc:
        print(f"Serial error: {exc}", file=sys.stderr)
        time.sleep(1)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

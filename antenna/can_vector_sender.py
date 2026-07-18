#!/usr/bin/env python3
import argparse
import sys
import time

try:
    import can
except ImportError:
    print("Missing dependency: python-can\nInstall with: pip install python-can", file=sys.stderr)
    raise


def parse_args():
    parser = argparse.ArgumentParser(description="Send simple DAQ test CAN frames with a Vector interface.")
    parser.add_argument("--channel", type=int, default=0, help="Vector channel index")
    parser.add_argument("--bitrate", type=int, default=500000, help="CAN bitrate")
    parser.add_argument("--interval", type=float, default=0.5, help="Seconds between frame groups")
    parser.add_argument("--count", type=int, default=0, help="Number of frame groups to send, or 0 forever")
    return parser.parse_args()


def main():
    args = parse_args()

    bus = can.interface.Bus(
        interface="vector",
        channel=args.channel,
        bitrate=args.bitrate,
    )

    sent = 0
    print("Sending Vector CAN test frames. Ctrl+C to stop.")
    print("0x200 DLC=2 DATA=34 12")
    print("0x100 DLC=1 DATA=00/01 alternating")

    try:
        while args.count == 0 or sent < args.count:
            cooling_fault = sent % 2
            frames = [
                can.Message(arbitration_id=0x200, data=[0x34, 0x12], is_extended_id=False),
                can.Message(arbitration_id=0x100, data=[cooling_fault], is_extended_id=False),
            ]

            for frame in frames:
                bus.send(frame, timeout=1.0)
                print(f"sent ID=0x{frame.arbitration_id:X} DLC={frame.dlc} DATA={frame.data.hex(' ')}")

            sent += 1
            time.sleep(args.interval)

    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        bus.shutdown()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

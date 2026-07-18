# Antenna Telemetry Receiver

This folder contains a serial receiver/logger for the DAQ telemetry radio link.

## Script
- `telemetry_receiver.py`
- `telemetry_sender.py`

## Install Dependencies
```bash
pip install -r ../requirements.txt
```

## Run
```bash
python3 antenna/telemetry_receiver.py --port COM7 --baud 57600
```

Linux/macOS example:
```bash
python3 antenna/telemetry_receiver.py --port /dev/cu.usbserial-D30AF7R6 --baud 57600
```

Send test lines from a laptop-connected radio:
```bash
python3 antenna/telemetry_sender.py --port /dev/cu.usbserial-D30AF7R6 --baud 57600
```

Send fake DAQ CSV rows at 10 Hz:
```bash
python3 antenna/telemetry_sender.py --port /dev/cu.usbserial-D30AF7R6 --baud 57600 --csv --interval 0.1
```

## Output
- Logs are written to `antenna/logs/` by default.
- Output file format is CSV with header:

```text
critical_timestamp_ms,chassis_timestamp_ms,wheel_speed_timestamp_ms,temp1_c,temp2_c,flow1_lpm,flow2_lpm,susp1,susp2,susp3,susp4,steering_angle_deg,speed_kmh
```

## Notes
- This assumes DAQ firmware telemetry is enabled (`SYSTEM_MODE` set to `MODE_FULL`).
- Firmware UART defaults used here are `57600` baud on `Serial2`.

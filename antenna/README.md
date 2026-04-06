# Antenna Telemetry Receiver

This folder contains a serial receiver/logger for the DAQ telemetry radio link.

## Script
- `telemetry_receiver.py`

## Install dependency
```bash
pip install pyserial
```

## Run
```bash
python3 antenna/telemetry_receiver.py --port COM7 --baud 57600
```

Linux/macOS example:
```bash
python3 antenna/telemetry_receiver.py --port /dev/ttyUSB0 --baud 57600
```

## Output
- Logs are written to `antenna/logs/` by default.
- Output file format is CSV with header:

```text
timestamp_ms,temp1_c,temp2_c,flow1_lpm,flow2_lpm,susp1,susp2,susp3,susp4,speed_kmh
```

## Notes
- This assumes DAQ firmware telemetry is enabled (`SYSTEM_MODE` set to `MODE_FULL`).
- Firmware UART defaults used here are `57600` baud on `Serial2`.

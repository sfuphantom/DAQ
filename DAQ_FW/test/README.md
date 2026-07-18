# Hardware Test Firmware

This folder contains small PlatformIO firmware images for hardware bring-up.

These files are **not the main DAQ firmware**. Each test builds as its own firmware image and replaces whatever is currently running on the ESP32 when uploaded. The normal DAQ application is built from `src/main.cpp` using the `esp32dev` environment.

Use these tests to isolate one hardware path at a time before debugging the full runtime.

## How PlatformIO Uses These Tests

Each test has a matching environment in `platformio.ini`.

For example:

```sh
pio run -e i2c_scan_test -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

That builds and uploads only `src/test/i2c_scan_test.cpp`. It does not run `src/main.cpp`, `SYSTEM_MODE`, or the normal DAQ task scheduler.

The main firmware is:

```sh
pio run -e esp32dev -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

Use the main firmware when testing `MODE_FULL`, `MODE_CAN_ONLY`, `MODE_SENSORS_ONLY`, SD logging in the real runtime, telemetry CSV output, or sensor simulation.

## Port Notes

The ESP32 upload/debug USB port is usually:

```text
/dev/cu.usbserial-0001
```

The antenna/telemetry USB adapter may be a different port, for example:

```text
/dev/cu.usbserial-D30AF4AU
```

Do not let PlatformIO upload to the telemetry adapter. If in doubt, pass the ESP32 port explicitly:

```sh
--upload-port /dev/cu.usbserial-0001
```

Only one program can monitor a serial port at a time. Close the PlatformIO serial monitor before uploading again.

## Available Tests

### `i2c_scan_test`

Scans the configured I2C bus and prints every address that ACKs.

Uses:

```cpp
I2C_SDA_PIN
I2C_SCL_PIN
```

Run:

```sh
pio run -e i2c_scan_test -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

Expected devices may include:

```text
0x48, 0x49, 0x4A, 0x4B = ADS1115 candidates
0x68 = DS3231 RTC candidate
0x57 = EEPROM on many DS3231 modules
```

### `suspension_sensor_test`

Checks the suspension ADS1115 configured as `ADCAddress::U2`, then prints channels 0-3 as CSV.

Run:

```sh
pio run -e suspension_sensor_test -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

The test prints one startup line for the suspension ADC:

```text
ADC U2 addr=0x48 i2c_ack=OK begin=OK
```

Then it prints all four `U2` channels every 500 ms:

```text
name,adc,address,channel,raw,volts,status
SuspensionSensor1,U2,0x48,0,12345,2.3156,ADC_CHANNEL_READ
```

Status values:

```text
ADC_CHANNEL_READ = ADC is online and the channel returned an in-range value
ADC_OFFLINE = the ADS1115 did not initialize
SATURATED = raw reading is at/near 0 or full scale, often wiring/power/range
UNCHANGED = the raw value has not changed for several samples
```

Move the attached suspension sensor and look for the channel whose raw/voltage value changes. Disconnected channels can float and still show in-range readings, so `ADC_CHANNEL_READ` means the ADC channel can be read, not that a sensor is definitely attached. `UNCHANGED` is expected if the sensor is sitting still; if every channel stays unchanged while you move the sensor, check wiring to the ADS1115 configured as `ADCAddress::U2`.

### `steering_angle_sensor_test`

Checks the steering angle ADS1115 configured as `ADCAddress::U3`, then prints channel 2 as labeled serial output. This is isolated from the main firmware and uses the same steering voltage-to-angle constants from `systemConfig.h`.

Run:

```sh
pio run -e steering_angle_sensor_test -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

The test prints one startup line for the steering ADC:

```text
ADC U3 addr=0x4B i2c_ack=OK begin=OK
```

Then it prints the steering channel every 500 ms:

```text
SteeringAngleSensor | adc=U3 address=0x4B channel=2 | voltage=2.3156 V | angle=163.4 deg | raw latest=12345 avg=12345 min=12340 max=12350 span=10 | status=ADC_CHANNEL_READ (valid ADC reading)
```

Status values:

```text
ADC_CHANNEL_READ = ADC is online and the channel returned an in-range changing value
ADC_OFFLINE = the ADS1115 did not initialize
SATURATED = raw reading is at/near 0 or full scale, often wiring/power/range
BELOW_CALIBRATED_RANGE = voltage is below STEERING_ANGLE_MIN_V and angle is clamped
ABOVE_CALIBRATED_RANGE = voltage is above STEERING_ANGLE_MAX_V and angle is clamped
UNCHANGED = the raw value has not changed for several samples
```

Turn the steering sensor and look for `latest_raw`, `latest_volts`, and `angle_deg` changes. `UNCHANGED` is expected if the sensor is sitting still; if it stays unchanged while you turn the sensor, check wiring to `ADCAddress::U3` channel 2.

### `rtc_test`

Initializes the DS3231 RTC and prints the current time once per second.

Run:

```sh
pio run -e rtc_test -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

If the RTC read fails, check I2C wiring, power, ground, and whether the scanner sees `0x68`.

### `rtc_set_time_test`

Sets the DS3231 RTC to the firmware compile time, then prints the current RTC time once per second.

Use this when the RTC reports lost power, has a new coin cell, or has never been set.

Run:

```sh
pio run -e rtc_set_time_test -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

After setting the RTC, upload the main firmware again before running the DAQ.

### `sd_card_test`

Checks basic SD card SPI wiring and file access.

This only proves the card can initialize, write `/sd_test.txt`, and read it back. It does not test the main DAQ SD logging service.

Run:

```sh
pio run -e sd_card_test -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

### `telemetry_test`

Sends a simple ping over the telemetry UART and prints debug output over ESP32 USB serial.

ESP32 side:

```sh
pio run -e telemetry_test -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

Mac antenna receiver side, in another terminal:

```sh
cd /Users/serafima/Developer/DAQ
python antenna/basic_telemetry_receiver.py --port /dev/cu.usbserial-D30AF4AU --baud 57600
```

The ESP32 monitor should print:

```text
sent telemetry test ping
```

The Mac receiver should print:

```text
telemetry test ping
```

This is a raw antenna/UART link test. It does not test the full DAQ telemetry CSV service.

### `can_internal_loopback_test`

Tests the ESP32 TWAI/CAN peripheral without requiring an external CAN transceiver or bus.

Run:

```sh
pio run -e can_internal_loopback_test -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

Expected output:

```text
Internal loopback OK id=0x123 data=CA FE
```

### `can_no_ack_transceiver_test`

Tests CAN TX through the configured ESP32 CAN pins and external transceiver without requiring another CAN node to ACK.

Run:

```sh
pio run -e can_no_ack_transceiver_test -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

This sends:

```text
ID 0x200
DATA 34 12
500 kbit/s
```

This can show `TX OK` even when no other device receives the frame. Use a real CAN analyzer or another CAN node to prove bus reception.

### `can_transceiver_test`

Tests normal CAN TX mode. This requires a valid CAN bus and another CAN node to ACK frames.

Run:

```sh
pio run -e can_transceiver_test -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

If no other CAN node is connected, `ESP_ERR_TIMEOUT` is expected.

### `can_receive_test`

Listens for CAN frames and prints them over ESP32 USB serial.

Run:

```sh
pio run -e can_receive_test -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

Use this when another CAN node is transmitting at 500 kbit/s and you want the ESP32 to act as a listener.

## What To Use For Main Firmware Testing

Use `esp32dev`, not this folder, for app-level testing:

```sh
pio run -e esp32dev -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor -p /dev/cu.usbserial-0001 -b 115200
```

Then configure behavior in `include/systemConfig.h`:

```cpp
#define SYSTEM_MODE MODE_FULL
#define SYSTEM_MODE MODE_CAN_ONLY
#define SYSTEM_MODE MODE_SENSORS_ONLY
```

Examples:

```text
MODE_CAN_ONLY
  Uses the real CAN driver and sends canned app-level CAN messages.

MODE_SENSORS_ONLY
  Reads and prints sensor snapshots with normal CAN traffic disabled.

MODE_FULL + ENABLE_SENSOR_SIMULATION
  Tests full runtime outputs using fake sensor data.

MODE_FULL + ENABLE_SD_LOGGING_OUTPUT
  Tests SD CSV logging inside the real DAQ runtime.
```

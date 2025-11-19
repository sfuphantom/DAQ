
# Data Aquisition System (DAQ)
The DAQ system collect, logs and transmits real-time sensor data from our vehicle to support driver safety, performance analysis, and debugging. The DAQ system interfaces with multiple analog/digital sensors and streams data over CAN to the VCU and other subsystems.  

## Contents 

- [Overview](#overview)
- [System Context](#system-context)
- [Project Status](#project-status)
- [Hardware](#hardware)
- [Firmware / Software](#firmware--software)
- [DAQ Additional Information](#daq-additional-information)
- [Getting Started](#getting-started)
- [PlatformIO Setup](#platformIO--setup)
- [Logic & Functionality](#logic--&--functionality)
- [Configurations & Toggles](#configurations--&--toggles)
- [Calibration & Procedures](#calibration--&--procedures)
- [Additional Notes](additional--N=notes)
- [Safety & FSAE Compliance](#safety--fsae-compliance)
- [Maintainers](#maintainers)
- [License](#license)


## Overview
The purpose of the DAQ system is to enable Phantom to **monitor, log, and analyze sensor data** in real time, supporting both driver feedback** and engineering development. It consolidates analog and digital signals (temperatures, pressures, wheel speed) into standardized CAN messages. Logged data is used for post-drive analysis and compliance with Formula SAE rules. 

**Key Features**
- Reads analog and digital sensor signals (wheel speed, two pressure sensors, two temperature sensors, IMU).  
- Communicates over CAN with the VCU and optionally with external loggers.
- Supports data logging for validation, driver feedback, and debugging.  
- Modular firmware for easy addition of new sensors.  

**Design Goals**
- Reliability under track conditions.  
- Easy calibration and debugging.  
- FSAE rule compliance (DAQ required for scrutineering).  
- Expandability for future sensors. 

## System Context
![DAQ System Context](docs/diagrams/daq-context.png)

## Project Status

- **Current state:** 
- Bench-tested on ESP32 dev kits with coolant temperature, pressure, and wheel-speed sensors streaming over CAN; ready for in-car integration.

- **Focus areas:** Finalize wiring harness pinout, expand calibration coverage for additional sensors, and document the warm-up/fault behavior for the VCU team.
    
- **Current board/major revision:** Rev 4 (ESP32-based DAQ board)
    
- **Last validated on:** <2025‑11‑18>
    
- **Health:** 
- Sensor bring-up, CAN messaging, and logging are stable; remaining tasks are workflow polish and extended testing.


## Hardware

- **MCU / ICs:** ESP32, ADS1115 (I^2C), CAN transceiver (SN65HVD230).
    
- **Files:** 
[DAQ Rev 4 Schematic](docs/hardware/Schematic%20PDF_[No%20Variations].pdf)

## Firmware / Software

- **Language/SDK:** C++ with Arduino-ESP32 framework, Python for data visualization.
    
- **Build System:** PlatformIO (VSCode).
    
- **Key Modules:** 
| Module | Location | Purpose | Notes |
| --- | --- | --- | --- |
| **Main firmware** | `src/main.cpp` | Initializes hardware, enforces safety limits, and pushes CAN data. | Contains all runtime toggles for sensors, CAN IDs, and warm-up timing. |
| **IADC Sensor drivers** | `lib/IADCSensor` | ADS1115 abstractions for coolant temperature/pressure. | `CoolantTemperatureSensor` / `CoolantPressureSensor` expose `Initialize()` + `GetData()`. |
| **CAN (TWAI) layer** | `lib/Can` | Configures ESP32 CAN driver and transmits frames. | `CAN_Init()` validates loopback; `CAN_SendInt16()` sends packed integers. |
| **Wheel speed** | `lib/WheelSpeed` | Captures interrupts, filters, and scales wheel-speed data. | Use `WheelSpeedReset()` + `getFinalWheelSpeed()` in `main.cpp`. |
| **Global config** | `include/system_config.h` | Shared constants (baud rate, log level, ADS addresses). | Adjust `CURRENT_LOG_LEVEL` or `ADCAddress` mappings here. |

## DAQ additional Information
Note:
- The original DAQ project was developed in Python for the Raspberry Pi 4. 
- Due to supply shortages, the project switched to an ESP32 microcontroller. 
- The codebase for the original project can be found in this branch: https://github.com/sfuphantom/DAQ/tree/RaspberryPi-Archive-2021.

## Getting Started
1. **Clone & Open** – `git clone` the repo, open it in VS Code or your preferred editor/terminal.
2. **Install Dependencies** – PlatformIO will pull libraries from `platformio.ini` automatically, or run `pio pkg install`.
3. **Review Config** – Edit the defines at the top of `src/main.cpp` (sensor enables, limits, CAN IDs, `SENSOR_TEST_MODE`, `FAULT_SUPPRESS_MS`) plus any globals in `include/system_config.h`.
4. **Build** – `pio run` (targets `[env:esp32dev]` by default).
5. **Flash** – `pio run -t upload` with the ESP32 connected over USB.
6. **Monitor** – `pio device monitor -b 115200` to view logs (`Logger::Notice/Trace/Error` output sensor snapshots and fault status).
7. **Verify** – Watch for the init message, confirm warm-up suppression, then inject real faults to ensure `FAULT_MSG_ID` toggles as expected while wheel-speed frames continue.

## PlatformIO Setup 
```
pip install platformio          # or install the VS Code PlatformIO IDE extension
pio run                         # build
pio run -t upload               # flash
pio device monitor -b 115200    # serial monitor (matches BAUD_RATE)
```
- Environment: `[env:esp32dev]` in `platformio.ini`.
- Dependencies: ArduinoLog, Adafruit ADS1X15, SPI, LSM6DS, Unified Sensor (auto-resolved by PlatformIO).
- Serial monitor uses the baud defined in `include/system_config.h` (`BAUD_RATE = 115200`).

**Prerequisites**
- ESP32 dev board wired to ADS1115 modules, coolant sensors, wheel-speed sensors, and CAN transceiver (TX = GPIO4, RX = GPIO5).
- PlatformIO Core (`pip install platformio`) or VS Code with the PlatformIO IDE extension.
- USB cable for flashing and serial monitoring.
- Access to the vehicle CAN bus or a bench CAN interface for testing.



## Logic & Functionality
1. **Setup Path (`setup()` in `src/main.cpp`)**
   - Starts Serial/I²C, logger, and CAN (`CAN_Init()` loops back a test frame).
   - Initializes each enabled sensor (based on `ENABLE_*` macros).
   - Sends an **initialization CAN frame** on `FAULT_MSG_ID` with value `0` when `SENSOR_TEST_MODE` is `0`, so the VCU knows the DAQ is alive.
   - Records `startupTime` for the warm-up buffer defined by `FAULT_SUPPRESS_MS`.

2. **Main Loop (`loop()` in `src/main.cpp`)**
   - Reads all enabled coolant sensors and wheel-speed data every iteration.
   - Logs a sensor snapshot once per second (`logSensorSnapshot()`).
   - Evaluates safety limits with `valueOutOfRange()`; faults are gated until `(millis() - startupTime) >= FAULT_SUPPRESS_MS`.
   - When a fault is active, logs it once and keeps transmitting `CAN_SendInt16(FAULT_MSG_ID, 1)` until readings recover; recovery logs a notice.
   - Resets wheel-speed capture, scales the filtered speed by 100, and pushes it on `WHEEL_MSG_ID`.

3. **Edge Modes**
   - `SENSOR_TEST_MODE == 1`: loop exits after logging, so no CAN traffic (safe for dry bench wiring).
   - `SENSOR_TEST_MODE == 0`: full behavior; initialization frame, warm-up buffer, fault logic, and wheel-speed CAN output are all active.

## Configurations & Toggles 
(in `src/main.cpp`)
- `SENSOR_TEST_MODE` – `1` disables CAN/faults; `0` is production behavior.
- Sensor toggles (`ENABLE_TEMP_SENSOR_1/2`, `ENABLE_PRESSURE_SENSOR_1/2`) – set to `1` for any wired channel; disabled sensors return `NAN` so they’re ignored.
- Safety bounds (`MAX_TEMP_1`, `MAX_TEMP_2`, `MIN_TEMP`, `MIN_PRESSURE`, `MAX_PRESSURE`) – adjust per calibration results.
- `FAULT_SUPPRESS_MS` – warm-up duration (default 3000 ms). Increase if sensors need longer to stabilize when the DAQ powers up before the cooling loop.
- CAN IDs (`FAULT_MSG_ID`, `WHEEL_MSG_ID`) – coordinate with the VCU team before changing.
- Wheel-speed scaling lives in `sendWheelSpeed()`: values are multiplied by 100 before sending and must be divided back out on the dash/VCU side.

> Keep these defines near the top of `src/main.cpp` so newcomers can review them before flashing firmware.

## Calibration & Procedures
1. **Sensor Verification**
   - Enable one channel at a time using the `ENABLE_*` macros.
   - Use `pio device monitor -b 115200` to confirm raw readings are stable.
   - Tune `MAX_TEMP_*`/`MIN/MAX_PRESSURE` after comparing with a trusted gauge.

2. **Warm-Up Buffer Check**
   - With sensors cold, boot the DAQ and confirm only the initialization CAN frame is sent for the first `FAULT_SUPPRESS_MS`.
   - After warm-up, force a known out-of-range value (e.g., unplug a sensor) to ensure `FAULT_MSG_ID` payload `1` repeats until recovery.

3. **Wheel-Speed Validation**
   - Spin the wheel sensor manually, verify `getFinalWheelSpeed()` outputs expected values, and confirm `WHEEL_MSG_ID` frames update accordingly.

4. **In-Car Procedure**
   - Ensure the cooling system is already circulating (or adjust `FAULT_SUPPRESS_MS`) so false faults don’t trigger when the DAQ powers up.
   - Coordinate with the VCU/dash teams so they expect the startup `FAULT_MSG_ID` = 0 frame.

## Additional Notes
- Keep `SENSOR_TEST_MODE` at `1` when doing dry testing without the cooling loop; remember to flip it back to `0` before any CAN validation or on-car run.
- When adding sensors, instantiate new objects alongside the existing ones in `src/main.cpp`, assign the proper ADS channel/`ADCAddress`, and extend the limit checks if needed.
- Share any calibration offsets or wiring changes with the team so this README stays accurate.

## Safety & FSAE Compliance
- **EV.7.1.4** – BMS, IMD, and BSPD must each have independent circuits capable of opening the shutdown system.  
- **EV.4.6** – Series-provided energy meter must monitor TS voltage/current. DAQ supplements this by logging additional sensor data for analysis.  
- **Accumulator monitoring** – At least one temperature sensor required inside the accumulator; DAQ can optionally mirror these values.  
- **Wheel speed (T.11.x)** – Required for scrutineering; DAQ measures and provides wheel speed data to the dashboard and logs for inspection.  
- **Cooling system monitoring** – While not explicitly mandated, DAQ logs coolant pressure/temperature and raises fault signals to the VCU to protect HV components.  

## Maintainers

**Lead:** Sera
**Members:** Andrew, Igor 

**Previous Lead:** Raf
**Previous Members:** Lona, Ethan

## License
Licensed under the [MIT License](./LICENSE).
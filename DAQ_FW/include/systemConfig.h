#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <Arduino.h>

// Serial baud rate used everywhere in the firmware/monitor
uint32_t constexpr BAUD_RATE = 115200;

// Telemetry antenna UART
static HardwareSerial &TELEMETRY_UART = Serial2;

// I2C pins for ADC
gpio_num_t constexpr I2C_SDA_PIN = GPIO_NUM_21;
gpio_num_t constexpr I2C_SCL_PIN = GPIO_NUM_23;

// CAN pins
gpio_num_t constexpr CAN_TX_PIN = GPIO_NUM_4;
gpio_num_t constexpr CAN_RX_PIN = GPIO_NUM_5;
// Set to 1 for single-node bench testing. Set to 0 on a real CAN bus with another node ACKing frames.
#define CAN_NO_ACK_MODE 0
// Restart the TWAI driver after repeated failed transmits so stale frames are dropped.
#define CAN_TX_FAILURE_RECOVERY_THRESHOLD 5
#define CAN_RECOVERY_COOLDOWN_MS 1000

// Telemetry UART pins (Antenna)
gpio_num_t constexpr TELEMETRY_TX_PIN = GPIO_NUM_17;
gpio_num_t constexpr TELEMETRY_RX_PIN = GPIO_NUM_16;
uint32_t constexpr TELEMETRY_BAUD = 57600;

// SD Card (SPI) pins
gpio_num_t constexpr SD_CS_PIN = GPIO_NUM_33;
gpio_num_t constexpr SD_MISO_PIN = GPIO_NUM_27;
gpio_num_t constexpr SD_MOSI_PIN = GPIO_NUM_13;
gpio_num_t constexpr SD_SCK_PIN = GPIO_NUM_14;

// System modes, 0 means both are enabled, 1 means only sensors are disabled, and 2 means only can is disabled
#define MODE_FULL 0
#define MODE_CAN_ONLY 1
#define MODE_SENSORS_ONLY 2
// Adjust what setting you would like to use here:
#define SYSTEM_MODE MODE_FULL

#define CAN_ENABLED (SYSTEM_MODE != MODE_SENSORS_ONLY)
#define SENSORS_ENABLED (SYSTEM_MODE != MODE_CAN_ONLY)

// Sensor simulation
#define ENABLE_SENSOR_SIMULATION 0

// Watchdog
#define WATCHDOG_ENABLED 1
#define WATCHDOG_TIMEOUT_S 5

// Telemetry (antenna) CSV rate
#define TELEMETRY_PERIOD_MS 100
#define SD_LOG_PERIOD_MS 100
#define SERIAL_LOG_PERIOD_MS 1000
#define SENSOR_NULL_TEXT "null"
uint32_t constexpr SENSOR_SIMULATION_PERIOD_MS = 50;

// Toggle wheel speed subsystem
#define ENABLE_WHEEL_SPEED_SENSORS 1

// Toggle logging outputs
#define ENABLE_TELEMETRY_OUTPUT 1
#define ENABLE_SD_LOGGING_OUTPUT 1
// Serial-only bench status logs. Does not add fields to telemetry or SD CSV data.
// Set to 1 when a laptop is connected, 0 for quieter car runs.
#define ENABLE_STATUS_LOGS 0

// Toggle individual sensors
#define ENABLE_TEMP_SENSOR_1 1
#define ENABLE_TEMP_SENSOR_2 1

#define ENABLE_FLOW_SENSOR_1 1
#define ENABLE_FLOW_SENSOR_2 1

#define ENABLE_STEERING_ANGLE_SENSOR 1

#define ENABLE_SUSP_SENSOR_1 1
#define ENABLE_SUSP_SENSOR_2 1
#define ENABLE_SUSP_SENSOR_3 1
#define ENABLE_SUSP_SENSOR_4 1

// Wheel speed sensor pins
gpio_num_t constexpr WHEEL_SPEED_FL_PIN = GPIO_NUM_34;
gpio_num_t constexpr WHEEL_SPEED_FR_PIN = GPIO_NUM_35;
gpio_num_t constexpr WHEEL_SPEED_RL_PIN = GPIO_NUM_32;
gpio_num_t constexpr WHEEL_SPEED_RR_PIN = GPIO_NUM_33;

// Flow sensors are Hall-effect pulse outputs read on MCU GPIO interrupts.
// Inputs must be pulled up to 3.3V before connecting to ESP32 GPIO.
gpio_num_t constexpr FLOW_SENSOR_1_PIN = GPIO_NUM_18;
gpio_num_t constexpr FLOW_SENSOR_2_PIN = GPIO_NUM_19;
uint32_t constexpr FLOW_SENSOR_SAMPLE_WINDOW_MS = 500;

// Cooling temp for the outer bypass cooling is 80 Celsius max.
// Inlet, coming out of the radiator, is 75 Celsius max.
float constexpr MAX_TEMP_1 = 80.0f;
float constexpr MAX_TEMP_2 = 75.0f;
float constexpr MIN_TEMP = 0.0f;
float constexpr MIN_FLOW_LPM = 0.0f;
float constexpr MAX_FLOW_LPM = 30.0f;
float constexpr FLOW_SENSOR_HZ_PER_LPM = 6.6f;

enum class CANMessageId : uint16_t {
    CoolingFault = 0x100,
    WheelSpeed = 0x200
};

// I2C addresses for the ADS1115 connection
enum class ADCAddress {
    U1 = 0x49,
    U2 = 0x48,
    U3 = 0x4B,
    U4 = 0x4A
};

// ADS1115 channel map. Keep this aligned with the wiring harness and schematic.
ADCAddress constexpr TEMP_SENSOR_1_ADC = ADCAddress::U1;
ADCAddress constexpr TEMP_SENSOR_2_ADC = ADCAddress::U1;
uint8_t constexpr TEMP_SENSOR_1_CHANNEL = 0;
uint8_t constexpr TEMP_SENSOR_2_CHANNEL = 1;

ADCAddress constexpr SUSP_SENSOR_1_ADC = ADCAddress::U2;
ADCAddress constexpr SUSP_SENSOR_2_ADC = ADCAddress::U2;
ADCAddress constexpr SUSP_SENSOR_3_ADC = ADCAddress::U2;
ADCAddress constexpr SUSP_SENSOR_4_ADC = ADCAddress::U2;
uint8_t constexpr SUSP_SENSOR_1_CHANNEL = 0;
uint8_t constexpr SUSP_SENSOR_2_CHANNEL = 1;
uint8_t constexpr SUSP_SENSOR_3_CHANNEL = 2;
uint8_t constexpr SUSP_SENSOR_4_CHANNEL = 3;

ADCAddress constexpr STEERING_ANGLE_ADC = ADCAddress::U3;
uint8_t constexpr STEERING_ANGLE_CHANNEL = 2;

// Steering angle sensor is modeled as an external ADC-backed analog input.
// Adjust these to the ADS1115 address/channel that the sensor is wired to.
float constexpr STEERING_ANGLE_SUPPLY_V = 5.0f;
float constexpr STEERING_ANGLE_MIN_V = 0.5f;
float constexpr STEERING_ANGLE_MAX_V = 4.5f;
float constexpr STEERING_ANGLE_FULL_SCALE_DEG = 360.0f;

// DAQ global Log Level Setup Filter
#define CURRENT_LOG_LEVEL LOG_LEVEL_NOTICE
// Options:
// LOG_LEVEL_SILENT     no output
// LOG_LEVEL_FATAL      fatal errors
// LOG_LEVEL_ERROR      all errors
// LOG_LEVEL_WARNING    errors, and warnings
// LOG_LEVEL_NOTICE     errors, warnings and notices
// LOG_LEVEL_TRACE      errors, warnings, notices & traces
// LOG_LEVEL_VERBOSE    all

// Defines global decimal places when displaying floats and doubles ('%D' AND '%F')
// param goes to ArduinoLog.cpp line 192: _logOutput->print(va_arg(*args, double), LOG_DECIMAL_PLACES);
#define LOG_DECIMAL_PLACES 5

#endif

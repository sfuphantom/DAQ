#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <Arduino.h>

// Serial baud rate used everywhere in the firmware/monitor
uint32_t constexpr BAUD_RATE = 115200;

// Telemetry antenna UART
static HardwareSerial &TELEMETRY_UART = Serial2;

// I2C pins for ADC
gpio_num_t constexpr I2C_SDA_PIN = GPIO_NUM_21;
gpio_num_t constexpr I2C_SCL_PIN = GPIO_NUM_22;

// CAN pins
gpio_num_t constexpr CAN_TX_PIN = GPIO_NUM_4;
gpio_num_t constexpr CAN_RX_PIN = GPIO_NUM_5;

// Telemetry UART pins
gpio_num_t constexpr TELEMETRY_TX_PIN = GPIO_NUM_17;
gpio_num_t constexpr TELEMETRY_RX_PIN = GPIO_NUM_16;
uint32_t constexpr TELEMETRY_BAUD = 57600;

// SD Card (SPI) pins
gpio_num_t constexpr SD_CS_PIN = GPIO_NUM_15;
gpio_num_t constexpr SD_MISO_PIN = GPIO_NUM_13;
gpio_num_t constexpr SD_MOSI_PIN = GPIO_NUM_12;
gpio_num_t constexpr SD_SCK_PIN = GPIO_NUM_14;

// System modes, 0 means both are enabled, 1 means only sensors are disabled, and 2 means only can is disabled
#define MODE_FULL 0
#define MODE_CAN_ONLY 1
#define MODE_SENSORS_ONLY 2
// Adjust what setting you would like to use here:
#define SYSTEM_MODE MODE_SENSORS_ONLY

#define CAN_ENABLED (SYSTEM_MODE != MODE_SENSORS_ONLY)
#define SENSORS_ENABLED (SYSTEM_MODE != MODE_CAN_ONLY)

// Watchdog
#define WATCHDOG_ENABLED 1
#define WATCHDOG_TIMEOUT_S 5

// Telemetry (antenna) CSV rate
#define TELEMETRY_PERIOD_MS 100
uint32_t constexpr SENSOR_SIMULATION_PERIOD_MS = 50;

// Toggle wheel speed subsystem
#define ENABLE_WHEEL_SPEED_SENSORS 1
#define ENABLE_SENSOR_SIMULATION 0
#define ENABLE_TELEMETRY_OUTPUT 1
#define ENABLE_SD_LOGGING_OUTPUT 1

// Toggle individual sensors
#define ENABLE_TEMP_SENSOR_1 0
#define ENABLE_TEMP_SENSOR_2 0

#define ENABLE_FLOW_SENSOR_1 0
#define ENABLE_FLOW_SENSOR_2 0

#define ENABLE_STEERING_ANGLE_SENSOR 0

#define ENABLE_SUSP_SENSOR_1 0
#define ENABLE_SUSP_SENSOR_2 0
#define ENABLE_SUSP_SENSOR_3 0
#define ENABLE_SUSP_SENSOR_4 0

// suspension sensors (ESP32 ADC2) pins
gpio_num_t constexpr SUSP_1 = GPIO_NUM_27;
gpio_num_t constexpr SUSP_2 = GPIO_NUM_25;
gpio_num_t constexpr SUSP_3 = GPIO_NUM_26;
gpio_num_t constexpr SUSP_4 = GPIO_NUM_2;

// Flow sensors are Hall-effect pulse outputs read on MCU GPIO interrupts. #TODO: NEED TO MAKE SURE THEYRE PULLED UP TO 3.3V not 5V
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

enum class CANMessageId : uint16_t
{
    CoolingFault = 0x100,
    WheelSpeed = 0x200

};

// I2C addresses for the ADS1115 connection
enum class ADCAddress
{
    U1 = 0x48,
    U2 = 0x49,
    U3 = 0x4A,
    U4 = 0x4B
};

// Steering angle sensor is modeled as an external ADC-backed analog input.
// Adjust these to the ADS1115 address/channel that the sensor is wired to.
float constexpr STEERING_ANGLE_SUPPLY_V = 5.0f;
float constexpr STEERING_ANGLE_MIN_V = 0.5f;
float constexpr STEERING_ANGLE_MAX_V = 4.5f;
float constexpr STEERING_ANGLE_FULL_SCALE_DEG = 360.0f;

// DAQ global Log Level Setup Filter
#define CURRENT_LOG_LEVEL LOG_LEVEL_VERBOSE
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

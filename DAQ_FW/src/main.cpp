#include "Logger.h"
#include "IADCSensor.h"
#include "wheelSpeed.h"
#include <Arduino.h>
#include <driver/twai.h>
#include "CanDriver.h"
#include "stdlib.h"
#include "time.h"

MAX_TEMP = 4
MIN_TEMP =  2
MIN_PRESSURE = 1.3 
MAX_PRESSURE = 1.7 

#define FAULT_MSG_ID 0x100
#define WHEEL_MSG_ID 0x200

// CAN pins
#define CAN_TX GPIO_NUM_21
#define CAN_RX GPIO_NUM_22

// object declarations can't be done in setup()

// params: sensorname, sensorID, adcAddress
ChildExample SensorTest("TestSensor", 1, ADCAddress::U1);

// cooloant pressure sensor object decleration
//CoolantPressureSensor CoolantPressure("CoolantPressureSensor", 2, ADCAddress::U1);

// coolant tempature sensor object decleration
//CoolantTemperatureSensor CoolantTemperature("CoolantTemperature Sensor", 2, ADCAddress::U1);

bool faultDetected = false;

// CAN initilization
void CanDriver::CanInit() {
    can_general_config_t general_config = {
        .mode = CAN_MODE_NORMAL,
        .tx_io = CAN_TX,
        .rx_io = CAN_RX,
        .clkout_io = CAN_IO_UNUSED,
        .bus_off_io = CAN_IO_UNUSED,
        .tx_queue_len = 65,
        .rx_queue_len = 65,
        .alerts_enabled = CAN_ALERT_ALL,
        .clkout_divider = 0
    };

    can_timing_config_t timing_config = CAN_TIMING_CONFIG_250KBITS();
    can_filter_config_t filter_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t error = can_driver_install(&general_config, &timing_config, &filter_config);

    if (error == ESP_OK) {
        Logger::Notice("CAN Driver Installation OK");
    } else {
        Logger::Error("CAN Driver Installation Failed");
        while (1) {} // Halt execution on failure
    }

    error = can_start();
    if (error == ESP_OK) {
        Logger::Notice("CAN Driver Started");
    } else {
        Logger::Error("CAN Driver Failed to Start");
        while (1) {}
    }
}

void sendFaultSignal(bool faultDetected) {
    twai_message_t faultMessage;
    faultMessage.identifier = FAULT_MSG_ID;
    faultMessage.extd = 0;                 // Standard ID
    faultMessage.data_length_code = 1;     // 1 byte of data
    faultMessage.data[0] = faultDetected ? 1 : 0;

    if (twai_transmit(&faultMessage, pdMS_TO_TICKS(1000)) == ESP_OK) {
        Logger::Notice(faultDetected ? "Fault signal sent successfully" : "No fault detected, signal sent.");
    } else {
        Logger::Error("Failed to send fault signal");
    }
}

void setup()
{
  Logger::Start();
  Logger::Notice("Setup");
  WheelSpeedSetup();
  CanDriver::CanInit();

  //SensorTest.Initialize();
  //CoolantPressure.Initialize();
  //CoolantTemperature.Initialize();
}

// Main
void loop()
{
  Logger::Notice("Hello World");

  //SensorTest.GetData();
  //float pressure = CoolantPressure.GetData();
  //float temp = CoolantTemperature.GetData();

  if (pressure < MIN_PRESSURE || pressure > MAX_PRESSURE || temp < MIN_TEMP || temp > MAX_TEMP){
    Logger::Error("Fault detected! Sending fault signal.");
    faultDetected = true; 
  } else {
        faultDetected = false;
    }

  sendFaultSignal(faultDetected);
  
  WheelSpeedReset();
  delay(1000);
}
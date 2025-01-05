#include "Logger.h"
#include "IADCSensor.h"
#include "wheelSpeed.h"
#include <Arduino.h>
#include <driver/twai.h>

MAX_TEMP = 4
MIN_TEMP =  2
MIN_PRESSURE = 1.3 
MAX_PRESSURE = 1.7 

#define FAULT_MSG_ID 0x100
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

void setup()
{
  // put your setup code here, to run once:
  Logger::Start();
  Logger::Notice("Setup");
  WheelSpeedSetup();

  //SensorTest.Initialize();

  //CoolantPressure.Initialize();

  //CoolantTemperature.Initialize();
}

void loop()
{
  // put your main code here, to run repeatedly:
  Logger::Notice("Hello World");

  //SensorTest.GetData();

  //float pressure = CoolantPressure.GetData();

  //float temp = CoolantTemperature.GetData();

  if (pressure < MIN_PRESSURE || pressure > MAX_PRESSURE || temp < MIN_TEMP || temp > MAX_TEMP){
    Logger::Error("Fault detected! Sending fault signal.");
    faultDetected = true; 
  }

  twai_message_t faultMessage;
  uint8_t faultSignal[] = {faultDetected ? 1 : 0}; // 1 = True
  twai_message_t faultMessage;
  faultMessage.identifier = FAULT_MSG_ID;  
  faultMessage.extd = 0;                  // Standard ID
  faultMessage.data_length_code = 1;      // 1 byte of data 
  faultMessage.data[0] = faultSignal[0];  // Fault signal (1 or 0)

  if (twai_transmit(&faultMessage, pdMS_TO_TICKS(1000)) == ESP_OK) {
    Logger::Notice(faultDetected ? "Fault signal sent successfully" : "No fault signal sent successfully");
  } else {
    Logger::Error("Failed to send fault signal");
  }

  WheelSpeedReset();
  delay(1000);
}
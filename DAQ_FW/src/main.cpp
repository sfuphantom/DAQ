#include "Logger.h"
#include "IADCSensor.h"
#include "wheelSpeed.h"
#include <Arduino.h>
#include <driver/twai.h>
#include "stdlib.h"
#include "CanDriver.h"

#define MAX_TEMP 4
#define MIN_TEMP 2
#define MIN_PRESSURE 1.3 
#define MAX_PRESSURE 1.7 

#define FAULT_MSG_ID 0x100
#define WHEEL_MSG_ID 0x200

// CAN pins
#define CAN_TX GPIO_NUM_21 // is data 
#define CAN_RX GPIO_NUM_22 // is clock 

bool faultDetected = false;

// object declarations can't be done in setup()

// params: sensorname, sensorID, adcAddress
// ChildExample SensorTest("TestSensor", 1, ADCAddress::U1);

// cooloant pressure sensor object decleration, MAKE SURE TO USE DIFF CHANNELS OF U1, we only have one chip physically
CoolantPressureSensor CoolantPressure1("CoolantPressureSensor", 0, ADCAddress::U1);

// coolant tempature sensor object decleration
CoolantTemperatureSensor CoolantTemperature1("CoolantTemperature Sensor", 1, ADCAddress::U1);

CoolantTemperatureSensor CoolantTemp2("CoolantTemp2", 2, ADCAddress::U1);
CoolantPressureSensor CoolantPressure2("CoolantPressure2", 3, ADCAddress::U1); 


// // For testing - wheel speed values in m/s
// float fl = 2.55; // Front left
// float fr = 2.54; // Front right
// float rl = 2.53; // Rear left
// float rr = 2.56; // Rear right

// float temp = 3.0; 
// float pressure = 1.5;


void sendWheelSpeeds(float frontLeft, float frontRight, float rearLeft, float rearRight) // Has to be decoded on dashboard side
{
  Logger::Notice("Sending wheel speeds over CAN");

  // preserving 2 decimal places
  int16_t fl = (int16_t)(frontLeft * 100);
  int16_t fr = (int16_t)(frontRight * 100);
  int16_t rl = (int16_t)(rearLeft * 100);
  int16_t rr = (int16_t)(rearRight * 100);

  Logger::Notice("FL: %d, FR: %d, RL: %d, RR: %d", fl, fr, rl, rr);

  uint64_t can_data = 0;

  // Define explicit bit positions
  can_data |= ((uint64_t)fl << 48);
  can_data |= ((uint64_t)fr << 32);
  can_data |= ((uint64_t)rl << 16);
  can_data |= ((uint64_t)rr);

  Logger::Notice("Sending CAN data: 0x%llX", can_data);
  const char* canData = (const char*)&can_data;

  // Sending the whole msg as a 64 bit uint
  if (canData != nullptr) {
    CanDriver::sendCanData(canData, 8, WHEEL_MSG_ID, can_data, true);
  } else {
      Logger::Error("Error: canData is NULL before calling sendCanData.");
  }
  //CanDriver::sendCanData(nullptr, 8, WHEEL_MSG_ID, can_data, true);
  Logger::Notice("Wheel speeds sent over CAN");
}

void setup()
{
  Wire.begin(21, 22); // SDA = GPIO21, SCL = GPIO22 for ESP32

  Logger::Start();
  Logger::Notice("Setup");

  CanDriver::CanInnit();

  // For testing 
  // SensorTest.Initialize();

  CoolantTemperature1.Initialize();
  CoolantTemperature2.Initialize();
  CoolantPressure1.Initialize();
  CoolantPressure2.Initialize();

  // Send initialization message
  CanDriver::sendCanData(nullptr, 1, FAULT_MSG_ID, 0, false);

  Logger::Notice("Setup complete");
}

// Main
void loop()
{
  Logger::Notice("Hello World");

  // SensorTest.GetData();

  float temp1 = CoolantTemperature1.GetData();
  float temp2 = CoolantTemperature2.GetData();
  float pressure1 = CoolantPressure1.GetData();
  float pressure2 = CoolantPressure2.GetData();

  if (pressure < MIN_PRESSURE || pressure > MAX_PRESSURE || temp < MIN_TEMP || temp > MAX_TEMP){
    Logger::Error("Fault detected! Sending fault signal.");
    CanDriver::sendCanData(nullptr, 1, 2, 1, false); 
  } 

  sendWheelSpeeds(fl, fr, rl, rr);
  
  //WheelSpeedReset();
  delay(1000);
}
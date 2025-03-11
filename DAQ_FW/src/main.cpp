#include "Logger.h"
#include "IADCSensor.h"
#include "wheelSpeed.h"
#include <Arduino.h>
#include <driver/twai.h>
#include "stdlib.h"
#include "CanDriver.h"

#define MAX_TEMP = 4
#define MIN_TEMP =  2
#define MIN_PRESSURE = 1.3 
#define MAX_PRESSURE = 1.7 

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


void sendWheelSpeeds(float frontLeft, float frontRight, float rearLeft, float rearRight) // Has to be decoded on dashboard side
{
  // preserving 2 decimal places
  int16_t fl = (int16_t)(frontLeft * 100);
  int16_t fr = (int16_t)(frontRight * 100);
  int16_t rl = (int16_t)(rearLeft * 100);
  int16_t rr = (int16_t)(rearRight * 100);

  uint64_t can_data = 0;

  // Define explicit bit positions
  can_data |= ((uint64_t)48 << 48);
  can_data |= ((uint64_t)32 << 32);
  can_data |= ((uint64_t)16 << 16);
  can_data |= ((uint64_t)rr);

  // Sending the whole msg as a 64 bit uint
  CanDriver::sendCanData(NULL, 8, 0x2, can_data, true);
}

void setup()
{
  Logger::Start();
  Logger::Notice("Setup");
  //WheelSpeedSetup();
  CanDriver::CanInnit();

  SensorTest.Initialize();
  //CoolantPressure.Initialize();
  //CoolantTemperature.Initialize();

  CanDriver::sendCanData(NULL, 1, 2, 0, false);
}

// Main
void loop()
{
  Logger::Notice("Hello World");

  SensorTest.GetData();
  //float pressure = CoolantPressure.GetData();
  //float temp = CoolantTemperature.GetData();

  if (pressure < MIN_PRESSURE || pressure > MAX_PRESSURE || temp < MIN_TEMP || temp > MAX_TEMP){
    Logger::Error("Fault detected! Sending fault signal.");
    CanDriver::sendCanData(NULL, 1, 2, 1, false); 
  } 

  // sendWheelSpeeds(fl, fr, rl, rr);
  
  //WheelSpeedReset();
  delay(1000);
}
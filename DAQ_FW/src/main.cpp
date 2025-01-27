#include "Logger.h"
#include "IADCSensor.h"
#include "wheelSpeed.h"
#include <Arduino.h>
#include <driver/twai.h>
#include "stdlib.h"

// MAX_TEMP = 4 write in define
// MIN_TEMP =  2
// MIN_PRESSURE = 1.3 
// MAX_PRESSURE = 1.7 

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


void sendFaultSignal(bool faultDetected) {
    twai_message_t faultMessage;
    faultMessage.identifier = FAULT_MSG_ID;
    faultMessage.extd = 0;                 // Standard ID
    faultMessage.data_length_code = 1;     // 1 byte of data
    faultMessage.data[0] = faultDetected ? 1 : 0;

    if (twai_transmit(&faultMessage, pdMS_TO_TICKS(1000)) == ESP_OK) {
        Logger::Notice(faultDetected ? "Fault signal sent successfully" : "No fault detected, signal sent");
    } else {
        Logger::Error("Failed to send fault signal");
    }
}

void sendWheelSpeeds(float frontLeft, float frontRight, float rearLeft, float rearRight) {
    twai_message_t wheelMessage;
    wheelMessage.identifier = WHEEL_MSG_ID; 
    wheelMessage.extd = 0;                   // 11-bit ID
    wheelMessage.data_length_code = 8;       // 8 bytes of data (2 bytes per wheel)
    
    // preserving 2 decimal places
    int16_t fl = (int16_t)(frontLeft * 100);
    int16_t fr = (int16_t)(frontRight * 100);
    int16_t rl = (int16_t)(rearLeft * 100);
    int16_t rr = (int16_t)(rearRight * 100);
    
    // extracting upper and lower bytes 
    wheelMessage.data[0] = (uint8_t)(fl >> 8);    
    wheelMessage.data[1] = (uint8_t)(fl & 0xFF);  
    wheelMessage.data[2] = (uint8_t)(fr >> 8);   
    wheelMessage.data[3] = (uint8_t)(fr & 0xFF);  
    wheelMessage.data[4] = (uint8_t)(rl >> 8);    
    wheelMessage.data[5] = (uint8_t)(rl & 0xFF);  
    wheelMessage.data[6] = (uint8_t)(rr >> 8);    
    wheelMessage.data[7] = (uint8_t)(rr & 0xFF);  
    
    if (twai_transmit(&wheelMessage, pdMS_TO_TICKS(1000)) == ESP_OK) {
        Logger::Notice("Wheel speeds sent successfully");
    } else {
        Logger::Error("Failed to send wheel speeds");
    }
}

void setup()
{
  Logger::Start();
  Logger::Notice("Setup");
  //WheelSpeedSetup();
  // CanDriver::CanInit();

  SensorTest.Initialize();
  //CoolantPressure.Initialize();
  //CoolantTemperature.Initialize();
}

// Main
void loop()
{
  Logger::Notice("Hello World");

  SensorTest.GetData();
  //float pressure = CoolantPressure.GetData();
  //float temp = CoolantTemperature.GetData();

  // if (pressure < MIN_PRESSURE || pressure > MAX_PRESSURE || temp < MIN_TEMP || temp > MAX_TEMP){
  //   Logger::Error("Fault detected! Sending fault signal.");
  //   faultDetected = true; 
  // } else {
  //       faultDetected = false;
  //   }

  // sendFaultSignal(faultDetected);
  // sendWheelSpeeds(fl, fr, rl, rr);
  
  //WheelSpeedReset();
  delay(1000);
}
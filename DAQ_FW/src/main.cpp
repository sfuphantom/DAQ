#include "Logger.h"
#include "IADCSensor.h"
#include "wheelSpeed.h"
#include "can.h"
#include "stdlib.h"
#include <Arduino.h>
#define SENSOR_TEST_MODE 1 

// Cooling temp for the outer by cooling is 80 celcius max
// Inlet, coming out of the radiator, is 75 celccius max 

#define MAX_TEMP_1 80
#define MAX_TEMP_2 75 
#define MIN_TEMP 0
#define MIN_PRESSURE 1.3 
#define MAX_PRESSURE 1.7 

#define FAULT_MSG_ID 0x100
#define WHEEL_MSG_ID 0x200

// CAN pins CHANGED FROM 21 and 22 TO 4 AND 5 BCS OF OVERLAP
#define CAN_TX GPIO_NUM_4 
#define CAN_RX GPIO_NUM_5 

// cooloant pressure sensor object decleration, MAKE SURE TO USE DIFF CHANNELS OF U1, we only have one chip physically
CoolantPressureSensor CoolantPressure1("CoolantPressureSensor", 2, ADCAddress::U2);
// CoolantPressureSensor CoolantPressure2("CoolantPressure2", 3, ADCAddress::U2); 

// coolant tempature sensor object decleration
CoolantTemperatureSensor CoolantTemperature1("CoolantTemperatureSensor", 0, ADCAddress::U2);
CoolantTemperatureSensor CoolantTemperature2("CoolantTemperatureSensor2", 1, ADCAddress::U2);


void sendWheelSpeed(float wheelSpeed) 
{
    Logger::Notice("Sending wheel speeds over CAN: %.2f m/s", wheelSpeed);
    Logger::Notice("The wheelspeed values are scaled by 100. Two bytes are sent each time");

    int16_t scaledSpeed = static_cast<int16_t>(wheelSpeed * 100);

    // Sending scaled by 100 value *DASH NEEDS TO REVERSE SCALE BY 100*
    CAN_SendInt16(WHEEL_MSG_ID, scaledSpeed);

    Logger::Notice("Wheel speed sent over CAN, float to int16, (scaled=%d)", scaledSpeed);
}

void setup()
{
    Serial.begin(BAUD_RATE);
    delay(1000);
    Wire.begin(21, 22); // SDA = GPIO21, SCL = GPIO22 for ESP32

    Logger::Start();
    Logger::Notice("Setup");

    CAN_Init();

    CoolantTemperature1.Initialize();
    CoolantTemperature2.Initialize();
    CoolantPressure1.Initialize();
    //   CoolantPressure2.Initialize();

    // Send initialization message
    #if !SENSOR_TEST_MODE
        CanDriver::sendCanData(nullptr, 1, FAULT_MSG_ID, 0, false);
    #endif
    Logger::Notice("Setup complete");
}

// Main
void loop()
{
    Logger::Notice("Starting Main Loop");

    float temp1 = CoolantTemperature1.GetData();
    float temp2 = CoolantTemperature2.GetData();
    float pressure1 = CoolantPressure1.GetData();
    // float pressure2 = CoolantPressure2.GetData();

    Logger::Trace("Coolant Temp1: %.2f C", temp1);
    Logger::Trace("Coolant Temp2: %.2f C", temp2);
    Logger::Trace("Coolant Pressure1: %.2f bar", pressure1);
    // Logger::Trace("Coolant Pressure2: %.2f bar", pressure2);

    Serial.print("Temp1: ");
    Serial.print(temp1);
    Serial.print(" C, Temp2: ");
    Serial.print(temp2);
    Serial.print(" C, Pressure1: ");
    Serial.print(pressure1);
    Serial.println(" bar");

    #if SENSOR_TEST_MODE
        delay(1000);
        return;
    #endif

    // pressure2 < MIN_PRESSURE || pressure2 > MAX_PRESSURE ||

    if (pressure1 < MIN_PRESSURE || pressure1 > MAX_PRESSURE || temp1 < MIN_TEMP || temp1 > MAX_TEMP_1 || temp2 < MIN_TEMP || temp2 > MAX_TEMP_2){
        Logger::Error("FAULT DETECTED: Out of range coolant values");
        CAN_SendInt16(FAULT_MSG_ID, 1);
    }

    // reads interrupts, median filters, stores final value
    WheelSpeedReset();
    float speed = getFinalWheelSpeed(); 

    sendWheelSpeed(speed);

    delay(1000); 
}

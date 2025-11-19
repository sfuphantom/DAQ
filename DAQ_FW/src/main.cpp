#include "Logger.h"
#include "IADCSensor.h"
#include "wheelSpeed.h"
#include "can.h"
#include "stdlib.h"
#include <Arduino.h>
#include <math.h>
#include <stdio.h>
#define SENSOR_TEST_MODE 1

// Toggle individual sensors 
#define ENABLE_TEMP_SENSOR_1 1
#define ENABLE_TEMP_SENSOR_2 1
#define ENABLE_PRESSURE_SENSOR_1 1
#define ENABLE_PRESSURE_SENSOR_2 0

// Cooling temp for the outer by cooling is 80 celcius max
// Inlet, coming out of the radiator, is 75 celccius max 

#define MAX_TEMP_1 80
#define MAX_TEMP_2 75 
#define MIN_TEMP 0
#define MIN_PRESSURE 1.3 
#define MAX_PRESSURE 1.7 

#define FAULT_MSG_ID 0x100
#define WHEEL_MSG_ID 0x200

#define FAULT_SUPPRESS_MS 3000

// CAN pins CHANGED FROM 21 and 22 TO 4 AND 5 BCS OF OVERLAP
#define CAN_TX GPIO_NUM_4 
#define CAN_RX GPIO_NUM_5 

#if ENABLE_PRESSURE_SENSOR_1
CoolantPressureSensor CoolantPressure1("CoolantPressureSensor", 2, ADCAddress::U2);
#endif

#if ENABLE_PRESSURE_SENSOR_2
CoolantPressureSensor CoolantPressure2("CoolantPressureSensor2", 3, ADCAddress::U2);
#endif

#if ENABLE_TEMP_SENSOR_1
CoolantTemperatureSensor CoolantTemperature1("CoolantTemperatureSensor", 0, ADCAddress::U2);
#endif

#if ENABLE_TEMP_SENSOR_2
CoolantTemperatureSensor CoolantTemperature2("CoolantTemperatureSensor2", 1, ADCAddress::U2);
#endif

namespace
{
    bool faultReported = false;
    unsigned long startupTime = 0;
}

const char *formatSensorValue(float value, char *buffer, size_t length, uint8_t precision)
{
    if (isnan(value))
    {
        snprintf(buffer, length, "null");
    }
    else
    {
        snprintf(buffer, length, "%.*f", precision, value);
    }
    return buffer;
}

bool valueOutOfRange(float value, float minValue, float maxValue)
{
    if (isnan(value))
    {
        return false;
    }
    return value < minValue || value > maxValue;
}

void logSensorSnapshot(float temp1, float temp2, float pressure1, float pressure2)
{
    char temp1Buffer[16];
    char temp2Buffer[16];
    char pressure1Buffer[16];
    char pressure2Buffer[16];

    Serial.println();
    Logger::Notice("[Data] Temp1: %s C, Temp2: %s C, Pressure1: %s bar, Pressure2: %s bar",
                   formatSensorValue(temp1, temp1Buffer, sizeof(temp1Buffer), 1),
                   formatSensorValue(temp2, temp2Buffer, sizeof(temp2Buffer), 1),
                   formatSensorValue(pressure1, pressure1Buffer, sizeof(pressure1Buffer), 2),
                   formatSensorValue(pressure2, pressure2Buffer, sizeof(pressure2Buffer), 2));
    Serial.println();
}

void sendWheelSpeed(float wheelSpeed) 
{
    int16_t scaledSpeed = static_cast<int16_t>(wheelSpeed * 100);

    // Sending scaled by 100 value *DASH NEEDS TO REVERSE SCALE BY 100*
    CAN_SendInt16(WHEEL_MSG_ID, scaledSpeed);
    Serial.println();
    Logger::Trace("[WheelSpeed] %.2f m/s (scaled=%d)", wheelSpeed, scaledSpeed);
    Serial.println();
}

void setup()
{
    Serial.begin(BAUD_RATE);
    delay(1000);
    Wire.begin(21, 22); // SDA = GPIO21, SCL = GPIO22 for ESP32

    Logger::Start();
    CAN_Init();
    Serial.println();
    Logger::Notice("CAN initialized");

#if ENABLE_TEMP_SENSOR_1
    CoolantTemperature1.Initialize();
#endif
#if ENABLE_TEMP_SENSOR_2
    CoolantTemperature2.Initialize();
#endif
#if ENABLE_PRESSURE_SENSOR_1
    CoolantPressure1.Initialize();
#endif
#if ENABLE_PRESSURE_SENSOR_2
    CoolantPressure2.Initialize();
#endif

    // Send initialization message
    #if !SENSOR_TEST_MODE
        CanDriver::sendCanData(nullptr, 1, FAULT_MSG_ID, 0, false);
    #endif
    Logger::Notice("Setup complete");
    Logger::Notice("Starting main loop...");
    Serial.println();

    startupTime = millis();
}


void loop()
{
#if ENABLE_TEMP_SENSOR_1
    float temp1 = CoolantTemperature1.GetData();
#else
    float temp1 = NAN;
#endif
#if ENABLE_TEMP_SENSOR_2
    float temp2 = CoolantTemperature2.GetData();
#else
    float temp2 = NAN;
#endif
#if ENABLE_PRESSURE_SENSOR_1
    float pressure1 = CoolantPressure1.GetData();
#else
    float pressure1 = NAN;
#endif
#if ENABLE_PRESSURE_SENSOR_2
    float pressure2 = CoolantPressure2.GetData();
#else
    float pressure2 = NAN;
#endif

    static unsigned long lastLogTime = 0;
    unsigned long now = millis();
    if (now - lastLogTime >= 1000) {
        logSensorSnapshot(temp1, temp2, pressure1, pressure2);
        lastLogTime = now;
    }
    #if SENSOR_TEST_MODE
        delay(1000);
        return;
    #endif

    bool warmupComplete = (millis() - startupTime) >= FAULT_SUPPRESS_MS;
    bool rawFaultDetected = valueOutOfRange(pressure1, MIN_PRESSURE, MAX_PRESSURE) ||
                            valueOutOfRange(pressure2, MIN_PRESSURE, MAX_PRESSURE) ||
                            valueOutOfRange(temp1, MIN_TEMP, MAX_TEMP_1) ||
                            valueOutOfRange(temp2, MIN_TEMP, MAX_TEMP_2);

    bool faultDetected = warmupComplete && rawFaultDetected;

    if (faultDetected)
    {
        if (!faultReported)
        {
            Serial.println();
            Logger::Error("FAULT DETECTED: coolant values outside limits");
        }
        CAN_SendInt16(FAULT_MSG_ID, 1);
    }
    else if (faultReported)
    {   
        Serial.println();
        Logger::Notice("Coolant values back within range");
    }
    faultReported = faultDetected;

    // reads interrupts, median filters, stores final value
    WheelSpeedReset();
    float speed = getFinalWheelSpeed(); 

    sendWheelSpeed(speed);

}

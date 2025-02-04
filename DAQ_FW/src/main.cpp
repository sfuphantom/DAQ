#include "Logger.h"
#include "IADCSensor.h"
#include "wheelSpeed.h"
#include <Arduino.h>
#include <driver/twai.h>
#include <SD.h>
#include <time.h>

// NTP server and timezone
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -28800;          // GMT-8 (Pacific Standard Time)
const int daylightOffset_sec = 3600;        // Adjust for daylight savings (1h right now)

MAX_TEMP = 4
MIN_TEMP =  2
MIN_PRESSURE = 1.3 
MAX_PRESSURE = 1.7 

#define FAULT_MSG_ID 0x100
// CAN pins
#define CAN_TX GPIO_NUM_21
#define CAN_RX GPIO_NUM_22

#define SD_CS_PIN 5  // GPIO for SD card Chip Select, Connect to any GPIO 

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

  // Initilize CD card
  if (!SD.begin(SD_CS_PIN)) {
    Logger::Error("Failed to initialize SD card");
    while (1);  

  Logger::Notice("SD card initialized");

  // Initialize time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Logger::Notice("Waiting for time sync...");
  while (!time(nullptr)) {
    delay(1000);  
  }
  Logger::Notice("Time synchronized");
 }

String getTimestamp() {
  time_t now = time(nullptr);
  struct tm *timeInfo = localtime(&now);

  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo); 
  return String(buffer);
 }
}

void loop()
{
  // put your main code here, to run repeatedly:
  Logger::Notice("Hello World");

  //SensorTest.GetData();

  //float pressure = CoolantPressure.GetData();

  //float temp = CoolantTemperature.GetData();
  
  String timestamp = getTimestamp();

  if (pressure < MIN_PRESSURE || pressure > MAX_PRESSURE || temp < MIN_TEMP || temp > MAX_TEMP){
    Logger::Error("Fault detected! Sending fault signal.");
    faultDetected = true; 
  }

  // Writing to CD Card
  File pressureFile = SD.open("/pressure_data.txt", FILE_APPEND);
  if (pressureFile) {
    pressureFile.println(timestamp + ", " + String(pressure));
    pressureFile.close();
    Logger::Notice("Pressure logged: " + String(pressure));
  } else {
    Logger::Error("Failed to open pressure file for writing");
  }

  File temperatureFile = SD.open("/temperature_data.txt", FILE_APPEND);
  if (temperatureFile) {
    temperatureFile.println(timestamp + ", " + String(temp));
    temperatureFile.close();
    Logger::Notice("Temperature logged: " + String(temp));
  } else {
    Logger::Error("Failed to open temperature file for writing");
  }

  WheelSpeedReset();
  delay(1000);
}

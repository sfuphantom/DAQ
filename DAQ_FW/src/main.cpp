#include "Logger.h"
#include "IADCSensor.h"
#include "wheelSpeed.h"

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

  //CoolantPressure.GetData();

  //CoolantTemperature.GetData();

  WheelSpeedReset();
  delay(1000);
}
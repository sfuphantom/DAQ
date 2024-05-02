#include "Logger.h"
#include "IADCSensor.h"

// object declarations can't be done in setup()

// params: sensorname, sensorID, adcAddress
ChildExample SensorTest("TestSensor", 1, ADCAddress::U1);

// coloant pressure sensor object decleration
CoolantPressureSensor CoolantPressure("CoolantPressureSensor", 2, ADCAddress::U2);

void setup()
{
  // put your setup code here, to run once:
  Logger::Start();
  Logger::Notice("Setup");

  SensorTest.Initialize();

  CoolantPressure.Initialize();
}

void loop()
{
  // put your main code here, to run repeatedly:
  Logger::Notice("Hello World");

  SensorTest.GetData();

  CoolantPressure.GetData();

  delay(1000);
}

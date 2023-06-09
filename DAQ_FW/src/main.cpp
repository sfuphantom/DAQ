
#include "Logger.h"
#include "IADCSensor.h"

// object declarations can't be done in setup()

// params: sensorname, sensorID, adcID
IADCSensor SensorTest("TestSensor", 1, 0);

void setup()
{
  // put your setup code here, to run once:
  Logger::Start();
  Logger::Notice("Setup");

  SensorTest.Initialize();
}

void loop()
{
  // put your main code here, to run repeatedly:
  Logger::Notice("Hello World");

  SensorTest.Process();

  delay(1000);
}
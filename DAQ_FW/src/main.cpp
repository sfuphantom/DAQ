
#include "Logger.h"
#include "IADCSensor.h"

// object declarations can't be done in setup()

// params: sensorname, sensorID, adcID, gainmode
IADCSensor SensorTest("TestSensor", 1, 1, GAIN_TWO);

void setup()
{
  // put your setup code here, to run once:
  Logger::Start();
  Logger::Notice("Setup");
}

void loop()
{
  // put your main code here, to run repeatedly:

  Logger::Notice("Hello World");

  SensorTest.Process();

  delay(1000);
}

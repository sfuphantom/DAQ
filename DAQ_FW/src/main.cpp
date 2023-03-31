
#include "Logger.h"
#include "ISensorData.h"

// object declarations can't be done in setup()
IsensorData SensorTest(GAIN_TWO);

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

  SensorTest.Read();
  SensorTest.Process();

  delay(1000);
}

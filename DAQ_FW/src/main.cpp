#include "Logger.h"

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

  delay(1000);
}

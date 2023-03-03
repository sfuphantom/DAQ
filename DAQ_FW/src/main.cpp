#include <Arduino.h>
#include "Log.h"
using namespace Logger;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Logger::Start();
  Notice("Setup");
}

void loop()
{
  // put your main code here, to run repeatedly:
  Notice("Hello World");

  delay(1000);
}

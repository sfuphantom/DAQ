#include <Arduino.h>
#include <iostream>
#include <wheelSpeed.h>

void wheelSpeedDisplay(int volatile wheelPulses, int wheelNumber){
  
  // calculate RPM from wheel pulse count
  int RPM = (wheelPulses/49.0)*60; // subject to change based on tire 
  
  // print results
  Serial.print("Wheel ");
  Serial.print(wheelNumber);
  Serial.print(" RPM = ");
  Serial.println(RPM);

  // print wheel pulse count for testing purposes
  //Serial.print(wheelPulses);
}
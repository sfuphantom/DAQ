#include <Arduino.h>
#include <iostream>
#include <wheelSpeed.h>

void wheelSpeedDisplay(int volatile WP1, int volatile WP2, int volatile WP3, int volatile WP4){
  
  // average wheel pulse counts

  //int wheelPulses = 0.25*(WP1 + WP2 + WP3 + WP4);
  int wheelPulses = WP1;

  // calculate angular velocity from wheel pulse count

  int omega = (wheelPulses / 49.0)*2*3.14; // wheel pulses divided by 49.0 indents per tire, in radians

  // convert to linear velocity

  int velocity = 0.25 * omega * 3.6; // for r = 10", 3.6 is proportionality const from m/s to km/h

  // calculate average RPM from wheel pulse count
  //int RPM = (wheelPulses/49.0)*60; // subject to change based on tire 

  
  // print results
  Serial.print("Speed = ");
  Serial.println(velocity, 0); // print with no decimal places

  //Serial.print("Wheel ");
  //Serial.print(wheelNumber);
  //Serial.print(" RPM = ");
  //Serial.println(RPM);

  // print wheel pulse count for testing purposes
  //Serial.print(wheelPulses);
}
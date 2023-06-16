#include <Arduino.h>
#include <iostream>

#define temp_PIN 4 // pin numbers
#define brakePressure_PIN 5
#define coolantPressure_PIN 6

#define wheelSpeed_PIN 14

#define tempConst 5 // adjustable temperature proportionality constant for testing

uint16_t wheelPulses=120; // set at random number for testing

void IRAM_ATTR ISR(){ // interrupt service routine
  wheelPulses ++; 
}

void setup() {
  // put your setup code here, to run once:
  /*pinMode(temp_PIN, OUTPUT);
  pinMode(brakePressure_PIN, OUTPUT);
  pinMode(coolantPressure_PIN, OUTPUT); */
  pinMode(wheelSpeed_PIN, OUTPUT);

  // attach interrupt that adds to pulse count when pin goes from HIGH to LOW
  attachInterrupt(wheelSpeed_PIN, ISR, FALLING); 

  Serial.begin(9600);
  Serial.println("Connected");
};

void loop() {
  //------------- functions for temperature and pressure sensors ------------//

  //int tempData = analogRead(temp_PIN);
  //int brakePressureData = analogRead(brakePressure_PIN);
  //int tirePressureData = analogRead(coolantPressure_PIN);

  // example implementation of functions
  // data = brakePressure(data); 

  // for tempurature sensors - analog reading is directly proportional to temperature in deg-C
  //uint16_t temperature = tempData*tempConst; 

  //for brake pressure sensor (MIPAN2XX500PSAAX) 
  //uint16_t brakePressure = 625*(brakePressureData-0.5); // in PSI, V_supply @ 25 deg-C = 5V (ratiometric)  //display data

  //for coolant pressure sensor (116CP31-M04S2-50)
  //uint16_t tirePressure = 20*(tirePressureData-0.5);

  // for wheel speed sensor
  //wheelPulses = 0; // start at 0 

  //Serial.print("Temperature reading= ");
  //Serial.println(temperature);

  //Serial.print("Brake pressure reading= ");
  //Serial.println(brakePressure);
  
  //Serial.print("Tire pressure reading= ");
  //Serial.println(tirePressure);

  //-------------- wheel speed interrupt data --------------//

  delay(1000); // interrupt detects pulses over 1000ms
  uint16_t wheelRPM = wheelPulses/20*60; //  20 pulses from wheel speed sensor corresponds to 1 revolution

  Serial.print("Wheel speed (RPM)= ");
  Serial.println(wheelRPM);

  //-------------- wheel speed sensor testing --------------//

  int wheelSpeedData = analogRead(wheelSpeed_PIN);

  
  if (wheelSpeedData == 1){
      Serial.println("Unblocked"); //test
  }; 
  
  if (wheelSpeedData==0){
      Serial.println("Blocked"); //test
  };

  //Serial.println(wheelPulses);

  };


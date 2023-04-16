#include <Arduino.h>
#include <ISensorDriver.h>

int sr = 4; // pin number

void setup() {
  // put your setup code here, to run once:
  pinMode(sr, OUTPUT);
  Serial.begin(9600);
  Serial.println("Connected");
};

void loop() {
  int data = analogRead(sr);

  float x=0.5; //temp sensor proportionality constant

  // example implementation of functions
  // data = brakePressure(data); 

  //display data
  Serial.print("Sensor reading=");
  Serial.println(data);

  delay(1000);
  };
#include <Arduino.h>
#include <iostream>

#define frontLeft_WSPIN 14
#define frontRight_WSPIN 16
#define backLeft_WSPIN 18
#define backRight_WSPIN 20

int volatile WP1=0; // set at random number for testing
int volatile WP2=0; // set at random number for testing
int volatile WP3=0; // set at random number for testing
int volatile WP4=0; // set at random number for testing

int volatile irFlag1 = 0; 
int volatile irFlag2 = 0; 
int volatile irFlag3 = 0; 
int volatile irFlag4 = 0; 

void IRAM_ATTR ISR1(){ // interrupt service routine
   irFlag1 = 1;
   WP1++;
}

void IRAM_ATTR ISR2(){ // interrupt service routine
   irFlag2 = 1;
   WP2++;
}

void IRAM_ATTR ISR3(){ // interrupt service routine
   irFlag3 = 1;
   WP3++;
}

void IRAM_ATTR ISR4(){ // interrupt service routine
   irFlag4 = 1;
   WP4++;
}

void interruptReset1(){
  detachInterrupt(frontLeft_WSPIN);
  
  irFlag1 = 0;
  WP1= 0;

  attachInterrupt(frontLeft_WSPIN, ISR1, FALLING);
};

void interruptReset2(){
  detachInterrupt(frontRight_WSPIN);
  
  irFlag2 = 0;
  WP2= 0;

  attachInterrupt(frontRight_WSPIN, ISR2, FALLING);
};

void interruptReset3(){
  detachInterrupt(backLeft_WSPIN);
  
  irFlag3 = 0;
  WP3= 0;

  attachInterrupt(backLeft_WSPIN, ISR3, FALLING);
};

void interruptReset4(){
  detachInterrupt(backRight_WSPIN);
  
  irFlag4 = 0;
  WP4= 0;

  attachInterrupt(backRight_WSPIN, ISR4, FALLING);
};

void setup() {
  pinMode(frontRight_WSPIN, INPUT);

  // attach interrupt that adds to pulse count when pin goes from HIGH to LOW
  attachInterrupt(frontLeft_WSPIN, ISR1, FALLING); 
  attachInterrupt(frontRight_WSPIN, ISR2, FALLING); 
  attachInterrupt(backLeft_WSPIN, ISR3, FALLING); 
  attachInterrupt(backRight_WSPIN, ISR4, FALLING); 


  Serial.begin(9600);
  Serial.println("Connected");

  
};

void loop() {
  //-------------- wheel speed interrupt data --------------//
  if(irFlag1){
    delay(1000); // interrupt detects pulses over 1000ms

    int RPM1 = WP1/49*60; //  20 pulses from wheel speed sensor corresponds to 1 revolution
  
    Serial.println(WP1);
    Serial.print("Wheel 1 Speed (RPM)= ");
    Serial.println(RPM1);


    interruptReset1();
  };

    if(irFlag1){
    delay(1000); // interrupt detects pulses over 1000ms

    int RPM1 = WP1/49*60; //  20 pulses from wheel speed sensor corresponds to 1 revolution
  
    Serial.println(WP1);
    Serial.print("Wheel 1 Speed (RPM)= ");
    Serial.println(RPM1);


    interruptReset1();
  };

    if(irFlag2){
    delay(1000); // interrupt detects pulses over 1000ms

    int RPM2 = WP2/49*60; //  20 pulses from wheel speed sensor corresponds to 1 revolution
  
    Serial.println(WP2);
    Serial.print("Wheel 2 Speed (RPM)= ");
    Serial.println(RPM2);


    interruptReset2();
  };

    if(irFlag3){
    delay(1000); // interrupt detects pulses over 1000ms

    int RPM3 = WP1/49*60; //  20 pulses from wheel speed sensor corresponds to 1 revolution
  
    Serial.println(WP3);
    Serial.print("Wheel 3 Speed (RPM)= ");
    Serial.println(RPM3);


    interruptReset1();
  };

    if(irFlag4){
    delay(1000); // interrupt detects pulses over 1000ms

    int RPM4 = WP4/49*60; //  20 pulses from wheel speed sensor corresponds to 1 revolution
  
    Serial.println(WP4);
    Serial.print("Wheel 4 Speed (RPM)= ");
    Serial.println(RPM4);


    interruptReset4();
  };

};
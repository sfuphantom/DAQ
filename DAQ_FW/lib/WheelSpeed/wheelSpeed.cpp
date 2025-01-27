#include <Arduino.h>
#include <iostream>
#include <wheelSpeed.h>


// define sensor pins

#define frontLeft_WSPIN 32 // wheel #1
#define frontRight_WSPIN 33 // wheel #2
#define backLeft_WSPIN 34 // wheel #3
#define backRight_WSPIN 35 // wheel #4

// hardware timer pointer
hw_timer_t * timer = NULL;

// define variables for counting

int volatile WP1=0; 
int volatile WP2=0; 
int volatile WP3=0; 
int volatile WP4=0; 

int volatile timerCounter = 0;

// interrupt service routines: count interrupts

void IRAM_ATTR ISR1(){ WP1++;}

void IRAM_ATTR ISR2(){ WP2++;}

void IRAM_ATTR ISR3(){ WP3++;}

void IRAM_ATTR ISR4(){ WP4++;}


// interrupt reset functions: detach and reset interrupts after each period

void interruptReset1(){
  detachInterrupt(frontLeft_WSPIN);  
  WP1= 0;
  attachInterrupt(frontLeft_WSPIN, ISR1, FALLING);
}

void interruptReset2(){
  detachInterrupt(frontRight_WSPIN);  
  WP2= 0;
  attachInterrupt(frontRight_WSPIN, ISR2, FALLING);
}

void interruptReset3(){
  detachInterrupt(backLeft_WSPIN);
  WP3= 0;
  attachInterrupt(backLeft_WSPIN, ISR3, FALLING);
}

void interruptReset4(){
  detachInterrupt(backRight_WSPIN);
  WP4= 0;
  attachInterrupt(backRight_WSPIN, ISR4, FALLING);
}


// timer interrupt service routine

void IRAM_ATTR onTimer(){ timerCounter ++;}

void wheelSpeedDisplay(int volatile WP1, int volatile WP2, int volatile WP3, int volatile WP4){
  
  // average wheel pulse counts

  //int wheelPulses = 0.25*(WP1 + WP2 + WP3 + WP4);
  printf("Wheel 1: %d\n", WP1);
  printf("Wheel 2: %d\n", WP2);
  printf("Wheel 3: %d\n", WP3);
  printf("Wheel 4: %d\n", WP4);
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


void WheelSpeedSetup(){
  pinMode(frontRight_WSPIN, INPUT);
  pinMode(frontLeft_WSPIN, INPUT);
  pinMode(backLeft_WSPIN, INPUT);
  pinMode(backRight_WSPIN, INPUT);

  // attach interrupt that adds to pulse count when pin goes from HIGH to LOW

  attachInterrupt(frontLeft_WSPIN, ISR1, FALLING); 
  attachInterrupt(frontRight_WSPIN, ISR2, FALLING); 
  attachInterrupt(backLeft_WSPIN, ISR3, FALLING); 
  attachInterrupt(backRight_WSPIN, ISR4, FALLING); 

  // begin timer

  timer = timerBegin(0, 80, true); 

  timerAttachInterrupt(timer, &onTimer, true); // attach timer interrupt
  timerAlarmWrite(timer, 1000000, true); // 1 second interrupt period
  timerAlarmEnable(timer); // enable timer interrupt
}

void WheelSpeedReset(){
  if (timerCounter > 0){
    timerCounter --;

    wheelSpeedDisplay(WP1, WP2, WP3, WP4); // prints velocity based on average wheel pulse counts


    // detach/reattach interrupts
    
    interruptReset1();
    interruptReset2();
    interruptReset3();
    interruptReset4();
    };

}
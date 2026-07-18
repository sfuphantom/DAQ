#include <Arduino.h>
#include "wheelSpeed.h"

// define sensor pins
#define frontLeft_WSPIN 34 // wheel #1
#define frontRight_WSPIN 35 // wheel #2
#define backLeft_WSPIN 32 // wheel #3
#define backRight_WSPIN 33 // wheel #4

// for getter function
static float finalWheelSpeed = 0.0f;
static float lastSpeedFL = 0.0f;
static float lastSpeedFR = 0.0f;
static float lastSpeedRL = 0.0f;
static float lastSpeedRR = 0.0f;

// hardware timer pointer
hw_timer_t * timer = NULL;

// define variables for counting
int volatile WP1=0; 
int volatile WP2=0; 
int volatile WP3=0; 
int volatile WP4=0; 

int volatile timerCounter = 0;

// ESP32 critical section guard for ISR-shared data
portMUX_TYPE wpMux = portMUX_INITIALIZER_UNLOCKED;

// tone ring / encoder tooth count
const float PULSES_PER_REV = 49.0; 

// tire diameter (meters)
const float TIRE_DIAMETER_M = 0.533; // CHANGE FOR ACCURACY 

// π * D
const float TIRE_CIRCUMFERENCE = TIRE_DIAMETER_M * 3.14159;

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

  Serial.printf("Wheel 1: %d\n", WP1);
  Serial.printf("Wheel 2: %d\n", WP2);
  Serial.printf("Wheel 3: %d\n", WP3);
  Serial.printf("Wheel 4: %d\n", WP4);

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

// frequency-to-speed conversion using pulses per wheel revolution (PPR), km/h 
// hall effect wheel speed forumla 
float convertPulsesToSpeed(int pulseCount, float samplePeriodSec){
    float frequencyHz = pulseCount / samplePeriodSec;
    float speedKmh = (frequencyHz / PULSES_PER_REV) * TIRE_CIRCUMFERENCE * 3.6;
    return speedKmh;
}

float calculateWheelSpeed(float speedFL,float speedFR, float speedRL, float speedRR){
  float speeds[4] = {speedFL, speedFR, speedRL, speedRR};

  // bubble sort for median
  for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3 - i; j++) {
            if (speeds[j] > speeds[j + 1]) {
                float tmp = speeds[j];
                speeds[j] = speeds[j + 1];
                speeds[j + 1]  = tmp;
            }
        }
    }

  float finalWheelSpeed = (speeds[1] + speeds[2]) / 2.0f;
  return finalWheelSpeed;
}

// getter function for main 
float getFinalWheelSpeed(){
    return finalWheelSpeed;
}

float getWheelSpeedFL(){
    return lastSpeedFL;
}

float getWheelSpeedFR(){
    return lastSpeedFR;
}

float getWheelSpeedRL(){
    return lastSpeedRL;
}

float getWheelSpeedRR(){
    return lastSpeedRR;
}

void WheelSpeedReset(){
  if (timerCounter > 0){
    timerCounter --;
    
    int wp1, wp2, wp3, wp4;

    portENTER_CRITICAL(&wpMux);
    wp1 = WP1; wp2 = WP2; wp3 = WP3; wp4 = WP4;
    WP1 = WP2 = WP3 = WP4 = 0;
    portEXIT_CRITICAL(&wpMux);

    // prints raw values
    wheelSpeedDisplay(wp1, wp2, wp3, wp4); 

    // 1 sec sample window
    float speedFL = convertPulsesToSpeed(wp1, 1.0);   
    float speedFR = convertPulsesToSpeed(wp2, 1.0);
    float speedRL = convertPulsesToSpeed(wp3, 1.0);
    float speedRR = convertPulsesToSpeed(wp4, 1.0);

    Serial.printf("FL: %.2f km/h  FR: %.2f km/h  RL: %.2f km/h  RR: %.2f km/h\n", speedFL, speedFR, speedRL, speedRR);
    lastSpeedFL = speedFL;
    lastSpeedFR = speedFR;
    lastSpeedRL = speedRL;
    lastSpeedRR = speedRR;
    finalWheelSpeed = calculateWheelSpeed(speedFL, speedFR, speedRL, speedRR);
    Serial.printf("Final wheelspeed value: %.2f km/h\n", finalWheelSpeed);

    // detach/reattach interrupts
    interruptReset1();
    interruptReset2();
    interruptReset3();
    interruptReset4();
    };

}

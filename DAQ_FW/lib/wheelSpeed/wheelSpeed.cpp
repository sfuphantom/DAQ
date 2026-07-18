#include <Arduino.h>
#include "wheelSpeed.h"
#include "systemConfig.h"

// for getter function
static float finalWheelSpeed = 0.0f;
static float lastSpeedFL = 0.0f;
static float lastSpeedFR = 0.0f;
static float lastSpeedRL = 0.0f;
static float lastSpeedRR = 0.0f;

// hardware timer pointer
static hw_timer_t * timer = NULL;

// define variables for counting
static volatile int WP1=0;
static volatile int WP2=0;
static volatile int WP3=0;
static volatile int WP4=0;

static volatile int timerCounter = 0;

// ESP32 critical section guard for ISR-shared data
static portMUX_TYPE wpMux = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// spokes count
const float PULSES_PER_REV = 49.0;

// tire diameter (meters)
const float TIRE_DIAMETER_M = 0.533; // CHANGE FOR ACCURACY

// π * D
const float TIRE_CIRCUMFERENCE = TIRE_DIAMETER_M * 3.14159;

// interrupt service routines: count interrupts
void IRAM_ATTR wheelPulseFlIsr(){
    portENTER_CRITICAL_ISR(&wpMux);
    WP1++;
    portEXIT_CRITICAL_ISR(&wpMux);
}
void IRAM_ATTR wheelPulseFrIsr(){
    portENTER_CRITICAL_ISR(&wpMux);
    WP2++;
    portEXIT_CRITICAL_ISR(&wpMux);
}
void IRAM_ATTR wheelPulseRlIsr(){
    portENTER_CRITICAL_ISR(&wpMux);
    WP3++;
    portEXIT_CRITICAL_ISR(&wpMux);
}
void IRAM_ATTR wheelPulseRrIsr(){
    portENTER_CRITICAL_ISR(&wpMux);
    WP4++;
    portEXIT_CRITICAL_ISR(&wpMux);
}

void IRAM_ATTR onTimer(){
  portENTER_CRITICAL_ISR(&timerMux);
  timerCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

static void wheelSpeedDisplay(int wp1, int wp2, int wp3, int wp4){
    Serial.printf("Wheel 1: %d\n", wp1);
    Serial.printf("Wheel 2: %d\n", wp2);
    Serial.printf("Wheel 3: %d\n", wp3);
    Serial.printf("Wheel 4: %d\n", wp4);
}

void wheelSpeedSetup(){
  pinMode(static_cast<uint8_t>(WHEEL_SPEED_FR_PIN), INPUT);
  pinMode(static_cast<uint8_t>(WHEEL_SPEED_FL_PIN), INPUT);
  pinMode(static_cast<uint8_t>(WHEEL_SPEED_RL_PIN), INPUT);
  pinMode(static_cast<uint8_t>(WHEEL_SPEED_RR_PIN), INPUT);

  // attach interrupt that adds to pulse count when pin goes from HIGH to LOW
  attachInterrupt(static_cast<uint8_t>(WHEEL_SPEED_FL_PIN), wheelPulseFlIsr, FALLING);
  attachInterrupt(static_cast<uint8_t>(WHEEL_SPEED_FR_PIN), wheelPulseFrIsr, FALLING);
  attachInterrupt(static_cast<uint8_t>(WHEEL_SPEED_RL_PIN), wheelPulseRlIsr, FALLING);
  attachInterrupt(static_cast<uint8_t>(WHEEL_SPEED_RR_PIN), wheelPulseRrIsr, FALLING);

  // begin timer
  timer = timerBegin(0, 80, true);

  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true); // 1 second interrupt period
  timerAlarmEnable(timer); // enable timer interrupt
}

// frequency-to-speed conversion using pulses per wheel revolution (PPR), km/h
// hall effect wheel speed formula
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

  return (speeds[1] + speeds[2]) / 2.0f;
}

// getter function for main
float getFinalWheelSpeed(){
    return finalWheelSpeed;
}

float getWheelSpeedFl(){
    return lastSpeedFL;
}

float getWheelSpeedFr(){
    return lastSpeedFR;
}

float getWheelSpeedRl(){
    return lastSpeedRL;
}

float getWheelSpeedRr(){
    return lastSpeedRR;
}

void wheelSpeedReset(){
  bool sampleReady = false;
  portENTER_CRITICAL(&timerMux);
  if (timerCounter > 0){
    timerCounter --;
    sampleReady = true;
  }
  portEXIT_CRITICAL(&timerMux);
  if (!sampleReady){
    return;
  }

  int wp1, wp2, wp3, wp4;
  portENTER_CRITICAL(&wpMux);
  wp1 = WP1; wp2 = WP2; wp3 = WP3; wp4 = WP4;
  WP1 = WP2 = WP3 = WP4 = 0;
  portEXIT_CRITICAL(&wpMux);

  // 1 sec sample window
  float speedFL = convertPulsesToSpeed(wp1, 1.0);
  float speedFR = convertPulsesToSpeed(wp2, 1.0);
  float speedRL = convertPulsesToSpeed(wp3, 1.0);
  float speedRR = convertPulsesToSpeed(wp4, 1.0);

  lastSpeedFL = speedFL;
  lastSpeedFR = speedFR;
  lastSpeedRL = speedRL;
  lastSpeedRR = speedRR;
  finalWheelSpeed = calculateWheelSpeed(speedFL, speedFR, speedRL, speedRR);

#if ENABLE_STATUS_LOGS
    wheelSpeedDisplay(wp1, wp2, wp3, wp4); // prints raw values
    Serial.printf("FL: %.2f km/h  FR: %.2f km/h  RL: %.2f km/h  RR: %.2f km/h\n", speedFL, speedFR, speedRL, speedRR);
    Serial.printf("Final wheelspeed value: %.2f km/h\n", finalWheelSpeed);
#endif
}

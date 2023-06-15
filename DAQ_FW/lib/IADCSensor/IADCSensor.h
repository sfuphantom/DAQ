#ifndef IADCSENSOR_LIB
#define IADCSENSOR_LIB

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include "Logger.h"
#include "system_config.h"

class IADCSensor
{
public:
    IADCSensor(const char *_SensorName, uint16_t _SensorID, uint16_t _ADC_ID);

    // call in setup(), initializes the adc
    // TODO: may need to be changed as we will have multiple sensors on single adc
    // needs to be hardware tested
    void Initialize();

    // Read function - to be overwritten by the Driver class
    // When child class is ready, make abstract
    virtual int16_t Read();

    float Process();

private:
    Adafruit_ADS1115 mADS;

    // SENSOR METADATA:
    // Currently Generic Parameters, to be expanded as needed
    // TODO: add more specific data, depends on the sensors
    const char *mSensorName;
    const uint32_t mSensorID;
    const uint32_t mADC_ID;
};

#endif
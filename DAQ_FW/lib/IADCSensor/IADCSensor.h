#ifndef IADCSENSOR_LIB
#define IADCSENSOR_LIB

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include "Logger.h"
#include "system_config.h"

class IADCSensor
{
public:
    IADCSensor(const char *_SensorName, const uint16_t _SensorID, const ADCAddress _ADCAddress);

    // call in setup(), initializes the adc
    // needs to be hardware tested
    void Initialize();

    // Read function - to be overwritten by the Driver class
    // When child class is ready, make abstract
    virtual int16_t Read();

    float Process();

    // for logging the address of the current ADC
    const char *PrintAddress();

private:
    Adafruit_ADS1115 mADS;

    // SENSOR METADATA:
    // Currently Generic Parameters, to be expanded as needed
    // TODO: add more specific data, depends on the sensors
    const char *mSensorName;
    const uint32_t mSensorID;
    const ADCAddress mADC_Address;
};

#endif
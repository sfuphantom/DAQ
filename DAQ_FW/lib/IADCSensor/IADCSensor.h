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

    // Reads 16 bits of data from the current mADC
    int16_t Read();

    float GetData();

    // processing code goes here - to be overwritten for different sensor types
    // converts voltage to sensor data, include warnings, errors
    virtual float Process(float inputData) = 0;

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

// example implementation of the senor child class
class ChildExample : public IADCSensor
{
public:
    ChildExample(const char *_SensorName, const uint16_t _SensorID, const ADCAddress _ADCAddress)
        : IADCSensor(_SensorName, _SensorID, _ADCAddress) {}
    float Process(float InputData);
};

class CoolantPressureSensor : public IADCSensor
{
    public:
        CoolantPressureSensor(const char *_SensorName, const uint16_t _SensorID, const ADCAddress _ADCAddress)
            : IADCSensor(_SensorName, _SensorID, _ADCAddress) {}
        
        // convert ADC readings to pressure values
        float Process(float inputData) override; 

    private:
        float convertToPressure(float inputData);
};

#endif

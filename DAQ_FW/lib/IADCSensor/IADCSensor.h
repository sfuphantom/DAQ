#ifndef IADCSENSOR_LIB
#define IADCSENSOR_LIB

#include "Logger.h"
#include "system_config.h"

class IADCSensor
{
public:
    IADCSensor(const char *_SensorName, uint16_t _SensorID, uint16_t _ADC_ID, uint16_t _ADC_GAIN_MODE);

    // Read function - to be overwritten by the Driver class
    // When child class is ready, make abstract
    virtual uint16_t Read() const;

    float Process() const;

private:
    // bit to voltage conversion factor
    float mVOLTAGE_PER_BIT;

    // SENSOR METADATA:
    // Currently Generic Parameters, to be expanded as needed
    // TODO: add more specific data, depends on the sensors
    const char *mSensorName;
    uint32_t mSensorID;
    uint32_t mADC_ID;

    void initializeVoltagePerBit(uint16_t _ADC_GAIN_MODE);
};

#endif
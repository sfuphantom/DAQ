#ifndef ISENSORDATA_LIB
#define ISENSORDATA_LIB

#include "Logger.h"
// #include <Adafruit_ADS1015.h>
#include "system_config.h"

class IsensorData
{
private:
    uint16_t InputData;
    // float VOLTAGE_PER_BIT;
    uint16_t ADC_GAIN_MODE;
    // TODO: store more data about the external adc andsensors for apporpriate processing

    float ADCcovert(uint16_t data);

public:
    // Is there need for default constructor?
    // TODO: add params for different adc and sensor types
    IsensorData(uint16_t _ADC_GAIN_MODE);

    // dummy read function for testing
    void Read();

    float Process();
};

#endif

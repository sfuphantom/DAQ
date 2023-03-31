#include "ISensorData.h"

// TODO: list possible sensor classes:

// ADC INFO
// adc voltage 2V~5.5V, 860 sample rate, SERIAL output, 16 bit resolution, 15 µV min voltage increment
// i2c interface
// PGA offers input ranges from ±0.256 V to ±6.144 V
// adc resolution : 65536

// PGA VALUES for a ADS1115 -
// TODO: MOVE TO SYS_CONFIG WHEN FINAL
#define ADS1115_GAIN_TWOTHIRDS 0.187500f
#define ADS1115_GAIN_ONE 0.125000f
#define ADS1115_GAIN_TWO 0.062500f
#define ADS1115_GAIN_FOUR 0.031250f
#define ADS1115_GAIN_EIGHT 0.015625f
#define ADS1115_GAIN_SIXTEEN 0.007813f


// CLASS METHODS:

// If no other declarations, can be put into header file
// TODO: add other possible parameters
IsensorData::IsensorData(uint16_t _ADC_GAIN_MODE) : ADC_GAIN_MODE(_ADC_GAIN_MODE)
{
    Logger::Notice("Class created with input: %u and gain mode: %u", InputData, ADC_GAIN_MODE);
    // TODO: Possible ADC/Sensor Declarations - Could be carried over from the driver class (?)
}

float IsensorData::Process()
{
    Logger::Trace("Initial Data: %u ", InputData);

    float final_data = ADCcovert(InputData);

    // processing code goes here - to be done for different sensor types
    // converts voltage to sensor data, include warnings, errors

    Logger::Notice("Processed Data: %D ", final_data);

    return (float)final_data;
}

// dummy read function for testing
void IsensorData::Read()
{
    InputData = 16500;
};

float IsensorData::ADCcovert(uint16_t data)
{
    float VOLTAGE_PER_BIT;

    switch (ADC_GAIN_MODE)
    {
    case GAIN_TWOTHIRDS:
        VOLTAGE_PER_BIT = ADS1115_GAIN_TWOTHIRDS;
        break;
    case GAIN_ONE:
        VOLTAGE_PER_BIT = ADS1115_GAIN_ONE;
        break;
    case GAIN_TWO:
        VOLTAGE_PER_BIT = ADS1115_GAIN_TWO;
        break;
    case GAIN_FOUR:
        VOLTAGE_PER_BIT = ADS1115_GAIN_FOUR;
        break;
    case GAIN_EIGHT:
        VOLTAGE_PER_BIT = ADS1115_GAIN_EIGHT;
        break;
    case GAIN_SIXTEEN:
        VOLTAGE_PER_BIT = ADS1115_GAIN_SIXTEEN;
        break;

    default:
        VOLTAGE_PER_BIT = 0.0f;
        Logger::Error("INVALID GAIN OPTION");
        break;
    }

    return (float)data * VOLTAGE_PER_BIT;
}

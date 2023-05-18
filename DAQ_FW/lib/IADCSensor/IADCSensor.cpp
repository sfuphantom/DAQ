#include "IADCSensor.h"

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

// Default constructor with generic metadata
IADCSensor::IADCSensor(const char *_SensorName, uint16_t _SensorID, uint16_t _ADC_ID, uint16_t _ADC_GAIN_MODE)
    : mSensorName(_SensorName), mSensorID(_SensorID), mADC_ID(_ADC_ID)
{
    ADCcovert(_ADC_GAIN_MODE);

    // obj declared when logger not active, could be hard to log
    Logger::Notice("Class created with gain mode: %u and V/per bit of %D", _ADC_GAIN_MODE, mVOLTAGE_PER_BIT);
}

float IADCSensor::Process()
{

    // TODO: put Read() function here once ready, currently for testing
    uint16_t raw_data = 5230;

    Logger::Trace("Initial Data: %u from sensor <%s> with ID: %u and ads id %u", raw_data, mSensorName, mSensorID, mADC_ID);

    // processing code goes here - to be done for different sensor types
    // converts voltage to sensor data, include warnings, errors

    float final_data = raw_data * mVOLTAGE_PER_BIT;

    Logger::Notice("Processed Data: %D, from sensor <%s> with ID: %u and adc id %u", final_data, mSensorName, mSensorID, mADC_ID);

    return (float)final_data;
}

// picks correct Voltage ber bit ration, according to the gain mode
void IADCSensor::ADCcovert(uint16_t _ADC_GAIN_MODE)
{
    switch (_ADC_GAIN_MODE)
    {
    case GAIN_TWOTHIRDS:
        mVOLTAGE_PER_BIT = ADS1115_GAIN_TWOTHIRDS;
        break;
    case GAIN_ONE:
        mVOLTAGE_PER_BIT = ADS1115_GAIN_ONE;
        break;
    case GAIN_TWO:
        mVOLTAGE_PER_BIT = ADS1115_GAIN_TWO;
        break;
    case GAIN_FOUR:
        mVOLTAGE_PER_BIT = ADS1115_GAIN_FOUR;
        break;
    case GAIN_EIGHT:
        mVOLTAGE_PER_BIT = ADS1115_GAIN_EIGHT;
        break;
    case GAIN_SIXTEEN:
        mVOLTAGE_PER_BIT = ADS1115_GAIN_SIXTEEN;
        break;

    default:
        mVOLTAGE_PER_BIT = 0.0f;
        Logger::Error("INVALID GAIN OPTION");
        break;
    }
}

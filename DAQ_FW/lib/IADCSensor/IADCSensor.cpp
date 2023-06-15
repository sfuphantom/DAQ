#include "IADCSensor.h"

// ADC INFO
// adc voltage 2V~5.5V, 860 sample rate, SERIAL output, 16 bit resolution, 15 µV min voltage increment
// i2c interface
// PGA offers input ranges from ±0.256 V to ±6.144 V
// adc resolution : 65536

// PGA VALUES for a ADS1115
// TODO: MOVE TO SYS_CONFIG WHEN FINAL
#define ADS1115_GAIN_TWOTHIRDS 0.187500f
#define ADS1115_GAIN_ONE 0.125000f
#define ADS1115_GAIN_TWO 0.062500f
#define ADS1115_GAIN_FOUR 0.031250f
#define ADS1115_GAIN_EIGHT 0.015625f
#define ADS1115_GAIN_SIXTEEN 0.007813f

// CLASS METHODS:

// Default constructor with generic metadata
IADCSensor::IADCSensor(const char *_SensorName, uint16_t _SensorID, uint16_t _ADC_ID)
    : mSensorName(_SensorName), mSensorID(_SensorID), mADC_ID(_ADC_ID) {}

// ADS1115 initialize func, to be called on setup()
void IADCSensor::Initialize()
{
    if (!mADS.begin(ADS1X15_ADDRESS))
    {
        while (1)
        {
            Logger::Error("Failed to start ads with ID: %u", mADC_ID);
        }
    }
    else
        Logger::Notice("ADC %u initialized", mADC_ID);
}

float IADCSensor::Process()
{
    // gets bit data from the adc
    int16_t raw_data = Read();

    Logger::Trace("Initial Data: %u from sensor <%s> with ID: %u and ads id %u", raw_data, mSensorName, mSensorID, mADC_ID);

    // processing code goes here - to be done for different sensor types
    // converts voltage to sensor data, include warnings, errors

    // adc bit to voltage conversion, gain mode can be set via the adc library
    float final_data = mADS.computeVolts(raw_data);

    Logger::Notice("Processed Data: %D, from sensor <%s> with ID: %u and adc id %u", final_data, mSensorName, mSensorID, mADC_ID);

    return (float)final_data;
}

int16_t IADCSensor::Read()
{
    Logger::Error("Default Read Function Called!");
    // testing read function - NOT FINAL
    int16_t adcBitData = mADS.readADC_SingleEnded(0);

    if (!adcBitData)
    {
        Logger::Warning("No Input Detected from sensor <%s> with ID: %u and adc id %u",
                        mSensorName, mSensorID, mADC_ID);
        return 0;
    }

    return (int16_t)adcBitData;
}

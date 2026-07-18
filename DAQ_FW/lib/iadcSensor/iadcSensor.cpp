#include "iadcSensor.h"
#include <math.h>

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
IADCSensor::IADCSensor(const char *_SensorName, const uint16_t _SensorID, const ADCAddress _ADCAddress, Adafruit_ADS1115 *adsDevice)
    : mADS(adsDevice), mSensorName(_SensorName), mSensorID(_SensorID), mADC_Address(_ADCAddress) {}

void IADCSensor::initialize(bool chipOnline) {
    if (mADS == nullptr || !chipOnline) {
        mInitialized = false;
        Logger::error("ADC %s unavailable for sensor %s", printAddress(), mSensorName);
        return;
    }

    mInitialized = true;
    Logger::notice("Sensor %s attached to ADC %s channel %u", mSensorName, printAddress(), mSensorID);
}

float IADCSensor::getData() {
    if (!mInitialized) {
        return NAN;
    }

    // gets bit data from the adc
    int16_t raw_data = read();

    Logger::trace("Initial Data: %d from sensor %s with ID: %u and ads component %s",
                  raw_data, mSensorName, mSensorID, printAddress());

    // adc bit to voltage conversion, gain mode can be set via the adc library
    float final_data = process(mADS->computeVolts(raw_data)); // process is overidden by the child class

    Logger::trace("Processed Data: %.4f, from sensor %s with ID: %u and adc component %s",
                  final_data, mSensorName, mSensorID, printAddress());

    return (float)final_data;
}

int16_t IADCSensor::read() {
    if (!mInitialized) {
        return 0;
    }

    int16_t adcBitData = mADS->readADC_SingleEnded(mSensorID);

    return (int16_t)adcBitData;
}

float ChildExample::process(float InputData) {
    Logger::error("Using example class");
    return InputData;
}

const char *IADCSensor::printAddress() {
    switch (mADC_Address) {
    case ADCAddress::U1:
        return "U1";
    case ADCAddress::U2:
        return "U2";
    case ADCAddress::U3:
        return "U3";
    case ADCAddress::U4:
        return "U4";
    default:
        return "<Error, invalid Address>";
    }
}

bool IADCSensor::isOnline() const {
    return mInitialized;
}

float CoolantTemperatureSensor::process(float inputData) {
    float temperature = convertToTemperature(inputData);
    return temperature;
}

float CoolantTemperatureSensor::convertToTemperature(float inputData) {
    // CTTS-302651-F01
    // Convert voltage -> thermistor resistance using a divider, then apply Beta equation.
    const float vRef = 5.0f;          // ADC reference / divider supply
    const float rFixed = 10000.0f;    // fixed resistor in divider (ohms)
    const float r25 = 10000.0f;       // thermistor resistance at 25°C (ohms)
    const float beta = 3435.0f;      // thermistor beta value (K)
    const float t25K = 298.15f;       // 25 celcius in Kelvin

    if (!isfinite(inputData)) return NAN;
    if (inputData <= 0.0f || inputData >= vRef) return NAN;

    // Assuming divider: vRef -> rFixed -> node -> thermistor -> GND.
    float rTherm = (rFixed * inputData) / (vRef - inputData);

    // Beta equation for NTC thermistor
    // temp goes up, resistance goes down
    float temperatureK = 1.0f / ((1.0f / t25K) + (1.0f / beta) * log(rTherm / r25));
    float temperature = temperatureK - 273.15f;

    return temperature;
}

float SteeringAngleSensor::process(float inputData) {
    return convertToAngleDegrees(inputData);
}

float SteeringAngleSensor::convertToAngleDegrees(float inputData) {
    if (!isfinite(inputData)) return NAN;
    if (STEERING_ANGLE_MAX_V <= STEERING_ANGLE_MIN_V) return NAN;

    float clampedVoltage = inputData;
    if (clampedVoltage < STEERING_ANGLE_MIN_V) {
        clampedVoltage = STEERING_ANGLE_MIN_V;
    }
    else if (clampedVoltage > STEERING_ANGLE_MAX_V) {
        clampedVoltage = STEERING_ANGLE_MAX_V;
    }

    float normalized = (clampedVoltage - STEERING_ANGLE_MIN_V) /
                       (STEERING_ANGLE_MAX_V - STEERING_ANGLE_MIN_V);
    return normalized * STEERING_ANGLE_FULL_SCALE_DEG;
}

float SuspensionSensor::process(float inputData) {
    if (!isfinite(inputData)) return NAN;
    return inputData;
}
